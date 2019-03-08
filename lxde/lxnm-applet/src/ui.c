/**
 * Copyright (c) 2009 LxDE Developers, see the file AUTHORS for details.
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

#include <glib.h>
#include <gtk/gtk.h> 
#include <glib/gi18n.h>
#include "lxnm-applet.h"
#include "backend.h"
#include "ui.h"

extern LXNMApplet *applet;

void lxnm_applet_ui_wireless_dialog_response(GtkDialog* dlg, gint response, LXNMAppletWirelessDialog *wdlg)
{
	if(G_LIKELY(response == GTK_RESPONSE_OK)) {
		if (!wdlg->info->essid) {
			wdlg->info->essid = gtk_entry_get_text(wdlg->essid_entry);
		}

		wdlg->info->encryption = gtk_entry_get_text(wdlg->key_entry);

		/* send command to lxnm daemon to connect to wireless network */
		lxnm_backend_wireless_connect(applet->lxnm, wdlg->info->device->ifname,
						wdlg->info->essid, wdlg->info->bssid,
						wdlg->info->encryption, wdlg->info->keymgmt,
						wdlg->info->group, wdlg->info->pairwise);
	}

	g_free(wdlg->info->essid);
	g_free(wdlg->info->bssid);
	g_free(wdlg->info->encryption);
	g_free(wdlg->info->keymgmt);
	g_free(wdlg->info->group);
	g_free(wdlg->info->pairwise);
	g_source_remove_by_user_data(wdlg->key_entry);
	gtk_widget_destroy((GtkWidget*)wdlg->dlg);
}

void lxnm_applet_ui_wireless_dialog_cb(GtkWidget *widget, LXNMAppletWirelessDialogInfo *info)
{
	LXNMAppletWirelessDialog *wdlg;
	GtkWidget *layout;

	/* set flag */
	info->selected = TRUE;

	/* allocate */
	wdlg = (LXNMAppletWirelessDialog *)g_new0(LXNMAppletWirelessDialog, 1);
	wdlg->info = info;

	/* create dialog */
	wdlg->dlg = gtk_dialog_new_with_buttons(_("Setting Encryption Key"),
                                       NULL,
                                       GTK_DIALOG_NO_SEPARATOR,
                                       GTK_STOCK_OK, GTK_RESPONSE_OK,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       NULL );
	gtk_dialog_set_default_response(GTK_WINDOW(wdlg->dlg), GTK_RESPONSE_OK);
	gtk_window_set_position(GTK_WINDOW(wdlg->dlg), GTK_WIN_POS_CENTER);

	/* messages */
	wdlg->message = gtk_label_new(_("This wireless network was encrypted.\nYou must have the encryption key."));
	gtk_box_pack_start(((GtkDialog*)(wdlg->dlg))->vbox, wdlg->message, FALSE, FALSE, 8);

	/* layout */
	layout = gtk_table_new(6,8, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(layout), 10);
	gtk_table_set_col_spacings(GTK_TABLE(layout), 10);
	gtk_box_pack_start(((GtkDialog*)(wdlg->dlg))->vbox, layout, FALSE, FALSE, 8);

	/* ESSID */
	wdlg->essid_label = gtk_label_new(_("Access Point(ESSID):"));
	gtk_misc_set_alignment(GTK_MISC(wdlg->essid_label), 1, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(layout), wdlg->essid_label, 2,4, 0,1);
	if (info->essid) {
		wdlg->essid_entry = gtk_label_new(info->essid);
		gtk_misc_set_alignment(GTK_MISC(wdlg->essid_entry), 0, 0.5);
	} else {
		wdlg->essid_entry = gtk_entry_new();
		gtk_entry_set_activates_default(GTK_ENTRY(wdlg->essid_entry), TRUE);
	}

	gtk_table_attach_defaults(GTK_TABLE(layout), wdlg->essid_entry, 4,6, 0,1);

	/* Encryption key */
	wdlg->key_label = gtk_label_new(_("Encryption Key:"));
	gtk_misc_set_alignment(GTK_MISC(wdlg->key_label), 1, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(layout), wdlg->key_label, 2,4, 1,2);
	wdlg->key_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(wdlg->key_entry), FALSE);
	gtk_entry_set_activates_default(GTK_ENTRY(wdlg->key_entry), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(layout), wdlg->key_entry, 4,6, 1,2);

	/* g_signal */
	g_signal_connect(wdlg->dlg, "response", G_CALLBACK(lxnm_applet_ui_wireless_dialog_response), wdlg);
//    g_object_weak_ref(pwdgui->dlg, passwd_gui_free, pr);

	gtk_widget_show_all(wdlg->dlg);

	info->device->dlg = wdlg;
}

void lxnm_applet_ui_wireless_connect(GtkWidget *widget, LXNMAppletWirelessDialogInfo *info)
{
	if (!info->encrypt) {
		lxnm_backend_wireless_connect(applet->lxnm, info->device->ifname,
						info->essid, info->bssid,
						info->encryption, info->keymgmt,
						info->group, info->pairwise);
		return;
	}

	lxnm_applet_ui_wireless_dialog_cb(widget, info);
}

void lxnm_applet_ui_wireless_add_otherap(GtkWidget *widget, LXNMAppletDevice *device)
{
	LXNMAppletWirelessDialogInfo *info;

	info = (LXNMAppletWirelessDialogInfo *)g_new0(LXNMAppletWirelessDialogInfo, 1);
	info->device = device;

	lxnm_applet_ui_wireless_dialog_cb(widget, info);
}

void lxnm_applet_ui_menu_clean(LXNMAppletDevice *device)
{
	GList *data;
	LXNMAppletWirelessDialogInfo *info;
	LXNMAppletWirelessMenu *wmenu = (LXNMAppletWirelessMenu *)(device->menu);

	for (data=wmenu->aps;data;data=g_list_next(data)) {
		info = (LXNMAppletWirelessDialogInfo *)data->data;

		wmenu->aps = g_list_remove(wmenu->aps, info);
		if (!info->selected) {
			g_free(info->essid);
			g_free(info->bssid);
			g_free(info->encryption);
			g_free(info->keymgmt);
			g_free(info->group);
			g_free(info->pairwise);
			g_free(info);
		}
	}
	g_free(device->menu);
	device->menu = NULL;
}

void lxnm_applet_scan(LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data)
{
	gchar *p;
	gchar *content = (gchar *)data;
	LXNMAppletWirelessMenu *wmenu = (LXNMAppletWirelessMenu *)((LXNMAppletDevice *)(user_data))->menu;
	LXNMAppletWirelessDialogInfo *info;
	GtkWidget *menu_layout;
	GtkWidget *menu_box;
	GtkWidget *menu_item;
	GtkWidget *menu_label;
	GtkWidget *menu_icon;
	GtkWidget *menu_quality;
	GList *list;

	if (!wmenu)
		return;

	if (event & LXNM_EVENT_RELEASE) {
		if (!task->result) {
			gtk_menu_item_set_label(wmenu->status, _("Wireless Networks not found in range"));
			return;
			/* show name colums */
			//printf("%16s  %17s  Quality  Encryption  keymgmt  Group  Pairware\n", "ESSID", "BSSID");
		} else { 
			gtk_widget_hide(wmenu->status);
			return;
		}
	}

	if (!task->result)
		task->result = TRUE;

	info = (LXNMAppletWirelessDialogInfo *)g_new0(LXNMAppletWirelessDialogInfo, 1);
	info->device = user_data;

	/* ESSID */
	p = strtok(content, " ");
	info->essid = LXNM_GET_ESSID(p);

	menu_item = gtk_image_menu_item_new();
	menu_layout = gtk_hbox_new(FALSE, 0);

	menu_label = gtk_label_new(info->essid);
	gtk_misc_set_alignment(GTK_MISC(menu_label), 0, 0.5);
	gtk_label_set_justify(menu_label, GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(menu_layout, menu_label, TRUE, TRUE, 0);

	/* BSSID */
	p = strtok(NULL, " ");
	info->bssid = g_strdup(p);

	/* QUALITY */
	p = strtok(NULL, " ");
	menu_quality = gtk_progress_bar_new();
	gtk_widget_set_size_request(menu_quality, 100, -1);
	gtk_progress_bar_set_orientation(menu_quality, GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_progress_bar_set_fraction(menu_quality, (gfloat)(((float)atoi(p))/100));
	gtk_box_pack_start(menu_layout, menu_quality, FALSE, FALSE, 0);

	/* ENCRYPTION */
	if ((p = strtok(NULL, " "))) {
		if (strcmp(p, "OFF")!=0) {
			info->encrypt = TRUE;
			info->encryption = g_strdup(p);
			menu_icon = gtk_image_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_LOCK]);
			gtk_widget_set_size_request(menu_icon, 18, 18);
			gtk_image_menu_item_set_image(menu_item, menu_icon);

			if (strcmp(p, "WPA")==0) {
				/* key management */
				p = strtok(NULL, " ");
				info->keymgmt = g_strdup(p);

				/* Group */
				p = strtok(NULL, " ");
				info->group = g_strdup(p);

				/* Pairwise */
				p = strtok(NULL, " ");
				info->pairwise = g_strdup(p);
			}
		}
	}

	gtk_container_add(GTK_CONTAINER(menu_item), menu_layout);
	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(lxnm_applet_ui_wireless_connect), info);
	gtk_menu_insert(GTK_MENU(wmenu->menu), menu_item, 2 + wmenu->apcount++);
	gtk_widget_show_all(menu_item);

	/* add to access point list */
	wmenu->aps = g_list_append(wmenu->aps, info);
}

void lxnm_applet_ui_device_repair_cb(GtkWidget *widget, LXNMAppletDevice *device)
{
	switch(device->type) {
		case LXNM_CONNECTION_TYPE_ETHERNET:
		case LXNM_CONNECTION_TYPE_PPP:
			lxnm_backend_ethernet_repair(device->parent->lxnm, device->ifname);
			break;
		case LXNM_CONNECTION_TYPE_WIRELESS:
			break;
	}
}

void lxnm_applet_ui_device_disable_cb(GtkWidget *widget, LXNMAppletDevice *device)
{
	switch(device->type) {
		case LXNM_CONNECTION_TYPE_ETHERNET:
		case LXNM_CONNECTION_TYPE_PPP:
			lxnm_backend_ethernet_down(device->parent->lxnm, device->ifname);
			break;
		case LXNM_CONNECTION_TYPE_WIRELESS:
			break;
	}
}

GtkWidget *lxnm_applet_ui_wireless_menupopup(LXNMAppletDevice *device)
{
	LXNMAppletWirelessMenu *wmenu;
	GtkWidget *menu_item;
	GtkWidget *menu_icon;

	/* allocate */
	wmenu = (LXNMAppletWirelessMenu *)g_new0(LXNMAppletWirelessMenu, 1);
	wmenu->aps = NULL;

	/* create a new menu */
	wmenu->menu = gtk_menu_new();
	g_signal_connect(wmenu->menu, "selection-done", gtk_widget_destroy, NULL);

	/* scanning access point */
	lxnm_backend_add_watch(device->parent->lxnm, LXNM_WIRELESS_SCAN, LXNM_EVENT_IN | LXNM_EVENT_RELEASE, lxnm_applet_scan, device, LXNM_WATCH_MODE_ONCE);
	lxnm_backend_scan(device->parent->lxnm, device->ifname);

	/* status for scanning */
	menu_item = gtk_image_menu_item_new();
	gtk_menu_item_set_label(menu_item, _("Wireless Netowrks"));
	gtk_widget_set_sensitive(menu_item, FALSE);
	gtk_menu_append(GTK_MENU(wmenu->menu), menu_item);
	/* separetor */
	gtk_menu_append(GTK_MENU(wmenu->menu), gtk_separator_menu_item_new());

	/* status for scanning */
	wmenu->status = gtk_image_menu_item_new();
	menu_icon = gtk_image_new_from_animation(applet->animation[LXNM_APPLET_ANIMATION_SCANNING]);
	gtk_image_menu_item_set_image(wmenu->status, menu_icon);
	gtk_menu_item_set_label(wmenu->status, _("Scanning Access Point..."));
	gtk_menu_append(GTK_MENU(wmenu->menu), wmenu->status);

	/* separetor */
	gtk_menu_append(GTK_MENU(wmenu->menu), gtk_separator_menu_item_new());

	/* item for connecting to hidden access point */
	wmenu->connect_btn = gtk_image_menu_item_new();
	g_signal_connect(G_OBJECT(wmenu->connect_btn), "activate", G_CALLBACK(lxnm_applet_ui_wireless_add_otherap), device);
	gtk_menu_item_set_label(wmenu->connect_btn, _("Connect to other Access Point"));
	menu_icon = gtk_image_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_ADD]);
	gtk_image_menu_item_set_image(wmenu->connect_btn, menu_icon);
	gtk_menu_append(GTK_MENU(wmenu->menu), wmenu->connect_btn);

	gtk_widget_show_all(wmenu->menu);

	device->menu = wmenu;
	g_object_weak_ref(wmenu->menu, lxnm_applet_ui_menu_clean, device);
	return wmenu->menu;
}

gint lxnm_applet_ui_statusicon_press(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *event_button;

	g_return_val_if_fail(event != NULL, FALSE);

	if (event->type == GDK_BUTTON_PRESS) {
		event_button = (GdkEventButton *)event;

		if (event_button->button==1) {
			/* wireless */
			if (((LXNMAppletDevice *)(user_data))->type==LXNM_CONNECTION_TYPE_WIRELESS) {
				if (!((LXNMAppletDevice *)(user_data))->menu)
					gtk_menu_popup(lxnm_applet_ui_wireless_menupopup(user_data), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
				return TRUE;
			}
		} else if (event_button->button==3) {
			GtkWidget *menu;
			GtkWidget *menu_item;
			GtkWidget *menu_icon;

			/* create a new menu */
			menu = gtk_menu_new();
			g_signal_connect(menu, "selection-done", gtk_widget_destroy, NULL);

			/* Repair */
			menu_item = gtk_image_menu_item_new();
			gtk_menu_item_set_label(menu_item, _("Repair"));
			g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(lxnm_applet_ui_device_repair_cb), user_data);
			menu_icon = gtk_image_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_REPAIR]);
			gtk_image_menu_item_set_image(menu_item, menu_icon);
			gtk_menu_append(GTK_MENU(menu), menu_item);

			/* disable */
			menu_item = gtk_image_menu_item_new();
			gtk_menu_item_set_label(menu_item, _("Disable"));
			g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(lxnm_applet_ui_device_disable_cb), user_data);
			menu_icon = gtk_image_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_DISABLE]);
			gtk_image_menu_item_set_image(menu_item, menu_icon);
			gtk_menu_append(GTK_MENU(menu), menu_item);
			gtk_widget_show_all(menu);
			gtk_menu_popup(menu, NULL, NULL, NULL, NULL, event_button->button, event_button->time);

		}
	}

	return FALSE;
}

