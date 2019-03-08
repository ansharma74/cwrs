


#ifndef GDATA_YOUTUBE_ENUMS_H
#define GDATA_YOUTUBE_ENUMS_H

#include <glib-object.h>

G_BEGIN_DECLS
/* enumerations from "./gdata/services/youtube/gdata-youtube-service.h" */
GType gdata_youtube_standard_feed_type_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_STANDARD_FEED_TYPE (gdata_youtube_standard_feed_type_get_type())
GType gdata_youtube_service_error_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_SERVICE_ERROR (gdata_youtube_service_error_get_type())
/* enumerations from "./gdata/services/youtube/gdata-youtube-video.h" */
GType gdata_youtube_permission_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_PERMISSION (gdata_youtube_permission_get_type())
/* enumerations from "./gdata/services/youtube/gdata-youtube-content.h" */
GType gdata_youtube_format_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_FORMAT (gdata_youtube_format_get_type())
/* enumerations from "./gdata/services/youtube/gdata-youtube-query.h" */
GType gdata_youtube_safe_search_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_SAFE_SEARCH (gdata_youtube_safe_search_get_type())
GType gdata_youtube_sort_order_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_SORT_ORDER (gdata_youtube_sort_order_get_type())
GType gdata_youtube_age_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_AGE (gdata_youtube_age_get_type())
GType gdata_youtube_uploader_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_YOUTUBE_UPLOADER (gdata_youtube_uploader_get_type())
G_END_DECLS

#endif /* !GDATA_YOUTUBE_ENUMS_H */



