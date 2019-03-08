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
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier_private.h"

static GObjectClass *parent_class = NULL;

static void xkl_config_rec_destroy(XklConfigRec * data);

G_DEFINE_TYPE(XklConfigItem, xkl_config_item, G_TYPE_OBJECT)

static void
xkl_config_item_init(XklConfigItem * this)
{
}

static void
xkl_config_item_class_init(XklConfigItemClass * klass)
{
}

XklConfigItem *
xkl_config_item_new(void)
{
	return
	    XKL_CONFIG_ITEM(g_object_new
			    (xkl_config_item_get_type(), NULL));
}

void
xkl_config_item_set_name(XklConfigItem * item,
			 const gchar * name)
{
	if (name != NULL)
		strncpy (item->name, name, XKL_MAX_CI_SHORT_DESC_LENGTH-1);
	else
		item->name[0] = '\0';
}

void
xkl_config_item_set_short_description(XklConfigItem * item,
				      const gchar * short_description)
{
	if (short_description != NULL)
		strncpy (item->short_description, short_description, XKL_MAX_CI_DESC_LENGTH-1);
	else
		item->short_description[0] = '\0';
}

void
xkl_config_item_set_description(XklConfigItem * item,
				const gchar * description)
{
	if (description != NULL)
		strncpy (item->description, description, XKL_MAX_CI_NAME_LENGTH-1);
	else
		item->description[0] = '\0';
}

G_DEFINE_TYPE(XklConfigRec, xkl_config_rec, G_TYPE_OBJECT)

static void
xkl_config_rec_finalize(GObject * obj)
{
	XklConfigRec *this = (XklConfigRec *) obj;
	xkl_config_rec_destroy(this);
	G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
xkl_config_rec_class_init(XklConfigRecClass * klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;
	parent_class = g_type_class_peek_parent(object_class);
	object_class->finalize = xkl_config_rec_finalize;
}

XklConfigRec *
xkl_config_rec_new(void)
{
	return
	    XKL_CONFIG_REC(g_object_new(xkl_config_rec_get_type(), NULL));
}

static gboolean
xkl_strings_equal(gchar * p1, gchar * p2)
{
	if (p1 == p2)
		return TRUE;
	if ((p1 == NULL && p2 != NULL) || (p1 != NULL && p2 == NULL))
		return FALSE;
	return !g_ascii_strcasecmp(p1, p2);
}

static gboolean
xkl_lists_equal(gchar ** items1, gchar ** items2)
{
	if (items1 == items2)
		return TRUE;

	if ((items1 == NULL && items2 != NULL) ||
	    (items1 != NULL && items2 == NULL))
		return FALSE;

	while (*items1 != NULL && *items2 != NULL)
		if (!xkl_strings_equal(*items1++, *items2++))
			return FALSE;

	return (*items1 == NULL && *items2 == NULL);
}

static gboolean
xkl_engine_get_default_names_prop(XklEngine * engine,
				  char **rules_file_out,
				  XklConfigRec * data)
{
	if (rules_file_out != NULL)
		*rules_file_out = g_strdup(XKB_DEFAULT_RULESET);
	data->model = g_strdup(xkl_engine_priv(engine, default_model));
/* keeping Nvariants = Nlayouts */
	data->layouts = g_new0(char *, 2);
	data->layouts[0] =
	    g_strdup(xkl_engine_priv(engine, default_layout));
	data->variants = g_new0(char *, 2);
	data->variants[0] = g_strdup("");
	data->options = g_new0(char *, 1);
	return TRUE;
}

gboolean
xkl_config_rec_get_full_from_server(char **rules_file_out,
				    XklConfigRec * data,
				    XklEngine * engine)
{
	gboolean rv = xkl_config_rec_get_from_root_window_property(data,
								   xkl_engine_priv
								   (engine,
								    base_config_atom),
								   rules_file_out,
								   engine);

	if (!rv)
		rv = xkl_engine_get_default_names_prop(engine,
						       rules_file_out,
						       data);

	return rv;
}

gboolean
xkl_config_rec_equals(XklConfigRec * data1, XklConfigRec * data2)
{
	if (data1 == data2)
		return TRUE;
	if (!xkl_strings_equal(data1->model, data2->model))
		return FALSE;
	if (!xkl_lists_equal(data1->layouts, data2->layouts))
		return FALSE;
	if (!xkl_lists_equal(data1->variants, data2->variants))
		return FALSE;
	return xkl_lists_equal(data1->options, data2->options);
}

void
xkl_config_rec_set_layouts(XklConfigRec * data,
			   const gchar ** new_layouts)
{
        g_strfreev (data->layouts);
	data->layouts = g_strdupv ((gchar**) new_layouts);
}

void
xkl_config_rec_set_variants(XklConfigRec * data,
			   const gchar ** new_variants)
{
        g_strfreev (data->variants);
	data->variants = g_strdupv ((gchar**) new_variants);
}

void
xkl_config_rec_set_options(XklConfigRec * data,
			   const gchar ** new_options)
{
        g_strfreev (data->options);
	data->options = g_strdupv ((gchar**) new_options);
}

void
xkl_config_rec_set_model(XklConfigRec * data,
			 const gchar * new_model)
{
        g_free (data->model);
	data->model = g_strdup (new_model);
}

void
xkl_config_rec_init(XklConfigRec * data)
{
	/* clear the structure VarDefsPtr... */
	data->model = NULL;
	data->layouts = data->variants = data->options = NULL;
}

void
xkl_config_rec_destroy(XklConfigRec * data)
{
	if (data->model != NULL)
		g_free(data->model);

	g_strfreev(data->layouts);
	g_strfreev(data->variants);
	g_strfreev(data->options);
	data->layouts = data->variants = data->options = NULL;
}

void
xkl_config_rec_reset(XklConfigRec * data)
{
	xkl_config_rec_destroy(data);
	xkl_config_rec_init(data);
}

gboolean
xkl_config_rec_get_from_server(XklConfigRec * data, XklEngine * engine)
{
	return xkl_config_rec_get_full_from_server(NULL, data, engine);
}

gboolean
xkl_config_rec_get_from_backup(XklConfigRec * data, XklEngine * engine)
{
	return xkl_config_rec_get_from_root_window_property(data,
							    xkl_engine_priv
							    (engine,
							     backup_config_atom),
							    NULL, engine);
}

gboolean
xkl_engine_backup_names_prop(XklEngine * engine)
{
	gboolean rv = TRUE;
	gchar *rf = NULL;
	XklConfigRec *data = xkl_config_rec_new();
	gboolean cgp = FALSE;

	if (xkl_config_rec_get_from_root_window_property
	    (data, xkl_engine_priv(engine, backup_config_atom), NULL,
	     engine)) {
		g_object_unref(G_OBJECT(data));
		return TRUE;
	}
	/* "backup" property is not defined */
	xkl_config_rec_reset(data);
	cgp = xkl_config_rec_get_full_from_server(&rf, data, engine);

	if (cgp) {
		if (!xkl_config_rec_set_to_root_window_property
		    (data, xkl_engine_priv(engine, backup_config_atom),
		     rf, engine)) {
			xkl_debug(150,
				  "Could not backup the configuration");
			rv = FALSE;
		}
		if (rf != NULL)
			g_free(rf);
	} else {
		xkl_debug(150,
			  "Could not get the configuration for backup");
		rv = FALSE;
	}
	g_object_unref(G_OBJECT(data));
	return rv;
}

gboolean
xkl_restore_names_prop(XklEngine * engine)
{
	gboolean rv = TRUE;
	gchar *rf = NULL;
	XklConfigRec *data = xkl_config_rec_new();

	if (!xkl_config_rec_get_from_root_window_property
	    (data, xkl_engine_priv(engine, backup_config_atom), NULL,
	     engine)) {
		g_object_unref(G_OBJECT(data));
		return FALSE;
	}

	if (!xkl_config_rec_set_to_root_window_property
	    (data, xkl_engine_priv(engine, base_config_atom), rf,
	     engine)) {
		xkl_debug(150, "Could not backup the configuration");
		rv = FALSE;
	}
	g_object_unref(G_OBJECT(data));
	return rv;
}

gboolean
xkl_config_rec_get_from_root_window_property(XklConfigRec * data,
					     Atom rules_atom,
					     gchar ** rules_file_out,
					     XklEngine * engine)
{
	Atom real_prop_type;
	int fmt;
	unsigned long nitems, extra_bytes;
	char *prop_data = NULL, *out;
	Status rtrn;

	/* no such atom! */
	if (rules_atom == None) {	/* property cannot exist */
		xkl_last_error_message = "Could not find the atom";
		return FALSE;
	}

	rtrn =
	    XGetWindowProperty(xkl_engine_get_display(engine),
			       xkl_engine_priv(engine, root_window),
			       rules_atom, 0L, XKB_RF_NAMES_PROP_MAXLEN,
			       False, XA_STRING, &real_prop_type, &fmt,
			       &nitems, &extra_bytes,
			       (unsigned char **) (void *) &prop_data);
	/* property not found! */
	if (rtrn != Success) {
		xkl_last_error_message = "Could not get the property";
		return FALSE;
	}
	/* set rules file to "" */
	if (rules_file_out)
		*rules_file_out = NULL;

	/* has to be array of strings */
	if ((extra_bytes > 0) || (real_prop_type != XA_STRING)
	    || (fmt != 8)) {
		if (prop_data)
			XFree(prop_data);
		xkl_last_error_message = "Wrong property format";
		return FALSE;
	}

	if (!prop_data) {
		xkl_last_error_message = "No properties returned";
		return FALSE;
	}

	/* rules file */
	out = prop_data;
	if (out && (*out) && rules_file_out)
		*rules_file_out = g_strdup(out);
	out += strlen(out) + 1;

	/* if user is interested in rules only - don't waste the time */
	if (!data) {
		XFree(prop_data);
		return TRUE;
	}

	if ((out - prop_data) < nitems) {
		if (*out)
			data->model = g_strdup(out);
		out += strlen(out) + 1;
	}

	if ((out - prop_data) < nitems) {
		xkl_config_rec_split_layouts(data, out);
		out += strlen(out) + 1;
	}

	if ((out - prop_data) < nitems) {
		gint nv, nl;
		gchar **layout, **variant;
		xkl_config_rec_split_variants(data, out);
		/*
		   Now have to ensure that number of variants matches the number of layouts
		   The 'remainder' is filled with NULLs (not ""s!)
		 */

		nv = g_strv_length(data->variants);
		nl = g_strv_length(data->layouts);
		if (nv < nl) {
			data->variants = g_realloc(data->variants,
						   (nl +
						    1) * sizeof(char *));
			memset(data->variants + nv + 1, 0,
			       (nl - nv) * sizeof(char *));
		}
		/* take variants from layouts like ru(winkeys) */
		layout = data->layouts;
		variant = data->variants;
		while (*layout != NULL && *variant == NULL) {
			gchar *varstart = g_strstr_len(*layout, -1, "(");
			if (varstart != NULL) {
				gchar *varend =
				    g_strstr_len(varstart, -1, ")");
				if (varend != NULL) {
					gint varlen = varend - varstart;
					gint laylen = varstart - *layout;
					/* I am not sure - but I assume variants in layout have priority */
					gchar *var = *variant =
					    (*variant !=
					     NULL) ? g_realloc(*variant,
							       varlen) :
					    g_new(gchar, varlen);
					memcpy(var, varstart + 1,
					       --varlen);
					var[varlen] = '\0';
					/* Resize the original layout */
					*layout =
					    g_realloc(*layout, laylen + 1);
					(*layout)[laylen] = '\0';
				}
			}
			layout++;
			variant++;
		}
		out += strlen(out) + 1;
	}

	if ((out - prop_data) < nitems) {
		xkl_config_rec_split_options(data, out);
	}
	XFree(prop_data);
	return TRUE;
}

/* taken from XFree86 maprules.c */
gboolean
xkl_config_rec_set_to_root_window_property(const XklConfigRec * data,
					   Atom rules_atom,
					   gchar * rules_file,
					   XklEngine * engine)
{
	gint len;
	gchar *pval;
	gchar *next;
	gchar *all_layouts = xkl_config_rec_merge_layouts(data);
	gchar *all_variants = xkl_config_rec_merge_variants(data);
	gchar *all_options = xkl_config_rec_merge_options(data);
	Display *display;

	len = (rules_file ? strlen(rules_file) : 0);
	len += (data->model ? strlen(data->model) : 0);
	len += (all_layouts ? strlen(all_layouts) : 0);
	len += (all_variants ? strlen(all_variants) : 0);
	len += (all_options ? strlen(all_options) : 0);
	if (len < 1) {
		if (all_layouts)
			g_free(all_layouts);
		if (all_variants)
			g_free(all_variants);
		if (all_options)
			g_free(all_options);
		return TRUE;
	}

	len += 5;		/* trailing NULs */

	pval = next = g_new(char, len + 1);
	if (!pval) {
		xkl_last_error_message = "Could not allocate buffer";
		if (all_layouts != NULL)
			g_free(all_layouts);
		if (all_variants != NULL)
			g_free(all_variants);
		if (all_options != NULL)
			g_free(all_options);
		return FALSE;
	}
	if (rules_file) {
		strcpy(next, rules_file);
		next += strlen(rules_file);
	}
	*next++ = '\0';
	if (data->model) {
		strcpy(next, data->model);
		next += strlen(data->model);
	}
	*next++ = '\0';
	if (data->layouts) {
		strcpy(next, all_layouts);
		next += strlen(all_layouts);
	}
	*next++ = '\0';
	if (data->variants) {
		strcpy(next, all_variants);
		next += strlen(all_variants);
	}
	*next++ = '\0';
	if (data->options) {
		strcpy(next, all_options);
		next += strlen(all_options);
	}
	*next++ = '\0';
	if ((next - pval) != len) {
		xkl_debug(150, "Illegal final position: %d/%d\n",
			  (next - pval), len);
		if (all_layouts != NULL)
			g_free(all_layouts);
		if (all_variants != NULL)
			g_free(all_variants);
		if (all_options != NULL)
			g_free(all_options);
		g_free(pval);
		xkl_last_error_message = "Internal property parsing error";
		return FALSE;
	}

	display = xkl_engine_get_display(engine);
	XChangeProperty(display, xkl_engine_priv(engine, root_window),
			     rules_atom, XA_STRING, 8, PropModeReplace,
			     (unsigned char *) pval, len);
	XSync(display, False);
#if 0
	for (i = len - 1; --i >= 0;)
		if (pval[i] == '\0')
			pval[i] = '?';
	XklDebug(150, "Stored [%s] of length %d to [%s] of %X\n", pval,
		 len, propName, _xklRootWindow);
#endif
	if (all_layouts != NULL)
		g_free(all_layouts);
	if (all_variants != NULL)
		g_free(all_variants);
	if (all_options != NULL)
		g_free(all_options);
	g_free(pval);
	return TRUE;
}
