INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

module_LTLIBRARIES = menu2.la

moduledir = $(libdir)/lxpanel/plugins

menu2_la_SOURCES = menu.c

menu2_la_LIBADD = $(PACKAGE_LIBS)

menu2_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@

