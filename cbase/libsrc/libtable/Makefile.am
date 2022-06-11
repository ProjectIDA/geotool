AM_CPPFLAGS = @EXTRA_CPPFLAGS@
AM_CFLAGS = @EXTRA_CFLAGS@
AM_CXXFLAGS = @EXTRA_CXXFLAGS@
AM_FFLAGS = @EXTRA_FFLAGS@
AM_JAVACFLAGS = @EXTRA_JAVACFLAGS@
AM_LDFLAGS = @EXTRA_LDFLAGS@
AM_JARFLAGS = @EXTRA_JARFLAGS@

INCLUDES = @TABLE_INCLUDES@

if HAVE_TABLE

lib_LTLIBRARIES = \
	libtable.la

include_HEADERS = \
	table.h

noinst_HEADERS = \
	a_list.h \
	hash.h

man_MANS = \
	libtable.3 \
	table.3

endif HAVE_TABLE

libtable_la_SOURCES = \
	a_list.c \
	hash.c \
	list.c \
	setcvar.c \
	table.c \
	tree.c \
	variables.c \
	$(include_HEADERS) \
	$(noinst_HEADERS)

libtable_la_LIBADD = @TABLE_LIBADD@

libtable_la_LDFLAGS = -static

libtable_la_DEPENDENCIES = @TABLE_DEPENDENCIES@

EXTRA_DIST = \
	$(man_MANS) \
	Changes.libtable
