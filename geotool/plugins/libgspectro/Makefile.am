INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	Spectro.h \
	SpectroParam.h

lib_LTLIBRARIES = libgspectro.la

libgspectro_la_SOURCES = \
	libgspectro.cpp \
	Spectro.cpp \
	SpectroParam.cpp

EXTRA_DIST = gspectro.input

bin_SCRIPTS = gspectro

gspectro: Makefile $(most_DEPENDENCIES) gspectro.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gspectro.input > $@)

endif HAVE_GSL

