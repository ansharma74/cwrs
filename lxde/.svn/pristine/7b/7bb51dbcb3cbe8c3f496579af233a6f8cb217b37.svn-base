INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

module_LTLIBRARIES = example.la

moduledir = $(libdir)/lxpanel/plugins

example_la_SOURCES = \
	example.c

example_la_LIBADD = \
	$(PACKAGE_LIBS)

example_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
