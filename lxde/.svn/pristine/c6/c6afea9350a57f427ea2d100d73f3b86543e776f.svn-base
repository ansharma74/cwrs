/*
 *      sqlite3-db.c
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
#include <unistd.h>
#include <sqlite3.h>

#include <lxconf/lxconf.h>
#include <lxconf/lxconf-db.h>
#include "sqlite3-db.h"

LXConfDBEngine engine =
{
  (LXCONF_DB_OPEN)          open_db,
  (LXCONF_DB_RELOAD)        reaload_db,
  (LXCONF_DB_CLOSE)         close_db,

  (LXCONF_DB_REMOVE_KEY)    remove_key,
  (LXCONF_DB_REMOVE_GROUP)  remove_group,

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

sqlite3  *db;
gchar*    db_file = NULL;
gboolean  is_loaded = FALSE;

gboolean sql_wait_and_finalize(const char* command, GError **err)
{
  sqlite3_stmt *stmt;
  int rc;

  rc = sqlite3_prepare(db, command, -1, &stmt, 0);
  if( rc != SQLITE_OK)
  {
    if(err)
      *err = g_error_new( SQLITE_ERROR, sqlite3_errcode(db), sqlite3_errmsg(db) );
    g_debug("Error: %s", (*err)->message);
    sqlite3_finalize(stmt);
    return FALSE;
  }
  else
  {
    while((rc = sqlite3_step(stmt)) != SQLITE_DONE)
    {
      switch(rc)
      {
        case SQLITE_BUSY:
          g_debug("SQLITE_BUSY");
          sleep(1);
          break;
        case SQLITE_ERROR:
          if(err)
            *err = g_error_new( SQLITE_ERROR, sqlite3_errcode(db), sqlite3_errmsg(db) );
          sqlite3_finalize(stmt);
          return FALSE;
        default:
          g_debug("SQLITE_ %d", rc);
          break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return TRUE;
}

gboolean sql_wait_for_row(const gchar* command, sqlite3_stmt **stmt, GError **err)
{
  int rc = sqlite3_prepare(db, command, -1, stmt, 0);
  if( rc != SQLITE_OK)
  {
    if(err)
      *err = g_error_new( SQLITE_ERROR, sqlite3_errcode(db), sqlite3_errmsg(db) );
    return FALSE;
  }
  else
  {
    while((rc = sqlite3_step(*stmt)) != SQLITE_DONE)
    {
      switch(rc)
      {
        case SQLITE_BUSY:
          g_debug("SQLITE_BUSY");
          sleep(1);
          break;
        case SQLITE_ERROR:
          if(err)
            *err = g_error_new( SQLITE_ERROR, sqlite3_errcode(db), sqlite3_errmsg(db) );
          return FALSE;
        case SQLITE_ROW:
          return TRUE;
      }
    }
    return FALSE;
  }
}

/*gboolean sql_wait_for_row_finalize(const gchar* command, GError **err)
{
  sqlite3_stmt *stmt;
  gboolean retr = sql_wait_for_row(command, &stmt, err);
  sqlite3_finalize(stmt);
  return retr;
}*/

gboolean gid_key_exists(gint gid, const gchar* key)
{
  GError* err = NULL;
  sqlite3_stmt *stmt;
  gchar cmd[1024];
  gboolean retr;

  g_sprintf(cmd, "SELECT EKEY FROM "DTABLE" WHERE GID=%d AND EKEY='%s';", gid, key);
  retr = sql_wait_for_row(cmd, &stmt, &err);
  sqlite3_finalize(stmt);
  return retr;
}

gboolean open_db()
{
  GError *err = NULL;
  int rc;
  db_file = g_build_filename( g_get_home_dir () , ".config/lxconf-db.db", NULL );

  if( !g_file_test(db_file, G_FILE_TEST_EXISTS) )
  {
    rc = sqlite3_open(db_file, &db);
    if( rc != SQLITE_OK )
    {
      sqlite3_close(db);
      g_warning("cannot open db\n");
      return FALSE;
    }
    sql_wait_and_finalize(CREATE_INDEX_TABLE, &err);
    sql_wait_and_finalize(CREATE_DATA_TABLE, &err);
  }
  else
  {
    rc = sqlite3_open(db_file, &db);
    if( rc != SQLITE_OK)
    {
      sqlite3_close(db);
      g_warning("cannot open db\n");
      return FALSE;
    }
  }

  return TRUE;
}

void reaload_db ()
{
  /* dummy function, no need for sqlite3 */
}

void close_db ()
{
  if( is_loaded )
  {
    sqlite3_close(db);
  }
}

gint get_group_index(const gchar* group, GError **err)
{
  sqlite3_stmt *stmt;
  int gid = -1;
  gchar cmd[1024];

  g_sprintf(cmd, "SELECT GID FROM "ITABLE" WHERE GNAME='%s';", group);
  if( sql_wait_for_row(cmd, &stmt, err) )
    gid = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  return gid;
}

gint add_group(const gchar* group, GError **err)
{
  sqlite3_stmt *stmt;
  int gid = -1;
  gchar cmd[1024];

  g_sprintf(cmd, "SELECT MAX(GID) FROM "ITABLE";");
  if( sql_wait_for_row(cmd, &stmt, err) )
    gid = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  gid++;
  g_sprintf(cmd, "INSERT INTO "ITABLE" VALUES (%d,'%s');", gid, group);
  if( sql_wait_and_finalize(cmd, err) )
    return gid;
  else
    return -1;
}

void write_string (const gchar* group, const gchar* key, gchar* value, GError **err)
{
  gchar cmd[4096];
  gint gid = get_group_index( group, err );
  gboolean update = FALSE;

  if( gid == -1 )
  {
    gid = add_group(group, err);
  }
  else
  {
    update = gid_key_exists(gid, key);
  }

  if(update)
    g_sprintf(cmd, "UPDATE "DTABLE" SET EVALUE = '%s' WHERE GID = %d AND EKEY = '%s'", value, gid, key);
  else
    g_sprintf(cmd, "INSERT INTO "DTABLE" VALUES (%d,'%s','%s');", gid, key, value);

  sql_wait_and_finalize(cmd, err);
}

void write_int (const gchar* group, const gchar* key, gint value, GError **err)
{
  gchar cmd[1024];
  gint gid = get_group_index( group, err );
  gboolean update = FALSE;

  if( gid == -1 )
  {
    gid = add_group(group, err);
    g_printf("add_group(%s)\n", group);
  }
  else
  {
    update = gid_key_exists(gid, key);
  }

  if(update)
    g_sprintf(cmd, "UPDATE "DTABLE" SET EVALUE = %d WHERE GID = %d AND EKEY = '%s'", value, gid, key);
  else
    g_sprintf(cmd, "INSERT INTO "DTABLE" VALUES (%d,'%s',%d);", gid, key, value);
  sql_wait_and_finalize(cmd, err);
}

void write_double (const gchar* group, const gchar* key, gdouble value, GError **err)
{
  gchar cmd[1024];
  gint gid = get_group_index( group, err );
  gboolean update = FALSE;

  if( gid == -1 )
  {
    gid = add_group(group, err);
  }
  else
  {
    update = gid_key_exists(gid, key);
  }

  if(update)
    g_sprintf(cmd, "UPDATE "DTABLE" SET EVALUE = %f WHERE GID = %d AND EKEY = '%s'", value, gid, key);
  else
    g_sprintf(cmd, "INSERT INTO "DTABLE" VALUES (%d,'%s',%f);", gid, key, value);
  sql_wait_and_finalize(cmd, err);
}

void write_bool (const gchar* group, const gchar* key, gboolean value, GError **err)
{
  write_int(group, key, value, err);
}

void remove_key (const gchar* group, const gchar* key, GError **err)
{
  gchar cmd[1024];
  gint gid = get_group_index( group, err );
  if( gid != -1 )
  {
    g_sprintf(cmd, "DELETE FROM "DTABLE" WHERE GID=%d AND EKEY='%s';", gid, key);
    sql_wait_and_finalize(cmd, err);
  }
}

void remove_group (const gchar* group, GError **err)
{
  gchar cmd[1024];
  gint gid = get_group_index( group, err );
  if( gid != -1 )
  {
    g_sprintf(cmd, "DELETE FROM "DTABLE" WHERE GID=%d;DELETE FROM "ITABLE" WHERE GNAME='%s'", gid, group);
    sql_wait_and_finalize(cmd, err);
  }
}

gchar* read_string (const gchar* group, const gchar* key, GError **err)
{
  sqlite3_stmt *stmt;
  gchar cmd[1024], *retr = NULL;
  gint gid = get_group_index( group, err );

  if( gid != -1 )
  {
    g_sprintf(cmd, "SELECT EVALUE FROM "DTABLE" WHERE GID=%d AND EKEY='%s';", gid, key);
    if( sql_wait_for_row(cmd, &stmt, err) )
    {
      switch( sqlite3_column_type(stmt, 0) )
      {
        case SQLITE_TEXT:
          retr = g_strdup_printf("%s", sqlite3_column_text(stmt, 0) );
          break;
        case SQLITE_INTEGER:
          retr = g_strdup_printf("%d", sqlite3_column_int(stmt, 0) );
          break;
        case SQLITE_FLOAT:
          retr = g_strdup_printf("%f", sqlite3_column_double(stmt, 0) );
          break;
        default:
          if(err)
            *err = g_error_new( SQLITE_ERROR, LXCONF_WRONG_TYPE, "Wrong value type" );
      }
    }
    else
    {
      if(err)
        *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Key not found" );
    }
    sqlite3_finalize(stmt);
  }
  else
  {
    if(err)
      *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Group not found" );
  }

  return retr;
}

gint read_int (const gchar* group, const gchar* key, GError **err)
{
  sqlite3_stmt *stmt;
  gint retr = 0;
  gchar cmd[1024];
  gint gid = get_group_index( group, err );

  //printf("get_int GROUP:%s KEY:%s GID: %d\n", group, key, gid);

  if( gid != -1 )
  {
    g_sprintf(cmd, "SELECT EVALUE FROM "DTABLE" WHERE GID=%d AND EKEY='%s';", gid, key);
    if( sql_wait_for_row(cmd, &stmt, err) )
    {
      switch( sqlite3_column_type(stmt, 0) )
      {
        case SQLITE_INTEGER:
          retr = sqlite3_column_int(stmt, 0);
          break;
        default:
          if(err)
            *err = g_error_new( SQLITE_ERROR, LXCONF_WRONG_TYPE, "Wrong value type" );
      }
    }
    else
    {
      if(err)
        *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Key not found" );
    }
    sqlite3_finalize(stmt);
  }
  else
  {
    if(err)
      *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Group not found" );
  }

  return retr;
}

gdouble read_double (const gchar* group, const gchar* key, GError **err)
{
  sqlite3_stmt *stmt;
  gdouble retr = 0.0;
  gchar cmd[1024];
  gint gid = get_group_index( group, err );

  if( gid != -1 )
  {
    g_sprintf(cmd, "SELECT EVALUE FROM "DTABLE" WHERE GID=%d AND EKEY='%s';", gid, key);
    if( sql_wait_for_row(cmd, &stmt, err) )
    {
      switch( sqlite3_column_type(stmt, 0) )
      {
        case SQLITE_INTEGER:
          retr = (gdouble)sqlite3_column_int(stmt, 0);
          break;
        case SQLITE_FLOAT:
          retr = sqlite3_column_double(stmt, 0);
          break;
        default:
          if(err)
            *err = g_error_new( SQLITE_ERROR, LXCONF_WRONG_TYPE, "Wrong value type" );
      }
    }
    else
    {
      if(err)
        *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Key not found" );
    }
    sqlite3_finalize(stmt);
  }
  else
  {
    if(err)
      *err = g_error_new( SQLITE_ERROR, LXCONF_NOT_FOUND, "Group not found" );
  }

  return retr;
}

gboolean read_bool (const gchar* group, const gchar* key, GError **err)
{
  return read_int(group, key, err);
}
