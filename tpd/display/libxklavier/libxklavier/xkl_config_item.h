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

#ifndef __XKL_CONFIG_ITEM_H__
#define __XKL_CONFIG_ITEM_H__

#include <glib-object.h>

/*
 * Maximum name length, including '\'0' character
 */
#define XKL_MAX_CI_NAME_LENGTH 32

/*
 * Maximum short description length, including '\\0' character
 * (this length is in bytes, so for UTF-8 encoding in 
 * XML file the actual maximum length can be smaller)
 */
#define XKL_MAX_CI_SHORT_DESC_LENGTH 10

/*
 * Maximum description length, including '\\0' character
 * (this length is in bytes, so for UTF-8 encoding in 
 * XML file the actual maximum length can be smaller)
 */
#define XKL_MAX_CI_DESC_LENGTH 192

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

	typedef struct _XklConfigItem XklConfigItem;
	typedef struct _XklConfigItemClass XklConfigItemClass;

#define XKL_TYPE_CONFIG_ITEM             (xkl_config_item_get_type ())
#define XKL_CONFIG_ITEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XKL_TYPE_CONFIG_ITEM, XklConfigItem))
#define XKL_CONFIG_ITEM_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), XKL_CONFIG_ITEM,  XklConfigItemClass))
#define XKL_IS_CONFIG_ITEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XKL_TYPE_CONFIG_ITEM))
#define XKL_IS_CONFIG_ITEM_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), XKL_TYPE_CONFIG_ITEM))
#define XKL_CONFIG_ITEM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XKL_TYPE_CONFIG_ITEM, XklConfigItemClass))

/**
 * The configuration item. Corresponds to XML element "configItem".
 */
	struct _XklConfigItem {
/**
 * The superclass object
 */
		GObject parent;
/**
 * The configuration item name. Corresponds to XML element "name".
 */
		gchar name[XKL_MAX_CI_NAME_LENGTH];

/**
 * The configuration item short description. Corresponds to XML element "shortDescription".
 */
		gchar short_description[XKL_MAX_CI_DESC_LENGTH];

/**
 * The configuration item description. Corresponds to XML element "description".
 */
		gchar description[XKL_MAX_CI_DESC_LENGTH];
	};

/**
 * Extra property for the XklConfigItem, defining whether the group allows multiple selection
 */
#define XCI_PROP_ALLOW_MULTIPLE_SELECTION "allowMultipleSelection"

/**
 * Extra property for the XklConfigItem, defining the vendor (used for models)
 */
#define XCI_PROP_VENDOR "vendor"

/**
 * Extra property for the XklConfigItem, defining the list of countries (used for layouts/variants)
 */
#define XCI_PROP_COUNTRY_LIST "countryList"

/**
 * Extra property for the XklConfigItem, defining the list of languages (used for layouts/variants)
 */
#define XCI_PROP_LANGUAGE_LIST "languageList"

/**
 * Extra property for the XklConfigItem, defining whether that item is exotic(extra)
 */
#define XCI_PROP_EXTRA_ITEM "extraItem"

/**
 * The XklConfigItem class, derived from GObject
 */
	struct _XklConfigItemClass {
    /**
     * The superclass
     */
		GObjectClass parent_class;
	};

/**
 * xkl_config_item_get_type:
 * 
 * Get type info for XklConfigItem
 *
 * Returns: GType for XklConfigItem
 */
	extern GType xkl_config_item_get_type(void);

/**
 * xkl_config_item_new:
 *
 * Create new XklConfigItem
 *
 * Returns: new instance
 */
	extern XklConfigItem *xkl_config_item_new(void);

/**
 * xkl_config_item_set_name:
 *
 * @item: the XklConfigItem object to be changed
 * @name: (transfer none) (allow-none): Name (max. 32 characters); can be NULL.
 *
 * Change the @name field of a XklConfigItem. This is mostly useful for
 * language bindings, in C you can manipulate the member directly.
 */
	extern void xkl_config_item_set_name(XklConfigItem * item,
					     const gchar * name);

/**
 * xkl_config_item_set_short_description:
 *
 * @item: the XklConfigItem object to be changed
 * @short_description: (transfer none) (allow-none): Short Description (max. 10
 *        characters); can be NULL.
 *
 * Change the @short_description field of a XklConfigItem. This is mostly useful for
 * language bindings, in C you can manipulate the member directly.
 */
	extern void xkl_config_item_set_short_description(XklConfigItem * item,
							  const gchar * short_description);
/**
 * xkl_config_item_set_description:
 *
 * @item: the XklConfigItem object to be changed
 * @description: (transfer none) (allow-none): Description (max. 192
 *        characters); can be NULL.
 *
 * Change the @description field of a XklConfigItem. This is mostly useful for
 * language bindings, in C you can manipulate the member directly.
 */
	extern void xkl_config_item_set_description(XklConfigItem * item,
						    const gchar * description);

/**
 * xkl_get_country_name:
 * @code: ISO 3166 Alpha 2 code: 2 chars, uppercase (US, RU, FR, ...)
 *
 * Get localized country name, from ISO code
 *
 * Returns: localized country name (USA, Russia, France, ... translated)
 */
	extern const gchar * xkl_get_country_name(const gchar * code);

/**
 * xkl_get_language_name:
 * @code: ISO 639 2B or 2T code: 3 chars, lowercase (eng, rus, fra, ...)
 *
 * Get localized language name, from ISO code
 *
 * Returns: localized country name (English, Russiam, French, ... translated)
 */
	extern const gchar * xkl_get_language_name(const gchar * code);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
