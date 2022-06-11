SUBDIRS = test_data test_scripts

docdir = $(prefix)/test

EXTRA_DIST = test_geotool.input

bin_SCRIPTS = test_geotool

test_geotool: Makefile $(most_DEPENDENCIES) test_geotool.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< test_geotool.input > $@)

