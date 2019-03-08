/*
 *      glib-db.c
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include <string.h>
#include <errno.h>

#include <lxconf/lxconf-db.h>
#include "glib-db.h"

LXConfDBEngine engine =
{
  (LXCONF_DB_OPEN)          open_db,
  (LXCONF_DB_RELOAD)        reaload_db,
  (LXCONF_DB_CLOSE)         close_db,

  (LXCONF_DB_REMOVE_KEY)    NULL,
  (LXCONF_DB_REMOVE_GROUP)  NULL,

  (LXCONF_DB_WRITE_DOUBLE)  write_double,
  (LXCONF_DB_WRITE_INT)     write_int,
  (LXCONF_DB_WRITE_BOOL)    write_bool,
  (LXCONF_DB_WRITE_STRING)  write_string,

  (LXCONF_DB_WRITE_STRING_LIST) NULL,
  (LXCONF_DB_WRITE_DOUBLE_LIST) NULL,
  (LXCONF_DB_WRITE_INT_LIST)    NULL,
  (LXCONF_DB_WRITE_BOOL_LIST)   NULL,

  (LXCONF_DB_READ_DOUBLE)  read_double,
  (LXCONF_DB_READ_INT)     read_int,
  (LXCONF_DB_READ_BOOL)    read_bool,
  (LXCONF_DB_READ_STRING)  read_string,

  (LXCONF_DB_READ_STRING_LIST) NULL,
  (LXCONF_DB_READ_DOUBLE_LIST) NULL,
  (LXCONF_DB_READ_INT_LIST)    NULL,
  (LXCONF_DB_READ_BOOL_LIST)   NULL
};

GKeyFile* keyfile = NULL;
gchar*    db_file = NULL;
gboolean  is_loaded = FALSE;

gboolean open_db()
{
  GError *error = NULL;
  gchar* buff;
  keyfile = g_key_file_new();

  db_file = g_build_filename( g_get_home_dir () , ".config/lxconf-db.ini", NULL );

  if( !g_file_test(db_file, G_FILE_TEST_EXISTS) )
  {
    buff = "# lxconf settings db\n\n[/]\n";
    g_file_set_contents( db_file, buff, -1, &error );
    if(error)
      g_error(error->message);

    if (g_chmod(db_file, 0644) < 0)
      g_error("Change [%s] permision failed: %s\n", db_file, g_strerror(errno));
  }

  error = NULL;
  is_loaded = g_key_file_load_from_file( keyfile, db_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);
  if( error )
  {
    g_error("%s\n", error->message);
  }
  return is_loaded;
}

void reaload_db()
{
  GError *error = NULL;
  g_key_file_free( keyfile );
  keyfile = g_key_file_new();
  is_loaded = g_key_file_load_from_file( keyfile, db_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);
  if( error )
  {
    g_error("%s\n", error->message);
  }
}

void close_db()
{
  g_key_file_free( keyfile );
  g_free( db_file );
}

void flush_data(GError **err)
{
  gsize len;
  gchar* str = NULL;

  if( (str = g_key_file_to_data( keyfile, &len, NULL )) )
  {
    if( g_file_set_contents( db_file, str, len, err ) )
    {

    }
    g_free(str);
  }
}

void write_double (const gchar* group, const gchar* key, gdouble value, GError **err)
{
  g_key_file_set_double(keyfile, group, key, value);
  flush_data(err);
}

void write_int (const gchar* group, const gchar* key, gint value, GError **err)
{
  g_key_file_set_integer(keyfile, group, key, value);
  flush_data(err);
}

void write_bool (const gchar* group, const gchar* key, gboolean value, GError **err)
{
  g_key_file_set_boolean(keyfile, group, key, value);
  flush_data(err);
}

void write_string (const gchar* group, const gchar* key, gchar* value, GError **err)
{
  g_key_file_set_string(keyfile, group, key, value);
  flush_data(err);
}

gdouble read_double (const gchar* group, const gchar* key, GError **err)
{
  return g_key_file_get_double(keyfile, group, key, err);
}

gint read_int (const gchar* group, const gchar* key, GError **err)
{
  reaload_db();
  return g_key_file_get_integer(keyfile, group, key, err);
}

gboolean read_bool (const gchar* group, const gchar* key, GError **err)
{
  return g_key_file_get_boolean(keyfile, group, key, err);
}

gchar* read_string (const gchar* group, const gchar* key, GError **err)
{
  reaload_db();
  return g_key_file_get_string(keyfile, group, key, err);
}
