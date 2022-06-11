AC_INIT(gbase-libs, 2.0.15, support@ctbto.org)
dnl
dnl initialize automake
dnl
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.9 foreign])
dnl
dnl place the results of running this script in the file config.h
dnl
AM_CONFIG_HEADER(config.h)
dnl
dnl
dnl ++++++++++++++++++++++++++ Updating this file <start> ++++++++++++++++++++++++++++++++++++++++
dnl
dnl Most people will never need to modify this file. But there are cases where a change may need
dnl to be made:
dnl
dnl 1. Change of version number.
dnl    In this case, the version number given to AC_INIT and AM_INIT_AUTOMAKE (both are above
dnl    this line) should be incremented to show the same number.
dnl
dnl 2. If you want to add a system library, go to the section lablelled
dnl    start LIBS section, and add the library there. Note that doing this will add the library
dnl    to all Makefiles, which is probably not what you want to do. See point 3.
dnl
dnl 3. An alternative way to add a system library is to add it to the section labelled
dnl    start optional library section
dnl    update the comment at the start of this section, and use the library name in the Makefile
dnl    and directive in source code as needed.
dnl
dnl 4. If you want to add a new directory to gbase, then go to the list of dirs under AC_CONFIG_FILES
dnl    and add the new directory to that list.
dnl
dnl 5. Once you have made any change to this file which you think would be of common interest,
dnl    please inform services@ctbto.org. This way, the modification may then be incoporated into
dnl    future releases.
dnl
dnl ++++++++++++++++++++++++++ Updating this file   <end> ++++++++++++++++++++++++++++++++++++++++
dnl
dnl specify default installation directory.
dnl you can override this with the --prefix option to this script.
dnl the intention is to install gbase and related software under the same root directory.
dnl The path is used when the "make install" command is used. This does not default to
dnl $HOME, since installing software in your home directory is not a good practice
dnl when there are multiple users.
dnl
AC_PREFIX_DEFAULT(/vobs/ctbto/install/gbase)
# === By default, turn on shared and static libraries ===
# This is needed so that libraries and executables can be built either
# static only (-static added to LDFLAGS), shared only (-shared added to
# LDFLAGS) or both (nothing added to LDFLAGS) [controlled in Makefile.am]
# HE (IDC/SA/SI, CTBTO PrepCom) - 13 November 2008
AC_ENABLE_SHARED
AC_ENABLE_STATIC
# === Try to avoid versioning for libraries and modules ===
# This is a libtool link mode option
# HE (IDC/SA/SI, CTBTO PrepCom) - 11 November 2008
EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -avoid-version"
# === END avoid versioning for libraries and modules ===
AC_SUBST(EXTRA_LDFLAGS)
dnl
dnl
dnl checks for programs
dnl
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
dnl

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

dnl === Enable 32-bit code generation if --enable-32bit is specified ===
AC_ARG_ENABLE([32bit], 
   AS_HELP_STRING([--enable-32bit], [Enable 32-bit code generation (default is disabled)]), 
            [ac_cv_enable_32bit=$enableval], 
	    [ac_cv_enable_32bit=no])
if test "x$ac_cv_enable_32bit" = "xyes"; then
  dnl GCC-specific options
  if test "$GCC" = "yes"; then
    dnl Add -Wall if using gcc
    AC_MSG_NOTICE([32bit: Enabling generation of 32-bit code with gcc -m32 option])
    CFLAGS="$CFLAGS -m32"
    CXXFLAGS="$CXXFLAGS -m32"
    FFLAGS="$FFLAGS -m32"
    LDFLAGS="$LDFLAGS -m32"
  fi
fi
dnl === END 32-bit compilation ===
dnl === Enable IDC options if --enable-idc is specified ===
AC_ARG_ENABLE([idc], 
   AS_HELP_STRING([--enable-idc], [Enable IDC options (default is disabled)]), 
            [ac_cv_enable_idc=$enableval], 
	    [ac_cv_enable_idc=no])
if test "x$ac_cv_enable_idc" = "xyes"; then
  dnl GCC-specific options
  if test "$GCC" = "yes"; then
    dnl Add -ggdb3 if using gcc
    AC_MSG_NOTICE([IDC: Enabling maximum GDB debugging information with gcc -ggdb3 option])
    CFLAGS="$CFLAGS -ggdb3"
    CXXFLAGS="$CXXFLAGS -ggdb3"
    FFLAGS="$FFLAGS -ggdb3"
    dnl Add -O2 if using gcc
    AC_MSG_NOTICE([IDC: Enabling gcc optimizations with gcc -O2 -fno-strict-aliasing options])
    dnl Note -fno-strict-aliasing is needed due to the extensive use of
    dnl type-casting back and forth between any kind of types
    CFLAGS="$CFLAGS -O2 -fno-strict-aliasing"
    CXXFLAGS="$CXXFLAGS -O2 -fno-strict-aliasing"
    FFLAGS="$FFLAGS -O2 -fno-strict-aliasing"
    dnl Add -mtune=opteron if using gcc
    AC_MSG_NOTICE([IDC: Enabling gcc tuning for AMD Opteron Core Processors with gcc -mtune=opteron option])
    CFLAGS="$CFLAGS -mtune=opteron"
    CXXFLAGS="$CXXFLAGS -mtune=opteron"
    FFLAGS="$FFLAGS -mtune=opteron"
  fi
fi
dnl === END IDC options ===

dnl
dnl find all libraries under the libsrc dir, and add those paths to LDFLAGS.
dnl This is done so we can do a "make", and have the programs under src find
dnl the libraries which are needed. This eliminates the need to maintain a long list
dnl of library dir names.
dnl The downside is that LDFLAGS becomes a long list. While unaesthetic, it causes
dnl no functional problems.
dnl
for lib in `find $PWD/libsrc/ -type d -name 'lib*'`
do
    LDFLAGS="$LDFLAGS -L$lib"
dnl
dnl CPPFLAGS is not used at the moment, but could be used in the future this way.
dnl This would allow .h files to "live" in the libsrc subdir, even if those .h files
dnl are used by applications.
dnl CPPFLAGS="$CPPFLAGS -I$lib"
dnl
done


dnl checks for header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(asm/poll.h fcntl.h ieeefp.h malloc.h memory.h stdarg.h stdlib.h string.h strings.h sys/param.h sys/time.h syslog.h unistd.h values.h varargs.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
dnl currently only have replacements for nint and strtok_r
AC_CHECK_FUNCS(floor gettimeofday memset mkdir pow nint rint strtok_r sqrt strcasecmp strchr strerror strncasecmp strstr strtol)

AC_C_BIGENDIAN
dnl
dnl some of the code requires threads, and is thread safe, so these flags are needed
dnl
CPPFLAGS="$CPPFLAGS -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT"

dnl if using gcc, add the -Wall cpp flag
if test "${GCC}" = "yes"; then
   CPPFLAGS="$CPPFLAGS -Wall"
fi

dnl +++++++++++++++++ start LIBS section ++++++++++++++++++++++++++++++++++++++++
dnl
dnl This section finds all of the libraries which may be added to the
dnl LIBS varible in each Makefile. If a library cannot be found, a warning message
dnl is given and it will not be added to LIBS.
dnl
dnl possible libraries are:
dnl   LIBS = -lnsl -lsocket -lresolv -lrt -lm
dnl
AC_CHECK_LIB(m, main)
dnl rt, socket, nsl are needed by libcd under solaris
AC_CHECK_LIB(rt, nanosleep)
dnl libresolv is needed by the socket library on several platforms
AC_CHECK_LIB(resolv, dn_expand)
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, inet_addr)


dnl ++++++++++++++++++ end LIBS section +++++++++++++++++++++++++++++++++++++++++

dnl ++++++++++++++++++ start optional library section +++++++++++++++++++++++++++
dnl
dnl This section finds all of the libraries which may be added to the
dnl specific varibles in each Makefile. It searches for the appropriate headers
dnl and libraries, and attempts to compile a trival test program. If the test
dnl fails, the functionality will not be available.
dnl
dnl possible libraries are:
dnl   library		library name in Makefile	directive in source code
dnl   -lz		Z_LIB				HAVE_LIBZ
dnl   -lcrypto		AUTH_LIB			HAVE_AUTHENTICATION
dnl   -lldap		LDAP_LIB			HAVE_LDAP
dnl   -lpthread		PTHREAD_LIB			HAVE_PTHREAD
dnl   -lnetcdf		NETCDF_LIB			HAVE_NETCDF (also HAVE_NETCDF4 for netcdf4 utilities)
dnl   -lodbc		ODBC_LIB			HAVE_LIBODBC

dnl
dnl libz include files and library
AC_CHECK_HEADERS(zlib.h,
    [AC_CHECK_LIB(z, gzopen,
	[Z_LIB="$Z_LIB -lz"
	 AC_DEFINE(HAVE_LIBZ) ],
	[Z_LIB= ],
    )],)dnl end of zlib checks
AC_SUBST(Z_LIB)
AH_TEMPLATE([HAVE_LIBZ], []) dnl end of zlib checks

dnl
dnl openssl include files and library
AC_CHECK_HEADERS(openssl/dsa.h,
    [AC_CHECK_LIB(crypto, CRYPTO_lock,
	[AUTH_LIB="$AUTH_LIB -lcrypto"
	 AC_DEFINE(HAVE_AUTHENTICATION) ],
	[AUTH_LIB= ],
    )],)dnl end of openssl checks
AC_SUBST(AUTH_LIB)
AH_TEMPLATE([HAVE_AUTHENTICATION], []) dnl end of athentication checks

dnl
dnl ldap include files and library
AC_CHECK_HEADERS(ldap.h,
    [AC_CHECK_LIB(ldap, ldap_open,
	[LDAP_LIB="$LDAP_LIB -lldap"
	 AC_DEFINE(HAVE_LDAP) ],
	[LDAP_LIB= ],
    )],)dnl end of ldap checks
AC_SUBST(LDAP_LIB)
AH_TEMPLATE([HAVE_LDAP], []) dnl end of ldap checks

dnl
dnl pthread include files and library
AC_CHECK_HEADERS(pthread.h,
    [AC_CHECK_LIB(pthread, pthread_create,
	[PTHREAD_LIB="$PTHREAD_LIB -lpthread"
	 AC_DEFINE(HAVE_PTHREAD) ],
	[PTHREAD_LIB= ],
    )],)dnl end of pthread checks
AC_SUBST(PTHREAD_LIB)
AH_TEMPLATE([HAVE_PTHREAD], []) dnl end of pthreads checks

dnl
dnl netcdf include files and library
AC_CHECK_HEADERS(netcdf.h,
    [AC_CHECK_LIB(netcdf, nc_open,
	[NETCDF_LIB="$NETCDF_LIB -lnetcdf"
         AC_DEFINE(HAVE_NETCDF) 
         AC_CHECK_LIB(netcdf,nc4_enddef_netcdf4_file,[AC_DEFINE(HAVE_NETCDF4)],[],)
        ],
	[NETCDF_LIB= ],
    )],)dnl end of netcdf checks
AC_SUBST(NETCDF_LIB)
AH_TEMPLATE([HAVE_NETCDF], [])
AH_TEMPLATE([HAVE_NETCDF4], []) dnl end of netcdf checks

dnl ODBC checks
dnl first see if sql.h sqlext.h can be found, then check if
dnl sqltypes.h defines SQLLEN, and then finally check for libodbc
dnl
AC_CHECK_HEADERS(sql.h sqlext.h,
dnl check to see if sqltypes.h includes the defintion for SQLLEN
[AC_MSG_CHECKING(if sqltypes.h defines SQLLEN)
    AC_CACHE_VAL(r_cv_sqllen,
      [AC_TRY_COMPILE([#include <sqltypes.h>],
      [SQLLEN ind;],
      r_cv_sqllen=yes, r_cv_sqllen=no)])
AC_MSG_RESULT($r_cv_sqllen)
])

if test "$r_cv_sqllen" = "yes"; then
  AC_CHECK_LIB(odbc, SQLConnect,
    [ODBC_LIB="$ODBC_LIB -lodbc"
     AC_DEFINE(HAVE_LIBODBC) ],
    [ODBC_LIB= ],
  )
else
  AC_MSG_WARN([can't find ODBC include files and library])
fi

AC_SUBST(ODBC_LIB)
AH_TEMPLATE([HAVE_LIBODBC], [])
dnl end of ODBC checks

dnl ++++++++++++++++++ end optional library section +++++++++++++++++++++++++++++

dnl ++++++++++++++++++ start check of specific functions section ++++++++++++++++
dnl

dnl check to see if asctime_r takes 3 args. Under Linux there are 2.
AC_MSG_CHECKING(if asctime_r takes 3 args)
AC_CACHE_VAL(r_cv_asctime_r_3,
[AC_TRY_COMPILE([#include <time.h>],
[struct tm *tm; char    buf[90]; char   *ret_val; int   buflen = 90;
	ret_val = asctime_r (tm, buf, buflen);],
r_cv_asctime_r_3=yes, r_cv_asctime_r_3=no)])
if test "$r_cv_asctime_r_3" = "yes"; then AC_DEFINE(HAVE_ASCTIME_R_3) fi
AC_MSG_RESULT($r_cv_asctime_r_3)
AH_TEMPLATE([HAVE_ASCTIME_R_3],    [])

dnl check to see if ctime_r takes 3 args. Under Linux there are 2.
AC_MSG_CHECKING(if ctime_r takes 3 args)
AC_CACHE_VAL(r_cv_ctime_r_3,
[AC_TRY_COMPILE([#include <time.h>],
[struct tm *tm; char    buf[90]; char   *ret_val; int   buflen = 90;
	ret_val = ctime_r (tm, buf, buflen);],
r_cv_ctime_r_3=yes, r_cv_ctime_r_3=no)])
if test "$r_cv_ctime_r_3" = "yes"; then AC_DEFINE(HAVE_CTIME_R_3) fi
AC_MSG_RESULT($r_cv_ctime_r_3)
AH_TEMPLATE([HAVE_CTIME_R_3],      [])

dnl check to see if altzone exists. Under Linux it does not.
AC_MSG_CHECKING(if altzone exists)
AC_CACHE_VAL(r_cv_altzone,
[AC_TRY_COMPILE([#include <time.h>
#include <sys/time.h>],
[double now = 1.; return(now + altzone);],
	r_cv_altzone=yes, r_cv_altzone=no)])
if test "$r_cv_altzone" = "yes"; then AC_DEFINE(HAVE_ALTZONE) fi
AC_MSG_RESULT($r_cv_altzone)
AH_TEMPLATE([HAVE_ALTZONE],        [])


dnl check to see if timezone exists. Under Mac OS X it does not.
AC_MSG_CHECKING(if timezone exists)
AC_CACHE_VAL(r_cv_timezone,
[AC_TRY_COMPILE([#include <time.h>
#include <sys/time.h>],
[double now = 1.; return(now + timezone);],
	r_cv_timezone=yes, r_cv_timezone=no)])
if test "$r_cv_timezone" = "yes"; then AC_DEFINE(HAVE_TIMEZONE) fi
AC_MSG_RESULT($r_cv_timezone)
AH_TEMPLATE([HAVE_TIMEZONE],        [])


dnl check to see if gettimeofday takes 1 args. Under Linux it takes 2.
AC_MSG_CHECKING(if gettimeofday takes 1 args)
AC_CACHE_VAL(r_cv_gettimeofday_1,
[AC_TRY_COMPILE([#include <sys/time.h>
#include <unistd.h>],
[struct timeval  tv; (void) gettimeofday (&tv);],
	r_cv_gettimeofday_1=yes, r_cv_gettimeofday_1=no)])
if test "$r_cv_gettimeofday_1" = "yes"; then AC_DEFINE(HAVE_GETTIMEOFDAY_1) fi
AC_MSG_RESULT($r_cv_gettimeofday_1)
AH_TEMPLATE([HAVE_GETTIMEOFDAY_1], [])

dnl this may need a header file for under Linux.
dnl check for sys_errlist
AC_MSG_CHECKING(for sys_errlist)
AC_CACHE_VAL(r_cv_have_sysel, AC_TRY_LINK([#include <stdio.h>],[char **foo = sys_errlist;], r_cv_have_sysel=yes, r_cv_have_sysel=no))
if test "$r_cv_have_sysel" = "yes"; then AC_DEFINE(HAVE_SYSERRLIST) fi
AC_MSG_RESULT($r_cv_have_sysel)
AH_TEMPLATE([HAVE_SYSERRLIST],     [])


dnl ++++++++++++++++++ end check of specific functions section ++++++++++++++++++

AC_CONFIG_FILES(Makefile
	include/Makefile
	include/css/Makefile
	include/gobject/Makefile
	include/ibase/Makefile
	libsrc/Makefile
	libsrc/libLP/Makefile
	libsrc/libgamp/Makefile
	libsrc/libgqc/Makefile
	libsrc/libloc/Makefile
	libsrc/libstring/Makefile
	libsrc/libcalib/Makefile
	libsrc/libgdetect/Makefile
	libsrc/libgstalta/Makefile
	libsrc/libmagnitude/Makefile
	libsrc/libtau/Makefile
	libsrc/libgCDF/Makefile
	libsrc/libgeog/Makefile
	libsrc/libinterp/Makefile
	libsrc/libshape/Makefile
	libsrc/libtime/Makefile
)
dnl
dnl write out the Makefiles and other output files
dnl
AC_OUTPUT

dnl the following checks look for variables that contain the results of tests that were done above.
dnl These values are used below and are printed to stdout. If a test was not run,
dnl there is a chance that the reported value was never initialized. If a value was not
dnl initialized, the following lines will set the value to be "no", so it will be printed properly.

if test -z "$ac_cv_lib_crypto_CRYPTO_lock"; then
   ac_cv_lib_crypto_CRYPTO_lock="no"
fi

if test -z "$ac_cv_lib_ldap_ldap_open"; then
   ac_cv_lib_ldap_ldap_open="no"
fi

if test -z "$ac_cv_lib_odbc_SQLConnect"; then
   ac_cv_lib_odbc_SQLConnect="no"
fi

if test -z "$ac_cv_lib_netcdf_nc_open"; then
   ac_cv_lib_netcdf_nc_open="no"
fi

if test -z "$ac_cv_lib_z_gzopen"; then
   ac_cv_lib_z_gzopen="no"
fi

if test -z "$ac_cv_lib_pthread_pthread_create"; then
   ac_cv_lib_pthread_pthread_create="no"
fi

dnl report on what was found
AC_MSG_NOTICE([])
AC_MSG_NOTICE([gbase is now configured for ${host}])
AC_MSG_NOTICE([                C compiler \${CC}: ${CC}])
AC_MSG_NOTICE([      C compiler flags \${CFLAGS}: ${CFLAGS}])
AC_MSG_NOTICE([C preprocessor flags \${CPPFLAGS}: ${CPPFLAGS}])
AC_MSG_NOTICE([         linker flags \${LDFLAGS}: ${LDFLAGS}])
AC_MSG_NOTICE([       default libraries \${LIBS}: ${LIBS}])
AC_MSG_NOTICE([              \${LD_LIBRARY_PATH}: ${LD_LIBRARY_PATH}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([IDC options (debugging, optimization, tuning): ${ac_cv_enable_idc}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([pthread support:                     ${ac_cv_lib_pthread_pthread_create}])
AC_MSG_NOTICE([   ZLIB support:                     ${ac_cv_lib_z_gzopen}])
AC_MSG_NOTICE([   LDAP support:                     ${ac_cv_lib_ldap_ldap_open}])
AC_MSG_NOTICE([OPENSSL support:                     ${ac_cv_lib_crypto_CRYPTO_lock}])
AC_MSG_NOTICE([ netcdf support:                     ${ac_cv_lib_netcdf_nc_open}])
AC_MSG_NOTICE([   ODBC support:                     ${ac_cv_lib_odbc_SQLConnect}])
dnl
