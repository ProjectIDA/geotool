AC_INIT(geotool, 2.1.46, support@ctbto.org)
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
dnl 2. If you want to add a system library, go to the section labelled
dnl    start LIBS section, and add the library there. Note that doing this will add the library
dnl    to all Makefiles, which is probably not what you want to do. See point 3.
dnl
dnl 3. An alternative way to add a system library is to add it to the section labelled
dnl    start optional library section
dnl    update the comment at the start of this section, and use the library name in the Makefile 
dnl    and directive in source code as needed.
dnl
dnl 4. If you want to add a new directory to geotool++, then go to the list of dirs under AC_CONFIG_FILES
dnl    and add the new directory to that list.
dnl
dnl 5. If you add a new library to geotool++, go to the section labelled
dnl    set some variables needed in makefiles
dnl    and add a new line for the new library, following the naming convention. This name
dnl    can then be referenced in the appropriate Makefile.am's
dnl
dnl 6. Once you have made any change to this file which you think would be of common interest,
dnl    please inform services@ctbto.org. This way, the modification may then be incoporated into
dnl    future releases.
dnl
dnl ++++++++++++++++++++++++++ Updating this file   <end> ++++++++++++++++++++++++++++++++++++++++
dnl
dnl specify default installation directory. 
dnl you can override this with the --prefix option to this script.
dnl the intention is to install geotool++ and related software under the same root directory.
dnl The path is used when the "make install" command is used. This does not default to
dnl $HOME, since installing software in your home directory is not a good practice 
dnl when there are multiple users.
dnl
AC_PREFIX_DEFAULT(/vobs/ctbto/install/geotool)
dnl
dnl
dnl checks for programs
dnl
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
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


dnl set some variables needed in makefiles
AC_SUBST(LIBDRAWX, ["libsrc/libdrawx"])
AC_SUBST(LIBGMATH, ["libsrc/libgmath"])
AC_SUBST(LIBGDB, ["libsrc/libgdb"])
AC_SUBST(LIBGBEAM, ["libsrc/libgbeam"])
AC_SUBST(LIBGIO, ["libsrc/libgio"])
AC_SUBST(LIBGMETHODPP, ["libsrc/libgmethod++"])
AC_SUBST(LIBGOBJECTPP, ["libsrc/libgobject++"])
AC_SUBST(LIBGPLOT, ["libsrc/libgplot"])
AC_SUBST(LIBGRESPPP, ["libsrc/libgresp++"])
AC_SUBST(LIBGXPP, ["libsrc/libgx++"])
AC_SUBST(LIBIDCSEED, ["libsrc/libidcseed"])
AC_SUBST(LIBMOTIFPP, ["libsrc/libmotif++"])
AC_SUBST(LIBWGETS, ["libsrc/libwgets"])

dnl checks for header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(asm/poll.h fcntl.h ieeefp.h limits.h malloc.h memory.h stdarg.h stdlib.h string.h strings.h sys/file.h sys/param.h sys/time.h syslog.h tzfile.h unistd.h varargs.h)

AC_CHECK_HEADERS(fnan.h,,
    [AC_MSG_ERROR([can't find fnan.h. Run make install in gbase before compiling geotool
     and make sure configure script can find gbase include files and libraries
     by using the flags LDFLAGS and CPPFLAGS
     for example: configure LDFLAGS="-L/my/path/lib" CPPFLAGS="-I/my/path/include"])
    ]) dnl


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
dnl currently only have replacements for nint and strtok_r
AC_CHECK_FUNCS(finite floor flock gethostname gettimeofday lockf memset mkdir pow putenv nint rint select statvfs statfs strtok_r sqrt strcasecmp strchr strerror strncasecmp strstr strtol)

# Check for largefile (Unix98) support
AC_SYS_LARGEFILE
AC_TYPE_OFF_T

# Check for byte order
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
dnl   LIBS = -lnsl -lsocket -lrt -lm 
dnl
AC_CHECK_LIB(m, main)
dnl rt, socket, nsl are needed by libcd under solaris
AC_CHECK_LIB(rt, nanosleep)
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, inet_addr)

dnl this is a gbase library
AC_CHECK_LIB(string, stringToBool)

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
dnl   -lnetcdf		NETCDF_LIB			HAVE_NETCDF
dnl   -lodbc		ODBC_LIB			HAVE_LIBODBC
dnl   -lgsl -lgslcblas  GSL_LIB	            		HAVE_LIBGSL
dnl   -lreadline	READLINE_LIB           		HAVE_READLINE
dnl   -lMesaGLw -lGLw -lGL (e.g.) (appropriate libs added to LIB) HAVE_OPENGL
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
AH_TEMPLATE([HAVE_AUTHENTICATION], []) dnl end of authentication checks

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
	 AC_DEFINE(HAVE_NETCDF) ],
	[NETCDF_LIB= ],
    )],)dnl end of netcdf checks
AC_SUBST(NETCDF_LIB)
AH_TEMPLATE([HAVE_NETCDF], []) dnl end of netcdf checks

dnl
dnl readline include files and library
AC_CHECK_HEADERS(readline/readline.h,
  [AC_SEARCH_LIBS([tgetnum], [ncursesw ncurses curses], 
    [AC_CHECK_LIB(readline, readline, 
	[READLINE_LIB="$READLINE_LIB -lreadline ${LIBS}"
	 AC_DEFINE(HAVE_READLINE) ],
	[READLINE_LIB= ],
    )])],)dnl end of readline checks
AC_SUBST(READLINE_LIB)
AH_TEMPLATE([HAVE_READLINE], []) dnl end of readline checks

dnl
dnl gsl include files and library
AC_CHECK_HEADERS(gsl/gsl_matrix.h,
    [AC_CHECK_LIB(gsl, gsl_matrix_set_all, 
	[GSL_LIB="$GSL_LIB -lgslcblas -lgsl"
	 AC_DEFINE(HAVE_GSL) ],
	[GSL_LIB= ], "-lgslcblas"
    )],)dnl end of gsl checks
AC_SUBST(GSL_LIB)
AH_TEMPLATE([HAVE_GSL], []) dnl end of gsl checks
AM_CONDITIONAL([HAVE_GSL], [test -n "${GSL_LIB}"])

dnl ODBC checks
dnl first see if sql.h sqlext.h can be found, then check if
dnl sqltypes.h defines SQLLEN, and then finally check for libodbc
dnl
AC_CHECK_HEADERS(sql.h sqlext.h,
dnl check to see if sqltypes.h includes the definition for SQLLEN
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

dnl checks for types
dnl try to retrive the size of SQLLEN in UNIX ODBC library
use32bitmode=yes
odbcinst -j 2>/dev/null >/dev/null
if test $? != 0 ; then
   AC_MSG_WARN([can not determine sizeof(SQLLEN) in ODBC library, assuming])
   AC_MSG_WARN([32 bit. If odbctest fails, re-run configure with])
   AC_MSG_WARN([CPPFLAGS="-DBUILD_REAL_64_BIT_MODE"])
else
  AC_MSG_CHECKING([whether sizeof(SQLLEN) in odbcinst -j is four])
  if odbcinst -j | grep SQLLEN | grep 8 > /dev/null; then
     CPPFLAGS="$CPPFLAGS -DBUILD_REAL_64_BIT_MODE"
     use32bitmode=no
  fi
  AC_MSG_RESULT([$use32bitmode])
fi



dnl === Check for tuxedo if --with-tuxedo is specified ===
_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"  
_path="${PATH}"
TUXEDO_DIR="${TUXDIR}"  # Default directory to look for Tuxedo
AC_ARG_WITH([tuxedo],
            AS_HELP_STRING([--with-tuxedo(=DIR)],
                           [use Tuxedo (default=no)]),
            [ if test "${withval}" != "no" -a "${withval}" != "yes"; then
                TUXEDO_DIR="${withval}"
              fi ],
            [ withval="no" ])
if test -n "$TUXEDO_DIR"; then
    CPPFLAGS="${CPPFLAGS} -I${TUXEDO_DIR}/include"
    LDFLAGS="${LDFLAGS} -L${TUXEDO_DIR}/lib64 ${LDFLAGS} -L${TUXEDO_DIR}/lib -ltux -lfml32"
    PATH="${TUXEDO_DIR}/bin:$PATH"
    AC_MSG_NOTICE([Checking for Tuxedo in ${TUXEDO_DIR}])
else
    AC_MSG_NOTICE([Checking for Tuxedo])
fi
AC_CHECK_HEADER([fml32.h],
	[ AC_CHECK_LIB([tux], 
	             [tpconnect], 
                      [ AC_DEFINE(HAVE_TUXEDO, [], [Do we have Tuxedo])
                       if test -n "${TUXEDO_DIR}"; then
                         TUXEDO_INCLUDES="-I${TUXEDO_DIR}/include"
			 TUXEDO_LIBS="-L${TUXEDO_DIR}/lib64 -L${TUXEDO_DIR}/lib -ltux -lfml32"
                       else 
                         TUXEDO_INCLUDES=""
			 TUXEDO_LIBS="-ltux -lfml32"
		       fi
		       tuxedo_found="yes"
		       AC_MSG_NOTICE([Tuxedo found])
                       
		       AC_MSG_NOTICE([TUXEDO_INCLUDES = ${TUXEDO_INCLUDES}])
		       AC_MSG_NOTICE([TUXEDO_LIBS = ${TUXEDO_LIBS}])	
                       ],
                      [ tuxedo_found="no"
		        AC_MSG_WARN([Tuxedo libraries not found, please use --with-tuxedo=DIR])], 
		      [ -ldl ]	
                      )], 
         [ AC_MSG_WARN([Tuxedo headers not found, please use --with-tuxedo=DIR])]
)
AC_SUBST(TUXEDO_INCLUDES)
AC_SUBST(TUXEDO_LIBS)
AM_CONDITIONAL([HAVE_TUXEDO], [ test "${tuxedo_found}" = "yes" ])
CPPFLAGS="${_cppflags}"
LDFLAGS="${_ldflags}"
PATH="${_path}"
dnl === END Tuxedo check ===


dnl === Check for libipcnt if --with-libipcnt is specified ===
_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"  
_path="${PATH}"
AC_ARG_WITH([libipcnt],
            AS_HELP_STRING([--with-libipcnt(=DIR)],
                           [use libipcnt (default=no)]),
            [ if test "${withval}" != "no" -a "${withval}" != "yes"; then
                LIBIPCNT_DIR="${withval}"
              fi ],
            [ withval="no" ])
if test "${withval}" != "no"; then
    CPPFLAGS="${CPPFLAGS} -I${LIBIPCNT_DIR}/include"
    LDFLAGS="${LDFLAGS} -L${LIBIPCNT_DIR}/lib64 -L${LIBIPCNT_DIR}/lib" 
    AC_MSG_NOTICE([checking for libipcnt in ${LIBIPCNT_DIR}])
else
    AC_MSG_NOTICE([checking for libipcnt])
fi

AC_CHECK_HEADER([libipcnt.h],
	[ AC_CHECK_LIB([ipcnt], 
	             [ipc_attach], 
                     [ AC_DEFINE(HAVE_LIBIPCNT, [], [Do we have libipcnt])
		       if test -n "${LIBIPCNT_DIR}"; then
			  IPCNT_LIBS="-L${LIBIPCNT_DIR}/lib64 -L${LIBIPCNT_DIR}/lib -lipcnt -lidcsyslog"
			  IPCNT_INCLUDES="-I${LIBIPCNT_DIR}/include"
		       else 
			  IPCNT_LIBS=""
			  IPCNT_INCLUDES=""
		        fi   
                       libipcnt_found="yes"	 
		       AC_MSG_NOTICE([libipcnt found])			
                      ],
                      [ libipcnt_found="no" 
                      AC_MSG_WARN([libipcnt not found, please use --with-libipcnt=DIR])], 
		      [ -lidcsyslog ]
		      )], 
           [ libipc_found="no"
             AC_MSG_WARN([libipcnt headers not found, please use --with-libipcnt=DIR])]
)
AC_SUBST(IPCNT_INCLUDES)
AC_SUBST(IPCNT_LIBS)
CPPFLAGS="${_cppflags}"
LDFLAGS="${_ldflags}"
PATH="${_path}"
dnl === END libipcnt check ===


dnl === Check for libipc ===
_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"  
_path="${PATH}"
AC_ARG_WITH([libipc],
            AS_HELP_STRING([--with-libipc(=DIR)],
                           [use libipc (default=no)]),
            [ if test "${withval}" != "no" -a "${withval}" != "yes"; then
                LIBIPC_DIR="${withval}"
              fi ],
            [ withval="no" ])
if test "${withval}" != "no"; then
    CPPFLAGS="${CPPFLAGS} -I${LIBIPC_DIR}/include"
    LDFLAGS="${LDFLAGS} -L${LIBIPC_DIR}/lib64 -L${LIBIPC_DIR}/lib"
    AC_MSG_NOTICE([checking for libipc in ${LIBIPC_DIR}])
else
    AC_MSG_NOTICE([checking for libipc.h])
fi
AC_CHECK_HEADER([libipc.h],
	[ AC_CHECK_LIB([ipc], 
	             [ipc_attach], 
                     [ AC_DEFINE(HAVE_LIBIPC, [], [Do we have libipc])
		       if test -n "${LIBIPC_DIR}"; then
			  IPC_LIBS="-L${LIBIPC_DIR}/lib64 -L${LIBIPC_DIR}/lib -lipc ${TUXEDO_LIBS}"
			  IPC_INCLUDES="-I${LIBIPC_DIR}/include ${TUXEDO_INCLUDES}"
		       else 
			  IPC_LIBS=""
			  IPC_INCLUDES="-lipc"
		        fi   
                       libipc_found="yes"	 
		       AC_MSG_NOTICE([libipc found])			
                      ],
                      [ libipc_found="no" 
                      AC_MSG_WARN([libipc not found, please use --with-libipc=DIR])], 
		      [ -lidcsyslog ${TUXEDO_LIBS} -ldl ]
		      )], 
           [ libipc_found="no"
             AC_MSG_WARN([libipc headers not found, please use --with-libipc=DIR])]
)
AC_SUBST(IPC_INCLUDES)
AC_SUBST(IPC_LIBS)
CPPFLAGS="${_cppflags}"
LDFLAGS="${_ldflags}"
PATH="${_path}"
dnl === END libipc check ===



dnl === Enable interactive IPC unless --disable-interactive-ipc is specified ===
AC_ARG_ENABLE([interactive-ipc], 
              AS_HELP_STRING([--enable-interactive-ipc(=METHOD)],
                             [enable interactive IPC code (default=no, METHOD=nontux|tuxedo)]),
              [  if test "${enableval}" != "no" \
                           -a "${enableval}" != "nontux" \
                           -a "${enableval}" != "tuxedo"; then
                    AC_MSG_ERROR([Unknown Interactive IPC method ${enableval}, support methods are: nontux tuxedo])
                fi ], 
              [ enableval="no" ])
interactive_ipc="${enableval}"
if test "${enableval}" = "no"; then
  AC_MSG_NOTICE([Interactive IPC disabled])
else
  AC_MSG_NOTICE([Interactive IPC enabled])
  AC_DEFINE(HAVE_INTERACTIVE_IPC, [], [Do we have (want) interactive IPC])
  if test "${enableval}" = "nontux"; then
    AC_DEFINE(HAVE_INTERACTIVE_IPC_NONTUX, [], [Do we have (want) interactive IPC nontux])
    INTERACTIVE_IPC_INCLUDES="${IPCNT_INCLUDES}"
    INTERACTIVE_IPC_LIBS="${IPCNT_LIBS}"
    libipc_flag="${libipcnt_found}"
  else
    if test "${enableval}" = "tuxedo"; then
       if test "$tuxedo_found" != "yes"; then 
	  libipc_flag="no"
       else   
	  AC_DEFINE(HAVE_INTERACTIVE_IPC_TUXEDO, [], [Do we have (want) interactive IPC tuxedo])
          INTERACTIVE_IPC_INCLUDES="${IPC_INCLUDES}"
          INTERACTIVE_IPC_LIBS="${IPC_LIBS}"
          libipc_flag="${libipc_found}"	
       fi
    fi
  fi
  if test "${libipc_flag}" != "yes"; then
    AC_MSG_ERROR([Missing libraries or dependencies for Interactive IPC method ${enableval}, either --disable-interactive-ipc or --with-interactive-ipc=METHOD needs to be specified])
  fi
  AC_MSG_NOTICE([Interactive IPC headers and libraries found for method ${enableval}])
  AC_MSG_NOTICE([INTERACTIVE_IPC_INCLUDES = ${INTERACTIVE_IPC_INCLUDES}])
  AC_MSG_NOTICE([INTERACTIVE_IPC_LIBS = ${INTERACTIVE_IPC_LIBS}])
fi
INTERACTIVE_IPC="${enableval}"
AC_SUBST(INTERACTIVE_IPC_INCLUDES)
AC_SUBST(INTERACTIVE_IPC_LIBS)
AM_CONDITIONAL([HAVE_INTERACTIVE_IPC], [test -n "${INTERACTIVE_IPC_LIBS}"])
dnl === END enable interactive IPC ===




dnl ++++++++++++++++++ end optional library section +++++++++++++++++++++++++++++

dnl ++++++++++++++++++ start check of X window system files +++++++++++++++++++++++++++
dnl
dnl Check for paths and flags for X include files and libraries
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
CPPFLAGS="$CPPFLAGS $X_CFLAGS"
LDFLAGS="$LDFLAGS $X_LIBS $X_EXTRA_LIBS"

AC_CHECK_HEADERS(X11/Intrinsic.h,,
    [AC_MSG_ERROR([can't find X11/Intrinsic.h. Make sure that X11 libraries and development
    package is installed before configuring geotool])
    ])

dnl in some cases Xm/Xm.h may require X11/Intrinsic.h to also be included
AC_CHECK_HEADERS(X11/Intrinsic.h Xm/Xm.h,,
    [AC_MSG_ERROR([can't find Xm/Xm.h. Make sure that Motif libraries and development
    package is installed before configuring geotool])
    ])

dnl 
dnl some versions of Motif do not define XmSCROLL_HOR. check for this
dnl
AC_CHECK_HEADERS(Xm/Xm.h,
dnl check to see if Xm.h includes the defintion for XmSCROLL_HOR
[AC_MSG_CHECKING(if Xm.h defines XmSCROLL_HOR)
    AC_CACHE_VAL(r_cv_xmscroll_hor,
      [AC_TRY_COMPILE([#include <Xm/Xm.h>],
      [int i = XmSCROLL_HOR;],
      r_cv_xmscroll_hor=yes, r_cv_xmscroll_hor=no)])
AC_MSG_RESULT($r_cv_xmscroll_hor)
dnl 
if test "$r_cv_xmscroll_hor" = "no"; then
    AC_MSG_WARN([can't find definition of XmSCROLL_HOR in Xm/Xm.h])
else
     AC_DEFINE(HAVE_XMSCROLL_HOR)
fi
])
AH_TEMPLATE([HAVE_XMSCROLL_HOR],    [])
dnl end of XmSCROLL_HOR check

dnl 
dnl GL/GLwDrawA.h No such file or directory under some versions of Solaris
dnl also need <X11/Intrinsic.h> for this to compile
dnl
AC_MSG_CHECKING(if GL/GLwDrawA.h can be found)
    AC_CACHE_VAL(r_cv_glwdrawa_h,
        [AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <GL/GLwDrawA.h>
		       ],
        [int i = 1;],
    r_cv_glwdrawa_h=yes, r_cv_glwdrawa_h=no)])
if test "$r_cv_glwdrawa_h" = "yes"; then AC_DEFINE(HAVE_GLWDRAWA_H) fi
AC_MSG_RESULT($r_cv_glwdrawa_h)
AH_TEMPLATE([HAVE_GLWDRAWA_H],    [])
dnl end of GL/GLwDrawA.h check

dnl 
dnl GLw/GLwDrawA.h No such file or directory under some versions of Solaris
dnl also need <X11/Intrinsic.h> for this to compile
dnl This is where the file may be found under some version of Solaris
dnl
AC_MSG_CHECKING(if GLw/GLwDrawA.h can be found)
    AC_CACHE_VAL(r_cv_glwglwdrawa_h,
        [AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <GLw/GLwDrawA.h>
		        ],
        [int i = 1;],
    r_cv_glwglwdrawa_h=yes, r_cv_glwglwdrawa_h=no)])
if test "$r_cv_glwglwdrawa_h" = "yes"; then AC_DEFINE(HAVE_GLWGLWDRAWA_H) fi
AC_MSG_RESULT($r_cv_glwglwdrawa_h)
AH_TEMPLATE([HAVE_GLWGLWDRAWA_H],    [])
dnl end of GLw/GLwDrawA.h check

dnl 
dnl X11/GLw/GLwDrawA.h No such file or directory under some versions of Solaris
dnl also need <X11/Intrinsic.h> for this to compile
dnl This is where the file may be found under some version of Solaris
dnl
AC_MSG_CHECKING(if X11/GLw/GLwDrawA.h can be found)
    AC_CACHE_VAL(r_cv_x11glwglwdrawa_h,
        [AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <X11/GLw/GLwDrawA.h>
		        ],
        [int i = 1;],
    r_cv_x11glwglwdrawa_h=yes, r_cv_x11glwglwdrawa_h=no)])
if test "$r_cv_x11glwglwdrawa_h" = "yes"; then AC_DEFINE(HAVE_X11GLWGLWDRAWA_H) fi
AC_MSG_RESULT($r_cv_x11glwglwdrawa_h)
AH_TEMPLATE([HAVE_X11GLWGLWDRAWA_H],    [])
dnl end of X11/GLw/GLwDrawA.h check

dnl Replace `main' with a function in -lX11:
AC_CHECK_LIB(X11, main)
dnl Replace `main' with a function in -lXt:
AC_CHECK_LIB(Xt, main)
dnl Replace `main' with a function in -lXmu:
AC_CHECK_LIB(Xmu, main)
dnl Replace `main' with a function in -lXm:
AC_SEARCH_LIBS(_XmStrings, Xm ,,
        [AC_MSG_ERROR([can't find Xm (motif) library])])

dnl check for OpenGL
AC_CHECK_LIB(GL, main)

dnl
dnl MesaGLw is needed under RedHat Linux, to define "glwDrawingAreaWidgetClass" and "ggl_CreateOpenGLWidget"
dnl for other instances GLw can be used
dnl NOTE: check for GLw must be after the Xm (motif) check, since some versions of GLw depend on motif
dnl
AC_CHECK_LIB(MesaGLw, main)
AC_CHECK_LIB(GLw, main)

dnl The correct .h file, and correct GL library are needed to define OpenGL.
dnl
if test "$ac_cv_lib_GL_main" = "yes" &&
    (test "$r_cv_glwdrawa_h" = "yes" || test "$r_cv_glwglwdrawa_h" = "yes" || test "$r_cv_x11glwglwdrawa_h" = "yes"); then
 opengl_ok="yes"; AC_DEFINE(HAVE_OPENGL)
else
 opengl_ok="no"
fi
AH_TEMPLATE([HAVE_OPENGL],    [])
AM_CONDITIONAL([HAVE_OPENGL], [test  "$opengl_ok"="yes"])


dnl
dnl ++++++++++++++++++ end check of X window system files +++++++++++++++++++++++++++

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

AC_OUTPUT(Makefile \
\
scripts/Makefile \
scripts/init/Makefile \
test/Makefile \
test/test_data/Makefile \
test/test_scripts/Makefile \
doc/Makefile \
doc/images/Makefile \
doc/examples/Makefile \
doc/scripts/Makefile \
include/Makefile \
include/gobject++/Makefile \
include/motif++/Makefile \
include/seed/Makefile \
include/widget/Makefile \
\
libsrc/Makefile \
libsrc/libcluster/Makefile \
libsrc/libdrawx/Makefile \
libsrc/libgmath/Makefile \
libsrc/libgdb/Makefile \
libsrc/libgio/Makefile \
libsrc/libgbeam/Makefile \
libsrc/libgmethod++/Makefile \
libsrc/libgobject++/Makefile \
libsrc/libgplot/Makefile \
libsrc/libidcseed/Makefile \
libsrc/libgresp++/Makefile \
libsrc/libgx++/Makefile \
libsrc/libmccc/Makefile \
libsrc/libmotif++/Makefile \
libsrc/libwgets/Makefile \
\
src/Makefile \
src/geotool/Makefile \
src/example1/Makefile \
src/seedtocss/Makefile \
\
plugins/Makefile \
plugins/libgarrival/Makefile \
plugins/libgbm/Makefile \
plugins/libgcal/Makefile \
plugins/libgcepstrum/Makefile \
plugins/libgcluster/Makefile \
plugins/libgcor/Makefile \
plugins/libgdataqc/Makefile \
plugins/libgfk/Makefile \
plugins/libgft/Makefile \
plugins/libgftrace/Makefile \
plugins/libghp/Makefile \
plugins/libglc/Makefile \
plugins/libgmap/Makefile \
plugins/libgmccc/Makefile \
plugins/libgorigin/Makefile \
plugins/libgpm/Makefile \
plugins/libgpolar/Makefile \
plugins/libgrot/Makefile \
plugins/libgrsp/Makefile \
plugins/libgscript/Makefile \
plugins/libgselfscan/Makefile \
plugins/libgspectro/Makefile \
plugins/libgstlt/Makefile \
plugins/libgtt/Makefile \
\
plugins/examples/Makefile \
plugins/examples/example1/Makefile \
plugins/examples/example2/Makefile \
plugins/examples/example3/Makefile \
plugins/examples/example4/Makefile \
)
dnl 
dnl write out the Makefiles and other output files
dnl

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

if test -z "$ac_cv_lib_gsl_gsl_matrix_set_all"; then
   ac_cv_lib_gsl_gsl_matrix_set_all="no"
fi

if test -z "$ac_cv_lib_readline_readline"; then
   ac_cv_lib_readline_readline="no"
fi

if test -z "$ac_cv_lib_z_gzopen"; then
   ac_cv_lib_z_gzopen="no"
fi

if test -z "$ac_cv_lib_pthread_pthread_create"; then
   ac_cv_lib_pthread_pthread_create="no"
fi

dnl report on what was found
AC_MSG_NOTICE([])
AC_MSG_NOTICE([${PACKAGE_STRING} is now configured for ${host}])
AC_MSG_NOTICE([                C compiler \${CC}: ${CC}])
AC_MSG_NOTICE([             C++ compiler \${CXX}: ${CXX}])
AC_MSG_NOTICE([      C compiler flags \${CFLAGS}: ${CFLAGS}])
AC_MSG_NOTICE([  C++ compiler flags \${CXXFLAGS}: ${CXXFLAGS}])
AC_MSG_NOTICE([C preprocessor flags \${CPPFLAGS}: ${CPPFLAGS}])
AC_MSG_NOTICE([         linker flags \${LDFLAGS}: ${LDFLAGS}])
AC_MSG_NOTICE([       default libraries \${LIBS}: ${LIBS}])
AC_MSG_NOTICE([              \${LD_LIBRARY_PATH}: ${LD_LIBRARY_PATH}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([ pthread support:          ${ac_cv_lib_pthread_pthread_create}])
AC_MSG_NOTICE([    ZLIB support:          ${ac_cv_lib_z_gzopen}])
AC_MSG_NOTICE([    LDAP support:          ${ac_cv_lib_ldap_ldap_open}])
AC_MSG_NOTICE([ OPENSSL support:          ${ac_cv_lib_crypto_CRYPTO_lock}])
AC_MSG_NOTICE([  netcdf support:          ${ac_cv_lib_netcdf_nc_open}])
AC_MSG_NOTICE([    ODBC support:          ${ac_cv_lib_odbc_SQLConnect}])
AC_MSG_NOTICE([    gsl  support:          ${ac_cv_lib_gsl_gsl_matrix_set_all}])
AC_MSG_NOTICE([readline support:          ${ac_cv_lib_readline_readline}])
AC_MSG_NOTICE([  openGL support:          ${opengl_ok}])
AC_MSG_NOTICE([     ipc enabled:          ${interactive_ipc}])
dnl
