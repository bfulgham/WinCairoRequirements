# CF_BSD_SOURCE
# --------------
AC_DEFUN([CF_BSD_SOURCE],
[AH_VERBATIM([_BSD_SOURCE],
[/* Enable BSD extensions on systems that have them.  */
#ifndef _BSD_SOURCE
# undef _BSD_SOURCE
#endif])dnl
AC_BEFORE([$0], [AC_COMPILE_IFELSE])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_DEFINE([_BSD_SOURCE])
])

# CF_REPLACE_STRLN_FUNC(SUFFIX...)
#
# Determine whether one or both of strl<suffix> and strn<suffix> exist
# and then how to handle their presence or absence accordingly.
# --------------------------------------------------------------------
AC_DEFUN([CF_REPLACE_STRLN_FUNC],
[
AC_REQUIRE([AC_CHECK_FUNCS])
AC_REQUIRE([AC_REPLACE_FUNCS])
AC_CHECK_FUNCS([strl$1 strn$1])
if test "x${ac_cv_func_strl$1}" = "xno"; then
	AC_MSG_CHECKING([how to handle missing strl$1])
	if test "x${ac_cv_func_strn$1}" = "xyes"; then
		AC_MSG_RESULT([mapped to strn$1])
		AC_DEFINE([strl$1],
			  [strn$1],
			  [Define this if the target system has strn$1 but not strl$1.])
	else
		AC_MSG_RESULT([using local implementation])
		AC_REPLACE_FUNCS([strl$1])
	fi
fi
])
