
/* Generated data (by glib-mkenums) */

#include "goaenums.h"
#include "goaenumtypes.h"

/* enumerations from "goaenums.h" */
GType
goa_error_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { GOA_ERROR_FAILED, "GOA_ERROR_FAILED", "failed" },
        { GOA_ERROR_NOT_SUPPORTED, "GOA_ERROR_NOT_SUPPORTED", "not-supported" },
        { GOA_ERROR_DIALOG_DISMISSED, "GOA_ERROR_DIALOG_DISMISSED", "dialog-dismissed" },
        { GOA_ERROR_ACCOUNT_EXISTS, "GOA_ERROR_ACCOUNT_EXISTS", "account-exists" },
        { GOA_ERROR_NOT_AUTHORIZED, "GOA_ERROR_NOT_AUTHORIZED", "not-authorized" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GoaError"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

