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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>

extern void xkl_config_dump(FILE * file, XklConfigRec * data);

static Display *dpy;

static void
print_usage()
{
	printf
	    ("Usage: test_monitor (-l1)(-l2)(-l3)(-h)(-d <debugLevel>)\n");
	printf("Options:\n");
	printf("         -d - Set the debug level (by default, 0)\n");
	printf("         -h - Show this help\n");
	printf("         -l1 - listen to manage layouts\n");
	printf("         -l2 - listen to manage window states\n");
	printf("         -l3 - listen to track the keyboard state\n");
}

static void
state_changed(XklEngine * engine, XklEngineStateChange type,
	      gint new_group, gboolean restore)
{
	XklState *state = xkl_engine_get_current_state(engine);
	xkl_debug(0,
		  "State changed: type %d, new group: %d, restore: %d. Current state %d %d\n",
		  type, new_group, restore, state->group, state->indicators);
	if (type == INDICATORS_CHANGED) {
		Bool state;
		Atom capsLock = XInternAtom(dpy, "Caps Lock", False);
		Atom numLock = XInternAtom(dpy, "Num Lock", False);
		Atom scrollLock = XInternAtom(dpy, "Scroll Lock", False);

		XkbGetNamedIndicator(dpy, capsLock, NULL, &state, NULL, NULL);
		xkl_debug(0, "Caps Lock: %d\n", state);
		XkbGetNamedIndicator(dpy, numLock, NULL, &state, NULL, NULL);
		xkl_debug(0, "Num Lock: %d\n", state);
		XkbGetNamedIndicator(dpy, scrollLock, NULL, &state, NULL, NULL);
		xkl_debug(0, "Scroll Lock: %d\n", state);
	}
}

static void
config_changed(XklEngine * engine)
{
	const gchar **gn;
	gint gt;
	gint i;

	xkl_debug(0, "Config changed\n");

	gn = xkl_engine_get_groups_names(engine);
	gt = xkl_engine_get_num_groups(engine);
	for (i = 0; i < gt; i++)
		xkl_debug(0, "group[%d]: [%s]\n", i, gn[i]);
}

static void
new_device(XklEngine * engine)
{
	xkl_debug(0, "New device attached!\n");
}

int
main(int argc, char *argv[])
{
	int c;
	int debug_level = -1;
	XkbEvent ev;
	XklEngine *engine;
	int listener_type = 0, lt;
	int listener_types[] = { XKLL_MANAGE_LAYOUTS,
		XKLL_MANAGE_WINDOW_STATES,
		XKLL_TRACK_KEYBOARD_STATE
	};

	g_type_init_with_debug_flags(G_TYPE_DEBUG_OBJECTS |
				     G_TYPE_DEBUG_SIGNALS);

	while (1) {
		c = getopt(argc, argv, "hd:l:");
		if (c == -1)
			break;
		switch (c) {
		case 'h':
			print_usage();
			exit(0);
		case 'd':
			debug_level = atoi(optarg);
			break;
		case 'l':
			lt = optarg[0] - '1';
			if (lt >= 0
			    && lt <
			    sizeof(listener_types) /
			    sizeof(listener_types[0]))
				listener_type |= listener_types[lt];
			break;
		default:
			fprintf(stderr,
				"?? getopt returned character code 0%o ??\n",
				c);
			print_usage();
			exit(0);
		}
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "Could not open display\n");
		exit(1);
	}
	if (debug_level != -1)
		xkl_set_debug_level(debug_level);
	engine = xkl_engine_get_instance(dpy);
	if (engine != NULL) {
		XklConfigRec *current_config;
		const gchar **names;

		xkl_debug(0, "Xklavier initialized\n");

		current_config = xkl_config_rec_new();
		xkl_config_rec_get_from_server(current_config, engine);

		names = xkl_engine_get_groups_names(engine);
		while (names != NULL && *names != NULL && **names != 0)
			xkl_debug(0, "Group: [%s]\n", *names++);

		names = xkl_engine_get_indicators_names(engine);
		while (names != NULL && *names != NULL && **names != 0)
			xkl_debug(0, "Indicator: [%s]\n", *names++);

		g_signal_connect(engine, "X-state-changed",
				 G_CALLBACK(state_changed), NULL);
		g_signal_connect(engine, "X-config-changed",
				 G_CALLBACK(config_changed), NULL);
		g_signal_connect(engine, "X-new-device",
				 G_CALLBACK(new_device), NULL);

		xkl_debug(0, "Now, listening: %X...\n", listener_type);
		xkl_engine_start_listen(engine, listener_type);

		while (1) {
			XNextEvent(dpy, &ev.core);
			if (!xkl_engine_filter_events(engine, &ev.core))
				xkl_debug(200, "Unknown event %d\n",
					  ev.type);
		}

		xkl_engine_stop_listen(engine, listener_type);

		g_object_unref(G_OBJECT(current_config));
		g_object_unref(G_OBJECT(engine));
		xkl_debug(0, "Xklavier terminating\n");
	} else {
		fprintf(stderr, "Could not init Xklavier\n");
		exit(2);
	}
	printf("closing display: %p\n", dpy);
	XCloseDisplay(dpy);
	return 0;
}
