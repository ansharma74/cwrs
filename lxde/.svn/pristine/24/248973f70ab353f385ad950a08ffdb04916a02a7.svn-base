SUBDIRS = src po

#pixmapsdir = $(datadir)/pixmaps
#pixmaps_DATA = gtknetcat.png

uidir   = $(datadir)/gtknetcat/
ui_DATA = data/wizard.glade

appdesktopdir = $(datadir)/applications
appdesktop_in_files = gtknetcat.desktop.in
appdesktop_DATA = $(appdesktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

DISTCLEANFILES =                \
	$(appdesktop_DATA)		\
        intltool-extract        \
        intltool-merge          \
        intltool-update

EXTRA_DIST = \
	$(appdesktop_in_files) \
	$(appdesktop_DATA) \
	$(ui_DATA) \
	$(pixmaps_DATA) \
        intltool-update.in \
        intltool-extract.in \
        intltool-merge.in

