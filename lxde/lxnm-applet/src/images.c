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
#include "images.h"
#include "lxnm-applet.h"

void lxnm_applet_images_init(LXNMApplet *applet)
{
	/* load icons */
	applet->image[LXNM_APPLET_IMAGE_MODEM] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-modem.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_ADD] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-add.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_REPAIR] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-repair.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_DISABLE] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-disable.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_CONNECTED] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-connected.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_DISCONNECT] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-disconnect.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_PROBLEM] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-problem.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_RECVDATA] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-recvdata.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_TRANSDATA] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-senddata.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_RTBOTH] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-bothrs.png", NULL);
	applet->image[LXNM_APPLET_IMAGE_LOCK] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-lock.png", NULL);

	/* local animations */
	applet->animation[LXNM_APPLET_ANIMATION_SCANNING] = gdk_pixbuf_animation_new_from_file(PACKAGE_DATA_DIR  "/lxnm-applet/ns-scanning.gif", NULL);
}
