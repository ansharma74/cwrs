NULL=

idldir = $(datadir)/idl/bonobo-2.0

idl_DATA =				\
	Bonobo.idl			\
	Bonobo_Canvas.idl		\
	Bonobo_Context.idl		\
	Bonobo_Control.idl		\
	Bonobo_Embeddable.idl		\
	Bonobo_Gdk.idl			\
	Bonobo_ItemContainer.idl	\
	Bonobo_Listener.idl		\
	Bonobo_Moniker.idl		\
	Bonobo_Persist.idl		\
	Bonobo_Print.idl		\
	Bonobo_Property.idl		\
	Bonobo_Storage.idl		\
	Bonobo_UI.idl			\
	Bonobo_Zoomable.idl		\
	Bonobo_Exception.idl		\
	Bonobo_Clipboard.idl		\
	Bonobo_Application.idl		\
	$(NULL)

activation_idldir=$(datadir)/idl/bonobo-activation-2.0
activation_idl_DATA =                   \
	Bonobo_Unknown.idl              \
	Bonobo_GenericFactory.idl       \
	Bonobo_Activation_types.idl     \
	Bonobo_ObjectDirectory.idl      \
	Bonobo_ActivationContext.idl

EXTRA_DIST = $(idl_DATA) $(activation_idl_DATA)

orbittypelib_LTLIBRARIES = Bonobo_module.la
orbittypelibdir = $(libdir)/orbit-2.0

IMODULE_SOURCE =	\
	Bonobo.h	\
	Bonobo-common.c	\
	Bonobo-imodule.c

idl_flags = \
	-I$(top_srcdir)/idl \
	-D__Bonobo_COMPILATION \
	-D__Bonobo_Unknown_COMPILATION \
	-D__Bonobo_GenericFactory_COMPILATION \
	-D__Bonobo_Activation_types_COMPILATION

INCLUDES =  \
	-I. \
	$(LIBBONOBO_CFLAGS) \
	$(WARN_CFLAGS)

Bonobo_module_la_SOURCES = \
	Bonobo-imodule.c

Bonobo_module_la_LIBADD = \
	$(LIBBONOBO_LIBS)

Bonobo_module_la_LDFLAGS = \
	-module -avoid-version -no-undefined

$(IMODULE_SOURCE): $(idl_DATA) $(ORBIT_IDL)
	$(AM_V_GEN) $(ORBIT_IDL) $(idl_flags) --imodule $(top_srcdir)/idl/Bonobo.idl

BUILT_SOURCES = $(IMODULE_SOURCE)
CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
