# Copyright 2007-2010, 2013 CTBTO Preparatory Commission. All rights reserved.
# Please document and attribute any changes.


# Initialize
AC_PREREQ([2.59])
AC_INIT([cbase], [1.2.1], [support@ctbto.org], [cbase])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.9 foreign])
enable_libtool_deps=0  # for IDC_SETUP_DEPS


# Change default install prefix
AC_PREFIX_DEFAULT([/opt/IDCSW])


# === By default, turn on shared and static libraries ===
# This is needed so that libraries and executables can be built either
# static only (-static added to LDFLAGS), shared only (-shared added to
# LDFLAGS) or both (nothing added to LDFLAGS) [controlled in Makefile.am]
# HE (IDC/SA/SI, CTBTO PrepCom) - 13 November 2008
AC_ENABLE_SHARED
AC_ENABLE_STATIC


# === Check for and use Sun WorkShop compilers (if SunOS or
#     --with-sun-workshop is specified) unless --without-sun-workshop is
#     specified ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 24 June 2008
SUN_WORKSHOP_DIR="/opt/SUNWspro"
AC_ARG_WITH([sun-workshop],
            AS_HELP_STRING([--with-sun-workshop@<:@=DIR@:>@],
                           [build using Sun WorkShop compilers @<:@default=yes on SunOS, default=no on non-SunOS, default DIR=/opt/SUNWspro@:>@]),
            [ if test "${withval}" != "no" -a "${withval}" != "yes"; then
                SUN_WORKSHOP_DIR="${withval}"
              fi ],
            [ if test "`uname -s`" = "SunOS"; then
                withval="yes"
              else
                withval="no"
              fi ])
if test "${withval}" != "no"; then
  AC_MSG_NOTICE([sun-workshop: Attempting to use Sun WorkShop compilers])
  if test -z "${CC}" -o -z "${CXX}" -o -z "${F77}" -o -z "${FC}"; then
    if test -z "$CC"; then
      CC=${SUN_WORKSHOP_DIR}/bin/cc
      AC_MSG_NOTICE([sun-workshop: CC = ${CC}])
    fi
    if test -z "$CXX"; then
      CXX=${SUN_WORKSHOP_DIR}/bin/CC
      AC_MSG_NOTICE([sun-workshop: CXX = ${CXX}])
    fi
    if test -z "$F77"; then
      F77=${SUN_WORKSHOP_DIR}/bin/f77
      AC_MSG_NOTICE([sun-workshop: F77 = ${F77}])
    fi
    if test -z "$FC"; then
      FC=${SUN_WORKSHOP_DIR}/bin/f95
      AC_MSG_NOTICE([sun-workshop: FC = ${FC}])
    fi
    EXTRA_LDFLAGS="-L${SUN_WORKSHOP_DIR}/lib ${EXTRA_LDFLAGS}"
  fi
fi


# Check for build and install programs
AC_PROG_CPP
##AC_PROG_CXXCPP
AC_PROG_CC
##AC_PROG_CXX
##if test "${GCC}" = "yes"; then
##  # gfortran is (as of this writing, 29 Jan 2009) not part of the default F77
##  # search list. We want to use gfortran over g77 because external Fortran
##  # libraries are built using gfortran at the IDC.
##  # The g77 compiler is deprecated in favour of gfortran.
##  AC_PROG_F77([gfortran g77 f77])
##else
##  AC_PROG_F77
##fi
AC_PROG_LIBTOOL
AC_PROG_INSTALL


# === Patch libtool script ===
# A hack to fix the libtool script in config.status so it avoids post-install
# commands on libraries if they would fail (like when no write permission) as
# this happens on checked in ClearCase elements for example. Note that in the
# regular expression, [ and ] must be @<:@ and @:>@ in order to avoid being
# eaten by autoconf.
# HE (IDC/SA/SI, CTBTO PrepCom) - 9 April 2009
AC_CONFIG_COMMANDS([libtool-patch-postinstall],
                   [test -w "${ac_top_build_prefix}libtool" && ed -s "${ac_top_build_prefix}libtool" <<EOF
H
1,$ g/postinstall_cmds=/s/\(@<:@^~@:>@\)\(~\)\(@<:@^~@:>@\)/\1\2~\3/g
1,$ g/postinstall_cmds=/s/\(@<:@"~@:>@\)\(@<:@^"~@:>@\{1,\}@<:@@<:@:blank:@:>@@:>@\{1,\}\)\(@<:@^@<:@:blank:@:>@"~@:>@\{1,\}lib\)\(@<:@"~@:>@\)/\1if test -w \3; then \2\3; else true; fi\4/g
1,$ g/postinstall_cmds=/s/\(@<:@^~@:>@\)\(~\)~\(@<:@^~@:>@\)/\1\2\3/g
w
EOF
])
# === END patch libtool script ===


# === Check for Doxygen ==
# HE (IDC/SA/SI, CTBTO PrepCom) - 1 July 2008
AC_ARG_VAR([DOXYGEN], [Doxygen command])
test -z "${DOXYGEN}" && AC_PATH_PROG([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "${DOXYGEN}"])
# === END check for Doxygen ===


# C stuff
AC_PROG_GCC_TRADITIONAL 
AC_C_BIGENDIAN
AC_C_CONST
AC_C_VOLATILE


# Check to if fnmatch works 
AC_MSG_CHECKING(if fnmatch works)
AC_CACHE_VAL(r_cv_func_fnmatch_works,
[AC_TRY_COMPILE([#include <fnmatch.h>],
[ fnmatch ("a*", "abc", 0);],
r_cv_func_fnmatch_works="yes", r_cv_func_fnmatch_works="no")])
AC_MSG_RESULT($r_cv_func_fnmatch_works)


# Check to if fnmatch is GNU style 
AC_MSG_CHECKING(if fnmatch is GNU style)
AC_CACHE_VAL(r_cv_func_fnmatch_gnu_style,
if test "$r_cv_func_fnmatch_works" = "yes"; then
   [AC_TRY_COMPILE([#include <fnmatch.h>],
   [ fnmatch ("xxXX", "xXxX", FNM_CASEFOLD);],
   r_cv_func_fnmatch_gnu_style="yes", r_cv_func_fnmatch_gnu_style="no")])
fi
AC_MSG_RESULT($r_cv_func_fnmatch_gnu_style)
if test "$r_cv_func_fnmatch_gnu_style" = "yes"; then 
   AC_DEFINE(HAVE_GNU_FNMATCH, [], [Do we have GNU fnmatch header ])
fi 


# This may need a header file under Linux
# Check for sys_errlist
AC_MSG_CHECKING(for sys_errlist)
AC_CACHE_VAL(r_cv_have_sysel, AC_TRY_LINK([#include <stdio.h>],[char **foo = sys_errlist;], r_cv_have_sysel=yes, r_cv_have_sysel=no))
if test "$r_cv_have_sysel" = "yes"; then AC_DEFINE(HAVE_SYSERRLIST, [], [Do we have syserrlist]) fi
AC_MSG_RESULT($r_cv_have_sysel)


# Check to see if asctime_r takes 3 args. Under Linux there are 2.
AC_MSG_CHECKING(if asctime_r takes 3 args)
AC_CACHE_VAL(r_cv_asctime_r_3,
[AC_TRY_COMPILE([#include <time.h>],
[struct tm *tm; char    buf[90]; char   *ret_val; int   buflen = 90;
        ret_val = asctime_r (tm, buf, buflen);],
r_cv_asctime_r_3=yes, r_cv_asctime_r_3=no)])
if test "$r_cv_asctime_r_3" = "yes"; then AC_DEFINE(HAVE_ASCTIME_R_3, [], [Does asctime_r take 3 args]) fi
AC_MSG_RESULT($r_cv_asctime_r_3)


# Check to see if ctime_r takes 3 args. Under Linux there are 2.
AC_MSG_CHECKING(if ctime_r takes 3 args)
AC_CACHE_VAL(r_cv_ctime_r_3,
[AC_TRY_COMPILE([#include <time.h>],
[struct tm *tm; char    buf[90]; char   *ret_val; int   buflen = 90;
        ret_val = ctime_r (tm, buf, buflen);],
r_cv_ctime_r_3=yes, r_cv_ctime_r_3=no)])
if test "$r_cv_ctime_r_3" = "yes"; then AC_DEFINE(HAVE_CTIME_R_3, [], [Does ctime_r take 3 args]) fi
AC_MSG_RESULT($r_cv_ctime_r_3)


# Check to see if gettimeofday takes 1 args. Under Linux it takes 2.
AC_MSG_CHECKING(if gettimeofday takes 1 args)
AC_CACHE_VAL(r_cv_gettimeofday_1,
[AC_TRY_COMPILE([#include <sys/time.h>
#include <unistd.h>],
[struct timeval  tv; (void) gettimeofday (&tv);],
        r_cv_gettimeofday_1=yes, r_cv_gettimeofday_1=no)])
if test "$r_cv_gettimeofday_1" = "yes"; then AC_DEFINE(HAVE_GETTIMEOFDAY_1, [], [Does gettimeofday have one argument]) fi
AC_MSG_RESULT($r_cv_gettimeofday_1)


# Checks for header files
AC_HEADER_DIRENT
AC_HEADER_STDC
# Some of the following might already have been checked by previous macro
AC_CHECK_HEADERS([ asm/poll.h fcntl.h ieeefp.h inttypes.h malloc.h memory.h \
                   stdarg.h stdint.h stdlib.h string.h strings.h sys/param.h \
                   sys/stat.h sys/systeminfo.h sys/time.h tzfile.h unistd.h ])


# Check for some functions and types
AC_CHECK_FUNCS(floor gettimeofday sincos)
AC_CHECK_TYPE(caddr_t, char *)


# Fortran 77 stuff
##AC_PROG_F77_C_O
##AC_F77_LIBRARY_LDFLAGS
##AC_F77_DUMMY_MAIN
##AC_F77_MAIN
##AC_F77_WRAPPERS


# === Enable almost all GCC warnings ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 13 October 2007, 29 June 2010
if test "${GCC}" = "yes"; then
  AC_MSG_NOTICE([GCC: Enabling almost all warnings])
  # Note -Wno-unused-variable because there are default values/structs defined
  # in header files causing warnings (noise)
  # Once only gcc is used, "__attribute__ ((unused))" could be added to
  # these variables and this warning enabled again
  EXTRA_CFLAGS="${EXTRA_CFLAGS} -Wall -Wno-unused-variable"
  EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -Wall -Wno-unused-variable"
  # Note -Wno-unused-variable because there are often function parameters
  # which are not used
  if test "${G77}" = "yes" -o "${GFORTRAN}" = "yes"; then
    EXTRA_FFLAGS="${EXTRA_FFLAGS} -Wall -Wno-unused-variable"
  fi
fi
# === END almost all GCC warnings ===


# === Enable GCC pedantic mode ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 28 June 2010
if test "${GCC}" = "yes"; then
  AC_MSG_NOTICE([GCC: Enabling pedantic mode])
  EXTRA_CFLAGS="${EXTRA_CFLAGS} -pedantic"
  EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -pedantic"
  if test "${GFORTRAN}" = "yes"; then
    EXTRA_FFLAGS="${EXTRA_CXXFLAGS} -pedantic"
  fi
fi
# === END GCC pedantic mode ===


# === Enable 32-bit code generation if --enable-32bit is specified ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 12 October 2007
AC_ARG_ENABLE([32bit], 
              AS_HELP_STRING([--enable-32bit],
                             [enable 32-bit code generation @<:@default=no@:>@]),
              [ ], 
              [ enableval="no" ])
if test "${enableval}" = "yes"; then
  if test "${GCC}" = "yes"; then
    AC_MSG_NOTICE([32bit: GCC: Enabling generation of code for a 32-bit environment])
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -m32"
    EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -m32"
    if test "${G77}" = "yes" -o "${GFORTRAN}" = "yes"; then
      EXTRA_FFLAGS="${EXTRA_FFLAGS} -m32"
    fi
    if test "${lt_cv_prog_gnu_ld}" = "yes"; then
      EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -m32"
    fi
  fi
fi
# === END 32-bit code generation ===


# === Enable IDC options if --enable-idc is specified ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 12 October 2007
AC_ARG_ENABLE([idc], 
   AS_HELP_STRING([--enable-idc],
                  [enable IDC options @<:@default=no@:>@]), 
                  [ ], 
                  [ enableval="no" ])
if test "${enableval}" = "yes"; then
  if test "${GCC}" = "yes"; then
    AC_MSG_NOTICE([idc: GCC: Enabling maximum GDB debugging information])
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -ggdb3"
    EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -ggdb3"
    if test "${G77}" = "yes" -o "${GFORTRAN}" = "yes"; then
      EXTRA_FFLAGS="${EXTRA_FFLAGS} -ggdb3"
    fi
    AC_MSG_NOTICE([idc: GCC: Enabling optimizations])
    # Note -fno-strict-aliasing is needed due to the extensive use of
    # type-casting back and forth between any kind of types
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -O2 -fno-strict-aliasing"
    EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -O2 -fno-strict-aliasing"
    if test "${G77}" = "yes" -o "${GFORTRAN}" = "yes"; then
      EXTRA_FFLAGS="${EXTRA_FFLAGS} -O2"
    fi
    AC_MSG_NOTICE([idc: GCC: Enabling tuning for local machine])
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -mtune=native"
    EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} -mtune=native"
    if test "${G77}" = "yes" -o "${GFORTRAN}" = "yes"; then
      EXTRA_FFLAGS="${EXTRA_FFLAGS} -mtune=native"
    fi
  fi
fi
# === END IDC options ===


# === Save user-defined environment settings ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 2 July 2008
_user_cppflags="${CPPFLAGS}"
_user_cflags="${CFLAGS}"
_user_cxxflags="${CXXFLAGS}"
_user_fflags="${FFLAGS}"
_user_javahflags="${JAVAHFLAGS}"
_user_javacflags="${JAVACFLAGS}"
_user_ldflags="${LDFLAGS}"
_user_jarflags="${JARFLAGS}"
# === END save user-defined environment settings ===


# === Set up package developer and user-defined configure environment ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 2 July 2008
CPPFLAGS="${EXTRA_CPPFLAGS} ${_user_cppflags}"
CFLAGS="${EXTRA_CFLAGS} ${_user_cflags}"
CXXFLAGS="${EXTRA_CXXFLAGS} ${_user_cxxflags}"
FFLAGS="${EXTRA_FFLAGS} ${_user_fflags}"
JAVAHFLAGS="${EXTRA_JAVAHFLAGS} ${_user_javahflags}"
JAVACFLAGS="${EXTRA_JAVACFLAGS} ${_user_javacflags}"
LDFLAGS="${EXTRA_LDFLAGS} ${_user_ldflags}"
JARFLAGS="${EXTRA_JARFLAGS} ${_user_jarflags}"
# === END set up package developer and user-defined configure environment ===


# === Check for pthread library ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 1 July 2008
_libs="${LIBS}"
AC_CHECK_HEADERS([pthread.h])
LIBS=""
AC_SEARCH_LIBS([pthread_kill], [pthread])
PTHREAD_LIBS="${LIBS}"
AC_SUBST(PTHREAD_LIBS)
LIBS=${_libs}


# Our own libraries (libsrc), these need to be in dependency order

# libcancomp
# (No cbase dependencies)
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libcancomp])
CANCOMP_INCLUDES="-I\$(top_srcdir)/libsrc/libcancomp"
IDC_UNIQ_VAR([CANCOMP_INCLUDES])
CANCOMP_LIBS="-L\$(top_srcdir)/libsrc/libcancomp -lcancomp"
CANCOMP_DEPS=""
IDC_SETUP_DEPS([CANCOMP])
LIBCANCOMP="\$(top_builddir)/libsrc/libcancomp/libcancomp.la"
CANCOMP_DEPENDENCIES=""
IDC_UNIQ_VAR([CANCOMP_DEPENDENCIES])
AC_DEFINE(HAVE_CANCOMP, [], [Do we have cancomp library])
AC_MSG_RESULT([yes])
AC_SUBST(CANCOMP_INCLUDES)
AC_SUBST(CANCOMP_LIBS)
AC_SUBST(CANCOMP_LIBADD)
AC_SUBST(LIBCANCOMP)
AC_SUBST(CANCOMP_DEPENDENCIES)
AM_CONDITIONAL([HAVE_CANCOMP], [test -n "${CANCOMP_LIBS}"])

# libidcsyslog
# (No cbase dependencies)
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libidcsyslog])
IDCSYSLOG_INCLUDES="-I\$(top_srcdir)/libsrc/libidcsyslog"
IDC_UNIQ_VAR([IDCSYSLOG_INCLUDES])
IDCSYSLOG_LIBS="-L\$(top_srcdir)/libsrc/libidcsyslog -lidcsyslog"
IDCSYSLOG_DEPS=""
IDC_SETUP_DEPS([IDCSYSLOG])
LIBIDCSYSLOG="\$(top_builddir)/libsrc/libidcsyslog/libidcsyslog.la"
IDCSYSLOG_DEPENDENCIES=""
IDC_UNIQ_VAR([IDCSYSLOG_DEPENDENCIES])
AC_DEFINE(HAVE_IDCSYSLOG, [], [Do we have idcsyslog library])
AC_MSG_RESULT([yes])
AC_SUBST(IDCSYSLOG_INCLUDES)
AC_SUBST(IDCSYSLOG_LIBS)
AC_SUBST(IDCSYSLOG_LIBADD)
AC_SUBST(LIBIDCSYSLOG)
AC_SUBST(IDCSYSLOG_DEPENDENCIES)
AM_CONDITIONAL([HAVE_IDCSYSLOG], [test -n "${IDCSYSLOG_LIBS}"])

# libstdtime
# (No cbase dependencies)
_libs="${LIBS}"
LIBS=""
AC_SEARCH_LIBS([pow], [m sunmath])
AC_SEARCH_LIBS([floor], [m sunmath])
AC_SEARCH_LIBS([rint], [m sunmath])
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libstdtime])
STDTIME_INCLUDES="-I\$(top_srcdir)/libsrc/libstdtime"
IDC_UNIQ_VAR([STDTIME_INCLUDES])
STDTIME_LIBS="-L\$(top_srcdir)/libsrc/libstdtime -lstdtime"
STDTIME_DEPS="${LIBS}"
IDC_SETUP_DEPS([STDTIME])
LIBSTDTIME="\$(top_builddir)/libsrc/libstdtime/libstdtime.la"
STDTIME_DEPENDENCIES=""
IDC_UNIQ_VAR([STDTIME_DEPENDENCIES])
AC_DEFINE(HAVE_STDTIME, [], [Do we have stdtime library])
AC_SUBST(STDTIME_INCLUDES)
AC_SUBST(STDTIME_LIBS)
AC_SUBST(STDTIME_LIBADD)
AC_SUBST(LIBSTDTIME)
AC_SUBST(STDTIME_DEPENDENCIES)
AM_CONDITIONAL([HAVE_STDTIME], [test -n "${STDTIME_LIBS}"])
AC_MSG_RESULT([yes])
LIBS="${_libs}"

# libaesir
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libaesir])
if test -n "${STDTIME_LIBS}"; then
  AESIR_INCLUDES="-I\$(top_srcdir)/libsrc/libaesir ${STDTIME_INCLUDES}"
  IDC_UNIQ_VAR([AESIR_INCLUDES])
  AESIR_LIBS="-L\$(top_srcdir)/libsrc/libaesir -laesir"
  AESIR_DEPS="${STDTIME_LIBS}"
  IDC_SETUP_DEPS([AESIR])
  LIBAESIR="\$(top_builddir)/libsrc/libaesir/libaesir.la"
  AESIR_DEPENDENCIES="${LIBSTDTIME} ${STDTIME_DEPENDENCIES}"
  IDC_UNIQ_VAR([AESIR_DEPENDENCIES])
  AC_DEFINE(HAVE_AESIR, [], [Do we have aesir library])
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
AC_SUBST(AESIR_INCLUDES)
AC_SUBST(AESIR_LIBS)
AC_SUBST(AESIR_LIBADD)
AC_SUBST(LIBAESIR)
AC_SUBST(AESIR_DEPENDENCIES)
AM_CONDITIONAL([HAVE_AESIR], [test -n "${AESIR_LIBS}"])

# libparidc
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libparidc])
if test -n "${AESIR_LIBS}"; then
  PARIDC_INCLUDES="-I\$(top_srcdir)/libsrc/libparidc ${AESIR_INCLUDES}"
  IDC_UNIQ_VAR([PARIDC_INCLUDES])
  PARIDC_LIBS="-L\$(top_srcdir)/libsrc/libparidc -lparidc"
  PARIDC_DEPS="${AESIR_LIBS}"
  IDC_SETUP_DEPS([PARIDC])
  LIBPARIDC="\$(top_builddir)/libsrc/libparidc/libparidc.la"
  PARIDC_DEPENDENCIES="${LIBAESIR} ${AESIR_DEPENDENCIES}"
  IDC_UNIQ_VAR([PARIDC_DEPENDENCIES])
  AC_DEFINE(HAVE_PARIDC, [], [Do we have paridc library])
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
AC_SUBST(PARIDC_INCLUDES)
AC_SUBST(PARIDC_LIBS)
AC_SUBST(PARIDC_LIBADD)
AC_SUBST(LIBPARIDC)
AC_SUBST(PARIDC_DEPENDENCIES)
AM_CONDITIONAL([HAVE_PARIDC], [test -n "${PARIDC_LIBS}"])

# libtable
# HE (IDC/SA/SI, CTBTO PrepCom) - 18 March 2013
AC_MSG_CHECKING([whether dependencies are fulfilled for cbase libtable])
if test -n "${AESIR_LIBS}"; then
  TABLE_INCLUDES="-I\$(top_srcdir)/libsrc/libtable ${AESIR_INCLUDES}"
  IDC_UNIQ_VAR([TABLE_INCLUDES])
  TABLE_LIBS="-L\$(top_srcdir)/libsrc/libtable -ltable"
  TABLE_DEPS="${AESIR_LIBS}"
  IDC_SETUP_DEPS([TABLE])
  LIBTABLE="\$(top_builddir)/libsrc/libtable/libtable.la"
  TABLE_DEPENDENCIES="${LIBAESIR} ${AESIR_DEPENDENCIES}"
  IDC_UNIQ_VAR([TABLE_DEPENDENCIES])
  AC_DEFINE(HAVE_TABLE, [], [Do we have table library])
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
AC_SUBST(TABLE_INCLUDES)
AC_SUBST(TABLE_LIBS)
AC_SUBST(TABLE_LIBADD)
AC_SUBST(LIBTABLE)
AC_SUBST(TABLE_DEPENDENCIES)
AM_CONDITIONAL([HAVE_TABLE], [test -n "${TABLE_LIBS}"])


# === Try to avoid versioning for libraries and modules ===
# This is a libtool link mode option
# HE (IDC/SA/SI, CTBTO PrepCom) - 11 November 2008
EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -avoid-version"
# === END avoid versioning for libraries and modules ===


# === Handle Fortran header/include files ===
# The content of INCLUDES (set in Makefile.am) is not used when compiling
#   Fortran source, so add it to EXTRA_FFLAGS here (note the dollar sign
#   quoting) just before the end so it only becomes available in Makefiles
#   and never in this file
# HE (IDC/SA/SI, CTBTO PrepCom) - 15 October 2008
EXTRA_FFLAGS="${EXTRA_FFLAGS} \$(INCLUDES)"
# === END Fortran header/include files ===


# === Handle package developer flags ===
# See http://www.gnu.org/software/libtool/manual/automake/User-Variables.html
# Check abuse with "automake -Woverride"
# HE (IDC/SA/SI, CTBTO PrepCom) - 1 July 2008
AC_SUBST(EXTRA_CPPFLAGS)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_CXXFLAGS)
AC_SUBST(EXTRA_FFLAGS)
AC_SUBST(EXTRA_JAVACFLAGS)
AC_SUBST(EXTRA_LDFLAGS)
AC_SUBST(EXTRA_JARFLAGS)
# === END package developer flags ===


# === Restore user-defined environment settings ===
# HE (IDC/SA/SI, CTBTO PrepCom) - 2 July 2008
CPPFLAGS="${_user_cppflags}"
CFLAGS="${_user_cflags}"
CXXFLAGS="${_user_cxxflags}"
FFLAGS="${_user_fflags}"
JAVACFLAGS="${_user_javacflags}"
LDFLAGS="${_user_ldflags}"
JARFLAGS="${_user_jarflags}"
# === END restore user-defined environment settings ===


# Use alphabetical order here for simplicity as ordering does not matter
AC_CONFIG_FILES([
        Makefile \
        test/Makefile \
	libsrc/Makefile \
        libsrc/libaesir/Makefile \
        libsrc/libcancomp/Makefile \
        libsrc/libidcsyslog/Makefile \
        libsrc/libparidc/Makefile \
        libsrc/libstdtime/Makefile \
        libsrc/libtable/Makefile \
        ])

AC_OUTPUT


AC_MSG_NOTICE([])
AC_MSG_NOTICE([${PACKAGE_STRING} is now configured for ${host}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([                      C preprocessor \${CPP}: ${CPP}])
AC_MSG_NOTICE([                 C++ preprocessor \${CXXCPP}: ${CXXCPP}])
AC_MSG_NOTICE([ Package Developer C/C++ preprocessor flags: ${EXTRA_CPPFLAGS}])
AC_MSG_NOTICE([  User C/C++ preprocessor flags \${CPPFLAGS}: ${CPPFLAGS}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([                           C compiler \${CC}: ${CC}])
AC_MSG_NOTICE([         Package Developer C compiler flags: ${EXTRA_CFLAGS}])
AC_MSG_NOTICE([            User C compiler flags \${CFLAGS}: ${CFLAGS}])
AC_MSG_NOTICE([])
##AC_MSG_NOTICE([                        C++ compiler \${CXX}: ${CXX}])
##AC_MSG_NOTICE([       Package Developer C++ compiler flags: ${EXTRA_CXXFLAGS}])
##AC_MSG_NOTICE([        User C++ compiler flags \${CXXFLAGS}: ${CXXFLAGS}])
##AC_MSG_NOTICE([])
##AC_MSG_NOTICE([                 Fortran 77 compiler \${F77}: ${F77}])
##AC_MSG_NOTICE([Package Developer Fortran 77 compiler flags: ${EXTRA_FFLAGS}])
##AC_MSG_NOTICE([   User Fortran 77 compiler flags \${FFLAGS}: ${FFLAGS}])
##AC_MSG_NOTICE([])
AC_MSG_NOTICE([                               Linker \${LD}: ${LD}])
AC_MSG_NOTICE([             Package Developer linker flags: ${EXTRA_LDFLAGS}])
AC_MSG_NOTICE([               User linker flags \${LDFLAGS}: ${LDFLAGS}])
AC_MSG_NOTICE([])
if test -z "${DOXYGEN}"; then
  AC_MSG_NOTICE([                                    Doxygen: no])
else
  AC_MSG_NOTICE([                                    Doxygen: ${DOXYGEN}])
fi
AC_MSG_NOTICE([])
AC_MSG_NOTICE([Installation prefix is ${prefix}])
AC_MSG_NOTICE([Installation exec_prefix is ${exec_prefix}])
AC_MSG_NOTICE([Binaries will be installed in ${bindir}])
AC_MSG_NOTICE([Libraries will be installed in ${libdir}])
AC_MSG_NOTICE([Shared headers will be installed in ${includedir}])
AC_MSG_NOTICE([Thank you for configuring and building ${PACKAGE_STRING}])
