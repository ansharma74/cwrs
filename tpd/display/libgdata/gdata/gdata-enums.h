


#ifndef GDATA_ENUMS_H
#define GDATA_ENUMS_H

#include <glib-object.h>

G_BEGIN_DECLS
/* enumerations from "./gdata/gdata-service.h" */
GType gdata_operation_type_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_DATA_OPERATION_TYPE (gdata_operation_type_get_type())
GType gdata_service_error_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_DATA_SERVICE_ERROR (gdata_service_error_get_type())
/* enumerations from "./gdata/gdata-parsable.h" */
GType gdata_parser_error_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_DATA_PARSER_ERROR (gdata_parser_error_get_type())
/* enumerations from "./gdata/gdata-batch-operation.h" */
GType gdata_batch_operation_type_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_DATA_BATCH_OPERATION_TYPE (gdata_batch_operation_type_get_type())
/* enumerations from "./gdata/gdata-client-login-authorizer.h" */
GType gdata_client_login_authorizer_error_get_type (void) G_GNUC_CONST;
#define GDATA_TYPE_DATA_CLIENT_LOGIN_AUTHORIZER_ERROR (gdata_client_login_authorizer_error_get_type())
G_END_DECLS

#endif /* !GDATA_ENUMS_H */



