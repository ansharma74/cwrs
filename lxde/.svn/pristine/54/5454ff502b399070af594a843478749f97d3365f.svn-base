/*
 *      sqlite3-db.h
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

#ifndef SQLITE3_DB_H
#define SQLITE3_DB_H

#define ITABLE "itable"
#define DTABLE "dtable"
#define CREATE_INDEX_TABLE "CREATE TABLE "ITABLE" (GID INTEGER, GNAME TEXT, PRIMARY KEY (GNAME), UNIQUE(GID));"
#define CREATE_DATA_TABLE  "CREATE TABLE "DTABLE" (GID INTEGER, EKEY TEXT, EVALUE NUMERIC );"

/* #define CREATE_DATA_TABLE  "CREATE TABLE dtable (GID INTEGER, EKEY TEXT, EVALUE NUMERIC, FOREIGN KEY (GID) REFERENCES itable(GID) );" */

gboolean open_db      ();
void     reaload_db   ();
void     close_db     ();

void     remove_key   (const gchar* group, const gchar* key, GError **err);
void     remove_group (const gchar* group, GError **err);

void     write_string (const gchar* group, const gchar* key, gchar* value, GError **err);
void     write_double (const gchar* group, const gchar* key, gdouble value, GError **err);
void     write_int    (const gchar* group, const gchar* key, gint value, GError **err);
void     write_bool   (const gchar* group, const gchar* key, gboolean value, GError **err);

gchar*   read_string  (const gchar* group, const gchar* key, GError **err);
gdouble  read_double  (const gchar* group, const gchar* key, GError **err);
gint     read_int     (const gchar* group, const gchar* key, GError **err);
gboolean read_bool    (const gchar* group, const gchar* key, GError **err);

#endif /* SQLITE3_DB_H */
