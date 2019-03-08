/*
 * manp.c: Manpath calculations
 *
 * Copyright (C) 1990, 1991 John W. Eaton.
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
 * John W. Eaton
 * jwe@che.utexas.edu
 * Department of Chemical Engineering
 * The University of Texas at Austin
 * Austin, Texas  78712
 *
 * unpack_locale_bits is derived from _nl_explode_name in libintl:
 * Copyright (C) 1995-1998, 2000-2001, 2003, 2005 Free Software Foundation,
 * Inc.
 * Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.
 * This was originally LGPL v2 or later, but I (Colin Watson) hereby
 * exercise my option under section 3 of LGPL v2 to distribute it under the
 * GPL v2 or later as above.
 *
 * Wed May  4 15:44:47 BST 1994 Wilf. (G.Wilford@ee.surrey.ac.uk): changes
 * to get_dirlist() and manpath().
 *
 * This whole code segment is unfriendly and could do with a complete 
 * overhaul.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "canonicalize.h"
#include "xgetcwd.h"
#include "xvasprintf.h"

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "cleanup.h"

#ifdef SECURE_MAN_UID
# include "security.h"
#endif

#include "manp.h"

struct list {
	char *key;
	char *cont;
	int flag;
	struct list *next;
};

static struct list *namestore, *tailstore;

#define SECTION_USER	-6
#define SECTION		-5
#define DEFINE_USER	-4
#define DEFINE		-3
#define MANDB_MAP_USER	-2
#define MANDB_MAP	-1
#define MANPATH_MAP	 0
#define MANDATORY	 1

/* DIRLIST list[MAXDIRS]; */
static char *tmplist[MAXDIRS];

char *user_config_file = NULL;
int disable_cache;
int min_cat_width = 80, max_cat_width = 80, cat_width = 0;

static inline char *has_mandir (const char *p);
static inline char *fsstnd (const char *path);
static char *def_path (int flag);
static void add_dir_to_list (char **lp, const char *dir);
static char **add_dir_to_path_list (char **mphead, char **mp, const char *p);


static void add_to_list (const char *key, const char *cont, int flag)
{
	struct list *list = XMALLOC (struct list);
	list->key = xstrdup (key);
	list->cont = xstrdup (cont);
	list->flag = flag;
	list->next = NULL;
	if (tailstore)
		tailstore->next = list;
	tailstore = list;
	if (!namestore)
		namestore = list;
}

static const char *get_from_list (const char *key, int flag)
{
	struct list *list;

	for (list = namestore; list; list = list->next)
		if (flag == list->flag && STREQ (key, list->key))
			return list->cont;

	return NULL;
}

static struct list *iterate_over_list (struct list *prev, char *key, int flag)
{
	struct list *list;

	for (list = prev ? prev->next : namestore; list; list = list->next)
		if (flag == list->flag && STREQ (key, list->key))
			return list;

	return NULL;
}

#ifdef SECURE_MAN_UID
/* Must not return DEFINEs set in ~/.manpath. This is used to fetch
 * definitions used in raised-privilege code; if in doubt, be conservative!
 *
 * If not setuid, this is identical to get_def_user.
 */
const char *get_def (const char *thing, const char *def)
{
	const char *config_def;

	if (!running_setuid ())
		return get_def_user (thing, def);

	config_def = get_from_list (thing, DEFINE);
	return config_def ? config_def : def;
}
#endif

const char *get_def_user (const char *thing, const char *def)
{
	const char *config_def = get_from_list (thing, DEFINE_USER);
	if (!config_def)
		config_def = get_from_list (thing, DEFINE);
	return config_def ? config_def : def;
}

static void print_list (void)
{
	struct list *list;

	for (list = namestore; list; list = list->next)
		debug ("`%s'\t`%s'\t`%d'\n",
		       list->key, list->cont, list->flag);
}

static void add_sections (char *sections, int user)
{
	char *section_list = xstrdup (sections);
	char *sect;

	for (sect = strtok (section_list, " "); sect;
	     sect = strtok (NULL, " ")) {
		add_to_list (sect, "", user ? SECTION_USER : SECTION);
		debug ("Added section `%s'.\n", sect);
	}
	free (section_list);
}

const char **get_sections (void)
{
	struct list *list;
	int length_user = 0, length = 0;
	const char **sections, **sectionp;
	int flag;

	for (list = namestore; list; list = list->next) {
		if (list->flag == SECTION_USER)
			length_user++;
		else if (list->flag == SECTION)
			length++;
	}
	if (length_user) {
		sections = xnmalloc (length_user + 1, sizeof *sections);
		flag = SECTION_USER;
	} else {
		sections = xnmalloc (length + 1, sizeof *sections);
		flag = SECTION;
	}
	sectionp = sections;
	for (list = namestore; list; list = list->next)
		if (list->flag == flag)
			*sectionp++ = list->key;
	*sectionp = NULL;
	return sections;
}

static void add_def (char *thing, char *config_def, int flag, int user)
{
	add_to_list (thing, flag == 2 ? config_def : "",
		     user ? DEFINE_USER : DEFINE);

	debug ("Defined `%s' as `%s'.\n", thing, config_def);
}

static void add_manpath_map (const char *path, const char *mandir)
{
	if (!path || !mandir)
		return;

	add_to_list (path, mandir, MANPATH_MAP);

	debug ("Path `%s' mapped to mandir `%s'.\n", path, mandir);
}

static void add_mandb_map (const char *mandir, const char *catdir,
			   int flag, int user)
{
	char *tmpcatdir;

	assert (flag > 0);

	if (!mandir)
		return;

	if (flag == 1)
		tmpcatdir = xstrdup (mandir);
	else if (STREQ (catdir, "FSSTND"))
		tmpcatdir = fsstnd (mandir);
	else
		tmpcatdir = xstrdup (catdir);

	if (!tmpcatdir)
		return;

	add_to_list (mandir, tmpcatdir, user ? MANDB_MAP_USER : MANDB_MAP);

	debug ("%s mandir `%s', catdir `%s'.\n",
	       user ? "User" : "Global", mandir, tmpcatdir);

	free (tmpcatdir);
}

static void add_mandatory (const char *mandir)
{
	if (!mandir)
		return;

	add_to_list (mandir, "", MANDATORY);

	debug ("Mandatory mandir `%s'.\n", mandir);
}

/* accept (NULL or oldpath) and new path component. return new path */
static char *pathappend (char *oldpath, const char *appendage)
{
	assert ((!oldpath || *oldpath) && appendage);
	/* Remove duplicates */
	if (oldpath) {
		char *oldpathtok = xstrdup (oldpath), *tok;
		char *app_dedup = xstrdup (appendage);
		char *oldpathtok_ptr = oldpathtok;
		for (tok = strsep (&oldpathtok_ptr, ":"); tok;
		     tok = strsep (&oldpathtok_ptr, ":")) {
			char *search;
			if (!*tok)	    /* ignore empty fields */
				continue;
			search = strstr (app_dedup, tok);
			while (search) {
				char *terminator = search + strlen (tok);
				if (!*terminator) {
					/* End of the string, so chop here. */
					*search = 0;
					while (search > app_dedup &&
					       *--search == ':')
						*search = 0;
					break;
				} else if (*terminator == ':') {
					char *newapp;
					*search = 0;
					newapp = appendstr (NULL, app_dedup,
							    terminator + 1,
							    NULL);
					free (app_dedup);
					app_dedup = newapp;
				}
				search = strstr (terminator, tok);
			}
		}
		free (oldpathtok);
		if (!STREQ (appendage, app_dedup))
			debug ("%s:%s reduced to %s%s%s\n",
			       oldpath, appendage,
			       oldpath, *app_dedup ? ":" : "", app_dedup);
		if (*app_dedup)
			oldpath = appendstr (oldpath, ":", app_dedup, NULL);
		free (app_dedup);
		return oldpath;
	} else
		return xstrdup (appendage);
}

static inline void gripe_reading_mp_config (const char *file)
{
	error (FAIL, 0,
	       _("can't make sense of the manpath configuration file %s"),
	       file);
}

static inline void gripe_stat_file (const char *file)
{
	debug_error (_("warning: %s"), file);
}

static inline void gripe_not_directory (const char *dir)
{
	if (!quiet)
		error (0, 0, _("warning: %s isn't a directory"), dir);
}

static void gripe_overlong_list (void)
{
	error (FAIL, 0, _("manpath list too long"));
}

/* accept a manpath list, separated with ':', return the associated 
   catpath list */
char *cat_manpath (char *manp)
{
	char *catp = NULL;
	const char *path, *catdir;

	for (path = strsep (&manp, ":"); path; path = strsep (&manp, ":")) {
		catdir = get_from_list (path, MANDB_MAP_USER);
		if (!catdir)
			catdir = get_from_list (path, MANDB_MAP);
		catp = catdir ? pathappend (catp, catdir) 
			      : pathappend (catp, path);
	}

	return catp;
}		

/* Unpack a glibc-style locale into its component parts.
 *
 * This function was inspired by _nl_explode_name in libintl; I've rewritten
 * it here with extensive modifications in order not to require libintl or
 * glibc internals, because this API is more convenient for man-db, and to
 * be consistent with surrounding style. I also dropped the normalised
 * codeset handling, which we don't need here.
 */
void unpack_locale_bits (const char *locale, struct locale_bits *bits)
{
	const char *p, *start;

	bits->language = NULL;
	bits->territory = NULL;
	bits->codeset = NULL;
	bits->modifier = NULL;

	/* Now we determine the single parts of the locale name. First look
	 * for the language. Termination symbols are '_', '.', and '@'.
	 */
	p = locale;
	while (*p && *p != '_' && *p != '.' && *p != '@')
		++p;
	if (p == locale) {
		/* This does not make sense: language has to be specified.
		 * Use this entry as it is without exploding. Perhaps it is
		 * an alias.
		 */
		bits->language = xstrdup (locale);
		goto out;
	}
	bits->language = xstrndup (locale, p - locale);

	if (*p == '_') {
		/* Next is the territory. */
		start = ++p;
		while (*p && *p != '.' && *p != '@')
			++p;
		bits->territory = xstrndup (start, p - start);
	}

	if (*p == '.') {
		/* Next is the codeset. */
		start = ++p;
		while (*p && *p != '@')
			++p;
		bits->codeset = xstrndup (start, p - start);
	}

	if (*p == '@')
		/* Next is the modifier. */
		bits->modifier = xstrdup (++p);

out:
	if (!bits->territory)
		bits->territory = xstrdup ("");
	if (!bits->codeset)
		bits->codeset = xstrdup ("");
	if (!bits->modifier)
		bits->modifier = xstrdup ("");
}

/* Free the contents of a locale_bits structure populated by
 * unpack_locale_bits. Does not free the pointer argument.
 */
void free_locale_bits (struct locale_bits *bits)
{
	free (bits->language);
	free (bits->territory);
	free (bits->codeset);
	free (bits->modifier);
}


static char *get_nls_manpath (const char *manpathlist, const char *locale)
{
	struct locale_bits lbits;
	char *manpath = NULL;
	char *manpathlist_copy, *path, *manpathlist_ptr;

	unpack_locale_bits (locale, &lbits);
	if (STREQ (lbits.language, "C") || STREQ (lbits.language, "POSIX")) {
		free_locale_bits (&lbits);
		return xstrdup (manpathlist);
	}

	manpathlist_copy = xstrdup (manpathlist);
	manpathlist_ptr = manpathlist_copy;
	for (path = strsep (&manpathlist_ptr, ":"); path;
	     path = strsep (&manpathlist_ptr, ":")) {
		DIR *mandir = opendir (path);
		struct dirent *mandirent;

		if (!mandir)
			continue;

		while ((mandirent = readdir (mandir)) != NULL) {
			const char *name;
			struct locale_bits mbits;
			char *fullpath;

			name = mandirent->d_name;
			if (STREQ (name, ".") || STREQ (name, ".."))
				continue;
			if (STRNEQ (name, "man", 3))
				continue;
			fullpath = appendstr (NULL, path, "/", name, NULL);
			if (is_directory (fullpath) != 1) {
				free (fullpath);
				continue;
			}

			unpack_locale_bits (name, &mbits);
			if (STREQ (lbits.language, mbits.language) &&
			    (!*mbits.territory ||
			     STREQ (lbits.territory, mbits.territory)) &&
			    (!*mbits.modifier ||
			     STREQ (lbits.modifier, mbits.modifier)))
				manpath = pathappend (manpath, fullpath);
			free_locale_bits (&mbits);
			free (fullpath);
		}

		if (STREQ (lbits.language, "en"))
			/* For English, we look in the subdirectories as
			 * above just in case there's something like
			 * en_GB.UTF-8, but it's more probable that English
			 * manual pages reside at the top level.
			 */
			manpath = pathappend (manpath, path);

		closedir (mandir);
	}

	free_locale_bits (&lbits);
	return manpath;
}

char *add_nls_manpaths (char *manpathlist, const char *locales)
{
	char *manpath = NULL;
	char *locales_copy, *tok, *locales_ptr;
	char *locale_manpath;

	debug ("add_nls_manpaths(): processing %s\n", manpathlist);

	if (locales == NULL || *locales == '\0')
		return manpathlist;

	/* For each locale, we iterate over the manpath and find appropriate
	 * locale directories for each item. We then concatenate the results
	 * for all locales. In other words, LANGUAGE=fr:de and
	 * manpath=/usr/share/man:/usr/local/share/man could result in
	 * something like this list:
	 *
	 *   /usr/share/man/fr
	 *   /usr/local/share/man/fr
	 *   /usr/share/man/de
	 *   /usr/local/share/man/de
	 *   /usr/share/man
	 *   /usr/local/share/man
	 *
	 * This assumes that it's more important to have documentation in
	 * the preferred language than to have documentation for the correct
	 * object (in the case where there are different versions of a
	 * program in different hierarchies, for example). It is not
	 * entirely obvious that this is the right assumption, but on the
	 * other hand the other choice is not entirely obvious either. We
	 * tie-break on "we've always done it this way", and people can use
	 * 'man -a' or whatever in the occasional case where we get it
	 * wrong.
	 *
	 * We go to no special effort to de-duplicate directories here.
	 * create_pathlist will sort it out later; note that it preserves
	 * order in that it keeps the first of any duplicate set in its
	 * original position.
	 */

	locales_copy = xstrdup (locales);
	locales_ptr = locales_copy;
	for (tok = strsep (&locales_ptr, ":"); tok;
	     tok = strsep (&locales_ptr, ":")) {
		if (!*tok)	/* ignore empty fields */
			continue;
		debug ("checking for locale %s\n", tok);

		locale_manpath = get_nls_manpath (manpathlist, tok);
		if (locale_manpath) {
			if (manpath)
				manpath = appendstr (manpath, ":",
						     locale_manpath, NULL);
			else
				manpath = xstrdup (locale_manpath);
			free (locale_manpath);
		}
	}

	/* Always try untranslated pages as a last resort. */
	locale_manpath = get_nls_manpath (manpathlist, "C");
	if (locale_manpath) {
		if (manpath)
			manpath = appendstr (manpath, ":",
					     locale_manpath, NULL);
		else
			manpath = xstrdup (locale_manpath);
		free (locale_manpath);
	}

	return manpath;
}

static char *add_system_manpath (const char *systems, const char *manpathlist)
{
	char *one_system;
	char *manpath = NULL;
	char *tmpsystems;

	if (!systems)
		systems = getenv ("SYSTEM");

	if (!systems || !*systems)
		return xstrdup (manpathlist);

	/* Avoid breaking the environment. */
	tmpsystems = xstrdup (systems);

	/* For each systems component */

	for (one_system = strtok (tmpsystems, ",:"); one_system;
	     one_system = strtok (NULL, ",:")) {

		/* For each manpathlist component */

		if (!STREQ (one_system, "man")) {
			const char *next, *path;
			char *newdir = NULL;
			for (path = manpathlist; path; path = next) {
				int status;
				char *element;

				next = strchr (path, ':');
				if (next) {
					element = xstrndup (path, next - path);
					++next;
				} else
					element = xstrdup (path);
				newdir = appendstr (newdir, element, "/",
						    one_system, NULL);
				free (element);

				status = is_directory (newdir);

				if (status == 0)
					gripe_not_directory (newdir);
				else if (status == 1) {
					debug ("adding %s to manpathlist\n",
					       newdir);
					manpath = pathappend (manpath, newdir);
				} else
					debug_error ("can't stat %s", newdir);
				/* reset newdir */
				*newdir = '\0';
			}
			if (newdir)
				free (newdir);
		} else
			manpath = pathappend (manpath, manpathlist);
	}
	free (tmpsystems);

	/*
	 * Thu, 21 Nov 1996 22:24:19 +0200 fpolacco@debian.org
	 * bug#5534 (man fails if env var SYSTEM is defined)
	 * with error [man: internal manpath equates to NULL]
	 * the reason: is_directory (newdir); returns -1
	 */
	if (!manpath) {
		debug ("add_system_manpath(): "
		       "internal manpath equates to NULL\n");
		return xstrdup (manpathlist);
	}
	return manpath;
}

/*
 * Always add system and locale directories to pathlist.
 * If the environment variable MANPATH is set, return it.
 * If the environment variable PATH is set and has a nonzero length,
 * try to determine the corresponding manpath, otherwise, return the
 * default manpath.
 *
 * The man_db.config file is used to map system wide /bin directories
 * to top level man page directories.
 *
 * For directories which are in the user's path but not in the
 * man_db.config file, see if there is a subdirectory `man' or `MAN'.
 * If so, add that directory to the path.  Example:  user has
 * $HOME/bin in his path and the directory $HOME/bin/man exists -- the
 * directory $HOME/bin/man will be added to the manpath.
 */
static char *guess_manpath (const char *systems)
{
	const char *path = getenv ("PATH");
	char *manpathlist, *manpath;

	if (path == NULL) {
		/* Things aren't going to work well, but hey... */
		if (!quiet)
			error (0, 0, _("warning: $PATH not set"));

		manpathlist = def_path (MANDATORY);
	} else {
		if (strlen (path) == 0) {
			/* Things aren't going to work well here either... */
			if (!quiet)
				error (0, 0, _("warning: empty $PATH"));
			
			return add_system_manpath (systems,
						   def_path (MANDATORY));
		}

		manpathlist = get_manpath_from_path (path, 1);
	}
	manpath = add_system_manpath (systems, manpathlist);
	free (manpathlist);
	return manpath;
}

char *get_manpath (const char *systems)
{
	char *manpathlist;

	/* need to read config file even if MANPATH set, for mandb(8) */
	read_config_file (0);

	manpathlist = getenv ("MANPATH");
	if (manpathlist && *manpathlist) {
		char *system1, *system2, *guessed;
		char *pos;
		/* This must be it. */
		if (manpathlist[0] == ':') {
			if (!quiet)
				error (0, 0,
				       _("warning: $MANPATH set, "
					 "prepending %s"),
				       CONFIG_FILE);
			system1 = add_system_manpath (systems, manpathlist);
			guessed = guess_manpath (systems);
			manpathlist = appendstr (NULL, guessed, system1, NULL);
			free (guessed);
			free (system1);
		} else if (manpathlist[strlen (manpathlist) - 1] == ':') {
			if (!quiet)
				error (0, 0,
				       _("warning: $MANPATH set, "
					 "appending %s"),
				       CONFIG_FILE);
			system1 = add_system_manpath (systems, manpathlist);
			guessed = guess_manpath (systems);
			manpathlist = appendstr (NULL, system1, guessed, NULL);
			free (guessed);
			free (system1);
		} else if ((pos = strstr (manpathlist,"::"))) {
			*(pos++) = '\0';
			if (!quiet)
				error (0, 0,
				       _("warning: $MANPATH set, "
					 "inserting %s"),
				       CONFIG_FILE);
			system1 = add_system_manpath (systems, manpathlist);
			guessed = guess_manpath (systems);
			system2 = add_system_manpath (systems, pos);
			manpathlist = appendstr (NULL, system1, ":", guessed,
						 system2, NULL);
			free (system2);
			free (guessed);
			free (system1);
		} else {
			if (!quiet)
				error (0, 0,
				       _("warning: $MANPATH set, ignoring %s"),
				       CONFIG_FILE);
			manpathlist = add_system_manpath (systems,
							  manpathlist);
		}
	} else
		manpathlist = guess_manpath (systems);

	return manpathlist;
}

/* Parse the manpath.config file, extracting appropriate information. */
static void add_to_dirlist (FILE *config, int user)
{
	char *bp;
	char *buf = NULL;
	size_t n = 0;
	char key[50], cont[512];
	int val;
	int c;

	while (getline (&buf, &n, config) >= 0) {
		bp = buf;

		while (CTYPE (isspace, *bp))
			bp++;

		/* TODO: would like a (limited) replacement for sscanf()
		 * here that allocates its own memory. At that point check
		 * everything that sprintf()s manpath et al!
		 */
		if (*bp == '#' || *bp == '\0')
			goto next;
		else if (strncmp (bp, "NOCACHE", 7) == 0)
			disable_cache = 1;
		else if (strncmp (bp, "NO", 2) == 0)
			goto next;	/* match any word starting with NO */
		else if (sscanf (bp, "MANBIN %*s") == 1)
			goto next;
		else if (sscanf (bp, "MANDATORY_MANPATH %49s", key) == 1)
			add_mandatory (key);	
		else if (sscanf (bp, "MANPATH_MAP %49s %511s", key, cont) == 2) 
			add_manpath_map (key, cont);
		else if ((c = sscanf (bp, "MANDB_MAP %49s %511s",
				      key, cont)) > 0) 
			add_mandb_map (key, cont, c, user);
		else if ((c = sscanf (bp, "DEFINE %49s %511[^\n]",
				      key, cont)) > 0)
			add_def (key, cont, c, user);
		else if (sscanf (bp, "SECTION %511[^\n]", cont) == 1)
			add_sections (cont, user);
		else if (sscanf (bp, "SECTIONS %511[^\n]", cont) == 1)
			/* Since I keep getting it wrong ... */
			add_sections (cont, user);
		else if (sscanf (bp, "MINCATWIDTH %d", &val) == 1)
			min_cat_width = val;
		else if (sscanf (bp, "MAXCATWIDTH %d", &val) == 1)
			max_cat_width = val;
		else if (sscanf (bp, "CATWIDTH %d", &val) == 1)
			cat_width = val;
	 	else {
			error (0, 0, _("can't parse directory list `%s'"), bp);
			gripe_reading_mp_config (CONFIG_FILE);
		}

next:
		free (buf);
		buf = NULL;
	}
}

static void free_config_file (void *unused ATTRIBUTE_UNUSED)
{
	struct list *list = namestore, *prev;

	while (list) {
		free (list->key);
		free (list->cont);
		prev = list;
		list = list->next;
		free (prev);
	}

	namestore = tailstore = NULL;
}

void read_config_file (int optional)
{
	static int done = 0;
	char *dotmanpath = NULL;
	FILE *config;

	if (done)
		return;

	push_cleanup (free_config_file, NULL, 0);

	if (user_config_file)
		dotmanpath = xstrdup (user_config_file);
	else {
		char *home = getenv ("HOME");
		if (home)
			dotmanpath = appendstr (NULL, home, "/.manpath", NULL);
	}
	if (dotmanpath) {
		config = fopen (dotmanpath, "r");
		if (config != NULL) {
			debug ("From the config file %s:\n\n", dotmanpath);
			add_to_dirlist (config, 1);
			fclose (config);
		}
		free (dotmanpath);
	}

	config = fopen (CONFIG_FILE, "r");
	if (config == NULL) {
		if (optional)
			debug ("can't open %s; continuing anyway\n",
			       CONFIG_FILE);
		else
			error (FAIL, 0,
			       _("can't open the manpath configuration file "
				 "%s"), CONFIG_FILE);
	} else {
		debug ("From the config file %s:\n\n", CONFIG_FILE);

		add_to_dirlist (config, 0);
		fclose (config);
	}

	print_list ();

	done = 1;
}


/*
 * Construct the default manpath.  This picks up mandatory manpaths
 * only.
 */
static char *def_path (int flag)
{
	char *manpath = NULL;
	struct list *list; 

	for (list = namestore; list; list = list->next)
		if (list->flag == flag) {
	 		int status = is_directory (list->key);

			if (status < 0)
				gripe_stat_file (list->key);
			else if (status == 0 && !quiet)
				error (0, 0,
				       _("warning: mandatory directory %s "
					 "doesn't exist"), list->key);
			else if (status == 1)
				manpath = pathappend (manpath, list->key);
		}

	/* If we have complete config file failure... */
	if (!manpath)
		return xstrdup ("/usr/man");

	return manpath;
}

/*
 * For each directory in the user's path, see if it is one of the
 * directories listed in the man_db.config file.  If so, and it is
 * not already in the manpath, add it.  If the directory is not listed
 * in the man_db.config file, see if there is a subdirectory `../man' or
 * `man', or, for FHS-compliance, `../share/man' or `share/man'.  If so,
 * and it is not already in the manpath, add it.
 * Example:  user has $HOME/bin in his path and the directory
 * $HOME/man exists -- the directory $HOME/man will be added
 * to the manpath.
 */
char *get_manpath_from_path (const char *path, int mandatory)
{
	int len;
	char *tmppath;
	char *t;
	char *p;
	char **lp;
	char *end;
	char *manpathlist;
	struct list *list;

	tmppath = xstrdup (path);

	for (end = p = tmppath; end; p = end + 1) {
		struct list *mandir_list;

		end = strchr (p, ':');
		if (end)
			*end = '\0';

		/* don't do this for current dir ("." or empty entry in PATH) */
		if (*p == '\0' || strcmp (p, ".") == 0)
			continue;

		debug ("\npath directory %s ", p);

		mandir_list = iterate_over_list (NULL, p, MANPATH_MAP);

		/*
      		 * The directory we're working on is in the config file.
      		 * If we haven't added it to the list yet, do.
      		 */

		if (mandir_list) {
			debug ("is in the config file\n");
			while (mandir_list) {
				add_dir_to_list (tmplist, mandir_list->cont);
				mandir_list = iterate_over_list
					(mandir_list, p, MANPATH_MAP);
			}

      		 /* The directory we're working on isn't in the config file.  
      		    See if it has ../man or man subdirectories.  
      		    If so, and it hasn't been added to the list, do. */

		} else {
			debug ("is not in the config file\n");

		 	t = has_mandir (p);
		 	if (t) {
				debug ("but does have a ../man, man, "
				       "../share/man, or share/man "
				       "subdirectory\n");
	
				add_dir_to_list (tmplist, t);
				free (t);
		 	} else
				debug ("and doesn't have ../man, man, "
				       "../share/man, or share/man "
				       "subdirectories\n");
		}
	}

	free (tmppath);

	if (mandatory) {
		debug ("\nadding mandatory man directories\n\n");

		for (list = namestore; list; list = list->next)
			if (list->flag == MANDATORY) 
				add_dir_to_list (tmplist, list->key);
	}

	len = 0;
	lp = tmplist;
	while (*lp != NULL) {
		len += strlen (*lp) + 1;
		lp++;
	}

	if (!len)
		/* No path elements in configuration file or with
		 * appropriate subdirectories.
		 */
		return xstrdup ("");

	manpathlist = xmalloc (len);
	*manpathlist = '\0';

	lp = tmplist;
	p = manpathlist;
	while (*lp != NULL) {
		len = strlen (*lp);
		memcpy (p, *lp, len);
		free (*lp);
		*lp = NULL;
		p += len;
		*p++ = ':';
		lp++;
	}

	p[-1] = '\0';

	return manpathlist;
}

/*
 * Add a directory to the manpath list if it isn't already there.
 */
static void add_dir_to_list (char **lp, const char *dir)
{
	int status;
	int pos = 0;

	while (*lp != NULL) {
		if (pos > MAXDIRS - 1)
			gripe_overlong_list ();
		if (!strcmp (*lp, dir)) {
			debug ("%s is already in the manpath\n", dir);
			return;
		}
		lp++;
		pos++;
	}

	/* Not found -- add it. */

	status = is_directory (dir);

	if (status < 0)
		gripe_stat_file (dir);
	else if (status == 0)
		gripe_not_directory (dir);
	else if (status == 1) {
		debug ("adding %s to manpath\n", dir);

		*lp = xstrdup (dir);
	}
}

/* path does not exist in config file: check to see if path/../man,
   path/man, path/../share/man, or path/share/man exist.  If so return
   it, if not return NULL. */
static inline char *has_mandir (const char *path)
{
	char *newpath;

	/* don't assume anything about path, especially that it ends in 
	   "bin" or even has a '/' in it! */
	   
	char *subdir = strrchr (path, '/');
	if (subdir) {
		newpath = xasprintf ("%.*s/man", (int) (subdir - path), path);
		if (is_directory (newpath) == 1)
			return newpath;
		free (newpath);
	}

	newpath = appendstr (NULL, path, "/man", NULL);
	if (is_directory (newpath) == 1)
		return newpath;
	free (newpath);

	if (subdir) {
		newpath = xasprintf ("%.*s/share/man",
				     (int) (subdir - path), path);
		if (is_directory (newpath) == 1)
			return newpath;
		free (newpath);
	}

	newpath = appendstr (NULL, path, "/share/man", NULL);
	if (is_directory (newpath) == 1)
		return newpath;
	free (newpath);

	return NULL;
}

static char **add_dir_to_path_list (char **mphead, char **mp, const char *p)
{
	int status;
	char *cwd;

	if (mp - mphead > MAXDIRS - 1)
		gripe_overlong_list ();

	status = is_directory (p);

	if (status < 0)
		gripe_stat_file (p);
	else if (status == 0)
		gripe_not_directory (p);
	else {
		/* deal with relative paths */

		if (*p != '/') {
			cwd = xgetcwd ();
			if (!cwd)
				error (FATAL, errno,
				       _("can't determine current directory"));
			*mp = appendstr (cwd, "/", p, NULL);
		} else 
			*mp = xstrdup (p);

		debug ("adding %s to manpathlist\n", *mp);
		mp++;
	}
	return mp;
}

void create_pathlist (const char *manp, char **mp)
{
	const char *p, *end;
	char **mphead = mp;

	/* Expand the manpath into a list for easier handling. */

	for (p = manp;; p = end + 1) {
		end = strchr (p, ':');
		if (end) {
			char *element = xstrndup (p, end - p);
			mp = add_dir_to_path_list (mphead, mp, element);
			free (element);
		} else {
			mp = add_dir_to_path_list (mphead, mp, p);
			break;
		}
	}
	*mp = NULL;

	/* Eliminate duplicates due to symlinks. */
	mp = mphead;
	while (*mp) {
		char *target, *oldmp = NULL;
		char **dupcheck;
		int found_dup = 0;

		/* After resolving all symlinks, is the target also in the
		 * manpath?
		 */
		target = canonicalize_file_name (*mp);
		if (target) {
			oldmp = *mp;
			*mp = target;
		}
		/* Only check up to the current list position, to keep item
		 * order stable across deduplication.
		 */
		for (dupcheck = mphead; *dupcheck && dupcheck != mp;
		     ++dupcheck) {
			if (!STREQ (*mp, *dupcheck))
				continue;
			debug ("Removing duplicate manpath entry %s (%td) -> "
			       "%s (%td)\n",
			       oldmp, mp - mphead,
			       *dupcheck, dupcheck - mphead);
			free (*mp);
			for (dupcheck = mp; *(dupcheck + 1); ++dupcheck)
				*dupcheck = *(dupcheck + 1);
			*dupcheck = NULL;
			found_dup = 1;
			break;
		}
		if (oldmp)
			free (oldmp);
		if (!found_dup)
			++mp;
	}

	if (debug_level) {
		int first = 1;

		debug ("final search path = ");
		for (mp = mphead; *mp; ++mp) {
			if (first) {
				debug ("%s", *mp);
				first = 0;
			} else
				debug (":%s", *mp);
		}
		debug ("\n");
	}
}

void free_pathlist (char **mp)
{
	while (*mp) {
		free (*mp);
		*mp++ = NULL;
	}
}

/* Routine to get list of named system and user manpaths (in reverse order). */
char *get_mandb_manpath (void)
{
	char *manpath = NULL;
	struct list *list;

	for (list = namestore; list; list = list->next)
		if (list->flag == MANDB_MAP || list->flag == MANDB_MAP_USER)
			manpath = pathappend (manpath, list->key);

	return manpath;
}

/* Take manpath or manfile path as the first argument, and the type of
 * catpaths we want as the other (system catpaths, user catpaths, or both).
 * Return catdir mapping or NULL if it isn't a global/user mandir (as
 * appropriate).
 *
 * This routine would seem to work correctly for nls subdirs and would 
 * specify the (correct) consistent catpath even if not defined in the 
 * config file.
 *
 * Do not return user catpaths when cattype == 0! This is used to decide
 * whether to drop privileges. When cattype != 0 it's OK to return global
 * catpaths.
 */
char *get_catpath (const char *name, int cattype)
{
	struct list *list;

	for (list = namestore; list; list = list->next)
		if (((cattype & SYSTEM_CAT) && list->flag == MANDB_MAP) ||
		    ((cattype & USER_CAT)   && list->flag == MANDB_MAP_USER)) {
			size_t manlen = strlen (list->key);
			if (STRNEQ (name, list->key, manlen)) {
				const char *suffix;
				char *infix;
				char *catpath = xstrdup (list->cont);

				/* For NLS subdirectories (e.g.
				 * /usr/share/man/de -> /var/cache/man/de),
				 * we need to find the second-last slash, as
				 * long as this strictly follows the key.
				 */
				suffix = strrchr (name, '/');
				if (!suffix)
					return appendstr (catpath,
							  name + manlen, NULL);

				while (suffix > name + manlen)
					if (*--suffix == '/')
						break;
				if (suffix < name + manlen)
					suffix = name + manlen;
				if (*suffix == '/')
					++suffix;
				infix = xstrndup (name + manlen,
						  suffix - (name + manlen));
				catpath = appendstr (catpath, infix, NULL);
				free (infix);
				if (STRNEQ (suffix, "man", 3)) {
					suffix += 3;
					catpath = appendstr (catpath, "cat",
							     NULL);
				}
				catpath = appendstr (catpath, suffix, NULL);
			  	return catpath;
			}
		}

	return NULL;
}

/* Check to see if the supplied man directory is a system-wide mandir.
 * Obviously, user directories must not be included here.
 */
int is_global_mandir (const char *dir)
{
	struct list *list;

	for (list = namestore; list; list = list->next)
		if (list->flag == MANDB_MAP &&
		    STRNEQ (dir, list->key, strlen (list->key)))
		    	return 1;
	return 0;
}

/* Accept a manpath (not a full pathname to a file) and return an FSSTND 
   equivalent catpath */
static inline char *fsstnd (const char *path)
{
	char *manpath;
	char *catpath;
	char *element;
	
	if (strncmp (path, MAN_ROOT, sizeof MAN_ROOT - 1) != 0) {
		if (!quiet)
			error (0, 0, _("warning: %s does not begin with %s"),
			       path, MAN_ROOT);
		return xstrdup (path);
	}
	/* get rid of initial "/usr" */
	path += sizeof MAN_ROOT - 1;
	manpath = xstrdup (path);
	catpath = xmalloc (strlen (path) + sizeof CAT_ROOT - 3);

	/* start with CAT_ROOT */ 
	(void) strcpy (catpath, CAT_ROOT);

	/* split up path into elements and deal with accordingly */
	for (element = strtok (manpath, "/"); element;
	     element = strtok (NULL, "/")) {
		if (strncmp (element, "man", 3) == 0) {
			if (*(element + 3)) { 
				*element = 'c';
				*(element + 2) = 't';
			} else
				continue;
		} 
		(void) strcat (catpath, "/");
		(void) strcat (catpath, element);
	}
	free (manpath);
	return catpath;
}
