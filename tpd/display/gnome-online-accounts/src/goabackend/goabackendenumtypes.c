
/* Generated data (by glib-mkenums) */

#include "goabackendenums.h"
#include "goabackendenumtypes.h"

/* enumerations from "goabackendenums.h" */
GType
goa_log_level_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { GOA_LOG_LEVEL_DEBUG, "GOA_LOG_LEVEL_DEBUG", "debug" },
        { GOA_LOG_LEVEL_INFO, "GOA_LOG_LEVEL_INFO", "info" },
        { GOA_LOG_LEVEL_NOTICE, "GOA_LOG_LEVEL_NOTICE", "notice" },
        { GOA_LOG_LEVEL_WARNING, "GOA_LOG_LEVEL_WARNING", "warning" },
        { GOA_LOG_LEVEL_ERROR, "GOA_LOG_LEVEL_ERROR", "error" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GoaLogLevel"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

