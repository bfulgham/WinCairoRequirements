AM_CFLAGS		= -I${top_srcdir}/include

EXTRA_DIST		= BundleExample.vcproj

if CF_BUILD_TESTS
check_PROGRAMS		= BundleExample
endif

BundleExample_LDADD	= ${top_builddir}/libCoreFoundation.la

BundleExample_SOURCES	= BundleExample.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./BundleExample

gdb:
	${LIBTOOL} --mode execute ${@} ./BundleExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./BundleExample
endif
