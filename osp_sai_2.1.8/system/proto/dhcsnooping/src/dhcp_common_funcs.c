/****************************************************************************
* $Id$
*  implement dhcp snooping feature
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-24 14:51
* Reason        : First Create.
****************************************************************************/

#include "dhcpd.h"
#include "dhcp_version.h"
#include "dhcp_relayagent.h"

/* common functions for dhcp snooping
 * dhcp libraries need it
 */

/* Needed to prevent linking against conflex.c. */
int lexline;
int lexchar;
char *token_line;
char *tlname;

/* 
 * define common functions for dhcp libraries
 * we don't need implement it!!!
 */
int write_lease (lease)
    struct lease *lease;
{
    return 1;
}

int write_host (host)
    struct host_decl *host;
{
    return 1;
}

int commit_leases ()
{
    return 1;
}

void bootp (packet)
    struct packet *packet;
{
}

void dhcp (packet)
    struct packet *packet;
{
}

int find_subnet (struct subnet **sp,
         struct iaddr addr, const char *file, int line)
{
    return 0;
}

int check_collection (struct packet *p, struct lease *l,
              struct collection *c)
{
    return 0;
}

void classify (struct packet *p, struct class *c)
{
}

isc_result_t find_class (struct class **class, const char *c1,
             const char *c2, int i)
{
    return ISC_R_NOTFOUND;
}

int parse_allow_deny (struct option_cache **oc, struct parse *p, int i)
{
    return 0;
}

/* As a wise man once said in dhcpctl/omshell.c: */
/* Sigh */
isc_result_t dhcp_set_control_state (control_object_state_t oldstate,
                     control_object_state_t newstate)
{
    return ISC_R_SUCCESS;
}

void dhcpv6(struct packet *packet) 
{
}
