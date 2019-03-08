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
#include "device.h"

extern LxND *lxnm;

void lxnm_status_compare_device_list_from_external_program(InterfaceStatus *ifstat, const gchar *filename)
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
		LXNMDevice *device;
		GList *list;
		GList *device_list = NULL;
		GList *device_list_tmp = NULL;
		gboolean found;
		gboolean updated = FALSE;
		gchar *temp_new = NULL;
		gchar *temp = NULL;
		gchar *value;
		gchar *c;
		gint i;
		gint type;

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

				type = atoi(strtok(NULL, " "));
				/* finding device*/
				found = FALSE;
				for (list=lxnm->devices;list;list=g_list_next(list)) {
					device = (LXNMDevice *)list->data;
					if (device->type==type) {
						if (strcmp(device->devname, value)==0) {
							device->ref++;
							found = TRUE;
							break;
						}
					}
				}

				/* this device is new */
				if (!found) {
					updated = TRUE;
					device = (LXNMDevice *)g_new0(LXNMDevice, 1);
					device->ref = 2;
					device->devname = g_strdup(value);
					device->type = type;
					lxnm->devices = g_list_append(lxnm->devices, device);
				}

				value = strtok(NULL, " ");
			}
		}

		close(pfd[0]);
		waitpid((pid_t)pid, &status, 0);

		/* remove device which is not detected */
		for (list=lxnm->devices;list;list=device_list_tmp) {
			device_list_tmp = g_list_next(list);
			device = (LXNMDevice *)list->data;
			if (device->ref>1) {
				device->ref--;
			} else {
				updated = TRUE;
				/* unregister status */
				lxnm_status_unregister_from_ifname(device->devname);
				lxnm->devices = g_list_delete_link(lxnm->devices, list);
				g_free(device->devname);
				g_free(device);
			}
		}

		/* notify clients to update device list */
		if (updated) {
			for (i=0;i<LXNM_DEVICE_TYPE_COUNT;i++) {
				if (i==LXNM_DEVICE_TYPE_CONNECTION)
					continue;

				/* print all of devices */
				found = FALSE;
				for (list=lxnm->devices;list;list=g_list_next(list)) {
					device = (LXNMDevice *)list->data;
					if (device->type==i) {
						if (!found) {
							msg = g_string_new("+STAT " LXNM_INTERFACE);
							g_string_append_printf(msg, " %d %d", LXNM_STATUS_DEVICE_LIST, i);
							found = TRUE;
						}

						g_string_append_printf(msg, " %s", device->devname);
					}
				}

				/* no device was found */
				if (!found)
					continue;

				g_string_append_c(msg, '\n');

				/* send */
				lxnm_status_push(ifstat, msg->str);
				g_string_free(msg, TRUE);
			}
		}
	}
}
