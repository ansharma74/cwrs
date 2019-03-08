/**
 * Copyright (c) 2009 Fred Chien <cfsghost@gmail.com>
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
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	char *buf, *tmp, *src;
	char c[3];

	if (argc!=2)
		return 1;

	src = argv[1];
	buf = malloc(sizeof(char)*(1+strlen(src)*2));
	tmp = buf;

	for (;*src!='\0';src++) {
		sprintf(c, "%X", *src);
		*tmp = c[0];
		*(tmp+1) = c[1];
		tmp += 2;
	}

	*tmp = '\0';

	printf("%s", buf);

	return 0;
}

