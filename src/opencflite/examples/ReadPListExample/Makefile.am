AM_CXXFLAGS			= -I${top_srcdir}/include

EXTRA_DIST			= schema.xml \
				  ReadPListExample.vcproj

if CF_BUILD_TESTS
check_PROGRAMS			= ReadPListExample
endif

ReadPListExample_LDADD		= ${top_builddir}/libCoreFoundation.la

ReadPListExample_SOURCES	= ReadPListExample.cpp

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./ReadPListExample ${srcdir}/schema.xml

gdb:
	${LIBTOOL} --mode execute ${@} ./ReadPListExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./ReadPListExample
endif
