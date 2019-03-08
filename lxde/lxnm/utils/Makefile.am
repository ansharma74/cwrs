## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = lxnetctl asc2hex getpppstat

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnetctl_SOURCES = \
	lxnetctl.c

lxnetctl_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

asc2hex_SOURCES = \
	asc2hex.c

asc2hex_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

getpppstat_SOURCES = \
	getpppstat.c \
	tdb.h \
	tdb.c \
	spinlock.h \
	spinlock.c

getpppstat_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)
