SUBDIRS = docs idl libspi login-helper registryd atk-bridge cspi pyatspi test po

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libspi-1.0.pc cspi-1.0.pc libloginhelper-1.0.pc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

schemasdir       = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = at-spi.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
		if test -z "$(DESTDIR)" ; then \
			for p in $(schemas_DATA) ; do \
				GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$$p ; \
			done \
		fi
else
install-data-local:
endif

EXTRA_DIST = \
	$(schemas_in_files) \
	MAINTAINERS \
	cspi-1.0.pc.in \
	cspi-1.0-uninstalled.pc.in \
	libloginhelper-1.0.pc.in \
	libspi-1.0.pc.in \
	libspi-1.0-uninstalled.pc.in

CLEANFILES = \
	$(schemas_DATA)
