INCLUDES= -I$(top_srcdir)/include -I. \
	-I$(top_srcdir)/include/ibase

LIBS =

lib_LTLIBRARIES = libgeog.la

libgeog_la_SOURCES = \
	area_of_interest.c \
	azimuth_cross_pt.c \
	dist_azimuth.c \
	ellip_dist.c \
	ellipticity_corr.c \
	in_polygon.c \
	lat_conv.c \
	lat_lon.c \
	reg.c \
	small_circle_cross_pts.c

libgeog_la_LDFLAGS = -static
