INCLUDES = -I../../include

LIBS =

libdir = @libdir@/plugins

noinst_HEADERS = \
	Origins.h

lib_LTLIBRARIES = libgorigin.la

libgorigin_la_SOURCES = \
	libgorigin.cpp \
	Origins.cpp

EXTRA_DIST = gorigin.input

bin_SCRIPTS = gorigin

gorigin: Makefile $(most_DEPENDENCIES) gorigin.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gorigin.input > $@)

