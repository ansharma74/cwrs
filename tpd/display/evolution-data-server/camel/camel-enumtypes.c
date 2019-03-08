
/* Generated data (by glib-mkenums) */

#include "camel-enumtypes.h"

/* enumerations from "camel-enums.h" */
#include "camel-enums.h"

GType
camel_authentication_result_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_AUTHENTICATION_ERROR,
			  "CAMEL_AUTHENTICATION_ERROR",
			  "error" },
			{ CAMEL_AUTHENTICATION_ACCEPTED,
			  "CAMEL_AUTHENTICATION_ACCEPTED",
			  "accepted" },
			{ CAMEL_AUTHENTICATION_REJECTED,
			  "CAMEL_AUTHENTICATION_REJECTED",
			  "rejected" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelAuthenticationResult"),
			values);
	}
	return the_type;
}

GType
camel_folder_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY,
			  "CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY",
			  "has-summary-capability" },
			{ CAMEL_FOLDER_FILTER_RECENT,
			  "CAMEL_FOLDER_FILTER_RECENT",
			  "filter-recent" },
			{ CAMEL_FOLDER_HAS_BEEN_DELETED,
			  "CAMEL_FOLDER_HAS_BEEN_DELETED",
			  "has-been-deleted" },
			{ CAMEL_FOLDER_IS_TRASH,
			  "CAMEL_FOLDER_IS_TRASH",
			  "is-trash" },
			{ CAMEL_FOLDER_IS_JUNK,
			  "CAMEL_FOLDER_IS_JUNK",
			  "is-junk" },
			{ CAMEL_FOLDER_FILTER_JUNK,
			  "CAMEL_FOLDER_FILTER_JUNK",
			  "filter-junk" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelFolderFlags"),
			values);
	}
	return the_type;
}

GType
camel_folder_info_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_FOLDER_NOSELECT,
			  "CAMEL_FOLDER_NOSELECT",
			  "noselect" },
			{ CAMEL_FOLDER_NOINFERIORS,
			  "CAMEL_FOLDER_NOINFERIORS",
			  "noinferiors" },
			{ CAMEL_FOLDER_CHILDREN,
			  "CAMEL_FOLDER_CHILDREN",
			  "children" },
			{ CAMEL_FOLDER_NOCHILDREN,
			  "CAMEL_FOLDER_NOCHILDREN",
			  "nochildren" },
			{ CAMEL_FOLDER_SUBSCRIBED,
			  "CAMEL_FOLDER_SUBSCRIBED",
			  "subscribed" },
			{ CAMEL_FOLDER_VIRTUAL,
			  "CAMEL_FOLDER_VIRTUAL",
			  "virtual" },
			{ CAMEL_FOLDER_SYSTEM,
			  "CAMEL_FOLDER_SYSTEM",
			  "system" },
			{ CAMEL_FOLDER_VTRASH,
			  "CAMEL_FOLDER_VTRASH",
			  "vtrash" },
			{ CAMEL_FOLDER_SHARED_TO_ME,
			  "CAMEL_FOLDER_SHARED_TO_ME",
			  "shared-to-me" },
			{ CAMEL_FOLDER_SHARED_BY_ME,
			  "CAMEL_FOLDER_SHARED_BY_ME",
			  "shared-by-me" },
			{ CAMEL_FOLDER_TYPE_NORMAL,
			  "CAMEL_FOLDER_TYPE_NORMAL",
			  "type-normal" },
			{ CAMEL_FOLDER_TYPE_INBOX,
			  "CAMEL_FOLDER_TYPE_INBOX",
			  "type-inbox" },
			{ CAMEL_FOLDER_TYPE_OUTBOX,
			  "CAMEL_FOLDER_TYPE_OUTBOX",
			  "type-outbox" },
			{ CAMEL_FOLDER_TYPE_TRASH,
			  "CAMEL_FOLDER_TYPE_TRASH",
			  "type-trash" },
			{ CAMEL_FOLDER_TYPE_JUNK,
			  "CAMEL_FOLDER_TYPE_JUNK",
			  "type-junk" },
			{ CAMEL_FOLDER_TYPE_SENT,
			  "CAMEL_FOLDER_TYPE_SENT",
			  "type-sent" },
			{ CAMEL_FOLDER_TYPE_CONTACTS,
			  "CAMEL_FOLDER_TYPE_CONTACTS",
			  "type-contacts" },
			{ CAMEL_FOLDER_TYPE_EVENTS,
			  "CAMEL_FOLDER_TYPE_EVENTS",
			  "type-events" },
			{ CAMEL_FOLDER_TYPE_MEMOS,
			  "CAMEL_FOLDER_TYPE_MEMOS",
			  "type-memos" },
			{ CAMEL_FOLDER_TYPE_TASKS,
			  "CAMEL_FOLDER_TYPE_TASKS",
			  "type-tasks" },
			{ CAMEL_FOLDER_READONLY,
			  "CAMEL_FOLDER_READONLY",
			  "readonly" },
			{ CAMEL_FOLDER_CHECK_FOR_NEW,
			  "CAMEL_FOLDER_CHECK_FOR_NEW",
			  "check-for-new" },
			{ CAMEL_FOLDER_FLAGGED,
			  "CAMEL_FOLDER_FLAGGED",
			  "flagged" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelFolderInfoFlags"),
			values);
	}
	return the_type;
}

GType
camel_store_info_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_STORE_INFO_FOLDER_NOSELECT,
			  "CAMEL_STORE_INFO_FOLDER_NOSELECT",
			  "noselect" },
			{ CAMEL_STORE_INFO_FOLDER_NOINFERIORS,
			  "CAMEL_STORE_INFO_FOLDER_NOINFERIORS",
			  "noinferiors" },
			{ CAMEL_STORE_INFO_FOLDER_CHILDREN,
			  "CAMEL_STORE_INFO_FOLDER_CHILDREN",
			  "children" },
			{ CAMEL_STORE_INFO_FOLDER_NOCHILDREN,
			  "CAMEL_STORE_INFO_FOLDER_NOCHILDREN",
			  "nochildren" },
			{ CAMEL_STORE_INFO_FOLDER_SUBSCRIBED,
			  "CAMEL_STORE_INFO_FOLDER_SUBSCRIBED",
			  "subscribed" },
			{ CAMEL_STORE_INFO_FOLDER_VIRTUAL,
			  "CAMEL_STORE_INFO_FOLDER_VIRTUAL",
			  "virtual" },
			{ CAMEL_STORE_INFO_FOLDER_SYSTEM,
			  "CAMEL_STORE_INFO_FOLDER_SYSTEM",
			  "system" },
			{ CAMEL_STORE_INFO_FOLDER_VTRASH,
			  "CAMEL_STORE_INFO_FOLDER_VTRASH",
			  "vtrash" },
			{ CAMEL_STORE_INFO_FOLDER_SHARED_TO_ME,
			  "CAMEL_STORE_INFO_FOLDER_SHARED_TO_ME",
			  "shared-to-me" },
			{ CAMEL_STORE_INFO_FOLDER_SHARED_BY_ME,
			  "CAMEL_STORE_INFO_FOLDER_SHARED_BY_ME",
			  "shared-by-me" },
			{ CAMEL_STORE_INFO_FOLDER_READONLY,
			  "CAMEL_STORE_INFO_FOLDER_READONLY",
			  "readonly" },
			{ CAMEL_STORE_INFO_FOLDER_CHECK_FOR_NEW,
			  "CAMEL_STORE_INFO_FOLDER_CHECK_FOR_NEW",
			  "check-for-new" },
			{ CAMEL_STORE_INFO_FOLDER_FLAGGED,
			  "CAMEL_STORE_INFO_FOLDER_FLAGGED",
			  "flagged" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelStoreInfoFlags"),
			values);
	}
	return the_type;
}

GType
camel_fetch_headers_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_FETCH_HEADERS_BASIC,
			  "CAMEL_FETCH_HEADERS_BASIC",
			  "basic" },
			{ CAMEL_FETCH_HEADERS_BASIC_AND_MAILING_LIST,
			  "CAMEL_FETCH_HEADERS_BASIC_AND_MAILING_LIST",
			  "basic-and-mailing-list" },
			{ CAMEL_FETCH_HEADERS_ALL,
			  "CAMEL_FETCH_HEADERS_ALL",
			  "all" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelFetchHeadersType"),
			values);
	}
	return the_type;
}

GType
camel_junk_status_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_JUNK_STATUS_INCONCLUSIVE,
			  "CAMEL_JUNK_STATUS_INCONCLUSIVE",
			  "inconclusive" },
			{ CAMEL_JUNK_STATUS_MESSAGE_IS_JUNK,
			  "CAMEL_JUNK_STATUS_MESSAGE_IS_JUNK",
			  "message-is-junk" },
			{ CAMEL_JUNK_STATUS_MESSAGE_IS_NOT_JUNK,
			  "CAMEL_JUNK_STATUS_MESSAGE_IS_NOT_JUNK",
			  "message-is-not-junk" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelJunkStatus"),
			values);
	}
	return the_type;
}

GType
camel_mime_filter_basic_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_MIME_FILTER_BASIC_INVALID,
			  "CAMEL_MIME_FILTER_BASIC_INVALID",
			  "invalid" },
			{ CAMEL_MIME_FILTER_BASIC_BASE64_ENC,
			  "CAMEL_MIME_FILTER_BASIC_BASE64_ENC",
			  "base64-enc" },
			{ CAMEL_MIME_FILTER_BASIC_BASE64_DEC,
			  "CAMEL_MIME_FILTER_BASIC_BASE64_DEC",
			  "base64-dec" },
			{ CAMEL_MIME_FILTER_BASIC_QP_ENC,
			  "CAMEL_MIME_FILTER_BASIC_QP_ENC",
			  "qp-enc" },
			{ CAMEL_MIME_FILTER_BASIC_QP_DEC,
			  "CAMEL_MIME_FILTER_BASIC_QP_DEC",
			  "qp-dec" },
			{ CAMEL_MIME_FILTER_BASIC_UU_ENC,
			  "CAMEL_MIME_FILTER_BASIC_UU_ENC",
			  "uu-enc" },
			{ CAMEL_MIME_FILTER_BASIC_UU_DEC,
			  "CAMEL_MIME_FILTER_BASIC_UU_DEC",
			  "uu-dec" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelMimeFilterBasicType"),
			values);
	}
	return the_type;
}

GType
camel_mime_filter_crlf_direction_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_MIME_FILTER_CRLF_ENCODE,
			  "CAMEL_MIME_FILTER_CRLF_ENCODE",
			  "encode" },
			{ CAMEL_MIME_FILTER_CRLF_DECODE,
			  "CAMEL_MIME_FILTER_CRLF_DECODE",
			  "decode" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelMimeFilterCRLFDirection"),
			values);
	}
	return the_type;
}

GType
camel_mime_filter_crlf_mode_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_MIME_FILTER_CRLF_MODE_CRLF_DOTS,
			  "CAMEL_MIME_FILTER_CRLF_MODE_CRLF_DOTS",
			  "dots" },
			{ CAMEL_MIME_FILTER_CRLF_MODE_CRLF_ONLY,
			  "CAMEL_MIME_FILTER_CRLF_MODE_CRLF_ONLY",
			  "only" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelMimeFilterCRLFMode"),
			values);
	}
	return the_type;
}

GType
camel_mime_filter_gzip_mode_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_MIME_FILTER_GZIP_MODE_ZIP,
			  "CAMEL_MIME_FILTER_GZIP_MODE_ZIP",
			  "zip" },
			{ CAMEL_MIME_FILTER_GZIP_MODE_UNZIP,
			  "CAMEL_MIME_FILTER_GZIP_MODE_UNZIP",
			  "unzip" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelMimeFilterGZipMode"),
			values);
	}
	return the_type;
}

GType
camel_mime_filter_yenc_direction_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_MIME_FILTER_YENC_DIRECTION_ENCODE,
			  "CAMEL_MIME_FILTER_YENC_DIRECTION_ENCODE",
			  "encode" },
			{ CAMEL_MIME_FILTER_YENC_DIRECTION_DECODE,
			  "CAMEL_MIME_FILTER_YENC_DIRECTION_DECODE",
			  "decode" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelMimeFilterYencDirection"),
			values);
	}
	return the_type;
}

GType
camel_network_security_method_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_NETWORK_SECURITY_METHOD_NONE,
			  "CAMEL_NETWORK_SECURITY_METHOD_NONE",
			  "none" },
			{ CAMEL_NETWORK_SECURITY_METHOD_SSL_ON_ALTERNATE_PORT,
			  "CAMEL_NETWORK_SECURITY_METHOD_SSL_ON_ALTERNATE_PORT",
			  "ssl-on-alternate-port" },
			{ CAMEL_NETWORK_SECURITY_METHOD_STARTTLS_ON_STANDARD_PORT,
			  "CAMEL_NETWORK_SECURITY_METHOD_STARTTLS_ON_STANDARD_PORT",
			  "starttls-on-standard-port" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelNetworkSecurityMethod"),
			values);
	}
	return the_type;
}

GType
camel_provider_conf_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_PROVIDER_CONF_END,
			  "CAMEL_PROVIDER_CONF_END",
			  "end" },
			{ CAMEL_PROVIDER_CONF_SECTION_START,
			  "CAMEL_PROVIDER_CONF_SECTION_START",
			  "section-start" },
			{ CAMEL_PROVIDER_CONF_SECTION_END,
			  "CAMEL_PROVIDER_CONF_SECTION_END",
			  "section-end" },
			{ CAMEL_PROVIDER_CONF_CHECKBOX,
			  "CAMEL_PROVIDER_CONF_CHECKBOX",
			  "checkbox" },
			{ CAMEL_PROVIDER_CONF_CHECKSPIN,
			  "CAMEL_PROVIDER_CONF_CHECKSPIN",
			  "checkspin" },
			{ CAMEL_PROVIDER_CONF_ENTRY,
			  "CAMEL_PROVIDER_CONF_ENTRY",
			  "entry" },
			{ CAMEL_PROVIDER_CONF_LABEL,
			  "CAMEL_PROVIDER_CONF_LABEL",
			  "label" },
			{ CAMEL_PROVIDER_CONF_HIDDEN,
			  "CAMEL_PROVIDER_CONF_HIDDEN",
			  "hidden" },
			{ CAMEL_PROVIDER_CONF_OPTIONS,
			  "CAMEL_PROVIDER_CONF_OPTIONS",
			  "options" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelProviderConfType"),
			values);
	}
	return the_type;
}

GType
camel_provider_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_PROVIDER_IS_REMOTE,
			  "CAMEL_PROVIDER_IS_REMOTE",
			  "is-remote" },
			{ CAMEL_PROVIDER_IS_LOCAL,
			  "CAMEL_PROVIDER_IS_LOCAL",
			  "is-local" },
			{ CAMEL_PROVIDER_IS_EXTERNAL,
			  "CAMEL_PROVIDER_IS_EXTERNAL",
			  "is-external" },
			{ CAMEL_PROVIDER_IS_SOURCE,
			  "CAMEL_PROVIDER_IS_SOURCE",
			  "is-source" },
			{ CAMEL_PROVIDER_IS_STORAGE,
			  "CAMEL_PROVIDER_IS_STORAGE",
			  "is-storage" },
			{ CAMEL_PROVIDER_SUPPORTS_SSL,
			  "CAMEL_PROVIDER_SUPPORTS_SSL",
			  "supports-ssl" },
			{ CAMEL_PROVIDER_HAS_LICENSE,
			  "CAMEL_PROVIDER_HAS_LICENSE",
			  "has-license" },
			{ CAMEL_PROVIDER_DISABLE_SENT_FOLDER,
			  "CAMEL_PROVIDER_DISABLE_SENT_FOLDER",
			  "disable-sent-folder" },
			{ CAMEL_PROVIDER_ALLOW_REAL_TRASH_FOLDER,
			  "CAMEL_PROVIDER_ALLOW_REAL_TRASH_FOLDER",
			  "allow-real-trash-folder" },
			{ CAMEL_PROVIDER_ALLOW_REAL_JUNK_FOLDER,
			  "CAMEL_PROVIDER_ALLOW_REAL_JUNK_FOLDER",
			  "allow-real-junk-folder" },
			{ CAMEL_PROVIDER_SUPPORTS_MOBILE_DEVICES,
			  "CAMEL_PROVIDER_SUPPORTS_MOBILE_DEVICES",
			  "supports-mobile-devices" },
			{ CAMEL_PROVIDER_SUPPORTS_BATCH_FETCH,
			  "CAMEL_PROVIDER_SUPPORTS_BATCH_FETCH",
			  "supports-batch-fetch" },
			{ CAMEL_PROVIDER_SUPPORTS_PURGE_MESSAGE_CACHE,
			  "CAMEL_PROVIDER_SUPPORTS_PURGE_MESSAGE_CACHE",
			  "supports-purge-message-cache" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelProviderFlags"),
			values);
	}
	return the_type;
}

GType
camel_provider_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_PROVIDER_STORE,
			  "CAMEL_PROVIDER_STORE",
			  "store" },
			{ CAMEL_PROVIDER_TRANSPORT,
			  "CAMEL_PROVIDER_TRANSPORT",
			  "transport" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelProviderType"),
			values);
	}
	return the_type;
}

GType
camel_sasl_anon_trace_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_SASL_ANON_TRACE_EMAIL,
			  "CAMEL_SASL_ANON_TRACE_EMAIL",
			  "email" },
			{ CAMEL_SASL_ANON_TRACE_OPAQUE,
			  "CAMEL_SASL_ANON_TRACE_OPAQUE",
			  "opaque" },
			{ CAMEL_SASL_ANON_TRACE_EMPTY,
			  "CAMEL_SASL_ANON_TRACE_EMPTY",
			  "empty" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelSaslAnonTraceType"),
			values);
	}
	return the_type;
}

GType
camel_service_connection_status_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_SERVICE_DISCONNECTED,
			  "CAMEL_SERVICE_DISCONNECTED",
			  "disconnected" },
			{ CAMEL_SERVICE_CONNECTING,
			  "CAMEL_SERVICE_CONNECTING",
			  "connecting" },
			{ CAMEL_SERVICE_CONNECTED,
			  "CAMEL_SERVICE_CONNECTED",
			  "connected" },
			{ CAMEL_SERVICE_DISCONNECTING,
			  "CAMEL_SERVICE_DISCONNECTING",
			  "disconnecting" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelServiceConnectionStatus"),
			values);
	}
	return the_type;
}

GType
camel_session_alert_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_SESSION_ALERT_INFO,
			  "CAMEL_SESSION_ALERT_INFO",
			  "info" },
			{ CAMEL_SESSION_ALERT_WARNING,
			  "CAMEL_SESSION_ALERT_WARNING",
			  "warning" },
			{ CAMEL_SESSION_ALERT_ERROR,
			  "CAMEL_SESSION_ALERT_ERROR",
			  "error" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelSessionAlertType"),
			values);
	}
	return the_type;
}

GType
camel_sort_type_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_SORT_ASCENDING,
			  "CAMEL_SORT_ASCENDING",
			  "ascending" },
			{ CAMEL_SORT_DESCENDING,
			  "CAMEL_SORT_DESCENDING",
			  "descending" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelSortType"),
			values);
	}
	return the_type;
}

GType
camel_store_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_STORE_VTRASH,
			  "CAMEL_STORE_VTRASH",
			  "vtrash" },
			{ CAMEL_STORE_VJUNK,
			  "CAMEL_STORE_VJUNK",
			  "vjunk" },
			{ CAMEL_STORE_PROXY,
			  "CAMEL_STORE_PROXY",
			  "proxy" },
			{ CAMEL_STORE_IS_MIGRATING,
			  "CAMEL_STORE_IS_MIGRATING",
			  "is-migrating" },
			{ CAMEL_STORE_REAL_JUNK_FOLDER,
			  "CAMEL_STORE_REAL_JUNK_FOLDER",
			  "real-junk-folder" },
			{ CAMEL_STORE_CAN_EDIT_FOLDERS,
			  "CAMEL_STORE_CAN_EDIT_FOLDERS",
			  "can-edit-folders" },
			{ CAMEL_STORE_USE_CACHE_DIR,
			  "CAMEL_STORE_USE_CACHE_DIR",
			  "use-cache-dir" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelStoreFlags"),
			values);
	}
	return the_type;
}

GType
camel_store_get_folder_info_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_STORE_FOLDER_INFO_FAST,
			  "CAMEL_STORE_FOLDER_INFO_FAST",
			  "fast" },
			{ CAMEL_STORE_FOLDER_INFO_RECURSIVE,
			  "CAMEL_STORE_FOLDER_INFO_RECURSIVE",
			  "recursive" },
			{ CAMEL_STORE_FOLDER_INFO_SUBSCRIBED,
			  "CAMEL_STORE_FOLDER_INFO_SUBSCRIBED",
			  "subscribed" },
			{ CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL,
			  "CAMEL_STORE_FOLDER_INFO_NO_VIRTUAL",
			  "no-virtual" },
			{ CAMEL_STORE_FOLDER_INFO_SUBSCRIPTION_LIST,
			  "CAMEL_STORE_FOLDER_INFO_SUBSCRIPTION_LIST",
			  "subscription-list" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelStoreGetFolderInfoFlags"),
			values);
	}
	return the_type;
}

GType
camel_store_permission_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_STORE_READ,
			  "CAMEL_STORE_READ",
			  "read" },
			{ CAMEL_STORE_WRITE,
			  "CAMEL_STORE_WRITE",
			  "write" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelStorePermissionFlags"),
			values);
	}
	return the_type;
}

GType
camel_tcp_stream_ssl_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ CAMEL_TCP_STREAM_SSL_ENABLE_SSL2,
			  "CAMEL_TCP_STREAM_SSL_ENABLE_SSL2",
			  "ssl2" },
			{ CAMEL_TCP_STREAM_SSL_ENABLE_SSL3,
			  "CAMEL_TCP_STREAM_SSL_ENABLE_SSL3",
			  "ssl3" },
			{ CAMEL_TCP_STREAM_SSL_ENABLE_TLS,
			  "CAMEL_TCP_STREAM_SSL_ENABLE_TLS",
			  "tls" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("CamelTcpStreamSSLFlags"),
			values);
	}
	return the_type;
}

GType
camel_transfer_encoding_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_TRANSFER_ENCODING_DEFAULT,
			  "CAMEL_TRANSFER_ENCODING_DEFAULT",
			  "encoding-default" },
			{ CAMEL_TRANSFER_ENCODING_7BIT,
			  "CAMEL_TRANSFER_ENCODING_7BIT",
			  "encoding-7bit" },
			{ CAMEL_TRANSFER_ENCODING_8BIT,
			  "CAMEL_TRANSFER_ENCODING_8BIT",
			  "encoding-8bit" },
			{ CAMEL_TRANSFER_ENCODING_BASE64,
			  "CAMEL_TRANSFER_ENCODING_BASE64",
			  "encoding-base64" },
			{ CAMEL_TRANSFER_ENCODING_QUOTEDPRINTABLE,
			  "CAMEL_TRANSFER_ENCODING_QUOTEDPRINTABLE",
			  "encoding-quotedprintable" },
			{ CAMEL_TRANSFER_ENCODING_BINARY,
			  "CAMEL_TRANSFER_ENCODING_BINARY",
			  "encoding-binary" },
			{ CAMEL_TRANSFER_ENCODING_UUENCODE,
			  "CAMEL_TRANSFER_ENCODING_UUENCODE",
			  "encoding-uuencode" },
			{ CAMEL_TRANSFER_NUM_ENCODINGS,
			  "CAMEL_TRANSFER_NUM_ENCODINGS",
			  "num-encodings" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelTransferEncoding"),
			values);
	}
	return the_type;
}

GType
camel_stream_vfs_open_method_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ CAMEL_STREAM_VFS_CREATE,
			  "CAMEL_STREAM_VFS_CREATE",
			  "create" },
			{ CAMEL_STREAM_VFS_APPEND,
			  "CAMEL_STREAM_VFS_APPEND",
			  "append" },
			{ CAMEL_STREAM_VFS_READ,
			  "CAMEL_STREAM_VFS_READ",
			  "read" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("CamelStreamVFSOpenMethod"),
			values);
	}
	return the_type;
}


/* Generated data ends here */

