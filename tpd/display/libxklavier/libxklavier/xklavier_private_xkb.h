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

#ifndef __XKLAVIER_PRIVATE_XKB_H__
#define __XKLAVIER_PRIVATE_XKB_H__

#ifdef LIBXKBFILE_PRESENT

#include <config.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#define ForPhysIndicators( i, bit ) \
    for ( i=0, bit=1; i<XkbNumIndicators; i++, bit<<=1 ) \
          if ( xkl_engine_backend(engine,XklXkb,cached_desc)->indicators->phys_indicators & bit )

typedef struct _XklXkb {

	gint event_type;

	gint error_code;

	XkbDescPtr cached_desc;

	gchar *indicator_names[XkbNumIndicators];

	XkbDescPtr actual_desc;

	gchar *group_names[XkbNumKbdGroups];

	int device_id;

#ifdef HAVE_XINPUT
	gint xi_event_type;

	gint xi_error_code;
#endif
} XklXkb;

extern void xkl_engine_dump_xkb_desc(XklEngine * engine,
				     const char *file_name,
				     XkbDescPtr kbd);

extern gboolean xkl_xkb_multiple_layouts_supported(XklEngine * engine);

extern const gchar *xkl_xkb_event_get_name(gint xkb_type);

extern gboolean xkl_xkb_config_native_prepare(XklEngine * engine,
					      const XklConfigRec * data,
					      XkbComponentNamesPtr
					      component_names);

extern void xkl_xkb_config_native_cleanup(XklEngine * engine,
					  XkbComponentNamesPtr
					  component_names);

extern gboolean xkl_xkb_set_indicator(XklEngine * engine,
				      gint indicator_num, gboolean set);

/* Start VTable methods */

extern gboolean xkl_xkb_activate_config_rec(XklEngine * engine,
					    const XklConfigRec * data);

extern void xkl_xkb_init_config_registry(XklConfigRegistry * config);

extern gboolean xkl_xkb_load_config_registry(XklConfigRegistry * config,
					     gboolean if_extras_needed);

extern gboolean xkl_xkb_write_config_rec_to_file(XklEngine * engine,
						 const char *file_name,
						 const XklConfigRec * data,
						 const gboolean binary);

extern gint xkl_xkb_process_x_event(XklEngine * engine, XEvent * xev);

extern gint xkl_xkb_process_x_error(XklEngine * engine, XErrorEvent * xerev);

extern void xkl_xkb_free_all_info(XklEngine * engine);

extern const gchar **xkl_xkb_get_groups_names(XklEngine * engine);

extern guint xkl_xkb_get_max_num_groups(XklEngine * engine);

extern guint xkl_xkb_get_num_groups(XklEngine * engine);

extern void xkl_xkb_get_server_state(XklEngine * engine,
				     XklState * current_state_out);

extern gboolean xkl_xkb_if_cached_info_equals_actual(XklEngine * engine);

extern gboolean xkl_xkb_load_all_info(XklEngine * engine);

extern void xkl_xkb_lock_group(XklEngine * engine, gint group);

extern gint xkl_xkb_pause_listen(XklEngine * engine);

extern gint xkl_xkb_resume_listen(XklEngine * engine);

extern void xkl_xkb_set_indicators(XklEngine * engine,
				   const XklState * window_state);

extern void xkl_xkb_term(XklEngine * engine);

/* End of VTable methods */

#else

/*
 * VERY VERY BAD STYLE, some kind of 'protected' methods - 
 * but some programs may want to hook into them.
 */

extern gboolean xkl_xkb_config_native_prepare(XklEngine * engine,
					      const XklConfigRec * data,
					      gpointer component_names);

extern void xkl_xkb_config_native_cleanup(XklEngine * engine,
					  gpointer component_names);

#endif

#endif
