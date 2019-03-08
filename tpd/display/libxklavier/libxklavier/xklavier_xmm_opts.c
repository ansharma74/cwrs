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

#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "config.h"

#include "xklavier_private.h"
#include "xklavier_private_xmm.h"

#define SHORTCUT_OPTION_PREFIX "grp:"

static XmmSwitchOption options[] = {
	{{{XK_Alt_R, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Alt_L, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Caps_Lock, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Caps_Lock, ShiftMask}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Shift_R, ShiftMask}
	  ,
	  {XK_Shift_L, ShiftMask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1}},
	{{{XK_Alt_R, Mod1Mask}
	  ,
	  {XK_Alt_L, Mod1Mask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1}},
	{{{XK_Control_R, ControlMask}
	  ,
	  {XK_Control_L, ControlMask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1}},
	{{{XK_Control_R, ShiftMask}
	  ,
	  {XK_Control_L, ShiftMask}
	  ,
	  {XK_Shift_R, ControlMask}
	  ,
	  {XK_Shift_L, ControlMask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1, 1, -1}},
	{{{XK_Control_R, Mod1Mask}
	  ,
	  {XK_Control_L, Mod1Mask}
	  ,
	  {XK_Alt_R, ControlMask}
	  ,
	  {XK_Alt_L, ControlMask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1, 1, -1}},
	{{{XK_Shift_R, Mod1Mask}
	  ,
	  {XK_Shift_L, Mod1Mask}
	  ,
	  {XK_Alt_R, ShiftMask}
	  ,
	  {XK_Alt_L, ShiftMask}
	  , {XK_VoidSymbol}
	  }
	 , {1, -1, 1, -1}},
	{{{XK_Menu, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Super_L, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Super_R, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Shift_L, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Shift_R, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Control_L, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}},
	{{{XK_Control_R, 0}
	  , {XK_VoidSymbol}
	  }
	 , {1}}
};

static const gchar *option_names[] = {
	"ralt_toggle",
	"lalt_toggle",
	"caps_toggle",
	"shift_caps_toggle",
	"shifts_toggle",
	"alts_toggle",
	"ctrls_toggle",
	"ctrl_shift_toggle",
	"ctrl_alt_toggle",
	"alt_shift_toggle",
	"menu_toggle",
	"lwin_toggle",
	"rwin_toggle",
	"lshift_toggle",
	"rshift_toggle",
	"lctrl_toggle",
	"rctrl_toggle"
};

void
xkl_xmm_init_switch_options(XklXmm * xmm)
{
	int i;
	const gchar **pname = option_names;
	const XmmSwitchOption *poption = options;

	xmm->switch_options = g_hash_table_new(g_str_hash, g_str_equal);

	for (i = sizeof(option_names) / sizeof(option_names[0]); --i >= 0;)
		g_hash_table_insert(xmm->switch_options,
				    (gpointer) (*pname++),
				    (gpointer) (poption++));
}

void
xkl_xmm_term_switch_options(XklXmm * xmm)
{
	g_hash_table_destroy(xmm->switch_options);
	xmm->switch_options = NULL;
}
