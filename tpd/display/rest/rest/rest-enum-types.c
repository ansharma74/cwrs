


#include "rest-proxy.h"
#include "rest-proxy-call.h"
#include "rest-enum-types.h"

/* enumerations from "rest-proxy.h" */
GType
rest_proxy_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { REST_PROXY_ERROR_CANCELLED, "REST_PROXY_ERROR_CANCELLED", "cancelled" },
      { REST_PROXY_ERROR_RESOLUTION, "REST_PROXY_ERROR_RESOLUTION", "resolution" },
      { REST_PROXY_ERROR_CONNECTION, "REST_PROXY_ERROR_CONNECTION", "connection" },
      { REST_PROXY_ERROR_SSL, "REST_PROXY_ERROR_SSL", "ssl" },
      { REST_PROXY_ERROR_IO, "REST_PROXY_ERROR_IO", "io" },
      { REST_PROXY_ERROR_FAILED, "REST_PROXY_ERROR_FAILED", "failed" },
      { REST_PROXY_ERROR_HTTP_MULTIPLE_CHOICES, "REST_PROXY_ERROR_HTTP_MULTIPLE_CHOICES", "http-multiple-choices" },
      { REST_PROXY_ERROR_HTTP_MOVED_PERMANENTLY, "REST_PROXY_ERROR_HTTP_MOVED_PERMANENTLY", "http-moved-permanently" },
      { REST_PROXY_ERROR_HTTP_FOUND, "REST_PROXY_ERROR_HTTP_FOUND", "http-found" },
      { REST_PROXY_ERROR_HTTP_SEE_OTHER, "REST_PROXY_ERROR_HTTP_SEE_OTHER", "http-see-other" },
      { REST_PROXY_ERROR_HTTP_NOT_MODIFIED, "REST_PROXY_ERROR_HTTP_NOT_MODIFIED", "http-not-modified" },
      { REST_PROXY_ERROR_HTTP_USE_PROXY, "REST_PROXY_ERROR_HTTP_USE_PROXY", "http-use-proxy" },
      { REST_PROXY_ERROR_HTTP_THREEOHSIX, "REST_PROXY_ERROR_HTTP_THREEOHSIX", "http-threeohsix" },
      { REST_PROXY_ERROR_HTTP_TEMPORARY_REDIRECT, "REST_PROXY_ERROR_HTTP_TEMPORARY_REDIRECT", "http-temporary-redirect" },
      { REST_PROXY_ERROR_HTTP_BAD_REQUEST, "REST_PROXY_ERROR_HTTP_BAD_REQUEST", "http-bad-request" },
      { REST_PROXY_ERROR_HTTP_UNAUTHORIZED, "REST_PROXY_ERROR_HTTP_UNAUTHORIZED", "http-unauthorized" },
      { REST_PROXY_ERROR_HTTP_FOUROHTWO, "REST_PROXY_ERROR_HTTP_FOUROHTWO", "http-fourohtwo" },
      { REST_PROXY_ERROR_HTTP_FORBIDDEN, "REST_PROXY_ERROR_HTTP_FORBIDDEN", "http-forbidden" },
      { REST_PROXY_ERROR_HTTP_NOT_FOUND, "REST_PROXY_ERROR_HTTP_NOT_FOUND", "http-not-found" },
      { REST_PROXY_ERROR_HTTP_METHOD_NOT_ALLOWED, "REST_PROXY_ERROR_HTTP_METHOD_NOT_ALLOWED", "http-method-not-allowed" },
      { REST_PROXY_ERROR_HTTP_NOT_ACCEPTABLE, "REST_PROXY_ERROR_HTTP_NOT_ACCEPTABLE", "http-not-acceptable" },
      { REST_PROXY_ERROR_HTTP_PROXY_AUTHENTICATION_REQUIRED, "REST_PROXY_ERROR_HTTP_PROXY_AUTHENTICATION_REQUIRED", "http-proxy-authentication-required" },
      { REST_PROXY_ERROR_HTTP_REQUEST_TIMEOUT, "REST_PROXY_ERROR_HTTP_REQUEST_TIMEOUT", "http-request-timeout" },
      { REST_PROXY_ERROR_HTTP_CONFLICT, "REST_PROXY_ERROR_HTTP_CONFLICT", "http-conflict" },
      { REST_PROXY_ERROR_HTTP_GONE, "REST_PROXY_ERROR_HTTP_GONE", "http-gone" },
      { REST_PROXY_ERROR_HTTP_LENGTH_REQUIRED, "REST_PROXY_ERROR_HTTP_LENGTH_REQUIRED", "http-length-required" },
      { REST_PROXY_ERROR_HTTP_PRECONDITION_FAILED, "REST_PROXY_ERROR_HTTP_PRECONDITION_FAILED", "http-precondition-failed" },
      { REST_PROXY_ERROR_HTTP_REQUEST_ENTITY_TOO_LARGE, "REST_PROXY_ERROR_HTTP_REQUEST_ENTITY_TOO_LARGE", "http-request-entity-too-large" },
      { REST_PROXY_ERROR_HTTP_REQUEST_URI_TOO_LONG, "REST_PROXY_ERROR_HTTP_REQUEST_URI_TOO_LONG", "http-request-uri-too-long" },
      { REST_PROXY_ERROR_HTTP_UNSUPPORTED_MEDIA_TYPE, "REST_PROXY_ERROR_HTTP_UNSUPPORTED_MEDIA_TYPE", "http-unsupported-media-type" },
      { REST_PROXY_ERROR_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE, "REST_PROXY_ERROR_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE", "http-requested-range-not-satisfiable" },
      { REST_PROXY_ERROR_HTTP_EXPECTATION_FAILED, "REST_PROXY_ERROR_HTTP_EXPECTATION_FAILED", "http-expectation-failed" },
      { REST_PROXY_ERROR_HTTP_INTERNAL_SERVER_ERROR, "REST_PROXY_ERROR_HTTP_INTERNAL_SERVER_ERROR", "http-internal-server-error" },
      { REST_PROXY_ERROR_HTTP_NOT_IMPLEMENTED, "REST_PROXY_ERROR_HTTP_NOT_IMPLEMENTED", "http-not-implemented" },
      { REST_PROXY_ERROR_HTTP_BAD_GATEWAY, "REST_PROXY_ERROR_HTTP_BAD_GATEWAY", "http-bad-gateway" },
      { REST_PROXY_ERROR_HTTP_SERVICE_UNAVAILABLE, "REST_PROXY_ERROR_HTTP_SERVICE_UNAVAILABLE", "http-service-unavailable" },
      { REST_PROXY_ERROR_HTTP_GATEWAY_TIMEOUT, "REST_PROXY_ERROR_HTTP_GATEWAY_TIMEOUT", "http-gateway-timeout" },
      { REST_PROXY_ERROR_HTTP_HTTP_VERSION_NOT_SUPPORTED, "REST_PROXY_ERROR_HTTP_HTTP_VERSION_NOT_SUPPORTED", "http-http-version-not-supported" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("RestProxyError", values);
  }
  return etype;
}

/* enumerations from "rest-proxy-call.h" */
GType
rest_proxy_call_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { REST_PROXY_CALL_FAILED, "REST_PROXY_CALL_FAILED", "failed" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("RestProxyCallError", values);
  }
  return etype;
}



