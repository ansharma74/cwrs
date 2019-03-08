#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <lxconf/lxconf-client.h>

static void lxconf_client_connected()
{
  GError *err = NULL;
  int i=0;
  i = lxconf_client_get_int("/lxconf/examples/", "count", &err);
  if( err )
  {
    printf(">> %s\n", err->message);
  }
  i++;

  printf("count: %d\n", i);

  err = NULL;
  lxconf_client_set_int("/lxconf/examples/", "count", i, &err);

  lxconf_client_cleanup();
  exit(0);
}

int main(void)
{
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  gchar *str;

  if(!lxconf_client_init( NULL, lxconf_client_connected));
  g_main_loop_run(loop);

  return 0;
}
