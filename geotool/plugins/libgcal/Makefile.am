INCLUDES = -I../../include

LIBS = -lcalib

libdir = @libdir@/plugins

noinst_HEADERS = \
	Calibration.h \
	CalParam.h \
	Iteration.h

lib_LTLIBRARIES = libgcal.la

libgcal_la_SOURCES = \
	Calibration.cpp \
	CalParam.cpp \
	Iteration.cpp \
	libgcal.cpp

EXTRA_DIST = gcal.input

bin_SCRIPTS = gcal

gcal: Makefile $(most_DEPENDENCIES) gcal.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gcal.input > $@)

