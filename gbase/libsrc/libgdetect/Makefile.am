INCLUDES= -I$(top_srcdir)/include -I$(top_srcdir)/include/ibase

LIBS =

noinst_HEADERS = libdetectP.h

lib_LTLIBRARIES = libgdetect.la

libgdetect_la_SOURCES = \
	avg.c \
	calc_deltime.c \
	detect_funcs.c \
	ltav.c \
	snr.c \
	standard_snr.c \
	state.c \
	stav.c \
	z_stat_snr.c

libgdetect_la_LDFLAGS = -shared
