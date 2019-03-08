/*
 * db_gdbm.c: low level gdbm interface routines for man.
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
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
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef GDBM

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "manconfig.h"

#include "hashtable.h"
#include "cleanup.h"

#include "mydbm.h"

static struct hashtable *parent_sortkey_hash;

struct sortkey {
	datum key;
	struct sortkey *next;
};

/* setjmp/longjmp handling to defend against _gdbm_fatal exiting under our
 * feet.  Not thread-safe, but there is no plan for man-db to ever use
 * threads.
 */
static jmp_buf open_env;
static int opening;

/* Mimic _gdbm_fatal's error output, but handle errors during open more
 * gracefully than exiting.
 */
static void trap_error (const char *val)
{
	if (opening) {
		debug ("gdbm error: %s\n", val);
		longjmp (open_env, 1);
	} else
		fprintf (stderr, "gdbm fatal: %s\n", val);
}

man_gdbm_wrapper man_gdbm_open_wrapper (const char *name, int flags)
{
	man_gdbm_wrapper wrap;
	GDBM_FILE file;
	datum key, content;

	opening = 1;
	if (setjmp (open_env))
		return NULL;
	file = gdbm_open ((char *) name, BLK_SIZE, flags, DBMODE, trap_error);
	if (!file)
		return NULL;

	wrap = xmalloc (sizeof *wrap);
	wrap->name = xstrdup (name);
	wrap->file = file;

	if ((flags & ~GDBM_FAST) != GDBM_NEWDB) {
		/* While the setjmp/longjmp guard is in effect, make sure we
		 * can read from the database at all.
		 */
		memset (&key, 0, sizeof key);
		MYDBM_SET (key, xstrdup (VER_KEY));
		content = MYDBM_FETCH (wrap, key);
		free (MYDBM_DPTR (key));
		free (MYDBM_DPTR (content));
	}

	opening = 0;

	return wrap;
}

static void parent_sortkey_hashtable_free (void *defn)
{
	/* Automatically free child hashtables on removal. */
	hashtable_free ((struct hashtable *) defn);
}

static void sortkey_hashtable_free (void *defn)
{
	struct sortkey *key = (struct sortkey *) defn;
	free (MYDBM_DPTR (key->key));
	free (key);
}

static int sortkey_compare (const void *a, const void *b)
{
	const struct sortkey **left = (const struct sortkey **) a;
	const struct sortkey **right = (const struct sortkey **) b;
	int cmp;
	size_t minsize;

	/* Sentinel NULL elements sort to the end. */
	if (!MYDBM_DPTR ((*left)->key))
		return 1;
	else if (!MYDBM_DPTR ((*right)->key))
		return -1;

	if (MYDBM_DSIZE ((*left)->key) < MYDBM_DSIZE ((*right)->key))
		minsize = MYDBM_DSIZE ((*left)->key);
	else
		minsize = MYDBM_DSIZE ((*right)->key);
	cmp = strncmp (MYDBM_DPTR ((*left)->key), MYDBM_DPTR ((*right)->key),
		       minsize);
	if (cmp)
		return cmp;
	else if (MYDBM_DSIZE ((*left)->key) < MYDBM_DSIZE ((*right)->key))
		return 1;
	else if (MYDBM_DSIZE ((*left)->key) > MYDBM_DSIZE ((*right)->key))
		return -1;
	else
		return 0;
}

static datum empty_datum = { NULL, 0 };

/* We keep a hashtable of filenames to sorted lists of keys. Each list is
 * stored both with links from each element to the next and in a hashtable,
 * so that both sequential access and random access are quick. This is
 * necessary for a reasonable ordered implementation of nextkey.
 */
datum man_gdbm_firstkey (man_gdbm_wrapper wrap)
{
	struct hashtable *sortkey_hash;
	struct sortkey **keys, *firstkey;
	int numkeys = 0, maxkeys = 256;
	int i;

	/* Build the raw list of keys and sort it. */
	keys = xnmalloc (maxkeys, sizeof *keys);
	keys[0] = xmalloc (sizeof **keys);
	keys[0]->key = gdbm_firstkey (wrap->file);
	while (MYDBM_DPTR (keys[numkeys]->key)) {
		if (++numkeys >= maxkeys) {
			maxkeys *= 2;
			keys = xnrealloc (keys, maxkeys, sizeof *keys);
		}
		keys[numkeys] = xmalloc (sizeof **keys);
		keys[numkeys]->key =
			gdbm_nextkey (wrap->file, keys[numkeys - 1]->key);
	}
	free (keys[numkeys]);
	keys[numkeys] = NULL;	/* simplifies the empty case */
	qsort (keys, numkeys, sizeof *keys, &sortkey_compare);

	/* Link the elements together and insert them into a hash. */
	sortkey_hash = hashtable_create (&sortkey_hashtable_free);
	for (i = 0; i < numkeys; ++i) {
		if (i < numkeys - 1)
			keys[i]->next = keys[i + 1];
		else
			keys[i]->next = NULL;
		hashtable_install (sortkey_hash,
				   MYDBM_DPTR (keys[i]->key),
				   MYDBM_DSIZE (keys[i]->key),
				   keys[i]);
	}
	firstkey = keys[0];
	free (keys);	/* element memory now owned by hashtable */

	if (!parent_sortkey_hash) {
		parent_sortkey_hash = hashtable_create
			(&parent_sortkey_hashtable_free);
		push_cleanup ((cleanup_fun) hashtable_free,
			      parent_sortkey_hash, 0);
	}

	/* Remember this structure for use by nextkey. */
	hashtable_install (parent_sortkey_hash,
			   wrap->name, strlen (wrap->name), sortkey_hash);

	if (firstkey)
		return copy_datum (firstkey->key);
	else
		return empty_datum; /* dptr is NULL, so no copy needed */
}

datum man_gdbm_nextkey (man_gdbm_wrapper wrap, datum key)
{
	struct hashtable *sortkey_hash;
	struct sortkey *sortkey;

	if (!parent_sortkey_hash)
		return empty_datum;
	sortkey_hash = hashtable_lookup (parent_sortkey_hash,
					 wrap->name, strlen (wrap->name));
	if (!sortkey_hash)
		return empty_datum;

	sortkey = hashtable_lookup (sortkey_hash,
				    MYDBM_DPTR (key), MYDBM_DSIZE (key));
	if (!sortkey || !sortkey->next)
		return empty_datum;

	return copy_datum (sortkey->next->key);
}

void man_gdbm_close (man_gdbm_wrapper wrap)
{
	if (!wrap)
		return;

	if (parent_sortkey_hash) {
		struct hashtable *sortkey_hash =
			hashtable_lookup (parent_sortkey_hash,
					  wrap->name, strlen (wrap->name));
		if (sortkey_hash)
			hashtable_remove (parent_sortkey_hash,
					  wrap->name, strlen (wrap->name));
	}

	free (wrap->name);
	gdbm_close (wrap->file);
	free (wrap);
}

#ifndef HAVE_GDBM_EXISTS

int gdbm_exists (GDBM_FILE file, datum key)
{
	char *memory;

	memory = MYDBM_DPTR (gdbm_fetch (file, key));
	if (memory) {
		free (memory);
		return 1;
	}

	return 0;
}

#endif /* !HAVE_GDBM_EXISTS */

#endif /* GDBM */
