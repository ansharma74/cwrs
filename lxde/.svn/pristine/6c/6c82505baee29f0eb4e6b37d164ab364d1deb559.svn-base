/*
 *      lxconf.h
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

#ifndef LXCONF_H
#define LXCONF_H

#include <glib.h>

#define LXCONF_SOCKET "/var/tmp/lxconfd.socket"
#define LXCONF_CLIENT_SOCK_DIR "/tmp/lxconfd"
#define LXCONF_CLIENT_SOCK "/tmp/lxconfd/clientXXXXXX"

#define LXCONF_OK         0
#define LXCONF_WRONG_TYPE 1
#define LXCONF_NOT_FOUND  2

typedef enum {
  LXCONF_CMD_UNKNOWN = -1,
  LXCONF_CMD_REG   = 0,
  LXCONF_CMD_UNREG,
  LXCONF_CMD_ERR,
  LXCONF_CMD_ID,
  LXCONF_CMD_TEST,
  LXCONF_CMD_WATCH_GROUP,
  LXCONF_CMD_WATCH_CHANNEL,
  LXCONF_CMD_VALUE_CHANGED,
  LXCONF_CMD_NEW_MESSAGE,
  LXCONF_CMD_FALLBACK,

  LXCONF_CMD_SET_DOUBLE,
  LXCONF_CMD_SET_INT,
  LXCONF_CMD_SET_BOOL,
  LXCONF_CMD_SET_STRING,

  LXCONF_CMD_SET_DOUBLE_LIST,
  LXCONF_CMD_SET_INT_LIST,
  LXCONF_CMD_SET_BOOL_LIST,
  LXCONF_CMD_SET_STRING_LIST,

  LXCONF_CMD_REMOVE_KEY,
  LXCONF_CMD_REMOVE_GROUP
} LXCONF_CMD;

typedef enum {
  LXCONF_ERR_NONE = -1,
  LXCONF_ERR_OK = 0,
  LXCONF_ERR_UNKNOWN_CMD,
  LXCONF_ERR_PARAM_COUNT,
  LXCONF_ERR_NOT_FOUND,
  LXCONF_ERR_SENDING
} LXCONF_ERR;

typedef enum {
  LXCONF_PARSE_NONE,
  LXCONF_PARSE_MORE,
  LXCONF_PARSE_DONE
} LXCONF_PARSE;

typedef enum{
  LXCONF_CLIENT_UNKNOWN = 0,
  LXCONF_CLIENT_FALLBACK,
  LXCONF_CLIENT_WAITING,
  LXCONF_CLIENT_CONNECTED
} LXConfClientState;

typedef struct {
  gint   cmd;
  gint   id;
  gchar* group;
  gchar* key;
  gchar* value;
  gint   size;
} LXConfMessage;

typedef struct {
  gint   id;
  const gchar* str;
} LXConfStringPair;

#define LXCONF_PAIR_ID(o)  (((LXConfStringPair*)o)->id)
#define LXCONF_PAIR_STR(o) (((LXConfStringPair*)o)->str)

typedef void (*LXConfNotifyFunc)    (LXConfMessage*);
typedef void (*LXConfConnectedFunc) ();

gboolean lxconf_init_socket (gchar*, LXConfNotifyFunc, LXConfConnectedFunc);
gboolean lxconf_send        (gchar* message, gsize length, gchar* ssock);

void     lxconf_process_requests ();

LXConfMessage *new_lxconf_message (gint cmd, gint id, const gchar* group, const gchar* key, const void* value);
gchar* gen_msg2                   (LXConfMessage *msg);
gchar* gen_msg3                   (gint cmd, gint id, const gchar* group, const gchar* key, const void* value,
                                                            gsize value_size, gsize* length, gboolean islast);
#endif /* LXCONF_H */
