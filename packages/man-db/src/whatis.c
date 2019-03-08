/*
 * whatis.c: search the index or whatis database(s) for words.
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
 * routines for whatis and apropos programs. Whatis looks up the 
 * keyword for the description, apropos searches the entire database 
 * for word matches.
 *
 * Mon Aug  8 20:35:30 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 *
 * CJW: Add more safety in the face of corrupted databases.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)

#ifdef HAVE_ICONV
#  include <iconv.h>
#endif /* HAVE_ICONV */

#include <sys/types.h>
#include <sys/stat.h>
#include "regex.h"

#include "argp.h"
#include "dirname.h"
#include "fnmatch.h"
#include "xvasprintf.h"

#include "manconfig.h"

#include "cleanup.h"
#include "error.h"
#include "pipeline.h"
#include "pathsearch.h"
#include "linelength.h"
#include "hashtable.h"
#include "lower.h"
#include "wordfnmatch.h"
#include "xregcomp.h"
#include "encodings.h"

#include "mydbm.h"
#include "db_storage.h"

#include "manp.h"

static char *manpathlist[MAXDIRS];

extern char *user_config_file;
static char **keywords;
static int num_keywords;

char *program_name;
int am_apropos;
char *database;
MYDBM_FILE dbf;
int quiet = 1;

#ifdef HAVE_ICONV
iconv_t conv_to_locale;
#endif /* HAVE_ICONV */

static regex_t *preg;  
static int regex_opt;
static int exact;

static int wildcard;

static int require_all;

static int long_output;

static char **sections;

static char *manp = NULL;
static const char *alt_systems = "";
static const char *locale = NULL;
static char *multiple_locale = NULL, *internal_locale;

static struct hashtable *apropos_seen = NULL;
static struct hashtable *display_seen = NULL;

const char *argp_program_version; /* initialised in main */
const char *argp_program_bug_address = PACKAGE_BUGREPORT;
error_t argp_err_exit_status = FAIL;

static const char args_doc[] = N_("KEYWORD...");
static const char apropos_doc[] = "\v" N_("The --regex option is enabled by default.");

static struct argp_option options[] = {
	{ "debug",		'd',	0,		0,	N_("emit debugging messages") },
	{ "verbose",		'v',	0,		0,	N_("print verbose warning messages") },
	{ "regex",		'r',	0,		0,	N_("interpret each keyword as a regex"),	10 },
	{ "exact",		'e',	0,		0,	N_("search each keyword for exact match") }, /* apropos only */
	{ "wildcard",		'w',	0,		0,	N_("the keyword(s) contain wildcards") },
	{ "and",		'a',	0,		0,	N_("require all keywords to match"),			20 }, /* apropos only */
	{ "long",		'l',	0,		0,	N_("do not trim output to terminal width"),		30 },
	{ "sections",		's',	N_("LIST"),	0,	N_("search only these sections (colon-separated)"),	40 },
	{ "section",		0,	0,		OPTION_ALIAS },
	{ "systems",		'm',	N_("SYSTEM"),	0,	N_("use manual pages from other systems") },
	{ "manpath",		'M',	N_("PATH"),	0,	N_("set search path for manual pages to PATH") },
	{ "locale",		'L',	N_("LOCALE"),	0,	N_("define the locale for this search") },
	{ "config-file",	'C',	N_("FILE"),	0,	N_("use this user configuration file") },
	{ "whatis",		'f',	0,		OPTION_HIDDEN,	0 },
	{ "apropos",		'k',	0,		OPTION_HIDDEN,	0 },
	{ 0, 'h', 0, OPTION_HIDDEN, 0 }, /* compatibility for --help */
	{ 0 }
};

static char **split_sections (const char *sections_str)
{
	int i = 0;
	char *str = xstrdup (sections_str);
	const char *section;
	char **out = NULL;

	/* Although this is documented as colon-separated, at least Solaris
	 * man's -s option takes a comma-separated list, so we accept that
	 * too for compatibility.
	 */
	for (section = strtok (str, ":,"); section;
	     section = strtok (NULL, ":,")) {
		out = xnrealloc (out, i + 2, sizeof *out);
		out[i++] = xstrdup (section);
	}
	if (i)
		out[i] = NULL;

	free (str);
	return out;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
		case 'd':
			debug_level = 1;
			return 0;
		case 'v':
			quiet = 0;
			return 0;
		case 'r':
			regex_opt = 1;
			return 0;
		case 'e':
			/* Only makes sense for apropos, but has
			 * historically been accepted by whatis anyway.
			 */
			regex_opt = 0;
			exact = 1;
			return 0;
		case 'w':
			regex_opt = 0;
			wildcard = 1;
			return 0;
		case 'a':
			if (am_apropos)
				require_all = 1;
			else
				argp_usage (state);
			return 0;
		case 'l':
			long_output = 1;
			return 0;
		case 's':
			sections = split_sections (arg);
			return 0;
		case 'm':
			alt_systems = arg;
			return 0;
		case 'M':
			manp = xstrdup (arg);
			return 0;
		case 'L':
			locale = arg;
			return 0;
		case 'C':
			user_config_file = arg;
			return 0;
		case 'f':
			/* helpful override if program name detection fails */
			am_apropos = 0;
			return 0;
		case 'k':
			/* helpful override if program name detection fails */
			am_apropos = 1;
			return 0;
		case 'h':
			argp_state_help (state, state->out_stream,
					 ARGP_HELP_STD_HELP &
					 ~ARGP_HELP_PRE_DOC);
			break;
		case ARGP_KEY_ARGS:
			keywords = state->argv + state->next;
			num_keywords = state->argc - state->next;
			return 0;
		case ARGP_KEY_NO_ARGS:
			/* Make sure that we have a keyword! */
			printf (_("%s what?\n"), program_name);
			exit (FAIL);
		case ARGP_KEY_SUCCESS:
			if (am_apropos && !exact && !wildcard)
				regex_opt = 1;
			return 0;
	}
	return ARGP_ERR_UNKNOWN;
}

static struct argp apropos_argp = { options, parse_opt, args_doc, apropos_doc };
static struct argp whatis_argp = { options, parse_opt, args_doc };

static char *locale_manpath (char *manpath)
{
	char *all_locales;
	char *new_manpath;

	if (multiple_locale && *multiple_locale) {
		if (internal_locale && *internal_locale)
			all_locales = xasprintf ("%s:%s", multiple_locale,
						 internal_locale);
		else
			all_locales = xstrdup (multiple_locale);
	} else {
		if (internal_locale && *internal_locale)
			all_locales = xstrdup (internal_locale);
		else
			all_locales = NULL;
	}

	new_manpath = add_nls_manpaths (manpath, all_locales);
	free (all_locales);

	return new_manpath;
}

#ifdef HAVE_ICONV
static char *simple_convert (iconv_t conv, char *string)
{
	if (conv != (iconv_t) -1) {
		size_t string_conv_alloc = strlen (string) + 1;
		char *string_conv = xmalloc (string_conv_alloc);
		for (;;) {
			char *inptr = string, *outptr = string_conv;
			size_t inleft = strlen (string);
			size_t outleft = string_conv_alloc - 1;
			if (iconv (conv, (ICONV_CONST char **) &inptr, &inleft,
				   &outptr, &outleft) == (size_t) -1 &&
			    errno == E2BIG) {
				string_conv_alloc <<= 1;
				string_conv = xrealloc (string_conv,
							string_conv_alloc);
			} else {
				/* Either we succeeded, or we've done our
				 * best; go ahead and print what we've got.
				 */
				string_conv[string_conv_alloc - 1 - outleft] =
					'\0';
				break;
			}
		}
		return string_conv;
	} else
		return xstrdup (string);
}
#else /* !HAVE_ICONV */
#  define simple_convert(conv, string) xstrdup (string)
#endif /* HAVE_ICONV */

/* Do the old thing, if we cannot find the relevant database.
 * This invokes grep once per argument; we can't do much about this because
 * we need to know which arguments failed.  The only way to speed this up
 * would be to implement grep internally, but it hardly seems worth it for a
 * legacy mechanism.
 */
static void use_grep (const char * const *pages, int num_pages, char *manpath,
		      int *found)
{
	char *whatis_file = appendstr (NULL, manpath, "/whatis", NULL);

	if (access (whatis_file, R_OK) == 0) {
		const char *flags;
		int i;

		if (am_apropos) {
			if (regex_opt)
				flags = get_def_user (
					"apropos_regex_grep_flags",
					APROPOS_REGEX_GREP_FLAGS);
			else
				flags = get_def_user ("apropos_grep_flags",
						      APROPOS_GREP_FLAGS);
		} else
			flags = get_def_user ("whatis_grep_flags",
					      WHATIS_GREP_FLAGS);

		for (i = 0; i < num_pages; ++i) {
			pipeline *grep_pl;
			pipecmd *grep_cmd;
			char *anchored_page;

			if (am_apropos)
				anchored_page = xstrdup (pages[i]);
			else
				anchored_page = appendstr (NULL, "^", pages[i],
							   NULL);

			grep_cmd = pipecmd_new_argstr (get_def_user ("grep",
								     GREP));
			pipecmd_argstr (grep_cmd, flags);
			pipecmd_args (grep_cmd, anchored_page, whatis_file,
				      NULL);
			grep_pl = pipeline_new_commands (grep_cmd, NULL);

			if (pipeline_run (grep_pl) == 0)
				found[i] = 1;

			free (anchored_page);
		}
	} else
		debug ("warning: can't read the fallback whatis text database "
		       "%s/whatis\n", manpath);

	free (whatis_file);
}

static struct mandata *resolve_pointers (struct mandata *info,
					 const char *page)
{
	int rounds;
	const char *newpage;

	if (*(info->pointer) == '-' ||
	    ((!info->name || STREQ (info->name, page)) &&
	     STREQ (info->pointer, page)))
		return info;

	/* Now we have to work through pointers. The limit of 10 is fairly
	 * arbitrary: it's just there to avoid an infinite loop.
	 */
	newpage = info->pointer;
	info = dblookup_exact (newpage, info->ext, 1);
	for (rounds = 0; rounds < 10; rounds++) {
		struct mandata *newinfo;

		/* If the pointer lookup fails, do nothing. */
		if (!info)
			return NULL;

		if (*(info->pointer) == '-' ||
		    ((!info->name || STREQ (info->name, newpage)) &&
		     STREQ (info->pointer, newpage)))
			return info;

		newinfo = dblookup_exact (info->pointer, info->ext, 1);
		free_mandata_struct (info);
		info = newinfo;
	}

	if (!quiet)
		error (0, 0, _("warning: %s contains a pointer loop"), page);
	return NULL;
}

/* fill_in_whatis() is really a ../libdb/db_lookup.c routine but whatis.c
   is the only file that actually requires access to the whatis text... */
   
/* Take mandata struct (earlier returned from a dblookup()) and return 
   the relative whatis */
static char *get_whatis (struct mandata *info, const char *page)
{
	if (!info)
		return xstrdup (_("(unknown subject)"));

	/* See if we need to fill in the whatis here. */
	if (info->whatis != NULL && *(info->whatis))
		return xstrdup (info->whatis);
	if (!quiet && *(info->pointer) != '-')
		error (0, 0, _("warning: %s contains a pointer loop"),
		       page);
	return xstrdup (_("(unknown subject)"));
}

/* print out any matches found */
static void display (struct mandata *info, const char *page)
{
	struct mandata *newinfo;
	char *string, *whatis, *string_conv;
	const char *page_name;
	char *key;
	int line_len, rest;

	newinfo = resolve_pointers (info, page);
	whatis = get_whatis (newinfo, page);
	if (newinfo == NULL)
		newinfo = info;

	dbprintf (newinfo);

	if (newinfo->name)
		page_name = newinfo->name;
	else
		page_name = page;

	key = xasprintf ("%s (%s)", page_name, newinfo->ext);
	if (hashtable_lookup_structure (display_seen, key, strlen (key)))
		goto out;
	hashtable_install (display_seen, key, strlen (key), NULL);

	line_len = get_line_length ();

	if (strlen (page_name) > (size_t) (line_len / 2)) {
		string = xstrndup (page_name, line_len / 2 - 3);
		string = appendstr (string, "...", NULL);
	} else
		string = xstrdup (page_name);
	string = appendstr (string, " (", newinfo->ext, ")", NULL);
	if (!STREQ (newinfo->pointer, "-") && !STREQ (newinfo->pointer, page))
		string = appendstr (string, " [", newinfo->pointer, "]", NULL);

	if (strlen (string) < (size_t) 20) {
		int i;
		string = xrealloc (string, 21);
		for (i = strlen (string); i < 20; ++i)
			string[i] = ' ';
		string[i] = '\0';
	}
	string = appendstr (string, " - ", NULL);

	rest = line_len - strlen (string);
	if (!long_output && strlen (whatis) > (size_t) rest) {
		whatis[rest - 3] = '\0';
		string = appendstr (string, whatis, "...\n", NULL);
	} else
		string = appendstr (string, whatis, "\n", NULL);

	string_conv = simple_convert (conv_to_locale, string);
	fputs (string_conv, stdout);

	free (string_conv);
	free (string);

out:
	free (key);
	free (whatis);
	if (newinfo != info)
		free_mandata_struct (newinfo);
}

/* lookup the page and display the results */
static inline int do_whatis_section (const char *page, const char *section)
{
	struct mandata *info;
	int count = 0;

	info = dblookup_all (page, section, 0);
	while (info) {
		struct mandata *pinfo;
			
		display (info, page);
		count++;
		pinfo = info->next;	/* go on to next structure */
		free_mandata_elements (info);
	 	free (info);
		info = pinfo;
	}
	return count;
}

static int suitable_manpath (const char *manpath, const char *page_dir)
{
	char *page_manp;
	char *page_manpathlist[MAXDIRS], **mp;
	int ret;

	page_manp = get_manpath_from_path (page_dir, 0);
	if (!page_manp || !*page_manp) {
		free (page_manp);
		return 0;
	}
	page_manp = locale_manpath (page_manp);
	create_pathlist (page_manp, page_manpathlist);

	ret = 0;
	for (mp = page_manpathlist; *mp; ++mp) {
		if (STREQ (*mp, manpath)) {
			ret = 1;
			break;
		}
	}

	for (mp = page_manpathlist; *mp; ++mp)
		free (*mp);
	free (page_manp);
	return ret;
}

static void do_whatis (const char * const *pages, int num_pages,
		       const char *manpath, int *found)
{
	int i;

	for (i = 0; i < num_pages; ++i) {
		char *page = xstrdup (pages[i]);
		struct stat st;

		if (strchr (page, '/') && stat (page, &st) == 0 &&
		    !S_ISDIR (st.st_mode) &&
		    st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
			/* Perhaps an executable.  If its directory is on
			 * $PATH, then we only want to process this page for
			 * matching manual hierarchies.
			 */
			char *page_dir = dir_name (page);

			if (directory_on_path (page_dir)) {
				if (suitable_manpath (manpath, page_dir)) {
					char *old_page = page;
					page = base_name (old_page);
					free (old_page);
				} else {
					debug ("%s not on manpath for %s\n",
					       manpath, page);
					free (page_dir);
					continue;
				}
			}
			free (page_dir);
		}

		if (sections) {
			char * const *section;

			for (section = sections; *section; ++section) {
				if (do_whatis_section (page, *section))
					found[i] = 1;
			}
		} else {
			if (do_whatis_section (page, NULL))
				found[i] = 1;
		}

		free (page);
	}
}

/* return 1 if any of pages matches name, else 0 */
static int parse_name (const char * const *pages, int num_pages,
		       const char *dbname, int *found)
{ 
	int i;
	int ret = 0;

	if (regex_opt) {
		for (i = 0; i < num_pages; ++i) {
			if (regexec (&preg[i], dbname, 0,
				     (regmatch_t *) 0, 0) == 0)
				found[i] = ret = 1;
		}
		return ret;
	}

	if (am_apropos && !wildcard) {
		char *lowdbname = lower (dbname);

		for (i = 0; i < num_pages; ++i) {
			if (STREQ (lowdbname, pages[i]))
				found[i] = ret = 1;
		}
		free (lowdbname);
		return ret;
	}

	for (i = 0; i < num_pages; ++i) {
		if (fnmatch (pages[i], dbname, 0) == 0)
			found[i] = ret = 1;
	}
	return ret;
}

/* return 1 on word match */
static int match (const char *lowpage, const char *whatis)
{
	char *lowwhatis = lower (whatis);
	size_t len = strlen (lowpage);
	char *p, *begin;

	begin = lowwhatis;
	
	/* check for string match, then see if it is a _word_ */
	while (lowwhatis && (p = strstr (lowwhatis, lowpage))) {
		char *left = p - 1; 
		char *right = p + len;

		if ((p == begin || (!CTYPE (islower, *left) && *left != '_')) &&
		    (!*right || (!CTYPE (islower, *right) && *right != '_'))) {
		    	free (begin);
		    	return 1;
		}
		lowwhatis = p + 1;
	}

	free (begin);
	return 0;
}

/* return 1 if any of pages matches whatis, else 0 */
static int parse_whatis (const char * const *pages, char * const *lowpages,
			 int num_pages, const char *whatis, int *found)
{ 
	int i;
	int ret = 0;

	if (regex_opt) {
		for (i = 0; i < num_pages; ++i) {
			if (regexec (&preg[i], whatis, 0,
				     (regmatch_t *) 0, 0) == 0)
				found[i] = ret = 1;
		}
		return ret;
	}

	if (wildcard) {
		for (i = 0; i < num_pages; ++i) {
			if (exact) {
				if (fnmatch (pages[i], whatis, 0) == 0)
					found[i] = ret = 1;
			} else {
				if (word_fnmatch (pages[i], whatis))
					found[i] = ret = 1;
			}
		}
		return ret;
	}

	for (i = 0; i < num_pages; ++i) {
		if (match (lowpages[i], whatis))
			found[i] = ret = 1;
	}
	return ret;
}

/* cjwatson: Optimized functions don't seem to be correct in some
 * circumstances; disabled for now.
 */
#undef BTREE

/* scan for the page, print any matches */
static void do_apropos (const char * const *pages, int num_pages, int *found)
{
	datum key, cont;
	char **lowpages;
	int i;
#ifndef BTREE
	datum nextkey;
#else /* BTREE */
	int end;
#endif /* !BTREE */

	lowpages = XNMALLOC (num_pages, char *);
	for (i = 0; i < num_pages; ++i) {
		lowpages[i] = lower (pages[i]);
		debug ("lower(%s) = \"%s\"\n", pages[i], lowpages[i]);
	}

#ifndef BTREE
	key = MYDBM_FIRSTKEY (dbf);
	while (MYDBM_DPTR (key)) {
		cont = MYDBM_FETCH (dbf, key);
#else /* BTREE */
	end = btree_nextkeydata (dbf, &key, &cont);
	while (!end) {
#endif /* !BTREE */
		char *tab;
		int got_match;
		struct mandata info;

		memset (&info, 0, sizeof (info));

		/* bug#4372, NULL pointer dereference in MYDBM_DPTR (cont),
		 * fix by dassen@wi.leidenuniv.nl (J.H.M.Dassen), thanx Ray.
		 * cjwatson: In that case, complain and exit, otherwise we
		 * might loop (bug #95052).
		 */
		if (!MYDBM_DPTR (cont))
		{
			debug ("key was %s\n", MYDBM_DPTR (key));
			error (FATAL, 0,
			       _("Database %s corrupted; rebuild with "
				 "mandb --create"),
			       database);
		}

		if (*MYDBM_DPTR (key) == '$')
			goto nextpage;

		if (*MYDBM_DPTR (cont) == '\t')
			goto nextpage;

		/* a real page */

		split_content (MYDBM_DPTR (cont), &info);

		/* If there are sections given, does any of them match
		 * either the section or extension of this page?
		 */
		if (sections) {
			char * const *section;
			int matched = 0;

			for (section = sections; *section; ++section) {
				if (STREQ (*section, info.sec) ||
				    STREQ (*section, info.ext)) {
					matched = 1;
					break;
				}
			}

			if (!matched)
				goto nextpage;
		}

		tab = strrchr (MYDBM_DPTR (key), '\t');
		if (tab) 
			 *tab = '\0';

		if (am_apropos) {
			char *whatis;
			char *seen_key;
			int *seen_count;

			if (info.name)
				seen_key = xstrdup (info.name);
			else
				seen_key = xstrdup (MYDBM_DPTR (key));
			seen_key = appendstr (seen_key, " (", info.ext, ")",
					      NULL);
			seen_count = hashtable_lookup (apropos_seen, seen_key,
						       strlen (seen_key));
			if (seen_count && !require_all)
				goto nextpage_tab;
			got_match = parse_name ((const char **) lowpages,
						num_pages, MYDBM_DPTR (key),
						found);
			whatis = info.whatis ? xstrdup (info.whatis) : NULL;
			if (!got_match && whatis)
				got_match = parse_whatis (pages, lowpages,
							  num_pages, whatis,
							  found);
			free (whatis);
			if (got_match) {
				if (!seen_count) {
					seen_count = xmalloc
						(sizeof *seen_count);
					*seen_count = 0;
					hashtable_install (apropos_seen,
							   seen_key,
							   strlen (seen_key),
							   seen_count);
				}
				++(*seen_count);
				if (!require_all ||
				    *seen_count == num_keywords)
					display (&info, MYDBM_DPTR (key));
			}
			free (seen_key);
		} else {
			got_match = parse_name (pages, num_pages,
						MYDBM_DPTR (key), found);
			if (got_match)
				display (&info, MYDBM_DPTR (key));
		}

nextpage_tab:
		if (tab)
			*tab = '\t';
nextpage:
#ifndef BTREE
		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (cont));
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey; 
#else /* BTREE */
		MYDBM_FREE (MYDBM_DPTR (cont));
		MYDBM_FREE (MYDBM_DPTR (key));
		end = btree_nextkeydata (dbf, &key, &cont);
#endif /* !BTREE */
		info.addr = NULL; /* == MYDBM_DPTR (cont), freed above */
		free_mandata_elements (&info);
	}

	for (i = 0; i < num_pages; ++i)
		free (lowpages[i]);
	free (lowpages);
}

/* loop through the man paths, searching for a match */
static int search (const char * const *pages, int num_pages)
{
	int *found = XCALLOC (num_pages, int);
	char *catpath, **mp;
	int any_found, i;

	for (mp = manpathlist; *mp; mp++) {
		catpath = get_catpath (*mp, SYSTEM_CAT | USER_CAT);
		
		if (catpath) {
			database = mkdbname (catpath);
			free (catpath);
		} else
			database = mkdbname (*mp);

		debug ("path=%s\n", *mp);

		dbf = MYDBM_RDOPEN (database);
		if (dbf && dbver_rd (dbf)) {
			MYDBM_CLOSE (dbf);
			dbf = NULL;
		}
		if (!dbf) {
			use_grep (pages, num_pages, *mp, found);
			continue;
		}

		if (am_apropos)
			do_apropos (pages, num_pages, found);
		else {
			if (regex_opt || wildcard)
				do_apropos (pages, num_pages, found);
			else
				do_whatis (pages, num_pages, *mp, found);
		}
		free (database);
		database = NULL;
		MYDBM_CLOSE (dbf);
	}

	chkr_garbage_detector ();

	any_found = 0;
	for (i = 0; i < num_pages; ++i) {
		if (found[i])
			any_found = 1;
		else
			fprintf (stderr, _("%s: nothing appropriate.\n"),
				 pages[i]);
	}

	free (found);
	return any_found;
}

int main (int argc, char *argv[])
{
#ifdef HAVE_ICONV
	char *locale_charset;
#endif
	int status = OK;

	program_name = base_name (argv[0]);
	if (STREQ (program_name, APROPOS_NAME)) {
		am_apropos = 1;
		argp_program_version = "apropos " PACKAGE_VERSION;
	} else {
		struct argp_option *optionp;
		am_apropos = 0;
		argp_program_version = "whatis " PACKAGE_VERSION;
		for (optionp = (struct argp_option *) whatis_argp.options;
		     optionp->name || optionp->key || optionp->arg ||
		     optionp->flags || optionp->doc || optionp->group;
		     ++optionp) {
			if (!optionp->name)
				continue;
			if (STREQ (optionp->name, "exact") ||
			    STREQ (optionp->name, "and"))
				optionp->flags |= OPTION_HIDDEN;
		}
	}

	init_debug ();
	pipeline_install_post_fork (pop_all_cleanups);
	init_locale ();

	internal_locale = setlocale (LC_MESSAGES, NULL);
	/* Use LANGUAGE only when LC_MESSAGES locale category is
	 * neither "C" nor "POSIX". */
	if (internal_locale && strcmp (internal_locale, "C") &&
	    strcmp (internal_locale, "POSIX"))
		multiple_locale = getenv ("LANGUAGE");
	internal_locale = xstrdup (internal_locale ? internal_locale : "C");

	if (argp_parse (am_apropos ? &apropos_argp : &whatis_argp, argc, argv,
			0, 0, 0))
		exit (FAIL);

	read_config_file (user_config_file != NULL);

	/* close this locale and reinitialise if a new locale was 
	   issued as an argument or in $MANOPT */
	if (locale) {
		free (internal_locale);
		internal_locale = setlocale (LC_ALL, locale);
		if (internal_locale)
			internal_locale = xstrdup (internal_locale);
		else
			internal_locale = xstrdup (locale);

		debug ("main(): locale = %s, internal_locale = %s\n",
		       locale, internal_locale);
		if (internal_locale) {
			setenv ("LANGUAGE", internal_locale, 1);
			locale_changed ();
			multiple_locale = NULL;
		}
	}

	/* sort out the internal manpath */
	if (manp == NULL)
		manp = locale_manpath (get_manpath (alt_systems));
	else
		free (get_manpath (NULL));

	create_pathlist (manp, manpathlist);

	apropos_seen = hashtable_create (&plain_hashtable_free);
	display_seen = hashtable_create (&null_hashtable_free);

#ifdef HAVE_ICONV
	locale_charset = appendstr (NULL, get_locale_charset (), "//IGNORE",
				    NULL);
	conv_to_locale = iconv_open (locale_charset, "UTF-8");
	free (locale_charset);
#endif /* HAVE_ICONV */

	if (regex_opt) {
		int i;
		preg = XNMALLOC (num_keywords, regex_t);
		for (i = 0; i < num_keywords; ++i)
			xregcomp (&preg[i], keywords[i],
				  REG_EXTENDED | REG_NOSUB | REG_ICASE);
	}

	if (!search ((const char **) keywords, num_keywords))
		status = NOT_FOUND;

	if (regex_opt) {
		int i;
		for (i = 0; i < num_keywords; ++i)
			regfree (&preg[i]);
		free (preg);
	}

#ifdef HAVE_ICONV
	if (conv_to_locale != (iconv_t) -1)
		iconv_close (conv_to_locale);
#endif /* HAVE_ICONV */
	hashtable_free (display_seen);
	hashtable_free (apropos_seen);
	free_pathlist (manpathlist);
	free (manp);
	free (internal_locale);
	free (program_name);
	exit (status);
}
