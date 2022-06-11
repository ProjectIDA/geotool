INCLUDES= -I$(top_srcdir)/include -I$(top_srcdir)/include/ibase

noinst_HEADERS = libdataqcp.h

LIBS =

lib_LTLIBRARIES = libgqc.la

libgqc_la_SOURCES = \
	percentile.c \
	points.c \
	qc.c \
	segments.c \
	sequences.c \
	spike.c

libgqc_la_LDFLAGS = -static
