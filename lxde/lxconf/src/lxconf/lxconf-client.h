/*
 *      lxconf-client.h
 *
 *      Copyright 2009 Daniel Kesler <kesler.daniel@gmail.com>
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

#ifndef LXCONF_CLIENT_H
#define LXCONF_CLIENT_H

#include <lxconf/lxconf.h>

gboolean lxconf_client_init       (LXConfNotifyFunc, LXConfConnectedFunc);
void     lxconf_client_cleanup    ();

void     lxconf_client_add_channel_watch (const gchar* channel);
void     lxconf_client_add_group_watch   (const gchar* group);

gboolean lxconf_client_send_message (const gchar* channel, const gchar* message);

void     lxconf_client_postpone     ();
void     lxconf_client_flush        ();

gboolean lxconf_client_set_double   (const gchar* group, const gchar* key, gdouble value, GError **err);
gboolean lxconf_client_set_int      (const gchar* group, const gchar* key, gint value, GError **err);
gboolean lxconf_client_set_bool     (const gchar* group, const gchar* key, gboolean value, GError **err);
gboolean lxconf_client_set_string   (const gchar* group, const gchar* key, gchar* value, GError **err);

gdouble  lxconf_client_get_double   (const gchar* group, const gchar* key, GError **err);
gint     lxconf_client_get_int      (const gchar* group, const gchar* key, GError **err);
gboolean lxconf_client_get_bool     (const gchar* group, const gchar* key, GError **err);
gchar*   lxconf_client_get_string   (const gchar* group, const gchar* key, GError **err);

void     lxconf_client_remove_key   (const gchar* group, const gchar* key, GError **err);
void     lxconf_client_remove_group (const gchar* group, GError **err);

#endif /* LXCONF_CLIENT_H */
