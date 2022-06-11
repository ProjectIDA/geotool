INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ibase

LIBS =

DEFS = @DEFS@  -D__svr4__

lib_LTLIBRARIES = libmagnitude.la

libmagnitude_la_SOURCES = \
        build_mag_obj.c \
        calc_mags.c \
        mag_access.c \
        mag_boot_strap.c \
        mag_error_msg.c \
        mag_max_lik.c \
        mag_params.c \
        mag_utils.c \
        network_mag.c \
        only_bound_amps.c \
        read_mdf.c \
        read_tl_table.c \
        TL_error_msg.c \
        TL_manipulation.c

man_MANS = libmagnitude.3

EXTRA_DIST = $(man_MANS) \
             Changes.libmagnitude \
             Makefile.orig \
             libmagnitude_version.c

libmagnitude_la_LDFLAGS = -static
