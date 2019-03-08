#ifndef LXNM_BACKEND_H
#define LXNM_BACKEND_H

#define LXNM_DEVICE_INFO_STATUS_NETDEV_ENABLE	 	(1<<0)
#define LXNM_DEVICE_INFO_STATUS_NETDEV_PLUG	 	(1<<1)
#define LXNM_DEVICE_INFO_STATUS_NETDEV_CONNECTING	(1<<2)
#define LXNM_DEVICE_INFO_STATUS_NETDEV_CONNECT		(1<<3)
#define LXNM_DEVICE_INFO_STATUS_NETDEV_PROBLEM		(1<<4)

/* LXNM status signals */
#define LXNM_STATUS_MESSAGE			0
#define LXNM_STATUS_DEVICE_LIST			1
#define LXNM_STATUS_NETDEV_INFO			2
#define LXNM_STATUS_NETDEV_ENABLE	 	3
#define LXNM_STATUS_NETDEV_DISABLE	 	4
#define LXNM_STATUS_NETDEV_PLUGGED	 	5
#define LXNM_STATUS_NETDEV_UNPLUG	 	6
#define LXNM_STATUS_NETDEV_CONNECTING		7
#define LXNM_STATUS_NETDEV_CONNECTED 		8
#define LXNM_STATUS_NETDEV_DISCONNECT		9
#define LXNM_STATUS_NETDEV_PROBLEM		10
#define LXNM_STATUS_NETDEV_BOTHRS		11
#define LXNM_STATUS_NETDEV_SENDDATA		12
#define LXNM_STATUS_NETDEV_RECVDATA		13
#define LXNM_STATUS_NETDEV_NORT			14
#define LXNM_STATUS_NETDEV_WIRELESS_QUALITY	15

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
	LXNM_DEVICE_TYPE_MODEM
} DeviceType;

typedef enum {
	LXNM_MODEM_TYPE_NORMAL,
	LXNM_MODEM_TYPE_GSM
} ModemType;

typedef enum {
	LXNM_FLAGS_MODEM_INFO_TYPE,
	LXNM_FLAGS_MODEM_INFO_PPPNUM
} ModemFlags;

/* Wireless */
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

#define LXNM_SOCKET "/var/run/lxnm.socket"
#define LXNM_INTERFACE "system"

typedef unsigned int LXNMPID;

typedef enum {
	LXNM_CONNECTION_TYPE_UNKNOWN,
	LXNM_CONNECTION_TYPE_PRIDEVICE,
	LXNM_CONNECTION_TYPE_ETHERNET,
	LXNM_CONNECTION_TYPE_WIRELESS,
	LXNM_CONNECTION_TYPE_PPP
} ConnectionType;

typedef struct _Task Task;
struct _Task {
	LXNMPID id;
	gint command;
	gboolean result;
	void (*callback)(struct _LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data);
	gpointer callback_data;
	void (*release)(struct _LXNMBackend *lxnm, Task *task, gint event, gpointer data, gpointer user_data);
	gpointer release_data;
};

#define LXNM_EVENT_IN		(1<<0)
#define LXNM_EVENT_RELEASE	(1<<1)

typedef enum {
	LXNM_WATCH_MODE_NORMAL,
	LXNM_WATCH_MODE_ONCE
} LXNMWatchMode;

typedef struct {
	gint mode;
	gint command;
	gint event;
	void (*func)();
	gpointer data;
} LXNMWatch;

typedef struct _LXNMBackend LXNMBackend;
struct _LXNMBackend {
	GIOChannel *gio;
	gint sockfd;
	GList *watch;
	GList *tasklist;
};

#define LXNM_GET_ESSID(a) hex2asc(a)

#endif
