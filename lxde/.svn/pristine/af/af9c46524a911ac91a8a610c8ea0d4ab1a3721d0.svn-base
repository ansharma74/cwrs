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

#include <glib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "lxnm.h"
#include "thread.h"
#include "status.h"
#include "handler.h"
#include "info.h"

extern LxND *lxnm;

gint lxnm_handler_device_info_from_external_program(const gchar *ifname, ConnectionType devtype, LXNMClient *client, LXNMPID cmd_id)
{
	int i = 0;
	int len;
	int pfd[2];
	char cmdid[8];
	pid_t pid;

	/* create pipe */
	if (pipe(pfd)<0)
		return -1;

	/* initalizing environment variable */
	sprintf(cmdid, "%u", cmd_id);
	setenv("LXNM_CMDID", cmdid, 1);
	setenv("LXNM_IFNAME", ifname, 1);

	/* fork to execute external program or scripts */
	pid = fork();
	if (pid<0) {
		return -2;
	} else if (pid==0) {
		/* child process */
		dup2(pfd[1], STDOUT_FILENO);
		close(STDIN_FILENO);

		switch(devtype) {
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
		//gchar msg[128];
		gchar buffer[1024];
		gchar *temp_new = NULL;
		gchar *temp = NULL;
		gchar *value;
		gchar *c;
		gint j = 0;
		gint status = 0;
		GString *msg;

		/* parent process */
		close(pfd[1]);

		while((len=read(pfd[0], buffer, 1023))>0) {
			buffer[len] = '\0';

			temp_new = temp ? g_strconcat(temp, buffer, NULL) : g_strdup(buffer);
			g_free(temp);
			temp = temp_new;

			value = strtok(temp, "\t");
			while(value!=NULL) {
				switch(i) {
					case 0:
						/* ignore CMDID */
						for (c=value;*c!=' ';++c,++j);

						/* create a new block to store message */
						msg = g_string_new(NULL);
						g_string_append_len(msg, value, j);

						status += atoi(c+1) ? LXNM_DEVICE_INFO_STATUS_NETDEV_ENABLE : 0; 
						break;
					case 1:
						status += atoi(value) ? LXNM_DEVICE_INFO_STATUS_NETDEV_PLUG : 0; 
						break;
					case 2:
						status += atoi(value) ? LXNM_DEVICE_INFO_STATUS_NETDEV_CONNECT : 0; 
						g_string_append_printf(msg, " %d", status);
						g_string_append_printf(msg, "\t%d", devtype);
						break;
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12: /* Wireless */
					case 13:
					case 14:
						g_string_append_printf(msg, "\t%s", value);
						break;
				}

				++i;
				value = strtok(NULL, "\t");
			}
		}

		g_string_append_c(msg, '\n');
		//sprintf(msg, "%s\n", msg);
		g_free(temp);

		lxnm_send_message(client->gio, msg->str);
		g_string_free(msg, TRUE);

		/* wait child process */
		close(pfd[0]);
		waitpid((pid_t)pid, NULL, 0);
	}
}
