NULL =

sbin_PROGRAMS = lxdm-binary

lxdm_binary_CFLAGS = \
	$(GTK_CFLAGS) \
	-DCONFIG_FILE=\"@sysconfdir@/@PACKAGE@/@PACKAGE@.conf\" \
	-DXSESSIONS_DIR=\"@datarootdir@/xsessions\" \
	-DLXDM_DATA_DIR=@datadir@/@PACKAGE@ \
	$(CONSOLEKIT_CFLAGS) \
	-Werror-implicit-function-declaration \
	-Wall \
	$(NULL)

lxdm_binary_LDADD = \
	$(GTK_LIBS) \
	$(CONSOLEKIT_LIBS) \
	$(NULL)

lxdm_binary_SOURCES = \
	lxdm.c lxdm.h \
	ui.c \
	$(NULL)

libexec_PROGRAMS = lxdm-greeter-gtk

lxdm_greeter_gtk_CFLAGS = \
	$(GTK_CFLAGS) \
	-DCONFIG_FILE=\"@sysconfdir@/@PACKAGE@/@PACKAGE@.conf\" \
	-DXSESSIONS_DIR=\"@datarootdir@/xsessions\" \
	-DLXDM_DATA_DIR=\"@datadir@/@PACKAGE@\" \
	-DPACKAGE_LOCALE_DIR=\"@localedir@\" \
	-DGNOMELOCALEDIR=\"@localedir@\" \
	-DLIBLOCALEDIR=\"@prefix@/lib/locale\" \
	-Werror-implicit-function-declaration
	$(NULL)

lxdm_greeter_gtk_LDADD = \
	$(GTK_LIBS) \
	$(NULL)

lxdm_greeter_gtk_SOURCES = \
	greeter.c \
	lang.c lang.h \
	gdm/gdm-languages.c gdm/gdm-languages.h \
	gdm/locarchive.h \
	$(NULL)

