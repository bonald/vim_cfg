/****************************************************************************
* $Id: nsm_dhcsnooping_main.c,v 1.2 2007/08/27 08:36:18 xliu Exp $
*  implement for dhcp snooping feature
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-24 15:11
* Reason        : First Create.
****************************************************************************/
#include "dhcpd.h"
#include "dhcp_version.h"
#include "dhcp_relayagent.h"

#include "proto.h"
#include "ctclib_hash.h"
#include "param_check.h"
#include "ctc_msg.h"
#include "genlog.h"
#include "glb_const.h"
#include "ctc_task.h"
#include "ctc_sock.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"

#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"

#include "lib_fs.h"
#include "param_check.h"

#include "netinet/ip.h"
#include "netinet/udp.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include <linux/ip.h>
#include <netinet/ip6.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "dhcsnooping_func.h"
#include "dhcsnooping.h"
#include "glb_message_define.h"
#include "../../switch/include/ipsour/ip_source.h"

//extern int ipsour_dynamic_add_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if);
//extern int ipsour_dynamic_del_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if);



#ifndef IN_CLASSD
#define IN_CLASSD(i)	((((long)(i))&0xf0000000)==0xe0000000)
#define IN_MULTICAST IN_CLASSD
#define ITIMER_REAL 0
#define MAXHOSTNAMELEN 64
#endif

/* getting informations from interface name */
#define DHCSNOOPING_VLAN_PREFIX   "vlan"
#define DHCSNOOPING_INTF_PREFIX   "eth-"
#define DHCSNOOPING_AGG_PREFIX   "agg"
#define DHCSNOOPING_TUNNEL_PREFIX   "tunnel"

#ifndef HASH_CLEAN_FUNC
typedef void (*HASH_CLEAN_FUNC) (void *);
#endif
void dhcsnooping_binding_hash_del(tbl_dhcbinding_t *pItem);

DEBUG_DECLAR(dhcp_debug);
static int g_nV4V6Flags = 0; /* V4/V6 snooping service status */

#ifdef HAVE_IPV6
void
dhcsnoopingv6_bindings_destroy(struct dhc6_ia **ppbindings, const char *file, int line);

extern isc_result_t
dhcsnoopingv6_parse_ia_na(struct dhc6_ia **pia, struct packet *packet,
            struct option_state *options);
extern isc_result_t
dhcsnoopingv6_parse_ia_ta(struct dhc6_ia **pia, struct packet *packet,
            struct option_state *options);
extern void dhcsnooping_save_timer6(void *);
void dhcsnooping_save_timer(void *pArg);
int init_binding_table6();
int dhcsnoopingv6_get_one(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pmsg);
#endif

/* Global variable container. */
/* snooping statistics */
S_DhcpSnoopingStat g_stStat;
dhcp_role_t g_dhcp_role         = DHCP_ROLE_SNOOPING; /*added by liuyang 2012-7-26*/
ctc_task_t *g_p_binding_timeout = NULL; /* binding entry timeout task */
ctc_task_t *g_p_binding_saver   = NULL; /* binding entry save task */

/* modified by liwh for bug 50994, 2019-02-13 */
static int g_snooping_service_on = 0;
/* liwh end */

int dhcsnooping_binding_queue_update(tbl_dhcbinding_t *);
void dhcsnooping_dump_packet (struct packet *tp);

void dhcsnooping_binding_timeout(void *);
int init_binding_table();
int dhcsnooping_is_server_side_msg(u_int type);
int32 dhcsnooping_process_l3pdu();
int32
dhcsnooping_add_tbl_to_ipsg(tbl_dhcbinding_t *p_db_item);

/* dhcp message strings */
static const char *dhcp_type_names [] = {
    "DHCPDISCOVER",
    "DHCPOFFER",
    "DHCPREQUEST",
    "DHCPDECLINE",
    "DHCPACK",
    "DHCPNAK",
    "DHCPRELEASE",
    "DHCPINFORM"
};
const int dhcp_type_name_max = ((sizeof dhcp_type_names) / sizeof (char *));

int is_dhcrelay_service_enabled(void)
{
    tbl_sys_global_t *pCfg = tbl_sys_global_get_sys_global();
    if (pCfg->dhcp_service_enable && pCfg->dhcp_relay_enable) {
        return 1;
    }
    return 0;
}

int 
dhcsnooping_vlan_cnt_enabled_check()
{
    tbl_dhcsnooping_t *p_db_dhcsnooping = NULL;
    tbl_vlan_t      *p_db_vlan          = NULL;
    tbl_vlan_key_t  vlan_key;
    vid_t           vid = 0;
    int             ret = 0;
    uint32          allowed_vlan_count = 0;
    
    p_db_dhcsnooping = tbl_dhcsnooping_get_dhcsnooping();
    if (NULL == p_db_dhcsnooping)
    {
        return 0;
    }

    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    GLB_BMP_ITER_BEGIN(p_db_dhcsnooping->vlans, vid)
    {
        vlan_key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            continue;
        }

        allowed_vlan_count++;
    }
    GLB_BMP_ITER_END(p_db_dhcsnooping->vlans, vid);

    if (allowed_vlan_count)
    {
        ret = 1;
    }

    return ret;
}

int is_dhcsnooping_service_enabled(void)
{
    tbl_sys_global_t *pCfg = tbl_sys_global_get_sys_global();
    int ret = 0;

    if (pCfg->dhcp_service_enable && pCfg->dhcp_snooping_enable
            && dhcsnooping_vlan_cnt_enabled_check()) 
    {
        ret = 1;
    }
    
    return ret;
}

int is_dhcsnoopingv6_service_enabled(void)
{
#if 0
    tbl_sys_global_t *pCfg = tbl_sys_global_get_sys_global();

    if (pCfg->dhcpv6_service_enable && pCfg->dhcp_snoopingv6_enable) {
        return 1;
    }
#endif
    return 0;
}

void reset_dhcsnooping_statistics()
{
    memset(&g_stStat, 0x00, sizeof(S_DhcpSnoopingStat));
    return;
}

void output_dhcsnooping_statistics(FILE *fp)
{
    fprintf(fp, "/dhcp_packets=%llu", g_stStat.ullDhcpPkts);
    fprintf(fp, "/bootp_packets=%llu", g_stStat.ullBootpPkts);
    fprintf(fp, "/invalid_packets=%llu", g_stStat.ullInvalid);
    fprintf(fp, "/forwarded_packets=%llu", g_stStat.ullForwarded);
    fprintf(fp, "/mac_verify_failed_packets=%llu", g_stStat.ullMacVerifyFailed);
    fprintf(fp, "/dropped_packets=%llu", g_stStat.ullDropped);
    return;
}


int
dhcsnooping_get_assignedip(S_DhcpSnoopingInHdr *pHdr, 
    struct packet *pDhcpPkt, addr_t *psu)
{
    if (NULL == pHdr || NULL == pDhcpPkt || NULL == psu) {
        return -1;
    }

    memset(psu, 0x00, sizeof(addr_t));
    if (!pHdr->bIPv6) {
        psu->family = AF_INET;
        psu->u.prefix4.s_addr = pHdr->dhcp->yiaddr.s_addr;
        return 0;
    }

#ifdef HAVE_IPV6
    if (NULL == pDhcpPkt->options) {
        return -1;
    }

    if (dhcsnoopingv6_parse_ia_na(&pHdr->bindings, pDhcpPkt,
                pDhcpPkt->options) != ISC_R_SUCCESS) {
        dhcsnoopingv6_bindings_destroy(&pHdr->bindings, MDL);
        return -1;
    }

    if (dhcsnoopingv6_parse_ia_ta(&pHdr->bindings, pDhcpPkt,
                pDhcpPkt->options) != ISC_R_SUCCESS) {
        dhcsnoopingv6_bindings_destroy(&pHdr->bindings, MDL);
        return -1;
    }

    if (NULL != pHdr->bindings) {
        struct dhc6_addr *pa = pHdr->bindings->addrs;
        if (NULL != pa) {
            psu->family = AF_INET6;
            memcpy(psu->u.prefix6.s6_addr, pa->address.iabuf, 16);
            return 0;
        }
    }
#endif

    return -1;
}

/* modified by liuyang for DUID without real mac address of src mac 2012-8-11*/
int
dhcsnooping_get_clientmac(S_DhcpSnoopingInHdr *pHdr, struct packet *pDhcpPkt,
                                DhcSnoopingCMsg_t *pcmsg, char *pszRet)
{
    if (NULL == pHdr || NULL == pszRet || NULL == pDhcpPkt || NULL == pcmsg) {
        return -1;
    }

    if (!pHdr->bIPv6) {
        if (pHdr->dhcp) {
            memcpy(pszRet, pHdr->dhcp->chaddr, ETH_ALEN);
            return 0;
        }

        return -1;
    }
#ifdef HAVE_IPV6
    else {   
        if (DHCPV6_REPLY == pDhcpPkt->dhcpv6_msg_type) {
            memcpy(pszRet, pcmsg->dest_mac, ETH_ALEN);      
        } else {
            memcpy(pszRet, pcmsg->src_mac, ETH_ALEN); 
        }   
    }
#endif
    return 0;
}

int
dhcsnooping_set_transid(S_DhcpSnoopingInHdr *pHdr, struct packet *pDhcpPkt,
                         tbl_dhcbinding_t *pItem)
{
    unsigned char *uc;

    if (NULL == pHdr || NULL == pDhcpPkt || NULL == pItem) {
        return -1;
    }

    if (!pHdr->bIPv6) {
        if (NULL == pDhcpPkt->raw) {
            return -1;
        }

        pItem->transid = pDhcpPkt->raw->xid;
        return 0;
    }

    uc = (unsigned char *)&pItem->transid;
    *uc = pDhcpPkt->dhcpv6_transaction_id[0];
    *(uc + 1) = pDhcpPkt->dhcpv6_transaction_id[1];
    *(uc + 2) = pDhcpPkt->dhcpv6_transaction_id[2];
    *(uc + 3) = 0x00;
    return 0;
}

/* add by wangjj for IP source guard */


/*******************************************************************************
 * Name:    dhcp_snooping_send_binding_table_msg
 * Purpose: send dhcp snooping binding table update message to nsm
 * Input: 
 *   pItem: binding item
 *   nCmd: command type (NSM_BINDING_TABLE_CMD_ADD/DEL/UPDATE/FLUSH)
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcp_snooping_send_binding_table_msg(tbl_dhcbinding_t *pItem, int nCmd)
{
    tbl_interface_t *p_db_if = NULL;

    /* begine add by zongyi at 2007-11-16 for ip source guard */
    if (NULL == pItem)
    {
        sal_assert(0);
        return -1;
    }

    switch (nCmd)
    {
       case NSM_BINDING_TABLE_CMD_ADD:
       {
           p_db_if = tbl_interface_get_interface_by_name((char*)pItem->ifname);
           if (NULL == p_db_if || (!if_check_phy_port_by_name(pItem->ifname) && sal_strncasecmp((char*)pItem->ifname, "agg", 3)))
           {
               return -1;
           }

           //return ipsour_dynamic_add_callback(pItem, p_db_if);
       }
       case NSM_BINDING_TABLE_CMD_DEL:
       {
           p_db_if = tbl_interface_get_interface_by_name((char*)pItem->ifname);
           if (NULL == p_db_if || (!if_check_phy_port_by_name(pItem->ifname) && sal_strncasecmp((char*)pItem->ifname, "agg", 3)))
           {
               return -1;
           }

           //return ipsour_dynamic_del_callback(pItem, p_db_if);
       }
       default:
       {
            return -1;
       }
    }

    return 0;
}


/*******************************************************************************
 * Name:    dhcpsnooping_add_ipsour_item
 * Purpose: add an ip source item
 * Input: 
 *  pItem
 * Output: 
 * Note: for bug 19756 add rate limit for log by liuyang 2012-7-4
 ******************************************************************************/
void 
dhcpsnooping_add_ipsour_item(tbl_dhcbinding_t *pItem)
{
    int iRet = 0;

    /* ipsg not support ipv6 now*/
    if (pItem->key.ipv6)
    {
        return;
    }
    
    if (DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR == pItem->cIpsour)
    {
        iRet = dhcp_snooping_send_binding_table_msg(pItem, NSM_BINDING_TABLE_CMD_ADD);
        if (IPSOUR_DUPLICATE == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_INFORMATIONAL, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "there is a same item in ip source guard already");
        }
        else if (IPSOUR_REACH_PORT_MAX == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_WARNING, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "reach ip source guard port number limitation");
        }
        else if (IPSOUR_REACH_TOTAL_MAX == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_WARNING, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "reach ip source guard total number limitation");
        }
        else if (IPSOUR_HAL_FAILED == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "add ip source binding to tcam error");
        }
        else if (IPSOUR_NO_SUCH_TYPE == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "ip source guard type error");
        }
        else if (-1 == iRet)
        {
            log_sys_rate_limit(M_MOD_DHCPSNOOPING, E_ERROR, LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT,
                "bad interface name");
        }
        else if (IPSOUR_OK == iRet)
        {
            pItem->cIpsour = DHCPSNOOPING_ITEM_ADD_IPSOUR;
        }
    }

    return;
}

/*******************************************************************************
 * Name:    dhcpsnooping_add_ipsour_item
 * Purpose: add an ip source item
 * Input: 
 *  pItem
 * Output: 
 * Note: 
 ******************************************************************************/
void
dhcpsnooping_del_ipsour_item(tbl_dhcbinding_t *pItem)
{
    int iRet = 0;
    if (DHCPSNOOPING_ITEM_ADD_IPSOUR == pItem->cIpsour)
    {
        iRet = dhcp_snooping_send_binding_table_msg(pItem, NSM_BINDING_TABLE_CMD_DEL);
        if (IPSOUR_HAL_FAILED == iRet)
        {
            log_sys(M_MOD_DHCPSNOOPING, E_ERROR, "delete ip source binding from tcam error.");
        }
        else if (IPSOUR_NO_SUCH_TYPE == iRet)
        {
            log_sys(M_MOD_DHCPSNOOPING, E_ERROR, "ip source guard type error.");
        }
        else if (-1 == iRet)
        {
            log_sys(M_MOD_DHCPSNOOPING, E_ERROR, "bad interface name.");
        }
        else if (IPSOUR_OK == iRet)
        {
            pItem->cIpsour = DHCPSNOOPING_ITEM_NOT_ADD_IPSOUR;
        }
    }

    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_get_msg_name
 * Purpose: get dhcp message name
 * Input: 
 *   type: message type
 * Output: 
 * Return: 
 *   success: name
 *   failed : "unknown"
 * Note: 
 ******************************************************************************/
const char *
dhcsnooping_get_msg_name(u_int type)
{
    if (type >= DHCPDISCOVER && type <= DHCPINFORM) {
        return dhcp_type_names[type - 1];
    }
    
    return "unknown";
}

/*******************************************************************************
 * Name:    dhcsnooping_bindinghash_make 
 * Purpose: get hash key
 * Input: 
 *   pItem: binding item
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
u_int32_t dhcsnooping_bindinghash_make (tbl_dhcbinding_t *pItem)
{
    u_int32_t val = 0;
    int i;
    
    if (NULL != pItem) {
        for (i = 0; i < ETH_ALEN; i++) {
            val += pItem->key.mac[i];
        }
        val += pItem->key.ipv6;
    }
    
    return val;

}

/*******************************************************************************
 * Name:    dhcsnooping_bindinghash_make_ip
 * Purpose: get hash key
 * Input: 
 *   pItem: binding item
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: add by George wei
 ******************************************************************************/
u_int32_t dhcsnooping_bindinghash_make_ip (tbl_dhcbinding_t *pItem)
{
    u_int32_t val = 0;
    
    if (NULL != pItem) {
        if(pItem->client_ip.family == AF_INET) {
            val = pItem->client_ip.u.prefix4.s_addr;
        }
#ifdef HAVE_IPV6
        else {
            int i;

            for (i = 0; i < 4; i++) {
                val += pItem->client_ip.u.prefix6.s6_addr32[i];
            }
        }
#endif
        val += pItem->key.ipv6;
    }
    
    return val;
}

/*for bug 20011 added by liuyang 2012-8-21*/
int dhcsnooping_bindinghash_intf_cmp(tbl_dhcbinding_t *pa,
        tbl_dhcbinding_t *pfind)
{
    /* processing request pkt*/
    if ('\0' != pfind->ifname[0]) {
        if (DHCBINDING_STATE_RUNNING == pfind->state) {
            if (!strcasecmp(pa->ifname, pfind->ifname)) {
                return 1;
            }                    
        } else if (DHCBINDING_STATE_REQUEST == pfind->state) {
            return 1;                    
        }

        return 0;
    }

    /* processing ack or reply pkt*/
    return 1;
}

/*******************************************************************************
 * Name:    dhcsnooping_bindinghash_cmp 
 * Purpose: hash compare function
 * Input: 
 *   a: item a
 *   b: item b
 * Output: 
 * Return: 0, 1
 * Note:  for bug 20011 modified by liuyang 2012-8-21
 ******************************************************************************/
int dhcsnooping_bindinghash_cmp (void *a, void *b)
{
    tbl_dhcbinding_t *pa = (tbl_dhcbinding_t *)a;
    tbl_dhcbinding_t *pfind = (tbl_dhcbinding_t *)b;

    if (!memcmp(&pa->key, &pfind->key, sizeof(tbl_dhcbinding_key_t))) {
        return 1;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_bindinghash_cmp_ip 
 * Purpose: hash compare function
 * Input: 
 *   a: item a
 *   b: item b
 * Output: 
 * Return: 0, 1
 * Note: for bug 20011 modified by liuyang 2012-8-21
 ******************************************************************************/
int dhcsnooping_bindinghash_cmp_ip (void *a, void *b)
{
    tbl_dhcbinding_t *pa = (tbl_dhcbinding_t *)a;
    tbl_dhcbinding_t *pfind = (tbl_dhcbinding_t *)b;
    
    if (pa->key.ipv6 != pfind->key.ipv6) {
        return 0;
    }

    if (!memcmp(&pa->client_ip, &pfind->client_ip, sizeof(addr_t))) {
        return dhcsnooping_bindinghash_intf_cmp(pa, pfind);
    }

    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_hash_finish 
 * Purpose: free hash items
 * Input: 
 *   pHash: interface hash
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcsnooping_binding_hash_finish (ctclib_hash_t *pHash)
{
    ctclib_hash_clean(pHash, (HASH_CLEAN_FUNC)dhcsnooping_binding_hash_del);
    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_hash_add
 * Purpose: add interface to hash
 * Input: 
 *   pHash: binding hash
 *   pHash: binding hash
 *   pItem: binding item
 * Output: 
 * Return: N/A
 * Note:  modified by George Wei add new hashtable
 ******************************************************************************/
void dhcsnooping_binding_hash_add(ctclib_hash_t *pHash,  ctclib_hash_t *pHashIp, tbl_dhcbinding_t *pItem)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    sal_memcpy(pItem->client_mac, pItem->key.mac, sizeof(mac_addr_t));
    if (pHashIp) 
    {
        ctclib_hash_get(pHashIp, pItem, ctclib_hash_alloc_intern);
    }

    if (pHash) 
    {
        tbl_dhcbinding_add_dhcbinding(pItem);
        
        /*for bug 18273 merged by liuyang 2012-2-23 */
        if (!pItem->key.ipv6) 
        {
            DHCSNOOPING_CUR_BINDINGS++;
        }
#ifdef HAVE_IPV6    
        else
        {
            DHCSNOOPING6_CUR_BINDINGS++;
        }
#endif
        if (DHCBINDING_STATE_REQUEST == pItem->state) 
        {
            u_int8_t *pmac = (u_int8_t*)pItem->key.mac;
            if (!pItem->key.ipv6) 
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "prepare binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s trans id 0x%x",
                        pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                        pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext), sal_ntoh32(pItem->transid));
            }
#ifdef HAVE_IPV6
            else 
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                        "prepare v6 binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s trans id 0x%x",
                        pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                        pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext), sal_ntoh32(pItem->transid));
            }
#endif
        }

    }

    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_hash_del
 * Purpose: delete interface from hash
 * Input: 
 *   pItem: binding item
 * Output: 
 * Return: N/A
 * Note: modified by George Wei add new hashtable
 ******************************************************************************/
void dhcsnooping_binding_hash_del(tbl_dhcbinding_t *pItem)
{
    u_char szIpBuf[128];
    sal_time_t nCurtime = ctc_time_boottime_sec(NULL);
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    /* add new hash with key ip, George wei, 2007-10-30 */
    if (NULL == pItem) 
    {
        return;
    }
        
    if (DHCBINDING_STATE_RUNNING == pItem->state) 
    {
        u_int8_t *pmac = (u_int8_t*)pItem->key.mac;
        if (!pItem->key.ipv6) 
        {
            /*for bug 21493 modified by liuyang 2012-12-25*/
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                    "remove binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s "
                    "%s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), pItem->lease ? abs(pItem->lease - nCurtime) : 0);
        }
#ifdef HAVE_IPV6
        else 
        {
            /*for bug 21493 modified by liuyang 2012-12-25*/
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                    "remove v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x %s "
                    "%s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), pItem->lease ? abs(pItem->lease - nCurtime) : 0);
        }
#endif
    }
    
    /* add by George wei */
    ctclib_hash_release(DHCSNOOPING_HASH_IP, pItem);

     /*for bug 18273 merged by liuyang 2012-2-23 */
    if (!pItem->key.ipv6) 
    {
        if (DHCSNOOPING_CUR_BINDINGS > 0) 
        {
            DHCSNOOPING_CUR_BINDINGS--;
        }
    }
#ifdef HAVE_IPV6    
    else 
    { 
        if (DHCSNOOPING6_CUR_BINDINGS > 0) 
        {
            DHCSNOOPING6_CUR_BINDINGS--;
        }
    }
#endif  
    tbl_dhcbinding_del_dhcbinding(&pItem->key);
    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_iter
 * Purpose: del binding hash item by callback
 * Input: 
 *   pHash: hash header
 *   pFunc: callback function
 *   pArg:  callback argument
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_binding_iter(DHCSNOOPING_HASH_CB_FUNC pFunc, void *pArg)
{
    tbl_dhcbinding_t *pItem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;

    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) {
        pFunc(pItem, pArg);
    }

    return 0;
}

int
dhcsnooping_binding_filter_by_ip(tbl_dhcbinding_t *pItem, void *pArg)
{
    addr_t      *pstSU;
    S_CmpArg    *pCmpArg = (S_CmpArg *)pArg;
    
    if (NULL == pArg || NULL == pItem) {
        return -1;
    }

    if (pItem->key.ipv6 != pCmpArg->bIPv6) {
        return -1;
    }
    
    /*for bug 21259 added by liuyang 2012-12-27*/
    if (pItem->type != pCmpArg->cType && DHCBINDING_TYPE_ALL != pCmpArg->cType) {
        return -1;
    }

    pstSU = (addr_t *)pCmpArg->pArg;
    if (!memcmp(&pItem->client_ip, pstSU, sizeof(addr_t))) {
        if (pCmpArg->cb) {
            pCmpArg->cb(pItem, pCmpArg->pCbArg);
        }
        return 0;
    }
    
    return -1;
}

int
dhcsnooping_binding_filter_by_if(tbl_dhcbinding_t *pItem, void *pArg)
{
    /*for bug 21259 added by liuyang 2012-12-27*/
    S_CmpArg *pCmpArg = (S_CmpArg *)pArg;

    if (pCmpArg->bIPv6 != pItem->key.ipv6) {
        return -1;
    }

    if (pItem->type != pCmpArg->cType && DHCBINDING_TYPE_ALL != pCmpArg->cType) {
        return -1;
    }
    
    if (!strcasecmp(pItem->ifname, (char *)pCmpArg->pArg)) {
        if (pCmpArg->cb) {
            pCmpArg->cb(pItem, pCmpArg->pCbArg);
        }
        return 0;
    }
    
    return -1;
}

int
dhcsnooping_binding_filter_by_all(tbl_dhcbinding_t *pItem, void *pArg)
{
    /*for bug 21259 added by liuyang 2012-12-27*/
    S_CmpArg *pCmpArg = (S_CmpArg *)pArg;

    if (pCmpArg->bIPv6 != pItem->key.ipv6) {
        return -1;
    }

    if (pItem->type != pCmpArg->cType && DHCBINDING_TYPE_ALL != pCmpArg->cType) {
        return -1;
    }

    if (pCmpArg->cb) {
        pCmpArg->cb(pItem, pCmpArg->pCbArg);
    }
    return 0;
}

int
dhcsnooping_binding_filter_by_vlan(tbl_dhcbinding_t *pItem, void *pArg)
{
    int nVlan = 0;
    S_CmpArg *pCmpArg = (S_CmpArg *)pArg;
    
    if (NULL == pArg || NULL == pItem) {
        return -1;
    }

    if (pItem->key.ipv6 != pCmpArg->bIPv6) {
        return -1;
    }
    /*for bug 21259 added by liuyang 2012-12-27*/
    if (pItem->type != pCmpArg->cType && DHCBINDING_TYPE_ALL != pCmpArg->cType) {
        return -1;
    }
    nVlan = *((u_int16_t *)pCmpArg->pArg);
    if (nVlan == pItem->vid) {
        if (pCmpArg->cb) {
            pCmpArg->cb(pItem, pCmpArg->pCbArg);
        }
        return 0;
    }
    
    return -1;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_hash_cmp_mac
 * Purpose: compare binding item by mac
 * Input: 
 *   pItem: binding item
 *   pArg:  mac address array
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_binding_filter_by_mac(tbl_dhcbinding_t *pItem, void *pArg)
{
    S_CmpArg *pCmpArg = (S_CmpArg *)pArg;
    if (NULL == pArg || NULL == pItem) {
        return -1;
    }

    if (pItem->key.ipv6 != pCmpArg->bIPv6) {
        return -1;
    }

    /*for bug 21259 added by liuyang 2012-12-27*/
    if (pItem->type != pCmpArg->cType && DHCBINDING_TYPE_ALL != pCmpArg->cType) {
        return -1;
    }
    
    if (!memcmp(&pItem->key.mac, pCmpArg->pArg, sizeof(mac_addr_t))) {
        if (pCmpArg->cb) {
            pCmpArg->cb(pItem, pCmpArg->pCbArg);
        }
        return 0;
    }
    
    return -1;
}

void dhcsnooping_binding_item_del(tbl_dhcbinding_t *pItem, void *pCbArg)
{
    dhcsnooping_binding_hash_del(pItem);
    return;
}

void dhcsnooping_binding_item_show(tbl_dhcbinding_t *pItem, void *pCbArg)
{
    FILE *fp = (FILE *)pCbArg;
    tbl_iter_args_t args;
    field_parse_t stField;

    if (DHCBINDING_STATE_REQUEST == pItem->state) {
        return;
    }

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&stField, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = &stField;
    args.argv[1] = fp;
    
    tbl_dhcbinding_dump_one(pItem, &args);
    return;
}

int dhcsnooping_binding_execute(int bIPv6, int cmpType, 
    int type, void *pArg, int delOrShow, void *pCbArg) 
{
    S_CmpArg stCmpArg;   
    DHCSNOOPING_HASH_CB_FUNC pcb = NULL;

    stCmpArg.bIPv6 = bIPv6;
    stCmpArg.cmpType = cmpType;
    stCmpArg.cType = type;
    stCmpArg.pArg = pArg;
    stCmpArg.pCbArg = pCbArg;
    if (delOrShow) {
        stCmpArg.cb = (ITEM_CB_FUNC)dhcsnooping_binding_item_del;
    } else {
        stCmpArg.cb = (ITEM_CB_FUNC)dhcsnooping_binding_item_show;
    }
    
    switch(cmpType) {
    case DHCBINDING_FILTER_INTERFACE:
        pcb = (DHCSNOOPING_HASH_CB_FUNC)dhcsnooping_binding_filter_by_if;
        break;
    case DHCBINDING_FILTER_VLAN:
        pcb = (DHCSNOOPING_HASH_CB_FUNC)dhcsnooping_binding_filter_by_vlan;
        break;
    case DHCBINDING_FILTER_IP:
        pcb = (DHCSNOOPING_HASH_CB_FUNC)dhcsnooping_binding_filter_by_ip;
        break;
    case DHCBINDING_FILTER_MAC:
        pcb = (DHCSNOOPING_HASH_CB_FUNC)dhcsnooping_binding_filter_by_mac;
        break;
    case DHCBINDING_FILTER_ALL:
    default:
        pcb = (DHCSNOOPING_HASH_CB_FUNC)dhcsnooping_binding_filter_by_all;
        break;
    }

    dhcsnooping_binding_iter(pcb, &stCmpArg);
    return 0;
}

int
dhcsnooping_binding_exist_check(tbl_dhcbinding_t *pItemNew, 
    char *pszErrMsg, size_t nErrBufsz)
{
    tbl_dhcbinding_t *pItem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;

    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) {
        if (pItem->key.ipv6 != pItemNew->key.ipv6) {
            continue;
        }
        
        if (!memcmp(&pItem->key.mac, pItemNew->key.mac, sizeof(mac_addr_t))
                && pItem->vid == pItemNew->vid) {
            snprintf(pszErrMsg, nErrBufsz, 
                "Binding item with the same MAC address and Vlan exist");
            return -1;
        }

        if (!memcmp(&pItem->client_ip, &pItemNew->client_ip, sizeof(addr_t))
                && pItem->vid == pItemNew->vid) {
            snprintf(pszErrMsg, nErrBufsz, 
                "Binding item with the same IP%s address and Vlan exist",
                pItemNew->key.ipv6 ? "v6" : "");
            return -1;
        }
    }
    
    return 0;
}


/*******************************************************************************
 * Name:    dhcsnooping_binding_hash_init
 * Purpose: initialize binding hash
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: add new hash with key ip, George wei, 2007-10-30
 ******************************************************************************/
int dhcsnooping_binding_hash_init(void)
{
    DHCSNOOPING_HASH->hash_key = dhcsnooping_bindinghash_make;
    DHCSNOOPING_HASH->hash_cmp = dhcsnooping_bindinghash_cmp;
    
    /* add a new hash and the key is ip, George wei, 2007-10-30 */    
    DHCSNOOPING_HASH_IP->hash_key = dhcsnooping_bindinghash_make_ip;
    DHCSNOOPING_HASH_IP->hash_cmp = dhcsnooping_bindinghash_cmp_ip;
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_queue_update
 * Purpose: update binding item from queue when lease change
 * Input: 
 *   pQueueHdr: queue header
 *   pItem: binding item
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_binding_queue_update(tbl_dhcbinding_t *pItem)
{
    ctclib_slistnode_delete(DHCSNOOPING_QUEUE, pItem);
    ctclib_slistnode_insert_sort(DHCSNOOPING_QUEUE, pItem);
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_queue_count
 * Purpose: getting running state binding item count
 * Input: 
 *   pQueueHdr: Queue header
 *   bIPv6: count for IPv6
 * Output: 
 * Return: 
 *   success: count
 * Note: 
 ******************************************************************************/
 /*for bug 21259 modified by liuyang 2012-12-27*/
int
dhcsnooping_binding_queue_count(int bIPv6, u_int32_t *learn, u_int32_t *config)
{
    ctclib_slistnode_t *listnode = NULL; 
    tbl_dhcbinding_t *pItem = NULL;
    int nRet = 0;

    if (NULL == DHCSNOOPING_QUEUE) {
        return 0;
    }

    CTCLIB_SLIST_LOOP(DHCSNOOPING_QUEUE, pItem, listnode) {
        if (pItem->key.ipv6 != bIPv6) {
            continue;
        }
        if (DHCBINDING_STATE_RUNNING != pItem->state) {
            /* unready item */
            continue;
        }
        if (config && (DHCBINDING_TYPE_CONFIGED == pItem->type)) {
            (*config)++;
        } else if (learn && (DHCBINDING_TYPE_LEARNING == pItem->type)) {
            (*learn)++;
        }        
        
        nRet++;
    }
    
    return nRet;
}

/*******************************************************************************
 * Name:    dhcsnooping_init
 * Purpose: initialize dhcp snooping module
 * Input: 
 *   zg: lib global pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
dhcsnooping_func_init()
{
    memset(&g_stStat, 0x00, sizeof(S_DhcpSnoopingStat));
    dhcsnooping_binding_hash_init();
    g_p_binding_timeout = ctc_task_add_timer(TASK_PRI_NORMAL,
            dhcsnooping_binding_timeout, NULL, DHCSNOOPING_TIMEOUT_INTERVAL);
    g_p_binding_saver = ctc_task_add_timer(TASK_PRI_NORMAL,
            dhcsnooping_save_timer, NULL, DHCSNOOPING_SAVE_BINDING_INTERVAL_DEFAULT);
    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_start 
 * Purpose: starting dhcp snooping module
 * Input: 
 *   pzg: lib globals
 * Output: 
 * Return: 
 *   success: 0
 *   failed : others
 * Note: 
 ******************************************************************************/
int
dhcsnooping_start()
{
    /* Initialize dhcsnooping module */
    dhcsnooping_func_init ();

    /* Set up the initial dhcp option universe. */
    initialize_common_option_spaces ();
    log_sys(M_MOD_INTERNAL, E_INFORMATIONAL, "DHCP snooping ready to service.");
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_stop 
 * Purpose: stop dhcp snooping module
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
dhcsnooping_stop (void)
{
    dhcsnooping_binding_hash_finish(DHCSNOOPING_HASH);
    ctclib_hash_clean(DHCSNOOPING_HASH_IP, NULL);

    sal_memset(&g_stStat, 0x00, sizeof(S_DhcpSnoopingStat));
    log_sys(M_MOD_INTERNAL, E_INFORMATIONAL, "DHCP snooping shutdown...");
    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_binding_timeout
 * Purpose: process timeout value for binding
 * Input: 
 *   t_timout: thread pointer
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
void
dhcsnooping_binding_timeout(void *pArg)
{
    ctclib_slistnode_t  *listnode = NULL;    
    tbl_dhcbinding_t    *p = NULL;
    tbl_dhcbinding_t    *pDel = NULL;
    sal_time_t  nCurtime = ctc_time_boottime_sec(NULL);
    u_char      szIpBuf[128];
    u_int8_t    *pMac;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    g_p_binding_timeout = NULL;
    
    /* binding item is ordered by lease time */
    CTCLIB_SLIST_LOOP(DHCSNOOPING_QUEUE, p, listnode) 
    {
        if (NULL != pDel) 
        {
            /* del binding item */
            if (DHCBINDING_STATE_RUNNING == pDel->state) 
            {
                pMac = pDel->key.mac;
                if (!pDel->key.ipv6) 
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                            "binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d timeouted", pDel->vid,
                            pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5],
                            cdb_addr_val2str((char*)szIpBuf, 128, &pDel->client_ip),
                            IFNAME_ETH2FULL(pDel->ifname, ifname_ext), abs(pDel->lease - nCurtime));
                }
#ifdef HAVE_IPV6
                else 
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                            "v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d timeouted", pDel->vid,
                            pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5],
                            cdb_addr_val2str((char*)szIpBuf, 128, &pDel->client_ip),
                            IFNAME_ETH2FULL(pDel->ifname, ifname_ext), abs(pDel->lease - nCurtime));
                }
#endif
            }

            dhcsnooping_binding_hash_del(pDel);
            pDel = NULL;
        }

        /* lease time zero indicate static entry */
        if (0 == p->lease) {
            continue;
        }

        if (p->lease_new != 0 && p->lease_new <= nCurtime + 1)
        {
            sal_memset(p->ifname_new, 0, sizeof(p->ifname_new));
            p->lease_new = 0;
        }
        
        if (p->lease > nCurtime + 1) {
            break;
        }

        /* indicate to del this binding item */
        pDel = p;
    }
       
    if (NULL != pDel) {
        if (DHCBINDING_STATE_RUNNING == pDel->state) {
            pMac = pDel->key.mac;
            if (!pDel->key.ipv6) {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                        "%s %s lease %d timeouted", pDel->vid,
                        pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5],
                        cdb_addr_val2str((char*)szIpBuf, 128, &pDel->client_ip),
                        IFNAME_ETH2FULL(pDel->ifname, ifname_ext), abs(pDel->lease - nCurtime));
            }
#ifdef HAVE_IPV6
            else {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                        "v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                        "%s %s lease %d timeouted", pDel->vid,
                        pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5],
                        cdb_addr_val2str((char*)szIpBuf, 128, &pDel->client_ip),
                        IFNAME_ETH2FULL(pDel->ifname, ifname_ext), abs(pDel->lease - nCurtime));
            }
#endif
        }

        /* del binding item */    
        dhcsnooping_binding_hash_del(pDel);
        pDel = NULL;
    }

    g_p_binding_timeout = ctc_task_add_timer(TASK_PRI_NORMAL,
            dhcsnooping_binding_timeout, NULL, DHCSNOOPING_TIMEOUT_INTERVAL);
    return;
}

int32 
__mlag_port_isolation_check(uint32 ifindex_in, uint32 ifindex_out)
{
	tbl_mlag_t				*p_mlag = NULL;
	tbl_mlag_peer_t 		*p_mlag_peer = NULL;
	tbl_mlag_port_t 		*p_mlag_port = NULL;
	tbl_mlag_port_master_t	*p_mlag_port_master = NULL;
	ctclib_slistnode_t      *listnode   = NULL; 

	/* 1. get mlag info */
	p_mlag = tbl_mlag_get_mlag();
	p_mlag_peer = tbl_mlag_peer_get_mlag_peer();
	p_mlag_port_master = tbl_mlag_port_get_master();
	if(NULL == p_mlag || NULL == p_mlag_peer || NULL == p_mlag_port_master)
	{
		goto normal_out;
	}

	/* 2. check mlag state */
	if(5 != p_mlag_peer->mpf_state)
	{
		goto normal_out;
	}

	/* 3. check mlag state */
	if(ifindex_in != p_mlag->peer_link_ifindex)
	{
		goto normal_out;
	}

	/* 4. foreach check mlagid is up and running */
	CTCLIB_SLIST_LOOP(p_mlag_port_master->port_list, p_mlag_port, listnode)
	{
		if(p_mlag_port->ifindex == ifindex_out)
		{
			if(p_mlag_port->peer_if_up && p_mlag_port->peer_conf)
			{
				return -1;			
			}
		}
	}
	
normal_out:
	return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_send_pkt
 * Purpose: send this packet after dhcsnooping
 * Input: 
 *   pHdr:    packet header
 *   nBufLen: packet size
 *   pcmsg:   address info
 *   nAddrLen: address info length
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int 
dhcsnooping_send_pkt(S_DhcpSnoopingInHdr *pHdr, int nBufLen, 
                      DhcSnoopingCMsg_t *pcmsg, int nAddrLen)
{
    tbl_vlan_t          *p_db_vlan  = NULL;
    tbl_interface_t     *p_mem_if   = NULL;
    tbl_interface_t     *p_db_if    = NULL;
    tbl_interface_t     *pst_if_lag = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_key_t      vlan_key;
    uint32              ifindex     = 0;
    unsigned char       *ptr        = ((unsigned char *)pHdr->uiph);
    int flags = 0;
    int rc;    
    /* modified by liwh for bug 40536, 2016-09-07 */
    uint8               buf[1600];
    uint32              pkt_len = 0; 
    /* liwh end */

    /* modified by liwh for bug 40536, 2016-09-07 */
    sal_memset(buf, 0, 1600);
    /* liwh end */
    
    if (!pHdr->bIPv6) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                  "send one DHCP message");
        DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP,
                 "dump the message (send)", (u_char *)&pHdr->uiph, nBufLen, 0);        
    }
#ifdef HAVE_IPV6
    else {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                  "send one DHCPv6 message");
        DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP,
                 "dump the message (send)", (u_char *)&pHdr->uiph, nBufLen, 0);
    }
#endif

    if (is_dhcrelay_service_enabled()) {
        flags |= GLB_PKT_FLAGS_TX_COPY_TO_KERNEL;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(pcmsg->ifindex);
    if (NULL == p_db_if)
    {
        return 0;
    }
    
    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    vlan_key.vid = pcmsg->vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL  == p_db_vlan)
    {
        return 0;
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        if (ifindex == pcmsg->ifindex)
        {
            continue;
        }
        
        p_mem_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_mem_if)
        {
            continue;   
        }
        else if (!(p_db_if->dhcp_snooping_trust) 
                && !p_mem_if->dhcp_snooping_trust && DHCPREQUEST == pcmsg->type)
        {
            continue;
        }

        if (!tbl_interface_is_running(p_mem_if))
        {
            continue;
        }

        if (GLB_IF_MODE_L2 == p_mem_if->mode && INTERFACE_IS_LAG_MEMBER(p_mem_if))
        {
            sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_mem_if->lag->lag_id);
            pst_if_lag = tbl_interface_get_interface(&if_key);
            if (NULL == pst_if_lag)
            {
                return 0;
            }

            if (pst_if_lag->ifindex == pcmsg->ifindex)
            {
                continue;
            }

            if (!tbl_interface_is_running(pst_if_lag))
            {
                continue;
            }

            ifindex =  pst_if_lag->ifindex;
        }

        /* modified by liwh for bug 40536, 2016-09-07 
           now, there is no cli to configure tpid, therefore use GLB_DEFAULT_TPID */
        if ((GLB_IF_MODE_L2 == p_mem_if->mode)
            && (GLB_VLAN_PORT_TYPE_TRUNK == p_mem_if->vlan_type))
        {
            sal_memcpy(buf, (ptr - ETH_HEADER_LEN), (ETH_HEADER_LEN - 2));
            buf[ETH_HEADER_LEN - 2] = (GLB_DEFAULT_TPID >> 8) & 0xff;
            buf[ETH_HEADER_LEN - 1] = GLB_DEFAULT_TPID & 0xff;
            buf[ETH_HEADER_LEN] = (pcmsg->vlanid >> 8) & 0xff;
            buf[ETH_HEADER_LEN + 1] = pcmsg->vlanid & 0xff;
            sal_memcpy(&buf[ETH_HEADER_LEN + 2], (ptr - 2), nBufLen + 2);
            pkt_len = nBufLen + ETH_HEADER_LEN + 4;
        }
        else
        {
            sal_memcpy(buf, ptr - ETH_HEADER_LEN, nBufLen + ETH_HEADER_LEN);
            pkt_len = nBufLen + ETH_HEADER_LEN;
        }
        /* liwh end */

		/* modified by xuwj for bug 45390, 2018-05-03 */
		if(__mlag_port_isolation_check(pcmsg->ifindex, ifindex) != 0)
		{
			continue;
		}
		/* xuwj end */

        /* modified by liwh for bug 40536, 2016-09-07 */
        //rc = ipc_pkt_tx_dhcp(ptr - ETH_HEADER_LEN, nBufLen + ETH_HEADER_LEN,
        rc = ipc_pkt_tx_dhcp((unsigned char*)buf, pkt_len,
        /* liwh end */
                ifindex, pcmsg->vlanid, flags);

        if (PM_E_NONE != rc) {
            if (!pHdr->bIPv6) {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "send packet to %s failed: %s", 
                        inet_ntoa(pHdr->uiph->ip_dst), 
                        strerror(errno));
            }
#ifdef HAVE_IPV6
            else {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                        "send packet to %s failed: %s", 
                        inet_ntoa(pHdr->uiph->ip_dst), 
                        strerror(errno));
            }
#endif
        }        
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_is_server_side_msg
 * Purpose: check message type
 * Input: 
 *   type: dhcp message type
 * Output: 
 * Return: 
 *   success: 1
 *   failed : 0
 * Note: 
 ******************************************************************************/
inline int
dhcsnooping_is_server_side_msg(u_int type)
{
    switch (type) {
    case DHCPOFFER:
    case DHCPACK:
    case DHCPNAK:
    /* for bug 27654 modified by liuyang 2014-4-8
    case DHCPINFORM: */
        return 1;
    default:
        break;
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_release_check
 * Purpose: is interface matched the snooping item ?
 * Input: 
 *   pHdr:  packet header
 *   pcmsg: address info
 *   ifp: interface info
 * Output: 
 * Return: 
 *   match:    1
 *   mismatch: 0
 * Note: 
 ******************************************************************************/
int
dhcsnooping_release_check(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg,
    char *pszIntfName, struct packet *pDhcpPkt)
{
    tbl_dhcbinding_t *pItem = NULL;
    tbl_dhcbinding_t stItem;

    sal_memset(&stItem, 0x00, sizeof(tbl_dhcbinding_t));
    if (0 != dhcsnooping_get_clientmac(pHdr, pDhcpPkt, pcmsg, (char*)stItem.key.mac)) {
        return 0;
    }
    if (0 != dhcsnooping_get_assignedip(pHdr, pDhcpPkt, &stItem.client_ip)) {
        return 0;
    }
    stItem.vid = pcmsg->vlanid;
    stItem.key.vid = pcmsg->vlanid;
    stItem.key.ipv6 = pHdr->bIPv6;

#ifdef HAVE_IPV6
    if (!stItem.key.ipv6)
#endif
        /*modified by weij for 14835, 2011-04-29*/
        stItem.client_ip.u.prefix4.s_addr = pHdr->dhcp->ciaddr.s_addr;

    if ((pItem = tbl_dhcbinding_get_dhcbinding(&stItem.key)) == NULL) {
        return 1;
    }

    /* is it interface match and ciaddr match */
    if (!strcasecmp(pItem->ifname, pszIntfName)
        && !sal_memcmp(&pItem->client_ip, &stItem.client_ip, sizeof(addr_t))) {
        return 1;
    }

    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_get_lease_time
 * Purpose: get lease time from dhcp packet
 * Input: 
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: lease time
 *   failed : default lease time
 * Note: 
 ******************************************************************************/
long
dhcsnooping_get_lease_time(S_DhcpSnoopingInHdr *pHdr, struct packet *pDhcpPkt)
{
    struct option_cache *op = NULL;
    time_t absTime = time(NULL);
    time_t nCurTime;
    time_t nTime;
    u_int32_t time_get = 0;
    struct sal_timeval tv;

    ctc_task_update_boottime(&tv);
    nCurTime = ctc_time_boottime_sec(NULL);
    nTime = DHCSNOOPING_DEFAULT_LEASE_TIME + nCurTime;

#ifdef HAVE_IPV6
    if (pHdr->bIPv6) {
        if (NULL != pHdr->bindings) {
            struct dhc6_addr *pa = pHdr->bindings->addrs;
            if (NULL != pa) {
                time_get =  pa->max_life;
                if ((time_get & ULONG_MAX) == ULONG_MAX) {
                    return 0;
                }
                /* if the time can't expressed by timet_t set it max */
                if ((time_get > LONG_MAX) || ((time_get + absTime) > LONG_MAX)) {
                    return LONG_MAX;
                }
                return time_get + nCurTime;
             }
        }

        return DHCSNOOPING_DEFAULT_LEASE_TIME + nCurTime;
    }
#endif
    
    assert(NULL != pDhcpPkt);
    if (0 == pDhcpPkt->packet_type) {
        /* it's bootp message */
        return DHCSNOOPING_DEFAULT_BOOTP_LEASE_TIME + nCurTime;
    }

    if (NULL == pDhcpPkt->options) {
        return nTime;
    }
    
    /*for bug 3976, use the error option DHO_DHCP_RENEWAL_TIME by weij*/
    op = lookup_option(&dhcp_universe, pDhcpPkt->options, DHO_DHCP_LEASE_TIME);
    if (NULL != op) {
        struct data_string ds;
        /*for bug 7090*/
        memset (&ds, 0, sizeof ds);
        if (evaluate_option_cache(&ds, pDhcpPkt, NULL, NULL,
                    pDhcpPkt->options, NULL, NULL, op, MDL)) {
            if (ds.len > 3) {
                /*for bug 4029, by weij , the time in linux is long but in the packet is unsignd long 
                 * we have to deal the time which is bigger than long
                 */
                time_get = getULong (ds.data);
                /*infinity*/
                if ((time_get & ULONG_MAX) == ULONG_MAX) {
                    return 0;
                }
                /* if the time can't expressed by timet_t set it max */
                if ((time_get > LONG_MAX) || ((time_get + absTime) > LONG_MAX)) {
                    return LONG_MAX;
                }
                nTime = time_get;
            } else {
                nTime = DHCSNOOPING_DEFAULT_LEASE_TIME;
            }
            data_string_forget (&ds, MDL);
        }
        nTime += nCurTime;
    }

    return nTime;
}

int
dhcsnooping_need_del_old_binding(S_DhcpSnoopingInHdr *pHdr,
        char *pszIntfName, struct packet *pDhcpPkt, tbl_dhcbinding_t *pItem)
{
    if (NULL == pHdr || NULL == pszIntfName || NULL == pDhcpPkt || NULL == pItem) 
    {
        return 0;
    }
#if 0
    if (DHCBINDING_STATE_REQUEST == pItem->state) {
        return 0;
    }

    if (strcmp(pszIntfName, pItem->ifname)) {
        return 1;
    }
#endif
    if (!pHdr->bIPv6) 
    {
        if (pDhcpPkt->raw 
            && pItem->client_ip.u.prefix4.s_addr == pDhcpPkt->raw->ciaddr.s_addr) 
        {
            return 0;
        }
    } 
    else 
    {
        if (DHCPV6_RENEW == pDhcpPkt->dhcpv6_msg_type
            || DHCPV6_REBIND == pDhcpPkt->dhcpv6_msg_type) 
        {
            addr_t su;
            if (0 == dhcsnooping_get_assignedip(pHdr, pDhcpPkt, &su)) 
            {
                if (!memcmp(&su, &pItem->client_ip, sizeof(addr_t))) 
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

/*for bug 20011 added by liuyang 2012-8-21*/
uint32 
dhcpsnooping_bindinghash_intf_cmp(tbl_dhcbinding_t *pItem, tbl_dhcbinding_t *pfind)
{
    char                 ifname[IFNAME_SIZE+1]; /* interface name */

    sal_memset(ifname, 0, sizeof(ifname));
    /* processing request pkt*/
    if (0 != sal_strcmp(pfind->ifname, ifname))
    {
        if (pfind->state == DHCBINDING_STATE_RUNNING)
        {
            if (!sal_strcasecmp((char*)pItem->ifname, (char *)pfind->ifname))
            {
                return 1;
            }                    
        }
        else if (pfind->state == DHCBINDING_STATE_REQUEST)
        {
            return 1;                    
        }
    }
    /* processing ack or reply pkt*/
    else 
    {
        return 1;
    }
    
    return 0;
}

/*get the equal ip & vlan dhcp snooping entry*/
tbl_dhcbinding_t *
dhcsnooping_binding_exist_ip_check_get(tbl_dhcbinding_t *pItemNew)
{
    tbl_dhcbinding_t *pItem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;
    uint8 szIpBuf[128];

    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) 
    {
        if (pItem->key.ipv6 != pItemNew->key.ipv6) 
        {
            continue;
        }
        
        /*ip & vlan is equal , mac not equal*/
        if ((!memcmp(&pItem->client_ip, &pItemNew->client_ip, sizeof(addr_t))) && 
            (pItem->vid == pItemNew->vid) && memcmp(&pItem->key.mac, pItemNew->key.mac, sizeof(mac_addr_t)))
        {
            sal_memset(szIpBuf, 0, sizeof (szIpBuf));
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "Binding item with the same IP %s and Vlan %d exist", cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip), pItemNew->vid);
            return pItem;
        }
    }
    
    return NULL;
}
            
/*******************************************************************************
 * Name:    dhcsnooping_prepare_bindingitem
 * Purpose: prepare binding item for DHCPREQUEST or BOOTREQUEST
 * Input: 
 *   pHdr:  packet info
 *   pcmsg: address info
 *   ifp: interface info
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: for bug 20011 modified by liuyang 2012-8-22
 ******************************************************************************/
int
dhcsnooping_prepare_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg,
    char *pszIntfName, struct packet *pDhcpPkt)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_dhcbinding_t *pItem = NULL;
    tbl_dhcbinding_t stItem;
    uint8 *pmac = NULL;
    uint32 rc = 0;
    
    sal_memset(&stItem, 0x00, sizeof(tbl_dhcbinding_t));

    if (0 != dhcsnooping_get_clientmac(pHdr, pDhcpPkt, pcmsg, (char*)stItem.key.mac)) 
    {
        return -1;
    }
        /*for bug 21582 added by liuyang 2012-12-25*/
        /*fix bug 43808:change by chenc*/
    if (dhcsnooping_get_assignedip(pHdr, pDhcpPkt, &stItem.client_ip) && pHdr->bIPv6) 
    {
        return -1;
    }
    stItem.vid = pcmsg->vlanid;
    stItem.key.vid = pcmsg->vlanid;
    stItem.key.ipv6 = pHdr->bIPv6;
    snprintf((char*)stItem.ifname, IFNAME_SIZE + 1, pszIntfName);

    pItem = tbl_dhcbinding_get_dhcbinding(&stItem.key);

    if (NULL == pItem) 
    { /* prepare new item */
#ifdef HAVE_IPV6
        if ((!pHdr->bIPv6 && DHCSNOOPING_CUR_BINDINGS >= DHCSNOOPING_MAX_BINDING_ITEMS)
            || (pHdr->bIPv6 && DHCSNOOPING6_CUR_BINDINGS >= DHCSNOOPINGV6_MAX_BINDING_ITEMS))
#else
        if ((!pHdr->bIPv6 && DHCSNOOPING_CUR_BINDINGS >= DHCSNOOPING_MAX_BINDING_ITEMS))
#endif
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "WARNING: too many DHCP snooping binding item");
            return -1;
        }

        pItem = &stItem;
        sal_snprintf(pItem->ifname, IFNAME_SIZE + 1, pszIntfName);
        pItem->type = DHCBINDING_TYPE_LEARNING;
        pItem->state = DHCBINDING_STATE_REQUEST; /* just for DHCPREQUEST */
        pItem->lease = DHCSNOOPING_DEFAULT_REQUEST_TIMEOUT + ctc_time_boottime_sec(NULL);
        pItem->vid = pcmsg->vlanid;
		pItem->key.vid = pcmsg->vlanid;
        pItem->key.ipv6 = pHdr->bIPv6;
        dhcsnooping_set_transid(pHdr, pDhcpPkt, pItem);
        /* add new hash with key ip, George wei, 2007-10-30 
            beacuse the ip of the item is not exist, we can't add item to DHCSNOOPING_HASH_IP 2007-12-4
        */
        dhcsnooping_binding_hash_add(DHCSNOOPING_HASH, NULL, pItem);        
        return 0;
    }

    rc = dhcpsnooping_bindinghash_intf_cmp(&stItem, pItem);

    /*check ipsg has this item, maybe ipsg tbl has been delete by no ipsg static*/
    dhcsnooping_add_tbl_to_ipsg(pItem);
    
    /* we does not over write configed items */
    if (DHCBINDING_TYPE_CONFIGED == pItem->type) 
    {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                "Can not overwrite for the configed binding item existing"); 
        return 0;
    }

    /* update request item */
    if (DHCBINDING_STATE_RUNNING != pItem->state) 
    {
        stItem.lease = DHCSNOOPING_DEFAULT_REQUEST_TIMEOUT + ctc_time_boottime_sec(NULL);
        snprintf(stItem.ifname, IFNAME_SIZE + 1, pszIntfName);
        dhcsnooping_set_transid(pHdr, pDhcpPkt, &stItem);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_LEASE);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_IFNAME);
        dhcsnooping_binding_queue_update(pItem);
        return 0;
    }

    /* update running item */
    dhcsnooping_set_transid(pHdr, pDhcpPkt, &stItem);
    tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_TRANSID);
    if (dhcsnooping_need_del_old_binding(pHdr, pszIntfName, pDhcpPkt, pItem)) 
    {
        pmac = pItem->key.mac;
        if (!pItem->key.ipv6) 
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                    "delete old binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s",
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext));
        }
        else 
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                    "delete old v6 binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s",
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext)); 
        }
        /*for bug 19489 del entry from hash_ip modified by liuyang 2012-6-4*/
        /*for bug 27662 modified by liuyang 2014-4-2*/     
        if (rc)
        {
            ctclib_hash_release(DHCSNOOPING_HASH_IP, pItem);
            
            sal_memset(&stItem.client_ip, 0x00, sizeof(addr_t));
            stItem.state = DHCBINDING_STATE_REQUEST;
            stItem.lease = DHCSNOOPING_DEFAULT_REQUEST_TIMEOUT + ctc_time_boottime_sec(NULL);
            tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_LEASE);
            tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_STATE);
            tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_CLIENT_IP);

            /*for bug 6454, the requst ifp may not the same 
            **save item with different ifname in hash, but only one in hash_ip
            **pal_snprintf(pItem->ifname, INTERFACE_NAMSIZ + 1, ifp->name);
            */
            if (!pItem->key.ipv6) 
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "prepare new binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s trans id 0x%x",
                        pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                        pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext), sal_ntoh32(pItem->transid));                
            }
            else 
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                        "prepare new v6 binding item %02x:%02x:%02x:%02x:%02x:%02x vlan %u %s trans id 0x%x",
                        pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                        pItem->vid, IFNAME_ETH2FULL(pItem->ifname, ifname_ext), sal_ntoh32(pItem->transid));
            }
        }
        /*change the client input port, name is not same*/
        else
        {
            sal_snprintf(pItem->ifname_new, IFNAME_SIZE + 1, pszIntfName);
            pItem->lease_new = DHCSNOOPING_DEFAULT_REQUEST_TIMEOUT + ctc_time_boottime_sec(NULL);
        }
    }

    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_add_bindingitem
 * Purpose: add binding item
 * Input: 
 *   pHdr:  packet info
 *   pcmsg: address info
 *   ifp: interface info
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: for bug 20011 modified by liuyang 2012-8-22 (request one must be after run one.)
 ******************************************************************************/
int
dhcsnooping_add_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    char *pszIntfName, struct packet *pDhcpPkt)
{
    tbl_dhcbinding_t *pItem = NULL;
    tbl_dhcbinding_t *pItemTmp = NULL;
    tbl_dhcbinding_t stItem;
    u_char szIpBuf[128];
    time_t nCurtime = ctc_time_boottime_sec(NULL);
    u_int8_t *pmac;
    char                 ifname[IFNAME_SIZE+1];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    memset(&stItem, 0x00, sizeof(tbl_dhcbinding_t));
    /* locate the DHCPREQUEST state item */
    if (0 != dhcsnooping_get_clientmac(pHdr, pDhcpPkt, pcmsg, (char*)stItem.key.mac)) {
        return -1;
    }
    if (0 != dhcsnooping_get_assignedip(pHdr, pDhcpPkt, &stItem.client_ip)) {
        return -1;
    }

    //repeate request
    stItem.vid = pcmsg->vlanid;
    stItem.key.vid = pcmsg->vlanid;
    stItem.key.ipv6 = pHdr->bIPv6;
    dhcsnooping_set_transid(pHdr, pDhcpPkt, &stItem);

    pItem = tbl_dhcbinding_get_dhcbinding(&stItem.key);     
    if (NULL == pItem || stItem.transid != pItem->transid) {
        if (!stItem.key.ipv6) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                    "can't add binding item for locate request information (MAC + trans id) faild");            
        }
#ifdef HAVE_IPV6
        else {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR,
                    "can't add v6 binding item for locate request information (MAC + trans id) faild");
        }
#endif
        return 0;
    }

    if (DHCBINDING_TYPE_CONFIGED == pItem->type) {
        /* we does not overwrite configed item */
        return 0;
    }

    if (DHCBINDING_STATE_REQUEST == pItem->state) 
    {
        /*fix bug 43832, add ip check when add dynamic dhcp binding, add by chenc*/
        if ((pItemTmp = dhcsnooping_binding_exist_ip_check_get(&stItem))) 
        {
            /*this entry had exist, delete it*/
            dhcsnooping_binding_hash_del(pItemTmp);
        }
        
        /* change to running state */
        stItem.state = DHCBINDING_STATE_RUNNING;
        stItem.lease = dhcsnooping_get_lease_time(pHdr, pDhcpPkt);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_STATE);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_LEASE);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_CLIENT_IP);
        
        dhcsnooping_binding_hash_add(NULL, DHCSNOOPING_HASH_IP, pItem);
        pmac = pItem->key.mac;
        if (!pItem->key.ipv6) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                    "add binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                    "%s %s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), abs(pItem->lease - nCurtime));
        }
#ifdef HAVE_IPV6
        else {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                    "add v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                    "%s %s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), abs(pItem->lease - nCurtime));
        }
#endif
    } 
    else 
    {
        sal_memset(ifname, 0, sizeof(ifname));
        /*change client input port*/
        if (0 != sal_strcmp(pItem->ifname_new, ifname))
        {
            sal_strcpy(pItem->ifname, pItem->ifname_new);
            tbl_dhcbinding_set_dhcbinding_field(pItem, TBL_DHCBINDING_FLD_IFNAME);
            sal_memset(pItem->ifname_new, 0, sizeof(ifname));
            pItem->lease_new = 0;
        }
        
        /* for running state item, just update it lease time , no need to readd ipsg entry for same ip*/
        stItem.lease = dhcsnooping_get_lease_time(pHdr, pDhcpPkt);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_LEASE);
        tbl_dhcbinding_set_dhcbinding_field(&stItem, TBL_DHCBINDING_FLD_CLIENT_IP);
        pmac = pItem->key.mac;
        if (!pItem->key.ipv6) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                    "update binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                    "%s %s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), abs(pItem->lease - nCurtime));
        }
#ifdef HAVE_IPV6
        else {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                    "update v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                    "%s %s lease %d", pItem->vid,
                    pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5],
                    cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
                    IFNAME_ETH2FULL(pItem->ifname, ifname_ext), abs(pItem->lease - nCurtime));
        }
#endif
        } 
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_del_bindingitem
 * Purpose: del binding item
 * Input: 
 *   pHdr: 
 *   pcmsg: 
 *   ifp: 
 *   pDhcpPkt: 
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: for bug 20011 modified by liuyang 2012-8-21
 ******************************************************************************/
int
dhcsnooping_del_bindingitem(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    char *pszIntfName, struct packet *pDhcpPkt)
{
    tbl_dhcbinding_t *pItem = NULL;
    tbl_dhcbinding_t stItem;
    u_int8_t bRun = 0;
    u_int8_t bReq = 0;
    
    memset(&stItem, 0x00, sizeof(tbl_dhcbinding_t));
    if (0 != dhcsnooping_get_clientmac(pHdr, pDhcpPkt, pcmsg, (char*)stItem.key.mac)) {
        return -1;
    }
    stItem.vid = pcmsg->vlanid;
    stItem.key.vid = pcmsg->vlanid;
    /* for bug 5919 
    stItem.state = DHCBINDING_STATE_RUNNING;
    */
    stItem.key.ipv6 = pHdr->bIPv6;
    /* for bug 22633 set transid modified by liuyang 2013-3-29 */
    dhcsnooping_set_transid(pHdr, pDhcpPkt, &stItem);
    
    pItem = tbl_dhcbinding_get_dhcbinding(&stItem.key);
    if (NULL == pItem || DHCBINDING_TYPE_CONFIGED == pItem->type) {
        /* we does not delete configed item */
        return 0;
    }

    if ((!stItem.key.ipv6 && DHCPNAK == pDhcpPkt->packet_type)
        || (stItem.key.ipv6 && DHCPV6_REPLY == pDhcpPkt->dhcpv6_msg_type)) {
        if (stItem.transid == pItem->transid) {
            if (DHCBINDING_STATE_REQUEST == pItem->state) {
                bReq = 1;
            } else {
                bRun = 1;
            }
        }       
    } else if ((!stItem.key.ipv6
                    && (DHCPDECLINE == pDhcpPkt->packet_type 
                    || DHCPRELEASE == pDhcpPkt->packet_type))
                || (stItem.key.ipv6
                    && (DHCPV6_RELEASE == pDhcpPkt->dhcpv6_msg_type
                    || DHCPV6_DECLINE == pDhcpPkt->dhcpv6_msg_type))) {
            /*for bug 21582 modified by liuyang 2012-12-25*/
        if (stItem.key.ipv6  
                && dhcsnooping_get_assignedip(pHdr, pDhcpPkt, &stItem.client_ip)) {
            return -1;
        }
        
        if (!strcasecmp(pItem->ifname, pszIntfName)) {
            if (DHCBINDING_STATE_REQUEST == pItem->state) {
                bReq = 1;
            } else {
                bRun = 1;
            }             
        }          
    }

    if (bRun) {   /* even if ip equal 0 ?*/
        dhcsnooping_binding_hash_del(pItem);
    }

    if (bReq) {
        tbl_dhcbinding_del_dhcbinding(&pItem->key);
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_find_interface_by_agent_option
 * Purpose: use relay agent options to locate interface
 * Input: 
 *   packet: dhcp packet
 *   buf: relay agent buffer
 *   len: buffer length
 * Output: 
 * Return: 
 *   success: 1
 *   failed : 0
 * Note:    for bug 23008 modified option82 by liuyang 2013-6-13
 ******************************************************************************/
int 
dhcsnooping_find_interface_by_agent_option(struct dhcp_packet *packet, DhcSnoopingCMsg_t *pcmsg,
        u_char *buf, int len)
{
    int i = 0;
    u_int8_t *circuit_id = 0;
    unsigned circuit_id_len = 0;
    u_int8_t *remote_id = 0;
    unsigned remote_id_len = 0;
    S_DhcpRelayCircuitId *pCId = NULL;
    /* u_char szName[INTERFACE_NAMSIZ + 1]; */
    u_int16_t nVlanId = 0;

    while (i < len) {
        /* If the next agent option overflows the end of the
         * packet, the agent option buffer is corrupt. 
         */
        if ((i + 1) == len || (i + buf [i + 1] + 2) > len) {
            return -1;
        }

        if (RAI_CIRCUIT_ID == buf [i]) {
            /* Remember where the circuit ID is... */
            circuit_id = &buf [i + 2];
            circuit_id_len = buf [i + 1];
            if (circuit_id[1] != (circuit_id_len - 2)) {
                return 0;
            }
            i += circuit_id_len + 2;
        } else if(RAI_REMOTE_ID == buf [i]) {
            /* Remember where the remote ID is... */
            remote_id = &buf [i + 2];
            remote_id_len = buf [i + 1];
            if (remote_id[1] != (remote_id_len - 2)) {
                return 0;
            }
            i += circuit_id_len + 2;
        } else {
            i += buf [i + 1] + 2;
        }
    }

    /* If there's no circuit ID or it length mismatch, 
     * it's not really ours, tell the caller it's no good. 
     */
    if (!circuit_id ||  DHCSNOOPING_MAX_CIRCUIT_ID_STRING < (circuit_id_len - 2)  
        || !remote_id || DHCSNOOPING_MAX_REMOTE_ID_STRING < (remote_id_len - 2)) {
        return 0;
    }

    /* not check circuit_id configured by user*/
    if (circuit_id[0] == 0) {   /*default*/
        pCId = (S_DhcpRelayCircuitId *)circuit_id;
        if (0xFFFF != pCId->nVlanId) {   /*for bug 18879 modified for different endian by liuyang 2012-3-30*/
            nVlanId = sal_ntoh16(pCId->nVlanId);
            /* pal_snprintf((char*)szName, INTERFACE_NAMSIZ + 1, "vlan%u", nVlanId); */
            /* search for vlan */
            if (nVlanId != pcmsg->vlanid) {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                        "Wrong default circuit id suboption for vlan id not matching\n");
                /*not matched */
                return 0;
            }
        } else {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
            "Wrong default circuit id suboption for invalid vlan id \n");
            /*not  matched */
            return 0;
        }
    }

    if (remote_id[0] == 0) { /*default*/
        tbl_sys_global_t *pGlobal = tbl_sys_global_get_sys_global();
        if (ETH_ALEN != (remote_id_len - 2)
            || memcmp(pGlobal->route_mac, &remote_id[2], ETH_ALEN)) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
            "Wrong default remote id suboption\n");
            return 0;
        }
    } else {
        tbl_dhcsnooping_t *pDhcsnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
        if (!memcmp(pDhcsnoopingCfg->remote_id_string, &remote_id[2], remote_id_len - 2)) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
                "Wrong user defined remote id suboption\n");
            return 0;
        }        
    }   
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "find option82 option inserted by local switch\n");
    return 1;
}

/*******************************************************************************
 * Name:    dhcsnooping_gen_relay_agent
 * Purpose: 
 * Input: 
 *   stCircuitId: circuit id
 *   stRemoteId:  remote id
 *   ifp: interface
 *   pcmsg: address info
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_gen_relay_agent(S_DhcpRelayCircuitId *pstCircuitId, 
    S_DhcpRelayRemoteId  *pstRemoteId, char *pszIntfName, DhcSnoopingCMsg_t *pcmsg)
{
    u_int16_t nVlanId = 0xFFFF;
    u_int32_t cModule = 0xFF;
    u_int32_t cPort = 0xFF;

    /*modified by wus for bug 54181, 2019-10-30*/
#ifndef GREATBELT
    u_int32_t csubPort = 0xFF;
#endif
    /*wus end*/

    /*add by huwx for bug 14939*/
    tbl_sys_global_t *pGlobal = tbl_sys_global_get_sys_global();
    
    if (NULL == pstCircuitId || NULL == pstRemoteId
        || NULL == pszIntfName || NULL == pcmsg) {
        return -1;
    }

    if (pcmsg->vlanid > 0 && pcmsg->vlanid <= GLB_VLAN_MAX) {
        nVlanId = pcmsg->vlanid;
    } else if (sal_strncasecmp(pszIntfName, DHCSNOOPING_VLAN_PREFIX, 
                         strlen(DHCSNOOPING_VLAN_PREFIX)) == 0) {
        nVlanId = sal_atoi(&pszIntfName[strlen(DHCSNOOPING_VLAN_PREFIX)]);
    }

    if (strncasecmp(pszIntfName, DHCSNOOPING_INTF_PREFIX, 
            sal_strlen(DHCSNOOPING_INTF_PREFIX)) == 0) {
        /* for eth-X-Y */
        /*modified by wus for bug 54181, 2019-10-30*/
        #ifndef GREATBELT
        sal_sscanf(pszIntfName, "eth-%u-%u_%u", &cModule, &cPort, &csubPort);
        #else
        sal_sscanf(pszIntfName, "eth-%u-%u", &cModule, &cPort);
        #endif
        /*wus end*/
    } else if (sal_strncasecmp(pszIntfName, DHCSNOOPING_AGG_PREFIX,
            sal_strlen(DHCSNOOPING_AGG_PREFIX)) == 0) {
        /* for aggX */
        sal_sscanf(pszIntfName, "agg%u", &cPort);
        cModule = 0xFE;
    } else if (sal_strncasecmp(pszIntfName, DHCSNOOPING_TUNNEL_PREFIX,
            sal_strlen(DHCSNOOPING_TUNNEL_PREFIX)) == 0) {
        /* for nullX */
        sal_sscanf(pszIntfName, "tunnel%u", &cPort);
        cModule = 0xFD;
    }

    /* generate agent options:
     * 1) Circuit Id Format: 
     *   Circuit ID Type + Length + VLAN + Module + Port + Subport
     *   1 byte + 1 byte + 2 bytes + 1 byte + 1 byte + 1 byte
     * 2) Remote Id Format: 
     *   Remote ID Type + Length + Mac Address
     *   1 byte + 1 byte + 6 bytes
     */
    pstCircuitId->cType = 0;
    pstCircuitId->cLength = sizeof(S_DhcpRelayCircuitId) - 2;
    pstCircuitId->nVlanId = sal_htons(nVlanId);
    pstCircuitId->cModule = (cModule & 0xFF);
    pstCircuitId->cPort = (cPort & 0xFF);
    
    /*modified by wus for bug 54181, 2019-10-30*/
#ifndef GREATBELT
    if (0xff == csubPort)
    {
        csubPort = 0;
    }
    pstCircuitId->cSubPort = (csubPort & 0xFF);
#endif
    /*wus end*/

    pstRemoteId->cType = 0;
    pstRemoteId->cLength = sizeof(S_DhcpRelayRemoteId) - 2;
    /*modify by huwx for bug 14939, 2011-05-23*/
    memcpy(pstRemoteId->nMacArr, pGlobal->route_mac, ETH_ALEN);
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_add_relay_agent_options
 * Purpose: add relay agent options
 * Input: 
 *   pHdr: packet info
 *   pcmsg: address info
 *   ifp: interface info
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_add_relay_agent_options(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    tbl_interface_t *pIntf, struct packet *pDhcpPkt)
{
    struct dhcp_packet *packet = pHdr->dhcp;
    u_int8_t *op        = NULL;
    u_int8_t *nextop    = NULL;
    u_int8_t *sp        = NULL;
    u_int8_t *max       = NULL;
    u_int8_t *end_pad   = NULL;
    struct ip *iph      = NULL;
    tbl_dhcsnooping_t       *pDhcSnoopingCfg    = NULL;
    ds_circuit_id_t         *pst_circuit_id     = NULL;
    ds_circuit_id_t         *pItem              = NULL;
    S_DhcpRelayCircuitId    stCircuitId;
    S_DhcpRelayRemoteId     stRemoteId;
    int         is_dhcp = 0;
    int         length  = 0;
    unsigned    optlen;
    vid_t       cur_vid;

    /* If there's no cookie, it's a bootp packet, so we should just
     *  forward it unchanged. 
     */
    if (sal_memcmp(packet->options, DHCP_OPTIONS_COOKIE, 4)) {
        return 0;
    }

    max = ((u_int8_t *)packet) + pHdr->nDhcpLen;
    /* Commence processing after the cookie. */
    sp = op = (u_int8_t *)&packet->options[4];
    while (op < max) {
        switch (*op) {
        case DHO_PAD:
            /* Skip padding... */

            /* Remember the first pad byte so we can commandeer
             * padded space.
             *
             * XXX: Is this really a good idea?  Sure, we can
             * seemingly reduce the packet while we're looking,
             * but if the packet was signed by the client then
             * this padding is part of the checksum (RFC3118),
             * and its nonpresence would break authentication.
             */
            if (end_pad == NULL) {
                end_pad = sp;
            }

            if (sp != op) {
                *sp++ = *op++;
            } else {
                sp = ++op;
            }

            continue;

        case DHO_DHCP_MESSAGE_TYPE:
            /* If we see a message type, it's a DHCP packet. */
            is_dhcp = 1;
            goto skip;
            break;

        case DHO_END:
            /* Quit immediately if we hit an End option. */
            goto out;

        case DHO_DHCP_AGENT_OPTIONS:
            /* We shouldn't see a relay agent option in a
             * packet before we've seen the DHCP packet type,
             * but if we do, we have to leave it alone. 
             */
            if (!is_dhcp) {
                goto skip;
            }

            end_pad = NULL;
            /* There's already a Relay Agent Information option
             * in this packet.   How embarrassing.   Decide what
             * to do based on the mode the user specified. 
             */
            goto skip;

            /* Skip over the agent option and start copying 
             * if we aren't copying already. 
             */
            op += op[1] + 2;
            break;

skip:
            /* Skip over other options. */
        default:
            /* Fail if processing this option will exceed the
             * buffer (op[1] is malformed).
             */
            nextop = op + op[1] + 2;
            if (nextop > max) {
                return 0;
            }

            end_pad = NULL;
            if (sp != op) {
                memmove(sp, op, op[1] + 2);
                sp += op[1] + 2;
                op = nextop;
            } else {
                op = sp = nextop;
            }
            break;
        }
    }

out:
    /* If it's not a DHCP packet, we're not supposed to touch it. */
    if (!is_dhcp) {
        return 0;
    }

    /* If the packet was padded out, we can store the agent option
     * at the beginning of the padding. 
     */
    if (end_pad != NULL) {
        sp = end_pad;
    }

    /* Remember where the end of the packet was after parsing it. */
    op = sp;
    
    /* getting relay agent options */
    cur_vid = 0;
    if (pcmsg->vlanid > 0 && pcmsg->vlanid <= GLB_VLAN_MAX) {
        cur_vid = pcmsg->vlanid;
    } else if (strncasecmp(pIntf->key.name, DHCSNOOPING_VLAN_PREFIX, 
                             sal_strlen(DHCSNOOPING_VLAN_PREFIX)) == 0) {
        cur_vid = sal_htons(atoi(&pIntf->key.name[sal_strlen(DHCSNOOPING_VLAN_PREFIX)]));
    }
    
    ctclib_slistnode_t *lnode = NULL;
    CTCLIB_SLIST_LOOP(pIntf->circuit_id.obj_list, pItem, lnode) {
        if (pItem->key.vid == cur_vid && '\0' != pItem->circuit[0]) {
            pst_circuit_id = pItem;
            break;
        }
    }

    pDhcSnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
    if (!(pst_circuit_id && sal_strlen(pDhcSnoopingCfg->remote_id_string))) {
        /*add default circuit id*/
        dhcsnooping_gen_relay_agent(&stCircuitId, &stRemoteId, pIntf->key.name, pcmsg);
    }
    /* Sanity check.  Had better not ever happen. */
    if ((sizeof(S_DhcpRelayCircuitId) > 255) || (sizeof(S_DhcpRelayCircuitId) < 1)) {
        log_sys(M_MOD_INTERNAL, E_ERROR, 
                "circuit id length %d out of range [1-255] on %s",
                sizeof(S_DhcpRelayCircuitId), pIntf->key.name);
        return -1;
    }

    if (pst_circuit_id) {
        optlen = sal_strlen(pst_circuit_id->circuit) + 4;
    } else {
        optlen = sizeof(S_DhcpRelayCircuitId) + 2;            /* RAI_CIRCUIT_ID + len */
    }

    if (sizeof(S_DhcpRelayRemoteId) > 255 || sizeof(S_DhcpRelayRemoteId) < 1) {
        log_sys(M_MOD_INTERNAL, E_ERROR, 
                "remote id length %d out of range [1-255] on %s", 
                sizeof(S_DhcpRelayRemoteId), pIntf->key.name);
        return -1;
    }
    if (sal_strlen(pDhcSnoopingCfg->remote_id_string)) {
        optlen += sal_strlen(pDhcSnoopingCfg->remote_id_string) + 4;
    } else {
        optlen += sizeof(S_DhcpRelayRemoteId) + 2;    /* RAI_REMOTE_ID + len */
    }

    /* We do not support relay option fragmenting (multiple options to
     * support an option data exceeding 255 bytes).
     */
    if ((optlen < 3) || (optlen > 255)) {
        log_sys(M_MOD_INTERNAL, E_ERROR, 
                "total agent option length (%u) out of range [3 - 255] on %s", 
                optlen, pIntf->key.name);
        return -1;
    }

    /* modified by weij for bug 3926, 
     * check whether the total length(include option length) is big than the max length 
     */
    if ((sp > max) || (sp - ((u_int8_t *)packet) + optlen + 3 > DHCP_MTU_MAX)) {
        return -1;
    }

    /* Okay, cons up *our* Relay Agent Information option. */
    *sp++ = DHO_DHCP_AGENT_OPTIONS;
    *sp++ = optlen;
    /*for bug 23008 modified option82 by liuyang 2013-6-13*/
    /* Copy in the circuit id... */
    *sp++ = RAI_CIRCUIT_ID;
    if (pst_circuit_id) {
        *sp++ = sal_strlen(pst_circuit_id->circuit) + 2;
        *sp++ = 1;
        *sp++ = sal_strlen(pst_circuit_id->circuit);
        memcpy(sp, pst_circuit_id->circuit, sal_strlen(pst_circuit_id->circuit));
        sp += sal_strlen(pst_circuit_id->circuit);
    } else {
        *sp++ = sizeof(S_DhcpRelayCircuitId);
        memcpy(sp, &stCircuitId, sizeof(S_DhcpRelayCircuitId));
        sp += sizeof(S_DhcpRelayCircuitId);
    }

    /* Copy in remote ID... */
    *sp++ = RAI_REMOTE_ID;
    if (sal_strlen(pDhcSnoopingCfg->remote_id_string)) {
        *sp++ = sal_strlen(pDhcSnoopingCfg->remote_id_string) + 2;
        *sp++ = 1;
        *sp++ = sal_strlen(pDhcSnoopingCfg->remote_id_string);
        memcpy(sp, pDhcSnoopingCfg->remote_id_string, sal_strlen(pDhcSnoopingCfg->remote_id_string));
        sp += sal_strlen(pDhcSnoopingCfg->remote_id_string);
    } else {
        *sp++ = sizeof(S_DhcpRelayRemoteId);
        memcpy(sp, &stRemoteId, sizeof(S_DhcpRelayRemoteId));
        sp += sizeof(S_DhcpRelayRemoteId);
    }
    /* Deposit an END option. */
    *sp++ = DHO_END;

    /* Recalculate total packet length. */
    /*modified by wus for bug 53375 in 2019-09-18*/
    if (max - sp > 0)
    {
        length = max - ((u_int8_t *)packet);
    }
    else
    {
        length = sp - ((u_int8_t *)packet);
    }

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "add relay agent informations %s", "success");
    
    /* Make sure the packet isn't short (this is unlikely, but WTH) 
    ** for bug 18244 modified ip length when add pad by liuyang 2012-4-16
    */
    if (length < BOOTP_MIN_LEN) {
        memset(sp, DHO_PAD, BOOTP_MIN_LEN - length);
        length = BOOTP_MIN_LEN;

        iph  = pHdr->uiph;
        iph->ip_len += sal_hton16(length - pHdr->nDhcpLen);
        /*ip_sum is short type while the result of function wrapsum is u_int32*/
        iph->ip_sum = 0;
        iph->ip_sum = wrapsum(checksum((u_char *)iph, IP_HL(iph), 0));
        /* change packet length */
        pDhcpPkt->packet_length += (length - pHdr->nDhcpLen);
    }
     /* added by weij for bug 3926, 
     * we add space to the packet, so should change the length and checksum
     */
    else if (max - op < optlen + 3) {
        /* change udp length 
        udp = pHdr->udp;
        udp->uh_ulen += optlen + 3 + op - max;
        */
        /* change ip length */
        iph  = pHdr->uiph;
        iph->ip_len += sal_hton16(optlen + 3 + op - max);
        
        /* change ip checksum */
        iph->ip_sum = 0;
        iph->ip_sum = wrapsum(checksum((u_char *)iph, IP_HL(iph), 0));
        /* change packet length */
        pDhcpPkt->packet_length += optlen + 3 + op - max;
    }

    pHdr->nTotalLen += (length - pHdr->nDhcpLen);
    pHdr->nDhcpLen = length;
    /*for bug 5289 */
    pHdr->udp->uh_ulen = sal_hton16(length + 8);
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_del_relay_agent_options
 * Purpose: del relay agent options
 * Input: 
 *   pHdr: packet info
 *   pcmsg: address info
 *   ifp: interface info
 *   pDhcpPkt: dhcp packet
 *   bDelUs: del relay agent added by us
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_del_relay_agent_options(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    char *pszIntfName, struct packet *pDhcpPkt, int bDelUs)
{
    struct dhcp_packet *packet = NULL;
    u_int8_t *op = NULL, *nextop = NULL, *sp = NULL, *max = NULL;
    int is_dhcp = 0;
    struct ip *iph = NULL;
    
    if (NULL == pHdr || NULL == pHdr->dhcp || NULL == pcmsg || NULL == pszIntfName 
        || NULL == pDhcpPkt || NULL == pDhcpPkt->options) {
        return -1;
    }

    packet = pHdr->dhcp;
    /* If there's no cookie, it's a bootp packet, 
     * so we should just forward it unchanged. 
     */
    if (memcmp(packet->options, DHCP_OPTIONS_COOKIE, 4)) {
        return 0;
    }

    max = ((u_int8_t *)packet) + pHdr->nDhcpLen;
    sp = op = (u_int8_t *)&packet->options[4];
    while (op < max) {
        switch (*op) {
        case DHO_PAD:
            /* Skip padding... */
            if (sp != op) {
                *sp = *op;
            }
            ++op;
            ++sp;
            continue;

        case DHO_DHCP_MESSAGE_TYPE:
            /* If we see a message type, it's a DHCP packet. */
            is_dhcp = 1;
            goto skip;
            break;

        case DHO_END:
            /* Quit immediately if we hit an End option. */
            if (sp != op) {
                *sp++ = *op++;
            }
            goto out;
            
        case DHO_DHCP_AGENT_OPTIONS:
            /* We shouldn't see a relay agent option in a
             * packet before we've seen the DHCP packet type,
             * but if we do, we have to leave it alone. 
             */
            if (!is_dhcp) {
                goto skip;
            }

            /* Do not process an agent option if it exceeds the
             * buffer.  Fail this packet.
             */
            nextop = op + op[1] + 2;
            if (nextop > max) {
                return -1;
            }

            /* del relay agent options added by us? */
            if (bDelUs && 
                  !dhcsnooping_find_interface_by_agent_option(packet, pcmsg, op + 2, op[1])) {
                goto skip;
            }
            op = nextop;
            break;

skip:
        default:
            /* Skip over other options. */
            /* Fail if processing this option will exceed the
             * buffer (op[1] is malformed).
             */
            nextop = op + op[1] + 2;
            if (nextop > max) {
                return -1;
            }

            if (sp != op) {
                memmove(sp, op, op[1] + 2);
                sp += op[1] + 2;
                op = nextop;
            } else {
                op = sp = nextop;
            }

            break;
        }
    }
    
out:
    /* If it's not a DHCP packet, we're not supposed to touch it. */
    if (!is_dhcp) {
        return 0;
    }

    /* Adjust the length... */
    if (sp != op) {
        int length = sp - ((u_int8_t *)packet);

        /* Make sure the packet isn't short (this is unlikely, but WTH) */
        if (length < BOOTP_MIN_LEN) {
            memset (sp, DHO_PAD, BOOTP_MIN_LEN - length);
            length = BOOTP_MIN_LEN;
        }

        pHdr->nTotalLen -= (pHdr->nDhcpLen - length);
        /* for bug 23008 modified udp-len ip-len ip-checksum by liuyang 2013-6-7*/
        iph = pHdr->uiph;

        /* add by huwx for bug 14962, 2011-08-16*/
        pHdr->udp->uh_ulen -= sal_hton16((pHdr->nDhcpLen - length));
        iph->ip_len -= sal_hton16(pHdr->nDhcpLen - length);

        /* change ip checksum */
        iph->ip_sum = 0;
        iph->ip_sum = wrapsum(checksum((u_char *)iph, IP_HL(iph), 0));
        /* change packet length */
        pDhcpPkt->packet_length -= (pHdr->nDhcpLen - length);
        pHdr->nDhcpLen = length;
    }
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "del relay agent informations %s", "success");
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_do_snooping_bootp
 * Purpose: do snooping for bootp protocol
 * Input: 
 *   pHdr:  packet headers
 *   pcmsg: address infos
 *   ifp: interface
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_do_snooping_bootp(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
tbl_interface_t *pIntf, struct packet *pDhcpPkt)
{
    u_int bSrvMsg = 0; /* is it server side message ? */
    struct dhcp_packet *dhcp = NULL;
    u_char szDestIp[128];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    tbl_dhcsnooping_t *pDhcSnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
        
    if (NULL == pDhcpPkt || NULL == pIntf || NULL == pHdr || NULL == pcmsg) {
        return -1;
    }

    dhcp = pHdr->dhcp;
    bSrvMsg = (dhcp->op == BOOTREPLY);
    sal_snprintf((char*)szDestIp, 128, "%s", inet_ntoa(pHdr->uiph->ip_dst));

    if (IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT)) {
        u_char *pSrcMac = NULL;
        u_char *pDstMac = NULL;
        
        pSrcMac = pcmsg->src_mac;
        pDstMac = pcmsg->dest_mac;

        DEBUG_SHOW(dhcp_debug, flags, 
                  "BOOTP %s on %s %02x:%02x:%02x:%02x:%02x:%02x > "
                  "%02x:%02x:%02x:%02x:%02x:%02x %s > %s",
                  bSrvMsg ? "reply" : "request", IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), 
                  pSrcMac[0], pSrcMac[1], pSrcMac[2], pSrcMac[3], pSrcMac[4], pSrcMac[5],
                  pDstMac[0], pDstMac[1], pDstMac[2], pDstMac[3], pDstMac[4], pDstMac[5],
                  inet_ntoa(pHdr->uiph->ip_src), szDestIp);
    }
    dhcsnooping_dump_packet(pDhcpPkt);

    /* check vlan information */
    if (!bSrvMsg) {
        if (pcmsg->vlanid == 0 || pcmsg->vlanid > GLB_VLAN_MAX
                || !GLB_BMP_ISSET(pDhcSnoopingCfg->vlans, pcmsg->vlanid)) {
            /* invalid vlan, resend this packet */
            ++g_stStat.ullForwarded;
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                    "forward DHCP message for unsnooping VLAN (VLAN %d)", pcmsg->vlanid);
            goto do_send;
        }
    }
    
    /* verify mac-address if needed */
    if (!bSrvMsg && pDhcSnoopingCfg->verify_mac_address) {
        if (memcmp(pcmsg->src_mac, dhcp->chaddr, ETH_ALEN) != 0) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                    "verify mac-address failed");
            ++g_stStat.ullMacVerifyFailed;
            return 0;
        }
    }

    /* process for untrust port */
    if (!(pIntf->dhcp_snooping_trust)) {
        if (bSrvMsg) {
            /* invalidate server side message from untrusted port */
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR, 
                      "drop server side message from untrusted port: %s message BOOTP %s",
                      IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), bSrvMsg ? "reply" : "request");
            ++g_stStat.ullDropped;
            return 0;
        }

        /* can't check option-82 for bootp message */

        /* drop message with relay agent ip address */
        if (0 != dhcp->giaddr.s_addr) {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                      "drop BOOTREQUEST message with relay agent ip address");
            ++g_stStat.ullDropped;
            return 0;
        }

        /* no DHCPDECLINE & DHCPRELEASE message for bootp */
    } else {
        /* process for trust port */

        /* how to determine host is directly connected to us: XXX */
        if (BOOTREPLY == dhcp->op) {
            dhcsnooping_add_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
        }
    }

    /* prepare binding item */
    if (BOOTREQUEST == dhcp->op) {
        dhcsnooping_prepare_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
    }

    /* can't insert relay agent information for bootp */

do_send:
    /* resend this packet */
    if (dhcsnooping_send_pkt(pHdr, pHdr->nTotalLen, 
                              pcmsg, sizeof(DhcSnoopingCMsg_t)) != 0) {
        log_sys(M_MOD_INTERNAL, E_ERROR, "send DHCP snooping packet failed");
        return -1;
    }

    ++g_stStat.ullForwarded;
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "forward BOOTP %s from %s to %s success.",
              bSrvMsg ? "reply" : "request",
              inet_ntoa(pHdr->uiph->ip_src), szDestIp);
    return 0;
}

int32
dhcsnooping_add_tbl_to_ipsg(tbl_dhcbinding_t *p_db_item)
{
    /*if ipsg not exsit this item, call ipsg to add*/
    if (FALSE == p_db_item->ipsg_tbl_exsit)
    {
        p_db_item->ipsg_tbl_exsit = TRUE;
        
        return tbl_dhcbinding_set_dhcbinding_field(p_db_item, TBL_DHCBINDING_FLD_IPSG_TBL_EXSIT);
    }
    
    return PM_E_NONE;
}
/*******************************************************************************
 * Name:    dhcsnooping_do_snooping_dhcp
 * Purpose: do snooping for dhcp protocol
 * Input: 
 *   pHdr: packet infos
 *   pcmsg: address infos
 *   ifp: interface info
 *   pDhcpPkt: dhcp packet info
 * Output: 
 * Return: 
 *   success: 0
 *   resend this packet: -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_do_snooping_dhcp(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    tbl_interface_t *pIntf, struct packet *pDhcpPkt)
{
    tbl_dhcsnooping_t *pDhcSnoopingCfg = tbl_dhcsnooping_get_dhcsnooping();
    u_int bSrvMsg = 0; /* is it server side message ? */
    struct option_cache *op = NULL;
    struct udphdr *udp = NULL;
    struct dhcp_packet *dhcp = NULL;
    u_char szDestIp[128];
    u_char *pSrcMac = NULL;
    u_char *pDstMac = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
        
    if (NULL == pDhcpPkt || NULL == pIntf || NULL == pHdr || NULL == pcmsg) 
    {
        return -1;
    }

    dhcp = pHdr->dhcp;
    sal_snprintf((char*)szDestIp, 128, "%s", inet_ntoa(pHdr->uiph->ip_dst));

    if (IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT)) 
    {
        pSrcMac = pcmsg->src_mac;
        pDstMac = pcmsg->dest_mac;
        
        DEBUG_SHOW(dhcp_debug, flags, 
                  "%s on %s %02x:%02x:%02x:%02x:%02x:%02x > "
                  "%02x:%02x:%02x:%02x:%02x:%02x %s > %s",
                  dhcsnooping_get_msg_name(pDhcpPkt->packet_type), IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), 
                  pSrcMac[0], pSrcMac[1], pSrcMac[2], pSrcMac[3], pSrcMac[4], pSrcMac[5],
                  pDstMac[0], pDstMac[1], pDstMac[2], pDstMac[3], pDstMac[4], pDstMac[5],
                  inet_ntoa(pHdr->uiph->ip_src), szDestIp);
    }
    dhcsnooping_dump_packet(pDhcpPkt);

    bSrvMsg = dhcsnooping_is_server_side_msg(pDhcpPkt->packet_type);

    /* check vlan information */
    if (!bSrvMsg) 
    {
        if (pcmsg->vlanid == 0 || pcmsg->vlanid > GLB_VLAN_MAX
                || !GLB_BMP_ISSET(pDhcSnoopingCfg->vlans, pcmsg->vlanid)) 
        {
            /* invalid vlan, resend this packet */
            ++g_stStat.ullForwarded;
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                    "forward DHCP message for unsnooping VLAN (VLAN %d)", pcmsg->vlanid);
            goto do_send;
        }
    }

    /* verify mac-address if needed */
    if (!bSrvMsg && pDhcSnoopingCfg->verify_mac_address) 
    {
        if (memcmp(pcmsg->src_mac, dhcp->chaddr, ETH_ALEN) != 0) 
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR, 
                      "verify mac-address failed");
            ++g_stStat.ullMacVerifyFailed;
            return 0;
        }
    }

    /* locate option 82 option */
    if (pDhcpPkt->options_valid) 
    {
        op = lookup_option(&dhcp_universe, pDhcpPkt->options, DHO_DHCP_AGENT_OPTIONS);
    }

    /* process for untrust port */
    if (!(pIntf->dhcp_snooping_trust)) 
    {
        if (bSrvMsg)
        {
            /* invalidate server side message from untrusted port */
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                      "drop server side message from untrusted port: %s message %s",
                      IFNAME_ETH2FULL(pIntf->key.name, ifname_ext), dhcsnooping_get_msg_name(pDhcpPkt->packet_type));
            ++g_stStat.ullDropped;
            return 0;
        }

        if (!pDhcSnoopingCfg->allow_untrusted) 
        {
            /* don't enable option-82 in untrusted port */
            if (NULL != op) 
            {
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                          "drop dhcp packet have relay agent information in untrusted port");
                ++g_stStat.ullDropped;
                return 0;
            }
        }

        /* drop message with relay agent ip address */
        if (0 != dhcp->giaddr.s_addr) 
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                      "drop %s message with relay agent ip address",
                      dhcsnooping_get_msg_name(pDhcpPkt->packet_type));
            ++g_stStat.ullDropped;
            return 0;
        }
    } 
    else 
    {
        /* process for trust port */

        if (DHCPACK == pDhcpPkt->packet_type) 
        {
            /* add binding table */
            dhcsnooping_add_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
        } 
        else if (DHCPNAK == pDhcpPkt->packet_type) 
        {
            /* del binding table */
            dhcsnooping_del_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
        }
    }

    /* check interface for DHCPDECLINE & DHCPRELEASE message */
    if (DHCPDECLINE == pDhcpPkt->packet_type 
        || DHCPRELEASE == pDhcpPkt->packet_type) 
    {
        /* drop mismatched interface message */
        if (!dhcsnooping_release_check(pHdr, pcmsg, pIntf->key.name, pDhcpPkt)) 
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                      "drop binding database mismatched %s",
                      dhcsnooping_get_msg_name(pDhcpPkt->packet_type));
            ++g_stStat.ullDropped;
            return 0;
        }
    
        /* del binding table */
        dhcsnooping_del_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
    }

    /* prepare binding item */
    if (DHCPREQUEST == pDhcpPkt->packet_type) 
    {
        dhcsnooping_prepare_bindingitem(pHdr, pcmsg, pIntf->key.name, pDhcpPkt);
    }

    /* insert relay agent information */
    if (pDhcSnoopingCfg->add_agent_options) 
    {
        /*we do not add agent info to DHCPINFORM, like Cisco */
        if (dhcp->op == BOOTREQUEST && (DHCPINFORM != pDhcpPkt->packet_type)) 
        {
            /* insert option 82 information */
            if (NULL != op) {
                /* remove old option 82 information */
                if (dhcsnooping_del_relay_agent_options(pHdr, pcmsg, 
                            pIntf->key.name, pDhcpPkt, 0) != 0) {
                    goto err_drop;
                }
            }
            if (dhcsnooping_add_relay_agent_options(pHdr, pcmsg, 
                        pIntf, pDhcpPkt) != 0) {
                goto err_drop;
            }
            
            /* I don't known how to setting giaddr: XXX */
        } 
        else 
        {
            /* process for BOOTREPLY */

            /* remove options 82 message added by us */
            if (dhcsnooping_del_relay_agent_options(pHdr, pcmsg, 
                    pIntf->key.name, pDhcpPkt, 1) != 0) {
                goto err_drop;
            }
        }

        /* recalculate udp checksum */
        udp = pHdr->udp;
        udp->uh_sum = 0;
        udp->uh_sum = wrapsum(checksum((u_char *)udp, sizeof *udp,
                checksum((u_char *)dhcp, pHdr->nDhcpLen, 
                checksum((unsigned char *)&pHdr->uiph->ip_src,
                2 * sizeof(pHdr->uiph->ip_src), IPPROTO_UDP + ntohs(udp->uh_ulen)))));

        DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP,
                      "dump the DHCP message (after snooping)",
                      (u_char *)(&pHdr->uiph), pHdr->nDhcpLen, 0);
        if (IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET)) {
            /* reparse the options */
            if (pDhcpPkt->options) {
                option_state_dereference(&pDhcpPkt->options, MDL);
                pDhcpPkt->options = NULL;
                pDhcpPkt->options_valid = 0;
            }
            if (!parse_options(pDhcpPkt)) {
                if (pDhcpPkt->options) {
                    option_state_dereference(&pDhcpPkt->options, MDL);
                }
            }

            dhcsnooping_dump_packet(pDhcpPkt);
        }
    }

do_send:
    /* resend this packet */
    if (dhcsnooping_send_pkt(pHdr, pHdr->nTotalLen, 
                              pcmsg, sizeof(DhcSnoopingCMsg_t)) != 0) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "send DHCP snooping packet %s", "failed");
        return -1;
    }
    ++g_stStat.ullForwarded;
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "forward %s from %s to %s success.",
              dhcsnooping_get_msg_name(pDhcpPkt->packet_type),
              inet_ntoa(pHdr->uiph->ip_src), szDestIp);
    return 0;

err_drop:
    /* drop this packet */
    ++g_stStat.ullDropped;
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
              "process relay agent options failed");
    return -1;
}

/*******************************************************************************
 * Name:    dhcsnooping_do_snooping
 * Purpose: do snooping action
 * Input: 
 *   pHdr:  packet infos
 *   pcmsg: address infos
 *   ifp: interface pointer
 *   pDhcpPkt: dhcp packet
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_do_snooping(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg, 
    tbl_interface_t *pIntf, struct packet *pDhcpPkt)
{
    struct dhcp_packet *dhcp = NULL;
    
    dhcp = pHdr->dhcp;
    if (NULL == dhcp || NULL == pDhcpPkt) {
        return -1;
    }

    if (pDhcpPkt->packet_type) {
        ++g_stStat.ullDhcpPkts;
        /* dhcp protocol */
        return dhcsnooping_do_snooping_dhcp(pHdr, pcmsg, pIntf, pDhcpPkt);
    }

    /* bootp protocol */
    ++g_stStat.ullBootpPkts;
    return dhcsnooping_do_snooping_bootp(pHdr, pcmsg, pIntf, pDhcpPkt);
}

/*******************************************************************************
 * Name:    dhcsnooping_decode_dhcp
 * Purpose: decode this dhcp packet
 * Input: 
 *   pHdr: packet data
 *   pcmsg: address info
 * Output: 
 *   pDhcpPkt: dhcp packet 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_decode_dhcp(S_DhcpSnoopingInHdr *pHdr, 
                         DhcSnoopingCMsg_t *pcmsg, struct packet *pDhcpPkt)
{
    struct hardware_ds *haddr = NULL;
    struct dhcp_packet *packet = NULL;
    struct option_cache *op = NULL;
    
    if (NULL == pHdr || NULL == pDhcpPkt) {
        return -1;
    }

    haddr = &pHdr->stHAddr;
    packet = pHdr->dhcp;
    pDhcpPkt->raw = packet;
    pDhcpPkt->packet_length = pHdr->nDhcpLen;
    pDhcpPkt->client_port = pHdr->udp->uh_sport;
    pDhcpPkt->client_addr.len = 4; /* supports IPv4 only */
    memcpy(&pDhcpPkt->client_addr.iabuf, &pHdr->uiph->ip_src, 4);
    haddr->hlen = ETH_ALEN + 1;
    haddr->hbuf[0] = ARPHRD_ETHER;
    memcpy(&haddr->hbuf[1], pcmsg->src_mac, ETH_ALEN);
    sal_memcpy(&pDhcpPkt->haddr, &haddr, sizeof(struct hardware_ds));

    if (packet->hlen > sizeof(packet->chaddr)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "discarding packet with bogus hlen");
        ++g_stStat.ullInvalid;
        return -1;
    }

    if (pDhcpPkt->packet_length >= DHCP_FIXED_NON_UDP + 4) {
        /* parse option */
        if (!parse_options (pDhcpPkt)) {
            if (pDhcpPkt->options) {
                option_state_dereference(&pDhcpPkt->options, MDL);
                packet_dereference(&pDhcpPkt, MDL);
                DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR, 
                          "parse DHCP options failed");
                ++g_stStat.ullInvalid;
                return -1;
            }
        }

        if (pDhcpPkt->options_valid &&
            (op = lookup_option(&dhcp_universe, pDhcpPkt->options, DHO_DHCP_MESSAGE_TYPE))) {
            struct data_string dp;
            memset (&dp, 0, sizeof dp);
            evaluate_option_cache (&dp, pDhcpPkt, (struct lease *)0, (struct client_state *)0,
                           pDhcpPkt -> options, (struct option_state *)0,
                           (struct binding_scope **)0, op, MDL);
            if (dp.len > 0) {
                pDhcpPkt->packet_type = dp.data[0];
            } else {
                pDhcpPkt->packet_type = 0;
            }
            data_string_forget (&dp, MDL);
        }
    }

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT, 
              "decode DHCP message success");
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_process_pkt
 * Purpose: process one packet from snooping socket
 * Input: 
 *   pHdr:     packet header
 *   pcmsg:    packet address info
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int 
dhcsnooping_process_pkt(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg)
{
    DhcSnoopingCMsg_t   *pVlan = pcmsg;
    tbl_interface_t     *pIntf = NULL;
    struct packet stDhcpPkt;
    int nRet = 0;
    
    if (NULL == pHdr || NULL == pcmsg) {
        return -1;
    }
    
    /* locate interface */
    if (NULL == (pIntf = tbl_interface_get_interface_by_ifindex(pVlan->ifindex))) { 
        /* can't locate interface, resend this packet */
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR, 
                  "forward DHCP message for unknown interface (ifindex %u)",
                  pVlan->ifindex);
        ++g_stStat.ullForwarded;
        goto send_out;
    }

    /* decode dhcp packets */
    memset(&stDhcpPkt, 0x00, sizeof(struct packet));
    if (dhcsnooping_decode_dhcp(pHdr, pcmsg, &stDhcpPkt)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                 "decode DHCP message failed");
        ++g_stStat.ullInvalid;
        return -1;
    }

    /* do true snooping operation */
    nRet = dhcsnooping_do_snooping(pHdr, pcmsg, pIntf, &stDhcpPkt);
    if (NULL != stDhcpPkt.options) {
        option_state_dereference(&stDhcpPkt.options, MDL);
    }
    return nRet;
    
send_out:
    if (dhcsnooping_send_pkt(pHdr, pHdr->nTotalLen, 
                              pcmsg, sizeof(DhcSnoopingCMsg_t)) != 0) {
        log_sys(M_MOD_INTERNAL, E_ERROR, 
                "send DHCP snooping packet %s", "failed");
    }
    return -1;
}

/*******************************************************************************
 * Name:    dhcsnooping_validate_pkt
 * Purpose: validate this dhcp packet
 * Input: 
 *   pHdr: packet header
 *   nLen: packet size
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_validate_pkt(S_DhcpSnoopingInHdr *pHdr, int nLen)
{
    struct ip *iph = NULL;
    struct udphdr *udp = NULL;
    int         nLeftLen = nLen;
    int         nIpHLen = 0;
    int         nUdpLen = 0;
    int         nDhcpLen = 0;
    u_int32_t   sum = 0, usum = 0;
    
    if (NULL == pHdr || nLen <= 0) {
        return -1;
    }
    iph  = pHdr->uiph;

    if (nLen < sizeof(struct ip)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "drop too small packet");
        ++g_stStat.ullInvalid;
        return -1;
    }

    if (iph->ip_p != IPPROTO_UDP) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR, 
                  "drop unknown protocol packet %u", iph->ip_p);
        ++g_stStat.ullInvalid;
        return -1;
    }

    /* modified by liwh for bug 40761, 2016-09-13 */
    /* Check the IP packet length. */
    /*    if (sal_ntohs(iph->ip_len) != nLeftLen) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "ip length %d disagrees with bytes received %d",
                  ntohs(iph->ip_len), nLeftLen);
        ++g_stStat.ullInvalid;
        return -1;
    }
    */
    /* liwh end */
    
    /* Check the IP header checksum - it should be zero. */
    nIpHLen = IP_HL(iph);
    if (wrapsum(checksum((u_char *)iph, nIpHLen, 0))) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "drop bad IP checksums packet");
        ++g_stStat.ullInvalid;
        return -1;
    }

    /* check udp packet length */
    pHdr->udp = (struct udphdr *)((u_char *)iph + nIpHLen);
    udp = pHdr->udp;
    if (udp->uh_dport != htons(68) && udp->uh_dport != htons(67)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "unknown UDP port number: %u", ntohs(udp->uh_dport));
        ++g_stStat.ullInvalid;
        return -1;
    }
    nUdpLen = sal_ntohs(udp->uh_ulen);
    if (nUdpLen < sizeof(struct udphdr) 
        || nUdpLen != (nLeftLen - nIpHLen)) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "bogus UDP packet length: %d", nUdpLen);
        ++g_stStat.ullInvalid;
        return -1;
    }

    /* Compute UDP checksums, including the ``pseudo-header'', the UDP
     * header and the data.   If the UDP checksum field is zero, we're
     * not supposed to do a checksum.
     */
    nDhcpLen = nUdpLen - sizeof(struct udphdr);
    if (nDhcpLen != (nLeftLen - nIpHLen - sizeof(struct udphdr))) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "bogus UDP packet length: %d", nUdpLen);
        ++g_stStat.ullInvalid;
        return -1;
    }
    
    pHdr->dhcp = (struct dhcp_packet *)((u_char *)udp + sizeof(struct udphdr));
    pHdr->nDhcpLen = nDhcpLen;
    usum = udp->uh_sum;
    udp->uh_sum = 0;
    sum = wrapsum(checksum((u_char *)udp, sizeof *udp,
                checksum((u_char *)pHdr->dhcp, nDhcpLen, checksum((unsigned char *)&iph->ip_src,
                2 * sizeof iph->ip_src, IPPROTO_UDP + (u_int32_t)nUdpLen))));
    if (usum && usum != sum) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR,
                  "drop bad udp checksums packet");
        ++g_stStat.ullInvalid;
        return -1;
    }
    /* for bug 21681 modified by liuyang 2012-12-26*/
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT, 
             "validate ip and udp header for DHCP packet success");
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_service_onoff
 * Purpose: starting/stopping dhcp/dhcpv6 snooping service
 * Input: 
 *   cli: client pointer
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_service_onoff()
{
    int nFlags = 0;
    
    if (is_dhcsnooping_service_enabled()) {
        nFlags |= DHS_F_V4;
    }

#ifdef HAVE_IPV6
    if (is_dhcsnoopingv6_service_enabled()) {
        nFlags |= DHS_F_V6;
    }
#endif

    if (nFlags == g_nV4V6Flags) {
        return 0;    
    }

    if ((nFlags & DHS_F_V4) && !(g_nV4V6Flags & DHS_F_V4)) {
        init_binding_table();
    } else if (!(nFlags & DHS_F_V4) && (g_nV4V6Flags & DHS_F_V4)) {
        /* modified by liwh for bug 50962, 2019-02-14 
             when service dhcp disable or no dhcp snooping, should delete learning binding entries */
        /*dhcsnooping_binding_execute(0, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_ALL, NULL, 1, NULL);*/
        /* liwh end */
    }
    
    if ((nFlags & DHS_F_V6) && !(g_nV4V6Flags & DHS_F_V6)) {
        init_binding_table6();
    } else if (!(nFlags & DHS_F_V6) && (g_nV4V6Flags & DHS_F_V6)) {
        /* modified by liwh for bug 50962, 2019-02-14 
             when service dhcp disable or no dhcp snooping, should delete learning binding entries */
        /* dhcsnooping_binding_execute(1, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_ALL, NULL, 1, NULL); */
        /* liwh end */
    }
    
    g_nV4V6Flags = nFlags;
    dhcsnooping_process_l3pdu();

    return 0;
}

/* modified by liwh for bug 50994, 2019-02-13 */ 
int dhcsnooping_reserve_service_onoff(unsigned char bOn)
{
    /* modified by liwh for bug 50962, 2019-02-14 
         when service dhcp disable or no dhcp snooping, should delete learning binding entries */
    if (g_snooping_service_on && (!bOn))
    {
        dhcsnooping_binding_execute(0, DHCBINDING_FILTER_ALL, 
            DHCBINDING_TYPE_LEARNING, NULL, 1, NULL);
    }
    /* liwh end */
    
    g_snooping_service_on = bOn;

    return 0;
}
/* liwh end */

/*******************************************************************************
 * Name:    dhcp_snooping_write_file
 * Purpose: save binding database to file
 * Input: 
 *   thread: thread argument
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: for dhcpv6 modified by liuyang 2012-9-21
 ******************************************************************************/
int
dhcp_snooping_write_file(u_int8_t bV4, u_int8_t bV6)
{
    ctclib_slistnode_t *listnode = NULL;   
    tbl_dhcbinding_t *pItem = NULL;
    time_t oppTime = ctc_time_boottime_sec(NULL);
    time_t absTime = time(NULL); 
    time_t leaseTime = 0;
    FILE *pFile = NULL;
    FILE *pFile6 = NULL;
    FILE *pFilePtr = NULL;

    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "save dhcp snooping binding database");
    if (bV4) {
        pFile = sal_fopen(DHCPSNOOPING_SAVE_BINDING_FILE, "w");
        if (NULL == pFile) {
            log_sys(M_MOD_INTERNAL, E_ERROR, "save binding database failed: %s",
                     sal_strerror(errno));
        } else {
            fprintf(pFile, "#\n");
            fprintf(pFile, "# dhcp snooping database writed at %s", ctime(&absTime));
            fprintf(pFile, "#\n");
            fprintf(pFile, "# format is: static(0:dynamic 1:static) + vid + mac + interface + lease(s) + ip\n");
            fprintf(pFile, "#\n\n");
        }
    }

    if (bV6) {
        pFile6 = sal_fopen(DHCPSNOOPING_SAVE_BINDING_FILE6, "w");
        if (NULL == pFile6) {
            log_sys(M_MOD_DHCPSNOOPING6, E_ERROR, "save binding database failed: %s",
                     sal_strerror(errno));
        } else {
            fprintf(pFile6, "#\n");
            fprintf(pFile6, "# dhcpv6 snooping database writed at %s", ctime(&absTime));
            fprintf(pFile6, "#\n");
            fprintf(pFile6, "# format is: static(0:dynamic 1:static) + vid + mac + interface + lease(s) + ipv6\n");
            fprintf(pFile6, "#\n\n");    
        }
    }

    
    if (!pFile && !pFile6) {
        return -1;
    }

    CTCLIB_SLIST_LOOP(DHCSNOOPING_QUEUE, pItem, listnode) {
        u_char szIpBuf[128];

        if (!(DHCBINDING_STATE_RUNNING == pItem->state)) {
            /* unready item */
            continue;
        }

        if (0 != pItem->lease) {
            if ((pItem->lease - oppTime) <= 0) {
                /* expired */
                continue;
            }
            leaseTime = pItem->lease - oppTime + absTime;
        } else {
            leaseTime = 0;
        }

        if (pItem->key.ipv6 && pFile6) {
            pFilePtr = pFile6;  
        } else if (!pItem->key.ipv6 && pFile) {
            pFilePtr = pFile;  
        } else {
            continue;
        }
        /*add by weij, we have to record the binding type*/
        fprintf(pFilePtr, 
                "%1d %-4u %04hx.%04hx.%04hx %10s %-12ld %s\n", 
                pItem->type, pItem->vid,
                sal_ntoh16(*((u_int16_t *)&pItem->key.mac[0])),
                sal_ntoh16(*((u_int16_t *)&pItem->key.mac[2])),
                sal_ntoh16(*((u_int16_t *)&pItem->key.mac[4])),
                pItem->ifname,
                leaseTime, cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip));
    }

    if (pFile) {
        fclose(pFile);        
    }
    if (pFile6) {
        fclose(pFile6);  
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_save_thread
 * Purpose: save binding database timer
 * Input: 
 *   thread: thread argument
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
void
dhcsnooping_save_timer(void *pArg)
{
    tbl_dhcsnooping_t *pCfg = tbl_dhcsnooping_get_dhcsnooping();
    g_p_binding_saver = NULL;    

    if (!is_dhcsnooping_service_enabled()) {
        goto finish_out;
    }

    dhcp_snooping_write_file(1, 0);

finish_out:    
    g_p_binding_saver = ctc_task_add_timer(TASK_PRI_NORMAL, 
            dhcsnooping_save_timer, NULL, pCfg->database_save_interval);
    return;
}

int dhcsnooping_modify_database_saver(u_int16_t nNewInterval)
{
    if (NULL != g_p_binding_saver) {
        ctc_task_delete_timer(g_p_binding_saver);
    }

    g_p_binding_saver = ctc_task_add_timer(TASK_PRI_NORMAL, 
            dhcsnooping_save_timer, NULL, nNewInterval);
    return 0;
}

#define DHCSNOOPING_MAX_TOKEN_CNT      6
/*******************************************************************************
 * Name:    dhcsnooping_load_binding_item
 * Purpose: check and add binding item
 * Input: 
 *   cli: client pointer
 *   ppToks: token array
 *   nToks: token number
 *   nCurLine: current line no
 *   bIPver: 1:ipv6 0:ipv4
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: modified squence in file by liuyang 2012-10-18
 ******************************************************************************/
int
dhcsnooping_load_binding_item(char **ppToks, int nToks, int nCurLine, u_int8_t bIPver,
    char *pszErrMsg, size_t nBufsz)
{
    tbl_dhcbinding_t        stItem;
    tbl_dhcbinding_t        *pItem  = NULL;
    tbl_interface_t         *pIntf  = NULL;
    ctclib_hash_backet_t    *phb    = NULL;
    u_int16_t   nVlanId = 0;
    u_char      szMac[ETH_ALEN] = {'\0'};
    u_int16_t   *psMac = (u_int16_t *)szMac;
    u_int8_t    *pChar;
    time_t nLeaseTime = 0;
    time_t oppTime = ctc_time_boottime_sec(NULL);
    time_t absTime = time(NULL);
    addr_t          suip;
    u_char          szIpBuf[128];
    int             idx = 0;
    int             type = 0;
    int             bIPv6 = 0;
    uint32_t        network = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    sal_memset(&suip, 0x00, sizeof(addr_t));
    /* ppToks[4] is lease field */
    if (sal_strspn(ppToks[4], "0123456789") != strlen(ppToks[4])) {
        sal_snprintf(pszErrMsg, nBufsz,
            "invalid lease time %s", ppToks[4]);
        return -1;
    }
    nLeaseTime = sal_strtou32(ppToks[4], NULL, 10);
    /* 0 for static lease time */
    if (nLeaseTime <= absTime && 0 != nLeaseTime) {
        /* ignore expired item */
        return 0;
    }

    /* ppToks[1] is vlan id field */
    if (sal_strspn(ppToks[1], "0123456789") != strlen(ppToks[1])) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid VLAN Id %s", ppToks[1]);
        return -1;
    }
    nVlanId = sal_atoi(ppToks[1]);
    if (nVlanId > GLB_VLAN_MAX) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid VLAN Id %s", ppToks[1]);
        return -1;
    }

    /* ppToks[2] is mac field */
    if (check_macaddr(ppToks[2]) != 0) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid MAC address %s", ppToks[2]);
        return -1;
    }

    sal_sscanf(ppToks[2], "%4hx.%4hx.%4hx", &psMac[0], &psMac[1], &psMac[2]);
    psMac[0] = sal_hton16(psMac[0]);
    psMac[1] = sal_hton16(psMac[1]);
    psMac[2] = sal_hton16(psMac[2]);

    /* for bug 27702 added by liuyang 2014-4-4*/
    if (MAC_IS_BROADCAST(szMac)
        || MAC_IS_UNSPECIFIED(szMac)
        || MAC_IS_MULTICAST(szMac)) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid MAC address %s", ppToks[2]);
        return -1;     
    }
    
    /* ppToks[5] is ip address field */
    if (cdb_addr_str2val(&suip, ppToks[5], strlen(ppToks[5])) != 0) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid IP address %s", ppToks[5]);
        return -1;
    }
    /* for bug 27702 added by liuyang 2014-4-4*/
    if (suip.family == AF_INET) {
        network = sal_ntoh32(suip.u.prefix4.s_addr);
        if (IN_BADCLASS(network)
            || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network)
            || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            sal_snprintf(pszErrMsg, nBufsz, "invalid IP address %s", ppToks[5]);
            return -1;
        }
    }
    /* ppToks[3] is interface, We not check interface name */
    if (strlen(ppToks[3]) > INTERFACE_NAMSIZ) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid interface %s", ppToks[3]);
        return -1;
    }

    /* for bug 21493 added intf check by liuyang 2012-12-25*/
    pIntf = tbl_interface_get_interface_by_name(ppToks[3]);
    if (NULL == pIntf) {
        sal_snprintf(pszErrMsg, nBufsz,
            "can't found interface %s", ppToks[3]);
        return -1;
    }
    if (sal_strncasecmp(ppToks[3], "eth-", 4) && sal_strncasecmp(ppToks[3], "agg", 3)) {
        sal_snprintf(pszErrMsg, nBufsz,
            "Invalid interface name: %s", ppToks[3]);
        return -1;
    }
    if (GLB_IF_MODE_L2 != pIntf->mode) {
        sal_snprintf(pszErrMsg, nBufsz, 
            "Only allowed on L2 interface");
        return -1;        
    }
    if (INTERFACE_IS_LAG_MEMBER(pIntf)) {
        sal_snprintf(pszErrMsg, nBufsz, 
            "Cannot configure on LAG member port");
        return -1;  
    }
    
    /* 
     * add by weij, we have to record the binding type
     * ppToks[5] is type of field 
     */
    if (sal_strspn(ppToks[0], "0123456789") != strlen(ppToks[0])) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid type %s", ppToks[0]);
        return -1;
    }
    type = sal_atoi(ppToks[0]);
    if ((DHCBINDING_TYPE_LEARNING != type) && (DHCBINDING_TYPE_CONFIGED != type)) {
        sal_snprintf(pszErrMsg, nBufsz, "invalid type %s", ppToks[0]);
        return -1;
    }

    if (suip.family == AF_INET6) {
        bIPv6 = 1;
    }

    if (bIPv6 != bIPver) {
        return 0;
    }
    /* check duplication item */
    for (idx = 0; idx < DHCSNOOPING_HASH->size; idx++) {
        for (phb = DHCSNOOPING_HASH->index[idx]; phb; phb = phb->next) {
            pItem = (tbl_dhcbinding_t *)phb->data;
            if (NULL == pItem) {
                continue;
            }
            if (pItem->key.ipv6 != bIPv6) {
                continue;
            }
            /*for bug 4203*/
            if ((!memcmp(szMac, pItem->key.mac, sizeof(szMac))) 
                    && (pItem->vid == nVlanId)) {
                /*modified by weij for bug 16138, 2011-09-02*/
                if (!memcmp(&pItem->client_ip, &suip, sizeof(addr_t))) {
                    if (DHCBINDING_TYPE_CONFIGED == type) {
                        if (pItem->vid == nVlanId) {
                            if (!strncmp((char *)pItem->ifname, ppToks[3], IFNAME_SIZE)) {
                                return 0;
                            }
                        }
                    }
                }
            
                sal_snprintf(pszErrMsg, nBufsz,
                    "MAC address %s and Vlan %s exist", ppToks[2], ppToks[1]);
                return -1;
            }
            /* for bug 20441 compare ip + vlan modified by liuyang 2012-9-6*/
            if (!memcmp(&pItem->client_ip, &suip, sizeof(addr_t))
                && (pItem->vid == nVlanId)) {
                sal_snprintf(pszErrMsg, nBufsz,
                    "IP address %s and Vlan %s exist", ppToks[5], ppToks[1]);
                return -1;
            }
        } /* for (phb */
    } /* for (idx */

#ifdef HAVE_IPV6
    if ((!bIPv6 && DHCSNOOPING_CUR_BINDINGS >= DHCSNOOPING_MAX_BINDING_ITEMS)
        || (bIPv6 && DHCSNOOPING6_CUR_BINDINGS >= DHCSNOOPINGV6_MAX_BINDING_ITEMS))
#else
     if ((!bIPv6 && DHCSNOOPING_CUR_BINDINGS >= DHCSNOOPING_MAX_BINDING_ITEMS))   
#endif
    {
        sal_snprintf(pszErrMsg, nBufsz,
            "The DHCP%s snooping binding entry num is more than maximum %u!", bIPv6 ? "v6" : "",
            bIPv6 ? DHCSNOOPINGV6_MAX_BINDING_ITEMS : DHCSNOOPING_MAX_BINDING_ITEMS);
        return -1;
    }

    pItem = &stItem;
    memset(pItem, 0x00, sizeof(tbl_dhcbinding_t));
    memcpy(pItem->key.mac, szMac, ETH_ALEN);
    pItem->type = type;
    pItem->state = DHCBINDING_STATE_RUNNING;
    pItem->vid = nVlanId;
    pItem->key.vid = nVlanId;
    pItem->key.ipv6 = bIPv6;
    if (0 == nLeaseTime) {
        pItem->lease = 0;
    } else {
        pItem->lease = nLeaseTime - absTime + oppTime;
    }
    memcpy(&pItem->client_ip, &suip, sizeof(addr_t));
    sal_snprintf(pItem->ifname, IFNAME_SIZE + 1, ppToks[3]);

    pChar = pItem->key.mac;
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT,
              "load binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
              "%s %s lease %d", pItem->vid,
              pChar[0], pChar[1], pChar[2], pChar[3], pChar[4], pChar[5],
              cdb_addr_val2str((char*)szIpBuf, 128, &pItem->client_ip),
              IFNAME_ETH2FULL(pItem->ifname, ifname_ext), (nLeaseTime ? (pItem->lease - oppTime) : 0));

    dhcsnooping_binding_hash_add(DHCSNOOPING_HASH, DHCSNOOPING_HASH_IP, pItem);
    return 0;
}

/*******************************************************************************
 * Name:    dhcsnooping_reload_database
 * Purpose: load binding item from file
 * Input: 
 *   cli:   client pointer 
 *   pFile: file handle
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcsnooping_reload_database(FILE *pFile, u_int8_t bIPv6,
    int bSyslog, char *pszErrMsg, size_t nErrBufsz)
{
#define DHCSNOOPING_LINE_BUF           256
    char szBuf[DHCSNOOPING_LINE_BUF];
    char *p = NULL;
    char *q = NULL;
    u_long nCnt = 0;
    char **ppToks = NULL;
    int nRet = 0;
    int nCurLine = 0;
    int nErrCnt = 0;
    char szErrMsg[256];
    int pmid = bIPv6 ? M_MOD_DHCPSNOOPING6 : M_MOD_DHCPSNOOPING;
    
    if (NULL == pFile) {
        return -1;
    }

    while ((nErrCnt < 10)
            && NULL != (sal_fgets(szBuf, DHCSNOOPING_LINE_BUF, pFile))) {
        nCurLine++;

        p = szBuf;
        /* trim the last \n & \r */
        if ('\n' == p[strlen(p) - 1]) {
            p[strlen(p) - 1] = '\0';
        }
        if ('\r' == p[strlen(p) - 1]) {
            p[strlen(p) - 1] = '\0';
        }
        
        /* trim space */
        while ('\0' != *p && ' ' == *p) {
            p++;
        }

        /* space line */
        if ('\0' == *p || '#' == *p) {
            continue;
        }

        /* trim # comment */
        if ((q = sal_strchr(p, '#')) != NULL) {
            *q = '\0';
        }
        if (strlen(p) <= 0) {
            continue;
        }

        if (split_tokens(p, strlen(p), DHCSNOOPING_MAX_TOKEN_CNT, 
                         " \t", &nCnt, &ppToks) != 0) {
            nRet = -1;
            
            sal_snprintf(pszErrMsg, nErrBufsz, 
                "Can't load binding item for invalid line %d: %s",
                nCurLine, p);
            if (bSyslog) {
                log_sys(pmid, E_ERROR, "%s", pszErrMsg); 
            }
            nErrCnt++;
            continue;
        }       

        if (DHCSNOOPING_MAX_TOKEN_CNT != nCnt) {
            nRet = -1;
            sal_snprintf(pszErrMsg, nErrBufsz, 
                "Can't load binding item for invalid line %d: %s",
                nCurLine, p);
            if (bSyslog) {
                log_sys(pmid, E_ERROR, "%s", pszErrMsg); 
            }
            free_tokens(&ppToks);
            nErrCnt++;
            continue;
        }

        if (dhcsnooping_load_binding_item(ppToks, nCnt, nCurLine, 
                bIPv6, szErrMsg, 256) != 0) {
            sal_snprintf(pszErrMsg, nErrBufsz, 
                "Can't load binding item: %s at line %d",
                szErrMsg, nCurLine);
            if (bSyslog) {
                log_sys(pmid, E_ERROR, "%s", pszErrMsg); 
            }
            nErrCnt++;
            nRet = -1;
        }
        free_tokens(&ppToks);
    }
    
    if (nErrCnt >= 10) {
        sal_snprintf(pszErrMsg, nErrBufsz, 
            "Too many errors, ignore at line %d", nCurLine);
        if (bSyslog) {
            log_sys(pmid, E_ERROR, "%s", pszErrMsg); 
        }
    }
    return nRet;
}

/*******************************************************************************
 * Name:    init_binding_table
 * Purpose: load dhcp snooping binding table at system startup
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int init_binding_table()
{
    FILE *fp = NULL;
    int ret = 0;
    char szErrMsg[256];
 
    if (NULL == (fp = sal_fopen(DHCPSNOOPING_SAVE_BINDING_FILE, "ro"))) {
        if (ENOENT == errno) {
            /* no such file */
            return 0;
        }

        log_sys(M_MOD_INTERNAL, E_ERROR, 
                "Load dhcp snooping binding item failed: can't open file: %s",
                sal_strerror(errno));
        return -1;
    }


    ret = dhcsnooping_reload_database(fp, 0, 1, szErrMsg, 256);
    fclose(fp);
    return ret;
}

/*modified by liuyang 2012-8-22*/
int dhcsnooping_process_dhcp_pkt_msg(void *pArg)
{
#if 0
    nsm_msg_dhcp_pkt_t *pmsg = message;
    tbl_dhcbinding_t *pItem = NULL;
    tbl_dhcbinding_t *pItem_req = NULL;
    tbl_dhcbinding_t *pItem_run = NULL;
    tbl_dhcbinding_t stItem;
    pal_time_t nCurtime = pal_time_boottime_sec(NULL);
    pal_time_t absTime = pal_time_sys_current(NULL);
    u_int32_t time_get = 0;
    u_char szIpBuf[128];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    memset (&stItem, 0x00, sizeof(tbl_dhcbinding_t));
    stItem.bIPv6 = (pmsg->clientip_len == 4) ? 0 : 1;
    if (!stItem.key.ipv6) {
        if (!is_dhcsnooping_service_enabled()) {
            return 0;
        }
        
        stItem.client_ip.family = AF_INET;
        memcpy(&(stItem.client_ip.u.prefix4.s_addr), pmsg->clientip, pmsg->clientip_len);
    }
#ifdef HAVE_IPV6
    else
    { /*not support now*/
        if (!is_dhcsnoopingv6_service_enabled())
        {
            return 0;
        }
        
        stItem.suClientIp.sin6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
        stItem.suClientIp.->sin6.sin6_len = sizeof(struct pal_sockaddr_in6);
#endif /* SIN6_LEN */
        memcpy(&stItem.suClientIp.sin6.sin6_addr, pmsg->clientip, 16);
        return 0;
    }
#endif
    stItem.ulTransId = pmsg->xid;
    stItem.vid = pmsg->vid;
    stItem.cCmpMethod = DHCSNOOPING_CMPMETHOD_MAC_VLAN;
    memcpy(&stItem.szClientMac, pmsg->mac, pmsg->mac_len);

    stItem.cState = DHCSNOOPING_ITEM_STATE_RUNNING;
    pItem_run= hash_lookup(DHCSNOOPING_HASH, &stItem);
    stItem.cState = DHCSNOOPING_ITEM_STATE_REQUEST;
    pItem_req= hash_lookup(DHCSNOOPING_HASH, &stItem);

    if (!pItem_req && !pItem_run)
    {
        if (!stItem.bIPv6)
        {
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_ERR,
                    "can't add binding item for locate request information (MAC + trans id) faild");            
        }
#ifdef HAVE_IPV6
        else
        {
            DEBUG_OUT(dhcpv6, snooping, DHCSNOOPING6_E_ERR,
                    "can't add v6 binding item for locate request information (MAC + trans id) faild");
        }
#endif
        return 0;
    }

    if (pItem_run && DHCSNOOPING_ITEM_TYPE_CONFIGED == pItem_run->cType)
    {
    /* we does not overwrite configed item */
        return 0;
    }
    
    switch (pmsg->pktType)
    {
        case DHCP_PKT_NAK:
        case DHCPV6_PKT_REPLY_FAILED:
            if (pItem_run && stItem.ulTransId == pItem_run->ulTransId)
            {   /* even if ip equal 0 ?*/
                pItem_run->cCmpMethod = DHCSNOOPING_CMPMETHOD_IP_VLAN;
                pItem_run->cCmpMethod |= DHCSNOOPING_CMPMETHOD_MAC_VLAN;

                /* zongyi add for ip source guard */
                dhcsnooping_del_ipsour_item(pItem_run);
                dhcsnooping_binding_hash_del(pItem_run);
            }

            if (pItem_req && stItem.ulTransId == pItem_req->ulTransId)
            {
                pItem_req->cCmpMethod |= DHCSNOOPING_CMPMETHOD_MAC_VLAN;

                /* zongyi add for ip source guard */
                dhcsnooping_binding_hash_del(pItem_req);
            }
            return 0;
        case DHCP_PKT_ACK:
        case DHCPV6_PKT_REPLY_SUCCESS:
            if (pItem_req && stItem.ulTransId == pItem_req->ulTransId)
            {
                /*delete old binding*/
                if (pItem_run)
                {
                    pItem_run->cCmpMethod = DHCSNOOPING_CMPMETHOD_IP_VLAN;
                    pItem_run->cCmpMethod |= DHCSNOOPING_CMPMETHOD_MAC_VLAN;
                    
                    dhcsnooping_del_ipsour_item(pItem_run);
                    dhcsnooping_binding_hash_del(pItem_run);      
                }

                 /* Added by tanker for bug9270 2009-08-20 */
                tbl_dhcbinding_t *tmppItem;

                stItem.cState = DHCSNOOPING_ITEM_STATE_RUNNING;        
                stItem.cCmpMethod = DHCSNOOPING_CMPMETHOD_IP_VLAN;/*added by ychen in 2011-02-21 for ipv6 merge*/
                tmppItem = hash_lookup(DHCSNOOPING_HASH_IP, &stItem);
                if(tmppItem)
                {
                    if(DHCSNOOPING_ITEM_TYPE_CONFIGED == tmppItem->cType)
                        return 0;

                    dhcsnooping_del_ipsour_item(tmppItem);
                    dhcsnooping_binding_hash_del(tmppItem);
                }
                /* End Added by tanker for bug9270 2009-08-20 */
        
                /* change to running state */
                pItem_req->cState = DHCSNOOPING_ITEM_STATE_RUNNING;
                memcpy(&pItem_req->suClientIp, &stItem.suClientIp, sizeof (addr_t));
                time_get = pmsg->lease_time;
                if ((time_get & ULONG_MAX) == ULONG_MAX)
                {
                    pItem_req->nLeaseTime = 0;
                }
                else if ((time_get > LONG_MAX) || ((time_get + absTime) > LONG_MAX))
                {
                    pItem_req->nLeaseTime = LONG_MAX;
                }
                else if (!time_get)
                {
                    pItem_req->nLeaseTime = DHCSNOOPING_DEFAULT_LEASE_TIME + nCurtime;
                }
                else
                {
                    pItem_req->nLeaseTime = time_get + nCurtime;
                }
                /* beacuse the ip of the item is not exist, 
                    we can't add item to DHCSNOOPING_HASH_IP 2007-12-4 
                 */
                pItem_req->cCmpMethod = DHCSNOOPING_CMPMETHOD_IP_VLAN;
                dhcsnooping_binding_hash_add(NULL, DHCSNOOPING_HASH_IP, pItem_req);
                if (!pItem_req->bIPv6)
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENTS,
                            "add binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d", pItem_req->vid,
                            pItem_req->szClientMac[0], pItem_req->szClientMac[1],
                            pItem_req->szClientMac[2], pItem_req->szClientMac[3],
                            pItem_req->szClientMac[4], pItem_req->szClientMac[5],
                            sockunion2str(&pItem_req->suClientIp, (char*)szIpBuf, 128),
                            IFNAME_ETH2FULL(pItem_req->ifname, ifname_ext), abs(pItem_req->nLeaseTime - nCurtime));
                }
#ifdef HAVE_IPV6
                else
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                            "add v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d", pItem_req->vid,
                            pItem_req->szClientMac[0], pItem_req->szClientMac[1],
                            pItem_req->szClientMac[2], pItem_req->szClientMac[3],
                            pItem_req->szClientMac[4], pItem_req->szClientMac[5],
                            sockunion2str(&pItem_req->suClientIp, (char*)szIpBuf, 128),
                            IFNAME_ETH2FULL(pItem_req->ifname, ifname_ext), abs(pItem_req->nLeaseTime - nCurtime));
                }
#endif
                /* zongyi modifies for ip source guard*/
                dhcsnooping_add_ipsour_item(pItem_req); 
                pItem = pItem_req;
            }
            else if (pItem_run && stItem.ulTransId == pItem_run->ulTransId)
            {
                /* for running state item, just update it lease time , no need to readd ipsg entry*/
                time_get = pmsg->lease_time;
                if ((time_get & ULONG_MAX) == ULONG_MAX)
                {
                    pItem_run->nLeaseTime = 0;
                }
                else if ((time_get > LONG_MAX) || ((time_get + absTime) > LONG_MAX))
                {
                    pItem_run->nLeaseTime = LONG_MAX;
                }
                else
                {
                    pItem_run->nLeaseTime = time_get + nCurtime;
                }
                
                if (memcpy(&pItem_run->suClientIp, &stItem.suClientIp, sizeof(addr_t))
                    ||DHCSNOOPING_ITEM_NOT_ADD_IPSOUR == pItem_run->cIpsour)
                {
                    dhcsnooping_del_ipsour_item(pItem_run);
                    memcpy(&pItem_run->suClientIp, &stItem.suClientIp, sizeof(addr_t));
                    dhcsnooping_add_ipsour_item(pItem_run);        
                }
                pItem = pItem_run;
                
                if (!pItem_run->bIPv6)
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENTS,
                            "update binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d", pItem_run->vid,
                            pItem_run->szClientMac[0], pItem_run->szClientMac[1],
                            pItem_run->szClientMac[2], pItem_run->szClientMac[3],
                            pItem_run->szClientMac[4], pItem_run->szClientMac[5],
                            sockunion2str(&pItem_run->suClientIp, (char*)szIpBuf, 128),
                            IFNAME_ETH2FULL(pItem_run->ifname, ifname_ext), abs(pItem_run->nLeaseTime - nCurtime));
                }
#ifdef HAVE_IPV6
                else
                {
                    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS,
                            "update v6 binding item vid %u %02x:%02x:%02x:%02x:%02x:%02x "
                            "%s %s lease %d", pItem_run->vid,
                            pItem_run->szClientMac[0], pItem_run->szClientMac[1],
                            pItem_run->szClientMac[2], pItem_run->szClientMac[3],
                            pItem_run->szClientMac[4], pItem_run->szClientMac[5],
                            sockunion2str(&pItem_run->suClientIp, (char*)szIpBuf, 128),
                            IFNAME_ETH2FULL(pItem_run->ifname, ifname_ext), abs(pItem_run->nLeaseTime - nCurtime));
                }
#endif
            }
            else
            {
                return 0;
            }

            dhcsnooping_binding_queue_update(pItem);
            break;
        default:
            log_sys(M_MOD_DHCSNOOPING, E_ERROR, 
                "Unknow packet type from relay/server :%d\n", pmsg->pktType);
    }
#endif
   return 0;    
}

/*for bug 19748 added by liuyang delete entry for interface 2012-7-4*/
void dhcsnooping_if_delete_cb(char *pszIntfName)
{
    tbl_dhcbinding_t    *pItem      = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    ctclib_slistnode_t  *nextnode   = NULL;
    u_int8_t bRun = 0;
    u_int8_t bReq = 0;
    
    CTCLIB_SLIST_LOOP_DEL(DHCSNOOPING_QUEUE, pItem, listnode, nextnode) {
        /*not this interface, continue*/
        if (sal_strcasecmp(pszIntfName, pItem->ifname)) {
            continue;
        }

        if (DHCBINDING_STATE_REQUEST == pItem->state) {
            bReq = 1;
        } else {
            bRun = 1;
        } 

        if (bRun) {   /* even if ip equal 0 ?*/
            dhcsnooping_binding_hash_del(pItem);
        }

        if (bReq) {
            tbl_dhcbinding_del_dhcbinding(&pItem->key);
        }
    }    
}

void 
dhcsnooping_dump_packet_option(struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    memset (&ds, 0, sizeof ds);

    if (u != &dhcp_universe) {
        name = u->name;
        dot = ".";
    } else {
        name = "";
        dot = "";
    }
    if (evaluate_option_cache (&ds, packet, lease, client,
                   in_options, cfg_options, scope, oc, MDL)) {
        DEBUG_SHOW(dhcp_debug, flags, "  option %s%s%s %s;",
                   name, dot, oc->option->name,
                   pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
        data_string_forget (&ds, MDL);
    }
    return;
}

/*******************************************************************************
 * Name:    dhcsnooping_dump_packet 
 * Purpose: dump packet fields
 * Input: 
 *   tp: packet
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
dhcsnooping_dump_packet (struct packet *tp)
{
    struct dhcp_packet *tdp = NULL;
    int i;
    
    if (NULL == tp || !(IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET))) {
        return;
    }
    
    tdp = tp->raw;
    DEBUG_SHOW(dhcp_debug, flags, "dump DHCP packet fields");
    DEBUG_SHOW(dhcp_debug, flags, "  packet length %d", tp->packet_length);
    DEBUG_SHOW(dhcp_debug, flags, "  op = %d  htype = %d  hlen = %d  hops = %d",
              tdp->op, tdp->htype, tdp->hlen, tdp->hops);
    DEBUG_SHOW(dhcp_debug, flags, "  xid = 0x%x  secs = %ld  flags = 0x%x",
              sal_ntoh32(tdp->xid), (unsigned long)tdp->secs, sal_ntoh16(tdp->flags));
    DEBUG_SHOW(dhcp_debug, flags, "  ciaddr = %s", inet_ntoa (tdp->ciaddr));
    DEBUG_SHOW(dhcp_debug, flags, "  yiaddr = %s", inet_ntoa (tdp->yiaddr));
    DEBUG_SHOW(dhcp_debug, flags, "  siaddr = %s", inet_ntoa (tdp->siaddr));
    DEBUG_SHOW(dhcp_debug, flags, "  giaddr = %s", inet_ntoa (tdp->giaddr));
    DEBUG_SHOW(dhcp_debug, flags, "  chaddr = %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
              ((unsigned char *)(tdp->chaddr)) [0],
              ((unsigned char *)(tdp->chaddr)) [1],
              ((unsigned char *)(tdp->chaddr)) [2],
              ((unsigned char *)(tdp->chaddr)) [3],
              ((unsigned char *)(tdp->chaddr)) [4],
              ((unsigned char *)(tdp->chaddr)) [5]);
    DEBUG_SHOW(dhcp_debug, flags, "  filename = %s", tdp->file);
    DEBUG_SHOW(dhcp_debug, flags, "  server_name = %s", tdp->sname);
    if (!tp->options_valid) {
        return;
    }

    for (i = 0; i < tp->options->universe_count; i++)  {
        if (!tp->options->universes[i]) {
            continue;
        }
        option_space_foreach(tp, (struct lease *)0,
                             (struct client_state *)0,
                             (struct option_state *)0,
                             tp->options,
                             &global_scope,
                             universes [i], 0,
                             dhcsnooping_dump_packet_option);
    }

    return;
}

int 
dhcsnooping_macfilter(S_DhcpSnoopingInHdr *pHdr, DhcSnoopingCMsg_t *pcmsg)
{
	tbl_macfilter_master_t 	*p_master  = tbl_macfilter_get_master();
	ctclib_slistnode_t 		*listnode  = NULL;
    ctclib_slistnode_t 		*next = NULL;
    tbl_macfilter_t 		*p_db_macflt = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->macflt_list, p_db_macflt, listnode, next)
    {
        if(0 == sal_memcmp(pcmsg->src_mac, p_db_macflt->key.mac, sizeof(mac_addr_t)))
        {
			return PM_E_EXIST;
		}
    }

	return 0;
}

/* modified by liwh for bug 50994, 2019-02-14 */
int 
dhcsnooping_vlan_snooping_enable(uint16 vid)
{
    tbl_vlan_t      *p_db_vlan = NULL;
    tbl_vlan_key_t  vlan_key;
	
    sal_memset(&vlan_key, 0 ,sizeof(vlan_key));    
    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return FALSE;
    }
		
    return p_db_vlan->dhcsnooping;
}
/* liwh end */

int32
dhcsnooping_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t     *p_db_if = NULL;
    S_DhcpSnoopingInHdr stHdr;
    S_DhcpSnoopingInHdr *pHdr   = &stHdr;
    DhcSnoopingCMsg_t   cmsg;
    uint32  ifindex = 0;
    int     nReadLen = 0;
    int     nRet = 0;
    struct ip *iph = NULL;

    /* modified by liwh for bug 50994, 2019-02-13 */ 
    if (!g_snooping_service_on)
    {
        return 0;
    }
    /* liwh end */
    
    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if) {
        return PM_E_FAIL;
    }

    /* modified by liwh for bug 50994, 2019-02-14 */
    if (!dhcsnooping_vlan_snooping_enable(p_msg->u_hdr.pkt.vid))
    {
        return 0;    
    }
    /* liwh end */
    
    sal_memcpy(&cmsg.dest_mac, (char *)p_msg->data, ETH_ALEN);
    sal_memcpy(&cmsg.src_mac, (char *)p_msg->data + ETH_ALEN, ETH_ALEN);
    cmsg.ifindex = ifindex;
    cmsg.vlanid = p_msg->u_hdr.pkt.vid;
    cmsg.type = p_msg->u_hdr.pkt.type;
    
    nReadLen = p_msg->data_len - ETH_HEADER_LEN; 
    pHdr->uiph = p_msg->data + ETH_HEADER_LEN; /* skip L2 header */
    pHdr->udp = NULL;
    pHdr->dhcp = NULL;
    pHdr->nTotalLen = nReadLen;
    pHdr->bIPv6 = (IP_V(pHdr->uiph) == 6);

    if (0 == pHdr->bIPv6) {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT, 
            "receive one DHCP message");
        DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP,
            "dump the DHCP message (receive)", (u_char *)(pHdr->uiph), nReadLen, 0);
    }
#ifdef HAVE_IPV6
    /* forward ipv6 packet */
    else {
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS, 
            "receive one DHCPv6 message");
        DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_DUMP,
            "dump the message (receive)", (u_char *)(pHdr->uiph), nReadLen, 0);
        
        if (dhcsnoopingv6_get_one(pHdr, &cmsg) != 0) {
            return PM_E_FAIL;
        }
    }
#endif

    /* modified by liwh for bug 40761, 2016-09-13 */
    /* Check the IP packet length. */
    iph  = pHdr->uiph;
    if (sal_ntohs(iph->ip_len) != nReadLen) {
        nReadLen = sal_ntohs(iph->ip_len);
    }
    /* liwh end */

    /* validate this packet */
    if (dhcsnooping_validate_pkt(pHdr, nReadLen) != 0) {  
        return PM_E_FAIL;
    }

	if (dhcsnooping_macfilter(pHdr, &cmsg) != 0) {  
        return PM_E_FAIL;
    }

    /* process this packet */
    nRet = dhcsnooping_process_pkt(pHdr, &cmsg);
    if (-1 == nRet) {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}


