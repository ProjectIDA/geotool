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
	Example2.h

lib_LTLIBRARIES = libexample2.la

libexample2_la_SOURCES = \
	Example2.cpp \
	libExample2.cpp

