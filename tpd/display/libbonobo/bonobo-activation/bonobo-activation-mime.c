/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *  Copyright (C) 2006 Red Hat, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *       Maciej Stachowiak <mjs@eazel.com>
 *       Alexander Larsson <alexl@redhat.com>
 */

#include <config.h>
#include <string.h>
#include "bonobo-activation-mime.h"
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo-activation/bonobo-activation-init.h>

static char *
extract_prefix_add_suffix (const char *string, const char *separator, const char *suffix)
{
        const char *separator_position;
        int prefix_length;
        char *result;

        separator_position = strstr (string, separator);
        prefix_length = separator_position == NULL
                ? strlen (string)
                : separator_position - string;

        result = g_malloc (prefix_length + strlen (suffix) + 1);
        
        strncpy (result, string, prefix_length);
        result[prefix_length] = '\0';

        strcat (result, suffix);

        return result;
}

static char *
get_supertype_from_mime_type (const char *mime_type)
{
	if (mime_type == NULL) {
		return NULL;
	}
        return extract_prefix_add_suffix (mime_type, "/", "/*");
}

static GList *
Bonobo_ServerInfoList_to_ServerInfo_g_list (Bonobo_ServerInfoList *info_list)
{
	GList *retval;
	int i;
	
	retval = NULL;
	if (info_list != NULL && info_list->_length > 0) {
		for (i = 0; i < info_list->_length; i++) {
			retval = g_list_prepend (retval, Bonobo_ServerInfo_duplicate (&info_list->_buffer[i]));
		}
		retval = g_list_reverse (retval);
	}

	return retval;
}

/**
 * bonobo_activation_get_default_component_for_mime_type:
 * @mime_type: a const char * containing a mime type, e.g. "image/png".
 * 
 * Query the MIME database for the default Bonobo component to be activated to 
 * view files of MIME type @mime_type.
 * 
 * Return value: a #Bonobo_ServerInfo * representing the OAF server to be activated
 * to get a reference to the proper component.
 *
 * Since: 2.16.0
 */
Bonobo_ServerInfo *
bonobo_activation_get_default_component_for_mime_type (const char *mime_type)
{
	Bonobo_ServerInfoList *info_list;
	Bonobo_ServerInfo *default_component;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[4];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	supertype = get_supertype_from_mime_type (mime_type);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);

	/* Prefer something that matches the exact type to something
           that matches the supertype */
	sort[0] = g_strconcat ("bonobo:supported_mime_types.has ('", mime_type, "')", NULL);

	/* Prefer something that matches the supertype to something that matches `*' */
	sort[1] = g_strconcat ("bonobo:supported_mime_types.has ('", supertype, "')", NULL);

	sort[2] = g_strdup ("name");
	sort[3] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	default_component = NULL;
	if (ev._major == CORBA_NO_EXCEPTION) {
		if (info_list != NULL && info_list->_length > 0) {
			default_component = Bonobo_ServerInfo_duplicate (&info_list->_buffer[0]);
		}
		CORBA_free (info_list);
	}

	g_free (supertype);
	g_free (query);
	g_free (sort[0]);
	g_free (sort[1]);
	g_free (sort[2]);
	g_free (sort[3]);

	CORBA_exception_free (&ev);

	return default_component;
}

/**
 * bonobo_activation_get_all_components_for_mime_type:
 * @mime_type: a const char * containing a mime type, e.g. "image/png".
 * 
 * Return an alphabetically sorted list of #Bonobo_ServerInfo
 * data structures representing all Bonobo components registered
 * to handle files of @mime_type (and supertypes).
 * 
 * Return value: a #GList * where the elements are #Bonobo_ServerInfo *
 * representing components that can handle @mime_type.
 *
 * Since: 2.16.0
 */ 
GList *
bonobo_activation_get_all_components_for_mime_type (const char *mime_type)
{
	Bonobo_ServerInfoList *info_list;
	GList *components_list;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[2];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* FIXME bugzilla.eazel.com 1142: should probably check for
           the right interfaces too. Also slightly semantically
           different from nautilus in other tiny ways.
	*/
	supertype = get_supertype_from_mime_type (mime_type);
	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);
	g_free (supertype);
	
	/* Alphebetize by name, for the sake of consistency */
	sort[0] = g_strdup ("name");
	sort[1] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	if (ev._major == CORBA_NO_EXCEPTION) {
		components_list = Bonobo_ServerInfoList_to_ServerInfo_g_list (info_list);
		CORBA_free (info_list);
	} else {
		components_list = NULL;
	}

	g_free (query);
	g_free (sort[0]);

	CORBA_exception_free (&ev);

	return components_list;
}

/* This is code that used to be in gnome-vfs, but that has been moved to bonobo so that
 * gnome-vfs doesn't have to depend on bonobo. We keep the old names to keep binary compatibility,
 * although new users of this should use the bonobo_activation_* calls above instead.
 */

/* There public headers are in gnome-vfs, but you really shouldn't use these functions */
Bonobo_ServerInfo *gnome_vfs_mime_get_default_component       (const char *mime_type);
GList *            gnome_vfs_mime_get_all_components          (const char *mime_type);
void               gnome_vfs_mime_component_list_free         (GList      *list);
GList *            gnome_vfs_mime_remove_component_from_list  (GList      *components,
							       const char *iid,
							       gboolean   *did_remove);
GList *            gnome_vfs_mime_id_list_from_component_list (GList      *components);
gboolean           gnome_vfs_mime_id_in_component_list        (const char *iid,
							       GList      *components);
GList *            gnome_vfs_mime_get_short_list_components   (const char *mime_type);

static void
initialize_bonobo (void)
{
	char *bogus_argv[2] = { "dummy", NULL };
	static gboolean initialized = FALSE;

	if (initialized) {
		return;
	}

	initialized = TRUE;

	if (!bonobo_activation_is_initialized ()) {
		bonobo_activation_init (0, bogus_argv);
	}
	
	return;
}


Bonobo_ServerInfo *
gnome_vfs_mime_get_default_component (const char *mime_type)
{
  initialize_bonobo ();

  return bonobo_activation_get_default_component_for_mime_type (mime_type);
}

GList *
gnome_vfs_mime_get_all_components (const char *mime_type)
{
	initialize_bonobo ();

	return bonobo_activation_get_all_components_for_mime_type (mime_type);
}

GList *
gnome_vfs_mime_get_short_list_components (const char *mime_type)
{
	Bonobo_ServerInfoList *info_list;
	GList *components_list;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[4];

	initialize_bonobo ();
	
	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* FIXME bugzilla.eazel.com 1142: should probably check for
           the right interfaces too. Also slightly semantically
           different from nautilus in other tiny ways.
	*/
	supertype = get_supertype_from_mime_type (mime_type);
	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);
	
        /* Prefer something that matches the exact type to something
           that matches the supertype */
	sort[0] = g_strconcat ("bonobo:supported_mime_types.has ('", mime_type, "')", NULL);

	/* Prefer something that matches the supertype to something that matches `*' */
	sort[1] = g_strconcat ("bonobo:supported_mime_types.has ('", supertype, "')", NULL);

	sort[2] = g_strdup ("name");
	sort[3] = NULL;

	info_list = bonobo_activation_query (query, sort, &ev);
	
	if (ev._major == CORBA_NO_EXCEPTION) {
		components_list = Bonobo_ServerInfoList_to_ServerInfo_g_list (info_list);
		CORBA_free (info_list);
	} else {
		components_list = NULL;
	}

	g_free (supertype);
	g_free (query);
	g_free (sort[0]);
	g_free (sort[1]);
	g_free (sort[2]);

	CORBA_exception_free (&ev);

	return components_list;
}

static gint 
gnome_vfs_mime_component_matches_id (Bonobo_ServerInfo *component, const char *iid)
{
	return strcmp (component->iid, iid);
}

GList *
gnome_vfs_mime_id_list_from_component_list (GList *components)
{
	GList *list = NULL;
	GList *node;

	for (node = components; node != NULL; node = node->next) {
		list = g_list_prepend 
			(list, g_strdup (((Bonobo_ServerInfo *)node->data)->iid));
	}
	return g_list_reverse (list);
}

gboolean
gnome_vfs_mime_id_in_component_list (const char *iid, GList *components)
{
	return g_list_find_custom
		(components, (gpointer) iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id) != NULL;
}

GList *
gnome_vfs_mime_remove_component_from_list (GList *components, 
					   const char *iid,
					   gboolean *did_remove)
{
	GList *matching_node;
	
	matching_node = g_list_find_custom 
		(components, (gpointer)iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id);
	if (matching_node != NULL) {
		components = g_list_remove_link (components, matching_node);
		gnome_vfs_mime_component_list_free (matching_node);
	}

	if (did_remove != NULL) {
		*did_remove = matching_node != NULL;
	}
	return components;
}

void
gnome_vfs_mime_component_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) CORBA_free, NULL);
	g_list_free (list);
}

