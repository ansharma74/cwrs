/*
 * man.h: Headers for the manual pager
 *
 * Copyright (C) 1990, 1991 John W. Eaton.
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
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

static inline const char *is_section (const char *name);
static const char **get_section_list (void);
static int do_global_apropos (const char *name, int *found);
static int man (const char *name, int *found);
static int display (const char *dir, const char *man_file,
		    const char *cat_file, const char *title,
		    const char *dbfilters);
static inline int do_prompt (const char *name);
