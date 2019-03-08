INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

AM_CFLAGS = -D__XKBDEFRULES__='"xorg"'

module_LTLIBRARIES = lxdrive.la

moduledir = $(libdir)/lxpanel/plugins

lxdrive_la_SOURCES = lxdrive.c

lxdrive_la_LIBADD = $(PACKAGE_LIBS)

lxdrive_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
