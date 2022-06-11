AM_CPPFLAGS = @EXTRA_CPPFLAGS@
AM_CFLAGS = @EXTRA_CFLAGS@
AM_CXXFLAGS = @EXTRA_CXXFLAGS@
AM_FFLAGS = @EXTRA_FFLAGS@
AM_JAVACFLAGS = @EXTRA_JAVACFLAGS@
AM_LDFLAGS = @EXTRA_LDFLAGS@
AM_JARFLAGS = @EXTRA_JARFLAGS@

INCLUDES = @AESIR_INCLUDES@

DEFS += -D__svr4__

if HAVE_AESIR

lib_LTLIBRARIES = \
	libaesir.la

include_HEADERS = \
	aesir.h \
	cdefs.h \
	dbug.h \
	dispatcher.h \
	dyn_array.h \
	error.h \
	libaesir.h \
	mask.h \
	sysdefs.h

man_MANS = \
	add_to_str.3 \
	array_create.3 \
	dyn_array.3 \
	error.3 \
	g2toint.3 \
	libaesir.3 \
	nap.3 \
	strtime.3

endif HAVE_AESIR

libaesir_la_SOURCES = \
	MilliTime.c \
	add_to_str.c \
	change_case.c \
	dbug.c \
	degain.c \
	dfxmsg.c \
	dyn_array.c \
	dyn_array_ext.c \
	error.c \
	expand_log_keys.c \
	fvec_funcs.c \
	mask.c \
	messages.c \
	nap.c \
	read.c \
	select.c \
	space.c \
	stringcat.c \
	strtime.c \
	version.c \
	write.c \
	$(include_HEADERS)

libaesir_la_LIBADD = @AESIR_LIBADD@

libaesir_la_LDFLAGS = -static

libaesir_la_DEPENDENCIES = @AESIR_DEPENDENCIES@

EXTRA_DIST = \
	$(man_MANS)
