SUBDIRS = init

docdir = $(prefix)/scripts

doc_DATA =  \
	align_on_phase \
	cmdline \
	detect1 \
	detect2 \
	exchange_instrument \
	fbands \
	fk \
	messages \
	README \
	spectro \
	table_example \
	view_arrival
EXTRA_DIST = \
	align_on_phase \
	cmdline \
	detect1 \
	detect2 \
	exchange_instrument \
	fbands \
	fk \
	gconvert.input \
	messages \
	README \
	spectro \
	table_example \
	view_arrival

bin_SCRIPTS = gconvert

gconvert: Makefile $(most_DEPENDENCIES) gconvert.input
	$(RM) $@
	(g=$(prefix); sed -e "s|%INSTALLATION_DIRECTORY%|$${g}|" \
	< gconvert.input > $@)

