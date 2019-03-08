/**
 * Copyright (c) 2006 LxDE Developers, see the file AUTHORS for details.
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

#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <menu-cache.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "panel.h"
#include "misc.h"
#include "plugin.h"
#include "bg.h"
#include "menu-policy.h"

#include "dbg.h"

#define DEFAULT_MENU_ICON PACKAGE_DATA_DIR "/lxpanel/images/menu-button.png"
#define DEFAULT_BG PACKAGE_DATA_DIR "/lxpanel/images/menu.png"
#define WIN_HINTS_SKIP_FOCUS      (1<<0) 
#define DEFAULT_FILE_MANAGER "pcmanfm"
#define DEFAULT_PACKAGE_MANAGER "gksu synaptic"
#define DEFAULT_TERMINAL "lxterminal"
#define DEFAULT_RUN_COMMAND "lxpanelctl run"
#define DEFAULT_LOCK_COMMAND "xscreensaver-command --lock"
#define DEFAULT_QUIT_COMMAND "guano-logout"
#define MENUP "menup"

static int width = 0 , height = 0;

enum {
    COL_ICON = 0,
    COL_TITLE,
    COL_BTN,
    N_COLS
};

enum {
    TARGET_URILIST,
    TARGET_UTF8_STRING,
      TARGET_STRING,
      TARGET_TEXT,
      TARGET_COMPOUND_TEXT
};

static const char desktop_ent[] = "Desktop Entry";

static const GtkTargetEntry target_table[] = {
    { "text/uri-list", 0, TARGET_URILIST},
    { "UTF8_STRING", 0, TARGET_UTF8_STRING },
    { "COMPOUND_TEXT", 0, 0 },
    { "TEXT",          0, 0 },
    { "STRING",        0, 0 }
};

typedef struct {
    GtkWidget *menu, *win, *box, *img, *config_dlg, *favorite_box;
    GSList * favorites;
    char *fname, *caption;
    gulong handler_id;
    int iconsize, paneliconsize;
    GSList *files;
    gboolean has_system_menu, show, popup;
    char* config_data;
    int sysmenu_pos;
    MenuCache* menu_cache;
    gpointer reload_notify;
} menup;

typedef struct btn_t {
    Plugin* plugin;
    GtkWidget* widget;
    gchar *desktop_id;
    gchar *image;
    gchar *action;
    gchar *tooltip;
    guchar use_terminal : 1;
    guchar customize_image : 1;
    guchar customize_action : 1;
    guchar customize_tooltip : 1;
} btn_t;

static guint idle_loader = 0;

GQuark SYS_MENU_ITEM_ID = 0;

/* a single-linked list storing all panels */
extern GSList* all_panels;

void panel_config_save(Panel* panel);

static void
menu_destructor(Plugin *p)
{
    menup *m = (menup *)p->priv;

    if( G_UNLIKELY( idle_loader ) )
    {
        g_source_remove( idle_loader );
        idle_loader = 0;
    }

    if( m->has_system_menu )
        p->panel->system_menus = g_slist_remove( p->panel->system_menus, p );

    g_signal_handler_disconnect(G_OBJECT(m->img), m->handler_id);
    gtk_widget_destroy(m->menu);

    if( m->menu_cache )
    {
        menu_cache_remove_reload_notify(m->menu_cache, m->reload_notify);
        menu_cache_unref( m->menu_cache );
    }

    g_free(m->fname);
    g_free(m->caption);
    g_free(m);
    RET();
}

gchar * trunc_label(gchar * str , int max)
{
    gchar * ret = strdup(str);
    ret[max] = '\0';
    strcat(ret,"...");
    return ret;
}

static gboolean btn_press_event(GtkWidget * widget, GdkEventButton * event, btn_t * btn)
{
    menup * m = (menup *)btn -> plugin -> priv;
    if (event->button == 1)    /* left button */
    {
        m -> show = 0;
        gtk_widget_hide( m -> win );
        lxpanel_launch_app(btn -> action, NULL, btn -> use_terminal);
    }
    return TRUE;
}

static gboolean focus_out_event(GtkWidget *widget, GdkEvent *event, menup * m)
{
    if ( m -> popup )
    {
        m -> popup = FALSE;
        return TRUE;
    }
    gtk_widget_hide(m -> win);
    m -> show = FALSE;
    return FALSE;
}

GtkWidget * menu_button_new( btn_t * btn,
                             gchar * icon,
                             gchar * label,
                             gchar * cmd ,
                             GtkIconSize size,
			                 gboolean IsMenuButton )
{
    btn->image   = g_strdup(icon);
    btn->tooltip = g_strdup(label);
    btn->action  = g_strdup(cmd);

    GtkImage * image = (GtkImage *)_gtk_image_new_from_file_scaled(icon, 24, 24, TRUE);
    btn->widget  = gtk_button_new_with_label( strlen((char*)label) < 21 || IsMenuButton ? label
                                              : trunc_label(label, 21));
    gtk_button_set_relief((GtkButton*)btn-> widget,GTK_RELIEF_NONE);
    gtk_button_set_alignment ((GtkButton*)btn-> widget,0.0,0.5);
    
    if ( IsMenuButton )
		gtk_widget_set_name(btn-> widget,"GMenuButton");
    
    gtk_button_set_image((GtkButton*)btn-> widget,
                         GDK_IS_PIXBUF(gtk_image_get_pixbuf(image)) ? image :
                         gtk_image_new_from_icon_name(icon,size));
    g_signal_connect( G_OBJECT(btn->widget), "button-press-event" , btn_press_event, btn );

    return  btn->widget;
}

GtkWidget * create_right_menubar( Plugin * p )
{
    GtkWidget * right_menubar = gtk_vbox_new(FALSE,6);

    btn_t * home = g_slice_new0( btn_t );
    home -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( home, GTK_STOCK_HOME, _("My Documents"), 
					   lxpanel_get_file_manager() ? lxpanel_get_file_manager(): DEFAULT_FILE_MANAGER,
                       GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE) , FALSE, FALSE,0);

    btn_t * package_manager  = g_slice_new0( btn_t );
    package_manager -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( package_manager, "system-software-install",
                         _("Package manager"), DEFAULT_PACKAGE_MANAGER ,
                        GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE), FALSE, FALSE,0);

    gtk_box_pack_start (GTK_BOX (right_menubar), gtk_hseparator_new() , FALSE, FALSE,0);

    btn_t * terminal = g_slice_new0( btn_t );
    terminal -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( terminal, "gnome-terminal", _("Terminal"), 
					   lxpanel_get_terminal() ? lxpanel_get_terminal() : DEFAULT_TERMINAL,
                       GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE ) , FALSE, FALSE,0);
    
    btn_t * run = g_slice_new0( btn_t );
    run -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( run, GTK_STOCK_EXECUTE, _("Run"), 
					   DEFAULT_RUN_COMMAND,GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE ) , FALSE, FALSE,0);

    btn_t * lock = g_slice_new0( btn_t );
    lock -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( lock, "system-lock-screen", _("Lock Screen"), 
					   DEFAULT_LOCK_COMMAND, GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE ) , FALSE, FALSE,0);

    btn_t * quit = g_slice_new0( btn_t );
    quit -> plugin = p;
    gtk_box_pack_start (GTK_BOX (right_menubar), menu_button_new( quit, GTK_STOCK_QUIT, _("Quit"), 
					   DEFAULT_QUIT_COMMAND,GTK_ICON_SIZE_LARGE_TOOLBAR , TRUE ) , FALSE, FALSE,0);

    return right_menubar;
}

void btn_free( btn_t* btn )
{
    g_free( btn->desktop_id );
    g_free( btn->image );
    g_free( btn->action );
    g_free( btn->tooltip );
    g_free( btn );
}    

static void on_menu_item( GtkMenuItem* mi, MenuCacheItem* item )
{
    menup * m = (menup *)g_object_get_data( mi , MENUP );
    m -> show = FALSE;
    gtk_widget_hide( m->win );
    lxpanel_launch_app( menu_cache_app_get_exec(MENU_CACHE_APP(item)),
            NULL, menu_cache_app_get_use_terminal(MENU_CACHE_APP(item)));
}

/* load icon when mapping the menu item to speed up */
static void on_menu_item_map(GtkWidget* mi, MenuCacheItem* item)
{
    GtkImage* img = GTK_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(mi)));
    if( img )
    {
        if( gtk_image_get_storage_type(img) == GTK_IMAGE_EMPTY )
        {
            GdkPixbuf* icon;
            int w, h;
            /* FIXME: this is inefficient */
            gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &w, &h);
            item = g_object_get_qdata(G_OBJECT(mi), SYS_MENU_ITEM_ID);
            icon = lxpanel_load_icon(menu_cache_item_get_icon(item), w, h, TRUE);
            if (icon)
            {
                gtk_image_set_from_pixbuf(img, icon);
                g_object_unref(icon);
            }
        }
    }
}

static void on_menu_item_style_set(GtkWidget* mi, GtkStyle* prev, MenuCacheItem* item)
{
    /* reload icon */
    on_menu_item_map(mi, item);
}

static void on_add_menu_item_to_desktop(GtkMenuItem* item, MenuCacheApp* app)
{
    char* dest;
    char* src;
    g_debug("app: %p", app);
    const char* desktop = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    int dir_len = strlen(desktop);
    int basename_len = strlen(menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
    int dest_fd;

    dest = g_malloc( dir_len + basename_len + 6 + 1 + 1 );
    memcpy(dest, desktop, dir_len);
    dest[dir_len] = '/';
    memcpy(dest + dir_len + 1, menu_cache_item_get_id(MENU_CACHE_ITEM(app)), basename_len + 1);

    /* if the destination file already exists, make a unique name. */
    if( g_file_test( dest, G_FILE_TEST_EXISTS ) )
    {
        memcpy( dest + dir_len + 1 + basename_len - 8 /* .desktop */, "XXXXXX.desktop", 15 );
        dest_fd = g_mkstemp(dest);
        if( dest_fd >= 0 )
            chmod(dest, 0600);
    }
    else
    {
        dest_fd = creat(dest, 0600);
    }

    if( dest_fd >=0 )
    {
        char* data;
        gsize len;
        src = menu_cache_item_get_file_path(MENU_CACHE_ITEM(app));
        if( g_file_get_contents(src, &data, &len, NULL) )
        {
            write( dest_fd, data, len );
            g_free(data);
        }
        close(dest_fd);
        g_free(src);
    }
    g_free(dest);
}

static void on_menu_item_properties(GtkMenuItem* item, MenuCacheApp* app)
{
    /* FIXME: if the source desktop is in AppDir other then default
     * applications dirs, where should we store the user-specific file?
    */
    char* ifile = menu_cache_item_get_file_path(MENU_CACHE_ITEM(app));
    char* ofile = g_build_filename(g_get_user_data_dir(), "applications",
				   menu_cache_item_get_file_basename(MENU_CACHE_ITEM(app)), NULL);
    char* argv[] = {
        "lxshortcut",
        "-i",
        NULL,
        "-o",
        NULL,
        NULL};
    argv[2] = ifile;
    argv[4] = ofile;
    g_spawn_async( NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL );
    g_free( ifile );
    g_free( ofile );
}

static void restore_grabs(GtkWidget *w, gpointer data)
{
    GtkWidget *menu_item = data;
    GtkMenu *menu = GTK_MENU(menu_item->parent);
    GtkWidget *xgrab_shell;
    GtkWidget *parent;

    /* Find the last viewable ancestor, and make an X grab on it
    */
    parent = GTK_WIDGET (menu);
    xgrab_shell = NULL;
    while (parent)
    {
        gboolean viewable = TRUE;
        GtkWidget *tmp = parent;

        while (tmp)
        {
            if (!GTK_WIDGET_MAPPED (tmp))
            {
                viewable = FALSE;
                break;
            }
            tmp = tmp->parent;
        }

        if (viewable)
            xgrab_shell = parent;

        parent = GTK_MENU_SHELL (parent)->parent_menu_shell;
    }

    /*only grab if this HAD a grab before*/
    if (xgrab_shell && (GTK_MENU_SHELL (xgrab_shell)->have_xgrab))
    {
        if (gdk_pointer_grab (xgrab_shell->window, TRUE,
                    GDK_BUTTON_PRESS_MASK |
                    GDK_BUTTON_RELEASE_MASK |
                    GDK_ENTER_NOTIFY_MASK |
                    GDK_LEAVE_NOTIFY_MASK,
                    NULL, NULL, 0) == 0)
        {
            if (gdk_keyboard_grab (xgrab_shell->window, TRUE,
                    GDK_CURRENT_TIME) == 0)
                GTK_MENU_SHELL (xgrab_shell)->have_xgrab = TRUE;
            else
                gdk_pointer_ungrab (GDK_CURRENT_TIME);
        }
    }
    gtk_grab_add (GTK_WIDGET (menu));
}

static gboolean on_menu_button_press(GtkWidget* mi, GdkEventButton* evt, MenuCacheItem* data)
{
    menup * m = g_object_get_data(mi , MENUP);
    if( evt->button == 3 )  /* right */
    {
        char* tmp;
        GtkWidget* item;
        GtkMenu* p = GTK_MENU(gtk_menu_new());

        item = gtk_menu_item_new_with_label(_("Add to desktop"));
        g_signal_connect(item, "activate", G_CALLBACK(on_add_menu_item_to_desktop), data);
        gtk_menu_shell_append(GTK_MENU_SHELL(p), item);
        m -> popup = TRUE;
        tmp = g_find_program_in_path("lxshortcut");
        if( tmp )
        {
            item = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(p), item);

            item = gtk_menu_item_new_with_label(_("Properties"));
            g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_properties), data);
            gtk_menu_shell_append(GTK_MENU_SHELL(p), item);
            g_free(tmp);
        }
        g_signal_connect(p, "selection-done", G_CALLBACK(gtk_widget_destroy), NULL);
        g_signal_connect(p, "deactivate", G_CALLBACK(restore_grabs), mi);
        
        gtk_widget_show_all(GTK_WIDGET(p));
        gtk_menu_popup(p, NULL, NULL, NULL, NULL, 0, evt->time);
        return TRUE;
    }
    return FALSE;
}

static GtkWidget* create_item( MenuCacheItem* item, menup * m )
{
    GtkWidget* mi;
    if( menu_cache_item_get_type(item) == MENU_CACHE_TYPE_SEP )
        mi = gtk_separator_menu_item_new();
    else
    {
        GtkWidget* img;
        mi = gtk_image_menu_item_new_with_label( menu_cache_item_get_name(item) );
        img = gtk_image_new();
        gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(mi), img );
        if( menu_cache_item_get_type(item) == MENU_CACHE_TYPE_APP )
        {
            gtk_widget_set_tooltip_text( mi, menu_cache_item_get_comment(item) );
            g_signal_connect( mi, "activate", G_CALLBACK(on_menu_item), item );
        }
        g_signal_connect(mi, "map", G_CALLBACK(on_menu_item_map), item);
        g_signal_connect(mi, "style-set", G_CALLBACK(on_menu_item_style_set), item);
        g_signal_connect(mi, "button-press-event", G_CALLBACK(on_menu_button_press), item);
    }
    gtk_widget_show( mi );
    g_object_set_qdata_full( G_OBJECT(mi), SYS_MENU_ITEM_ID, menu_cache_item_ref(item), (GDestroyNotify) menu_cache_item_unref );
    g_object_set_data( G_OBJECT(mi), MENUP , m );
    return mi;
}

static void load_menu(menup * m , MenuCacheDir* dir, GtkWidget* menu, int pos )
{
    GSList * l;
    for( l = menu_cache_dir_get_children(dir); l; l = l->next )
    {
        MenuCacheItem* item = MENU_CACHE_ITEM(l->data);
        if ((menu_cache_item_get_type(item) != MENU_CACHE_TYPE_APP)
        || (panel_menu_item_evaluate_visibility(item)))
        {
            GtkWidget * mi = create_item(item , m );
            if (mi != NULL)
            {
                gtk_menu_shell_insert( (GtkMenuShell*)menu, mi, pos );
                if( pos >= 0 )
                    ++pos;
                if (menu_cache_item_get_type(item) == MENU_CACHE_TYPE_DIR)
                {
                    GtkWidget* sub = gtk_menu_new();
                    load_menu( m , MENU_CACHE_DIR(item), sub, -1 );    /* always pass -1 for position */
                    gtk_menu_item_set_submenu( GTK_MENU_ITEM(mi), sub );
                }
            }
        }
    }
}

static gboolean sys_menu_item_has_data( GtkMenuItem* item )
{
    return (g_object_get_qdata( G_OBJECT(item), SYS_MENU_ITEM_ID ) != NULL);
}

static void unload_old_icons(GtkMenu* menu, GtkIconTheme* theme)
{
    GList *children, *child;
    GtkMenuItem* item;
    GtkWidget* sub_menu=NULL;

    children = gtk_container_get_children( GTK_CONTAINER(menu) );
    for( child = children; child; child = child->next )
    {
        item = GTK_MENU_ITEM( child->data );
        if( sys_menu_item_has_data( item ) )
        {
            GtkImage* img;
            item = GTK_MENU_ITEM( child->data );
            if( GTK_IS_IMAGE_MENU_ITEM(item) )
            {
	        img = GTK_IMAGE(gtk_image_menu_item_get_image(GTK_IMAGE_MENU_ITEM(item)));
                gtk_image_clear(img);
                if( GTK_WIDGET_MAPPED(img) )
		    on_menu_item_map(GTK_WIDGET(item),
			(MenuCacheItem*)g_object_get_qdata(G_OBJECT(item), SYS_MENU_ITEM_ID) );
            }
        }
        else if( ( sub_menu = gtk_menu_item_get_submenu( item ) ) )
        {
	    unload_old_icons( GTK_MENU(sub_menu), theme );
        }
    }
    g_list_free( children );
}

static void remove_change_handler(gpointer id, GObject* menu)
{
    g_signal_handler_disconnect(gtk_icon_theme_get_default(), GPOINTER_TO_INT(id));
}

static void sys_menu_insert_items( menup* m, GtkMenu* menu, int position )
{
    MenuCacheDir* dir;
    guint change_handler;

    if( G_UNLIKELY( SYS_MENU_ITEM_ID == 0 ) )
        SYS_MENU_ITEM_ID = g_quark_from_static_string( "SysMenuItem" );

    dir = menu_cache_get_root_dir( m->menu_cache );
    load_menu( m , dir, GTK_WIDGET(menu), position );
    change_handler = g_signal_connect_swapped( gtk_icon_theme_get_default(), "changed", G_CALLBACK(unload_old_icons), menu );
    g_object_weak_ref( G_OBJECT(menu), remove_change_handler, GINT_TO_POINTER(change_handler) );
}

static void
reload_system_menu( menup* m, GtkMenu* menu )
{
    GList *children, *child;
    GtkMenuItem* item;
    GtkWidget* sub_menu;
    gint idx;

    children = gtk_container_get_children( GTK_CONTAINER(menu) );
    for( child = children, idx = 0; child; child = child->next, ++idx )
    {
        item = GTK_MENU_ITEM( child->data );
        if( sys_menu_item_has_data( item ) )
        {
            do
            {
                item = GTK_MENU_ITEM( child->data );
                child = child->next;
                gtk_widget_destroy( GTK_WIDGET(item) );
            }while( child && sys_menu_item_has_data( child->data ) );
            sys_menu_insert_items( m, menu, idx );
            if( ! child )
                break;
        }
        else if( ( sub_menu = gtk_menu_item_get_submenu( item ) ) )
        {
            reload_system_menu( m, GTK_MENU(sub_menu) );
        }
    }
    g_list_free( children );
}

static gboolean
my_button_pressed(GtkWidget *widget, GdkEventButton *event, Plugin* p)
{
    ENTER;
    menup* m = (menup*)p->priv;
    /* Standard right-click handling. */
    if (plugin_button_press_event(widget, event, p))
        return TRUE;

    if ((event->type == GDK_BUTTON_PRESS)
          && (event->x >=0 && event->x < widget->allocation.width)
          && (event->y >=0 && event->y < widget->allocation.height)
          &&  m -> show == FALSE)
    {
        m -> show = TRUE;
        gtk_window_set_position(GTK_WINDOW(m -> win), GTK_WIN_POS_MOUSE);
        gtk_widget_show_all( m -> win );
    }
    else
    {
        m -> show = FALSE;
        gtk_widget_hide_all( m -> win );
    }
    RET(TRUE);
}

static GtkWidget *
read_separator(Plugin *p, char **fp)
{
    line s;

    ENTER;
    s.len = 256;
    if( fp )
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            ERR("menu: error - separator can not have paramteres\n");
            RET(NULL);
        }
    }
    RET(gtk_separator_menu_item_new());
}

static void on_reload_menu( MenuCache* cache, menup* m )
{
    /* g_debug("reload system menu!!"); */
    reload_system_menu( m, GTK_MENU(m->menu) );
}

static void
read_system_menu(GtkMenu* menu, Plugin *p, char** fp)
{
    line s;
    menup *m = (menup *)p->priv;

    if (m->menu_cache == NULL)
    {
        m->menu_cache = panel_menu_cache_new();
        if (m->menu_cache == NULL)
        {
            ERR("error loading applications menu");
            return;
        }
        m->reload_notify = menu_cache_add_reload_notify(m->menu_cache, (GFunc) on_reload_menu, m);
    }

    s.len = 256;
    if( fp )
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            ERR("menu: error - system can not have paramteres\n");
            return;
        }
    }

    sys_menu_insert_items( m, menu, -1 );
    m->has_system_menu = TRUE;

    p->panel->system_menus = g_slist_append( p->panel->system_menus, p );
}

static void
read_include(Plugin *p, char **fp)
{
    ENTER;
#if 0
    gchar *name;
    line s;
    menup *m = (menup *)p->priv;
    /* FIXME: this is disabled */
    ENTER;
    s.len = 256;
    name = NULL;
    if( fp )
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            if (s.type == LINE_VAR) {
                if (!g_ascii_strcasecmp(s.t[0], "name"))
                    name = expand_tilda(s.t[1]);
                else  {
                    ERR( "menu/include: unknown var %s\n", s.t[0]);
                    RET();
                }
            }
        }
    }
    if ((fp = fopen(name, "r"))) {
        LOG(LOG_INFO, "Including %s\n", name);
        m->files = g_slist_prepend(m->files, fp);
        p->fp = fp;
    } else {
        ERR("Can't include %s\n", name);
    }
    if (name) g_free(name);
#endif
    RET();
}

static void on_menu_realized( GtkWidget * dlg, gpointer data )
{
    GdkPixbuf *pixbuf, *spixbuf;
    Window gdk_win;
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    guint32 val;
    pixbuf = gdk_pixbuf_new_from_file(DEFAULT_BG,
                                      NULL);
    gtk_window_get_size( GTK_WINDOW(dlg), &width, &height );
    spixbuf = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_TILES);
    gdk_pixbuf_render_pixmap_and_mask (spixbuf, &pixmap, &mask, 0);
    gdk_win = GDK_WINDOW_XWINDOW(GTK_WIDGET(dlg)->window);
    val = WIN_HINTS_SKIP_FOCUS;
    XChangeProperty(GDK_DISPLAY(), gdk_win,
          XInternAtom(GDK_DISPLAY(), "_WIN_HINTS", False), XA_CARDINAL, 32,
          PropModeReplace, (unsigned char *) &val, 1);
    gtk_widget_set_app_paintable(dlg, TRUE );
    gdk_window_set_back_pixmap(GTK_WIDGET(dlg) -> window, pixmap, FALSE);
}

static int read_button( Plugin *p , char** fp , GtkWidget * box)
{
    menup * m = (menup *)p->priv;
    gchar *fname;
    GtkWidget *button;
    line s;
    int w, h;
    btn_t * btn;

    ENTER;
    btn = g_slice_new0( btn_t );
    btn -> plugin = p;

    s.len = 256;
    fname= NULL;

    if( fp )
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            if (s.type == LINE_NONE) {
                ERR( "Menu II: illegal token %s\n", s.str);
                RET(0);
            }
            if (s.type == LINE_VAR)
            {
                if( !g_ascii_strcasecmp(s.t[0], "id") )
                    btn->desktop_id = g_strdup(s.t[1]);
                else if (!g_ascii_strcasecmp(s.t[0], "image"))
                {
                    btn->customize_image = 1;
                    btn->image = g_strdup(s.t[1]);
                    fname = expand_tilda(s.t[1]);
                }
                else if (!g_ascii_strcasecmp(s.t[0], "tooltip"))
                {
                    btn->customize_tooltip = 1;
                    btn->tooltip = g_strdup(s.t[1]);
                }
                else if (!g_ascii_strcasecmp(s.t[0], "action"))
                {
                    btn->customize_action = 1;
                    btn->action = g_strdup(s.t[1]);
                }
                else
                {
                    ERR( "Menu II: unknown var %s\n", s.t[0]);
                    goto error;

                }
                DBG("action=%s\n", action);
            } else {
                ERR( "Menu II: illegal in this context %s\n", s.str);
                goto error;
            }
        }
    }

    if( btn->desktop_id )
    {
        gchar *desktop_file = NULL;
        gchar *full_id = NULL;
        GKeyFile* desktop = g_key_file_new();
	gboolean loaded;
	
	if ( g_path_is_absolute( btn->desktop_id ) ) 
	{
	    desktop_file = g_strdup( btn->desktop_id );
	    loaded =  g_key_file_load_from_file( desktop, desktop_file,
						 G_KEY_FILE_NONE, NULL );
	}
	else 
	{
	    full_id = g_strconcat( "applications/", btn->desktop_id, NULL );
	    loaded = g_key_file_load_from_data_dirs( desktop, full_id, &desktop_file,
						     G_KEY_FILE_NONE, NULL );
	    g_free( full_id );
	}
    
	/* key file located */
	if ( loaded )
        {
            gchar *icon = NULL, *title = NULL;
            icon = g_key_file_get_string( desktop, desktop_ent, "Icon", NULL);
            title = g_key_file_get_locale_string( desktop, desktop_ent,
                                                "Name", NULL, NULL);
            if( !fname && icon )
                fname = icon;

            if( ! btn->customize_action )
            {
                gchar* exec;
                exec = g_key_file_get_string( desktop, desktop_ent, "Exec", NULL);
                btn->action = translate_exec_to_cmd( exec, icon, title, desktop_file );
                g_free( exec );
            }

            btn->use_terminal = g_key_file_get_boolean(desktop, desktop_ent, "Terminal", NULL);

            if( ! btn->customize_tooltip )
                btn->tooltip = title;
            if( fname != icon )
                g_free( icon );
            if( btn->tooltip != title )
                g_free( title );
        }
        g_free( desktop_file );
        g_key_file_free( desktop );
    }

    // button
    if (p->panel->orientation == ORIENT_HORIZ) {
        h = p->panel->ah;
        w = h;
    } else {
        w = p->panel->aw;
        h = w;
    }
    button = menu_button_new( btn, fname ,  btn->tooltip , 
                               btn -> action , GTK_ICON_SIZE_LARGE_TOOLBAR,
						       FALSE );
    btn->widget = button;

    GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);

    // DnD support
    gtk_drag_dest_set (GTK_WIDGET(button),
          GTK_DEST_DEFAULT_ALL, //GTK_DEST_DEFAULT_HIGHLIGHT,
          target_table, G_N_ELEMENTS (target_table),
          GDK_ACTION_COPY);

    gtk_box_pack_start(GTK_BOX( box ), button, FALSE, TRUE, 0);

    /* append is more time-consuming, but we really care about the order. */
    m -> favorites = g_slist_append( m -> favorites, btn );

    gtk_widget_show(button);
    plugin_widget_set_background( button, p->panel );

    g_free(fname);

    /* tooltip */
    if ( btn->tooltip ) {
        gtk_widget_set_tooltip_text(button, btn->tooltip);
    }
    RET(1);

error:
    g_free(fname);
    //btn_free( btn );
    RET(0);
}

static void
menu_win_constructor( Plugin *p, char **fp )
{
    GtkWidget * menubar,
              * mi,
              * image,
              * sw, *vbox, *thbox, *tvbox;
    line s;

    menup *m = (menup *)p->priv;
    m-> show = FALSE;
    m-> popup = FALSE;
    static char rc_string[] = 
        "style \"menu-buttons\"\n"
        "{\n"
            "font_name = \"Sans Bold 9\"\n"
            "GtkButton::default_border                    = {0, 0, 0, 0}\n"
            "GtkButton::default_outside_border            = {0, 0, 0, 0}\n"
            "GtkButton::child_displacement_x              = 0\n"
            "GtkButton::child_displacement_y              = 1\n"
            "GtkButton::default_spacing                   = 4\n"
            "GtkButton::focus-padding                     = 0\n"
            "GtkWidget::focus-line-width                  = 1\n"
            "GtkWidget::focus_padding                     = 1\n"
            "GtkWidget::interior_focus                    = 1\n"
            "GtkWidget::internal_padding                  = 2\n" 
            "xthickness        = 1\n"
            "ythickness        = 1\n"
            "base[ACTIVE]      = \"#2f519a\"\n"
            "base[INSENSITIVE] = \"#303030\"\n"
            "base[NORMAL]      = \"#121212\"\n"
            "base[PRELIGHT]    = \"#002849\"\n"
            "base[SELECTED]    = \"#003263\"\n"
            "bg[ACTIVE]        = \"#151515\"\n"
            "bg[INSENSITIVE]   = \"#303030\"\n"
            "bg[NORMAL]        = \"#232323\"\n"
            "bg[PRELIGHT]      = \"#003263\"\n"
            "bg[SELECTED]      = \"#002849\"\n"
            "fg[ACTIVE]        = \"#dadada\"\n"
            "fg[INSENSITIVE]   = \"#151515\"\n"
            "fg[NORMAL]        = \"#ffffff\"\n"
            "fg[PRELIGHT]      = \"#fff\"\n"
            "fg[SELECTED]      = \"#fff\"\n"
            "text[ACTIVE]      = \"#fff\"\n"
            "text[INSENSITIVE] = \"#fff\"\n"
            "text[NORMAL]      = \"#fff\"\n"
            "text[PRELIGHT]    = \"#fff\"\n"
            "text[SELECTED]    = \"#fff\"\n"
    "}\n"
    "widget \"*GMenuButton*\" style \"menu-buttons\"\n"
    "style \"app-menu\"\n"
    "{\n"
	    "bg_pixmap[NORMAL] = \"<parent>\"\n"
    	"bg_pixmap[INSENSITIVE] = \"<parent>\"\n"
	    "bg_pixmap[PRELIGHT] = \"<parent>\"\n"
    	"bg_pixmap[SELECTED] = \"<parent>\"\n"
	    "bg_pixmap[ACTIVE] = \"<parent>\"\n"
    "}\n"
    "widget \"*AppMenu*\" style \"app-menu\"\n";
    
    gtk_rc_parse_string(rc_string);

    /* Menu button */
    m->img = gtk_image_new();
    g_file_test( DEFAULT_MENU_ICON, G_FILE_TEST_EXISTS ) ?
    gtk_image_set_from_file((GtkImage*)m->img, DEFAULT_MENU_ICON ):
    gtk_image_set_from_icon_name((GtkImage*)m->img,
                                 "gtk-index",
                                 GTK_ICON_SIZE_SMALL_TOOLBAR); 

    /* All app menu */   
    m->menu = gtk_menu_new();
    read_system_menu( m->menu , p , NULL );
    menubar = gtk_menu_bar_new ();
    image = gtk_image_new_from_icon_name(GTK_STOCK_INDEX, GTK_ICON_SIZE_LARGE_TOOLBAR);
    mi = gtk_image_menu_item_new_with_label ( _("All Programs") );
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(mi),image);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi),m->menu);
    gtk_container_set_border_width (GTK_CONTAINER (mi), 6);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), mi);
    g_object_set (menubar,"pack-direction",GTK_PACK_DIRECTION_BTT,NULL);
    gtk_widget_set_name( menubar, "AppMenu" );

    /* Main Window */
    m -> win =  gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW( m -> win), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER( m -> win), 6);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW( m -> win), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW( m -> win), TRUE);
    gtk_window_set_type_hint(GTK_WINDOW( m -> win), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_signal_connect ( G_OBJECT(m -> win), "style-set", on_menu_realized , NULL );
    g_signal_connect ( G_OBJECT(m -> win), "show", on_menu_realized , NULL );
    g_signal_connect (G_OBJECT (m -> win), "focus_out_event", G_CALLBACK (focus_out_event), m);

    /* Favorites Menu */
    m -> favorite_box = gtk_vbox_new(FALSE,6);
    if( fp )
    {
        s.len = 256;
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END) {
            if (s.type == LINE_NONE) {
                ERR( "Menu II: illegal token %s\n", s.str);
                goto error;
            }
            if (s.type == LINE_BLOCK_START) {
                if (!g_ascii_strcasecmp(s.t[0], "button")) {
                    if (!read_button(p, fp , m -> favorite_box )) {
                        ERR( "Menu II: can't init button\n");
                        goto error;
                    }
                } else {
                    ERR( "Menu II: unknown var %s\n", s.t[0]);
                    goto error;
                }
            } else {
                ERR( "Menu II: illegal in this context %s\n", s.str);
                goto error;
            }
        }
    }
    
    /* Static Icons */
    /********************************************************************************************/
    thbox = gtk_hbox_new(FALSE,6);
    tvbox = gtk_vbox_new(FALSE,6);
    vbox  = gtk_vbox_new(FALSE,6);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					  GTK_POLICY_AUTOMATIC,
					  GTK_POLICY_AUTOMATIC);

    gtk_container_set_border_width (GTK_CONTAINER (vbox),6);
    gtk_scrolled_window_add_with_viewport( sw,  m -> favorite_box );

    gtk_box_pack_start (GTK_BOX (tvbox), sw , FALSE, FALSE,0);
    gtk_box_pack_end   (GTK_BOX (tvbox), menubar , FALSE, FALSE,0);

    gtk_box_pack_start (GTK_BOX (thbox),tvbox, FALSE, FALSE,1);
    gtk_box_pack_end   (GTK_BOX (thbox), create_right_menubar(p) , FALSE, FALSE,3);
    gtk_box_pack_end   (GTK_BOX (vbox), thbox, TRUE, TRUE, 0);
    gtk_container_add  (GTK_CONTAINER (  m -> win ), vbox);

    gtk_widget_set_size_request( m -> win , 412 , 400 );
    gtk_widget_set_size_request( sw , 200 , 335 );
    
error:
/*  menu_destructor(p); */
    return;
}

static int
menu_constructor(Plugin *p, char **fp)
{
    menup *m;
    int iw, ih;
    m = g_new0(menup, 1);
    g_return_val_if_fail(m != NULL, 0);
    m->fname = NULL;
    m->caption = NULL;
    p->priv = m;
    m->box = gtk_event_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(m->box), 0);
    menu_win_constructor( p, fp );
    gtk_container_add(GTK_CONTAINER( m->box), m->img );
    g_signal_connect(G_OBJECT(m->box), "button-press-event",
                         G_CALLBACK(my_button_pressed), p);
    gtk_widget_show_all( m->box );
    p->pwid = m->box;
    RET(1);
}

/* Handler for "drag-data-received" event from menu button. */
static void btn_drag_data_received_event(
    GtkWidget * widget,
    GdkDragContext * context,
    gint x,
    gint y,
    GtkSelectionData * sd,
    guint info,
    guint time,
    btn_t * b)
{
    if (sd->length > 0)
    {
        if (info == TARGET_URILIST)
        {
            gchar * s = (gchar *) sd->data;
            gchar * end = s + sd->length;
            gchar * str = g_strdup(b->action);
            while (s < end)
            {
                while (s < end && g_ascii_isspace(*s))
                    s++;
                gchar * e = s;
                while (e < end && !g_ascii_isspace(*e))
                    e++;
                if (s != e)
                {
                    *e = 0;
                    s = g_filename_from_uri(s, NULL, NULL);
                    if (s)
                    {
                        gchar * tmp = g_strconcat(str, " '", s, "'", NULL);
                        g_free(str);
                        g_free(s);
                        str = tmp;
                    }
                }
                s = e+1;
            }

            g_spawn_command_line_async(str, NULL);
            g_free(str);
        }
    }
}

static void btn_build_gui(Plugin * p, btn_t * btn)
{
    menup * menu = (menup *) p->priv;

    if (btn->desktop_id != NULL)
    {
        /* There is a valid desktop file name.  Try to open it. */
        GKeyFile * desktop = g_key_file_new();
        
	gchar * desktop_file = NULL;
        gboolean loaded;	
	if (g_path_is_absolute(btn->desktop_id))
        {
            desktop_file = g_strdup(btn->desktop_id);
            loaded = g_key_file_load_from_file(desktop, desktop_file, G_KEY_FILE_NONE, NULL );
	}
	else 
	{
            /* Load from the freedesktop.org specified data directories. */
            gchar * full_id = g_strconcat("applications/", btn->desktop_id, NULL);
            loaded = g_key_file_load_from_data_dirs(
                desktop, full_id, &desktop_file, G_KEY_FILE_NONE, NULL);
            g_free(full_id);
        }

	if (loaded)
        {
            /* Desktop file located.  Get Icon, Name, Exec, and Terminal parameters. */
            gchar * icon = g_key_file_get_string(desktop, desktop_ent, "Icon", NULL);
            gchar * title = g_key_file_get_locale_string(desktop, desktop_ent, "Name", NULL, NULL);
            if ((btn->image == NULL) && (icon != NULL))
                btn->image = icon;

            if ( ! btn->customize_action )
            {
                gchar * exec = g_key_file_get_string(desktop, desktop_ent, "Exec", NULL);
                btn->action = translate_exec_to_cmd(exec, icon, title, desktop_file);
                g_free(exec);
            }

            btn->use_terminal = g_key_file_get_boolean(desktop, desktop_ent, "Terminal", NULL);

            if ( ! btn->customize_tooltip)
                btn->tooltip = title;
            if (btn->image != icon)
                g_free(icon);
            if (btn->tooltip != title)
                g_free(title);
        }

        g_free(desktop_file);
        g_key_file_free(desktop);
    }

    /* Create a button with the specified icon. */
    GtkWidget * button = menu_button_new ( btn , btn -> image, 
                                            btn -> tooltip, btn -> action, 
                                            GTK_ICON_SIZE_LARGE_TOOLBAR , FALSE);
    btn->widget = button;

    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
    if (btn->tooltip != NULL)
        gtk_widget_set_tooltip_text(button, btn->tooltip);

    /* Add the button to the icon grid. */
    // icon_grid_add(lb->icon_grid, button, TRUE);

    /* Drag and drop support. */
    gtk_drag_dest_set(GTK_WIDGET(button),
        GTK_DEST_DEFAULT_ALL,
        target_table, G_N_ELEMENTS(target_table),
        GDK_ACTION_COPY);

    /* Connect signals. */
    g_signal_connect(button, "button-press-event", G_CALLBACK(btn_press_event), (gpointer) btn);
    g_signal_connect(button, "drag_data_received", G_CALLBACK(btn_drag_data_received_event), (gpointer) btn);

    /* Append at end of list to preserve configured order. */
    menu -> favorites = g_slist_append( menu -> favorites, btn);

    /* Show the widget and return. */
    gtk_widget_show(button);
    plugin_widget_set_background(button, p->panel);
}

static void menu_configure_add_button(GtkButton * widget, Plugin * p)
{
    menup * menu = (menup *) p->priv;
    GtkTreeView * menu_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(menu->config_dlg), "menu_view"));
    GtkTreeView * defined_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(menu->config_dlg), "defined_view"));
    GtkTreeModel * list;
    GtkTreeIter it;
    if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(menu_view), &list, &it))
    {
        btn_t * btn;
        gtk_tree_model_get(list, &it, COL_BTN, &btn, -1);

        /* We have located a selected button.
         * Add a launch button to the menu and refresh the view in the configuration dialog. */
        btn_t * defined_button = g_new0(btn_t, 1);
        defined_button->plugin = p;
        defined_button->desktop_id = g_strdup(btn->desktop_id);
        btn_build_gui(p, defined_button);
        gtk_box_pack_start(GTK_BOX(menu->favorite_box), defined_button-> widget, FALSE, TRUE, 0);
        GtkListStore * list = GTK_LIST_STORE(gtk_tree_view_get_model(defined_view));
        GtkTreeIter it;
        gtk_list_store_append(list, &it);
        gtk_list_store_set(list, &it,
            COL_ICON, lxpanel_load_icon(btn->image, PANEL_ICON_SIZE, PANEL_ICON_SIZE, TRUE),
            COL_TITLE, ((btn->tooltip != NULL) ? btn->tooltip : btn->action),
            COL_BTN, defined_button,
            -1);
    }
}

static void menu_configure_remove_button(GtkButton * widget, Plugin * p)
{
    menup * menu = (menup *) p->priv;
    GtkTreeView * defined_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(menu->config_dlg), "defined_view"));
    GtkTreeModel * list;
    GtkTreeIter it;
    if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(defined_view), &list, &it))
    {
        btn_t * btn;
        gtk_tree_model_get(list, &it, COL_BTN, &btn, -1);

        /* We have found a selected button.
         * Remove it from the icon grid, the data structure, and the view. */
        gtk_list_store_remove(GTK_LIST_STORE(list), &it);
        menu -> favorites = g_slist_remove( menu -> favorites, btn);
        gtk_widget_destroy( btn->widget );
    }
}

static void menu_configure_move_up_button(GtkButton * widget, Plugin * p)
{
    menup * menu = (menup *) p->priv;

    GtkTreeView * defined_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(menu->config_dlg), "defined_view"));
    GtkTreeModel * list;
    GtkTreeIter it;
    if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(defined_view), &list, &it))
    {
        btn_t *btn;
        gtk_tree_model_get(GTK_TREE_MODEL(list), &it, COL_BTN, &btn, -1);
        GtkTreePath * path = gtk_tree_model_get_path(GTK_TREE_MODEL(list), &it);
        if ((gtk_tree_path_get_indices(path)[0] > 0)
        && (gtk_tree_path_prev(path)))
        {
            GtkTreeIter it2;
            if (gtk_tree_model_get_iter(list, &it2, path))
            {
                /* We have found a selected button that can be moved.
                 * Reorder it in the icon grid, the data structure, and the view. */
                int i = gtk_tree_path_get_indices(path)[0];
                menu->favorites = g_slist_remove(menu->favorites, btn);
                menu->favorites = g_slist_insert(menu->favorites, btn, i);
                gtk_list_store_move_before(GTK_LIST_STORE(list), &it, &it2);
                gtk_box_reorder_child( menu->favorite_box, btn->widget, i );
            }
        }
        gtk_tree_path_free(path);
    }
}

/* Handler for "clicked" action on menu configuration dialog "Move Down" button. */
static void menu_configure_move_down_button(GtkButton * widget, Plugin * p)
{
    menup * menu = (menup *) p->priv;

    GtkTreeView * defined_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(menu->config_dlg), "defined_view"));
    GtkTreeModel * list;
    GtkTreeIter it;
    if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(defined_view), &list, &it))
    {
        btn_t *btn;
        gtk_tree_model_get(GTK_TREE_MODEL(list), &it, COL_BTN, &btn, -1);
        GtkTreePath * path = gtk_tree_model_get_path(GTK_TREE_MODEL(list), &it);
        int n = gtk_tree_model_iter_n_children(list, NULL);
        if (gtk_tree_path_get_indices(path)[0] < (n - 1))
        {
            gtk_tree_path_next(path);
            GtkTreeIter it2;
            if (gtk_tree_model_get_iter( list, &it2, path))
            {
                /* We have found a selected button that can be moved.
                 * Reorder it in the icon grid, the data structure, and the view. */
                int i = gtk_tree_path_get_indices(path)[0];
                menu->favorites = g_slist_remove(menu->favorites, btn);
                menu->favorites = g_slist_insert(menu->favorites, btn, i + 1);
                gtk_list_store_move_after(GTK_LIST_STORE(list), &it, &it2);
                gtk_box_reorder_child( menu->favorite_box, btn->widget, i );
            }
        }
        gtk_tree_path_free(path);
    }
}

static void menu_configure_response(GtkDialog * dlg, int response, Plugin * p)
{
    menup * menu = (menup *) p->priv;

    /* Deallocate btn_ts that were loaded from the menu. */
    GtkTreeView * menu_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT( menu -> config_dlg), "menu_view"));
    GtkTreeModel * model = gtk_tree_view_get_model(menu_view);
    GtkTreeIter it;
    if (gtk_tree_model_get_iter_first(model, &it))
    {
        do
        {
            btn_t * btn;
            gtk_tree_model_get(model, &it, COL_BTN, &btn, -1);
            btn_free(btn);           
        }
        while (gtk_tree_model_iter_next(model, &it));
    }

    /* Deallocate the configuration dialog. */
    menu -> config_dlg = NULL;
    gtk_widget_destroy(GTK_WIDGET(dlg));
}

static void menu_configure_add_menu_recursive(GtkListStore * list, MenuCacheDir * menu_dir)
{
    /* Iterate over all menu items in this directory. */
    GSList * l;
    for (l = menu_cache_dir_get_children(menu_dir); l != NULL; l = l->next)
    {
        /* Get the next menu item. */
        MenuCacheItem * item = MENU_CACHE_ITEM(l->data);
        switch (menu_cache_item_get_type(item))
        {
            case MENU_CACHE_TYPE_NONE:
            case MENU_CACHE_TYPE_SEP:
                break;

            case MENU_CACHE_TYPE_APP:
                {
                /* If an application, build a btn_t data structure so we can identify
                 * the button in the handler.  In this application, the desktop_id is the
                 * fully qualified desktop file path.  The image and tooltip are what is displayed in the view. */
                btn_t * btn = g_new0(btn_t, 1);
                btn->desktop_id = g_strdup(menu_cache_item_get_file_path(item));
                btn->image = g_strdup(menu_cache_item_get_icon(item));
                btn->tooltip = g_strdup(menu_cache_item_get_name(item));

                /* Add the row to the view. */
                GtkTreeIter it;
                gtk_list_store_append(list, &it);
                gtk_list_store_set(list, &it,
                    COL_ICON, lxpanel_load_icon(btn->image, PANEL_ICON_SIZE, PANEL_ICON_SIZE, TRUE),
                    COL_TITLE, ((btn->tooltip != NULL) ? btn->tooltip : btn->desktop_id),
                    COL_BTN, btn,
                    -1);
                }
                break;

            case MENU_CACHE_TYPE_DIR:
                /* If a directory, recursively add its menu items. */
                menu_configure_add_menu_recursive(list, MENU_CACHE_DIR(item));
                break;
        }

    }
}

static void menu_configure_initialize_list(Plugin * p, GtkWidget * dlg, GtkTreeView * view, gboolean from_menu)
{
    menup * menu = (menup *) p->priv;

    /* Set the selection mode. */
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(view), GTK_SELECTION_BROWSE);

    /* Establish the column data types. */
    GtkListStore * list = gtk_list_store_new(N_COLS,
        GDK_TYPE_PIXBUF,
        G_TYPE_STRING,
        G_TYPE_POINTER);

    /* Define a column. */
    GtkTreeViewColumn * col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, ((from_menu) ? _("Available Applications") : _("Applications")));

    /* Establish the pixbuf column cell renderer. */
    GtkCellRenderer * render = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(col, render, FALSE);
    gtk_tree_view_column_set_attributes(col, render, "pixbuf", COL_ICON, NULL);

    /* Establish the text column cell renderer. */
    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", COL_TITLE);

    /* Append the column to the view. */
    gtk_tree_view_append_column(view, col);

    if (from_menu)
    {
        /* Initialize from all menu items. */
        MenuCache * menu_cache = panel_menu_cache_new();
        if (menu_cache != NULL)
        {
            MenuCacheDir * dir = menu_cache_get_root_dir(menu_cache);
            menu_configure_add_menu_recursive(list, dir);
            menu_cache_unref(menu_cache);
        }
        g_object_set_data(G_OBJECT(dlg), "menu_view", view);
    }
    else
    {
        /* Initialize from defined menu buttons. */
        GSList * l;
        for (l = menu->favorites; l != NULL; l = l->next)
        {
            btn_t * btn = (btn_t *) l->data;
            GtkTreeIter it;
            gtk_list_store_append(list, &it);
            gtk_list_store_set(list, &it,
                COL_ICON, lxpanel_load_icon(btn->image, PANEL_ICON_SIZE, PANEL_ICON_SIZE, TRUE),
                COL_TITLE, ((btn->tooltip != NULL) ? btn->tooltip : btn->action),
                COL_BTN, btn,
                -1);
        }
        g_object_set_data(G_OBJECT(dlg), "defined_view", view);
    }

    /* Finish the setup and return. */
    gtk_tree_view_set_model(view, GTK_TREE_MODEL(list));
}

static void menu_config( Plugin *p, GtkWindow* parent )
{
    menup * menu = (menup *) p->priv;

    if (menu->config_dlg == NULL)
    {
        /* Create the configuration dialog. */
        GtkWidget * dlg = gtk_dialog_new_with_buttons(
            _(p->class->name),
            parent,
            0,
            GTK_STOCK_CLOSE,
            GTK_RESPONSE_CLOSE,
            NULL);
        gtk_window_set_default_size(GTK_WINDOW(dlg), 640, 400);
        panel_apply_icon(GTK_WINDOW(dlg));

        /* Create a horizontal box. */
        GtkWidget * hbox = gtk_hbox_new(FALSE, 4);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), hbox, TRUE, TRUE, 2);

        /* Create a scrollbar as the child of the horizontal box. */
        GtkWidget * defined_scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(defined_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(defined_scroll), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX(hbox), defined_scroll, TRUE, TRUE, 2);

        /* Create a tree view as the child of the scrollbar. */
        GtkWidget * defined_view = gtk_tree_view_new();
        gtk_container_add(GTK_CONTAINER(defined_scroll), defined_view);

        /* Create a vertical box as the child of the horizontal box. */
        GtkWidget * vbox = gtk_vbox_new(FALSE, 2);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);

        /* Create a scrollbar as the child of the horizontal box. */
        GtkWidget * menu_scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(menu_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(menu_scroll), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX(hbox), menu_scroll, TRUE, TRUE, 2);

        /* Create a tree view as the child of the scrollbar. */
        GtkWidget * menu_view = gtk_tree_view_new();
        gtk_container_add(GTK_CONTAINER(menu_scroll), menu_view);

        /* Create an "Add" button as the child of the vertical box. */
        GtkWidget * btn = gtk_button_new_from_stock(GTK_STOCK_ADD);
        g_signal_connect(btn, "clicked", G_CALLBACK(menu_configure_add_button), p);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);

        /* Create a "Remove" button as the child of the vertical box. */
        btn = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
        g_signal_connect(btn, "clicked", G_CALLBACK(menu_configure_remove_button), p);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);

        /* Create a "Move Up" button as the child of the vertical box. */
        btn = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
        g_signal_connect(btn, "clicked", G_CALLBACK(menu_configure_move_up_button), p);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);

        /* Create a "Move Down" button as the child of the vertical box. */
        btn = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
        g_signal_connect(btn, "clicked", G_CALLBACK(menu_configure_move_down_button), p);
        gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);

        /* Connect signals. */
        g_signal_connect(dlg, "response", G_CALLBACK(menu_configure_response), p);

        /* Initialize the tree view contents. */
        menu_configure_initialize_list(p, dlg, GTK_TREE_VIEW(defined_view), FALSE);
        menu_configure_initialize_list(p, dlg, GTK_TREE_VIEW(menu_view), TRUE);

        /* Show the dialog. */
        gtk_widget_show_all(dlg);

        /* Establish a callback when the dialog completes. */
        g_object_weak_ref(G_OBJECT(dlg), (GWeakNotify) panel_config_save, p->panel);
        gtk_window_present(GTK_WINDOW(dlg));
        menu -> config_dlg = dlg;
    }
}

static void save_config( Plugin* p, FILE* fp )
{
    menup * menu = (menup *)p->priv;
    GSList* l;
    for( l = menu->favorites; l; l = l->next ) {
        btn_t* btn = (btn_t*)l->data;
        if ( btn->widget != NULL)
        {
            lxpanel_put_line( fp, "Button {" );
            if( btn->desktop_id )
                lxpanel_put_str( fp, "id", btn->desktop_id );
            if( btn->customize_image )
                lxpanel_put_str( fp, "image", btn->image );
            if( btn->customize_tooltip )
                lxpanel_put_str( fp, "tooltip", btn->tooltip );
            if( btn->customize_action )
                lxpanel_put_str( fp, "action", btn->action );
            lxpanel_put_line( fp, "}" );
        }
    } 
}

PluginClass menu2_plugin_class = {
    PLUGINCLASS_VERSIONING, 
    
    type : "menu2",
    name : N_("Menu II"),
    version: "0.2",
    description : N_("Windows OS like menu"),

    constructor : menu_constructor,
    destructor  : menu_destructor,
    config : menu_config,
    save : save_config,
    panel_configuration_changed : NULL
};

