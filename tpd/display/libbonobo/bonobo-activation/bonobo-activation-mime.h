/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 2006 Red Hat, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Alexander Larsson <alexl@redhat.com>
 */

#ifndef BONOBO_ACTIVATION_MIME_H
#define BONOBO_ACTIVATION_MIME_H

#include <bonobo-activation/bonobo-activation-server-info.h>

Bonobo_ServerInfo *bonobo_activation_get_default_component_for_mime_type (const char *mime_type);
GList *            bonobo_activation_get_all_components_for_mime_type    (const char *mime_type);

#endif /* BONOBO_ACTIVATION_MIME_H */
