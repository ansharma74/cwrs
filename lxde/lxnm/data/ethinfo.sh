#/bin/sh

LXNM_ENABLE=0
LXNM_PLUGED=`cat /sys/class/net/${LXNM_IFNAME}/carrier`
LXNM_CONNECTED=0
LXNM_MAC=
LXNM_IPADDR=
LXNM_BROADCAST=
LXNM_NETMASK=
LXNM_DEST=0.0.0.0
LXNM_RX_BYTES=
LXNM_RX_PACKETS=
LXNM_TX_BYTES=
LXNM_TX_PACKETS=
LXNM_ESSID=
LXNM_BSSID=
LXNM_QUALITY=0

while read ARG1 ARG2 ARG3 ARG4 ARG5 ARG6 ARG7; do
	if [ "$ARG1" = "UP" ]; then
		LXNM_ENABLE=1
	fi

	if [ "$ARG4" = "HWaddr" ]; then
		LXNM_MAC=$ARG5
	elif [ "$ARG1" = "inet" ]; then
		LXNM_IPADDR=${ARG2#*:}
		LXNM_BROADCAST=${ARG3#*:}
		LXNM_NETMASK=${ARG4#*:}
		LXNM_CONNECTED=1
	elif [ "$ARG1" = "RX" ] && [ "${ARG2%:*}" = "packets" ]; then
		LXNM_RX_PACKETS=${ARG2#*:}
	elif [ "$ARG1" = "TX" ] && [ "${ARG2%:*}" = "packets" ]; then
		LXNM_TX_PACKETS=${ARG2#*:}
	elif [ "$ARG1" = "RX" ] && [ "$ARG5" = "TX" ]; then
		LXNM_RX_BYTES=${ARG2#*:}
		LXNM_TX_BYTES=${ARG6#*:}
	fi
done <<-EOF
`ifconfig $LXNM_IFNAME`
EOF

while read ARG1 ARG2 ARG3 ARG4 ARG5 ARG6; do
	if [ "$ARG6" = "$LXNM_IFNAME" ]; then
		LXNM_DEST=$ARG1
		break
	fi
done <<-EOF
`cat /proc/net/arp`
EOF

echo -e "+$LXNM_CMDID $LXNM_ENABLE\t$LXNM_PLUGED\t$LXNM_CONNECTED\t$LXNM_MAC\t$LXNM_IPADDR\t$LXNM_DEST\t$LXNM_BROADCAST\t$LXNM_NETMASK\t$LXNM_RX_BYTES\t$LXNM_RX_PACKETS\t$LXNM_TX_BYTES\t$LXNM_TX_PACKETS"
