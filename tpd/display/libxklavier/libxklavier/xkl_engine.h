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

#ifndef __XKL_ENGINE_H__
#define __XKL_ENGINE_H__

#include <X11/Xlib.h>

#include <glib-object.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _XklEngine XklEngine;
	typedef struct _XklEnginePrivate XklEnginePrivate;
	typedef struct _XklEngineClass XklEngineClass;

#define XKL_TYPE_ENGINE             (xkl_engine_get_type ())
#define XKL_ENGINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XKL_TYPE_ENGINE, XklEngine))
#define XKL_ENGINE_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), XKL_TYPE_ENGINE,  XklEngineClass))
#define XKL_IS_ENGINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XKL_TYPE_ENGINE))
#define XKL_IS_ENGINE_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), XKL_TYPE_ENGINE))
#define XKL_ENGINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XKL_TYPE_ENGINE, XklEngineClass))

/**
 * The type of the keyboard state change
 *   @GROUP_CHANGED: Group was changed
 *   @INDICATORS_CHANGED: Indicators were changed
 */
	typedef enum {
		GROUP_CHANGED,
		INDICATORS_CHANGED
	} XklEngineStateChange;

/**
 * A set of flags used to indicate the capabilities of the active backend
 *   @XKLF_CAN_TOGGLE_INDICATORS: Backend allows to toggls indicators on/off
 *   @XKLF_CAN_OUTPUT_CONFIG_AS_ASCII: Backend allows writing ASCII representation of the configuration
 *   @XKLF_CAN_OUTPUT_CONFIG_AS_BINARY: Backend allows writing binary representation of the configuration
 *   @XKLF_MULTIPLE_LAYOUTS_SUPPORTED: Backend supports multiple layouts
 *   @XKLF_REQUIRES_MANUAL_LAYOUT_MANAGEMENT: Backend requires manual configuration, some daemon should do 
 *                                   xkl_start_listen(engine,XKLL_MANAGE_LAYOUTS);
 *   @XKLF_DEVICE_DISCOVERY: Backend supports device discovery, can notify
 */
	typedef enum { /*< flags >*/
		XKLF_CAN_TOGGLE_INDICATORS = 1 << 0,
		XKLF_CAN_OUTPUT_CONFIG_AS_ASCII = 1 << 1,
		XKLF_CAN_OUTPUT_CONFIG_AS_BINARY = 1 << 2,
		XKLF_MULTIPLE_LAYOUTS_SUPPORTED = 1 << 3,
		XKLF_REQUIRES_MANUAL_LAYOUT_MANAGEMENT = 1 << 4,
		XKLF_DEVICE_DISCOVERY = 1 << 5
	} XklEngineFeatures;

/**
 * XKB state. Can be global or per-window
 */
	typedef struct {
/** 
 * selected group 
 */
		gint32 group;
/**
 * set of active indicators
 */
		guint32 indicators;
	} XklState;

#define XKL_TYPE_STATE (xkl_state_get_type())

        GType xkl_state_get_type (void) G_GNUC_CONST;

/**
 *	The main Xklavier engine class
 */
	struct _XklEngine {
/**
 * The superclass object
 */
		GObject parent;
/**
 * Private data
 */
		XklEnginePrivate *priv;
	};

/**
 * The XklEngine class, derived from GObject
 */
	struct _XklEngineClass {
/**
 * The superclass
 */
		GObjectClass parent_class;

/**
 * XklEngine::config-notify:
 * @engine: the object on which the signal is emitted
 *
 * Used for notifying application of the XKB configuration change.
 */
		void (*config_notify) (XklEngine * engine);

/**
 * XklEngine::new_window_notify:
 * @engine: the object on which the signal is emitted
 * @win: new window
 * @parent: new window's parent
 *
 * Used for notifying application of new window creation (actually, 
 * registration).
 *
 * Returns: the initial group id for the window (-1 to use the default value)
 */
		 gint(*new_window_notify) (XklEngine * engine, Window win,
					   Window parent);
/**
 * XklEngine::state_notify
 * @engine: the object on which the signal is emitted
 * @change_type: mask of changes
 * @group: new group
 * @restore: whether this state is restored from
 * saved state of set as new.
 *
 * Used for notifying application of the window state change.
 */
		void (*state_notify) (XklEngine * engine,
				      XklEngineStateChange change_type,
				      gint group, gboolean restore);

/**
 * XklEngine::new_device_notify
 * @engine: the object on which the signal is emitted
 *
 * Used for notifying application of the new keyboard attached
 */
		void (*new_device_notify) (XklEngine * engine);

	};


/**
 * xkl_engine_get_type:
 *
 * Get type info for XklEngine
 *
 * Returns: GType for XklEngine
 */
	extern GType xkl_engine_get_type(void);


/**
 * xkl_engine_get_instance:
 * @display: the X display used by the application
 *
 * Get the instance of the XklEngine. Within a process, there is always once instance.
 *
 * Returns: (transfer none): the singleton instance
 */
	extern XklEngine *xkl_engine_get_instance(Display * display);


/**
 * xkl_engine_get_backend_name:
 * @engine: the engine
 * 
 * What kind of backend is used
 *
 * Returns: some string id of the backend
 */
	extern const gchar *xkl_engine_get_backend_name(XklEngine *
							engine);

/**
 * xkl_engine_get_features:
 * @engine: the engine
 *
 * Provides information regarding available backend features
 * (combination of XKLF_* constants)
 *
 * Returns: ORed XKLF_* constants
 */
	extern guint xkl_engine_get_features(XklEngine * engine);

/**
 * xkl_engine_get_max_num_groups:
 * @engine: the engine
 *
 * Provides the information on maximum number of simultaneously supported 
 * groups (layouts)
 *
 * Returns: maximum number of the groups in configuration, 
 *         0 if no restrictions.
 */
	extern guint xkl_engine_get_max_num_groups(XklEngine * engine);

/**
 * The listener action modes:
 *   @XKLL_MANAGE_WINDOW_STATES: The listener process should handle the per-window states 
 *                       and all the related activity
 *   @XKLL_TRACK_KEYBOARD_STATE: Just track the state and pass it to the application above.
 *   @XKLL_MANAGE_LAYOUTS: The listener process should help backend to maintain the configuration
 *                  (manually switch layouts etc).
 */
	typedef enum {
		XKLL_MANAGE_WINDOW_STATES = 0x01,
		XKLL_TRACK_KEYBOARD_STATE = 0x02,
		XKLL_MANAGE_LAYOUTS = 0x04,
	} XklEngineListenModes;

/**
 * xkl_engine_start_listen:
 * @engine: the engine
 * @flags: any combination of XKLL_* constants
 *
 * Starts listening for XKB-related events
 *
 * Returns: 0
 */
	extern gint xkl_engine_start_listen(XklEngine * engine,
					    guint flags);

/**
 * xkl_engine_stop_listen:
 * @engine: the engine
 * @flags: any combination of XKLL_* constants
 *
 * Stops listening for XKB-related events
 * Returns: 0
 */
	extern gint xkl_engine_stop_listen(XklEngine * engine,
					   guint flags);

/**
 * xkl_engine_pause_listen:
 * @engine: the engine
 *
 * Temporary pauses listening for XKB-related events
 *
 * Returns: 0
 */
	extern gint xkl_engine_pause_listen(XklEngine * engine);

/**
 * xkl_engine_resume_listen:
 * @engine: the engine
 *
 * Resumes listening for XKB-related events
 *
 * Returns: 0
 */
	extern gint xkl_engine_resume_listen(XklEngine * engine);

/**
 * xkl_engine_grab_key:
 * @engine: the engine
 * @keycode: keycode
 * @modifiers: bitmask of modifiers
 *
 * Grabs some key
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_engine_grab_key(XklEngine * engine,
					    gint keycode, guint modifiers);

/**
 * xkl_engine_ungrab_key:
 * @engine: the engine
 * @keycode: keycode
 * @modifiers: bitmask of modifiers
 *
 * Ungrabs some key
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_engine_ungrab_key(XklEngine * engine,
					      gint keycode,
					      guint modifiers);

/**
 * xkl_engine_filter_events:
 * @engine: the engine
 * @evt: (skip): delivered X event
 *
 * Processes X events. Should be included into the main event cycle of an
 * application. One of the most important functions. 
 *
 * Returns: 0 if the event it processed - 1 otherwise
 */
	extern gint xkl_engine_filter_events(XklEngine * engine,
					     XEvent * evt);

/**
 * xkl_engine_allow_one_switch_to_secondary_group:
 * @engine: the engine
 *
 * Allows to switch (once) to the secondary group
 *
 */
	extern void
	 xkl_engine_allow_one_switch_to_secondary_group(XklEngine *
							engine);

/**
 * xkl_engine_get_current_window:
 * @engine: the engine
 *
 * Returns: currently focused window
 */
	extern Window xkl_engine_get_current_window(XklEngine * engine);

/**
 * xkl_engine_get_current_state:
 * @engine: the engine
 *
 * Returns: (transfer none): current state of the keyboard.
 * Returned value is a statically allocated buffer, should not be freed.
 */
	extern XklState *xkl_engine_get_current_state(XklEngine * engine);

/**
 * xkl_engine_get_window_title:
 * @engine: the engine
 * @win: X window
 *
 * Returns: the window title of some window or NULL. 
 * If not NULL, it should be freed with XFree
 */
	extern gchar *xkl_engine_get_window_title(XklEngine * engine,
						  Window win);

/** 
 * xkl_engine_get_state:
 * @engine: the engine
 * @win: window to query
 * @state_out: structure to store the state
 * 
 * Finds the state for a given window (for its "App window").
 *
 * Returns: TRUE on success, otherwise FALSE 
 * (the error message can be obtained using xkl_GetLastError).
 */
	extern gboolean xkl_engine_get_state(XklEngine * engine,
					     Window win,
					     XklState * state_out);

/**
 * xkl_engine_delete_state:
 * @engine: the engine
 * @win: target window
 *
 * Drops the state of a given window (of its "App window").
 */
	extern void xkl_engine_delete_state(XklEngine * engine,
					    Window win);

/** 
 * xkl_engine_save_state:
 * @engine: the engine
 * @win: target window
 * @state: new state of the window
 *
 * Stores ths state for a given window
 */
	extern void xkl_engine_save_state(XklEngine * engine, Window win,
					  XklState * state);

/**
 * xkl_engine_set_window_transparent:
 * @engine: the engine
 * @win: window do set the flag for.
 * @transparent: if true, the windows is transparent.
 *
 * Sets the "transparent" flag. It means focus switching onto 
 * this window will never change the state.
 */
	extern void xkl_engine_set_window_transparent(XklEngine *
						      engine,
						      Window win,
						      gboolean
						      transparent);

/**
 * xkl_engine_is_window_transparent:
 * @engine: the engine
 * @win: window to get the transparent flag from.
 *
 * Returns: TRUE if the window is "transparent"
 */
	extern gboolean xkl_engine_is_window_transparent(XklEngine
							 * engine,
							 Window win);

/**
 * xkl_engine_is_window_from_same_toplevel_window:
 * @engine: the engine
 * @win1: first window
 * @win2: second window
 *
 * Checks whether 2 windows have the same topmost window
 *
 * Returns: TRUE is windows are in the same application
 */
	extern gboolean
	    xkl_engine_is_window_from_same_toplevel_window(XklEngine *
							   engine,
							   Window win1,
							   Window win2);

/**
 * xkl_engine_get_num_groups:
 * @engine: the engine
 *
 * Returns: the total number of groups in the current configuration 
 * (keyboard)
 */
	extern guint xkl_engine_get_num_groups(XklEngine * engine);

/**
 * xkl_engine_get_groups_names:
 * @engine: the engine
 *
 * Returns: (transfer none): the array of group names for the current XKB
 * configuration (keyboard).
 * This array is static, should not be freed
 */
	extern const gchar **xkl_engine_get_groups_names(XklEngine *
							 engine);

/**
 * xkl_engine_get_indicators_names:
 * @engine: the engine
 *
 * Returns: (transfer none): the array of indicator names for the current XKB
 * configuration (keyboard).
 * This array is static, should not be freed
 */
	extern const gchar **xkl_engine_get_indicators_names(XklEngine *
							     engine);

/**
 * xkl_engine_get_next_group:
 * @engine: the engine
 *
 * Calculates next group id. Does not change the state of anything.
 *
 * Returns: next group id
 */
	extern gint xkl_engine_get_next_group(XklEngine * engine);

/**
 * xkl_engine_get_prev_group:
 * @engine: the engine
 *
 * Calculates prev group id. Does not change the state of anything.
 *
 * Returns: prev group id
 */
	extern gint xkl_engine_get_prev_group(XklEngine * engine);

/**
 * xkl_engine_get_current_window_group:
 * @engine: the engine
 *
 * Returns: saved group id of the current window. 
 */
	extern gint xkl_engine_get_current_window_group(XklEngine *
							engine);

/**
 * xkl_engine_lock_group:
 * @engine: the engine
 * @group: group number for locking
 *
 * Locks the group. Can be used after xkl_GetXXXGroup functions
 */
	extern void xkl_engine_lock_group(XklEngine * engine, gint group);

/**
 * xkl_engine_set_group_per_toplevel_window:
 * @engine: the engine
 * @is_global: new parameter value
 *
 * Sets the configuration parameter: group per application
 */
	extern void xkl_engine_set_group_per_toplevel_window(XklEngine *
							     engine,
							     gboolean
							     is_global);

/**
 * xkl_engine_is_group_per_toplevel_window:
 * @engine: the engine
 * 
 * Returns: the value of the parameter: group per application
 */
	extern gboolean xkl_engine_is_group_per_toplevel_window(XklEngine *
								engine);

/**
 * xkl_engine_set_indicators_handling:
 * @engine: the engine
 * @whether_handle: new parameter value
 *
 * Sets the configuration parameter: perform indicators handling
 */
	extern void xkl_engine_set_indicators_handling(XklEngine * engine,
						       gboolean
						       whether_handle);

/**
 * xkl_engine_get_indicators_handling:
 * @engine: the engine
 *
 * Returns: the value of the parameter: perform indicator handling
 */
	extern gboolean xkl_engine_get_indicators_handling(XklEngine *
							   engine);

/**
 * xkl_engine_set_secondary_groups_mask:
 * @engine: the engine
 * @mask: new group mask
 *
 * Sets the secondary groups (one bit per group). 
 * Secondary groups require explicit "allowance" for switching
 */
	extern void xkl_engine_set_secondary_groups_mask(XklEngine *
							 engine,
							 guint mask);

/**
 * xkl_engine_get_secondary_groups_mask:
 * @engine: the engine
 *
 * Returns: the secondary group mask
 */
	extern guint xkl_engine_get_secondary_groups_mask(XklEngine *
							  engine);

/**
 * xkl_engine_set_default_group:
 * @engine: the engine
 * @group: default group
 *
 * Configures the default group set on window creation.
 * If -1, no default group is used
 */
	extern void xkl_engine_set_default_group(XklEngine * engine,
						 gint group);

/**
 * xkl_engine_get_default_group:
 * @engine: the engine
 *
 * Returns the default group set on window creation
 * If -1, no default group is used
 *
 * Returns: the default group
 */
	extern gint xkl_engine_get_default_group(XklEngine * engine);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
