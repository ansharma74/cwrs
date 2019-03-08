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

#ifndef __XKLAVIER_H__
#define __XKLAVIER_H__

#include <stdarg.h>

#include <X11/Xlib.h>

#include <glib-object.h>

#include <libxklavier/xkl_engine.h>
#include <libxklavier/xkl_config_rec.h>
#include <libxklavier/xkl_config_item.h>
#include <libxklavier/xkl_config_registry.h>
#include <libxklavier/xkl-enum-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xkl_get_last_error:
 *
 * Returns: the text message (statically allocated) of the last error
 */
	extern const gchar *xkl_get_last_error(void);

/**
 * _xkl_debug:
 * @file: the name of the source file. 
 * Preprocessor symbol__FILE__ should be used here
 * @function: name of the function
 * Preprocessor symbol__func__ should be used here
 * @level: level of the message
 * @format: is a format (like in printf)
 *
 * Output (optionally) some debug info
 */
	extern void _xkl_debug(const gchar file[], const gchar function[],
			       gint level, const gchar format[], ...);

/**
 * XklLogAppender:
 * @file: name of the source file. 
 * Preprocessor symbol__FILE__ should be used here
 * @function: name of the function
 * Preprocessor symbol__func__ should be used here
 * @level: level of the message
 * @format: format (like in printf)
 * @args: list of parameters
 *
 * Custom log output method for _xkl_debug. This appender is NOT called if the
 * level of the message is greater than currently set debug level.
 */
	typedef void (*XklLogAppender) (const gchar file[],
					const gchar function[],
					gint level,
					const gchar format[],
					va_list args);

/**
 * xkl_default_log_appender: (skip):
 * @file: name of the source file. 
 * Preprocessor symbol__FILE__ should be used here
 * @function: name of the function
 * Preprocessor symbol__func__ should be used here
 * @level: level of the message
 * @format: format (like in printf)
 * @args: list of parameters
 *
 * Default log output method. Sends everything to stdout.
 */
	extern void xkl_default_log_appender(const gchar file[],
					     const gchar function[],
					     gint level,
					     const gchar format[],
					     va_list args);

/**
 * xkl_set_log_appender: (skip):
 * @fun: new log appender
 *
 * Installs the custom log appender.function
 */
	extern void xkl_set_log_appender(XklLogAppender fun);

/**
 * xkl_set_debug_level:
 * @level: new debug level
 *
 * Sets maximum debug level. 
 * Message of the level more than the one set here - will be ignored
 */
	extern void xkl_set_debug_level(gint level);

#ifdef G_HAVE_ISO_VARARGS
/**
 * xkl_debug:
 * @level: level of the message
 *
 * Output (optionally) some debug info
 */
#define xkl_debug( level, ... ) \
  _xkl_debug( __FILE__, __func__, level, __VA_ARGS__ )
#elif defined(G_HAVE_GNUC_VARARGS)
/**
 * xkl_debug:
 * @level: level of the message
 * @format: format (like in printf)
 *
 * Output (optionally) some debug info
 */
#define xkl_debug( level, format, args... ) \
   _xkl_debug( __FILE__, __func__, level, format, ## args )
#else
#define xkl_debug( level, ... ) \
  _xkl_debug( __FILE__, __func__, level, __VA_ARGS__ )
#endif

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
