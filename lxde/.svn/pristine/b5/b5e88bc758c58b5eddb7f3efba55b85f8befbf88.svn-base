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
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "backend.h"

gchar *hex2asc(gchar *hexsrc)
{
	gchar *buf, *tmp;
	gchar c[3];

	buf = malloc(sizeof(gchar)*(1+strlen(hexsrc)/2));
	tmp = buf;

	for (;*hexsrc!='\0';hexsrc+=2) {
		c[0] = *hexsrc;
		c[1] = *(hexsrc+1);
		c[2] = '\0';

		*tmp = strtol(c, NULL, 16);
		tmp++;
	}

	*tmp = '\0';
	return buf;
}

gchar *asc2hex(gchar *src)
{
	gchar *buf, *tmp;
	gchar c[3];

	buf = malloc(sizeof(gchar)*(1+strlen(src)*2));
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

static LXNMWatch *lxnm_backend_watch_find(LXNMBackend *lxnm, gint command, gint event)
{
	GList *data;
	LXNMWatch *watch;

	for (data=lxnm->watch;data;data=g_list_next(data)) {
		watch = (LXNMWatch *)data->data;

		if (watch->command==command&&watch->event&event) {
			return watch;

		}
	}

	return NULL;
}

void lxnm_backend_add_watch(LXNMBackend *lxnm, gint command, gint event, gpointer func, gpointer data, gint mode)
{
	LXNMWatch *watch;

	watch = (LXNMWatch *)g_new0(LXNMWatch, 1);
	watch->mode = mode;
	watch->command = command;
	watch->event = event;
	watch->func = func;
	watch->data = data;

	/* add to list */
	lxnm->watch = g_list_append(lxnm->watch, watch);
}

static void lxnm_backend_command_parser(LXNMBackend *lxnm, gchar *cmd)
{
	Task *task;
	LXNMPID pid;
	GList *data;
	gchar *p;

	printf("%s\n", cmd);
	if (*cmd!='+')
		return;

	*(cmd+strlen(cmd)-1) = '\0';
	p = strtok(cmd, " ");

	if (strcmp(p, "+OK")==0) {
		LXNMWatch *watch;
		p = strtok(NULL, " ");

		/* create a task */
		task = g_new0(Task, 1);
		task->result = FALSE;
		task->command = atoi(p);

		p = strtok(NULL, " ");
		task->id = (LXNMPID)atoi(p);

		if (watch = lxnm_backend_watch_find(lxnm, task->command, LXNM_EVENT_IN)) {
			if (watch->mode==LXNM_WATCH_MODE_ONCE) {
				/* the watch is only running once, remove it from watch list  */
				lxnm->watch = g_list_remove(lxnm->watch, watch);
			}

			task->callback = watch->func;
			task->callback_data = watch->data;
			if (watch->event & LXNM_EVENT_RELEASE) {
				task->release = watch->func;
				task->release_data = watch->data;
			}

			if (watch->mode==LXNM_WATCH_MODE_ONCE) {
				g_free(watch);
			}
		}

		if (task->release==NULL) {
			if (watch = lxnm_backend_watch_find(lxnm, task->command, LXNM_EVENT_RELEASE)) {
				if (watch->mode==LXNM_WATCH_MODE_ONCE) {
					/* the watch is only running once, remove it from watch list  */
					lxnm->watch = g_list_remove(lxnm->watch, watch);
				}
				task->release = watch->func;
				task->release_data = watch->data;

				if (watch->mode==LXNM_WATCH_MODE_ONCE) {
					g_free(watch);
				}
			}
		}

		/* add to task list */
		lxnm->tasklist = g_list_append(lxnm->tasklist, task);
	} else if (strcmp(p, "+STAT")==0) {
		LXNMWatch *watch;
		void (*callback)(struct _LXNMBackend *lxnm, gpointer data, gpointer user_data);

		watch = lxnm_backend_watch_find(lxnm, LXNM_DEVICE_STATUS, LXNM_EVENT_IN);
		
		/* skip to content area */
		for (;*p;p++);
		watch->func(lxnm, LXNM_EVENT_IN, p+1, watch->data);
	} else if (lxnm->tasklist) {
		if (strcmp(p, "+DONE")==0) {
			p = strtok(NULL, " ");
			pid = (LXNMPID)atoi(p);

			/* find task to call release function */
			for (data=lxnm->tasklist;data;data=g_list_next(data)) {
				task = (Task *)data->data;
				if (task->id==pid) {
					if (task->release)
						task->release(lxnm, task, LXNM_EVENT_RELEASE, NULL, task->release_data);
					break;
				}
			}

			lxnm->tasklist = g_list_remove(lxnm->tasklist, task);
			g_free(task);
		} else {
			pid = (LXNMPID)atoi(p+1);

			/* find task */
			for (data=lxnm->tasklist;data;data=g_list_next(data)) {
				task = (Task *)data->data;
				if (task->id==pid) {
					if (task->callback) {
						/* skip to content area */
						for (;*p;p++);
						task->callback(lxnm, task, LXNM_EVENT_IN, p+1, task->callback_data);
					}
					break;
				}
			}

		}
	}
}

static gboolean
lxnm_backend_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
	GIOStatus ret;
	GError *err = NULL;
	gchar *msg;
	gchar *cmd;
	gsize len;

	if (condition & G_IO_HUP)
		exit(1);

	if (condition & G_IO_IN) {
		ret = g_io_channel_read_line(gio, &msg, &len, NULL, &err);
		if (ret == G_IO_STATUS_ERROR)
			g_error ("Error reading: %s\n", err->message);

		if (len > 0) {
			if (msg[0]!='\n') {
				cmd = g_strdup(msg);
				lxnm_backend_command_parser((LXNMBackend *)data, cmd);
			}
		}

		g_free(msg);
	}

	return TRUE;
}

void lxnm_backend_scan(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_WIRELESS_SCAN, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_ethernet_up(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_ETHERNET_UP, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_ethernet_down(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_ETHERNET_DOWN, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_ethernet_repair(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_ETHERNET_REPAIR, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_wireless_up(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_WIRELESS_UP, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_wireless_down(LXNMBackend *lxnm, gchar *ifname)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %s\n", LXNM_WIRELESS_DOWN, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_wireless_connect(LXNMBackend *lxnm, gchar *ifname,
				   gchar *essid, gchar *bssid,
				   gchar *key, gchar *proto,
				   gchar *keymgmt, gchar *group,
				   gchar *pairwise)
{
	gchar *command;
	gchar *essid_hex, *key_hex;
	gsize len;

	if (!essid)
		return;

	/* transform to hex */
	essid_hex = asc2hex(essid);

	if (key)
		key_hex = asc2hex(key);

	command = g_strdup_printf("%d %s %s %s %s %s %s %s %s\n", LXNM_WIRELESS_CONNECT, ifname,
								  essid, bssid, key, proto,
								  keymgmt, group, pairwise);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(essid_hex);
	g_free(key_hex);
	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_status(LXNMBackend *lxnm, gchar *ifname, DeviceType *devtype)
{
	gchar *command;
	gsize len;

	if (!ifname)
		return;

	command = g_strdup_printf("%d %d %s\n", LXNM_DEVICE_STATUS, devtype, ifname);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_ppp_info(LXNMBackend *lxnm, LXNMPPPInfoType opttype, gchar *name)
{
	gchar *command;
	gsize len;

	if (!name)
		return;

	command = g_strdup_printf("%d %d %s\n", LXNM_PPP_INFORMATION, opttype, name);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void lxnm_backend_list(LXNMBackend *lxnm, DeviceType type)
{
	gchar *command;
	gsize len;

	command = g_strdup_printf("%d %d\n", LXNM_DEVICE_LIST, type);
	if (g_io_channel_write_chars(lxnm->gio, command, -1, &len, NULL)==G_IO_STATUS_ERROR)
		g_error("Error writing!");

	g_free(command);
	g_io_channel_flush(lxnm->gio, NULL);
}

void *lxnm_backend_close(LXNMBackend *lxnm)
{
	close(lxnm->sockfd);
}

LXNMBackend *lxnm_backend_open()
{
	LXNMBackend *lxnm;
	gint flags;
	struct sockaddr_un sa_un;

	/* Initializing structure */
	lxnm = (LXNMBackend *)g_new0(LXNMBackend, 1);
	lxnm->watch = NULL;
	lxnm->tasklist = NULL;
	
	/* crate socket */
	lxnm->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (lxnm->sockfd < 0) {
		printf("Cannot create socket!\n");
		return NULL;
	}

	/* Initiate socket */
	bzero(&sa_un, sizeof(sa_un));

	/* setting UNIX socket */
	sa_un.sun_family = AF_UNIX;
	snprintf(sa_un.sun_path, sizeof(sa_un.sun_path), LXNM_SOCKET);

	if (connect(lxnm->sockfd, (struct sockaddr *) &sa_un, sizeof (sa_un)) < 0) {
		printf("Cannot connect!\n");
		return NULL;
	}

//	flags = fcntl(lxnm->sockfd, F_GETFL, 0);
  //      fcntl(lxnm->sockfd, F_SETFL, flags | O_NONBLOCK);

	lxnm->gio = g_io_channel_unix_new(lxnm->sockfd);
//	g_io_channel_set_flags(lxnm->gio,(GIOFlags)(g_io_channel_get_flags(lxnm->gio) | G_IO_FLAG_NONBLOCK), NULL); 
	g_io_channel_set_encoding(lxnm->gio, NULL, NULL);
	g_io_add_watch(lxnm->gio, G_IO_IN | G_IO_HUP, lxnm_backend_read_channel, lxnm);

	/* channel will automatically shutdown when the watch returns FALSE */
	g_io_channel_set_close_on_unref(lxnm->gio, TRUE);
	g_io_channel_unref(lxnm->gio);

	return lxnm;
}
