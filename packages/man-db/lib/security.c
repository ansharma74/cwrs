/*
 * security.c: Routines to aid secure uid operations 
 *  
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2003, 2004, 2007, 2010, 2011 Colin Watson.
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
 * Mon Aug  8 20:35:30 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include "gettext.h"
#define _(String) gettext (String)

#include "manconfig.h"

#include "error.h"
#include "cleanup.h"
#include "pipeline.h"

#include "security.h"

#ifdef SECURE_MAN_UID

   /*
    * This is the name of the user that the preformatted man pages belong to.
    * If you are running man as a setuid program, you should make sure
    * that all of the cat pages and the directories that
    * they live in are writeable by this user.
    */

#  include <unistd.h>

#  include "idpriv.h"

uid_t ruid;				/* initial real user id */
uid_t euid;				/* initial effective user id */
uid_t uid;				/* current euid */

static struct passwd *man_owner;

/* Keep a count of how many times we've dropped privileges, and only regain
 * them if regain_effective_privs() is called an equal number of times.
 */
static int priv_drop_count = 0;

static inline void gripe_set_euid (void)
{
	error (FATAL, errno, _("can't set effective uid"));
}

void init_security (void)
{
	ruid = getuid ();
	uid = euid = geteuid ();
	debug ("ruid=%d, euid=%d\n", (int) ruid, (int) euid);
	priv_drop_count = 0;
	drop_effective_privs ();
}

int running_setuid (void)
{
	return ruid != euid;
}

/* Return a pointer to the password entry structure for MAN_OWNER. This
 * structure will be statically stored.
 */
struct passwd *get_man_owner (void)
{
	if (man_owner)
		return man_owner;

	man_owner = getpwnam (MAN_OWNER);
	if (!man_owner)
		error (FAIL, 0, _("the setuid man user \"%s\" does not exist"),
		       MAN_OWNER);
	assert (man_owner);
	return man_owner;
}

#endif /* SECURE_MAN_UID */

/* 
 * function to gain user privs by either (a) dropping effective privs 
 * completely (saved ids) or (b) reversing euid w/ uid.
 * Ignore if superuser.
 */
void drop_effective_privs (void)
{
#ifdef SECURE_MAN_UID
	if (uid != ruid) {
		debug ("drop_effective_privs()\n");
		if (idpriv_temp_drop ())
			gripe_set_euid ();
		uid = ruid;
	}

	priv_drop_count++;
	debug ("++priv_drop_count = %d\n", priv_drop_count);
#endif /* SECURE_MAN_UID */
}

/* 
 * function to (re)gain setuid privs by (a) setting euid from suid or (b)
 * (re)reversing uid w/ euid. Ignore if superuser.
 */
void regain_effective_privs (void)
{
#ifdef SECURE_MAN_UID
	if (priv_drop_count) {
		priv_drop_count--;
		debug ("--priv_drop_count = %d\n", priv_drop_count);
		if (priv_drop_count)
			return;
	}

	if (uid != euid) {
		debug ("regain_effective_privs()\n");
		if (idpriv_temp_restore ())
			gripe_set_euid ();

		uid = euid;
	}
#endif /* SECURE_MAN_UID */
}

#ifdef SECURE_MAN_UID
void do_system_drop_privs_child (void *data)
{
	pipeline *p = data;

	if (idpriv_drop ())
		gripe_set_euid ();
	exit (pipeline_run (p));
}
#endif /* SECURE_MAN_UID */

/* The safest way to execute a pipeline with no effective privileges is to
 * fork, permanently drop privileges in the child, run the pipeline from the
 * child, and wait for it to die.
 *
 * It is possible to use saved IDs to avoid the fork, since effective IDs
 * are copied to saved IDs on execve; we used to do this.  However, forking
 * is not expensive enough to justify the extra code.
 *
 * Note that this frees the supplied pipeline.
 */
int do_system_drop_privs (pipeline *p)
{
#ifdef SECURE_MAN_UID
	pipecmd *child_cmd;
	pipeline *child;
	int status;

	child_cmd = pipecmd_new_function ("unprivileged child",
					  do_system_drop_privs_child, NULL, p);
	child = pipeline_new_commands (child_cmd, NULL);
	status = pipeline_run (child);

	pipeline_free (p);
	return status;
#else  /* !SECURE_MAN_UID */
	return pipeline_run (p);
#endif /* SECURE_MAN_UID */
}
