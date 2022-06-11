INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	Correlation.h \
	MinCorrOverlap.h

lib_LTLIBRARIES = libgcor.la

libgcor_la_SOURCES = \
	Correlation.cpp \
	libgcor.cpp \
	MinCorrOverlap.cpp \
	tsCorrelate.cpp

EXTRA_DIST = gcor.input

bin_SCRIPTS = gcor

gcor: Makefile $(most_DEPENDENCIES) gcor.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gcor.input > $@)

endif HAVE_GSL


