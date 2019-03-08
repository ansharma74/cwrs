#include <stdio.h>

#include <glib.h>
#include <lxconf-client.h>

static void lxconf_client_received(LXCONF_COMMAND* msg)
{
  /*gchar* str;
  GError *err = NULL;*/
  switch(msg->cmd)
  {
    case LXCMD_NEW_MESSAGE:
      printf("%s:%s\n", msg->group, msg->value);
      break;
  }
}

static void lxconf_client_connected()
{
  lxconf_client_add_channel_watch("SAY");
  lxconf_client_send_message("SAY", "hello world");
  /*lxconf_client_cleanup();
  exit(0);*/
}

int main(void)
{
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  if( !lxconf_client_init(lxconf_client_received, lxconf_client_connected) )
  {
    /* error */
  }
  g_main_loop_run(loop);
  return 0;
}
