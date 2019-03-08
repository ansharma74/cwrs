/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2001, 2002 Sun Microsystems Inc.,
 * Copyright 2001, 2002 Ximian, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef AT_SPI_DEBUG
#include <stdlib.h>
#endif

#include <config.h>
#include <string.h>
#include <gdk/gdkx.h>
#include <libbonobo.h>
#include <glib.h>
#include "registry.h"
#include <dbus/dbus-glib.h>
#include <gconf/gconf-client.h>


#define spi_get_display() GDK_DISPLAY()

static void registry_set_ior (SpiRegistry *registry);
#ifdef RELOCATE
static void set_gtk_path (DBusGProxy *gsm);
#endif
static void set_gtk_modules (DBusGProxy *gsm);

#ifdef RELOCATE
#define CORBA_GCONF_KEY  "/desktop/gnome/interface/at-spi-corba"
#else
#define DBUS_GCONF_KEY  "/desktop/gnome/interface/at-spi-dbus"
#endif

#define SM_DBUS_NAME      "org.gnome.SessionManager"
#define SM_DBUS_PATH      "/org/gnome/SessionManager"
#define SM_DBUS_INTERFACE "org.gnome.SessionManager"

#define SM_CLIENT_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

static DBusGConnection *bus_connection = NULL;
static DBusGProxy      *sm_proxy = NULL;
static char            *client_id = NULL;
static DBusGProxy      *client_proxy = NULL;

static gboolean
session_manager_connect (void)
{

        if (bus_connection == NULL) {
                GError *error;

                error = NULL;
                bus_connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
                if (bus_connection == NULL) {
                        g_message ("Failed to connect to the session bus: %s",
                                   error->message);
                        g_error_free (error);
                        exit (1);
                }
        }

        sm_proxy = dbus_g_proxy_new_for_name (bus_connection,
                                              SM_DBUS_NAME,
                                              SM_DBUS_PATH,
                                              SM_DBUS_INTERFACE);
        return (sm_proxy != NULL);
}

static void
stop_cb (gpointer data)
{
        bonobo_main_quit ();
}

static gboolean
end_session_response (gboolean is_okay, const gchar *reason)
{
        gboolean ret;
        GError *error = NULL;

        ret = dbus_g_proxy_call (client_proxy, "EndSessionResponse",
                                 &error,
                                 G_TYPE_BOOLEAN, is_okay,
                                 G_TYPE_STRING, reason,
                                 G_TYPE_INVALID,
                                 G_TYPE_INVALID);

        if (!ret) {
                g_warning ("Failed to send session response %s", error->message);
                g_error_free (error);
        }

        return ret;
}

static void
query_end_session_cb (guint flags, gpointer data)
{
        end_session_response (TRUE, NULL);
}

static void
end_session_cb (guint flags, gpointer data)
{
        Atom  AT_SPI_IOR = XInternAtom (spi_get_display (), "AT_SPI_IOR", TRUE);
        XDeleteProperty (spi_get_display(),
                         XDefaultRootWindow (spi_get_display ()),
                         AT_SPI_IOR);
        XFlush (spi_get_display());

        end_session_response (TRUE, NULL);
        bonobo_main_quit ();
}
static gboolean
register_client (void)
{
        GError     *error;
        gboolean    res;
        const char *startup_id;
        const char *app_id;

        startup_id = g_getenv ("DESKTOP_AUTOSTART_ID");
        app_id = "at-spi-registryd.desktop";

        error = NULL;
        res = dbus_g_proxy_call (sm_proxy,
                                 "RegisterClient",
                                 &error,
                                 G_TYPE_STRING, app_id,
                                 G_TYPE_STRING, startup_id,
                                 G_TYPE_INVALID,
                                 DBUS_TYPE_G_OBJECT_PATH, &client_id,
                                 G_TYPE_INVALID);
        if (! res) {
                g_warning ("Failed to register client: %s", error->message);
                g_error_free (error);
                return FALSE;
        }

        g_debug ("Client registered with session manager: %s", client_id);
        client_proxy = dbus_g_proxy_new_for_name (bus_connection,
                                                  SM_DBUS_NAME,
                                                  client_id,
                                                  SM_CLIENT_DBUS_INTERFACE);

        dbus_g_proxy_add_signal (client_proxy, "Stop", G_TYPE_INVALID);
        dbus_g_proxy_connect_signal (client_proxy, "Stop",
                                     G_CALLBACK (stop_cb), NULL, NULL);

        dbus_g_proxy_add_signal (client_proxy, "QueryEndSession", G_TYPE_UINT, G_TYPE_INVALID);
        dbus_g_proxy_connect_signal (client_proxy, "QueryEndSession",
                                     G_CALLBACK (query_end_session_cb), NULL, NULL);

        dbus_g_proxy_add_signal (client_proxy, "EndSession", G_TYPE_UINT, G_TYPE_INVALID);
        dbus_g_proxy_connect_signal (client_proxy, "EndSession",
                                     G_CALLBACK (end_session_cb), NULL, NULL);

        g_unsetenv ("DESKTOP_AUTOSTART_ID");

        return TRUE;
}

int
main (int argc, char **argv)
{
  int          ret;
  char        *obj_id;
  const char  *display_name;
  char        *cp, *dp;
  SpiRegistry *registry;

  DBusGConnection *connection;
  DBusGProxy      *gsm;
  GError          *error;

  GConfClient *gconf_client;
  gboolean need_to_quit;

  /* If we've been relocated, we will only run if the at-spi-corba gconf key
   * has been set.  If we have not been relocated, we will exit if the
   * at-spi-dbus key has been set.
   */
  gconf_client = gconf_client_get_default ();
#ifdef RELOCATE
  need_to_quit = !gconf_client_get_bool (gconf_client, CORBA_GCONF_KEY, NULL);
#else
  need_to_quit = gconf_client_get_bool (gconf_client, DBUS_GCONF_KEY, NULL);
#endif
  g_object_unref (gconf_client);

  if (need_to_quit)
    return 0;

  if (!bonobo_init (&argc, argv))
    {
      g_error ("Could not initialize oaf / Bonobo");
    }

  obj_id = "OAFIID:Accessibility_Registry:1.0";

  registry = spi_registry_new ();

  display_name = g_getenv ("AT_SPI_DISPLAY");
  if (!display_name)
  {
      display_name = g_getenv ("DISPLAY");
      cp = strrchr (display_name, '.');
      dp = strrchr (display_name, ':');
      if (cp && dp && (cp > dp)) *cp = '\0';
  }
  ret = bonobo_activation_register_active_server (
	  obj_id,
	  bonobo_object_corba_objref (bonobo_object (registry)),
	  NULL);

  if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
    {
#ifdef AT_SPI_DEBUG
      fprintf (stderr, "SpiRegistry Message: SpiRegistry daemon was already running.\n");
#endif
    }
  else
    {
#ifdef AT_SPI_DEBUG
      fprintf (stderr, "SpiRegistry Message: SpiRegistry daemon is running.\n");
#endif
      error = NULL;
      connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
      if (connection == NULL)
        {
          g_error ("couldn't get D-Bus connection: %s", error->message);
        }
      gsm = dbus_g_proxy_new_for_name (connection,
                                       "org.gnome.SessionManager",
                                       "/org/gnome/SessionManager",
                                       "org.gnome.SessionManager");
#ifdef RELOCATE
      set_gtk_path (gsm);
#endif
      set_gtk_modules (gsm);

      registry_set_ior (registry);

      if (!session_manager_connect ())
        {
          g_warning ("Unable to connect to session manager");
        }

      if (!register_client ())
        {
          g_warning ("Unable to register client with session manager");
        }

      bonobo_main ();
    }

  return 0;
}

static void
registry_set_ior (SpiRegistry *registry){
  CORBA_Environment ev;
  Atom  AT_SPI_IOR = XInternAtom (spi_get_display (), "AT_SPI_IOR", FALSE);
  char *iorstring = NULL;

  CORBA_exception_init (&ev);

  iorstring = CORBA_ORB_object_to_string (bonobo_activation_orb_get (),
                                     bonobo_object_corba_objref (bonobo_object (registry)),
                                     &ev);

  XChangeProperty (spi_get_display(),
		   XDefaultRootWindow (spi_get_display ()),
		   AT_SPI_IOR, (Atom) 31, 8,
		   PropModeReplace,
		   (unsigned char *) iorstring,
		   iorstring ? strlen (iorstring) : 0);

  if (ev._major != CORBA_NO_EXCEPTION)
	  {
		  g_error ("Error setting IOR %s",
			   CORBA_exception_id (&ev));
                  CORBA_exception_free (&ev);
           }

  CORBA_exception_free (&ev);

}

#ifdef RELOCATE
static void
set_gtk_path (DBusGProxy *gsm)
{
        const char *old;
	char       *corba_path;
        char       *value;
        gboolean    found;
        GError     *error;
        int         i, j;

	corba_path = g_build_filename (GTK_LIBDIR,
				       "gtk-2.0",
				       "modules",
				       "at-spi-corba",
				       NULL);

        old = g_getenv ("GTK_PATH");
        if (old != NULL) {
                char **old_path;
                char **path;

                old_path = g_strsplit (old, ":", -1);
		found = FALSE;
                for (i = 0; old_path[i]; i++) {
                        if (!strcmp (old_path[i], corba_path)) {
                                found = TRUE;
		        }
		}
                path = g_new (char *, i + (found ? 0 : 1) + 1);
		if (!found) {
		      path[0] = corba_path;
		      for (i = 0; old_path[i]; i++) {
                              path[i + 1] = old_path[i];
		      }
		      path[i + 1] = NULL;
                } else {
		      for (i = 0; old_path[i]; i++) {
                              path[i] = old_path[i];
		      }
		      path[i] = NULL;
		}
                value = g_strjoinv (":", path);
                g_free (path);
                g_strfreev (old_path);
        } else {
                value =  g_strdup (corba_path);
        }

	if (gsm != NULL) {
		error = NULL;
		if (!dbus_g_proxy_call (gsm, "Setenv", &error,
					G_TYPE_STRING, "GTK_PATH",
					G_TYPE_STRING, value,
					G_TYPE_INVALID,
					G_TYPE_INVALID)) {
			g_warning ("Could not set GTK_PATH: %s", error->message);
			g_error_free (error);
		}
	} else {
		g_setenv ("GTK_PATH", value, TRUE);
	}

        g_free (value);
	g_free (corba_path);
        return;
}
#endif

static void
set_gtk_modules (DBusGProxy *gsm)
{
        const char *old;
        char       *value;
        gboolean    found_gail;
        gboolean    found_atk_bridge;
        GError     *error;
        int         i;

        found_gail = FALSE;
        found_atk_bridge = FALSE;

        old = g_getenv ("GTK_MODULES");
        if (old != NULL) {
                char **old_modules;
                char **modules;

                old_modules = g_strsplit (old, ":", -1);
                for (i = 0; old_modules[i]; i++) {
                        if (!strcmp (old_modules[i], "gail")) {
                                found_gail = TRUE;
                        } else if (!strcmp (old_modules[i], "atk-bridge")) {
                                found_atk_bridge = TRUE;
                        }
                }

                modules = g_new (char *, i + (found_gail ? 0 : 1) +
                                 (found_atk_bridge ? 0 : 1) + 1);
                for (i = 0; old_modules[i]; i++) {
                        modules[i] = old_modules[i];
                }
                if (!found_gail) {
                                modules[i++] = "gail";
                }
                if (!found_atk_bridge) {
                        modules[i++] = "atk-bridge";
                }
                modules[i] = NULL;

                value = g_strjoinv (":", modules);
                g_free (modules);
                g_strfreev (old_modules);
        } else {
                value = g_strdup ("gail:atk-bridge");
        }

	if (gsm != NULL) {
		error = NULL;
		if (!dbus_g_proxy_call (gsm, "Setenv", &error,
					G_TYPE_STRING, "GTK_MODULES",
					G_TYPE_STRING, value,
					G_TYPE_INVALID,
					G_TYPE_INVALID)) {
			g_warning ("Could not set GTK_MODULES: %s", error->message);
			g_error_free (error);
		}
	} else {
		g_setenv ("GTK_MODULES", value, TRUE);
	}

        g_free (value);
        return;
}
