## Process this file with automake to produce Makefile.in
##
## Copyright (C) 2009, 2010, 2011 Colin Watson.
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

TESTS_ENVIRONMENT = PATH=..:$$PATH DBTYPE=$(DBTYPE) \
		    top_builddir=$(top_builddir) \
		    @LOCALCHARSET_TESTS_ENVIRONMENT@ $(SHELL)
ALL_TESTS = \
	lexgrog-1 \
	man-1 man-2 man-3 \
	manconv-1 manconv-2 manconv-3 \
	mandb-1 mandb-2 mandb-3 mandb-4 mandb-5 mandb-6 \
	whatis-1 \
	zsoelim-1
if !CROSS_COMPILING
TESTS = $(ALL_TESTS)
endif

dist_check_SCRIPTS = testlib.sh $(ALL_TESTS)
