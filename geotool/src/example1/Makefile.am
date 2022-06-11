LIBDRAWXDIR = ../../@LIBDRAWX@
LIBGIODIR = ../../@LIBGIO@
LIBGMATHDIR = ../../@LIBGMATH@
LIBGBEAMDIR = ../../@LIBGBEAM@
LIBGDBDIR = ../../@LIBGDB@
LIBGMETHODPPDIR = ../../@LIBGMETHODPP@
LIBGOBJECTPPDIR = ../../@LIBGOBJECTPP@
LIBGRESPPPDIR = ../../@LIBGRESPPP@
LIBGPLOTDIR = ../../@LIBGPLOT@
LIBGXPPDIR = ../../@LIBGXPP@
LIBIDCSEEDDIR = ../../@LIBIDCSEED@
LIBMOTIFPPDIR = ../../@LIBMOTIFPP@
LIBWGETSDIR = ../../@LIBWGETS@

bin_PROGRAMS = example1

#
# note that version.o is here rather than as version.c in example1_SOURCES
# since version.c depends on $(example1_OBJECTS), which is derived from
# example1_SOURCES. If we add version.c to example1_SOURCES, version.o will
# appear in $(example1_OBJECTS) and then version.o will depend on itself and
# cause an infinite loop.


example1_LDADD= \
	version.o \
	-L$(LIBGXPPDIR) -lgx++ \
	-L$(LIBGMETHODPPDIR) -lgmethod++ \
	-L$(LIBGOBJECTPPDIR) -lgobject++ \
	-L$(LIBWGETSDIR) -lwgets \
	-L$(LIBDRAWXDIR) -ldrawx \
	-L$(LIBGIODIR) -lgio \
	-L$(LIBGBEAMDIR) -lgbeam \
	-L$(LIBGRESPPPDIR) -lgresp++ \
	-L$(LIBIDCSEEDDIR) -lidcseed \
	-L$(LIBGPLOTDIR) -lgplot \
	-L$(LIBGMATHDIR) -lgmath \
	-L$(LIBGDBDIR) -lgdb \
	-lloc \
	-lgeog \
	-lcancomp \
	-ltau \
	-ltime \
	-lstring \
	-linterp \
	-laesir \
	-lLP \
	-lshape \
	-L$(LIBMOTIFPPDIR) -lmotif++ \
	$(Z_LIB) \
        $(AUTH_LIB) \
        $(LDAP_LIB) \
        $(ODBC_LIB) \
	$(READLINE_LIB) \
        $(PTHREAD_LIB) \
	$(GSL_LIB)

#
# -lmotif++ defines WorkingDialog(), which can be called from gbase functions
# which is why this library is AFTER all of the gbase libraries
#
if HAVE_INTERACTIVE_IPC
INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/gmath \
	-I$(top_srcdir)/include/gobject++ \
	-I$(top_srcdir)/include/motif++ \
	-I$(top_srcdir)/include/widget
else 
INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/gmath \
	-I$(top_srcdir)/include/gobject++ \
	-I$(top_srcdir)/include/motif++ \
	-I$(top_srcdir)/include/widget \
	$(INTERACTIVE_IPC_INCLUDES) 
endif

EXTRA_DIST = version.c.input

example1_SOURCES = \
	main.cpp \
	Example.cpp

noinst_HEADERS = Example.h

# Misc UNIX commands used in makefiles
DATE = date
HOSTNAME = uname -n
BASENAME = basename
PWD = pwd

version.c: Makefile $(example1_OBJECTS) $(most_DEPENDENCIES)  version.c.input
	$(RM) $@
	(u=$${USER-root} v=$(VERSION) d=`$(PWD)` h=`$(HOSTNAME)` g=$(prefix) \
	t=`$(DATE)`; sed -e "s|%WHEN%|$${t}|" \
	-e "s|%WHO%|$${u}@$${h}|" \
	-e "s|%WHERE%|$${d}|" \
	-e "s|%VERSION%|$${v}|" \
	-e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< version.c.input > $@)

version.o: version.c
	$(CC) -c version.c

