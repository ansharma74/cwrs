/*
 * ult_src.h: Interface to finding the ultimate source of a page
 *
 * Copyright (C) 1990, 1991 John W. Eaton.
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2002, 2003, 2011 Colin Watson.
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

#define SO_LINK		0001
#define SOFT_LINK	0002
#define HARD_LINK	0004

/* Trace of the link chain from a given file.  Any names listed here should
 * not have WHATIS_MAN entries created for them.
 */
struct ult_trace {
	char **names;
	size_t len;
	size_t max;
};

struct stat;

extern const char *ult_src (const char *name, const char *path,
			    struct stat *buf, int flags,
			    struct ult_trace *trace);
extern void free_ult_trace (struct ult_trace *trace);
