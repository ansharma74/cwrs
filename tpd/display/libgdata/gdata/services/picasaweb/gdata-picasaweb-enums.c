


#include "gdata-picasaweb-album.h"
#include "gdata-picasaweb-enums.h"

/* enumerations from "./gdata/services/picasaweb/gdata-picasaweb-album.h" */
GType
gdata_picasaweb_visibility_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_PICASAWEB_PUBLIC, "GDATA_PICASAWEB_PUBLIC", "public" },
      { GDATA_PICASAWEB_PRIVATE, "GDATA_PICASAWEB_PRIVATE", "private" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataPicasaWebVisibility", values);
  }
  return etype;
}



