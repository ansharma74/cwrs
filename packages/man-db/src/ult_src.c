/*
 * ult_src.c: Find the ultimate source of a page
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011,
 *               2012 Colin Watson.
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
 * code to seek out the original (ultimate) source man file for
 * any specified man file. Soft and hard links and .so inclusions
 * are traced. Use: reduce amount of cat files to a minimum.
 *
 * Mon May  2 11:14:28 BST 1994 Wilf. (G.Wilford@ee.surrey.ac.uk)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

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

#include "canonicalize.h"

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "pipeline.h"
#include "decompress.h"

#include "ult_src.h"

/* Find minimum value hard link filename for given file and inode.
 * Returns a newly allocated string.
 */
static char *ult_hardlink (const char *fullpath, ino_t inode)
{
	DIR *mdir;
	struct dirent *manlist;
	char *base, *dir, *ret;
	const char *slash;

	slash = strrchr (fullpath, '/');
	assert (slash);
	dir = xstrndup (fullpath, slash - fullpath);
	base = xstrdup (++slash);

	mdir = opendir (dir);
	if (mdir == NULL) {
		if (quiet < 2)
			error (0, errno, _("can't search directory %s"), dir);
		free (dir);
		free (base);
		return NULL;
	}

	while ((manlist = readdir (mdir))) {
		if (manlist->d_ino == inode &&
		    strcmp (base, manlist->d_name) > 0) {
			free (base);
			base = xstrdup (manlist->d_name);
			debug ("ult_hardlink: (%s)\n", base);
		}
	}
	closedir (mdir);

	/* If we already are the link with the smallest name value */
	/* return NULL */

	if (strcmp (base, slash) == 0) {
		free (dir);
		free (base);
		return NULL;
	}

	ret = appendstr (NULL, dir, "/", base, NULL);
	free (dir);
	free (base);
	return ret;
}

/* Resolve all symbolic links within 'fullpath'.
 * Returns a newly allocated string.
 */
static char *ult_softlink (const char *fullpath)
{
	char *resolved_path;

	resolved_path = canonicalize_file_name (fullpath);
	if (!resolved_path) {
		/* discard the unresolved path */
		if (quiet < 2) {
			if (errno == ENOENT)
				error (0, 0,
				       _("warning: %s is a dangling symlink"),
				       fullpath);
			else
				error (0, errno, _("can't resolve %s"),
				       fullpath);
		}
		return NULL;
	}

	debug ("ult_softlink: (%s)\n", resolved_path);

	return resolved_path;
}

/* Test 'buffer' to see if it contains a .so include. If so and it's not an 
 * absolute filename, return newly allocated string whose contents are the
 * include.
 */
static char *test_for_include (const char *buffer)
{
	/* strip out any leading whitespace (if any) */
	while (CTYPE (isspace, *buffer))
		buffer++;

	/* see if the `command' is a .so */
	if (strncmp (buffer, ".so", 3) == 0) {
		buffer += 3;

		/* strip out any whitespace between the command and 
		   it's argumant */
		while (CTYPE (isspace, *buffer))
			buffer++;

		/* If .so's argument is an absolute filename, it could be
		 * either (i) a macro inclusion, (ii) a non local manual page
		 * or (iii) a (somewhat bogus) reference to a local manual 
		 * page.
		 * 
		 * If (i) or (ii), we must not follow the reference. (iii) is
		 * a problem with the manual page, thus we don't want to 
		 * follow any absolute inclusions in our quest for the 
		 * ultimate source file */
		if (*buffer != '/') {
			const char *end = buffer;
			while (*end && !CTYPE (isspace, *end))
				++end;
			return xstrndup (buffer, end - buffer);
		}
	}
	return NULL;
}

static void ult_trace (struct ult_trace *trace, const char *s)
{
	if (!trace)
		return;
	if (trace->len >= trace->max) {
		trace->max *= 2;
		trace->names = xrealloc (trace->names,
					 sizeof (char *) * trace->max);
	}
	trace->names[trace->len++] = xstrdup (s);
}

void free_ult_trace (struct ult_trace *trace)
{
	size_t i;
	for (i = 0; i < trace->len; ++i)
		free (trace->names[i]);
	free (trace->names);
}

/*
 * recursive function which finds the ultimate source file by following
 * any ".so filename" directives in the first line of the man pages.
 * Also (optionally) traces symlinks and hard links(!).
 *
 * name is full pathname, path is the MANPATH directory (/usr/man)
 * flags is a combination of SO_LINK | SOFT_LINK | HARD_LINK
 */
const char *ult_src (const char *name, const char *path,
		     struct stat *buf, int flags, struct ult_trace *trace)
{
	static char *base;		/* must be static */
	static short recurse; 		/* must be static */

	/* initialise the function */

	if (trace) {
		if (!trace->names) {
			trace->len = 0;
			trace->max = 16;
			trace->names = XNMALLOC (trace->max, char *);
		}
		ult_trace (trace, name);
	}

	/* as ult_softlink() & ult_hardlink() do all of their respective
	 * resolving in one call, only need to sort them out once
	 */

	if (recurse == 0) {
		struct stat new_buf;
		if (base)
			free (base);
		base = xstrdup (name);

		debug ("\nult_src: File %s in mantree %s\n", name, path);

		/* If we don't have a buf, allocate and assign one */
		if (!buf && ((flags & SOFT_LINK) || (flags & HARD_LINK))) {
			buf = &new_buf;
			if (lstat (base, buf) == -1) {
				if (quiet < 2)
					error (0, errno, _("can't resolve %s"),
					       base);
				return NULL;
			}
		}

		/* Permit semi local (inter-tree) soft links */
		if (flags & SOFT_LINK) {
			assert (buf); /* initialised above */
			if (S_ISLNK (buf->st_mode)) {
				/* Is a symlink, resolve it. */
				char *softlink = ult_softlink (base);
				if (softlink) {
					free (base);
					base = softlink;
				} else
					return NULL;
			}
		}

		/* Only deal with local (inter-dir) HARD links */
		if (flags & HARD_LINK) {
			assert (buf); /* initialised above */
			if (buf->st_nlink > 1) {
				/* Has HARD links, find least value */
				char *hardlink = ult_hardlink (base,
							       buf->st_ino);
				if (hardlink) {
					free (base);
					base = hardlink;
				}
			}
		}
	}

	/* keep a check on recursion level */
	else if (recurse == 10) {
		if (quiet < 2)
			error (0, 0, _("%s is self referencing"), name);
		return NULL;
	}

	if (flags & SO_LINK) {
		const char *buffer;
		char *decomp_base;
		pipeline *decomp;
#ifdef COMP_SRC
		struct stat st;

		if (stat (base, &st) < 0) {
			struct compression *comp = comp_file (base);

			if (comp) {
				if (base)
					free (base);
				base = comp->stem;
				comp->stem = NULL; /* steal memory */
			} else {
				if (quiet < 2)
					error (0, errno, _("can't open %s"),
					       base);
				return NULL;
			}
		}
#endif

		/* base may change for recursive calls to ult_src, but
		 * decompress_open doesn't keep its own copy.
		 */
		decomp_base = xstrdup (base);
		decomp = decompress_open (decomp_base);
		if (!decomp) {
			if (quiet < 2)
				error (0, errno, _("can't open %s"), base);
			free (decomp_base);
			return NULL;
		}
		pipeline_start (decomp);

		/* make sure that we skip over any comments */
		do {
			buffer = pipeline_readline (decomp);
		} while (buffer && STRNEQ (buffer, ".\\\"", 3));

		if (buffer) {
			char *include = test_for_include (buffer);
			if (include) {
				const char *ult;

				/* Restore the original path from before
				 * ult_softlink() etc., in case it went
				 * outside the mantree.
				 */
				free (base);
				base = appendstr (NULL, path, "/", include,
						  NULL);
				free (include);

				debug ("ult_src: points to %s\n", base);

				recurse++;
				ult = ult_src (base, path, NULL, flags, trace);
				recurse--;

				pipeline_wait (decomp);
				pipeline_free (decomp);
				free (decomp_base);
				return ult;
			}
		}

		pipeline_wait (decomp);
		pipeline_free (decomp);
		free (decomp_base);
	}

	/* We have the ultimate source */
	if (trace)
		ult_trace (trace, base);
	return base;
}
