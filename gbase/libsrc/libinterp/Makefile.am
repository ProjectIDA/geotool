AM_LDFLAGS = @EXTRA_LDFLAGS@

INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ibase

LIBS =

lib_LTLIBRARIES = libinterp.la

libinterp_la_SOURCES = \
	bracket_vectors.c \
	interp_tbl_val.c \
	nrutil.c \
	ratint.c \
	splie2.c \
	splin2.c \
	spline.c \
	splint.c \
	splint_deriv.c

libinterp_la_LDFLAGS = -shared
