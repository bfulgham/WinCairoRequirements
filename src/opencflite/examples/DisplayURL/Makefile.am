AM_CFLAGS		= -I${top_srcdir}/include

EXTRA_DIST		= DisplayURL.vcproj \
			  README.txt

if CF_BUILD_TESTS
check_PROGRAMS		= DisplayURL
endif

DisplayURL_LDADD	= ${top_builddir}/libCoreFoundation.la

DisplayURL_SOURCES	= DisplayURL.c

if CF_BUILD_TESTS
check:
	${LIBTOOL} --mode execute ./DisplayURL -u http://www.apple.com/
	${LIBTOOL} --mode execute ./DisplayURL -u http://developer.apple.com/referencelibrary/GettingStarted/GS_CoreFoundation/index.html
	${LIBTOOL} --mode execute ./DisplayURL -u http://opencflite.sourceforge.net/
	${LIBTOOL} --mode execute ./DisplayURL -u http://downloads.sourceforge.net:80/opencflite/opencflite-476.17.0.tar.gz
	${LIBTOOL} --mode execute ./DisplayURL -u ftp://anonymous:guest@downloads.sourceforge.net:21/opencflite/opencflite-476.17.0.tar.gz
	${LIBTOOL} --mode execute ./DisplayURL -u file://localhost/var/tmp
	${LIBTOOL} --mode execute ./DisplayURL -u file:///etc/motd

gdb:
	${LIBTOOL} --mode execute ${@} ./DisplayURL

valgrind:
	${LIBTOOL} --mode execute ${@} ${VALGRINDFLAGS} ./DisplayURL
endif
