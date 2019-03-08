/*
 *      lxconf-client.c
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

#include <glib.h>
#include <glib/gstdio.h>

#include <lxconf/lxconf.h>
#include <lxconf/lxconf-client.h>
#include <lxconf/lxconf-db.h>

gchar*            client_sock = NULL;
LXConfNotifyFunc  faked_server = NULL;

extern LXConfClientState client_state;
extern gint client_id;
extern gsize max_buff_size;
extern gboolean postpone_requests;
extern LXConfDBEngine engine;
extern GSList* requests;

void lxconf_client_reg (gchar* csock);

void lxconf_client_postpone ()
{
  postpone_requests = TRUE;
}

void lxconf_client_flush ()
{
  lxconf_process_requests();
  postpone_requests = FALSE;
}

gboolean lxconf_client_init (LXConfNotifyFunc nfunc, LXConfConnectedFunc cfunc)
{
  gsize len;
  gchar* buff = NULL;
  buff = gen_msg3(LXCONF_CMD_TEST, 0, NULL, NULL, NULL, 0, &len, TRUE);
  if( (engine.open_db)() )
  {
    /*it is OK */
  }
  else
    g_error("cannot open configureation file");

  if( lxconf_send(buff, len, LXCONF_SOCKET) )
  {
    client_sock = g_strdup(LXCONF_CLIENT_SOCK);
    if( g_mkstemp(client_sock) != -1 )
    {
      lxconf_init_socket (client_sock, nfunc, cfunc);
      lxconf_client_reg  (client_sock);
      client_state = LXCONF_CLIENT_WAITING;
    }
    else
    {
      g_error("mkstemp\n");
    }
    g_free( buff );
    return TRUE;
  }
  else
  {
    g_warning("No active lxcond, falling back to internal fake-server\n");
    faked_server = nfunc;
    client_state = LXCONF_CLIENT_FALLBACK;
    if(cfunc) (cfunc)();
  }
  g_free( buff );
  return FALSE;
}

void fallback_server_notify(const gchar* group, const gchar* key)
{
  LXConfMessage msg;
  msg.cmd = LXCONF_CMD_VALUE_CHANGED;
  msg.id = -1;
  msg.group = g_strdup(group);
  msg.key = g_strdup(key);
  msg.value = NULL;
  if(faked_server)
    (faked_server)(&msg);
  g_free(msg.group);
  g_free(msg.key);
}

void lxconf_client_cleanup ()
{
  gsize len;
  gchar* buff;

  if( !client_sock ) return;
  if( client_state == LXCONF_CLIENT_CONNECTED )
  {
    buff = gen_msg3(LXCONF_CMD_UNREG, client_id, NULL, NULL, client_sock, -1, &len, TRUE);
    lxconf_send(buff, len, LXCONF_SOCKET);
    g_free( buff );
  }
  g_unlink( client_sock );
  g_free( client_sock );
}

void lxconf_client_reg (gchar* csock)
{
  gsize len;
  gchar* buff;
  buff = gen_msg3(LXCONF_CMD_REG, 0, NULL, NULL, csock, -1, &len, TRUE);
  lxconf_send(buff, len, LXCONF_SOCKET);
  g_free( buff );
}

void lxconf_client_add_channel_watch (const gchar* channel)
{
  gsize len;
  gchar* buff;
  if( client_state == LXCONF_CLIENT_FALLBACK ) return;

  if( postpone_requests || (client_state == LXCONF_CLIENT_WAITING) )
  {
    requests = g_slist_append(requests,
                              new_lxconf_message(LXCONF_CMD_WATCH_CHANNEL, client_id, channel, NULL, NULL)
                              );
  }
  else
  {
    buff = gen_msg3(LXCONF_CMD_WATCH_CHANNEL, client_id, channel, NULL, NULL, 0, &len, TRUE);
    lxconf_send(buff, len, LXCONF_SOCKET);
    g_free( buff );
  }
}

void lxconf_client_add_group_watch (const gchar* group)
{
  gsize len;
  gchar* buff;
  if( client_state == LXCONF_CLIENT_FALLBACK ) return;

  if( postpone_requests || (client_state == LXCONF_CLIENT_WAITING) )
  {

    requests = g_slist_append(requests,
                              new_lxconf_message(LXCONF_CMD_WATCH_GROUP, client_id, group, NULL, NULL)
                              );
  }
  else
  {
    buff = gen_msg3(LXCONF_CMD_WATCH_GROUP, client_id, group, NULL, NULL, 0, &len, TRUE);
    lxconf_send(buff, len, LXCONF_SOCKET);
    g_free( buff );
  }
}


gboolean lxconf_client_set_double (const gchar* group, const gchar* key, gdouble value, GError **err)
{
  gsize len;
  gchar* buff;
  gboolean retr;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.write_double)(group, key, value, err);
    fallback_server_notify(group, key);
  }
  else
  {
    if( postpone_requests || (client_state == LXCONF_CLIENT_WAITING) )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_SET_DOUBLE, client_id, group, key, &value)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_SET_DOUBLE, client_id, group, key, &value, sizeof(gdouble), &len, TRUE);
      retr = lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
      return retr;
    }
  }
  return TRUE;
}

gboolean lxconf_client_set_int (const gchar* group, const gchar* key, gint value, GError **err)
{
  gsize len;
  gchar* buff;
  gboolean retr;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.write_int)(group, key, value, err);
    fallback_server_notify(group, key);
  }
  else
  {
    if( postpone_requests || (client_state == LXCONF_CLIENT_WAITING) )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_SET_INT, client_id, group, key, &value)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_SET_INT, client_id, group, key, &value, sizeof(gint), &len, TRUE);
      retr = lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
      return retr;
    }
  }
  return TRUE;
}

gboolean lxconf_client_set_bool (const gchar* group, const gchar* key, gboolean value, GError **err)
{
  gsize len;
  gchar* buff;
  gboolean retr;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.write_bool)(group, key, value, err);
    fallback_server_notify(group, key);
  }
  else
  {
    if( postpone_requests || (client_state == LXCONF_CLIENT_WAITING)  )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_SET_BOOL, client_id, group, key, &value)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_SET_BOOL, client_id, group, key, &value, sizeof(gboolean), &len, TRUE);
      retr = lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
      return retr;
    }
  }
  return TRUE;
}

gboolean lxconf_client_set_string (const gchar* group, const gchar* key, gchar* value, GError **err)
{
  gsize len;
  gchar* buff;
  gboolean retr;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.write_string)(group, key, value, err);
    fallback_server_notify(group, key);
  }
  else
  {
    if( postpone_requests  || (client_state == LXCONF_CLIENT_WAITING) )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_SET_STRING, client_id, group, key, &value)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_SET_STRING, client_id, group, key, value, -1, &len, TRUE);
      retr = lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
      return retr;
    }
  }
  return TRUE;
}

gboolean lxconf_client_send_message (const gchar* channel, const gchar* message)
{
  gsize len;
  gchar* buff;
  gboolean retr;

  if( client_state == LXCONF_CLIENT_FALLBACK ) return FALSE;

  if( postpone_requests  || (client_state == LXCONF_CLIENT_WAITING) )
  {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_NEW_MESSAGE, client_id, channel, NULL, message)
                               );
  }
  else
  {
    buff = gen_msg3(LXCONF_CMD_NEW_MESSAGE, client_id, channel, NULL, message, -1, &len, TRUE);
    retr = lxconf_send(buff, len, LXCONF_SOCKET);
    g_free( buff );
    return retr;
  }

  return TRUE;
}

void lxconf_client_remove_key (const gchar* group, const gchar* key, GError **err)
{
  gsize len;
  gchar* buff;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.remove_key)(group, key, err);
    /*fallback_server_notify(group, key);*/
  }
  else
  {
    if( postpone_requests  || (client_state == LXCONF_CLIENT_WAITING) )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_REMOVE_KEY, client_id, group, key, NULL)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_REMOVE_KEY, client_id, group, key, NULL, 0, &len, TRUE);
      lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
    }
  }
}

void lxconf_client_remove_group (const gchar* group, GError **err)
{
  gsize len;
  gchar* buff;

  if( client_state == LXCONF_CLIENT_FALLBACK )
  {
    (engine.remove_group)(group, err);
    /*fallback_server_notify(group, NULL);*/
  }
  else
  {
    if( postpone_requests  || (client_state == LXCONF_CLIENT_WAITING) )
    {
      requests = g_slist_append(requests,
                                new_lxconf_message(LXCONF_CMD_REMOVE_GROUP, client_id, group, NULL, NULL)
                               );
    }
    else
    {
      buff = gen_msg3(LXCONF_CMD_REMOVE_GROUP, client_id, group, NULL, NULL, 0, &len, TRUE);
      lxconf_send(buff, len, LXCONF_SOCKET);
      g_free( buff );
    }
  }
}


gdouble lxconf_client_get_double (const gchar* group, const gchar* key, GError **err)
{
  return (engine.read_double)(group, key, err);
}

gint lxconf_client_get_int (const gchar* group, const gchar* key, GError **err)
{
  return (engine.read_int)(group, key, err);
}

gboolean lxconf_client_get_bool (const gchar* group, const gchar* key, GError **err)
{
  return (engine.read_bool)(group, key, err);
}

gchar* lxconf_client_get_string (const gchar* group, const gchar* key, GError **err)
{
  return (engine.read_string)(group, key, err);
}
