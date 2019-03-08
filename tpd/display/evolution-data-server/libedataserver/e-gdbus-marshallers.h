
#ifndef __e_gdbus_marshallers_MARSHAL_H__
#define __e_gdbus_marshallers_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:POINTER (e-gdbus-marshallers.list:1) */
extern void e_gdbus_marshallers_BOOLEAN__POINTER (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* BOOLEAN:OBJECT (e-gdbus-marshallers.list:2) */
extern void e_gdbus_marshallers_BOOLEAN__OBJECT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* BOOLEAN:OBJECT,BOOLEAN (e-gdbus-marshallers.list:3) */
extern void e_gdbus_marshallers_BOOLEAN__OBJECT_BOOLEAN (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* BOOLEAN:OBJECT,BOXED (e-gdbus-marshallers.list:4) */
extern void e_gdbus_marshallers_BOOLEAN__OBJECT_BOXED (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* BOOLEAN:OBJECT,STRING (e-gdbus-marshallers.list:5) */
extern void e_gdbus_marshallers_BOOLEAN__OBJECT_STRING (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* BOOLEAN:OBJECT,UINT (e-gdbus-marshallers.list:6) */
extern void e_gdbus_marshallers_BOOLEAN__OBJECT_UINT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* VOID:UINT,BOXED (e-gdbus-marshallers.list:7) */
extern void e_gdbus_marshallers_VOID__UINT_BOXED (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* VOID:UINT,BOXED,STRING (e-gdbus-marshallers.list:8) */
extern void e_gdbus_marshallers_VOID__UINT_BOXED_STRING (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* VOID:UINT,BOXED,BOXED (e-gdbus-marshallers.list:9) */
extern void e_gdbus_marshallers_VOID__UINT_BOXED_BOXED (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:UINT,STRING (e-gdbus-marshallers.list:10) */
extern void e_gdbus_marshallers_VOID__UINT_STRING (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);

/* VOID:STRING,STRING (e-gdbus-marshallers.list:11) */
extern void e_gdbus_marshallers_VOID__STRING_STRING (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* VOID:BOXED (e-gdbus-marshallers.list:13) */
#define e_gdbus_marshallers_VOID__BOXED	g_cclosure_marshal_VOID__BOXED

/* VOID:STRING (e-gdbus-marshallers.list:14) */
#define e_gdbus_marshallers_VOID__STRING	g_cclosure_marshal_VOID__STRING

G_END_DECLS

#endif /* __e_gdbus_marshallers_MARSHAL_H__ */

