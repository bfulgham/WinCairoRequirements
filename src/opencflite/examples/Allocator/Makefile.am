AM_CFLAGS			= -I${top_srcdir}/include

EXTRA_DIST			= Allocator.vcproj

if CF_BUILD_TESTS
check_PROGRAMS			= AllocatorExample
endif

AllocatorExample_LDADD		= ${top_builddir}/libCoreFoundation.la

AllocatorExample_SOURCES	= AllocatorExample.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./AllocatorExample

gdb:
	${LIBTOOL} --mode execute ${@} ./AllocatorExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./AllocatorExample
endif
