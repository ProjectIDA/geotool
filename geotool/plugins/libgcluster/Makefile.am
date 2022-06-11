INCLUDES = -I../../include 

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	GCluster.h \
	GClusterParams.h \
	GClusterSummary.h

lib_LTLIBRARIES = \
	libgcluster.la

libgcluster_la_SOURCES = \
	libgcluster.cpp \
	GCluster.cpp \
	GClusterParams.cpp \
	GClusterSummary.cpp

libgcluster_la_LIBADD = \
	-L../../libsrc/libcluster -lcluster \
	-L../../libsrc/libmccc -lmccc

EXTRA_DIST = \
	gcluster.input

bin_SCRIPTS = \
	gcluster

gcluster: Makefile $(most_DEPENDENCIES) gcluster.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gcluster.input > $@)

endif HAVE_GSL

