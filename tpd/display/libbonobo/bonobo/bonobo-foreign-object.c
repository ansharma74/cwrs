/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <bonobo/bonobo-foreign-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-running-context.h>

static void
bonobo_foreign_object_class_init (BonoboForeignObjectClass *klass)
{
}

static void
bonobo_foreign_object_instance_init (GObject    *g_object,
				     GTypeClass *klass)
{
}

GType
bonobo_foreign_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboForeignObjectClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) bonobo_foreign_object_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboForeignObject),
			0, /* n_preallocs */
			(GInstanceInitFunc) bonobo_foreign_object_instance_init
		};
		
		type = g_type_register_static (BONOBO_TYPE_OBJECT, "BonoboForeignObject",
					       &info, 0);
	}

	return type;
}


BonoboObject *
bonobo_foreign_object_new (CORBA_Object corba_objref)
{
	BonoboObject *object;
	CORBA_Environment ev[1];

	g_return_val_if_fail (corba_objref != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (ev);
	if (!CORBA_Object_is_a (corba_objref, "IDL:Bonobo/Unknown:1.0", ev)) {
		if (ev->_major != CORBA_NO_EXCEPTION) {
			char *text = bonobo_exception_get_text (ev);
			g_warning ("CORBA_Object_is_a: %s", text);
			g_free (text);
		} else
			g_warning ("bonobo_foreign_object_new: corba_objref"
				   " doesn't have interface Bonobo::Unknown");
		object = NULL;

	} else {
		object = BONOBO_OBJECT (g_object_new (BONOBO_TYPE_FOREIGN_OBJECT, NULL));
		object->corba_objref = CORBA_Object_duplicate (corba_objref, NULL);
		bonobo_running_context_add_object_T (object->corba_objref);
	}
	CORBA_exception_free (ev);

	return object;
}

