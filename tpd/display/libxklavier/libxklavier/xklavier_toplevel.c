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
#include <string.h>

#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xklavier_private.h"

Window xkl_toplevel_window_prev;

void
xkl_engine_set_toplevel_window_transparent(XklEngine * engine,
					   Window toplevel_win,
					   gboolean transparent)
{
	gboolean oldval;

	oldval =
	    xkl_engine_is_toplevel_window_transparent(engine,
						      toplevel_win);
	xkl_debug(150, "toplevel_win " WINID_FORMAT " was %stransparent\n",
		  toplevel_win, oldval ? "" : "not ");
	if (transparent && !oldval) {
		CARD32 prop = 1;
		XChangeProperty(xkl_engine_get_display(engine),
				toplevel_win,
				xkl_engine_priv(engine, atoms)
				[XKLAVIER_TRANSPARENT], XA_INTEGER, 32,
				PropModeReplace,
				(const unsigned char *) &prop, 1);
	} else if (!transparent && oldval) {
		XDeleteProperty(xkl_engine_get_display(engine),
				toplevel_win,
				xkl_engine_priv(engine, atoms)
				[XKLAVIER_TRANSPARENT]);
	}
}

/*
 * "Adds" app window to the set of managed windows.
 * Actually, no data structures involved. The only thing we do is save app state
 * and register ourselves us listeners.
 * Note: User's callback is called
 */
void
xkl_engine_add_toplevel_window(XklEngine * engine, Window toplevel_win,
			       Window parent,
			       gboolean ignore_existing_state,
			       XklState * init_state)
{
	XklState state = *init_state;
	gint default_group_to_use = -1;
	GValue params[3];
	GValue rv;
	guint signal_id;

	if (toplevel_win == xkl_engine_priv(engine, root_window))
		xkl_debug(150, "??? root app win ???\n");

	xkl_debug(150,
		  "Trying to add window " WINID_FORMAT
		  "/%s with group %d\n", toplevel_win,
		  xkl_get_debug_window_title(engine, toplevel_win),
		  init_state->group);

	if (!ignore_existing_state) {
		gboolean have_state =
		    xkl_engine_get_toplevel_window_state(engine,
							 toplevel_win,
							 &state);

		if (have_state) {
			xkl_debug(150,
				  "The window " WINID_FORMAT
				  " does not require to be added, it already has the xklavier state \n",
				  toplevel_win);
			return;
		}
	}
	memset(params, 0, sizeof(params));
	g_value_init(params, XKL_TYPE_ENGINE);
	g_value_set_object(params, engine);
	g_value_init(params + 1, G_TYPE_LONG);
	g_value_set_long(params + 1, toplevel_win);
	g_value_init(params + 2, G_TYPE_LONG);
	g_value_set_long(params + 2, parent);

	memset(&rv, 0, sizeof(rv));
	g_value_init(&rv, G_TYPE_INT);
	g_value_set_int(&rv, default_group_to_use);

	signal_id =
	    g_signal_lookup("new-toplevel-window", xkl_engine_get_type());
	g_signal_emitv(params, signal_id, 0, &rv);
	default_group_to_use = g_value_get_int(&rv);
        
	if (default_group_to_use == -1) {
		Window transient_for = 0;
		if (XGetTransientForHint(xkl_engine_get_display(engine), toplevel_win, &transient_for)) {
			if (transient_for) {
				XklState trans_state;
				gboolean have_state =
					xkl_engine_get_toplevel_window_state(engine,
							 transient_for,
							 &trans_state);
				if (have_state) {
					default_group_to_use = trans_state.group;
				}
			}
		}
	}

	if (default_group_to_use == -1)
		default_group_to_use =
		    xkl_engine_priv(engine, default_group);

	if (default_group_to_use != -1)
		state.group = default_group_to_use;

	xkl_engine_save_toplevel_window_state(engine, toplevel_win,
					      &state);
	xkl_engine_select_input_merging(engine, toplevel_win,
					FocusChangeMask |
					PropertyChangeMask);

	if (default_group_to_use != -1) {
		if (xkl_engine_priv(engine, curr_toplvl_win) ==
		    toplevel_win) {
			if ((xkl_engine_priv(engine, secondary_groups_mask)
			     & (1 << default_group_to_use)) != 0)
				xkl_engine_allow_one_switch_to_secondary_group
				    (engine);
			xkl_engine_lock_group(engine,
					      default_group_to_use);
		}
	}

	if (parent == (Window) NULL)
		parent =
		    xkl_engine_get_registered_parent(engine, toplevel_win);

	xkl_debug(150, "done\n");
}

/*
 * Checks the window and goes up
 */
gboolean
xkl_engine_find_toplevel_window_bottom_to_top(XklEngine * engine,
					      Window win,
					      Window * toplevel_win_out)
{
	Window parent = (Window) NULL, rwin = (Window) NULL, *children =
	    NULL;
	guint num = 0;

	if (win == (Window) NULL
	    || win == xkl_engine_priv(engine, root_window)) {
		*toplevel_win_out = win;
		xkl_last_error_message = "The window is either 0 or root";
		return FALSE;
	}

	if (xkl_engine_if_window_has_wm_state(engine, win)) {
		*toplevel_win_out = win;
		return TRUE;
	}

	xkl_engine_priv(engine, last_error_code) =
	    xkl_engine_query_tree(engine, win, &rwin, &parent, &children,
				  &num);

	if (xkl_engine_priv(engine, last_error_code) != Success) {
		*toplevel_win_out = (Window) NULL;
		return FALSE;
	}

	if (children != NULL)
		XFree(children);

	return xkl_engine_find_toplevel_window_bottom_to_top(engine,
							     parent,
							     toplevel_win_out);
}

/*
 * Recursively finds "App window" (window with WM_STATE) for given window.
 * First, checks the window itself
 * Then, for first level of recursion, checks childen,
 * Then, goes to parent.
 * NOTE: root window cannot be "App window" under normal circumstances
 */
gboolean
xkl_engine_find_toplevel_window(XklEngine * engine, Window win,
				Window * toplevel_win_out)
{
	Window parent = (Window) NULL,
	    rwin = (Window) NULL, *children = NULL, *child;
	guint num = 0;
	gboolean rv;

	if (win == (Window) NULL
	    || win == PointerRoot
	    || win == xkl_engine_priv(engine, root_window)) {
		*toplevel_win_out = (Window) NULL;
		xkl_last_error_message = "The window is either 0 or root";
		xkl_debug(150,
			  "Window " WINID_FORMAT
			  " is either 0 or root so could not get the app window for it\n",
			  win);
		return FALSE;
	}

	if (xkl_engine_if_window_has_wm_state(engine, win)) {
		*toplevel_win_out = win;
		return TRUE;
	}

	xkl_engine_priv(engine, last_error_code) =
	    xkl_engine_query_tree(engine, win, &rwin, &parent, &children,
				  &num);

	if (xkl_engine_priv(engine, last_error_code) != Success) {
		*toplevel_win_out = (Window) NULL;
		xkl_debug(150,
			  "Could not get tree for window " WINID_FORMAT
			  " so could not get the app window for it\n",
			  win);
		return FALSE;
	}

	/*
	 * Here we first check the children (in case win is just above some "App Window")
	 * and then go upstairs
	 */
	child = children;
	while (num) {
		if (xkl_engine_if_window_has_wm_state(engine, *child)) {
			*toplevel_win_out = *child;
			if (children != NULL)
				XFree(children);
			return TRUE;
		}
		child++;
		num--;
	}

	if (children != NULL)
		XFree(children);

	rv = xkl_engine_find_toplevel_window_bottom_to_top(engine, parent,
							   toplevel_win_out);

	if (!rv)
		xkl_debug(200,
			  "Could not get the app window for " WINID_FORMAT
			  "/%s\n", win, xkl_get_debug_window_title(engine,
								   win));

	return rv;
}

/*
 * Gets the state from the window property
 */
gboolean
xkl_engine_get_toplevel_window_state(XklEngine * engine,
				     Window toplevel_win,
				     XklState * state_out)
{
	Atom type_ret;
	int format_ret;
	unsigned long nitems, rest;
	CARD32 *prop = NULL;
	gboolean ret = FALSE;

	gint grp = -1;
	guint inds = 0;

	if ((XGetWindowProperty
	     (xkl_engine_get_display(engine), toplevel_win,
	      xkl_engine_priv(engine, atoms)[XKLAVIER_STATE], 0L,
	      XKLAVIER_STATE_PROP_LENGTH, False, XA_INTEGER, &type_ret,
	      &format_ret, &nitems, &rest,
	      (unsigned char **) (void *) &prop) == Success)
	    && (type_ret == XA_INTEGER) && (format_ret == 32)) {
		grp = prop[0];
		if (grp >= xkl_engine_get_num_groups(engine) || grp < 0)
			grp = 0;

		inds = prop[1];

		if (state_out != NULL) {
			state_out->group = grp;
			state_out->indicators = inds;
		}
		if (prop != NULL)
			XFree(prop);

		ret = TRUE;
	}

	if (ret)
		xkl_debug(150,
			  "Appwin " WINID_FORMAT
			  ", '%s' has the group %d, indicators %X\n",
			  toplevel_win,
			  xkl_get_debug_window_title(engine, toplevel_win),
			  grp, inds);
	else
		xkl_debug(150,
			  "Appwin " WINID_FORMAT
			  ", '%s' does not have state\n", toplevel_win,
			  xkl_get_debug_window_title(engine,
						     toplevel_win));

	return ret;
}

/*
 * Deletes the state from the window properties
 */
void
xkl_engine_remove_toplevel_window_state(XklEngine * engine,
					Window toplevel_win)
{
	XDeleteProperty(xkl_engine_get_display(engine), toplevel_win,
			xkl_engine_priv(engine, atoms)[XKLAVIER_STATE]);
}

/*
 * Saves the state into the window properties
 */
void
xkl_engine_save_toplevel_window_state(XklEngine * engine,
				      Window toplevel_win,
				      XklState * state)
{
	CARD32 prop[XKLAVIER_STATE_PROP_LENGTH];

	prop[0] = state->group;
	prop[1] = state->indicators;

	XChangeProperty(xkl_engine_get_display(engine), toplevel_win,
			xkl_engine_priv(engine, atoms)[XKLAVIER_STATE],
			XA_INTEGER, 32, PropModeReplace,
			(const unsigned char *) prop,
			XKLAVIER_STATE_PROP_LENGTH);

	xkl_debug(160,
		  "Saved the group %d, indicators %X for appwin "
		  WINID_FORMAT "\n", state->group, state->indicators,
		  toplevel_win);
}

gboolean
xkl_engine_is_toplevel_window_transparent(XklEngine * engine,
					  Window toplevel_win)
{
	Atom type_ret;
	int format_ret;
	unsigned long nitems, rest;
	CARD32 *prop = NULL;
	if ((XGetWindowProperty
	     (xkl_engine_get_display(engine), toplevel_win,
	      xkl_engine_priv(engine, atoms)[XKLAVIER_TRANSPARENT], 0L, 1,
	      False, XA_INTEGER, &type_ret, &format_ret, &nitems, &rest,
	      (unsigned char **) (void *) &prop) == Success)
	    && (type_ret == XA_INTEGER) && (format_ret == 32)) {
		if (prop != NULL)
			XFree(prop);
		return TRUE;
	}
	return FALSE;
}
