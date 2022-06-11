INCLUDES = -I../../include

LIBS = -lgCDF -lshape $(NETCDF_LIB)

libdir = @libdir@/plugins

noinst_HEADERS = \
	MapCursor.h \
	MapMeasureForm.h \
	MapOverlayForm.h \
	MapOverlay.h \
	MapThemeColors.h \
	MapThemeFile.h \
	MapThemes.h \
	MapWindow.h \
	ShapeTable.h


lib_LTLIBRARIES = libgmap.la

libgmap_la_SOURCES = \
	libgmap.cpp \
	MapCursor.cpp \
	MapMeasureForm.cpp \
	MapOverlay.cpp \
	MapOverlayForm.cpp \
	MapThemeColors.cpp \
	MapThemeFile.cpp \
	MapThemes.cpp \
	MapWindow.cpp \
	ShapeTable.cpp

EXTRA_DIST = gmap.input

bin_SCRIPTS = gmap

gmap: Makefile $(most_DEPENDENCIES) gmap.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gmap.input > $@)

