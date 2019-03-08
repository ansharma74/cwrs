#include <glib.h>

#include <gconf/gconf-client.h>

static GConfClient* gconf;

int main(void)
{
  //GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  {
  	int i,j;

  /*lxconf_client_init( lxconf_client_received, lxconf_client_connected);
  */
  
  	gconf = gconf_client_get_default();
  	gconf_client_set_int(gconf, "/lxconf/benchmark/int", 2, NULL);
  	g_printf( "v: %d\n", gconf_client_get_int(gconf, "/lxconf/benchmark/int", NULL) );
    for(i=0, j=0; i<1000; i++)
  	  j+=gconf_client_get_int(gconf, "/lxconf/benchmark/int", NULL);

  }
  //g_main_loop_run(loop);

  //g_object_unref(gconf);

  return 0;
}
