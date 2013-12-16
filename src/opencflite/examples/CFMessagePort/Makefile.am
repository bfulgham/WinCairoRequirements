AM_CFLAGS		= -I${top_srcdir}/include

if CF_BUILD_TESTS
EXTRA_PROGRAMS		= client server
endif

client_LDADD		= ${top_builddir}/libCoreFoundation.la
server_LDADD		= ${top_builddir}/libCoreFoundation.la

client_SOURCES		= client.c
server_SOURCES		= server.c
