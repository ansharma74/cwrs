## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = lxnm

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnm_SOURCES = \
	port.h \
	device.h \
	device.c \
	modem.h \
	modem.c \
	info.h \
	info.c \
	status.c \
	status.h \
	misc.c \
	misc.h \
	thread.c \
	thread.h \
	handler.c \
	handler.h \
	timer.c \
	timer.h \
	wireless.c \
	wireless.h \
	lxnm.h \
	lxnm.c

lxnm_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS) \
		-liw
