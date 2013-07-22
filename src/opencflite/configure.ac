#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#
# Declare autoconf version requirements
#
AC_PREREQ(2.61)

#
# Initialize autoconf for the package
#
# Package release version conventions are x.y.z, where x and y track
# the Apple CoreFoundation release version and z tracks the OpenCFLite
# release thereof.
#
AC_INIT([CoreFoundation],[635.1.0],[bfulgham@users.sourceforge.net],[opencflite])

#
# Interface number, revision and age of the library interface passed to
# libtool when the library is linked.
#
AC_SUBST(CF_VERSION_INFO, [635:1:0])

#
# Check the sanity of thes source directory by checking for the
# presence of a key watch file
#
AC_CONFIG_SRCDIR([CoreFoundation.h])

#
# Tell autoconf where to find auxilliary build tools (e.g. config.guess,
# install-sh, missing, etc.)
#
AC_CONFIG_AUX_DIR([config])

#
# Tell autoconf where to find auxilliary M4 macros
#
AC_CONFIG_MACRO_DIR([m4])

#
# Tell autoconf what file the package is using to aggregate C preprocessor
# defines.
#
AC_CONFIG_HEADERS([include/config.h])

#
# Tell autoconf where to find source files for interfaces that are missing
# in the target environment (see AC_REPLACE_FUNCS below).
#
AC_CONFIG_LIBOBJ_DIR([compat])

#
# Initialize automake for the package, specifying the minimum automake
# version allowed and that we are not an official GNU package, but
# rather a non-GNU package leveraging automake.
#
AM_INIT_AUTOMAKE([1.9 foreign])

#
# Enable maintainer mode to prevent the package from constantly trying
# to rebuild configure, Makefile.in, etc. Rebuilding such files rarely,
# if ever, needs to be done "in the field".
#
AM_MAINTAINER_MODE

#
# Figure out what the canonical build, host and target tuples are.
#
AC_CANONICAL_HOST

AC_MSG_CHECKING([for supported deployment target])

case ${host_os} in
	*darwin*)
		CPPFLAGS="-DDEPLOYMENT_TARGET_MACOSX=1 -DTARGET_OS_MAC=1 ${CPPFLAGS}"
		deployment_target="Mac OS X"
		;;

	*freebsd*)
		CPPFLAGS="-DDEPLOYMENT_TARGET_FREEBSD=1 -DTARGET_OS_FREEBSD=1 ${CPPFLAGS}"
		deployment_target="FreeBSD"
		;;

	*linux*)
		CPPFLAGS="-DDEPLOYMENT_TARGET_LINUX=1 -DTARGET_OS_LINUX=1 ${CPPFLAGS}"
		deployment_target="Linux"
		;;

	*mingw*)
		CPPFLAGS="-DDEPLOYMENT_TARGET_WINDOWS=1 -DTARGET_OS_WIN32=1 ${CPPFLAGS}"
		deployment_target="Windows"
		;;

	*)
		AC_MSG_RESULT([failed])
		AC_MSG_ERROR([Unsupported deployment target ${host_os}]);
		;;
esac

AC_MSG_RESULT(${deployment_target})

#
# Checks for programs
#
AC_PROG_CC
AC_PROG_CXX
AC_PROG_OBJC
CF_BSD_SOURCE
AC_GNU_SOURCE
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PATH_PROG(CMP, cmp)

LT_PREREQ([2.2])
LT_INIT
AC_SUBST([LIBTOOL_DEPS])


# Support building DLLs on Windows
AC_LIBTOOL_WIN32_DLL

#
# Checks for language qualifiers and semantics
#
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_VOLATILE

#
# Checks for compiler characteristics
#

# If using GCC specify some additional parameters

test_gcc_flag() {
    AC_LANG_CONFTEST([int main() {}])
    $CC -c conftest.c $CFLAGS $@ > /dev/null 2> /dev/null
    ret=$?
    rm -f conftest.o
    return $ret
}

if test "x$GCC" = "xyes" ; then
#    DESIRED_CFLAGS="-Wall -Wmost -pipe -Wno-trigraphs -Wno-parentheses -fexceptions -std=gnu99 -xc -mmacosx-version-min=10.5 -fconstant-cfstrings"
    DESIRED_CFLAGS="-Wall -Wmost -pipe -Wno-trigraphs -Wno-parentheses -fexceptions -std=gnu99 -mmacosx-version-min=10.5 -fconstant-cfstrings"

    for flag in $DESIRED_CFLAGS ; do
        AC_MSG_CHECKING([whether ${CC} accepts ${flag}])
        if test_gcc_flag $flag ; then
           CFLAGS="$CFLAGS $flag"
           AC_MSG_RESULT([yes])
        else
           AC_MSG_RESULT([no])
        fi
    done

     OBJCFLAGS="$CFLAGS"
fi

# Check whether the current compiler supports atomic built-ins

AC_MSG_NOTICE([checking for compiler atomic built-ins])

AC_CACHE_CHECK([whether the target supports __sync_fetch_and_add],
	       [cf_cv_have_sync_fetch_and_add],
	       [
			AC_LINK_IFELSE([
				AC_LANG_PROGRAM([[
				]],[[
					int foo = 0;
					return __sync_fetch_and_add(&foo, 1);
				]])],
				[cf_cv_have_sync_fetch_and_add=yes],
				[cf_cv_have_sync_fetch_and_add=no])
		])

if test "x${cf_cv_have_sync_fetch_and_add}" != "xno"; then
	AC_DEFINE(HAVE___SYNC_FETCH_AND_ADD, 1,
		  [Define this if your target compiler supports __sync_fetch_and_add])
else
	AC_MSG_ERROR([Support for __sync_fetch_and_add is required.])
fi

AC_CACHE_CHECK([whether the target supports __sync_bool_compare_and_swap],
	       [cf_cv_have_sync_bool_compare_and_swap],
	       [
			AC_LINK_IFELSE([
				AC_LANG_PROGRAM([[
				]],[[
					int foo;
					return __sync_bool_compare_and_swap(&foo, 0, 1);
				]])],
				[cf_cv_have_sync_bool_compare_and_swap=yes],
				[cf_cv_have_sync_bool_compare_and_swap=no])
		])

if test "x${cf_cv_have_sync_bool_compare_and_swap}" != "xno"; then
	AC_DEFINE(HAVE___SYNC_BOOL_COMPARE_AND_SWAP, 1,
		  [Define this if your target compiler supports __sync_bool_compare_and_swap])
else
	AC_MSG_ERROR([Support for __sync_bool_compare_and_swap is required.])
fi

AC_CACHE_CHECK([whether the target supports __sync_synchronize],
	       [cf_cv_have_sync_synchronize],
	       [
			AC_LINK_IFELSE([
				AC_LANG_PROGRAM([[
				]],[[
					__sync_synchronize();
				]])],
				[cf_cv_have_sync_synchronize=yes],
				[cf_cv_have_sync_synchronize=no])
		])

if test "x${cf_cv_have_sync_synchronize}" != "xno"; then
	AC_DEFINE(HAVE___SYNC_SYNCHRONIZE, 1,
		  [Define this if your target compiler supports __sync_synchronize])
else
	AC_MSG_ERROR([Support for __sync_synchronize is required.])
fi

#
# Check whether or not we should build the package with debug and profile
# versions of the library
#

AC_MSG_NOTICE([checking library build options])

AC_MSG_CHECKING([whether to build a debug library instance])
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--disable-debug], [Disable the generation of a debug library instance @<:@default=no@:>@.]),
	[build_debug=${enableval}],
	[build_debug=yes])
AC_MSG_RESULT(${build_debug})
AM_CONDITIONAL([CF_BUILD_DEBUG], [test "x${build_debug}" = "xyes"])

AC_MSG_CHECKING([whether to build a profile library instance])
AC_ARG_ENABLE(profile,
	AS_HELP_STRING([--disable-profile], [Disable the generation of a profile library instance @<:@default=no@:>@.]),
	[build_profile=${enableval}],
	[build_profile=yes])
AC_MSG_RESULT(${build_profile})
AM_CONDITIONAL([CF_BUILD_PROFILE], [test "x${build_profile}" = "xyes"])

AC_MSG_CHECKING([whether to build tests and examples])
AC_ARG_ENABLE(tests,
	AS_HELP_STRING([--disable-tests],[Disable building of tests and examples @<:@default=no@:>@.]),
	[build_tests=${enableval}],
	[build_tests=yes])
AC_MSG_RESULT(${build_tests})
AM_CONDITIONAL([CF_BUILD_TESTS], [test "x${build_tests}" = "xyes"])

#
# Checks for libraries
#
# We expect that we'll need at minimum:
#
#   * dl
#   * icui18n
#   * m
#   * pthread
#   * uuid
#
# and, maybe:
#
#   * rt
#

AC_MSG_NOTICE([checking required package dependencies])

AC_PATH_PROG([PKG_CONFIG],[pkg-config])

#
# Sort out where the ICU components are at.
#

AC_MSG_NOTICE([checking the International Components for Unicode (ICU) package])

AC_PATH_PROG([ICU_CONFIG],[icu-config])

AC_ARG_WITH(icu,
	AS_HELP_STRING([--with-icu=DIR],
		       [Specify location of the International Components for Unicode (ICU) headers and libraries.]),
	[
		if test "x${withval}" = "xno"; then
			AC_MSG_ERROR([${PACKAGE_NAME} requires the International Components for Unicode (ICU) package])
		else
			icu_dir=${withval}
		fi
	],
	[icu_dir=])
AC_ARG_WITH(icu-includes,
	AS_HELP_STRING([--with-icu-includes=DIR],
		       [Specify location of International Components for Unicode (ICU) headers.]),
	[icu_header_dir=${withval}],
	[icu_header_dir=])
AC_ARG_WITH(icu-libs,
	AS_HELP_STRING([--with-icu-libs=DIR],
		       [Specify location of International Components for Unicode (ICU) libraries.]),
	[icu_library_dir=${withval}],
	[icu_library_dir=])

# We always prefer checking the values of the various '--with-icu-...' 
# options first to using icu-config because the former might be used
# in a cross-compilation environment on a system that also contains
# icu-config. In such a case, the user wants what he/she specified
# rather than what icu-config indicates.

if test "x${icu_dir}" != "x" -o "x${icu_header_dir}" != "x" -o "x${icu_library_dir}" != "x"; then
	if test "x${icu_dir}" != "x"; then
		if test -d "${icu_dir}"; then
			if test -d "${icu_dir}/include"; then
				ICU_CPPFLAGS="-I${icu_dir}/include"
			else
				ICU_CPPFLAGS="-I${icu_dir}"
			fi

			if test -d "${icu_dir}/lib"; then
				ICU_LDFLAGS="-L${icu_dir}/lib"
			else
				ICU_LDFLAGS="-L${icu_dir}"
			fi
		else
			AC_MSG_ERROR([No such directory ${icu_dir}])
		fi
	fi

	if test "x${icu_header_dir}" != "x"; then
		if test -d "${icu_header_dir}"; then
			ICU_CPPFLAGS="-I${icu_header_dir}"
		else
			AC_MSG_ERROR([No such directory ${icu_header_dir}])
		fi
	fi

	if test "x${icu_library_dir}" != "x"; then
		if test -d "${icu_library_dir}"; then
			ICU_LDFLAGS="-L${icu_library_dir}"
		else
			AC_MSG_ERROR([No such directory ${icu_library_dir}])
		fi
	fi

	# Most ICU libraries have versioned symbols
	# (e.g. ucal_open_3_8), so the stock AC_CHECK_LIB will not
	# work here given the way it checks for symbol
	# availability. Punt and assume the library set normally
	# reported by icu-config.

	ICU_LIBS="${ICU_LDFLAGS} -licui18n -licuuc -licudata"

elif test "x${ICU_CONFIG}" != "x"; then
	ICU_CPPFLAGS="`${ICU_CONFIG} --cppflags`"
	ICU_LDFLAGS="`${ICU_CONFIG} --ldflags`"
	ICU_LIBS="`${ICU_CONFIG} --ldflags-libsonly`"

elif test "x${PKG_CONFIG}" != "x" -a "${PKG_CONFIG} --exists icu"; then
	ICU_CPPFLAGS="`${PKG_CONFIG} --cflags icu`"
	ICU_LDFLAGS="`${PKG_CONFIG} --libs-only-L icu`"
	ICU_LIBS="`${PKG_CONFIG} --libs-only-l icu`"

else
	AC_MSG_ERROR([Cannot find the International Components for Unicode (ICU) package.])
fi

AC_SUBST(ICU_CPPFLAGS)
AC_SUBST(ICU_LDFLAGS)
AC_SUBST(ICU_LIBS)

CPPFLAGS="${ICU_CPPFLAGS} ${CPPFLAGS}"
LDFLAGS="${ICU_LDFLAGS} ${LDFLAGS}"
LIBS="${ICU_LIBS} ${LIBS}"

AC_CHECK_HEADERS([unicode/putil.h])
AC_CHECK_HEADERS([unicode/ucal.h])
AC_CHECK_HEADERS([unicode/ucol.h])
AC_CHECK_HEADERS([unicode/ucurr.h])
AC_CHECK_HEADERS([unicode/ucurr.h])
AC_CHECK_HEADERS([unicode/udat.h])
AC_CHECK_HEADERS([unicode/ulocdata.h])
AC_CHECK_HEADERS([unicode/uloc.h])
AC_CHECK_HEADERS([unicode/uloc.h])
AC_CHECK_HEADERS([unicode/umsg.h])
AC_CHECK_HEADERS([unicode/unum.h])
AC_CHECK_HEADERS([unicode/uset.h])

#
# Sort out where the UUID components are at.
#

AC_MSG_NOTICE([checking Universally Unique Identifier (UUID) package])

AC_PATH_PROG([UUID_CONFIG],[uuid-config])

AC_ARG_WITH(uuid,
	AS_HELP_STRING([--with-uuid=DIR],
		       [Specify location of Universally Unique Identifier (UUID) headers and libraries.]),
	[
		if test "x${withval}" = "xno"; then
			AC_MSG_ERROR([${PACKAGE_NAME} requires the Universally Unique Identifier (UUID) package])
		fi
		uuid_dir=${withval}
	],
	[uuid_dir=])
AC_ARG_WITH(uuid-includes,
	AS_HELP_STRING([--with-uuid-includes=DIR],
		       [Specify location of Universally Unique Identifier (UUID) headers.]),
	[uuid_header_dir=${withval}],
	[uuid_header_dir=])
AC_ARG_WITH(uuid-libs,
	AS_HELP_STRING([--with-uuid-libs=DIR],
		       [Specify location of Universally Unique Identifier (UUID) libraries.]),
	[uuid_library_dir=${withval}],
	[uuid_library_dir=])

# We always prefer checking the values of the various '--with-uuid-...' 
# options first to using uuid-config because the former might be used
# in a cross-compilation environment on a system that also contains
# uuid-config. In such a case, the user wants what he/she specified
# rather than what uuid-config indicates.

if test "x${uuid_dir}" != "x" -o "x${uuid_header_dir}" != "x" -o "x${uuid_library_dir}" != "x"; then
	if test "x${uuid_dir}" != "x"; then
		if test -d "${uuid_dir}"; then
			if test -d "${uuid_dir}/include"; then
				UUID_CPPFLAGS="-I${uuid_dir}/include"
			else
				UUID_CPPFLAGS="-I${uuid_dir}"
			fi

			if test -d "${uuid_dir}/lib"; then
				UUID_LDFLAGS="-L${uuid_dir}/lib"
			else
				UUID_LDFLAGS="-L${uuid_dir}"
			fi
		else
			AC_MSG_ERROR([No such directory ${uuid_dir}])
		fi
	fi

	if test "x${uuid_header_dir}" != "x"; then
		if test -d "${uuid_header_dir}"; then
			UUID_CPPFLAGS="-I${uuid_header_dir}"
		else
			AC_MSG_ERROR([No such directory ${uuid_header_dir}])
		fi
	fi

	if test "x${uuid_library_dir}" != "x"; then
		if test -d "${uuid_library_dir}"; then
			UUID_LDFLAGS="-L${uuid_library_dir}"
		else
			AC_MSG_ERROR([No such directory ${uuid_library_dir}])
		fi
	fi

	UUID_saved_libs="${LIBS}"
	UUID_saved_ldflags="${LDFLAGS}"
	LDFLAGS="${UUID_LDFLAGS} ${LDFLAGS}"
	LIBS=""

		AC_CHECK_LIB(uuid, uuid_compare)

 	UUID_LIBS="${UUID_LDFLAGS} ${LIBS}"
	LDFLAGS="${UUID_saved_ldflags}"
 	LIBS="${UUID_saved_libs}"
 	unset UUID_saved_libs
	unset UUID_saved_ldflags

elif test "x${UUID_CONFIG}" != "x"; then
	UUID_CPPFLAGS="`${UUID_CONFIG} --cflags`"
	UUID_LDFLAGS="`${UUID_CONFIG} --ldflags`"
	UUID_LIBS="`${UUID_CONFIG} --libs`"

elif test "x${PKG_CONFIG}" != "x" -a "${PKG_CONFIG} --exists uuid"; then
	UUID_CPPFLAGS="`${PKG_CONFIG} --cflags uuid`"
	UUID_LDFLAGS="`${PKG_CONFIG} --libs-only-L uuid`"
	UUID_LIBS="`${PKG_CONFIG} --libs-only-l uuid`"

else
	AC_MSG_ERROR([Cannot find the Universally Unique Identifier (UUID) package.])
fi

AC_SUBST(UUID_CPPFLAGS)
AC_SUBST(UUID_LDFLAGS)
AC_SUBST(UUID_LIBS)

CPPFLAGS="${UUID_CPPFLAGS} ${CPPFLAGS}"
LDFLAGS="${UUID_LDFLAGS} ${LDFLAGS}"
LIBS="${UUID_LIBS} ${LIBS}"

AC_CHECK_HEADERS([uuid/uuid.h])

#
# Check for POSIX thread support.
#
AX_PTHREAD([],AC_MSG_ERROR([${PACKAGE_NAME} requires POSIX threads.]))

CFLAGS="${PTHREAD_CFLAGS} ${CFLAGS}"
CPPFLAGS="${PTHREAD_CPPFLAGS} ${CPPFLAGS}"
LDFLAGS="${PTHREAD_LDFLAGS} ${LDFLAGS}"
LIBS="${PTHREAD_LIBS} ${LIBS}"

#
# Check for where clock_gettime and friends are located.
#

AC_CHECK_LIB(rt, clock_gettime)

#
# Check for DLFCN support.
#
AC_CHECK_LIB(dl, dlopen)

AC_CHECK_HEADERS([dlfcn.h])

#
# Check for math support.
#
AC_CHECK_LIB(m, floor)

AC_CHECK_HEADERS([math.h])

#
# Check for where the default local time zone data file and time zone
# data files are/will be located on the target system (this might be
# different from where they are on the build system).
#

AC_MSG_CHECKING([where to find the default local time zone data file])
AC_ARG_WITH(tz-default,
	AS_HELP_STRING([--with-tz-default=FILE],
		       [Specify the location of the default local time zone data file on the target system @<:@default=/etc/localtime@:>@.]),
	[tz_default_file=${withval}],
	[tz_default_file="/etc/localtime"])
AC_MSG_RESULT(${tz_default_file})
AC_DEFINE_UNQUOTED([TZDEFAULT],["${tz_default_file}"],[Define this to the location of the default local time zone data file (e.g. /etc/localtime) on the target system.])

AC_MSG_CHECKING([where to find the time zone data files])
AC_ARG_WITH(tz-data,
	AS_HELP_STRING([--with-tz-data=DIR],
		       [Specify the location of the time zone data files on the target system @<:@default=/usr/share/zoneinfo@:>@.]),
	[tz_data_dir=${withval}],
	[tz_data_dir="/usr/share/zoneinfo"])
AC_MSG_RESULT(${tz_data_dir})
AC_DEFINE_UNQUOTED([TZDIR],["${tz_data_dir}"],[Define this to the location of the time zone data files (e.g. /usr/share/zoneinfo) on the target system.])

#
# Check for tzcode header support.
#
# Most systems based on the GNU C library do not have the header from
# tzcode (see ftp://elsie.nci.nih.gov/pub/), "tzfile.h" installed which is
# needed. Check if it exists. If it doesn't let the user point us at it.
#

AC_ARG_WITH(tz-includes,
	AS_HELP_STRING([--with-tz-includes=DIR],
		       [Specify location of Time Zone (tzcode) headers @<:@default=/usr/include@:>@.]),
	[
		if test "x${withval}" != "xyes"; then
			if test -d "${withval}/include"; then
				TZ_CPPFLAGS="-I${withval}/include"
			else
				TZ_CPPFLAGS="-I${withval}"
			fi
		fi
	])

CPPFLAGS="${TZ_CPPFLAGS} ${CPPFLAGS}"

AC_CHECK_HEADER([tzfile.h],[true],AC_MSG_ERROR([${PACKAGE_NAME} requires 'tzfile.h'. Download the latest tzcode package from 'ftp://elsie.nci.nih.gov/pub/' and/or use the '--with-tz-includes=DIR' configuration option.]))

#
# Check for headers
#

AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_TIME

AC_CHECK_HEADERS([alloca.h])
AC_CHECK_HEADERS([arpa/nameser.h])
AC_CHECK_HEADERS([assert.h])
AC_CHECK_HEADERS([dns_sd.h])
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([locale.h])
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_HEADERS([malloc/malloc.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([objc/objc.h])
AC_CHECK_HEADERS([objc/runtime.h])
AC_CHECK_HEADERS([poll.h])
AC_CHECK_HEADERS([pwd.h])
AC_CHECK_HEADERS([semaphore.h])
AC_CHECK_HEADERS([setjmp.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([stdbool.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([xlocale.h])
AC_CHECK_HEADERS([sys/cdefs.h])
AC_CHECK_HEADERS([sys/errno.h])
AC_CHECK_HEADERS([sys/fcntl.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/mman.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/uio.h])

#
# Checks for types and structures
#
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_STRUCT_TIMEZONE

#
# Checks for library functions
#

AC_FUNC_ALLOCA
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES

AC_CHECK_FUNCS([getcwd])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([malloc realloc])
AC_CHECK_FUNCS([memchr memcmp memmove memset])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([munmap])
AC_CHECK_FUNCS([rmdir])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([setenv])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([getgid getuid getegid geteuid])
AC_CHECK_FUNCS([getpid])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([rint strchr strdup strrchr strstr strtod strtol strtoul])
AC_CHECK_FUNCS([strncmp strncasecmp])
AC_CHECK_FUNCS([ustrncmp ustrncasecmp])
AC_CHECK_FUNCS([symlink])
AC_CHECK_FUNCS([readlink])
AC_CHECK_FUNCS([lstat stat])
AC_CHECK_FUNCS([snprintf])

# Figure out how to map strlcat and strlcpy if they don't exist by
# either simply redefining them or by using a local compatibility
# implementation.
#
# Some caveats are worth mentioning with respect to defining them to
# strn{cat,cpy}. In particular, strl{cat,cpy}:
# 
#   1) ALWAYS NULL-terminate if the destination has length.
#   2) Take the full size of the buffer (not the space left over).
#   3) Don't NULL-fill the remainder of its destination.
#   4) Return a size_t size rather than a char * pointer.
#
# While CoreFoundation has not been found to rely on any of these
# differences, it may in the future.

CF_REPLACE_STRLN_FUNC([cat])
CF_REPLACE_STRLN_FUNC([cpy])

#
# Other ibrary functions we might need to provide local compatibility
# implementations of.
#
AC_REPLACE_FUNCS([gettimeofday])

#
# Identify the various makefiles and auto-generated files for the package
#
AC_CONFIG_FILES([
CoreFoundation.pc
Makefile
include/Makefile
compat/Makefile
tests/Makefile
examples/Makefile
examples/Allocator/Makefile
examples/BundleExample/Makefile
examples/CFLocalServer/Makefile
examples/CFMessagePort/Makefile
examples/CFRunLoopTimerExample/Makefile
examples/CFTest/Makefile
examples/DisplayURL/Makefile
examples/DNSServiceMetaQuery/Makefile
examples/PLConvertExample/Makefile
examples/ReadPListExample/Makefile
examples/SimpleWebServer/Makefile
examples/StringExample/Makefile
examples/WritePListExample/Makefile
])

#
# Generate them
#
AC_OUTPUT

AC_MSG_NOTICE([

  Configuration Summary
  ---------------------
  Package                     : ${PACKAGE_NAME}
  Version                     : ${PACKAGE_VERSION}
  Deployment target           : ${deployment_target}
  Build shared library        : ${enable_shared}
  Build static library        : ${enable_static}
  Build debug library         : ${build_debug}
  Build profile library       : ${build_profile}
  Build examples and tests    : ${build_tests}
  Prefix		      : ${prefix}
  ICU compile options         : ${ICU_CPPFLAGS}
  ICU link options            : ${ICU_LDFLAGS}
  ICU link libraries          : ${ICU_LIBS}
  POSIX thread compile options: ${PTHREAD_CFLAGS}
  POSIX thread link options   : ${PTHREAD_LIBS}
  UUID compile options        : ${UUID_CPPFLAGS}
  UUID link options           : ${UUID_LDFLAGS}
  UUID link libraries	      : ${UUID_LIBS}
  Time Zone compile options   : ${TZ_CPPFLAGS}
  Time Zone default file      : ${tz_default_file}
  Time Zone data directory    : ${tz_data_dir}
  Compiler                    : ${CC}
  Preprocessor flags          : ${CPPFLAGS}
  Compile flags               : ${CFLAGS}
  Link flags                  : ${LDFLAGS}
  Link libraries              : ${LIBS}

])
