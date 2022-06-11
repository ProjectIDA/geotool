INCLUDES = -I../../include

LIBS = $(GSL_LIB)

libdir = @libdir@/plugins

if HAVE_GSL

noinst_HEADERS = \
	Fk3D.h \
	FKGram3C.h \
	FKGram.h \
	FK.h \
	FkParamDialog.h \
	FKParam.h \
	FkSignal.h \
	ggl.h

if HAVE_OPENGL

lib_LTLIBRARIES = libgfk.la libgfk3d.la

else

lib_LTLIBRARIES = libgfk.la

endif

libgfk_la_SOURCES = \
	FK.cpp \
	FKGram3C.cpp \
	FKGram.cpp \
	FKParam.cpp \
	FkParamDialog.cpp \
	FkSignal.cpp \
	libgfk.cpp

if HAVE_OPENGL

libgfk3d_la_SOURCES = \
	con.c \
	Fk3D.cpp \
	ggl_widget.c \
	libgfk3d.cpp

endif

EXTRA_DIST = gfk.input gfkmb.input

bin_SCRIPTS = gfk gfkmb


gfk: Makefile $(most_DEPENDENCIES) gfk.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gfk.input > $@)
                        
gfkmb: Makefile $(most_DEPENDENCIES) gfkmb.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gfkmb.input > $@)

endif HAVE_GSL

