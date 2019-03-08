## Process this file with automake to produce Makefile.in

SUBDIRS = src po

desktopdir = $(datadir)/applications
desktop_in_files = lxsession-edit.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
DISTCLEANFILES = lxsession-edit.desktop

#icondir = $(datadir)/share/icons/hicolor/128x128
#icon_DATA = pcmanfm.png

uidir=$(datadir)/lxsession-edit
# GtkBuilder UI definition files
ui_in_files= \
	data/lxsession-edit.glade
ui_DATA = $(ui_in_files:.glade=.ui)

EXTRA_DIST = \
	autogen.sh \
	lxsession-edit.desktop.in \
	$(ui_DATA)

CLEANFILES = \
	$(ui_DATA) 

# Generate GtkBuilder UI files from Glade files
%.ui: %.glade
	$(GTK_BUILDER_CONVERT) $< $@
	$(top_builddir)/src/xml-purge $@

