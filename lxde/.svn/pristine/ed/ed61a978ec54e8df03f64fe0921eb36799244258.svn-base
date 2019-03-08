#ifndef LXNM_APPLET_UI_H
#define LXNM_APPLET_UI_H

typedef struct {
	GtkWidget *menu;
	GtkWidget *status;
	GtkWidget *connect_btn;
	gint apcount;
	GList *aps;
} LXNMAppletWirelessMenu;

typedef struct {
	LXNMAppletDevice *device;
	gchar *essid;
	gchar *bssid;
	gboolean encrypt;
	gchar *encryption;
	gchar *keymgmt;
	gchar *group;
	gchar *pairwise;
	gboolean selected;
} LXNMAppletWirelessDialogInfo;

typedef struct {
	GtkWidget *dlg;
	GtkWidget *message;
	GtkWidget *essid_label;
	GtkWidget *essid_entry;
	GtkWidget *key_label;
	GtkWidget *key_entry;
	LXNMAppletWirelessDialogInfo *info;
} LXNMAppletWirelessDialog;

gint lxnm_applet_ui_statusicon_press(GtkWidget *widget, GdkEvent *event, gpointer user_data);

#endif
