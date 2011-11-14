AM_CFLAGS			= -I${top_srcdir}/include

EXTRA_DIST			= CFRunLoopTimerExample.vcproj

if CF_BUILD_TESTS
check_PROGRAMS			= CFRunLoopTimerExample
endif

CFRunLoopTimerExample_LDADD	= ${top_builddir}/libCoreFoundation.la

CFRunLoopTimerExample_SOURCES	= CFRunLoopTimerExample.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.25  5.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.40  2.30
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 1.00  1.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 1.00  0.50
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.10  1.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 1.00  2.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 1.00 10.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.33  1.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.50  0.33 4.00
	${LIBTOOL} --mode execute ./CFRunLoopTimerExample 0.50  1.00 2.00 8.00

ddd gdb:
	${LIBTOOL} --mode execute ${@} ./CFRunLoopTimerExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./CFRunLoopTimerExample
endif
