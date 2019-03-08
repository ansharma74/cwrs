/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef SERVER_H
#define SERVER_H

#include <bonobo-activation/bonobo-activation.h>
#include "bonobo-activation/Bonobo_ActivationContext.h"
#include "object-directory.h"

/*
 *    Define, and export BONOBO_ACTIVATION_DEBUG_OUTPUT
 * for a smoother, closer debugging experience.
 */
#define noBONOBO_ACTIVATION_DEBUG 1

/*
 *    Time delay after all servers are de-registered / dead
 * before quitting the server. (ms)
 */
#define SERVER_IDLE_QUIT_TIMEOUT 1

#define NAMING_CONTEXT_IID "OAFIID:Bonobo_CosNaming_NamingContext"
#define EVENT_SOURCE_IID "OAFIID:Bonobo_Activation_EventSource"

/* object-directory-load.c */
void bonobo_server_info_load         (char                  **dirs,
                                      Bonobo_ServerInfoList  *servers,
                                      GPtrArray const        *runtime_servers,
                                      GHashTable            **by_iid,
                                      const char             *host);
void bonobo_parse_server_info_memory (const char             *server_info,
                                      GSList                **entries,
                                      const char             *host);


/* od-activate.c */
typedef struct {
	Bonobo_ActivationContext ac;
	Bonobo_ActivationFlags flags;
	CORBA_Context ctx;
} ODActivationInfo;

/* object-directory-activate.c */
CORBA_Object             od_server_activate              (Bonobo_ServerInfo                  *si,
                                                          ODActivationInfo                   *actinfo,
                                                          CORBA_Object                        od_obj,
                                                          const Bonobo_ActivationEnvironment *environment,
                                                          Bonobo_ActivationClient             client,
                                                          CORBA_Environment                  *ev);

/* activation-context-corba.c */
Bonobo_ActivationContext activation_context_get          (void);

void                     activation_clients_cache_notify (void);
gboolean                 activation_clients_is_empty_scan(void);
void                     add_initial_locales             (void);
gboolean                 register_interest_in_locales    (const char            *locales);

typedef glong ServerLockState;
void                     server_lock                     (void);
void                     server_unlock                   (void);
ServerLockState          server_lock_drop                (void);
void                     server_lock_resume              (ServerLockState state);

#ifdef G_OS_WIN32
const char *server_win32_replace_prefix (const char *configure_time_path);
#endif

#endif /* SERVER_H */
