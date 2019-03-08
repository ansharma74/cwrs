/**
 * Copyright (c) 2008 LxDE Developers, see the file AUTHORS for details.
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

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <lxpanel/plugin.h>
#include <dbus/dbus-glib.h>
#include <dbus-1.0/dbus/dbus.h>
#include <libnotify/notify.h>
#include <unistd.h>
#include <sys/types.h>

#define VOLUME_PATH "volume-path"
#define POPUP_DRV "popup-drv"
#define GTK_NO_ICON PACKAGE_DATA_DIR "/lxpanel/images/empty.png" 

typedef struct {
    Plugin* plugin;
    GtkWidget * mainw,
              * tray_icon,
              * dlg;
    int show, count;
       GSList * drives;
    DBusGConnection * connection;
} drive_t;



static void update_display( drive_t * drv);

/***************************************************************************
*  Taked from Guano's discover-pluged-devices.c of Felix Prieto Carratala  *
****************************************************************************/

static DBusGProxy *proxy_device = NULL;
static DBusGProxy *proxy_manager = NULL;
static DBusGProxy *proxy_volume = NULL;
static char *label = NULL;
static char *model = NULL;
static char *fstype = NULL;

static char*
get_dir_name(char *name)
{
    gchar * dir_name = g_strjoinv("_", g_strsplit(name, " ", 0));
    gint counter = 0;
    GString *_counter = g_string_new("");
    while(g_access(g_strconcat("/media/", dir_name, NULL), F_OK) == 0)
    {
        dir_name = g_strjoinv("_", g_strsplit(name, " ", 0));
        dir_name = g_strconcat(dir_name, _counter->str, NULL);
        counter++;
        g_string_printf(_counter, "%i", counter);
    }   
    g_string_free(_counter, FALSE);
    return dir_name;
}

static gchar* 
mount_device (drive_t * drv, gchar * path )
{
    GError* error = NULL;
    gchar * dir_name = NULL, *mount_point = NULL;

    if (proxy_device)
        g_object_unref(proxy_device);
    proxy_device = dbus_g_proxy_new_for_name(drv -> connection, 
                                    "org.freedesktop.Hal",
    	                            path, 
                                    "org.freedesktop.Hal.Device");
    g_return_val_if_fail(proxy_device, NULL);
    if(!dbus_g_proxy_call(proxy_device, "GetPropertyString", &error,
                            G_TYPE_STRING, "volume.mount_point", G_TYPE_INVALID,
                    	    G_TYPE_STRING, &mount_point, G_TYPE_INVALID)) 
    {        
        if (error)
	    return NULL; 
    }
    if(!dbus_g_proxy_call(proxy_device, "GetPropertyString", &error,
                            G_TYPE_STRING, "volume.fstype", G_TYPE_INVALID,
                    	    G_TYPE_STRING, &fstype, G_TYPE_INVALID)) 
    {        
        if (error)
	    return NULL;
    }
    if (proxy_volume)
            g_object_unref(proxy_volume);
        proxy_volume = dbus_g_proxy_new_for_name(drv -> connection, 
                                    "org.freedesktop.Hal",
    	                            path, 
                                    "org.freedesktop.Hal.Device.Volume");
    g_return_val_if_fail(proxy_volume, NULL);
    dir_name = get_dir_name(label);
    gchar *uid = g_malloc (sizeof (gchar)*6);
    g_sprintf (uid, "uid=%i", getuid ());   
    const gchar *options[] = {"exec", uid};
    error = NULL;
    dbus_g_proxy_call(proxy_volume, "Mount", &error,
                      G_TYPE_STRING, dir_name,   
                      G_TYPE_STRING, NULL,    
                      G_TYPE_STRV, options, G_TYPE_INVALID,
                      G_TYPE_INT, NULL, G_TYPE_INVALID);
    g_free (uid);
    //g_strfreev (options);
    g_free(mount_point);    
    
    return (error != NULL ? NULL : g_strdup(dir_name)); 
}

gboolean umount_volume( GtkWidget * btn,  drive_t * drv )
{
    GError * error = NULL;
    gchar * path = (gchar *)g_object_get_data(btn, VOLUME_PATH);
    if (proxy_volume)
        g_object_unref(proxy_volume);
    proxy_volume = dbus_g_proxy_new_for_name( drv -> connection, 
                                    "org.freedesktop.Hal",
                                    path, 
                                    "org.freedesktop.Hal.Device.Volume");
    g_return_val_if_fail(proxy_volume, NULL);
    
    const gchar *options[] = {""};

    if (dbus_g_proxy_call(proxy_volume, "Unmount", &error,
                      G_TYPE_STRV, options, G_TYPE_INVALID,
                      G_TYPE_INT, NULL, G_TYPE_INVALID))
    {
        update_display( drv );
    }
    else
        fprintf(stderr,"%s\n",error -> message);
    g_free(path);
    g_error_free(error);
}

static void action_cb (NotifyNotification *notification, gchar *action, gpointer user_data)
{
    notify_notification_close(notification, NULL);    
    gchar *dir_name = (gchar*)user_data;
    gchar *cmd = g_strconcat("pcmanfm /media/", dir_name ,NULL);
    GError *error = NULL;
    if(!g_spawn_command_line_async(cmd, &error))
    {
        g_printerr("%s\n", error->message);
        g_error_free(error);
    }    
    g_free(dir_name);
}

static void closed_cb(NotifyNotification *notification, gchar *action, gpointer user_data)
{
    g_object_unref(notification);
    notify_uninit();    
}

static void DeviceAdded(DBusGProxy *proxy, char* udi, drive_t * drv)
{           
	char *namespace = NULL;
    GError *error = NULL; 
    NotifyNotification* notification = NULL;
        
    if (proxy_device)        
        g_object_unref(proxy_device);
	proxy_device = dbus_g_proxy_new_for_name(drv -> connection, 
                                    "org.freedesktop.Hal",
    	                            udi, 
                                    "org.freedesktop.Hal.Device");    
    g_return_if_fail(proxy_device);
	if(!dbus_g_proxy_call(proxy_device, "GetPropertyString", &error,
                       	  G_TYPE_STRING, "info.category", G_TYPE_INVALID,
                    	  G_TYPE_STRING, &namespace, G_TYPE_INVALID))
        return;

    if (g_ascii_strcasecmp(namespace, "storage") == 0)
    {
        model = NULL;
        dbus_g_proxy_call(proxy_device, "GetPropertyString", &error,
    	                    G_TYPE_STRING, "storage.model", G_TYPE_INVALID,
                    	    G_TYPE_STRING, &model, G_TYPE_INVALID);
    }    
    if(g_ascii_strcasecmp(namespace, "volume") == 0)
    {   
        label = NULL;         
        if(!dbus_g_proxy_call(proxy_device, "GetPropertyString", &error,
    	                    G_TYPE_STRING, "volume.label", G_TYPE_INVALID,
                    	    G_TYPE_STRING, &label, G_TYPE_INVALID))
            return; 
        
        if (g_ascii_strcasecmp(label, "") == 0)        
            label = model;
        gchar * path = g_strdup(udi);        
        gchar * dir_name = mount_device ( drv , path);
        drv -> drives = g_slist_append (drv -> drives, g_strconcat(udi,",",label,",",path,NULL) );
        update_display( drv );

        if(notify_init("Notification"))
        {                                
            notification = notify_notification_new (
                            g_strconcat (_("File system mounted on "), dir_name, NULL), 
                            label, NULL, NULL); 
            
            notify_notification_add_action (notification,
                                        "Open",
                                        "Open",//_("Open") 
                                        NOTIFY_ACTION_CALLBACK(action_cb),
                                        (gpointer)dir_name, NULL);
            g_signal_connect(notification, "closed", G_CALLBACK(closed_cb), 
                                 NULL);
            if (!notify_notification_show(notification, &error))
            {
                notify_uninit();
            }
        }
        else
            g_printerr("libnotify failed\n");                    
	}
    g_free(namespace);
}

static void DeviceRemoved(DBusGProxy *proxy, char * udi, drive_t * drv )
{
    if( ! drv -> drives )return;
    if( g_slist_length(drv -> drives) == 1 ) drv -> drives = NULL;
    else drv -> drives = g_slist_remove(drv -> drives, udi);
    update_display( drv );
}

/************************** End of dbus section ****************************/

static void update_display( drive_t * drv )
{
    if ( g_slist_length(drv -> drives) ) 
    gtk_image_set_from_icon_name( (GtkImage*)drv -> tray_icon, 
                                  "gnome-dev-removable-usb",
                                  GTK_ICON_SIZE_SMALL_TOOLBAR);
    else  gtk_image_set_from_file( (GtkImage*)drv -> tray_icon, GTK_NO_ICON );
}

static void g_list_append_to_menu(char * data,  GtkMenu* popup )
{
    GtkWidget * mitem,
              * image;        
    image = gtk_image_new_from_icon_name("gnome-dev-removable-usb",
                                        GTK_ICON_SIZE_MENU);
    mitem = gtk_image_menu_item_new_with_label ( g_strsplit(data,",",-1)[1]);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(mitem),image);
    gtk_menu_shell_append (GTK_MENU_SHELL (popup), mitem);
    g_signal_connect(mitem,"activate", umount_volume , (drive_t * )g_object_get_data(popup , POPUP_DRV) );
    g_object_set_data(mitem, VOLUME_PATH, g_strsplit(data,",",-1)[2]);
}

static gboolean tray_icon_press(GtkWidget *widget, GdkEventButton *event, drive_t *drv)
{
    if( event->button == 3 && drv -> drives != NULL )  /* right button */
    {
        GtkWidget * popup = gtk_menu_new();
        g_object_set_data(popup, POPUP_DRV, drv );
        g_slist_foreach(drv -> drives, (GFunc)g_list_append_to_menu, popup);
        gtk_widget_show_all(popup);
        gtk_menu_popup( (GtkMenu*)popup, NULL, NULL, NULL, NULL,
                        event->button, event->time );

        return TRUE;
    }
    g_spawn_command_line_async("pcmanfm /media",NULL);
    return TRUE;
}

static void lxdrive_destructor(Plugin *p)
{
    drive_t * drv = (drive_t *) p->priv;
    ENTER;
    g_free(drv);
    RET();
}

static int lxdrive_constructor(Plugin *p, char **fp)
{
    drive_t * drv;
    GError * error = NULL;
    line s;
    ENTER;
    s.len = 256;
    drv = g_new0(drive_t, 1);
    drv -> plugin = p;
    g_return_val_if_fail(drv != NULL, 0);
    p->priv = drv;
   
    drv -> drives = NULL;
    drv -> connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    if (!error)
        proxy_manager = dbus_g_proxy_new_for_name (drv -> connection,"org.freedesktop.Hal",
                       "/org/freedesktop/Hal/Manager", "org.freedesktop.Hal.Manager");
    else {fprintf(stderr,"%s\n",error-> message);RET(0);}
    
    drv -> count = 0;
    drv -> show  = 0;

    /* main */
   
    drv -> tray_icon = gtk_image_new();

    drv -> mainw = gtk_event_box_new();

    gtk_widget_add_events(drv -> mainw, GDK_BUTTON_PRESS_MASK);
    
    gtk_container_add(GTK_CONTAINER(drv->mainw), drv->tray_icon);

    gtk_widget_show_all(drv -> mainw);

    gtk_widget_set_tooltip_text( drv->mainw, _("Manage removable devices"));

    /* store the created plugin widget in plugin->pwid */

    p->pwid = drv -> mainw;
	
	/* connect signals */	
	
	g_signal_connect(G_OBJECT(drv->mainw), "button-press-event",
                         G_CALLBACK(tray_icon_press), drv);
    
    dbus_g_proxy_add_signal(proxy_manager,
                            "DeviceAdded",
                            G_TYPE_STRING, 
                            G_TYPE_INVALID);

    dbus_g_proxy_connect_signal(proxy_manager, 
                                "DeviceAdded", 
                                G_CALLBACK(DeviceAdded),
                                drv, NULL);

    dbus_g_proxy_add_signal(proxy_manager,
                            "DeviceRemoved",
                            G_TYPE_STRING,
                            G_TYPE_INVALID);

    dbus_g_proxy_connect_signal(proxy_manager, 
                                "DeviceRemoved", 
                                G_CALLBACK(DeviceRemoved),
                                drv, NULL);
    RET(1);
}

PluginClass lxdrive_plugin_class = {
    fname: NULL,
    count: 0,

    type : "lxdrive",
    name : N_("Removable devices manager"),
    version: "0.1",
    description : "Manage removable devices",

    constructor : lxdrive_constructor,
    destructor  : lxdrive_destructor,
    config : NULL,
    save : NULL
};
