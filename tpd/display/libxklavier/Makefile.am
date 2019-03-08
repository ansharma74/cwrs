ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = libxklavier doc tests

EXTRA_DIST = libxklavier.spec libxklavier.spec.in \
             autogen.sh \
             CREDITS libxklavier.pc.in \
             m4/introspection.m4 \
             $(config_xml_DATA)

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --enable-introspection

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libxklavier.pc
