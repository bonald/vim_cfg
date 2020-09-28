/****************************************************************************
* $Id: dhcrelay_func.c,v 1.8 2007/09/03 04:13:09 xliu Exp $
*  process functions for dhcp relay
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Alexander Liu
* Date          : 2007-01-09 18:35
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <signal.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "proto.h"
#include "dhcpd.h"
#include "dhcp_version.h"

#include "ctclib_hash.h"
#include "ctc_msg.h"
#include "dhcrelay_func.h"
#include "dhcp_relayagent.h"
#ifdef HAVE_BM            
#include "bmlib.h"
#endif
#include "genlog.h"
#include "glb_const.h"
#include "ctc_task.h"
#include "ctc_sock.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_vlan_define.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"
#include "glb_ip_define.h"
#include "lib_fs.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

DEBUG_DECLAR(dhcp_debug);

#ifndef VRF_ID_DEFAULT
#define VRF_ID_DEFAULT   0
#endif

#ifndef NSM_IF_TYPE_L2
#define NSM_IF_TYPE_L2        1
#endif

typedef void (*HASH_CLEAN_FUNC) (void *);

typedef struct {
    struct interface_info *pInfo;
    struct packet *packet;
    struct dhcp_packet *raw;
    size_t len;
    struct in_addr from;
    int *pnErrCnt;
    int *pnRelayedCnt;
} send_br_arg_t;

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
/* dhcp service switch */
int g_dhcprelay_trustall = 0x00;

static int g_relay_service_on = 0;
    
extern u_int16_t local_port;
extern omapi_io_object_t omapi_io_states;
extern int timeval_cmp (struct timeval a, struct timeval b);
struct interface_info *dhcprelay_register_interface(char *name);
void dhcprelay_unregister_interface(char *name);
struct interface_info *dhcprelay_locate_interface(char *name);
extern int is_dhcp_relay_enabled();
/* statistics */
extern int bad_circuit_id;
extern int bogus_giaddr_drops;
extern int client_packet_errors;
extern int client_packets_relayed;
extern int corrupt_agent_options;
extern int missing_agent_option;
extern int missing_circuit_id;
extern int server_packet_errors;
extern int server_packets_relayed;
void dhcprelay_gen_agent_option(struct interface_info *pInfo);

void reset_dhcrelay_statistics()
{
    client_packets_relayed = 0;
    server_packets_relayed = 0;
    client_packet_errors = 0;
    server_packet_errors = 0;
    bogus_giaddr_drops = 0;
    bad_circuit_id = 0;
    corrupt_agent_options = 0;
    missing_agent_option = 0;
    missing_circuit_id = 0;
    return;
}

void output_dhcrelay_statistics(FILE *fp)
{
    fprintf(fp, "/client_packets_relayed=%d", client_packets_relayed);
    fprintf(fp, "/server_packets_relayed=%d", server_packets_relayed);
    fprintf(fp, "/client_packet_errors=%d", client_packet_errors);
    fprintf(fp, "/server_packet_errors=%d", server_packet_errors);
    fprintf(fp, "/bogus_giaddr_drops=%d", bogus_giaddr_drops);
    fprintf(fp, "/bad_circuit_id=%d", bad_circuit_id);
    fprintf(fp, "/corrupt_agent_options=%d", corrupt_agent_options);
    fprintf(fp, "/missing_agent_option=%d", missing_agent_option);
    fprintf(fp, "/missing_circuit_id=%d\r\n", missing_circuit_id);
    return;
}

/****************************************************************************
 *
* Function
* 
****************************************************************************/

void dhcprelay_stop(void)
{
    return;
}

/*******************************************************************************
 * Name:    sighup 
 * Purpose: SIGHUP handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sighup (int sig)
{
    log_sys(M_MOD_DHCPRLY, E_INFORMATIONAL, "SIGHUP received");
}

/*******************************************************************************
 * Name:    sigint 
 * Purpose: SIGINT handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sigint (int sig)
{
    log_sys(M_MOD_DHCPRLY, E_INFORMATIONAL, "SIGINT received");
    
    dhcprelay_stop ();
    exit(0);
}

/*******************************************************************************
 * Name:    sigusr1 
 * Purpose: SIGUSR1 handler
 * Input: 
 *   sig: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void sigusr1 (int sig)
{
}

/*******************************************************************************
 * Name:    signal_init 
 * Purpose: Initialize signal handles
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay_signal_init ()
{
    signal(SIGHUP, sighup);
    signal(SIGINT, sigint);
    signal(SIGTERM, sigint);
    signal(SIGUSR1, sigusr1);
}

/*******************************************************************************
 * Name:    dhcprelay_dump_packet_option
 * Purpose: dump dhcp packet option for debug
 * Input: 
 *   oc:
 *   packet: dhcp packet
 *   lease:
 *   client:
 *   in_options:
 *   cfg_options:
 *   scope: 
 *   u: 
 *   foo: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
dhcprelay_dump_packet_option(struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    memset (&ds, 0, sizeof ds);

    if (u != &dhcp_universe) 
    {
        name = u->name;
        dot = ".";
    }
    else 
    {
        name = "";
        dot = "";
    }
    if (evaluate_option_cache (&ds, packet, lease, client,
                   in_options, cfg_options, scope, oc, MDL)) 
    {
        DEBUG_SHOW(dhcp_debug, flags, "  option %s%s%s %s;",
                   name, dot, oc->option->name,
                   pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
        data_string_forget (&ds, MDL);
    }
    return;
}

/*******************************************************************************
 * Name:    dhcprelay_dump_packet 
 * Purpose: dump packet fields
 * Input: 
 *   tp: packet
 *   len: packet length
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
dhcprelay_dump_packet (struct dhcp_packet *tdp, int len)
{
    struct packet stTP;
    struct packet *tp = &stTP;
    int i;
    
    if (NULL == tdp || !(IS_DEBUG_ON(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_PACKET))) {
        return;
    }
    
    memset(&stTP, 0x00, sizeof(struct packet));
    tp->raw = tdp;
    tp->packet_length = len;

    if (!parse_options(tp))
    {
        if (tp->options)
        {
            option_state_dereference(&tp->options, MDL);
            packet_dereference(&tp, MDL);
        }
        /* add by liul for bug43846, if option field abnormal in dhcp pkt, we will return */
        return;
    }
    
    DEBUG_SHOW(dhcp_debug, flags, "dump DHCP packet fields");
    DEBUG_SHOW(dhcp_debug, flags, "  packet length %d", tp->packet_length);
    DEBUG_SHOW(dhcp_debug, flags, "  op = %d  htype = %d  hlen = %d  hops = %d",
              tdp->op, tdp->htype, tdp->hlen, tdp->hops);
    DEBUG_SHOW(dhcp_debug, flags, "  xid = 0x%x  secs = %ld  flags = 0x%x",
              ntohl(tdp->xid), (unsigned long)tdp->secs, ntohs(tdp->flags));
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
    if (!tp->options_valid) 
    {
        return;
    }

    for (i = 0; i < tp->options->universe_count; i++) 
    {
        if (!tp->options->universes[i]) 
        {
            continue;
        }
        option_space_foreach(tp, (struct lease *)0,
                             (struct client_state *)0,
                             (struct option_state *)0,
                             tp->options,
                             &global_scope,
                             universes [i], 0,
                             dhcprelay_dump_packet_option);
    }

    if (tp->options)
    {
        option_state_dereference(&tp->options, MDL);
    }
    return;
}

static int32
_dhcrelay_service_intf_onoff(tbl_route_if_t *p_intf, tbl_iter_args_t *pargs)
{
    int onoff = *((int *)pargs);

    if (NULL == p_intf) {
        return -1;
    }

    if (0 == onoff) {
        dhcprelay_unregister_interface(p_intf->key.name);
        return 0;
    }
  
    if (dhcprelay_locate_interface(p_intf->key.name) == NULL) {
        dhcprelay_register_interface(p_intf->key.name);
    }
    return 0;
}

static int32
dhcrelay_service_intf_onoff(int onoff)
{
    tbl_route_if_iterate((TBL_ITER_CB_FUNC)_dhcrelay_service_intf_onoff, (tbl_iter_args_t *)(&onoff));
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay_service_onoff
 * Purpose: process service on/off
 * Input: 
 *   bOn: On or off flag
 * Output: 
 * Return:
 *   success: 0
 *   failed:  -1
 * Note: 
 ******************************************************************************/
int dhcprelay_service_onoff(unsigned char bOn)
{
    if (bOn) {
        if (g_relay_service_on) {
            return 0;
        }

        /* service on */
        g_relay_service_on = 1;
        dhcrelay_service_intf_onoff(1);
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS,
                  "initialize DHCP relay interfaces success");
        log_sys(M_MOD_DHCPRLY, E_INFORMATIONAL, "turn DHCP relay function on");
        return 0;
    }

    /* service off */
    if (!g_relay_service_on) {
        return 0;
    }
    g_relay_service_on = 0;
    dhcrelay_service_intf_onoff(0);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS,
              "deinitialize DHCP relay interfaces success");
    log_sys(M_MOD_DHCPRLY, E_INFORMATIONAL, "turn DHCP relay function off");
    return 0;
}

int
dhcprelay_recv_interface_add(char *pszIntfName, int isLayer3)
{
    /* is it L3 interface?
     * for bug 21603 modified by liuyang 2012-12-8
     */
    if (!isLayer3) {
        return 0;
    }

    if (is_dhcp_relay_enabled()) {
        if (NULL == dhcprelay_locate_interface(pszIntfName)) {
            dhcprelay_register_interface(pszIntfName);
        }
    }
    return 0;
}

int
dhcprelay_recv_interface_delete(char *pszIntfName)
{
    dhcprelay_unregister_interface(pszIntfName);
    return 0;
}

int dhcprelay_recv_trust_all_interface(int bTrustAll)
{
    g_dhcprelay_trustall = bTrustAll;
    return 0;
}

int dhcprelay_recv_intftype_change(char *pszIntfName, int isLayer3)
{
    dhcprelay_unregister_interface(pszIntfName);
    if (!isLayer3) {
        return 0;
    }

    /* interface change to L3, register interface if need */
    if (is_dhcp_relay_enabled()) {
        dhcprelay_register_interface(pszIntfName);
    }
    
    return 0;
}

int
dhcprelay_recv_ipaddr_change(char *pszIntfName, prefix_t   *pAddress)
{
    struct interface_info *pInfo = NULL;

    if ((pInfo = dhcprelay_locate_interface(pszIntfName)) != NULL) {
        dhcprelay_update_primary_address(pInfo, (uint32*)&pAddress->u.prefix4);
    }
    return 0;
}

/* modified by liwh for bug 43886, 2018-11-06 */
int
dhcprelay_recv_ipaddr_delete(char *pszIntfName)
{
    struct interface_info *pInfo = NULL;

    if ((pInfo = dhcprelay_locate_interface(pszIntfName)) != NULL) {
        dhcprelay_delete_primary_address(pInfo);
    }
    return 0;
}
/* liwh end */

int dhcprelay_recv_select_group(char *pszIntfName, int nSrvGroupIdx)
{
#if 0
    if (0 == nSrvGroupIdx) {
      	dhcprelay_unregister_interface(pszIntfName);
        return 0;
    }

    if (is_dhcp_relay_enabled()) {
        if (NULL == dhcprelay_locate_interface(pszIntfName) {
            dhcprelay_register_interface(pszIntfName);
        }
    }
#endif
    return 0;
}

int32
dhcrelay_process_intf_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    if (NULL != p_if_key) {
        p_db_if = tbl_interface_get_interface(p_if_key);
    }
    
    switch (oper) {
    case CDB_OPER_ADD:
        if (NULL == p_db_if) {
            rc = PM_E_FAIL;
            SAL_ASSERT(0);
            break;
        }
        if (GLB_IF_MODE_L3 != p_db_if->mode) {
            break;
        }

        dhcprelay_recv_interface_add(p_db_if->key.name, (GLB_IF_MODE_L3 == p_db_if->mode));
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_if) {
            rc = PM_E_FAIL;
            SAL_ASSERT(0);
            break;
        }
        if (TBL_INTERFACE_FLD_MODE == field_id) {
            dhcprelay_recv_intftype_change(p_db_if->key.name, (GLB_IF_MODE_L3 == p_db_if->mode));
        }
        break;

    case CDB_OPER_DEL:
        if (p_ds_node) {
            /* delete sub DS */
            break;
        }
        if (p_if_key && '\0' != p_if_key->name[0]) {
            dhcprelay_recv_interface_delete(p_if_key->name);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

ds_connected_t *
rt_if_get_primary_connected(tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ds_connected_t      *p_db_conn  = NULL;

    if (!p_db_rtif)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            return p_db_conn;
        }
    }

    return NULL;
}

int32
dhcrelay_process_route_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_route_if_key_t *p_if_key = NULL;
    tbl_route_if_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    ds_connected_t *ds_conn = NULL;
    /* modified by liwh for bug 50993, 2019-02-15 */
    struct interface_info *pInfo = NULL;
    /* liwh end */

    p_if_key = (tbl_route_if_key_t*)p_tbl;
    if (NULL != p_if_key) {
        p_db_if = tbl_route_if_get_route_if(p_if_key);
        if (!p_db_if)
        {
             return rc;    
        }
    }

    ds_conn = rt_if_get_primary_connected(p_db_if);
    
    switch (oper) {
    case CDB_OPER_ADD:
        if (!ds_conn) {
            return rc;     
        }
        
        if (p_db_if) {
            dhcprelay_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
        }
        break;

    case CDB_OPER_SET:
        /* modified by liwh for bug 50993, 2019-02-15 */
        if (p_db_if && TBL_ROUTE_IF_FLD_DHCP_SERVER_GROUP == field_id) {
             pInfo = dhcprelay_locate_interface(p_db_if->key.name);
            if (pInfo)
            {
                pInfo->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
                pInfo->hw_address.hbuf[0] = ARPHRD_ETHER;
                sal_memcpy(&pInfo->hw_address.hbuf[1], p_db_if->mac, GLB_ETH_ADDR_LEN);
                dhcprelay_gen_agent_option(pInfo);
            }
        }
        /* liwh end */
        
        if (!ds_conn) {
            return rc;     
        }
        
        if (p_db_if && TBL_ROUTE_IF_FLD_IFC_IPV4 == field_id) {
            dhcprelay_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
            break;
        }
        break;

    case CDB_OPER_DEL:
        if (!ds_conn) {
            dhcprelay_recv_ipaddr_delete(p_db_if->key.name);    
        }
        else
        {
            if (p_db_if && TBL_ROUTE_IF_FLD_IFC_IPV4 == field_id) {
                dhcprelay_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
            }
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}


/*******************************************************************************
 * Name:    init_fallback_interface
 * Purpose: initialize fallback interface for dhcp relay
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void init_fallback_interface(void)
{
    maybe_setup_fallback();    
#if defined (HAVE_SETFD)
    if (fallback_interface) 
    {
        if (fcntl (fallback_interface -> rfdesc, F_SETFD, 1) < 0)
        {
            log_error ("Can't set close-on-exec on fallback: %m");
        }
        if (fallback_interface -> rfdesc != fallback_interface -> wfdesc) 
        {
            if (fcntl (fallback_interface -> wfdesc, F_SETFD, 1) < 0)
            {
                log_error ("Can't set close-on-exec on fallback: %m");
            }
        }
    }
#endif
    return;
}

/*modified by xgu for bug 14721, 2011-04-26*/
/*******************************************************************************
 * Name:    dhcprelay_update_if_vrfid
 * Purpose: update vrfid of this interface
 * Input: 
 *   pInfo: interface info struct 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay_update_if_vrfid(struct interface_info *pInfo)
{
    tbl_interface_key_t stKey;
    tbl_interface_t *pItem;
    
    if (NULL == pInfo) {
        return -1;
    }

    memset(&stKey, 0x00, sizeof(tbl_interface_key_t));
    snprintf(stKey.name, IFNAME_SIZE, (char *)pInfo->name);
    pItem = tbl_interface_get_interface(&stKey);
    if(NULL == pItem) {
        pInfo->vrfid = VRF_ID_DEFAULT;
        return 0;
    }

    /* XXX: Please Modify it to support VRF */
    /* pInfo->vrfid = pItem->vrfid */
    pInfo->vrfid = VRF_ID_DEFAULT;

    /* keep up ifindex for send packet process */
    pInfo->ifindex = pItem->ifindex;
    return 0;
}

int sock_set_bindtofib(int sock, int vrf)
{
#ifndef SO_PERMVRF
#define SO_PERMVRF      101
#endif
    setsockopt(sock, SOL_SOCKET, SO_PERMVRF, (u_int8_t *)&vrf, sizeof(vrf));
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay_update_primary_address
 * Purpose: update primary address
 * Input: 
 *   pInfo: interface info struct 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: modified by liuyang for interface_info changed 2012-7-26
 ******************************************************************************/
int dhcprelay_update_primary_address(struct interface_info *pInfo, uint32 *addr)
{
    struct ifreq ifr;
    int sock = -1;
    
    if (NULL == pInfo)
    {
        return -1;
    }

    if (pInfo->addresses == NULL) 
    {
        pInfo->addresses = dmalloc(sizeof(struct in_addr), MDL);
        if (pInfo->addresses == NULL) 
        {
            log_sys(M_MOD_DHCPRLY, E_ERROR, "Out of memory saving IPv4 address "
                                       "on interface.");
                return -1;
        }
        pInfo->address_count = 0;
        pInfo->address_max = 4;
    } 
    memset (pInfo->addresses, 0 ,sizeof (struct in_addr));

    /* modified by liwh for bug 43080, 2018-10-10 */
    if (addr)
    {
        pInfo->address_count = 1;
        memcpy(&pInfo->addresses[0], addr, sizeof(uint32));
        return 0;
    }
    /* liwh end */

/* modified by liwh for bug 43886, 2018-11-06 
    addresses should be configured from route_if */
    if (!addr)
    {
        /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
        if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            log_fatal ("Can't create addrlist socket");
            return -1;
        }   
    
        /*modified by xgu for bug 14721, 2011-04-26*/
        sock_set_bindtofib (sock, pInfo->vrfid);
        /* Read the hardware address from this interface. */    
        snprintf(ifr.ifr_name, IFNAMSIZ, pInfo->name);
       
        if (ioctl (sock, SIOCGIFADDR, &ifr) < 0)
        {
            close(sock);
            return -1;
        }
        close(sock);
    
         
        memcpy(&pInfo->addresses[0], &ifr.ifr_addr.sa_data[2], sizeof(struct in_addr));
        pInfo->address_count = 1;
    }
//#endif
/* liwh end */

    return 0;
}

/* modified by liwh for bug 43886, 2018-11-06 */
#ifdef _CENTEC_
int dhcprelay_delete_primary_address(struct interface_info *pInfo)
{
    if (pInfo->addresses)
    {
        dfree(pInfo->addresses, MDL);
        pInfo->addresses = NULL;
        pInfo->address_count = 0;
    }

    return 0;
}
#endif
/* liwh end */
/*******************************************************************************
 * Name:    dhcprelay_init_hardware_address
 * Purpose: get hardware address for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay_init_hardware_address(struct interface_info *tmp)
{
    /* modified by liwh for bug 50993, 2019-02-27 */
    tbl_route_if_t*     p_db_if = NULL;
    tbl_route_if_key_t     if_key;       
    
    if (NULL == tmp) {
        return -1;
    }

    memset(&if_key, 0, sizeof(tbl_route_if_key_t));
    strcpy(if_key.name, tmp->name);

    p_db_if = tbl_route_if_get_route_if(&if_key);
    if (p_db_if)
    {
        tmp->hw_address.hlen = GLB_ETH_ADDR_LEN + 1;
        tmp->hw_address.hbuf[0] = ARPHRD_ETHER;
        sal_memcpy(&tmp->hw_address.hbuf[1], p_db_if->mac, GLB_ETH_ADDR_LEN);
    }
    /* liwh end */

    struct ifreq ifr;
    struct sockaddr sa;
    struct ifreq *tif = NULL;
    int sock = -1;

    if (NULL == tmp) {
        return -1;
    }

    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        log_fatal ("Can't create addrlist socket");
        return -1;
    }   

    if (!tmp -> ifp) 
    {
        /* Make up an ifreq structure. */
        tif = (struct ifreq *)dmalloc (sizeof (struct ifreq), MDL);
        if (!tif)
        {
            close(sock);
            log_fatal ("no space to remember ifp.");
            return -1;
        }
        
        memset (tif, 0, sizeof (struct ifreq));
        strcpy (tif -> ifr_name, tmp -> name);
        tmp -> ifp = tif;
    }

    /*modified by xgu for bug 14721, 2011-04-26*/
    sock_set_bindtofib (sock, tmp->vrfid);
    /* Read the hardware address from this interface. */
    ifr = *tmp -> ifp;
    if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(sock);
#if 0
        log_error("get hardware address for interface %s failed: %s",
                  tmp->name, strerror(errno));
#endif
        return -1;
    }

#if 0
    sa = *(struct sockaddr *)&ifr.ifr_hwaddr;
    switch (sa.sa_family) 
    {
#ifdef HAVE_ARPHRD_TUNNEL
    case ARPHRD_TUNNEL:
        /* ignore tunnel interfaces. */
#endif
#ifdef HAVE_ARPHRD_ROSE
    case ARPHRD_ROSE:
#endif
#ifdef HAVE_ARPHRD_IRDA
    case ARPHRD_IRDA:
        /* ignore infrared interfaces. */
#endif
#ifdef HAVE_ARPHRD_SIT
    case ARPHRD_SIT:
        /* ignore IPv6-in-IPv4 interfaces. */
#endif
#ifdef HAVE_ARPHRD_IEEE1394
    case ARPHRD_IEEE1394:
        /* ignore IEEE1394 interfaces. */
#endif
#ifdef HAVE_ARPHRD_LOOPBACK
    case ARPHRD_LOOPBACK:
        /* ignore loopback interface */
        break;
#endif
    case ARPHRD_ETHER:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_ETHER;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;

#ifndef HAVE_ARPHRD_IEEE802
# define ARPHRD_IEEE802 HTYPE_IEEE802
#endif
#if defined (HAVE_ARPHRD_IEEE802_TR)
    case ARPHRD_IEEE802_TR:
#endif
    case ARPHRD_IEEE802:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_IEEE802;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;

#ifndef HAVE_ARPHRD_FDDI
# define ARPHRD_FDDI HTYPE_FDDI
#endif
    case ARPHRD_FDDI:
        tmp -> hw_address.hlen = 17;
        tmp -> hw_address.hbuf [0] = HTYPE_FDDI; /* XXX */
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 16);
        break;

#ifdef HAVE_ARPHRD_METRICOM
    case ARPHRD_METRICOM:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_METRICOM;
        memcpy (&tmp -> hw_address.hbuf [0], sa.sa_data, 6);
        break;
#endif

#ifdef HAVE_ARPHRD_AX25
    case ARPHRD_AX25:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_AX25;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;
#endif

#ifdef HAVE_ARPHRD_NETROM
    case ARPHRD_NETROM:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_NETROM;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        break;
#endif
    default:
        log_error("%s: unknown hardware address type %d",
                  ifr.ifr_name, sa.sa_family);
        break;
    }
#endif

    close(sock);
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay_gen_agent_option
 * Purpose: generate circuit id and remote id for agent, 
 *   limitations:
 
 * Input: 
 *   pInfo: interface info pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay_gen_agent_option(struct interface_info *pInfo)
{
    S_DhcpRelayCircuitId *pstCircuitId = NULL;
    S_DhcpRelayRemoteId *pstRemoteId = NULL;
    u_int16_t nVlanId = 0xFFFF;
    u_int32_t cModule = 0xFF;
    u_int32_t cPort = 0xFF;

    /*added by chenc for bug 52447, 2019-06-27*/
#ifndef GREATBELT
    u_int32_t csubPort = 0xFF;
#endif
    /*chenc end*/
    
    if (NULL == pInfo)
    {
        return;
    }

    pstCircuitId = calloc(1, sizeof(S_DhcpRelayCircuitId));
    if (NULL == pstCircuitId)
    {
        return;
    }

    pstRemoteId = calloc(1, sizeof(S_DhcpRelayRemoteId));
    if (NULL == pstRemoteId)
    {
        free(pstCircuitId);
        return;
    }

    if (NULL != pInfo->remote_id)
    {
        /* we already generated this item, let's free it */
        free(pInfo->remote_id);
        free(pInfo->circuit_id);
        pInfo->circuit_id = (u_int8_t *)pInfo->name;
        pInfo->circuit_id_len = strlen(pInfo->name);
        pInfo->remote_id = NULL;
        pInfo->remote_id_len = 0;
    }

    /* getting informations from interface name */
#define DHCPRELAY_VLAN_PREFIX   "vlan"
#define DHCPRELAY_INTF_PREFIX   "eth-"
    if (strncasecmp(pInfo->name, DHCPRELAY_VLAN_PREFIX, 
                         strlen(DHCPRELAY_VLAN_PREFIX)) == 0)
    {
        nVlanId = atoi(&pInfo->name[strlen(DHCPRELAY_VLAN_PREFIX)]);
    }
    else if (strncasecmp(pInfo->name, DHCPRELAY_INTF_PREFIX,
                              strlen(DHCPRELAY_INTF_PREFIX)) == 0)
    {
        /* for eth-X-Y */
        /*modified by chenc for bug 52447, 2019-06-27*/
        #ifndef GREATBELT
        sscanf(pInfo->name, "eth-%u-%u_%u", &cModule, &cPort, &csubPort);
        #else
        sscanf(pInfo->name, "eth-%u-%u", &cModule, &cPort);
        #endif
        /*chenc end*/
    }

    /* generate agent options:
     * 1) Circuit Id Format: 
     *   Circuit ID Type + Length + VLAN + Module + Port
     *   1 byte + 1 byte + 2 bytes + 1 byte + 1 byte
     * 2) Remote Id Format: 
     *   Remote ID Type + Length + Mac Address
     *   1 byte + 1 byte + 6 bytes
     */
    pstCircuitId->cType = 0;
    pstCircuitId->cLength = sizeof(S_DhcpRelayCircuitId) - 2;
    pstCircuitId->nVlanId = htons(nVlanId);
    pstCircuitId->cModule = (cModule & 0xFF);
    pstCircuitId->cPort = (cPort & 0xFF);
    
    /*added by chenc for bug 52447, 2019-06-27*/
#ifndef GREATBELT
    if (0xff == csubPort)
    {
        csubPort = 0;
    }
    pstCircuitId->cSubPort = (csubPort & 0xFF);
#endif
    /*chenc end*/
    
    pInfo->circuit_id = (u_int8_t *)pstCircuitId;
    pInfo->circuit_id_len = sizeof(S_DhcpRelayCircuitId);
    
    pstRemoteId->cType = 0;
    pstRemoteId->cLength = sizeof(S_DhcpRelayRemoteId) - 2;
    memcpy(pstRemoteId->nMacArr, &pInfo->hw_address.hbuf[1], ETH_ALEN);
    pInfo->remote_id = (u_int8_t *)pstRemoteId;
    pInfo->remote_id_len = sizeof(S_DhcpRelayRemoteId);

    return;
}

/*******************************************************************************
 * Name:    dhcprelay_register_interface
 * Purpose: register interface to dhcp relay interface list
 * Input: 
 *   name:  interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcprelay_register_interface(char *name)
{
    struct interface_info *tmp = (struct interface_info *)0;
    isc_result_t status = ISC_R_SUCCESS;
	char  ifname_ext[IFNAME_EXT_SIZE];
    
    status = interface_allocate (&tmp, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_fatal("%s: interface_allocate: %s", 
                  name, isc_result_totext (status));
        return NULL;
    }
    
    strcpy (tmp->name, name);
    tmp->index = -1;
    interface_snorf(tmp, INTERFACE_REQUESTED);
    /*modified by xgu for bug 14721, 2011-04-26*/
    dhcprelay_update_if_vrfid(tmp);
    dhcprelay_init_hardware_address(tmp);
    dhcprelay_gen_agent_option(tmp);
    dhcprelay_update_primary_address(tmp, NULL);

#ifndef _CENTEC_  /* rx/tx pkt from hsrv directly, not pass through kernel in OSP, so wo don't register socket to kernel */
    /* Register the interface... */
    if_register_receive (tmp);
    if_register_send (tmp);
#endif

    /* interface_stash (tmp); merge bug 19775 by liuyang 2012-7-30 */
    
    /* Now register the interfaces as protocols. */
    if (tmp->rfdesc != -1)
    {
        status = omapi_register_io_object((omapi_object_t *)tmp,
                                          if_readsocket, 0, got_one, 0, 0);
        if (status != ISC_R_SUCCESS)
        {
        	IFNAME_ETH2FULL(tmp->name, ifname_ext);
            log_fatal("Can't register I/O handle for %s: %s",
                      ifname_ext, isc_result_totext(status));
        }
    }
    
    interface_dereference(&tmp, MDL);
    return tmp;
}

/*******************************************************************************
 * Name:    dhcprelay_locate_interface
 * Purpose: locate interface 
 * Input: 
 *   name: interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcprelay_locate_interface(char *name)
{
    struct interface_info *ip = NULL;

	for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (!strcmp(ip->name, name))
        {
            return ip;
        }
	}
    
	return NULL;
}

/*******************************************************************************
 * Name:    dhcprelay_unregister_interface
 * Purpose: unregister interface from dhcp relay interface list
 * Input: 
 *   name: interface name
 * Output: 
 * Return: N/A
 * Note: merge bug 19775 by liuyang 2012-7-30
 ******************************************************************************/
void dhcprelay_unregister_interface(char *name)
{
    struct interface_info *interface = NULL;
    struct interface_info *ip = NULL;
    struct interface_info *last = NULL;

    /* remove from interfaces */
    last = 0;
    for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (!strcmp(ip->name, name))
        {
            if (NULL != ip->remote_id)
            {
                free(ip->remote_id);
                ip->remote_id = NULL;
                ip->remote_id_len = 0;
                free(ip->circuit_id);
                ip->circuit_id = (u_int8_t *)ip->name;
                ip->circuit_id_len = strlen(ip->name);
            }
            interface_reference (&interface, ip, MDL);
            if (last) 
            {
                interface_dereference (&last -> next, MDL);
                if (ip -> next)
                {
                    interface_reference (&last -> next, ip -> next, MDL);
                }
            } 
            else 
            {
                interface_dereference (&interfaces, MDL);
                if (ip -> next)
                {
                    interface_reference (&interfaces, ip -> next, MDL);
                }
            }
            if (ip -> next)
            {
                interface_dereference (&ip -> next, MDL);
            }
            break;
        }

        last = ip;
    }
    
    if (!ip)
    {
        /* not found */
        return;
    }
#if 0
	/* add the interface to the dummy_interface list */
	if (dummy_interfaces) 
    {
		interface_reference (&interface -> next, dummy_interfaces, MDL);
		interface_dereference (&dummy_interfaces, MDL);
	}
	interface_reference (&dummy_interfaces, interface, MDL);

	/* do a DHCPRELEASE */
	if (dhcp_interface_shutdown_hook)
    {
        (*dhcp_interface_shutdown_hook) (interface);
    }
#endif
    /* remove the io object */
    omapi_unregister_io_object ((omapi_object_t *)interface);
    if_deregister_send (interface);
    if_deregister_receive (interface);
    interface_dereference(&interface, MDL);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay_locate_servergroup
 * Purpose: locate dhcp-server group item, create if not found
 * Input:  
 *   nGroup: group index
 * Output: 
 * Return: 
 *   success: group pointer
 *   failed : NULL for out of memory
 * Note: 
 ******************************************************************************/
tbl_dhcsrvgrp_t *dhcprelay_locate_servergroup(unsigned long nGroup)
{
    tbl_dhcsrvgrp_key_t stKey;

    /* group index in cdb is zero based
     * group index in CLI is one based
     */
    if (nGroup > GLB_MAX_DHCSRVGRP_NUM || nGroup < 1) {
        return NULL;
    }

    memset(&stKey, 0x00, sizeof(tbl_dhcsrvgrp_key_t));
    stKey.id = (nGroup - 1);
    return tbl_dhcsrvgrp_get_dhcsrvgrp(&stKey);
}

static int 
_send_bootrequest(int idx, int vrf, gensock_t *psu, void *pArg)
{
    send_br_arg_t *pBrArg = (send_br_arg_t *)pArg;
    struct sockaddr_in to;
    struct hardware *hto = NULL;
     
    /* only support ipv4 at present */
    to = *((struct sockaddr_in *)psu);
    to.sin_port = local_port;

    if (send_packet (pBrArg->pInfo, pBrArg->packet, pBrArg->raw, 
        pBrArg->len, pBrArg->from, &to, hto) < 0) {
        if (NULL != pBrArg->pnErrCnt) {
            (*(pBrArg->pnErrCnt))++;
            DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_ERROR,
          "forwarded BOOTREQUEST for %s to %s failed",
           print_hw_addr(pBrArg->raw->htype, pBrArg->raw->hlen, pBrArg->raw->chaddr),
          inet_ntoa (to.sin_addr));
        }
        return -1;
    }
    
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS,
              "forwarded BOOTREQUEST for %s to %s success",
              print_hw_addr(pBrArg->raw->htype, pBrArg->raw->hlen, pBrArg->raw->chaddr),
              inet_ntoa(to.sin_addr));
    if (NULL != pBrArg->pnRelayedCnt) {
        (*(pBrArg->pnRelayedCnt))++;
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay_send_bootrequest
 * Purpose: send boot request to server
 * Input: 
 *   pInfo: interface info
 *   packet: packet
 *   raw: raw packet
 *   len: pakcet size
 *   from: primary address for this interface
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcprelay_send_bootrequest(struct interface_info *pInfo,
                                struct interface_info *pRawInfo,
                                struct packet *packet, struct dhcp_packet *raw,
                                size_t len, struct in_addr from,
                                int *pnErrCnt, int *pnRelayedCnt)
{
    tbl_route_if_key_t stKey;
    tbl_route_if_t *pItem; 
    tbl_dhcsrvgrp_t *pGroup;
    send_br_arg_t brArg;
	char  ifname_ext[IFNAME_EXT_SIZE];
    
    if (NULL == pInfo || NULL == raw || NULL == pRawInfo) {
        return;
    }

	IFNAME_ETH2FULL(pRawInfo->name, ifname_ext);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS,
              "send BOOTREQUEST for interface %s from %s", 
              ifname_ext, print_hw_addr(raw->htype, raw->hlen, raw->chaddr));

    /* locate server group */
    memset(&stKey, 0x00, sizeof(tbl_interface_key_t));
    snprintf(stKey.name, IFNAME_SIZE, (char *)pRawInfo->name);
    if ((pItem = tbl_route_if_get_route_if(&stKey)) == NULL) {
        log_sys(M_MOD_DHCPRLY, E_ERROR, "Can't locate interface control block for %s",
                 ifname_ext);
        return;
    }

    if (!pItem->dhcp_server_group) {   
        /* for bug 19709 modified by liuyang 2012-6-28*/
        DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS,
                "please setting dhcp-server group for interface %s",
                ifname_ext);
        return;
    }

    pGroup = dhcprelay_locate_servergroup(pItem->dhcp_server_group);
    if (NULL == pGroup) {
        return;
    }

    if (pGroup->cnt <= 0) {
        log_sys(M_MOD_DHCPRLY, E_ERROR, "please setting server address for dhcp-server group %u",
                 pGroup->key.id + 1);
        return;
    }

    /* send boot request to all servers */
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "send one DHCP message");
    dhcprelay_dump_packet(raw, len);
    DEBUG_OUT_HEX(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_DUMP,
                  "dump the BOOTREQUEST message (send)",
                  (u_char *)raw, len, 0);
    brArg.pInfo = pInfo;
    brArg.packet = packet;
    brArg.raw = raw;
    brArg.len = len;
    brArg.from = from;
    brArg.pnErrCnt = pnErrCnt;
    brArg.pnRelayedCnt = pnRelayedCnt;
    dhcsrvgrp_addr_iter(pGroup->addrs, _send_bootrequest, &brArg);
    return;
}

/*******************************************************************************
 * Name:    dhcprelay_trusted_interface
 * Purpose: check to see is this interface trusted?
 * Input: 
 *   pInfo: 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcprelay_trusted_interface(struct interface_info *pInfo)
{
    tbl_route_if_key_t stKey;
    tbl_route_if_t *pItem;
    
    if (NULL == pInfo) {
        return -1;
    }

    memset(&stKey, 0x00, sizeof(tbl_interface_key_t));
    snprintf(stKey.name, IFNAME_SIZE, (char *)pInfo->name);
    if ((pItem = tbl_route_if_get_route_if(&stKey)) == NULL) {
        return -1;
    }

    if (g_dhcprelay_trustall || pItem->dhcp_relay_option_trust)
    {
        return 0;
    }

    /* this packet not being trusted */
    return -1;
}

/* added by liuyang 2012-8-21*/
#if 0
int dhcprelay_encode_pkt_msg(nsm_msg_dhcp_pkt_t *pmsg, u_int8_t **buf, u_int16_t *size,
    int (*pencode_func)(u_int8_t **, u_int16_t *, nsm_msg_dhcp_pkt_t *))
{
    int nbytes;

    if (NULL == pmsg || NULL == buf || NULL == size)
    {
        return -1;
    }

    assert(pmsg->pktType);
    assert(NULL != pmsg->clientip);
    assert(NULL != pmsg->mac);

    memset(&pmsg->cindex, 0x00, sizeof(cindex_t));
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_PKT_TYPE);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_CLIENT_IP);
    NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_CLIENT_MAC);
    if (pmsg->vid)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_INTERFACE_VID);
    }
    if (pmsg->vrfid)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_INTERFACE_VRFID);
    }
    if (pmsg->ifindex)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_INTERFACE_INDEX);
    }

    if (NULL != pmsg->ifname)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_INTERFACE_NAME);
    }

    if (pmsg->lease_time)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_LEASE_TIME);
    }
    if (pmsg->xid)
    {
        NSM_SET_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_DHCPD_TRANS_ID);
    }

    nbytes = pencode_func(buf, size, pmsg);
    if (nbytes < 0)
    {
        return -1;
    }

    return nbytes;
}
#endif

/* only for vlanif added by liuyang 2012-8-21*/
int dhcprelay_send_msg_to_snooping(struct dhcp_packet *packet, int length,
                                                                struct interface_info *ifp)
{
#if 0
    nsm_msg_dhcp_pkt_t msg;
    struct option_cache *oc;
    struct nsm_client *nc = ZG->nc;
    struct nsm_client_handler *nch;
    u_int32_t msg_id;
    u_char *pnt;
    u_int16_t size;
    int nbytes;
    struct option_cache *op;
    struct packet *decoded_packet;

    nch = nc->async;
    if (! nch || ! nch->up)
    {
        return -1;
    }
    memset(&msg, 0, sizeof msg);


    decoded_packet = (struct packet *)0;
    if (!packet_allocate (&decoded_packet, MDL))
    {
        log_error ("do_packet: no memory for packet!");
        return -1;
    }
    decoded_packet -> raw = packet;
    decoded_packet -> packet_length = length;   

	/* If there's an option buffer, try to parse it. */
    if (!parse_options (decoded_packet))
    {
        if (decoded_packet -> options)
        {
            option_state_dereference
                (&decoded_packet -> options, MDL);
            packet_dereference (&decoded_packet, MDL);
            return -1;
        }
    }

    if (decoded_packet -> options_valid &&
        (op = lookup_option (&dhcp_universe,
            decoded_packet -> options, 
            DHO_DHCP_MESSAGE_TYPE)))
    {
        struct data_string dp;
        memset (&dp, 0, sizeof dp);
        evaluate_option_cache (&dp, decoded_packet,
                (struct lease *)0,
                (struct client_state *)0,
                decoded_packet -> options,
                (struct option_state *)0,
                (struct binding_scope **)0,
                op, MDL);
        if (dp.len > 0)
            decoded_packet -> packet_type = dp.data [0];
        else
            decoded_packet -> packet_type = 0;
        data_string_forget (&dp, MDL);
    }
  
    if (DHCPACK == decoded_packet -> packet_type)
    {
        msg.pktType = DHCP_PKT_ACK;        
    }
    else if (DHCPNAK == decoded_packet -> packet_type)
    {
        msg.pktType = DHCP_PKT_NAK;  
    }
    else
    {
        packet_dereference (&decoded_packet, MDL);
        return 0;
    }

    msg.xid = packet->xid;
    msg.vid = atoi(&ifp->name[4]); /* vlanxx */
    msg.clientip_len = sizeof (struct in_addr) ;
    msg.clientip = (u_char *)malloc(1, msg.clientip_len);
    memcpy (msg.clientip, &packet->yiaddr, msg.clientip_len);

    msg.mac_len = packet->hlen;
    msg.mac = (u_int8_t*)malloc(1, msg.mac_len);
    memcpy (msg.mac, packet->chaddr, msg.mac_len);

    /* get from option*/
    if ((oc = lookup_option (&dhcp_universe, decoded_packet->options,
            DHO_DHCP_LEASE_TIME) )!= NULL)
    {
        struct data_string dp;
        memset (&dp, 0, sizeof dp);
        evaluate_option_cache (&dp, decoded_packet, (struct lease *)0, (struct client_state *)0,
               decoded_packet -> options, (struct option_state *)0,
               (struct binding_scope **)0, oc, MDL);
        if (dp.len > 3)
        {
            msg.lease_time = getULong (dp.data);;
        }
        else
        {
            msg.lease_time = 0;
        }
        data_string_forget (&dp, MDL);
    } 
    else
    {
        goto err_out;
    }
    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    nbytes = dhcprelay_encode_pkt_msg(&msg, &pnt, &size, nsm_encode_dhcp_pkt_msg);
    if (nbytes < 0)
    {
        goto err_out;
    }

    nsm_client_send_message(nch, 0, 0,
            NSM_MSG_DHCP_PKT_SEND, nbytes, &msg_id);

err_out:
    if (msg.clientip)
    {
        free(msg.clientip);
    }

    if (msg.mac)
    {
        free(msg.mac);
    }

    packet_dereference (&decoded_packet, MDL);
#endif
    return 0;        
}

int32
dhcrelay_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    int result;
    uint32  ifindex = 0;
    struct sockaddr_in from;
    struct hardware hfrom;
    struct iaddr ifrom;
    tbl_interface_t     *p_db_if = NULL;
    tbl_interface_t     *p_db_vlan_if = NULL;
    struct interface_info *ip = NULL;
    tbl_interface_key_t if_key;
    uint16      vlanid = 0;
    char        name[IFNAME_SIZE];

    union {
    	unsigned char packbuf [10240]; /* Packet input buffer.
    				 	 Must be as large as largest
    					 possible MTU. */
    	struct dhcp_packet packet;
    } u;

    /* modified by liwh for bug 50994, 2019-02-13 */
    if (!g_relay_service_on)
    {
         return 0;   
    }
    /* liwh end */

    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if) {
        return PM_E_FAIL;
    }

    /* modified by chenc for bug 52107, 2019-05-22 */
    uint32 is_running = FALSE;
    is_running = tbl_interface_is_running(p_db_if);
    if (!is_running)
    {
        return PM_E_FAIL;
    }
    /* chenc end */

    memcpy(name, p_db_if->key.name, IFNAME_SIZE);

    /* receive from vlan interface ? */
    vlanid = p_msg->u_hdr.pkt.vid;
    if (0 != vlanid) {

        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_VLAN_PREFIX"%u", vlanid);
        p_db_vlan_if = tbl_interface_get_interface(&if_key);
        if (NULL  == p_db_vlan_if)
        {
            return 0;
        }

        /* modified by chenc for bug 52107, 2019-05-22 */
        is_running = FALSE;
        is_running = tbl_interface_is_running(p_db_vlan_if);
        if (!is_running)
        {
            return PM_E_FAIL;
        }
        /* chenc end */

        memcpy(name, p_db_vlan_if->key.name, IFNAME_SIZE);
    }

    for (ip = interfaces; ip; ip = ip->next) {
    	if (!strcmp(ip->name, name))
    		break;
    }

    /* this port is not enable dhcrelay now, return */
    if (ip == NULL) {
        return PM_E_NONE;
    }

    /* it should a omapi_interface type, not omapi_io or other type */
    if (ip-> type != dhcp_type_interface)
    	return DHCP_R_INVALIDARG;

    /* parse pakcet bootstrap header, put it to u.packbuf */
      again:
	if ((result =
	     dhcrelay_receive_packet (ip, u.packbuf, sizeof u, &from, &hfrom, (unsigned char *)p_msg->data, p_msg->data_len)) < 0) {
		return ISC_R_UNEXPECTED;
	}
	if (result == 0)
		return ISC_R_UNEXPECTED;

	/*
	 * If we didn't at least get the fixed portion of the BOOTP
	 * packet, drop the packet.
	 * Previously we allowed packets with no sname or filename
	 * as we were aware of at least one client that did.  But
	 * a bug caused short packets to not work and nobody has
	 * complained, it seems rational to tighten up that
	 * restriction.
	 */
	if (result < DHCP_FIXED_NON_UDP)
		return ISC_R_UNEXPECTED;
//todo:check use_v4_pktinfo
#if defined(IP_PKTINFO) && defined(IP_RECVPKTINFO) && defined(USE_V4_PKTINFO)
	{
		/* We retrieve the ifindex from the unused hfrom variable */
		unsigned int ifindex;

		memcpy(&ifindex, hfrom.hbuf, sizeof (ifindex));

		/*
		 * Seek forward from the first interface to find the matching
		 * source interface by interface index.
		 */
		ip = interfaces;
		while ((ip != NULL) && (if_nametoindex(ip->name) != ifindex))
			ip = ip->next;
		if (ip == NULL)
			return ISC_R_NOTFOUND;
	}
#endif

	if (bootp_packet_handler) {
		ifrom.len = 4;
		memcpy (ifrom.iabuf, &from.sin_addr, ifrom.len);

		(*bootp_packet_handler) (ip, &u.packet, (unsigned)result,
					 from.sin_port, ifrom, &hfrom);
	}
    
	/* If there is buffered data, read again.    This is for, e.g.,
	   bpf, which may return two packets at once. */
	if (ip->rbuf_offset != ip->rbuf_len)
		goto again;

    return PM_E_NONE;
}
