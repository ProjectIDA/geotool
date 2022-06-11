INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	MultiChannelCC.h

lib_LTLIBRARIES = libgmccc.la

libgmccc_la_SOURCES = \
	libgmccc.cpp \
	MultiChannelCC.cpp

libgmccc_la_LIBADD = \
	-L../../libsrc/libmccc -lmccc

EXTRA_DIST = gmccc.input

bin_SCRIPTS = gmccc

gmccc: Makefile $(most_DEPENDENCIES) gmccc.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gmccc.input > $@)
            
endif HAVE_GSL

