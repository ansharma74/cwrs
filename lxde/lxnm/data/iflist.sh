#/bin/sh

RSLINE="+$LXNM_CMDID"

while read IFNAME TMP; do
	IFNAME=${IFNAME%:*}
	if [ "lo" = "$IFNAME" ]; then
		continue
	elif [ "wmaster" = "${IFNAME/[0-9]/}" ]; then
		continue
	fi

	RSLINE="${RSLINE} ${IFNAME}"
done <<-EOF
`grep ':' /proc/net/dev`
EOF

echo $RSLINE
