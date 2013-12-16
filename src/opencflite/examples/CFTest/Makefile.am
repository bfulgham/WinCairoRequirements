EXTRA_DIST			= Chiquitita.xml \
				  test.xml \
				  mac/CFTest.xcodeproj/project.pbxproj \
				  win_cw/MyMFCHeaders.pch++ \
				  win_cw/CFTest.mcp \
				  win_cw/MyMFCHeaders.h \
				  win_vs/CFTest.vcproj \
				  win_vs/CFTest.sln

noinst_HEADERS			= source/shared/CFUtils.h \
				  source/shared/SuperString.h \
				  source/main/CFTest.h \
				  source/main/stdafx.h

if CF_BUILD_TESTS
EXTRA_PROGRAMS			= CFTest
endif

CFTest_LDADD			= ${top_builddir}/libCoreFoundation.la

CFTest_CPPFLAGS			= -D_CFTEST_ \
				  -I${srcdir}/source/shared \
				  -I${top_srcdir}/include

CFTest_SOURCES			= source/main/main.cpp \
				  source/main/CFTest.cpp \
				  source/shared/SuperString.cpp \
				  source/shared/CFUtils.cpp

if CF_BUILD_TESTS
ddd gdb: CFTest
	${LIBTOOL} --mode execute ${@} ./CFTest

valgrind: CFTest
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./CFTest
endif
