#ifndef LXNM_STATUS_H
#define LXNM_STATUS_H

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

typedef struct {
	DeviceType  devtype; 
	gint        ref;
	gchar      *devname;
} DeviceStatus;

typedef struct {
	DeviceType  devtype; 
	gint        ref;
	gchar      *ifname;
	ConnectionType  type; 
	gpointer    info;
	GList      *clients; /* who is listening for this interface */
} InterfaceStatus;
typedef void InterfaceInfo;

typedef struct {
	gboolean enable;
	gboolean plugged;
	gboolean connected;

	gchar *mac;
	gchar *ipaddr;
	gchar *dest;
	gchar *bcast;
	gchar *mask;

	gulong recv_bytes;
	gulong recv_packets;
	gulong trans_bytes;
	gulong trans_packets;
	gboolean rtstat;
} EthernetInfo;

typedef struct {
	gboolean enable;
	gboolean plugged;
	gboolean connected;

	gchar *mac;
	gchar *ipaddr;
	gchar *dest;
	gchar *bcast;
	gchar *mask;

	gulong recv_bytes;
	gulong recv_packets;
	gulong trans_bytes;
	gulong trans_packets;
	gboolean rtstat;

	/* AP */
	gchar            *essid;
	gchar            *bssid;
	gint              quality;
	EncryptionMethod  encryption;
	IEKeyMgmt         keymgmt;
	IECypher          group;
	IECypher          pairwise;
} WirelessInfo;

void lxnm_status_register(const gchar *ifname, ConnectionType conntype, LXNMClient *client, DeviceType devtype);
void lxnm_status_unregister(const gchar *ifname, LXNMClient *client);
ConnectionType lxnm_status_get_device_type(const gchar *ifname);
InterfaceStatus *lxnm_status_get_ifstat(const gchar *ifname);
void lxnm_status_push(InterfaceStatus *ifstat, const gchar *msg);

#endif
