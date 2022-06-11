INCLUDES = -I../../include

LIBS =

libdir = @libdir@/plugins

noinst_HEADERS = \
	Resp.h \
	RespTapers.h

lib_LTLIBRARIES = libgrsp.la

libgrsp_la_SOURCES = \
	libgrsp.cpp \
	Resp.cpp \
	RespTapers.cpp

EXTRA_DIST = grsp.input

bin_SCRIPTS = grsp

grsp: Makefile $(most_DEPENDENCIES) grsp.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< grsp.input > $@)

