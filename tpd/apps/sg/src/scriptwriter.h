/*---[ scriptwriter.h ]-----------------------------------------------
 * Copyright (C) 2000 Tomas Junnonen (majix@sci.fi)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Functions to write firewall shell scripts
 *--------------------------------------------------------------------*/

#ifndef _FIRESTARTER_SCRIPTWRITER
#define _FIRESTARTER_SCRIPTWRITER

#include <config.h>
#include <gnome.h>
#include "wizard.h"
#include "policyview.h"

#define RETURN_EXT_FAILED 2
#define RETURN_INT_FAILED 3
#define RETURN_NO_IPTABLES 100

#define FIRESTARTER_CONTROL_SCRIPT       FIRESTARTER_RULES_DIR "/firestarter/firestarter.sh"
#define FIRESTARTER_FIREWALL_SCRIPT      FIRESTARTER_RULES_DIR "/firestarter/firewall"
#define FIRESTARTER_CONFIGURATION_SCRIPT FIRESTARTER_RULES_DIR "/firestarter/configuration"
#define FIRESTARTER_SYSCTL_SCRIPT        FIRESTARTER_RULES_DIR "/firestarter/sysctl-tuning"
#define FIRESTARTER_USER_PRE_SCRIPT      FIRESTARTER_RULES_DIR "/firestarter/user-pre"
#define FIRESTARTER_USER_POST_SCRIPT     FIRESTARTER_RULES_DIR "/firestarter/user-post"
#define FIRESTARTER_NON_ROUTABLES_SCRIPT FIRESTARTER_RULES_DIR "/firestarter/non-routables"
#define FIRESTARTER_FILTER_HOSTS_SCRIPT  FIRESTARTER_RULES_DIR "/firestarter/events-filter-hosts"
#define FIRESTARTER_FILTER_PORTS_SCRIPT  FIRESTARTER_RULES_DIR "/firestarter/events-filter-ports"
#define FIRESTARTER_INBOUND_SETUP        POLICY_IN_DIR"/setup"
#define FIRESTARTER_OUTBOUND_SETUP       POLICY_OUT_DIR"/setup"

gboolean script_exists (void);
void scriptwriter_output_scripts (void);

void scriptwriter_output_firestarter_script (void);
void scriptwriter_output_configuration (void);

void scriptwriter_write_ppp_hook (void);
void scriptwriter_remove_ppp_hook (void);

void scriptwriter_write_dhcp_hook (void);
void scriptwriter_remove_dhcp_hook (void);

gboolean scriptwriter_versions_match (void);

#endif
