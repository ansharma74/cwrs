INCLUDES = \
	-I$(top_srcdir) 				\
	$(WARN_CFLAGS)					\
	$(LIBBONOBO_CFLAGS)				\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
        -DBONOBO_LIBDIR=\""$(libdir)"\" 		\
        -DBONOBO_DATADIR=\""$(datadir)"\" 		\
        -DBONOBO_PIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBO_BINDIR=\""$(bindir)"\" 		\
        -DBONOBO_LOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBO_LOCALEDIR=\""$(datadir)/locale"\" 	\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"

Bonobo_Moniker_std.server.in: Bonobo_Moniker_std.server.in.in
	$(AM_V_GEN) $(SED) -e "s|\@MONIKER_LIBDIR\@|$(monikerdir_in_server_file)|" -e "s|\@SOPREFIX\@|$(SOPREFIX)|" $< > $@

CLEANFILES = Bonobo_Moniker_std.server Bonobo_Moniker_std.server.in

serverdir = $(libdir)/bonobo/servers
server_DATA = Bonobo_Moniker_std.server

@INTLTOOL_SERVER_RULE@

monikerdir = $(libdir)/bonobo/monikers

if OS_WIN32
monikerdir_in_server_file = ../monikers
else
monikerdir_in_server_file = $(monikerdir)
endif

moniker_LTLIBRARIES = libmoniker_std_2.la

libmoniker_std_2_la_SOURCES =			\
	bonobo-moniker-extender-stream.c  	\
	bonobo-moniker-item.c			\
	bonobo-moniker-new.c			\
	bonobo-moniker-ior.c			\
	bonobo-moniker-oaf.c			\
	bonobo-moniker-cache.c			\
	bonobo-stream-cache.h			\
	bonobo-stream-cache.c			\
	bonobo-moniker-query.c			\
	bonobo-moniker-query.h			\
	bonobo-moniker-std.h            	\
	bonobo-moniker-std.c			

libmoniker_std_2_la_LDFLAGS = -module -avoid-version -no-undefined
libmoniker_std_2_la_LIBADD = $(top_builddir)/bonobo/libbonobo-2.la \
	$(top_builddir)/bonobo-activation/libbonobo-activation.la \
	$(LIBBONOBO_LIBS) \
	$(INTLLIBS)

EXTRA_DIST = Bonobo_Moniker_std.server.in.in
