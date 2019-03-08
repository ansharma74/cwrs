


#include "gdata-service.h"
#include "gdata-parsable.h"
#include "gdata-batch-operation.h"
#include "gdata-enums.h"
#include "gdata-client-login-authorizer.h"

/* enumerations from "./gdata/gdata-service.h" */
GType
gdata_operation_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_OPERATION_QUERY, "GDATA_OPERATION_QUERY", "query" },
      { GDATA_OPERATION_INSERTION, "GDATA_OPERATION_INSERTION", "insertion" },
      { GDATA_OPERATION_UPDATE, "GDATA_OPERATION_UPDATE", "update" },
      { GDATA_OPERATION_DELETION, "GDATA_OPERATION_DELETION", "deletion" },
      { GDATA_OPERATION_DOWNLOAD, "GDATA_OPERATION_DOWNLOAD", "download" },
      { GDATA_OPERATION_UPLOAD, "GDATA_OPERATION_UPLOAD", "upload" },
      { GDATA_OPERATION_AUTHENTICATION, "GDATA_OPERATION_AUTHENTICATION", "authentication" },
      { GDATA_OPERATION_BATCH, "GDATA_OPERATION_BATCH", "batch" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataOperationType", values);
  }
  return etype;
}
GType
gdata_service_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_SERVICE_ERROR_UNAVAILABLE, "GDATA_SERVICE_ERROR_UNAVAILABLE", "unavailable" },
      { GDATA_SERVICE_ERROR_PROTOCOL_ERROR, "GDATA_SERVICE_ERROR_PROTOCOL_ERROR", "protocol-error" },
      { GDATA_SERVICE_ERROR_ENTRY_ALREADY_INSERTED, "GDATA_SERVICE_ERROR_ENTRY_ALREADY_INSERTED", "entry-already-inserted" },
      { GDATA_SERVICE_ERROR_AUTHENTICATION_REQUIRED, "GDATA_SERVICE_ERROR_AUTHENTICATION_REQUIRED", "authentication-required" },
      { GDATA_SERVICE_ERROR_NOT_FOUND, "GDATA_SERVICE_ERROR_NOT_FOUND", "not-found" },
      { GDATA_SERVICE_ERROR_CONFLICT, "GDATA_SERVICE_ERROR_CONFLICT", "conflict" },
      { GDATA_SERVICE_ERROR_FORBIDDEN, "GDATA_SERVICE_ERROR_FORBIDDEN", "forbidden" },
      { GDATA_SERVICE_ERROR_BAD_QUERY_PARAMETER, "GDATA_SERVICE_ERROR_BAD_QUERY_PARAMETER", "bad-query-parameter" },
      { GDATA_SERVICE_ERROR_NETWORK_ERROR, "GDATA_SERVICE_ERROR_NETWORK_ERROR", "network-error" },
      { GDATA_SERVICE_ERROR_PROXY_ERROR, "GDATA_SERVICE_ERROR_PROXY_ERROR", "proxy-error" },
      { GDATA_SERVICE_ERROR_WITH_BATCH_OPERATION, "GDATA_SERVICE_ERROR_WITH_BATCH_OPERATION", "with-batch-operation" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataServiceError", values);
  }
  return etype;
}

/* enumerations from "./gdata/gdata-parsable.h" */
GType
gdata_parser_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_PARSER_ERROR_PARSING_STRING, "GDATA_PARSER_ERROR_PARSING_STRING", "parsing-string" },
      { GDATA_PARSER_ERROR_EMPTY_DOCUMENT, "GDATA_PARSER_ERROR_EMPTY_DOCUMENT", "empty-document" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataParserError", values);
  }
  return etype;
}

/* enumerations from "./gdata/gdata-batch-operation.h" */
GType
gdata_batch_operation_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_BATCH_OPERATION_QUERY, "GDATA_BATCH_OPERATION_QUERY", "query" },
      { GDATA_BATCH_OPERATION_INSERTION, "GDATA_BATCH_OPERATION_INSERTION", "insertion" },
      { GDATA_BATCH_OPERATION_UPDATE, "GDATA_BATCH_OPERATION_UPDATE", "update" },
      { GDATA_BATCH_OPERATION_DELETION, "GDATA_BATCH_OPERATION_DELETION", "deletion" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataBatchOperationType", values);
  }
  return etype;
}

/* enumerations from "./gdata/gdata-client-login-authorizer.h" */
GType
gdata_client_login_authorizer_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_BAD_AUTHENTICATION, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_BAD_AUTHENTICATION", "bad-authentication" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_NOT_VERIFIED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_NOT_VERIFIED", "not-verified" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_TERMS_NOT_AGREED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_TERMS_NOT_AGREED", "terms-not-agreed" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_CAPTCHA_REQUIRED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_CAPTCHA_REQUIRED", "captcha-required" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_DELETED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_DELETED", "account-deleted" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_DISABLED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_DISABLED", "account-disabled" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_SERVICE_DISABLED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_SERVICE_DISABLED", "service-disabled" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_MIGRATED, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_ACCOUNT_MIGRATED", "account-migrated" },
      { GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_INVALID_SECOND_FACTOR, "GDATA_CLIENT_LOGIN_AUTHORIZER_ERROR_INVALID_SECOND_FACTOR", "invalid-second-factor" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GDataClientLoginAuthorizerError", values);
  }
  return etype;
}



