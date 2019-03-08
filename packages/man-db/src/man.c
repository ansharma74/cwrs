/*
 * man.c: The manual pager
 *
 * Copyright (C) 1990, 1991 John W. Eaton.
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *               2011, 2012 Colin Watson.
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
 * Mostly written/re-written by Wilf, some routines by Markus Armbruster.
 *
 * CJW: Various robustness, security, and internationalization fixes.
 * Improved HTML support (originally written by Fabrizio Polacco).
 * Rewrite of page location routines for improved maintainability and
 * accuracy.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#ifndef R_OK
#  define R_OK		4
#  define X_OK		1
#endif /* !R_OK */

#include <limits.h>

static char *cwd;

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "argp.h"
#include "dirname.h"
#include "minmax.h"
#include "regex.h"
#include "xvasprintf.h"
#include "xgetcwd.h"

#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)

#include "manconfig.h"

#include "error.h"
#include "cleanup.h"
#include "hashtable.h"
#include "pipeline.h"
#include "pathsearch.h"
#include "linelength.h"
#include "decompress.h"
#include "xregcomp.h"
#include "security.h"
#include "encodings.h"

#include "mydbm.h"
#include "db_storage.h"

#include "check_mandirs.h"
#include "filenames.h"
#include "globbing.h"
#include "ult_src.h"
#include "manp.h"
#include "convert_name.h"
#include "zsoelim.h"
#include "manconv_client.h"
#include "man.h"

#ifdef SECURE_MAN_UID
extern uid_t ruid;
extern uid_t euid;
#endif /* SECURE_MAN_UID */

/* the default preprocessor sequence */
#ifndef DEFAULT_MANROFFSEQ
#  define DEFAULT_MANROFFSEQ ""
#endif

/* placeholder for the manual page name in the less prompt string */
#define MAN_PN "$MAN_PN"

/* Some systems lack these */
#ifndef STDIN_FILENO
#  define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#  define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#  define STDERR_FILENO 2
#endif

char *lang;

/* external formatter programs, one for use without -t, and one with -t */
#define NFMT_PROG "./mandb_nfmt"
#define TFMT_PROG "./mandb_tfmt"
#undef ALT_EXT_FORMAT	/* allow external formatters located in cat hierarchy */

static int global_manpath = -1;	/* global or user manual page hierarchy? */
static int skip;		/* page exists but has been skipped */

#if defined _AIX || defined __sgi
char **global_argv;
#endif

struct candidate {
	const char *req_name;
	char from_db;
	char cat;
	const char *path;
	char *ult;
	struct mandata *source;
	int add_index; /* for sort stabilisation */
	struct candidate *next;
};

#define CANDIDATE_FILESYSTEM 0
#define CANDIDATE_DATABASE   1

static inline void gripe_system (pipeline *p, int status)
{
	error (CHILD_FAIL, 0, _("command exited with status %d: %s"),
	       status, pipeline_tostring (p));
}

enum opts {
	OPT_WARNINGS = 256,
	OPT_REGEX,
	OPT_WILDCARD,
	OPT_NAMES,
	OPT_NO_HYPHENATION,
	OPT_NO_JUSTIFICATION,
	OPT_NO_SUBPAGES,
	OPT_MAX
};

struct string_llist;
struct string_llist {
	const char *name;
	struct string_llist *next;
};


static char *manpathlist[MAXDIRS];

/* globals */
int quiet = 1;
char *program_name;
char *database;
MYDBM_FILE dbf; 
extern const char *extension; /* for globbing.c */
extern char *user_config_file;	/* defined in manp.c */
extern int disable_cache;
extern int min_cat_width, max_cat_width, cat_width;

/* locals */
static const char *alt_system_name;
static const char **section_list;		
static const char *section;
static char *colon_sep_section_list;
static const char *preprocessors;
static const char *pager;
static const char *locale;
static char *internal_locale, *multiple_locale;
static const char *prompt_string;
static char *less;
static const char *std_sections[] = STD_SECTIONS;
static char *manp;
static const char *external;
static struct hashtable *db_hash = NULL;

static int troff;
static const char *roff_device = NULL;
static const char *want_encoding = NULL;
#ifdef TROFF_IS_GROFF
static const char default_roff_warnings[] = "mac";
static struct string_llist *roff_warnings = NULL;
#endif /* TROFF_IS_GROFF */
static int global_apropos;
static int print_where, print_where_cat;
static int catman;
static int local_man_file;
static int findall;
static int update;
static int match_case;
static int regex_opt;
static int wildcard;
static int names_only;
static int ult_flags = SO_LINK | SOFT_LINK | HARD_LINK;
static const char *recode = NULL;
static int no_hyphenation;
static int no_justification;
static int subpages = 1;

static int ascii;		/* insert tr in the output pipe */
static int save_cat; 		/* security breach? Can we save the cat? */

static int first_arg;

static int found_a_stray;		/* found a straycat */

#ifdef MAN_CATS
static char *tmp_cat_file;	/* for open_cat_stream(), close_cat_stream() */
static int created_tmp_cat;			/* dto. */
#endif
static int tmp_cat_fd;
static int man_modtime;		/* modtime of man page, for commit_tmp_cat() */

# ifdef TROFF_IS_GROFF
static int ditroff;
static const char *gxditview;
static int htmlout;
static const char *html_pager;
# endif /* TROFF_IS_GROFF */

const char *argp_program_version = "man " PACKAGE_VERSION;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;
error_t argp_err_exit_status = FAIL;

static const char args_doc[] = N_("[SECTION] PAGE...");

# ifdef TROFF_IS_GROFF
#  define MAYBE_HIDDEN 0
# else
#  define MAYBE_HIDDEN OPTION_HIDDEN
# endif

/* Please keep these options in the same order as in parse_opt below. */
static struct argp_option options[] = {
	{ "config-file",	'C',	N_("FILE"),	0,		N_("use this user configuration file") },
	{ "debug",		'd',	0,		0,		N_("emit debugging messages") },
	{ "default",		'D',	0,		0,		N_("reset all options to their default values") },
	{ "warnings",  OPT_WARNINGS,    N_("WARNINGS"), MAYBE_HIDDEN | OPTION_ARG_OPTIONAL,
									N_("enable warnings from groff") },

	{ 0,			0,	0,		0,		N_("Main modes of operation:"),					10 },
	{ "whatis",		'f',	0,		0,		N_("equivalent to whatis") },
	{ "apropos",		'k',	0,		0,		N_("equivalent to apropos") },
	{ "global-apropos",	'K',	0,		0,		N_("search for text in all pages") },
	{ "where",		'w',	0,		0,		N_("print physical location of man page(s)") },
	{ "path",		0,	0,		OPTION_ALIAS },
	{ "location",		0,	0,		OPTION_ALIAS },
	{ "where-cat",		'W',	0,		0,		N_("print physical location of cat file(s)") },
	{ "location-cat",	0,	0,		OPTION_ALIAS },
	{ "local-file",		'l',	0,		0,		N_("interpret PAGE argument(s) as local filename(s)") },
	{ "catman",		'c',	0,		0,		N_("used by catman to reformat out of date cat pages"),		11 },
	{ "recode",		'R',	N_("ENCODING"),	0,		N_("output source page encoded in ENCODING") },

	{ 0,			0,	0,		0,		N_("Finding manual pages:"),					20 },
	{ "locale",		'L',	N_("LOCALE"),	0,		N_("define the locale for this particular man search") },
	{ "systems",		'm',	N_("SYSTEM"),	0,		N_("use manual pages from other systems") },
	{ "manpath",		'M',	N_("PATH"),	0,		N_("set search path for manual pages to PATH") },
	{ "sections",		'S',	N_("LIST"),	0,		N_("use colon separated section list"),				21 },
	{ 0,			's',	0,		OPTION_ALIAS },
	{ "extension",		'e',	N_("EXTENSION"),
							0,		N_("limit search to extension type EXTENSION"),			22 },
	{ "ignore-case",	'i',	0,		0,		N_("look for pages case-insensitively (default)"),		23 },
	{ "match-case",		'I',	0,		0,		N_("look for pages case-sensitively") },
	{ "regex",	  OPT_REGEX,	0,		0,		N_("show all pages matching regex"),				24 },
	{ "wildcard",  OPT_WILDCARD,	0,		0,		N_("show all pages matching wildcard") },
	{ "names-only",	  OPT_NAMES,	0,		0,		N_("make --regex and --wildcard match page names only, not "
									   "descriptions"),						25 },
	{ "all",		'a',	0,		0,		N_("find all matching manual pages"),				26 },
	{ "update",		'u',	0,		0,		N_("force a cache consistency check") },
	{ "no-subpages",
		    OPT_NO_SUBPAGES,	0,		0,		N_("don't try subpages, e.g. 'man foo bar' => 'man foo-bar'"),	27 },

	{ 0,			0,	0,		0,		N_("Controlling formatted output:"),				30 },
	{ "pager",		'P',	N_("PAGER"),	0,		N_("use program PAGER to display output") },
	{ "prompt",		'r',	N_("STRING"),	0,		N_("provide the `less' pager with a prompt") },
	{ "ascii",		'7',	0,		0,		N_("display ASCII translation of certain latin1 chars"),	31 },
	{ "encoding",		'E',	N_("ENCODING"),	0,		N_("use selected output encoding") },
	{ "no-hyphenation",
		 OPT_NO_HYPHENATION,	0,		0,		N_("turn off hyphenation") },
	{ "nh",			0,	0,		OPTION_ALIAS },
	{ "no-justification",
	       OPT_NO_JUSTIFICATION,	0,		0,		N_("turn off justification") },
	{ "nj",			0,	0,		OPTION_ALIAS },
	{ "preprocessor",	'p',	N_("STRING"),	0,		N_("STRING indicates which preprocessors to run:\n"
									   "e - [n]eqn, p - pic, t - tbl,\n"
									   "g - grap, r - refer, v - vgrind") },
#ifdef HAS_TROFF
	{ "troff",		't',	0,		0,		N_("use %s to format pages"),					32 },
	{ "troff-device",	'T',	N_("DEVICE"),	OPTION_ARG_OPTIONAL,
									N_("use %s with selected device") },
	{ "html",		'H',	N_("BROWSER"),	MAYBE_HIDDEN | OPTION_ARG_OPTIONAL,
									N_("use %s or BROWSER to display HTML output"),			33 },
	{ "gxditview",		'X',	N_("RESOLUTION"),
							MAYBE_HIDDEN | OPTION_ARG_OPTIONAL,
									N_("use groff and display through gxditview (X11):\n"
									   "-X = -TX75, -X100 = -TX100, -X100-12 = -TX100-12") },
	{ "ditroff",		'Z',	0,		MAYBE_HIDDEN,	N_("use groff and force it to produce ditroff") },
#endif /* HAS_TROFF */

	{ 0, 'h', 0, OPTION_HIDDEN, 0 }, /* compatibility for --help */
	{ 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	static int apropos, whatis; /* retain values between calls */

	/* Please keep these keys in the same order as in options above. */
	switch (key) {
		case 'C':
			user_config_file = arg;
			return 0;
		case 'd':
			debug_level = 1;
			return 0;
		case 'D':
			/* discard all preset options */
			local_man_file = findall = update = catman =
				debug_level = troff = global_apropos =
				print_where = print_where_cat =
				ascii = match_case =
				regex_opt = wildcard = names_only =
				no_hyphenation = no_justification = 0;
#ifdef TROFF_IS_GROFF
			ditroff = 0;
			gxditview = NULL;
			htmlout = 0;
			html_pager = NULL;
#endif
			roff_device = want_encoding = extension = pager =
				locale = alt_system_name = external =
				preprocessors = NULL;
			colon_sep_section_list = manp = NULL;
			return 0;

		case OPT_WARNINGS:
#ifdef TROFF_IS_GROFF
			{
				char *s = xstrdup
					(arg ? arg : default_roff_warnings);
				const char *warning;

				for (warning = strtok (s, ","); warning;
				     warning = strtok (NULL, ",")) {
					struct string_llist *new;
					new = xmalloc (sizeof *new);
					new->name = xstrdup (warning);
					new->next = roff_warnings;
					roff_warnings = new;
				}

				free (s);
			}
#endif /* TROFF_IS_GROFF */
			return 0;

		case 'f':
			external = WHATIS;
			whatis = 1;
			return 0;
		case 'k':
			external = APROPOS;
			apropos = 1;
			return 0;
		case 'K':
			global_apropos = 1;
			return 0;
		case 'w':
			print_where = 1;
			return 0;
		case 'W':
			print_where_cat = 1;
			return 0;
		case 'l':
			local_man_file = 1;
			return 0;
		case 'c':
			catman = 1;
			return 0;
		case 'R':
			recode = arg;
			ult_flags &= ~SO_LINK;
			return 0;

		case 'L':
			locale = arg;
			return 0;
		case 'm':
			alt_system_name = arg;
			return 0;
		case 'M':
			manp = arg;
			return 0;
		case 'S':
		case 's':
			if (*arg)
				colon_sep_section_list = arg;
			return 0;
		case 'e':
			extension = arg;
			return 0;
		case 'i':
			match_case = 0;
			return 0;
		case 'I':
			match_case = 1;
			return 0;
		case OPT_REGEX:
			regex_opt = 1;
			findall = 1;
			return 0;
		case OPT_WILDCARD:
			wildcard = 1;
			findall = 1;
			return 0;
		case OPT_NAMES:
			names_only = 1;
			return 0;
		case 'a':
			findall = 1;
			return 0;
		case 'u':
			update = 1;
			return 0;
		case OPT_NO_SUBPAGES:
			subpages = 0;
			return 0;

		case 'P':
			pager = arg;
			return 0;
		case 'r':
			prompt_string = arg;
			return 0;
		case '7':
			ascii = 1;
			return 0;
		case 'E':
			want_encoding = arg;
			if (is_roff_device (want_encoding))
				roff_device = want_encoding;
			return 0;
		case OPT_NO_HYPHENATION:
			no_hyphenation = 1;
			return 0;
		case OPT_NO_JUSTIFICATION:
			no_justification = 1;
			return 0;
		case 'p':
			preprocessors = arg;
			return 0;
#ifdef HAS_TROFF
		case 't':
			troff = 1;
			return 0;
		case 'T':
			/* Traditional nroff knows -T; troff does not (gets
			 * ignored). All incarnations of groff know it. Why
			 * does -T imply -t?
			 */
			roff_device = (arg ? arg : "ps");
			troff = 1;
			return 0;
		case 'H':
# ifdef TROFF_IS_GROFF
			if (arg)
				html_pager = arg;
			htmlout = 1;
			troff = 1;
			roff_device = "html";
# endif /* TROFF_IS_GROFF */
			return 0;
		case 'X':
# ifdef TROFF_IS_GROFF
			troff = 1;
			gxditview = (arg ? arg : "75");
# endif /* TROFF_IS_GROFF */
			return 0;
		case 'Z':
# ifdef TROFF_IS_GROFF
			ditroff = 1;
			troff = 1;
# endif /* TROFF_IS_GROFF */
			return 0;
#endif /* HAS_TROFF */

		case 'h':
			argp_state_help (state, state->out_stream,
					 ARGP_HELP_STD_HELP);
			break;
		case ARGP_KEY_SUCCESS:
			/* check for incompatible options */
			if (troff + whatis + apropos + catman +
			    (print_where || print_where_cat) > 1) {
				char *badopts = appendstr
					(NULL,
					 troff ? "-[tTZH] " : "",
					 whatis ? "-f " : "",
					 apropos ? "-k " : "",
					 catman ? "-c " : "",
					 print_where ? "-w " : "",
					 print_where_cat ? "-W " : "",
					 NULL);
				argp_error (state,
					    _("%s: incompatible options"),
					    badopts);
			}
			if (regex_opt + wildcard > 1) {
				char *badopts = appendstr
					(NULL,
					 regex_opt ? "--regex " : "",
					 wildcard ? "--wildcard " : "",
					 NULL);
				argp_error (state,
					    _("%s: incompatible options"),
					    badopts);
			}
			return 0;
	}
	return ARGP_ERR_UNKNOWN;
}

static char *help_filter (int key, const char *text,
			  void *input ATTRIBUTE_UNUSED)
{
#ifdef HAS_TROFF
# ifdef TROFF_IS_GROFF
	static const char formatter[] = "groff";
	const char *browser;
# else
	static const char formatter[] = "troff";
# endif /* TROFF_IS_GROFF */
#endif /* HAS_TROFF */

	switch (key) {
#ifdef HAS_TROFF
		case 't':
		case 'T':
			return xasprintf (text, formatter);
# ifdef TROFF_IS_GROFF
		case 'H':
			browser = html_pager;
			if (STRNEQ (browser, "exec ", 5))
				browser += 5;
			return xasprintf (text, browser);
# endif /* TROFF_IS_GROFF */
#endif /* HAS_TROFF */
		default:
			return (char *) text;
	}
}

static struct argp argp = { options, parse_opt, args_doc, 0, 0, help_filter };

/*
 * changed these messages from stdout to stderr,
 * (Fabrizio Polacco) Fri, 14 Feb 1997 01:30:07 +0200
 */
static void gripe_no_name (const char *sect)
{
	if (sect) {
		fprintf (stderr, _("No manual entry for %s\n"), sect);
		fprintf (stderr,
			 _("(Alternatively, what manual page do you want from "
			   "section %s?)\n"),
			 sect);
	} else
		fputs (_("What manual page do you want?\n"), stderr);

	exit (FAIL);
}

static struct termios tms;
static int tms_set = 0;

static void set_term (void)
{
	if (tms_set)
		tcsetattr (STDIN_FILENO, TCSANOW, &tms);
}

static void get_term (void)
{
	if (isatty (STDOUT_FILENO)) {
		debug ("is a tty\n");
		tcgetattr (STDIN_FILENO, &tms);
		if (!tms_set++)
			atexit (set_term);
	}
}

#if defined(TROFF_IS_GROFF) || defined(HEIRLOOM_NROFF)
static int get_roff_line_length (void)
{
	int line_length = cat_width ? cat_width : get_line_length ();

	/* groff >= 1.18 defaults to 78. */
	if (!troff && line_length != 80) {
		int length = line_length * 39 / 40;
		if (length > line_length - 2)
			return line_length - 2;
		else
			return length;
	} else
		return 0;
}

#ifdef HEIRLOOM_NROFF
static void heirloom_line_length (void *data)
{
	printf (".ll %sn\n", (const char *) data);
	/* TODO: This fails to do anything useful.  Why? */
	printf (".lt %sn\n", (const char *) data);
}
#endif /* HEIRLOOM_NROFF */

static pipecmd *add_roff_line_length (pipecmd *cmd, int *save_cat_p)
{
	int length;
	pipecmd *ret = NULL;

	if (!catman) {
		int line_length = get_line_length ();
		debug ("Terminal width %d\n", line_length);
		if (line_length >= min_cat_width &&
		    line_length <= max_cat_width)
			debug ("Terminal width %d within cat page range "
			       "[%d, %d]\n",
			       line_length, min_cat_width, max_cat_width);
		else {
			debug ("Terminal width %d not within cat page range "
			       "[%d, %d]\n",
			       line_length, min_cat_width, max_cat_width);
			*save_cat_p = 0;
		}
	}

	length = get_roff_line_length ();
	if (length) {
#ifdef HEIRLOOM_NROFF
		char *name;
		char *lldata;
		pipecmd *llcmd;
#endif /* HEIRLOOM_NROFF */

		debug ("Using %d-character lines\n", length);
#if defined(TROFF_IS_GROFF)
		pipecmd_argf (cmd, "-rLL=%dn", length);
		pipecmd_argf (cmd, "-rLT=%dn", length);
#elif defined(HEIRLOOM_NROFF)
		name = xasprintf ("echo .ll %dn && echo .lt %dn",
				  length, length);
		lldata = xasprintf ("%d", length);
		llcmd = pipecmd_new_function (name, heirloom_line_length, free,
					      lldata);
		ret = pipecmd_new_sequence ("line-length", llcmd,
					    pipecmd_new_passthrough (), NULL);
		free (name);
#endif /* HEIRLOOM_NROFF */
	}

	return ret;
}
#endif /* TROFF_IS_GROFF || HEIRLOOM_NROFF */

static inline void gripe_no_man (const char *name, const char *sec)
{
	/* On AIX and IRIX, fall back to the vendor supplied browser. */
#if defined _AIX || defined __sgi
	if (!troff) {
		pipecmd *vendor_man;
		int i;

		vendor_man = pipecmd_new ("/usr/bin/man");
		for (i = 1; i < argc; ++i)
			pipecmd_arg (vendor_man, global_argv[i]);
		pipecmd_unsetenv (vendor_man, "MANPATH");
		pipecmd_exec (vendor_man);
	}
#endif

	if (sec)
		fprintf (stderr, _("No manual entry for %s in section %s\n"),
			 name, sec);
	else
		fprintf (stderr, _("No manual entry for %s\n"), name);

#ifdef UNDOC_COMMAND
	if (pathsearch_executable (name))
		fprintf (stderr,
			 _("See '%s' for help when manual pages are not "
			   "available.\n"), UNDOC_COMMAND);
#endif
}

/* fire up the appropriate external program */
static void do_extern (int argc, char *argv[])
{
	pipeline *p;
	pipecmd *cmd;

	cmd = pipecmd_new (external);
	/* Please keep these in the same order as they are in whatis.c. */
	if (debug_level)
		pipecmd_arg (cmd, "-d");
	if (colon_sep_section_list)
		pipecmd_args (cmd, "-s", colon_sep_section_list, NULL);
	if (alt_system_name)
		pipecmd_args (cmd, "-m", alt_system_name, NULL);
	if (manp)
		pipecmd_args (cmd, "-M", manp, NULL);
	if (locale)
		pipecmd_args (cmd, "-L", locale, NULL);
	if (user_config_file)
		pipecmd_args (cmd, "-C", user_config_file, NULL);
	while (first_arg < argc)
		pipecmd_arg (cmd, argv[first_arg++]);
	p = pipeline_new_commands (cmd, NULL);

	/* privs are already dropped */
	exit (pipeline_run (p));
}

/* lookup $MANOPT and if available, put in *argv[] format for argp */
static inline char **manopt_to_env (int *argc)
{
	char *manopt, *manopt_copy, *opt_start, **argv;

	manopt = getenv ("MANOPT");
	if (manopt == NULL || *manopt == '\0')
		return NULL;

	opt_start = manopt = manopt_copy = xstrdup (manopt);

	/* allocate space for the program name */
	*argc = 0;
	argv = XNMALLOC (*argc + 3, char *);
	argv[(*argc)++] = program_name;
	
	/* for each [ \t]+ delimited string, allocate an array space and fill
	   it in. An escaped space is treated specially */	
	while (*manopt) {
		switch (*manopt) {
			case ' ':
			case '\t':
				if (manopt != opt_start) {
					*manopt = '\0';
					argv = xnrealloc (argv, *argc + 3,
							  sizeof (char *));
					argv[(*argc)++] = xstrdup (opt_start);
				}
				while (CTYPE (isspace, *(manopt + 1)))
					*++manopt = '\0';
				opt_start = manopt + 1;
				break;
			case '\\':
				if (*(manopt + 1) == ' ')
					manopt++;
				break;
			default:
				break;
		}
		manopt++;
	}

	if (*opt_start)
		argv[(*argc)++] = xstrdup (opt_start);
	argv[*argc] = NULL;			

	free (manopt_copy);
	return argv;
}

/* Return char array with 'less' special chars escaped. Uses static storage. */
static inline const char *escape_less (const char *string)
{
	static char *escaped_string; 
	char *ptr;

	/* 2*strlen will always be long enough to hold the escaped string */
	ptr = escaped_string = xrealloc (escaped_string, 
					 2 * strlen (string) + 1);
	
	while (*string) {
		if (*string == '?' ||
		    *string == ':' ||
		    *string == '.' ||
		    *string == '%' ||
		    *string == '\\')
			*ptr++ = '\\';

		*ptr++ = *string++;
	}

	*ptr = *string;
	return escaped_string;
}

#if defined(MAN_DB_CREATES) || defined(MAN_DB_UPDATES)
/* Run mandb to ensure databases are up to date. Only used with -u.
 * Returns the exit status of mandb.
 *
 * If filename is non-NULL, uses mandb's -f option to update a single file.
 */
static int run_mandb (int create, const char *manpath, const char *filename)
{
	pipeline *mandb_pl = pipeline_new ();
	pipecmd *mandb_cmd = pipecmd_new ("mandb");

	if (debug_level)
		pipecmd_arg (mandb_cmd, "-d");
	else
		pipecmd_arg (mandb_cmd, "-q");

	if (user_config_file)
		pipecmd_args (mandb_cmd, "-C", user_config_file, NULL);

	if (filename)
		pipecmd_args (mandb_cmd, "-f", filename, NULL);
	else if (create) {
		pipecmd_arg (mandb_cmd, "-c");
		pipecmd_setenv (mandb_cmd, "MAN_MUST_CREATE", "1");
	} else
		pipecmd_arg (mandb_cmd, "-p");

	if (manpath)
		pipecmd_arg (mandb_cmd, manpath);

	pipeline_command (mandb_pl, mandb_cmd);

	if (debug_level) {
		debug ("running mandb: ");
		pipeline_dump (mandb_pl, stderr);
	}

	return pipeline_run (mandb_pl);
}
#endif /* MAN_DB_CREATES || MAN_DB_UPDATES */


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


/* man issued with `-l' option */
static int local_man_loop (const char *argv)
{
	int exit_status = OK;
	int local_mf = local_man_file;

	drop_effective_privs ();
	local_man_file = 1;
	if (strcmp (argv, "-") == 0)
		display (NULL, "", NULL, "(stdin)", NULL);
	else {
		struct stat st;

		if (cwd[0]) {
			debug ("chdir %s\n", cwd);
			if (chdir (cwd)) {
				error (0, errno, _("can't chdir to %s"), cwd);
				regain_effective_privs ();
				return 0;
			}
		}

		/* Check that the file exists and isn't e.g. a directory */
		if (stat (argv, &st)) {
			error (0, errno, "%s", argv);
			return NOT_FOUND;
		}

		if (S_ISDIR (st.st_mode)) {
			error (0, EISDIR, "%s", argv);
			return NOT_FOUND;
		}

		if (S_ISCHR (st.st_mode) || S_ISBLK (st.st_mode)) {
			/* EINVAL is about the best I can do. */
			error (0, EINVAL, "%s", argv);
			return NOT_FOUND;
		}

		if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
			/* Perhaps an executable. If its directory is on
			 * $PATH, then we want to look up the corresponding
			 * manual page in the appropriate hierarchy rather
			 * than displaying the executable.
			 */
			char *argv_dir = dir_name (argv);
			int found = 0;

			if (directory_on_path (argv_dir)) {
				char *argv_base = base_name (argv);
				char *new_manp;
				char **old_manpathlist, **mp;

				debug ("recalculating manpath for executable "
				       "in %s\n", argv_dir);

				new_manp = get_manpath_from_path (argv_dir, 0);
				if (!new_manp || !*new_manp) {
					debug ("no useful manpath for "
					       "executable\n");
					goto executable_out;
				}
				new_manp = locale_manpath (new_manp);

				old_manpathlist = XNMALLOC (MAXDIRS, char *);
				memcpy (old_manpathlist, manpathlist,
					MAXDIRS * sizeof (*manpathlist));
				create_pathlist (new_manp, manpathlist);

				man (argv_base, &found);

				for (mp = manpathlist; *mp; ++mp)
					free (*mp);
				memcpy (manpathlist, old_manpathlist,
					MAXDIRS * sizeof (*manpathlist));
				free (old_manpathlist);
executable_out:
				free (new_manp);
				free (argv_base);
			}
			free (argv_dir);

			if (found)
				return OK;
		}

		if (exit_status == OK) {
			char *argv_base = base_name (argv);
			char *argv_abs;
			if (argv[0] == '/')
				argv_abs = xstrdup (argv);
			else {
				argv_abs = xgetcwd ();
				if (argv_abs)
					argv_abs = appendstr (argv_abs, "/",
							      argv, NULL);
				else
					argv_abs = xstrdup (argv);
			}
			lang = lang_dir (argv_abs);
			free (argv_abs);
			if (!display (NULL, argv, NULL, argv_base, NULL)) {
				if (local_mf)
					error (0, errno, "%s", argv);
				exit_status = NOT_FOUND;
			}
			free (lang);
			lang = NULL;
			free (argv_base);
		}
	}
	local_man_file = local_mf;
	regain_effective_privs ();
	return exit_status;
}

int main (int argc, char *argv[])
{
	int argc_env, exit_status = OK;
	char **argv_env;
	const char *tmp;

	program_name = base_name (argv[0]);

	init_debug ();
	pipeline_install_post_fork (pop_all_cleanups);

	umask (022);
	init_locale ();

	internal_locale = setlocale (LC_MESSAGES, NULL);
	/* Use LANGUAGE only when LC_MESSAGES locale category is
	 * neither "C" nor "POSIX". */
	if (internal_locale && strcmp (internal_locale, "C") &&
	    strcmp (internal_locale, "POSIX"))
		multiple_locale = getenv ("LANGUAGE");
	internal_locale = xstrdup (internal_locale ? internal_locale : "C");

/* export argv, it might be needed when invoking the vendor supplied browser */
#if defined _AIX || defined __sgi
	global_argv = argv;
#endif

	{ /* opens base streams in case someone like "info" closed them */
		struct stat buf;
		if (STDIN_FILENO < 0 ||
		    ((fstat (STDIN_FILENO, &buf) < 0) && (errno == EBADF))) 
			freopen ("/dev/null", "r", stdin);
		if (STDOUT_FILENO < 0 ||
		    ((fstat (STDOUT_FILENO, &buf) < 0) && (errno == EBADF)))
			freopen ("/dev/null", "w", stdout);
		if (STDERR_FILENO < 0 ||
		    ((fstat (STDERR_FILENO, &buf) < 0) && (errno == EBADF)))
			freopen ("/dev/null", "w", stderr);
	}

	/* This will enable us to do some profiling and know
	where gmon.out will end up. Must chdir(cwd) before we return */
	cwd = xgetcwd ();
	if (!cwd) {
		cwd = xmalloc (1);
		cwd[0] = '\0';
	}

#ifdef TROFF_IS_GROFF
	/* used in --help, so initialise early */
	if (!html_pager) {
		html_pager = getenv ("BROWSER");
		if (!html_pager)
			html_pager = WEB_BROWSER;
	}
#endif /* TROFF_IS_GROFF */

	/* First of all, find out if $MANOPT is set. If so, put it in 
	   *argv[] format for argp to play with. */
	argv_env = manopt_to_env (&argc_env);
	if (argv_env)
		if (argp_parse (&argp, argc_env, argv_env, ARGP_NO_ARGS, 0, 0))
			exit (FAIL);

	/* parse the actual program args */
	if (argp_parse (&argp, argc, argv, ARGP_NO_ARGS, &first_arg, 0))
		exit (FAIL);

#ifdef SECURE_MAN_UID
	/* record who we are and drop effective privs for later use */
	init_security ();
#endif /* SECURE_MAN_UID */

	read_config_file (local_man_file || user_config_file);

	/* if the user wants whatis or apropos, give it to them... */
	if (external)
		do_extern (argc, argv);

	get_term (); /* stores terminal settings */
#ifdef SECURE_MAN_UID
	debug ("real user = %d; effective user = %d\n", ruid, euid);
#endif /* SECURE_MAN_UID */

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

#ifdef TROFF_IS_GROFF
	if (htmlout)
		pager = html_pager;
#endif /* TROFF_IS_GROFF */
	if (pager == NULL) {
		pager = getenv ("MANPAGER");
		if (pager == NULL) {
			pager = getenv ("PAGER");
			if (pager == NULL)
				pager = get_def_user ("pager", PAGER);
		}
	}
	if (*pager == '\0')
		pager = get_def_user ("cat", CAT);

	if (prompt_string == NULL)
#ifdef LESS_PROMPT
		prompt_string = LESS_PROMPT;
#else
		prompt_string = _(
				" Manual page " MAN_PN
				" ?ltline %lt?L/%L.:byte %bB?s/%s..?e (END):"
				"?pB %pB\\%.. "
				"(press h for help or q to quit)");
#endif

	/* Restore and save $LESS in $MAN_ORIG_LESS so that recursive uses
	 * of man work as expected.
	 */
	less = getenv ("MAN_ORIG_LESS");
	if (less == NULL)
		less = getenv ("LESS");
	setenv ("MAN_ORIG_LESS", less ? less : "", 1);

	debug ("\nusing %s as pager\n", pager);

	if (first_arg == argc) {
		/* http://twitter.com/#!/marnanel/status/132280557190119424 */
		time_t now = time (NULL);
		struct tm *localnow = localtime (&now);
		if (localnow &&
		    localnow->tm_hour == 0 && localnow->tm_min == 30)
			fprintf (stderr, "gimme gimme gimme\n");

		if (print_where) {
			manp = get_manpath ("");
			printf ("%s\n", manp);
			exit (OK);
		} else
			gripe_no_name (NULL);
	}

	section_list = get_section_list ();

	if (manp == NULL)
		manp = locale_manpath (get_manpath (alt_system_name));
	else
		free (get_manpath (NULL));

	debug ("manpath search path (with duplicates) = %s\n", manp);

	create_pathlist (manp, manpathlist);

	/* man issued with `-l' option */
	if (local_man_file) {
		while (first_arg < argc) {
			exit_status = local_man_loop (argv[first_arg]);
			++first_arg;
		}
		free (cwd);
		free (internal_locale);
		free (program_name);
		exit (exit_status);
	}

	/* finished manpath processing, regain privs */
	regain_effective_privs ();

#ifdef MAN_DB_UPDATES
	/* If `-u', do it now. */
	if (update) {
		int status = run_mandb (0, NULL, NULL);
		if (status)
			error (0, 0,
			       _("mandb command failed with exit status %d"),
			       status);
	}
#endif /* MAN_DB_UPDATES */

	while (first_arg < argc) {
		int status = OK;
		int found = 0;
		static int maybe_section = 0;
		const char *nextarg = argv[first_arg++];

		/*
     		 * See if this argument is a valid section name.  If not,
      		 * is_section returns NULL.
      		 */
		if (!catman) {
			tmp = is_section (nextarg);
			if (tmp) {
				section = tmp;
				debug ("\nsection: %s\n", section);
				maybe_section = 1;
			}
		}

		if (maybe_section) {
			if (first_arg < argc)
				/* e.g. 'man 3perl Shell' */
				nextarg = argv[first_arg++];
			else
				/* e.g. 'man 9wm' */
				section = NULL;
				/* ... but leave maybe_section set so we can
				 * tell later that this happened.
				 */
		}

		/* this is where we actually start looking for the man page */
		skip = 0;
		if (global_apropos)
			status = do_global_apropos (nextarg, &found);
		else {
			int found_subpage = 0;
			if (subpages && first_arg < argc) {
				char *subname = xasprintf (
					"%s-%s", nextarg, argv[first_arg]);
				status = man (subname, &found);
				free (subname);
				if (status == OK) {
					found_subpage = 1;
					++first_arg;
				}
			}
			if (!found_subpage && subpages && first_arg < argc) {
				char *subname = xasprintf (
					"%s_%s", nextarg, argv[first_arg]);
				status = man (subname, &found);
				free (subname);
				if (status == OK) {
					found_subpage = 1;
					++first_arg;
				}
			}
			if (!found_subpage)
				status = man (nextarg, &found);
		}

		/* clean out the cache of database lookups for each man page */
		hashtable_free (db_hash);
		db_hash = NULL;

		if (section && maybe_section) {
			if (status != OK && !catman) {
				/* Maybe the section wasn't a section after
				 * all? e.g. 'man 9wm fvwm'.
				 */
				int found_subpage = 0;
				debug ("\nRetrying section %s as name\n",
				       section);
				tmp = section;
				section = NULL;
				if (subpages) {
					char *subname = xasprintf (
						"%s-%s", tmp, nextarg);
					status = man (subname, &found);
					free (subname);
					if (status == OK) {
						found_subpage = 1;
						++first_arg;
					}
				}
				if (!found_subpage)
					status = man (tmp, &found);
				hashtable_free (db_hash);
				db_hash = NULL;
				/* ... but don't gripe about it if it doesn't
				 * work!
				 */
				if (status == OK) {
					/* It was a name after all, so arrange
					 * to try the next page again with a
					 * null section.
					 */
					nextarg = tmp;
					--first_arg;
				} else
					/* No go, it really was a section. */
					section = tmp;
			}
		}

		if (status != OK && !catman) {
			if (!skip) {
				exit_status = status;
				if (exit_status == NOT_FOUND) {
					if (!section && maybe_section &&
					    CTYPE (isdigit, nextarg[0]))
						gripe_no_name (nextarg);
					else
						gripe_no_man (nextarg, section);
				}
			}
		} else {
			debug ("\nFound %d man pages\n", found);
			if (catman) {
				printf ("%s", nextarg);
				if (section)
					printf ("(%s)", section);
				if (first_arg != argc)
					fputs (", ", stdout);
				else
					fputs (".\n", stdout);
			}
		}

		maybe_section = 0;

		chkr_garbage_detector ();
	}
	hashtable_free (db_hash);
	db_hash = NULL;

	drop_effective_privs ();

	/* For profiling */
	if (cwd[0])
		chdir (cwd);

	free_pathlist (manpathlist);
	free (cwd);
	free (internal_locale);
	free (program_name);
	exit (exit_status);
}

/*
 * Check to see if the argument is a valid section number. 
 * If the name matches one of
 * the sections listed in section_list, we'll assume that it's a section.
 * The list of sections in config.h simply allows us to specify oddly
 * named directories like .../man3f.  Yuk.
 */
static inline const char *is_section (const char *name)
{
	const char **vs;

	for (vs = section_list; *vs; vs++) {
		if (STREQ (*vs, name))
			return name;
		/* allow e.g. 3perl but disallow 8139too and libfoo */
		if (strlen (*vs) == 1 && CTYPE (isdigit, **vs) &&
		    strlen (name) > 1 && !CTYPE (isdigit, name[1]) &&
		    STRNEQ (*vs, name, 1))
			return name;
	}
	return NULL;
}

/* Snarf pre-processors from file, return (static) string or NULL on failure */
static const char *get_preprocessors_from_file (pipeline *decomp)
{
	static char *directive = NULL;

	if (directive) {
		free (directive);
		directive = NULL;
	}

#ifdef PP_COOKIE
	const char *line;

	if (!decomp)
		return NULL;

	line = pipeline_peekline (decomp);
	if (!line)
		return NULL;

	if (!strncmp (line, PP_COOKIE, 4)) {
		const char *newline = strchr (line, '\n');
		if (newline)
			directive = xstrndup (line + 4, newline - (line + 4));
		else
			directive = xstrdup (line + 4);
	}

	/* if we didn't find PP_COOKIE, then directive == NULL */
#endif
	return directive;
}


/* Determine pre-processors, set save_cat and return
   (static) string */
static const char *get_preprocessors (pipeline *decomp, const char *dbfilters)
{
	const char *pp_string;
	const char *pp_source;

	/* try in order: database, command line, file, environment, default */
	/* command line overrides the database, but database empty overrides default */
	if (dbfilters && (dbfilters[0] != '-') && !preprocessors) {
		pp_string = dbfilters;
		pp_source = "database";
		save_cat = 1;
	} else if ((pp_string = preprocessors)) {
		pp_source = "command line";
		save_cat = 0;
	} else if ((pp_string = get_preprocessors_from_file (decomp))) {
		pp_source = "file";
		save_cat = 1;
	} else if ((pp_string = getenv ("MANROFFSEQ"))) {
		pp_source = "environment";
		save_cat = 0;
	} else if (!dbfilters) {
		pp_string = DEFAULT_MANROFFSEQ;
		pp_source = "default";
		save_cat = 1;
	} else {
		pp_string = "";
		pp_source = "no filters";
		save_cat = 1;
	}

	debug ("pre-processors `%s' from %s\n", pp_string, pp_source);
	return pp_string;
}

static const char *my_locale_charset (void)
{
	if (want_encoding && !is_roff_device (want_encoding))
		return want_encoding;
	else
		return get_locale_charset ();
}

static void add_col (pipeline *p, const char *locale_charset, ...)
{
	pipecmd *cmd;
	va_list argv;
	char *col_locale = NULL;

	cmd = pipecmd_new (COL);
	va_start (argv, locale_charset);
	pipecmd_argv (cmd, argv);
	va_end (argv);

	if (locale_charset)
		col_locale = find_charset_locale (locale_charset);
	if (col_locale) {
		pipecmd_setenv (cmd, "LC_CTYPE", col_locale);
		free (col_locale);
	}

	pipeline_command (p, cmd);
}

/* Return pipeline to format file to stdout. */
static pipeline *make_roff_command (const char *dir, const char *file,
				    pipeline *decomp, const char *dbfilters,
				    char **result_encoding)
{
	const char *pp_string;
	const char *roff_opt;
	char *fmt_prog;
	pipeline *p = pipeline_new ();
	pipecmd *cmd;
	char *page_encoding = NULL;
	const char *output_encoding = NULL;
	const char *locale_charset = NULL;

	*result_encoding = xstrdup ("UTF-8"); /* optimistic default */

	pp_string = get_preprocessors (decomp, dbfilters);

	roff_opt = getenv ("MANROFFOPT");
	if (!roff_opt)
		roff_opt = "";

#ifdef ALT_EXT_FORMAT
	/* Check both external formatter locations */
	if (dir && !recode) {
		char *catpath = get_catpath
			(dir, global_manpath ? SYSTEM_CAT : USER_CAT);

		/* If we have an alternate catpath */
		if (catpath) {
			fmt_prog = appendstr (catpath, "/",
					      troff ? TFMT_PROG : NFMT_PROG, 
					      NULL);
			if (access (fmt_prog, X_OK)) {
				free (fmt_prog);
				fmt_prog = xstrdup (troff ? TFMT_PROG :
							    NFMT_PROG);
				if (access (fmt_prog, X_OK)) {
					free (fmt_prog);
					fmt_prog = NULL;
				}
			}
		/* If we don't */
		} else {
#endif /* ALT_EXT_FORMAT */

			fmt_prog = xstrdup (troff ? TFMT_PROG : NFMT_PROG);
			if (access (fmt_prog, X_OK)) {
				free (fmt_prog);
				fmt_prog = NULL;
			}

#ifdef ALT_EXT_FORMAT
		}
	} else
		fmt_prog = NULL;
#endif /* ALT_EXT_FORMAT */
	
	if (fmt_prog)
		debug ("External formatter %s\n", fmt_prog);
				
	if (!fmt_prog) {
		/* we don't have an external formatter script */
		const char *source_encoding, *roff_encoding;
		const char *groff_preconv;

		if (!recode) {
			struct zsoelim_stdin_data *zsoelim_data;

			zsoelim_data = zsoelim_stdin_data_new (dir,
							       manpathlist);
			cmd = pipecmd_new_function (SOELIM, &zsoelim_stdin,
						    zsoelim_stdin_data_free,
						    zsoelim_data);
			pipeline_command (p, cmd);
		}

		page_encoding = check_preprocessor_encoding (decomp);
		if (!page_encoding)
			page_encoding = get_page_encoding (lang);
		if (page_encoding && !STREQ (page_encoding, "UTF-8"))
			source_encoding = page_encoding;
		else
			source_encoding = get_source_encoding (lang);
		debug ("page_encoding = %s\n", page_encoding);
		debug ("source_encoding = %s\n", source_encoding);

		/* Load the roff_device value dependent on the language dir
		 * in the path.
		 */
		if (!troff) {
#define STRC(s, otherwise) ((s) ? (s) : (otherwise))

			locale_charset = my_locale_charset ();
			debug ("locale_charset = %s\n",
			       STRC (locale_charset, "NULL"));

			/* Pick the default device for this locale if there
			 * wasn't one selected explicitly.
			 */
			if (!roff_device) {
				roff_device =
					get_default_device (locale_charset,
							    source_encoding);
#ifdef HEIRLOOM_NROFF
				/* In Heirloom, if LC_CTYPE is a UTF-8
				 * locale, then -Tlocale will be equivalent
				 * to -Tutf8 except that it will do a
				 * slightly better job of rendering some
				 * special characters.
				 */
				if (STREQ (roff_device, "utf8")) {
					const char *real_locale_charset =
						get_locale_charset ();
					if (real_locale_charset &&
					    STREQ (real_locale_charset,
						   "UTF-8"))
						roff_device = "locale";
				}
#endif /* HEIRLOOM_NROFF */
				debug ("roff_device (locale) = %s\n",
				       STRC (roff_device, "NULL"));
			}
		}

		roff_encoding = get_roff_encoding (roff_device,
						   source_encoding);
		debug ("roff_encoding = %s\n", roff_encoding);

		/* We may need to recode:
		 *   from page_encoding to roff_encoding on input;
		 *   from output_encoding to locale_charset on output
		 *     (if not troff).
		 * If we have preconv, then use it to recode the
		 * input to a safe escaped form.
		 * The --recode option overrides everything else.
		 */
		groff_preconv = get_groff_preconv ();
		if (recode)
			add_manconv (p, page_encoding, recode);
		else if (groff_preconv) {
			add_manconv (p, page_encoding, "UTF-8");
			pipeline_command_args
				(p, groff_preconv, "-e", "UTF-8", NULL);
		} else if (roff_encoding)
			add_manconv (p, page_encoding, roff_encoding);
		else
			add_manconv (p, page_encoding, page_encoding);

		if (!troff && !recode) {
			output_encoding = get_output_encoding (roff_device);
			if (!output_encoding)
				output_encoding = source_encoding;
			debug ("output_encoding = %s\n", output_encoding);
			free (*result_encoding);
			*result_encoding = xstrdup (output_encoding);

			if (!getenv ("LESSCHARSET")) {
				const char *less_charset =
					get_less_charset (locale_charset);
				debug ("less_charset = %s\n", less_charset);
				setenv ("LESSCHARSET", less_charset, 1);
			}

			if (!getenv ("JLESSCHARSET")) {
				const char *jless_charset =
					get_jless_charset (locale_charset);
				if (jless_charset) {
					debug ("jless_charset = %s\n",
					       jless_charset);
					setenv ("JLESSCHARSET",
						jless_charset, 1);
				}
			}
		}
	}

	if (recode)
		;
	else if (!fmt_prog) {
#ifndef GNU_NROFF
		int using_tbl = 0;
#endif /* GNU_NROFF */

		do {
#ifdef TROFF_IS_GROFF
			struct string_llist *cur;
#endif /* TROFF_IS_GROFF */
			int wants_dev = 0; /* filter wants a dev argument */
			int wants_post = 0; /* postprocessor arguments */

			cmd = NULL;
			/* set cmd according to *pp_string, on
                           errors leave cmd as NULL */
			switch (*pp_string) {
			case 'e':
				if (troff)
					cmd = pipecmd_new_argstr
						(get_def ("eqn", EQN));
				else
					cmd = pipecmd_new_argstr
						(get_def ("neqn", NEQN));
				wants_dev = 1;
				break;
			case 'g':
				cmd = pipecmd_new_argstr
					(get_def ("grap", GRAP));
				break;
			case 'p':
				cmd = pipecmd_new_argstr
					(get_def ("pic", PIC));
				break;
			case 't':
				cmd = pipecmd_new_argstr
					(get_def ("tbl", TBL));
#ifndef GNU_NROFF
				using_tbl = 1;
#endif /* GNU_NROFF */
				break;
			case 'v':
				cmd = pipecmd_new_argstr
					(get_def ("vgrind", VGRIND));
				break;
			case 'r':
				cmd = pipecmd_new_argstr
					(get_def ("refer", REFER));
				break;
			case ' ':
			case '-':
			case 0:
				/* done with preprocessors, now add roff */
				if (troff) {
					cmd = pipecmd_new_argstr
						(get_def ("troff", TROFF));
					save_cat = 0;
				} else
					cmd = pipecmd_new_argstr
						(get_def ("nroff", NROFF));

#ifdef TROFF_IS_GROFF
				if (troff && ditroff)
					pipecmd_arg (cmd, "-Z");
#endif /* TROFF_IS_GROFF */

#if defined(TROFF_IS_GROFF) || defined(HEIRLOOM_NROFF)
				if (!troff) {
					pipecmd *seq = add_roff_line_length
						(cmd, &save_cat);
					if (seq)
						pipeline_command (p, seq);
				}
#endif /* TROFF_IS_GROFF || HEIRLOOM_NROFF */

#ifdef TROFF_IS_GROFF
				for (cur = roff_warnings; cur;
				     cur = cur->next)
					pipecmd_argf (cmd, "-w%s", cur->name);
#endif /* TROFF_IS_GROFF */

#ifdef HEIRLOOM_NROFF
				if (running_setuid ())
					pipecmd_unsetenv (cmd, "TROFFMACS");
#endif /* HEIRLOOM_NROFF */

				pipecmd_argstr (cmd, roff_opt);

				wants_dev = 1;
				wants_post = 1;
				break;
			}

			if (!cmd) {
				assert (*pp_string); /* didn't fail on roff */
				error (0, 0,
				       _("ignoring unknown preprocessor `%c'"),
				       *pp_string);
				continue;
			}

			if (wants_dev) {
				if (roff_device)
					pipecmd_argf (cmd,
						      "-T%s", roff_device);
#ifdef TROFF_IS_GROFF
				else if (gxditview)
					pipecmd_argf (cmd, "-TX%s", gxditview);
#endif /* TROFF_IS_GROFF */
			}

			if (wants_post) {
#ifdef TROFF_IS_GROFF
				if (gxditview)
					pipecmd_arg (cmd, "-X");
#endif /* TROFF_IS_GROFF */

				if (roff_device && STREQ (roff_device, "ps"))
					/* Tell grops to guess the page
					 * size.
					 */
					pipecmd_arg (cmd, "-P-g");
			}

			pipeline_command (p, cmd);

			if (*pp_string == ' ' || *pp_string == '-')
				break;
		} while (*pp_string++);

		if (!troff && *COL) {
			const char *man_keep_formatting =
				getenv ("MAN_KEEP_FORMATTING");
			if ((!man_keep_formatting || !*man_keep_formatting) &&
			    !isatty (STDOUT_FILENO))
				/* we'll run col later, but prepare for it */
				setenv ("GROFF_NO_SGR", "1", 1);
#ifndef GNU_NROFF
			/* tbl needs col */
			else if (using_tbl && !troff && *COL)
				add_col (p, locale_charset, NULL);
#endif /* GNU_NROFF */
		}
	} else {
		/* use external formatter script, it takes arguments
		   input file, preprocessor string, and (optional)
		   output device */
		cmd = pipecmd_new_args (fmt_prog, file, pp_string, NULL);
		if (roff_device)
			pipecmd_arg (cmd, roff_device);
		pipeline_command (p, cmd);
	}

	free (page_encoding);
	return p;
}

#ifdef TROFF_IS_GROFF
/* Return pipeline to run a browser on a given file, observing
 * http://www.tuxedo.org/~esr/BROWSER/.
 *
 * (Actually, I really implement
 * http://www.dwheeler.com/browse/secure_browser.html, but it's
 * backward-compatible.)
 *
 * TODO: Is there any way to use the pipeline library better here?
 */
static pipeline *make_browser (const char *pattern, const char *file)
{
	pipeline *p;
	char *browser = xmalloc (1);
	int found_percent_s = 0;
	char *percent;
	char *esc_file;

	*browser = '\0';

	percent = strchr (pattern, '%');
	while (percent) {
		size_t len = strlen (browser);
		browser = xrealloc (browser, len + 1 + (percent - pattern));
		strncat (browser, pattern, percent - pattern);
		switch (*(percent + 1)) {
			case '\0':
			case '%':
				browser = appendstr (browser, "%", NULL);
				break;
			case 'c':
				browser = appendstr (browser, ":", NULL);
				break;
			case 's':
				esc_file = escape_shell (file);
				browser = appendstr (browser, esc_file, NULL);
				free (esc_file);
				found_percent_s = 1;
				break;
			default:
				len = strlen (browser); /* cannot be NULL */
				browser = xrealloc (browser, len + 3);
				strncat (browser, percent, 2);
				break;
		}
		if (*(percent + 1))
			pattern = percent + 2;
		else
			pattern = percent + 1;
		percent = strchr (pattern, '%');
	}
	browser = appendstr (browser, pattern, NULL);
	if (!found_percent_s) {
		esc_file = escape_shell (file);
		browser = appendstr (browser, " ", esc_file, NULL);
		free (esc_file);
	}

	p = pipeline_new_command_args ("/bin/sh", "-c", browser, NULL);
	pipeline_ignore_signals (p, 1);
	free (browser);

	return p;
}
#endif /* TROFF_IS_GROFF */

static void setenv_less (pipecmd *cmd, const char *title)
{
	const char *esc_title;
	char *less_opts, *man_pn;
	const char *force = getenv ("MANLESS");

	if (force) {
		pipecmd_setenv (cmd, "LESS", force);
		return;
	}

	esc_title = escape_less (title);
	less_opts = xasprintf (LESS_OPTS, prompt_string, prompt_string);
	less_opts = appendstr (less_opts, less, NULL);
	man_pn = strstr (less_opts, MAN_PN);
	while (man_pn) {
		char *subst_opts =
			xmalloc (strlen (less_opts) - strlen (MAN_PN) +
				 strlen (esc_title) + 1);
		strncpy (subst_opts, less_opts, man_pn - less_opts);
		subst_opts[man_pn - less_opts] = '\0';
		strcat (subst_opts, esc_title);
		strcat (subst_opts, man_pn + strlen (MAN_PN));
		free (less_opts);
		less_opts = subst_opts;
		man_pn = strstr (less_opts, MAN_PN);
	}

	debug ("Setting LESS to %s\n", less_opts);
	pipecmd_setenv (cmd, "LESS", less_opts);

	debug ("Setting MAN_PN to %s\n", esc_title);
	pipecmd_setenv (cmd, "MAN_PN", esc_title);

	free (less_opts);
}

static void add_output_iconv (pipeline *p,
			      const char *source, const char *target)
{
	debug ("add_output_iconv: source %s, target %s\n", source, target);
	if (source && target && !STREQ (source, target)) {
		char *target_translit = xasprintf ("%s//TRANSLIT", target);
		pipeline_command_args (p, "iconv", "-c",
				       "-f", source, "-t", target_translit,
				       NULL);
		free (target_translit);
	}
}

/* Return pipeline to display file provided on stdin.
 *
 * TODO: htmlout case is pretty weird now. I'd like the intelligence to be
 * somewhere other than format_display.
 */
static pipeline *make_display_command (const char *encoding, const char *title)
{
	pipeline *p = pipeline_new ();
	const char *locale_charset = NULL;
	pipecmd *pager_cmd = NULL;

	locale_charset = my_locale_charset ();

	if (!troff && (!want_encoding || !is_roff_device (want_encoding)))
		add_output_iconv (p, encoding, locale_charset);

	if (!troff && *COL) {
		/* get rid of special characters if not writing to a
		 * terminal
		 */
		const char *man_keep_formatting =
			getenv ("MAN_KEEP_FORMATTING");
		if ((!man_keep_formatting || !*man_keep_formatting) &&
		    !isatty (STDOUT_FILENO))
			add_col (p, locale_charset, "-b", "-p", "-x", NULL);
	}

	if (isatty (STDOUT_FILENO)) {
		if (ascii) {
			pipeline_command_argstr
				(p, get_def_user ("tr", TR TR_SET1 TR_SET2));
			pager_cmd = pipecmd_new_argstr (pager);
		} else
#ifdef TROFF_IS_GROFF
		if (!htmlout)
			/* format_display deals with html_pager */
#endif
			pager_cmd = pipecmd_new_argstr (pager);
	}

	if (pager_cmd) {
		setenv_less (pager_cmd, title);
		pipeline_command (p, pager_cmd);
	}
	pipeline_ignore_signals (p, 1);

	if (!pipeline_get_ncommands (p))
		/* Always return at least a dummy pipeline. */
		pipeline_command (p, pipecmd_new_passthrough ());
	return p;
}


/* return a (malloced) temporary name in cat_file's directory */
static char *tmp_cat_filename (const char *cat_file)
{
	char *name;

	if (debug_level) {
		name = xstrdup ("/dev/null");
		tmp_cat_fd = open (name, O_WRONLY);
	} else {
		char *slash;
		name = xstrdup (cat_file);
		slash = strrchr (name, '/');
		if (slash)
			*(slash + 1) = '\0';
		else
			*name = '\0';
		name = appendstr (name, "catXXXXXX", NULL);
		tmp_cat_fd = mkstemp (name);
	}

	if (tmp_cat_fd == -1) {
		free (name);
		return NULL;
	} else
		return name;
}


/* If delete unlink tmp_cat, else commit tmp_cat to cat_file.
   Return non-zero on error.
 */
static int commit_tmp_cat (const char *cat_file, const char *tmp_cat,
			   int delete)
{
	int status = 0;

#ifdef SECURE_MAN_UID
	if (!delete && global_manpath && euid == 0) {
		if (debug_level) {
			debug ("fixing temporary cat's ownership\n");
			status = 0;
		} else {
			struct passwd *man_owner = get_man_owner ();
			status = chown (tmp_cat, man_owner->pw_uid, -1);
			if (status)
				error (0, errno, _("can't chown %s"), tmp_cat);
		}
	}
#endif /* SECURE_MAN_UID */

	if (!delete && !status) {
		if (debug_level) {
			debug ("fixing temporary cat's mode\n");
			status = 0;
		} else {
			status = chmod (tmp_cat, CATMODE);
			if (status)
				error (0, errno, _("can't chmod %s"), tmp_cat);
		}
	}

	if (!delete && !status) {
		if (debug_level) {
			debug ("renaming temporary cat to %s\n", cat_file);
			status = 0;
		} else {
			status = rename (tmp_cat, cat_file);
			if (status)
				error (0, errno, _("can't rename %s to %s"),
				       tmp_cat, cat_file);
		}
	}

	if (!delete && !status) {
		if (debug_level) {
			debug ("setting modtime on cat file %s\n", cat_file);
			status = 0;
		} else {
			time_t now = time (NULL);
			struct utimbuf utb;
			utb.actime = now;
			if (man_modtime)
				utb.modtime = man_modtime;
			else
				utb.modtime = 0;
			status = utime (cat_file, &utb);
			if (status)
				error (0, errno, _("can't set times on %s"),
				       cat_file);
		}
	}

	if (delete || status) {
		if (debug_level)
			debug ("unlinking temporary cat\n");
		else if (unlink (tmp_cat))
			error (0, errno, _("can't unlink %s"), tmp_cat);
	}

	return status;
}

/* TODO: This should all be refactored after work on the decompression
 * library is complete.
 */
static void discard_stderr (pipeline *p)
{
	int i;

	for (i = 0; i < pipeline_get_ncommands (p); ++i)
		pipecmd_discard_err (pipeline_get_command (p, i), 1);
}

static void maybe_discard_stderr (pipeline *p)
{
	const char *man_keep_stderr = getenv ("MAN_KEEP_STDERR");
	if ((!man_keep_stderr || !*man_keep_stderr) && isatty (STDOUT_FILENO))
		discard_stderr (p);
}

#ifdef MAN_CATS

/* Return pipeline to write formatted manual page to for saving as cat file. */
static pipeline *open_cat_stream (const char *cat_file, const char *encoding)
{
	pipeline *cat_p;
#  ifdef COMP_CAT
	pipecmd *comp_cmd;
#  endif

	created_tmp_cat = 0;

	debug ("creating temporary cat for %s\n", cat_file);

	tmp_cat_file = tmp_cat_filename (cat_file);
	if (tmp_cat_file)
		created_tmp_cat = 1;
	else {
		if (!debug_level && (errno == EACCES || errno == EROFS)) {
			/* No permission to write to the cat file. Oh well,
			 * return NULL and let the caller sort it out.
			 */
			debug ("can't write to temporary cat for %s\n",
			       cat_file);
			return NULL;
		} else
			error (FATAL, errno,
			       _("can't create temporary cat for %s"),
			       cat_file);
	}

	if (!debug_level)
		push_cleanup ((cleanup_fun) unlink, tmp_cat_file, 1);

	cat_p = pipeline_new ();
	add_output_iconv (cat_p, encoding, "UTF-8");
#  ifdef COMP_CAT
	/* fork the compressor */
	comp_cmd = pipecmd_new_argstr (get_def ("compressor", COMPRESSOR));
	pipecmd_nice (comp_cmd, 10);
	pipeline_command (cat_p, comp_cmd);
#  endif
	/* pipeline_start will close tmp_cat_fd */
	pipeline_want_out (cat_p, tmp_cat_fd);

	return cat_p;
}

/* Close the cat page stream, return non-zero on error.
   If delete don't update the cat file.
 */
static int close_cat_stream (pipeline *cat_p, const char *cat_file,
			     int delete)
{
	int status;

	status = pipeline_wait (cat_p);
	debug ("cat-saver exited with status %d\n", status);

	pipeline_free (cat_p);

	if (created_tmp_cat) {
		status |= commit_tmp_cat (cat_file, tmp_cat_file,
					  delete || status);
		if (!debug_level)
			pop_cleanup ();
	}
	free (tmp_cat_file);
	return status;
}

/*
 * format a manual page with format_cmd, display it with disp_cmd, and
 * save it to cat_file
 */
static int format_display_and_save (pipeline *decomp,
				    pipeline *format_cmd,
				    pipeline *disp_cmd,
				    const char *cat_file, const char *encoding)
{
	pipeline *sav_p = open_cat_stream (cat_file, encoding);
	int instat;

	if (global_manpath)
		drop_effective_privs ();

	maybe_discard_stderr (format_cmd);

	pipeline_connect (decomp, format_cmd, NULL);
	if (sav_p) {
		pipeline_connect (format_cmd, disp_cmd, sav_p, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, sav_p, NULL);
	} else {
		pipeline_connect (format_cmd, disp_cmd, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, NULL);
	}

	if (global_manpath)
		regain_effective_privs ();

	pipeline_wait (decomp);
	instat = pipeline_wait (format_cmd);
	if (sav_p)
		close_cat_stream (sav_p, cat_file, instat);
	pipeline_wait (disp_cmd);
	return instat;
}
#endif /* MAN_CATS */

/* Format a manual page with format_cmd and display it with disp_cmd.
 * Handle temporary file creation if necessary.
 * TODO: merge with format_display_and_save
 */
static void format_display (pipeline *decomp,
			    pipeline *format_cmd, pipeline *disp_cmd,
			    const char *man_file)
{
	int status;
#ifdef TROFF_IS_GROFF
	char *old_cwd = NULL;
	char *htmldir = NULL, *htmlfile = NULL;
#endif /* TROFF_IS_GROFF */

	if (format_cmd)
		maybe_discard_stderr (format_cmd);

	drop_effective_privs ();

#ifdef TROFF_IS_GROFF
	if (format_cmd && htmlout) {
		char *man_base, *man_ext;
		int htmlfd;

		old_cwd = xgetcwd ();
		if (!old_cwd) {
			old_cwd = xmalloc (1);
			old_cwd[0] = '\0';
		}
		htmldir = create_tempdir ("hman");
		if (!htmldir)
			error (FATAL, errno,
			       _("can't create temporary directory"));
		if (chdir (htmldir) == -1)
			error (FATAL, errno, _("can't change to directory %s"),
			       htmldir);
		man_base = base_name (man_file);
		man_ext = strchr (man_base, '.');
		if (man_ext)
			*man_ext = '\0';
		htmlfile = xstrdup (htmldir);
		htmlfile = appendstr (htmlfile, "/", man_base, ".html", NULL);
		free (man_base);
		htmlfd = open (htmlfile, O_CREAT | O_EXCL | O_WRONLY, 0644);
		if (htmlfd == -1)
			error (FATAL, errno, _("can't open temporary file %s"),
			       htmlfile);
		pipeline_want_out (format_cmd, htmlfd);
		pipeline_connect (decomp, format_cmd, NULL);
		pipeline_pump (decomp, format_cmd, NULL);
		pipeline_wait (decomp);
		status = pipeline_wait (format_cmd);
	} else
#endif /* TROFF_IS_GROFF */
	    if (format_cmd) {
		pipeline_connect (decomp, format_cmd, NULL);
		pipeline_connect (format_cmd, disp_cmd, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, NULL);
		pipeline_wait (decomp);
		pipeline_wait (format_cmd);
		status = pipeline_wait (disp_cmd);
	} else {
		pipeline_connect (decomp, disp_cmd, NULL);
		pipeline_pump (decomp, disp_cmd, NULL);
		pipeline_wait (decomp);
		status = pipeline_wait (disp_cmd);
	}

#ifdef TROFF_IS_GROFF
	if (format_cmd && htmlout) {
		char *browser_list, *candidate;

		assert (old_cwd); /* initialised above */

		if (status) {
			if (chdir (old_cwd) == -1) {
				error (0, errno,
				       _("can't change to directory %s"),
				       old_cwd);
				chdir ("/");
			}
			if (remove_directory (htmldir, 0) == -1)
				error (0, errno,
				       _("can't remove directory %s"),
				       htmldir);
			free (htmlfile);
			free (htmldir);
			gripe_system (format_cmd, status);
		}

		browser_list = xstrdup (html_pager);
		for (candidate = strtok (browser_list, ":"); candidate;
		     candidate = strtok (NULL, ":")) {
			pipeline *browser;
			debug ("Trying browser: %s\n", candidate);
			browser = make_browser (candidate, htmlfile);
			status = do_system_drop_privs (browser);
			if (!status)
				break;
		}
		if (!candidate)
			error (CHILD_FAIL, 0,
			       "couldn't execute any browser from %s",
			       html_pager);
		free (browser_list);
		if (chdir (old_cwd) == -1) {
			error (0, errno, _("can't change to directory %s"),
			       old_cwd);
			chdir ("/");
		}
		if (remove_directory (htmldir, 0) == -1)
			error (0, errno, _("can't remove directory %s"),
			       htmldir);
		free (htmlfile);
		free (htmldir);
	} else
#endif /* TROFF_IS_GROFF */
	/* TODO: check format_cmd status too? */
	    if (status && status != (SIGPIPE + 0x80) * 256)
		gripe_system (disp_cmd, status);

	regain_effective_privs ();
}

/* "Display" a page in catman mode, which amounts to saving it. */
/* TODO: merge with format_display_and_save? */
static void display_catman (const char *cat_file, pipeline *decomp,
			    pipeline *format_cmd, const char *encoding)
{
	char *tmpcat = tmp_cat_filename (cat_file);
	int status;

	add_output_iconv (format_cmd, encoding, "UTF-8");

#ifdef COMP_CAT
	pipeline_command_argstr (format_cmd,
				 get_def ("compressor", COMPRESSOR));
#endif /* COMP_CAT */

	maybe_discard_stderr (format_cmd);
	pipeline_want_out (format_cmd, tmp_cat_fd);

	push_cleanup ((cleanup_fun) unlink, tmpcat, 1);

	/* save the cat as real user
	 * (1) required for user man hierarchy
	 * (2) else depending on ruid's privs is ok, effectively disables
	 *     catman for non-root.
	 */
	drop_effective_privs ();
	pipeline_connect (decomp, format_cmd, NULL);
	pipeline_pump (decomp, format_cmd, NULL);
	pipeline_wait (decomp);
	status = pipeline_wait (format_cmd);
	regain_effective_privs ();
	if (status)
		gripe_system (format_cmd, status);

	close (tmp_cat_fd);
	commit_tmp_cat (cat_file, tmpcat, status);
	pop_cleanup();
	free (tmpcat);
}

static void disable_hyphenation (void *data ATTRIBUTE_UNUSED)
{
	fputs (".nh\n"
	       ".de hy\n"
	       "..\n", stdout);
}

static void disable_justification (void *data ATTRIBUTE_UNUSED)
{
	fputs (".na\n"
	       ".de ad\n"
	       "..\n", stdout);
}

#ifdef TROFF_IS_GROFF
static void locale_macros (void *data)
{
	const char *macro_lang = data;
	const char *hyphen_lang = STREQ (lang, "en") ? "us" : macro_lang;

	debug ("Macro language %s; hyphenation language %s\n",
	       macro_lang, hyphen_lang);

	printf (
		/* If we're using groff >= 1.20.2 (for the 'file' warning
		 * category):
		 */
		".if (\\n[.g] & ((\\n[.x] > 1) :"
		" ((\\n[.x] == 1) & (\\n[.y] > 20)) :"
		" ((\\n[.x] == 1) & (\\n[.y] == 20) & (\\n[.Y] >= 2)))) "
		"\\{\\\n"
		/*   disable warnings of category 'file' */
		".  warn (\\n[.warn] -"
		" (\\n[.warn] / 1048576 %% 2 * 1048576))\n"
		/*   and load the appropriate per-locale macros */
		".  mso %s.tmac\n"
		".\\}\n"
		/* set the hyphenation language anyway, to make sure groff
		 * only hyphenates languages it knows about
		 */
		".hla %s\n", macro_lang, hyphen_lang);
}
#endif /* TROFF_IS_GROFF */

/*
 * optionally chdir to dir, if necessary update cat_file from man_file
 * and display it.  if man_file is NULL cat_file is a stray cat.  If
 * !save_cat or cat_file is NULL we must not save the formatted cat.
 * If man_file is "" this is a special case -- we expect the man page
 * on standard input.
 */
static int display (const char *dir, const char *man_file,
		    const char *cat_file, const char *title,
		    const char *dbfilters)
{
	int found;
	static int prompt;
	pipeline *format_cmd;	/* command to format man_file to stdout */
	char *formatted_encoding = NULL;
	int display_to_stdout;
	pipeline *decomp = NULL;
	int decomp_errno = 0;

	/* if dir is set chdir to it */
	if (dir) {
		debug ("chdir %s\n", dir);

		if (chdir (dir)) {
			error (0, errno, _("can't chdir to %s"), dir);
			return 0;
		}
	}

	/* define format_cmd */
	if (man_file) {
		pipecmd *seq = pipecmd_new_sequence ("decompressor", NULL);
		int seq_ncmds = 0;

		if (*man_file)
			decomp = decompress_open (man_file);
		else
			decomp = decompress_fdopen (dup (STDIN_FILENO));

		if (no_hyphenation) {
			pipecmd *hcmd = pipecmd_new_function (
				"echo .nh && echo .de hy && echo ..",
				disable_hyphenation, NULL, NULL);
			pipecmd_sequence_command (seq, hcmd);
			++seq_ncmds;
		}

		if (no_justification) {
			pipecmd *jcmd = pipecmd_new_function (
				"echo .na && echo .de ad && echo ..",
				disable_justification, NULL, NULL);
			pipecmd_sequence_command (seq, jcmd);
			++seq_ncmds;
		}

#ifdef TROFF_IS_GROFF
		/* This only works with preconv, since the per-locale macros
		 * may change the assumed input encoding.
		 */
		if (*man_file && get_groff_preconv ()) {
			char *page_lang = lang_dir (man_file);

			if (page_lang && *page_lang &&
			    !STREQ (page_lang, "C")) {
				struct locale_bits bits;
				char *name;
				pipecmd *lcmd;

				unpack_locale_bits (page_lang, &bits);
				name = xasprintf ("echo .mso %s.tmac",
						  bits.language);
				lcmd = pipecmd_new_function (
					name, locale_macros, free, page_lang);
				pipecmd_sequence_command (seq, lcmd);
				++seq_ncmds;
				free (name);
				free_locale_bits (&bits);
			}
		}
#endif /* TROFF_IS_GROFF */

		if (seq_ncmds) {
			assert (pipeline_get_ncommands (decomp) <= 1);
			if (pipeline_get_ncommands (decomp)) {
				pipecmd_sequence_command
					(seq,
					 pipeline_get_command (decomp, 0));
				pipeline_set_command (decomp, 0, seq);
			} else {
				pipecmd_sequence_command
					(seq, pipecmd_new_passthrough ());
				pipeline_command (decomp, seq);
			}
		} else
			pipecmd_free (seq);
	}

	if (decomp) {
		pipeline_start (decomp);
		format_cmd = make_roff_command (dir, man_file, decomp,
						dbfilters,
						&formatted_encoding);
		debug ("formatted_encoding = %s\n", formatted_encoding);
	} else {
		format_cmd = NULL;
		decomp_errno = errno;
	}

	/* Get modification time, for commit_tmp_cat(). */
	if (man_file && *man_file) {
		struct stat stb;
		if (stat (man_file, &stb))
			man_modtime = 0;
		else
			man_modtime = stb.st_mtime;
	}

	display_to_stdout = troff;
#ifdef TROFF_IS_GROFF
	if (htmlout)
		display_to_stdout = 0;
#endif
	if (recode)
		display_to_stdout = 1;

	if (display_to_stdout) {
		/* If we're reading stdin via '-l -', man_file is "". See
		 * below.
		 */
		assert (man_file);
		if (!decomp) {
			assert (!format_cmd); /* no need to free it */
			error (0, decomp_errno, _("can't open %s"), man_file);
			return 0;
		}
		if (*man_file == '\0')
			found = 1;
		else
			found = !access (man_file, R_OK);
		if (found) {
			int status;
			if (prompt && do_prompt (title)) {
				pipeline_free (format_cmd);
				pipeline_free (decomp);
				return 0;
			}
			drop_effective_privs ();
			pipeline_connect (decomp, format_cmd, NULL);
			pipeline_pump (decomp, format_cmd, NULL);
			pipeline_wait (decomp);
			status = pipeline_wait (format_cmd);
			regain_effective_privs ();
			if (status != 0)
				gripe_system (format_cmd, status);
		}
	} else {
		int format = 1;
		int status;

		/* The caller should already have checked for any
		 * FSSTND-style (same hierarchy) cat page that may be
		 * present, and we don't expect to have to update the cat
		 * page in that case. If by some chance we do have to update
		 * it, then there's no harm trying; open_cat_stream() will
		 * refuse gracefully if the file isn't writeable.
		 */

		/* In theory we might be able to get away with saving cats
		 * for want_encoding, but it does change the roff device so
		 * perhaps that's best avoided.
		 */
		if (want_encoding
#ifdef TROFF_IS_GROFF
		    || htmlout
#endif
		    || local_man_file
		    || recode
		    || disable_cache)
			save_cat = 0;

		if (!man_file) {
			/* Stray cat. */
			assert (cat_file);
			format = 0;
		} else if (!cat_file) {
			assert (man_file);
			save_cat = 0;
			format = 1;
		} else if (format && save_cat) {
			char *cat_dir;
			char *tmp;

			status = is_changed (man_file, cat_file);
			format = (status == -2) || ((status & 1) == 1);

			/* don't save if we haven't a cat directory */
			cat_dir = xstrdup (cat_file);
			tmp = strrchr (cat_dir, '/');
			if (tmp)
				*tmp = 0;
			save_cat = is_directory (cat_dir) == 1;
			if (!save_cat)
				debug ("cat dir %s does not exist\n", cat_dir);
			free (cat_dir);
		}

		if (format && (!format_cmd || !decomp)) {
			assert (man_file);
			/* format_cmd is NULL iff decomp is NULL; no need to
			 * free either of them.
			 */
			assert (!format_cmd);
			assert (!decomp);
			error (0, decomp_errno, _("can't open %s"), man_file);
			return 0;
		}

		/* if we're trying to read stdin via '-l -' then man_file
		 * will be "" which access() obviously barfs on, but all is
		 * well because the format_cmd will have been created to
		 * expect input via stdin. So we special-case this to avoid
		 * the bogus access() check.
		*/
		if (format == 1 && *man_file == '\0')
			found = 1;
		else
			found = !access (format ? man_file : cat_file, R_OK);

		debug ("format: %d, save_cat: %d, found: %d\n",
		       format, save_cat, found);

		if (!found) {
			pipeline_free (format_cmd);
			pipeline_free (decomp);
			return found;
		}

		if (print_where || print_where_cat) {
			int printed = 0;
			if (print_where && man_file) {
				printf ("%s", man_file);
				printed = 1;
			}
			if (print_where_cat && cat_file && !format) {
				if (printed)
					putchar (' ');
				printf ("%s", cat_file);
				printed = 1;
			}
			if (printed)
				putchar ('\n');
		} else if (catman) {
			if (format) {
				if (!save_cat)
					error (0, 0,
					       _("\ncannot write to "
						 "%s in catman mode"),
					       cat_file);
				else
					display_catman (cat_file, decomp,
							format_cmd,
							formatted_encoding);
			}
		} else if (format) {
			/* no cat or out of date */
			pipeline *disp_cmd;

			if (prompt && do_prompt (title)) {
				pipeline_free (format_cmd);
				pipeline_free (decomp);
				if (local_man_file)
					return 1;
				else
					return 0;
			}

			disp_cmd = make_display_command (formatted_encoding,
							 title);

#ifdef MAN_CATS
			if (save_cat) {
				/* save cat */
				assert (disp_cmd); /* not htmlout for now */
				format_display_and_save (decomp,
							 format_cmd,
							 disp_cmd,
							 cat_file,
							 formatted_encoding);
			} else 
#endif /* MAN_CATS */
				/* don't save cat */
				format_display (decomp, format_cmd, disp_cmd,
						man_file);

			pipeline_free (disp_cmd);

		} else {
			/* display preformatted cat */
			pipeline *disp_cmd;
			pipeline *decomp_cat;

			if (prompt && do_prompt (title)) {
				pipeline_free (format_cmd);
				pipeline_free (decomp);
				return 0;
			}

			decomp_cat = decompress_open (cat_file);
			if (!decomp_cat) {
				error (0, errno, _("can't open %s"), cat_file);
				pipeline_free (format_cmd);
				pipeline_free (decomp);
				return 0;
			}
			disp_cmd = make_display_command ("UTF-8", title);
			format_display (decomp_cat, NULL, disp_cmd, man_file);
			pipeline_free (disp_cmd);
			pipeline_free (decomp_cat);
		}
	}

	pipeline_free (format_cmd);
	pipeline_free (decomp);

	if (!prompt)
		prompt = found;

	return found;
}


static char *find_cat_file (const char *path, const char *original,
			    const char *man_file)
{
	size_t path_len = strlen (path);
	char *cat_file, *cat_path;
	int status;

	/* Try the FSSTND way first, namely a cat page in the same hierarchy
	 * as the original path to the man page. We don't create these
	 * unless no alternate cat hierarchy is available, but will use them
	 * if they happen to exist already and have the same timestamp as
	 * the corresponding man page. (In practice I'm betting that this
	 * means we'll hardly ever use them at all except for user
	 * hierarchies; but compatibility, eh?)
	 */
	cat_file = convert_name (original, 1);
	if (cat_file) {
		status = is_changed (original, cat_file);
		if (status != -2 && !(status & 1) == 1) {
			debug ("found valid FSSTND cat file %s\n", cat_file);
			return cat_file;
		}
		free (cat_file);
	}

	/* Otherwise, find the cat page we actually want to use or create,
	 * taking any alternate cat hierarchy into account. If the original
	 * path and man_file differ (i.e. original was a symlink or .so
	 * link), try the link target and then the source.
	 */
	if (!STREQ (man_file, original)) {
		global_manpath = is_global_mandir (man_file);
		cat_path = get_catpath
			(man_file, global_manpath ? SYSTEM_CAT : USER_CAT);

		if (cat_path) {
			cat_file = convert_name (cat_path, 0);
			free (cat_path);
		} else if (STRNEQ (man_file, path, path_len) &&
			   man_file[path_len] == '/')
			cat_file = convert_name (man_file, 1);
		else
			cat_file = NULL;

		if (cat_file) {
			char *cat_dir = xstrdup (cat_file);
			char *tmp = strrchr (cat_dir, '/');
			if (tmp)
				*tmp = 0;
			if (is_directory (cat_dir)) {
				debug ("will try cat file %s\n", cat_file);
				return cat_file;
			} else
				debug ("cat dir %s does not exist\n", cat_dir);
			free (cat_dir);
		} else
			debug ("no cat path for %s\n", man_file);
	}

	global_manpath = is_global_mandir (original);
	cat_path = get_catpath
		(original, global_manpath ? SYSTEM_CAT : USER_CAT);

	if (cat_path) {
		cat_file = convert_name (cat_path, 0);
		free (cat_path);
	} else
		cat_file = convert_name (original, 1);

	if (cat_file)
		debug ("will try cat file %s\n", cat_file);
	else
		debug ("no cat path for %s\n", original);

	return cat_file;
}

static int get_ult_flags (char from_db, char id)
{
	if (!from_db)
		return ult_flags;
	else if (id == ULT_MAN)
		/* Checking .so links is expensive, as we have to open the
		 * file. Therefore, if the database lists it as ULT_MAN,
		 * that's good enough for us and we won't recheck that. This
		 * does mean that if a page changes from ULT_MAN to SO_MAN
		 * then you might get duplicates until mandb is next run,
		 * but that isn't a big deal.
		 */
		return ult_flags & ~SO_LINK;
	else
		return ult_flags;
}

/* Is this candidate substantially a duplicate of a previous one?
 * Returns non-zero if so, otherwise zero.
 */
static int duplicate_candidates (struct candidate *left,
				 struct candidate *right)
{
	const char *slash1, *slash2;
	struct locale_bits bits1, bits2;
	int ret;

	if (left->ult && right->ult && STREQ (left->ult, right->ult))
		return 1; /* same ultimate source file */

	if (!STREQ (left->source->name, right->source->name) ||
	    !STREQ (left->source->sec, right->source->sec) ||
	    !STREQ (left->source->ext, right->source->ext))
		return 0; /* different name/section/extension */

	if (STREQ (left->path, right->path))
		return 1; /* same path */

	/* Figure out if we've had a sufficiently similar candidate for this
	 * language already.
	 */
	slash1 = strrchr (left->path, '/');
	slash2 = strrchr (right->path, '/');
	if (!slash1 || !slash2 ||
	    !STRNEQ (left->path, right->path,
		     MAX (slash1 - left->path, slash2 - right->path)))
		return 0; /* different path base */

	unpack_locale_bits (++slash1, &bits1);
	unpack_locale_bits (++slash2, &bits2);

	if (!STREQ (bits1.language, bits2.language) ||
	    !STREQ (bits1.territory, bits2.territory) ||
	    !STREQ (bits1.modifier, bits2.modifier))
		ret = 0; /* different language/territory/modifier */
	else
		/* Everything seems to be the same; we can find nothing to
		 * choose between them.
		 */
		ret = 1;

	free_locale_bits (&bits1);
	free_locale_bits (&bits2);
	return ret;
}

static int compare_candidates (const struct candidate *left,
			       const struct candidate *right)
{
	const struct mandata *lsource = left->source, *rsource = right->source;
	int sec_left = 0, sec_right = 0;
	int cmp;
	const char *slash1, *slash2;

	/* If one candidate matches the requested name exactly, sort it
	 * first. This makes --ignore-case behave more sensibly.
	 */
	/* name is never NULL here, see add_candidate() */
	if (STREQ (lsource->name, left->req_name)) {
		if (!STREQ (rsource->name, right->req_name))
			return -1;
	} else {
		if (STREQ (rsource->name, right->req_name))
			return 1;
	}

	/* Compare pure sections first, then ids, then extensions.
	 * Rationale: whatis refs get the same section and extension as
	 * their source, but may be supplanted by a real page with a
	 * slightly different extension, possibly in another hierarchy (!);
	 * see Debian bug #204249 for the gory details.
	 *
	 * Any extension spelt out in full in section_list effectively
	 * becomes a pure section; this allows extensions to be selectively
	 * moved out of order with respect to their parent sections.
	 */
	if (strcmp (lsource->ext, rsource->ext)) {
		const char **sp;

		/* If the user asked for an explicit section, sort exact
		 * matches first.
		 */
		if (section) {
			if (STREQ (lsource->ext, section)) {
				if (!STREQ (rsource->ext, section))
					return -1;
			} else {
				if (STREQ (rsource->ext, section))
					return 1;
			}
		}

		/* Find out whether lsource->ext is ahead of rsource->ext in
		 * section_list.
		 */
		for (sp = section_list; *sp; ++sp) {
			if (!*(*sp + 1)) {
				/* No extension */
				if (!sec_left  && **sp == *(lsource->ext))
					sec_left  = sp - section_list + 1;
				if (!sec_right && **sp == *(rsource->ext))
					sec_right = sp - section_list + 1;
			} else if (STREQ (*sp, lsource->ext)) {
				sec_left  = sp - section_list + 1;
			} else if (STREQ (*sp, rsource->ext)) {
				sec_right = sp - section_list + 1;
			}
			/* Keep looking for a more specific match */
		}
		if (sec_left != sec_right)
			return sec_left - sec_right;

		cmp = strcmp (lsource->sec, rsource->sec);
		if (cmp)
			return cmp;
	}

	/* ULT_MAN comes first, etc. Consider SO_MAN equivalent to ULT_MAN. */
	cmp = compare_ids (lsource->id, rsource->id, 1);
	if (cmp)
		return cmp;

	/* The order in section_list has already been compared above. For
	 * everything not mentioned explicitly there, we just compare
	 * lexically.
	 */
	cmp = strcmp (lsource->ext, rsource->ext);
	if (cmp)
		return cmp;

	/* Try comparing based on language. We used to prefer to display a
	 * page in the user's preferred language than a page from a better
	 * section, but that attracted objections, so now we prefer to get
	 * the section right. See Debian bug #519547.
	 */
	slash1 = strrchr (left->path, '/');
	slash2 = strrchr (right->path, '/');
	if (slash1 && slash2) {
		char *locale_copy, *p;
		struct locale_bits bits1, bits2, lbits;
		const char *codeset1, *codeset2;

		unpack_locale_bits (++slash1, &bits1);
		unpack_locale_bits (++slash2, &bits2);

		/* We need the current locale as well. */
		locale_copy = xstrdup (internal_locale);
		p = strchr (locale_copy, ':');
		if (p)
			*p = '\0';
		unpack_locale_bits (locale_copy, &lbits);
		free (locale_copy);

#define COMPARE_LOCALE_ELEMENTS(elt) do { \
	/* For different elements, prefer one that matches the locale if
	 * possible.
	 */ \
	if (*lbits.elt) { \
		if (STREQ (lbits.elt, bits1.elt)) { \
			if (!STREQ (lbits.elt, bits2.elt)) { \
				cmp = -1; \
				goto out; \
			} \
		} else { \
			if (STREQ (lbits.elt, bits2.elt)) { \
				cmp = 1; \
				goto out; \
			} \
		} \
	} \
	cmp = strcmp (bits1.territory, bits2.territory); \
	if (cmp) \
		/* No help from locale; might as well sort lexically. */ \
		goto out; \
} while (0)

		COMPARE_LOCALE_ELEMENTS (language);
		COMPARE_LOCALE_ELEMENTS (territory);
		COMPARE_LOCALE_ELEMENTS (modifier);

#undef COMPARE_LOCALE_ELEMENTS

		/* Prefer UTF-8 if available. Otherwise, consider them
		 * equal.
		 */
		codeset1 = get_canonical_charset_name (bits1.codeset);
		codeset2 = get_canonical_charset_name (bits2.codeset);
		if (STREQ (codeset1, "UTF-8")) {
			if (!STREQ (codeset2, "UTF-8")) {
				cmp = -1;
				goto out;
			}
		} else {
			if (STREQ (codeset2, "UTF-8")) {
				cmp = 1;
				goto out;
			}
		}

out:
		free_locale_bits (&lbits);
		free_locale_bits (&bits1);
		free_locale_bits (&bits2);
		if (cmp)
			return cmp;
	}

	/* Explicitly stabilise the sort as a last resort, so that manpath
	 * ordering (e.g. language-specific hierarchies) works.
	 */
	if (left->add_index < right->add_index)
		return -1;
	else if (left->add_index > right->add_index)
		return 1;
	else
		return 0;

	return 0;
}

static int compare_candidates_qsort (const void *l, const void *r)
{
	const struct candidate *left = *(const struct candidate **)l;
	const struct candidate *right = *(const struct candidate **)r;

	return compare_candidates (left, right);
}

static void free_candidate (struct candidate *candidate)
{
	if (candidate)
		free (candidate->ult);
	free (candidate);
}

/* Add an entry to the list of candidates. */
static int add_candidate (struct candidate **head, char from_db, char cat,
			  const char *req_name, const char *path,
			  const char *ult, struct mandata *source)
{
	struct candidate *search, *prev, *insert, *candp;
	static int add_index = 0;

	if (!ult) {
		const char *name;
		char *filename;

		if (*source->pointer != '-')
			name = source->pointer;
		else if (source->name)
			name = source->name;
		else
			name = req_name;

		filename = make_filename (path, name, source, cat ? "cat" : "man");
		ult = ult_src (filename, path, NULL,
			       get_ult_flags (from_db, source->id), NULL);
		free (filename);
	}

	debug ("candidate: %d %d %s %s %s %c %s %s %s\n",
	       from_db, cat, req_name, path, ult,
	       source->id, source->name ? source->name : "-",
	       source->sec, source->ext);

	if (!source->name)
		source->name = xstrdup (req_name);

	candp = XMALLOC (struct candidate);
	candp->req_name = req_name;
	candp->from_db = from_db;
	candp->cat = cat;
	candp->path = path;
	candp->ult = ult ? xstrdup (ult) : NULL;
	candp->source = source;
	candp->add_index = add_index++;
	candp->next = NULL;

	/* insert will be NULL (insert at start) or a pointer to the element
	 * after which this element should be inserted.
	 */
	insert = NULL;
	search = *head;
	prev = NULL;
	/* This search produces quadratic-time behaviour, although in
	 * practice it doesn't seem to be too bad at the moment since the
	 * run-time is dominated by calls to ult_src. In future it might be
	 * worth optimising this; the reason I haven't done this yet is that
	 * it involves quite a bit of tedious bookkeeping. A practical
	 * approach would be to keep two hashes, one that's just a set to
	 * keep track of whether candp->ult has been seen already, and one
	 * that keeps a list of candidates for each candp->name that could
	 * then be quickly checked by brute force.
	 */
	while (search) {
		int dupcand = duplicate_candidates (candp, search);

		debug ("search: %d %d %s %s %s %c %s %s %s "
		       "(dup: %d)\n",
		       search->from_db, search->cat, search->req_name,
		       search->path, search->ult, search->source->id,
		       search->source->name ? search->source->name : "-",
		       search->source->sec, search->source->ext, dupcand);

		/* Check for duplicates. */
		if (dupcand) {
			int cmp = compare_candidates (candp, search);

			if (cmp >= 0) {
				debug ("other duplicate is at least as "
				       "good\n");
				free_candidate (candp);
				return 0;
			} else {
				debug ("this duplicate is better; removing "
				       "old one\n");
				if (prev) {
					prev->next = search->next;
					free_candidate (search);
					search = prev->next;
				} else {
					*head = search->next;
					free_candidate (search);
					search = *head;
				}
				continue;
			}
		}

		prev = search;
		if (search->next)
			search = search->next;
		else
			break;
	}
	/* Insert the new candidate at the end of the list (having had to go
	 * through them all looking for duplicates anyway); we'll sort it
	 * into place later.
	 */
	insert = prev;

	candp->next = insert ? insert->next : *head;
	if (insert)
		insert->next = candp;
	else
		*head = candp;

	return 1;
}

/* Sort the entire list of candidates. */
static void sort_candidates (struct candidate **candidates)
{
	struct candidate *cand, **allcands;
	size_t count = 0, i;

	for (cand = *candidates; cand; cand = cand->next)
		++count;

	if (count == 0)
		return;

	allcands = XNMALLOC (count, struct candidate *);
	i = 0;
	for (cand = *candidates; cand; cand = cand->next) {
		assert (i < count);
		allcands[i++] = cand;
	}
	assert (i == count);

	qsort (allcands, count, sizeof *allcands, compare_candidates_qsort);

	*candidates = cand = allcands[0];
	for (i = 1; i < count; ++i) {
		cand->next = allcands[i];
		cand = cand->next;
	}
	cand->next = NULL;

	free (allcands);
}

/*
 * See if the preformatted man page or the source exists in the given
 * section.
 */
static int try_section (const char *path, const char *sec, const char *name,
			struct candidate **cand_head)
{
	int found = 0;
	char **names = NULL, **np;
	char cat = 0;
	int lff_opts = (match_case ? LFF_MATCHCASE : 0) |
		       (regex_opt ? LFF_REGEX : 0) |
		       (wildcard ? LFF_WILDCARD : 0);

	debug ("trying section %s with globbing\n", sec);

#ifndef NROFF_MISSING /* #ifdef NROFF */
	/*
  	 * Look for man page source files.
  	 */

	names = look_for_file (path, sec, name, 0, lff_opts);
	if (!names)
		/*
    		 * No files match.  
    		 * See if there's a preformatted page around that
    		 * we can display.
    		 */
#endif /* NROFF_MISSING */
	{
		if (catman)
			return 1;

		if (!troff && !want_encoding && !recode) {
			names = look_for_file (path, sec, name, 1, lff_opts);
			cat = 1;
		}
	}

	for (np = names; np && *np; np++) {
		struct mandata *info = infoalloc ();
		char *info_buffer = filename_info (*np, info, name);
		const char *ult;
		if (!info_buffer) {
			free_mandata_struct (info);
			continue;
		}
		info->addr = info_buffer;

		/* What kind of page is this? Since it's a real file, it
		 * must be either ULT_MAN or SO_MAN. ult_src() can tell us
		 * which.
		 */
		ult = ult_src (*np, path, NULL, ult_flags, NULL);
		if (!ult) {
			/* already warned */
			debug ("try_section(): bad link %s\n", *np);
			free (info_buffer);
			info->addr = NULL;
			free_mandata_struct (info);
			continue;
		}
		if (STREQ (ult, *np))
			info->id = ULT_MAN;
		else
			info->id = SO_MAN;

		found += add_candidate (cand_head, CANDIDATE_FILESYSTEM,
					cat, name, path, ult, info);
		/* Don't free info and info_buffer here. */
	}

	return found;
}

static int display_filesystem (struct candidate *candp)
{
	char *filename = make_filename (candp->path, NULL, candp->source,
					candp->cat ? "cat" : "man");
	/* source->name is never NULL thanks to add_candidate() */
	char *title = appendstr (NULL, candp->source->name,
				 "(", candp->source->ext, ")", NULL);
	if (candp->cat) {
		if (troff || want_encoding || recode)
			return 0;
		return display (candp->path, NULL, filename, title, NULL);
	} else {
		const char *man_file;
		char *cat_file;
		int found;

		man_file = ult_src (filename, candp->path, NULL, ult_flags,
				    NULL);
		if (man_file == NULL) {
			free (title);
			return 0;
		}

		debug ("found ultimate source file %s\n", man_file);
		lang = lang_dir (man_file);

		cat_file = find_cat_file (candp->path, filename, man_file);
		found = display (candp->path, man_file, cat_file, title, NULL);
		if (cat_file)
			free (cat_file);
		free (lang);
		lang = NULL;
		free (title);

		return found;
	}
}

#ifdef MAN_DB_UPDATES
/* wrapper to dbdelete which deals with opening/closing the db */
static void dbdelete_wrapper (const char *page, struct mandata *info)
{
	if (!catman) {
		dbf = MYDBM_RWOPEN (database);
		if (dbf) {
			if (dbdelete (page, info) == 1)
				debug ("%s(%s) not in db!\n", page, info->ext);
			MYDBM_CLOSE(dbf);
		}
	}
}
#endif /* MAN_DB_UPDATES */

/* This started out life as try_section, but a lot of that routine is 
   redundant wrt the db cache. */
static int display_database (struct candidate *candp)
{
	int found = 0;
	char *file;
	const char *name;
	char *title;
	struct mandata *in = candp->source;

	debug ("trying a db located file.\n");
	dbprintf (in);

	/* if the pointer holds some data, this is a reference to the 
	   real page, use that instead. */
	if (*in->pointer != '-')
		name = in->pointer;
	else if (in->name)
		name = in->name;
	else
		name = candp->req_name;

	if (in->id == WHATIS_MAN || in->id == WHATIS_CAT)
		debug (_("%s: relying on whatis refs is deprecated\n"), name);

	title = appendstr (NULL, name, "(", in->ext, ")", NULL);

#ifndef NROFF_MISSING /* #ifdef NROFF */
	/*
  	 * Look for man page source files.
  	 */

	if (in->id < STRAY_CAT) {	/* There should be a src page */
		file = make_filename (candp->path, name, in, "man");
		debug ("Checking physical location: %s\n", file);

		if (access (file, R_OK) == 0) {
			const char *man_file;
			char *cat_file;

			man_file = ult_src (file, candp->path, NULL,
					    get_ult_flags (1, in->id), NULL);
			if (man_file == NULL) {
				free (title);
				return found; /* zero */
			}

			debug ("found ultimate source file %s\n", man_file);
			lang = lang_dir (man_file);

			cat_file = find_cat_file (candp->path, file, man_file);
			found += display (candp->path, man_file, cat_file,
					  title, in->filter);
			if (cat_file)
				free (cat_file);
			free (lang);
			lang = NULL;
		} /* else {drop through to the bottom and return 0 anyway} */
	} else 

#endif /* NROFF_MISSING */

	if (in->id <= WHATIS_CAT) {
		/* The db says we have a stray cat or whatis ref */

		if (catman) {
			free (title);
			return ++found;
		}

		/* show this page but force an update later to make sure
		   we haven't just added the new page */
		found_a_stray = 1;

		/* If explicitly asked for troff or a different encoding,
		 * don't show a stray cat.
		 */
		if (troff || want_encoding || recode) {
			free (title);
			return found;
		}

		file = make_filename (candp->path, name, in, "cat");
		debug ("Checking physical location: %s\n", file);

		if (access (file, R_OK) != 0) {
			char *catpath;
			catpath = get_catpath (candp->path,
					       global_manpath ? SYSTEM_CAT
							      : USER_CAT);

			if (catpath && strcmp (catpath, candp->path) != 0) {
				file = make_filename (catpath, name,
						      in, "cat");
				free (catpath);
				debug ("Checking physical location: %s\n",
				       file);

				if (access (file, R_OK) != 0) {
					/* don't delete here, 
					   return==0 will do that */
					free (title);
					return found; /* zero */
				}
			} else {
				if (catpath)
					free (catpath);
				free (title);
				return found; /* zero */
			}
		}

		found += display (candp->path, NULL, file, title, in->filter);
	}
	free (title);
	return found;
}

/* test for existence, if fail: call dbdelete_wrapper, else return amount */
static int display_database_check (struct candidate *candp)
{
	int exists = display_database (candp);

#ifdef MAN_DB_UPDATES
	if (!exists && !skip) {
		debug ("dbdelete_wrapper (%s, %p)\n",
		       candp->req_name, candp->source);
		dbdelete_wrapper (candp->req_name, candp->source);
	}
#endif /* MAN_DB_UPDATES */

	return exists;
}

static void db_hashtable_free (void *defn)
{
	free_mandata_struct (defn);
}

#ifdef MAN_DB_UPDATES
static int maybe_update_file (const char *manpath, const char *name,
			      struct mandata *info)
{
	const char *real_name;
	char *file;
	struct stat buf;
	int status;

	if (!update)
		return 0;

	/* If the pointer holds some data, then we need to look at that
	 * name in the filesystem instead.
	 */
	if (!STRNEQ (info->pointer, "-", 1))
		real_name = info->pointer;
	else if (info->name)
		real_name = info->name;
	else
		real_name = name;

	file = make_filename (manpath, real_name, info, "man");
	if (lstat (file, &buf) != 0)
		return 0;
	if (buf.st_mtime == info->_st_mtime)
		return 0;

	debug ("%s needs to be recached: %ld %ld\n",
	       file, (long) info->_st_mtime, (long) buf.st_mtime);
	status = run_mandb (0, manpath, file);
	if (status)
		error (0, 0, _("mandb command failed with exit status %d"),
		       status);

	return 1;
}
#endif /* MAN_DB_UPDATES */

/* Special return values from try_db(). */

#define TRY_DATABASE_OPEN_FAILED  -1

#ifdef MAN_DB_CREATES
#define TRY_DATABASE_CREATED      -2
#endif /* MAN_DB_CREATES */

#ifdef MAN_DB_UPDATES
#define TRY_DATABASE_UPDATED      -3
#endif /* MAN_DB_UPDATES */

/* Look for a page in the database. If db not accessible, return -1,
   otherwise return number of pages found. */
static int try_db (const char *manpath, const char *sec, const char *name,
		   struct candidate **cand_head)
{
	struct mandata *loc, *data;
	char *catpath;
	int found = 0;
#ifdef MAN_DB_UPDATES
	int found_stale = 0;
#endif /* MAN_DB_UPDATES */

	/* find out where our db for this manpath should be */

	catpath = get_catpath (manpath, global_manpath ? SYSTEM_CAT : USER_CAT);
	if (catpath) {
		database = mkdbname (catpath);
		free (catpath);
	} else
		database = mkdbname (manpath);

	if (!db_hash)
		db_hash = hashtable_create (&db_hashtable_free);

	/* Have we looked here already? */
	data = hashtable_lookup (db_hash, manpath, strlen (manpath));

	if (!data) {
		dbf = MYDBM_RDOPEN (database);
		if (dbf && dbver_rd (dbf)) {
			MYDBM_CLOSE (dbf);
			dbf = NULL;
		}
		if (dbf) {
			debug ("Succeeded in opening %s O_RDONLY\n", database);

			/* if section is set, only return those that match,
			   otherwise NULL retrieves all available */
			if (regex_opt || wildcard)
				data = dblookup_pattern
					(name, section, match_case,
					 regex_opt, !names_only);
			else
				data = dblookup_all (name, section,
						     match_case);
			hashtable_install (db_hash, manpath, strlen (manpath),
					   data);
			MYDBM_CLOSE (dbf);
#ifdef MAN_DB_CREATES
		} else if (!global_manpath) {
			/* create one */
			debug ("Failed to open %s O_RDONLY\n", database);
			if (run_mandb (1, manpath, NULL)) {
				data = infoalloc ();
				data->next = NULL;
				data->addr = NULL;
				hashtable_install (db_hash,
						   manpath, strlen (manpath),
						   data);
				return TRY_DATABASE_OPEN_FAILED;
			}
			return TRY_DATABASE_CREATED;
#endif /* MAN_DB_CREATES */
		} else {
			debug ("Failed to open %s O_RDONLY\n", database);
			data = infoalloc ();
			data->next = (struct mandata *) NULL;
			data->addr = NULL;
			hashtable_install (db_hash, manpath, strlen (manpath),
					   data);
			return TRY_DATABASE_OPEN_FAILED;
		}
	}

	/* if we already know that there is nothing here, get on with it */
	if (!data)
		return 0;

	/* We already tried (and failed) to open this db before */
	if (!data->addr)
		return TRY_DATABASE_OPEN_FAILED;

#ifdef MAN_DB_UPDATES
	/* Check that all the entries found are up to date. If not, the
	 * caller should try again.
	 */
	for (loc = data; loc; loc = loc->next)
		if (STREQ (sec, loc->sec) &&
		    (!extension || STREQ (extension, loc->ext)
				|| STREQ (extension, loc->ext + strlen (sec))))
			if (maybe_update_file (manpath, name, loc))
				found_stale = 1;

	if (found_stale) {
		hashtable_remove (db_hash, manpath, strlen (manpath));
		return TRY_DATABASE_UPDATED;
	}
#endif /* MAN_DB_UPDATES */

	/* cycle through the mandata structures (there's usually only 
	   1 or 2) and see what we have w.r.t. the current section */
	for (loc = data; loc; loc = loc->next)
		if (STREQ (sec, loc->sec) &&
		    (!extension || STREQ (extension, loc->ext)
				|| STREQ (extension, loc->ext + strlen (sec))))
			found += add_candidate (cand_head, CANDIDATE_DATABASE,
						0, name, manpath, NULL, loc);

	return found;
}

/* Try to locate the page under the specified manpath, in the desired section,
 * with the supplied name. Glob if necessary. Initially search the filesystem;
 * if that fails, try finding it via a db cache access. */
static int locate_page (const char *manpath, const char *sec, const char *name,
			struct candidate **candidates)
{
	int found, db_ok;

	/* sort out whether we want to treat this hierarchy as 
	   global or user. Differences:

	   global: if setuid, use privs; don't create db.
	   user  : if setuid, drop privs; allow db creation. */

	global_manpath = is_global_mandir (manpath);
	if (!global_manpath)
		drop_effective_privs ();

	debug ("searching in %s, section %s\n", manpath, sec);

	found = try_section (manpath, sec, name, candidates);

	if ((!found || findall) && !global_apropos) {
		db_ok = try_db (manpath, sec, name, candidates);

#ifdef MAN_DB_CREATES
		if (db_ok == TRY_DATABASE_CREATED)
			/* we created a db in the last call */
			db_ok = try_db (manpath, sec, name, candidates);
#endif /* MAN_DB_CREATES */

#ifdef MAN_DB_UPDATES
		if (db_ok == TRY_DATABASE_UPDATED)
			/* We found some outdated entries and rebuilt the
			 * database in the last call. If this keeps
			 * happening, though, give up and punt to the
			 * filesystem.
			 */
			db_ok = try_db (manpath, sec, name, candidates);
#endif /* MAN_DB_UPDATES */

		if (db_ok > 0)  /* we found/opened a db and found something */
			found += db_ok;
	}

	if (!global_manpath)
		regain_effective_privs ();

	return found;
}

static int display_pages (struct candidate *candidates)
{
	struct candidate *candp;
	int found = 0;

	for (candp = candidates; candp; candp = candp->next) {
		global_manpath = is_global_mandir (candp->path);
		if (!global_manpath)
			drop_effective_privs ();

		switch (candp->from_db) {
			case CANDIDATE_FILESYSTEM:
				found += display_filesystem (candp);
				break;
			case CANDIDATE_DATABASE:
				found += display_database_check (candp);
				break;
			default:
				error (0, 0,
				       _("internal error: candidate type %d "
					 "out of range"), candp->from_db);
		}

		if (!global_manpath)
			regain_effective_privs ();

		if (found && !findall)
			return found;
	}

	return found;
}

/*
 * Search for text in all manual pages.
 *
 * This is not a real full-text search, but a brute-force on-demand search.
 * The idea, name, and approach originate in the 'man' package, added (I
 * believe) by Andries Brouwer, although the implementation is new for
 * man-db and much faster due to running in-process.
 *
 * Conceptually, this really belongs in whatis.c, as part of apropos.
 * However, the implementation in 'man' offers pages for immediate display
 * on request rather than simply listing them, which is currently awkward to
 * do in apropos. If we ever add support to apropos/whatis for either
 * calling back to man or displaying pages directly, we should revisit this.
 */
static int grep (const char *file, const char *string, const regex_t *search)
{
	struct stat st;
	pipeline *decomp;
	const char *line;
	int ret = 0;

	/* pipeline_start makes file open failures unconditionally fatal.
	 * Here, we'd rather just ignore any such files.
	 */
	if (stat (file, &st) < 0)
		return 0;

	decomp = decompress_open (file);
	if (!decomp)
		return 0;
	pipeline_start (decomp);
	while ((line = pipeline_readline (decomp)) != NULL) {
		if (regex_opt) {
			if (regexec (search, line,
				     0, (regmatch_t *) 0, 0) == 0) {
				ret = 1;
				break;
			}
		} else {
			if (match_case ?
			    strstr (line, string) :
			    strcasestr (line, string)) {
				ret = 1;
				break;
			}
		}
	}

	pipeline_free (decomp);
	return ret;
}

static int do_global_apropos_section (const char *path, const char *sec,
				      const char *name)
{
	int found = 0;
	char **names, **np;
	regex_t search;

	global_manpath = is_global_mandir (path);
	if (!global_manpath)
		drop_effective_privs ();

	debug ("searching in %s, section %s\n", path, sec);

	names = look_for_file (path, sec, "*", 0, LFF_WILDCARD);
	if (regex_opt)
		xregcomp (&search, name,
			  REG_EXTENDED | REG_NOSUB |
			  (match_case ? 0 : REG_ICASE));
	else
		memset (&search, 0, sizeof search);

	for (np = names; np && *np; ++np) {
		struct mandata *info;
		char *info_buffer;
		char *title = NULL;
		const char *man_file;
		char *cat_file = NULL;

		if (!grep (*np, name, &search))
			continue;

		info = infoalloc ();
		info_buffer = filename_info (*np, info, NULL);
		if (!info_buffer)
			goto next;
		info->addr = info_buffer;

		title = appendstr (NULL, strchr (info_buffer, '\0') + 1,
				   "(", info->ext, ")", NULL);
		man_file = ult_src (*np, path, NULL, ult_flags, NULL);
		if (!man_file)
			goto next;
		lang = lang_dir (man_file);
		cat_file = find_cat_file (path, *np, man_file);
		if (display (path, man_file, cat_file, title, NULL))
			found = 1;
		free (lang);
		lang = NULL;

next:
		free (cat_file);
		free (title);
		free_mandata_struct (info);
	}

	if (regex_opt)
		regfree (&search);

	if (!global_manpath)
		regain_effective_privs ();

	return found;
}

static int do_global_apropos (const char *name, int *found)
{
	const char **my_section_list;
	const char **sp;
	char **mp;

	if (section) {
		my_section_list = XNMALLOC (2, const char *);
		my_section_list[0] = section;
		my_section_list[1] = NULL;
	} else
		my_section_list = section_list;

	for (sp = my_section_list; *sp; sp++)
		for (mp = manpathlist; *mp; mp++)
			*found += do_global_apropos_section (*mp, *sp, name);

	if (section)
		free (my_section_list);

	return *found ? OK : NOT_FOUND;
}

/*
 * Search for manual pages.
 *
 * If preformatted manual pages are supported, look for the formatted
 * file first, then the man page source file.  If they both exist and
 * the man page source file is newer, or only the source file exists,
 * try to reformat it and write the results in the cat directory.  If
 * it is not possible to write the cat file, simply format and display
 * the man file.
 *
 * If preformatted pages are not supported, or the troff option is
 * being used, only look for the man page source file.
 *
 */
static int man (const char *name, int *found)
{
	struct candidate *candidates = NULL, *cand, *candnext;

	*found = 0;
	fflush (stdout);

	if (strchr (name, '/')) {
		int status = local_man_loop (name);
		if (status == OK)
			*found = 1;
		return status;
	}

	if (section) {
		char **mp;

		for (mp = manpathlist; *mp; mp++)
			*found += locate_page (*mp, section, name, &candidates);
	} else {
		const char **sp;

		for (sp = section_list; *sp; sp++) {
			char **mp;

			for (mp = manpathlist; *mp; mp++)
				*found += locate_page (*mp, *sp, name,
						       &candidates);
		}
	}

	sort_candidates (&candidates);

	if (*found)
		*found = display_pages (candidates);

	for (cand = candidates; cand; cand = candnext) {
		candnext = cand->next;
		free (cand);
	}

	return *found ? OK : NOT_FOUND;
}


static const char **get_section_list (void)
{
	int i = 0;
	const char **config_sections;
	const char **sections = NULL;
	const char *sec;

	/* Section list from configuration file, or STD_SECTIONS if it's
	 * empty.
	 */
	config_sections = get_sections ();
	if (!*config_sections) {
		free (config_sections);
		config_sections = std_sections;
	}

	if (colon_sep_section_list == NULL)
		colon_sep_section_list = getenv ("MANSECT");
	if (colon_sep_section_list == NULL || *colon_sep_section_list == '\0')
		return config_sections;

	/* Although this is documented as colon-separated, at least Solaris
	 * man's -s option takes a comma-separated list, so we accept that
	 * too for compatibility.
	 */
	for (sec = strtok (colon_sep_section_list, ":,"); sec; 
	     sec = strtok (NULL, ":,")) {
		sections = xnrealloc (sections, i + 2, sizeof *sections);
 		sections[i++] = sec;
 	}

	if (i > 0) {
		sections[i] = NULL;
		return sections;
	} else {
		if (sections)
			free (sections);
		return config_sections;
	}
}

/* allow user to skip a page or quit after viewing desired page 
   return 1 to skip
   return 0 to view
 */
static inline int do_prompt (const char *name)
{
	int ch;

	skip = 0;
	if (!isatty (STDOUT_FILENO) || !isatty (STDIN_FILENO))
		return 0;

	fprintf (stderr, _( 
		 "--Man-- next: %s "
		 "[ view (return) | skip (Ctrl-D) | quit (Ctrl-C) ]\n"), 
		 name);
	fflush (stderr);

	do {
		ch = getchar ();
		switch (ch) {
			case '\n':
				return 0;
			case EOF:
				skip = 1;
				return 1;
			default:
				break;
		}
	} while (1);

	return 0;
}
