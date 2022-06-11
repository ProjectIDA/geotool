INCLUDES = -I../../include

LIBS =

libdir = @libdir@/plugins

noinst_HEADERS = \
	CepstrumParams.h \
	GCepstrum.h

lib_LTLIBRARIES = libgcepstrum.la

libgcepstrum_la_SOURCES = \
	CepstrumParams.cpp \
	GCepstrum.cpp \
	libgcepstrum.cpp

EXTRA_DIST = gcepstrum.input

bin_SCRIPTS = gcepstrum

gcepstrum: Makefile $(most_DEPENDENCIES) gcepstrum.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gcepstrum.input > $@)

