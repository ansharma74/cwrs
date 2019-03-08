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
#include <glib/gi18n.h>
#include "lxnm.h"
#include "timer.h"

extern LxND *lxnm;

gint lxnm_timer_add(void *func)
{
	LXNMTimerTask *task;

	task = (LXNMTimerTask *)g_new0(LXNMTimerTask, 1);
	task->id = ++lxnm->timercount;
	task->func = func;
	lxnm->timer_tasks = g_list_append(lxnm->timer_tasks, task);

	return task->id;
}

void lxnm_timer_remove(gint id)
{
	LXNMTimerTask *task;
	GList *list;

	/* remove functions of task list */
	for (list=lxnm->timer_tasks;list;list=g_list_next(list)) {
		task = (LXNMTimerTask *)list->data;

		if (task->id==id) {
			lxnm->timer_tasks = g_list_delete_link(lxnm->timer_tasks, list);
			g_free(task);
			break;
		}
	}
}

static gboolean lxnm_timer_schedule()
{
	LXNMTimerTask *task;
	GList *list;

	/* run functions of task list */
	for (list=lxnm->timer_tasks;list;list=g_list_next(list)) {
		task = (LXNMTimerTask *)list->data;

		task->func();
			//lxnm->timer_tasks = g_list_remove(lxnm->timer_tasks, task);
			//g_free(task);
	}

	return TRUE;	
}

void lxnm_timer_init()
{
	if (!lxnm->timer_id)
		lxnm->timer_id = g_timeout_add(LXNM_TIMER_DELAY, (GSourceFunc)lxnm_timer_schedule, NULL);
}
