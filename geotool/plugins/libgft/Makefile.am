INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	FT.h \
	FtPlotClass.h \
	FtPlot.h \
	FtPlotP.h \
	FtSmooth.h \
	FtWindows.h

lib_LTLIBRARIES = libgft.la

libgft_la_SOURCES = \
	FT.cpp \
	FtPlotClass.cpp \
	FtPlot.cpp \
	FtSmooth.cpp \
	FtWindows.cpp \
	libgft.cpp

EXTRA_DIST = gft.input

bin_SCRIPTS = gft

gft: Makefile $(most_DEPENDENCIES) gft.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gft.input > $@)

endif HAVE_GSL

