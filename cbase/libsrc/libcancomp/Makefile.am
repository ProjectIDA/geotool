AM_CPPFLAGS = @EXTRA_CPPFLAGS@
AM_CFLAGS = @EXTRA_CFLAGS@
AM_CXXFLAGS = @EXTRA_CXXFLAGS@
AM_FFLAGS = @EXTRA_FFLAGS@
AM_JAVACFLAGS = @EXTRA_JAVACFLAGS@
AM_LDFLAGS = @EXTRA_LDFLAGS@
AM_JARFLAGS = @EXTRA_JARFLAGS@

INCLUDES = @CANCOMP_INCLUDES@

if HAVE_CANCOMP

lib_LTLIBRARIES = \
	libcancomp.la

include_HEADERS = \
	canada_compress.h

man_MANS = \
	cancomp.3

endif HAVE_CANCOMP

libcancomp_la_SOURCES = \
	canada.c \
	$(include_HEADERS)

libcancomp_la_LIBADD = @CANCOMP_LIBADD@

libcancomp_la_LDFLAGS = -static

libcancomp_la_DEPENDENCIES = @CANCOMP_DEPENDENCIES@

EXTRA_DIST = \
	$(man_MANS) \
	Changes.libcancomp \
	Rnotes.libcancomp
