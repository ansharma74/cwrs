/**
 * Desktop purge: Remove unnecessary translations from *.desktop files
 * 
 * Copyright (c) 2006.10.15 Hong Jen Yee (PCMan)   <pcman.tw( at )gmail.com>
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
 *
 * NOTE: This program requires glib 2.4+.
 */

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

const char desktop_ent[] = "Desktop Entry";
const char app_dir_name[] = "applications";

static char* keep_locales = NULL;
static char* file_to_purge = NULL;
static GOptionEntry option_entries[] = 
{
    { "keep-locales", 'k', 0, G_OPTION_ARG_STRING, &keep_locales, "Specify the locales you want to keep, seperated by colon. By default only the locale currently in use will be kept.", "locale1:locale2:..." },
    { "file-to-purge", 'f', 0, G_OPTION_ARG_FILENAME, &file_to_purge, "Specify a file to purge. By default the whole desktop-database on the system will be purged", "FILE" },
    { NULL }
};

static gsize saved_size = 0;

static GKeyFile* file = NULL;

static void purge_file ( const char* file_path, struct stat* statbuf, const char* file_name )
{
    if( g_key_file_load_from_file( file, file_path, G_KEY_FILE_KEEP_COMMENTS, NULL ) )
    {
        gsize len;
        char* data;

        if( G_UNLIKELY( ! g_str_has_suffix(file_name, ".desktop") ) )
            return;

        if( (data = g_key_file_to_data( file, &len, NULL )) )
        {
            char* pdata = data;
            while( *pdata && *pdata == '\n' )
            {
                ++pdata;
                --len;
            }

            if( G_LIKELY( len < statbuf->st_size ) )
            {
                int fd = open( file_path, O_CREAT|O_WRONLY|O_TRUNC );
                if( G_LIKELY(fd != -1) )
                {
                    if( write( fd, pdata, len ) != -1 )
                    {
                        statbuf->st_size -= len;
                        saved_size += statbuf->st_size;
                        g_print( "%s purged, save %d bytes\n",
                        file_path, (int) statbuf->st_size );
                    }
                    else
                        g_print( "Error: %s\n", g_strerror( errno ) );
                    close( fd );
                }
                else
                    g_print( "Error: %s\n", g_strerror( errno ) );
            }
            else
                g_print( "Skip %s\n", file_path );

            g_free( data );
        }
    }
    else
    {
        g_print( "Error parsing: %s\n", file_path );
    }
}

static void do_purge( const char* dir_path, gpointer user_data )
{
    G_CONST_RETURN gchar* file_name;
    GDir *dir;
    struct stat statbuf;

    if( ! (dir = g_dir_open( dir_path, 0, NULL )) )
        return;

    while( (file_name = g_dir_read_name( dir )) )
    {
        char* file_path;

        file_path = g_build_filename( dir_path, file_name, NULL );
        if( stat( file_path, &statbuf ) == -1 )
        {
            g_free( file_path );
            continue;
        }

        if( S_ISDIR(statbuf.st_mode) )
            do_purge( file_path, user_data );
        else
            purge_file( file_path, &statbuf, file_name );

        g_free( file_path );
    }
    g_dir_close( dir );
}

static void app_dirs_foreach( GFunc func, gpointer user_data )
{
    const char** sys_dirs = (const char**)g_get_system_data_dirs();
    char* path;
    int i, len;

    len = g_strv_length((gchar **) sys_dirs);

    for( i = 0; i < len; ++i )
    {
        path = g_build_filename( sys_dirs[i], app_dir_name, NULL );
        func( path, user_data );
        g_free( path );
    }
    path = g_build_filename( g_get_user_data_dir(), app_dir_name, NULL );
    func( path, user_data );
    g_free( path );
}

int main( int argc, char** argv )
{
    GError *error = NULL;
    GOptionContext *context;

    g_print("Desktop purge 0.1\nDeveloped by Hong Jen Yee (PCMan) <pcman.tw@gmail.com>\n\n");

    context = g_option_context_new("- purge unnecessary locales in desktop database");
    g_option_context_add_main_entries(context, option_entries, NULL);

    if( !g_option_context_parse(context, &argc, &argv, &error) )
    {
        g_print("Error: %s\n", error->message);
        g_error_free( error );
        return 1;
    }

    if( keep_locales ) /* reserved locales */
        g_setenv( "LANGUAGE", keep_locales, TRUE );

    file = g_key_file_new();

    if( file_to_purge )
    {
        struct stat statbuf;
        if( 0 == stat( file_to_purge, &statbuf ) )
            purge_file( file_to_purge, &statbuf, file_to_purge );
        else
            g_print( "Error - file %s cannot be opened", file_to_purge );
    }
    else
        app_dirs_foreach( (GFunc) do_purge, NULL );

    g_key_file_free(file);

    g_print( "%d KB saved\n", saved_size/1024 );
    return 0;
}
