
/* Generated data (by glib-mkenums) */


#include <glib-object.h>

#include "goaidentity.h"
/* enumerations from "goaidentity.h" */
GType goa_identity_error_get_type (void) G_GNUC_CONST;

GType
goa_identity_error_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GEnumValue values[] = {
                { GOA_IDENTITY_ERROR_NOT_FOUND, "GOA_IDENTITY_ERROR_NOT_FOUND", "not-found" },
                { GOA_IDENTITY_ERROR_VERIFYING, "GOA_IDENTITY_ERROR_VERIFYING", "verifying" },
                { GOA_IDENTITY_ERROR_RENEWING, "GOA_IDENTITY_ERROR_RENEWING", "renewing" },
                { GOA_IDENTITY_ERROR_CREDENTIALS_UNAVAILABLE, "GOA_IDENTITY_ERROR_CREDENTIALS_UNAVAILABLE", "credentials-unavailable" },
                { GOA_IDENTITY_ERROR_ENUMERATING_CREDENTIALS, "GOA_IDENTITY_ERROR_ENUMERATING_CREDENTIALS", "enumerating-credentials" },
                { GOA_IDENTITY_ERROR_ALLOCATING_CREDENTIALS, "GOA_IDENTITY_ERROR_ALLOCATING_CREDENTIALS", "allocating-credentials" },
                { GOA_IDENTITY_ERROR_AUTHENTICATION_FAILED, "GOA_IDENTITY_ERROR_AUTHENTICATION_FAILED", "authentication-failed" },
                { GOA_IDENTITY_ERROR_SAVING_CREDENTIALS, "GOA_IDENTITY_ERROR_SAVING_CREDENTIALS", "saving-credentials" },
                { GOA_IDENTITY_ERROR_REMOVING_CREDENTIALS, "GOA_IDENTITY_ERROR_REMOVING_CREDENTIALS", "removing-credentials" },
                { GOA_IDENTITY_ERROR_PARSING_IDENTIFIER, "GOA_IDENTITY_ERROR_PARSING_IDENTIFIER", "parsing-identifier" },
                { 0, NULL, NULL }
        };

        etype = g_enum_register_static (g_intern_static_string ("GoaIdentityError"), values);
    }

    return etype;
}

GType goa_identity_sign_in_flags_get_type (void) G_GNUC_CONST;

GType
goa_identity_sign_in_flags_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GFlagsValue values[] = {
                { GOA_IDENTITY_SIGN_IN_FLAGS_NONE, "GOA_IDENTITY_SIGN_IN_FLAGS_NONE", "none" },
                { GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_RENEWAL, "GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_RENEWAL", "disallow-renewal" },
                { GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_FORWARDING, "GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_FORWARDING", "disallow-forwarding" },
                { GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_PROXYING, "GOA_IDENTITY_SIGN_IN_FLAGS_DISALLOW_PROXYING", "disallow-proxying" },
                { 0, NULL, NULL }
        };

        etype = g_flags_register_static (g_intern_static_string ("GoaIdentitySignInFlags"), values);
    }

    return etype;
}

#include "goaidentityinquiry.h"
/* enumerations from "goaidentityinquiry.h" */
GType goa_identity_query_mode_get_type (void) G_GNUC_CONST;

GType
goa_identity_query_mode_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GEnumValue values[] = {
                { GOA_IDENTITY_QUERY_MODE_INVISIBLE, "GOA_IDENTITY_QUERY_MODE_INVISIBLE", "invisible" },
                { GOA_IDENTITY_QUERY_MODE_VISIBLE, "GOA_IDENTITY_QUERY_MODE_VISIBLE", "visible" },
                { 0, NULL, NULL }
        };

        etype = g_enum_register_static (g_intern_static_string ("GoaIdentityQueryMode"), values);
    }

    return etype;
}

#include "goaidentitymanager.h"
/* enumerations from "goaidentitymanager.h" */
GType goa_identity_manager_error_get_type (void) G_GNUC_CONST;

GType
goa_identity_manager_error_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GEnumValue values[] = {
                { GOA_IDENTITY_MANAGER_ERROR_INITIALIZING, "GOA_IDENTITY_MANAGER_ERROR_INITIALIZING", "initializing" },
                { GOA_IDENTITY_MANAGER_ERROR_IDENTITY_NOT_FOUND, "GOA_IDENTITY_MANAGER_ERROR_IDENTITY_NOT_FOUND", "identity-not-found" },
                { GOA_IDENTITY_MANAGER_ERROR_CREATING_IDENTITY, "GOA_IDENTITY_MANAGER_ERROR_CREATING_IDENTITY", "creating-identity" },
                { GOA_IDENTITY_MANAGER_ERROR_ACCESSING_CREDENTIALS, "GOA_IDENTITY_MANAGER_ERROR_ACCESSING_CREDENTIALS", "accessing-credentials" },
                { GOA_IDENTITY_MANAGER_ERROR_UNSUPPORTED_CREDENTIALS, "GOA_IDENTITY_MANAGER_ERROR_UNSUPPORTED_CREDENTIALS", "unsupported-credentials" },
                { 0, NULL, NULL }
        };

        etype = g_enum_register_static (g_intern_static_string ("GoaIdentityManagerError"), values);
    }

    return etype;
}

#include "goakerberosidentity.h"
/* enumerations from "goakerberosidentity.h" */
GType struct_struct_get_type (void) G_GNUC_CONST;

GType
struct_struct_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GEnumValue values[] = {
                { GOA_KERBEROS_IDENTITY_DESCRIPTION_REALM, "GOA_KERBEROS_IDENTITY_DESCRIPTION_REALM", "realm" },
                { GOA_KERBEROS_IDENTITY_DESCRIPTION_USERNAME_AND_REALM, "GOA_KERBEROS_IDENTITY_DESCRIPTION_USERNAME_AND_REALM", "username-and-realm" },
                { GOA_KERBEROS_IDENTITY_DESCRIPTION_USERNAME_ROLE_AND_REALM, "GOA_KERBEROS_IDENTITY_DESCRIPTION_USERNAME_ROLE_AND_REALM", "username-role-and-realm" },
                { 0, NULL, NULL }
        };

        etype = g_enum_register_static (g_intern_static_string ("struct"), values);
    }

    return etype;
}

#include "goakerberosidentityinquiry.h"
/* enumerations from "goakerberosidentityinquiry.h" */
GType goa_kerberos_identity_query_mode_get_type (void) G_GNUC_CONST;

GType
goa_kerberos_identity_query_mode_get_type (void)
{
        static GType etype = 0;

        if (G_UNLIKELY(etype == 0)) {
                static const GEnumValue values[] = {
                { GOA_KERBEROS_IDENTITY_QUERY_MODE_INVISIBLE, "GOA_KERBEROS_IDENTITY_QUERY_MODE_INVISIBLE", "invisible" },
                { GOA_KERBEROS_IDENTITY_QUERY_MODE_VISIBLE, "GOA_KERBEROS_IDENTITY_QUERY_MODE_VISIBLE", "visible" },
                { 0, NULL, NULL }
        };

        etype = g_enum_register_static (g_intern_static_string ("GoaKerberosIdentityQueryMode"), values);
    }

    return etype;
}

 /**/

/* Generated data ends here */

