AM_CFLAGS			= -I${top_srcdir}/include

EXTRA_DIST			= DNSServiceMetaQuery.vcproj \
				  README.txt

if CF_BUILD_TESTS
EXTRA_PROGRAMS			= DNSServiceMetaQuery
endif

DNSServiceMetaQuery_LDADD	= ${top_builddir}/libCoreFoundation.la -ldns_sd

DNSServiceMetaQuery_SOURCES	= DNSServiceMetaQuery.c
