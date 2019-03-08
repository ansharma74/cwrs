/*
 * check_mandirs.c: used to auto-update the database caches
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2004, 2007, 2008, 2009, 2010, 2011
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
 * Mon May  2 17:36:33 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk)
 *
 * CJW: Many changes to whatis parsing. Added database purging.
 * See ChangeLog for details.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#else /* not HAVE_DIRENT_H */
#  define dirent direct
#  ifdef HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif /* HAVE_SYS_NDIR_H */
#  ifdef HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif /* HAVE_SYS_DIR_H */
#  ifdef HAVE_NDIR_H
#    include <ndir.h>
#  endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H  */

#include <unistd.h>

#include "dirname.h"
#include "xvasprintf.h"

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "hashtable.h"
#include "security.h"

#include "mydbm.h"
#include "db_storage.h"

#include "descriptions.h"
#include "filenames.h"
#include "globbing.h"
#include "manp.h"
#include "ult_src.h"
#include "check_mandirs.h"

int opt_test;		/* don't update db */
int pages;
int force_rescan = 0;

static struct hashtable *whatis_hash = NULL;

struct whatis_hashent {
	char *whatis;
	struct ult_trace trace;
};

static void whatis_hashtable_free (void *defn)
{
	struct whatis_hashent *hashent = defn;

	free (hashent->whatis);
	free_ult_trace (&hashent->trace);
	free (hashent);
}

static void gripe_multi_extensions (const char *path, const char *sec, 
				    const char *name, const char *ext)
{
	if (quiet < 2)
		error (0, 0,
		       _("warning: %s/man%s/%s.%s*: competing extensions"),
		       path, sec, name, ext);
}

static void gripe_rwopen_failed (void)
{
	if (errno == EACCES || errno == EROFS)
		debug ("database %s is read-only\n", database);
	else {
#ifdef MAN_DB_UPDATES
		if (!quiet)
#endif /* MAN_DB_UPDATES */
			error (0, errno, _("can't update index cache %s"),
			       database);
	}
}

/* Take absolute filename and path (for ult_src) and do sanity checks on
 * file. Also check that file is non-zero in length and is not already in
 * the db. If not, find its ult_src() and see if we have the whatis cached,
 * otherwise cache it in case we trace another manpage back to it. Next,
 * store it in the db along with any references found in the whatis.
 */
void test_manfile (const char *file, const char *path)
{
	char *manpage_base;
	const char *ult;
	struct lexgrog lg;
	char *manpage;
	struct mandata info, *exists;
	struct stat buf;
	size_t len;
	struct ult_trace ult_trace;
	struct whatis_hashent *whatis;

	memset (&lg, 0, sizeof (struct lexgrog));
	memset (&info, 0, sizeof (struct mandata));
	memset (&ult_trace, 0, sizeof (struct ult_trace));

	manpage = filename_info (file, &info, NULL);
	if (!manpage)
		return;
	manpage_base = manpage + strlen (manpage) + 1;

	len  = strlen (manpage) + 1;		/* skip over directory name */
	len += strlen (manpage + len) + 1;	/* skip over base name */
	len += strlen (manpage + len);		/* skip over section ext */

	/* to get mtime info */
	(void) lstat (file, &buf);
	info._st_mtime = buf.st_mtime;

	/* check that our file actually contains some data */
	if (buf.st_size == 0) {
		/* man-db pre 2.3 place holder ? */
		free (manpage);
		return;
	}

	/* See if we already have it, before going any further. This will
	 * save both an ult_src() and a find_name(), amongst other wastes of
	 * time.
	 */
	exists = dblookup_exact (manpage_base, info.ext, 1);

	/* Ensure we really have the actual page. Gzip keeps the mtime the
	 * same when it compresses, so we have to compare compression
	 * extensions as well.
	 */
	if (exists) {
		if (strcmp (exists->comp, info.comp ? info.comp : "-") == 0) {
			if (exists->_st_mtime == info._st_mtime 
			    && exists->id < WHATIS_MAN) {
				free_mandata_struct (exists);
				free (manpage);
				return;
			}
		} else {
			struct stat physical;
			char *abs_filename;
			
			/* see if the cached file actually exists. It's 
			   evident at this point that we have multiple 
			   comp extensions */
			abs_filename = make_filename (path, NULL,
						      exists, "man");
			debug ("test_manfile(): stat %s\n", abs_filename);
			if (stat (abs_filename, &physical) == -1) {
				if (!opt_test)
					dbdelete (manpage_base, exists);
			} else {
				gripe_multi_extensions (path, exists->sec,
							manpage_base,
							exists->ext);
				free_mandata_struct (exists);
				free (manpage);
				return;
			}
		}
		free_mandata_struct (exists);
	}

	/* Check if it happens to be a symlink/hardlink to something already
	 * in our cache. This just does some extra checks to avoid scanning
	 * links quite so many times.
	 */
	{
		/* Avoid too much noise in debug output */
		int save_debug = debug_level;
		debug_level = 0;
		ult = ult_src (file, path, &buf, SOFT_LINK | HARD_LINK, NULL);
		debug_level = save_debug;
	}

	if (!ult) {
		/* already warned about this, don't do so again */
		debug ("test_manfile(): bad link %s\n", file);
		free (manpage);
		return;
	}

	if (!whatis_hash)
		whatis_hash = hashtable_create (&whatis_hashtable_free);

	whatis = hashtable_lookup (whatis_hash, ult, strlen (ult));
	if (!whatis) {
		if (!STRNEQ (ult, file, len))
			debug ("\ntest_manfile(): link not in cache:\n"
			       " source = %s\n"
			       " target = %s\n", file, ult);
		/* Trace the file to its ultimate source, otherwise we'll be
		 * looking for whatis info in files containing only '.so
		 * manx/foo.x', which will give us an unobtainable whatis
		 * for the entry. */
		ult = ult_src (file, path, &buf,
			       SO_LINK | SOFT_LINK | HARD_LINK, &ult_trace);
	}

	if (!ult) {
		if (quiet < 2)
			error (0, 0,
			       _("warning: %s: bad symlink or ROFF `.so' request"),
			       file);
		free (manpage);
		return;
	}

	pages++;			/* pages seen so far */

	if (strncmp (ult, file, len) == 0)
		info.id = ULT_MAN;	/* ultimate source file */
	else
		info.id = SO_MAN;	/* .so, sym or hard linked file */

	/* Ok, here goes: Use a hash tree to store the ult_srcs with
	 * their whatis. Anytime after, check the hash tree, if it's there, 
	 * use it. This saves us a find_name() which is a real hog.
	 *
	 * Use the full path in ult as the hash key so we don't have to
	 * clear the hash between calls.
	 */

	if (whatis)
		lg.whatis = whatis->whatis ? xstrdup (whatis->whatis) : NULL;
	else {
		/* Cache miss; go and get the whatis info in its raw state. */
		char *file_base = base_name (file);

		lg.type = MANPAGE;
		drop_effective_privs ();
		find_name (ult, file_base, &lg, NULL);
		free (file_base);
		regain_effective_privs ();

		whatis = XMALLOC (struct whatis_hashent);
		whatis->whatis = lg.whatis ? xstrdup (lg.whatis) : NULL;
		/* We filled out ult_trace above. */
		memcpy (&whatis->trace, &ult_trace, sizeof (ult_trace));
		hashtable_install (whatis_hash, ult, strlen (ult), whatis);
	}

	debug ("\"%s\"\n", lg.whatis);

	/* split up the raw whatis data and store references */
	info.pointer = NULL;	/* direct page, so far */
	info.filter = lg.filters;
	if (lg.whatis) {
		struct page_description *descs =
			parse_descriptions (manpage_base, lg.whatis);
		if (descs) {
			if (!opt_test)
				store_descriptions (descs, &info,
						    path, manpage_base,
						    &whatis->trace);
			free_descriptions (descs);
		}
	} else if (quiet < 2) {
		(void) stat (ult, &buf);
		if (buf.st_size == 0)
			error (0, 0, _("warning: %s: ignoring empty file"),
			       ult);
		else
			error (0, 0,
			       _("warning: %s: whatis parse for %s(%s) failed"),
			       ult, manpage_base, info.ext);
	}

	free (manpage);
	if (lg.whatis)
		free (lg.whatis);
}

static inline void add_dir_entries (const char *path, char *infile)
{
	char *manpage;
	int len;
	struct dirent *newdir;
	DIR *dir;

	manpage = appendstr (NULL, path, "/", infile, "/", NULL);
	len = strlen (manpage);

	/*
	 * All filename entries in this dir should either be valid manpages
	 * or . files (such as current, parent dir).
	 */

	dir = opendir (infile);
	if (!dir) {
		error (0, errno, _("can't search directory %s"), manpage);
		free (manpage);
                return;
        }
        
        /* strlen(newdir->d_name) could be replaced by newdir->d_reclen */
        
	while ( (newdir = readdir (dir)) )
		if (!(*newdir->d_name == '.' && 
		      strlen (newdir->d_name) < (size_t) 3)) {
			manpage = appendstr (manpage, newdir->d_name, NULL);
			test_manfile (manpage, path);
			*(manpage + len) = '\0';
		}
		
	free (manpage);
	closedir (dir);
}

#ifdef SECURE_MAN_UID
extern uid_t ruid;			/* initial real user id */
#endif /* SECURE_MAN_UID */

/* create the catman hierarchy if it doesn't exist */
static void mkcatdirs (const char *mandir, const char *catdir)
{
	char *manname, *catname;
#ifdef SECURE_MAN_UID
	struct passwd *man_owner = get_man_owner ();
#endif

	if (catdir) {
		int oldmask = umask (022);
		/* first the base catdir */
		if (is_directory (catdir) != 1) {
			regain_effective_privs ();
			if (mkdir (catdir, S_ISGID | 0755) < 0) {
				if (!quiet)
					error (0, 0,
					       _("warning: cannot create catdir %s"),
					       catdir);
				debug ("warning: cannot create catdir %s\n",
				       catdir);
			} else
				debug ("created base catdir %s\n", catdir);
#ifdef SECURE_MAN_UID
			if (ruid == 0)
				chown (catdir, man_owner->pw_uid, 0);
#endif /* SECURE_MAN_UID */
			drop_effective_privs ();
		}
		/* then the hierarchy */
		catname = appendstr (NULL, catdir, "/cat1", NULL);
		manname = appendstr (NULL, mandir, "/man1", NULL);
		if (is_directory (catdir) == 1) {
			int j;
			regain_effective_privs ();
			debug ("creating catdir hierarchy %s	", catdir);
			for (j = 1; j <= 9; j++) {
				catname[strlen (catname) - 1] = '0' + j;
				manname[strlen (manname) - 1] = '0' + j;
				if ((is_directory (manname) == 1)
				 && (is_directory (catname) != 1)) {
					if (mkdir (catname,
						   S_ISGID | 0755) < 0) {
						if (!quiet)
							error (0, 0, _("warning: cannot create catdir %s"), catname);
						debug ("warning: cannot create catdir %s\n", catname);
					} else
						debug (" cat%d", j);
#ifdef SECURE_MAN_UID
					if (ruid == 0)
						chown (catname,
						       man_owner->pw_uid, 0);
#endif /* SECURE_MAN_UID */
				}
			}
			debug ("\n");
			drop_effective_privs ();
		}
		free (catname);
		free (manname);
		umask (oldmask);
	}
}

/*
 * accepts the raw man dir tree eg. "/usr/man" and the time stored in the db
 * any dirs of the tree that have been modified (ie added to) will then be
 * scanned for new files, which are then added to the db.
 */
static int testmandirs (const char *path, const char *catpath, time_t last,
			int create)
{
	DIR *dir;
	struct dirent *mandir;
	struct stat stbuf;
	int amount = 0;
	int created = 0;

	debug ("Testing %s for new files\n", path);

	dir = opendir (path);
	if (!dir) {
		error (0, errno, _("can't search directory %s"), path);
		return 0;
	}

	chdir (path);

	while( (mandir = readdir (dir)) ) {
		if (strncmp (mandir->d_name, "man", 3) != 0)
			continue;

		debug ("Examining %s\n", mandir->d_name);

		if (stat (mandir->d_name, &stbuf) != 0)	/* stat failed */
			continue;
		if (!S_ISDIR(stbuf.st_mode))		/* not a directory */
			continue;
		if (last && stbuf.st_mtime <= last) {
			/* scanned already */
			debug ("%s modified %ld, db modified %ld\n",
			       mandir->d_name, (long) stbuf.st_mtime,
			       (long) last);
			continue;
		}

		debug ("\tsubdirectory %s has been 'modified'\n",
		       mandir->d_name);

		if (create && !created) {
			/* We seem to have something to do, so create the
			 * database now.
			 */
			mkcatdirs (path, catpath);

			/* Open the db in CTRW mode to store the $ver$ ID */

			dbf = MYDBM_CTRWOPEN (database);
			if (dbf == NULL) {
				if (errno == EACCES || errno == EROFS) {
					debug ("database %s is read-only\n",
					       database);
					return 0;
				} else {
					error (0, errno,
					       _("can't create index cache %s"),
					       database);
					return -errno;
				}
			}

			dbver_wr (dbf);

			created = 1;
		} else
			dbf = MYDBM_RWOPEN(database);

		if (!dbf) {
			gripe_rwopen_failed ();
			return 0;
		}

		if (!quiet) {
			int tty = isatty (STDERR_FILENO);

			if (tty)
				fprintf (stderr, "\r");
			fprintf (stderr,
				 _("Updating index cache for path "
				   "`%s/%s'. Wait..."), path, mandir->d_name);
			if (!tty)
				fprintf (stderr, "\n");
		}
		add_dir_entries (path, mandir->d_name);
		MYDBM_CLOSE (dbf);
		amount++;
	}
	closedir (dir);

	return amount;
}

/* update the time key stored within `database' */
void update_db_time (void)
{
	datum key, content;
#ifdef FAST_BTREE
	datum key1, content1;
#endif /* FAST_BTREE */

	memset (&key, 0, sizeof key);
	memset (&content, 0, sizeof content);
#ifdef FAST_BTREE
	memset (&key1, 0, sizeof key);
	memset (&content1, 0, sizeof content);
#endif

	MYDBM_SET (key, xstrdup (KEY));
	MYDBM_SET (content, xasprintf ("%ld", (long) time (NULL)));

	/* Open the db in RW to store the $mtime$ ID */
	/* we know that this should succeed because we just updated the db! */
	dbf = MYDBM_RWOPEN (database);
	if (dbf == NULL) {
#ifdef MAN_DB_UPDATES
		if (!quiet)
#endif /* MAN_DB_UPDATES */
			error (0, errno, _("can't update index cache %s"),
			       database);
		free (MYDBM_DPTR (content));
		return;
	}
#ifndef FAST_BTREE
	MYDBM_REPLACE (dbf, key, content);
#else /* FAST_BTREE */
	MYDBM_SET (key1, KEY);

	(dbf->seq) (dbf, (DBT *) &key1, (DBT *) &content1, R_CURSOR);
	
	if (strcmp (MYDBM_DPTR (key1), MYDBM_DPTR (key)) == 0)
		(dbf->put) (dbf, (DBT *) &key, (DBT *) &content, R_CURSOR);
	else
		(dbf->put) (dbf, (DBT *) &key, (DBT *) &content, 0);
#endif /* !FAST_BTREE */

	MYDBM_CLOSE (dbf);
	free (MYDBM_DPTR (key));
	free (MYDBM_DPTR (content));
}

/* remove the db's time key - called prior to update_db if we want
   to `force' a full consistency check */
void reset_db_time (void)
{
	datum key;

	memset (&key, 0, sizeof key);

	MYDBM_SET (key, xstrdup (KEY));

	/* we don't really care if we can't open it RW - it's not fatal */
	dbf = MYDBM_RWOPEN (database);
	if (dbf == NULL) {
		debug_error ("reset_db_time(): can't open db");
		return;
	}

	MYDBM_DELETE (dbf, key);
	debug ("reset_db_time()\n");
	MYDBM_CLOSE (dbf);
	free (MYDBM_DPTR (key));
}

/* routine to prepare/create the db prior to calling testmandirs() */
int create_db (const char *manpath, const char *catpath)
{
	int amount;
	
	debug ("create_db(%s): %s\n", manpath, database);

	amount = testmandirs (manpath, catpath, (time_t) 0, 1);

	if (amount) {
		update_db_time ();
		if (!quiet)
			fputs (_("done.\n"), stderr);
	}

	return amount;
}

/* Make sure an existing database is essentially sane. */
int sanity_check_db (void)
{
	datum key;

	if (dbver_rd (dbf))
		return 0;

	key = MYDBM_FIRSTKEY (dbf);
	while (MYDBM_DPTR (key) != NULL) {
		datum content, nextkey;

		content = MYDBM_FETCH (dbf, key);
		if (!MYDBM_DPTR (content)) {
			debug ("warning: %s has a key with no content (%s); "
			       "rebuilding\n", database, MYDBM_DPTR (key));
			MYDBM_FREE (MYDBM_DPTR (key));
			return 0;
		}
		MYDBM_FREE (MYDBM_DPTR (content));
		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey;
	}

	return 1;
}

/* routine to update the db, ensure that it is consistent with the 
   filesystem */
int update_db (const char *manpath, const char *catpath)
{
	dbf = MYDBM_RDOPEN (database);
	if (dbf && !sanity_check_db ()) {
		MYDBM_CLOSE (dbf);
		dbf = NULL;
	}
	if (dbf) {
		datum key, content;
		int new;

		memset (&key, 0, sizeof key);
		memset (&content, 0, sizeof content);

		MYDBM_SET (key, xstrdup (KEY));
		content = MYDBM_FETCH (dbf, key);
		MYDBM_CLOSE (dbf);
		free (MYDBM_DPTR (key));

		debug ("update_db(): %ld\n",
		       MYDBM_DPTR (content) ? atol (MYDBM_DPTR (content)) : 0L);
		if (MYDBM_DPTR (content)) {
			new = testmandirs (
				manpath, catpath,
				(time_t) atol (MYDBM_DPTR (content)), 0);
			MYDBM_FREE (MYDBM_DPTR (content));
		} else
			new = testmandirs (manpath, catpath, (time_t) 0, 0);

		if (new) {
			update_db_time ();
			if (!quiet)
				fputs (_("done.\n"), stderr);
		}
		
		return new;
	}
		
	debug ("failed to open %s O_RDONLY\n", database);
		
	return EOF;
}

/* Purge any entries pointing to name. This currently assumes that pointers
 * are always shallow, which may not be a good assumption yet; it should be
 * close, though.
 *
 * Assumes that the appropriate database is already open on dbf.
 */
void purge_pointers (const char *name)
{
	datum key = MYDBM_FIRSTKEY (dbf);

	debug ("Purging pointers to vanished page \"%s\"\n", name);

	while (MYDBM_DPTR (key) != NULL) {
		datum content, nextkey;
		struct mandata entry;
		char *nicekey, *tab;

		/* Ignore db identifier keys. */
		if (*MYDBM_DPTR (key) == '$')
			goto pointers_next;

		content = MYDBM_FETCH (dbf, key);
		if (!MYDBM_DPTR (content))
			return;

		/* Get just the name. */
		nicekey = xstrdup (MYDBM_DPTR (key));
		tab = strchr (nicekey, '\t');
		if (tab)
			*tab = '\0';

		if (*MYDBM_DPTR (content) == '\t')
			goto pointers_contentnext;

		split_content (MYDBM_DPTR (content), &entry);
		if (entry.id != SO_MAN && entry.id != WHATIS_MAN)
			goto pointers_contentnext;

		if (STREQ (entry.pointer, name)) {
			if (!opt_test)
				dbdelete (nicekey, &entry);
			else
				debug ("%s(%s): pointer vanished, "
				       "would delete\n", nicekey, entry.ext);
		}

pointers_contentnext:
		free (nicekey);
		MYDBM_FREE (MYDBM_DPTR (content));
pointers_next:
		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey;
	}
}

/* Count the number of exact extension matches returned from look_for_file()
 * (which may return inexact extension matches in some cases). It may turn
 * out that this is better handled in look_for_file() itself.
 */
static int count_glob_matches (const char *name, const char *ext,
			       char **source, long db_mtime)
{
	char **walk;
	int count = 0;

	for (walk = source; walk && *walk; ++walk) {
		struct mandata info;
		struct stat statbuf;
		char *buf;

		memset (&info, 0, sizeof (struct mandata));

		if (stat (*walk, &statbuf) == -1) {
			debug ("count_glob_matches: excluding %s "
			       "because stat failed\n", *walk);
			continue;
		}
		if (db_mtime != -1 && statbuf.st_mtime <= db_mtime) {
			debug ("count_glob_matches: excluding %s, "
			       "no newer than database\n", *walk);
			continue;
		}

		buf = filename_info (*walk, &info, name);
		if (buf) {
			if (STREQ (ext, info.ext))
				++count;
			if (info.name)
				free (info.name);
			free (buf);
		}
	}

	return count;
}

/* Decide whether to purge a reference to a "normal" (ULT_MAN or SO_MAN)
 * page.
 */
static int purge_normal (const char *name, struct mandata *info,
			 char **found)
{
	/* TODO: On some systems, the cat page extension differs from the
	 * man page extension, so this may be too strict.
	 */
	if (count_glob_matches (name, info->ext, found, -1))
		return 0;

	if (!opt_test)
		dbdelete (name, info);
	else
		debug ("%s(%s): missing page, would delete\n",
		       name, info->ext);

	return 1;
}

/* Decide whether to purge a reference to a WHATIS_MAN or WHATIS_CAT page. */
static int purge_whatis (const char *path, int cat, const char *name,
			 struct mandata *info, char **found, long db_mtime)
{
	/* TODO: On some systems, the cat page extension differs from the
	 * man page extension, so this may be too strict.
	 */
	if (count_glob_matches (name, info->ext, found, db_mtime)) {
		/* If the page exists and didn't beforehand, then presumably
		 * we're about to rescan, which will replace the WHATIS_MAN
		 * entry with something better. However, there have been
		 * bugs that created false WHATIS_MAN entries, so force the
		 * rescan just to be sure; since in the absence of a bug we
		 * would rescan anyway, this isn't a problem.
		 */
		if (!force_rescan)
			debug ("%s(%s): whatis replaced by real page; "
			       "forcing a rescan just in case\n",
			       name, info->ext);
		force_rescan = 1;
		return 0;
	} else if (STREQ (info->pointer, "-")) {
		/* This is broken; a WHATIS_MAN should never have an empty
		 * pointer field. This might have happened due to the first
		 * name in a page being different from what the file name
		 * says; that's fixed now, so delete and force a rescan.
		 */
		if (!opt_test)
			dbdelete (name, info);
		else
			debug ("%s(%s): whatis with empty pointer, "
			       "would delete\n", name, info->ext);

		if (!force_rescan)
			debug ("%s(%s): whatis had empty pointer; "
			       "forcing a rescan just in case\n",
			       name, info->ext);
		force_rescan = 1;
		return 1;
	} else {
		/* Does the real page still exist? */
		char **real_found;
		int save_debug = debug_level;
		debug_level = 0;
		real_found = look_for_file (path, info->ext,
					    info->pointer, cat, LFF_MATCHCASE);
		debug_level = save_debug;

		if (count_glob_matches (info->pointer, info->ext, real_found,
					-1))
			return 0;

		if (!opt_test)
			dbdelete (name, info);
		else
			debug ("%s(%s): whatis target was deleted, "
			       "would delete\n", name, info->ext);
		return 1;
	}
}

/* Check that multi keys are correctly constructed. */
static int check_multi_key (const char *name, const char *content)
{
	const char *walk, *next;

	if (!*content)
		return 0;

	for (walk = content; walk && *walk; walk = next) {
		/* The name in the multi key should only differ from the
		 * name of the key itself in its case, if at all.
		 */
		int valid = 1;
		++walk; /* skip over initial tab */
		next = strchr (walk, '\t');
		if (next) {
			if (strncasecmp (name, walk, next - walk))
				valid = 0;
		} else {
			if (strcasecmp (name, walk))
				valid = 0;
		}
		if (!valid) {
			debug ("%s: broken multi key \"%s\", "
			       "forcing a rescan\n", name, content);
			force_rescan = 1;
			return 1;
		}

		/* If the name was valid, skip over the extension and
		 * continue the scan.
		 */
		walk = next;
		next = walk ? strchr (walk + 1, '\t') : NULL;
	}

	return 0;
}

/* Go through the database and purge references to man pages that no longer
 * exist.
 */
int purge_missing (const char *manpath, const char *catpath)
{
	struct stat st;
	datum key;
	int count = 0;
	long db_mtime = -1;

	if (stat (database, &st) != 0)
		/* nothing to purge */
		return 0;

	if (!quiet)
		printf (_("Purging old database entries in %s...\n"), manpath);

	dbf = MYDBM_RWOPEN (database);
	if (!dbf) {
		gripe_rwopen_failed ();
		return 0;
	}

	/* Extract the database mtime. */
	key = MYDBM_FIRSTKEY (dbf);
	while (MYDBM_DPTR (key) != NULL) {
		datum content, nextkey;

		if (STREQ (MYDBM_DPTR (key), KEY)) {
			content = MYDBM_FETCH (dbf, key);
			if (MYDBM_DPTR (content)) {
				errno = 0;
				db_mtime = strtol (MYDBM_DPTR (content), NULL,
						   10);
				if (errno)
					db_mtime = -1;
				MYDBM_FREE (MYDBM_DPTR (key));
				break;
			}
		}

		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey;
	}

	key = MYDBM_FIRSTKEY (dbf);

	while (MYDBM_DPTR (key) != NULL) {
		datum content, nextkey;
		struct mandata entry;
		char *nicekey, *tab;
		int save_debug;
		char **found;

		/* Ignore db identifier keys. */
		if (*MYDBM_DPTR (key) == '$') {
			nextkey = MYDBM_NEXTKEY (dbf, key);
			MYDBM_FREE (MYDBM_DPTR (key));
			key = nextkey;
			continue;
		}

		content = MYDBM_FETCH (dbf, key);
		if (!MYDBM_DPTR (content)) {
			nextkey = MYDBM_NEXTKEY (dbf, key);
			MYDBM_FREE (MYDBM_DPTR (key));
			key = nextkey;
			continue;
		}

		/* Get just the name. */
		nicekey = xstrdup (MYDBM_DPTR (key));
		tab = strchr (nicekey, '\t');
		if (tab)
			*tab = '\0';

		/* Deal with multi keys. */
		if (*MYDBM_DPTR (content) == '\t') {
			if (check_multi_key (nicekey, MYDBM_DPTR (content)))
				MYDBM_DELETE (dbf, key);
			free (nicekey);
			MYDBM_FREE (MYDBM_DPTR (content));
			nextkey = MYDBM_NEXTKEY (dbf, key);
			MYDBM_FREE (MYDBM_DPTR (key));
			key = nextkey;
			continue;
		}

		split_content (MYDBM_DPTR (content), &entry);

		save_debug = debug_level;
		debug_level = 0;	/* look_for_file() is quite noisy */
		if (entry.id <= WHATIS_MAN)
			found = look_for_file (manpath, entry.ext,
					       entry.name ? entry.name
							  : nicekey,
					       0, LFF_MATCHCASE);
		else
			found = look_for_file (catpath, entry.ext,
					       entry.name ? entry.name
							  : nicekey,
					       1, LFF_MATCHCASE);
		debug_level = save_debug;

		/* Now actually decide whether to purge, depending on the
		 * type of entry.
		 */
		if (entry.id == ULT_MAN || entry.id == SO_MAN ||
		    entry.id == STRAY_CAT)
			count += purge_normal (nicekey, &entry, found);
		else if (entry.id == WHATIS_MAN)
			count += purge_whatis (manpath, 0, nicekey,
					       &entry, found, db_mtime);
		else	/* entry.id == WHATIS_CAT */
			count += purge_whatis (catpath, 1, nicekey,
					       &entry, found, db_mtime);

		free (nicekey);

		free_mandata_elements (&entry);
		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey;
	}

	MYDBM_REORG (dbf);
	MYDBM_CLOSE (dbf);
	return count;
}
