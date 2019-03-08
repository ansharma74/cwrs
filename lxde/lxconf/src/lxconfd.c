/*
 *      lxconfd.c
 *
 *      Copyright 2009 Daniel Kesler <kesler.daniel@gmail.com>
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <lxconf/lxconf.h>
#include <lxconf/lxconf-db.h>

GSList* clients = NULL;
GSList* watch = NULL;
GSList* channels = NULL;
extern LXConfDBEngine engine;


gboolean add_watch            (gint id, gchar* group);
void     remove_watch         (gint id);
gboolean add_channel          (gint id, gchar* channel);
void     remove_channel_watch (gint id);
void     lxcond_cleanup       ();

void catcher(int sig)
{
  switch(sig)
  {
    case SIGINT:
      signal(SIGINT, catcher);
      g_printf("SIGINT\n");
      lxcond_cleanup ();
      exit(0);
      break;
    case SIGTERM:
      signal(SIGTERM, catcher);
      g_printf("SIGTERM\n");
      lxcond_cleanup ();
      exit(0);
      break;
    case SIGKILL:
      signal(SIGKILL, catcher);
      g_printf("SIGKILL\n");
      lxcond_cleanup ();
      exit(0);
      break;
    case SIGHUP:
      signal(SIGHUP, catcher);
      g_printf("SIGHUP\n");
      lxcond_cleanup ();
      break;
  }
}


static gint idcmp(gpointer a, gpointer b)
{
  return ( LXCONF_PAIR_ID(a) - LXCONF_PAIR_ID(b) );
}

static gboolean is_pair_in_list(GSList* list, gint id, gchar* str)
{
  GSList* i;
  for(i = list; i; i = i->next)
  {
    if( LXCONF_PAIR_ID(i->data) == id)
    {
      if( !strcmp( LXCONF_PAIR_STR(i->data), str ) )
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

static gboolean is_str_in_pair_list(GSList* list, gchar* sock)
{
  GSList* i;
  for(i = list; i; i = i->next)
  {
    if( g_str_equal( LXCONF_PAIR_STR(i->data), sock ) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

gchar* find_str2id(GSList* list, gint id)
{
  GSList* i;
  for(i = list; i; i = i->next)
  {
    if( LXCONF_PAIR_ID(i->data) == id )
    {
      return (gchar*)LXCONF_PAIR_STR(i->data);
    }
  }
  return NULL;
}

static gint next_id(GSList* list)
{
  GSList* i;
  gint id = 1;

  for(i = list; i; i = i->next)
  {
    if( LXCONF_PAIR_ID(i->data) == id ) id++;
  }

  return id;
}

gboolean add_client(gint id, gchar* sock)
{
  LXConfStringPair* lc = NULL;

  if(!is_str_in_pair_list(clients, sock))
  {
    lc = g_malloc(sizeof(LXConfStringPair));
    lc->str = g_strdup(sock);
    lc->id   = id;
    clients = g_slist_append(clients, lc);
    clients = g_slist_sort(clients, (GCompareFunc)idcmp);
    return TRUE;
  }

  return FALSE;
}

/*gint g_slist_count(GSList* list)
{
  gint count = 0;
  GSList* i;
  for(i = list; i; i = i->next)
  {
    count++;
  }
  return count;
}*/

gboolean remove_client(gint id)
{
  GSList* i;

  for(i = clients; i; i = i->next)
  {
    if( LXCONF_PAIR_ID(i->data) == id )
    {
      g_free( LXCONF_PAIR_STR(i->data) );
      clients = g_slist_remove(clients, i->data);
    }
  }
  return TRUE;
}

gboolean add_watch(gint id, char* group)
{
  LXConfStringPair* wp = NULL;
  if( !is_pair_in_list(watch, id, group) )
  {
    g_debug("add_watch %d", id);
    wp = g_malloc(sizeof(LXConfStringPair));
    wp->str = g_strdup(group);
    wp->id   = id;
    watch = g_slist_append(watch, wp);
    return TRUE;
  }
  return FALSE;
}

void remove_watch(gint id)
{
  GSList* i, *next = NULL;

  i = watch;
  while(i)
  {
    if( LXCONF_PAIR_ID(i->data) == id )
    {
      next = i->next;
      watch = g_slist_remove(watch, i->data);
      i = next;
    }
    else
      i = i->next;
  }
}

gboolean add_channel(gint id, char* channel)
{
  LXConfStringPair* wp = NULL;
  if( !is_pair_in_list(channels, id, channel) )
  {
    wp = g_malloc(sizeof(LXConfStringPair));
    wp->str = g_strdup(channel);
    wp->id   = id;
    channels = g_slist_append(channels, wp);
    return TRUE;
  }
  return FALSE;
}

void remove_channel_watch(gint id)
{
  GSList* i, *next = NULL;

  i = channels;
  while(i)
  {
    if( LXCONF_PAIR_ID(i->data) == id )
    {
      next = i->next;
      channels = g_slist_remove(channels, i->data);
      i = next;
    }
    else
      i = i->next;
  }
}

void notify_clients(LXConfMessage* msg)
{
  GSList* i;
  gchar *buff, *sock;
  gsize  len;
  buff = gen_msg3(LXCONF_CMD_VALUE_CHANGED, -1, msg->group, msg->key, NULL, 0, &len, TRUE);

  //g_debug("notify about: [%s] [%s]", msg->group, msg->key);

  for(i = watch; i; i = i->next)
  {
    if( g_str_equal( LXCONF_PAIR_STR(i->data), msg->group ) )
    {
      sock = find_str2id(clients, LXCONF_PAIR_ID(i->data));
      if(sock)
      {
        //g_debug("notify SOCK: [%s]\n", sock);
        lxconf_send(buff, len, sock);
      }
    }
  }

  g_free(buff);
}

void multicast_message(LXConfMessage* msg)
{
  GSList* i;
  gchar *buff, *sock;
  gsize  len;
  buff = gen_msg3(LXCONF_CMD_NEW_MESSAGE, -1, msg->group, NULL, msg->value, -1, &len, TRUE);

  for(i = channels; i; i = i->next)
  {
    if ( g_str_equal( LXCONF_PAIR_STR(i->data), msg->group) )
    {
      sock = find_str2id(clients, LXCONF_PAIR_ID(i->data));
      if(sock)
      {
        lxconf_send(buff, len, sock);
      }
    }
  }

  g_free( buff );
}

static void process_cmd(LXConfMessage* msg)
{
  gchar* buff;
  gsize  len;
  gint id;
  GError *err = NULL;

  switch(msg->cmd)
  {
    case LXCONF_CMD_TEST:
      //g_printf("client test\n");
      break;
    case LXCONF_CMD_REG:
      if( add_client( id = next_id(clients), msg->value ) )
      {
        g_debug("new client: %s [%d]", msg->value, id );
        buff = gen_msg3(LXCONF_CMD_ID, id, NULL, NULL, NULL, 0, &len, TRUE);
        lxconf_send(buff, len, msg->value);
        g_free( buff );
      }
      else
      {
        /*lxconf_send("err 0 Client Exists", 3, msg->path);*/
      }
      break;
    case LXCONF_CMD_UNREG:
      g_debug("client left: [%d]", msg->id );
      remove_client(msg->id);
      remove_watch(msg->id);
      break;
    case LXCONF_CMD_WATCH_GROUP:
      g_debug("client add watch group: [%s]", msg->group );
      if( !add_watch(msg->id, msg->group) )
      {
        /* send err */
      }
      break;
    case LXCONF_CMD_WATCH_CHANNEL:
      g_debug("client add watch channel: [%s]", msg->group );
      if( !add_channel(msg->id, msg->group) )
      {
        /* send err */
      }
      break;
    case LXCONF_CMD_NEW_MESSAGE:
      g_debug("MSG [%s:%s]", msg->group, msg->value );
      multicast_message(msg);
      break;
    case LXCONF_CMD_SET_DOUBLE:
      (engine.write_double)(msg->group, msg->key, *((gdouble*)msg->value), &err);
      notify_clients(msg);
      break;
    case LXCONF_CMD_SET_INT:
      (engine.write_int)(msg->group, msg->key, *((gint*)msg->value), &err);
      notify_clients(msg);
      break;
    case LXCONF_CMD_SET_BOOL:
      (engine.write_bool)(msg->group, msg->key, *((gboolean*)msg->value), &err);
      notify_clients(msg);
      break;
    case LXCONF_CMD_SET_STRING:
      (engine.write_string)(msg->group, msg->key, msg->value, &err);
      notify_clients(msg);
      break;
    case LXCONF_CMD_REMOVE_KEY:
      (engine.remove_key)(msg->group, msg->key, &err);
      break;
    case LXCONF_CMD_REMOVE_GROUP:
      (engine.remove_group)(msg->group, &err);
      break;
  }
}

void lxcond_cleanup()
{
  g_slist_free(clients);
  g_slist_free(watch);
  g_slist_free(channels);
  g_unlink(LXCONF_SOCKET);
}

int main(int argc, char** argv)
{
  /*id_t pid;*/
  /* Run daemon in the background */
  /*pid = fork();
  if(pid>0)
  {
    return 0;
  }*/

  signal(SIGHUP,catcher);
  signal(SIGINT, catcher);
  signal(SIGTERM, catcher);
  signal(SIGKILL, catcher);

  {
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    if( (engine.open_db)() )
    {
      g_printf("DB: OK\n");
    }
    else
    {
      g_printf("DB: Error\n");
    }
    if( lxconf_init_socket(LXCONF_SOCKET, process_cmd, NULL) )
    {
      if( !g_file_test(LXCONF_CLIENT_SOCK_DIR, G_FILE_TEST_EXISTS) )
      {
        g_mkdir(LXCONF_CLIENT_SOCK_DIR, 0777);
      }
    }
    g_main_loop_run(loop);
  }

  /* cleanup */
  lxcond_cleanup();

  return 0;
}
