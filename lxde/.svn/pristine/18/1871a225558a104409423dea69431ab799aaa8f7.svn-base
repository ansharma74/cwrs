/**
 * Copyright (c) 2008 LxDE Developers, see the file AUTHORS for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <poll.h>
#include "xkb-config.h"
#include <lxpanel/plugin.h>

#ifndef  DFLT_XKB_LAYOUT
#define DFLT_XKB_LAYOUT "us"
#endif

GtkTreeIter iter, child;

enum
{
	GROUP_MAP,
	VARIANT_MAP,
	DESCRIPTION
};

typedef struct
{
  const gchar    *group;
  const gchar    *description;
}
Layouts;

static Layouts data[] =
{
  { "us",     		"U.S.English" },
  { "es",     		"Spain/Mexico" },
  { "cf",     		"Canada/Quebec" },
  { "hu",     		"Hungria"},
  { "it",     		"Italia" },
  { "ru",     		"Russia"},
  { "uk",     		"United Kingdom" },
  { "fr-latin1",    "France"},
  { "be-latin1",    "Belgique"  },
  { "br-abnt2",     "Brazil" },
  { "croat",        "Croat" },
  { "cz-lat2",      "Czech"},
  { "de_CH-latin1", "Schweizer Deutsch" },
  { "nl2",          "Netherlands" },
  { "no-latin1",    "Norway"},
  { "pl2",          "Poland" },
  { "pt-latin1",    "Portugal" },
  { "se-lat6",      "Sweden" },
  { "sg-latin1",    "Singapore"},
};

gchar *layouts_name[50];
gint layout_indx;

GtkWidget *treeview;
GtkTreeSelection *selection;

/*gui implementation*/
void panel_config_save(Panel* panel);

char *
xci_desc_to_utf8 (XklConfigItem * ci)
{
    char *sd = g_strstrip (ci->description);
    return sd[0] == 0 ? g_strdup (ci->name) : g_strdup(sd);
}

static void
xkb_settings_add_variant_to_available_layouts_tree (XklConfigRegistry * config_registry,
                                                    XklConfigItem * config_item,
                                                    GtkTreeStore *treestore)
{
  char *utf_variant_name = xci_desc_to_utf8 (config_item);

  gtk_tree_store_append (treestore, &child, &iter);
  gtk_tree_store_set (treestore, &child,
              0, utf_variant_name,
              1, config_item->name, -1);
  g_free (utf_variant_name);
}

static void
xkb_settings_add_layout_to_available_layouts_tree (XklConfigRegistry * config_registry,
                                                   XklConfigItem * config_item,
                                                   GtkTreeStore *treestore)
{
	char *utf_layout_name = xci_desc_to_utf8 (config_item);

	gtk_tree_store_append (treestore, &iter, NULL);
	gtk_tree_store_set (treestore, &iter,
				0, utf_layout_name,
				1, config_item->name, -1);
	g_free (utf_layout_name);

	xkl_config_registry_foreach_layout_variant (config_registry, config_item->name,
				   (ConfigItemProcessFunc)xkb_settings_add_variant_to_available_layouts_tree,
							   treestore);
}

gchar *
xkb_settings_layout_dialog_run ()
{
    GtkWidget *dialog;
    GtkTreeStore *treestore;
    GtkWidget *t_view = gtk_tree_view_new ();
    GtkCellRenderer *renderer;
    GtkWidget *scrolledw;
    XklConfigRegistry *registry;


    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;

    registry = xkb_config_get_xkl_registry ();

    dialog = gtk_dialog_new_with_buttons(_("Add layout"),
                            NULL,
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_STOCK_CANCEL,
                            GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,
                            GTK_RESPONSE_OK,
                            NULL);

    treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    xkl_config_registry_foreach_layout (registry, (ConfigItemProcessFunc)
            xkb_settings_add_layout_to_available_layouts_tree, treestore);

    renderer = gtk_cell_renderer_text_new ();

    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL,
                                    renderer,
                                    "text",
                                    0,
                                    NULL);
    gtk_tree_view_set_model (GTK_TREE_VIEW (t_view),
               GTK_TREE_MODEL (treestore));
    gtk_tree_view_append_column (GTK_TREE_VIEW (t_view), column);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (treestore),
                                        0, GTK_SORT_ASCENDING);

    scrolledw = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), scrolledw);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_show (scrolledw);

    gtk_container_add (GTK_CONTAINER (scrolledw), t_view);
    gtk_widget_show (GTK_WIDGET (t_view));

    gtk_window_set_default_size(GTK_WINDOW (dialog), 360, 420);
    gtk_widget_show (dialog);

    int response = gtk_dialog_run (GTK_DIALOG (dialog));


    if (response == GTK_RESPONSE_OK)
    {
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (t_view));
        gchar *id, *desc;
        gchar *strings[2];

        gtk_tree_selection_get_selected (selection, &model, &iter);
        gtk_tree_model_get (model, &iter, 0, &desc, 1, &id, -1);

        path = gtk_tree_model_get_path (model, &iter);
        if (gtk_tree_path_get_depth (path) == 1)
        {
            strings[0] = id;
            strings[1] = "";
        }
        else
        {
        	gchar *group_desc;

            strings[1] = id;
            gtk_tree_path_up(path);
            gtk_tree_model_get_iter(model, &iter, path);
            gtk_tree_model_get (model, &iter, 0, &group_desc, 1, &id, -1);
            strings[0] = id;
            desc = g_strconcat(group_desc, " (", desc, ")", NULL);
        }

        gtk_widget_destroy (dialog);
        return g_strconcat(strings[0], ",", strings[1], ",", desc, NULL);

    }
    gtk_widget_destroy (dialog);
    return NULL;
}

static void lxkb_add_layout(GtkWidget *btn, GtkWidget *combo)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	gchar **sel_layout;
	sel_layout = g_strsplit(xkb_settings_layout_dialog_run(), ",", -1);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	gtk_list_store_set (GTK_LIST_STORE (model),
						&iter,
						0, FALSE,
						1, sel_layout[GROUP_MAP],
						2, sel_layout[DESCRIPTION],
						-1);

	xkb_config_add_layout(sel_layout[GROUP_MAP], sel_layout[VARIANT_MAP]);

//		check[indx] = TRUE;
}

void lxkb_remove_layout (GtkWidget *menuitem, gpointer userdata)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(treeview));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
	  gint layout_selected;
	  GtkTreePath *path;

	  path = gtk_tree_model_get_path (model, &iter);
	  layout_selected = gtk_tree_path_get_indices (path)[0];
	  gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

	  xkb_config_remove_group(layout_selected);

	  gtk_tree_path_free (path);
	}
}

static void fixed_toggled (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter, iter_old;
  GtkTreePath *path, *path_old;
  gint *ind, cur;

  cur = xkb_config_get_current_group();

  path = gtk_tree_path_new_from_string (path_str);
  path_old = gtk_tree_path_new_from_indices(cur, -1);

  ind = gtk_tree_path_get_indices(path);

  if (*ind != cur)
  {
	  /* get toggled iter */
	  gtk_tree_model_get_iter (model, &iter, path);
	  gtk_tree_model_get_iter (model, &iter_old, path_old);

	/* set new value */
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, TRUE, -1);
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter_old, 0, FALSE, -1);

	  xkb_config_set_group(*ind);
  }

  /* clean up */
  gtk_tree_path_free (path);
  gtk_tree_path_free (path_old);
}


static void lxkb_add_columns_selected_layouts ()
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(treeview));

  /* column for fixed toggles */
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled",
		    G_CALLBACK (fixed_toggled), model);

  gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);

  column = gtk_tree_view_column_new_with_attributes ("default",
						     renderer,
						     "active", 0,
						     NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

  /* column for bug numbers */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Layout",
						     renderer,
						     "text",
						     1,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, 1);
  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

  /* column for severities */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Description",
						     renderer,
						     "text",
						     2,
						     NULL);
  gtk_tree_view_column_set_sort_column_id (column, 2);
  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

}

static GtkTreeModel *lxkb_create_model_selected_layouts (void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  gchar *group_map, *variant_map;
  gint i, group_count, current_group;

  /* create list store */
  store = gtk_list_store_new (3,
			      G_TYPE_BOOLEAN,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

  /* add data to the list store */

  current_group = xkb_config_get_current_group();
  group_count = xkb_config_get_group_count();

  for (i = 0; i < group_count; i++)
  {
	  group_map = xkb_config_get_group_map(i);
	  variant_map = xkb_config_get_variant_map(i);

	  gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  0, (i != current_group ? FALSE : TRUE),
			  1, group_map,
			  2, xkb_config_get_layout_desc(group_map, variant_map),
			  -1);
  }

  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *lxkb_create_combo_box_model()
{
	gint  i;
	GtkTreeIter it;
	GtkTreeStore *store;
	XklConfigRegistry *registry;
	registry = xkb_config_get_xkl_registry ();

	store = gtk_tree_store_new(1, G_TYPE_STRING);

//    xkl_config_registry_foreach_layout (registry, (ConfigItemProcessFunc)
//            xkb_settings_add_layout_to_available_layouts_tree, store);

	for (i = 0; i < G_N_ELEMENTS(data); i++)
	{
		gtk_tree_store_append(store, &it, NULL);
		gtk_tree_store_set(store, &it, 0, data[i].description, -1);
	}

	return GTK_TREE_MODEL(store);
}

GtkWidget *lxkb_content_area()
{
	GtkWidget *vbox,
			  *vbox_tv,
			  *hbox_layouts,
			  *hbox_btn,
			  *button,
			  *sw,
			  *frame_layouts,
			  *frame_tv,
			  *check,
			  *combo_layouts;

	GtkTreeModel *model;
	GtkCellRenderer *renderer;


	vbox = gtk_vbox_new (FALSE, 8);

	/*Combo with layouts*/

	frame_layouts = gtk_frame_new (_("Select one input language"));

	hbox_layouts = gtk_hbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), frame_layouts, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_layouts), 5);
	gtk_container_add(GTK_CONTAINER(frame_layouts), hbox_layouts);

	model = lxkb_create_combo_box_model();

	combo_layouts = gtk_combo_box_new_with_model(model);
	g_object_unref (model);

	gtk_combo_box_set_active (GTK_COMBO_BOX (combo_layouts), 0);

	gtk_container_add(GTK_CONTAINER(hbox_layouts), combo_layouts);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_layouts), renderer, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_layouts), renderer,
			 "text", 0,	NULL);

	/*Treeview & buttons*/

	frame_tv = gtk_frame_new (_("Selected Layouts"));

	vbox_tv = gtk_vbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(vbox), frame_tv);

	gtk_container_set_border_width (GTK_CONTAINER (vbox_tv), 5);
	gtk_container_add(GTK_CONTAINER(frame_tv), vbox_tv);

	/*scrolled window*/

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
										   GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
										  GTK_POLICY_NEVER,
										  GTK_POLICY_AUTOMATIC);

	gtk_box_pack_start (GTK_BOX (vbox_tv), sw, TRUE, TRUE, 0);

	gtk_widget_set_size_request(GTK_WIDGET(sw), -1, 150);

	/* creating tree view */

	model = lxkb_create_model_selected_layouts ();

	treeview = gtk_tree_view_new_with_model (model);
	g_object_unref (model);

	gtk_container_add(GTK_CONTAINER(sw), treeview);

	lxkb_add_columns_selected_layouts ();


	/*creating buttons*/

	hbox_btn = gtk_hbutton_box_new ();

	gtk_container_set_border_width (GTK_CONTAINER (hbox_btn), 5);
	gtk_container_add(GTK_CONTAINER(vbox_tv), hbox_btn);

	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox_btn), GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbox_btn), 8);

	button = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_container_add (GTK_CONTAINER (hbox_btn), button);

	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(lxkb_add_layout), NULL);

	button = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_container_add (GTK_CONTAINER (hbox_btn), button);

	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(lxkb_remove_layout), NULL);

	button = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	gtk_container_add (GTK_CONTAINER (hbox_btn), button);

	/*creating checkbox*/

	check = gtk_check_button_new_with_label(_("Manage layouts per application"));

	gtk_box_pack_start(GTK_BOX(vbox), check, FALSE, FALSE, 0 );

	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

	return vbox;
}

/*Plugin implementation*/

void change_current_layout(t_xkb_settings * xkb)
{
    xkb_config_next_group ();
}


static void on_configuration_changed(Plugin * p)
{
    t_xkb_settings * xkb = (t_xkb_settings *)p->priv;
    panel_draw_label_text(xkb -> plugin -> panel,
                          xkb -> tray_icon,
                          g_utf8_strup( xkb -> current ,-1),
                          TRUE);
}

void xkb_state_changed(gint current_group, gboolean config_changed, 
                       gpointer user_data)
{
    t_xkb_settings * xkb = (t_xkb_settings *)user_data;
    update_display(xkb);
}

static gboolean
tray_icon_press(GtkWidget *widget, GdkEventButton *event, t_xkb_settings *xkb)
{
    if( event->button == 3 )  /* right button */
    {
        GtkMenu* popup = (GtkMenu*)lxpanel_get_panel_menu( xkb->plugin->panel,
                          xkb->plugin, FALSE );
        gtk_menu_popup( popup, NULL, NULL, NULL, NULL,
                        event->button, event->time );
        return TRUE;
    }

    change_current_layout(xkb);

    return TRUE;
}
    

static void lxkb_config( Plugin *p )
{
	GtkWidget *dlg,
			  *box;

	t_xkb_settings * xkb = (t_xkb_settings *)p->priv;

    dlg = gtk_dialog_new_with_buttons( _("Keyboard layouts settings"),
                                                      NULL,
                                                      GTK_DIALOG_MODAL,
                                                      GTK_STOCK_CLOSE,
                                                      GTK_RESPONSE_CLOSE,
                                                      NULL);

    GValue val = {0};

    g_value_init (&val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val, FALSE);

    g_object_set_property (G_OBJECT(dlg), "resizable", &val);

    gtk_container_add ((GtkBox*)((GtkDialog*)dlg)->vbox,
                       lxkb_content_area());

    g_object_weak_ref(G_OBJECT(dlg), (GWeakNotify) panel_config_save, p->panel);
    gtk_widget_show_all (dlg);
    gint response = gtk_dialog_run((GtkDialog*)dlg);

    gtk_widget_destroy(dlg);
}


static void lxkb_save_config(Plugin* p, FILE* fp)
{
	gint g_count, i;
	gchar *groups,
		  *variants;

	g_count = xkb_config_get_group_count();

	if (fp && g_count > 0)
	{
		groups = g_strdup(xkb_config_get_group_map(0));
		variants = g_strdup(xkb_config_get_variant_map(0));

		for (i = 1; i < g_count; i++)
		{
			groups = g_strconcat(groups, ",", xkb_config_get_group_map(i), NULL);
			variants= g_strconcat(variants, ",", xkb_config_get_variant_map(i), NULL);
		}
		lxpanel_put_str(fp, "groups", groups);
		lxpanel_put_str(fp, "variants", variants);
	}
}

static void lxkb_open_config(char **fp, t_xkb_settings * xkb)
{
	if (fp)
	{
		line s;
		s.len = 256;

		lxpanel_get_line(fp, &s);

		if (s.type == LINE_BLOCK_END)
		{
			xkb->kbd_config->layouts = g_strdup(DFLT_XKB_LAYOUT);
		} else
		{
			/*groups line*/
			if (!g_ascii_strcasecmp(s.t[0], "groups"))
				xkb->kbd_config->layouts = g_strdup(s.t[1]);
			else
				ERR( "Xkb SWitcher: unknown var %s\n", s.t[0]);

			/*variants line*/
			lxpanel_get_line(fp, &s);

			if (!g_ascii_strcasecmp(s.t[0], "variants"))
				xkb->kbd_config->variants = g_strdup(s.t[1]);
			else
				ERR( "Xkb SWitcher: unknown var %s\n", s.t[0]);

			/*options line*/
/*			lxpanel_get_line(fp, &s);

			if (!g_ascii_strcasecmp(s.t[0], "descriptions"))
				xkb->kbd_config->descriptions = g_strdup(s.t[1]);
			else
				ERR( "Xkb SWitcher: unknown var %s\n", s.t[0]);*/

		}
	} else
		xkb->kbd_config->layouts = g_strdup(DFLT_XKB_LAYOUT);

	xkb_config_update_settings (xkb);
}

static int lxkb_constructor(Plugin *p, char **fp)
{
    t_xkb_settings * xkb;

    xkb = g_new0(t_xkb_settings, 1);
    xkb -> plugin = p;
    g_return_val_if_fail(xkb != NULL, 0);
    p->priv = xkb;

    xkb -> group_policy = GROUP_POLICY_GLOBAL;
	
    xkb -> never_modify_config = FALSE; 

    xkb -> current = strdup(DFLT_XKB_LAYOUT);

    xkb -> tray_icon = gtk_label_new(xkb -> current);

    xkb -> config_changed = FALSE;

    xkb -> next = 1;

    xkb_config_initialize ( xkb , xkb_state_changed , xkb);

    /*open configuration file*/

    lxkb_open_config(fp, xkb);

    /* main */
    
    xkb -> mainw = gtk_event_box_new();

    gtk_widget_add_events(xkb -> mainw, GDK_BUTTON_PRESS_MASK);
    
    gtk_widget_set_size_request( xkb -> mainw, 24, 24 );
    
    gtk_container_add(GTK_CONTAINER(xkb->mainw), xkb->tray_icon);

    gtk_widget_show_all(xkb->mainw);

    gtk_widget_set_tooltip_text( xkb->mainw, _("KeyBoard Layout Switcher"));

    /* store the created plugin widget in plugin->pwid */

    p->pwid = xkb -> mainw;
	
	/* connect signals */	
	
	g_signal_connect(G_OBJECT(xkb->mainw), "button-press-event",
                         G_CALLBACK(tray_icon_press), xkb);
	
}

static void lxkb_destructor(Plugin *p)
{
    t_xkb_settings * xkb = (t_xkb_settings *) p->priv;
    xkb_config_finalize ();
    g_free(xkb);
}

PluginClass lxkb_plugin_class = {

    PLUGINCLASS_VERSIONING,

    type : "lxkb",
    name : N_("KeyBoard Layout Switcher"),
    version: "0.1",
    description : "Switch KeyBoard Layout",

    constructor : lxkb_constructor,
    destructor  : lxkb_destructor,
    config 		: lxkb_config,
    save 		: lxkb_save_config,

    panel_configuration_changed : on_configuration_changed

};
