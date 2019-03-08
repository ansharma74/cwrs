#ifndef LXNM_APPLET_H
#define LXNM_APPLET_H

#define LXNM_APPLET_POLL_DELAY 1000

#include "backend.h"
#include "images.h"

typedef struct {
	GdkPixbuf *image[LXNM_APPLET_IMAGES_COUNT];
	GdkPixbufAnimation *animation[LXNM_APPLET_ANIMATION_COUNT];
	GList *devices;
	struct _LXNMBackend *lxnm;
	gint timer_id;
} LXNMApplet;

typedef struct {
	gchar *devname;
	gint type;
} LXNMAppletDeviceInfo;
typedef LXNMAppletDeviceInfo LXNMAppletModemInfo;

typedef struct {
	LXNMApplet *parent;
	GtkStatusIcon *statusicon;

	gchar *ifname;
	ConnectionType type;
	gint ref;
	void *menu;
	void *dlg;

	DeviceType devicetype;
	void *deviceinfo;
} LXNMAppletDevice;

void lxnm_applet_ppp_info(LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data);

#endif
