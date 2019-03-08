/*
 * wordfnmatch.c: fnmatch on word boundaries
 *
 * Copyright (C) 2001, 2003, 2008 Colin Watson.
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
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <ctype.h>

#include "fnmatch.h"

#include "manconfig.h"

#include "lower.h"
#include "wordfnmatch.h"

/* TODO: How on earth do we allow multiple-word matches without
 * reimplementing fnmatch()?
 */
int word_fnmatch (const char *lowpattern, const char *string)
{
	char *lowstring = lower (string);
	char *begin = lowstring, *p;

	for (p = lowstring; *p; p++) {
		if (CTYPE (islower, *p) || *p == '_')
			continue;

		/* Check for multiple non-word characters in a row. */
		if (p <= begin + 1)
			begin++;
		else {
			*p = '\0';
			if (fnmatch (lowpattern, begin, 0) == 0) {
				free (lowstring);
				return 1;
			}
			begin = p + 1;
		}
	}

	free (lowstring);
	return 0;
}
