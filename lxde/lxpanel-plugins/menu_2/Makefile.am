SUBDIRS = src po

menudir =  $(datadir)/lxpanel/images
menu_DATA = data/menu.png \
            data/menu-button.png
       

EXTRA_DIST = \
	$(menu_DATA)

install-exec-hook:
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.la
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.a
