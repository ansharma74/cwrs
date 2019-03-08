/*---[ gui.h ]---------------------------------------------------------
 * Copyright (C) 2002 Tomas Junnonen (majix@sci.fi)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The GUI header file
 *--------------------------------------------------------------------*/

#ifndef _FIRESTARTER_GUI
#define _FIRESTARTER_GUI

#include <config.h>
#include <gnome.h>
#include "firestarter.h"

void gui_construct (void);
gboolean gui_toggle_visibility (void);
void gui_set_visibility (gboolean visible);

void show_about (GtkWidget *widget, gpointer data);

/* Defintion of a GtkTreeView column */
typedef struct _View_col View_col;
struct _View_col
{
	gchar *label;
	GType type;
	gboolean visible;
};

/* Defintion of a GtkTreeView from which the model */
typedef struct _View_def View_def;
struct _View_def
{
	gint num_columns;
	View_col columns[6];
};

GtkWidget *gui_create_list_view (View_def *def, gint width, gint height);

typedef enum
{
	STATUS_VIEW,
	EVENTS_VIEW,
	POLICY_VIEW,
	NUM_VIEWS
} FirestarterView;

void gui_widget_sensitivity_sync (GtkToggleButton *source, GtkWidget *target);

FirestarterView gui_get_active_view (void);

#endif
