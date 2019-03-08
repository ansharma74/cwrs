/*
 *      lxdm.c - basic ui of lxdm
 *
 *      Copyright 2009 dgod <dgod.osa@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */


#define XLIB_ILLEGAL_ACCESS
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <X11/Xlib.h>

#include <string.h>
#include <poll.h>
#include <grp.h>
#include <unistd.h>
#include <ctype.h>

#include "lxdm.h"

#define MAX_INPUT_CHARS     32
#define MAX_VISIBLE_CHARS   14

static Display *dpy;
static GdkWindow *root, *win;
static PangoLayout *layout;
static char user[MAX_INPUT_CHARS];
static char pass[MAX_INPUT_CHARS];
static int stage;
static GdkRectangle rc;
static GdkColor bg, border, hint, text, msg;

static GSList *sessions;
static int session_select = -1;

static char *message;

static pid_t greeter = -1;
static guint greeter_watch = 0;
static int greeter_pipe[2];
static GIOChannel *greeter_io;
static guint io_id;

static int get_text_layout(char *s, int *w, int *h)
{
    pango_layout_set_text(layout, s, -1);
    pango_layout_get_pixel_size(layout, w, h);
    return 0;
}

static void draw_text(cairo_t *cr, double x, double y, char *text, GdkColor *color)
{
    pango_layout_set_text(layout, text, -1);
    cairo_move_to(cr, x, y);
    gdk_cairo_set_source_color(cr, color);
    pango_cairo_show_layout(cr, layout);
}

static void on_ui_expose(void)
{
    cairo_t *cr = gdk_cairo_create(win);
    char *p = (stage == 0) ? user : pass;
    int len = strlen(p);
    GdkColor *color=&text;

    gdk_cairo_set_source_color(cr, &bg);
    cairo_rectangle(cr, 0, 0, rc.width, rc.height);
    cairo_fill(cr);
    gdk_cairo_set_source_color(cr, &border);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
    cairo_rectangle(cr, 0, 0, rc.width, rc.height);

    if( message )
    {
        color = &msg;
        p = message;
    }
    else if( stage == 0 )
    {
        if( len < MAX_VISIBLE_CHARS )
            p = user;
        else
            p = user + len - MAX_VISIBLE_CHARS;
        if( len == 0 )
        {
            p = "Username";
            color = &hint;
        }
    }
    else if( stage == 1 )
    {
        char spy[MAX_VISIBLE_CHARS + 1];
        p = spy;
        if( len < MAX_VISIBLE_CHARS )
        {
            memset(spy, '*', len);
            p[len] = 0;
        }
        else
        {
            memset(spy, '*', MAX_VISIBLE_CHARS);
            p[MAX_VISIBLE_CHARS] = 0;
        }
        if( len == 0 )
        {
            p = "Password";
            color = &hint;
        }
    }
    draw_text(cr, 3, 3, p, color);
    cairo_destroy(cr);
}

static void on_ui_key(GdkEventKey *event)
{
    char *p;
    int len;
    int key;

    if( stage != 0 && stage != 1 )
        return;
    message = 0;
    key = event->keyval;
    p = (stage == 0) ? user : pass;
    len = strlen(p);
    if( key == GDK_Escape )
    {
        user[0] = 0;
        pass[0] = 0;
        stage = 0;
        session_select = -1;
    }
    else if( key == GDK_BackSpace )
    {
        if( len > 0 )
            p[--len] = 0;
    }
    else if( key == GDK_Return )
    {
        if( stage == 0 && len == 0 )
            return;
        stage++;
        if( stage == 1 )
            if( !strcmp(user, "reboot") || !strcmp(user, "shutdown") )
                stage = 2;
    }
    else if( key == GDK_F1 )
    {
        Session *sess;
        if( !sessions )
        {
            sessions = do_scan_xsessions();
            session_select = 0;
        }
        else
        {
            session_select++;
            if( session_select >= g_slist_length(sessions) )
                session_select = 0;
        }
        sess = g_slist_nth_data(sessions, session_select);
        if( sess ) message = sess->name;
    }
    else if( key >= 0x20 && key <= 0x7e )
        if( len < MAX_INPUT_CHARS - 1 )
        {
            p[len++] = key;
            p[len] = 0;
        }
    on_ui_expose();
    if( stage == 2 )
    {
        ui_do_login();
        if( stage != 2 )
            on_ui_expose();
    }
}

int ui_do_login(void)
{
    struct passwd *pw;
    int ret;

    if( stage != 2 )
        return -1;

    if( !strcmp(user, "reboot") )
        lxdm_do_reboot();
    else if( !strcmp(user, "shutdown") )
        lxdm_do_shutdown();
    ret = lxdm_auth_user(user, pass, &pw);
    if( AUTH_SUCCESS == ret && pw != NULL )
    {
        char *exec = 0;
        if( sessions && session_select > 0 )
        {
            Session *sess;
            sess = g_slist_nth_data(sessions, session_select);
            exec = g_strdup(sess->exec);
            free_xsessions(sessions);
        }
        sessions = 0; session_select = -1;
        ui_drop();
        lxdm_do_login(pw, exec, 0);
        g_free(exec);
        if( lxdm_cur_session() <= 0 )
            ui_prepare();
    }
    else
    {
        user[0] = pass[0] = 0;
        stage = 0;
    }
    return 0;
}

void ui_event_cb(GdkEvent *event, gpointer data)
{
    if( stage == 2 )
        return;
    if( event->type == GDK_KEY_PRESS )
        on_ui_key( (GdkEventKey*)event );
    else if( event->type == GDK_EXPOSE )
        on_ui_expose();
}

void ui_drop(void)
{
    /* drop connect event */
    if( dpy )
        if( win )
        {
            gdk_window_destroy(win);
            win = NULL;
            XUngrabKeyboard(dpy, CurrentTime);
        }

    /* destroy the font */
    if( layout )
    {
        g_object_unref(layout);
        layout = NULL;
    }

    /* if greeter, do quit */
    if( greeter > 0 )
    {
        write(greeter_pipe[0], "exit\n", 5);
        g_source_remove(io_id);
        io_id = 0;
        g_io_channel_unref(greeter_io);
        greeter_io = NULL;
        close(greeter_pipe[1]);
        close(greeter_pipe[0]);
        kill(greeter, SIGTERM);
    }
}

#if 1
void ui_set_bg(void)
{
    char *p;
    GdkWindow *root = gdk_get_default_root_window();
    GdkColor screen;
    GdkPixbuf *bg_img = NULL;

    /* get background */
    p = g_key_file_get_string(config, "display", "bg", 0);
    if( !p ) p = g_strdup("#222E45");
    if( p && p[0] != '#' )
    {
        GdkPixbuf *pb = gdk_pixbuf_new_from_file(p, 0);
        if( !pb )
        {
            g_free(p);
            p = g_strdup("#222E45");
        }
        else
            bg_img = pb;
    }
    if( p && p[0] == '#' )
        gdk_color_parse(p, &screen);
    g_free(p);

    /* set background */
    if( !bg_img )
    {
        GdkColormap *map = gdk_window_get_colormap(root);
        gdk_color_alloc(map, &screen);
        gdk_window_set_background(root, &screen);
    }
    else
    {
        GdkPixmap *pix = NULL;
        p = g_key_file_get_string(config, "display", "bg_style", 0);
        if( !p || !strcmp(p, "stretch") )
        {
            GdkPixbuf *pb = gdk_pixbuf_scale_simple(bg_img,
                                                    gdk_screen_width(),
                                                    gdk_screen_height(),
                                                    GDK_INTERP_HYPER);
            g_object_unref(bg_img);
            bg_img = pb;
        }
        g_free(p);
        gdk_pixbuf_render_pixmap_and_mask(bg_img, &pix, NULL, 0);
        g_object_unref(bg_img);
        /* call x directly, because gdk will ref the pixmap */
        //gdk_window_set_back_pixmap(root,pix,FALSE);
        XSetWindowBackgroundPixmap( GDK_WINDOW_XDISPLAY(root),
                                   GDK_WINDOW_XID(root), GDK_PIXMAP_XID(pix) );
        g_object_unref(pix);
    }
    gdk_window_clear(root);
}
#else
void ui_set_bg(void)
{
    char *p;
    GdkWindow *root = gdk_get_default_root_window();
    GdkColor screen;

    p = g_key_file_get_string(config, "display", "bg", 0);
    if( !p ) p = g_strdup("#222E45");
    if( p && p[0] != '#' )
    {
        char *style = g_key_file_get_string(config, "display", "bg_style", 0);
        GdkPixbuf *pb;
        GdkPixmap *pix = NULL;
        if( !p || !strcmp(p, "stretch") )
            pb = gdk_pixbuf_new_from_file_at_scale(p,
                                                   gdk_screen_width(), gdk_screen_height(), TRUE, NULL);
        else
            pb = gdk_pixbuf_new_from_file(p, 0);
        g_free(style);
        if( pb )
        {
            gdk_pixbuf_render_pixmap_and_mask(pb, &pix, NULL, 0);
            g_object_unref(pb);
            gdk_window_set_back_pixmap(root, pix, FALSE);
            g_object_unref(pix);
        }
    }
    if( p && p[0] == '#' )
    {
        GdkColormap *map = gdk_window_get_colormap(root);
        gdk_color_parse(p, &screen);
        gdk_color_alloc(map, &screen);
        gdk_window_set_background(root, &screen);
    }
    g_free(p);
    gdk_window_clear(root);
}
#endif

static void greeter_setup(gpointer user)
{
    struct passwd *pw;
    if( AUTH_SUCCESS == lxdm_auth_user("lxdm", NULL, &pw) )
    {
        initgroups(pw->pw_name, pw->pw_gid);
        setgid(pw->pw_gid);
        setuid(pw->pw_uid);
    }
}

static gchar *greeter_param(char *str, char *name)
{
    char *temp, *p;
    char ret[128];
    int i;
    temp = g_strdup_printf(" %s=", name);
    p = strstr(str, temp);
    if( !p )
    {
        g_free(temp);
        return NULL;
    }
    p += strlen(temp);
    g_free(temp);
    for( i = 0; i < 127; i++ )
    {
        if( !p[i] || isspace(p[i]) )
            break;
        ret[i] = p[i];
    }
    ret[i] = 0;
    return g_strdup(ret);
}

static gboolean on_greeter_input(GIOChannel *source, GIOCondition condition, gpointer data)
{
    GIOStatus ret;
    char *str;

    if( !(G_IO_IN & condition) )
        return FALSE;
    ret = g_io_channel_read_line(source, &str, NULL, NULL, NULL);
    if( ret != G_IO_STATUS_NORMAL )
        return FALSE;

    if( !strncmp(str, "reboot", 6) )
        lxdm_do_reboot();
    else if( !strncmp(str, "shutdown", 6) )
        lxdm_do_shutdown();
    else if( !strncmp(str, "log ", 4) )
        log_print(str + 4);
    else if( !strncmp(str, "login ", 6) )
    {
        char *user = greeter_param(str, "user");
        char *pass = greeter_param(str, "pass");
        char *session = greeter_param(str, "session");
        char *lang = greeter_param(str, "lang");
        if( user && pass )
        {
            struct passwd *pw;
            int ret = lxdm_auth_user(user, pass, &pw);
            if( AUTH_SUCCESS == ret && pw != NULL )
            {
                ui_drop();
                lxdm_do_login(pw, session, lang);
                if( lxdm_cur_session() <= 0 )
                    ui_prepare();
            }
            else
                write(greeter_pipe[0], "reset\n", 6);
        }
        g_free(user);
        g_free(pass);
        g_free(session);
        g_free(lang);
    }
    g_free(str);
    return TRUE;
}

static void on_greeter_exit(GPid pid, gint status, gpointer data)
{
    if( pid != greeter )
        return;
    greeter = -1;
    g_source_remove(greeter_watch);
}

void ui_prepare(void)
{
    cairo_t *cr;
    PangoFontDescription *desc;
    char *p;
    int w, h;

    /* get current display */
    dpy = gdk_x11_get_default_xdisplay();
    root = gdk_get_default_root_window();

    /* if session is running */
    if( lxdm_cur_session() > 0 )
        return;

    /* if find greeter, run it */
    p = g_key_file_get_string(config, "base", "greeter", NULL);
    if( p && p[0] )
    {
        char **argv;
        gboolean ret;
        g_shell_parse_argv(p, NULL, &argv, NULL);
        
        /* FIXME: what's this? */
        if( greeter > 0 && kill(greeter, 0) == 0 )
            return;

        ret = g_spawn_async_with_pipes(NULL, argv, NULL,
                                       G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, greeter_setup, 0,
                                       &greeter, greeter_pipe + 0, greeter_pipe + 1, NULL, NULL);
        g_strfreev(argv);
        if( ret == TRUE )
        {
            g_free(p);
            greeter_io = g_io_channel_unix_new(greeter_pipe[1]);
            io_id = g_io_add_watch(greeter_io, G_IO_IN | G_IO_HUP | G_IO_ERR,
                                   on_greeter_input, NULL);
            greeter_watch = g_child_watch_add(greeter, on_greeter_exit, 0);
            return;
        }
    }
    g_free(p);

    /* set root window bg */
    ui_set_bg();

    /* init something */
    if( sessions )
        free_xsessions(sessions);
    sessions = 0;
    session_select = 0;
    user[0] = pass[0] = 0;
    stage = 0;

    p = g_key_file_get_string(config, "input", "border", 0);
    if( !p )
        p = g_strdup("#CBCAE6");
    gdk_color_parse(p, &border);
    g_free(p);

    p = g_key_file_get_string(config, "input", "bg", 0);
    if( !p )
        p = g_strdup("#ffffff");
    gdk_color_parse(p, &bg);
    g_free(p);

    p = g_key_file_get_string(config, "input", "hint", 0);
    if( !p )
        p = g_strdup("#CBCAE6");
    gdk_color_parse(p, &hint);
    g_free(p);

    p = g_key_file_get_string(config, "input", "text", 0);
    if( !p )
        p = g_strdup("#000000");
    gdk_color_parse(p, &text);
    g_free(p);

    p = g_key_file_get_string(config, "input", "msg", 0);
    if( !p )
        p = g_strdup("#ff0000");
    gdk_color_parse(p, &msg);
    g_free(p);

    /* create the window */
    if( !win )
    {
        GdkWindowAttr attr;
        int mask = 0;
        memset( &attr, 0, sizeof(attr) );
        attr.window_type = GDK_WINDOW_CHILD;
        attr.event_mask = GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK;
        attr.wclass = GDK_INPUT_OUTPUT;
        win = gdk_window_new(0, &attr, mask);
    }

    /* create the font */
    if( layout )
    {
        g_object_unref(layout);
        layout = NULL;
    }
    cr = gdk_cairo_create(win);
    layout = pango_cairo_create_layout(cr);
    cairo_destroy(cr);
    p = g_key_file_get_string(config, "input", "font", 0);
    if( !p ) p = g_strdup("Sans 14");
    desc = pango_font_description_from_string(p);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    g_free(p);

    /* set window size */
    if( layout )
    {
        char temp[MAX_VISIBLE_CHARS + 1 + 1];
        memset( temp, 'A', sizeof(temp) );
        temp[sizeof(temp) - 1] = 0;
        get_text_layout(temp, &w, &h);
        rc.width = w + 6; rc.height = h + 6;
        rc.x = (gdk_screen_width() - rc.width) / 2;
        rc.y = (gdk_screen_height() - rc.height) / 2;
        gdk_window_move_resize(win, rc.x, rc.y, rc.width, rc.height);
    }

    /* connect event */
    gdk_window_set_events(win, GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK);
    XGrabKeyboard(dpy, GDK_WINDOW_XWINDOW(win), False, GrabModeAsync, GrabModeAsync, CurrentTime);

    /* draw the first time */
    gdk_window_show(win);
    gdk_window_focus(win, 0);
}

void ui_add_cursor(void)
{
    GdkCursor *cur;
    if( !root ) return;
    cur = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor(root, cur);
    gdk_cursor_unref(cur);
}

int ui_main(void)
{
    GMainLoop *loop = g_main_loop_new(NULL, 0);
    ui_add_cursor();
    ui_prepare();
    if(greeter == -1) /* if greeter is not used */
        gdk_event_handler_set(ui_event_cb, 0, 0);
    g_main_loop_run(loop);
    return 0;
}

