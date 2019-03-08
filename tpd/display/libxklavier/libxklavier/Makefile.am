if LIBXKBFILE_PRESENT
  LIBXKBFILE_PRESENT_CFLAG = -DLIBXKBFILE_PRESENT=1
  LIBXKBFILE_PRESENT_LDFLAGS = -lxkbfile
endif

if ENABLE_XKB_SUPPORT
  ENABLE_XKB_SUPPORT_CFLAG = -DENABLE_XKB_SUPPORT=1
else
  ENABLE_XKB_SUPPORT_CFLAG = -DDISABLE_XKB_SUPPORT=1
endif

if ENABLE_XMODMAP_SUPPORT
  ENABLE_XMODMAP_SUPPORT_CFLAG = -DENABLE_XMODMAP_SUPPORT=1
else
  ENABLE_XMODMAP_SUPPORT_CFLAG = -DDISABLE_XMODMAP_SUPPORT=1
endif

AM_CFLAGS=-Wall -DDATA_DIR=\"$(datadir)/$(PACKAGE)\" \
  -I. -I$(top_srcdir) $(X_CFLAGS) \
  $(XML_CFLAGS) $(GLIB_CFLAGS) $(XINPUT_CFLAGS) \
  $(LIBXKBFILE_PRESENT_CFLAG) \
  $(ENABLE_XKB_SUPPORT_CFLAG) \
  $(ENABLE_XMODMAP_SUPPORT_CFLAG)

lib_LTLIBRARIES = libxklavier.la
noinst_HEADERS = xklavier_private.h xklavier_private_xkb.h xklavier_private_xmm.h
xklavier_headers = xkl_engine.h xkl_config_item.h xkl_config_registry.h \
	xkl_config_rec.h xkl_engine_marshal.h xklavier.h

BUILT_SOURCES = $(xklavier_built_headers) $(xklavier_built_cfiles)

xklavier_built_headers = xkl-enum-types.h
xklavier_built_cfiles = xkl-enum-types.c
xklavierincdir = $(includedir)/libxklavier
xklavierinc_HEADERS = $(xklavier_headers) $(xklavier_built_headers)

libxklavier_la_SOURCES = $(xklavier_built_cfiles) xklavier.c xklavier_evt.c xklavier_config.c xklavier_config_iso.c \
	xklavier_xkb.c xklavier_evt_xkb.c xklavier_config_xkb.c xklavier_toplevel.c \
	xklavier_xmm.c xklavier_xmm_opts.c xklavier_evt_xmm.c xklavier_config_xmm.c \
	xklavier_util.c xklavier_props.c xklavier_dump.c xkl_engine_marshal.c \
	$(noinst_HEADERS) $(xklavierinc_HEADERS) 
libxklavier_la_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined -export-symbols $(srcdir)/libxklavier.public
libxklavier_la_LIBADD = \
 $(XML_LIBS) $(GLIB_LIBS) $(XINPUT_LIBS) \
 $(LIBXKBFILE_PRESENT_LDFLAGS) \
 $(X_LIBS) -lX11 $(LIBICONV) 

EXTRA_DIST=marshal.list libxklavier.public

GLIB_GENMARSHAL = `$(PKG_CONFIG) --variable=glib_genmarshal glib-2.0`

xkl_engine_marshal.h: marshal.list
	$(GLIB_GENMARSHAL) --prefix=xkl_engine $(srcdir)/marshal.list --header > xkl_engine_marshal.h

xkl_engine_marshal.c: xkl_engine_marshal.h
	$(GLIB_GENMARSHAL) --prefix=xkl_engine $(srcdir)/marshal.list --body > xkl_engine_marshal.c

xkl-enum-types.h: stamp-xkl-enum-types.h
	@true
stamp-xkl-enum-types.h: $(xklavier_headers)
	(cd $(srcdir) \
	&& glib-mkenums \
		--fhead "#ifndef __XKL_ENUM_TYPES_H__\n#define __XKL_ENUM_TYPES_H__\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define XKL_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "#endif /* __XKL_ENUM_TYPES_H__ */" \
		$(xklavier_headers)) > new-xkl-enum-types.h \
	&& (cmp -s new-xkl-enum-types.h xkl-enum-types.h || cp new-xkl-enum-types.h xkl-enum-types.h) \
	&& rm -f new-xkl-enum-types.h \
	&& echo timestamp > $(@F)

xkl-enum-types.c: $(xklavier_headers) xkl-enum-types.h
	(cd $(srcdir) \
	&& glib-mkenums \
		--fhead "#include <libxklavier/xklavier.h>" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(xklavier_headers)) > new-xkl-enum-types.c \
	&& cp new-xkl-enum-types.c $(@F) \
	&& rm -f new-xkl-enum-types.c

DISTCLEANFILES=$(xklavier_built_headers) \
		$(xklavier_built_cfiles) \
		stamp-xkl-enum-types.h \
		xkl_engine_marshal.c \
		xkl_engine_marshal.h
CLEANFILES=

# build intospection typelib
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
# avoid introspecting private headers
introspection_sources = $(xklavier_headers) $(xklavier_built_headers) $(filter %.c, $(libxklavier_la_SOURCES))

Xkl-1.0.gir: libxklavier.la
Xkl_1_0_gir_INCLUDES = GObject-2.0 xlib-2.0
Xkl_1_0_gir_CFLAGS = -I$(top_srcdir) -I$(top_builddir) $(INCLUDES) $(X_CFLAGS) $(XML_CFLAGS) $(GLIB_CFLAGS) $(XINPUT_CFLAGS)
Xkl_1_0_gir_LIBS = libxklavier.la
Xkl_1_0_gir_FILES = $(xklavier_headers) $(introspection_sources)
INTROSPECTION_GIRS += Xkl-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
