## Process this file with automake to produce Makefile.in
##
## Copyright (C) 2008 Colin Watson.
##
## This file is part of man-db.
##
## man-db is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## man-db is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with man-db; if not, write to the Free Software Foundation,
## Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

POFILES = po/de.po po/fr.po po/id.po po/ja.po po/nl.po po/pl.po po/ru.po

EXTRA_DIST = po4a.cfg Locale/Po4a/Manext.pm po/man-db-manpages.pot $(POFILES)

if PO4A
PO4A_ENVIRONMENT = PERL5LIB=$(srcdir)

# Always generate output files even if poorly translated, since Automake's
# generated rules will get upset otherwise.
PO4A_ARGS = --variable srcdir=$(top_srcdir)/man \
	    --variable builddir=$(top_builddir)/man \
	    --keep 0

all-local:
	$(PO4A_ENVIRONMENT) $(PO4A) $(PO4A_ARGS) $(srcdir)/po4a.cfg

update-po:
	$(PO4A_ENVIRONMENT) $(PO4A) $(PO4A_ARGS) --no-translations \
		$(srcdir)/po4a.cfg

po/man-db-manpages.pot: update-po
endif
