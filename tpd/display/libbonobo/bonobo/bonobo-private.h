/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-private.h: internal private init & shutdown routines
 *                    used by bonobo_init & bonobo_shutdown
 *
 * Authors:
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_PRIVATE_H_
#define _BONOBO_PRIVATE_H_

#ifdef HAVE_GTHREADS
extern GMutex *_bonobo_lock;
#define BONOBO_LOCK()   g_mutex_lock(_bonobo_lock);
#define BONOBO_UNLOCK() g_mutex_unlock(_bonobo_lock);
#else
#define BONOBO_LOCK()   G_STMT_START{ (void)0; }G_STMT_END
#define BONOBO_UNLOCK() G_STMT_START{ (void)0; }G_STMT_END
#endif

void    bonobo_context_init     (void);
void    bonobo_context_shutdown (void);

int     bonobo_object_shutdown  (void);

void    bonobo_exception_shutdown       (void);
void    bonobo_property_bag_shutdown    (void);
void    bonobo_running_context_shutdown (void);

#endif /* _BONOBO_PRIVATE_H_ */
