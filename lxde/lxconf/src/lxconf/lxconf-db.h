/*
 *      lxconf-db.h
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

#ifndef LXCONF_DB_H
#define LXCONF_DB_H

#include <glib.h>

typedef gboolean (*LXCONF_DB_OPEN)();
typedef void     (*LXCONF_DB_RELOAD)();
typedef void     (*LXCONF_DB_CLOSE)();

typedef void (*LXCONF_DB_REMOVE_KEY)   (const gchar* group, const gchar* key, GError **err);
typedef void (*LXCONF_DB_REMOVE_GROUP) (const gchar* group, GError **err);

typedef void (*LXCONF_DB_WRITE_DOUBLE) (const gchar* group, const gchar* key, gdouble value, GError **err);
typedef void (*LXCONF_DB_WRITE_INT)    (const gchar* group, const gchar* key, gint value, GError **err);
typedef void (*LXCONF_DB_WRITE_BOOL)   (const gchar* group, const gchar* key, gboolean value, GError **err);
typedef void (*LXCONF_DB_WRITE_STRING) (const gchar* group, const gchar* key, gchar* value, GError **err);

typedef void (*LXCONF_DB_WRITE_STRING_LIST) (const gchar* group, const gchar* key, const gchar * const list[], gsize length, GError **err);
typedef void (*LXCONF_DB_WRITE_DOUBLE_LIST) (const gchar* group, const gchar* key, gdouble list[], gsize length, GError **err);
typedef void (*LXCONF_DB_WRITE_INT_LIST)    (const gchar* group, const gchar* key, gint list[], gsize length, GError **err);
typedef void (*LXCONF_DB_WRITE_BOOL_LIST)   (const gchar* group, const gchar* key, gboolean list[], gsize length, GError **err);

typedef gdouble  (*LXCONF_DB_READ_DOUBLE) (const gchar* group, const gchar* key, GError **err);
typedef gint     (*LXCONF_DB_READ_INT)    (const gchar* group, const gchar* key, GError **err);
typedef gboolean (*LXCONF_DB_READ_BOOL)   (const gchar* group, const gchar* key, GError **err);
typedef gchar*   (*LXCONF_DB_READ_STRING) (const gchar* group, const gchar* key, GError **err);

typedef gchar**   (*LXCONF_DB_READ_STRING_LIST) (const gchar* group, const gchar* key, gsize* length, GError **err);
typedef gdouble*  (*LXCONF_DB_READ_DOUBLE_LIST) (const gchar* group, const gchar* key, gsize* length, GError **err);
typedef gint*     (*LXCONF_DB_READ_INT_LIST)    (const gchar* group, const gchar* key, gsize* length, GError **err);
typedef gboolean* (*LXCONF_DB_READ_BOOL_LIST)   (const gchar* group, const gchar* key, gsize* length, GError **err);

typedef struct {
  LXCONF_DB_OPEN          open_db;
  LXCONF_DB_RELOAD        reload_db;
  LXCONF_DB_CLOSE         close_db;

  LXCONF_DB_REMOVE_KEY    remove_key;
  LXCONF_DB_REMOVE_GROUP  remove_group;

  LXCONF_DB_WRITE_DOUBLE  write_double;
  LXCONF_DB_WRITE_INT     write_int;
  LXCONF_DB_WRITE_BOOL    write_bool;
  LXCONF_DB_WRITE_STRING  write_string;

  LXCONF_DB_WRITE_STRING_LIST write_string_list;
  LXCONF_DB_WRITE_DOUBLE_LIST write_double_list;
  LXCONF_DB_WRITE_INT_LIST    write_int_list;
  LXCONF_DB_WRITE_BOOL_LIST   write_bool_list;

  LXCONF_DB_READ_DOUBLE  read_double;
  LXCONF_DB_READ_INT     read_int;
  LXCONF_DB_READ_BOOL    read_bool;
  LXCONF_DB_READ_STRING  read_string;

  LXCONF_DB_READ_STRING_LIST read_string_list;
  LXCONF_DB_READ_DOUBLE_LIST read_double_list;
  LXCONF_DB_READ_INT_LIST    read_int_list;
  LXCONF_DB_READ_BOOL_LIST   read_bool_list;
} LXConfDBEngine;

#endif /* LXCONF_DB_H */
