/****************************************************************************
* $Id$
*  define struct and macros for relay agent
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-24 14:46
* Reason        : First Create.
****************************************************************************/

#ifndef _DHCP_RELAYAGENT_H_
#define _DHCP_RELAYAGENT_H_

#ifndef ETH_ALEN
#define ETH_ALEN    6
#endif

#define M_AGG_MODULE_ID     254

/* Circuit Id */
typedef struct tagCircuitId
{
    u_int8_t cType; /* circui id */
    u_int8_t cLength; /* length == 4 */
    u_int16_t nVlanId; /* Vlan Id */
    u_int8_t cModule; /* Module Id */
    u_int8_t cPort; /* Port Number */
    /*added by chenc for bug 52447, 2019-06-27*/
#ifndef GREATBELT
    u_int8_t cSubPort; /* Sub Port Number */
#endif
    /*chenc end*/
} __attribute__((packed)) S_DhcpRelayCircuitId;

/* Remote Id */
typedef struct tagRemoteId
{
    u_int8_t cType; /* remote id */
    u_int8_t cLength; /* length == 6 */
    u_int8_t nMacArr[ETH_ALEN]; /* mac address */
} __attribute__((packed)) S_DhcpRelayRemoteId;

#endif /* !_DHCP_RELAYAGENT_H_ */
