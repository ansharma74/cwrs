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

#include <glib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define LXNM_STATUS_MESSAGE			0
#define LXNM_STATUS_NETDEV_INFO			1
#define LXNM_STATUS_NETDEV_ENABLE	 	2
#define LXNM_STATUS_NETDEV_DISABLE	 	3
#define LXNM_STATUS_NETDEV_PLUGGED	 	4
#define LXNM_STATUS_NETDEV_UNPLUG	 	5
#define LXNM_STATUS_NETDEV_CONNECTING		6
#define LXNM_STATUS_NETDEV_CONNECTED 		7
#define LXNM_STATUS_NETDEV_DISCONNECTED 	8
#define LXNM_STATUS_NETDEV_PROBLEM		9
#define LXNM_STATUS_NETDEV_BOTHRS		10
#define LXNM_STATUS_NETDEV_SENDDATA		11
#define LXNM_STATUS_NETDEV_RECVDATA		12
#define LXNM_STATUS_NETDEV_WIRELESS_QUALITY	13

/* Command */
typedef enum {
	LXNM_VERSION,
	LXNM_DEVICE_LIST,
	LXNM_DEVICE_STATUS,
	LXNM_DEVICE_INFORMATION,
	LXNM_MODEM_INFORMATION,
	LXNM_PPP_INFORMATION,
	LXNM_ETHERNET_UP,
	LXNM_ETHERNET_DOWN,
	LXNM_ETHERNET_REPAIR,
	LXNM_WIRELESS_UP,
	LXNM_WIRELESS_DOWN,
	LXNM_WIRELESS_REPAIR,
	LXNM_WIRELESS_CONNECT,
	LXNM_WIRELESS_SCAN
} LXNMCommand;


typedef enum {
	LXNM_PPP_INFORMATION_TYPE_DEVICE,
	LXNM_PPP_INFORMATION_TYPE_INTERFACE
} LXNMPPPInfoType;

typedef enum {
	LXNM_DEVICE_TYPE_UNKNOWN,
	LXNM_DEVICE_TYPE_CONNECTION,
	LXNM_DEVICE_TYPE_MODEM
} DeviceType;

#define LXNM_SOCKET "/var/run/lxnm.socket"

typedef unsigned int LXNMPID;

typedef struct _Task Task;
struct _Task {
	LXNMPID id;
	gint command;
	gboolean result;
	void (*callback)(Task *task, gpointer data);
	void (*release)(Task *task, gpointer data);
};

static gchar helpmsg[] = {
	"Usage:\n"
	"  lxnetctl [interface] up\n"
	"           [interface] down\n"
	"           [interface] scan\n"
	"           [interface] info\n"
	"           [interface] stat\n"
	"           [interface] connect [essid] [key] [protocol] [keymgmt] [group] [pairwise] [-bssid <bssid>]\n"
	"           list [connection|modem] - list connections or devices (default:connection)\n"
	"           version \n"
};

static GList *list = NULL;

gchar *asc2hex(gchar *src)
{
	gchar *buf, *tmp;
	gchar c[3];

	buf = malloc(sizeof(gchar)+strlen(src)*2);
	tmp = buf;

	for (;*src!='\0';src++) {
		sprintf(c, "%X", *src);
		*tmp = c[0];
		*(tmp+1) = c[1];
		tmp += 2;
	}

	*tmp = '\0';
	return buf;
}

char *hex2asc(char *hexsrc)
{
	char *buf, *tmp;
	char c[3];

	buf = malloc(sizeof(char)+strlen(hexsrc)/2);
	tmp = buf;

	for (;*hexsrc;hexsrc+=2) {
		c[0] = *hexsrc;
		c[1] = *(hexsrc+1);
		c[2] = '\0';

		*tmp = strtol(c, NULL, 16);
		tmp++;
	}

	*tmp = '\0';
	return buf;
}

static void lxnetctl_release(Task *task, gpointer data)
{
	exit(0);
}

static void lxnetctl_protocol_version(Task *task, gpointer data)
{
	gchar *content = (gchar *)data;

	printf("LXNM protocol version: %s", content);
}

static void lxnetctl_device_list(Task *task, gpointer data)
{
	gchar *p;
	gchar *content = (gchar *)data;

	p = strtok(content, " ");
	if (!p)
		return;

	do {
		printf("%s\n", p);
	} while((p = strtok(NULL, " ")));
}

static void lxnetctl_device_info(Task *task, gpointer data)
{
	printf("%s\n", data);
}

static void lxnetctl_device_stat_release(Task *task, gpointer data)
{
	return;
}

static void lxnetctl_wireless_scan(Task *task, gpointer data)
{
	gchar *p;
	gchar *str;
	gchar *content = (gchar *)data;

	if (!task->result) {
		task->result = TRUE;
		/* show name colums */
		printf("%16s  %17s  Quality  Encryption  keymgmt  Group  Pairware\n", "ESSID", "BSSID");
	}

	/* ESSID */
	p = strtok(content, " ");
	str = hex2asc(p);
	printf("%16s  ", str);
	g_free(str);

	/* BSSID */
	p = strtok(NULL, " ");
	printf("%s  ", p);

	/* QUALITY */
	p = strtok(NULL, " ");
	printf("%7s   ", p);

	/* ENCRYPTION */
	p = strtok(NULL, " ");
	if (strcmp(p, "WPA")!=0) {
		printf("%9s\n", p);
	} else {
		printf("%9s  ", p);

		/* key management */
		p = strtok(NULL, " ");
		printf("%7s  ", p);

		/* Group */
		p = strtok(NULL, " ");
		printf("%5s  ", p);

		/* Pairwise */
		p = strtok(NULL, " ");
		printf("%9s\n", p);
	}
}

static void lxnetctl_command_parser(gchar *cmd)
{
	Task *task;
	LXNMPID pid;
	GList *data;
	gchar *p;

	if (*cmd!='+')
		return;

	*(cmd+strlen(cmd)-1) = '\0';
	p = strtok(cmd, " ");

	if (strcmp(p, "+OK")==0) {
		p = strtok(NULL, " ");

		/* create a task */
		task = g_new0(Task, 1);
		task->result = FALSE;
		task->command = atoi(p);

		p = strtok(NULL, " ");
		task->id = (LXNMPID)atoi(p);
		task->release = lxnetctl_release;

		switch(task->command) {
			case LXNM_VERSION:
				task->callback = lxnetctl_protocol_version;
				break;
			case LXNM_DEVICE_LIST:
				task->callback = lxnetctl_device_list;
				break;
			case LXNM_DEVICE_STATUS:
				task->callback = NULL;
				task->release = lxnetctl_device_stat_release;
				break;
			case LXNM_DEVICE_INFORMATION:
				task->callback = lxnetctl_device_info;
				break;
			case LXNM_ETHERNET_UP:
				break;
			case LXNM_ETHERNET_DOWN:
				break;
			case LXNM_WIRELESS_UP:
				break;
			case LXNM_WIRELESS_DOWN:
				break;
			case LXNM_WIRELESS_SCAN:
				task->callback = lxnetctl_wireless_scan;
				break;
		}

		/* add to task list */
		list = g_list_append(list, task);
	} else if (strcmp(p, "+STAT")==0) {
		p = strtok(NULL, " ");
		switch(atoi(p)) {
			case LXNM_STATUS_MESSAGE:
				break;
			case LXNM_STATUS_NETDEV_INFO:
				p = strtok(NULL, " ");
				printf("%s\n", p);
				break;
			case LXNM_STATUS_NETDEV_ENABLE:
				p = strtok(NULL, " ");
				printf("%s Enable\n", p);
				break;
			case LXNM_STATUS_NETDEV_DISABLE:
				p = strtok(NULL, " ");
				printf("%s Disable\n", p);
				break;
			case LXNM_STATUS_NETDEV_PLUGGED:
				p = strtok(NULL, " ");
				printf("%s Plugged\n", p);
				break;
			case LXNM_STATUS_NETDEV_UNPLUG:
				p = strtok(NULL, " ");
				printf("%s Unplug\n", p);
				break;
			case LXNM_STATUS_NETDEV_CONNECTING:
				p = strtok(NULL, " ");
				printf("%s Connecting\n", p);
				break;
			case LXNM_STATUS_NETDEV_CONNECTED:
				p = strtok(NULL, " ");
				printf("%s Connected\n", p);
				break;
			case LXNM_STATUS_NETDEV_DISCONNECTED:
				p = strtok(NULL, " ");
				printf("%s Disconnect\n", p);
				break;
			case LXNM_STATUS_NETDEV_PROBLEM:
				break;
			case LXNM_STATUS_NETDEV_BOTHRS:
				p = strtok(NULL, " ");
				printf("%s Send and Recv\n", p);
				break;
			case LXNM_STATUS_NETDEV_SENDDATA:
				p = strtok(NULL, " ");
				printf("%s Send\n", p);
				break;
			case LXNM_STATUS_NETDEV_RECVDATA:
				p = strtok(NULL, " ");
				printf("%s Recv\n", p);
				break;
			case LXNM_STATUS_NETDEV_WIRELESS_QUALITY:
				break;
		}
	} else if (list) {
		if (strcmp(p, "+DONE")==0) {
			p = strtok(NULL, " ");
			pid = (LXNMPID)atoi(p);

			/* find task */
			for (data=list;data;data=g_list_next(data)) {
				task = (Task *)data->data;
				if (task->id==pid) {
					task->release(pid, NULL);
					break;
				}
			}

			list = g_list_remove(list, task);
		} else {
			pid = (LXNMPID)atoi(p+1);

			/* find task */
			for (data=list;data;data=g_list_next(data)) {
				task = (Task *)data->data;
				if (task->id==pid) {
					/* skip to content area */
					for (;*p;p++);
					task->callback(task, p+1);
					break;
				}
			}

		}
	}
}

static gboolean
lxnetctl_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gchar *cmd;
	gsize len;

	if (condition & G_IO_HUP)
		exit(0);

	ret = g_io_channel_read_line(gio, &msg, &len, NULL, &err);
	if (ret == G_IO_STATUS_ERROR)
		g_error ("Error reading: %s\n", err->message);

	do {
		if (len > 0) {
			if (msg[0]!='\n') {
				//printf("%s", msg);
				cmd = g_strdup(msg);
				lxnetctl_command_parser(cmd);
			}
		}

		ret = g_io_channel_read_line(gio, &msg, &len, NULL, &err);
		if (ret == G_IO_STATUS_ERROR)
			g_error ("Error reading: %s\n", err->message);
	} while(ret!=G_IO_STATUS_EOF);

	g_free(msg);

	return TRUE;
}

int
main(gint argc, gchar** argv)
{
	GIOChannel *gio;
	gchar *command;
	gsize len;
	int i;
	int sockfd;
	int flags;
	struct sockaddr_un sa_un;

	if (argc>1) {
		if ((strncmp(argv[1], "version", 7)!=0)&&(strncmp(argv[1], "list", 4)!=0)&&(argc<3)) {
			printf("%s\n", helpmsg);
			return 0;
		}
	} else {
		printf("%s\n", helpmsg);
		return 0;
	}

	/* crate socket */
	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Cannot create socket!");
		return 1;
	}

	/* Initiate socket */
	bzero(&sa_un, sizeof(sa_un));

	/* setting UNIX socket */
	sa_un.sun_family = AF_UNIX;
	snprintf(sa_un.sun_path, sizeof(sa_un.sun_path), LXNM_SOCKET);

	if (connect(sockfd, (struct sockaddr *) &sa_un, sizeof (sa_un)) < 0) {
		printf("Cannot connect!");
		return 1;
	}

	flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	gio = g_io_channel_unix_new(sockfd);
	g_io_channel_set_flags(gio,(GIOFlags)(g_io_channel_get_flags(gio) | G_IO_FLAG_NONBLOCK), NULL); 
	g_io_channel_set_encoding(gio, NULL, NULL);
	g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxnetctl_read_channel, NULL);

	/* send command */
/* "7 ath0 1F WEP testest",*/
	if (strncmp(argv[1], "list", 4)==0) {
		if (argc==3) {
			if (strncmp(argv[2], "modem", 5)==0)
				command = g_strdup_printf("%d %d\n", LXNM_DEVICE_LIST, LXNM_DEVICE_TYPE_MODEM);
			else
				command = g_strdup_printf("%d\n", LXNM_DEVICE_LIST);
		} else
			command = g_strdup_printf("%d\n", LXNM_DEVICE_LIST);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[1], "version", 7)==0) {
		command = g_strdup_printf("%d\n", LXNM_VERSION);
		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "up", 2)==0) {
		command = g_strdup_printf("%d %s\n", LXNM_ETHERNET_UP, argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
		return 0;
	} else if (strncmp(argv[2], "down", 4)==0) {
		command = g_strdup_printf("%d %s\n", LXNM_ETHERNET_DOWN, argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
		return 0;
	} else if (strncmp(argv[2], "info", 4)==0) {
		command = g_strdup_printf("%d %s\n", LXNM_DEVICE_INFORMATION, argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "stat", 4)==0) {
		command = g_strdup_printf("%d %s\n", LXNM_DEVICE_STATUS, argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "scan", 4)==0) {
		command = g_strdup_printf("%d %s\n", LXNM_WIRELESS_SCAN, argv[1]);

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	} else if (strncmp(argv[2], "connect", 4)==0) {
		if (argc==4) {
			/* ESSID only */
			command = g_strdup_printf("%d %s %s\n", LXNM_WIRELESS_CONNECT, argv[1], argv[3]);
		} else if (argc==5) {
			/* with key */
			command = g_strdup_printf("%d %s %s (NULL) %s\n", LXNM_WIRELESS_CONNECT, argv[1], argv[3], argv[4]);
		} else if (argc>5) {
			if (strcmp(argv[argc-2], "-bssid")==0) {
				if (argc==6) {
					command = g_strdup_printf("%d %s %s %s\n", LXNM_WIRELESS_CONNECT, argv[1], argv[3], argv[argc-1]);
				} else if (argc==7) {
					command = g_strdup_printf("%d %s %s %s %s\n", LXNM_WIRELESS_CONNECT, argv[1], argv[3], argv[argc-1], argv[4]);
				} else if (argc==11) {
					command = g_strdup_printf("%d %s %s %s %s %s %s %s %s\n",
							LXNM_WIRELESS_CONNECT,
							              argv[1],
							              argv[3],
							              argv[10],
							              argv[4],
							              argv[5],
							              argv[6],
							              argv[7],
							              argv[8]);
				} else {
					printf("%s\n", helpmsg);
					return 0;
				}
			} else if (argc==8) {
				/* encryption */
				command = g_strdup_printf("%d %s %s (NULL) %s %s %s %s %s\n",
							LXNM_WIRELESS_CONNECT,
							              argv[1],
							              argv[2],
							              argv[3],
							              argv[4],
							              argv[5],
							              argv[6],
							              argv[7]);
			} else {
				printf("%s\n", helpmsg);
				return 0;
			}
		}

		if (g_io_channel_write_chars(gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
			g_error("Error writing!");

		g_free(command);
		g_io_channel_flush(gio, NULL);
	}

	/* waiting for response from LXNM Daemon */
	{
		GMainLoop *loop = g_main_loop_new(NULL, FALSE);
		g_main_loop_run(loop);
	}
	return 0;
}

