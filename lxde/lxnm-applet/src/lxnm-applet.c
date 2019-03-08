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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <stdio.h> 
#include <glib.h> 
#include <gtk/gtk.h> 
#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include "lxnm-applet.h"
#include "backend.h"
#include "ui.h"

LXNMApplet *applet;

LXNMAppletDevice *lxnm_applet_device_register(LXNMApplet *applet, const gchar *ifname, ConnectionType conntype, DeviceType devicetype, void *deviceinfo)
{
	LXNMAppletDevice *device;

	/* allocate */
	device = (LXNMAppletDevice *)g_new0(LXNMAppletDevice, 1);
	device->parent = applet;
	device->ifname = ifname ? g_strdup(ifname) : NULL;
	device->type = conntype;
	device->ref = 1;

	/* status icon */
	if (devicetype==LXNM_DEVICE_TYPE_CONNECTION) {
		device->statusicon = gtk_status_icon_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_DISCONNECT]);
		gtk_status_icon_set_tooltip(device->statusicon, device->ifname);
		gtk_status_icon_set_visible(device->statusicon, FALSE);
	} else if (devicetype==LXNM_DEVICE_TYPE_MODEM) {
		device->statusicon = gtk_status_icon_new_from_pixbuf(applet->image[LXNM_APPLET_IMAGE_MODEM]);
		//gtk_status_icon_set_tooltip(device->statusicon, ((LXNMAppletModemInfo *)deviceinfo)->devname);
		gtk_status_icon_set_tooltip(device->statusicon, _("GSM Modem"));
		gtk_status_icon_set_visible(device->statusicon, TRUE);
	}

	g_signal_connect(device->statusicon, "button_press_event", G_CALLBACK(lxnm_applet_ui_statusicon_press), device);

	/* informations for specific device */
	device->devicetype = devicetype;
	device->deviceinfo = deviceinfo;

	applet->devices = g_list_append(applet->devices, device);

	/* get device status */
	lxnm_backend_status(applet->lxnm, device->ifname, devicetype);

	return device;
}

gboolean lxnm_applet_update(LXNMApplet *applet)
{
	GList *data;
	LXNMAppletDevice *device;

	for (data=applet->devices;data;data=g_list_next(data)) {
		device = (LXNMAppletDevice *)data->data;

		if (!device->statusicon) {
			device->statusicon = gtk_status_icon_new();
			gtk_status_icon_set_from_file(device->statusicon, PACKAGE_DATA_DIR  "/lxnm-applet/ns-disconnect.png");
			gtk_status_icon_set_tooltip(device->statusicon, device->ifname);
			gtk_status_icon_set_visible(device->statusicon, TRUE);
		}
	}

	return TRUE;
}

void lxnm_applet_status(LXNMBackend *lxnm, gint event, gpointer data, gpointer user_data)
{
	gint cmd;
	gboolean found;
	gchar *ifname;
	gchar *p;
	gchar *content = (gchar *)data;
	GList *list;
	GList *next_list;
	LXNMApplet *applet = (LXNMApplet *)user_data;
	LXNMAppletDevice *device;

	/* interface */
	ifname = strtok(content, " ");

	/* parsing */
	cmd = atoi(strtok(NULL, " "));

	/* internal interface */
	if (strcmp(ifname, LXNM_INTERFACE)==0) {
		switch(cmd) {
			case LXNM_STATUS_DEVICE_LIST:
				p = strtok(NULL, " ");
				/* connections */
				if (atoi(p)==LXNM_DEVICE_TYPE_CONNECTION) {
					while(p = strtok(NULL, " ")) {
						printf("*** interface status: %s\n", p);
						found = FALSE;
						for (list=applet->devices;list;list=g_list_next(list)) {
							device = (LXNMAppletDevice *)list->data;
							if (!device->ifname)
								continue;

							if (strcmp(device->ifname, p)==0) {
								found = TRUE;
								device->ref++;
								break;
							}
						}

						/* new device */
						if (!found) {
							device = lxnm_applet_device_register(applet, p, LXNM_CONNECTION_TYPE_ETHERNET, LXNM_DEVICE_TYPE_CONNECTION, NULL);
							device->ref++;
						}
					}
	
					/* update device list */
					for (list=applet->devices;list;list=next_list) {
						next_list = g_list_next(list);
						device = (LXNMAppletDevice *)list->data;
						if (device->ref>1) {
							device->ref--;
						} else if (device->ifname) {
							g_free(device->ifname);

							/* if it is modem */
							if (device->devicetype==LXNM_DEVICE_TYPE_MODEM&&device->deviceinfo) {
								/* change icon */
								gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_MODEM]);
							} else {
								g_object_unref(device->statusicon);
								if (!device->deviceinfo) {
									applet->devices = g_list_delete_link(applet->devices, list);
									g_free(device);
								}
							}
						}
					}
					return;
				} else { /* devices */
					while(p = strtok(NULL, " ")) {
						printf("*** device status: %s\n", p);
						found = FALSE;
						for (list=applet->devices;list;list=g_list_next(list)) {
							device = (LXNMAppletDevice *)list->data;
							/* not pure connection */
							if (device->deviceinfo)
								if (strcmp(((LXNMAppletDeviceInfo *)(device->deviceinfo))->devname, p)) {
									found = TRUE;
									device->ref++;
									break;
								}
						}

						/* new device */
						if (!found) {
							LXNMAppletDeviceInfo *deviceinfo;
							deviceinfo = (LXNMAppletDeviceInfo *)g_new0(LXNMAppletDeviceInfo, 1);
							deviceinfo->devname = g_strdup(p);
							device = lxnm_applet_device_register(applet, NULL, LXNM_CONNECTION_TYPE_UNKNOWN, LXNM_DEVICE_TYPE_UNKNOWN, deviceinfo);
							device->ref++;
						}
					}
	
					/* update device list */
					for (list=applet->devices;list;list=next_list) {
						next_list = g_list_next(list);
						device = (LXNMAppletDevice *)list->data;
						if (device->ref>1) {
							device->ref--;
						} else if (device->ifname) {
							applet->devices = g_list_delete_link(applet->devices, list);
							g_free(device->ifname);
							g_object_unref(device->statusicon);
							g_free(((LXNMAppletDeviceInfo *)(device->deviceinfo))->devname);
							g_free(device->deviceinfo);
							g_free(device);
						}
					}
					return;
				}
		}
	}

	/* find device */
	for (list=applet->devices;list;list=g_list_next(list)) {
		device = (LXNMAppletDevice *)list->data;
		if (!device->ifname)
			continue;

		if (strcmp(device->ifname, ifname)==0)
			break;
		else
			device = NULL;
	}

	if (device)
		switch(cmd) {
			case LXNM_STATUS_MESSAGE:
				break;
			case LXNM_STATUS_NETDEV_INFO:
				p = strtok(NULL, "\t");
				device->type = atoi(p);
				/* get this interface's modem if it is ppp */
				if (device->deviceinfo==NULL) {
					if (device->type==LXNM_CONNECTION_TYPE_PPP) {
						lxnm_backend_add_watch(applet->lxnm, LXNM_PPP_INFORMATION, LXNM_EVENT_IN, lxnm_applet_ppp_info, applet, LXNM_WATCH_MODE_ONCE);
						lxnm_backend_ppp_info(applet->lxnm, LXNM_PPP_INFORMATION_TYPE_INTERFACE, device->ifname);
					}
				}
//				printf("%s\n", p);
				break;
			case LXNM_STATUS_NETDEV_ENABLE:
				printf("%s Enable\n", device->ifname);
				gtk_status_icon_set_visible(device->statusicon, TRUE);
				break;
			case LXNM_STATUS_NETDEV_DISABLE:
				printf("%s Disable\n", device->ifname);
				gtk_status_icon_set_visible(device->statusicon, FALSE);
				break;
			case LXNM_STATUS_NETDEV_PLUGGED:
				printf("%s Plugged\n", device->ifname);
				break;
			case LXNM_STATUS_NETDEV_UNPLUG:
				printf("%s Unplug\n", device->ifname);
				break;
			case LXNM_STATUS_NETDEV_CONNECTING:
				printf("%s Connecting\n", device->ifname);
				break;
			case LXNM_STATUS_NETDEV_CONNECTED:
				printf("%s Connected\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_CONNECTED]);
				break;
			case LXNM_STATUS_NETDEV_DISCONNECT:
				printf("%s Disconnect\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_DISCONNECT]);
				break;
			case LXNM_STATUS_NETDEV_PROBLEM:
				break;
			case LXNM_STATUS_NETDEV_BOTHRS:
				printf("%s Send and Recv\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_RTBOTH]);
				break;
			case LXNM_STATUS_NETDEV_SENDDATA:
				printf("%s Send\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_TRANSDATA]);
				break;
			case LXNM_STATUS_NETDEV_RECVDATA:
				printf("%s Recv\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_RECVDATA]);
				break;
			case LXNM_STATUS_NETDEV_NORT:
				printf("%s No Send and Recv\n", device->ifname);
				gtk_status_icon_set_from_pixbuf(device->statusicon, applet->image[LXNM_APPLET_IMAGE_CONNECTED]);
				break;
			case LXNM_STATUS_NETDEV_WIRELESS_QUALITY:
				break;
		}
}

void lxnm_applet_list(LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data)
{
	gchar *p;
	gchar *content = (gchar *)data;
	LXNMApplet *applet = (LXNMApplet *)user_data;

	p = strtok(content, " ");
	if (!p)
		return;

	do {
		lxnm_applet_device_register(applet, p, LXNM_CONNECTION_TYPE_ETHERNET, LXNM_DEVICE_TYPE_CONNECTION, NULL);
	} while(p = strtok(NULL, " "));

	/* need all status of connection */
	lxnm_backend_status(lxnm, LXNM_INTERFACE, LXNM_DEVICE_TYPE_CONNECTION);
}

void lxnm_applet_list_modem(LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data)
{
	gchar *p;
	gchar *content = (gchar *)data;
	LXNMApplet *applet = (LXNMApplet *)user_data;
	LXNMAppletDevice *curdevice;
	LXNMAppletModemInfo *modeminfo;
	GList *list;

	/* Initializing Modem Information */
	modeminfo = (LXNMAppletModemInfo *)g_new0(LXNMAppletModemInfo, 1);
	/* device name */
	p = strtok(content, " ");
	modeminfo->devname = g_strdup(p);
	/* modem type */
	p = strtok(NULL, " ");
	modeminfo->type = atoi(p);

	/* no interface name because connection is not created yet */
	lxnm_applet_device_register(applet, NULL, LXNM_CONNECTION_TYPE_UNKNOWN, LXNM_DEVICE_TYPE_MODEM, modeminfo);

	lxnm_backend_add_watch(applet->lxnm, LXNM_PPP_INFORMATION, LXNM_EVENT_IN, lxnm_applet_ppp_info, applet, LXNM_WATCH_MODE_ONCE);
	lxnm_backend_ppp_info(applet->lxnm, LXNM_PPP_INFORMATION_TYPE_DEVICE, modeminfo->devname);
}

void lxnm_applet_ppp_info(LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data)
{
	gchar *p;
	gchar *content = (gchar *)data;
	LXNMApplet *applet = (LXNMApplet *)user_data;
	LXNMAppletModemInfo *modeminfo;
	LXNMAppletDevice *curdevice;
	LXNMAppletDevice *device;
	GList *list;

	/* result code */
	p = strtok(content, " ");
	if (atoi(p)==0) {
		return;
	}

	/* interface */
	p = strtok(NULL, " ");
	printf("PPP INTERFACE ===%s\n", p);

	/* find interface */
	for (list=applet->devices;list;list=g_list_next(list)) {
		curdevice = (LXNMAppletDevice *)list->data;

		if (!curdevice->ifname)
			continue;

		if (strcmp(curdevice->ifname, p)==0) {
			break;
		}
	}

	/* device name */
	p = strtok(NULL, " ");
	printf("PPP DEVICE ===%s\n", p);

	/* find device */
	for (list=applet->devices;list;list=g_list_next(list)) {
		device = (LXNMAppletDevice *)list->data;
		if (!device->ifname&&device->devicetype==LXNM_DEVICE_TYPE_MODEM) {
			modeminfo = device->deviceinfo;

			if (strcmp(modeminfo->devname, p)==0) {
				curdevice->devicetype = device->devicetype;
				curdevice->deviceinfo = device->deviceinfo;

				/* remove device icon */
				gtk_status_icon_set_visible(device->statusicon, FALSE);
				applet->devices = g_list_delete_link(applet->devices, list);
				g_object_unref(device->statusicon);
				g_free(device);
				break;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
	bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
	textdomain ( GETTEXT_PACKAGE );
#endif

	applet = (LXNMApplet *)g_new0(LXNMApplet, 1);
	applet->devices = NULL;

	/* initialing connection with LXNM daemon */
	if ((applet->lxnm = lxnm_backend_open())==NULL)
		return 1;

	/* preload images */
	lxnm_applet_images_init(applet);

	/* get device list */
	lxnm_backend_add_watch(applet->lxnm, LXNM_DEVICE_LIST, LXNM_EVENT_IN, lxnm_applet_list, applet, LXNM_WATCH_MODE_ONCE);
	lxnm_backend_add_watch(applet->lxnm, LXNM_DEVICE_STATUS, LXNM_EVENT_IN, lxnm_applet_status, applet, LXNM_WATCH_MODE_NORMAL);
	lxnm_backend_list(applet->lxnm, LXNM_DEVICE_TYPE_CONNECTION);

	/* get modem list */
	lxnm_backend_add_watch(applet->lxnm, LXNM_DEVICE_LIST, LXNM_EVENT_IN, lxnm_applet_list_modem, applet, LXNM_WATCH_MODE_ONCE);
	lxnm_backend_list(applet->lxnm, LXNM_DEVICE_TYPE_MODEM);

	{
		GMainLoop *loop = g_main_loop_new(NULL, FALSE);
		g_main_loop_run(loop);
	}

	lxnm_backend_close(applet->lxnm);
	return 0;
}
