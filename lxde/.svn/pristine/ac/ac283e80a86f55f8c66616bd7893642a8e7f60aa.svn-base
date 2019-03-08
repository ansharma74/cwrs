/*
 *      Copyright 2009 Fred Chien <cfsghost@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lxnm.h"
#include "thread.h"
#include "status.h"
#include "handler.h"
#include "timer.h"
#include "device.h"

extern LxND *lxnm;
gboolean lxnm_status_update();

static InterfaceInfo *lxnm_status_info_init(ConnectionType devtype)
{
	switch(devtype) {
		case LXNM_CONNECTION_TYPE_PRIDEVICE:
			return NULL;
		case LXNM_CONNECTION_TYPE_ETHERNET:
		case LXNM_CONNECTION_TYPE_PPP:
			return (InterfaceInfo *)g_new0(EthernetInfo, 1);
		case LXNM_CONNECTION_TYPE_WIRELESS:
			return (InterfaceInfo *)g_new0(WirelessInfo, 1);
	}
}

static void *lxnm_status_info_free(InterfaceInfo *ifinfo, ConnectionType type)
{
	EthernetInfo *info = ifinfo;

	/* release ethernet info */
	g_free(info->mac);
	g_free(info->ipaddr);
	g_free(info->dest);
	g_free(info->bcast);
	g_free(info->mask);

	/* release wireless info */
	if (type==LXNM_CONNECTION_TYPE_WIRELESS) {
		WirelessInfo *wifiinfo = ifinfo;

		g_free(wifiinfo->essid);
		g_free(wifiinfo->bssid);
	}

	g_free(ifinfo);
}

static void *lxnm_status_client_list_free(InterfaceStatus *ifstat)
{
	GList *list;
	GList *list_next;

	/* delete all links */
	for (list=ifstat->clients;list;list=list_next) {
		list_next = g_list_next(list);
		ifstat->clients = g_list_delete_link(ifstat->clients, list);
	}
}

void lxnm_status_register(const gchar *ifname, ConnectionType conntype, LXNMClient *client, DeviceType devtype)
{
	InterfaceStatus *ifstat;
	GList *list;
	gboolean has_reg = FALSE;

	for (list=lxnm->ifstatus;list;list=g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;

		/* the interface has been registered */
		if (strcmp(ifstat->ifname, ifname)==0) {
			ifstat->ref++;
			ifstat->clients = g_list_append(ifstat->clients, client);
			has_reg = TRUE;
			break;
		}
	}

	/* it is a new interface we want */
	if (!has_reg) {
		ifstat = g_new0(InterfaceStatus, 1);
		ifstat->ref = 1;
		ifstat->ifname = g_strdup(ifname);
		ifstat->type = conntype;
		ifstat->info = lxnm_status_info_init(conntype);
		ifstat->clients = g_list_append(ifstat->clients, client);

		/* add to list */
		lxnm->ifstatus = g_list_append(lxnm->ifstatus, ifstat);
	}

	if (!lxnm->status_timer_id)
		lxnm->status_timer_id = lxnm_timer_add(lxnm_status_update);
}

void lxnm_status_unregister_from_ifname(const gchar *ifname)
{
	InterfaceStatus *ifstat;
	GList *list;

	for (list=lxnm->ifstatus;list;list=g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;
		if (strcmp(ifstat->ifname, ifname)==0) {
			/* frees all */
			lxnm->ifstatus = g_list_delete_link(lxnm->ifstatus, list);

			/* release info */
			lxnm_status_info_free(ifstat->info, ifstat->type);
			lxnm_status_client_list_free(ifstat);
			g_free(ifstat->ifname);
			g_free(ifstat);
			break;
		}
	}
}

void lxnm_status_unregister_client(LXNMClient *client)
{
	InterfaceStatus *ifstat;
	GList *list;
	GList *list_next = NULL;

	/* find client from all device listener */
	for (list=lxnm->ifstatus;list;list=list_next) {
		list_next = g_list_next(list);
		ifstat = (InterfaceStatus *)list->data;

		/* remove client from the list */
		ifstat->clients = g_list_remove(ifstat->clients, client);
		ifstat->ref--;

		/* there is no client */
		if (ifstat->ref==0) {
			lxnm->ifstatus = g_list_delete_link(lxnm->ifstatus, list);

			/* release */
			lxnm_status_client_list_free(ifstat);
			g_free(ifstat->ifname);
			g_free(ifstat->info);
			g_free(ifstat);
		}
	}
}

gboolean lxnm_status_exists(const gchar *ifname)
{
	InterfaceStatus *ifstat;
	GList *list;

	for (list=lxnm->ifstatus;list;list=g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;

		/* the interface has been registered */
		if (strcmp(ifstat->ifname, ifname)==0)
			return TRUE;
	}

	return FALSE;
}

InterfaceStatus *lxnm_status_get_ifstat(const gchar *ifname)
{
	InterfaceStatus *ifstat;
	GList *list;

	for (list=lxnm->ifstatus;list;list=g_list_next(list)) {
		ifstat = (InterfaceStatus *)list->data;

		/* the interface has been registered */
		if (strcmp(ifstat->ifname, ifname)==0)
			return ifstat;
	}

	return NULL;
}

ConnectionType lxnm_status_get_connection_type(const gchar *ifname)
{
	if (lxnm_iswireless(ifname))
		return LXNM_CONNECTION_TYPE_WIRELESS;
	else if (lxnm_isppp(ifname))
		return LXNM_CONNECTION_TYPE_PPP;
	else if (strcmp(ifname, LXNM_INTERFACE)==0)
		return LXNM_CONNECTION_TYPE_PRIDEVICE;
	else
		return LXNM_CONNECTION_TYPE_ETHERNET;
}

void lxnm_status_push(InterfaceStatus *ifstat, const gchar *msg)
{
	LXNMClient *client;
	GList *list;

	/* push status to all of client */
	for (list=ifstat->clients;list;list=g_list_next(list)) {
		client = (LXNMClient *)list->data;

		lxnm_send_message_with_client(client, msg);
	}
}
#ifdef OS_Linux
gint lxnm_status_info_update_internal(InterfaceStatus *ifstat)
{

}
#endif

gint lxnm_status_info_update_from_external_program(InterfaceStatus *ifstat)
{
	int i = 0;
	int len;
	int pfd[2];
	int status;
	pid_t pid;

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	setenv("LXNM_IFNAME", ifstat->ifname, 1);
	unsetenv("LXNM_CMDID");

	/* fork to execute external program or scripts */
	pid = fork();
	if (pid<0) {
		return -2;
	} else if (pid==0) {
		/* child process */
		dup2(pfd[1], STDOUT_FILENO);
		close(STDIN_FILENO);

		switch(ifstat->type) {
			case LXNM_CONNECTION_TYPE_ETHERNET:
			case LXNM_CONNECTION_TYPE_PPP:
				execlp(lxnm->setting->eth_info->value, lxnm->setting->eth_info->value, NULL);
				break;
			case LXNM_CONNECTION_TYPE_WIRELESS:
				execlp(lxnm->setting->wifi_info->value, lxnm->setting->wifi_info->value, NULL);
				break;
		}

		exit(0);
	} else {
		gchar msg[256];
		gchar buffer[1024];
		gchar *temp_new = NULL;
		gchar *temp = NULL;
		gchar *value;

		/* parent process */
		close(pfd[1]);

		while((len=read(pfd[0], buffer, 1023))>0) {
			/* make sure the string is null-terminated */
			buffer[len] = '\0';

			temp_new = temp ? g_strconcat(temp, buffer, NULL) : g_strdup(buffer);
			g_free(temp);
			temp = temp_new;

			value = strtok(temp, "\t");
			while(value!=NULL) {
				switch(i) {
					case 0:
						if (((EthernetInfo *)ifstat->info)->enable!=(atoi(value+2) ? TRUE : FALSE)) {
							((EthernetInfo *)ifstat->info)->enable = ((EthernetInfo *)ifstat->info)->enable ? FALSE : TRUE;
							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, ((EthernetInfo *)ifstat->info)->enable ? LXNM_STATUS_NETDEV_ENABLE : LXNM_STATUS_NETDEV_DISABLE);
							lxnm_status_push(ifstat, msg);

						}
						break;
					case 1:
						if (((EthernetInfo *)ifstat->info)->plugged!=(atoi(value) ? TRUE : FALSE)) {
							((EthernetInfo *)ifstat->info)->plugged = ((EthernetInfo *)ifstat->info)->plugged ? FALSE : TRUE;
							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, ((EthernetInfo *)ifstat->info)->plugged ? LXNM_STATUS_NETDEV_PLUGGED : LXNM_STATUS_NETDEV_UNPLUG);
							lxnm_status_push(ifstat, msg);

							/* autofix */
							if (lxnm->autofix) {
								lxnm_handler_autofix(ifstat);
							}
						}
						break;
					case 2:
						if (((EthernetInfo *)ifstat->info)->connected!=(atoi(value) ? TRUE : FALSE)) {
							((EthernetInfo *)ifstat->info)->connected = ((EthernetInfo *)ifstat->info)->connected ? FALSE : TRUE;

							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, ((EthernetInfo *)ifstat->info)->connected ? LXNM_STATUS_NETDEV_CONNECTED : LXNM_STATUS_NETDEV_DISCONNECT);
							lxnm_status_push(ifstat, msg);
						}
						break;
					case 3:
						((EthernetInfo *)ifstat->info)->mac = g_strdup(value);
						break;
					case 4:
						((EthernetInfo *)ifstat->info)->ipaddr = g_strdup(value);
						break;
					case 5:
						((EthernetInfo *)ifstat->info)->dest = g_strdup(value);
						break;
					case 6:
						((EthernetInfo *)ifstat->info)->bcast = g_strdup(value);
						break;
					case 7:
						((EthernetInfo *)ifstat->info)->mask = g_strdup(value);
						break;
					case 8:
						((EthernetInfo *)ifstat->info)->recv_bytes = atoi(value);
						break;
					case 9:
						if (((EthernetInfo *)ifstat->info)->recv_packets!=atoi(value))
							status = TRUE;
						else
							status = FALSE;

						((EthernetInfo *)ifstat->info)->recv_packets = atoi(value);
						break;
					case 10:
						((EthernetInfo *)ifstat->info)->trans_bytes = atoi(value);
						break;
					case 11:
						if (((EthernetInfo *)ifstat->info)->trans_packets!=atoi(value)) {
							((EthernetInfo *)ifstat->info)->rtstat = TRUE;
							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, status ? LXNM_STATUS_NETDEV_BOTHRS : LXNM_STATUS_NETDEV_SENDDATA);
							lxnm_status_push(ifstat, msg);
						} else if (status) {
							((EthernetInfo *)ifstat->info)->rtstat = TRUE;
							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, LXNM_STATUS_NETDEV_RECVDATA);
							lxnm_status_push(ifstat, msg);
						} else if (((EthernetInfo *)ifstat->info)->rtstat) {
							((EthernetInfo *)ifstat->info)->rtstat = FALSE;
							sprintf(msg, "+STAT %s %d\n", ifstat->ifname, LXNM_STATUS_NETDEV_NORT);
							lxnm_status_push(ifstat, msg);
						}

						((EthernetInfo *)ifstat->info)->trans_packets = atoi(value);

						break;
					case 12: /* Wireless */
						((WirelessInfo *)ifstat->info)->essid = g_strdup(value);
						break;
					case 13:
						((WirelessInfo *)ifstat->info)->bssid = g_strdup(value);
						break;
					case 14:
						((WirelessInfo *)ifstat->info)->quality = atoi(value);
						break;
				}

				++i;
				value = strtok(NULL, "\t");
			}
		}
		g_free(temp);

		/* wait child process */
		close(pfd[0]);
		waitpid((pid_t)pid, NULL, 0);

	}
}

void lxnm_status_compare_interface_list_from_external_program(InterfaceStatus *ifstat, const gchar *filename)
{
	int len;
	int pfd[2];
	int status;
	pid_t pid;
	char buffer[1024] = { 0 };

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	/* fork to execute external program or scripts */
	pid = fork();
	if (pid<0) {
		return;
	} else if (pid==0) {
		/* child process */
		dup2(pfd[1], STDOUT_FILENO);
		close(STDIN_FILENO);
		execlp(filename, filename, NULL);
		exit(0);
	} else { 
		GString *msg;
		LXNMInterface *interface;
		GList *list;
		GList *device_list = NULL;
		GList *device_list_tmp = NULL;
		gboolean found;
		gboolean updated = FALSE;
		gchar *temp_new = NULL;
		gchar *temp = NULL;
		gchar *value;
		gchar *c;

		/* parent process */
		close(pfd[1]);

		while((len=read(pfd[0], buffer, 1023))>0) {
			buffer[len] = '\0';

			temp_new = temp ? g_strconcat(temp, buffer, NULL) : g_strdup(buffer);
			g_free(temp);
			temp = temp_new;

			value = strtok(temp, " ");
			while(value!=NULL) {
				/* ignore */
				if (*value=='+') {
					value = strtok(NULL, " ");
					continue;
				}

				for (c=value;*c;c++) {
					if (*c=='\n')
						*c = '\0';
				}

				/* finding device*/
				found = FALSE;
				for (list=lxnm->interfaces;list;list=g_list_next(list)) {
					interface = (LXNMInterface *)list->data;
					if (strcmp(interface->ifname, value)==0) {
						interface->ref++;
						found = TRUE;
						break;
					}
				}

				/* this interface is new */
				if (!found) {
					updated = TRUE;
					interface = (LXNMInterface *)g_new0(LXNMInterface, 1);
					interface->ref = 2;
					interface->ifname = g_strdup(value);
					lxnm->interfaces = g_list_append(lxnm->interfaces, interface);
					/* check ppp interface */
				}

				value = strtok(NULL, " ");
			}
		}

		close(pfd[0]);
		waitpid((pid_t)pid, &status, 0);

		/* remove device which is not detected */
		for (list=lxnm->interfaces;list;list=device_list_tmp) {
			device_list_tmp = g_list_next(list);
			interface = (LXNMInterface *)list->data;
			if (interface->ref>1) {
				interface->ref--;
			} else {
				updated = TRUE;
				/* unregister status */
				lxnm_status_unregister_from_ifname(interface->ifname);
				lxnm->interfaces = g_list_delete_link(lxnm->interfaces, list);
				g_free(interface->ifname);
				g_free(interface);
			}
		}

		/* notify clients to update device list */
		if (updated) {
			msg = g_string_new("+STAT " LXNM_INTERFACE);
			g_string_append_printf(msg, " %d %d", LXNM_STATUS_DEVICE_LIST, LXNM_DEVICE_TYPE_CONNECTION);

			/* print all of devices */
			for (list=lxnm->interfaces;list;list=g_list_next(list)) {
				interface = (LXNMInterface *)list->data;
				g_string_append_printf(msg, " %s", interface->ifname);

			}

			g_string_append_c(msg, '\n');

			/* send */
			lxnm_status_push(ifstat, msg->str);
			g_string_free(msg, TRUE);
		}
	}
}


gint lxnm_status_info_update(InterfaceStatus *ifstat)
{
	switch(ifstat->type) {
		case LXNM_CONNECTION_TYPE_PRIDEVICE:
			/* connections */
			switch (lxnm->setting->iflist->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_status_compare_interface_list_from_external_program(ifstat, lxnm->setting->iflist->value);
					break;
			}

			/* devices */
			switch (lxnm->setting->devlist->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_status_compare_device_list_from_external_program(ifstat, lxnm->setting->devlist->value);
					break;
			}
			break;
		case LXNM_CONNECTION_TYPE_ETHERNET:
			switch (lxnm->setting->eth_info->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_status_info_update_from_external_program(ifstat);
					break;
			}
			break;
		case LXNM_CONNECTION_TYPE_PPP:
			switch (lxnm->setting->eth_info->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_status_info_update_from_external_program(ifstat);
					break;
			}
			break;
		case LXNM_CONNECTION_TYPE_WIRELESS:
			switch (lxnm->setting->wifi_info->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_status_info_update_from_external_program(ifstat);
					break;
			}
			break;
	}
}

gboolean lxnm_status_update()
{
	InterfaceStatus *ifstat;
	GList *list;
	GList *next_list;
	gchar msg[256];

	/* check all of interfaces */
	for (list=lxnm->ifstatus;list;list=next_list) {
		next_list = g_list_next(list);
		ifstat = (InterfaceStatus *)list->data;

		/* update device status */
		lxnm_status_info_update(ifstat);

		/* preparing to notify all client who has listened to the device */
		switch(ifstat->type) {
			case LXNM_CONNECTION_TYPE_ETHERNET:
			case LXNM_CONNECTION_TYPE_PPP:
				sprintf(msg, "+STAT %s %d %d\t%s\t%s\t%s\t%s\t%s\t%ld\t%ld\t%ld\t%ld\n",
						ifstat->ifname,
						LXNM_STATUS_NETDEV_INFO,
						ifstat->type,
						((EthernetInfo *)ifstat->info)->mac,
						((EthernetInfo *)ifstat->info)->ipaddr,
						((EthernetInfo *)ifstat->info)->dest,
						((EthernetInfo *)ifstat->info)->bcast,
						((EthernetInfo *)ifstat->info)->mask,
						((EthernetInfo *)ifstat->info)->recv_bytes,
						((EthernetInfo *)ifstat->info)->recv_packets,
						((EthernetInfo *)ifstat->info)->trans_bytes,
						((EthernetInfo *)ifstat->info)->trans_packets);
				break;
			case LXNM_CONNECTION_TYPE_WIRELESS:
				sprintf(msg, "+STAT %s %d %d\t%s\t%s\t%s\t%s\t%s\t%ld\t%ld\t%ld\t%ld\t%s\t%s\t%d\n",
						ifstat->ifname,
						LXNM_STATUS_NETDEV_INFO,
						ifstat->type,
						((EthernetInfo *)ifstat->info)->mac,
						((EthernetInfo *)ifstat->info)->ipaddr,
						((EthernetInfo *)ifstat->info)->dest,
						((EthernetInfo *)ifstat->info)->bcast,
						((EthernetInfo *)ifstat->info)->mask,
						((EthernetInfo *)ifstat->info)->recv_bytes,
						((EthernetInfo *)ifstat->info)->recv_packets,
						((EthernetInfo *)ifstat->info)->trans_bytes,
						((EthernetInfo *)ifstat->info)->trans_packets,
						((WirelessInfo *)ifstat->info)->essid,
						((WirelessInfo *)ifstat->info)->bssid,
						((WirelessInfo *)ifstat->info)->quality);
				break;
		}

		if (ifstat->type!=LXNM_CONNECTION_TYPE_PRIDEVICE)
			lxnm_status_push(ifstat, msg);
	}

	return TRUE;
}
