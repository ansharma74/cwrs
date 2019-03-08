## Process this file with automake to produce Makefile.in

bin_PROGRAMS = lxnm-applet

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnm_applet_SOURCES = \
	ui.h \
	ui.c \
	images.h \
	images.c \
	backend.h \
	backend.c \
	lxnm-applet.h \
	lxnm-applet.c


lxnm_applet_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)
