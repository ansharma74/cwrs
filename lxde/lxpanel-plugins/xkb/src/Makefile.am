INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS) \
    -I/usr/include/libxml2


AM_CFLAGS = -D__XKBDEFRULES__='"xorg"'

module_LTLIBRARIES = lxkb.la

moduledir = $(libdir)/lxpanel/plugins

lxkb_la_SOURCES = xkb-config.h \
				  xkb-config.c \
				  lxkb.c

lxkb_la_LIBADD = $(PACKAGE_LIBS) \
                 -lxklavier \
                 -lxml2

lxkb_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
