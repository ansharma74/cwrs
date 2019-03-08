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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xklavier_private.h"
#include "xklavier_private_xkb.h"

#ifdef HAVE_XINPUT
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#endif

#ifdef LIBXKBFILE_PRESENT

const gchar **
xkl_xkb_get_groups_names(XklEngine * engine)
{
	return (const gchar **) xkl_engine_backend(engine, XklXkb,
						   group_names);
}

const gchar **
xkl_xkb_get_indicators_names(XklEngine * engine)
{
	return (const gchar **) xkl_engine_backend(engine, XklXkb,
						   indicator_names);
}

gint
xkl_xkb_pause_listen(XklEngine * engine)
{
	XkbSelectEvents(xkl_engine_get_display(engine),
			xkl_engine_backend(engine, XklXkb, device_id),
			XkbAllEventsMask, 0);
	return 0;
}

gint
xkl_xkb_resume_listen(XklEngine * engine)
{
#ifdef HAVE_XINPUT
	int xitype;
	XEventClass xiclass;
#endif
	/* What events we want */
#define XKB_EVT_MASK \
         (XkbStateNotifyMask| \
          XkbNamesNotifyMask| \
          XkbControlsNotifyMask| \
          XkbIndicatorStateNotifyMask| \
          XkbIndicatorMapNotifyMask| \
          XkbNewKeyboardNotifyMask)

	Display *display = xkl_engine_get_display(engine);
	XkbSelectEvents(display,
			xkl_engine_backend(engine, XklXkb, device_id),
			XKB_EVT_MASK, XKB_EVT_MASK);

#define XKB_STATE_EVT_DTL_MASK \
         (XkbGroupStateMask)

	XkbSelectEventDetails(display,
			      xkl_engine_backend(engine, XklXkb,
						 device_id),
			      XkbStateNotify, XKB_STATE_EVT_DTL_MASK,
			      XKB_STATE_EVT_DTL_MASK);

#define XKB_NAMES_EVT_DTL_MASK \
         (XkbGroupNamesMask|XkbIndicatorNamesMask)

	XkbSelectEventDetails(display,
			      xkl_engine_backend(engine, XklXkb,
						 device_id),
			      XkbNamesNotify, XKB_NAMES_EVT_DTL_MASK,
			      XKB_NAMES_EVT_DTL_MASK);
#ifdef HAVE_XINPUT
	if (xkl_engine_priv(engine, features) & XKLF_DEVICE_DISCOVERY) {
		DevicePresence(display, xitype, xiclass);
		XSelectExtensionEvent(display,
				      xkl_engine_priv(engine, root_window),
				      &xiclass, 1);
		xkl_engine_backend(engine, XklXkb, xi_event_type) = xitype;
	} else
		xkl_engine_backend(engine, XklXkb, xi_event_type) = -1;
#endif
	return 0;
}

guint
xkl_xkb_get_max_num_groups(XklEngine * engine)
{
	return xkl_engine_priv(engine,
			       features) & XKLF_MULTIPLE_LAYOUTS_SUPPORTED
	    ? XkbNumKbdGroups : 1;
}

guint
xkl_xkb_get_num_groups(XklEngine * engine)
{
	return xkl_engine_backend(engine, XklXkb,
				  cached_desc)->ctrls->num_groups;
}

#define KBD_MASK \
    ( 0 )
#define CTRLS_MASK \
  ( XkbSlowKeysMask )
#define NAMES_MASK \
  ( XkbGroupNamesMask | XkbIndicatorNamesMask )

void
xkl_xkb_free_all_info(XklEngine * engine)
{
	gint i;
	gchar **pi = xkl_engine_backend(engine, XklXkb, indicator_names);
	XkbDescPtr desc;

	for (i = 0; i < XkbNumIndicators; i++, pi++) {
		/* only free non-empty ones */
		if (*pi && **pi)
			XFree(*pi);
	}
	desc = xkl_engine_backend(engine, XklXkb, cached_desc);
	if (desc != NULL) {
		int i;
		char **group_name =
		    xkl_engine_backend(engine, XklXkb, group_names);
		for (i = desc->ctrls->num_groups; --i >= 0; group_name++)
			if (*group_name) {
				XFree(*group_name);
				*group_name = NULL;
			}
		XkbFreeKeyboard(desc, XkbAllComponentsMask, True);
		xkl_engine_backend(engine, XklXkb, cached_desc) = NULL;
	}

	/* just in case - never actually happens... */
	desc = xkl_engine_backend(engine, XklXkb, actual_desc);
	if (desc != NULL) {
		XkbFreeKeyboard(desc, XkbAllComponentsMask, True);
		xkl_engine_backend(engine, XklXkb, actual_desc) = NULL;
	}
}

static gboolean
xkl_xkb_load_actual_desc(XklEngine * engine)
{
	gboolean rv = FALSE;
	Status status;

	Display *display = xkl_engine_get_display(engine);
	XkbDescPtr desc = XkbGetMap(display, KBD_MASK,
				    xkl_engine_backend(engine, XklXkb,
						       device_id));
	xkl_engine_backend(engine, XklXkb, actual_desc) = desc;
	if (desc != NULL) {
		rv = Success == (status = XkbGetControls(display,
							 CTRLS_MASK,
							 desc)) &&
		    Success == (status = XkbGetNames(display,
						     NAMES_MASK,
						     desc)) &&
		    Success == (status = XkbGetIndicatorMap(display,
							    XkbAllIndicatorsMask,
							    desc));
		if (!rv) {
			xkl_last_error_message =
			    "Could not load controls/names/indicators";
			xkl_debug(0, "%s: %d\n", xkl_last_error_message,
				  status);
			XkbFreeKeyboard(desc, XkbAllComponentsMask, True);
			xkl_engine_backend(engine, XklXkb, actual_desc) =
			    NULL;
		}
	}
	return rv;
}

gboolean
xkl_xkb_if_cached_info_equals_actual(XklEngine * engine)
{
	gint i;
	Atom *pa1, *pa2;
	gboolean rv = FALSE;

	if (xkl_xkb_load_actual_desc(engine)) {
		/* First, compare the number of groups */
		XkbDescPtr cached =
		    xkl_engine_backend(engine, XklXkb, cached_desc);
		XkbDescPtr actual =
		    xkl_engine_backend(engine, XklXkb, actual_desc);

		if (cached->ctrls->num_groups == actual->ctrls->num_groups) {
			/* Then, compare group names, just atoms */
			pa1 = cached->names->groups;
			pa2 = actual->names->groups;
			for (i = cached->ctrls->num_groups; --i >= 0;
			     pa1++, pa2++)
				if (*pa1 != *pa2)
					break;

			/* Then, compare indicator names, just atoms */
			if (i < 0) {
				pa1 = cached->names->indicators;
				pa2 = actual->names->indicators;
				for (i = XkbNumIndicators; --i >= 0;
				     pa1++, pa2++)
					if (*pa1 != *pa2)
						break;
				rv = i < 0;
			}
		}
		/* 
		 * in case of failure, reuse in _XklXkbLoadAllInfo
		 * in case of success - free it
		 */
		if (rv) {
			XkbFreeKeyboard(actual, XkbAllComponentsMask,
					True);
			xkl_engine_backend(engine, XklXkb, actual_desc) =
			    NULL;
		}
	} else {
		xkl_debug(0,
			  "Could not load the XkbDescPtr for comparison\n");
	}
	return rv;
}

/*
 * Load some XKB parameters
 */
gboolean
xkl_xkb_load_all_info(XklEngine * engine)
{
	gint i;
	Atom *pa;
	gchar **group_name;
	gchar **pi = xkl_engine_backend(engine, XklXkb, indicator_names);
	Display *display = xkl_engine_get_display(engine);
	XkbDescPtr actual =
	    xkl_engine_backend(engine, XklXkb, actual_desc);
	XkbDescPtr cached;

	if (actual == NULL)
		if (!xkl_xkb_load_actual_desc(engine)) {
			xkl_last_error_message = "Could not load keyboard";
			return FALSE;
		}

	/* take it from the cache (in most cases LoadAll is called from ResetAll which in turn ...) */
	cached = actual = xkl_engine_backend(engine, XklXkb, actual_desc);
	xkl_engine_backend(engine, XklXkb, cached_desc) =
	    xkl_engine_backend(engine, XklXkb, actual_desc);
	xkl_engine_backend(engine, XklXkb, actual_desc) = NULL;

	/* First, output the number of the groups */
	xkl_debug(200, "found %d groups\n", cached->ctrls->num_groups);

	/* Then, cache (and output) the names of the groups */
	pa = cached->names->groups;
	group_name = xkl_engine_backend(engine, XklXkb, group_names);
	for (i = cached->ctrls->num_groups; --i >= 0; pa++, group_name++) {
		*group_name =
		    XGetAtomName(display,
				 *pa == None ? XInternAtom(display, "-",
							   False) : *pa);
		xkl_debug(200, "Group %d has name [%s]\n", i, *group_name);
	}

	xkl_engine_priv(engine, last_error_code) =
	    XkbGetIndicatorMap(display, XkbAllIndicatorsMask, cached);

	if (xkl_engine_priv(engine, last_error_code) != Success) {
		xkl_last_error_message = "Could not load indicator map";
		return FALSE;
	}

	/* Then, cache (and output) the names of the indicators */
	pa = cached->names->indicators;
	for (i = XkbNumIndicators; --i >= 0; pi++, pa++) {
		Atom a = *pa;
		if (a != None)
			*pi = XGetAtomName(display, a);
		else
			*pi = "";

		xkl_debug(200, "Indicator[%d] is %s\n", i, *pi);
	}

	xkl_debug(200, "Real indicators are %X\n",
		  cached->indicators->phys_indicators);

	g_signal_emit_by_name(engine, "X-config-changed");

	return TRUE;
}

void
xkl_xkb_lock_group(XklEngine * engine, gint group)
{
	Display *display = xkl_engine_get_display(engine);
	xkl_debug(100, "Posted request for change the group to %d ##\n",
		  group);
	XkbLockGroup(display,
		     xkl_engine_backend(engine, XklXkb, device_id), group);
	XSync(display, False);
}

/*
 * Updates current internal state from X state
 */
void
xkl_xkb_get_server_state(XklEngine * engine, XklState * current_state_out)
{
	XkbStateRec state;
	Display *display = xkl_engine_get_display(engine);

	current_state_out->group = 0;
	if (Success ==
	    XkbGetState(display,
			xkl_engine_backend(engine, XklXkb, device_id),
			&state))
		current_state_out->group = state.locked_group;

	if (Success ==
	    XkbGetIndicatorState(display,
				 xkl_engine_backend(engine, XklXkb,
						    device_id),
				 &current_state_out->indicators))
		current_state_out->indicators &=
		    xkl_engine_backend(engine, XklXkb,
				       cached_desc)->indicators->
		    phys_indicators;
	else
		current_state_out->indicators = 0;
}

/*
 * Actually taken from mxkbledpanel, valueChangedProc
 */
gboolean
xkl_xkb_set_indicator(XklEngine * engine, gint indicator_num, gboolean set)
{
	XkbIndicatorMapPtr map;
	Display *display = xkl_engine_get_display(engine);
	XkbDescPtr cached =
	    xkl_engine_backend(engine, XklXkb, cached_desc);

	map = cached->indicators->maps + indicator_num;

	/* The 'flags' field tells whether this indicator is automatic
	 * (XkbIM_NoExplicit - 0x80), explicit (XkbIM_NoAutomatic - 0x40),
	 * or neither (both - 0xC0).
	 *
	 * If NoAutomatic is set, the server ignores the rest of the 
	 * fields in the indicator map (i.e. it disables automatic control 
	 * of the LED).   If NoExplicit is set, the server prevents clients 
	 * from explicitly changing the value of the LED (using the core 
	 * protocol *or* XKB).   If NoAutomatic *and* NoExplicit are set, 
	 * the LED cannot be changed (unless you change the map first).   
	 * If neither NoAutomatic nor NoExplicit are set, the server will 
	 * change the LED according to the indicator map, but clients can 
	 * override that (until the next automatic change) using the core 
	 * protocol or XKB.
	 */
	switch (map->flags & (XkbIM_NoExplicit | XkbIM_NoAutomatic)) {
	case XkbIM_NoExplicit | XkbIM_NoAutomatic:
		{
			/* Can do nothing. Just ignore the indicator */
			return TRUE;
		}

	case XkbIM_NoAutomatic:
		{
			if (cached->names->indicators[indicator_num] !=
			    None)
				XkbSetNamedIndicator(display,
						     xkl_engine_backend
						     (engine, XklXkb,
						      device_id),
						     cached->names->
						     indicators
						     [indicator_num], set,
						     False, NULL);
			else {
				XKeyboardControl xkc;
				xkc.led = indicator_num;
				xkc.led_mode =
				    set ? LedModeOn : LedModeOff;
				XChangeKeyboardControl(display,
						       KBLed | KBLedMode,
						       &xkc);
				XSync(display, False);
			}

			return TRUE;
		}

	case XkbIM_NoExplicit:
		break;
	}

	/* The 'ctrls' field tells what controls tell this indicator to
	 * to turn on:  RepeatKeys (0x1), SlowKeys (0x2), BounceKeys (0x4),
	 *              StickyKeys (0x8), MouseKeys (0x10), AccessXKeys (0x20),
	 *              TimeOut (0x40), Feedback (0x80), ToggleKeys (0x100),
	 *              Overlay1 (0x200), Overlay2 (0x400), GroupsWrap (0x800),
	 *              InternalMods (0x1000), IgnoreLockMods (0x2000),
	 *              PerKeyRepeat (0x3000), or ControlsEnabled (0x4000)
	 */
	if (map->ctrls) {
		gulong which = map->ctrls;

		XkbGetControls(display, XkbAllControlsMask, cached);
		if (set)
			cached->ctrls->enabled_ctrls |= which;
		else
			cached->ctrls->enabled_ctrls &= ~which;
		XkbSetControls(display, which | XkbControlsEnabledMask,
			       cached);
	}

	/* The 'which_groups' field tells when this indicator turns on
	 * for the 'groups' field:  base (0x1), latched (0x2), locked (0x4),
	 * or effective (0x8).
	 */
	if (map->groups) {
		gint i;
		guint group = 1;

		/* Turning on a group indicator is kind of tricky.  For
		 * now, we will just Latch or Lock the first group we find
		 * if that is what this indicator does.  Otherwise, we're
		 * just going to punt and get out of here.
		 */
		if (set) {
			for (i = XkbNumKbdGroups; --i >= 0;)
				if ((1 << i) & map->groups) {
					group = i;
					break;
				}
			if (map->which_groups & (XkbIM_UseLocked |
						 XkbIM_UseEffective)) {
				/* Important: Groups should be ignored here - because they are handled separately! */
				/* XklLockGroup( group ); */
			} else if (map->which_groups & XkbIM_UseLatched)
				XkbLatchGroup(display,
					      xkl_engine_backend(engine,
								 XklXkb,
								 device_id),
					      group);
			else {
				/* Can do nothing. Just ignore the indicator */
				return TRUE;
			}
		} else
			/* Turning off a group indicator will mean that we just
			 * Lock the first group that this indicator doesn't watch.
			 */
		{
			for (i = XkbNumKbdGroups; --i >= 0;)
				if (!((1 << i) & map->groups)) {
					group = i;
					break;
				}
			xkl_xkb_lock_group(engine, group);
		}
	}

	/* The 'which_mods' field tells when this indicator turns on
	 * for the modifiers:  base (0x1), latched (0x2), locked (0x4),
	 *                     or effective (0x8).
	 *
	 * The 'real_mods' field tells whether this turns on when one of 
	 * the real X modifiers is set:  Shift (0x1), Lock (0x2), Control (0x4),
	 * Mod1 (0x8), Mod2 (0x10), Mod3 (0x20), Mod4 (0x40), or Mod5 (0x80). 
	 *
	 * The 'virtual_mods' field tells whether this turns on when one of
	 * the virtual modifiers is set.
	 *
	 * The 'mask' field tells what real X modifiers the virtual_modifiers
	 * map to?
	 */
	if (map->mods.real_mods || map->mods.mask) {
		guint affect, mods;

		affect = (map->mods.real_mods | map->mods.mask);

		mods = set ? affect : 0;

		if (map->which_mods &
		    (XkbIM_UseLocked | XkbIM_UseEffective))
			XkbLockModifiers(display,
					 xkl_engine_backend(engine, XklXkb,
							    device_id),
					 affect, mods);
		else if (map->which_mods & XkbIM_UseLatched)
			XkbLatchModifiers(display,
					  xkl_engine_backend(engine,
							     XklXkb,
							     device_id),
					  affect, mods);
		else {
			return TRUE;
		}
	}
	return TRUE;
}

#endif

gint
xkl_xkb_init(XklEngine * engine)
{
	Display *display = xkl_engine_get_display(engine);

#ifdef LIBXKBFILE_PRESENT
	gint opcode;
	gboolean xkl_xkb_ext_present;
	int xi_opc;

	xkl_engine_priv(engine, backend_id) = "XKB";
	xkl_engine_priv(engine, features) = XKLF_CAN_TOGGLE_INDICATORS |
	    XKLF_CAN_OUTPUT_CONFIG_AS_ASCII |
	    XKLF_CAN_OUTPUT_CONFIG_AS_BINARY;
	xkl_engine_priv(engine, activate_config_rec) =
	    xkl_xkb_activate_config_rec;
	xkl_engine_priv(engine, init_config_registry) =
	    xkl_xkb_init_config_registry;
	xkl_engine_priv(engine, load_config_registry) =
	    xkl_xkb_load_config_registry;
	xkl_engine_priv(engine, write_config_rec_to_file) =
	    xkl_xkb_write_config_rec_to_file;
	xkl_engine_priv(engine, get_groups_names) =
	    xkl_xkb_get_groups_names;
	xkl_engine_priv(engine, get_indicators_names) =
	    xkl_xkb_get_indicators_names;
	xkl_engine_priv(engine, get_max_num_groups) =
	    xkl_xkb_get_max_num_groups;
	xkl_engine_priv(engine, get_num_groups) = xkl_xkb_get_num_groups;
	xkl_engine_priv(engine, lock_group) = xkl_xkb_lock_group;
	xkl_engine_priv(engine, process_x_event) = xkl_xkb_process_x_event;
	xkl_engine_priv(engine, process_x_error) = xkl_xkb_process_x_error;
	xkl_engine_priv(engine, free_all_info) = xkl_xkb_free_all_info;
	xkl_engine_priv(engine, if_cached_info_equals_actual) =
	    xkl_xkb_if_cached_info_equals_actual;
	xkl_engine_priv(engine, load_all_info) = xkl_xkb_load_all_info;
	xkl_engine_priv(engine, get_server_state) =
	    xkl_xkb_get_server_state;
	xkl_engine_priv(engine, pause_listen) = xkl_xkb_pause_listen;
	xkl_engine_priv(engine, resume_listen) = xkl_xkb_resume_listen;
	xkl_engine_priv(engine, set_indicators) = xkl_xkb_set_indicators;
	xkl_engine_priv(engine, finalize) = xkl_xkb_term;

	if (getenv("XKL_XKB_DISABLE") != NULL)
		return -1;

	xkl_engine_priv(engine, backend) = g_new0(XklXkb, 1);
	xkl_engine_backend(engine, XklXkb, device_id) = XkbUseCoreKbd;

	xkl_xkb_ext_present = XkbQueryExtension(display,
						&opcode,
						&xkl_engine_backend(engine,
								    XklXkb,
								    event_type),
						&xkl_engine_backend(engine,
								    XklXkb,
								    error_code),
						NULL, NULL);
	if (!xkl_xkb_ext_present)
		return -1;

	xkl_debug(160,
		  "xkbEvenType: %X, xkbError: %X, display: %p, root: "
		  WINID_FORMAT "\n", xkl_engine_backend(engine, XklXkb,
							event_type),
		  xkl_engine_backend(engine, XklXkb, error_code), display,
		  xkl_engine_priv(engine, root_window));

	xkl_engine_priv(engine, base_config_atom) =
	    XInternAtom(display, _XKB_RF_NAMES_PROP_ATOM, False);
	xkl_engine_priv(engine, backup_config_atom) =
	    XInternAtom(display, "_XKB_RULES_NAMES_BACKUP", False);

	xkl_engine_priv(engine, default_model) = "pc101";
	xkl_engine_priv(engine, default_layout) = "us";

	/* First, we have to assign xkl_vtable - 
	   because this function uses it */

	if (xkl_xkb_multiple_layouts_supported(engine))
		xkl_engine_priv(engine, features) |=
		    XKLF_MULTIPLE_LAYOUTS_SUPPORTED;

#if HAVE_XINPUT
	if (XQueryExtension
	    (display, "XInputExtension", &xi_opc,
	     &xkl_engine_backend(engine, XklXkb, xi_event_type),
	     &xkl_engine_backend(engine, XklXkb, xi_error_code))) {
		XExtensionVersion *ev =
		    XGetExtensionVersion(display, "XInputExtension");
		xkl_debug(150,
			  "XInputExtension found (%d, %d, %d) version %d.%d\n",
			  xi_opc, xkl_engine_backend(engine, XklXkb,
						     xi_event_type),
			  xkl_engine_backend(engine, XklXkb,
					     xi_error_code),
			  ev->major_version, ev->minor_version);
		/* DevicePresence is available from XI 1.4 */
		if ((ev->major_version * 10) + ev->minor_version >= 14) {
			xkl_debug(200, "DevicePresence available\n");
			xkl_engine_priv(engine, features) |=
			    XKLF_DEVICE_DISCOVERY;
		} else {
			xkl_debug(200, "DevicePresence not available\n");
		}
		XFree(ev);
	} else {
		xkl_debug(0, "XInputExtension not found\n");
		xkl_engine_backend(engine, XklXkb, xi_event_type) = -1;
		xkl_engine_backend(engine, XklXkb, xi_error_code) = -1;
	}
#endif
	return 0;
#else
	xkl_debug(160,
		  "NO XKB LIBS, display: %p, root: " WINID_FORMAT
		  "\n", display, xkl_engine_priv(engine, root_window));
	return -1;
#endif
}

void
xkl_xkb_term(XklEngine * engine)
{
}

#ifdef LIBXKBFILE_PRESENT
const gchar *
xkl_xkb_event_get_name(gint xkb_type)
{
	/* Not really good to use the fact of consecutivity
	   but XKB protocol extension is already standartized so... */
	static const gchar *evt_names[] = {
		"XkbNewKeyboardNotify",
		"XkbMapNotify",
		"XkbStateNotify",
		"XkbControlsNotify",
		"XkbIndicatorStateNotify",
		"XkbIndicatorMapNotify",
		"XkbNamesNotify",
		"XkbCompatMapNotify",
		"XkbBellNotify",
		"XkbActionMessage",
		"XkbAccessXNotify",
		"XkbExtensionDeviceNotify",
		"LASTEvent"
	};
	xkb_type -= XkbNewKeyboardNotify;
	if (xkb_type < 0 ||
	    xkb_type >= (sizeof(evt_names) / sizeof(evt_names[0])))
		return "UNKNOWN/OOR";
	return evt_names[xkb_type];
}
#endif
