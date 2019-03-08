/*
 * hashtable.h: contains struct nlist
 *  
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2002, 2003, 2004, 2005, 2007, 2009 Colin Watson.
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
 * Sat Aug 20 15:01:02 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 */

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

typedef void (*hashtable_free_ptr) (void *defn);

struct hashtable {
	struct nlist **hashtab;		/* the storage array */
	int unique;			/* unique hash values */
	int identical;			/* identical hash values */
	hashtable_free_ptr free_defn;	/* function to free a hash entry */
};

struct nlist {
        struct nlist *next;	/* next in the chain */
        char *name;		/* the _name_ */
        void *defn;		/* the _definition_ */
};

struct hashtable_iter;

extern void null_hashtable_free (void *defn);
extern void plain_hashtable_free (void *defn);

extern struct hashtable *hashtable_create (hashtable_free_ptr free_defn);
extern struct nlist *hashtable_lookup_structure (const struct hashtable *ht,
						 const char *s, size_t len);
extern void *hashtable_lookup (const struct hashtable *ht,
			       const char *s, size_t len);
extern struct nlist *hashtable_install (struct hashtable *ht,
					const char *name, size_t len,
					void *defn);
extern struct nlist *hashtable_iterate (const struct hashtable *ht,
					struct hashtable_iter **iterp);
extern void hashtable_remove (struct hashtable *ht, const char *s, size_t len);
extern void hashtable_free (struct hashtable *ht);

#endif /* _HASHTABLE_H */
