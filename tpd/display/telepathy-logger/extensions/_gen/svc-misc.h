#include <glib-object.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

typedef struct _TplSvcLogger TplSvcLogger;

typedef struct _TplSvcLoggerClass TplSvcLoggerClass;

GType tpl_svc_logger_get_type (void);
#define TPL_TYPE_SVC_LOGGER \
  (tpl_svc_logger_get_type ())
#define TPL_SVC_LOGGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), TPL_TYPE_SVC_LOGGER, TplSvcLogger))
#define TPL_IS_SVC_LOGGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), TPL_TYPE_SVC_LOGGER))
#define TPL_SVC_LOGGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE((obj), TPL_TYPE_SVC_LOGGER, TplSvcLoggerClass))


typedef void (*tpl_svc_logger_get_favourite_contacts_impl) (TplSvcLogger *self,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_get_favourite_contacts (TplSvcLoggerClass *klass, tpl_svc_logger_get_favourite_contacts_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_get_favourite_contacts (DBusGMethodInvocation *context,
    const GPtrArray *out_Favourite_Contacts);
static inline void
tpl_svc_logger_return_from_get_favourite_contacts (DBusGMethodInvocation *context,
    const GPtrArray *out_Favourite_Contacts)
{
  dbus_g_method_return (context,
      out_Favourite_Contacts);
}

typedef void (*tpl_svc_logger_add_favourite_contact_impl) (TplSvcLogger *self,
    const gchar *in_Account,
    const gchar *in_Identifier,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_add_favourite_contact (TplSvcLoggerClass *klass, tpl_svc_logger_add_favourite_contact_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_add_favourite_contact (DBusGMethodInvocation *context);
static inline void
tpl_svc_logger_return_from_add_favourite_contact (DBusGMethodInvocation *context)
{
  dbus_g_method_return (context);
}

typedef void (*tpl_svc_logger_remove_favourite_contact_impl) (TplSvcLogger *self,
    const gchar *in_Account,
    const gchar *in_Identifier,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_remove_favourite_contact (TplSvcLoggerClass *klass, tpl_svc_logger_remove_favourite_contact_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_remove_favourite_contact (DBusGMethodInvocation *context);
static inline void
tpl_svc_logger_return_from_remove_favourite_contact (DBusGMethodInvocation *context)
{
  dbus_g_method_return (context);
}

typedef void (*tpl_svc_logger_clear_impl) (TplSvcLogger *self,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_clear (TplSvcLoggerClass *klass, tpl_svc_logger_clear_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_clear (DBusGMethodInvocation *context);
static inline void
tpl_svc_logger_return_from_clear (DBusGMethodInvocation *context)
{
  dbus_g_method_return (context);
}

typedef void (*tpl_svc_logger_clear_account_impl) (TplSvcLogger *self,
    const gchar *in_Account,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_clear_account (TplSvcLoggerClass *klass, tpl_svc_logger_clear_account_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_clear_account (DBusGMethodInvocation *context);
static inline void
tpl_svc_logger_return_from_clear_account (DBusGMethodInvocation *context)
{
  dbus_g_method_return (context);
}

typedef void (*tpl_svc_logger_clear_entity_impl) (TplSvcLogger *self,
    const gchar *in_Account,
    const gchar *in_Identifier,
    gint in_Type,
    DBusGMethodInvocation *context);
void tpl_svc_logger_implement_clear_entity (TplSvcLoggerClass *klass, tpl_svc_logger_clear_entity_impl impl);
static inline
/* this comment is to stop gtkdoc realising this is static */
void tpl_svc_logger_return_from_clear_entity (DBusGMethodInvocation *context);
static inline void
tpl_svc_logger_return_from_clear_entity (DBusGMethodInvocation *context)
{
  dbus_g_method_return (context);
}

void tpl_svc_logger_emit_favourite_contacts_changed (gpointer instance,
    const gchar *arg_Account,
    const gchar **arg_Added,
    const gchar **arg_Removed);


G_END_DECLS
