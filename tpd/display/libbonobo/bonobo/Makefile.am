
INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir) 				\
	$(WARN_CFLAGS)					\
	$(LIBBONOBO_CFLAGS)				\
	-DPREFIX=\""$(prefix)"\"			\
        -DBONOBO_LIBDIR=\""$(libdir)"\" 		\
        -DBONOBO_DATADIR=\""$(datadir)"\" 		\
        -DBONOBO_PIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBO_BINDIR=\""$(bindir)"\" 		\
        -DBONOBO_LOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBO_LOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBONOBO_EXPLICIT_TRANSLATION_DOMAIN="GETTEXT_PACKAGE"	\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"

AM_LDFLAGS=$(LIBBONOBO_LT_VERSION_INFO)

CORBA_SOURCE =			\
	Bonobo.h		\
	Bonobo-common.c		\
	Bonobo-stubs.c		\
	Bonobo-skels.c

idls =							\
	$(top_srcdir)/idl/Bonobo.idl			\
	$(top_srcdir)/idl/Bonobo_Canvas.idl		\
	$(top_srcdir)/idl/Bonobo_Context.idl		\
	$(top_srcdir)/idl/Bonobo_Control.idl		\
	$(top_srcdir)/idl/Bonobo_Embeddable.idl		\
	$(top_srcdir)/idl/Bonobo_Gdk.idl		\
	$(top_srcdir)/idl/Bonobo_ItemContainer.idl	\
	$(top_srcdir)/idl/Bonobo_Listener.idl		\
	$(top_srcdir)/idl/Bonobo_Moniker.idl		\
	$(top_srcdir)/idl/Bonobo_Persist.idl		\
	$(top_srcdir)/idl/Bonobo_Print.idl		\
	$(top_srcdir)/idl/Bonobo_Property.idl		\
	$(top_srcdir)/idl/Bonobo_Storage.idl		\
	$(top_srcdir)/idl/Bonobo_UI.idl			\
	$(top_srcdir)/idl/Bonobo_Zoomable.idl		\
	$(top_srcdir)/idl/Bonobo_Exception.idl		\
	$(top_srcdir)/idl/Bonobo_Application.idl	\
	$(top_srcdir)/idl/Bonobo_Clipboard.idl

idl_flags = -I$(top_srcdir)/idl -D__Bonobo_COMPILATION

$(CORBA_SOURCE): $(idls) $(ORBIT_IDL)
	$(AM_V_GEN) $(ORBIT_IDL) $(idl_flags) $(top_srcdir)/idl/Bonobo.idl

lib_LTLIBRARIES = libbonobo-2.la

libbonobo_2_la_LIBADD = \
	$(LIBBONOBO_LIBS) \
	$(top_builddir)/bonobo-activation/libbonobo-activation.la \
	$(INTLLIBS)

libbonobo_2_la_LDFLAGS = -no-undefined

libbonoboincludedir = $(includedir)/libbonobo-2.0/bonobo

libbonoboinclude_HEADERS =		\
	Bonobo.h			\
	bonobo-arg.h			\
	bonobo-context.h		\
	bonobo-event-source.h		\
	bonobo-exception.h		\
	bonobo-generic-factory.h	\
	bonobo-item-container.h		\
	bonobo-item-handler.h		\
	bonobo-listener.h		\
	bonobo-main.h			\
	bonobo-macros.h			\
	bonobo-moniker-extender.h	\
	bonobo-moniker-simple.h		\
	bonobo-moniker-util.h		\
	bonobo-moniker.h		\
	bonobo-object.h			\
	bonobo-foreign-object.h		\
	bonobo-persist-file.h		\
	bonobo-persist-stream.h		\
	bonobo-persist.h		\
	bonobo-persist-client.h		\
	bonobo-property-bag.h		\
	bonobo-property-bag-client.h	\
	bonobo-shlib-factory.h		\
	bonobo-storage.h		\
	bonobo-stream.h			\
	bonobo-stream-client.h		\
	bonobo-stream-memory.h		\
	bonobo-storage-memory.h		\
	bonobo-xobject.h		\
	bonobo-i18n.h			\
	bonobo-application.h		\
	bonobo-app-client.h		\
	bonobo-types.h

noinst_HEADERS =			\
	bonobo-moniker-context.h	\
	bonobo-running-context.h	\
	bonobo-private.h		\
	bonobo-marshal.h

libbonobo_src =				\
	bonobo-moniker-context.c	\
	bonobo-arg.c			\
	bonobo-context.c		\
	bonobo-event-source.c		\
	bonobo-exception.c		\
	bonobo-generic-factory.c	\
	bonobo-item-container.c		\
	bonobo-item-handler.c		\
	bonobo-listener.c		\
	bonobo-main.c			\
	bonobo-moniker.c		\
	bonobo-moniker-extender.c	\
	bonobo-moniker-simple.c		\
	bonobo-moniker-util.c		\
	bonobo-object.c			\
	bonobo-foreign-object.c		\
	bonobo-persist-file.c		\
	bonobo-persist-stream.c		\
	bonobo-persist.c		\
	bonobo-persist-client.c		\
	bonobo-property-bag.c		\
	bonobo-property-bag-client.c	\
	bonobo-shlib-factory.c		\
	bonobo-storage.c		\
	bonobo-stream-client.c		\
	bonobo-stream-memory.c		\
	bonobo-storage-memory.c		\
	bonobo-running-context.c	\
	bonobo-application.c		\
	bonobo-app-client.c		\
	bonobo-types.c			\
	bonobo-debug.c			\
	bonobo-debug.h

$(libbonobo_2_la_OBJECTS): Bonobo.h $(marshal_sources)

libbonobo_2_la_SOURCES =		\
	$(libbonoboinclude_HEADERS)	\
	$(libbonobo_src)		\
	$(CORBA_SOURCE)

marshal_sources = \
	bonobo-marshal.c \
	bonobo-marshal.h

bonobo-marshal.h: bonobo-marshal.list Makefile
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=bonobo_marshal > $@
bonobo-marshal.c: bonobo-marshal.list Makefile
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --body --prefix=bonobo_marshal > $@

EXTRA_DIST = libbonobo-2.0.pc.in libbonobo-2.0-uninstalled.pc.in bonobo-marshal.list

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbonobo-2.0.pc

BUILT_SOURCES = $(CORBA_SOURCE) $(marshal_sources)
CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

