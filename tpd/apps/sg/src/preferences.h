/*---[ preferences.h ]------------------------------------------------
 * Copyright (C) 2000-2004 Tomas Junnonen (majix@sci.fi)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Functions for modifying/reading the program preferences and the
 * preferences GUI
 *--------------------------------------------------------------------*/

#ifndef _FIRESTARTER_PREFERENCES
#define _FIRESTARTER_PREFERENCES

#define PREFS_FIRST_RUN "/apps/firestarter/client/first_run"
#define PREFS_SYSLOG_FILE "/apps/firestarter/client/system_log"

#define PREFS_ENABLE_TRAY_ICON "/apps/firestarter/client/enable_tray_icon"
#define PREFS_MINIMIZE_TO_TRAY "/apps/firestarter/client/minimize_to_tray"

#define PREFS_SKIP_REDUNDANT "/apps/firestarter/client/filter/redundant"
#define PREFS_SKIP_NOT_FOR_FIREWALL "/apps/firestarter/client/filter/not_for_firewall"

#define PREFS_APPLY_POLICY_INSTANTLY "/apps/firestarter/client/policy_auto_apply"

#define PREFS_START_ON_BOOT "/apps/firestarter/client/start_firewall_on_boot"
#define PREFS_START_ON_GUI "/apps/firestarter/client/start_firewall_on_gui"
#define PREFS_START_ON_DIAL_OUT "/apps/firestarter/client/start_firewall_on_dial_out"
#define PREFS_START_ON_DHCP "/apps/firestarter/client/start_firewall_on_dhcp"

#define PREFS_FW_EXT_IF "/apps/firestarter/firewall/ext_if"
#define PREFS_FW_INT_IF "/apps/firestarter/firewall/int_if"
#define PREFS_FW_NAT "/apps/firestarter/firewall/nat"

#define PREFS_FW_DHCP_ENABLE "/apps/firestarter/firewall/dhcp/enable_server"
#define PREFS_FW_DHCP_LOWEST_IP "/apps/firestarter/firewall/dhcp/lowest_ip"
#define PREFS_FW_DHCP_HIGHEST_IP "/apps/firestarter/firewall/dhcp/highest_ip"
#define PREFS_FW_DHCP_NAMESERVER "/apps/firestarter/firewall/dhcp/nameserver"

#define PREFS_FW_FILTER_ICMP "/apps/firestarter/firewall/icmp/enable"
#define PREFS_FW_ICMP_ECHO_REQUEST "/apps/firestarter/firewall/icmp/echo_request"
#define PREFS_FW_ICMP_ECHO_REPLY "/apps/firestarter/firewall/icmp/echo_reply"
#define PREFS_FW_ICMP_TRACEROUTE "/apps/firestarter/firewall/icmp/traceroute"
#define PREFS_FW_ICMP_MSTRACEROUTE "/apps/firestarter/firewall/icmp/mstraceroute"
#define PREFS_FW_ICMP_UNREACHABLE "/apps/firestarter/firewall/icmp/unreachable"
#define PREFS_FW_ICMP_TIMESTAMPING "/apps/firestarter/firewall/icmp/timestamping"
#define PREFS_FW_ICMP_MASKING "/apps/firestarter/firewall/icmp/masking"
#define PREFS_FW_ICMP_REDIRECTION "/apps/firestarter/firewall/icmp/redirection"
#define PREFS_FW_ICMP_SOURCE_QUENCHES "/apps/firestarter/firewall/icmp/source_quenches"

#define PREFS_FW_FILTER_TOS "/apps/firestarter/firewall/tos/enable"
#define PREFS_FW_TOS_CLIENT "/apps/firestarter/firewall/tos/client"
#define PREFS_FW_TOS_SERVER "/apps/firestarter/firewall/tos/server"
#define PREFS_FW_TOS_X "/apps/firestarter/firewall/tos/x"
#define PREFS_FW_TOS_OPT_TROUGHPUT "/apps/firestarter/firewall/tos/optimize_troughput"
#define PREFS_FW_TOS_OPT_RELIABILITY "/apps/firestarter/firewall/tos/optimize_reliability"
#define PREFS_FW_TOS_OPT_DELAY "/apps/firestarter/firewall/tos/optimize_delay"

#define PREFS_FW_DENY_PACKETS "/apps/firestarter/firewall/deny_packets"
#define PREFS_FW_BLOCK_EXTERNAL_BROADCAST "/apps/firestarter/firewall/block_external_broadcast"
#define PREFS_FW_BLOCK_INTERNAL_BROADCAST "/apps/firestarter/firewall/block_internal_broadcast"
#define PREFS_FW_BLOCK_NON_ROUTABLES "/apps/firestarter/firewall/block_non_routables"

#define PREFS_FW_RESTRICTIVE_OUTBOUND_MODE "/apps/firestarter/firewall/restrictive_outbound"

#define PREFS_HITVIEW_TIME_COL "/apps/firestarter/client/ui/hitview_time_col"
#define PREFS_HITVIEW_DIRECTION_COL "/apps/firestarter/client/ui/hitview_direction_col"
#define PREFS_HITVIEW_IN_COL "/apps/firestarter/client/ui/hitview_in_col"
#define PREFS_HITVIEW_OUT_COL "/apps/firestarter/client/ui/hitview_out_col"
#define PREFS_HITVIEW_PORT_COL "/apps/firestarter/client/ui/hitview_port_col"
#define PREFS_HITVIEW_SOURCE_COL "/apps/firestarter/client/ui/hitview_source_col"
#define PREFS_HITVIEW_DESTINATION_COL "/apps/firestarter/client/ui/hitview_destination_col"
#define PREFS_HITVIEW_LENGTH_COL "/apps/firestarter/client/ui/hitview_length_col"
#define PREFS_HITVIEW_TOS_COL "/apps/firestarter/client/ui/hitview_tos_col"
#define PREFS_HITVIEW_PROTOCOL_COL "/apps/firestarter/client/ui/hitview_protocol_col"
#define PREFS_HITVIEW_SERVICE_COL "/apps/firestarter/client/ui/hitview_service_col"

#include <config.h>
#include <gnome.h>

void preferences_check_schema (void);

gboolean preferences_get_bool   (const gchar *gconf_key);
void     preferences_set_bool   (const gchar *gconf_key, gboolean data);
gchar   *preferences_get_string (const gchar *gconf_key);
void     preferences_set_string (const gchar *gconf_key, const gchar *data);

void preferences_update_conf_from_widget (GtkWidget *widget, const gchar *gconf_key);
void preferences_update_widget_from_conf (GtkWidget *widget, const gchar *gconf_key);

void preferences_show (void);

#endif
