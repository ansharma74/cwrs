
#ifndef __gdata_marshal_MARSHAL_H__
#define __gdata_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:OBJECT,OBJECT,POINTER (gdata/gdata-marshal.list:1) */
extern void gdata_marshal_VOID__OBJECT_OBJECT_POINTER (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* STRING:OBJECT,STRING (gdata/gdata-marshal.list:2) */
extern void gdata_marshal_STRING__OBJECT_STRING (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

G_END_DECLS

#endif /* __gdata_marshal_MARSHAL_H__ */

