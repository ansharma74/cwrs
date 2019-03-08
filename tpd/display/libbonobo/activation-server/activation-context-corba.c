/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation-server: CORBA activation dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 1999, 2000 Eazel, Inc.
 *  Copyright (C) 1999, 2003 Ximian, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>,
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

#include <glib/gi18n.h>

#include "server.h"

#include "activation-context.h"
#include "bonobo-activation-id.h"
#include "activation-context-query.h"
#include "activation-server-corba-extensions.h"
#include <libbonobo.h>

#undef LOCALE_DEBUG

#define Bonobo_LINK_TIME_TO_LIVE 256

static GObjectClass *parent_class = NULL;

static void
directory_info_free (ActivationContext *actx, CORBA_Environment *ev)
{
	CORBA_Object_release (actx->obj, ev);
        actx->obj = CORBA_OBJECT_NIL;

        if (actx->by_iid) {
                g_hash_table_destroy (actx->by_iid);
                actx->by_iid = NULL;
        }

        if (actx->list) {
                CORBA_sequence_set_release (actx->list, CORBA_TRUE);
                CORBA_free (actx->list);
                actx->list = NULL;
        }

        if (actx->active_servers)
                g_hash_table_destroy (actx->active_servers);
        CORBA_free (actx->active_server_list);
        actx->active_server_list = NULL;
}

static void
ac_update_active (ActivationContext *actx, CORBA_Environment *ev)
{
	int i;
	Bonobo_ServerStateCache *cache;

	cache = Bonobo_ObjectDirectory_get_active_servers (
                actx->obj, actx->time_active_pulled, ev);

	if (ev->_major != CORBA_NO_EXCEPTION) {
                CORBA_Object_release (actx->obj, ev);
                actx->obj = CORBA_OBJECT_NIL;
	}

	if (cache->_d) {
		if (actx->active_servers) {
			g_hash_table_destroy (actx->active_servers);
			CORBA_free (actx->active_server_list);
		}

		actx->active_server_list = cache;

		actx->time_active_pulled = time (NULL);
		actx->active_servers =
			g_hash_table_new (g_str_hash, g_str_equal);
		for (i = 0; i < cache->_u.active_servers._length; i++)
			g_hash_table_insert (actx->active_servers,
					     cache->_u.
					     active_servers._buffer[i],
					     GINT_TO_POINTER (1));
	} else
		CORBA_free (cache);
}

static char *
ac_CORBA_Context_get_value (CORBA_Context         ctx, 
                            const char           *propname,
                            CORBA_Environment    *ev)
{
        return activation_server_CORBA_Context_get_value (
                ctx, propname,
                ex_Bonobo_Activation_IncompleteContext, ev);
}

static void
ac_update_list (ActivationContext *actx, CORBA_Environment *ev)
{
	int i;
	Bonobo_ServerInfoListCache *cache;

	cache = Bonobo_ObjectDirectory_get_servers (
                actx->obj, actx->time_list_pulled, ev);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		actx->list = NULL;
                CORBA_Object_release (actx->obj, ev);
                actx->obj = CORBA_OBJECT_NIL;
		return;
	}

	if (cache->_d) {
		if (actx->by_iid)
			g_hash_table_destroy (actx->by_iid);
		if (actx->list) {
			CORBA_sequence_set_release (actx->list, CORBA_TRUE);
			CORBA_free (actx->list);
			actx->list = NULL;
		}

                actx->list = ORBit_copy_value (&cache->_u.server_list,
                                               TC_Bonobo_ServerInfoList);

		actx->time_list_pulled = time (NULL);
		actx->by_iid = g_hash_table_new (g_str_hash, g_str_equal);
		for (i = 0; i < actx->list->_length; i++)
			g_hash_table_insert (actx->by_iid,
					     actx->list->_buffer[i].iid,
					     &(actx->list->_buffer[i]));
	}

	CORBA_free (cache);
}

static QueryExprConst
ac_query_get_var (Bonobo_ServerInfo *si, const char *id, QueryContext *qctx)
{
	QueryExprConst retval;
        ActivationContext *actx = qctx->user_data;

	retval.value_known = FALSE;
	retval.needs_free = FALSE;

	if (!g_ascii_strcasecmp (id, "_active")) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		ac_update_active (actx, &ev);
		CORBA_exception_free (&ev);

		retval.value_known = TRUE;
		retval.type = CONST_BOOLEAN;
		retval.u.v_boolean =
			g_hash_table_lookup (actx->active_servers,
					     si->iid) ? TRUE : FALSE;
	}

	return retval;
}

/* This function should only be called by
 * impl_Bonobo_ActivationContext_query and
 * impl_Bonobo_ActivationContext_activateMatching - hairy implicit preconditions
 * exist. */
static void
ac_query_run (ActivationContext        *actx,
	      const CORBA_char        *requirements,
	      const Bonobo_StringList *selection_order,
	      CORBA_Context            ctx,
	      Bonobo_ServerInfo      **items,
              CORBA_Environment       *ev)
{
	int total, i;
	QueryContext qctx;

	Bonobo_ServerInfo **orig_items;
	int item_count, orig_item_count;
	char *errstr;
	Bonobo_Activation_ParseFailed *ex;

	QueryExpr *qexp_requirements;
	QueryExpr **qexp_sort_items;

	/* First, parse the query */
	errstr = (char *) qexp_parse (requirements, &qexp_requirements);
	if (errstr) {
		puts (errstr);

		g_strstrip (errstr);
		ex = Bonobo_Activation_ParseFailed__alloc ();
		ex->description = CORBA_string_dup (errstr);
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Activation_ParseFailed,
				     ex);
		return;
	}

	qexp_sort_items =
		g_alloca (selection_order->_length * sizeof (QueryExpr *));
	for (i = 0; i < selection_order->_length; i++) {
		errstr =
			(char *) qexp_parse (selection_order->_buffer[i],
					     &qexp_sort_items[i]);

		if (errstr) {
			qexp_free (qexp_requirements);
			for (i--; i >= 0; i--)
				qexp_free (qexp_sort_items[i]);

			g_strstrip (errstr);
			ex = Bonobo_Activation_ParseFailed__alloc ();
			ex->description = CORBA_string_dup (errstr);

			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Activation_ParseFailed,
					     ex);
			return;
		}
	}

	total = actx->total_servers;
	orig_items = g_alloca (total * sizeof (Bonobo_ServerInfo *));

        {
		int i;

                item_count = 0;

		if (actx->obj != CORBA_OBJECT_NIL)
                        for (i = 0; i < actx->list->_length; i++, item_count++)
                                items[item_count] = &actx->list->_buffer[i];
	}

	memcpy (orig_items, items, item_count * sizeof (Bonobo_ServerInfo *));
	orig_item_count = item_count;

	qctx.sil = orig_items;
	qctx.nservers = orig_item_count;
	qctx.cctx = ctx;
	qctx.id_evaluator = ac_query_get_var;
	qctx.user_data = actx;

	for (i = 0; i < item_count; i++) {
		if (!qexp_matches (items[i], qexp_requirements, &qctx))
			items[i] = NULL;
	}

	qexp_sort (items, item_count, qexp_sort_items,
		   selection_order->_length, &qctx);

        qexp_free (qexp_requirements);
        for (i = 0; i < selection_order->_length; i++)
                qexp_free (qexp_sort_items[i]);
}

static void
ac_update_lists (ActivationContext *actx,
		 CORBA_Environment *ev)
{
	int prev, new;

	if (actx->refs > 0) {
                /* FIXME: what happens on re-enterency here ?
                 * looks like this could get seriously out of date */
		return;
        }

	if (actx->list)
		prev = actx->list->_length;
	else
		prev = 0;

	ac_update_list (actx, ev);

	if (actx->list)
		new = actx->list->_length;
	else
		new = 0;

	actx->total_servers += (new - prev);
}

static GList *clients = NULL;

void
activation_clients_cache_notify (void)
{
        GList *l;
        GSList *notify = NULL, *l2;
        CORBA_Environment ev;

        CORBA_exception_init (&ev);

        for (l = clients; l; l = l->next)
                notify = g_slist_prepend (notify, CORBA_Object_duplicate (l->data, &ev));

        for (l2 = notify; l2; l2 = l2->next) {
                Bonobo_ActivationClient_resetCache (l2->data, &ev);
                if (ev._major != CORBA_NO_EXCEPTION)
                        clients = g_list_remove (clients, l2->data);

                CORBA_Object_release (l2->data, &ev);
                CORBA_exception_free (&ev);
        }

	g_slist_free (notify);
}

gboolean
activation_clients_is_empty_scan (void)
{
        GList *l, *check = NULL;

        for (l = clients; l; l = l->next)
                check = g_list_prepend (check, CORBA_Object_duplicate (l->data, NULL));

        for (l = check; l; l = l->next) {
                if (ORBit_small_get_connection_status (l->data) ==
                    ORBIT_CONNECTION_DISCONNECTED) {
                        GList *remove;
                        if ((remove = g_list_find (clients, l->data))) {
                                CORBA_Object_release (remove->data, NULL);
                                clients = g_list_delete_link (clients, remove);
                        }
                }
                CORBA_Object_release (l->data, NULL);
        }

        g_list_free (check);

        return clients == NULL;
}

static gboolean
ac_rescan (gpointer is_idle_rescan)
{
        static gboolean in_rescan = FALSE;
        static guint idle_id = 0;

        server_lock ();

        if (GPOINTER_TO_UINT (is_idle_rescan))
                idle_id = 0;

        /* We tend to get a lot of 'broken' callbacks at once */
        if (in_rescan) {
                if (!idle_id)
                        idle_id = g_timeout_add (100, ac_rescan, GUINT_TO_POINTER(1));
                server_unlock ();
                return FALSE;
        }
        in_rescan = TRUE;

        if (activation_clients_is_empty_scan ()) {
#ifdef BONOBO_ACTIVATION_DEBUG
                g_warning ("All clients dead");
#endif
                check_quit ();
        }

        in_rescan = FALSE;
        server_unlock ();

        return FALSE;
}

static void
active_client_cnx_broken (ORBitConnection *cnx,
                          gpointer         dummy)
{
        ac_rescan (NULL);
}

static void
impl_Bonobo_ActivationContext_addClient (PortableServer_Servant        servant,
                                         const Bonobo_ActivationClient client,
                                         const CORBA_char             *locales,
                                         CORBA_Environment            *ev)
{
        GList *l;
        gboolean new_locale;
        ORBitConnection *cnx;

        server_lock ();

        new_locale = register_interest_in_locales (locales);

        cnx = ORBit_small_get_connection (client);
        for (l = clients; l; l = l->next)
                if (cnx == ORBit_small_get_connection (l->data))
                        break;
        
        clients = g_list_prepend (
                clients, CORBA_Object_duplicate (client, ev));

        if (!l) {
                ORBit_small_listen_for_broken
                        (client, G_CALLBACK (active_client_cnx_broken), NULL);
                check_quit ();
        }

        if (new_locale)
                bonobo_object_directory_reload ();

        server_unlock ();
}

static Bonobo_ObjectDirectoryList *
impl_Bonobo_ActivationContext__get_directories (PortableServer_Servant servant,
                                                CORBA_Environment     *ev)
{
        ActivationContext *actx;
	Bonobo_ObjectDirectoryList *retval;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

	retval = Bonobo_ObjectDirectoryList__alloc ();
        if (actx->obj != CORBA_OBJECT_NIL) {
                retval->_length = 1;
                retval->_buffer =
                        CORBA_sequence_Bonobo_ObjectDirectory_allocbuf (1);
		retval->_buffer[0] = CORBA_Object_duplicate (actx->obj, ev);
	} else {
                retval->_length = 0;
        }

	CORBA_sequence_set_release (retval, CORBA_TRUE);

        server_unlock ();

	return retval;
}

static void
impl_Bonobo_ActivationContext_addDirectory (PortableServer_Servant servant,
                                            Bonobo_ObjectDirectory dir,
                                            CORBA_Environment     *ev)
{
        ActivationContext *actx;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

        if (actx->obj == dir)
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
                                     ex_Bonobo_Activation_AlreadyListed,
                                     NULL);
	else {
                CORBA_Object_release (actx->obj, ev);
                actx->obj = CORBA_Object_duplicate (dir, ev);
        }

        server_unlock ();
}

static void
impl_Bonobo_ActivationContext_removeDirectory (PortableServer_Servant servant,
                                               Bonobo_ObjectDirectory dir,
                                               CORBA_Environment     *ev)
{
        ActivationContext *actx;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

        if (dir != actx->obj)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Activation_NotListed,
				     NULL);
        else {
                if (actx->refs) {
                        CORBA_Object_release (actx->obj, ev);
                        actx->obj = CORBA_OBJECT_NIL;
                } else
                        directory_info_free (actx, ev);
        }

        server_unlock ();
}

static void
ac_do_activation (ActivationContext                  *actx,
		  Bonobo_ServerInfo                  *server,
		  const Bonobo_ActivationEnvironment *environment,
		  Bonobo_ActivationResult            *out,
		  Bonobo_ActivationFlags              flags,
		  const char                         *hostname,
                  Bonobo_ActivationClient             client,
		  CORBA_Context                       ctx,
                  CORBA_Environment                  *ev)
{
	int num_layers;
	Bonobo_ServerInfo *activatable;
        gchar *aid = NULL;

	/* When doing checks for shlib loadability, we 
         * have to find the info on the factory object in case
	 * a factory is inside a shlib 
         */
	if (!actx->obj || ev->_major != CORBA_NO_EXCEPTION) {
		Bonobo_GeneralError *errval = Bonobo_GeneralError__alloc ();
		errval->description =
			CORBA_string_dup
			(_("Couldn't find which child the server was listed in"));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_GeneralError, errval);
		return;
	}

	for (num_layers = 0, activatable = server;
             activatable && activatable->server_type &&
                     !strcmp (activatable->server_type, "factory") &&
             num_layers < Bonobo_LINK_TIME_TO_LIVE; num_layers++) {

		activatable = g_hash_table_lookup (actx->by_iid, activatable->location_info);
	}

	if (activatable == NULL) {		
		Bonobo_GeneralError *errval = Bonobo_GeneralError__alloc ();
		errval->description = CORBA_string_dup ("Couldn't find the factory server");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_GeneralError, errval);
		return;
	} 
	else if (num_layers == Bonobo_LINK_TIME_TO_LIVE) {
		Bonobo_GeneralError *errval = Bonobo_GeneralError__alloc ();
		errval->description = CORBA_string_dup ("Location loop");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_GeneralError, errval);
		return;
        }

	/* A shared library must be on the same host as the activator in
	 * order for loading to work properly (no, we're not going to
	 * bother with loading a remote shlib into a process - it gets far too complicated
	 * far too quickly :-) */

        if (activatable)
                aid = g_strdup_printf ("OAFAID:[%s,%s,%s]", activatable->iid,
                                       activatable->username, activatable->hostname);
	
	if (activatable && activatable->server_type && !strcmp (activatable->server_type, "shlib")
	    && !(flags & Bonobo_ACTIVATION_FLAG_NO_LOCAL)
	    && (hostname && !strcmp (activatable->hostname, hostname))) {
		int j;
		
		out->res._d = Bonobo_ACTIVATION_RESULT_SHLIB;		

		/* Here is an explanation as to why we add 2 to num_layers.
		 * At the end of the string list, after all the factory iids are added
		 * to the string list, we then add the iid of the shaed library and the 
		 * location info.  This data is later used in oaf_server_activate_shlib
		 * to activate the component
		 */		 
		out->res._u.res_shlib._length = num_layers + 2;
		out->res._u.res_shlib._buffer = CORBA_sequence_CORBA_string_allocbuf (num_layers + 2);

		/* Copy over factory info */
		for (j = 0, activatable = server; activatable
		     && !strcmp (activatable->server_type, "factory"); j++) {
			out->res._u.res_shlib._buffer[j] = CORBA_string_dup (activatable->iid);
			activatable = g_hash_table_lookup (actx->by_iid,
						     	   activatable->location_info);
		}

		/* Copy shlib iid into buffer */
		out->res._u.res_shlib._buffer[j] = CORBA_string_dup (activatable->iid);

		/* Copy location into last buffer slot for use in later activation */
		out->res._u.res_shlib._buffer[j+1] = CORBA_string_dup (activatable->location_info);

		out->aid = CORBA_string_dup (aid);
	} else {
		CORBA_Object retval;
                char *iid = g_strdup (server->iid);

		retval = Bonobo_ObjectDirectory_activate (
                        actx->obj, iid, BONOBO_OBJREF (actx),
                        environment, flags, client, ctx, ev);
                g_free (iid);

		if (ev->_major == CORBA_NO_EXCEPTION) {
			out->res._d = Bonobo_ACTIVATION_RESULT_OBJECT;
			out->res._u.res_object = retval;
			out->aid = CORBA_string_dup (aid);
		}
#ifdef BONOBO_ACTIVATION_DEBUG
                else
                        g_warning ("Activation of '%s' failed with exception '%s'",
                                   aid, ev->_id);
#endif
	}
        g_free (aid);
}


static Bonobo_ActivationResult *
impl_Bonobo_ActivationContext_activateMatchingFull (
        PortableServer_Servant              servant,
        const CORBA_char                   *requirements,
        const Bonobo_StringList            *selection_order,
	const Bonobo_ActivationEnvironment *environment,
        const Bonobo_ActivationFlags        flags,
        Bonobo_ActivationClient             client,
        CORBA_Context                       ctx,
        CORBA_Environment                  *ev)
{
        ActivationContext *actx;
	Bonobo_ActivationResult *retval = NULL;
	Bonobo_ServerInfo **items, *curitem;
	int i;
	char *hostname;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

	ac_update_lists (actx, ev);

	actx->refs++;

	items = g_alloca (actx->total_servers *
                          sizeof (Bonobo_ServerInfo *));
	ac_query_run (actx, requirements, selection_order, ctx, items, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto out;

	hostname = ac_CORBA_Context_get_value (ctx, "hostname", ev);

	retval = Bonobo_ActivationResult__alloc ();
	retval->res._d = Bonobo_ACTIVATION_RESULT_NONE;

        bonobo_object_ref (actx);
	for (i = 0; (retval->res._d == Bonobo_ACTIVATION_RESULT_NONE) && items[i]
	     && (i < actx->total_servers); i++) {
		curitem = items[i];

		ac_do_activation (actx, curitem, environment,
				  retval, flags, hostname, client, ctx, ev);
	}
        bonobo_object_unref (actx);

	if (retval->res._d == Bonobo_ACTIVATION_RESULT_NONE)
		retval->aid = CORBA_string_dup ("");

	g_free (hostname);

 out:
	if (ev->_major != CORBA_NO_EXCEPTION) {
                CORBA_free (retval);
                retval = NULL;
        }

	actx->refs--;

        server_unlock ();

	return retval;
}

static Bonobo_ActivationResult *
impl_Bonobo_ActivationContext_activateMatching (
        PortableServer_Servant              servant,
        const CORBA_char                   *requirements,
        const Bonobo_StringList            *selection_order,
	const Bonobo_ActivationEnvironment *environment,
        const Bonobo_ActivationFlags        flags,
        CORBA_Context                       ctx,
        CORBA_Environment                  *ev)
{
        return impl_Bonobo_ActivationContext_activateMatchingFull
                (servant, requirements, selection_order,
                 environment, flags, CORBA_OBJECT_NIL, ctx, ev);
}

static Bonobo_ServerInfoList *
impl_Bonobo_ActivationContext_query (PortableServer_Servant servant,
                                     const CORBA_char * requirements,
                                     const Bonobo_StringList * selection_order,
                                     CORBA_Context ctx, CORBA_Environment * ev)
{
        ActivationContext *actx;
	Bonobo_ServerInfoList *retval;
	Bonobo_ServerInfo **items;
	int item_count;
	int i, j, total;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

	retval = Bonobo_ServerInfoList__alloc ();
	retval->_length = 0;
	retval->_buffer = NULL;
	CORBA_sequence_set_release (retval, CORBA_TRUE);

	/* Pull in new lists from OD servers */
	ac_update_lists (actx, ev);
	actx->refs++;

	items = g_alloca (actx->total_servers *
                          sizeof (Bonobo_ServerInfo *));
	item_count = actx->total_servers;

	ac_query_run (actx, requirements, selection_order, ctx, items, ev);

	if (ev->_major == CORBA_NO_EXCEPTION) {
		for (total = i = 0; i < item_count; i++) {
			if (items[i])
				total++;
		}

		retval->_length = total;
		retval->_buffer =
			CORBA_sequence_Bonobo_ServerInfo_allocbuf (total);

		for (i = j = 0; i < item_count; i++) {
			if (!items[i])
				continue;

			Bonobo_ServerInfo_copy (&retval->_buffer[j], items[i]);

			j++;
		}
	}

	actx->refs--;

        server_unlock ();

	return retval;
}

static char *
ac_aid_to_query_string (const CORBA_char *aid)
{
        char *requirements;
        char *iid_requirement;
        char *username_requirement;
        char *hostname_requirement;
	BonoboActivationInfo *ainfo;

	ainfo = bonobo_activation_id_parse (aid);
	if (!ainfo)
                return NULL;

        iid_requirement = g_strconcat ("iid == \'", ainfo->iid, "\' ", NULL);

        if (ainfo->user) {
                username_requirement = g_strconcat ("AND username == \'", ainfo->user, "\'", NULL);
        } else {
                username_requirement = g_strdup ("");
        }
        
        if (ainfo->host) {
                hostname_requirement = g_strconcat ("AND hostname == \'", ainfo->host, "\'", NULL);
        } else {
                hostname_requirement = g_strdup ("");
        }
        
        requirements = g_strconcat (iid_requirement, username_requirement, 
                                    hostname_requirement, NULL);

        g_free (iid_requirement);
        g_free (username_requirement);
        g_free (hostname_requirement);
        bonobo_activation_info_free (ainfo);

        return requirements;
}

static void
ac_context_to_string_array (CORBA_Context context, char **sort_criteria,
                            CORBA_Environment *ev)
{
	char *context_username;
	char *context_hostname;

        context_username = ac_CORBA_Context_get_value (context, "username", ev);
        context_hostname = ac_CORBA_Context_get_value (context, "hostname", ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
                g_free (context_username);
                g_free (context_hostname);
                return;
        }

        sort_criteria[0] = g_strconcat ("username == \'", context_username, "\'", NULL);
        sort_criteria[1] = g_strconcat ("hostname == \'", context_hostname, "\'", NULL);
        sort_criteria[2] = NULL;

        g_free (context_username);
        g_free (context_hostname);
}

#define PARSE_ERROR_NOT_AN_AID (_("Not a valid Activation ID"))

static Bonobo_ActivationResult *
impl_Bonobo_ActivationContext_activateFromAidFull (PortableServer_Servant  servant,
                                                   const CORBA_char       *aid,
                                                   Bonobo_ActivationFlags  flags,
                                                   Bonobo_ActivationClient client,
                                                   CORBA_Context           ctx,
                                                   CORBA_Environment      *ev)
{
        ActivationContext *actx;
	Bonobo_ActivationResult *retval = NULL;
        char *requirements;
        char *sort_criteria[3];
        Bonobo_StringList selection_order;
	Bonobo_ActivationEnvironment environment;

        server_lock ();

        actx = ACTIVATION_CONTEXT (servant);

	if (strncmp ("OAFAID:", aid, 7) != 0) {
		Bonobo_Activation_ParseFailed *ex;

		ex = Bonobo_Activation_ParseFailed__alloc ();
		ex->description = CORBA_string_dup (PARSE_ERROR_NOT_AN_AID);

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Activation_ParseFailed,
				     ex);
                goto act_out;
	}

	ac_update_lists (actx, ev);
        if (ev->_major != CORBA_NO_EXCEPTION)
                goto act_out;

	actx->refs++;

        requirements = ac_aid_to_query_string (aid);
        if (requirements == NULL) {
                actx->refs--;
                goto act_out;
        }

        ac_context_to_string_array (ctx, sort_criteria, ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                actx->refs--;
                g_free (requirements);
                goto act_out;
        }

        selection_order._length = 2;
        selection_order._buffer = sort_criteria;
        CORBA_sequence_set_release (&selection_order, CORBA_FALSE);

	memset (&environment, 0, sizeof (Bonobo_ActivationEnvironment));

        retval = impl_Bonobo_ActivationContext_activateMatchingFull (
                actx, requirements, &selection_order, &environment,
                flags, client, ctx, ev);

        g_free (sort_criteria[0]);
        g_free (sort_criteria[1]);
        g_free (requirements);

        actx->refs--;

 act_out:
        server_unlock ();

        return retval;
}

static Bonobo_ActivationResult *
impl_Bonobo_ActivationContext_activateFromAid (PortableServer_Servant  servant,
					       const CORBA_char       *aid,
					       Bonobo_ActivationFlags  flags,
					       CORBA_Context           ctx,
					       CORBA_Environment      *ev)
{
        return impl_Bonobo_ActivationContext_activateFromAidFull
                (servant, aid, flags, CORBA_OBJECT_NIL, ctx, ev);
}

static CORBA_long
impl_Bonobo_ActivationContext_getVersion (PortableServer_Servant  servant,
                                          CORBA_Environment      *ev)
{
        return (BONOBO_ACTIVATION_MAJOR_VERSION*10000 + 
                BONOBO_ACTIVATION_MINOR_VERSION*100 +
                BONOBO_ACTIVATION_MICRO_VERSION);
}

static ActivationContext *main_ac = NULL;

void
activation_context_setup (PortableServer_POA     poa,
                          Bonobo_ObjectDirectory dir,
                          CORBA_Environment     *ev)
{
        main_ac = g_object_new (activation_context_get_type (),
                                "poa", poa, NULL);

        impl_Bonobo_ActivationContext_addDirectory
                (BONOBO_OBJECT (main_ac), dir, ev);
}

void
activation_context_shutdown (void)
{
        if (main_ac) {
                bonobo_object_set_immortal (BONOBO_OBJECT (main_ac), FALSE);
                bonobo_object_unref (BONOBO_OBJECT (main_ac));
                main_ac = NULL;
        }        
}

Bonobo_ActivationContext
activation_context_get (void)
{
        if (!main_ac)
                return CORBA_OBJECT_NIL;
        else
                return BONOBO_OBJREF (main_ac);
}

static void 
activation_context_finalize (GObject *object)
{
        CORBA_Environment ev[1];
        ActivationContext *actx = (ActivationContext *) object;

        CORBA_exception_init (ev);

        directory_info_free (actx, ev);

        CORBA_exception_free (ev);

        parent_class->finalize (object);
}

static void
activation_context_class_init (ActivationContextClass *klass)
{
        GObjectClass *object_class = (GObjectClass *) klass;
	POA_Bonobo_ActivationContext__epv *epv = &klass->epv;

        parent_class = g_type_class_peek_parent (klass);
        object_class->finalize = activation_context_finalize;

        epv->_get_directories = impl_Bonobo_ActivationContext__get_directories;
	epv->addClient         = impl_Bonobo_ActivationContext_addClient;
	epv->addDirectory      = impl_Bonobo_ActivationContext_addDirectory;
	epv->removeDirectory   = impl_Bonobo_ActivationContext_removeDirectory;
	epv->query             = impl_Bonobo_ActivationContext_query;
	epv->activateMatching  = impl_Bonobo_ActivationContext_activateMatching;
	epv->activateFromAid   = impl_Bonobo_ActivationContext_activateFromAid;
	epv->getVersion        = impl_Bonobo_ActivationContext_getVersion;
	epv->activateMatchingFull = impl_Bonobo_ActivationContext_activateMatchingFull;
	epv->activateFromAidFull  = impl_Bonobo_ActivationContext_activateFromAidFull;
}

static void
activation_context_init (ActivationContext *actx)
{
        bonobo_object_set_immortal (BONOBO_OBJECT (actx), TRUE);
}

BONOBO_TYPE_FUNC_FULL (ActivationContext,
                       Bonobo_ActivationContext,
                       BONOBO_TYPE_OBJECT,
                       activation_context)
