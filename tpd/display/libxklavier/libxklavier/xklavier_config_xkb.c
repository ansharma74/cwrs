/*
 * Copyright (C) 2002-2006 Sergey V. Udaltsov <svu@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <fcntl.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier_private.h"
#include "xklavier_private_xkb.h"

#ifdef LIBXKBFILE_PRESENT
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKM.h>
#endif

#define XKBCOMP ( XKB_BIN_BASE "/xkbcomp" )

#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#ifdef LIBXKBFILE_PRESENT
static XkbRF_RulesPtr xkl_rules;

static XkbRF_RulesPtr
xkl_rules_set_load(XklEngine * engine)
{
	XkbRF_RulesPtr rules_set = NULL;
	char file_name[MAXPATHLEN] = "";
	char *rf =
	    xkl_engine_get_ruleset_name(engine, XKB_DEFAULT_RULESET);
	char *locale = NULL;

	if (rf == NULL) {
		xkl_last_error_message =
		    "Could not find the XKB rules set";
		return NULL;
	}

	locale = setlocale(LC_ALL, NULL);

	g_snprintf(file_name, sizeof file_name, XKB_BASE "/rules/%s", rf);
	xkl_debug(160, "Loading rules from [%s]\n", file_name);

	rules_set = XkbRF_Load(file_name, locale, True, True);

	if (rules_set == NULL) {
		xkl_last_error_message = "Could not load rules";
		return NULL;
	}
	return rules_set;
}

static void
xkl_rules_set_free(void)
{
	if (xkl_rules)
		XkbRF_Free(xkl_rules, True);
	xkl_rules = NULL;
}
#endif

void
xkl_xkb_init_config_registry(XklConfigRegistry * config)
{
#ifdef LIBXKBFILE_PRESENT
	XkbInitAtoms(NULL);
#endif
}

gboolean
xkl_xkb_load_config_registry(XklConfigRegistry * config,
			     gboolean if_extras_needed)
{
	return xkl_config_registry_load_helper(config,
					       XKB_DEFAULT_RULESET,
					       XKB_BASE "/rules",
					       if_extras_needed);
}

#ifdef LIBXKBFILE_PRESENT
gboolean
xkl_xkb_config_native_prepare(XklEngine * engine,
			      const XklConfigRec * data,
			      XkbComponentNamesPtr component_names_ptr)
{
	XkbRF_VarDefsRec xkl_var_defs;
	gboolean got_components;

	memset(&xkl_var_defs, 0, sizeof(xkl_var_defs));

	xkl_rules = xkl_rules_set_load(engine);
	if (!xkl_rules) {
		return FALSE;
	}

	xkl_var_defs.model = (char *) data->model;

	if (data->layouts != NULL)
		xkl_var_defs.layout = xkl_config_rec_merge_layouts(data);

	if (data->variants != NULL)
		xkl_var_defs.variant = xkl_config_rec_merge_variants(data);

	if (data->options != NULL)
		xkl_var_defs.options = xkl_config_rec_merge_options(data);

	got_components =
	    XkbRF_GetComponents(xkl_rules, &xkl_var_defs,
				component_names_ptr);

	g_free(xkl_var_defs.layout);
	g_free(xkl_var_defs.variant);
	g_free(xkl_var_defs.options);

	if (!got_components) {
		xkl_last_error_message =
		    "Could not translate rules into components";
		/* Just cleanup the stuff in case of failure */
		xkl_xkb_config_native_cleanup(engine, component_names_ptr);

		return FALSE;
	}

	if (xkl_debug_level >= 200) {
		xkl_debug(200, "keymap: %s\n",
			  component_names_ptr->keymap);
		xkl_debug(200, "keycodes: %s\n",
			  component_names_ptr->keycodes);
		xkl_debug(200, "compat: %s\n",
			  component_names_ptr->compat);
		xkl_debug(200, "types: %s\n", component_names_ptr->types);
		xkl_debug(200, "symbols: %s\n",
			  component_names_ptr->symbols);
		xkl_debug(200, "geometry: %s\n",
			  component_names_ptr->geometry);
	}
	return TRUE;
}

void
xkl_xkb_config_native_cleanup(XklEngine * engine,
			      XkbComponentNamesPtr component_names_ptr)
{
	xkl_rules_set_free();

	g_free(component_names_ptr->keymap);
	g_free(component_names_ptr->keycodes);
	g_free(component_names_ptr->compat);
	g_free(component_names_ptr->types);
	g_free(component_names_ptr->symbols);
	g_free(component_names_ptr->geometry);
}

static gchar *
xkl_config_get_current_group_description(XklEngine * engine)
{
	XklState state;

	xkl_xkb_get_server_state(engine, &state);

	int group = state.group;
	if ((group < 0) ||
	    (group >=
	     xkl_engine_backend(engine, XklXkb,
				cached_desc)->ctrls->num_groups))
		return NULL;

	return g_strdup(xkl_engine_backend(engine, XklXkb, group_names)
			[group]);
}

static void
xkl_config_set_group_by_description(XklEngine * engine, gchar * descr)
{
	int group, n_groups;
	gchar **group_names;

	if (descr == NULL)
		return;

	// perhaps could be made mode lightweight?
	xkl_engine_reset_all_info(engine, FALSE,
				  "Direct reload on activation");

	n_groups =
	    xkl_engine_backend(engine, XklXkb,
			       cached_desc)->ctrls->num_groups;
	group_names = xkl_engine_backend(engine, XklXkb, group_names);

	for (group = 0; group < n_groups; group++, group_names++) {
		if (!g_ascii_strcasecmp(descr, *group_names)) {
			xkl_debug(150,
				  "Found the group with the same description, %d: [%s]\n",
				  group, *group_names);
			xkl_engine_lock_group(engine, group);
			break;
		}
	}

	g_free(descr);
}

static XkbDescPtr
xkl_config_get_keyboard(XklEngine * engine,
			XkbComponentNamesPtr component_names_ptr,
			gboolean activate)
{
	XkbDescPtr xkb = NULL;

	char xkm_fn[L_tmpnam];
	char xkb_fn[L_tmpnam];
	FILE *tmpxkm;
	XkbFileInfo result;
	int xkmloadres;

	Display *display = xkl_engine_get_display(engine);

	gchar *preactivation_group_description = activate ?
	    xkl_config_get_current_group_description(engine) : NULL;

	if (tmpnam(xkm_fn) != NULL && tmpnam(xkb_fn) != NULL) {
		pid_t cpid, pid;
		int status = 0;
		FILE *tmpxkb;

		xkl_debug(150, "tmp XKB/XKM file names: [%s]/[%s]\n",
			  xkb_fn, xkm_fn);
		if ((tmpxkb = fopen(xkb_fn, "w")) != NULL) {
			fprintf(tmpxkb, "xkb_keymap {\n");
			fprintf(tmpxkb,
				"        xkb_keycodes  { include \"%s\" };\n",
				component_names_ptr->keycodes);
			fprintf(tmpxkb,
				"        xkb_types     { include \"%s\" };\n",
				component_names_ptr->types);
			fprintf(tmpxkb,
				"        xkb_compat    { include \"%s\" };\n",
				component_names_ptr->compat);
			fprintf(tmpxkb,
				"        xkb_symbols   { include \"%s\" };\n",
				component_names_ptr->symbols);
			fprintf(tmpxkb,
				"        xkb_geometry  { include \"%s\" };\n",
				component_names_ptr->geometry);
			fprintf(tmpxkb, "};\n");
			fclose(tmpxkb);

			xkl_debug(150, "xkb_keymap {\n"
				  "        xkb_keycodes  { include \"%s\" };\n"
				  "        xkb_types     { include \"%s\" };\n"
				  "        xkb_compat    { include \"%s\" };\n"
				  "        xkb_symbols   { include \"%s\" };\n"
				  "        xkb_geometry  { include \"%s\" };\n};\n",
				  component_names_ptr->keycodes,
				  component_names_ptr->types,
				  component_names_ptr->compat,
				  component_names_ptr->symbols,
				  component_names_ptr->geometry);

			XSync(display, False);
			/* From this point, ALL errors should be intercepted only by libxklavier */
			xkl_engine_priv(engine, critical_section) = TRUE;

			cpid = fork();
			switch (cpid) {
			case -1:
				xkl_debug(0, "Could not fork: %d\n",
					  errno);
				break;
			case 0:
				/* child */
				xkl_debug(160, "Executing %s\n", XKBCOMP);
				xkl_debug(160, "%s %s %s %s %s %s %s %s\n",
					  XKBCOMP, XKBCOMP, "-w0", "-I",
					  "-I" XKB_BASE, "-xkm", xkb_fn,
					  xkm_fn);
				execl(XKBCOMP, XKBCOMP, "-w0", "-I",
				      "-I" XKB_BASE, "-xkm", xkb_fn,
				      xkm_fn, NULL);
				xkl_debug(0, "Could not exec %s: %d\n",
					  XKBCOMP, errno);
				exit(1);
			default:
				/* parent */
				pid = waitpid(cpid, &status, 0);
				xkl_debug(150,
					  "Return status of %d (well, started %d): %d\n",
					  pid, cpid, status);
				memset((char *) &result, 0,
				       sizeof(result));
				result.xkb = XkbAllocKeyboard();

				if (Success ==
				    XkbChangeKbdDisplay(display,
							&result)) {
					xkl_debug(150,
						  "Hacked the kbddesc - set the display...\n");
					if ((tmpxkm =
					     fopen(xkm_fn, "r")) != NULL) {
						xkmloadres =
						    XkmReadFile(tmpxkm,
								XkmKeymapLegal,
								XkmKeymapLegal,
								&result);
						xkl_debug(150,
							  "Loaded %s output as XKM file, got %d (comparing to %d)\n",
							  XKBCOMP,
							  (int) xkmloadres,
							  (int)
							  XkmKeymapLegal);
						if ((int) xkmloadres !=
						    (int) XkmKeymapLegal) {
							xkl_debug(150,
								  "Loaded legal keymap\n");
							if (activate) {
								xkl_debug
								    (150,
								     "Activating it...\n");
								if (XkbWriteToServer(&result)) {
									xkl_debug
									    (150,
									     "Updating the keyboard...\n");
									xkb = result.xkb;
								} else {
									xkl_debug
									    (0,
									     "Could not write keyboard description to the server\n");
								}
							} else	/* no activate, just load */
								xkb =
								    result.xkb;
						} else {	/* could not load properly */

							xkl_debug(0,
								  "Could not load %s output as XKM file, got %d (asked %d)\n",
								  XKBCOMP,
								  (int)
								  xkmloadres,
								  (int)
								  XkmKeymapLegal);
						}
						fclose(tmpxkm);
						xkl_debug(160,
							  "Unlinking the temporary xkm file %s\n",
							  xkm_fn);
						if (xkl_debug_level < 500) {	/* don't remove on high debug levels! */
							if (remove(xkm_fn)
							    == -1)
								xkl_debug
								    (0,
								     "Could not unlink the temporary xkm file %s: %d\n",
								     xkm_fn,
								     errno);
						} else
							xkl_debug(500,
								  "Well, not really - the debug level is too high: %d\n",
								  xkl_debug_level);
					} else {	/* could not open the file */

						xkl_debug(0,
							  "Could not open the temporary xkm file %s\n",
							  xkm_fn);
					}
				} else {	/* could not assign to display */

					xkl_debug(0,
						  "Could not change the keyboard description to display\n");
				}
				if (xkb == NULL)
					XkbFreeKeyboard(result.xkb,
							XkbAllComponentsMask,
							True);
				break;
			}
			XSync(display, False);
			/* Return to normal X error processing */
			xkl_engine_priv(engine, critical_section) = FALSE;

			if (activate)
				xkl_config_set_group_by_description(engine,
								    preactivation_group_description);

			xkl_debug(160,
				  "Unlinking the temporary xkb file %s\n",
				  xkb_fn);
			if (xkl_debug_level < 500) {	/* don't remove on high debug levels! */
				if (remove(xkb_fn) == -1)
					xkl_debug(0,
						  "Could not unlink the temporary xkb file %s: %d\n",
						  xkb_fn, errno);
			} else
				xkl_debug(500,
					  "Well, not really - the debug level is too high: %d\n",
					  xkl_debug_level);
		} else {	/* could not open input tmp file */

			xkl_debug(0,
				  "Could not open tmp XKB file [%s]: %d\n",
				  xkb_fn, errno);
		}
	} else {
		xkl_debug(0, "Could not get tmp names\n");
	}

	return xkb;
}
#else				/* no XKB headers */
gboolean
xkl_xkb_config_native_prepare(XklEngine * engine,
			      const XklConfigRec * data,
			      gpointer componentNamesPtr)
{
	return FALSE;
}

void
xkl_xkb_config_native_cleanup(XklEngine * engine,
			      gpointer component_names_ptr)
{
}
#endif

/* check only client side support */
gboolean
xkl_xkb_multiple_layouts_supported(XklEngine * engine)
{
	enum { NON_SUPPORTED, SUPPORTED, UNCHECKED };

	static int support_state = UNCHECKED;

	if (support_state == UNCHECKED) {
		XklConfigRec *data = xkl_config_rec_new();
#ifdef LIBXKBFILE_PRESENT
		XkbComponentNamesRec component_names;
		memset(&component_names, 0, sizeof(component_names));
#endif

		data->model = g_strdup("pc105");
		data->layouts = g_strsplit_set("us:de", ":", -1);
		data->variants = g_strsplit_set(":", ":", -1);
		data->options = NULL;

		xkl_debug(100, "!!! Checking multiple layouts support\n");
		support_state = NON_SUPPORTED;
#ifdef LIBXKBFILE_PRESENT
		if (xkl_xkb_config_native_prepare
		    (engine, data, &component_names)) {
			xkl_debug(100,
				  "!!! Multiple layouts ARE supported\n");
			support_state = SUPPORTED;
			xkl_xkb_config_native_cleanup(engine,
						      &component_names);
		} else {
			xkl_debug(100,
				  "!!! Multiple layouts ARE NOT supported\n");
		}
#endif
		g_object_unref(G_OBJECT(data));
	}
	return support_state == SUPPORTED;
}

gboolean
xkl_xkb_activate_config_rec(XklEngine * engine, const XklConfigRec * data)
{
	gboolean rv = FALSE;
#if 0
	{
		int i;
		xkl_debug(150, "New model: [%s]\n", data->model);
		xkl_debug(150, "New layouts: %p\n", data->layouts);
		for (i = 0; i < g_strv_length(data->layouts); i++)
			xkl_debug(150, "New layout[%d]: [%s]\n", i,
				  data->layouts[i]);
		xkl_debug(150, "New variants: %p\n", data->variants);
		for (i = 0; i < g_strv_length(data->variants); i++)
			xkl_debug(150, "New variant[%d]: [%s]\n", i,
				  data->variants[i]);
		xkl_debug(150, "New options: %p\n", data->options);
		for (i = 0; i < g_strv_length(data->options); i++)
			xkl_debug(150, "New option[%d]: [%s]\n", i,
				  data->options[i]);
	}
#endif

#ifdef LIBXKBFILE_PRESENT
	XkbComponentNamesRec component_names;
	memset(&component_names, 0, sizeof(component_names));

	if (xkl_xkb_config_native_prepare(engine, data, &component_names)) {
		XkbDescPtr xkb;
		xkb =
		    xkl_config_get_keyboard(engine, &component_names,
					    TRUE);
		if (xkb != NULL) {
			if (xkl_config_rec_set_to_root_window_property
			    (data,
			     xkl_engine_priv(engine, base_config_atom),
			     xkl_engine_get_ruleset_name(engine,
							 XKB_DEFAULT_RULESET),
			     engine))
				/* We do not need to check the result of _XklGetRulesSetName - 
				   because PrepareBeforeKbd did it for us */
				rv = TRUE;
			else
				xkl_last_error_message =
				    "Could not set names property";
			XkbFreeKeyboard(xkb, XkbAllComponentsMask, True);
		} else {
			xkl_last_error_message =
			    "Could not load keyboard description";
		}
		xkl_xkb_config_native_cleanup(engine, &component_names);
	}
#endif
	return rv;
}

gboolean
xkl_xkb_write_config_rec_to_file(XklEngine * engine, const char *file_name,
				 const XklConfigRec * data,
				 const gboolean binary)
{
	gboolean rv = FALSE;

#ifdef LIBXKBFILE_PRESENT
	XkbComponentNamesRec component_names;
	FILE *output = fopen(file_name, "w");
	XkbFileInfo dump_info;

	if (output == NULL) {
		xkl_last_error_message = "Could not open the XKB file";
		return FALSE;
	}

	memset(&component_names, 0, sizeof(component_names));

	if (xkl_xkb_config_native_prepare(engine, data, &component_names)) {
		XkbDescPtr xkb;
		xkb =
		    xkl_config_get_keyboard(engine, &component_names,
					    FALSE);
		if (xkb != NULL) {
			dump_info.defined = 0;
			dump_info.xkb = xkb;
			dump_info.type = XkmKeymapFile;
			if (binary)
				rv = XkbWriteXKMFile(output, &dump_info);
			else
				rv = XkbWriteXKBFile(output, &dump_info,
						     True, NULL, NULL);

			XkbFreeKeyboard(xkb, XkbGBN_AllComponentsMask,
					True);
		} else
			xkl_last_error_message =
			    "Could not load keyboard description";
		xkl_xkb_config_native_cleanup(engine, &component_names);
	}
	fclose(output);
#endif
	return rv;
}
