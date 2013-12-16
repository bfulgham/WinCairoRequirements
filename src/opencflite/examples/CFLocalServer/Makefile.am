AM_CFLAGS		= -I${top_srcdir}/include

if CF_BUILD_TESTS
check_PROGRAMS		= Client Server
check_LTLIBRARIES	= libCommon.la
endif

Client_LDADD		= ${top_builddir}/libCoreFoundation.la libCommon.la
Server_LDADD		= ${top_builddir}/libCoreFoundation.la libCommon.la

Client_SOURCES		= Client.c
Server_SOURCES		= Server.c

libCommon_la_SOURCES	= Common.c

noinst_HEADERS		= Common.h Protocol.h
