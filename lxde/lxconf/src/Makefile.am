INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

#lib_LIBRARIES = liblxconf.a
lib_LTLIBRARIES = liblxconf.la

liblxconf_la_SOURCES = \
	liblxconf.c \
	lxconf-client.c \
	lxconf-db.c

liblxconf_la_LDFLAGS = -version-info 1:0:1

if HAVE_SQLITE3
liblxconf_la_SOURCES += sqlite3-db.c
else
liblxconf_la_SOURCES += glib-db.c
endif

liblxconf_a_CFLAGS = $(PACKAGE_CFAGS)


libl_xconf_includedir = $(includedir)/lxconf

libl_xconf_include_HEADERS = lxconf/lxconf.h lxconf/lxconf-client.h lxconf/lxconf-db.h

EXTRA_DIST = liblxconf.pc.in

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = liblxconf.pc

bin_PROGRAMS = lxconfd

lxconfd_SOURCES = lxconfd.c
lxconfd_LDADD = liblxconf.la $(PACKAGE_LIBS) $(INTLLIBS)