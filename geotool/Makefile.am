ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
        doc  \
	include \
	libsrc \
	src \
	plugins \
	scripts \
	test	

EXTRA_DIST = \
	bootstrap \
	install-sh-clearcase \
        GNU.license \
	Doxyfile.input

noinst_SCRIPTS = Doxyfile

Doxyfile: Makefile $(geotool___OBJECTS) $(most_DEPENDENCIES)  Doxyfile.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< Doxyfile.input > $@)
