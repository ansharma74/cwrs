
/* Generated data (by glib-mkenums) */

#include "e-backend-enumtypes.h"

/* enumerations from "e-backend-enums.h" */
#include "e-backend-enums.h"

GType
e_authentication_session_result_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ E_AUTHENTICATION_SESSION_ERROR,
			  "E_AUTHENTICATION_SESSION_ERROR",
			  "error" },
			{ E_AUTHENTICATION_SESSION_SUCCESS,
			  "E_AUTHENTICATION_SESSION_SUCCESS",
			  "success" },
			{ E_AUTHENTICATION_SESSION_DISMISSED,
			  "E_AUTHENTICATION_SESSION_DISMISSED",
			  "dismissed" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("EAuthenticationSessionResult"),
			values);
	}
	return the_type;
}

GType
e_dbus_server_exit_code_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ E_DBUS_SERVER_EXIT_NONE,
			  "E_DBUS_SERVER_EXIT_NONE",
			  "none" },
			{ E_DBUS_SERVER_EXIT_NORMAL,
			  "E_DBUS_SERVER_EXIT_NORMAL",
			  "normal" },
			{ E_DBUS_SERVER_EXIT_RELOAD,
			  "E_DBUS_SERVER_EXIT_RELOAD",
			  "reload" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("EDBusServerExitCode"),
			values);
	}
	return the_type;
}

GType
e_source_permission_flags_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GFlagsValue values[] = {
			{ E_SOURCE_PERMISSION_NONE,
			  "E_SOURCE_PERMISSION_NONE",
			  "none" },
			{ E_SOURCE_PERMISSION_WRITABLE,
			  "E_SOURCE_PERMISSION_WRITABLE",
			  "writable" },
			{ E_SOURCE_PERMISSION_REMOVABLE,
			  "E_SOURCE_PERMISSION_REMOVABLE",
			  "removable" },
			{ 0, NULL, NULL }
		};
		the_type = g_flags_register_static (
			g_intern_static_string ("ESourcePermissionFlags"),
			values);
	}
	return the_type;
}


/* Generated data ends here */

