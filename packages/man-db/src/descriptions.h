/*
 * descriptions.h: Interface to manipulating man page descriptions
 *
 * Copyright (C) 2002, 2007, 2008, 2011 Colin Watson.
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

struct page_description {
	char *name;
	char *whatis;
	struct page_description *next;
};

struct ult_trace;

extern struct page_description *parse_descriptions (const char *base,
						    const char *whatis);
extern void store_descriptions (const struct page_description *head,
				struct mandata *info,
				const char *path, const char *base,
				struct ult_trace *trace);
extern void free_descriptions (struct page_description *head);
