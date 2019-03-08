SUBDIRS = src utils data man

EXTRA_DIST = \
	autogen.sh \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

install-exec-hook:
	for script in $(DESTDIR)/$(datadir)/lxnm/*.sh; do \
	    chmod +x $$script; \
	done
