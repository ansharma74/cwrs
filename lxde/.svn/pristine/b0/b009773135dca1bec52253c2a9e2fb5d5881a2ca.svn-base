SUBDIRS = src po

# GtkBuilder UI definition files
uidir=$(datadir)/lxshortcut
ui_in_files= \
	data/lxshortcut.glade \
	data/choose-icon.glade
ui_DATA = $(ui_in_files:.glade=.ui)

# Generate GtkBuilder UI files from Glade files
%.ui: %.glade
	$(GTK_BUILDER_CONVERT) $< $@

#desktopfiledir=$(datadir)/applications
#desktopfile_DATA=data/lxrandr.desktop

EXTRA_DIST = \
	$(ui_DATA)
#	autogen.sh

CLEANFILES = \
	$(ui_DATA) 
