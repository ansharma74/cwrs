#ifndef LXNM_H
#define LXNM_H

#include "port.h"

#define LXNM_SOCKET "/var/run/lxnm.socket"
#define LXNM_PRIORITY -4

#define LXNM_PROTOCOL "1.0"

#define LXNM_INTERFACE "system"

/* Command */
typedef enum {
	LXNM_VERSION,
	LXNM_DEVICE_LIST,
	LXNM_DEVICE_STATUS,
	LXNM_DEVICE_INFORMATION,
	LXNM_MODEM_INFORMATION,
	LXNM_PPP_INFORMATION,
	LXNM_ETHERNET_UP,
	LXNM_ETHERNET_DOWN,
	LXNM_ETHERNET_REPAIR,
	LXNM_WIRELESS_UP,
	LXNM_WIRELESS_DOWN,
	LXNM_WIRELESS_REPAIR,
	LXNM_WIRELESS_CONNECT,
	LXNM_WIRELESS_SCAN
} LXNMCommand;

typedef enum {
	LXNM_PPP_INFORMATION_TYPE_DEVICE,
	LXNM_PPP_INFORMATION_TYPE_INTERFACE
} LXNMPPPInfoType;

typedef enum {
	LXNM_DEVICE_TYPE_UNKNOWN,
	LXNM_DEVICE_TYPE_CONNECTION,
	LXNM_DEVICE_TYPE_MODEM,
	LXNM_DEVICE_TYPE_COUNT
} DeviceType;

typedef enum {
	LXNM_MODEM_TYPE_NORMAL,
	LXNM_MODEM_TYPE_GSM
} ModemType;

typedef enum {
	LXNM_FLAGS_MODEM_INFO_TYPE,
	LXNM_FLAGS_MODEM_INFO_PPPNUM
} ModemFlags;

typedef unsigned int LXNMPID;
typedef unsigned int LXNMClientID;

typedef enum {
	LXNM_CONNECTION_TYPE_UNKNOWN,
	LXNM_CONNECTION_TYPE_PRIDEVICE,
	LXNM_CONNECTION_TYPE_ETHERNET,
	LXNM_CONNECTION_TYPE_WIRELESS,
	LXNM_CONNECTION_TYPE_PPP
} ConnectionType;

typedef enum {
	LXNM_ENCRYPTION_OFF,
	LXNM_ENCRYPTION_WEP,
	LXNM_ENCRYPTION_WPA
} EncryptionMethod;

typedef enum {
	LXNM_KEYMGMT_NONE,
	LXNM_KEYMGMT_8021X,
	LXNM_KEYMGMT_PSK
} IEKeyMgmt;

typedef enum {
	LXNM_CYPHER_NONE,
	LXNM_CYPHER_WEP40,
	LXNM_CYPHER_WEP104,
	LXNM_CYPHER_TKIP,
	LXNM_CYPHER_WRAP,
	LXNM_CYPHER_CCMP
} IECypher;

typedef struct _LXNMHandler LXNMHandler;
typedef struct {
	LXNMHandler *devlist;
	LXNMHandler *iflist;
	LXNMHandler *modem_info;
	LXNMHandler *modem_detect;
	LXNMHandler *ppp_info;
	LXNMHandler *eth_up;
	LXNMHandler *eth_down;
	LXNMHandler *eth_repair;
	LXNMHandler *eth_info;
	LXNMHandler *eth_autofix;
	LXNMHandler *wifi_up;
	LXNMHandler *wifi_down;
	LXNMHandler *wifi_repair;
	LXNMHandler *wifi_connect;
	LXNMHandler *wifi_scan;
	LXNMHandler *wifi_info;
	LXNMHandler *wifi_autofix;
} Setting;

typedef struct {
	LXNMClientID  id;
	GIOChannel   *gio;
} LXNMClient;

typedef struct {
	gint ref;
	gchar *devname;
	DeviceType  type; 
} LXNMDevice;

typedef struct {
	gint ref;
	gchar *ifname;
	gchar *devname;
} LXNMInterface;

typedef struct {
	LXNMPID       cur_id;
	LXNMClientID  cur_cid;
	int      sockfd;
	Setting *setting;
	GList   *devices;
	GList   *interfaces;
	GList   *ifstatus;
	gint     status_timer_id;
	gboolean client_lock;

	/* autofix */
	gboolean autofix;

	/* timer */
	gint timer_id;
	gint timercount;
	GList *timer_tasks;
} LxND;

typedef struct {
	char *ifname;
	char *essid;
	char *apaddr;
	char *key;
	char *protocol;
	char *key_mgmt;
	char *group;
	char *pairwise;
} wificonn;

#endif
