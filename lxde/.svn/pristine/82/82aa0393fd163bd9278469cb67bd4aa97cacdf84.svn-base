SUBDIRS = src po

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

install-data-local:
	cp -fr $(srcdir)/data/* $(DESTDIR)/$(LXPANEL_DATADIR)/images

install-exec-hook:
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.la
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.a
