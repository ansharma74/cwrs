/*
 * db_delete.c: dbdelete(), database delete routine.
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002 Colin Watson.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Mon Aug  8 20:35:30 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"

#include "mydbm.h"
#include "db_storage.h"

/* Delete an entry for a page.
   Again, 3 possibilities:

   1) page is singular reference, just delete it :)
   2) page has 2+ companions. Delete page and alter multi entry to not
      point to it anymore.
   3) page has 1 companion. Could do as (2), but we'd waste an entry in
      the db. Should delete page, extract friend and reinsert as singular,
      overwriting the old multi entry.
*/

#define NO_ENTRY	1;

int dbdelete (const char *name, struct mandata *info)
{
	datum key, cont;

	memset (&key, 0, sizeof key);
	memset (&cont, 0, sizeof cont);

	/* get entry for info */

	debug ("Attempting delete of %s(%s) entry.\n", name, info->ext);

	MYDBM_SET (key, name_to_key (name));
	cont = MYDBM_FETCH (dbf, key);

	if (!MYDBM_DPTR (cont)) {			/* 0 entries */
		free (MYDBM_DPTR (key));
		return NO_ENTRY;
	} else if (*MYDBM_DPTR (cont) != '\t') {	/* 1 entry */
		MYDBM_DELETE (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (cont));
	} else {					/* 2+ entries */
		char **names, **ext;
		char *multi_content = NULL;
		datum multi_key;
		int refs, i, j;

		/* Extract all of the extensions associated with
		   this key */

		refs = list_extensions (MYDBM_DPTR (cont) + 1, &names, &ext);

		for (i = 0; i < refs; ++i)
			if (STREQ (names[i], name) &&
			    STREQ (ext[i], info->ext))
				break;

		if (i >= refs) {
			free (names);
			free (ext);
			MYDBM_FREE (MYDBM_DPTR (cont));
			free (MYDBM_DPTR (key));
			return NO_ENTRY;
		}

		multi_key = make_multi_key (names[i], ext[i]);
		if (!MYDBM_EXISTS (dbf, multi_key)) {
			error (0, 0,
			       _( "multi key %s does not exist"),
			       MYDBM_DPTR (multi_key));
			gripe_corrupt_data ();
		}
		MYDBM_DELETE (dbf, multi_key);
		free (MYDBM_DPTR (multi_key));

		/* refs *may* be 1 if all manual pages with this name
		   have been deleted. In this case, we'll have to remove
		   the key too */

		if (refs == 1) {
			free (names);
			free (ext);
			MYDBM_FREE (MYDBM_DPTR (cont));
			MYDBM_DELETE (dbf, key);
			free (MYDBM_DPTR (key));
			return 0;
		}

		/* create our new multi content */
		for (j = 0; j < refs; ++j)
			if (i != j)
				multi_content = appendstr (multi_content,
							   "\t", names[j],
							   "\t", ext[j], NULL);

		MYDBM_FREE (MYDBM_DPTR (cont));

		/* if refs = 2 do something else. Doesn't really matter as
		   the gdbm db file does not shrink any after a deletion
		   anyway */

		MYDBM_SET (cont, multi_content);

		if (MYDBM_REPLACE (dbf, key, cont))
			gripe_replace_key (MYDBM_DPTR (key));

		free (names);
		free (ext);
	}

	free (MYDBM_DPTR (key));
	return 0;
}
