
/* Generated data (by glib-mkenums) */

#include "e-source-enumtypes.h"

/* enumerations from "e-source-enums.h" */
#include "e-source-enums.h"

GType
e_mdn_response_policy_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ E_MDN_RESPONSE_POLICY_NEVER,
			  "E_MDN_RESPONSE_POLICY_NEVER",
			  "never" },
			{ E_MDN_RESPONSE_POLICY_ALWAYS,
			  "E_MDN_RESPONSE_POLICY_ALWAYS",
			  "always" },
			{ E_MDN_RESPONSE_POLICY_ASK,
			  "E_MDN_RESPONSE_POLICY_ASK",
			  "ask" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("EMdnResponsePolicy"),
			values);
	}
	return the_type;
}

GType
e_source_authentication_result_get_type (void)
{
	static GType the_type = 0;
	
	if (the_type == 0) {
		static const GEnumValue values[] = {
			{ E_SOURCE_AUTHENTICATION_ERROR,
			  "E_SOURCE_AUTHENTICATION_ERROR",
			  "error" },
			{ E_SOURCE_AUTHENTICATION_ACCEPTED,
			  "E_SOURCE_AUTHENTICATION_ACCEPTED",
			  "accepted" },
			{ E_SOURCE_AUTHENTICATION_REJECTED,
			  "E_SOURCE_AUTHENTICATION_REJECTED",
			  "rejected" },
			{ 0, NULL, NULL }
		};
		the_type = g_enum_register_static (
			g_intern_static_string ("ESourceAuthenticationResult"),
			values);
	}
	return the_type;
}


/* Generated data ends here */

