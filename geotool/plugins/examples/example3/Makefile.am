INCLUDES = -I../../../include -I../../../gbase/include

LIBS =

# The library is intalled in the local directory. 
# To load this example plugin into geotool++, move the library to
# $(prefix)/lib/plugins/  (The installation_top_dir/lib/plugins/)
# or move the library to ~/.geotool++/plugins/
#
#  mv *.so* ~/geotool++/plugins
#

libdir = $(PWD)

noinst_HEADERS = \
	Example3.h \
	hilbt.h

lib_LTLIBRARIES = libexample3.la

libexample3_la_SOURCES = \
	Example3.cpp \
	hilbt.cpp \
	libExample3.cpp

