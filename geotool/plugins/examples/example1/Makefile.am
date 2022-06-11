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
	Example1.h

lib_LTLIBRARIES = libexample1.la

libexample1_la_SOURCES = \
	Example1.cpp \
	libExample1.cpp

