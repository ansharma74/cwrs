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

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "config.h"

#include "xklavier_private.h"
#include "xklavier_private_xmm.h"

#define SHORTCUT_OPTION_PREFIX "grp:"

const gchar **
xkl_xmm_get_groups_names(XklEngine * engine)
{
	return (const gchar **) xkl_engine_backend(engine, XklXmm,
						   current_config).layouts;
}

const gchar **
xkl_xmm_get_indicators_names(XklEngine * engine)
{
	return NULL;
}

void
xkl_xmm_shortcuts_grab(XklEngine * engine)
{
	const XmmShortcut *shortcut;
	const XmmSwitchOption *option =
	    xkl_xmm_shortcut_get_current(engine);

	xkl_debug(150, "Found shortcut option: %p\n", option);
	if (option == NULL)
		return;

	shortcut = option->shortcuts;
	while (shortcut->keysym != XK_VoidSymbol) {
		int keycode =
		    XKeysymToKeycode(xkl_engine_get_display(engine),
				     shortcut->keysym);
		xkl_xmm_grab_ignoring_indicators(engine, keycode,
						 shortcut->modifiers);
		shortcut++;
	}
}

void
xkl_xmm_shortcuts_ungrab(XklEngine * engine)
{
	const XmmShortcut *shortcut;
	const XmmSwitchOption *option =
	    xkl_xmm_shortcut_get_current(engine);

	if (option == NULL)
		return;

	shortcut = option->shortcuts;
	while (shortcut->keysym != XK_VoidSymbol) {
		int keycode =
		    XKeysymToKeycode(xkl_engine_get_display(engine),
				     shortcut->keysym);
		xkl_xmm_ungrab_ignoring_indicators(engine, keycode,
						   shortcut->modifiers);
		shortcut++;
	}
}

XmmSwitchOption *
xkl_xmm_shortcut_get_current(XklEngine * engine)
{
	const gchar *option_name =
	    xkl_xmm_shortcut_get_current_option_name(engine);

	xkl_debug(150, "Configured switch option: [%s]\n", option_name);

	if (option_name == NULL)
		return NULL;

	return (XmmSwitchOption *)
	    g_hash_table_lookup(xkl_engine_backend
				(engine, XklXmm, switch_options),
				(gconstpointer) option_name);
}

const gchar *
xkl_xmm_shortcut_get_current_option_name(XklEngine * engine)
{
	gchar **option =
	    xkl_engine_backend(engine, XklXmm, current_config).options;
	if (option == NULL)
		return NULL;

	while (*option != NULL) {
		/* starts with "grp:" */
		if (strstr(*option, SHORTCUT_OPTION_PREFIX) != NULL) {
			return *option + sizeof SHORTCUT_OPTION_PREFIX - 1;
		}
		option++;
	}
	return NULL;
}

const XmmSwitchOption *
xkl_xmm_find_switch_option(XklEngine * engine, gint keycode,
			   guint state, gint * current_shortcut_rv)
{
	const XmmSwitchOption *rv = xkl_xmm_shortcut_get_current(engine);

	if (rv != NULL) {
		const XmmShortcut *sc = rv->shortcuts;
		while (sc->keysym != XK_VoidSymbol) {
			if ((XKeysymToKeycode
			     (xkl_engine_get_display(engine),
			      sc->keysym) == keycode)
			    && ((state & sc->modifiers) == sc->modifiers)) {
				return rv;
			}
			sc++;
		}
	}
	return NULL;
}

gint
xkl_xmm_resume_listen(XklEngine * engine)
{
	if (xkl_engine_is_listening_for(engine, XKLL_MANAGE_LAYOUTS))
		xkl_xmm_shortcuts_grab(engine);
	return 0;
}

gint
xkl_xmm_pause_listen(XklEngine * engine)
{
	if (xkl_engine_is_listening_for(engine, XKLL_MANAGE_LAYOUTS))
		xkl_xmm_shortcuts_ungrab(engine);
	return 0;
}

guint
xkl_xmm_get_max_num_groups(XklEngine * engine)
{
	return 0;
}

guint
xkl_xmm_get_num_groups(XklEngine * engine)
{
	gint rv = 0;
	gchar **p =
	    xkl_engine_backend(engine, XklXmm, current_config).layouts;
	if (p != NULL)
		while (*p++ != NULL)
			rv++;
	return rv;
}

void
xkl_xmm_free_all_info(XklEngine * engine)
{
	gchar *current_rules =
	    xkl_engine_backend(engine, XklXmm, current_rules);
	if (current_rules != NULL) {
		g_free(current_rules);
		current_rules = NULL;
		xkl_engine_backend(engine, XklXmm, current_rules) = NULL;
	}
	xkl_config_rec_reset(&xkl_engine_backend
			     (engine, XklXmm, current_config));
}

gboolean
xkl_xmm_if_cached_info_equals_actual(XklEngine * engine)
{
	return FALSE;
}

gboolean
xkl_xmm_load_all_info(XklEngine * engine)
{
	return
	    xkl_config_rec_get_full_from_server(&xkl_engine_backend
						(engine, XklXmm,
						 current_rules),
						&xkl_engine_backend(engine,
								    XklXmm,
								    current_config),
						engine);
}

void
xkl_xmm_get_server_state(XklEngine * engine, XklState * state)
{
	unsigned char *propval = NULL;
	Atom actual_type;
	int actual_format;
	unsigned long bytes_remaining;
	unsigned long actual_items;
	int result;

	memset(state, 0, sizeof(*state));

	result =
	    XGetWindowProperty(xkl_engine_get_display(engine),
			       xkl_engine_priv(engine, root_window),
			       xkl_engine_backend(engine, XklXmm,
						  state_atom), 0L, 1L,
			       False, XA_INTEGER, &actual_type,
			       &actual_format, &actual_items,
			       &bytes_remaining, &propval);

	if (Success == result) {
		if (actual_format == 32 || actual_items == 1) {
			state->group = *(CARD32 *) propval;
		} else {
			xkl_debug(160,
				  "Could not get the xmodmap current group\n");
		}
		XFree(propval);
	} else {
		xkl_debug(160,
			  "Could not get the xmodmap current group: %d\n",
			  result);
	}
}

void
xkl_xmm_actualize_group(XklEngine * engine, gint group)
{
	char cmd[1024];
	int res;
	const gchar *layout_name = NULL;

	if (xkl_xmm_get_num_groups(engine) < group)
		return;

	layout_name =
	    xkl_engine_backend(engine, XklXmm,
			       current_config).layouts[group];

	g_snprintf(cmd, sizeof cmd,
		   "xmodmap %s/xmodmap.%s", XMODMAP_BASE, layout_name);

	res = system(cmd);
	if (res > 0) {
		xkl_debug(0, "xmodmap error %d\n", res);
	} else if (res < 0) {
		xkl_debug(0, "Could not execute xmodmap: %d\n", res);
	}
	XSync(xkl_engine_get_display(engine), False);
}

void
xkl_xmm_lock_group(XklEngine * engine, gint group)
{
	CARD32 propval;
	Display *display;

	if (xkl_xmm_get_num_groups(engine) < group)
		return;

	/* updating the status property */
	propval = group;
	display = xkl_engine_get_display(engine);
	XChangeProperty(display, xkl_engine_priv(engine, root_window),
			xkl_engine_backend(engine, XklXmm, state_atom),
			XA_INTEGER, 32, PropModeReplace,
			(unsigned char *) &propval, 1);
	XSync(display, False);
}

void
xkl_xmm_set_indicators(XklEngine * engine, const XklState * window_state)
{
}


gint
xkl_xmm_init(XklEngine * engine)
{
	Display *display;

	xkl_engine_priv(engine, backend_id) = "xmodmap";
	xkl_engine_priv(engine, features) =
	    XKLF_MULTIPLE_LAYOUTS_SUPPORTED |
	    XKLF_REQUIRES_MANUAL_LAYOUT_MANAGEMENT;
	xkl_engine_priv(engine, activate_config_rec) =
	    xkl_xmm_activate_config_rec;
	xkl_engine_priv(engine, init_config_registry) =
	    xkl_xmm_init_config_registry;
	xkl_engine_priv(engine, load_config_registry) =
	    xkl_xmm_load_config_registry;
	xkl_engine_priv(engine, write_config_rec_to_file) = NULL;

	xkl_engine_priv(engine, get_groups_names) =
	    xkl_xmm_get_groups_names;
	xkl_engine_priv(engine, get_indicators_names) =
	    xkl_xmm_get_indicators_names;
	xkl_engine_priv(engine, get_max_num_groups) =
	    xkl_xmm_get_max_num_groups;
	xkl_engine_priv(engine, get_num_groups) = xkl_xmm_get_num_groups;
	xkl_engine_priv(engine, lock_group) = xkl_xmm_lock_group;

	xkl_engine_priv(engine, process_x_event) = xkl_xmm_process_x_event;
	xkl_engine_priv(engine, process_x_error) = NULL;
	xkl_engine_priv(engine, free_all_info) = xkl_xmm_free_all_info;
	xkl_engine_priv(engine, if_cached_info_equals_actual) =
	    xkl_xmm_if_cached_info_equals_actual;
	xkl_engine_priv(engine, load_all_info) = xkl_xmm_load_all_info;
	xkl_engine_priv(engine, get_server_state) =
	    xkl_xmm_get_server_state;
	xkl_engine_priv(engine, pause_listen) = xkl_xmm_pause_listen;
	xkl_engine_priv(engine, resume_listen) = xkl_xmm_resume_listen;
	xkl_engine_priv(engine, set_indicators) = xkl_xmm_set_indicators;
	xkl_engine_priv(engine, finalize) = xkl_xmm_term;

	if (getenv("XKL_XMODMAP_DISABLE") != NULL)
		return -1;

	display = xkl_engine_get_display(engine);
	xkl_engine_priv(engine, base_config_atom) =
	    XInternAtom(display, "_XMM_NAMES", False);
	xkl_engine_priv(engine, backup_config_atom) =
	    XInternAtom(display, "_XMM_NAMES_BACKUP", False);

	xkl_engine_priv(engine, backend) = g_new0(XklXmm, 1);

	xkl_engine_backend(engine, XklXmm, state_atom) =
	    XInternAtom(display, "_XMM_STATE", False);

	xkl_engine_priv(engine, default_model) = "generic";
	xkl_engine_priv(engine, default_layout) = "us";

	xkl_xmm_init_switch_options((XklXmm *)
				    xkl_engine_priv(engine, backend));

	return 0;
}

void
xkl_xmm_term(XklEngine * engine)
{
	xkl_xmm_term_switch_options((XklXmm *)
				    xkl_engine_priv(engine, backend));
}
