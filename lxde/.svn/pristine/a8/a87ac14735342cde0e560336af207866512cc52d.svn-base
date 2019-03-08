/*
 *      liblxconf.c
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

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <lxconf/lxconf.h>
#include <lxconf/lxconf-db.h>

gint     client_id = -1;
gboolean postpone_requests = FALSE;
gsize    max_buff_size = 0;
LXConfClientState   client_state = LXCONF_CLIENT_UNKNOWN;
LXConfNotifyFunc    notify_func = NULL;
LXConfConnectedFunc conn_func = NULL;
GSList* requests = NULL;

static gint parse_message(gchar* msg, gint length, LXConfMessage* lxmsg)
{
  gchar *tmp = msg;
  gint len1 = 0, len2 = 0, len3 = 0;
  lxmsg->cmd = LXCONF_CMD_UNKNOWN;
  lxmsg->group = NULL;
  lxmsg->key   = NULL;
  lxmsg->value = NULL;

  memcpy( &(lxmsg->cmd) , tmp, 4 );
  tmp += sizeof(lxmsg->cmd);

  memcpy( &(lxmsg->id) , tmp, sizeof(lxmsg->id) );
  tmp += sizeof(lxmsg->id);

  memcpy( &len1 , tmp, sizeof(len1) );
  tmp += sizeof(len1);
  if(len1)
  {
    lxmsg->group = g_malloc( len1+1 );
    memcpy(lxmsg->group, tmp, len1);
    tmp += len1;
    lxmsg->group[len1] = '\0';
  }

  memcpy( &len2 , tmp, sizeof(len2) );
  tmp += sizeof(len2);
  if(len2)
  {
    lxmsg->key = g_malloc( len2+1 );
    memcpy(lxmsg->key, tmp, len2);
    tmp += len2;
    lxmsg->key[len2] = '\0';
  }

  memcpy( &len3 , tmp, sizeof(len3) );
  tmp += sizeof(len3);
  if(len3)
  {
    lxmsg->value = g_malloc( len3+1 );
    memcpy(lxmsg->value, tmp, len3);
    tmp += len3;
    lxmsg->value[len3] = '\0';
  }

  return LXCONF_PARSE_DONE;
}

void lxconf_process_requests()
{
  GSList *i = NULL;
  gchar* buff = NULL;
  gint len = 0;
  LXConfMessage* msg;
  /* TODO send block of messages */
  for(i=requests; i; i=i->next)
  {
    msg = (LXConfMessage*)(i->data);
    msg->id = client_id;
    buff = gen_msg2(msg);
    lxconf_send(buff, msg->size, LXCONF_SOCKET);
    g_free(buff);
  }
  g_slist_free(requests);
  requests = NULL;
}

static gboolean lxconf_read_channel(GIOChannel *gio, GIOCondition condition, gpointer data)
{
  GIOStatus ret;
  GError *err = NULL;
  gsize len;
  gchar *msg = g_malloc( max_buff_size );

  LXConfMessage cmd;

  //g_debug("g_io_channel_read_chars");

  ret = g_io_channel_read_chars(gio, msg, max_buff_size, &len, &err);
  if (ret == G_IO_STATUS_ERROR)
  {
    g_free( msg );
    g_error("Error reading: %s\n", err->message);
  }

  if (len > 0) {
    ret = parse_message(msg, len, &cmd);
    if(ret == LXCONF_PARSE_DONE)
    {
      if(cmd.cmd == LXCONF_CMD_ID)
      {
        client_id = cmd.id;
        client_state = LXCONF_CLIENT_CONNECTED;
        if(!postpone_requests)
          lxconf_process_requests();
        if(conn_func)
        {
          (conn_func)();
        }
      }
      if(cmd.cmd == LXCONF_CMD_FALLBACK)
      {
        client_state = LXCONF_CLIENT_FALLBACK;
      }
      if( notify_func )
        (notify_func)(&cmd);
    }
    g_free(cmd.group);
    g_free(cmd.key);
    g_free(cmd.value);
  }

  g_free( msg );

  if (condition & G_IO_HUP)
    return FALSE;
  return TRUE;
}

static gboolean lxconf_accept_client(GIOChannel *source, GIOCondition condition, gpointer data G_GNUC_UNUSED)
{
  if (condition & G_IO_IN) {
    GIOChannel *gio;
    int fd;
    int flags;

    /* new connection */
    fd = accept(g_io_channel_unix_get_fd(source), NULL, NULL);
    if (fd < 0)
      g_error("Accept failed: %s\n", g_strerror(errno));

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    gio = g_io_channel_unix_new(fd);
    if (!gio)
      g_error("Cannot create new GIOChannel!\n");

    g_io_channel_set_encoding(gio, NULL, NULL);

    g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxconf_read_channel, NULL);

    g_io_channel_unref(gio);
  }

  /* our listener socket hung up - we are dead */
  if (condition & G_IO_HUP)
    g_error("Server listening socket died!\n");

  return TRUE;
}

gboolean lxconf_init_socket(gchar* ssock, LXConfNotifyFunc rfunc, LXConfConnectedFunc cfunc)
{
  struct sockaddr_un skaddr;
  GIOChannel *gio;
  int skfd, len;

  /* create socket */
  skfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (skfd < 0)
    g_error("Cannot create socket!");

  /* Initiate socket */
  unlink(ssock);
  bzero(&skaddr, sizeof(skaddr));

  /* setting UNIX socket */
  skaddr.sun_family = AF_UNIX;
  strcpy(skaddr.sun_path, ssock);
        len = strlen(skaddr.sun_path) + sizeof(skaddr.sun_family);
  /* bind to socket */
  if (bind(skfd, (struct sockaddr *)&skaddr, len) < 0)
    g_error("Bind on socket failed: %s\n", g_strerror(errno));

  /* listen on socket */
  if (listen(skfd, 5) < 0)
    g_error("Listen on socket failed: %s\n", g_strerror(errno));

  /* owner and permision */
  /*if (chown(ssock, 0, 0) < 0)
    g_error("Change LXCONF_SOCKET owner failed: %s\n", g_strerror(errno));*/
  if (chmod(ssock, 0666) < 0)
    g_error("Change LXCONF_SOCKET permision failed: %s\n", g_strerror(errno));

  /* create I/O channel */
  gio = g_io_channel_unix_new(skfd);
  if (!gio)
    g_error("Cannot create new GIOChannel!\n");

  /* setting encoding */
  g_io_channel_set_encoding(gio, NULL, NULL);
  g_io_channel_set_buffered(gio, TRUE);
  g_io_channel_set_close_on_unref(gio, TRUE);

  /* I/O channel into the main event loop */
  if (!g_io_add_watch(gio, G_IO_IN | G_IO_HUP, lxconf_accept_client, NULL))
    g_error("Cannot add watch on GIOChannel\n");
  notify_func = rfunc;
  conn_func = cfunc;
  /* channel will automatically shutdown when the watch returns FALSE */
  g_io_channel_set_close_on_unref(gio, TRUE);
  g_io_channel_unref(gio);

  max_buff_size = g_io_channel_get_buffer_size(gio);

  return TRUE;
}

gboolean lxconf_send(gchar* message, gsize length, gchar* ssock)
{
  GIOChannel *gio;
  GIOStatus ret;
  gsize len;
  int sockfd;
  struct sockaddr_un sa_un;

  if( !length ) return FALSE;

  /* create socket */
  sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    g_warning("Cannot create socket!");
    return FALSE;
  }

  /* Initiate socket */
  bzero(&sa_un, sizeof(sa_un));

  /* setting UNIX socket */
  sa_un.sun_family = AF_UNIX;
        strcpy(sa_un.sun_path, ssock);
        len = strlen(sa_un.sun_path) + sizeof(sa_un.sun_family);


  if( connect(sockfd, (struct sockaddr *) &sa_un, len) == -1 ) {
   g_warning("Cannot connect!\n");
   return FALSE;
  }

  gio = g_io_channel_unix_new(sockfd);
  g_io_channel_set_encoding(gio, NULL, NULL);
  g_io_channel_set_buffered (gio, FALSE);

  ret = g_io_channel_write_chars(gio, message, length, &len, NULL);
  close(sockfd);
  return TRUE;
}

/*gchar* gen_msg(gint cmd, gint id, const gchar* group, const gchar* key, const gchar* value, gsize* length)
{
  int len1 = 0, len2 = 0, len3 = 0;
  gchar *buff, *tmp;
  if( group ) len1 = strlen(group);
  if( key )   len2 = strlen(key);
  if( value ) len3 = strlen(value);

  *length = sizeof(cmd)+sizeof(id)+sizeof(len1)+len1+sizeof(len2)+len2+sizeof(len3)+len3;
  buff = g_malloc( *length );
  tmp = buff;

  memcpy(tmp, &cmd, sizeof(cmd) );   tmp += sizeof(cmd);
  memcpy(tmp, &id, sizeof(id) );     tmp += sizeof(id);
  memcpy(tmp, &len1, sizeof(len1) ); tmp += sizeof(len1);
  memcpy(tmp, group, len1);          tmp += len1;
  memcpy(tmp, &len2, sizeof(len2) ); tmp += sizeof(len2);
  memcpy(tmp, key,   len2);          tmp += len2;
  memcpy(tmp, &len3, sizeof(len3) ); tmp += sizeof(len3);
  memcpy(tmp, value, len3);          tmp += len3;
  return buff;
}*/

gchar* gen_msg2(LXConfMessage *msg)
{
  int len1 = 0, len2 = 0, len3 = 0;
  gchar *buff, *tmp;

  if( msg->group ) len1 = strlen(msg->group);
  if( msg->key )   len2 = strlen(msg->key);
  switch(msg->cmd)
  {
    case LXCONF_CMD_SET_DOUBLE:
      len3=sizeof(gdouble);
      break;
    case LXCONF_CMD_SET_INT: case LXCONF_CMD_SET_BOOL:
      len3=sizeof(gint);
      break;
    case LXCONF_CMD_SET_STRING: case LXCONF_CMD_NEW_MESSAGE:
      if(msg->value) len3=strlen(msg->value);
      break;
/* TODO
  case LXCONF_CMD_SET_DOUBLE_LIST:
  case LXCONF_CMD_SET_INT_LIST:
  case LXCONF_CMD_SET_BOOL_LIST:
  case LXCONF_CMD_SET_STRING_LIST: */
  }

  buff = g_malloc( msg->size );
  tmp = buff;

  memcpy(tmp, &msg->cmd, sizeof(msg->cmd) );   tmp += sizeof(msg->cmd);
  memcpy(tmp, &msg->id, sizeof(msg->id) );     tmp += sizeof(msg->id);

  memcpy(tmp, &len1, sizeof(len1) );
  tmp += sizeof(len1);
  memcpy(tmp, msg->group, len1);
  tmp += len1;

  memcpy(tmp, &len2, sizeof(len2) );
  tmp += sizeof(len2);
  memcpy(tmp, msg->key,   len2);
  tmp += len2;

  memcpy(tmp, &len3, sizeof(len3) );
  tmp += sizeof(len3);
  memcpy(tmp, msg->value, len3);
  tmp += len3;

  return buff;
}

gchar* gen_msg3(gint cmd, gint id, const gchar* group, const gchar* key, const void* value, gsize value_size, gsize* length, gboolean islast)
{
  int len1 = 0, len2 = 0, len3 = value_size;
  gchar *buff, *tmp;
  if( group ) len1 = strlen(group);
  if( key )   len2 = strlen(key);
  if( (value_size == -1)  ) len3 = strlen(value);
  /*printf("c: %d g: %s k: %s v: %s\n", cmd, group, key, value);*/

  /* [int][int] [int] [~]    [int]   [~]*/
  /* [CMD] [ID] [LEN1] [GROUP] [LEN2] [KEY] [LEN3] [VALUE]*/
  *length = sizeof(cmd)+sizeof(id)+sizeof(len1)+len1+sizeof(len2)+len2+sizeof(len3)+len3;
  buff = g_malloc( *length );
  tmp = buff;

  memcpy(tmp, &cmd, sizeof(cmd) );   tmp += sizeof(cmd);
  memcpy(tmp, &id, sizeof(id) );     tmp += sizeof(id);
  memcpy(tmp, &len1, sizeof(len1) ); tmp += sizeof(len1);
  memcpy(tmp, group, len1);          tmp += len1;
  memcpy(tmp, &len2, sizeof(len2) ); tmp += sizeof(len2);
  memcpy(tmp, key,   len2);          tmp += len2;
  memcpy(tmp, &len3, sizeof(len3) ); tmp += sizeof(len3);
  memcpy(tmp, value, len3);          tmp += len3;
  return buff;
}

LXConfMessage *new_lxconf_message(gint cmd, gint id, const gchar* group, const gchar* key, const void* value)
{
  LXConfMessage *lxmsg = g_malloc( sizeof(LXConfMessage) );
  lxmsg->size = 5*sizeof(gint);
  lxmsg->cmd = cmd;
  lxmsg->id  = id;
  lxmsg->group = g_strdup(group);
  lxmsg->key = g_strdup(key);
  if(lxmsg->group) lxmsg->size+= strlen(lxmsg->group);
  if(lxmsg->key)   lxmsg->size+= strlen(lxmsg->key);
  switch(cmd)
  {
    case LXCONF_CMD_SET_DOUBLE:
      {
        lxmsg->value = g_malloc( sizeof(gdouble) );
        memcpy(lxmsg->value, value, sizeof(gdouble) );
        lxmsg->size+=sizeof(gdouble);
      }
      break;
    case LXCONF_CMD_SET_INT: case LXCONF_CMD_SET_BOOL:
      {
        lxmsg->value = g_malloc( sizeof(gint) );
        memcpy(lxmsg->value, value, sizeof(gint) );
        lxmsg->size+=sizeof(gint);
      }
      break;
    case LXCONF_CMD_SET_STRING: case LXCONF_CMD_NEW_MESSAGE:
      lxmsg->value = g_strdup(value);
      if(lxmsg->value) lxmsg->size+= strlen(value);
      break;
/* TODO
  case LXCONF_CMD_SET_DOUBLE_LIST:
  case LXCONF_CMD_SET_INT_LIST:
  case LXCONF_CMD_SET_BOOL_LIST:
  case LXCONF_CMD_SET_STRING_LIST: */
    default:
      lxmsg->value = NULL;
  }
  return lxmsg;
}
