AM_CPPFLAGS = @EXTRA_CPPFLAGS@
AM_CFLAGS = @EXTRA_CFLAGS@
AM_CXXFLAGS = @EXTRA_CXXFLAGS@
AM_FFLAGS = @EXTRA_FFLAGS@
AM_JAVACFLAGS = @EXTRA_JAVACFLAGS@
AM_LDFLAGS = @EXTRA_LDFLAGS@
AM_JARFLAGS = @EXTRA_JARFLAGS@

INCLUDES = @PARIDC_INCLUDES@

DEFS += -DENVIRONMENT

if HAVE_PARIDC

lib_LTLIBRARIES = \
	libparidc.la

include_HEADERS = \
	libpar.h

man_MANS = \
	getarg.3 \
	getpar.3 \
	libnewpar.3 \
	libpar.3 \
	newgetpar.3

endif HAVE_PARIDC

libparidc_la_SOURCES = \
	buildarg.c \
	defpar.c \
	getarg.c \
	getpar.c \
	$(include_HEADERS)

libparidc_la_LIBADD = @PARIDC_LIBADD@

libparidc_la_LDFLAGS = -static

libparidc_la_DEPENDENCIES = @PARIDC_DEPENDENCIES@

EXTRA_DIST = \
	$(man_MANS)
