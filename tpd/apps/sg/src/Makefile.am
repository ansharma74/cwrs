gladedir = $(datadir)/firestarter/glade/
SUBDIRS = xpm

INCLUDES = \
	$(WARN_CFLAGS) \
	@FIRESTARTER_CFLAGS@ \
	-DG_LOG_DOMAIN=\"Firestarter\" \
	-DFIRESTARTER_RULES_DIR=\"@sysconfdir@\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGLADEDIR=\""$(datadir)/firestarter/glade"\"

bin_PROGRAMS = firestarter

firestarter_SOURCES = \
	firestarter.c	\
	gui.c		\
	util.c		\
	logread.c	\
	menus.c  	\
	wizard.c 	\
	wizard-choices.c \
	service.c	\
	preferences.c	\
	scriptwriter.c	\
	savelog.c	\
	netfilter-script.c \
	hitview.c	\
	eggtrayicon.c	\
	tray.c		\
	dhcp-server.c	\
	statusview.c	\
	policyview.c	\
	globals.h	\
	firestarter.h	\
	gui.h		\
	util.h		\
	logread.h	\
	menus.h  	\
	wizard.h 	\
	wizard-choices.h \
	service.h	\
	preferences.h	\
	scriptwriter.h	\
	savelog.h	\
	netfilter-script.h \
	hitview.h	\
	eggtrayicon.h	\
	tray.h		\
	dhcp-server.h	\
	statusview.h	\
	policyview.h

glade_DATA = \
	preferences.glade

firestarter_LDADD = @FIRESTARTER_LIBS@

EXTRA_DIST = $(glade_DATA)
