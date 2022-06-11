AM_LDFLAGS = @EXTRA_LDFLAGS@

INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ibase

LIBS =

lib_LTLIBRARIES = libLP.la

libLP_la_SOURCES = \
	LP_trace_ray.c \
	geoc_distaz.c \
	geoc_lat_lon.c \
	non_Naperian.c

libLP_la_LDFLAGS = -shared
