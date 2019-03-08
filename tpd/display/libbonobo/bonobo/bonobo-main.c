/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-main.c: Bonobo Main
 *
 * Author:
 *    Miguel de Icaza  (miguel@gnu.org)
 *    Nat Friedman     (nat@nat.org)
 *    Peter Wainwright (prw@wainpr.demo.co.uk)
 *    Michael Meeks    (michael@ximian.com)
 *
 * Copyright 1999,2003 Ximian, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-private.h>
#include <bonobo/bonobo-arg.h>

#ifdef G_OS_WIN32
#include "bonobo-activation/bonobo-activation-private.h"
#endif

#include <libintl.h>
#include <string.h>

#include <glib.h>

#ifdef HAVE_GTHREADS
GMutex                   *_bonobo_lock;
#endif
static CORBA_ORB                 __bonobo_orb = CORBA_OBJECT_NIL;
static PortableServer_POA        __bonobo_poa = CORBA_OBJECT_NIL;
static PortableServer_POAManager __bonobo_poa_manager = CORBA_OBJECT_NIL;

static guint              bonobo_main_loop_level = 0;
static GSList *           bonobo_main_loops = NULL;

#ifdef G_OS_WIN32

#undef BONOBO_LOCALEDIR
#define BONOBO_LOCALEDIR _bonobo_activation_win32_get_localedir ()

#endif

/**
 * bonobo_orb:
 *
 * Returns: The ORB used for this Bonobo application.  The ORB
 * is created in bonobo_init().
 */
CORBA_ORB
bonobo_orb (void)
{
	return __bonobo_orb;
}

/**
 * bonobo_poa:
 *
 * Returns: The POA used for this Bonobo application.  The POA
 * is created when bonobo_init() is called.
 */
PortableServer_POA
bonobo_poa (void)
{
	return __bonobo_poa;
}

/**
 * bonobo_poa_manager:
 *
 * Returns: The POA Manager used for this Bonobo application.  The POA
 * Manager is created when bonobo_init() is called, but it is not
 * activated until bonobo_main() is called.
 */
PortableServer_POAManager
bonobo_poa_manager (void)
{
	return __bonobo_poa_manager;
}

static gint bonobo_inited = 0;

/**
 * bonobo_is_initialized:
 * 
 *   This allows you to protect against double
 * initialization in your code.
 * 
 * Return value: whether the ORB is initialized
 **/
gboolean
bonobo_is_initialized (void)
{
	return (bonobo_inited > 0);
}

/**
 * bonobo_debug_shutdown:
 * 
 *   This shuts down the ORB and any other bonobo related
 * resources.
 * 
 * Return value: whether the shutdown was clean, a good
 * value to return from 'main'.
 **/
int
bonobo_debug_shutdown (void)
{
	int retval = 0;

	if (bonobo_inited == 1) {
		CORBA_Environment ev;

		bonobo_inited--;

		CORBA_exception_init (&ev);

		bonobo_property_bag_shutdown ();
		bonobo_running_context_shutdown ();
		bonobo_context_shutdown ();
		if (bonobo_object_shutdown ())
			retval = 1;
		bonobo_exception_shutdown ();

		if (__bonobo_poa != CORBA_OBJECT_NIL)
			CORBA_Object_release (
				(CORBA_Object) __bonobo_poa, &ev);
		__bonobo_poa = CORBA_OBJECT_NIL;

		if (__bonobo_poa_manager != CORBA_OBJECT_NIL)
			CORBA_Object_release (
				(CORBA_Object) __bonobo_poa_manager, &ev);
		__bonobo_poa_manager = CORBA_OBJECT_NIL;

		if (!bonobo_activation_debug_shutdown ())
			retval = 1;

		__bonobo_orb = CORBA_OBJECT_NIL;
	} else if (bonobo_inited > 1) {
		bonobo_inited--;		
	} else /* shutdown when we didn't need to error */
		retval = 1;

	return retval;
}

/**
 * bonobo_init_full:
 * @argc: a pointer to the number of arguments
 * @argv: the array of arguments
 * @opt_orb: the ORB in which we run
 * @opt_poa: optional, a POA
 * @opt_manager: optional, a POA Manager
 *
 * Initializes the bonobo document model.  It requires at least
 * the value for @orb.  If @poa is CORBA_OBJECT_NIL, then the
 * RootPOA will be used, in this case @manager should be CORBA_OBJECT_NIL.
 *
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_init_full (int *argc, char **argv,
		  CORBA_ORB opt_orb, PortableServer_POA opt_poa,
		  PortableServer_POAManager opt_manager)
{
	CORBA_Environment ev;

	bonobo_activation_init (argc ? *argc : 0, argv);

	bonobo_inited++;
	if (bonobo_inited > 1)
		return TRUE;

	/* Init neccessary bits */
	g_type_init_with_debug_flags (0);

	bonobo_arg_init ();

	CORBA_exception_init (&ev);

	/*
	 * Create the POA.
	 */
	if (opt_orb == CORBA_OBJECT_NIL) {
		opt_orb = bonobo_activation_orb_get ();
		if (opt_orb == CORBA_OBJECT_NIL) {
			g_warning ("Can not resolve initial reference to ORB");
			CORBA_exception_free (&ev);
			return FALSE;
		}
	}
	
	if (opt_poa == CORBA_OBJECT_NIL) {
		opt_poa = (PortableServer_POA)
			CORBA_ORB_resolve_initial_references (
				opt_orb, "RootPOA", &ev);
		if (BONOBO_EX (&ev)) {
			g_warning ("Can not resolve initial reference to RootPOA");
			CORBA_exception_free (&ev);
			return FALSE;
		}
		
	}

	/*
	 * Create the POA Manager.
	 */
	if (opt_manager == CORBA_OBJECT_NIL) {
		opt_manager = PortableServer_POA__get_the_POAManager (opt_poa, &ev);
		if (BONOBO_EX (&ev)){
			g_warning ("Can not get the POA manager");
			CORBA_exception_free (&ev);
			return FALSE;
		}
	}

	/*
	 * Store global copies of these which can be retrieved with
	 * bonobo_orb()/bonobo_poa()/bonobo_poa_manager().
	 */
	__bonobo_orb = opt_orb;
	__bonobo_poa = opt_poa;
	__bonobo_poa_manager = opt_manager;

	CORBA_exception_free (&ev);

#ifdef HAVE_GTHREADS
	_bonobo_lock = g_mutex_new ();
#endif

	bonobo_context_init ();

	bindtextdomain (GETTEXT_PACKAGE, BONOBO_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

	return TRUE;
}

/**
 * bonobo_init:
 * @argc: a pointer to the number of arguments or %NULL
 * @argv: the array of arguments or %NULL
 *
 * Initializes the bonobo component model.
 *
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_init (int *argc, char **argv)
{
	return bonobo_init_full (
		argc, argv, NULL, NULL, NULL);
}

/**
 * bonobo_activate:
 *
 * Activates the Bonobo POA manager registered by bonobo_init.
 * This should be called at the end of application initialization.
 * You do not need to call this function if you use bonobo_main().
 * 
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_activate (void)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (!__bonobo_poa_manager) {
		g_warning ("Tried to activate Bonobo before initializing");
		CORBA_exception_free (&ev);
		return FALSE;
	}
	PortableServer_POAManager_activate (__bonobo_poa_manager, &ev);
	if (BONOBO_EX (&ev)){
		g_warning ("Failed to activate the Bonobo POA manager");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	CORBA_exception_free (&ev);
	
	return TRUE;
}

/**
 * bonobo_main:
 * 
 * Activates the Bonobo POA Manager and enters the main event loop.
 */
void
bonobo_main (void)
{
	GMainLoop *loop;

	bonobo_activate ();

	bonobo_main_loop_level++;
  
	loop = g_main_loop_new (NULL, TRUE);
	bonobo_main_loops = g_slist_prepend (bonobo_main_loops, loop);

	if (g_main_loop_is_running (bonobo_main_loops->data))
		g_main_loop_run (loop);

	bonobo_main_loops = g_slist_remove (bonobo_main_loops, loop);

	g_main_loop_unref (loop);

	bonobo_main_loop_level--;
}

/**
 * bonobo_main_level:
 *
 * Determines the number of times the bonobo main loop has been entered (minus
 * the number of exits from the main loop).
 *
 * Returns: The number of main loops currently running (0 if no main loops are
 * running).
 */
guint
bonobo_main_level (void)
{
	return bonobo_main_loop_level;
}

/**
 * bonobo_main_quit:
 * 
 * Quits the main event loop.
 */
void
bonobo_main_quit (void)
{
	g_return_if_fail (bonobo_main_loops != NULL);

	g_main_loop_quit (bonobo_main_loops->data);
}

/**
 * bonobo_poa_get_threadedv:
 * @hint: the desired thread hint
 * @args: va_args related to that hint
 * 
 * Get a predefined POA for a given threading policy/hint.  The
 * returned POA can be passed as the "poa" constructor property of a
 * #BonoboOject.
 * 
 * Return value: the requested POA.
 **/
PortableServer_POA
bonobo_poa_get_threadedv (ORBitThreadHint hint,
			  va_list         args)
{
	PortableServer_POA  poa;
	CORBA_Environment   ev[1];
	CORBA_PolicyList    policies;
	CORBA_Object        policy_vals[1];
	const char         *poa_name = NULL;

#define MAP(a,b) \
	case ORBIT_THREAD_HINT_##a: \
		poa_name = b; \
		break

	switch (hint) {
		MAP (NONE,           "BonoboPOAHintNone");
		MAP (PER_OBJECT,     "BonoboPOAHintPerObject");
		MAP (PER_REQUEST,    "BonoboPOAHintPerRequest");
		MAP (PER_POA,        "BonoboPOAHintPerPOA");
		MAP (PER_CONNECTION, "BonoboPOAHintPerConnection");
		MAP (ONEWAY_AT_IDLE, "BonoboPOAHintOnewayAtIdle");
		MAP (ALL_AT_IDLE,    "BonoboPOAHintAllAtIdle");
		MAP (ON_CONTEXT,     "BonoboPOAHintOnContext");
#undef MAP
	default:
		g_assert_not_reached();
	}

	CORBA_exception_init (ev);

	/* (Copy-paste from ORBit2/test/poa/poatest-basic08.c) */

	policies._length = 1;
	policies._buffer = policy_vals;
	policies._buffer[0] = (CORBA_Object)
		PortableServer_POA_create_thread_policy
			(bonobo_poa (),
			 PortableServer_ORB_CTRL_MODEL,
			 ev);

	poa = bonobo_poa_new_from (__bonobo_poa,
				   poa_name, &policies, ev);

	CORBA_Object_release (policies._buffer[0], NULL);

	if (ev->_major == CORBA_NO_EXCEPTION)
		ORBit_ObjectAdaptor_set_thread_hintv
			((ORBit_ObjectAdaptor) poa, hint, args);

	else {
		if (strcmp (CORBA_exception_id (ev),
			    ex_PortableServer_POA_AdapterAlreadyExists) == 0) {
			CORBA_exception_free (ev);
			poa = PortableServer_POA_find_POA (bonobo_poa (),
							   poa_name,
							   CORBA_FALSE, ev);
		}
	}
	CORBA_exception_free (ev);
	if (!poa)
		g_warning ("Could not create/get poa '%s'", poa_name);

	return poa;
}

/**
 * bonobo_poa_get_threaded:
 * @hint: the desired thread hint
 * 
 * Get a predefined POA for a given threading policy/hint.  The
 * returned POA can be passed as the "poa" constructor property of a
 * #BonoboOject.
 * 
 * Return value: the requested POA.
 **/
PortableServer_POA
bonobo_poa_get_threaded (ORBitThreadHint hint, ...)
{
	va_list args;
	PortableServer_POA poa;

	va_start (args, hint);
	poa = bonobo_poa_get_threadedv (hint, args);
	va_end (args);

	return poa;
}

PortableServer_POA
bonobo_poa_new_from (PortableServer_POA      tmpl,
		     const char             *name,
		     const CORBA_PolicyList *opt_policies,
		     CORBA_Environment      *opt_ev)
{
	PortableServer_POA poa;
	CORBA_Environment real_ev[1], *ev;

	if (!opt_ev)
		CORBA_exception_init ((ev = real_ev));
	else
		ev = opt_ev;

	poa = ORBit_POA_new_from (bonobo_orb(),
				  tmpl, name, opt_policies, ev);

	if (!opt_ev)
		CORBA_exception_free (real_ev);

	return poa;
}
