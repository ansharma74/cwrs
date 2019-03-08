


#include "gdata-documents-service.h"
#include "gdata-documents-text.h"
#include "gdata-documents-spreadsheet.h"
#include "gdata-documents-drawing.h"
#include "gdata-documents-presentation.h"
#include "gdata-documents-enums.h"

/* enumerations from "./gdata/services/documents/gdata-documents-service.h" */
GType
gdata_documents_service_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_DOCUMENTS_SERVICE_ERROR_INVALID_CONTENT_TYPE, "GDATA_DOCUMENTS_SERVICE_ERROR_INVALID_CONTENT_TYPE", "type" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataDocumentsServiceError", values);
  }
  return etype;
}



