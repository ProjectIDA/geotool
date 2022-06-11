INCLUDES = -I../../include

LIBS = -linterp -lloc

libdir = @libdir@/plugins

noinst_HEADERS = \
	Details.h \
	Locate.h \
	Residuals.h

lib_LTLIBRARIES = libglc.la

libglc_la_SOURCES = \
	Details.cpp \
	libglc.cpp \
	Locate.cpp \
	Residuals.cpp

libglc_la_LIBADD = -linterp -lloc

EXTRA_DIST = glc.input

bin_SCRIPTS = glc

glc: Makefile $(most_DEPENDENCIES) glc.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< glc.input > $@)



