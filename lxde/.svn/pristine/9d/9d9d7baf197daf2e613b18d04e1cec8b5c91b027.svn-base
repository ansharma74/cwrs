/*
 *      Copyright 2008 - 2009 Fred Chien <cfsghost@gmail.com>
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
#include <fcntl.h>
#include <sys/wait.h>
#include "lxnm.h"
#include "thread.h"
#include "status.h"
#include "handler.h"
#include "info.h"
#include "wireless.h"

extern LxND *lxnm;

static const char *protocol_name[] = {
	"NONE",
	"WEP",
	"WPA",
};

static const char *cypher_name[] = {
	"NONE",
	"WEP40",
	"TKIP",
	"WRAP",
	"CCMP",
	"WEP104",
};

static const char *key_mgmt_name[] = {
	"NONE",
	"IEEE8021X",
	"WPA-PSK",
};

LXNMHandler *lxnm_handler_new(const gchar *strings)
{
	LXNMHandler *handler;

	handler = g_new0(LXNMHandler, 1);

	if (!strings) {
		handler->method = LXNM_HANDLER_METHOD_INTERNAL;
		handler->value = NULL;
	} else if (strncmp(strings, "Execute:", 8)==0) {
		handler->method = LXNM_HANDLER_METHOD_EXECUTE;
		handler->value = g_strdup(strings+8);
	} else if (strncmp(strings, "Internal", 8)==0) {
		handler->method = LXNM_HANDLER_METHOD_INTERNAL;
		handler->value = NULL;
	} else
		return NULL;

	return handler;
}

static int lxnm_handler_execute(const gchar *filename, GIOChannel *gio, LXNMPID cmd_id, gboolean response)
{
	int len;
	int pfd[2];
	int status;
	pid_t pid;
	char cmdid[8];
	char buffer[1024] = { 0 };

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	/* initalizing environment variable */
	sprintf(cmdid, "%u", cmd_id);
	setenv("LXNM_CMDID", cmdid, 1);

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
		/* parent process */
		close(pfd[1]);

		if (response) {
			while((len=read(pfd[0], buffer, 1023))>0) {
				buffer[len] = '\0';
				printf("%s\n", buffer);
				lxnm_send_message(gio, buffer);
			}
		}

		close(pfd[0]);
		waitpid((pid_t)pid, &status, 0);
	}
}

static int lxnm_handler_execute_with_boardcast(InterfaceStatus *ifstat, const gchar *filename, gboolean response)
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
		/* parent process */
		close(pfd[1]);

		if (response) {
			while((len=read(pfd[0], buffer, 1023))>0) {
				buffer[len] = '\0';
				lxnm_status_push(ifstat, buffer);
			}
		}

		close(pfd[0]);
		waitpid((pid_t)pid, &status, 0);
	}
}

int lxnm_handler_version(LxThread *lxthread)
{
	LXNMPID id;
	gchar *msg;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_VERSION);
	msg = g_strdup_printf("+%u " LXNM_PROTOCOL "\n", id);
	lxnm_send_message(lxthread->client->gio, msg);
	lxnm_pid_unregister(lxthread->client->gio, id);
	g_free(msg);
	return 0;
}

int lxnm_handler_autofix(InterfaceStatus *ifstat)
{
	setenv("LXNM_IFNAME", ifstat->ifname, 1);
	switch(ifstat->type) {
		case LXNM_CONNECTION_TYPE_ETHERNET:
		case LXNM_CONNECTION_TYPE_PPP:
			switch (lxnm->setting->eth_autofix->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_handler_execute_with_boardcast(ifstat, lxnm->setting->eth_autofix->value, TRUE);
					break;
			}
			break;
		case LXNM_CONNECTION_TYPE_WIRELESS:
			switch (lxnm->setting->wifi_autofix->method) {
				case LXNM_HANDLER_METHOD_INTERNAL:
					/* FIXME: support this feature for each operating system */
					break;
				case LXNM_HANDLER_METHOD_EXECUTE:
					lxnm_handler_execute_with_boardcast(ifstat, lxnm->setting->wifi_autofix->value, TRUE);
					break;
			}
			break;
	}

	return 0;
}

int lxnm_handler_device_list(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	DeviceType option;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_DEVICE_LIST);
	if (p = strtok(NULL, " "))
		option = atoi(p);
	else
		option = LXNM_DEVICE_TYPE_CONNECTION;

	switch (lxnm->setting->iflist->method) {
		case LXNM_HANDLER_METHOD_INTERNAL:
			/* FIXME: support this feature for each operating system */
			break;
		case LXNM_HANDLER_METHOD_EXECUTE:
			if (option==LXNM_DEVICE_TYPE_CONNECTION)
				lxnm_handler_execute(lxnm->setting->iflist->value, lxthread->client->gio, id, TRUE);
			else if (option==LXNM_DEVICE_TYPE_MODEM)
				lxnm_handler_execute(lxnm->setting->devlist->value, lxthread->client->gio, id, TRUE);
			break;
	}
	lxnm_pid_unregister(lxthread->client->gio, id);

	return 0;
}

int lxnm_handler_modem_info(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	gchar *devname;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_DEVICE_LIST);
	/* devname */
	devname = strtok(NULL, " ");
	/* flags */
	p = strtok(NULL, " ");

	switch (lxnm->setting->modem_info->method) {
		case LXNM_HANDLER_METHOD_INTERNAL:
			/* FIXME: support this feature for each operating system */
			break;
		case LXNM_HANDLER_METHOD_EXECUTE:
			setenv("LXNM_DEVNAME", devname, 1);
			setenv("LXNM_FLAGS", p, 1);
			lxnm_handler_execute(lxnm->setting->modem_info->value, lxthread->client->gio, id, TRUE);
			break;
	}
	lxnm_pid_unregister(lxthread->client->gio, id);

	return 0;
}

int lxnm_handler_modem_detect(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	gchar *devname;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_DEVICE_LIST);
	/* devname */
	devname = strtok(NULL, " ");

	switch (lxnm->setting->modem_detect->method) {
		case LXNM_HANDLER_METHOD_INTERNAL:
			lxnm_modem_handler_detect(devname, lxthread->client->gio, id);
			break;
		case LXNM_HANDLER_METHOD_EXECUTE:
			setenv("LXNM_DEVNAME", devname, 1);
			setenv("LXNM_FLAGS", p, 1);
			lxnm_handler_execute(lxnm->setting->modem_detect->value, lxthread->client->gio, id, TRUE);
			break;
	}
	lxnm_pid_unregister(lxthread->client->gio, id);

	return 0;
}

int lxnm_handler_ppp_info(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	LXNMPPPInfoType opttype;

	printf("!!!\n");
	id = lxnm_pid_register(lxthread->client->gio, LXNM_PPP_INFORMATION);
	/* type */
	p = strtok(NULL, " ");
	opttype = atoi(p);
	/* devname or interface */
	p = strtok(NULL, " ");
	printf("%s\n", p);

	switch (lxnm->setting->ppp_info->method) {
		case LXNM_HANDLER_METHOD_INTERNAL:
			/* FIXME: support this feature for each operating system */
			break;
		case LXNM_HANDLER_METHOD_EXECUTE:
			if (opttype==LXNM_PPP_INFORMATION_TYPE_DEVICE) {
				setenv("LXNM_DEVNAME", p, 1);
			} else {
				printf("***INTERFACE\n");
				setenv("LXNM_IFNAME", p, 1);
			}

			lxnm_handler_execute(lxnm->setting->ppp_info->value, lxthread->client->gio, id, TRUE);
			break;
	}
	lxnm_pid_unregister(lxthread->client->gio, id);

	return 0;
}

int lxnm_handler_device_info(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	InterfaceStatus *ifstat;
	ConnectionType conntype;
	DeviceType devtype;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_DEVICE_INFORMATION);
	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		/* execute external program or own way to get information */
		conntype = lxnm_status_get_connection_type(p);

		/* whether the interface has been registered in status list */
		if ((ifstat=lxnm_status_get_ifstat(p))) {
			gchar msg[256];
			gint status = 0;

			/* reading interface information from status list */
			status += ((EthernetInfo *)ifstat->info)->enable ? LXNM_DEVICE_INFO_STATUS_NETDEV_ENABLE : 0; 
			status += ((EthernetInfo *)ifstat->info)->plugged ? LXNM_DEVICE_INFO_STATUS_NETDEV_PLUG : 0; 
			status += ((EthernetInfo *)ifstat->info)->connected ? LXNM_DEVICE_INFO_STATUS_NETDEV_CONNECT : 0; 

			switch(conntype) {
				case LXNM_CONNECTION_TYPE_ETHERNET:
				case LXNM_CONNECTION_TYPE_PPP:
					sprintf(msg, "+%u %d\t%d\t%s\t%s\t%s\t%s\t%s\t%ld\t%ld\t%ld\t%ld\n",
						id,
						status,
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
					/* FIXME: wireless information */
					sprintf(msg, "+%u %d\t%d\t%s\t%s\t%s\t%s\t%s\t%ld\t%ld\t%ld\t%ld\t%s\t%s\t%d\n",
						id,
						status,
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

			lxnm_send_message(lxthread->client->gio, msg);
		} else {
			switch(conntype) {
				case LXNM_CONNECTION_TYPE_ETHERNET:
				case LXNM_CONNECTION_TYPE_PPP:
					switch (lxnm->setting->eth_info->method) {
						case LXNM_HANDLER_METHOD_INTERNAL:
							/* FIXME: support this feature for each operating system */
							break;
						case LXNM_HANDLER_METHOD_EXECUTE:
							lxnm_handler_execute(lxnm->setting->eth_info->value, lxthread->client->gio, id, TRUE);
							break;
					}
					break;
				case LXNM_CONNECTION_TYPE_WIRELESS:
					switch (lxnm->setting->wifi_info->method) {
						case LXNM_HANDLER_METHOD_INTERNAL:
							/* FIXME: support this feature for each operating system */
							break;
						case LXNM_HANDLER_METHOD_EXECUTE:
							lxnm_handler_device_info_from_external_program(p, conntype, lxthread->client, id);
							break;
					}
					break;
			}
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_device_status(LxThread *lxthread)
{
	LXNMPID id;
	char *p;
	DeviceType devtype;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_DEVICE_STATUS);
	/* device type */
	p = strtok(NULL, " ");
	devtype = atoi(p);
	/* interface or device name */
	p = strtok(NULL, " ");

	if (devtype==LXNM_DEVICE_TYPE_CONNECTION) {
		if (lxnm_isifname(p)||strcmp(p, LXNM_INTERFACE)==0) {
			/* register interface */
			lxnm_status_register(p, lxnm_status_get_connection_type(p), lxthread->client, devtype);
		}
	} else if (devtype==LXNM_DEVICE_TYPE_MODEM) {
		if (lxnm_isdevice(p)) {
			/* register device */
			//lxnm_status_register(p, lxnm_status_get_connection_type(p), lxthread->client, devtype);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_ethernet_up(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_ETHERNET_UP);
	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_up->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_up->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_ethernet_down(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_ETHERNET_DOWN);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_down->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_down->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_ethernet_repair(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_ETHERNET_REPAIR);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->eth_repair->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_wireless_up(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_WIRELESS_UP);
	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_up->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_up->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_wireless_down(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_WIRELESS_DOWN);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->eth_down->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_down->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_wireless_repair(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_WIRELESS_REPAIR);
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->wifi_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			lxnm_handler_execute(lxnm->setting->wifi_repair->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_wireless_connect(LxThread *lxthread)
{
	LXNMPID id;
	char *p;

	id = lxnm_pid_register(lxthread->client->gio, LXNM_WIRELESS_CONNECT);
	/* <interface> <essid> <apaddr> <key> <protocol> <key_mgmt> <grpup> <pairwise> */
	/* interface name */
	p = strtok(NULL, "");

	if (lxnm_isifname(p)) {
		if (lxnm->setting->wifi_repair->method==LXNM_HANDLER_METHOD_EXECUTE) {
			setenv("LXNM_IFNAME", p, 1);
			/* ESSID */
			p = strtok(NULL, " ");
			setenv("LXNM_WIFI_ESSID", hex2asc(p), 1);

			if (p = strtok(NULL, " ")) {
				/* AP Addr */
				setenv("LXNM_WIFI_APADDR", p, 1);
				/* key */
				p = strtok(NULL, " ");
				setenv("LXNM_WIFI_KEY", hex2asc(p), 1);

				if (p = strtok(NULL, " ")) {
					/* protocol */
					setenv("LXNM_WIFI_PROTO", protocol_name[atoi(p)], 1);
					/* key_mgmt */
					p = strtok(NULL, " ");
					setenv("LXNM_WIFI_KEYMGMT", key_mgmt_name[atoi(p)], 1);
					/* group */
					p = strtok(NULL, " ");
					setenv("LXNM_WIFI_GROUP", cypher_name[atoi(p)], 1);
					/* pairwise */
					p = strtok(NULL, " ");
					setenv("LXNM_WIFI_PAIRWISE", cypher_name[atoi(p)], 1);
				}
			}
			lxnm_handler_execute(lxnm->setting->wifi_connect->value, lxthread->client->gio, id, FALSE);
		}
	}

	lxnm_pid_unregister(lxthread->client->gio, id);
	return 0;
}

int lxnm_handler_wireless_scan(LxThread *lxthread)
{
	LXNMPID id;
	int iwsockfd;
	char *p;
	APLIST *aplist;
	APLIST *ptr;
	ap_info *apinfo;

	/* interface name */
	p = strtok(NULL, " ");

	if (lxnm_isifname(p)) {
		id = lxnm_pid_register(lxthread->client->gio, LXNM_WIRELESS_SCAN);

		switch (lxnm->setting->wifi_scan->method) {
			case LXNM_HANDLER_METHOD_INTERNAL:
#ifdef OS_Linux
				iwsockfd = iw_sockets_open();
				aplist = wireless_scanning(iwsockfd, p);
				if (aplist) {
					ptr = aplist;
					do {
						apinfo = ptr->info;
						ptr = ptr->next;
					} while (ptr);
				}
#endif
				break;
			case LXNM_HANDLER_METHOD_EXECUTE:
				setenv("LXNM_IFNAME", p, 1);

				lxnm_handler_execute(lxnm->setting->wifi_scan->value, lxthread->client->gio, id, TRUE);
				break;
		}
		lxnm_pid_unregister(lxthread->client->gio, id);
	}

	return 0;
}
