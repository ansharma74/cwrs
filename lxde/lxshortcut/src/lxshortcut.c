/*
 *      lxshortcut.c
 *
 *      Copyright 2008  <pcman.tw@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>

#define GRP_NAME    "Desktop Entry"

static GKeyFile* kf;
static GtkWidget *dlg, *icon, *change_icon, *name, *cmd, *browse, *tooltip, *terminal, *use_sn;

static gboolean no_display = FALSE;
static char* ifile = NULL;
static char* ofile = NULL;

static char* icon_name_val = NULL;

static GtkWidget* themed_icon_view = NULL;
static GtkTreeModel* themed_icon_model = NULL;
static GThreadPool* thread_pool = NULL;
static GAsyncQueue* icon_queue = NULL;

static GOptionEntry opt_entries[] =
{
    {"no-display", 'n', 0, G_OPTION_ARG_NONE, &no_display, NULL, NULL},
    {"input", 'i', 0, G_OPTION_ARG_FILENAME, &ifile, N_("input file"), N_("file name or desktop id")},
    {"output", 'o', 0, G_OPTION_ARG_FILENAME, &ofile, N_("file name"), NULL},
    { NULL }
};

static GdkPixbuf* vfs_load_icon( GtkIconTheme* theme, const char* icon_name, int size );

static void on_update_preview( GtkFileChooser* chooser, GtkWidget* img )
{
    char* file = gtk_file_chooser_get_preview_filename( chooser );
    if( file )
    {
        GdkPixbuf* pix = gdk_pixbuf_new_from_file_at_scale( file, 48, 48, TRUE, NULL );
        if( pix )
        {
            gtk_image_set_from_pixbuf( GTK_IMAGE(img), pix );
            g_object_unref( pix );
            return;
        }
    }
    gtk_image_clear( GTK_IMAGE(img) );
}

static void on_switch_page( GtkNotebook* notebook, GtkWidget* page, int n, gpointer user_data )
{

}

static void on_toggle_theme( GtkToggleButton* btn, GtkNotebook* notebook )
{
    gtk_notebook_set_current_page( notebook, 0 );
}

static void on_toggle_files( GtkToggleButton* btn, GtkNotebook* notebook )
{
    gtk_notebook_set_current_page( notebook, 1 );
}

static gpointer load_themed_icon( GtkIconTheme* theme )
{
    GdkPixbuf* pix;
    char* icon_name = g_async_queue_pop( icon_queue );
    gdk_threads_enter();
    pix = vfs_load_icon( theme, icon_name, 48 );
    gdk_threads_leave();
    g_thread_yield();
    if( pix )
    {
        GtkTreeIter it;
        gdk_threads_enter();
        gtk_list_store_append( GTK_LIST_STORE(themed_icon_model), &it );
        gtk_list_store_set( GTK_LIST_STORE(themed_icon_model), &it, 0, pix, 1, icon_name, -1 );
        g_object_unref( pix );
        gdk_threads_leave();
    }
    g_thread_yield();
    if( g_async_queue_length(icon_queue) == 0 )
    {
        gdk_threads_enter();
        if( gtk_icon_view_get_model(GTK_ICON_VIEW(themed_icon_view)) == NULL )
        {
            gtk_icon_view_set_model(GTK_ICON_VIEW(themed_icon_view), themed_icon_model);
            if( GTK_WIDGET_REALIZED(themed_icon_view) )
                gdk_window_set_cursor(themed_icon_view->window, NULL);
        }
        gdk_threads_leave();
    }
    /* g_debug("load: %s", icon_name); */
    g_free(icon_name);
    return NULL;
}

static void on_change_icon( GtkButton* btn, GtkImage* icon )
{
    GtkBuilder* builder;
    GtkWidget *chooser_dlg, *chooser, *preview, *icons, *notebook;
    GtkFileFilter* filter;
    GtkIconTheme* theme;
    GList *contexts, *l;

    builder = gtk_builder_new();
    gtk_builder_add_from_file( builder, PACKAGE_DATA_DIR "/choose-icon.ui", NULL );
    chooser_dlg = GTK_WIDGET(gtk_builder_get_object( builder, "dlg" ));
    chooser = GTK_WIDGET(gtk_builder_get_object( builder, "chooser" ));
    themed_icon_view = icons = GTK_WIDGET(gtk_builder_get_object( builder, "icons" ));
    notebook = GTK_WIDGET(gtk_builder_get_object( builder, "notebook" ));
    g_signal_connect( gtk_builder_get_object(builder,"theme"), "toggled", G_CALLBACK(on_toggle_theme), notebook );
    g_signal_connect( gtk_builder_get_object(builder,"files"), "toggled", G_CALLBACK(on_toggle_files), notebook );

    gtk_window_set_default_size( GTK_WINDOW(chooser_dlg), 600, 440 );
    gtk_window_set_transient_for( GTK_WINDOW(chooser_dlg), GTK_WINDOW(dlg) );

    preview = gtk_image_new();
    gtk_widget_show( preview );
    gtk_file_chooser_set_preview_widget( GTK_FILE_CHOOSER(chooser), preview );
    g_signal_connect( chooser, "update-preview", G_CALLBACK(on_update_preview), preview );

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( GTK_FILE_FILTER(filter), _("Image files") );
    gtk_file_filter_add_pixbuf_formats( GTK_FILE_FILTER(filter) );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(chooser), filter );

    gtk_widget_show(chooser_dlg);
    while (gtk_events_pending ())
        gtk_main_iteration();

    gdk_window_set_cursor(themed_icon_view->window, gdk_cursor_new(GDK_WATCH));

    /* load themed icons */
    thread_pool = g_thread_pool_new( (GFunc) load_themed_icon, builder, 1, TRUE, NULL );
    g_thread_pool_set_max_threads(thread_pool, 1, NULL);
    icon_queue = g_async_queue_new();

    themed_icon_model = GTK_TREE_MODEL(gtk_list_store_new( 2, GDK_TYPE_PIXBUF, G_TYPE_STRING ));
    theme = gtk_icon_theme_get_default();

    gtk_icon_view_set_pixbuf_column( GTK_ICON_VIEW(icons), 0 );
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(icons), 80);
    gtk_icon_view_set_text_column( GTK_ICON_VIEW(icons), 1 );

    /* GList* contexts = gtk_icon_theme_list_contexts( theme ); */
    contexts = g_list_alloc();
    /* FIXME: we should enable more contexts */
    /* http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html#context */
    contexts->data = g_strdup("Applications");
    for( l = contexts; l; l = l->next )
    {
        /* g_debug(l->data); */
        GList* icon_names = gtk_icon_theme_list_icons( theme, (char*)l->data );
        GList* icon_name;
        for( icon_name = icon_names; icon_name; icon_name = icon_name->next )
        {
            g_async_queue_push( icon_queue, icon_name->data );
            g_thread_pool_push( thread_pool, theme, NULL );
        }
        g_list_free( icon_names );
        g_free( l->data );
    }
    g_list_free( contexts );

    if( gtk_dialog_run( GTK_DIALOG(chooser_dlg) ) == GTK_RESPONSE_OK )
    {
        char* icon_name = NULL;
        GdkPixbuf* pix = NULL;
        if( gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)) == 0 )
        {
            GList* sels = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(icons));
            GtkTreePath* tp = (GtkTreePath*)sels->data;
            GtkTreeIter it;
            if( gtk_tree_model_get_iter(themed_icon_model, &it, tp) )
            {
                gtk_tree_model_get(themed_icon_model, &it, 1, &icon_name, -1);
                pix = vfs_load_icon(gtk_icon_theme_get_default(), icon_name, 48);
            }
            g_list_foreach(sels, (GFunc)gtk_tree_path_free, NULL);
            g_list_free(sels);
        }
        else
        {
            icon_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(chooser) );
            pix = gdk_pixbuf_new_from_file_at_scale(icon_name, 32, 32, TRUE, NULL);
        }
        if (pix)
        {
            gtk_image_set_from_pixbuf(icon, pix);
            g_object_unref(pix);
        }
        g_free(icon_name_val);
        icon_name_val = icon_name;
    }
    g_thread_pool_free( thread_pool, TRUE, FALSE );
    gtk_widget_destroy( chooser_dlg );
}

static gboolean exe_filter( const GtkFileFilterInfo* inf, gpointer user_data )
{
    return g_file_test( inf->filename, G_FILE_TEST_IS_EXECUTABLE );
}

static void on_browse_cmd( GtkButton* btn, gpointer user_data )
{
    GtkWidget* chooser;
    GtkFileFilter* filter;
    chooser = gtk_file_chooser_dialog_new( _("Choose an executable file"),
                                        NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL );
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(chooser), "/usr/bin" );
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( GTK_FILE_FILTER(filter), _("Executable files") );
    gtk_file_filter_add_custom( GTK_FILE_FILTER(filter), GTK_FILE_FILTER_FILENAME, exe_filter, NULL, NULL );
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(chooser), filter );

    if( gtk_dialog_run( GTK_DIALOG(chooser) ) == GTK_RESPONSE_OK )
    {
        char* file = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(chooser) );
        gtk_entry_set_text( GTK_ENTRY(cmd), file );
        g_free( file );
    }
    gtk_widget_destroy( chooser );
}

static GdkPixbuf* load_icon_file( const char* file_name, int size )
{
    GdkPixbuf* icon = NULL;
    char* file_path;
    const gchar** dirs = (const gchar**) g_get_system_data_dirs();
    const gchar** dir;
    for( dir = dirs; *dir; ++dir )
    {
        file_path = g_build_filename( *dir, "pixmaps", file_name, NULL );
        icon = gdk_pixbuf_new_from_file_at_scale( file_path, size, size, TRUE, NULL );
        g_free( file_path );
        if( icon )
            break;
    }
    return icon;
}

GdkPixbuf* vfs_load_icon( GtkIconTheme* theme, const char* icon_name, int size )
{
    GdkPixbuf* icon = NULL;
    const char* file;
    GtkIconInfo* inf = gtk_icon_theme_lookup_icon( theme, icon_name, size,
                                             GTK_ICON_LOOKUP_USE_BUILTIN );
    if( G_UNLIKELY( ! inf ) )
        return NULL;

    file = gtk_icon_info_get_filename( inf );
    if( G_LIKELY( file ) )
        icon = gdk_pixbuf_new_from_file_at_scale( file, size, size, TRUE, NULL );
    else
        {
        icon = gtk_icon_info_get_builtin_pixbuf( inf );
        g_object_ref( icon );
        }
    gtk_icon_info_free( inf );

    if( G_LIKELY( icon ) )  /* scale down the icon if it's too big */
    {
        int width, height;
        height = gdk_pixbuf_get_height(icon);
        width = gdk_pixbuf_get_width(icon);

        if( G_UNLIKELY( height > size || width > size ) )
        {
            GdkPixbuf* scaled;
            if( height > width )
            {
                width = size * height / width;
                height = size;
            }
            else if( height < width )
            {
                height = size * width / height;
                width = size;
            }
            else
                height = width = size;
            scaled = gdk_pixbuf_scale_simple( icon, width, height, GDK_INTERP_BILINEAR );
            g_object_unref( icon );
            icon = scaled;
        }
    }
    return icon;
}

static GdkPixbuf* load_icon( const char* name, int size, gboolean use_fallback )
{
    GtkIconTheme* theme;
    char *icon_name = NULL, *suffix;
    GdkPixbuf* icon = NULL;

    if( name )
    {
        if( g_path_is_absolute( name) )
        {
            icon = gdk_pixbuf_new_from_file_at_scale( name,
                                                     size, size, TRUE, NULL );
        }
        else
        {
            theme = gtk_icon_theme_get_default();
            suffix = strrchr( name, '.' );
            if( suffix ) /* has file extension, it's a basename of icon file */
            {
                /* try to find it in pixmaps dirs */
                icon = load_icon_file( name, size );
                if( G_UNLIKELY( ! icon ) )  /* unfortunately, not found */
                {
                    /* Let's remove the suffix, and see if this name can match an icon
                         in current icon theme */
                    icon_name = g_strndup( name,
                                           (suffix - name) );
                    icon = vfs_load_icon( theme, icon_name, size );
                    g_free( icon_name );
                }
            }
            else  /* no file extension, it could be an icon name in the icon theme */
            {
                icon = vfs_load_icon( theme, name, size );
            }
        }
    }
    if( G_UNLIKELY( ! icon ) && use_fallback )  /* fallback to generic icon */
    {
        theme = gtk_icon_theme_get_default();
        icon = vfs_load_icon( theme, "application-x-executable", size );
        if( G_UNLIKELY( ! icon ) )  /* fallback to generic icon */
        {
            icon = vfs_load_icon( theme, "gnome-mime-application-x-executable", size );
        }
    }
    return icon;
}

int main(int argc, char** argv)
{
    GtkBuilder* builder;
    GError* err = NULL;
    char* str = NULL;
    GdkPixbuf* pix;
    gboolean no_eng_name = FALSE;
    gsize len;

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    /* init threading support */
    g_thread_init(NULL);
    gdk_threads_init();

    /* initialize GTK+ and parse the command line arguments */
    if( G_UNLIKELY( ! gtk_init_with_args( &argc, &argv, "", opt_entries, GETTEXT_PACKAGE, &err ) ) )
    {
        g_print( "Error: %s\n", err->message );
        return 1;
    }

    if( ! ifile && ! ofile )
        return 1;

    /* build the UI */
    builder = gtk_builder_new();

    gtk_builder_add_from_file( builder, PACKAGE_DATA_DIR "/lxshortcut.ui", NULL );

    dlg = GTK_WIDGET(gtk_builder_get_object( builder, "dlg" ));
    icon = GTK_WIDGET(gtk_builder_get_object( builder, "icon" ));
    change_icon = GTK_WIDGET(gtk_builder_get_object( builder, "change_icon" ));
    name = GTK_WIDGET(gtk_builder_get_object( builder, "name" ));
    cmd = GTK_WIDGET(gtk_builder_get_object( builder, "cmd" ));
    browse = GTK_WIDGET(gtk_builder_get_object( builder, "browse" ));
    tooltip = GTK_WIDGET(gtk_builder_get_object( builder, "tooltip" ));
    terminal = GTK_WIDGET(gtk_builder_get_object( builder, "terminal" ));
    use_sn = GTK_WIDGET(gtk_builder_get_object( builder, "use_sn" ));

    gtk_dialog_set_alternative_button_order( GTK_DIALOG(dlg), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1 );

    g_object_unref( builder );

    /* signals */
    g_signal_connect( change_icon, "clicked", G_CALLBACK(on_change_icon), icon );
    g_signal_connect( browse, "clicked", G_CALLBACK(on_browse_cmd), cmd );

    /* read the desktop entry flie */
    kf = g_key_file_new();
    if( ifile )
    {
        // FIXME: this won't work for files without full path
        if( strchr( ifile, '/' ) )  /* if this is a file path */
        {
            g_key_file_load_from_file( kf, ifile, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL );

            if( ! ofile ) /* store to the same file if not otherwise specified. */
                ofile = g_strdup(ifile);
        }
        else /* otherwise treat it as desktop id */
        {
            str = g_strconcat( "applications/", ifile, NULL );
            g_key_file_load_from_data_dirs( kf, str, NULL, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, NULL );
            g_free( str );

            if( ! ofile )
                ofile = g_build_filename( g_get_user_data_dir(), "applications", ifile, NULL );
        }

        /* icon */
        icon_name_val = str = g_key_file_get_string( kf, GRP_NAME, "Icon", NULL );
        pix = load_icon( str, 48, TRUE );
        if (pix)
        {
            gtk_image_set_from_pixbuf( GTK_IMAGE(icon), pix );
            g_object_unref( pix );
        }

        /* name */
        str = g_key_file_get_locale_string( kf, GRP_NAME, "Name", NULL, NULL );
        if( ! str )
            str = g_key_file_get_string( kf, GRP_NAME, "Name", NULL );
        if( str )
        {
            gtk_entry_set_text( GTK_ENTRY(name), str );
            g_free( str );
        }
        else
        {
            no_eng_name = TRUE;
        }

        /* exec */
        str = g_key_file_get_value( kf, GRP_NAME, "Exec", NULL );
        if( str )
        {
            gtk_entry_set_text( GTK_ENTRY(cmd), str );
            g_free( str );
        }

        /* comment/tooltip */
        str = g_key_file_get_locale_string( kf, GRP_NAME, "Comment", NULL, NULL );
        if( ! str )
            str = g_key_file_get_string( kf, GRP_NAME, "Comment", NULL );
        if( str )
        {
            gtk_entry_set_text( GTK_ENTRY(tooltip), str );
            g_free( str );
        }

        /* terminal */
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(terminal), g_key_file_get_boolean( kf, GRP_NAME, "Terminal", NULL ) );

        /* startup notify */
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(use_sn), g_key_file_get_boolean( kf, GRP_NAME, "StartupNotify", NULL ) );
    }
    else /* no input file, create new shortcut */
    {
        pix = load_icon( NULL, 48, TRUE );  /* load fallback icon */
        if (pix)
        {
            gtk_image_set_from_pixbuf( GTK_IMAGE(icon), pix );
            g_object_unref( pix );
        }

        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(use_sn), TRUE );
    }

    if( gtk_dialog_run( GTK_DIALOG(dlg) ) == GTK_RESPONSE_OK )
    {
        const gchar* const *langs = g_get_language_names();
        char* lang = NULL;
        /* get locale name */
        if( strcmp( langs[0], "C" ) )
        {
            /* remove encoding from locale name */
            char* sep = strchr( langs[0], '.' );
            if( sep )
                lang = g_strndup( langs[0], sep - langs[0] );
            else
                lang = g_strdup( langs[0] );
        }
        g_key_file_set_string( kf, GRP_NAME, "Encoding", "UTF-8" );
        g_key_file_set_string( kf, GRP_NAME, "Type", "Application" );

        if( ! lang || !ifile || no_eng_name )
            g_key_file_set_string( kf, GRP_NAME, "Name", gtk_entry_get_text( GTK_ENTRY(name) ) );

        if( lang )
            g_key_file_set_locale_string( kf, GRP_NAME, "Name", lang, gtk_entry_get_text( GTK_ENTRY(name) ) );

        if (icon_name_val)
            g_key_file_set_string( kf, GRP_NAME, "Icon", icon_name_val );
        g_free(icon_name_val);

        g_key_file_set_value( kf, GRP_NAME, "Exec", gtk_entry_get_text( GTK_ENTRY(cmd) ) );

        if( lang )
            g_key_file_set_locale_string( kf, GRP_NAME, "Comment", lang, gtk_entry_get_text( GTK_ENTRY(tooltip) ) );
        else
            g_key_file_set_string( kf, GRP_NAME, "Comment", gtk_entry_get_text( GTK_ENTRY(tooltip) ) );

        /* if Terminal key is changed, save it */
        if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal)) != g_key_file_get_boolean(kf, GRP_NAME, "Terminal", NULL) )
            g_key_file_set_boolean( kf, GRP_NAME, "Terminal", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(terminal)) );

        /* if StartupNotify key is changed, save it */
        if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_sn)) != g_key_file_get_boolean(kf, GRP_NAME, "StartupNotify", NULL) )
            g_key_file_set_boolean( kf, GRP_NAME, "StartupNotify", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_sn)) );

        /* if this shortcut shouldn't be displayed in menu */
        if( no_display )
            g_key_file_set_boolean( kf, GRP_NAME, "NoDisplay", TRUE );

        g_free( lang );

        if( str = g_key_file_to_data( kf, &len, NULL ) )
        {
            if( ! g_file_set_contents( ofile, str, len, &err ) )
            {
                g_error_free( err );
            }
            g_free(str);
        }
    }

    gtk_widget_destroy( dlg );

    g_key_file_free( kf );

    return 0;
}
