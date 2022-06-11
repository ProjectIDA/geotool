INCLUDES= -I$(top_srcdir)/include -I$(top_srcdir)/include/ibase

LIBS =

noinst_HEADERS = libampP.h

lib_LTLIBRARIES = libgamp.la

libgamp_la_SOURCES = \
	amp.c \
	avg_max.c \
	butterworth_response.c \
	find_peak_trough.c \
	first_motion.c \
	interp_period.c \
	max_peak_trough.c \
	max_stav.c \
	peak_trough.c \
	remove_filt_resp.c \
	smooth_peak_trough.c

libgamp_la_LDFLAGS = -static
