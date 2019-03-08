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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xklavier_private.h"

XklState *
_xkl_state_copy(XklState * state)
{
	XklState * copy;

	copy = g_new(XklState, 1);
	copy->group = state->group;
	copy->indicators = state->indicators;

	return copy;
}

G_DEFINE_BOXED_TYPE (XklState, xkl_state, _xkl_state_copy, g_free);

XklState *
xkl_engine_get_current_state(XklEngine * engine)
{
	return &xkl_engine_priv(engine, curr_state);
}

const gchar *
xkl_get_last_error()
{
	return xkl_last_error_message;
}

gchar *
xkl_engine_get_window_title(XklEngine * engine, Window w)
{
	Atom type_ret;
	int format_ret;
	unsigned long nitems, rest;
	unsigned char *prop;

	if ((w == xkl_engine_priv(engine, root_window))
	    || (w == PointerRoot))
		return g_strdup("ROOT");

	if (Success ==
	    XGetWindowProperty(xkl_engine_get_display(engine), w,
			       xkl_engine_priv(engine, atoms)[WM_NAME], 0L,
			       -1L, False, XA_STRING, &type_ret,
			       &format_ret, &nitems, &rest, &prop))
		return (gchar *) prop;
	else
		return NULL;
}

gboolean
xkl_engine_is_window_from_same_toplevel_window(XklEngine * engine,
					       Window win1, Window win2)
{
	Window app1, app2;
	return xkl_engine_find_toplevel_window(engine, win1, &app1) &&
	    xkl_engine_find_toplevel_window(engine, win2, &app2)
	    && app1 == app2;
}

gboolean
xkl_engine_get_state(XklEngine * engine, Window win, XklState * state_out)
{
	Window app_win;

	if (!xkl_engine_find_toplevel_window(engine, win, &app_win)) {
		if (state_out != NULL)
			state_out->group = -1;
		return FALSE;
	}

	return xkl_engine_get_toplevel_window_state(engine, app_win,
						    state_out);
}

void
xkl_engine_delete_state(XklEngine * engine, Window win)
{
	Window app_win;

	if (xkl_engine_find_toplevel_window(engine, win, &app_win))
		xkl_engine_remove_toplevel_window_state(engine, app_win);
}

void
xkl_engine_save_state(XklEngine * engine, Window win, XklState * state)
{
	Window app_win;

	if (!
	    (xkl_engine_is_listening_for
	     (engine, XKLL_MANAGE_WINDOW_STATES)))
		return;

	if (xkl_engine_find_toplevel_window(engine, win, &app_win))
		xkl_engine_save_toplevel_window_state(engine, app_win,
						      state);
}

/*
 *  Prepares the name of window suitable for debugging (32characters long).
 */
gchar *
xkl_get_debug_window_title(XklEngine * engine, Window win)
{
	static gchar sname[33];
	gchar *name;
	strcpy(sname, "NULL");
	if (win != (Window) NULL) {
		name = xkl_engine_get_window_title(engine, win);
		if (name != NULL) {
			g_snprintf(sname, sizeof(sname), "%.32s", name);
			g_free(name);
		}
	}
	return sname;
}

Window
xkl_engine_get_current_window(XklEngine * engine)
{
	return xkl_engine_priv(engine, curr_toplvl_win);
}

/*
 * Loads subtree. 
 * All the windows with WM_STATE are added.
 * All the windows within level 0 are listened for focus and property
 */
gboolean
xkl_engine_load_subtree(XklEngine * engine, Window window, gint level,
			XklState * init_state)
{
	Window rwin = (Window) NULL,
	    parent = (Window) NULL, *children = NULL, *child;
	guint num = 0;
	gboolean retval = True;

	xkl_engine_priv(engine, last_error_code) =
	    xkl_engine_query_tree(engine, window, &rwin, &parent,
				  &children, &num);

	if (xkl_engine_priv(engine, last_error_code) != Success) {
		return FALSE;
	}

	child = children;
	while (num) {
		if (xkl_engine_if_window_has_wm_state(engine, *child)) {
			xkl_debug(160,
				  "Window " WINID_FORMAT
				  " '%s' has WM_STATE so we'll add it\n",
				  *child,
				  xkl_get_debug_window_title(engine,
							     *child));
			xkl_engine_add_toplevel_window(engine, *child,
						       window, TRUE,
						       init_state);
		} else {
			xkl_debug(200,
				  "Window " WINID_FORMAT
				  " '%s' does not have have WM_STATE so we'll not add it\n",
				  *child,
				  xkl_get_debug_window_title(engine,
							     *child));

			if (level == 0) {
				xkl_debug(200,
					  "But we are at level 0 so we'll spy on it\n");
				xkl_engine_select_input_merging(engine,
								*child,
								FocusChangeMask
								|
								PropertyChangeMask);
			} else
				xkl_debug(200,
					  "And we are at level %d so we'll not spy on it\n",
					  level);

			retval =
			    xkl_engine_load_subtree(engine, *child,
						    level + 1, init_state);
		}

		child++;
		num--;
	}

	if (children != NULL)
		XFree(children);

	return retval;
}

/*
 * Checks whether given window has WM_STATE property (i.e. "App window").
 */
gboolean
xkl_engine_if_window_has_wm_state(XklEngine * engine, Window win)
{				/* ICCCM 4.1.3.1 */
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long after;
	unsigned char *data = NULL;	/* Helps in the case of BadWindow error */

	XGetWindowProperty(xkl_engine_get_display(engine), win,
			   xkl_engine_priv(engine, atoms)[WM_STATE], 0, 0,
			   False, xkl_engine_priv(engine, atoms)[WM_STATE],
			   &type, &format, &nitems, &after, &data);
	if (data != NULL)
		XFree(data);	/* To avoid an one-byte memory leak because after successfull return
				 * data array always contains at least one nul byte (NULL-equivalent) */
	return type != None;
}

/*
 * Finds out the official parent window (accortind to XQueryTree)
 */
Window
xkl_engine_get_registered_parent(XklEngine * engine, Window win)
{
	Window parent = (Window) NULL, rw = (Window) NULL, *children =
	    NULL;
	guint nchildren = 0;

	xkl_engine_priv(engine, last_error_code) =
	    xkl_engine_query_tree(engine, win, &rw, &parent, &children,
				  &nchildren);

	if (children != NULL)
		XFree(children);

	return xkl_engine_priv(engine, last_error_code) ==
	    Success ? parent : (Window) NULL;
}

/**
 * Make sure about the result. Origial XQueryTree is pretty stupid beast:)
 */
Status
xkl_engine_query_tree(XklEngine * engine, Window w,
		      Window * root_out,
		      Window * parent_out,
		      Window ** children_out, guint * nchildren_out)
{
	gboolean result;
	unsigned int nc;

	result = (gboolean) XQueryTree(xkl_engine_get_display(engine),
				       w,
				       root_out,
				       parent_out, children_out, &nc);
	*nchildren_out = nc;

	if (!result) {
		xkl_debug(160,
			  "Could not get tree info for window "
			  WINID_FORMAT ": %d\n", w, result);
		xkl_last_error_message = "Could not get the tree info";
	}

	return result ? Success : FirstExtensionError;
}

const gchar *
xkl_event_get_name(gint type)
{
	/* Not really good to use the fact of consecutivity
	   but X protocol is already standartized so... */
	static const gchar *evt_names[] = {
		"KeyPress",
		"KeyRelease",
		"ButtonPress",
		"ButtonRelease",
		"MotionNotify",
		"EnterNotify",
		"LeaveNotify",
		"FocusIn",
		"FocusOut",
		"KeymapNotify",
		"Expose",
		"GraphicsExpose",
		"NoExpose",
		"VisibilityNotify",
		"CreateNotify",
		"DestroyNotify",
		"UnmapNotify",
		"MapNotify",
		"MapRequest",
		"ReparentNotify",
		"ConfigureNotify",
		"ConfigureRequest",
		"GravityNotify",
		"ResizeRequest",
		"CirculateNotify",
		"CirculateRequest",
		"PropertyNotify",
		"SelectionClear",
		"SelectionRequest",
		"SelectionNotify",
		"ColormapNotify", "ClientMessage", "MappingNotify",
		"LASTEvent"
	};
	type -= KeyPress;
	if (type < 0 || type >= (sizeof(evt_names) / sizeof(evt_names[0])))
		return "UNKNOWN";
	return evt_names[type];
}

void
xkl_engine_update_current_state(XklEngine * engine, int group,
				unsigned indicators, const char reason[])
{
	xkl_debug(150,
		  "Updating the current state with [g:%d/i:%u], reason: %s\n",
		  group, indicators, reason);
	xkl_engine_priv(engine, curr_state).group = group;
	xkl_engine_priv(engine, curr_state).indicators = indicators;
}
