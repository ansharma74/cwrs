/*
 * globbing.c: interface to the POSIX glob routines
 *  
 * Copyright (C) 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008 Colin Watson.
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
 * Mon Mar 13 20:27:36 GMT 1995  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>

#include "fnmatch.h"
#include "regex.h"

#include "manconfig.h"

#include "error.h"
#include "hashtable.h"
#include "cleanup.h"
#include "xregcomp.h"

#include "globbing.h"

const char *extension;
static const char *mandir_layout = MANDIR_LAYOUT;

static char *make_pattern (const char *name, const char *sec, int opts)
{
	char *pattern = xstrdup (name);

	if (opts & LFF_REGEX) {
		if (extension) {
			char *esc_ext = escape_shell (extension);
			pattern = appendstr (pattern, "\\..*", esc_ext, ".*",
					     NULL);
			free (esc_ext);
		} else {
			char *esc_sec = escape_shell (sec);
			pattern = appendstr (pattern, "\\.", esc_sec, ".*",
					     NULL);
			free (esc_sec);
		}
	} else {
		if (extension)
			pattern = appendstr (pattern, ".*", extension, "*",
					     NULL);
		else
			pattern = appendstr (pattern, ".", sec, "*", NULL);
	}

	return pattern;
}

#define LAYOUT_GNU	1
#define LAYOUT_HPUX	2
#define LAYOUT_IRIX	4
#define LAYOUT_SOLARIS	8
#define LAYOUT_BSD	16

static int parse_layout (const char *layout)
{
	if (!*layout)
		return LAYOUT_GNU | LAYOUT_HPUX | LAYOUT_IRIX |
		       LAYOUT_SOLARIS | LAYOUT_BSD;
	else {
		int flags = 0;

		char *upper_layout = xstrdup (layout);
		char *layoutp;
		for (layoutp = upper_layout; *layoutp; layoutp++)
			*layoutp = CTYPE (toupper, *layoutp);

		if (strstr (upper_layout, "GNU"))
			flags |= LAYOUT_GNU;
		if (strstr (upper_layout, "HPUX"))
			flags |= LAYOUT_HPUX;
		if (strstr (upper_layout, "IRIX"))
			flags |= LAYOUT_IRIX;
		if (strstr (upper_layout, "SOLARIS"))
			flags |= LAYOUT_SOLARIS;
		if (strstr (upper_layout, "BSD"))
			flags |= LAYOUT_BSD;

		free (upper_layout);
		return flags;
	}
}

struct dirent_hashent {
	char **names;
	size_t names_len, names_max;
};

static void dirent_hashtable_free (void *defn)
{
	struct dirent_hashent *hashent = defn;
	size_t i;

	for (i = 0; i < hashent->names_len; ++i)
		free (hashent->names[i]);
	free (hashent->names);
	free (hashent);
}

static struct hashtable *dirent_hash = NULL;

static int cache_compare (const void *a, const void *b)
{
	const char *left = *(const char **) a;
	const char *right = *(const char **) b;
	return strcasecmp (left, right);
}

static struct dirent_hashent *update_directory_cache (const char *path)
{
	struct dirent_hashent *cache;
	DIR *dir;
	struct dirent *entry;

	if (!dirent_hash) {
		dirent_hash = hashtable_create (&dirent_hashtable_free);
		push_cleanup ((cleanup_fun) hashtable_free, dirent_hash, 0);
	}
	cache = hashtable_lookup (dirent_hash, path, strlen (path));

	/* Check whether we've got this one already. */
	if (cache) {
		debug ("update_directory_cache %s: hit\n", path);
		return cache;
	}

	debug ("update_directory_cache %s: miss\n", path);

	dir = opendir (path);
	if (!dir) {
		debug_error ("can't open directory %s", path);
		return NULL;
	}

	cache = XMALLOC (struct dirent_hashent);
	cache->names_len = 0;
	cache->names_max = 1024;
	cache->names = XNMALLOC (cache->names_max, char *);

	/* Dump all the entries into cache->names, resizing if necessary. */
	for (entry = readdir (dir); entry; entry = readdir (dir)) {
		if (cache->names_len >= cache->names_max) {
			cache->names_max *= 2;
			cache->names =
				xrealloc (cache->names,
					  sizeof (char *) * cache->names_max);
		}
		cache->names[cache->names_len++] = xstrdup (entry->d_name);
	}

	qsort (cache->names, cache->names_len, sizeof *cache->names,
	       &cache_compare);

	hashtable_install (dirent_hash, path, strlen (path), cache);
	closedir (dir);

	return cache;
}

struct pattern_bsearch {
	char *pattern;
	size_t len;
};

static int pattern_compare (const void *a, const void *b)
{
	const struct pattern_bsearch *key = a;
	const char *memb = *(const char **) b;
	return strncasecmp (key->pattern, memb, key->len);
}

static void clear_glob (glob_t *pglob)
{
	/* look_for_file declares this static, so it's zero-initialised.
	 * globfree() can deal with checking it before freeing.
	 */
	globfree (pglob);

	pglob->gl_pathc = 0;
	pglob->gl_pathv = NULL;
	pglob->gl_offs = 0;
}

static void match_in_directory (const char *path, const char *pattern, int opts,
				glob_t *pglob, size_t *allocated)
{
	struct dirent_hashent *cache;
	size_t my_allocated = 0;
	int flags;
	regex_t preg;
	struct pattern_bsearch pattern_start;
	char **bsearched;
	size_t i;

	if (!allocated)
		allocated = &my_allocated;
	if (!*allocated)
		clear_glob (pglob);

	cache = update_directory_cache (path);
	if (!cache) {
		debug ("directory cache update failed\n");
		return;
	}

	debug ("globbing pattern in %s: %s\n", path, pattern);

	if (!*allocated) {
		*allocated = 4;
		pglob->gl_pathv = XNMALLOC (*allocated, char *);
		pglob->gl_pathv[0] = NULL;
	}

	if (opts & LFF_REGEX)
		flags = REG_EXTENDED | REG_NOSUB |
			((opts & LFF_MATCHCASE) ? 0 : REG_ICASE);
	else
		flags = (opts & LFF_MATCHCASE) ? 0 : FNM_CASEFOLD;

	if (opts & LFF_REGEX) {
		xregcomp (&preg, pattern, flags);
		bsearched = cache->names;
	} else {
		pattern_start.pattern = xstrndup (pattern,
						  strcspn (pattern, "?*{}\\"));
		pattern_start.len = strlen (pattern_start.pattern);
		bsearched = bsearch (&pattern_start, cache->names,
				     cache->names_len, sizeof *cache->names,
				     &pattern_compare);
		if (!bsearched) {
			free (pattern_start.pattern);
			return;
		}
		while (bsearched > cache->names &&
		       !strncasecmp (pattern_start.pattern, *(bsearched - 1),
				     pattern_start.len))
			--bsearched;
	}

	for (i = bsearched - cache->names; i < cache->names_len; ++i) {
		if (opts & LFF_REGEX) {
			if (regexec (&preg, cache->names[i], 0, NULL, 0) != 0)
				continue;
		} else {
			if (strncasecmp (pattern_start.pattern,
					 cache->names[i], pattern_start.len))
				break;

			if (fnmatch (pattern, cache->names[i], flags) != 0)
				continue;
		}

		debug ("matched: %s/%s\n", path, cache->names[i]);

		if (pglob->gl_pathc >= *allocated) {
			*allocated *= 2;
			pglob->gl_pathv = xnrealloc (
				pglob->gl_pathv, *allocated, sizeof (char *));
		}
		pglob->gl_pathv[pglob->gl_pathc++] =
			appendstr (NULL, path, "/", cache->names[i], NULL);
	}

	if (opts & LFF_REGEX)
		regfree (&preg);
	else
		free (pattern_start.pattern);

	if (pglob->gl_pathc >= *allocated) {
		*allocated *= 2;
		pglob->gl_pathv = xnrealloc (pglob->gl_pathv,
					     *allocated, sizeof (char *));
	}
	pglob->gl_pathv[pglob->gl_pathc] = NULL;

	return;
}

char **look_for_file (const char *hier, const char *sec,
		      const char *unesc_name, int cat, int opts)
{
	char *pattern, *path = NULL;
	static glob_t gbuf;
	static int cleanup_installed = 0;
	static int layout = -1;
	char *name;

	if (!cleanup_installed) {
		/* appease valgrind */
		push_cleanup ((cleanup_fun) globfree, &gbuf, 0);
		cleanup_installed = 1;
	}

	clear_glob (&gbuf);

	/* This routine only does a minimum amount of matching. It does not
	   find cat files in the alternate cat directory. */

	if (layout == -1) {
		layout = parse_layout (mandir_layout);
		debug ("Layout is %s (%d)\n", mandir_layout, layout);
	}

	if (opts & (LFF_REGEX | LFF_WILDCARD))
		name = xstrdup (unesc_name);
	else
		name = escape_shell (unesc_name);

	/* allow lookups like "3x foo" to match "../man3/foo.3x" */

	if (layout & LAYOUT_GNU) {
		glob_t dirs;
		size_t i;
		size_t allocated = 0;

		memset (&dirs, 0, sizeof (dirs));
		pattern = appendstr (NULL, cat ? "cat" : "man", "\t*", NULL);
		*strrchr (pattern, '\t') = *sec;
		match_in_directory (hier, pattern, LFF_MATCHCASE, &dirs, NULL);
		free (pattern);

		pattern = make_pattern (name, sec, opts);
		for (i = 0; i < dirs.gl_pathc; ++i) {
			if (path)
				*path = '\0';
			match_in_directory (dirs.gl_pathv[i], pattern, opts,
					    &gbuf, &allocated);
		}
		free (pattern);
		globfree (&dirs);
	}

	/* Try HPUX style compressed man pages */
	if ((layout & LAYOUT_HPUX) && gbuf.gl_pathc == 0) {
		if (path)
			*path = '\0';
		path = appendstr (path, hier, cat ? "/cat" : "/man",
				  sec, ".Z", NULL);
		pattern = make_pattern (name, sec, opts);

		match_in_directory (path, pattern, opts, &gbuf, NULL);
		free (pattern);
	}

	/* Try man pages without the section extension --- IRIX man pages */
	if ((layout & LAYOUT_IRIX) && gbuf.gl_pathc == 0) {
		if (path)
			*path = '\0';
		path = appendstr (path, hier, cat ? "/cat" : "/man", sec,
				  NULL);
		if (opts & LFF_REGEX)
			pattern = appendstr (NULL, name, "\\..*", NULL);
		else
			pattern = appendstr (NULL, name, ".*", NULL);

		match_in_directory (path, pattern, opts, &gbuf, NULL);
		free (pattern);
	}

	/* Try Solaris style man page directories */
	if ((layout & LAYOUT_SOLARIS) && gbuf.gl_pathc == 0) {
		if (path)
			*path = '\0';
		/* TODO: This needs to be man/sec*, not just man/sec. */
		path = appendstr (path, hier, cat ? "/cat" : "/man", sec,
				  NULL);
		pattern = make_pattern (name, sec, opts);

		match_in_directory (path, pattern, opts, &gbuf, NULL);
		free (pattern);
	}

	/* BSD cat pages take the extension .0 */
	if ((layout & LAYOUT_BSD) && gbuf.gl_pathc == 0) {
		if (path)
			*path = '\0';
		if (cat) {
			path = appendstr (path, hier, "/cat", sec, NULL);
			if (opts & LFF_REGEX)
				pattern = appendstr (NULL, name, "\\.0.*",
						     NULL);
			else
				pattern = appendstr (NULL, name, ".0*", NULL);
		} else {
			path = appendstr (path, hier, "/man", sec, NULL);
			pattern = make_pattern (name, sec, opts);
		}
		match_in_directory (path, pattern, opts, &gbuf, NULL);
		free (pattern);
	}

	free (name);
	free (path);

	if (gbuf.gl_pathc == 0)
		return NULL;
	else
		return gbuf.gl_pathv;
}
