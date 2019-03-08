#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <signal.h>

#include <lxconf-client.h>

void catcher(int sig)
{
  if( sig != SIGHUP)
  {
    signal(sig, catcher);
    lxconf_client_cleanup ();
    exit(0);
  }
  else
  {
    signal(SIGHUP, catcher);
  }
}

static void lxconf_client_received(LXCONF_COMMAND* msg)
{
  /*gchar* str;
  GError *err = NULL;*/
  switch(msg->cmd)
  {
    case LXCMD_NEW_MESSAGE:
      printf("%s:%s\n", msg->group, msg->value);
      break;
    case LXCMD_VALUE_CHANGED:
      printf("LXCMD_VALUE_CHANGED: %s %s\n", msg->group, msg->key);
      /*str = lxconf_client_get_string(msg->group, msg->key, &err);
      printf("value = %s\n", str);
      g_free(str);*/
      break;
    case LXCMD_ID:
      printf("ID: %d\n", msg->id);
      break;
  }
}

static void lxconf_client_connected()
{
  GError *err = NULL;
  gchar* str;
  /*lxconf_client_add_group_watch("root");
  lxconf_client_add_group_watch("other");
  lxconf_client_add_channel_watch("SAY");*/
  /*lxconf_client_set_string("/system/", "string", "www.lxde.org", &err);
  lxconf_client_set_int("/system/", "int", -1987, &err);
  lxconf_client_set_bool("/system/", "bool", TRUE, &err);
  lxconf_client_set_double("/system/", "double", 3.14f, &err);*/
  /*lxconf_client_remove_group("/test/", &err);*/
  /*str = lxconf_client_get_string("/system/", "string", &err);
  g_printf("/system/string = [%s]\n", str);
  g_free(str);*/
  /*gboolean i = lxconf_client_get_bool("/system/", "bool", &err);
  printf("int: %d\n", i);*/
  lxconf_client_set_string("/apple/", "string", "www.apple.com", &err);
}

int main(void)
{
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  gchar *str;

  signal(SIGINT, catcher);
  signal(SIGTERM, catcher);
  signal(SIGKILL, catcher);

  if(!lxconf_client_init(lxconf_client_received, lxconf_client_connected))
  {
    /* fallback */
    /*lxconf_client_connected();*/
  }
  g_main_loop_run(loop);

  return 0;
}
