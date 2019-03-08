/*
 * convert_name.c
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2012
 *               Colin Watson.
 *
 * This file is part of man-db.
 *
 * man-db is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * man-db is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with man-db; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * code to do appropriate pathname conversion
 *
 * Mon May  2 11:14:28 BST 1994 Wilf. (G.Wilford@ee.surrey.ac.uk)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"

#include "manp.h"
#include "convert_name.h"

static inline void gripe_converting_name (const char *name) ATTRIBUTE_NORETURN;
static inline void gripe_converting_name (const char *name)
{
	error (FATAL, 0, _("Can't convert %s to cat name"), name);
	abort (); /* error should have exited; help compilers prove noreturn */
}

/* Convert the trailing part of 'name' to be a cat page path by altering its
 * extension appropriately. If fsstnd is set, also try converting the
 * containing directory name from "man1" to "cat1" etc., returning NULL if
 * that doesn't work.
 *
 * fsstnd should only be set if name is the original path of a man page
 * found in a man hierarchy, not something like a symlink target or a file
 * named with 'man -l'. Otherwise, a symlink to "/home/manuel/foo.1.gz"
 * would be converted to "/home/catuel/foo.1.gz", which would be bad.
 */
char *convert_name (const char *name, int fsstnd)
{
	char *to_name, *t1 = NULL;
	char *t2 = NULL;
#ifdef COMP_SRC
	struct compression *comp;
#endif /* COMP_SRC */
	char *namestem;

#ifdef COMP_SRC
	comp = comp_info (name, 1);
	if (comp)
		namestem = comp->stem;
	else
#endif /* COMP_SRC */
		namestem = xstrdup (name);

#ifdef COMP_CAT
	/* TODO: BSD layout requires .0. */
	to_name = appendstr (NULL, namestem, "." COMPRESS_EXT, NULL);
#else /* !COMP_CAT */
	to_name = xstrdup (namestem);
#endif /* COMP_CAT */
	free (namestem);

	if (fsstnd) {
		t1 = strrchr (to_name, '/');
		if (!t1)
			gripe_converting_name (name);
		*t1 = '\0';

		t2 = strrchr (to_name, '/');
		if (!t2)
			gripe_converting_name (name);
		++t2;
		*t1 = '/';

		if (STRNEQ (t2, "man", 3)) {
			/* If the second-last component starts with "man",
			 * replace "man" with "cat".
			 */
			*t2 = 'c';
			*(t2 + 2) = 't';
		} else {
			free (to_name);
			debug ("couldn't convert %s to FSSTND cat file\n",
			       name);
			return NULL;
		}
	}

	debug ("converted %s to %s\n", name, to_name);

	return to_name;
}
