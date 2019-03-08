#
# Makefile for DAHDI Linux kernel modules and tools
#
# Copyright (C) 2008 Digium, Inc.
#
#

all:
	$(MAKE) -C linux all
	(cd tools && [ -f config.status ] || ./configure --with-dahdi=../linux)
	$(MAKE) -C tools all

clean:
	$(MAKE) -C linux clean
	$(MAKE) -C tools clean

distclean: clean
	$(MAKE) -C linux distclean
	$(MAKE) -C tools distclean

dist-clean: distclean

install: all
	$(MAKE) -C linux install
	$(MAKE) -C tools install

config: install
	$(MAKE) -C tools config

update:
	@if [ -d .svn ]; then \
		echo "Updating from Subversion..." ; \
		svn update --non-recursive .; \
		rm -f .version; \
		$(MAKE) -C linux update; \
		$(MAKE) -C tools update; \
	else \
		echo "Not under version control";  \
	fi

.PHONY: all clean distclean dist-clean install config update
