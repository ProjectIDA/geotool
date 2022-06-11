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

bin_PROGRAMS = geotool

#
# note that version.o is here rather than as version.c in geotool_SOURCES
# since version.c depends on $(geotool_OBJECTS), which is derived from
# geotool_SOURCES. If we add version.c to geotool_SOURCES, version.o will
# appear in $(geotool_OBJECTS) and then version.o will depend on itself and
# cause an infinite loop.


geotool_LDADD= \
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
	-I$(top_srcdir)/include/gbase \
	-I$(top_srcdir)/include/gobject++ \
	-I$(top_srcdir)/include/motif++ \
	-I$(top_srcdir)/include/widget
else 
INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/gbase \
	-I$(top_srcdir)/include/gobject++ \
	-I$(top_srcdir)/include/motif++ \
	-I$(top_srcdir)/include/widget \
	$(INTERACTIVE_IPC_INCLUDES) 
endif

EXTRA_DIST = version.c.input gtq.input gtv.input

geotool_SOURCES = \
	main.cpp

bin_SCRIPTS = gtq gtv

# Misc UNIX commands used in makefiles
DATE = date
HOSTNAME = uname -n
BASENAME = basename
PWD = pwd

version.c: Makefile $(geotool_OBJECTS) $(most_DEPENDENCIES)  version.c.input
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

gtq: Makefile $(most_DEPENDENCIES) gtq.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gtq.input > $@)

gtv: Makefile $(most_DEPENDENCIES) gtv.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gtv.input > $@)

