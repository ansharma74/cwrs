


#include "gdata-youtube-service.h"
#include "gdata-youtube-query.h"
#include "gdata-youtube-enums.h"

/* enumerations from "./gdata/services/youtube/gdata-youtube-service.h" */
GType
gdata_youtube_standard_feed_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_TOP_RATED_FEED, "GDATA_YOUTUBE_TOP_RATED_FEED", "top-rated-feed" },
      { GDATA_YOUTUBE_TOP_FAVORITES_FEED, "GDATA_YOUTUBE_TOP_FAVORITES_FEED", "top-favorites-feed" },
      { GDATA_YOUTUBE_MOST_VIEWED_FEED, "GDATA_YOUTUBE_MOST_VIEWED_FEED", "most-viewed-feed" },
      { GDATA_YOUTUBE_MOST_POPULAR_FEED, "GDATA_YOUTUBE_MOST_POPULAR_FEED", "most-popular-feed" },
      { GDATA_YOUTUBE_MOST_RECENT_FEED, "GDATA_YOUTUBE_MOST_RECENT_FEED", "most-recent-feed" },
      { GDATA_YOUTUBE_MOST_DISCUSSED_FEED, "GDATA_YOUTUBE_MOST_DISCUSSED_FEED", "most-discussed-feed" },
      { GDATA_YOUTUBE_MOST_LINKED_FEED, "GDATA_YOUTUBE_MOST_LINKED_FEED", "most-linked-feed" },
      { GDATA_YOUTUBE_MOST_RESPONDED_FEED, "GDATA_YOUTUBE_MOST_RESPONDED_FEED", "most-responded-feed" },
      { GDATA_YOUTUBE_RECENTLY_FEATURED_FEED, "GDATA_YOUTUBE_RECENTLY_FEATURED_FEED", "recently-featured-feed" },
      { GDATA_YOUTUBE_WATCH_ON_MOBILE_FEED, "GDATA_YOUTUBE_WATCH_ON_MOBILE_FEED", "watch-on-mobile-feed" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeStandardFeedType", values);
  }
  return etype;
}
GType
gdata_youtube_service_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_SERVICE_ERROR_API_QUOTA_EXCEEDED, "GDATA_YOUTUBE_SERVICE_ERROR_API_QUOTA_EXCEEDED", "api-quota-exceeded" },
      { GDATA_YOUTUBE_SERVICE_ERROR_ENTRY_QUOTA_EXCEEDED, "GDATA_YOUTUBE_SERVICE_ERROR_ENTRY_QUOTA_EXCEEDED", "entry-quota-exceeded" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeServiceError", values);
  }
  return etype;
}

/* enumerations from "./gdata/services/youtube/gdata-youtube-video.h" */
GType
gdata_youtube_permission_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_PERMISSION_ALLOWED, "GDATA_YOUTUBE_PERMISSION_ALLOWED", "allowed" },
      { GDATA_YOUTUBE_PERMISSION_DENIED, "GDATA_YOUTUBE_PERMISSION_DENIED", "denied" },
      { GDATA_YOUTUBE_PERMISSION_MODERATED, "GDATA_YOUTUBE_PERMISSION_MODERATED", "moderated" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubePermission", values);
  }
  return etype;
}

/* enumerations from "./gdata/services/youtube/gdata-youtube-content.h" */
GType
gdata_youtube_format_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_FORMAT_UNKNOWN, "GDATA_YOUTUBE_FORMAT_UNKNOWN", "unknown" },
      { GDATA_YOUTUBE_FORMAT_RTSP_H263_AMR, "GDATA_YOUTUBE_FORMAT_RTSP_H263_AMR", "rtsp-h263-amr" },
      { GDATA_YOUTUBE_FORMAT_HTTP_SWF, "GDATA_YOUTUBE_FORMAT_HTTP_SWF", "http-swf" },
      { GDATA_YOUTUBE_FORMAT_RTSP_MPEG4_AAC, "GDATA_YOUTUBE_FORMAT_RTSP_MPEG4_AAC", "rtsp-mpeg4-aac" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeFormat", values);
  }
  return etype;
}

/* enumerations from "./gdata/services/youtube/gdata-youtube-query.h" */
GType
gdata_youtube_safe_search_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_SAFE_SEARCH_NONE, "GDATA_YOUTUBE_SAFE_SEARCH_NONE", "none" },
      { GDATA_YOUTUBE_SAFE_SEARCH_MODERATE, "GDATA_YOUTUBE_SAFE_SEARCH_MODERATE", "moderate" },
      { GDATA_YOUTUBE_SAFE_SEARCH_STRICT, "GDATA_YOUTUBE_SAFE_SEARCH_STRICT", "strict" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeSafeSearch", values);
  }
  return etype;
}
GType
gdata_youtube_sort_order_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_SORT_NONE, "GDATA_YOUTUBE_SORT_NONE", "none" },
      { GDATA_YOUTUBE_SORT_ASCENDING, "GDATA_YOUTUBE_SORT_ASCENDING", "ascending" },
      { GDATA_YOUTUBE_SORT_DESCENDING, "GDATA_YOUTUBE_SORT_DESCENDING", "descending" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeSortOrder", values);
  }
  return etype;
}
GType
gdata_youtube_age_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_AGE_ALL_TIME, "GDATA_YOUTUBE_AGE_ALL_TIME", "all-time" },
      { GDATA_YOUTUBE_AGE_TODAY, "GDATA_YOUTUBE_AGE_TODAY", "today" },
      { GDATA_YOUTUBE_AGE_THIS_WEEK, "GDATA_YOUTUBE_AGE_THIS_WEEK", "this-week" },
      { GDATA_YOUTUBE_AGE_THIS_MONTH, "GDATA_YOUTUBE_AGE_THIS_MONTH", "this-month" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeAge", values);
  }
  return etype;
}
GType
gdata_youtube_uploader_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_YOUTUBE_UPLOADER_ALL, "GDATA_YOUTUBE_UPLOADER_ALL", "all" },
      { GDATA_YOUTUBE_UPLOADER_PARTNER, "GDATA_YOUTUBE_UPLOADER_PARTNER", "partner" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataYouTubeUploader", values);
  }
  return etype;
}



