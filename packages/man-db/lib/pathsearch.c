/*
 * pathsearch.c: $PATH-searching functions.
 *
 * Copyright (C) 2004, 2007, 2008, 2009, 2011 Colin Watson.
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
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xgetcwd.h"

#include "manconfig.h"
#include "pathsearch.h"

static int pathsearch (const char *name, const mode_t bits)
{
	char *cwd = NULL;
	char *path = getenv ("PATH");
	char *pathtok;
	const char *element;
	struct stat st;
	int ret = 0;

	if (!path)
		/* Eh? Oh well. */
		return 0;

	if (strchr (name, '/')) {
		/* Qualified name; look directly. */
		if (stat (name, &st) == -1)
			return 0;
		if (S_ISREG (st.st_mode) && (st.st_mode & bits))
			return 1;
		return 0;
	}

	pathtok = path = xstrdup (path);

	/* Unqualified name; iterate over $PATH looking for it. */
	for (element = strsep (&pathtok, ":"); element;
	     element = strsep (&pathtok, ":")) {
		char *filename;

		if (!*element) {
			if (!cwd)
				cwd = xgetcwd ();
			element = cwd;
		}

		filename = appendstr (NULL, element, "/", name, NULL);
		if (stat (filename, &st) == -1) {
			free (filename);
			continue;
		}

		free (filename);

		if (S_ISREG (st.st_mode) && (st.st_mode & bits)) {
			ret = 1;
			break;
		}
	}

	free (path);
	if (cwd)
		free (cwd);
	return ret;
}

int pathsearch_executable (const char *name)
{
	return pathsearch (name, 0111);
}

int directory_on_path (const char *dir)
{
	char *cwd = NULL;
	char *path = getenv ("PATH");
	char *pathtok;
	const char *element;
	int ret = 0;

	if (!path)
		/* Eh? Oh well. */
		return 0;

	pathtok = path = xstrdup (path);

	for (element = strsep (&pathtok, ":"); element;
	     element = strsep (&pathtok, ":")) {
		if (!*element) {
			if (!cwd)
				cwd = xgetcwd ();
			element = cwd;
		}

		if (STREQ (element, dir)) {
			ret = 1;
			break;
		}
	}

	free (path);
	if (cwd)
		free (cwd);
	return ret;
}
