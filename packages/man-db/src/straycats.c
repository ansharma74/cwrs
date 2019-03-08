/*
 * straycats.c: find and process stray cat files
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2010, 2011
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
 * Tue May  3 21:24:51 BST 1994 Wilf. (G.Wilford@ee.surrey.ac.uk)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

#include "canonicalize.h"
#include "dirname.h"

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "pipeline.h"
#include "decompress.h"
#include "encodings.h"
#include "security.h"

#include "mydbm.h"
#include "db_storage.h"

#include "descriptions.h"
#include "manp.h"
#include "manconv_client.h"
#include "ult_src.h"

static char *catdir, *mandir;

static int check_for_stray (void)
{
	DIR *cdir;
	struct dirent *catlist;
	size_t lenman, lencat;
	int strays = 0;

	cdir = opendir (catdir);
	if (!cdir) {
		error (0, errno, _("can't search directory %s"), catdir);
		return 0;
	}

	mandir = appendstr (mandir, "/", NULL);
	catdir = appendstr (catdir, "/", NULL);
	lenman = strlen (mandir);
	lencat = strlen (catdir);

	while ((catlist = readdir (cdir))) {
		struct mandata info;
		char *ext, *section;
		short found;
		struct stat buf;
#ifdef COMP_SRC
		struct compression *comp;
#endif

		memset (&info, 0, sizeof (struct mandata));

		if (*catlist->d_name == '.' && 
		    strlen (catlist->d_name) < (size_t) 3)
			continue;

		*(mandir + lenman) = *(catdir + lencat) = '\0';
		mandir = appendstr (mandir, catlist->d_name, NULL);
		catdir = appendstr (catdir, catlist->d_name, NULL);

		ext = strrchr (mandir, '.');
		if (!ext) {
			if (quiet < 2)
				error (0, 0,
				       _("warning: %s: "
					 "ignoring bogus filename"),
				       catdir);
			continue;

#if defined(COMP_SRC) || defined(COMP_CAT)

#  if defined(COMP_SRC)
		} else if (comp_info (ext, 0)) {
#  elif defined(COMP_CAT)
		} else if (strcmp (ext + 1, COMPRESS_EXT) == 0) {
#  endif /* COMP_* */
			*ext = '\0';
			info.comp = ext + 1;
#endif /* COMP_SRC || COMP_CAT */

		} else
			info.comp = NULL;

		ext = strrchr (mandir, '.');
		*(mandir + lenman - 1) = '\0';
		section = xstrdup (strrchr (mandir, '/') + 4);
		*(mandir + lenman - 1) = '/';

		/* check for bogosity */
		
		if (!ext || strncmp (ext + 1, section, strlen (section)) != 0) {
			if (quiet < 2)
				error (0, 0,
				       _("warning: %s: "
					 "ignoring bogus filename"),
				       catdir);
			goto next_section;
		}

		/*
		 * now that we've stripped off the cat compression
		 * extension (if it has one), we can try some of ours.
		 */

		debug ("Testing for existence: %s\n", mandir);

		if (stat (mandir, &buf) == 0) 
			found = 1;
#ifdef COMP_SRC 
		else if ((comp = comp_file (mandir))) {
			found = 1;
			free (comp->stem);
		}
#endif
		else 
			found = 0;

		if (!found) {
			pipeline *decomp;
			struct mandata *exists;
			lexgrog lg;
			char *lang, *page_encoding;
			char *mandir_base;
			pipecmd *col_cmd;
			char *col_locale;
			char *fullpath;

			/* we have a straycat. Need to filter it and get
			   its whatis (if necessary)  */

			lg.whatis = 0;
			*(ext++) = '\0';
			info.ext = ext;

			/* see if we already have it, before going any 
			   further */
			mandir_base = base_name (mandir);
			exists = dblookup_exact (mandir_base, info.ext, 1);
#ifndef FAVOUR_STRAYCATS
			if (exists && exists->id != WHATIS_CAT)
#else /* FAVOUR_STRAYCATS */
			if (exists && exists->id != WHATIS_CAT &&
			    exists->id != WHATIS_MAN)
#endif /* !FAVOUR_STRAYCATS */
				goto next_exists;
			debug ("%s(%s) is not in the db.\n",
			       mandir_base, info.ext);

			/* fill in the missing parts of the structure */
			info.name = NULL;
			info.sec = section;
			info.id = STRAY_CAT;
			info.pointer = NULL;
			info.filter = "-";
			info._st_mtime = 0L;

			drop_effective_privs ();
			decomp = decompress_open (catdir);
			regain_effective_privs ();
			if (!decomp) {
				error (0, errno, _("can't open %s"), catdir);
				goto next_exists;
			}

			lang = lang_dir (mandir);
			page_encoding = get_page_encoding (lang);
			if (page_encoding)
				add_manconv (decomp, page_encoding, "UTF-8");
			free (page_encoding);
			free (lang);

			col_cmd = pipecmd_new_argstr
				(get_def_user ("col", COL));
			pipecmd_arg (col_cmd, "-bx");
			col_locale = find_charset_locale ("UTF-8");
			if (col_locale) {
				pipecmd_setenv (col_cmd, "LC_CTYPE",
						col_locale);
				free (col_locale);
			}
			pipeline_command (decomp, col_cmd);

			fullpath = canonicalize_file_name (catdir);
			if (!fullpath) {
				if (quiet < 2) {
					if (errno == ENOENT)
						error (0, 0, _("warning: %s is a dangling symlink"), fullpath);
					else
						error (0, errno,
						       _("can't resolve %s"),
						       catdir);
				}
			} else {
				char *catdir_base;

				free (fullpath);
				drop_effective_privs ();
				pipeline_start (decomp);
				regain_effective_privs ();

				strays++;

				lg.type = CATPAGE;
				catdir_base = base_name (catdir);
				if (find_name_decompressed (decomp,
							    catdir_base,
							    &lg)) {
					struct page_description *descs;
					strays++;
					descs = parse_descriptions
						(mandir_base, lg.whatis);
					if (descs) {
						store_descriptions
							(descs, &info,
							 NULL, mandir_base,
							 NULL);
						free_descriptions (descs);
					}
				} else if (quiet < 2)
					error (0, 0, _("warning: %s: whatis parse for %s(%s) failed"),
					       catdir, mandir_base, info.sec);
				free (catdir_base);
			}

			if (lg.whatis)
				free (lg.whatis);
			pipeline_free (decomp);
next_exists:
			free_mandata_struct (exists);
			free (mandir_base);
		}
next_section:
		free (section);
	}
	closedir (cdir);
	return strays;
}

static int open_catdir (void)
{
	DIR *cdir;
	struct dirent *catlist;
	size_t catlen, manlen;
	int strays = 0;

	cdir = opendir (catdir);
	if (!cdir) {
		error (0, errno, _("can't search directory %s"), catdir);
		return 0;
	}

	if (!quiet)
		printf (_("Checking for stray cats under %s...\n"), catdir);

	catdir = appendstr (catdir, "/", NULL);
	mandir = appendstr (mandir, "/", NULL);
	catlen = strlen (catdir);
	manlen = strlen (mandir);

	/* should make this case insensitive */
	while ((catlist = readdir (cdir))) {
		char *t1;

		if (strncmp (catlist->d_name, "cat", 3) != 0)
			continue;

		catdir = appendstr (catdir, catlist->d_name, NULL);
		mandir = appendstr (mandir, catlist->d_name, NULL);

		*(t1 = mandir + manlen) = 'm';
		*(t1 + 2) = 'n';

		strays += check_for_stray ();

		*(catdir + catlen) = *(mandir + manlen) = '\0';
	}
	closedir (cdir);
	return strays;
}

int straycats (const char *manpath)
{
	char *catpath;
	int strays;

	dbf = MYDBM_RWOPEN (database);
	if (dbf && dbver_rd (dbf)) {
		MYDBM_CLOSE (dbf);
		dbf = NULL;
	}
	if (!dbf) {
		error (0, errno, _("warning: can't update index cache %s"),
		       database);
		return 0;
	}

	catpath = get_catpath (manpath, SYSTEM_CAT | USER_CAT);

	/* look in the usual catpath location */
	mandir = xstrdup (manpath);
	catdir = xstrdup (manpath);
	strays = open_catdir (); 

	/* look in the alternate catpath location if we have one 
	   and it's different from the usual catpath */

	if (catpath)
		debug ("catpath: %s, manpath: %s\n", catpath, manpath);
		
	if (catpath && strcmp (catpath, manpath) != 0) {
		*mandir = *catdir = '\0';
		mandir = appendstr (mandir, manpath, NULL);
		catdir = appendstr (catdir, catpath, NULL);
		strays += open_catdir ();
	}

	free (mandir);
	free (catdir);

	if (catpath)
		free (catpath);

	MYDBM_CLOSE (dbf);
	return strays;
}
