dnl 
dnl AUTHOR: Francesco Chemolli
dnl
dnl SQUID Web Proxy Cache          http://www.squid-cache.org/
dnl ----------------------------------------------------------
dnl Squid is the result of efforts by numerous individuals from
dnl the Internet community; see the CONTRIBUTORS file for full
dnl details.   Many organizations have provided support for Squid's
dnl development; see the SPONSORS file for full details.  Squid is
dnl Copyrighted (C) 2001 by the Regents of the University of
dnl California; see the COPYRIGHT file for full details.  Squid
dnl incorporates software developed and/or copyrighted by other
dnl sources; see the CREDITS file for full details.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.


# check if the compiler accepts a supplied flag
# first argument is the variable containing the result 
# (will be set to "yes" or "no")
# second argument is the flag to be tested, verbatim
#
AC_DEFUN([SQUID_CC_CHECK_ARGUMENT],[
  AC_CACHE_CHECK([whether compiler accepts $2],[$1],
  [{
    AC_REQUIRE([AC_PROG_CC])
    SAVED_FLAGS="$CFLAGS"
    SAVED_CXXFLAGS="$CXXFLAGS"
    CFLAGS="$CXXFLAGS $2"
    CXXFLAGS="$CXXFLAGS $2"
    AC_TRY_LINK([],[int foo; ],
      [$1=yes],[$1=no])
    CFLAGS="$SAVED_CFLAGS"
    CXXFLAGS="$SAVED_CXXFLAGS"
  }])
])

# check if the c++ compiler supports the -fhuge-objects flag
# sets the variable squid_cv_cxx_arg_fhugeobjects to either "yes" or "no"
#
AC_DEFUN([SQUID_CXX_CHECK_ARG_FHUGEOBJECTS],[
  AC_LANG_PUSH([C++])
  if test "$GCC" = "yes"; then
    SQUID_CC_CHECK_ARGUMENT([squid_cv_cxx_arg_fhugeobjects],[-Werror -fhuge-objects])
  else
    squid_cv_cxx_arg_fhugeobjects=no
  fi
  AC_LANG_POP([C++])
])

# detect what kind of compiler we're using, either by using hints from
# autoconf itself, or by using predefined preprocessor macros
# sets the variable squid_cv_compiler to one of
#  - gcc
#  - sunstudio
#  - none (undetected)
# 
AC_DEFUN([SQUID_CC_GUESS_VARIANT], [
 AC_CACHE_CHECK([what kind of compiler we're using],[squid_cv_compiler],
 [
  AC_REQUIRE([AC_PROG_CC])
  if test "$GCC" = "yes" ; then
   squid_cv_compiler="gcc"
  fi
  dnl repeat the next block for each compiler, changing the
  dnl preprocessor definition type so that it depends on platform-specific
  dnl predefined macros
  dnl SunPro CC
  if test -z "$squid_cv_compiler" ; then
   AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([[
#if !defined(__SUNPRO_C) && !defined(__SUNPRO_CC)
#error "not sunpro c"
#endif
    ]])],[squid_cv_compiler="sunstudio"],[])
  fi
  dnl Intel CC
  if test -z "$squid_cv_compiler" ; then
   AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([[
#if !defined(__ICC)
#error "not Intel(R) C++ Compiler"
#endif
    ]])],[squid_cv_compiler="icc"],[])
  fi
  dnl end of block to be repeated
  if test -z "$squid_cv_compiler" ; then
   squid_cv_compiler="none"
  fi
  ])
 ])

# define the flag to use to have the compiler treat warnings as errors
# requirs SQUID_CC_GUESS_VARIANT
# Sets a few variables to contain some compiler-dependent command line
# options, or to empty strings if the compiler doesn't support those
# options
# They are (with their GCC equivalent):
# squid_cv_cc_option_werror   (-Werror)
# squid_cv_cxx_option_werror  (-Werror)
# squid_cv_cc_option_wall     (-Wall)
# squid_cv_cc_option_optimize (-O3)
# 
AC_DEFUN([SQUID_CC_GUESS_OPTIONS], [
 AC_REQUIRE([SQUID_CC_GUESS_VARIANT])
 AC_MSG_CHECKING([for compiler variant])
 case "$squid_cv_compiler" in
  gcc) 
   squid_cv_cc_option_werror="-Werror" 
   squid_cv_cxx_option_werror="-Werror" 
   squid_cv_cc_option_wall="-Wall"
   squid_cv_cc_option_optimize="-O3"
   squid_cv_cc_arg_pipe="-pipe"
   ;;
  sunstudio) 
   squid_cv_cc_option_werror="-errwarn=%all -errtags" 
   squid_cv_cxx_option_werror="-errwarn=%all,no%badargtype2w,no%wbadinit,no%wbadasg -errtags" 
   squid_cv_cc_option_wall="+w"
   squid_cv_cc_option_optimize="-fast"
   squid_cv_cc_arg_pipe=""
   ;;
  icc) 
   squid_cv_cxx_option_werror="-Werror"
   squid_cv_cc_option_werror="$squid_cv_cxx_option_werror" 
   squid_cv_cc_option_wall="-Wall"
   squid_cv_cc_option_optimize="-O2"
   squid_cv_cc_arg_pipe=""
   ;;
  *) 
   squid_cv_cxx_option_werror="" 
   squid_cv_cc_option_werror="" 
   squid_cv_cc_option_wall=""
   squid_cv_cc_option_optimize="-O"
   squid_cv_cc_arg_pipe=""
   ;;
 esac
 AC_MSG_RESULT([$squid_cv_compiler])
])
