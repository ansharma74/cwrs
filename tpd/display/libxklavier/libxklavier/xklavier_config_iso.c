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

#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "config.h"

#include "xklavier_private.h"

#define ISO_CODES_DATADIR    ISO_CODES_PREFIX "/share/xml/iso-codes"
#define ISO_CODES_LOCALESDIR ISO_CODES_PREFIX "/share/locale"

static GHashTable *country_code_names = NULL;
static GHashTable *lang_code_names = NULL;

typedef struct {
	const gchar *domain;
	const gchar **attr_names;
} LookupParams;

typedef struct {
	GHashTable *code_names;
	const gchar *tag_name;
	LookupParams *params;
} CodeBuildStruct;

static const char *countryLookupNames[] = { "alpha_2_code", NULL };
static const char *languageLookupNames[] =
    { "iso_639_2B_code", "iso_639_2T_code", NULL };

static LookupParams countryLookup = { "iso_3166", countryLookupNames };
static LookupParams languageLookup = { "iso_639", languageLookupNames };

static void
iso_codes_parse_start_tag(GMarkupParseContext * ctx,
			  const gchar * element_name,
			  const gchar ** attr_names,
			  const gchar ** attr_values,
			  gpointer user_data, GError ** error)
{
	const gchar *name;
	const gchar **san = attr_names, **sav = attr_values;
	CodeBuildStruct *cbs = (CodeBuildStruct *) user_data;

	/* Is this the tag we are looking for? */
	if (!g_str_equal(element_name, cbs->tag_name) ||
	    attr_names == NULL || attr_values == NULL) {
		return;
	}

	name = NULL;

	/* What would be the value? */
	while (*attr_names && *attr_values) {
		if (g_str_equal(*attr_names, "name")) {
			name = *attr_values;
			break;
		}

		attr_names++;
		attr_values++;
	}

	if (!name) {
		return;
	}

	attr_names = san;
	attr_values = sav;

	/* Walk again the attributes */
	while (*attr_names && *attr_values) {
		const gchar **attr = cbs->params->attr_names;
		/* Look through all the attributess we are interested in */
		while (*attr) {
			if (g_str_equal(*attr_names, *attr)) {
				if (**attr_values) {
					g_hash_table_insert
					    (cbs->code_names,
					     g_strdup(*attr_values),
					     g_strdup(name));
				}
			}
			attr++;
		}

		attr_names++;
		attr_values++;
	}
}

static GHashTable *
iso_code_names_init(LookupParams * params)
{
	GError *err = NULL;
	gchar *buf, *filename, *tag_name;
	gsize buf_len;
	CodeBuildStruct cbs;

	GHashTable *ht = g_hash_table_new_full(g_str_hash, g_str_equal,
					       g_free, g_free);

	tag_name = g_strdup_printf("%s_entry", params->domain);

	cbs.code_names = ht;
	cbs.tag_name = tag_name;
	cbs.params = params;

	bindtextdomain(params->domain, ISO_CODES_LOCALESDIR);
	bind_textdomain_codeset(params->domain, "UTF-8");

	filename =
	    g_strdup_printf("%s/%s.xml", ISO_CODES_DATADIR,
			    params->domain);
	if (g_file_get_contents(filename, &buf, &buf_len, &err)) {
		GMarkupParseContext *ctx;
		GMarkupParser parser = {
			iso_codes_parse_start_tag,
			NULL, NULL, NULL, NULL
		};

		ctx = g_markup_parse_context_new(&parser, 0, &cbs, NULL);
		if (!g_markup_parse_context_parse(ctx, buf, buf_len, &err)) {
			g_warning("Failed to parse '%s/%s.xml': %s",
				  ISO_CODES_DATADIR,
				  params->domain, err->message);
			g_error_free(err);
		}

		g_markup_parse_context_free(ctx);
		g_free(buf);
	} else {
		g_warning("Failed to load '%s/%s.xml': %s",
			  ISO_CODES_DATADIR, params->domain, err->message);
		g_error_free(err);
	}
	g_free(filename);
	g_free(tag_name);

	return ht;
}

typedef const gchar *(*DescriptionGetterFunc) (const gchar * code);

const gchar *
xkl_get_language_name(const gchar * code)
{
	const gchar *name;

	if (!lang_code_names) {
		lang_code_names = iso_code_names_init(&languageLookup);
	}

	name = g_hash_table_lookup(lang_code_names, code);
	if (!name) {
		return NULL;
	}

	return dgettext("iso_639", name);
}

const gchar *
xkl_get_country_name(const gchar * code)
{
	const gchar *name;

	if (!country_code_names) {
		country_code_names = iso_code_names_init(&countryLookup);
	}

	name = g_hash_table_lookup(country_code_names, code);
	if (!name) {
		return NULL;
	}

	return dgettext("iso_3166", name);
}

static void
xkl_config_registry_foreach_iso_code(XklConfigRegistry * config,
				     XklConfigItemProcessFunc func,
				     const gchar * xpath_exprs[],
				     DescriptionGetterFunc dgf,
				     gboolean to_upper, gpointer data)
{
	GHashTable *code_pairs;
	GHashTableIter iter;
	xmlXPathObjectPtr xpath_obj;
	const gchar **xpath_expr;
	gpointer key, value;
	XklConfigItem *ci;
	gint di;

	if (!xkl_config_registry_is_initialized(config))
		return;

	code_pairs = g_hash_table_new(g_str_hash, g_str_equal);

	for (xpath_expr = xpath_exprs; *xpath_expr; xpath_expr++) {
		for (di = 0; di < XKL_NUMBER_OF_REGISTRY_DOCS; di++) {
			gint ni;
			xmlNodePtr *node;
			xmlNodeSetPtr nodes;

			xmlXPathContextPtr xmlctxt =
			    xkl_config_registry_priv(config,
						     xpath_contexts[di]);
			if (xmlctxt == NULL)
				continue;

			xpath_obj =
			    xmlXPathEval((unsigned char *) *xpath_expr,
					 xmlctxt);
			if (xpath_obj == NULL)
				continue;

			nodes = xpath_obj->nodesetval;
			if (nodes == NULL) {
				xmlXPathFreeObject(xpath_obj);
				continue;
			}

			node = nodes->nodeTab;
			for (ni = nodes->nodeNr; --ni >= 0;) {
				gchar *iso_code =
				    (gchar *) (*node)->children->content;
				const gchar *description;
				iso_code =
				    to_upper ?
				    g_ascii_strup(iso_code,
						  -1) : g_strdup(iso_code);
				description = dgf(iso_code);
/* If there is a mapping to some ISO description - consider it as ISO code (well, it is just an assumption) */
				if (description)
					g_hash_table_insert
					    (code_pairs,
					     g_strdup
					     (iso_code),
					     g_strdup(description));
				g_free(iso_code);
				node++;
			}

			xmlXPathFreeObject(xpath_obj);
		}
	}

	g_hash_table_iter_init(&iter, code_pairs);
	ci = xkl_config_item_new();
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		g_strlcpy(ci->name, (const gchar *) key, sizeof(ci->name));
		g_strlcpy(ci->description, (const gchar *) value,
			  sizeof(ci->description));
		func(config, ci, data);
	}
	g_object_unref(G_OBJECT(ci));
	g_hash_table_unref(code_pairs);
}

void
xkl_config_registry_foreach_country(XklConfigRegistry *
				    config,
				    XklConfigItemProcessFunc
				    func, gpointer data)
{
	const gchar *xpath_exprs[] = {
		XKBCR_LAYOUT_PATH "/configItem/countryList/iso3166Id",
		XKBCR_LAYOUT_PATH "/configItem/name",
		NULL
	};

	xkl_config_registry_foreach_iso_code(config, func, xpath_exprs,
					     xkl_get_country_name, TRUE,
					     data);
}

void
xkl_config_registry_foreach_language(XklConfigRegistry *
				     config,
				     XklConfigItemProcessFunc
				     func, gpointer data)
{
	const gchar *xpath_exprs[] = {
		XKBCR_LAYOUT_PATH "/configItem/languageList/iso639Id",
		XKBCR_VARIANT_PATH "/configItem/languageList/iso639Id",
		NULL
	};

	xkl_config_registry_foreach_iso_code(config, func, xpath_exprs,
					     xkl_get_language_name, FALSE,
					     data);
}

void
xkl_config_registry_foreach_iso_variant(XklConfigRegistry *
					config,
					const gchar *
					iso_code,
					XklTwoConfigItemsProcessFunc
					func, gpointer data,
					const gchar * layout_xpath_exprs[],
					const gboolean
					should_code_be_lowered1[],
					const gchar *
					variant_xpath_exprs[],
					const gboolean
					should_code_be_lowered2[])
{
	xmlXPathObjectPtr xpath_obj;
	xmlNodeSetPtr nodes;
	const gchar **xpath_expr;
	const gboolean *is_low_id = should_code_be_lowered1;
	gchar *low_iso_code;

	if (!xkl_config_registry_is_initialized(config))
		return;

	low_iso_code = g_ascii_strdown(iso_code, -1);

	for (xpath_expr = layout_xpath_exprs; *xpath_expr;
	     xpath_expr++, is_low_id++) {
		const gchar *aic = *is_low_id ? low_iso_code : iso_code;
		gchar *xpe = g_strdup_printf(*xpath_expr, aic);
		gint di;
		GSList *processed_ids = NULL;

		for (di = 0; di < XKL_NUMBER_OF_REGISTRY_DOCS; di++) {
			xmlXPathContextPtr xmlctxt =
			    xkl_config_registry_priv(config,
						     xpath_contexts[di]);
			if (xmlctxt == NULL)
				continue;

			xpath_obj =
			    xmlXPathEval((unsigned char *) xpe, xmlctxt);
			if (xpath_obj == NULL)
				continue;

			nodes = xpath_obj->nodesetval;
			if (nodes != NULL) {
				gint ni;
				xmlNodePtr *node = nodes->nodeTab;
				XklConfigItem *ci = xkl_config_item_new();
				for (ni = nodes->nodeNr; --ni >= 0;) {
					if (xkl_read_config_item
					    (config, di, *node, ci)) {
						if (g_slist_find_custom
						    (processed_ids,
						     ci->name,
						     (GCompareFunc)
						     g_ascii_strcasecmp) ==
						    NULL) {
							func(config, ci,
							     NULL, data);
							processed_ids =
							    g_slist_append
							    (processed_ids,
							     g_strdup
							     (ci->name));
						}
					}
					node++;
				}
				g_object_unref(G_OBJECT(ci));
			}
			xmlXPathFreeObject(xpath_obj);
		}
		g_free(xpe);
	}

	is_low_id = should_code_be_lowered2;
	for (xpath_expr = variant_xpath_exprs; *xpath_expr;
	     xpath_expr++, is_low_id++) {
		const gchar *aic = *is_low_id ? low_iso_code : iso_code;
		gchar *xpe = g_strdup_printf(*xpath_expr, aic);
		gint di;
		for (di = 0; di < XKL_NUMBER_OF_REGISTRY_DOCS; di++) {
			xmlXPathContextPtr xmlctxt =
			    xkl_config_registry_priv(config,
						     xpath_contexts[di]);
			if (xmlctxt == NULL)
				continue;

			xpath_obj =
			    xmlXPathEval((unsigned char *) xpe, xmlctxt);
			if (xpath_obj == NULL)
				continue;

			nodes = xpath_obj->nodesetval;
			if (nodes != NULL) {
				gint ni;
				xmlNodePtr *node = nodes->nodeTab;
				XklConfigItem *ci = xkl_config_item_new();
				XklConfigItem *pci = xkl_config_item_new();
				for (ni = nodes->nodeNr; --ni >= 0;) {
					if (xkl_read_config_item
					    (config, di, *node, ci) &&
					    xkl_read_config_item
					    (config, di,
					     (*node)->parent->parent, pci))
						func(config, pci, ci,
						     data);
					node++;
				}
				g_object_unref(G_OBJECT(pci));
				g_object_unref(G_OBJECT(ci));
			}
			xmlXPathFreeObject(xpath_obj);
		}
		g_free(xpe);
	}

	g_free(low_iso_code);
}

void
xkl_config_registry_foreach_country_variant(XklConfigRegistry *
					    config,
					    const gchar *
					    country_code,
					    XklTwoConfigItemsProcessFunc
					    func, gpointer data)
{
	const gchar *layout_xpath_exprs[] = {
		XKBCR_LAYOUT_PATH "[configItem/name = '%s']",
		XKBCR_LAYOUT_PATH
		    "[configItem/countryList/iso3166Id = '%s']",
		NULL
	};
	const gboolean should_code_be_lowered1[] = { TRUE, FALSE };

	const gchar *variant_xpath_exprs[] = {
		XKBCR_VARIANT_PATH
		    "[configItem/countryList/iso3166Id = '%s']",
		XKBCR_VARIANT_PATH
		    "[../../configItem/name = '%s' and not(configItem/countryList/iso3166Id)]",
		XKBCR_VARIANT_PATH
		    "[../../configItem/countryList/iso3166Id = '%s' and not(configItem/countryList/iso3166Id)]",
		NULL
	};

	const gboolean should_code_be_lowered2[] = { FALSE, TRUE, FALSE };

	xkl_config_registry_foreach_iso_variant(config,
						country_code,
						func, data,
						layout_xpath_exprs,
						should_code_be_lowered1,
						variant_xpath_exprs,
						should_code_be_lowered2);
}

void
xkl_config_registry_foreach_language_variant(XklConfigRegistry *
					     config,
					     const gchar *
					     language_code,
					     XklTwoConfigItemsProcessFunc
					     func, gpointer data)
{
	const gchar *layout_xpath_exprs[] = {
		XKBCR_LAYOUT_PATH
		    "[configItem/languageList/iso639Id = '%s']",
		NULL
	};
	const gboolean should_code_be_lowered1[] = { FALSE };

	const gchar *variant_xpath_exprs[] = {
		XKBCR_VARIANT_PATH
		    "[configItem/languageList/iso639Id = '%s']",
		XKBCR_VARIANT_PATH
		    "[../../configItem/languageList/iso639Id = '%s' and not(configItem/languageList/iso639Id)]",
		NULL
	};
	const gboolean should_code_be_lowered2[] = { FALSE, FALSE };

	xkl_config_registry_foreach_iso_variant(config,
						language_code,
						func, data,
						layout_xpath_exprs,
						should_code_be_lowered1,
						variant_xpath_exprs,
						should_code_be_lowered2);
}
