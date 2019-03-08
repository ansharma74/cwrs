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

#ifndef __XKL_CONFIG_REGISTRY_H__
#define __XKL_CONFIG_REGISTRY_H__

#include <glib-object.h>
#include <libxklavier/xkl_engine.h>
#include <libxklavier/xkl_config_item.h>

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

	typedef struct _XklConfigRegistry XklConfigRegistry;
	typedef struct _XklConfigRegistryPrivate XklConfigRegistryPrivate;
	typedef struct _XklConfigRegistryClass XklConfigRegistryClass;

#define XKL_TYPE_CONFIG_REGISTRY             (xkl_config_registry_get_type ())
#define XKL_CONFIG_REGISTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XKL_TYPE_CONFIG_REGISTRY, XklConfigRegistry))
#define XKL_CONFIG_REGISTRY_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), XKL_TYPE_CONFIG_REGISTRY,  XklConfigRegistryClass))
#define XKL_IS_CONFIG_REGISTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XKL_TYPE_CONFIG_REGISTRY))
#define XKL_IS_CONFIG_REGISTRY_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), XKL_TYPE_CONFIG_REGISTRY))
#define XKL_CONFIG_REGISTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XKL_TYPE_CONFIG_REGISTRY, XklConfigRegistryClass))

/**
 * The configuration manager. Corresponds to XML element "configItem".
 */
	struct _XklConfigRegistry {
/**
 * The superclass object
 */
		GObject parent;

		XklConfigRegistryPrivate *priv;
	};


/**
 * The XklConfigRegistry class, derived from GObject
 */
	struct _XklConfigRegistryClass {
    /**
     * The superclass
     */
		GObjectClass parent_class;
	};


/**
 * xkl_config_registry_get_type:
 *
 * Get type info for XklConfigRegistry
 *
 * Returns: GType for XklConfigRegistry
 */
	extern GType xkl_config_registry_get_type(void);

/**
 * xkl_config_registry_get_instance:
 * @engine: the engine to use for accessing X in all the operations
 * (like accessing root window properties etc)
 *
 * Create new XklConfig
 *
 * Returns: (transfer none): new instance
 */
	extern XklConfigRegistry
	    * xkl_config_registry_get_instance(XklEngine * engine);

/**
 * xkl_config_registry_load:
 * @config: the config registry
 * @if_extras_needed: whether exotic materials (layouts, options) 
 * should be loaded as well
 *
 * Loads XML configuration registry. The name is taken from X server
 * (for XKB/libxkbfile, from the root window property)
 *
 * Returns: TRUE on success
 */
	extern gboolean xkl_config_registry_load(XklConfigRegistry *
						 config,
						 gboolean
						 if_extras_needed);

/**
 * XklConfigItemProcessFunc:
 * @config: the config registry
 * @item: the item from registry
 * @data: anything which can be stored into the pointer
 *
 * Callback type used for enumerating keyboard models, layouts, variants, options
 */
	typedef void (*XklConfigItemProcessFunc) (XklConfigRegistry * config,
						  const XklConfigItem * item,
						  gpointer data);

/**
 * XklTwoConfigItemsProcessFunc:
 * @config: the config registry
 * @item: the item from registry
 * @subitem: the item from registry
 * @data: anything which can be stored into the pointer
 *
 * Callback type used for enumerating layouts/variants for countries/languages
 */
	typedef void (*XklTwoConfigItemsProcessFunc) (XklConfigRegistry *
					      config,
						      const XklConfigItem *
						      item,
						      const XklConfigItem *
						      subitem, gpointer data);

/* provide the old names for backwards compatibility */
	typedef XklConfigItemProcessFunc ConfigItemProcessFunc;
	typedef XklTwoConfigItemsProcessFunc TwoConfigItemsProcessFunc;

/**
 * xkl_config_registry_foreach_model:
 * @config: the config registry
 * @func: (scope call): callback to call for every model
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard models from the XML configuration registry
 */
	extern void xkl_config_registry_foreach_model(XklConfigRegistry *
						      config,
						      XklConfigItemProcessFunc
						      func, gpointer data);

/**
 * xkl_config_registry_foreach_layout:
 * @config: the config registry
 * @func: (scope call): callback to call for every layout
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard layouts from the XML configuration registry
 */
	extern void xkl_config_registry_foreach_layout(XklConfigRegistry *
						       config,
						       XklConfigItemProcessFunc
						       func,
						       gpointer data);

/**
 * xkl_config_registry_foreach_layout_variant:
 * @config: the config registry
 * @layout_name: layout name for which variants will be listed
 * @func: (scope call): callback to call for every layout variant
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard layout variants from the XML configuration registry
 */
	extern void
	 xkl_config_registry_foreach_layout_variant(XklConfigRegistry *
						    config,
						    const gchar *
						    layout_name,
						    XklConfigItemProcessFunc
						    func, gpointer data);

/**
 * xkl_config_registry_foreach_option_group:
 * @config: the config registry
 * @func: (scope call): callback to call for every option group
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard option groups from the XML configuration registry
 */
	extern void
	 xkl_config_registry_foreach_option_group(XklConfigRegistry *
						  config,
						  XklConfigItemProcessFunc
						  func, gpointer data);

/**
 * xkl_config_registry_foreach_option:
 * @config: the config registry
 * @option_group_name: option group name for which variants 
 * will be listed
 * @func: (scope call): callback to call for every option
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard options from the XML configuration registry
 */
	extern void xkl_config_registry_foreach_option(XklConfigRegistry *
						       config,
						       const gchar *
						       option_group_name,
						       XklConfigItemProcessFunc
						       func,
						       gpointer data);

/**
 * xkl_config_registry_find_model:
 * @config: the config registry
 * @item: pointer to a XklConfigItem containing the name of the
 * keyboard model. On successfull return, the descriptions are filled.
 *
 * Loads a keyboard model information from the XML configuration registry.
 *
 * Returns: TRUE if appropriate element was found and loaded
 */
	extern gboolean xkl_config_registry_find_model(XklConfigRegistry *
						       config,
						       XklConfigItem *
						       item);

/**
 * xkl_config_registry_find_layout:
 * @config: the config registry
 * @item: pointer to a XklConfigItem containing the name of the
 * keyboard layout. On successfull return, the descriptions are filled.
 *
 * Loads a keyboard layout information from the XML configuration registry.
 *
 * Returns: TRUE if appropriate element was found and loaded
 */
	extern gboolean xkl_config_registry_find_layout(XklConfigRegistry *
							config,
							XklConfigItem *
							item);

/**
 * xkl_config_registry_find_variant:
 * @config: the config registry
 * @layout_name: name of the parent layout
 * @item: pointer to a XklConfigItem containing the name of the
 * keyboard layout variant. On successfull return, the descriptions are filled.
 *
 * Loads a keyboard layout variant information from the XML configuration 
 * registry.
 *
 * Returns: TRUE if appropriate element was found and loaded
 */
	extern gboolean xkl_config_registry_find_variant(XklConfigRegistry
							 * config,
							 const char
							 *layout_name,
							 XklConfigItem *
							 item);

/**
 * xkl_config_registry_find_option_group:
 * @config: the config registry
 * @item: pointer to a XklConfigItem containing the name of the
 * keyboard option group. On successfull return, the descriptions are filled.
 *
 * Loads a keyboard option group information from the XML configuration 
 * registry.
 *
 * Returns: TRUE if appropriate element was found and loaded
 */
	extern gboolean
	    xkl_config_registry_find_option_group(XklConfigRegistry *
						  config,
						  XklConfigItem * item);

/**
 * xkl_config_registry_find_option:
 * @config: the config registry
 * @option_group_name: name of the option group
 * @item: pointer to a XklConfigItem containing the name of the
 * keyboard option. On successfull return, the descriptions are filled.
 *
 * Loads a keyboard option information from the XML configuration 
 * registry.
 *
 * Returns: TRUE if appropriate element was found and loaded
 */
	extern gboolean xkl_config_registry_find_option(XklConfigRegistry *
							config,
							const gchar *
							option_group_name,
							XklConfigItem *
							item);

/**
 * xkl_config_registry_foreach_country:
 * @config: the config registry
 * @func: (scope call): callback to call for every ISO 3166 country code
 * @data: anything which can be stored into the pointer
 *
 * Enumerates countries for which layouts are available,
 * from the XML configuration registry
 */
	extern void xkl_config_registry_foreach_country(XklConfigRegistry *
							config,
							XklConfigItemProcessFunc
							func,
							gpointer data);

/**
 * xkl_config_registry_foreach_country_variant:
 * @config: the config registry
 * @country_code: country ISO code for which variants will be listed
 * @func: (scope call): callback to call for every country variant
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard layout variants for the country,
 * from the XML configuration registry
 */
	extern void
	 xkl_config_registry_foreach_country_variant(XklConfigRegistry *
						     config,
						     const gchar *
						     country_code,
						     XklTwoConfigItemsProcessFunc
						     func, gpointer data);

/**
 * xkl_config_registry_foreach_language:
 * @config: the config registry
 * @func: (scope call): callback to call for every ISO 639-2 country code
 * @data: anything which can be stored into the pointer
 *
 * Enumerates languages for which layouts are available,
 * from the XML configuration registry
 */
	extern void xkl_config_registry_foreach_language(XklConfigRegistry
							 * config,
							 XklConfigItemProcessFunc
							 func,
							 gpointer data);

/**
 * xkl_config_registry_foreach_language_variant:
 * @config: the config registry
 * @language_code: language ISO code for which variants will be listed
 * @func: (scope call): callback to call for every country variant
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard layout variants for the language,
 * from the XML configuration registry
 */
	extern void
	 xkl_config_registry_foreach_language_variant(XklConfigRegistry *
						      config,
						      const gchar *
						      language_code,
						      XklTwoConfigItemsProcessFunc
						      func, gpointer data);


/**
 * xkl_config_registry_search_by_pattern:
 * @config: the config registry
 * @pattern: pattern to search for (NULL means "all")
 * @func: (scope call): callback to call for every matching layout/variant
 * @data: anything which can be stored into the pointer
 *
 * Enumerates keyboard layout/variants that match the pattern.
 * The layout/variant is considered as matching if one of the following
 * is true:
 * 1. Country description (from the country list or name) contains pattern as substring
 * 2. Language description (from the language list or name) contains pattern as substring
 */
	extern void
	 xkl_config_registry_search_by_pattern(XklConfigRegistry * config,
					       const gchar * pattern,
					       XklTwoConfigItemsProcessFunc
					       func, gpointer data);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
