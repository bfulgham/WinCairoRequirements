AM_CXXFLAGS			= -I${top_srcdir}/include

EXTRA_DIST			= data/expected_schema.xml \
				  WritePListExample.vcproj

if CF_BUILD_TESTS
check_PROGRAMS			= WritePListExample
endif

WritePListExample_LDADD		= ${top_builddir}/libCoreFoundation.la

WritePListExample_SOURCES	= WritePListExample.cpp

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./WritePListExample
	${CMP} ${srcdir}/data/expected_schema.xml schema.xml

gdb:
	${LIBTOOL} --mode execute ${@} ./WritePListExample

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./WritePListExample
endif
