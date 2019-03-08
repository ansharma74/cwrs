G_BEGIN_DECLS

typedef void (*tpl_cli_logger_signal_callback_favourite_contacts_changed) (TpProxy *proxy,
    const gchar *arg_Account,
    const gchar **arg_Added,
    const gchar **arg_Removed,
    gpointer user_data, GObject *weak_object);
TpProxySignalConnection *tpl_cli_logger_connect_to_favourite_contacts_changed (TpProxy *proxy,
    tpl_cli_logger_signal_callback_favourite_contacts_changed callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object,
    GError **error);

typedef void (*tpl_cli_logger_callback_for_get_favourite_contacts) (TpProxy *proxy,
    const GPtrArray *out_Favourite_Contacts,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_get_favourite_contacts (TpProxy *proxy,
    gint timeout_ms,
    tpl_cli_logger_callback_for_get_favourite_contacts callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


typedef void (*tpl_cli_logger_callback_for_add_favourite_contact) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_add_favourite_contact (TpProxy *proxy,
    gint timeout_ms,
    const gchar *in_Account,
    const gchar *in_Identifier,
    tpl_cli_logger_callback_for_add_favourite_contact callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


typedef void (*tpl_cli_logger_callback_for_remove_favourite_contact) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_remove_favourite_contact (TpProxy *proxy,
    gint timeout_ms,
    const gchar *in_Account,
    const gchar *in_Identifier,
    tpl_cli_logger_callback_for_remove_favourite_contact callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


typedef void (*tpl_cli_logger_callback_for_clear) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_clear (TpProxy *proxy,
    gint timeout_ms,
    tpl_cli_logger_callback_for_clear callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


typedef void (*tpl_cli_logger_callback_for_clear_account) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_clear_account (TpProxy *proxy,
    gint timeout_ms,
    const gchar *in_Account,
    tpl_cli_logger_callback_for_clear_account callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


typedef void (*tpl_cli_logger_callback_for_clear_entity) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *tpl_cli_logger_call_clear_entity (TpProxy *proxy,
    gint timeout_ms,
    const gchar *in_Account,
    const gchar *in_Identifier,
    gint in_Type,
    tpl_cli_logger_callback_for_clear_entity callback,
    gpointer user_data,
    GDestroyNotify destroy,
    GObject *weak_object);


G_END_DECLS
