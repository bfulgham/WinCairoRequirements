/*
 *    Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>
 *    Copyright (c) 2009 Brent Fulgham <bfulgham@gmail.com>
 *    All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the BSD-style license (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 */

/*
    File:       Common.h

    Contains:   Common code between client and server.

    Written by: DTS

    Copyright:  Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Change History (most recent first):

$Log: Common.h,v $
Revision 1.1  2005/05/17 12:19:20         
First checked in.


*/

#ifndef _COMMON_H
#define _COMMON_H

/////////////////////////////////////////////////////////////////

// System interfaces

#include <signal.h>

#include <CoreFoundation/CoreFoundation.h>

// Project interfaces

#include "Protocol.h"

#if defined(_WIN32)
// Windows doesn't understand UNIX sockets by default
struct sockaddr_un
{
   unsigned char sun_len;
   short sun_family;       // AF_UNIX
   char  sun_path[108];
};

#define SUN_LEN(su) \
   (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))

typedef int socklen_t
typedef long ssize_t;
#endif /* defined(_WIN32) */

/////////////////////////////////////////////////////////////////

extern int MoreUNIXErrno(int result);
extern int MoreUNIXIgnoreSIGPIPE(void);
extern int MoreUNIXRead( int fd,       void *buf, size_t bufSize, size_t *bytesRead   );
extern int MoreUNIXWrite(int fd, const void *buf, size_t bufSize, size_t *bytesWritten);
    // The above routines are taken from the MoreIsBetter library.

extern int MoreUNIXSetNonBlocking(int fd);
    
#if !defined(_WIN32)
typedef void (*SignalSocketCallback)(const siginfo_t *sigInfo, void *refCon);
    // This callback is called when a signal occurs.  It's called in the 
    // context of the runloop specified when you registered the callback.  
    // sigInfo describes the signal and refCon is the value you supplied 
    // when you registered the callback.

extern int InstallSignalToSocket(
	const sigset_t *		sigSet, 
	CFRunLoopRef			runLoop,
	CFStringRef				runLoopMode,
	SignalSocketCallback	callback, 
	void *					refCon
);
    // A method for routing signals to a runloop-based program. 
    //
    // sigSet is the set of signals that you're interested in. 
    // Use the routines documented in <x-man-page://3/sigsetopts> 
    // to construct this.
    //
    // runLoop and runLoopMode specify how you want the callback 
    // to be run.  You typically pass CFRunLoopGetCurrent and 
    // kCFRunLoopDefaultMode.
    //
    // callback is the routine you want called, and refCon is an 
    // uninterpreted value that's passed to that callback.
    //
    // The function result is an errno-style error code.
    //
    // IMPORTANT:
    // You can only call this routine once for any given application; 
    // you must register all of the signals you're interested in at that 
    // time.  There is no way to deregister.

#endif

extern void DebugPrintDescriptorTable(void);
    // Prints a nice dump of the file descriptor table to stdout.

extern void InitPacketHeader(PacketHeader *packet, OSType packetType, size_t packetSize, Boolean rpc);
    // Initialises the PacketHeader structure with the values specified 
    // in the parameters.  The rpc parameter controls whether the fID 
    // field is set to kPacketIDNone (rpc false, hence no ID) or an 
    // incrementing sequence number (rpc true).

#endif
