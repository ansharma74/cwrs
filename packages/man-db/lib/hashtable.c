/*
 * hashtable.c: in core hash table routines.
 *  
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2002, 2003, 2004, 2005, 2007, 2008, 2009 Colin Watson.
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
 * All of these routines except hashtable_free() can be found in K&R II.
 *
 * Sat Aug 20 15:01:02 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 */

/* which hash function do we want ? */
/* #define PROLOGUE */
#define KRII 

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>

#include "manconfig.h"
#include "hashtable.h"

#if defined(PROLOGUE)
#define HASHSIZE 2048
#elif defined(KRII)
#define HASHSIZE 2001
#else
#error hash function not defined
#endif

/* Return hash value for string. */
static unsigned int hash (const char *s, size_t len)
{
	unsigned int hashval = 0;
	size_t i;

	for (i = 0; i < len && s[i]; ++i)
#if defined(KRII)
		hashval = s[i] + 31 * hashval;
	return hashval % HASHSIZE;
#elif defined(PROLOGUE)
		hashval = (hashval << 1) + s[i];
	return hashval & (HASHSIZE - 1);
#endif
}

void null_hashtable_free (void *defn ATTRIBUTE_UNUSED)
{
}

void plain_hashtable_free (void *defn)
{
	if (defn)
		free (defn);
}

/* Create a hashtable. */
struct hashtable *hashtable_create (hashtable_free_ptr free_defn)
{
	struct hashtable *ht = XMALLOC (struct hashtable);
	ht->hashtab = XCALLOC (HASHSIZE, struct nlist *);
	ht->unique = 0;
	ht->identical = 0;
	ht->free_defn = free_defn;
	return ht;
}

/* Return pointer to hash entry structure containing s, or NULL if it
 * doesn't exist.
 */
struct nlist *hashtable_lookup_structure (const struct hashtable *ht,
					  const char *s, size_t len)
{
	struct nlist *np;

	for (np = ht->hashtab[hash (s, len)]; np; np = np->next) {
		if (strncmp (s, np->name, len) == 0)
			return np;
	}
	return NULL;
}

/* Return pointer to definition of s, or NULL if it doesn't exist. */
void *hashtable_lookup (const struct hashtable *ht, const char *s, size_t len)
{
	struct nlist *np = hashtable_lookup_structure (ht, s, len);
	if (np)
		return np->defn;
	else
		return NULL;
}

/* Return structure containing definition (never NULL). */
struct nlist *hashtable_install (struct hashtable *ht,
				 const char *name, size_t len, void *defn)
{
	struct nlist *np;

	np = hashtable_lookup_structure (ht, name, len);
	if (np) {
		if (np->defn)
			ht->free_defn (np->defn);
	} else {
		unsigned int hashval;

		np = XMALLOC (struct nlist);
		np->name = xstrndup (name, len);
		hashval = hash (name, len);

		/* record uniqueness if debugging */
		if (debug_level) {
			if (ht->hashtab[hashval])
				ht->identical++;
			else
				ht->unique++;
		}

		/* point to last entry with this hash */
		np->next = ht->hashtab[hashval];

		/* attach to hashtab array */
		ht->hashtab[hashval] = np;
	}

	np->defn = defn;

	return np;
}

/* Remove structure containing name from the hash tree. */
void hashtable_remove (struct hashtable *ht, const char *name, size_t len)
{
	struct nlist *np, *prev;
	unsigned int hashval = hash (name, len);

	for (np = ht->hashtab[hashval], prev = NULL; np;
	     prev = np, np = np->next) {
		if (strncmp (name, np->name, len) == 0) {
			if (prev)
				prev->next = np->next;
			else
				ht->hashtab[hashval] = np->next;
			if (np->defn)
				ht->free_defn (np->defn);
			free (np->name);
			free (np);
			return;
		}
	}
}

struct hashtable_iter {
	struct nlist **bucket;
	struct nlist *np;
};

/* Iterate over hash.  Do not modify hash while iterating. */
struct nlist *hashtable_iterate (const struct hashtable *ht,
				 struct hashtable_iter **iterp)
{
	struct hashtable_iter *iter = *iterp;

	if (!iter)
		*iterp = iter = XZALLOC (struct hashtable_iter);

	if (iter->np && iter->np->next)
		return iter->np = iter->np->next;

	if (iter->bucket)
		++iter->bucket;
	else
		iter->bucket = ht->hashtab;

	while (iter->bucket < ht->hashtab + HASHSIZE) {
		if (*iter->bucket)
			return iter->np = *iter->bucket;
		++iter->bucket;
	}

	free (iter);
	*iterp = NULL;
	return NULL;
}

/* Free up the hash tree (garbage collection). Also call the free_defn()
 * hook to free up values if necessary.
 */
void hashtable_free (struct hashtable *ht)
{
	int i;

	if (!ht)
		return;

	debug ("hashtable_free: %d entries, %d (%d%%) unique\n",
	       ht->unique + ht->identical,
	       ht->unique,
	       ht->unique ? (ht->unique * 100) / (ht->unique + ht->identical)
	                  : 0);

	for (i = 0; i < HASHSIZE; i++) {
		struct nlist *np;

		np = ht->hashtab[i];
		while (np) {
			struct nlist *next;

			if (np->defn)
				ht->free_defn (np->defn);
			free (np->name);
			next = np->next;
			free (np);
			np = next;
		}
	}

	free (ht->hashtab);
	free (ht);
}
