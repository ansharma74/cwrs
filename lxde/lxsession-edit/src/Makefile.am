## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@

bin_PROGRAMS = lxsession-edit

lxsession_edit_SOURCES = \
	lxsession-edit.c

lxsession_edit_LDADD = @GTK_LIBS@ $(INTLLIBS)

noinst_PROGRAMS=xml-purge
xml_purge_SOURCES=xml-purge.c
xml_purge_CFLAGS=@GTK_CFLAGS@
xml_purge_LDADD=@GTK_LIBS@
