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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xklavier_private.h"
#include "xklavier_private_xkb.h"

#ifdef HAVE_XINPUT
#include "config.h"
#include "X11/extensions/XInput.h"

static gint
xkl_xinput_process_x_event(XklEngine * engine, XEvent * xev)
{
	XDevicePresenceNotifyEvent *dpne =
	    (XDevicePresenceNotifyEvent *) xev;
	xkl_debug(200, "XInput event detected: %d\n", dpne->devchange);
	if (dpne->devchange == DeviceEnabled) {
		xkl_debug(150, "Device enabled: %d\n", dpne->deviceid);
		g_signal_emit_by_name(engine, "X-new-device");
	}
	return 1;
}
#endif

/*
 * XKB event handler
 */
gint
xkl_xkb_process_x_event(XklEngine * engine, XEvent * xev)
{
#ifdef LIBXKBFILE_PRESENT
	gint i;
	guint bit;
	guint inds;
	XkbEvent *kev = (XkbEvent *) xev;

	if (!
	    (xkl_engine_is_listening_for(engine, XKLL_MANAGE_WINDOW_STATES)
	     | xkl_engine_is_listening_for(engine,
					   XKLL_TRACK_KEYBOARD_STATE)))
		return 0;

#ifdef HAVE_XINPUT
	/* Special case XInput event */
	if (xev->type == xkl_engine_backend(engine, XklXkb, xi_event_type))
		return xkl_xinput_process_x_event(engine, xev);
#endif

	if (xev->type != xkl_engine_backend(engine, XklXkb, event_type))
		return 0;

	xkl_debug(150, "Xkb event detected\n");

	switch (kev->any.xkb_type) {
		/*
		 * Group is changed!
		 */
	case XkbStateNotify:
#define GROUP_CHANGE_MASK \
    ( XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask )

		xkl_debug(150,
			  "XkbStateNotify detected, changes: %X/(mask %X), new group %d\n",
			  kev->state.changed, GROUP_CHANGE_MASK,
			  kev->state.locked_group);

		if (kev->state.changed & GROUP_CHANGE_MASK)
			xkl_engine_process_state_modification(engine,
							      GROUP_CHANGED,
							      kev->
							      state.locked_group,
							      0, FALSE);
		else {		/* ...not interested... */

			xkl_debug(200,
				  "This type of state notification is not regarding groups\n");
			if (kev->state.locked_group !=
			    xkl_engine_priv(engine, curr_state).group)
				xkl_debug(0,
					  "ATTENTION! Currently cached group %d is not equal to the current group from the event: %d\n!",
					  xkl_engine_priv(engine,
							  curr_state).group,
					  kev->state.locked_group);
		}

		break;

		/*
		 * Indicators are changed!
		 */
	case XkbIndicatorStateNotify:

		xkl_debug(150, "XkbIndicatorStateNotify\n");

		inds = xkl_engine_priv(engine, curr_state).indicators;

		ForPhysIndicators(i,
				  bit) if (kev->indicators.changed & bit) {
			if (kev->indicators.state & bit)
				inds |= bit;
			else
				inds &= ~bit;
		}

		xkl_engine_process_state_modification(engine,
						      INDICATORS_CHANGED,
						      0, inds, TRUE);
		break;

		/*
		 * The configuration is changed!
		 */
	case XkbIndicatorMapNotify:
	case XkbControlsNotify:
	case XkbNamesNotify:
#if 0
		/* not really fair - but still better than flooding... */
		XklDebug(200,
			 "warning: configuration event %s is not actually processed\n",
			 _XklXkbGetXkbEventName(kev->any.xkb_type));
		break;
#endif
	case XkbNewKeyboardNotify:
		xkl_debug(150, "%s\n",
			  xkl_xkb_event_get_name(kev->any.xkb_type));
		xkl_engine_reset_all_info(engine, FALSE,
					  "XKB event: XkbNewKeyboardNotify");
		break;

		/*
		 * ...Not interested...
		 */
	default:
		xkl_debug(150, "Unknown XKB event %d [%s]\n",
			  kev->any.xkb_type,
			  xkl_xkb_event_get_name(kev->any.xkb_type));
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}

/*
 * XKB error handler
 */
gint
xkl_xkb_process_x_error(XklEngine * engine, XErrorEvent * xerev)
{
#ifdef HAVE_XINPUT
	/* Ignore XInput errors */
	if (xerev->error_code >=
	    xkl_engine_backend(engine, XklXkb, xi_error_code)
	    && xerev->error_code <=
	    (xkl_engine_backend(engine, XklXkb, xi_error_code) +
	     XI_BadClass))

		return 1;
#endif

	return 0;
}

void
xkl_xkb_set_indicators(XklEngine * engine, const XklState * window_state)
{
#ifdef LIBXKBFILE_PRESENT
	int i;
	unsigned bit;

	XkbDescPtr cached =
	    xkl_engine_backend(engine, XklXkb, cached_desc);
	ForPhysIndicators(i, bit) if (cached->names->indicators[i] != None) {
		gboolean status;
		status = xkl_xkb_set_indicator(engine, i,
					       (window_state->indicators &
						bit) != 0);
		xkl_debug(150, "Set indicator \"%s\"/%d to %d: %d\n",
			  xkl_engine_backend(engine, XklXkb,
					     indicator_names)[i],
			  cached->names->indicators[i],
			  window_state->indicators & bit, status);
	}
#endif
}
