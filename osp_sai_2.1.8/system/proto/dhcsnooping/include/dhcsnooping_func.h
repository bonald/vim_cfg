/****************************************************************************
* $Id: nsm_dhcsnooping.h,v 1.1 2007/08/13 01:26:51 xliu Exp $
*  structs and macros for dhcp snooping feature
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-24 14:53
* Reason        : First Create.
****************************************************************************/
//#include "../../../apps/dhcp/includes/dhcpd.h"

#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"

#ifndef _DHCSNOOPING_H_
#define _DHCSNOOPING_H_

/*
typedef struct hardware_dhcs {
    u_int8_t hlen;
    u_int8_t hbuf[21];
}hardware_t;
*/

typedef enum dhcp_snooping_error_no
{
    DHCSNOOPING_SUCCESS                       = 0,
    DHCSNOOPING_ERR_INVALIS_PARAMETER         = -100,
    DHCSNOOPING_ERR_L3_IF                     = -99,
    DHCSNOOPING_ERR_AGG_MEMBER                = -98, 
    DHCSNOOPING_ERR_NO_MEMERY                 = -97, 
    DHCSNOOPING_ERR_ENTRY_UNEXIST             = -96, 
    DHCSNOOPING_ERR_STRING_LENGTH             = -95, 
    DHCSNOOPING_ERR_NO_HOSTNAME               = -94, 
} dhcp_snooping_error_no_t;

/* internal service enable/disable flag for v4/v6 */
#define DHS_F_V4 1
#define DHS_F_V6 2

#ifndef MAC_IS_BROADCAST
#define MAC_IS_BROADCAST(laddr)    ( \
    *(u_int32_t *)laddr == 0xffffffff && \
    *(u_int16_t *)&laddr[4] == 0xffff \
)

#define MAC_IS_UNSPECIFIED(laddr)  ( \
    *(u_int32_t *)laddr == 0 && \
    *(u_int16_t *)&laddr[4] == 0 \
)

#define MAC_IS_MULTICAST(laddr)    ( \
    laddr[0] & 0x01 \
)
#endif

#define DHCPSNOOPING_ITEM_STATE_REQUEST     0x01 /* just after request */
#define DHCPSNOOPING_ITEM_STATE_RUNNING     0x02 /* ready now */

#define DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR    0x00 /*no add to ip source guard */
#define DHCPSNOOPING_ITEM_ADD_IPSOUR        0x01 /*add to ip source guard */

/* for bug 22282 modified by liuyang 2013-2-19 */
#define DHCPSNOOPING_ITEM_TYPE_ALL          0x03      /* ALL */

extern tbl_dhcbinding_master_t *_g_p_tbl_dhcbinding_master;
#define DHCSNOOPING_HASH    (_g_p_tbl_dhcbinding_master->dhcbinding_hash)
#define DHCSNOOPING_HASH_IP (_g_p_tbl_dhcbinding_master->dhcbinding_ip_hash)
#define DHCSNOOPING_QUEUE   (_g_p_tbl_dhcbinding_master->dhcbinding_list)
#define DHCSNOOPING6_CUR_BINDINGS   _g_p_tbl_dhcbinding_master->dhcbindingv6_count
#define DHCSNOOPING_CUR_BINDINGS    _g_p_tbl_dhcbinding_master->dhcbinding_count

/* statistics */
typedef struct tagDhcpSnoopingStat
{
    unsigned long long ullDhcpPkts; /* DHCP messages */
    unsigned long long ullBootpPkts; /* BOOTP messages */
    
    unsigned long long ullInvalid; /* invalid pkts */
    unsigned long long ullForwarded; /* forwarded pkts */
    unsigned long long ullMacVerifyFailed;   /* MAC verify failed */
    unsigned long long ullDropped; /* drop ports */
#ifdef HAVE_IPV6
    unsigned long long ullDhcpPkts6; /* DHCPv6 messages */
    unsigned long long ullInvalid6; /* invalid pkts */
    unsigned long long ullForwarded6; /* forwarded pkts */
    unsigned long long ullDropped6; /* drop ports */
    unsigned long long ullMacVerifyFailed6;   /* MAC verify failed */
#endif
} S_DhcpSnoopingStat;

/* return CLI_SUCCESS to indicate match */
typedef int(*DHCSNOOPING_HASH_CB_FUNC)(void *pItem, void *);

typedef struct {
    unsigned char dest_mac[ETH_ALEN];   /* Destination Mac address */
    unsigned char src_mac[ETH_ALEN];    /* Source Mac address */
    u_int32_t ifindex;  /* Outgoing/Incoming interface index */
    u_int16_t vlanid;   /* vlan id */
    u_int16_t type;     /* pkt type */
} DhcSnoopingCMsg_t;

struct hardware_ds {
    u_int8_t hlen;
    u_int8_t hbuf[21];
};

/* DHCP snooping Pkt header */
typedef struct tagDhcpSnoopingInHdr
{
    struct udphdr *udp; /* udp header */
    struct dhcp_packet *dhcp; /* dhcp header */
#ifdef HAVE_IPV6
    struct dhcpv6_packet *dhcpv6; /* for DHCPv6 */
    struct dhc6_ia *bindings; /* ipv6 bindings */
#endif
    int nDhcpLen; /* dhcp packet size */
    struct hardware_ds stHAddr; /* Hardware address */
    //hardware_t stHAddr;
    
    int nTotalLen; /* total packet size */
    int bIPv6; /* is DHCPv6 ? */
    union {
        struct ip *ipv4h; /* ip header */
#ifdef HAVE_IPV6
        struct ip6_hdr *ipv6h; /* ipv6 header */
#endif
    } hdru;
} S_DhcpSnoopingInHdr;
#define uiph hdru.ipv4h
#ifdef HAVE_IPV6
#define uip6h hdru.ipv6h
#endif

int 
dhcsnooping_vlan_cnt_enabled_check();

int
dhcsnooping_service_onoff();

#endif /* !_DHCSNOOPING_H_ */
