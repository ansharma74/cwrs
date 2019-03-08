/*
 * Copyright (C) 2002-2006 Sergey V. Udaltsov <svu@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <fcntl.h>

#include <libxml/xpath.h>

#include "config.h"

#include "xklavier_private.h"
#include "xklavier_private_xmm.h"

#define XK_XKB_KEYS
#include <X11/keysymdef.h>

void
xkl_xmm_init_config_registry(XklConfigRegistry * config)
{
}

gboolean
xkl_xmm_load_config_registry(XklConfigRegistry * config,
			     gboolean if_extras_needed)
{
	return xkl_config_registry_load_helper(config,
					       XMODMAP_DEFAULT_RULESET,
					       XMODMAP_BASE,
					       if_extras_needed);
}

gboolean
xkl_xmm_activate_config_rec(XklEngine * engine, const XklConfigRec * data)
{
	gboolean rv;
	rv = xkl_config_rec_set_to_root_window_property(data,
							xkl_engine_priv
							(engine,
							 base_config_atom),
							xkl_engine_backend
							(engine, XklXmm,
							 current_rules),
							engine);
	if (rv)
		xkl_xmm_lock_group(engine, 0);
	return rv;
}
