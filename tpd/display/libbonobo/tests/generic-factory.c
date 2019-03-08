/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>
#include <libbonobo.h>
#include <stdio.h>
#include <string.h>

static void destroy_cb (BonoboObject *obj, gpointer data)
{
	printf ("instance #%i destroyed\n",
		GPOINTER_TO_INT (data));
}

static BonoboObject *
generic_factory_cb (BonoboGenericFactory *this,
		    const char           *object_id,
		    void                 *data)
{
	BonoboObject *object = NULL;
	
	g_return_val_if_fail (object_id != NULL, NULL);

	if (!strcmp (object_id, "OAFIID:Test_Generic_Factory")) {
		static int counter  = 1;
		int        instance = counter++;

		printf("new instance: #%i\n", instance);
		object = (BonoboObject *) bonobo_event_source_new ();
		g_signal_connect (object, "destroy", G_CALLBACK (destroy_cb),
				  GINT_TO_POINTER (instance));
	} else
		g_warning ("Unknown OAFIID '%s'", object_id);

	return object;
}

BONOBO_ACTIVATION_FACTORY("OAFIID:Test_Generic_FactoryFactory",
			  "a test", "0", generic_factory_cb, NULL)

