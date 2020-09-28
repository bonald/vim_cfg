#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "route.h"
#include "prefix.h"
#include "rt_fea.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_log.h"
#include "arp_api.h"
#include "arp_sm.h"

/* action type str */
static char *arp_act_str[ARP_ACT_MAX] = 
{
    "send",
    "rcvd"
};

/* action type str */
static char *arp_op_str[ARP_OP_MAX] = 
{
    "",
    "req",
    "rep",
    "rreq",
    "rrep",
    "",
    "",
    "",
    "inreq",
    "inrep",
    ""
};

/* action type str */
static char *arp_entry_op_str[ARP_ENTRY_OP_MAX] = 
{
    "creating",
    "removing",
    "updating"
};

int32
arp_debug_entry_trace(tbl_arp_t *ae_p, arp_entry_action_t op)
{
    char ip_addr[INET_ADDRSTRLEN];
    char mac_addr[INET_ADDRSTRLEN];

    ARP_PROTO_DEBUG("IP ARP: %s entry for IP address: %s, hw: %s",
                 arp_entry_op_str[op],
                 sal_inet_ntop(AF_INET, &ae_p->key.ip, ip_addr, INET_ADDRSTRLEN), 
                 cdb_mac_val2str(mac_addr, MAC_ADDR_STR_LEN, ae_p->mac_addr));

    return PM_E_NONE;
}

int32
arp_debug_pkt_trace(char *ifname, arp_eth_t *arp_p, arp_pkt_action_t act)
{
    char ip_addr_s[INET_ADDRSTRLEN];
    char ip_addr_t[INET_ADDRSTRLEN];
    char mac_addr[INET_ADDRSTRLEN];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    ARP_PKT_DEBUG("IP ARP: %s %s src %s %s, dst %s %s",
                       arp_act_str[act],
                       arp_op_str[sal_hton16(arp_p->ar_op)],
                       sal_inet_ntop(AF_INET, arp_p->ar_sip, ip_addr_s, INET_ADDRSTRLEN),
                       cdb_mac_val2str(mac_addr, MAC_ADDR_STR_LEN, arp_p->ar_sha),
                       sal_inet_ntop(AF_INET, arp_p->ar_tip, ip_addr_t, INET_ADDRSTRLEN), 
                       IFNAME_ETH2FULL(ifname, ifname_ext));

    return PM_E_NONE;
}

int32
arp_debug_cache_overflow()
{
    //ARP_LOG_USER(E_WARNING, ARP_4_DB_CACHE_OVERFLOW, LOG_RATE_LIMIT_TYPE_DEF);
    logid_sys(LOG_ARP_4_OVERFLOW);
    
    return PM_E_NONE;
}

