#!/bin/sh
dhcpcd --release $LXNM_IFNAME
ifconfig $LXNM_IFNAME down
