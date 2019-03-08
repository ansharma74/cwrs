#!/bin/sh
ifconfig $LXNM_IFNAME up
dhcpcd --renew $LXNM_IFNAME
