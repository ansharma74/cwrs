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

#ifndef __XKL_CONFIG_REC_H__
#define __XKL_CONFIG_REC_H__

#include <glib-object.h>
#include <libxklavier/xkl_engine.h>

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

        /**
         * XklConfigRec:
         * @layouts: (array zero-terminated=1):
         * @variants: (array zero-terminated=1):
         * @options: (array zero-terminated=1):
         */
	typedef struct _XklConfigRec XklConfigRec;
	typedef struct _XklConfigRecClass XklConfigRecClass;

#define XKL_TYPE_CONFIG_REC             (xkl_config_rec_get_type ())
#define XKL_CONFIG_REC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XKL_TYPE_CONFIG_REC, XklConfigRec))
#define XKL_CONFIG_REC_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), XKL_CONFIG_REC,  XklConfigRecClass))
#define XKL_IS_CONFIG_REC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XKL_TYPE_CONFIG_REC))
#define XKL_IS_CONFIG_REC_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), XKL_TYPE_CONFIG_REC))
#define XKL_CONFIG_REC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XKL_TYPE_CONFIG_REC, XklConfigRecClass))

/*
 * Basic configuration params
 */
	struct _XklConfigRec {
/*
 * The superclass object
 */
		GObject parent;
/*
 * The keyboard model
 */
		gchar *model;
/*
 * The array of keyboard layouts
 */
		gchar **layouts;
/*
 * The array of keyboard layout variants (if any)
 */
		gchar **variants;
/*
 * The array of keyboard layout options
 */
		gchar **options;
	};

/*
 * The XklConfigRec class, derived from GObject
 */
	struct _XklConfigRecClass {
		/*
		 * The superclass
		 */
		GObjectClass parent_class;
	};

/**
 * xkl_config_rec_get_type:
 *
 * Get type info for XConfigRec
 *
 * Returns: GType for XConfigRec
 */
	extern GType xkl_config_rec_get_type(void);

/**
 * xkl_config_rec_new:
 *
 * Create new XklConfigRec
 *
 * Returns: new instance
 */
	extern XklConfigRec *xkl_config_rec_new(void);

/**
 * xkl_config_rec_activate:
 * @data: valid XKB configuration
 * @engine: the engine
 *
 * Activates some XKB configuration
 * description. Can be NULL
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_rec_activate(const XklConfigRec * data,
						XklEngine * engine);

/**
 * xkl_config_rec_get_from_server:
 * @data: buffer for XKB configuration
 * @engine: the engine
 *
 * Loads the current XKB configuration (from X server)
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_rec_get_from_server(XklConfigRec * data,
						       XklEngine * engine);

/**
 * xkl_config_rec_get_from_backup:
 * @data: buffer for XKB configuration
 * @engine: the engine
 *
 * Loads the current XKB configuration (from backup)
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_rec_get_from_backup(XklConfigRec * data,
						       XklEngine * engine);

/**
 * xkl_config_rec_write_to_file:
 * @file_name: name of the file to create
 * @data: valid XKB configuration
 * description. Can be NULL
 * @binary: flag indicating whether the output file should be binary
 * @engine: the engine
 *
 * Writes some XKB configuration into XKM/XKB/... file
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_rec_write_to_file(XklEngine * engine,
						     const gchar *
						     file_name,
						     const XklConfigRec *
						     data,
						     const gboolean
						     binary);

/**
 * xkl_config_rec_get_from_root_window_property:
 * @rules_atom_name: atom name of the root window property to read
 * @rules_file_out: pointer to hold the file name
 * @config_out: buffer to hold the result
 * @engine: the engine
 *
 * Gets the XKB configuration from any root window property
 *
 * Returns: TRUE on success
 */
	extern gboolean
	    xkl_config_rec_get_from_root_window_property(XklConfigRec *
							 config_out,
							 Atom
							 rules_atom_name,
							 gchar **
							 rules_file_out,
							 XklEngine *
							 engine);

/**
 * xkl_config_rec_set_to_root_window_property:
 * @rules_atom_name: atom name of the root window property to write
 * @rules_file: rules file name
 * @config: configuration to save 
 * @engine: the engine
 *
 * Saves the XKB configuration into any root window property
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_rec_set_to_root_window_property(const
								   XklConfigRec
								   *
								   config,
								   Atom
								   rules_atom_name,
								   gchar *
								   rules_file,
								   XklEngine
								   *
								   engine);

/**
 * xkl_engine_backup_names_prop:
 * @engine: the engine
 *
 * Backups current XKB configuration into some property - 
 * if this property is not defined yet.
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_engine_backup_names_prop(XklEngine * engine);

/**
 * xkl_restore_names_prop:
 * @engine: the engine
 *
 * Restores XKB from the property saved by xkl_backup_names_prop
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_restore_names_prop(XklEngine * engine);

/**
 * xkl_config_rec_reset:
 * @data: record to reset
 *
 * Resets the record (equal to Destroy and Init)
 */
	extern void xkl_config_rec_reset(XklConfigRec * data);

/**
 * xkl_config_rec_equals:
 * @data1: record to compare
 * @data2: another record
 *
 * Compares two records
 *
 * Returns: TRUE if records are same
 */
	extern gboolean xkl_config_rec_equals(XklConfigRec * data1,
					      XklConfigRec * data2);

/**
 * xkl_config_rec_set_layouts:
 * @data: record to change
 * @new_layouts: (array zero-terminated=1) (transfer none): zero terminated
 * list of new layout names.
 *
 * Sets a new layout list. 
 *
 * Frees the previous layout list. This is primarily useful for bindings, in C
 * you can manipulate the @layouts record member directly.
 */
	extern void xkl_config_rec_set_layouts(XklConfigRec * data,
					       const gchar ** new_layouts);

/**
 * xkl_config_rec_set_variants:
 * @data: record to change
 * @new_variants: (transfer none) (array zero-terminated=1): zero terminated
 * list of new variant names.
 *
 * Sets a new variant list. 
 *
 * Frees the previous variant list. This is primarily useful for bindings, in C
 * you can manipulate the @variants record member directly.
 */
	extern void xkl_config_rec_set_variants(XklConfigRec * data,
						const gchar ** new_variants);

/**
 * xkl_config_rec_set_options:
 * @data: record to change
 * @new_options: (transfer none) (array zero-terminated=1): zero terminated
 * list of new option names.
 *
 * Sets a new option list. 
 *
 * Frees the previous option list. This is primarily useful for bindings, in C
 * you can manipulate the @options record member directly.
 */
	extern void xkl_config_rec_set_options(XklConfigRec * data,
					       const gchar ** new_options);

/**
 * xkl_config_rec_set_model:
 * @data: record to change
 * @new_model: (transfer none): new keyboard name.
 *
 * Sets a new model. 
 *
 * Frees the previous model. This is primarily useful for bindings, in C
 * you can manipulate the @model record member directly.
 */
	extern void xkl_config_rec_set_model(XklConfigRec * data,
					     const gchar * new_model);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
