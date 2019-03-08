/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-arg.c Bonobo argument support:
 *
 *  A thin wrapper of CORBA_any's with macros
 * to assist in handling values safely.
 *
 * Author:
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Ximian., Inc.
 */
#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-arg.h>
#include "bonobo-types.h"


/* Key: CORBA_TypeCode; Value: BonoboArgToGValueFn. */
static GHashTable *bonobo_arg_to_gvalue_mapping   = NULL;
/* Key: GType; Value: BonoboArgFromGValueFn */
static GHashTable *bonobo_arg_from_gvalue_mapping = NULL;


/**
 * bonobo_arg_new:
 * @t: the BonoboArgType eg. TC_CORBA_long
 * 
 * Create a new BonoboArg with the specified type
 * the value of the BonoboArg is initially empty.
 * 
 * Returns: the new #BonoboArg
 **/
BonoboArg *
bonobo_arg_new (BonoboArgType t)
{
	CORBA_any *any = CORBA_any__alloc ();
	any->_release = TRUE;
	any->_type = (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) t, NULL);
	any->_value = ORBit_small_alloc (t);
	return any;
}

/**
 * bonobo_arg_new_from
 * @t: the BonoboArgType eg. TC_CORBA_long
 * @data: the data for the BonoboArg to be created
 *
 * Create a new BonoboArg with the specified type and data
 *
 * Returns: the new #BonoboArg
 */
BonoboArg * 
bonobo_arg_new_from (BonoboArgType t, gconstpointer data)
{
	BonoboArg *arg;

	arg = CORBA_any__alloc ();
	arg->_release = TRUE;
	arg->_type = (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) t, NULL);
	arg->_value = ORBit_copy_value (data, t);

	return arg;
}

/**
 * bonobo_arg_release:
 * @arg: the bonobo arg.
 * 
 * This frees the memory associated with @arg
 **/
void
bonobo_arg_release (BonoboArg *arg)
{
	if (arg)
		CORBA_free (arg);	
}

/**
 * bonobo_arg_copy:
 * @arg: the bonobo arg
 * 
 * This function duplicates @a by a deep copy
 * 
 * Returns: a copy of @arg
 **/
BonoboArg *
bonobo_arg_copy (const BonoboArg *arg)
{
	BonoboArg *copy = CORBA_any__alloc ();

	if (!arg) {
		copy->_type = TC_null;
		g_warning ("Duplicating a NULL Bonobo Arg");
	} else
		CORBA_any__copy (copy, arg);

	return copy;
}

#undef MAPPING_DEBUG

/**
 * bonobo_arg_type_from_gruntime:
 * @id: the GType id.
 * 
 * This maps a GType to a BonoboArgType
 * 
 * Return value: the mapped type or %NULL on failure.
 **/
BonoboArgType
bonobo_arg_type_from_gtype (GType id)
{
	switch (id) {
	case G_TYPE_CHAR:   return TC_CORBA_char;
	case G_TYPE_UCHAR:  return TC_CORBA_char;
	case G_TYPE_BOOLEAN:return TC_CORBA_boolean;
	case G_TYPE_INT:    return TC_CORBA_short;
	case G_TYPE_UINT:   return TC_CORBA_unsigned_short;
	case G_TYPE_LONG:   return TC_CORBA_long;
	case G_TYPE_ULONG:  return TC_CORBA_unsigned_long;
	case G_TYPE_FLOAT:  return TC_CORBA_float;
	case G_TYPE_DOUBLE: return TC_CORBA_double;
	case G_TYPE_STRING: return TC_CORBA_string;
	default:
#ifdef MAPPING_DEBUG
		g_warning ("Unmapped arg type '%d'", id);
#endif
		break;
	}

	return NULL;
}

/**
 * bonobo_arg_type_to_gtype:
 * @id: the BonoboArgType
 * 
 * This maps a BonoboArgType to a GType
 * 
 * Return value: the mapped type or %0 on failure
 **/
GType
bonobo_arg_type_to_gtype (BonoboArgType id)
{
	CORBA_Environment ev;
	GType g_type = G_TYPE_NONE;

	CORBA_exception_init (&ev);

	if (bonobo_arg_type_is_equal (TC_CORBA_char, id, &ev))
		g_type = G_TYPE_CHAR;
	else if (bonobo_arg_type_is_equal (TC_CORBA_boolean, id, &ev))
		g_type = G_TYPE_BOOLEAN;
	else if (bonobo_arg_type_is_equal (TC_CORBA_short,   id, &ev))
		g_type = G_TYPE_INT;
	else if (bonobo_arg_type_is_equal (TC_CORBA_unsigned_short, id, &ev))
		g_type = G_TYPE_UINT;
	else if (bonobo_arg_type_is_equal (TC_CORBA_long,    id, &ev))
		g_type = G_TYPE_LONG;
	else if (bonobo_arg_type_is_equal (TC_CORBA_unsigned_long, id, &ev))
		g_type = G_TYPE_ULONG;
	else if (bonobo_arg_type_is_equal (TC_CORBA_float,   id, &ev))
		g_type = G_TYPE_FLOAT;
	else if (bonobo_arg_type_is_equal (TC_CORBA_double,  id, &ev))
		g_type = G_TYPE_DOUBLE;
	else if (bonobo_arg_type_is_equal (TC_CORBA_string,  id, &ev))
		g_type = G_TYPE_STRING;
#ifdef MAPPING_DEBUG
	else
		g_warning ("Unmapped bonobo arg type");
#endif

	CORBA_exception_free (&ev);

	return g_type;
}

#define MAKE_FROM_GVALUE(gtype,gtypename,tcid,unionid,corbatype,corbakind)	\
	case G_TYPE_##gtype:							\
		*((corbatype *)a->_value) = (corbatype)g_value_get_##gtypename (value);	\
		break;

/**
 * bonobo_arg_from_gvalue:
 * @a: pointer to blank BonoboArg
 * @value: #GValue to copy
 * 
 * This maps a GValue @value to a BonoboArg @a;
 * @a must point to a freshly allocated BonoboArg
 * eg. such as returned by bonobo_arg_new
 **/
void
bonobo_arg_from_gvalue (BonoboArg *a, const GValue *value)
{
	int        id;

	g_return_if_fail (a != NULL);
	g_return_if_fail (value != NULL);

	id = G_VALUE_TYPE (value);

	switch (id) {

	case G_TYPE_INVALID:
	case G_TYPE_NONE:
		g_warning ("Strange GValue type %s", g_type_name (id));
		break;
		
		MAKE_FROM_GVALUE (CHAR,    char,    TC_CORBA_char,     char_data, CORBA_char,           CORBA_tk_char);
		MAKE_FROM_GVALUE (UCHAR,   uchar,   TC_CORBA_char,    uchar_data, CORBA_char,           CORBA_tk_char);
		MAKE_FROM_GVALUE (BOOLEAN, boolean, TC_CORBA_boolean,  bool_data, CORBA_boolean,        CORBA_tk_boolean);
		MAKE_FROM_GVALUE (INT,     int,     TC_CORBA_short,     int_data, CORBA_short,          CORBA_tk_short);
		MAKE_FROM_GVALUE (UINT,    uint,    TC_CORBA_unsigned_short,   uint_data, CORBA_unsigned_short, CORBA_tk_ushort);
		MAKE_FROM_GVALUE (LONG,    long,    TC_CORBA_long,     long_data, CORBA_long,           CORBA_tk_long);
		MAKE_FROM_GVALUE (ULONG,   ulong,   TC_CORBA_unsigned_long,   ulong_data, CORBA_unsigned_long,  CORBA_tk_ulong);
		MAKE_FROM_GVALUE (FLOAT,   float,   TC_CORBA_float,   float_data, CORBA_float,          CORBA_tk_float);
		MAKE_FROM_GVALUE (DOUBLE,  double,  TC_CORBA_double, double_data, CORBA_double,         CORBA_tk_double);

	case G_TYPE_STRING:
		if (G_VALUE_HOLDS_STRING (value))
			*((CORBA_char **)a->_value) = CORBA_string_dup (g_value_get_string (value));
		else
			*((CORBA_char **)a->_value) = CORBA_string_dup ("");
		break;

	case G_TYPE_POINTER:
		g_warning ("We can map user data callbacks locally");
		break;

	case G_TYPE_OBJECT:
		g_warning ("All objects can be mapped to base gtk types"
			   "in due course");
		break;

	case G_TYPE_ENUM:
	case G_TYPE_FLAGS:
	case G_TYPE_BOXED:
	default:
		g_warning ("Unmapped GValue type %d", id);
		break;
	}
}

#define MAKE_TO_GVALUE(gtype,gtypename,tcid,unionid,corbatype,corbakind)	\
	case corbakind:								\
		g_value_set_##gtypename (value, *((corbatype *)arg->_value));	\
		break;

/**
 * bonobo_arg_to_gvalue:
 * @value: pointer to a blank #GValue
 * @arg: the BonoboArg to copy
 * 
 * Maps a BonoboArg to a GtkArg; @a must point
 * to a blank GtkArg.
 **/
void
bonobo_arg_to_gvalue (GValue *value, const BonoboArg *arg)
{
	int     id;

	g_return_if_fail (value != NULL);
	g_return_if_fail (arg != NULL);
	g_return_if_fail (arg->_type != NULL);

	id = arg->_type->kind;
	switch (id) {

	case CORBA_tk_null:
	case CORBA_tk_void:
		g_warning ("Strange corba arg type %d", id);
		break;
		
		MAKE_TO_GVALUE (CHAR,    char,    TC_CORBA_char,     char_data, CORBA_char,           CORBA_tk_char);
/*		MAKE_TO_GVALUE (UCHAR,   uchar,   TC_CORBA_char,    uchar_data, CORBA_char,           CORBA_tk_char);*/
		MAKE_TO_GVALUE (BOOLEAN, boolean, TC_CORBA_boolean,  bool_data, CORBA_boolean,        CORBA_tk_boolean);
		MAKE_TO_GVALUE (INT,     int,     TC_CORBA_short,     int_data, CORBA_short,          CORBA_tk_short);
		MAKE_TO_GVALUE (UINT,    uint,    TC_CORBA_ushort,   uint_data, CORBA_unsigned_short, CORBA_tk_ushort);
		MAKE_TO_GVALUE (LONG,    long,    TC_CORBA_long,     long_data, CORBA_long,           CORBA_tk_long);
		MAKE_TO_GVALUE (ULONG,   ulong,   TC_CORBA_ulong,   ulong_data, CORBA_unsigned_long,  CORBA_tk_ulong);
		MAKE_TO_GVALUE (FLOAT,   float,   TC_CORBA_float,   float_data, CORBA_float,          CORBA_tk_float);
		MAKE_TO_GVALUE (DOUBLE,  double,  TC_CORBA_double, double_data, CORBA_double,         CORBA_tk_double);

	case CORBA_tk_string:
		g_value_set_string (value, BONOBO_ARG_GET_STRING (arg));
		break;

	case CORBA_tk_objref:
		g_warning ("All objects can be mapped to base gtk types"
			   "in due course");
		break;

	case CORBA_tk_sequence:
	case CORBA_tk_alias:
	case CORBA_tk_enum:
	case CORBA_tk_array:
	case CORBA_tk_union:
		g_warning ("Clever things can be done for these");
		break;

	default:
		g_warning ("Unmapped corba arg type %d", id);
		break;
	}
}

/**
 * bonobo_arg_type_is_equal:
 * @a: a type code
 * @b: a type code
 * @opt_ev: optional exception environment or NULL.
 * 
 * This compares two #BonoboArgType's in @a and @b.
 * The @opt_ev is an optional #CORBA_Environment for
 * exceptions, or %NULL. This function is commutative.
 * 
 * Return value: %TRUE if equal, %FALSE if different
 **/
gboolean
bonobo_arg_type_is_equal (BonoboArgType a, BonoboArgType b, CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	gboolean retval;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	retval = CORBA_TypeCode_equal (a, b, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

/**
 * bonobo_arg_is_equal:
 * @a: a bonobo arg
 * @b: another bonobo arg
 * @opt_ev: optional exception environment or %NULL.
 * 
 * Compares two #BonoboArg's for equivalence; will return %TRUE
 * if equivalent for all simple cases. For Object references
 * CORBA sometimes denies 2 object references are equivalent
 * even if they are [ this is a feature_not_bug ].
 *
 * This function is commutative.
 * 
 * Return value: %TRUE if @a == @b
 **/
gboolean
bonobo_arg_is_equal (const BonoboArg *a, const BonoboArg *b, CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	gboolean retval;

	if (!opt_ev) {

		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	retval = ORBit_any_equivalent ((CORBA_any *) a, (CORBA_any *) b, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}


/**
 * bonobo_arg_to_gvalue_alloc:
 * @arg: source value
 * @value: destination value
 * 
 * Converts a #BonoboArg @arg into a #GValue.  Unlike
 * bonobo_arg_to_gvalue(), the destination #GValue does not need to --
 * and should not -- be initialized.
 * 
 * Return value: Returns %TRUE if conversion succeeds, %FALSE otherwise.
 **/
gboolean
bonobo_arg_to_gvalue_alloc (BonoboArg const *arg, GValue *value)
{
	BonoboArgToGValueFn converter;

	g_assert (bonobo_arg_from_gvalue_mapping);

#define TO_GVALUE_CASE(gtypename, bonoboargname, typename, typecode)			\
	if (CORBA_TypeCode_equal(arg->_type, typecode, NULL)) {				\
		g_value_init (value, G_TYPE_##gtypename);				\
		g_value_set_##typename (value, BONOBO_ARG_GET_##bonoboargname(arg));	\
		return TRUE;								\
	}

	TO_GVALUE_CASE (STRING,  STRING,  string,  TC_CORBA_string);
	TO_GVALUE_CASE (CHAR,    CHAR,    char,    TC_CORBA_char);
	TO_GVALUE_CASE (BOOLEAN, BOOLEAN, boolean, TC_CORBA_boolean);
	TO_GVALUE_CASE (LONG,    LONG,    long,    TC_CORBA_long);
	TO_GVALUE_CASE (ULONG,   ULONG,   ulong,   TC_CORBA_unsigned_long);
	TO_GVALUE_CASE (FLOAT,   FLOAT,   float,   TC_CORBA_float);
	TO_GVALUE_CASE (DOUBLE,  DOUBLE,  double,  TC_CORBA_double);
	
	converter = g_hash_table_lookup (bonobo_arg_to_gvalue_mapping,
					 arg->_type);
	if (converter)
	    converter (arg, value);
	else
	    return FALSE;
	return TRUE;
}


/**
 * bonobo_arg_to_gvalue_alloc:
 * @arg: destination value
 * @value: source value
 * 
 * Converts a #GValue into a #BonoboArg.  Unlike
 * bonobo_arg_from_gvalue(), the destination #BonoboArg does not need
 * to -- and should not -- be initialized.
 * 
 * Return value: Returns %TRUE if conversion succeeds, %FALSE otherwise.
 **/
gboolean
bonobo_arg_from_gvalue_alloc (BonoboArg *arg, GValue const *value)
{
	BonoboArgFromGValueFn converter;

	g_return_val_if_fail (arg, FALSE);
	g_return_val_if_fail (value, FALSE);
	g_assert (bonobo_arg_from_gvalue_mapping);

#define FROM_GVALUE_CASE(gtype, gtypename, tcid, corbatype)				\
case G_TYPE_##gtype:									\
	arg->_type = tcid;								\
	arg->_value = ORBit_alloc_tcval (tcid, 1);					\
	*((corbatype *)arg->_value) = (corbatype) g_value_get_##gtypename (value);	\
	arg->_release = CORBA_TRUE;							\
	return TRUE;
	
	switch (G_VALUE_TYPE (value))
	{
		FROM_GVALUE_CASE (CHAR,    char,    TC_CORBA_char,          CORBA_char);
		FROM_GVALUE_CASE (UCHAR,   uchar,   TC_CORBA_char,          CORBA_char);
		FROM_GVALUE_CASE (BOOLEAN, boolean, TC_CORBA_boolean,       CORBA_boolean);
		FROM_GVALUE_CASE (INT,     int,     TC_CORBA_long,          CORBA_long);
		FROM_GVALUE_CASE (UINT,    uint,    TC_CORBA_unsigned_long, CORBA_unsigned_long);
		FROM_GVALUE_CASE (LONG,    long,    TC_CORBA_long,          CORBA_long);
		FROM_GVALUE_CASE (ULONG,   ulong,   TC_CORBA_unsigned_long, CORBA_unsigned_long);
		FROM_GVALUE_CASE (FLOAT,   float,   TC_CORBA_float,         CORBA_float);
		FROM_GVALUE_CASE (DOUBLE,  double,  TC_CORBA_double,        CORBA_double);
#undef FROM_GVALUE_FN

	case G_TYPE_STRING: {
		const char *str = g_value_get_string (value);
		arg->_type = TC_CORBA_string;
		arg->_value = ORBit_alloc_tcval (TC_CORBA_string, 1);
		if (str) {
			*((CORBA_char **)arg->_value) =	CORBA_string_dup (str);
			arg->_release = CORBA_TRUE;
		} else {
			*((CORBA_char **)arg->_value) = "";
			arg->_release = CORBA_FALSE;
		}
		return TRUE;
	}
	}
	  /* default: try to lookup a converter function */
	converter = g_hash_table_lookup (bonobo_arg_from_gvalue_mapping,
					 GUINT_TO_POINTER (G_VALUE_TYPE (value)));
	if (converter)
		converter (arg, value);
	else
		return FALSE;
	return TRUE;
}


/* GValue => BonoboArg converters */

static void
__bonobo_arg_from_CORBA_ANY (BonoboArg    *out_arg,
			     GValue const *value)
{
	out_arg->_type    = TC_CORBA_any;
	out_arg->_value   = bonobo_value_get_corba_any (value);
	out_arg->_release = CORBA_TRUE;
}

static void
__TC_CORBA_any_to_gvalue (BonoboArg const *arg,
			  GValue          *out_value)
{
	g_value_init (out_value, BONOBO_TYPE_CORBA_ANY);
	bonobo_value_set_corba_any (out_value, arg->_value);
}


void
bonobo_arg_register_from_gvalue_converter (GType                 gtype,
					   BonoboArgFromGValueFn converter)
{
	g_return_if_fail (bonobo_arg_from_gvalue_mapping != NULL);
	g_hash_table_insert (bonobo_arg_from_gvalue_mapping,
			     GUINT_TO_POINTER (gtype),
			     converter);
}

void
bonobo_arg_register_to_gvalue_converter (BonoboArgType       arg_type,
					 BonoboArgToGValueFn converter)
{
	g_return_if_fail (bonobo_arg_to_gvalue_mapping != NULL);
	g_hash_table_insert (bonobo_arg_to_gvalue_mapping,
			     arg_type, converter);
}

void bonobo_arg_init (void)
{
	g_assert (bonobo_arg_to_gvalue_mapping == NULL);
	g_assert (bonobo_arg_from_gvalue_mapping == NULL);

	bonobo_arg_to_gvalue_mapping   = g_hash_table_new (g_direct_hash, g_direct_equal);
	bonobo_arg_from_gvalue_mapping = g_hash_table_new (g_direct_hash, g_direct_equal);

	bonobo_arg_register_from_gvalue_converter 
		(BONOBO_TYPE_CORBA_ANY, __bonobo_arg_from_CORBA_ANY);
	bonobo_arg_register_to_gvalue_converter
		(TC_CORBA_any, __TC_CORBA_any_to_gvalue);
}

