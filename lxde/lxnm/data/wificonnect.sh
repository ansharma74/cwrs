#!/bin/sh
# <ifname> <essid> <en_type> <password> <bssid>

stop_daemons() {
	if [ -f /var/run/dhcpcd-"$LXNM_IFNAME".pid ]; then
		kill -9 `cat /var/run/dhcpcd-"$LXNM_IFNAME".pid`
	fi
	if [ -f /var/run/wpa_supplicant.pid ]; then
		kill -9 `cat /var/run/wpa_supplicant.pid`
	fi
}

if [ A"$LXNM_WIFI_PROTO" = A"NONE" ]; then
	# without encryption
	stop_daemons
	wpa_supplicant -g/var/run/wpa_supplicant-global -B -P/var/run/wpa_supplicant.pid

	wpa_cli -g/var/run/wpa_supplicant-global interface_remove $LXNM_IFNAME
	wpa_cli -g/var/run/wpa_supplicant-global interface_add $LXNM_IFNAME "" wext /var/run/wpa_supplicant

	wpa_cli -i$LXNM_IFNAME add_network
	wpa_cli -i$LXNM_IFNAME set_network 0 ssid \""$LXNM_WIFI_ESSID"\"
	wpa_cli -i$LXNM_IFNAME set_network 0 key_mgmt "$LXNM_WIFI_KEYMGMT"
	wpa_cli -i$LXNM_IFNAME set_network 0 psk \""$LXNM_WIFI_KEY"\"
	wpa_cli -i$LXNM_IFNAME set_network 0 pairwise "$LXNM_WIFI_PAIRWISE"
	wpa_cli -i$LXNM_IFNAME set_network 0 group "$LXNM_WIFI_GROUP"
	wpa_cli -i$LXNM_IFNAME set_network 0 proto "$LXNM_WIFI_PROTO"
	wpa_cli -i$LXNM_IFNAME enable_network 0

	dhcpcd --renew $LXNM_IFNAME

elif [ A"$LXNM_WIFI_PROTO" = A"WEP" ]; then
	# WEP
	stop_daemons
	wpa_supplicant -g/var/run/wpa_supplicant-global -B -P/var/run/wpa_supplicant.pid

	wpa_cli -g/var/run/wpa_supplicant-global interface_remove $LXNM_IFNAME
	wpa_cli -g/var/run/wpa_supplicant-global interface_add $LXNM_IFNAME "" wext /var/run/wpa_supplicant

	wpa_cli -i$LXNM_IFNAME add_network
	wpa_cli -i$LXNM_IFNAME set_network 0 ssid \""$LXNM_WIFI_ESSID"\"
	wpa_cli -i$LXNM_IFNAME set_network 0 key_mgmt "$LXNM_WIFI_KEYMGMT"
	wpa_cli -i$LXNM_IFNAME set_network 0 wep_key0 \""$LXNM_WIFI_KEY"\"
	wpa_cli -i$LXNM_IFNAME enable_network 0

	dhcpcd --renew $LXNM_IFNAME

else
	# WPA
	# start trying to associate with the WPA network using SSID test.
	stop_daemons
	wpa_supplicant -g/var/run/wpa_supplicant-global -B -P/var/run/wpa_supplicant.pid

	wpa_cli -g/var/run/wpa_supplicant-global interface_remove $LXNM_IFNAME
	wpa_cli -g/var/run/wpa_supplicant-global interface_add $LXNM_IFNAME "" wext /var/run/wpa_supplicant

	wpa_cli -i$LXNM_IFNAME add_network
	wpa_cli -i$LXNM_IFNAME set_network 0 ssid \""$LXNM_WIFI_ESSID"\"
	wpa_cli -i$LXNM_IFNAME set_network 0 key_mgmt "$LXNM_WIFI_KEYMGMT"
	wpa_cli -i$LXNM_IFNAME set_network 0 psk \""$LXNM_WIFI_KEY"\"
	wpa_cli -i$LXNM_IFNAME set_network 0 pairwise "$LXNM_WIFI_PAIRWISE"
	wpa_cli -i$LXNM_IFNAME set_network 0 group "$LXNM_WIFI_GROUP"
	wpa_cli -i$LXNM_IFNAME set_network 0 proto "$LXNM_WIFI_PROTO"
	wpa_cli -i$LXNM_IFNAME enable_network 0

	dhcpcd --renew $LXNM_IFNAME
fi

