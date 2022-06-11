INCLUDES = -I../../include

LIBS =

libdir = @libdir@/plugins

noinst_HEADERS = \
	AddGroup.h \
	AddRecipe.h \
	BeamGroups.h \
	BGStations.h \
	Recipes.h \
	Slowness.h

lib_LTLIBRARIES = libgbm.la

libgbm_la_SOURCES = \
	AddGroup.cpp \
	AddRecipe.cpp \
	BeamGroups.cpp \
	BGStations.cpp \
	libgbm.cpp \
	Recipes.cpp \
	Slowness.cpp

EXTRA_DIST = gdbm.input gobm.input

bin_SCRIPTS = gdbm gobm

gdbm: Makefile $(most_DEPENDENCIES) gdbm.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gdbm.input > $@)

gobm: Makefile $(most_DEPENDENCIES) gobm.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gobm.input > $@)

