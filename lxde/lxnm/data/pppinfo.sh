#/bin/sh

if [ ! A"$LXNM_IFNAME" = A ]; then
read PPPDPID IFNAME DEVICE NOTIMPORTANT <<-EOF
$(getpppstat -i $LXNM_IFNAME)
EOF
elif [ ! A"$LXNM_DEVNAME" = A ]; then
read PPPDPID IFNAME DEVICE NOTIMPORTANT <<-EOF
$(getpppstat -d /dev/$LXNM_DEVNAME)
EOF
fi

if [ ! A$PPPDPID = A ]; then
	echo "+$LXNM_CMDID 1 $IFNAME ${DEVICE##*/}"
else
	echo "+$LXNM_CMDID 0"
fi
