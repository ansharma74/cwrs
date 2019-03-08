


#include "gdata-media-content.h"
#include "gdata-media-enums.h"

/* enumerations from "./gdata/media/gdata-media-content.h" */
GType
gdata_media_expression_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_MEDIA_EXPRESSION_SAMPLE, "GDATA_MEDIA_EXPRESSION_SAMPLE", "sample" },
      { GDATA_MEDIA_EXPRESSION_FULL, "GDATA_MEDIA_EXPRESSION_FULL", "full" },
      { GDATA_MEDIA_EXPRESSION_NONSTOP, "GDATA_MEDIA_EXPRESSION_NONSTOP", "nonstop" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataMediaExpression", values);
  }
  return etype;
}
GType
gdata_media_medium_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_MEDIA_UNKNOWN, "GDATA_MEDIA_UNKNOWN", "unknown" },
      { GDATA_MEDIA_IMAGE, "GDATA_MEDIA_IMAGE", "image" },
      { GDATA_MEDIA_AUDIO, "GDATA_MEDIA_AUDIO", "audio" },
      { GDATA_MEDIA_VIDEO, "GDATA_MEDIA_VIDEO", "video" },
      { GDATA_MEDIA_DOCUMENT, "GDATA_MEDIA_DOCUMENT", "document" },
      { GDATA_MEDIA_EXECUTABLE, "GDATA_MEDIA_EXECUTABLE", "executable" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataMediaMedium", values);
  }
  return etype;
}



