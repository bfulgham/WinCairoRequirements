AM_CFLAGS		= -I${top_srcdir}/include

EXTRA_DIST		= Make_win32.bat

if CF_BUILD_TESTS
check_PROGRAMS		= date_test
endif

date_test_LDADD		= ${top_builddir}/libCoreFoundation.la

date_test_SOURCES	= date_test.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./date_test

gdb:
	${LIBTOOL} --mode execute ${@} ./date_test

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./date_test
endif
