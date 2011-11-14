ACLOCAL_AMFLAGS = -I m4

# We need to ensure that the top-level (i.e. source) directory is
# built before tests and examples, so we explicitly add '.' to the
# list of subdirectories.l

SUBDIRS				= include compat . tests examples

#
# We don't want to modify any of the package source to include the
# configuration header, so we use the compiler to include it as a
# prefix header. This is likely GCC specific, so we need to probably
# abstract this further as other deployment targets are supported.
#

DEFS				+= -include ${CONFIG_HEADER}

#
# Data file for pkg-config
#

pkgconfigdir			= ${libdir}/pkgconfig
pkgconfig_DATA			= CoreFoundation.pc

#
#
#

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

#
# Define where we will install and where CoreFoundation can find its
# Unicode character mappings.
#

libCoreFoundation_la_datadir	= ${datadir}/CoreServices

libCoreFoundation_la_data_DATA	= CFCharacterSetBitmaps.bitmap		\
				  CFUniCharPropertyDatabase.data	\
				  CFUnicodeData-B.mapping		\
				  CFUnicodeData-L.mapping		\
				  Info.plist

CF_COMMON_CPPFLAGS		= $(AM_CPPFLAGS)			\
				  -DU_SHOW_DRAFT_API=1			\
				  -DCF_BUILDING_CF=1			\
				  -D__kCFCharacterSetDir=\"${libCoreFoundation_la_datadir}\" \
				  -DMAC_OS_X_VERSION_MAX_ALLOWED=MAC_OS_X_VERSION_10_7	\
				  -DTARGET_OS_LINUX			\
				  -DDEPLOYMENT_TARGET_LINUX		\
				  -I$(top_srcdir)/include		\
				  -I$(top_srcdir)/include/mach_support

CF_COMMON_CFLAGS		= $(AM_CFLAGS)

CF_COMMON_LDFLAGS		= $(AM_LDFLAGS)				\
				  -version-info $(CF_VERSION_INFO)

CF_COMMON_LIBADD		= $(AM_LDADD) compat/libcompat.la 

noinst_HEADERS			= auto_stubs.h				\
				  CFBundle_BinaryTypes.h		\
				  CFBundle_Internal.h			\
				  CFInternal.h				\
				  CFUniCharPriv.h			\
				  CFWindowsMessageQueue.h		\
				  CFXMLInputStream.h

CF_includedir			= ${includedir}/CoreFoundation

CF_include_HEADERS		= CFArray.h				\
				  CFBag.h				\
				  CFBase.h				\
				  CFBinaryHeap.h			\
				  CFBitVector.h				\
				  CFBundle.h				\
				  CFBundlePriv.h			\
				  CFByteOrder.h				\
				  CFCalendar.h				\
				  CFCharacterSet.h			\
				  CFCharacterSetPriv.h			\
				  CFData.h				\
				  CFDateFormatter.h			\
				  CFDate.h				\
				  CFDictionary.h			\
				  CFError.h				\
				  CFError_Private.h			\
				  CFFileDescriptor.h			\
				  CFLocale.h				\
				  CFLogUtilities.h			\
				  CFMachPort.h				\
				  CFMessagePort.h			\
				  CFNotificationCenter.h		\
				  CFNumberFormatter.h			\
				  CFNumber.h				\
				  CFPlugInCOM.h				\
				  CFPlugIn_Factory.h			\
				  CFPlugIn.h				\
				  CFPreferences.h			\
				  CFPriv.h				\
				  CFPropertyList.h			\
				  CFRunLoop.h				\
				  CFRuntime.h				\
				  CFSet.h				\
				  CFSocket.h				\
				  CFStorage.h				\
				  CFStreamAbstract.h			\
				  CFStream.h				\
				  CFStreamInternal.h			\
				  CFStreamPriv.h			\
				  CFStringDefaultEncoding.h		\
				  CFStringEncodingConverterExt.h	\
				  CFStringEncodingConverter.h		\
				  CFStringEncodingConverterPriv.h	\
				  CFStringEncodingExt.h			\
				  CFString.h				\
				  CFTimeZone.h				\
				  CFTree.h				\
				  CFUniChar.h				\
				  CFUnicodeDecomposition.h		\
				  CFUnicodePrecomposition.h		\
				  CFURLAccess.h				\
				  CFURL.h				\
				  CFUserNotification.h			\
				  CFUUID.h				\
				  CFXMLNode.h				\
				  CFXMLParser.h				\
				  CoreFoundation.h			\
				  ForFoundationOnly.h

CF_COMMON_SOURCES		= CFApplicationPreferences.c		\
				  CFArray.c				\
				  CFBag.c				\
				  CFBase.c				\
				  CFBasicHash.c				\
				  CFBinaryHeap.c			\
				  CFBinaryPList.c			\
				  CFBitVector.c				\
				  CFBuiltinConverters.c			\
				  CFBundle.c				\
				  CFBundle_Resources.c			\
				  CFCalendar.c				\
				  CFCharacterSet.c			\
				  CFConcreteStreams.c			\
				  CFData.c				\
				  CFDateFormatter.c			\
				  CFDate.c				\
				  CFDictionary.c			\
				  CFError.c				\
				  CFFileDescriptor.c			\
				  CFFileUtilities.c			\
				  CFICUConverters.c			\
				  CFLocaleIdentifier.c			\
				  CFLocale.c				\
				  CFLocaleKeys.c			\
				  CFMachPort.c				\
				  CFMessagePort.c			\
				  CFNotificationCenter.c		\
				  CFNumberFormatter.c			\
				  CFNumber.c				\
				  CFPlatform.c				\
				  CFPlatformConverters.c		\
				  CFPlugIn_Factory.c			\
				  CFPlugIn_Instance.c			\
				  CFPlugIn.c				\
				  CFPlugIn_PlugIn.c			\
				  CFPreferences.c			\
				  CFPropertyList.c			\
				  CFRunLoop.c				\
				  CFRuntime.c				\
				  CFSet.c				\
				  CFSocket.c				\
				  CFSocketStream.c			\
				  CFSortFunctions.c			\
				  CFStorage.c				\
				  CFStream.c				\
				  CFStringEncodingConverter.c		\
				  CFStringEncodingDatabase.c		\
				  CFStringEncodings.c			\
				  CFString.c				\
				  CFStringScanner.c			\
				  CFStringUtilities.c			\
				  CFSystemDirectories.c			\
				  CFTimeZone.c				\
				  CFTree.c				\
				  CFURL.c				\
				  CFURLAccess.c				\
				  CFUUID.c				\
				  CFUniChar.c				\
				  CFUnicodeDecomposition.c		\
				  CFUnicodePrecomposition.c		\
				  CFUserNotification.c			\
				  CFUtilities.c				\
				  CFVersion.c				\
				  CFWindowsMessageQueue.c		\
				  CFXMLInputStream.c			\
				  CFXMLNode.c				\
				  CFXMLParser.c				\
				  CFXMLPreferencesDomain.c		\
				  CFXMLTree.c

#				  Cocotron.c

lib_LTLIBRARIES 			= libCoreFoundation.la

libCoreFoundation_la_CPPFLAGS		= $(CF_COMMON_CPPFLAGS)
libCoreFoundation_la_CFLAGS		= $(CF_COMMON_CFLAGS)
libCoreFoundation_la_LDFLAGS		= $(CF_COMMON_LDFLAGS)
libCoreFoundation_la_LIBADD		= $(CF_COMMON_LIBADD)
libCoreFoundation_la_SOURCES		= $(CF_COMMON_SOURCES)

#
# Only generate a debug instance of the library if --enable-debug was
# passed at configuration time.
#

if CF_BUILD_DEBUG
lib_LTLIBRARIES				+= libCoreFoundation_debug.la

libCoreFoundation_debug_la_CPPFLAGS	= $(CF_COMMON_CPPFLAGS) -DDEBUG
libCoreFoundation_debug_la_CFLAGS	= $(CF_COMMON_CFLAGS)
libCoreFoundation_debug_la_LDFLAGS	= $(CF_COMMON_LDFLAGS)
libCoreFoundation_debug_la_LIBADD	= $(CF_COMMON_LIBADD)
libCoreFoundation_debug_la_SOURCES	= $(CF_COMMON_SOURCES)
endif

#
# Only generate a profile instance of the library if --enable-debug was
# passed at configuration time.
#

if CF_BUILD_PROFILE
lib_LTLIBRARIES				+= libCoreFoundation_profile.la

libCoreFoundation_profile_la_CPPFLAGS	= $(CF_COMMON_CPPFLAGS) -DPROFILE
libCoreFoundation_profile_la_CFLAGS	= $(CF_COMMON_CFLAGS) -pg
libCoreFoundation_profile_la_LDFLAGS	= $(CF_COMMON_LDFLAGS)
libCoreFoundation_profile_la_LIBADD	= $(CF_COMMON_LIBADD)
libCoreFoundation_profile_la_SOURCES	= $(CF_COMMON_SOURCES)
endif

EXTRA_DIST			= APPLE_LICENSE	\
				  CHANGES \
				  CONTRIBUTORS \
				  INSTALL \
				  LICENSE \
				  README \
				  TODO \
				  BuildCFLite \
				  BuildCFLite.windows \
				  macosx/CFLite.xcodeproj/project.pbxproj \
				  windows/CFLiteLib.vcproj \
				  CFLite.sln \
				  Cocotron/Cocotron.c \
				  Cocotron/CoreFoundation.xcodeproj/project.pbxproj \
				  icu/bin/icudt40.dll \
				  icu/bin/icuin40.dll \
				  icu/bin/icuuc40.dll \
				  icu/bin/WHAT_IS_THIS.txt \
				  icu/lib/icuin.lib \
				  icu/lib/icuuc.lib \
				  icu/lib/WHAT_IS_THIS.txt \
				  icu/unicode/basictz.h \
				  icu/unicode/brkiter.h \
				  icu/unicode/calendar.h \
				  icu/unicode/caniter.h \
				  icu/unicode/chariter.h \
				  icu/unicode/choicfmt.h \
				  icu/unicode/coleitr.h \
				  icu/unicode/coll.h \
				  icu/unicode/curramt.h \
				  icu/unicode/currunit.h \
				  icu/unicode/datefmt.h \
				  icu/unicode/dbbi.h \
				  icu/unicode/dcfmtsym.h \
				  icu/unicode/decimfmt.h \
				  icu/unicode/docmain.h \
				  icu/unicode/dtfmtsym.h \
				  icu/unicode/dtintrv.h \
				  icu/unicode/dtitvfmt.h \
				  icu/unicode/dtitvinf.h \
				  icu/unicode/dtptngen.h \
				  icu/unicode/dtrule.h \
				  icu/unicode/fieldpos.h \
				  icu/unicode/fmtable.h \
				  icu/unicode/format.h \
				  icu/unicode/gregocal.h \
				  icu/unicode/locid.h \
				  icu/unicode/measfmt.h \
				  icu/unicode/measunit.h \
				  icu/unicode/measure.h \
				  icu/unicode/msgfmt.h \
				  icu/unicode/normlzr.h \
				  icu/unicode/numfmt.h \
				  icu/unicode/parseerr.h \
				  icu/unicode/parsepos.h \
				  icu/unicode/platform.h \
				  icu/unicode/plurfmt.h \
				  icu/unicode/plurrule.h \
				  icu/unicode/ppalmos.h \
				  icu/unicode/putil.h \
				  icu/unicode/pwin32.h \
				  icu/unicode/rbbi.h \
				  icu/unicode/rbnf.h \
				  icu/unicode/rbtz.h \
				  icu/unicode/regex.h \
				  icu/unicode/rep.h \
				  icu/unicode/resbund.h \
				  icu/unicode/schriter.h \
				  icu/unicode/search.h \
				  icu/unicode/simpletz.h \
				  icu/unicode/smpdtfmt.h \
				  icu/unicode/sortkey.h \
				  icu/unicode/strenum.h \
				  icu/unicode/stsearch.h \
				  icu/unicode/symtable.h \
				  icu/unicode/tblcoll.h \
				  icu/unicode/timezone.h \
				  icu/unicode/translit.h \
				  icu/unicode/tzrule.h \
				  icu/unicode/tztrans.h \
				  icu/unicode/ubidi.h \
				  icu/unicode/ubrk.h \
				  icu/unicode/ucal.h \
				  icu/unicode/ucasemap.h \
				  icu/unicode/ucat.h \
				  icu/unicode/uchar.h \
				  icu/unicode/uchriter.h \
				  icu/unicode/uclean.h \
				  icu/unicode/ucnv_cb.h \
				  icu/unicode/ucnv_err.h \
				  icu/unicode/ucnv.h \
				  icu/unicode/ucoleitr.h \
				  icu/unicode/ucol.h \
				  icu/unicode/uconfig.h \
				  icu/unicode/ucsdet.h \
				  icu/unicode/ucurr.h \
				  icu/unicode/udata.h \
				  icu/unicode/udat.h \
				  icu/unicode/udatpg.h \
				  icu/unicode/udeprctd.h \
				  icu/unicode/udraft.h \
				  icu/unicode/uenum.h \
				  icu/unicode/uidna.h \
				  icu/unicode/uintrnal.h \
				  icu/unicode/uiter.h \
				  icu/unicode/ulocdata.h \
				  icu/unicode/uloc.h \
				  icu/unicode/umachine.h \
				  icu/unicode/umisc.h \
				  icu/unicode/umsg.h \
				  icu/unicode/unifilt.h \
				  icu/unicode/unifunct.h \
				  icu/unicode/unimatch.h \
				  icu/unicode/unirepl.h \
				  icu/unicode/uniset.h \
				  icu/unicode/unistr.h \
				  icu/unicode/unorm.h \
				  icu/unicode/unum.h \
				  icu/unicode/uobject.h \
				  icu/unicode/uobslete.h \
				  icu/unicode/urbtok.h \
				  icu/unicode/uregex.h \
				  icu/unicode/urename.h \
				  icu/unicode/urep.h \
				  icu/unicode/ures.h \
				  icu/unicode/uscript.h \
				  icu/unicode/usearch.h \
				  icu/unicode/uset.h \
				  icu/unicode/usetiter.h \
				  icu/unicode/ushape.h \
				  icu/unicode/usprep.h \
				  icu/unicode/ustdio.h \
				  icu/unicode/ustream.h \
				  icu/unicode/ustring.h \
				  icu/unicode/usystem.h \
				  icu/unicode/utext.h \
				  icu/unicode/utf16.h \
				  icu/unicode/utf32.h \
				  icu/unicode/utf8.h \
				  icu/unicode/utf.h \
				  icu/unicode/utf_old.h \
				  icu/unicode/utmscale.h \
				  icu/unicode/utrace.h \
				  icu/unicode/utrans.h \
				  icu/unicode/utypes.h \
				  icu/unicode/uversion.h \
				  icu/unicode/vtzone.h \
				  Makefile \
				  XCode_Configs/ConfigDebug.xcconfig \
				  XCode_Configs/Config_Master.xcconfig \
				  XCode_Configs/ConfigRelease.xcconfig \
				  ${libCoreFoundation_la_data_DATA}

