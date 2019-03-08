/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 */
#ifndef BONOBO_ACTIVATION_PRIVATE_H
#define BONOBO_ACTIVATION_PRIVATE_H

#include <config.h>
#include <string.h>
#include <bonobo-activation/bonobo-activation-base-service.h>
#include <bonobo-activation/Bonobo_ActivationContext.h>

#define BONOBO_ACTIVATION_FACTORY_PRIVATE_TIMEOUT 1

extern GStaticRecMutex _bonobo_activation_guard;
#define BONOBO_ACTIVATION_LOCK()   g_static_rec_mutex_lock   (&_bonobo_activation_guard)
#define BONOBO_ACTIVATION_UNLOCK() g_static_rec_mutex_unlock (&_bonobo_activation_guard)

void         bonobo_activation_timeout_reg_check_set  (gboolean           on);
gboolean     bonobo_activation_timeout_reg_check      (gpointer           data);

void         bonobo_activation_server_fork_init  (gboolean                            threaded);
typedef CORBA_Object (*BonoboForkReCheckFn)      (const Bonobo_ActivationEnvironment *environemnt,
                                                  const char                         *act_iid,
                                                  gpointer                            user_data);
CORBA_Object bonobo_activation_server_by_forking (const char                        **cmd, 
						  gboolean                            set_process_group,
						  int                                 fd_arg,
						  const Bonobo_ActivationEnvironment *environemnt,
						  const char                         *od_iorstr,
						  const char                         *act_iid,
                                                  gboolean                            use_new_loop,
						  BonoboForkReCheckFn                 re_check,
						  gpointer                            user_data,
						  CORBA_Environment                  *ev);

void         bonobo_activation_base_service_init      (void);
int          bonobo_activation_ior_fd_get             (void);
CORBA_Object bonobo_activation_activation_context_get (void);
CORBA_Object bonobo_activation_object_directory_get   (const char        *username,
                                                       const char        *hostname);
void         bonobo_activation_init_activation_env    (void);

extern gboolean bonobo_activation_private;

CORBA_Object bonobo_activation_internal_activation_context_get_extended (
                                                       gboolean           existing_only,
                                                       CORBA_Environment *ev);

CORBA_Object bonobo_activation_internal_service_get_extended (
                                                       const BonoboActivationBaseService *base_service,
                                                       gboolean           existing_only,
                                                       CORBA_Environment *ev);

gboolean Bonobo_ActivationEnvironment_match (const Bonobo_ActivationEnvironment *a,
					     const Bonobo_ActivationEnvironment *b);

void Bonobo_ActivationEnvValue_set  (Bonobo_ActivationEnvValue *env,
				     const char                *name,
				     const char                *value);
void Bonobo_ActivationEnvValue_copy (Bonobo_ActivationEnvValue *dest,
				     Bonobo_ActivationEnvValue *src);

Bonobo_ActivationClient bonobo_activation_client_get (void);

    /* used only for unit testing */
CORBA_char * _bonobo_activation_get_activation_env_value (const char *name);

char * _bonobo_activation_ior_fname_get (void);
char * _bonobo_activation_lock_fname_get (void);

#ifdef G_OS_WIN32

char *_bonobo_activation_win32_replace_prefix (const char *runtime_prefix,
                                               const char *configure_time_path);
const char *_bonobo_activation_win32_get_prefix (void);
const char *_bonobo_activation_win32_get_server_libexecdir (void);
const char *_bonobo_activation_win32_get_serverinfodir (void);
const char *_bonobo_activation_win32_get_server_confdir (void);
const char *_bonobo_activation_win32_get_localedir (void);

#endif

#endif /* BONOBO_ACTIVATION_PRIVATE_H */

