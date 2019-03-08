#include <glib.h>

#include <lxconf/lxconf-client.h>

void lxconf_client_received(LXConfMessage* msg)
{
	
}

void lxconf_client_connected()
{
	int i;
	
	//lxconf_client_add_group_watch("/lxconf/benchmark/");
	g_printf( "v: %d\n", lxconf_client_get_int("/lxconf/benchmark/","int", NULL) );
	for(i=0; i<1000; i++)
		lxconf_client_set_int("/lxconf/benchmark/", "int", i, NULL);
	g_printf("Done\n");
	
}

int main(void)
{
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  gchar *str;

  lxconf_client_init( lxconf_client_received, lxconf_client_connected);
  
  g_main_loop_run(loop);

  lxconf_client_cleanup();

  return 0;
}
