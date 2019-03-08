INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"/lxshortcut\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@ \
	-Werror-implicit-function-declaration

bin_PROGRAMS = lxshortcut

lxshortcut_SOURCES = \
	lxshortcut.c 

lxshortcut_LDADD = @GTK_LIBS@ $(INTLLIBS)

