all: CoreFoundation

check install: all

CS_includedir		= ${includedir}

CS_include_HEADERS	= AssertMacros.h		\
			  Availability.h		\
			  AvailabilityMacros.h		\
			  ConditionalMacros.h		\
			  TargetConditionals.h

EXTRA_DIST		= c99/stdbool.h			\
			  c99/stdint.h			\
			  mach_support/bootstrap_priv.h	\
			  mach_support/vproc_priv.h

CF_SOURCE		= ${top_srcdir}

CoreFoundation: | ${CF_SOURCE}
	${LN_S} ${CF_SOURCE} $@
