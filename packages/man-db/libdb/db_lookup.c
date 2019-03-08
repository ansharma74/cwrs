/*
 * db_lookup.c: low level database interface routines for man.
 *
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2012
 *               Colin Watson.
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

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "fnmatch.h"
#include "regex.h"
#include "xvasprintf.h"

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "lower.h"
#include "wordfnmatch.h"
#include "xregcomp.h"

#include "mydbm.h"
#include "db_storage.h"

/* If using ndbm or BTREE, copy the static storage before doing anything
 * interesting with it. If using gdbm, firstkey and nextkey need to copy the
 * storage because our ordered wrappers keep an effectively static copy.
 */
datum copy_datum (datum dat)
{
	if (MYDBM_DPTR (dat)) {
		MYDBM_SET_DPTR (dat, memcpy (xmalloc (MYDBM_DSIZE (dat) + 1),
						      MYDBM_DPTR (dat),
						      MYDBM_DSIZE (dat)));
		MYDBM_DPTR (dat)[MYDBM_DSIZE (dat)] = '\0';
	}
	return dat;
}

/* gdbm does locking itself. */
#if defined(NDBM) || defined(BTREE)
void gripe_lock (char *filename)
{
	error (0, errno, _("can't lock index cache %s"), filename);
}
#endif /* NDBM || BTREE */

/* issue fatal message, then exit */
void gripe_corrupt_data (void)
{
	error (FATAL, 0, _("index cache %s corrupt"), database);
}

/* deal with situation where we cannot replace a key */
void gripe_replace_key (const char *data)
{
	error (0, 0, _("cannot replace key %s"), data);
	gripe_corrupt_data ();
}

char *copy_if_set (const char *str)
{
	if (STREQ (str, "-"))
		return NULL;
	else
		return xstrdup (str);
}

const char *dash_if_unset (const char *str)
{
	if (str)
		return str;
	else
		return "-";
}

/* Just print out what would be stored in the db */
void dbprintf (const struct mandata *info)
{
	debug ("name:      %s\n"
	       "sec. ext:  %s\n"
	       "section:   %s\n"
	       "comp. ext: %s\n"
	       "id:        %c\n"
	       "st_mtime   %ld\n"
	       "pointer:   %s\n"
	       "filter:    %s\n"
	       "whatis:    %s\n\n",
	       dash_if_unset (info->name),
	       info->ext, info->sec, info->comp,
	       info->id, (long) info->_st_mtime,
	       info->pointer, info->filter, info->whatis);
}

/* Form a multi-style key from page and extension info. The page should
 * *not* be name_to_key()'d - that should only happen to the parent.
 */
datum make_multi_key (const char *page, const char *ext)
{
	datum key;

	memset (&key, 0, sizeof key);
	MYDBM_SET (key, xasprintf ("%s\t%s", page, ext));
	return key;
}

/* Free allocated elements of a mandata structure, but not the structure
 * itself.
 */
void free_mandata_elements (struct mandata *pinfo)
{
	if (pinfo->addr)
		/* TODO: this memory appears to be properly owned by the
		 * caller; why do we free it here?
		 */
		free (pinfo->addr);		/* free the 'content' */
	if (pinfo->name)
		free (pinfo->name);		/* free the real name */
}

/* Go through the linked list of structures, free()ing the 'content' and the
 * structs themselves.
 */
void free_mandata_struct (struct mandata *pinfo)
{
	while (pinfo) {
		struct mandata *next;

		next = pinfo->next;
		free_mandata_elements (pinfo);
		free (pinfo);			/* free the structure */
		pinfo = next;
	}
}

/* Get the key that should be used for a given name. The caller is
 * responsible for freeing the return value.
 */
char *name_to_key (const char *name)
{
	return lower (name);
}

/* return char ptr array to the data's fields */
char **split_data (char *content, char *start[])
{
        int count;

	/* initialise pointers to first N-1 fields */
	for (count = 0; count < FIELDS - 1 ; count++) {
		start[count] = strsep (&content, "\t");
		if (!start[count]) {
			error (0, 0,
			       ngettext ("only %d field in content",
					 "only %d fields in content", count),
			       count);
			gripe_corrupt_data ();
		}
	}

	/* initialise pointer to Nth field (whatis) */
	start[FIELDS - 1] = content;
	if (!start[FIELDS - 1]) {
		error (0, 0,
		       ngettext ("only %d field in content",
				 "only %d fields in content", FIELDS - 1),
		       FIELDS - 1);
		gripe_corrupt_data ();
	}

	return start;
}

/* Parse the db-returned data and put it into a mandata format */
void split_content (char *cont_ptr, struct mandata *pinfo)
{
	char *start[FIELDS];
	char **data;

	data = split_data (cont_ptr, start);

	pinfo->name = copy_if_set (*(data++));
	pinfo->ext = *(data++);
	pinfo->sec = *(data++);
	pinfo->_st_mtime = (time_t) atol (*(data++));	/* time_t format */
	pinfo->id = **(data++);				/* single char id */
	pinfo->pointer = *(data++);
	pinfo->filter = *(data++);
	pinfo->comp = *(data++);
	pinfo->whatis = *(data);

	pinfo->addr = cont_ptr;
	pinfo->next = (struct mandata *) NULL;
}

/* The complement of split_content */
datum make_content (struct mandata *in)
{
	datum cont;
	static const char dash[] = "-";

	memset (&cont, 0, sizeof cont);

	if (!in->pointer)
		in->pointer = dash;
	if (!in->filter)
		in->filter = dash;
	if (!in->comp)
		in->comp = dash;
	if (!in->whatis)
		in->whatis = dash + 1;

	MYDBM_SET (cont, xasprintf (
		"%s\t%s\t%s\t%ld\t%c\t%s\t%s\t%s\t%s",
		dash_if_unset (in->name),
		in->ext,
		in->sec,
		(long) in->_st_mtime,
		in->id,
		in->pointer,
		in->filter,
		in->comp,
		in->whatis));

#ifdef NDBM
	/* limit of 4096 bytes of data using ndbm */
	if (MYDBM_DSIZE (cont) > 4095) {
		MYDBM_DPTR (cont)[4095] = '\0';
		MYDBM_DSIZE (cont) = 4096;
	}
#endif
	return cont;
}

/* Extract all of the names/extensions associated with this key. Each case
 * variant of a name will be returned separately.
 *
 * names and ext should be pointers to valid memory which will be filled in
 * with the address of the allocated arrays of names and extensions. The
 * caller is expected to free these arrays.
 */
int list_extensions (char *data, char ***names, char ***ext)
{
	int count = 0;
	int bound = 4;	/* most multi keys will have fewer than this */

	*names = xnmalloc (bound, sizeof **names);
	*ext   = xnmalloc (bound, sizeof **ext);
	while (((*names)[count] = strsep (&data, "\t")) != NULL) {
		(*ext)[count] = strsep (&data, "\t");
		if ((*ext)[count])
			++count;
		else
			break;

		if (count >= bound) {
			bound *= 2;
			*names = xnrealloc (*names, bound, sizeof **names);
			*ext   = xnrealloc (*ext,   bound, sizeof **ext);
		}
	}

	debug ("found %d names/extensions\n", count);
	return count;
}

/* These should be bitwise-ored together. */
#define ALL	    0
#define	EXACT	    1
#define MATCH_CASE  2

/*
 There are three possibilities on lookup:

 1) No data exists, lookup will fail, returned structure will be NULL.
 2) One data item exists. Item is returned as first in set of structures.
 3) Many items exist. They are all returned, in a multiple structure set.
 */
#ifndef FAST_BTREE
static struct mandata *dblookup (const char *page, const char *section,
				 int flags)
{
	struct mandata *info = NULL;
	datum key, cont;

	memset (&key, 0, sizeof key);
	memset (&cont, 0, sizeof cont);

	MYDBM_SET (key, name_to_key (page));
	cont = MYDBM_FETCH (dbf, key);
	free (MYDBM_DPTR (key));

	if (MYDBM_DPTR (cont) == NULL) {	/* No entries at all */
		return info;			/* indicate no entries */
	} else if (*MYDBM_DPTR (cont) != '\t') {	/* Just one entry */
		info = infoalloc ();
		split_content (MYDBM_DPTR (cont), info);
		if (!info->name)
			info->name = xstrdup (page);
		if (!(flags & MATCH_CASE) || STREQ (info->name, page)) {
			if (section == NULL)
				return info;
			if (flags & EXACT) {
				if (STREQ (section, info->ext))
					return info;
			} else {
				if (STRNEQ (section, info->ext,
					    strlen (section)))
					return info;
			}
		}
		free_mandata_struct (info);
		return NULL;
	} else {				/* multiple entries */
		char **names, **ext;
		struct mandata *ret = NULL;
		int refs, i;

		/* Extract all of the case-variant-names/extensions
		 * associated with this key.
		 */

		refs = list_extensions (MYDBM_DPTR (cont) + 1, &names, &ext);

		/* Make the multi keys and look them up */

		for (i = 0; i < refs; ++i) {
			datum multi_cont;

			memset (&multi_cont, 0, sizeof multi_cont);

			/* Decide whether this part of a multi key is
			 * suitable.
			 */

			if ((flags & MATCH_CASE) && !STREQ (names[i], page))
				continue;

			if (section != NULL) {
				if (flags & EXACT) {
					if (!STREQ (section, ext[i]))
						continue;
				} else {
					if (!STRNEQ (section, ext[i],
						     strlen (section)))
						continue;
				}
			}

			/* So the key is suitable ... */
			key = make_multi_key (names[i], ext[i]);
			debug ("multi key lookup (%s)\n", MYDBM_DPTR (key));
			multi_cont = MYDBM_FETCH (dbf, key);
			if (MYDBM_DPTR (multi_cont) == NULL) {
				error (0, 0, _("bad fetch on multi key %s"),
				       MYDBM_DPTR (key));
				gripe_corrupt_data ();
			}
			free (MYDBM_DPTR (key));

			/* allocate info struct, fill it in and
			   point info to the next in the list */
			if (!ret)
				ret = info = infoalloc ();
			else
				info = info->next = infoalloc ();
			split_content (MYDBM_DPTR (multi_cont), info);
			if (!info->name)
				info->name = xstrdup (names[i]);
		}

		free (names);
		free (ext);
		MYDBM_FREE (MYDBM_DPTR (cont));
		return ret;
	}
}
#endif /* !FAST_BTREE */

struct mandata *dblookup_all (const char *page, const char *section,
			      int match_case)
{
	return dblookup (page, section, ALL | (match_case ? MATCH_CASE : 0));
}

struct mandata *dblookup_exact (const char *page, const char *section,
				int match_case)
{
	return dblookup (page, section, EXACT | (match_case ? MATCH_CASE : 0));
}

struct mandata *dblookup_pattern (const char *pattern, const char *section,
				  int match_case, int pattern_regex,
				  int try_descriptions)
{
	struct mandata *ret = NULL, *tail = NULL;
	datum key, cont;
	regex_t preg;

	if (pattern_regex)
		xregcomp (&preg, pattern,
			  REG_EXTENDED | REG_NOSUB |
			  (match_case ? 0 : REG_ICASE));

#ifndef BTREE
	datum nextkey;

	key = MYDBM_FIRSTKEY (dbf);
	while (MYDBM_DPTR (key)) {
		cont = MYDBM_FETCH (dbf, key);
#else /* BTREE */
	int end;

	end = btree_nextkeydata (dbf, &key, &cont);
	while (!end) {
#endif /* !BTREE */
		struct mandata info;
		char *tab;
		int got_match;

		memset (&info, 0, sizeof (info));

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

		/* If there's a section given, does it match either the
		 * section or extension of this page?
		 */
		if (section &&
		    (!STREQ (section, info.sec) && !STREQ (section, info.ext)))
			goto nextpage;

		tab = strrchr (MYDBM_DPTR (key), '\t');
		if (tab) 
			 *tab = '\0';

		if (!info.name)
			info.name = xstrdup (MYDBM_DPTR (key));

		if (pattern_regex)
			got_match = (regexec (&preg, info.name,
					      0, NULL, 0) == 0);
		else
			got_match = fnmatch (pattern, info.name,
					     match_case ? 0
							: FNM_CASEFOLD) == 0;
		if (try_descriptions && !got_match && info.whatis) {
			if (pattern_regex)
				got_match = (regexec (&preg, info.whatis,
						      0, NULL, 0) == 0);
			else
				got_match = word_fnmatch (pattern,
							  info.whatis);
		}
		if (!got_match)
			goto nextpage_tab;

		if (!ret)
			ret = tail = infoalloc ();
		else
			tail = tail->next = infoalloc ();
		memcpy (tail, &info, sizeof (info));
		info.name = NULL; /* steal memory */
		MYDBM_SET_DPTR (cont, NULL); /* == info.addr */

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
		info.addr = NULL;
		free_mandata_elements (&info);
	}

	if (pattern_regex)
		regfree (&preg);

	return ret;
}
