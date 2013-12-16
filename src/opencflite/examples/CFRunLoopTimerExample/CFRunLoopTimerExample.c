/*
 *    Copyright (c) 2009 Nuovation System Designs, LLC
 *    All rights reserved.
 *
 *    Description:
 *      This file implements a trivial CFRunLoopTimer example that
 *      fires N timers every T[n] seconds for up to L seconds.
 */

/*
 * Copyright (c) 2003, Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <AssertMacros.h>

#include <CoreFoundation/CoreFoundation.h>

/* Type Definitions */

typedef struct _TimerData {
	CFIndex				mIndex;
	double				mInterval;
	struct {
		unsigned long	mDid;
		unsigned long	mShould;
	} mIterations;
	CFRunLoopTimerRef	mRef;
} TimerData;

typedef TimerData ** TimerContainerRef;

/*
 *  void TimerCallback()
 *
 *  Description:
 *    This routine is the callback handler for a CFRunLoopTimer. It
 *    simply prints out the time of day--in the current system time
 *    zone--and the iteration number and then increments the iteration
 *    number.
 *
 *  Input(s):
 *    timer - A CoreFoundation run loop timer reference to the timer
 *            associated with this callback.
 *    info  - A pointer to the callback data associated with this time
 *            when it was created. In this case, the iteration state.
 *
 *  Output(s):
 *    info  - The callback state with the iteration count incremented by
 *            one.
 *
 *  Returns:
 *    N/A
 *
 */
static void
TimerCallback(CFRunLoopTimerRef timer, void *info)
{
	TimerData *theData = (TimerData*)info;
    CFTimeZoneRef tz = NULL;
    CFGregorianDate theDate;

	(void)timer;

	tz = CFTimeZoneCopySystem();
	require(tz != NULL, done);

	theDate = CFAbsoluteTimeGetGregorianDate(CFAbsoluteTimeGetCurrent(), tz);

	printf("%04d-%02d-%02d %02d:%02d:%06.3f, timer: %lu, iteration: %lu\n",
		   theDate.year, theDate.month, theDate.day,
		   theDate.hour, theDate.minute, theDate.second,
		   theData->mIndex,
		   theData->mIterations.mDid++);

	CFRelease(tz);

 done:
	return;
}

/*
 *  TimerContainerRef TimerContainerCreate()
 *
 *  Description:
 *    This routine allocates a timer container capable of storing the
 *    specified number of timers.
 *
 *  Input(s):
 *    inElements - The size of the timer container to create.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    A timer container reference if OK; otherwise, NULL on error.
 *
 */
static TimerContainerRef
TimerContainerCreate(CFIndex inElements)
{
	return ((TimerContainerRef)CFAllocatorAllocate(kCFAllocatorSystemDefault,
												   inElements *
												   sizeof(TimerData *),
												   0));
}

/*
 *  TimerData * TimerContainerGet()
 *
 *  Description:
 *    This routine returns the timer data at the specified index from
 *    the timer container.
 *
 *  Input(s):
 *    inContainer - A reference to the timer container element to
 *                  return the element from.
 *    inIndex     - The timer container element to return.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    A timer data pointer if OK; otherwise, NULL.
 *
 */
static TimerData *
TimerContainerGet(TimerContainerRef inContainer, CFIndex inIndex)
{
	return (inContainer ? inContainer[inIndex] : NULL);
}

/*
 *  void TimerContainerSet()
 *
 *  Description:
 *    This routine sets the timer data at the specified index in
 *    the timer container.
 *
 *  Input(s):
 *    inContainer - A reference to the timer container element to
 *                  set the element in.
 *    inIndex     - The timer container element to set.
 *    inData      - A pointer to the timer data to set.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    A timer data pointer if OK; otherwise, NULL.
 *
 */
static void
TimerContainerSet(TimerContainerRef inContainer,
				  CFIndex inIndex,
				  TimerData *inData)
{
	inContainer[inIndex] = inData;
}

/*
 *  void TimerContainerDestroy()
 *
 *  Description:
 *    This routine deallocates the resources associated with the
 *    specified timer container.
 *
 *    NOTE: The container owner is responsible for deallocating any
 *    resources assigned to the container elements.
 *
 *  Input(s):
 *    inContainer - A reference to the timer container to deallocate.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    N/A
 *
 */
static void
TimerContainerDestroy(TimerContainerRef inContainer)
{
	CFAllocatorDeallocate(kCFAllocatorSystemDefault, inContainer);
}

/*
 *  long local_lround()
 *
 *  Description:
 *    This routine, provided by Steven Kargl (see copyright above)
 *    rounds the specified value to the nearest integer value,
 *    rounding away from zero, regardless of the current rounding
 *    direction.
 *
 *    NOTE: This function is implemented rather than just using C99's
 *    lround because this code MUST work with Microsoft Visual Studio
 *    2005 and 2008, both of which lack round or lround in their math
 *    libraries.
 *
 *  Input(s):
 *    x - The value to round.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    The rounded value.
 */
static
long local_lround(double x)
{
	double t;

	if (x >= 0.0) {
		t = ceilf(x);

		if (t - x > 0.5)
			t -= 1.0;

		return (long)t;

	} else {
		t = ceilf(-x);

		if (t + x > 0.5)
			t -= 1.0;

		return (long)-t;

	}
}

/*
 *  TimerData * TimerDataCreate()
 *
 *  Description:
 *    This routine allocates and initializes timer data, a wrapper
 *    around a CFRunLoopTimer.
 *
 *  Input(s):
 *    inIndex    - The instance or index of the allocated timer data.
 *    inLimit    - The maximum amount of time, in seconds, the timer
 *                 will be allowed to run.
 *    inInterval - A pointer to a NULL-terminated C string
 *                 representing the firing interval of the timer as a
 *                 floating point number.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    A pointer to the created timer data on success; otherwise, NULL
 *    on error.
 *
 */
static TimerData *
TimerDataCreate(CFIndex inIndex, double inLimit, const char *inInterval)
{
	double timerInterval = 0.0;
	unsigned long timerIterations = 0;
	char *end = NULL;
	TimerData *theData = NULL;
	CFRunLoopTimerRef theTimer = NULL;
    CFRunLoopTimerContext theContext = { 0, NULL, NULL, NULL, NULL };
 
	/* Parse and validate the timer interval. */

	timerInterval = strtod(inInterval, &end);
	verify(errno != ERANGE);

	if (errno == ERANGE || timerInterval <= 0) {
		fprintf(stderr, "Timer %lu interval must be greater than zero.\n",
				inIndex);
		goto done;
	}

	/* Compute the expected number of timer iterations. */

	timerIterations = local_lround(inLimit / timerInterval);

	/* Allocate storage for the timer data. */

	theData = (TimerData*)CFAllocatorAllocate(kCFAllocatorSystemDefault,
											  sizeof (TimerData),
											  0);
	require(theData != NULL, done);

	theContext.info = theData;

	/* Create a CoreFoundation run loop timer. */

	theTimer = CFRunLoopTimerCreate(kCFAllocatorDefault,
									CFAbsoluteTimeGetCurrent() + timerInterval,
									timerInterval,
									0,
									0,
									TimerCallback,
									&theContext);
	require(theTimer != NULL, fail);

	printf("Will fire timer %lu every %g seconds for %g seconds, "
		   "up to %lu time%s.\n",
		   inIndex, timerInterval, inLimit, timerIterations,
		   timerIterations == 1 ? "" : "s");

	/* Initialize timer data members. */

	theData->mIndex					= inIndex;
	theData->mInterval				= timerInterval;
	theData->mIterations.mDid		= 0;
	theData->mIterations.mShould	= timerIterations;
	theData->mRef					= theTimer;

 done:
	return (theData);

 fail:
	CFAllocatorDeallocate(kCFAllocatorSystemDefault, theData);

	return (NULL);
}

/*
 *  void TimerDataDestroy()
 *
 *  Description:
 *    This routine deallocates the resources associated with
 *    previously-allocated timer data.
 *
 *  Input(s):
 *    inData - A pointer to the timer data to deallocate.
 *
 *  Output(s):
 *    N/A
 *
 *  Returns:
 *    N/A
 *
 */
static void
TimerDataDestroy(TimerData *inData)
{
	verify_action(inData != NULL, return);

	if (inData->mRef != NULL) {
		CFRunLoopTimerInvalidate(inData->mRef);
		CFRelease(inData->mRef);
	}

	CFAllocatorDeallocate(kCFAllocatorSystemDefault, inData);
}

int
main(int argc, const char * const argv[])
{
	int status = 0;
	int i, timerCount = 0;
	double timerLimit = 0;
	char *end = NULL;
	TimerData *theTimer;
	TimerContainerRef timerContainer;
	CFStringRef theMode = CFSTR("TimerMode");

	/*
	 * Perform a basic sanity check on usage: at least one timer and a limit.
	 */

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <interval 1 / s> [<interval 2 / s> ...] "
				"<limit / s>\n", argv[0]);
		goto done;
	}

	/* Convert and check the specified limit value. */

	timerLimit = strtod(argv[argc - 1], &end);
	verify(errno != ERANGE);

	if (errno == ERANGE || timerLimit <= 0) {
		fprintf(stderr, "Timer limit must be greater than zero.\n");
		goto done;
	}

	timerCount = argc - 2;

	/*
	 * Allocate a container for all the timers we are going to
	 * allocate and run.
	 */

	timerContainer = TimerContainerCreate(timerCount);

	printf("Will fire a total of %d timer%s for %g seconds.\n",
		   timerCount, ((timerCount == 1) ? "" : "s"), timerLimit);

	/* Allocate each timer and add it to the container. */

	for (i = 0; i < timerCount; i++) {
		theTimer = TimerDataCreate(i, timerLimit, argv[i + 1]);
		require(theTimer != NULL, finish);

		TimerContainerSet(timerContainer, i, theTimer);

		CFRunLoopAddTimer(CFRunLoopGetCurrent(), theTimer->mRef, theMode);

	}

	/* Run the timers */

	CFRunLoopRunInMode(theMode, timerLimit, false);
	CFRunLoopRun();

	/* Determine the run results and deallocate resources. */

 finish:
	for (i = 0; i < timerCount; i++) {
		bool result;
		theTimer = TimerContainerGet(timerContainer, i);

		result = (theTimer->mIterations.mShould >= theTimer->mIterations.mDid);

		if (!result) {
			fprintf(stderr,
					"Timer %lu fired %lu of the expected %lu times.\n",
					theTimer->mIndex,
					theTimer->mIterations.mDid,
					theTimer->mIterations.mShould);
		}

		status += result;

		TimerDataDestroy(theTimer);
	}

	TimerContainerDestroy(timerContainer);

 done:
	return ((status == timerCount) ? EXIT_SUCCESS : EXIT_FAILURE);
}
