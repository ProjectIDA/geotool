INCLUDES = -I../../include

LIBS =

libdir = @libdir@/plugins

noinst_HEADERS = \
	Polarization.h \
	PolarParam.h

lib_LTLIBRARIES = libgpolar.la

libgpolar_la_SOURCES = \
	libgpolar.cpp \
	Polarization.cpp \
	PolarParam.cpp

EXTRA_DIST = gpolar.input

bin_SCRIPTS = gpolar

gpolar: Makefile $(most_DEPENDENCIES) gpolar.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gpolar.input > $@)

