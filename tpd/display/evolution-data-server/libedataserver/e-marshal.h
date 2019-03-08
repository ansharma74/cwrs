
#ifndef __e_marshal_MARSHAL_H__
#define __e_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:OBJECT,BOXED (e-marshal.list:1) */
extern void e_marshal_VOID__OBJECT_BOXED (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);
#define e_marshal_NONE__OBJECT_BOXED	e_marshal_VOID__OBJECT_BOXED

G_END_DECLS

#endif /* __e_marshal_MARSHAL_H__ */

