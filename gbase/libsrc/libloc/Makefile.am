AM_LDFLAGS = @EXTRA_LDFLAGS@

INCLUDES= -I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ibase

LIBS =

lib_LTLIBRARIES = libloc.la

libloc_la_SOURCES = \
	az_slow_corr.c \
	best_guess.c \
	blk_subs.c \
	compute_deltim.c \
	compute_hypo.c \
	daxpy.c \
	ddot.c \
	dnrm2.c \
	drot.c \
	drotg.c \
	dscal.c \
	dsvdc.c \
	dswap.c \
	ellips.c \
	ellipticity_corr.c \
	f_test.c \
	last_leg.c \
	loc_error_msg.c \
	locate_event.c \
	predsat.c \
	print_loc_results.c \
	read_tt_tables.c \
	solve_via_svd.c \
	srst.c \
	sssc.c \
	trv_time_default.c \
	trv_time_specific.c \
	ts_corr.c \
	tt_info.c

libloc_la_LDFLAGS = -shared
