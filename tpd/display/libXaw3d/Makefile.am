SUBDIRS = include src

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xaw3d.pc

MAINTAINERCLEANFILES = ChangeLog INSTALL

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
