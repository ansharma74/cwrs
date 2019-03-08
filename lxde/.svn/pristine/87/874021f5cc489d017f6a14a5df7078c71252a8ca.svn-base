/**
 * Copyright (c) 2009 Fred Chien <cfsghost@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundatoin; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundatoin,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "tdb.h"

typedef struct {
	int pppd_pid;
	char *ifname;
	char *device;
	char *ipaddr;
	char *dest;
	char *dns_primary;
	char *dns_secondary;
} GPPPStatData;

typedef enum {
	GPPPSTAT_MODE_USE_DEVICE_NAME,
	GPPPSTAT_MODE_USE_IF_NAME
} GPPPStatMode;

const char options[] = {
	"getpppstat - a utility for getting PPP informatoins\n"
	"Usage:\n"
	"  getpppstat [-d|-u] [options...]\n"
	"Options:\n"
	"    -d <device>          Device name\n"
	"    -i <interface>       PPP virtual network interface\n"
};

static char *parse_str(char *str, const char *key)
{
	char *p, *endp;
	char *string;
	int i, found;

	if (!str)
		return 0;

	p = strstr(str, key);
	if (p) {
		found = 0;
		for (endp=p;*endp!='\0';endp++) {
			if (*endp==';')
				break;

			if (!found&&*endp=='=') {
				i = 0;
				p = endp+1;
				found = 1;
			} else
				i++;
		}

		if (i>0) {
			string = (char *)malloc(sizeof(char)*(i+1));
			strncpy(string, p, i);
			*(string+i) = '\0';
			return string;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	char *value;
	int unit = -1;
	GPPPStatMode gpppsmode;
	GPPPStatData *gpppsdata = NULL;
	TDB_CONTEXT *pppdb;
	TDB_DATA key;
	TDB_DATA pid;
	TDB_DATA rec;

	if (argc!=3) {
		printf("%s", options);
		return 1;
	}

	if (strcmp(argv[1], "-d")==0) {
		gpppsmode = GPPPSTAT_MODE_USE_DEVICE_NAME;
	} else {
		gpppsmode = GPPPSTAT_MODE_USE_IF_NAME;
	}

	/* open database */
	pppdb = tdb_open("/var/run/pppd2.tdb", 0, 0, O_RDONLY, 0644);
	if (!pppdb)
		pppdb = tdb_open("/etc/ppp/pppd2.tdb", 0, 0, O_RDWR, 0644);

	if (pppdb!=NULL) {
		/* getting first key */
		key = tdb_firstkey(pppdb);
		while (key.dptr != NULL) {
			/* read data with the key */
			pid = tdb_fetch(pppdb, key);
			if (pid.dptr != NULL && pid.dsize > 0) {
				pid.dptr[pid.dsize-1] = '\0';
				if (strncmp(pid.dptr, "pppd", 4)!=0) {
					if (gpppsmode==GPPPSTAT_MODE_USE_DEVICE_NAME) {
						value = parse_str(pid.dptr, "DEVICE");
						if (value) {
							if (strcmp(value, argv[2])==0) {
								gpppsdata = (GPPPStatData *)malloc(sizeof(GPPPStatData));
								gpppsdata->ifname = parse_str(pid.dptr, "IFNAME");
								gpppsdata->device = value;
								value = parse_str(pid.dptr, "PPPD_PID");
								gpppsdata->pppd_pid = atoi(value);
								gpppsdata->dest = parse_str(pid.dptr, "IPREMOTE");
								gpppsdata->dns_primary = parse_str(pid.dptr, "DNS1");
								gpppsdata->dns_secondary = parse_str(pid.dptr, "DNS2");
								break;
							} else
								free(value);
						}
					} else {
						value = parse_str(pid.dptr, "IFNAME");
						if (value) {
							if (strcmp(value, argv[2])==0) {
								gpppsdata = (GPPPStatData *)malloc(sizeof(GPPPStatData));
								gpppsdata->device = parse_str(pid.dptr, "DEVICE");
								gpppsdata->ifname = value;
								value = parse_str(pid.dptr, "PPPD_PID");
								gpppsdata->pppd_pid = atoi(value);
								gpppsdata->ipaddr = parse_str(pid.dptr, "IPLOCAL");
								gpppsdata->dest = parse_str(pid.dptr, "IPREMOTE");
								gpppsdata->dns_primary = parse_str(pid.dptr, "DNS1");
								gpppsdata->dns_secondary = parse_str(pid.dptr, "DNS2");
								break;
							} else
								free(value);
						}
					}
				}
			}

			/* getting next key */
			key = tdb_nextkey(pppdb, key);
		}

		tdb_close(pppdb);

		if (gpppsdata) {
			printf("%d", gpppsdata->pppd_pid);
			printf(" %s", gpppsdata->ifname ? gpppsdata->ifname : "(null)");
			printf(" %s", gpppsdata->device ? gpppsdata->device : "(null)");
			printf(" %s", gpppsdata->ipaddr ? gpppsdata->ipaddr : "(null)");
			printf(" %s", gpppsdata->dest ? gpppsdata->dest : "(null)");

			if (gpppsdata->dns_primary)
				printf(" %s", gpppsdata->dns_primary);

			if (gpppsdata->dns_secondary)
				printf(" %s", gpppsdata->dns_secondary);

			printf("\n");
			return 0;
		}
	}

	return 1;
}

