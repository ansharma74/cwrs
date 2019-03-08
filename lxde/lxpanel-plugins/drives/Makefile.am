SUBDIRS = src po

drivedir =  $(datadir)/lxpanel/images
drive_DATA = \
        data/empty.png

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
    $(drive_DATA)

install-exec-hook:
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.la
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.a
