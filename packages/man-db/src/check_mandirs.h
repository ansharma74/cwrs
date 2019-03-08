/*
 * check_mandirs.h: Interface to updating database caches
 *
 * Copyright (C) 2001, 2002 Colin Watson.
 *
 * This file is part of man-db.
 *
 * man-db is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * man-db is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with man-db; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "db_storage.h"

/* check_mandirs.c */
extern void test_manfile (const char *file, const char *path);
extern void update_db_time (void);
extern void reset_db_time (void);
extern int create_db (const char *manpath, const char *catpath);
extern int update_db (const char *manpath, const char *catpath);
extern void purge_pointers (const char *name);
extern int purge_missing (const char *manpath, const char *catpath);
