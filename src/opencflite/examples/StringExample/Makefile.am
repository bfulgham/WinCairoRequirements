AM_CFLAGS		= -I${top_srcdir}/include

EXTRA_DIST		= StringExample.vcproj

if CF_BUILD_TESTS
check_PROGRAMS		= StringExample
endif

StringExample_LDADD	= ${top_builddir}/libCoreFoundation.la

StringExample_SOURCES	= StringExample.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./StringExample

gdb:
	${LIBTOOL} --mode execute ${@} ./StringExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./StringExample
endif
