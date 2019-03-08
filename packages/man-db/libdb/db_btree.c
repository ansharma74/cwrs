/*
 * db_btree.c: low level btree interface routines for man.
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2002 Colin Watson.
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

/* below this line are routines only useful for the BTREE interface */
#ifdef BTREE

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/file.h> /* for flock() */
#include <sys/types.h> /* for open() */
#include <sys/stat.h>

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#include <unistd.h>

#include "manconfig.h"

#include "error.h"
#include "hashtable.h"

#include "mydbm.h"
#include "db_storage.h"

struct hashtable *loop_check_hash;

/* the Berkeley database libraries do nothing to arbitrate between concurrent
   database accesses, so we do a simple flock(). If the db is opened in
   anything but O_RDONLY mode, an exclusive lock is enabled. Otherwise, the
   lock is shared. A file cannot have both locks at once, and the non
   blocking method is used ": Try again". This adopts GNU dbm's approach. */

#ifdef FAST_BTREE
#define B_FLAGS		R_DUP 	/* R_DUP: allow duplicates in tree */

void test_insert (int line, const datum key, const datum cont)
{
	debug ("(%d) key: \"%s\", cont: \"%.40s\"\n",
	       line, MYDBM_DPTR (key), MYDBM_DPTR (cont));
}
#else /* !FAST_BTREE */
#define B_FLAGS		0	/* do not allow any duplicate keys */
#endif /* FAST_BTREE */

/* release the lock and close the database */
int btree_close (DB *db)
{
	(void) flock ((db->fd) (db), LOCK_UN);
	return (db->close) (db);
}

/* open a btree type database, with file locking. */
DB *btree_flopen (char *filename, int flags, int mode)
{
	DB *db;
	BTREEINFO b;
	int lock_op;
	int lock_failed;

	b.flags = B_FLAGS;

	b.cachesize = 0;	/* default size */
	b.maxkeypage = 0;	/* default */
	b.minkeypage = 0;	/* default */
	b.psize = 0;		/* default page size (2048?) */
	b.compare = NULL;	/* builtin compare() function */
	b.prefix = NULL;	/* builtin function */
	b.lorder = 0;		/* byte order of host */

	if (flags & ~O_RDONLY) {
		/* flags includes O_RDWR or O_WRONLY, need an exclusive lock */
		lock_op = LOCK_EX | LOCK_NB;
	} else {
		lock_op = LOCK_SH | LOCK_NB;
	}

	if (!(flags & O_CREAT)) {
		/* Berkeley DB thinks that a zero-length file means that
		 * somebody else is writing it, and sleeps for a few
		 * seconds to give the writer a chance. All very well, but
		 * the common case is that the database is just zero-length
		 * because mandb was interrupted or ran out of disk space or
		 * something like that - so we check for this case by hand
		 * and ignore the database if it's zero-length.
		 */
		struct stat iszero;
		if (stat (filename, &iszero) < 0)
			return NULL;
		if (iszero.st_size == 0) {
			errno = EINVAL;
			return NULL;
		}
	}

	if (flags & O_TRUNC) {
		/* opening the db is destructive, need to lock first */
		int fd;

		db = NULL;
		lock_failed = 1;
		fd = open (filename, flags & ~O_TRUNC, mode);
		if (fd != -1) {
			if (!(lock_failed = flock (fd, lock_op)))
				db = dbopen (filename, flags, mode,
					     DB_BTREE, &b);
			close (fd);
		}
	} else {
		db = dbopen (filename, flags, mode, DB_BTREE, &b);
		if (db)
			lock_failed = flock ((db->fd) (db), lock_op);
	}

	if (!db)
		return NULL;

	if (lock_failed) {
		gripe_lock (filename);
		btree_close (db);
		return NULL;
	}

	return db;
}

/* do a replace when we have the duplicate flag set on the database -
   we must do a del and insert, as a direct insert will not wipe out the
   old entry */
int btree_replace (DB *db, datum key, datum cont)
{
#ifdef FAST_BTREE
	test_insert (__LINE__, key, cont);
	return (db->put) (db, (DBT *) &key, (DBT *) &cont, R_CURSOR);
#else /* normal BTREE */
	return (db->put) (db, (DBT *) &key, (DBT *) &cont, 0);
#endif /* FAST_BTREE */
}

int btree_insert (DB *db, datum key, datum cont)
{
	return (db->put) (db, (DBT *) &key, (DBT *) &cont, R_NOOVERWRITE);
}

/* generic fetch routine for the btree database */
datum btree_fetch (DB *db, datum key)
{
	datum data;

	memset (&data, 0, sizeof data);

	if ((db->get) (db, (DBT *) &key, (DBT *) &data, 0)) {
		memset (&data, 0, sizeof data);
		return data;
	}

	return copy_datum (data);
}

/* return 1 if the key exists, 0 otherwise */
int btree_exists (DB *db, datum key)
{
	datum data;
	return ((db->get) (db, (DBT *) &key, (DBT *) &data, 0) ? 0 : 1);
}

/* initiate a sequential access */
static inline datum btree_findkey (DB *db, u_int flags)
{
	datum key, data;

	memset (&key, 0, sizeof key);
	memset (&data, 0, sizeof data);

	if (flags == R_FIRST) {
		if (loop_check_hash) {
			hashtable_free (loop_check_hash);
			loop_check_hash = NULL;
		}
	}
	if (!loop_check_hash)
		loop_check_hash = hashtable_create (&plain_hashtable_free);

	if (((db->seq) (db, (DBT *) &key, (DBT *) &data, flags))) {
		memset (&key, 0, sizeof key);
		return key;
	}

	if (hashtable_lookup (loop_check_hash,
			      MYDBM_DPTR (key), MYDBM_DSIZE (key))) {
		/* We've seen this key already, which is broken. Return NULL
		 * so the caller doesn't go round in circles.
		 */
		debug ("Corrupt database! Already seen %*s. "
		       "Attempting to recover ...\n",
		       (int) MYDBM_DSIZE (key), MYDBM_DPTR (key));
		memset (&key, 0, sizeof key);
		return key;
	}

	hashtable_install (loop_check_hash,
			   MYDBM_DPTR (key), MYDBM_DSIZE (key), NULL);

	return copy_datum (key);
}

/* return the first key in the db */
datum btree_firstkey (DB *db)
{
	return btree_findkey (db, R_FIRST);
}

/* return the next key in the db. NB. This routine only works if the cursor
   has been previously set by btree_firstkey() since it was last opened. So
   if we close/reopen a db mid search, we have to manually set up the
   cursor again. */
datum btree_nextkey (DB *db)
{
	return btree_findkey (db, R_NEXT);
}

/* compound nextkey routine, initialising key and content */
int btree_nextkeydata (DB *db, datum *key, datum *cont)
{
	int status;

	if ((status = (db->seq) (db, (DBT *) key, (DBT *) cont, R_NEXT)) != 0)
		return status;

	*key = copy_datum (*key);
	*cont = copy_datum (*cont);

	return 0;
}

#ifdef FAST_BTREE

/* EXTREMELY experimental and broken code, leave well alone for the time
   being */

#define free(x)

void gripe_get (int lineno)
{
	error (0, 0, "Oops, bad fetch, line %d ", lineno);
}

/* the simplified storage routine */
int dbstore (struct mandata *in, char *base)
{
	datum key, cont;
	int status;

	memset (&key, 0, sizeof key);
	memset (&cont, 0, sizeof cont);

	MYDBM_SET (key, base);
 	if (!*base) {
		dbprintf (in);
 		return 2;
 	}

	/* initialise the cursor to (possibly) our key/cont */
	status = (dbf->seq) (dbf, (DBT *) &key, (DBT *) &cont, R_CURSOR);

	if (status == -1)
		gripe_get (__LINE__);

	/* either nothing was found or the key was not an exact match */
	else if (status == 1 || !STREQ (MYDBM_DPTR (key), base)) {
		cont = make_content (in);
		MYDBM_SET (key, base);
		test_insert (__LINE__, key, cont);
		status = (dbf->put) (dbf, (DBT *) &key, (DBT *) &cont, 0);
		free (MYDBM_DPTR (cont));

	/* There is already a key with this name */
	} else {
		/* find an exact match and see if it needs replacing or put
		   our new key in */
		while (1) {
			struct mandata old;

			/* TODO: what if cont is unset? */
			cont = copy_datum (cont);
			split_content (MYDBM_DPTR (cont), &old);
			if (STREQ (in->ext, old.ext)) {
				cont = make_content (in);
				status = replace_if_necessary (in, &old,
							       key, cont);
				free (MYDBM_DPTR (cont));
				free_mandata_elements (&old);
				break;
			}
			free_mandata_elements (&old);
			status = (dbf->seq) (dbf, (DBT *) &key, (DBT *) &cont,
					     R_NEXT);
			if (!STREQ (MYDBM_DPTR (key), base)) {
				MYDBM_SET (key, base);
				cont = make_content (in);
				test_insert (__LINE__, key, cont);
				status = (dbf->put) (dbf, (DBT *) &key,
						     (DBT *) &cont, 0);
				free (MYDBM_DPTR (cont));
				break;
			}
		}
	}

	return status;
}

/* FIXME: I'm broken as I don't return properly */
static struct mandata *dblookup (char *page, char *section, int flags)
{
	struct mandata *info, *ret = NULL, **null_me;
	datum key, cont;
	int status;

	memset (&key, 0, sizeof key);
	memset (&cont, 0, sizeof cont);

	MYDBM_SET (key, page);

	/* initialise the cursor to (possibly) our key/cont */
	status = (dbf->seq) (dbf, (DBT *) &key, (DBT *) &cont, R_CURSOR);

	/* either nothing was found or the key was not an exact match */
	if (status == 1 || !STREQ (page, MYDBM_DPTR (key)))
		return NULL;
	if (status == -1)
		gripe_get (__LINE__);

	ret = info = infoalloc ();
	null_me = &(info->next);

	do {
		/* TODO: what if cont is unset? */
		cont = copy_datum (cont);
		split_content (MYDBM_DPTR (cont), info);

		if (!(section == NULL ||
		    STRNEQ (section, info->ext,
		    	    flags & EXACT ? strlen (info->ext)
					  : strlen (section)))) {
			free_mandata_elements (info);
		} else {
			null_me = &(info->next);
			info = info->next = infoalloc ();
		}

		/* get next in the db */
		status = (dbf->seq) (dbf, (DBT *) &key, (DBT *) &cont, R_NEXT);

		if (status == -1)
			gripe_get (__LINE__);

		/* run out of identical keys */
	} while (!(status == 1 || !STREQ (page, MYDBM_DPTR (key))));

	free (info);
	*null_me = NULL;
	return ret;
}

#endif /* FAST_BTREE */
#endif /* BTREE */
