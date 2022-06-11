INCLUDES = -I../../include

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = Ftrace.h

lib_LTLIBRARIES = libgftrace.la

libgftrace_la_SOURCES = \
	libgftrace.cpp \
	Ftrace.cpp

EXTRA_DIST = gftrace.input

bin_SCRIPTS = gftrace

gftrace: Makefile $(most_DEPENDENCIES) gftrace.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gftrace.input > $@)

endif HAVE_GSL                   

