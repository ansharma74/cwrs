/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2012 Julien Danjou <julien@danjou.info>
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <fcntl.h>
#include <glib-object.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hidpp-device.h"

/* Arbitrary value used in ping */
#define HIDPP_PING_DATA						0x42

#define HIDPP_RECEIVER_ADDRESS					0xff

#define HIDPP_RESPONSE_SHORT_LENGTH				7
#define HIDPP_RESPONSE_LONG_LENGTH				20

#define HIDPP_HEADER_REQUEST					0x10
#define HIDPP_HEADER_RESPONSE					0x11

/* HID++ 1.0 */
#define HIDPP_READ_SHORT_REGISTER				0x81
#define HIDPP_READ_SHORT_REGISTER_BATTERY			0x0d

#define HIDPP_READ_LONG_REGISTER				0x83
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE			11
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_KEYBOARD		0x1
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_MOUSE		0x2
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_NUMPAD		0x3
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_PRESENTER		0x4
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_REMOTE_CONTROL	0x7
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TRACKBALL		0x8
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TOUCHPAD		0x9
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TABLET		0xa
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_GAMEPAD		0xb
#define HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_JOYSTICK		0xc

#define HIDPP_ERR_INVALID_SUBID					0x8f

/* HID++ 2.0 */

/* HID++2.0 error codes */
#define HIDPP_ERROR_CODE_NOERROR				0x00
#define HIDPP_ERROR_CODE_UNKNOWN				0x01
#define HIDPP_ERROR_CODE_INVALIDARGUMENT			0x02
#define HIDPP_ERROR_CODE_OUTOFRANGE				0x03
#define HIDPP_ERROR_CODE_HWERROR				0x04
#define HIDPP_ERROR_CODE_LOGITECH_INTERNAL			0x05
#define HIDPP_ERROR_CODE_INVALID_FEATURE_INDEX			0x06
#define HIDPP_ERROR_CODE_INVALID_FUNCTION_ID			0x07
#define HIDPP_ERROR_CODE_BUSY					0x08
#define HIDPP_ERROR_CODE_UNSUPPORTED				0x09

#define HIDPP_FEATURE_ROOT					0x0000
#define HIDPP_FEATURE_ROOT_INDEX				0x00
#define HIDPP_FEATURE_ROOT_FN_GET_FEATURE			(0x00 << 4)
#define HIDPP_FEATURE_ROOT_FN_PING				(0x01 << 4)
#define HIDPP_FEATURE_I_FEATURE_SET				0x0001
#define HIDPP_FEATURE_I_FEATURE_SET_FN_GET_COUNT		(0x00 << 4)
#define HIDPP_FEATURE_I_FEATURE_SET_FN_GET_FEATURE_ID		(0x01 << 4)
#define HIDPP_FEATURE_I_FIRMWARE_INFO				0x0003
#define HIDPP_FEATURE_I_FIRMWARE_INFO_FN_GET_COUNT		(0x00 << 4)
#define HIDPP_FEATURE_I_FIRMWARE_INFO_FN_GET_INFO		(0x01 << 4)
#define HIDPP_FEATURE_GET_DEVICE_NAME_TYPE			0x0005
#define HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_COUNT		(0x00 << 4)
#define HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_NAME		(0x01 << 4)
#define HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_TYPE		(0x02 << 4)
#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS			0x1000
//#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS_FN_GET_STATUS	(0x00 << 4)
//#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS_BE			(0x01 << 4)
#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS_FN_GET_CAPABILITY	(0x02 << 4)

#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS_FN_GET_STATUS	0x02
#define HIDPP_FEATURE_BATTERY_LEVEL_STATUS_BE			0x02

#define HIDPP_FEATURE_SPECIAL_KEYS_MSE_BUTTONS			0x1B00
#define HIDPP_FEATURE_WIRELESS_DEVICE_STATUS			0x1D4B
#define HIDPP_FEATURE_WIRELESS_DEVICE_STATUS_BE			(0x00 << 4)

#define HIDPP_FEATURE_SOLAR_DASHBOARD				0x4301
#define HIDPP_FEATURE_SOLAR_DASHBOARD_FN_SET_LIGHT_MEASURE	(0x00 << 4)
#define HIDPP_FEATURE_SOLAR_DASHBOARD_BE_BATTERY_LEVEL_STATUS	(0x01 << 4)

#define HIDPP_DEVICE_READ_RESPONSE_TIMEOUT			3000 /* miliseconds */

struct HidppDevicePrivate
{
	gboolean		 enable_debug;
	gchar			*hidraw_device;
	gchar			*model;
	GIOChannel		*channel;
	GPtrArray		*feature_index;
	guint			 batt_percentage;
	guint			 channel_source_id;
	guint			 device_idx;
	guint			 version;
	HidppDeviceBattStatus	 batt_status;
	HidppDeviceKind		 kind;
	int			 fd;
};

typedef struct {
	gint			 idx;
	guint16			 feature;
	gchar			*name;
} HidppDeviceMap;

G_DEFINE_TYPE (HidppDevice, hidpp_device, G_TYPE_OBJECT)
#define HIDPP_DEVICE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), HIDPP_TYPE_DEVICE, HidppDevicePrivate))

/**
 * hidpp_device_map_print:
 **/
static void
hidpp_device_map_print (HidppDevice *device)
{
	guint i;
	HidppDeviceMap *map;
	HidppDevicePrivate *priv = device->priv;

	if (!device->priv->enable_debug)
		return;
	for (i = 0; i < priv->feature_index->len; i++) {
		map = g_ptr_array_index (priv->feature_index, i);
		g_print ("%02x\t%s [%i]\n", map->idx, map->name, map->feature);
	}
}

/**
 * hidpp_device_map_get_by_feature:
 *
 * Gets the cached index from the function number.
 **/
static const HidppDeviceMap *
hidpp_device_map_get_by_feature (HidppDevice *device, guint16 feature)
{
	guint i;
	HidppDeviceMap *map;
	HidppDevicePrivate *priv = device->priv;

	for (i = 0; i < priv->feature_index->len; i++) {
		map = g_ptr_array_index (priv->feature_index, i);
		if (map->feature == feature)
			return map;
	}
	return NULL;
}

/**
 * hidpp_device_map_get_by_idx:
 *
 * Gets the cached index from the function index.
 **/
static const HidppDeviceMap *
hidpp_device_map_get_by_idx (HidppDevice *device, gint idx)
{
	guint i;
	HidppDeviceMap *map;
	HidppDevicePrivate *priv = device->priv;

	for (i = 0; i < priv->feature_index->len; i++) {
		map = g_ptr_array_index (priv->feature_index, i);
		if (map->idx == idx)
			return map;
	}
	return NULL;
}

/**
 * hidpp_device_print_buffer:
 *
 * Pretty print the send/recieve buffer.
 **/
static void
hidpp_device_print_buffer (HidppDevice *device, const guint8 *buffer)
{
	guint i;
	const HidppDeviceMap *map;

	if (!device->priv->enable_debug)
		return;
	for (i = 0; i < HIDPP_RESPONSE_LONG_LENGTH; i++)
		g_print ("%02x ", buffer[i]);
	g_print ("\n");

	/* direction */
	if (buffer[0] == HIDPP_HEADER_REQUEST)
		g_print ("REQUEST\n");
	else if (buffer[0] == HIDPP_HEADER_RESPONSE)
		g_print ("RESPONSE\n");
	else
		g_print ("??\n");

	/* dev index */
	g_print ("device-idx=%02x ", buffer[1]);
	if (buffer[1] == HIDPP_RECEIVER_ADDRESS) {
		g_print ("[Receiver]\n");
	} else if (device->priv->device_idx == buffer[1]) {
		g_print ("[This Device]\n");
	} else {
		g_print ("[Random Device]\n");
	}

	/* feature index */
	if (buffer[2] == HIDPP_READ_LONG_REGISTER) {
		g_print ("feature-idx=%s [%02x]\n",
			 "v1(ReadLongRegister)", buffer[2]);
	} else {
		map = hidpp_device_map_get_by_idx (device, buffer[2]);
		g_print ("feature-idx=v2(%s) [%02x]\n",
			 map != NULL ? map->name : "unknown", buffer[2]);
	}

	g_print ("function-id=%01x\n", buffer[3] & 0xf);
	g_print ("software-id=%01x\n", buffer[3] >> 4);
	g_print ("param[0]=%02x\n\n", buffer[4]);
}

/**
 * hidpp_device_cmd:
 **/
static gboolean
hidpp_device_cmd (HidppDevice	*device,
		  guint8	 device_idx,
		  guint8	 feature_idx,
		  guint8	 function_idx,
		  guint8	*request_data,
		  gsize		 request_len,
		  guint8	*response_data,
		  gsize		 response_len,
		  GError	**error)
{
	gboolean ret = TRUE;
	gssize wrote;
	guint8 buf[HIDPP_RESPONSE_LONG_LENGTH];
	guint i;
	HidppDevicePrivate *priv = device->priv;
	GPollFD poll[] = {
		{
			.fd = priv->fd,
			.events = G_IO_IN | G_IO_OUT | G_IO_ERR,
		},
	};

	/* make the request packet */
	memset (buf, 0x00, HIDPP_RESPONSE_LONG_LENGTH);
	buf[0] = HIDPP_HEADER_REQUEST;
	buf[1] = device_idx;
	buf[2] = feature_idx;
	buf[3] = function_idx;
	for (i = 0; i < request_len; i++)
		buf[4 + i] = request_data[i];

	/* write to the device */
	hidpp_device_print_buffer (device, buf);
	wrote = write (priv->fd, buf, 4 + request_len);
	if ((gsize) wrote != 4 + request_len) {
		g_set_error (error, 1, 0,
			     "Unable to write request to device: %" G_GSIZE_FORMAT,
			     wrote);
		ret = FALSE;
		goto out;
	}

	/* read from the device */
	wrote = g_poll (poll, G_N_ELEMENTS(poll),
		        HIDPP_DEVICE_READ_RESPONSE_TIMEOUT);
	if (wrote <= 0) {
		g_set_error (error, 1, 0,
			     "Attempt to read response from device timed out: %" G_GSIZE_FORMAT,
			     wrote);
		ret = FALSE;
		goto out;
	}
	memset (buf, 0x00, HIDPP_RESPONSE_LONG_LENGTH);
	wrote = read (priv->fd, buf, sizeof (buf));
	if (wrote <= 0) {
		g_set_error (error, 1, 0,
			     "Unable to read response from device: %" G_GSIZE_FORMAT,
			     wrote);
		ret = FALSE;
		goto out;
	}

	/* is device offline */
	hidpp_device_print_buffer (device, buf);
	if (buf[0] == HIDPP_HEADER_REQUEST &&
	    buf[1] == device_idx &&
	    buf[2] == HIDPP_ERR_INVALID_SUBID &&
	    buf[3] == 0x00 &&
	    buf[4] == HIDPP_FEATURE_ROOT_FN_PING) {
		/* HID++ 1.0 ping reply, so fake success  */
		if (buf[5] == HIDPP_ERROR_CODE_UNKNOWN) {
			buf[0] = 1;
			goto out;
		}
		if (buf[5] == HIDPP_ERROR_CODE_UNSUPPORTED) {
			/* device offline / unreachable */
			g_set_error_literal (error, 1, 0,
					     "device is unreachable");
			ret = FALSE;
			goto out;
		}
	}
	if (buf[0] != HIDPP_HEADER_RESPONSE ||
	    buf[1] != device_idx ||
	    buf[2] != feature_idx ||
	    buf[3] != function_idx) {
		g_set_error (error, 1, 0,
			     "invalid response from device: %" G_GSIZE_FORMAT,
			     wrote);
		ret = FALSE;
		goto out;
	}
	for (i = 0; i < response_len; i++)
		response_data[i] = buf[4 + i];
out:
	return ret;
}

/**
 * hidpp_device_map_add:
 *
 * Requests the index for a function, and adds it to the memeory cache
 * if it exists.
 **/
static gboolean
hidpp_device_map_add (HidppDevice *device,
		      guint16 feature,
		      const gchar *name)
{
	gboolean ret;
	GError *error = NULL;
	guint8 buf[3];
	HidppDeviceMap *map;
	HidppDevicePrivate *priv = device->priv;

	buf[0] = feature >> 8;
	buf[1] = feature;
	buf[2] = 0x00;

	g_debug ("Getting idx for feature %s [%02x]", name, feature);
	ret = hidpp_device_cmd (device,
				priv->device_idx,
				HIDPP_FEATURE_ROOT_INDEX,
				HIDPP_FEATURE_ROOT_FN_GET_FEATURE,
				buf, sizeof (buf),
				buf, sizeof (buf),
				&error);
	if (!ret) {
		g_warning ("Failed to get feature idx: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* zero index */
	if (buf[0] == 0x00) {
		ret = FALSE;
		g_debug ("Feature not found");
		goto out;
	}

	/* add to map */
	map = g_new0 (HidppDeviceMap, 1);
	map->idx = buf[0];
	map->feature = feature;
	map->name = g_strdup (name);
	g_ptr_array_add (priv->feature_index, map);
	g_debug ("Added feature %s [%02x] as idx %02x",
		 name, feature, map->idx);
out:
	return ret;
}

/**
 * hidpp_device_get_model:
 **/
const gchar *
hidpp_device_get_model (HidppDevice *device)
{
	g_return_val_if_fail (HIDPP_IS_DEVICE (device), NULL);
	return device->priv->model;
}

/**
 * hidpp_device_get_batt_percentage:
 **/
guint
hidpp_device_get_batt_percentage (HidppDevice *device)
{
	g_return_val_if_fail (HIDPP_IS_DEVICE (device), 0);
	return device->priv->batt_percentage;
}

/**
 * hidpp_device_get_version:
 **/
guint
hidpp_device_get_version (HidppDevice *device)
{
	g_return_val_if_fail (HIDPP_IS_DEVICE (device), 0);
	return device->priv->version;
}

/**
 * hidpp_device_get_batt_status:
 **/
HidppDeviceBattStatus
hidpp_device_get_batt_status (HidppDevice *device)
{
	g_return_val_if_fail (HIDPP_IS_DEVICE (device), HIDPP_DEVICE_BATT_STATUS_UNKNOWN);
	return device->priv->batt_status;
}

/**
 * hidpp_device_get_kind:
 **/
HidppDeviceKind
hidpp_device_get_kind (HidppDevice *device)
{
	g_return_val_if_fail (HIDPP_IS_DEVICE (device), HIDPP_DEVICE_KIND_UNKNOWN);
	return device->priv->kind;
}

/**
 * hidpp_device_set_hidraw_device:
 **/
void
hidpp_device_set_hidraw_device (HidppDevice *device,
				const gchar *hidraw_device)
{
	g_return_if_fail (HIDPP_IS_DEVICE (device));
	device->priv->hidraw_device = g_strdup (hidraw_device);
}

/**
 * hidpp_device_set_index:
 **/
void
hidpp_device_set_index (HidppDevice *device,
			guint device_idx)
{
	g_return_if_fail (HIDPP_IS_DEVICE (device));
	device->priv->device_idx = device_idx;
}

/**
 * hidpp_device_set_enable_debug:
 **/
void
hidpp_device_set_enable_debug (HidppDevice *device,
			       gboolean enable_debug)
{
	g_return_if_fail (HIDPP_IS_DEVICE (device));
	device->priv->enable_debug = enable_debug;
}

/**
 * hidpp_device_refresh:
 **/
gboolean
hidpp_device_refresh (HidppDevice *device,
		      HidppRefreshFlags refresh_flags,
		      GError **error)
{
	const HidppDeviceMap *map;
	gboolean ret = TRUE;
	GString *name = NULL;
	guint8 buf[HIDPP_RESPONSE_LONG_LENGTH];
	guint i;
	guint len;
	HidppDevicePrivate *priv = device->priv;

	g_return_val_if_fail (HIDPP_IS_DEVICE (device), FALSE);

	/* open the device if it's not already opened */
	if (priv->fd < 0) {
		priv->fd = open (device->priv->hidraw_device, O_RDWR | O_NONBLOCK);
		if (priv->fd < 0) {
			g_set_error (error, 1, 0,
				     "cannot open device file %s",
				     priv->hidraw_device);
			ret = FALSE;
			goto out;
		}

		/* add features we are going to use */
//		hidpp_device_map_add (device,
//				      HIDPP_FEATURE_I_FEATURE_SET,
//				      "IFeatureSet");
//		hidpp_device_map_add (device,
//				      HIDPP_FEATURE_I_FIRMWARE_INFO,
//				      "IFirmwareInfo");
		hidpp_device_map_add (device,
				      HIDPP_FEATURE_GET_DEVICE_NAME_TYPE,
				      "GetDeviceNameType");
		hidpp_device_map_add (device,
				      HIDPP_FEATURE_BATTERY_LEVEL_STATUS,
				      "BatteryLevelStatus");
//		hidpp_device_map_add (device,
//				      HIDPP_FEATURE_WIRELESS_DEVICE_STATUS,
//				      "WirelessDeviceStatus");
		hidpp_device_map_add (device,
				      HIDPP_FEATURE_SOLAR_DASHBOARD,
				      "SolarDashboard");
		hidpp_device_map_print (device);
	}

	/* get version */
	if ((refresh_flags & HIDPP_REFRESH_FLAGS_VERSION) > 0) {
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = HIDPP_PING_DATA;
		ret = hidpp_device_cmd (device,
					priv->device_idx,
					HIDPP_FEATURE_ROOT_INDEX,
					HIDPP_FEATURE_ROOT_FN_PING,
					buf, 3,
					buf, 4,
					error);
		if (!ret)
			goto out;
		priv->version = buf[0];
	}

	/* get device kind */
	if ((refresh_flags & HIDPP_REFRESH_FLAGS_KIND) > 0) {

		if (priv->version == 1) {
			buf[0] = 0x20 | (priv->device_idx - 1);
			buf[1] = 0x00;
			buf[2] = 0x00;
			ret = hidpp_device_cmd (device,
						HIDPP_RECEIVER_ADDRESS,
						HIDPP_READ_LONG_REGISTER,
						0xb5,
						buf, 3,
						buf, 7,
						error);
			if (!ret)
				goto out;
			switch (buf[7]) {
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_KEYBOARD:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_NUMPAD:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_REMOTE_CONTROL:
				priv->kind = HIDPP_DEVICE_KIND_KEYBOARD;
				break;
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_MOUSE:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TRACKBALL:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TOUCHPAD:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_PRESENTER:
				priv->kind = HIDPP_DEVICE_KIND_MOUSE;
				break;
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_TABLET:
				priv->kind = HIDPP_DEVICE_KIND_TABLET;
				break;
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_GAMEPAD:
			case HIDPP_READ_LONG_REGISTER_DEVICE_TYPE_JOYSTICK:
				/* upower doesn't have something for this yet */
				priv->kind = HIDPP_DEVICE_KIND_UNKNOWN;
				break;
			}
		} else if (priv->version == 2) {

			/* send a BatteryLevelStatus report */
			map = hidpp_device_map_get_by_feature (device, HIDPP_FEATURE_GET_DEVICE_NAME_TYPE);
			if (map != NULL) {
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0x00;
				ret = hidpp_device_cmd (device,
							priv->device_idx,
							map->idx,
							HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_TYPE,
							buf, 3,
							buf, 1,
							error);
				if (!ret)
					goto out;
				switch (buf[0]) {
				case 0: /* keyboard */
				case 2: /* numpad */
					priv->kind = HIDPP_DEVICE_KIND_KEYBOARD;
					break;
				case 3: /* mouse */
				case 4: /* touchpad */
				case 5: /* trackball */
					priv->kind = HIDPP_DEVICE_KIND_MOUSE;
					break;
				case 1: /* remote-control */
				case 6: /* presenter */
				case 7: /* receiver */
					priv->kind = HIDPP_DEVICE_KIND_UNKNOWN;
					break;
				}
			}
		}
	}

	/* get device model string */
	if ((refresh_flags & HIDPP_REFRESH_FLAGS_MODEL) > 0) {
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x00;
		map = hidpp_device_map_get_by_feature (device, HIDPP_FEATURE_GET_DEVICE_NAME_TYPE);
		if (map != NULL) {
			ret = hidpp_device_cmd (device,
						priv->device_idx,
						map->idx,
						HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_COUNT,
						buf, 3,
						buf, 1,
						error);
			if (!ret)
				goto out;
		}
		len = buf[0];
		name = g_string_new ("");
		for (i = 0; i < len; i +=4 ) {
			buf[0] = i;
			buf[1] = 0x00;
			buf[2] = 0x00;
			ret = hidpp_device_cmd (device,
						priv->device_idx,
						map->idx,
						HIDPP_FEATURE_GET_DEVICE_NAME_TYPE_FN_GET_NAME,
						buf, 3,
						buf, 4,
						error);
			if (!ret)
				goto out;
			g_string_append_len (name, (gchar *) &buf[0], 4);
		}
		priv->model = g_strdup (name->str);
	}

	/* get battery status */
	if ((refresh_flags & HIDPP_REFRESH_FLAGS_BATTERY) > 0) {
		if (priv->version == 1) {
			buf[0] = HIDPP_READ_SHORT_REGISTER;
			buf[1] = HIDPP_READ_SHORT_REGISTER_BATTERY;
			buf[2] = 0x00;
			buf[3] = 0x00;
			buf[4] = 0x00;
			ret = hidpp_device_cmd (device,
						priv->device_idx,
						HIDPP_FEATURE_ROOT_INDEX,
						HIDPP_FEATURE_ROOT_FN_PING,
						buf, 5,
						buf, 1,
						error);
			if (!ret)
				goto out;
			priv->batt_percentage = buf[0];
			priv->batt_status = HIDPP_DEVICE_BATT_STATUS_DISCHARGING;
		} else if (priv->version == 2) {

			/* sent a SetLightMeasure report */
			map = hidpp_device_map_get_by_feature (device, HIDPP_FEATURE_SOLAR_DASHBOARD);
			if (map != NULL) {
				buf[0] = 0x01; /* Max number of reports: number of report sent after function call */
				buf[1] = 0x01; /* Report period: time between reports, in seconds */
				ret = hidpp_device_cmd (device,
							priv->device_idx,
							map->idx,
							HIDPP_FEATURE_SOLAR_DASHBOARD_FN_SET_LIGHT_MEASURE,
							buf, 2,
							buf, 3,
							error);
				if (!ret)
					goto out;
				priv->batt_percentage = buf[0];
				priv->batt_status = HIDPP_DEVICE_BATT_STATUS_DISCHARGING;
			}

			/* send a BatteryLevelStatus report */
			map = hidpp_device_map_get_by_feature (device, HIDPP_FEATURE_BATTERY_LEVEL_STATUS);
			if (map != NULL) {
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0x00;
				ret = hidpp_device_cmd (device,
							priv->device_idx,
							map->idx,
							HIDPP_FEATURE_BATTERY_LEVEL_STATUS_FN_GET_STATUS,
							buf, 3,
							buf, 3,
							error);
				if (!ret)
					goto out;

				/* convert the HID++ v2 status into something
				 * we can set on the device */
				switch (buf[2]) {
				case 0: /* discharging */
					priv->batt_status = HIDPP_DEVICE_BATT_STATUS_DISCHARGING;
					break;
				case 1: /* recharging */
				case 2: /* charge nearly complete */
				case 4: /* charging slowly */
					priv->batt_status = HIDPP_DEVICE_BATT_STATUS_CHARGING;
					break;
				case 3: /* charging complete */
					priv->batt_status = HIDPP_DEVICE_BATT_STATUS_CHARGED;
					break;
				default:
					break;
				}
				priv->batt_percentage = buf[0];
				g_debug ("level=%i%%, next-level=%i%%, battery-status=%i",
					 buf[0], buf[1], buf[2]);
			}
		}
	}
out:
	if (name != NULL)
		g_string_free (name, TRUE);
	return ret;
}

/**
 * hidpp_device_init:
 **/
static void
hidpp_device_init (HidppDevice *device)
{
	HidppDeviceMap *map;

	device->priv = HIDPP_DEVICE_GET_PRIVATE (device);
	device->priv->fd = -1;
	device->priv->feature_index = g_ptr_array_new_with_free_func (g_free);
	device->priv->batt_status = HIDPP_DEVICE_BATT_STATUS_UNKNOWN;
	device->priv->kind = HIDPP_DEVICE_KIND_UNKNOWN;

	/* add known root */
	map = g_new0 (HidppDeviceMap, 1);
	map->idx = HIDPP_FEATURE_ROOT_INDEX;
	map->feature = HIDPP_FEATURE_ROOT;
	map->name = g_strdup ("Root");
	g_ptr_array_add (device->priv->feature_index, map);
}

/**
 * hidpp_device_finalize:
 **/
static void
hidpp_device_finalize (GObject *object)
{
	HidppDevice *device;

	g_return_if_fail (object != NULL);
	g_return_if_fail (HIDPP_IS_DEVICE (object));

	device = HIDPP_DEVICE (object);
	g_return_if_fail (device->priv != NULL);

	if (device->priv->channel_source_id > 0)
		g_source_remove (device->priv->channel_source_id);

	if (device->priv->channel) {
		g_io_channel_shutdown (device->priv->channel, FALSE, NULL);
		g_io_channel_unref (device->priv->channel);
	}
	g_ptr_array_unref (device->priv->feature_index);

	g_free (device->priv->hidraw_device);
	g_free (device->priv->model);

	G_OBJECT_CLASS (hidpp_device_parent_class)->finalize (object);
}

/**
 * hidpp_device_class_init:
 **/
static void
hidpp_device_class_init (HidppDeviceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = hidpp_device_finalize;
	g_type_class_add_private (klass, sizeof (HidppDevicePrivate));
}

/**
 * hidpp_device_new:
 **/
HidppDevice *
hidpp_device_new (void)
{
	return g_object_new (HIDPP_TYPE_DEVICE, NULL);
}
