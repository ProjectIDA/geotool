AM_CPPFLAGS = @EXTRA_CPPFLAGS@
AM_CFLAGS = @EXTRA_CFLAGS@
AM_CXXFLAGS = @EXTRA_CXXFLAGS@
AM_FFLAGS = @EXTRA_FFLAGS@
AM_JAVACFLAGS = @EXTRA_JAVACFLAGS@
AM_LDFLAGS = @EXTRA_LDFLAGS@
AM_JARFLAGS = @EXTRA_JARFLAGS@

INCLUDES = @STDTIME_INCLUDES@

# _XOPEN_SOURCE to get strptime
# _XOPEN_SOURCE_EXTENDED for other extensions
DEFS += -D__svr4__ -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED

if HAVE_STDTIME

lib_LTLIBRARIES = \
	libstdtime.la

include_HEADERS = \
	libstdtime.h \
	libstdtime_f77.h

noinst_HEADERS = \
	libstdtimeP.h

man_MANS = \
	libstdtime.3

endif HAVE_STDTIME

libstdtime_la_SOURCES = \
	stdtime_asctime.c \
	stdtime_calendar.c \
	stdtime_cd.c \
	stdtime_check_value.c \
	stdtime_ctime.c \
	stdtime_errno.c \
	stdtime_expr_time.c \
	stdtime_format_time.c \
	stdtime_get_lddate.c \
	stdtime_gmt.c \
	stdtime_gse.c \
	stdtime_human_format.c \
	stdtime_julian.c \
	stdtime_localtime.c \
	stdtime_log_date.c \
	stdtime_milliTime.c \
	stdtime_new_formats.c \
	stdtime_present.c \
	stdtime_short_human.c \
	stdtime_string.c \
	stdtime_time_struct.c \
	stdtime_unformat_time.c \
	stdtime_yyyydoy_format.c \
	$(include_HEADERS) \
	$(noinst_HEADERS)

libstdtime_la_LIBADD = @STDTIME_LIBADD@

libstdtime_la_LDFLAGS = -static

libstdtime_la_DEPENDENCIES = @STDTIME_DEPENDENCIES@

# stdtime_expr_time_lex.c (and related files) requires yacc, but does not need
# to be regenerated every time. Leave out this complication for now.
EXTRA_DIST = \
	$(man_MANS) \
	Changes.libstdtime \
	stdtime_expr_time_lex.c \
	stdtime_expr_time.lex.l \
	stdtime_expr_time.y
