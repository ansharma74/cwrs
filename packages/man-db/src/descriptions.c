/*
 * descriptions.c: manipulate man page descriptions
 *
 * Copyright (C) 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011 Colin Watson.
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

#include <string.h>
#include <stdlib.h>

#include "manconfig.h"
#include "descriptions.h"

/* Parse the description in a whatis line returned by find_name() into a
 * sequence of names and whatis descriptions.
 */
struct page_description *parse_descriptions (const char *base,
					     const char *whatis)
{
	const char *sep, *nextsep;
	struct page_description *desc = NULL, *head = NULL;
	int seen_base = 0;

	if (!whatis)
		return NULL;

	sep = whatis;

	while (sep) {
		char *record;
		size_t length;
		const char *dash;
		char *names;
		const char *token;

		/* Use a while loop so that we skip over things like the
		 * result of double line breaks.
		 */
		while (*sep == 0x11 || *sep == ' ')
			++sep;
		nextsep = strchr (sep, 0x11);

		/* Get this record as a null-terminated string. */
		if (nextsep)
			length = (size_t) (nextsep - sep);
		else
			length = strlen (sep);
		if (length == 0)
			break;

		record = xstrndup (sep, length);
		debug ("record = '%s'\n", record);

		/* Split the record into name and whatis description. */
		dash = strstr (record, " - ");
		if (dash)
			names = xstrndup (record, dash - record);
		else if (!desc)
			/* Some pages have a NAME section with just the page
			 * name and no whatis.  We might as well include
			 * this.
			 */
			names = xstrdup (record);
		else
			/* Once at least one record has been seen, further
			 * cases where there is no whatis usually amount to
			 * garbage following the useful records, and can
			 * cause problems due to false WHATIS_MAN entries in
			 * the database.  On the whole it seems best to
			 * ignore these.
			 */
			goto next;

		for (token = strtok (names, ","); token;
		     token = strtok (NULL, ",")) {
			char *name = trim_spaces (token);

			/* Skip name tokens containing whitespace. They are
			 * almost never useful as manual page names.
			 */
			if (strpbrk (name, " \t") != NULL) {
				free (name);
				continue;
			}

			/* Allocate new description node. */
			if (head) {
				desc->next = xmalloc (sizeof *desc);
				desc = desc->next;
			} else {
				desc = xmalloc (sizeof *desc);
				head = desc;
			}
			desc->name   = name; /* steal memory */
			desc->whatis = dash ? trim_spaces (dash + 3) : NULL;
			desc->next   = NULL;

			if (base && STREQ (base, desc->name))
				seen_base = 1;
		}

		free (names);
next:
		free (record);

		sep = nextsep;
	}

	/* If it isn't there already, add the base name onto the returned
	 * list.
	 */
	if (base && !seen_base) {
		if (head) {
			desc->next = xmalloc (sizeof *desc);
			desc = desc->next;
			desc->whatis =
				head->whatis ? xstrdup (head->whatis) : NULL;
		} else {
			desc = xmalloc (sizeof *desc);
			head = desc;
			desc->whatis = NULL;
		}
		desc->name = xstrdup (base);
		desc->next = NULL;
	}

	return head;
}

/* Free a description list and all its contents. */
void free_descriptions (struct page_description *head)
{
	struct page_description *desc = head, *prev;

	while (desc) {
		free (desc->name);
		if (desc->whatis)
			free (desc->whatis);
		prev = desc;
		desc = desc->next;
		free (prev);
	}
}
