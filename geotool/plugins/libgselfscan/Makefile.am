INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	SelfScan.h \
	SelfScanParams.h 	

lib_LTLIBRARIES = \
	libgselfscan.la

libgselfscan_la_SOURCES = \
	libgselfscan.cpp \
	SelfScan.cpp \
	SelfScanParams.cpp 

libgselfscan_la_LIBADD = \
	-L../../libsrc/libcluster -lcluster \
	-L../../libsrc/libmccc -lmccc

EXTRA_DIST = \
	gselfscan.input

bin_SCRIPTS = \
	gselfscan

gselfscan: Makefile $(most_DEPENDENCIES) gselfscan.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gselfscan.input > $@)

endif HAVE_GSL

