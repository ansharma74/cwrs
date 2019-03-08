SUBDIRS = \
	idl bonobo-activation bonobo \
	activation-server monikers \
	utils samples tests doc po

libincludedir = $(includedir)/libbonobo-2.0
libinclude_HEADERS = \
	libbonobo.h

EXTRA_DIST = \
	MAINTAINERS \
	bonobo-activation-config.xml \
	libbonobo-zip.in     \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	changes.txt          \
	gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

install-data-local:
	-mkdir -p $(DESTDIR)$(sysconfdir)/bonobo-activation
	if test -f $(DESTDIR)$(sysconfdir)/bonobo-activation/bonobo-activation-config.xml; then	\
		echo "bonobo-activation-config.xml configuration file not changed";		\
	else											\
		$(INSTALL_DATA) $(top_srcdir)/bonobo-activation-config.xml			\
			$(DESTDIR)$(sysconfdir)/bonobo-activation;				\
	fi

uninstall-local:
	if cmp $(DESTDIR)$(sysconfdir)/bonobo-activation/bonobo-activation-config.xml $(top_srcdir)/bonobo-activation-config.xml > /dev/null ; then \
		rm -f $(DESTDIR)$(sysconfdir)/bonobo-activation/bonobo-activation-config.xml; \
	fi

DISTCLEANFILES=intltool-extract intltool-merge intltool-update
