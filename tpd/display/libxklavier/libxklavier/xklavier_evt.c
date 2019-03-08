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

#include <string.h>
#include <time.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xklavier_private.h"

gint
xkl_engine_filter_events(XklEngine * engine, XEvent * xev)
{
	XAnyEvent *pe = (XAnyEvent *) xev;
	xkl_debug(400,
		  "**> Filtering event %d of type %d from window %d\n",
		  pe->serial, pe->type, pe->window);
	xkl_engine_ensure_vtable_inited(engine);
	if (!xkl_engine_vcall(engine, process_x_event) (engine, xev))
		switch (xev->type) {	/* core events */
		case FocusIn:
			xkl_engine_process_focus_in_evt(engine,
							&xev->xfocus);
			break;
		case FocusOut:
			xkl_engine_process_focus_out_evt(engine,
							 &xev->xfocus);
			break;
		case PropertyNotify:
			xkl_engine_process_property_evt(engine,
							&xev->xproperty);
			break;
		case CreateNotify:
			xkl_engine_process_create_window_evt(engine,
							     &xev->xcreatewindow);
			break;
		case DestroyNotify:
			xkl_debug(150,
				  "Window " WINID_FORMAT " destroyed\n",
				  xev->xdestroywindow.window);
			break;
		case UnmapNotify:
			xkl_debug(200,
				  "Window " WINID_FORMAT " unmapped\n",
				  xev->xunmap.window);
			break;
		case MapNotify:
		case GravityNotify:
			xkl_debug(200, "%s\n",
				  xkl_event_get_name(xev->type));
			break;	/* Ignore these events */
		case ReparentNotify:
			xkl_debug(200,
				  "Window " WINID_FORMAT " reparented to "
				  WINID_FORMAT "\n", xev->xreparent.window,
				  xev->xreparent.parent);
			break;	/* Ignore these events */
		case MappingNotify:
			xkl_debug(200, "%s\n",
				  xkl_event_get_name(xev->type));
			xkl_engine_reset_all_info(engine, FALSE,
						  "X event: MappingNotify");
			break;
		default:
			{
				xkl_debug(200, "Unknown event %d [%s]\n",
					  xev->type,
					  xkl_event_get_name(xev->type));
				return 1;
			}
		}
	xkl_debug(400, "Filtered event %d of type %d from window %d **>\n",
		  pe->serial, pe->type, pe->window);
	return 1;
}

/*
 * FocusIn handler
 */
void
xkl_engine_process_focus_in_evt(XklEngine * engine,
				XFocusChangeEvent * fev)
{
	Window win;
	Window prev_toplevel_win =
	    xkl_engine_priv(engine, curr_toplvl_win);
	Window toplevel_win;
	XklState selected_window_state;

	if (!
	    (xkl_engine_is_listening_for
	     (engine, XKLL_MANAGE_WINDOW_STATES)))
		return;

	win = fev->window;

	switch (fev->mode) {
	case NotifyNormal:
	case NotifyWhileGrabbed:
		break;
	default:
		xkl_debug(160,
			  "Window " WINID_FORMAT
			  " has got focus during special action %d\n", win,
			  fev->mode);
		return;
	}

	xkl_debug(150, "Window " WINID_FORMAT ", '%s' has got focus\n",
		  win, xkl_get_debug_window_title(engine, win));

	if (!xkl_engine_find_toplevel_window(engine, win, &toplevel_win)) {
		return;
	}

	xkl_debug(150, "Appwin " WINID_FORMAT ", '%s' has got focus\n",
		  toplevel_win, xkl_get_debug_window_title(engine,
							   toplevel_win));

	if (xkl_engine_get_toplevel_window_state
	    (engine, toplevel_win, &selected_window_state)) {
		if (prev_toplevel_win != toplevel_win) {
			gboolean new_win_transparent;
			Window parent = (Window) NULL, root =
			    (Window) NULL, *children = NULL;
			guint nchildren = 0;

			/*
			 * If previous focused window exists - handle transparency and state
			 * (optional)
			 */
			if (xkl_engine_query_tree
			    (engine, prev_toplevel_win, &root, &parent,
			     &children, &nchildren) == Success) {
				XklState tmp_state;
				gboolean old_win_transparent =
				    xkl_engine_is_toplevel_window_transparent
				    (engine, prev_toplevel_win);

				if (children != NULL)
					XFree(children);

				if (old_win_transparent)
					xkl_debug(150,
						  "Leaving transparent window\n");
				/*
				 * Reload the current state from the current window. 
				 * Do not do it for transparent window - we keep the state from 
				 * the _previous_ window.
				 */
				if (!old_win_transparent
				    &&
				    xkl_engine_get_toplevel_window_state
				    (engine, prev_toplevel_win,
				     &tmp_state)) {
					xkl_engine_update_current_state
					    (engine, tmp_state.group,
					     tmp_state.indicators,
					     "Loading current (previous) state from the current (previous) window");
				}
			} else
				xkl_debug(150,
					  "Current (previous) window "
					  WINID_FORMAT
					  " does not exist any more, so transparency/state are not analyzed\n",
					  prev_toplevel_win);

			xkl_engine_priv(engine, curr_toplvl_win) =
			    toplevel_win;
			xkl_debug(150,
				  "CurClient:changed to " WINID_FORMAT
				  ", '%s'\n", xkl_engine_priv(engine,
							      curr_toplvl_win),
				  xkl_get_debug_window_title(engine,
							     xkl_engine_priv
							     (engine,
							      curr_toplvl_win)));

			new_win_transparent =
			    xkl_engine_is_toplevel_window_transparent
			    (engine, toplevel_win);
			if (new_win_transparent)
				xkl_debug(150,
					  "Entering transparent window\n");

			if (xkl_engine_is_group_per_toplevel_window(engine)
			    == !new_win_transparent) {
				/* We skip restoration only if we return to the same app window */
				gboolean do_skip = FALSE;
				if (xkl_engine_priv
				    (engine, skip_one_restore)) {
					xkl_engine_priv(engine,
							skip_one_restore) =
					    FALSE;
					if (toplevel_win ==
					    xkl_engine_priv(engine,
							    prev_toplvl_win))
						do_skip = TRUE;
				}

				if (do_skip) {
					xkl_debug(150,
						  "Skipping one restore as requested - instead, "
						  "saving the current group into the window state\n");
					xkl_engine_save_toplevel_window_state
					    (engine, toplevel_win,
					     &xkl_engine_priv(engine,
							      curr_state));
				} else {
					if (xkl_engine_priv
					    (engine,
					     curr_state).group !=
					    selected_window_state.group) {
						xkl_debug(150,
							  "Restoring the group from %d to %d after gaining focus\n",
							  xkl_engine_priv
							  (engine,
							   curr_state).group,
							  selected_window_state.group);
						/*
						 *  For fast mouse movements - the state is probably not updated yet
						 *  (because of the group change notification being late).
						 *  so we'll enforce the update. But this should only happen in GPA mode
						 */
						xkl_engine_update_current_state
						    (engine,
						     selected_window_state.group,
						     selected_window_state.indicators,
						     "Enforcing fast update of the current state");
						xkl_engine_lock_group
						    (engine,
						     selected_window_state.group);
						xkl_engine_priv(engine, skip_one_save) = TRUE;
					} else {
						xkl_debug(150,
							  "Both old and new focused window "
							  "have group %d so no point restoring it\n",
							  selected_window_state.group);
						xkl_engine_one_switch_to_secondary_group_performed
						    (engine);
					}
				}

				if ((xkl_engine_priv(engine, features) &
				     XKLF_CAN_TOGGLE_INDICATORS)
				    &&
				    xkl_engine_get_indicators_handling
				    (engine)) {
					xkl_debug(150,
						  "Restoring the indicators from %X to %X after gaining focus\n",
						  xkl_engine_priv(engine,
								  curr_state).indicators,
						  selected_window_state.indicators);
					xkl_engine_ensure_vtable_inited
					    (engine);
					xkl_engine_vcall(engine,
							 set_indicators)
					    (engine,
					     &selected_window_state);
				} else
					xkl_debug(150,
						  "Not restoring the indicators %X after gaining focus: indicator handling is not enabled\n",
						  xkl_engine_priv(engine,
								  curr_state).indicators);
			} else
				xkl_debug(150,
					  "Not restoring the group %d after gaining focus: global layout (xor transparent window)\n",
					  xkl_engine_priv(engine,
							  curr_state).group);
		} else
			xkl_debug(150,
				  "Same app window - just do nothing\n");
	} else {
		xkl_debug(150, "But it does not have xklavier_state\n");
		if (xkl_engine_if_window_has_wm_state(engine, win)) {
			xkl_debug(150,
				  "But it does have wm_state so we'll add it\n");
			xkl_engine_priv(engine, curr_toplvl_win) =
			    toplevel_win;
			xkl_debug(150,
				  "CurClient:changed to " WINID_FORMAT
				  ", '%s'\n", xkl_engine_priv(engine,
							      curr_toplvl_win),
				  xkl_get_debug_window_title(engine,
							     xkl_engine_priv
							     (engine,
							      curr_toplvl_win)));
			xkl_engine_add_toplevel_window(engine,
						       xkl_engine_priv
						       (engine,
							curr_toplvl_win),
						       (Window) NULL,
						       FALSE,
						       &xkl_engine_priv
						       (engine,
							curr_state));
		} else
			xkl_debug(150,
				  "And it does have wm_state either\n");
	}
}

/* 
 * FocusOut handler
 */
void
xkl_engine_process_focus_out_evt(XklEngine * engine,
				 XFocusChangeEvent * fev)
{
	if (!
	    (xkl_engine_is_listening_for
	     (engine, XKLL_MANAGE_WINDOW_STATES)))
		return;

	if (fev->mode != NotifyNormal) {
		xkl_debug(200,
			  "Window " WINID_FORMAT
			  " has lost focus during special action %d\n",
			  fev->window, fev->mode);
		return;
	}

	xkl_debug(160, "Window " WINID_FORMAT ", '%s' has lost focus\n",
		  fev->window, xkl_get_debug_window_title(engine,
							  fev->window));

	if (xkl_engine_is_toplevel_window_transparent(engine, fev->window)) {
		xkl_debug(150, "Leaving transparent window!\n");
/* 
 * If we are leaving the transparent window - we skip the restore operation.
 * This is useful for secondary groups switching from the transparent control 
 * window.
 */
		xkl_engine_priv(engine, skip_one_restore) = TRUE;
	} else {
		Window p;
		if (xkl_engine_find_toplevel_window
		    (engine, fev->window, &p))
			xkl_engine_priv(engine, prev_toplvl_win) = p;
	}
}

/*
 * PropertyChange handler
 * Interested in :
 *  + for XKLL_MANAGE_WINDOW_STATES
 *    - WM_STATE property for all windows
 *    - Configuration property of the root window
 *  + for XKLL_TRACK_KEYBOARD_STATE
 *    - Configuration property of the root window
 */
void
xkl_engine_process_property_evt(XklEngine * engine, XPropertyEvent * pev)
{
	if (400 <= xkl_debug_level) {
		char *atom_name =
		    XGetAtomName(xkl_engine_get_display(engine),
				 pev->atom);
		if (atom_name != NULL) {
			xkl_debug(400,
				  "The property '%s' changed for "
				  WINID_FORMAT "\n", atom_name,
				  pev->window);
			XFree(atom_name);
		} else {
			xkl_debug(200,
				  "Some magic property changed for "
				  WINID_FORMAT "\n", pev->window);
		}
	}

	if (pev->atom == xkl_engine_priv(engine, atoms)[WM_STATE]) {
		if (xkl_engine_is_listening_for
		    (engine, XKLL_MANAGE_WINDOW_STATES)) {
			gboolean has_xkl_state =
			    xkl_engine_get_state(engine, pev->window,
						 NULL);

			if (pev->state == PropertyNewValue) {
				xkl_debug(160,
					  "New value of WM_STATE on window "
					  WINID_FORMAT "\n", pev->window);
				if (!has_xkl_state) {	/* Is this event the first or not? */
					xkl_engine_add_toplevel_window
					    (engine, pev->window, (Window)
					     NULL, FALSE,
					     &xkl_engine_priv(engine,
							      curr_state));
				}
			} else {	/* ev->xproperty.state == PropertyDelete, either client or WM can remove it, ICCCM 4.1.3.1 */
				xkl_debug(160,
					  "Something (%d) happened to WM_STATE of window 0x%x\n",
					  pev->state, pev->window);
				xkl_engine_select_input_merging(engine,
								pev->window,
								PropertyChangeMask);
				if (has_xkl_state) {
					xkl_engine_delete_state(engine,
								pev->window);
				}
			}
		}		/* XKLL_MANAGE_WINDOW_STATES */
	} else if (pev->atom == xkl_engine_priv(engine, base_config_atom)
		   && pev->window == xkl_engine_priv(engine, root_window)) {
		if (xkl_engine_is_listening_for
		    (engine,
		     XKLL_MANAGE_WINDOW_STATES) |
		    xkl_engine_is_listening_for(engine,
						XKLL_TRACK_KEYBOARD_STATE))
		{
			if (pev->state == PropertyNewValue) {
				/* If root window got new *_NAMES_PROP_ATOM -
				   it most probably means new keyboard config is loaded by somebody */
				xkl_engine_reset_all_info
				    (engine, TRUE,
				     "New value of *_NAMES_PROP_ATOM on root window");
			}
		}		/* XKLL_MANAGE_WINDOW_STATES | XKLL_TRACK_KEYBOARD_STATE */
	}
}

/*
 * CreateNotify handler. Just interested in properties and focus events...
 */
void
xkl_engine_process_create_window_evt(XklEngine * engine,
				     XCreateWindowEvent * cev)
{
	if (!xkl_engine_is_listening_for
	    (engine, XKLL_MANAGE_WINDOW_STATES))
		return;

	xkl_debug(200,
		  "Under-root window " WINID_FORMAT
		  "/%s (%d,%d,%d x %d) is created\n", cev->window,
		  xkl_get_debug_window_title(engine, cev->window), cev->x,
		  cev->y, cev->width, cev->height);

	if (!cev->override_redirect) {
/* ICCCM 4.1.6: override-redirect is NOT private to
* client (and must not be changed - am I right?) 
* We really need only PropertyChangeMask on this window but even in the case of
* local server we can lose PropertyNotify events (the trip time for this CreateNotify
* event + SelectInput request is not zero) and we definitely will (my system DO)
* lose FocusIn/Out events after the following call of PropertyNotifyHandler.
* So I just decided to purify this extra FocusChangeMask in the FocusIn/OutHandler. */
		xkl_engine_select_input_merging(engine, cev->window,
						PropertyChangeMask |
						FocusChangeMask);

		if (xkl_engine_if_window_has_wm_state(engine, cev->window)) {
			xkl_debug(200,
				  "Just created window already has WM_STATE - so I'll add it");
			xkl_engine_add_toplevel_window(engine, cev->window,
						       (Window) NULL,
						       FALSE,
						       &xkl_engine_priv
						       (engine,
							curr_state));
		}
	}
}

/*
 * Just error handler - sometimes we get BadWindow error for already gone 
 * windows, so we'll just ignore
 * This handler can be called in the middle of the engine initialization -
 * so it is not fair to assume that the engine is available
 */
int
xkl_process_error(Display * dpy, XErrorEvent * evt)
{
	char buf[128] = "";
	XklEngine *engine = xkl_get_the_engine();

	if (engine != NULL)
		xkl_engine_priv(engine, last_error_code) = evt->error_code;

	switch (evt->error_code) {
	case BadAccess:
	case BadDrawable:
	case BadWindow:
	case BadMatch:
		{
			XGetErrorText(evt->display, evt->error_code, buf,
				      sizeof(buf));
			/* in most cases this means we are late:) */
			xkl_debug(200,
				  "ERROR: %p, " WINID_FORMAT ", %d [%s], "
				  "X11 request: %d, minor code: %d\n", dpy,
				  (unsigned long) evt->resourceid,
				  (int) evt->error_code, buf,
				  (int) evt->request_code,
				  (int) evt->minor_code);
			break;
		}
	default:
		if (engine != NULL
		    && xkl_engine_priv(engine, process_x_error)) {
			if (xkl_engine_priv(engine, process_x_error)
			    (engine, evt)) {
				xkl_debug(200,
					  "X ERROR processed by the engine: %p, "
					  WINID_FORMAT ", %d [%s], "
					  "X11 request: %d, minor code: %d\n",
					  dpy,
					  (unsigned long) evt->resourceid,
					  (int) evt->error_code, buf,
					  (int) evt->request_code,
					  (int) evt->minor_code);
				break;
			}
		}
		xkl_debug(200,
			  "Unexpected by libxklavier X ERROR: %p, "
			  WINID_FORMAT ", %d [%s], "
			  "X11 request: %d, minor code: %d\n", dpy,
			  (unsigned long) evt->resourceid,
			  (int) evt->error_code, buf,
			  (int) evt->request_code, (int) evt->minor_code);
		if (engine != NULL)
			if (!xkl_engine_priv(engine, critical_section))
				(*xkl_engine_priv
				 (engine, default_error_handler))
				    (dpy, evt);
	}

	/* X ignores this return value anyway */
	return 0;
}

/*
 * Some common functionality for Xkb handler
 */
void
xkl_engine_process_state_modification(XklEngine * engine,
				      XklEngineStateChange change_type,
				      gint grp, guint inds,
				      gboolean set_inds)
{
	Window focused, focused_toplevel;
	XklState old_state;
	gint revert;
	gboolean have_old_state = TRUE;
	gboolean set_group = change_type == GROUP_CHANGED;

	if (xkl_engine_priv(engine, skip_one_save)) {
		xkl_debug(160, "Skipping one callback");
		xkl_engine_priv(engine, skip_one_save) = FALSE;
		return;
	}

	XGetInputFocus(xkl_engine_get_display(engine), &focused, &revert);

	if ((focused == None) || (focused == PointerRoot)) {
		xkl_debug(160, "Something with focus: " WINID_FORMAT "\n",
			  focused);
		return;
	}

	/* 
	 * Only if we manage states - otherwise xkl_engine_priv(engine,curr_toplvl_win) does not make sense 
	 */
	if (!xkl_engine_find_toplevel_window
	    (engine, focused, &focused_toplevel)
	    && xkl_engine_is_listening_for(engine,
					   XKLL_MANAGE_WINDOW_STATES))
		focused_toplevel = xkl_engine_priv(engine, curr_toplvl_win);	/* what else can I do */

	xkl_debug(150, "Focused window: " WINID_FORMAT ", '%s'\n",
		  focused_toplevel,
		  xkl_get_debug_window_title(engine, focused_toplevel));
	if (xkl_engine_is_listening_for(engine, XKLL_MANAGE_WINDOW_STATES)) {
		xkl_debug(150, "CurClient: " WINID_FORMAT ", '%s'\n",
			  xkl_engine_priv(engine, curr_toplvl_win),
			  xkl_get_debug_window_title(engine,
						     xkl_engine_priv
						     (engine,
						      curr_toplvl_win)));

		if (focused_toplevel !=
		    xkl_engine_priv(engine, curr_toplvl_win)) {
			/*
			 * If not state - we got the new window
			 */
			if (!xkl_engine_get_toplevel_window_state
			    (engine, focused_toplevel, &old_state)) {
				xkl_engine_update_current_state(engine,
								grp, inds,
								"Updating the state from new focused window");
				if (xkl_engine_is_listening_for
				    (engine, XKLL_MANAGE_WINDOW_STATES))
					xkl_engine_add_toplevel_window
					    (engine, focused_toplevel,
					     (Window) NULL, FALSE,
					     &xkl_engine_priv(engine,
							      curr_state));
			}
			/*
			 * There is state - just get the state from the window
			 */
			else {
				grp = old_state.group;
				inds = old_state.indicators;
			}
			xkl_engine_priv(engine, curr_toplvl_win) =
			    focused_toplevel;
			xkl_debug(160,
				  "CurClient:changed to " WINID_FORMAT
				  ", '%s'\n", xkl_engine_priv(engine,
							      curr_toplvl_win),
				  xkl_get_debug_window_title(engine,
							     xkl_engine_priv
							     (engine,
							      curr_toplvl_win)));
		}
		/* If the window already has this this state - we are just restoring it!
		   (see the second parameter of stateCallback */
		have_old_state =
		    xkl_engine_get_toplevel_window_state(engine,
							 xkl_engine_priv
							 (engine,
							  curr_toplvl_win),
							 &old_state);
	} else {		/* just tracking the stuff, no smart things */

		xkl_debug(160,
			  "Just updating the current state in the tracking mode\n");
		memcpy(&old_state, &xkl_engine_priv(engine, curr_state),
		       sizeof(XklState));
	}

	if (set_group || have_old_state) {
		xkl_engine_update_current_state(engine,
						set_group ? grp :
						old_state.group,
						set_inds ? inds :
						old_state.indicators,
						"Restoring the state from the window");
	}

	if (have_old_state)
		xkl_engine_try_call_state_func(engine, change_type,
					       &old_state);

	if (xkl_engine_is_listening_for(engine, XKLL_MANAGE_WINDOW_STATES))
		xkl_engine_save_toplevel_window_state(engine,
						      xkl_engine_priv
						      (engine,
						       curr_toplvl_win),
						      &xkl_engine_priv
						      (engine,
						       curr_state));
}
