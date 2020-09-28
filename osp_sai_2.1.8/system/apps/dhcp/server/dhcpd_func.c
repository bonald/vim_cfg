/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "dhcpd.h"

#include <ifaddrs.h>

#include "sal.h"
//#include "pal.h"
#include "proto.h"
#include "lib.h"
/*#include "thread.h"
#include "log.h"
#include "cli.h"
#include "hash.h"
#include "nsm_client.h"
#include "nsm_message.h"*/
#include "dhcpd_func.h"
#ifdef _CENTEC_
#ifdef HAVE_BM            
#include "bmlib.h"
#endif
#endif       

#include "glb_debug_define.h"
#include "genlog.h"
#include "lib_fs.h"

#include "proto_module.h"
#include <assert.h>
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

#include "ctclib_hash.h"

DEBUG_DECLAR(dhcp_debug);

#define MTYPE_TMP 0

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/

#undef DHCPD_DEBUG 

#ifndef NSM_IF_TYPE_L2
#define NSM_IF_TYPE_L2        1
#endif

typedef void (*HASH_CLEAN_FUNC) (void *);
extern unsigned int g_nOrigIfIdx;

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
/* dhcp service switch */
int g_dhcpserverSwitch = 0x00;
int g_dhcpserver_initialized = 0; /* dhcp server initialized flag */
struct hash *g_dhcpserverIntfhash = NULL; /* only hold L3 interfaces which is act as dhcp server */
ctclib_hash_t *g_dhcpserverPoolhash = NULL;
dhcp_exclude_ip_range_t *g_dhcpserverExcludeIpList = NULL;

/* global read / write / exception fd sets */
//static struct thread_select_arg stThreadSelArg;

static unsigned dhcp_option_code_support_list[] = {
    DHO_TIME_OFFSET,
    DHO_TIME_SERVERS,
    DHO_NAME_SERVERS,
    DHO_LOG_SERVERS,
    DHO_COOKIE_SERVERS,
    DHO_LPR_SERVERS,
    DHO_IMPRESS_SERVERS,
    DHO_RESOURCE_LOCATION_SERVERS,
    DHO_HOST_NAME,
    DHO_BOOT_SIZE,
    DHO_MERIT_DUMP,
    DHO_SWAP_SERVER,
    DHO_ROOT_PATH,
    DHO_EXTENSIONS_PATH,
    DHO_IP_FORWARDING,
    DHO_NON_LOCAL_SOURCE_ROUTING,
    DHO_POLICY_FILTER,
    DHO_MAX_DGRAM_REASSEMBLY,
    DHO_DEFAULT_IP_TTL,
    DHO_PATH_MTU_AGING_TIMEOUT,
    DHO_PATH_MTU_PLATEAU_TABLE    ,
    DHO_INTERFACE_MTU,
    DHO_ALL_SUBNETS_LOCAL,
    DHO_BROADCAST_ADDRESS,
    DHO_PERFORM_MASK_DISCOVERY,
    DHO_MASK_SUPPLIER,
    DHO_ROUTER_DISCOVERY,
    DHO_ROUTER_SOLICITATION_ADDRESS,
    DHO_STATIC_ROUTES,
    DHO_TRAILER_ENCAPSULATION,
    DHO_ARP_CACHE_TIMEOUT,
    DHO_IEEE802_3_ENCAPSULATION,
    DHO_DEFAULT_TCP_TTL,
    DHO_TCP_KEEPALIVE_INTERVAL,
    DHO_TCP_KEEPALIVE_GARBAGE,
    DHO_NIS_DOMAIN,
    DHO_NIS_SERVERS,
    DHO_NTP_SERVERS,
    DHO_VENDOR_ENCAPSULATED_OPTIONS,
    DHO_NETBIOS_DD_SERVER,
    DHO_NETBIOS_SCOPE,
    DHO_FONT_SERVERS,
    DHO_X_DISPLAY_MANAGER,
    DHO_NWIP_DOMAIN_NAME,
    DHO_TFTP_SERVER_NAME,
    DHO_AUTHENTICATE,
    DHO_CLASSLESS_STATIC_ROUTES,
    DHO_CLASSLESS_STATIC_ROUTES_MS
};

struct lib_globals *ZG = NULL;
char *config_file = NULL;

extern u_int16_t local_port;
extern omapi_io_object_t omapi_io_states;
extern int timeval_cmp (struct sal_timeval a, struct sal_timeval b);
/*extern struct thread * thread_list_delete (struct thread_list *list, struct thread *thread);
extern void thread_enqueue_middle (struct thread_master *m, struct thread *thread);
extern struct sal_timeval * thread_timer_wait (struct thread_master *m, struct sal_timeval *timer_val);
extern struct thread *thread_trim_head(struct thread_list *list);
extern struct thread * thread_run (struct thread_master *m, struct thread *thread, struct thread *fetch); 
extern void thread_enqueue_high (struct thread_master *m, struct thread *thread);
extern void thread_enqueue_low (struct thread_master *m, struct thread *thread);*/
//extern int thread_process_fd(struct thread_master *m, struct thread_list *list,
//                             pal_sock_set_t *fdset, pal_sock_set_t *mfdset);
extern u_int32_t pal_if_index_lookup_by_name (char* name);
//void dhcpserver_do_register_interface(struct hash_backet *hb, void *arg);
//void dhcpserver_do_unregister_interface(struct hash_backet *hb, void *arg);



/****************************************************************************
 *
* Function
* 
****************************************************************************/
/*******************************************************************************
 * Name:    dhcpd6_locate_interface
 * Purpose: locate interface 
 * Input: 
 *   name: interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcpd6_locate_interface(char *name)
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

int
dhcpd6_recv_interface_add(char *pszIntfName, int isLayer3, uint32  ifindex)
{
    /* is it L3 interface?
     * for bug 21603 modified by liuyang 2012-12-8
     */
    if (!isLayer3) {
        return 0;
    }

    //if (is_dhcp_relay_enabled()) {
        if (NULL == dhcpd6_locate_interface(pszIntfName)) {
            dhcpserver_register_interface(pszIntfName, ifindex);
        }
    //}
    return 0;
}

int
dhcpd6_recv_interface_delete(char *pszIntfName)
{
    dhcpserver_unregister_interface(pszIntfName);
    return 0;
}

int dhcpd6_recv_intftype_change(char *pszIntfName, int isLayer3, uint32  ifindex)
{
    dhcpserver_unregister_interface(pszIntfName);
    if (!isLayer3) {
        return 0;
    }

    /* interface change to L3, register interface if need */
    //if (is_dhcp_relay_enabled()) {
        dhcpserver_register_interface(pszIntfName, ifindex);
    //}
    
    return 0;
}

int32
dhcpd6_process_intf_msg_after(cdb_node_t* p_tbl_node, 
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

        dhcpd6_recv_interface_add(p_db_if->key.name, (GLB_IF_MODE_L3 == p_db_if->mode), p_db_if->ifindex);
        break;

    case CDB_OPER_SET:
        if (NULL == p_db_if) {
            rc = PM_E_FAIL;
            SAL_ASSERT(0);
            break;
        }
        if (TBL_INTERFACE_FLD_MODE == field_id) {
            dhcpd6_recv_intftype_change(p_db_if->key.name, (GLB_IF_MODE_L3 == p_db_if->mode), p_db_if->ifindex);
        }
        break;

    case CDB_OPER_DEL:
        if (p_ds_node) {
            /* delete sub DS */
            break;
        }
        if (p_if_key && '\0' != p_if_key->name[0]) {
            dhcpd6_recv_interface_delete(p_if_key->name);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
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
 * Name:    dhcpd6_update_primary_address
 * Purpose: update primary address
 * Input: 
 *   pInfo: interface info struct 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: modified by liuyang for interface_info changed 2012-7-26
 ******************************************************************************/
int dhcpd6_update_primary_address(struct interface_info *pInfo, uint32 *addr)
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

//#if 0
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
//#endif

    memcpy(&pInfo->addresses[0], &ifr.ifr_addr.sa_data[2], sizeof(struct in_addr));
    pInfo->address_count = 1;
    return 0;
}

int
dhcpd6_recv_ipaddr_change(char *pszIntfName, prefix_t   *pAddress)
{
    struct interface_info *pInfo = NULL;

    if ((pInfo = dhcpd6_locate_interface(pszIntfName)) != NULL) {
        dhcpd6_update_primary_address(pInfo, (uint32*)&pAddress->u.prefix4);
    }
    return 0;
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
dhcpd6_process_route_msg_after(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_route_if_key_t *p_if_key = NULL;
    tbl_route_if_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    ds_connected_t *ds_conn = NULL;

    p_if_key = (tbl_route_if_key_t*)p_tbl;
    if (NULL != p_if_key) {
        p_db_if = tbl_route_if_get_route_if(p_if_key);
    }

    ds_conn = rt_if_get_primary_connected(p_db_if);
    if (!ds_conn)
    {
         return rc;     
    }
    
    switch (oper) {
    case CDB_OPER_ADD:
        if (p_db_if) {
            dhcpd6_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
        }
        break;

    case CDB_OPER_SET:
        if (p_db_if && TBL_ROUTE_IF_FLD_IFC_IPV4 == field_id) {
            dhcpd6_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
            break;
        }
        break;

    case CDB_OPER_DEL:
        if (p_db_if && TBL_ROUTE_IF_FLD_IFC_IPV4 == field_id) {
            dhcpd6_recv_ipaddr_change(p_db_if->key.name, &ds_conn->key.address);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

/*******************************************************************************
 * Name:    dhcpserver_stop
 * Purpose: shutdown dhcpserver PM
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_stop(void)
{
    dhcpserver_module_free();
    /* Stop the system. */
    //lib_stop (ZG);
    ZG = NULL;
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
void sighup (int sig)
{
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "SIGHUP received");
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
void sigint (int sig)
{
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "SIGINT received");
    
    dhcpserver_stop ();
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
void sigusr1 (int sig)
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
void signal_init ()
{
    /*pal_signal_init ();
    pal_signal_set (SIGHUP, sighup);
    pal_signal_set (SIGINT, sigint);
    pal_signal_set (SIGTERM, sigint);
    pal_signal_set (SIGUSR1, sigusr1);*/
}

#ifdef DHCPD_DEBUG
/*******************************************************************************
 * Name:    dhcpserver_debug_show_interface
 * Purpose: show this interface's status
 * Input: 
 *   hb: 
 *   arg: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_debug_show_interface(struct hash_backet *hb, void *arg)
{
    dhcp_server_interface_t *pIntf = NULL;

    if (NULL == hb)
    {
        return;
    }
    
    pIntf = (dhcp_server_interface_t *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }
    
    fprintf(stderr, "--debug dhcpserver interface %s %lu ifindex %lu\n",
               pIntf->szName, pIntf->nIfIndex);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_debug_show_interfaces
 * Purpose: show all interfaces current status
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_debug_show_interfaces(void)
{
    hash_iterate(g_dhcpserverIntfhash, dhcpserver_debug_show_interface, NULL);
    return;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_dump_packet_option
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
void dhcpserver_dump_packet_option (struct option_cache *oc,
        struct packet *packet, struct lease *lease, struct client_state *client,
        struct option_state *in_options, struct option_state *cfg_options,
        struct binding_scope **scope, struct universe *u, void *foo)
{
    const char *name, *dot;
    struct data_string ds;
    
    sal_memset (&ds, 0, sizeof ds);
    
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
         log_error ("  option %s%s%s %s;",
                    name, dot, oc->option->name,
                    pretty_print_option(oc->option, ds.data, ds.len, 1, 1));
         data_string_forget (&ds, MDL);
    }
    return;
}

/*******************************************************************************
* Name:    dhcpserver_dump_packet 
* Purpose: dump packet fields
* Input: 
*   tp: packet after decode
*   len: packet length
* Output: 
* Return: N/A
* Note: 
******************************************************************************/
void dhcpserver_dump_packet (struct packet *tp)
{
#if 0
    struct dhcp_packet *tdp;

    if (NULL == tp || NULL == tp->raw)
    {
        return;
    }

    /*if (!(IS_DEBUG_ON(dhcp, server, DHCPSERVER_E_PKT)))
    {
        return;
    }*/

    tdp = tp -> raw;
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "Dump DHCP packet fields:");
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "packet length %d", tp -> packet_length);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "op = %d  htype = %d  hlen = %d  hops = %d",
            tdp -> op, tdp -> htype, tdp -> hlen, tdp -> hops);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "xid = 0x%x  secs = %ld  flags = 0x%x",
            pal_ntoh32(tdp -> xid), (unsigned long)tdp -> secs, pal_ntoh16(tdp -> flags));
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "ciaddr = %s", inet_ntoa (tdp -> ciaddr));
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "yiaddr = %s", inet_ntoa (tdp -> yiaddr));
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "siaddr = %s", inet_ntoa (tdp -> siaddr));
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "giaddr = %s", inet_ntoa (tdp -> giaddr));
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "chaddr = %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
            ((unsigned char *)(tdp -> chaddr)) [0],
            ((unsigned char *)(tdp -> chaddr)) [1],
            ((unsigned char *)(tdp -> chaddr)) [2],
            ((unsigned char *)(tdp -> chaddr)) [3],
            ((unsigned char *)(tdp -> chaddr)) [4],
            ((unsigned char *)(tdp -> chaddr)) [5]);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "filename = %s", tdp -> file);
    DEBUG_OUT(dhcp_debug, flags, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS, "server_name = %s", tdp -> sname);
    if (tp -> options_valid)
    {
        int i;

        for (i = 0; i < tp -> options -> universe_count; i++) 
        {
            if (tp -> options -> universes [i])
            {
                option_space_foreach (tp, (struct lease *)0,
                                        (struct client_state *)0,
                                        (struct option_state *)0,
                                        tp -> options,
                                        &global_scope,
                                        universes [i], 0,
                                        dhcpserver_dump_packet_option);
            }
        }
    }
#endif
    
    return;
}

/*******************************************************************************
* Name:    dhcpserver_dump_dhcp_packet 
* Purpose: dump packet fields for dhcp server
* Input: 
*   tdp: packet after decode
*   len: packet length
* Output: 
* Return: N/A
* Note: 
******************************************************************************/
void dhcpserver_dump_dhcp_packet (struct dhcp_packet *tdp, int len)
{
#if 0
    struct packet stTP;
    struct packet *tp = &stTP;
    
    if (NULL == tdp)
    {
        return;
    }
    if (!(IS_DEBUG_ON(dhcp, server, DHCPSERVER_E_PKT))
            && !(IS_DEBUG_ON(dhcp, server, DHCPSERVER_E_DUMP)))
    {
        return;
    }
    
    sal_memset(&stTP, 0x00, sizeof(struct packet));
    tp->raw = tdp;
    tp->packet_length = len;

    if (!parse_options(tp))
    {
        if (tp->options)
        {
            option_state_dereference(&tp->options, MDL);
            packet_dereference(&tp, MDL); //??? sam_zhu
        }
    }

    if (IS_DEBUG_ON(dhcp, server, DHCPSERVER_E_PKT))
    {
        dhcpserver_dump_packet(&stTP);
    }

    DEBUG_OUT_HEX(dhcp, server, DHCPSERVER_E_DUMP,
            "DUMP dhcp message (send)",
            (u_char *)&stTP, len, 0);
    
    if (tp->options)
    {
        option_state_dereference(&tp->options, MDL);
    }
 #endif
    
    return;
}

void dhcpserver_timer_string_get(TIME *timer, char* tbuf)
{
#if 0
    char weekday[7][5] = {"Mon", "Tues", "Wed", "Thur", "Fri", "Sat", "Sun"};
    struct tm *t;
    struct pal_timeval now;
    TIME time;
    
    if(NULL == timer || NULL == tbuf)
        return;

    /*calculate aboslute time added by liuyang 2012-3-31*/
    gettimeofday(&now, NULL);
    time = now.tv_sec + *timer - cur_time;
    /* for bug 28178 modified by liuyang 2014-4-23*/
    t = localtime(&time);
    
    sprintf (tbuf, "%s %d.%02d.%02d %02d:%02d:%02d",
         ((t->tm_wday > 0 && t->tm_wday < 8)? weekday[t->tm_wday-1]:""),
         t ->tm_year + 1900,
         t ->tm_mon + 1, t ->tm_mday,
         t ->tm_hour, t ->tm_min, t ->tm_sec);
 #endif
 
    return;
}

char *dhcpserver_piaddrmaskv4(struct iaddr addr, struct iaddr mask)
{
    static char tbuf[sizeof("255.255.255.255/32")];
    char *s;
    int mw;
    unsigned i, oct, bit;

    if (addr.len != 4)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "dhcpserver_piaddrmaskv4():%s:%d: Address length %d invalid",
            MDL, addr.len);
        return NULL;
    }
    if (addr.len != mask.len)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "dhcpserver_piaddrmaskv4():%s:%d: Address and mask size mismatch",
            MDL);
        return NULL;
    }
    /* Determine netmask width in bits. */
    for (mw = 32; mw > 0; )
    {
        oct = (mw - 1) / 8;
        bit = 0x80 >> ((mw - 1) % 8);
        if (!mask.iabuf [oct])
            mw -= 8;
        else if (mask.iabuf [oct] & bit)
            break;
        else
            mw--;
    }

    s = tbuf;
    for (i = 0 ; i <= oct ; i++)
    {
        sprintf(s, "%s%d", i ? "." : "", addr.iabuf[i]);
        s += strlen(s);
    }
    sprintf(s, "/%d", mw);
    return tbuf;
}

#if 0
/*******************************************************************************
 * Name:    dhcpserver_do_unregister_interface
 * Purpose: unregister interface for dhcp server
 * Input: 
 *   hb: hash backet 
 *   arg: callback argument
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_do_unregister_interface(struct hash_backet *hb, void *arg)
{
    dhcp_server_interface_t *pIntf = NULL;
    
    if (NULL == hb)
    {
        return;
    }

    pIntf = (dhcp_server_interface_t *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }

    dhcpserver_unregister_interface(pIntf->szName);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_do_register_interface
 * Purpose: register interface for dhcp server
 * Input: 
 *   hb: hash backet
 *   arg: callback argument
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_do_register_interface(struct hash_backet *hb, void *arg)
{
    dhcp_server_interface_t *pIntf = NULL;
    
    if (NULL == hb)
    {
        return;
    }

    if (!DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)))
    {
        return;
    }
    
    /* all interfaces in hash is L3 interface, so does't care it here */
    pIntf = (dhcp_server_interface_t *)hb->data;
    if (NULL == pIntf)
    {
        return;
    }

    dhcpserver_register_interface(pIntf->szName);
    return;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_service_onoff
 * Purpose: process service on/off
 * Input: 
 *   bOn: On or off flag
 * Output: 
 * Return:
 *   success: 0
 *   failed:  CLI_ERROR
 * Note: 
 ******************************************************************************/
int dhcpserver_service_onoff(unsigned char bOn)
{
    static int g_server_service_on = 0;

    if (bOn)
    {
        if (g_server_service_on)
        {
            return 0;
        }

        /* service on */
        g_server_service_on = 1;
        //hash_iterate(g_dhcpserverIntfhash, dhcpserver_do_register_interface, NULL);
        log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "turn DHCP server function on");
        return 0;
    }

    /* service off */
    if (!g_server_service_on)
    {
        return 0;
    }
    
    g_server_service_on = 0;
    //hash_iterate(g_dhcpserverIntfhash, dhcpserver_do_unregister_interface, NULL);
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "turn DHCP server function off");
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_get_intf_by_ifindex
 * Purpose: find interface by ifindex
 * Input: 
 *   ifindex: interface index
 * Output: 
 * Return:
 *   success: interface pointer
 *   fail:    NULL
 * Note: 
 ******************************************************************************/
dhcp_server_interface_t *dhcpserver_get_intf_by_ifindex(unsigned long ifindex)
{
#if 0
    dhcp_server_interface_t *pIntf = NULL;
    dhcp_server_interface_t *pTmp = NULL;
    //struct hash *hash = g_dhcpserverIntfhash;
    struct hash_backet *hb = NULL;
    int i;

    if (NULL == hash)
    {
        return NULL;
    }

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = (dhcp_server_interface_t *)hb->data;
            if (ifindex == pTmp->nIfIndex)
            {
                pIntf = pTmp;
                break;
            }
        }
    }
 
    return pIntf;
#endif   

    return 0;    
}

/*******************************************************************************
 * Name:    dhcpserver_recv_ipaddr_change
 * Purpose: interface ip addr change callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
#if 0
int
dhcpserver_recv_ipaddr_change(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    //struct nsm_msg_address *pmsg = (struct nsm_msg_address *)message;
    dhcp_server_interface_t *pIntf = NULL;
    struct interface_info *pInfo = NULL;
    //struct in_addr newAddr;
    
#ifdef DHCPD_DEBUG    
    nsm_address_dump(ZG, pmsg);
#endif
        
    pIntf = dhcpserver_get_intf_by_ifindex(pmsg->ifindex);
    if (NULL == pIntf)
    {
        return RESULT_OK;
    }

    pInfo = dhcpserver_locate_interface(pIntf->szName);
    if (NULL != pInfo)
    {
        dhcpserver_update_primary_address(pInfo);
    }

    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_recv_interface_add
 * Purpose: interface add callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
#if 0
int
dhcpserver_recv_interface_add(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    struct nsm_msg_link *pmsg = (struct nsm_msg_link *)message;
    dhcp_server_interface_t *pIntf = NULL;
    dhcp_server_interface_t stIntf;
    
#ifdef DHCPD_DEBUG    
    nsm_interface_dump(ZG, pmsg);
#endif
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_TYPE))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        return RESULT_OK;
    }

    /* is it L3 interface? */
    if (pmsg->type == NSM_IF_TYPE_L2 || pmsg->dhcp_mode != NSM_DHCP_MODE_SERVER)
    {
        return RESULT_OK;
    }
    
    if (NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        sal_snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pmsg->ifname);
    }
    
    //if (NULL != (pIntf = hash_lookup(g_dhcpserverIntfhash, &stIntf)))
    //{
        /* item exists */
        //return RESULT_OK;
    //}

    /* add item to hash */
    if (NULL == (pIntf = dhcpserver_intf_calloc()))
    {
        log_alarm(M_MOD_DHCPRLY6, HIGH, "WARNING: out of memory.");
        return RESULT_OK;
    }
    
    pIntf->nIfIndex = pmsg->ifindex;
    sal_snprintf(pIntf->szName, INTERFACE_NAMSIZ + 1, stIntf.szName);
    //dhcpserver_intf_hash_add(g_dhcpserverIntfhash, pIntf);
    if (DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)))
    {
        dhcpserver_register_interface(pIntf->szName);
    }
     
    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_recv_interface_delete
 * Purpose: interface delete callback
 * Input: 
 *   header: nsm message header
 *   arg: argument
 *   message: message pointer
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : RESULT_ERROR
 * Note: 
 ******************************************************************************/
#if 0
int
dhcpserver_recv_interface_delete(struct nsm_msg_header *header,
                             void *arg, void *message)
{
    struct nsm_msg_link *pmsg = (struct nsm_msg_link *)message;
    dhcp_server_interface_t *pIntf = NULL;
    dhcp_server_interface_t stIntf;
    
#ifdef DHCPD_DEBUG    
    nsm_interface_dump(ZG, pmsg);
#endif

    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_LINK_CTYPE_NAME))
    {
        return RESULT_OK;
    }

    sal_snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, pmsg->ifname);
    //pIntf = hash_lookup(g_dhcpserverIntfhash, &stIntf);
    if (NULL != pIntf)
    {
        //dhcpserver_intf_hash_del(g_dhcpserverIntfhash, pIntf);
        dhcpserver_intf_free(pIntf);
        dhcpserver_unregister_interface(pmsg->ifname);
    }
     
    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_recv_intftype_change
 * Purpose: interface type change notify
 * Input: 
 *   header:  message header
 *   arg: message argument
 *   message: message 
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : 
 * Note: 
 ******************************************************************************/
#if 0
int dhcpserver_recv_intftype_change(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcp_t *pmsg = (nsm_msg_dhcp_t *)message;
    dhcp_server_interface_t *pIntf = NULL;
    dhcp_server_interface_t stIntf;
    int ifindex = 0;
    
#ifdef DHCPSERVER_DEBUG    
    nsm_dhcp_dump(ZG, pmsg);
#endif

    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_NAME))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_TYPE))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_SERVER_VRFID))
    {
        return RESULT_OK;
    }
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_L3_IF_DHCP_MODE))
    {
        return RESULT_OK;
    }

    /* locate interface item in the hash */
    sal_snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
    //pIntf = hash_lookup(g_dhcpserverIntfhash, &stIntf);
    
    if (pmsg->type == NSM_IF_TYPE_L2 || pmsg->dhcp_mode != NSM_DHCP_MODE_SERVER)
    {
        /* interface change to L2
         * delete hash item and unregister interface
         */
        if (NULL != pIntf)
        {
            dhcpserver_unregister_interface((char *)pmsg->ifname);
            //dhcpserver_intf_hash_del(g_dhcpserverIntfhash, pIntf);
            dhcpserver_intf_free(pIntf);
        }
        return RESULT_OK;
    }
    
    //ifindex = pal_if_index_lookup_by_name((char *)pmsg->ifname);
    if (ifindex <= 0)
    {
        return RESULT_OK;
    }
    
    /*Update vrfid, free old interface*/
    if(pIntf && (pIntf->vrfId != pmsg->vrfid))
    {
        dhcpserver_unregister_interface((char *)pmsg->ifname);
        pIntf->vrfId = pmsg->vrfid;
        
    }

    /* interface change to L3
     * add interface to hash
     * and register interface if need
     */
    if (NULL == pIntf)
    {
        pIntf = dhcpserver_intf_calloc();
        if (NULL == pIntf)
        {
            log_alarm(M_MOD_DHCPRLY6, HIGH, "WARNING: out of memory.");
            return RESULT_OK;
        }
        sal_snprintf(pIntf->szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
        pIntf->nIfIndex = pmsg->ifindex;
        pIntf->vrfId = pmsg->vrfid;
        //dhcpserver_intf_hash_add(g_dhcpserverIntfhash, pIntf);
    }
    
    if (DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)))
    {
        if (NULL == dhcpserver_locate_interface((char *)pmsg->ifname))
        {
            dhcpserver_register_interface((char *)pmsg->ifname);
        }
    }
    
    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_recv_l3_if_dhcp_mode_select
 * Purpose: l3 interface dhcp mode select 
 * Input: 
 *   header: message header
 *   arg:    message argument
 *   message: message
 * Output: 
 * Return: 
 *   success: RESULT_OK
 *   failed : 
 * Note: 
 ******************************************************************************/
#if 0
int dhcpserver_recv_l3_if_dhcp_mode_select(struct nsm_msg_header *header,
        void *arg, void *message)
{
    nsm_msg_dhcp_t *pmsg = (nsm_msg_dhcp_t *)message;
    dhcp_server_interface_t *pIntf = NULL;
    dhcp_server_interface_t stIntf;
     u_int8_t dhcp_mode = 0;
#ifdef DHCPD_DEBUG    
    nsm_dhcp_dump(ZG, pmsg);
#endif

    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_L3_IF_DHCP_MODE))
    {
        return RESULT_OK;
    }
    
    if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_INTERFACE_NAME))
    {
        return RESULT_OK;
    }

    dhcp_mode = pmsg->dhcp_mode;
    /* locate interface item in the hash */
    sal_snprintf(stIntf.szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
    //pIntf = hash_lookup(g_dhcpserverIntfhash, &stIntf);
    
    switch(dhcp_mode)
    {
        case NSM_DHCP_MODE_NONE:
        case NSM_DHCP_MODE_RELAY:
            if(NULL != pIntf)
            {
                dhcpserver_unregister_interface((char *)pmsg->ifname);
                //dhcpserver_intf_hash_del(g_dhcpserverIntfhash, pIntf);
                dhcpserver_intf_free(pIntf);
                pIntf = NULL;
            }
            break;
        case NSM_DHCP_MODE_SERVER:
            if (!NSM_CHECK_CTYPE (pmsg->cindex, NSM_DHCP_CTYPE_SERVER_VRFID))
            {
                return RESULT_OK;
            }
            
            if (NULL == pIntf)
            {
                pIntf = dhcpserver_intf_calloc();
                if (NULL == pIntf)
                {
                    log_alarm(M_MOD_DHCPRLY, HIGH, "WARNING: out of memory.");
                    return RESULT_OK;
                }
                        sal_snprintf(pIntf->szName, INTERFACE_NAMSIZ + 1, (char *)pmsg->ifname);
                        pIntf->nIfIndex = pmsg->ifindex;
                        pIntf->vrfId = pmsg->vrfid;
                        //dhcpserver_intf_hash_add(g_dhcpserverIntfhash, pIntf);
                
                if (DHCPSERVER_CHK_FLAG((DHCP_SERVICE_FLAG | DHCP_SERVER_SERVICE_FLAG)))
                {
                    if (NULL == dhcpserver_locate_interface((char *)pmsg->ifname))
                    {
                        dhcpserver_register_interface((char *)pmsg->ifname);
                    }
                }
            }
            break;
        default:
            log_sys(M_MOD_DHCPRLY, E_ERROR, "un-support dhcp mode %02x.", dhcp_mode);
            break;
    }

    return RESULT_OK;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_interface_notify_init
 * Purpose: register interface notice process functions
 * Input: 
 *   zg: lib globals pointer
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_interface_notify_init(struct lib_globals *zg)
{
#if 0
    struct nsm_client *nc = NULL;

    if (NULL == zg)
    {
        return;
    }
 
    nc = nsm_client_create (zg, 0);
    if (!nc)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "initialize interface change notify socket error.");
        return;
    }

    /* Set version and protocol.  */
    nsm_client_set_version (nc, NSM_PROTOCOL_VERSION_1);
    nsm_client_set_protocol (nc, IPI_PROTO_DHCPD);
    
    /* dhcp message */
    nsm_client_set_service (nc, NSM_SERVICE_DHCP);
    /*vrf change or dhcp mode change or l2 <-> l3*/
    nsm_client_set_callback (nc, NSM_MSG_DHCP_INTERFACE_TYPE, dhcpserver_recv_intftype_change);
    nsm_client_set_callback (nc, NSM_MSG_DHCP_L3_IF_DHCP_MODE_SELECT, dhcpserver_recv_l3_if_dhcp_mode_select);

    /* link message */
    nsm_client_set_service (nc, NSM_SERVICE_INTERFACE);
    //nsm_client_set_callback (nc, NSM_MSG_LINK_ADD, dhcpserver_recv_interface_add);
    nsm_client_set_callback (nc, NSM_MSG_LINK_DELETE, dhcpserver_recv_interface_delete);
    nsm_client_set_callback (nc, NSM_MSG_ADDR_ADD, dhcpserver_recv_ipaddr_change);
    nsm_client_set_callback (nc, NSM_MSG_ADDR_DELETE, dhcpserver_recv_ipaddr_change);
        
    /* dont set disconnect callback, let it reconnect automaticlly */
    nc->debug = 0;
    nsm_client_start(nc);
 #endif
    
    return;
}

/*******************************************************************************
 * Name:    omapi_one_pre_dispatch
 * Purpose: pre omapi socket for select
 * Input: 
 *   wo: omapi object
 *   t: timeout value
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
isc_result_t omapi_one_pre_dispatch(omapi_object_t *wo, struct timeval *t)
{
#if 0
    int desc;
    struct timeval now, to;
    omapi_io_object_t *io;
    omapi_waiter_object_t *waiter;

    if (!wo || wo -> type != omapi_type_waiter)
    {
        waiter = (omapi_waiter_object_t *)0;
    }
    else
    {
        waiter = (omapi_waiter_object_t *)wo;
    }

    /* First, see if the timeout has expired, and if so return. */
    if (t)
    {
        gettimeofday (&now, (struct timezone *)0);
        cur_time = now.tv_sec;
        if (now.tv_sec > t -> tv_sec ||
            (now.tv_sec == t -> tv_sec && now.tv_usec >= t -> tv_usec))
        {
            return ISC_R_TIMEDOUT;
        }
            
        /* We didn't time out, so figure out how long until
           we do. */
        to.tv_sec = t -> tv_sec - now.tv_sec;
        to.tv_usec = t -> tv_usec - now.tv_usec;
        if (to.tv_usec < 0) 
        {
            to.tv_usec += 1000000;
            to.tv_sec--;
        }

        /* It is possible for the timeout to get set larger than
           the largest time select() is willing to accept.
           Restricting the timeout to a maximum of one day should
           work around this.  -DPN.  (Ref: Bug #416) */
        if (to.tv_sec > (60 * 60 * 24))
        {
            to.tv_sec = 60 * 60 * 24;
        }
    }
    
    /* If the object we're waiting on has reached completion,
       return now. */
    if (waiter && waiter -> ready)
    {
        return ISC_R_SUCCESS;
    }
    
    /* If we have no I/O state, we can't proceed. */
    if (!(io = omapi_io_states.next))
    {
        return ISC_R_NOMORE;
    }

    /* Set up the read and write masks. */
    for (; io; io = io -> next) 
    {
        /* Check for a read socket.   If we shouldn't be
           trying to read for this I/O object, either there
           won't be a readfd function, or it'll return -1. */
        if (io -> readfd && io -> inner &&
            (desc = (*(io -> readfd)) (io -> inner)) >= 0) 
        {
            //PAL_SOCK_HANDLESET_SET(desc, &stThreadSelArg.readfd);
            if (desc > stThreadSelArg.max_fd)
            {
                stThreadSelArg.max_fd = desc;
            }
        }
        
        /* Same deal for write fdets. */
        if (io -> writefd && io -> inner &&
            (desc = (*(io -> writefd)) (io -> inner)) >= 0) 
        {
            //PAL_SOCK_HANDLESET_SET(desc, &stThreadSelArg.writefd);
            if (desc > stThreadSelArg.max_fd)
            {
                stThreadSelArg.max_fd = desc;
            }
        }
    }
#endif

    return ISC_R_SUCCESS;
}

/*******************************************************************************
 * Name:    omapi_one_post_dispatch
 * Purpose: process omapi sockets
 * Input: 
 *   wo: 
 *   t: 
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
isc_result_t omapi_one_post_dispatch(omapi_object_t *wo, struct timeval *t)
{
    fd_set r, w, x;
    int count = 0; /* Alexander Liu: we don't need this count ? */
    int desc;
    omapi_io_object_t *io, *prev;
    omapi_object_t *tmp = (omapi_object_t *)0;

    /* We probably have a bad file descriptor.   Figure out which one.
       When we find it, call the reaper function on it, which will
       maybe make it go away, and then try again. */
    if (count < 0) 
    {
        struct timeval t0;
        omapi_io_object_t *prev = (omapi_io_object_t *)0;
        io = (omapi_io_object_t *)0;
        if (omapi_io_states.next)
        {
            omapi_io_reference (&io, omapi_io_states.next, MDL);
        }

        while (io) 
        {
            omapi_object_t *obj;
            FD_ZERO (&r);
            FD_ZERO (&w);
            t0.tv_sec = t0.tv_usec = 0;

            if (io -> readfd && io -> inner &&
                (desc = (*(io -> readfd)) (io -> inner)) >= 0) 
            {
                //PAL_SOCK_HANDLESET_SET (desc, &r);
                count = select (desc + 1, &r, &w, &x, &t0);
bogon:
                if (count < 0) 
                {
                    log_sys(M_MOD_DHCPRLY6, E_ERROR, 
                        "Bad descriptor %d.", desc);
                    for (obj = (omapi_object_t *)io;
                        obj -> outer;
                        obj = obj -> outer)
                    {
                        ;
                    }
                    for (; obj; obj = obj -> inner) 
                    {
                        omapi_value_t *ov;
                        int len;
                        const char *s;
                        ov = (omapi_value_t *)0;
                        omapi_get_value_str (obj, (omapi_object_t *)0, "name", &ov);
                        if (ov && ov -> value &&
                            (ov -> value -> type == omapi_datatype_string)) 
                        {
                            s = (char *) ov -> value -> u.buffer.value;
                            len = ov -> value -> u.buffer.len;
                        } 
                        else 
                        {
                            s = "";
                            len = 0;
                        }
                        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
                            "Object %lx %s%s%.*s",
                            (unsigned long)obj, obj -> type ->name,
                            len ? " " : "", len, s);
                        if (len)
                        {
                            omapi_value_dereference (&ov, MDL);
                        }
                    }
                    (*(io -> reaper)) (io -> inner);
                    if (prev) 
                    {
                        omapi_io_dereference (&prev -> next, MDL);
                        if (io -> next)
                        {
                            omapi_io_reference (&prev -> next, io -> next, MDL);
                        }
                    } 
                    else 
                    {
                        omapi_io_dereference (&omapi_io_states.next, MDL);
                        if (io -> next)
                        {
                            omapi_io_reference (&omapi_io_states.next, io -> next, MDL);
                        }
                    }
                    omapi_io_dereference (&io, MDL);
                    return ISC_R_SUCCESS;
                }
            }
            
            FD_ZERO (&r);
            FD_ZERO (&w);
            t0.tv_sec = t0.tv_usec = 0;

            /* Same deal for write fdets. */
            if (io -> writefd && io -> inner &&
                (desc = (*(io -> writefd)) (io -> inner)) >= 0)
            {
                //PAL_SOCK_HANDLESET_SET (desc, &w);
                count = select (desc + 1, &r, &w, &x, &t0);
                if (count < 0)
                    goto bogon;
            }
            if (prev)
            {
                omapi_io_dereference (&prev, MDL);
            }
            omapi_io_reference (&prev, io, MDL);
            omapi_io_dereference (&io, MDL);
            if (prev -> next)
            {
                omapi_io_reference (&io, prev -> next, MDL);
            }
        } /* while */
        if (prev)
        {
            omapi_io_dereference (&prev, MDL);
        }
    } /* if (cout < 0 */

    for (io = omapi_io_states.next; io; io = io -> next) 
    {
        if (!io -> inner)
        {
            continue;
        }
        
        omapi_object_reference (&tmp, io -> inner, MDL);
        /* Check for a read descriptor, and if there is one,
           see if we got input on that socket. */
        if (io -> readfd && (desc = (*(io -> readfd)) (tmp)) >= 0)
        {
           // if (PAL_SOCK_HANDLESET_ISSET(desc, &stThreadSelArg.readfd))
            {
                ((*(io -> reader)) (tmp));
            }
        }
        
        /* Same deal for write descriptors. */
        if (io -> writefd && (desc = (*(io -> writefd)) (tmp)) >= 0)
        {
           // if (PAL_SOCK_HANDLESET_ISSET(desc, &stThreadSelArg.writefd))
            {
                ((*(io -> writer)) (tmp));
            }
        }

        omapi_object_dereference (&tmp, MDL);
    }

    /* Now check for I/O handles that are no longer valid,
       and remove them from the list. */
    prev = (omapi_io_object_t *)0;
    for (io = omapi_io_states.next; io; io = io -> next) 
    {
        if (io -> reaper) 
        {
            if (!io -> inner || ((*(io -> reaper)) (io -> inner) != ISC_R_SUCCESS)) 
            {
                omapi_io_object_t *tmp =
                    (omapi_io_object_t *)0;
                /* Save a reference to the next
                   pointer, if there is one. */
                if (io -> next)
                {
                    omapi_io_reference (&tmp, io -> next, MDL);
                }
                
                if (prev) 
                {
                    omapi_io_dereference (&prev -> next, MDL);
                    if (tmp)
                    {
                        omapi_io_reference (&prev -> next, tmp, MDL);
                    }
                } 
                else 
                {
                    omapi_io_dereference (&omapi_io_states.next, MDL);
                    if (tmp)
                    {
                        omapi_io_reference (&omapi_io_states.next, tmp, MDL);
                    }
                    else
                    {
                        omapi_signal_in ((omapi_object_t *) &omapi_io_states, "ready");
                    }
                }
                
                if (tmp)
                {
                    omapi_io_dereference (&tmp, MDL);
                }
            }
        }
        prev = io;
    }

    return ISC_R_SUCCESS;
}

/*******************************************************************************
 * Name:    dhcpserver_dispatch
 * Purpose: dhcp server main loop
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_dispatch(void)
{
#if 0
    struct pal_timeval tv;
    struct pal_timeval *tvp = NULL;
    struct pal_timeval tv2;
    struct pal_timeval *tvp2 = NULL;
    struct pal_timeval tv_select = {1, 0};
    isc_result_t status;
    int count = 0;
    struct pal_timeval now;
    
    while (1)
    {
        /* use relative time modified by liuyang 2012-3-31*/
        #ifndef _GLB_UML_SYSTEM_    
        /* modified by jqiu 2013-03-06
         * some platform don't support param -1 in gettimeofday.
         * use another sys call clock_gettime() */
         /* use relative time modified by liuyang 2012-8-16*/        
        pal_ctc_clock_gettime(&now);
        #else
        gettimeofday (&now, (struct timezone *)-1);
        #endif
        cur_time = now.tv_sec;

    /* process timeout */
        tv_select.tv_sec = 1;
        tv_select.tv_usec = 0;
        tvp = thread_fetch_calc_timeouts(ZG, &tv);
        tvp2 = process_outstanding_timeouts(&tv2);

        if (NULL != tvp && NULL != tvp2)
        {
            if (tvp->tv_sec >= tvp2->tv_sec)
            {
                tv_select.tv_sec = tvp2->tv_sec;
                tv_select.tv_usec = tvp2->tv_usec;
            }
            else
            {
                tv_select.tv_sec = tvp->tv_sec;
                tv_select.tv_usec = tvp->tv_usec;
            }
        }
        else if (NULL != tvp)
        {
            tv_select.tv_sec = tvp->tv_sec;
            tv_select.tv_usec = tvp->tv_usec;
        }
        else if (NULL != tvp2)
        {
            tv_select.tv_sec = tvp2->tv_sec;
            tv_select.tv_usec = tvp2->tv_usec;
        }

        /* prepare select sockets */
        thread_select_arg_init(&stThreadSelArg);
        thread_fetch_pre_process (ZG);
        thread_get_select_arg(ZG, &stThreadSelArg);
        omapi_one_pre_dispatch (0, NULL);

        /* do select */
        count = sal_sock_select(stThreadSelArg.max_fd + 1, 
                                &stThreadSelArg.readfd, 
                                &stThreadSelArg.writefd, 
                                &stThreadSelArg.exceptfd, &tv_select);

        /* process sockets */
        if (count > 0)
        {
            /* process ready sockets */
            thread_fetch_post_process (ZG, &stThreadSelArg);
            status = omapi_one_post_dispatch (0, tvp2);
            if (status != ISC_R_TIMEDOUT && status != ISC_R_SUCCESS)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, "process socket failed: %s.", isc_result_totext (status));
            }
            continue;
        }
        
        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            log_sys(M_MOD_DHCPRLY6, E_ERROR, "shutdown for socket select error: %s.", sal_strerror(errno));
            break;
        }
        
        /* timed out */
        continue;
    }
#endif
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_start
 * Purpose: starting dhcp server module
 * Input: 
 *   conffile: configuration file
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int
dhcpserver_start(char *conffile)
{
#if 0
    s_int32_t vty_port = DHCPD_VTY_PORT;
    int ret;

    /* Initialize memcpy. */
    memory_init (IPI_PROTO_DHCPD);
    
    /* Allocate global variable container. */
    ZG = lib_create ();
    if (! ZG)
        return -1;

    ret = lib_start (ZG);
    if (ret < 0)
    {
        lib_stop (ZG);
        return ret;
    }

    ZG->protocol = IPI_PROTO_DHCPD;
    ZG->log = openzlog (ZG, ZG->vr_instance, IPI_PROTO_DHCPD, LOGDEST_MAIN);
    ZG->master = thread_master_create ();

    /* Initialize CLI */
    host_vty_init (ZG);

    /* Initialize memmgr */
#ifdef MEMMGR
    memmgr_cli_init (ZG);
#endif

    /* Start the configuration management */
    host_config_start (ZG, config_file, vty_port);
    
//    cli_install_config(ZG->ctree, INTERFACE_MODE, dhcpserver_if_config_write);
//    cli_install_default(ZG->ctree, INTERFACE_MODE);
    dhcpserver_cli_init(ZG->ctree);
    
    dhcpserver_module_init();
    
    dhcpserver_interface_notify_init(ZG);

#ifdef _CENTEC_
#ifdef HAVE_BM            
    bmcli_start(ZG, "dhcpd6");
#endif
#endif                    
    db_startup (0);

    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "Ready to service.");
    return dhcpserver_dispatch();
#endif

    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_update_if_vrfid
 * Purpose: update vrfid of this interface
 * Input: 
 *   pInfo: interface info struct 
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_update_if_vrfid(struct interface_info *pInfo)
{
#if 0
    dhcp_server_interface_t stIntf, *pIntf = NULL;

    if (NULL == pInfo)
    {
        return -1;
    }

    sal_snprintf(stIntf.szName, IFNAMSIZ, pInfo->name);
    //pIntf = hash_lookup(g_dhcpserverIntfhash, &stIntf);
    if(NULL == pIntf)
    { 
        pInfo->vrfid = VRF_ID_DEFAULT;
        return 0;
    }
    pInfo->vrfid = pIntf->vrfId;
 #endif
 
    return 0;
}

/*modified for add current ip addr to exclude ip list and delete old one by liuyang*/
int dhcpserver_update_primary_address(struct interface_info *pInfo)
{
    struct ifreq ifr;
    int sock = -1;
    struct sal_in4_addr siaddr;
    struct sal_in4_addr tmpaddr;
    
    if (NULL == pInfo)
    {
        return -1;
    }

    sal_memset (&tmpaddr, 0 ,sizeof (struct sal_in4_addr));
    sal_memset (&siaddr, 0 ,sizeof (struct sal_in4_addr));
    
    if (pInfo->addresses == NULL) 
    {
        pInfo->addresses = dmalloc(4 * sizeof(struct in_addr), MDL);
        if (pInfo->addresses == NULL) 
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Out of memory saving IPv4 address "
                                       "on interface.");
                return -1;
        }
        pInfo->address_count = 0;
        pInfo->address_max = 4;
    } 
    else
    {
        siaddr = *pInfo->addresses;
        dhcpserver_exclude_ip_item_delete(siaddr, tmpaddr);    
    }

    sal_memset (pInfo->addresses, 0 ,sizeof (struct in_addr));
    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        log_fatal ("Can't create addrlist socket");
        return -1;
    }   

    //pal_sock_set_bindtofib (sock, pInfo->vrfid);
    snprintf(ifr.ifr_name, IFNAMSIZ, pInfo->name);
    if (ioctl (sock, SIOCGIFADDR, &ifr) < 0)
    {
        close(sock);
        return -1;
    }
    
    close(sock);
    
    sal_memcpy(&pInfo->addresses[0], &ifr.ifr_addr.sa_data[2], sizeof(struct in_addr));
    pInfo->address_count = 1;
    siaddr = *pInfo->addresses;
    if(!dhcpserver_exclude_ip_item_lookup(siaddr, tmpaddr))
    {
        dhcpserver_exclude_ip_item_add(siaddr, tmpaddr);
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_init_hardware_address
 * Purpose: get hardware address for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_init_hardware_address(struct interface_info *tmp)
{
    struct ifreq ifr;
    struct sockaddr sa;
    struct ifreq *tif = NULL;
    int sock = -1;

    if (NULL == tmp)
    {
        return -1;
    }

    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't create addrlist socket");
        return -1;
    }

    if (!tmp -> ifp) 
    {
        /* Make up an ifreq structure. */
        tif = (struct ifreq *)dmalloc (sizeof (struct ifreq), MDL);
        if (!tif)
        {
            close(sock);
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "no space to remember ifp.");
            return -1;
        }
        
        memset (tif, 0, sizeof (struct ifreq));
        strcpy (tif -> ifr_name, tmp -> name);
        tmp -> ifp = tif;
    }

    /*modified by xgu for bug 14721, 2011-04-26*/
    //pal_sock_set_bindtofib (sock, tmp->vrfid);
    /* Read the hardware address from this interface. */
    ifr = *tmp -> ifp;
    if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(sock);
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "get hardware address for interface %s failed: %s",
            tmp->name, sal_strerror(errno));
        return -1;
    }
    
    sa = *(struct sockaddr *)&ifr.ifr_hwaddr;
    switch (sa.sa_family) 
    {
    case ARPHRD_ETHER:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_ETHER;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        /*log_error ("Initialize hardware address to ETHER %02x.%02x.%02x.%02x.%02x.%02x",
            tmp->hw_address.hbuf [1],
            tmp->hw_address.hbuf [2],
            tmp->hw_address.hbuf [3],
            tmp->hw_address.hbuf [4],
            tmp->hw_address.hbuf [5],
            tmp->hw_address.hbuf [6]);*/
        break;
    case ARPHRD_IEEE802:
        tmp -> hw_address.hlen = 7;
        tmp -> hw_address.hbuf [0] = ARPHRD_IEEE802;
        memcpy (&tmp -> hw_address.hbuf [1], sa.sa_data, 6);
        /*log_error ("Initialize hardware address to IEEE802 %02x.%02x.%02x.%02x.%02x.%02x",
            tmp->hw_address.hbuf [1],
            tmp->hw_address.hbuf [2],
            tmp->hw_address.hbuf [3],
            tmp->hw_address.hbuf [4],
            tmp->hw_address.hbuf [5],
            tmp->hw_address.hbuf [6]);*/
        break;
#ifdef HAVE_ARPHRD_LOOPBACK
        case ARPHRD_LOOPBACK:
            /* ignore loopback interface */
            break;
#endif
    default:
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "%s: unsupport hardware address type %d",
            ifr.ifr_name, sa.sa_family);
        break;
    }

    close(sock);
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_setup_if_share_network
 * Purpose: setup share network for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_setup_if_share_network(struct interface_info *tmp)
{
    struct iaddr addr;
    struct subnet *subnet = (struct subnet *)0;

    addr.len = 4;
    memcpy (addr.iabuf, &tmp -> addresses[0].s_addr, addr.len);
    if (dhcp_interface_setup_hook)
    {
        (*dhcp_interface_setup_hook)(tmp, &addr);
    }

    for (subnet = (tmp -> shared_network
            ? tmp -> shared_network -> subnets
            : (struct subnet *)0);
        subnet; subnet = subnet -> next_sibling)
    {
            if (!subnet -> interface_address.len)
            {
                /* Set the interface address for this subnet
                to the first address we found. */
                subnet -> interface_address.len = 4;
                memcpy (subnet -> interface_address.iabuf,
                &addr.iabuf, 4);
            }
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_remove_if_share_network
 * Purpose: remove share network for this interface 
 * Input: 
 *   tmp: interface pointer
 * Output: 
 * Return: 
 *   success:  0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_remove_if_share_network(struct interface_info *tmp)
{
    struct iaddr addr;
    struct subnet *subnet = (struct subnet *)0;

    addr.len = 4;
    memcpy (addr.iabuf, &tmp -> addresses[0].s_addr, addr.len);

    for (subnet = (tmp -> shared_network
            ? tmp -> shared_network -> subnets
            : (struct subnet *)0);
        subnet; subnet = subnet -> next_sibling)
    {
            if (!subnet -> interface_address.len)
            {
                /* Set the interface address for this subnet
                to the first address we found. */
                subnet -> interface_address.len = 0;
                memset (&subnet -> interface_address,0,sizeof(struct iaddr));
            }
    }
    dhcpd_interface_delete_hook(tmp, &addr);
    return 0;
}

void
dhcpserver_remove_from_intf_list(struct interface_info **list, struct interface_info *tmp)
{
    struct interface_info *last, *ip;

    last = 0;
    interface_reference(&last, tmp, MDL);
    last = 0;
    for (ip = *list; ip; ip = ip->next) {
        if (!strcmp (ip->name, tmp->name)) {
            /* Remove from list */
            if (last) {
                ip = (struct interface_info *)0;
                interface_reference (&ip, last->next, MDL);
                interface_dereference (&last->next, MDL);
                if (ip->next) {
                    interface_reference (&last->next, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            } else {
                ip = (struct interface_info *)0;
                interface_reference (&ip, *list, MDL);
                interface_dereference (list, MDL);
                if (ip->next) {
                    interface_reference (list, ip->next, MDL);
                    interface_dereference (&ip->next, MDL);
                }
            }
            if (ip->client)
            {
                tmp->client = ip->client;
                tmp->client->interface = tmp;
            }
            break;
        }
        last = ip;
    }

    interface_dereference(&tmp, MDL);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_register_interface
 * Purpose: register interface to dhcp server interface list
 * Input: 
 *   name:  interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcpserver_register_interface(char *name, uint32  ifindex)
{
    struct interface_info *tmp = (struct interface_info *)0;
    isc_result_t status = ISC_R_SUCCESS;
    
    if ((tmp = dhcpserver_locate_interface(name)) != NULL)
    {
        return tmp;
    }
    
    //log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "Enable dhcp server function for interface %s", name);
    
    status = interface_allocate (&tmp, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "%s: interface_allocate: %s", 
              name, isc_result_totext (status));
        return NULL;
    }
    
    strcpy (tmp->name, name);
    tmp->index = -1;

    tmp->ifindex = ifindex;
    
    interface_snorf(tmp, INTERFACE_REQUESTED);
    dhcpserver_update_if_vrfid(tmp);
    dhcpserver_init_hardware_address(tmp);
    /*when vrf changed ,ip update will fail for no ip address on interface*/
    dhcpserver_update_primary_address(tmp);

    /* Register the interface... */
    if_register_receive (tmp);
    if_register_send (tmp);

    //interface_stash (tmp);
    
    /* Now register the interfaces as protocols. */
    if (tmp->rfdesc != -1)
    {
        status = omapi_register_io_object((omapi_object_t *)tmp,
                                     if_readsocket, 0, got_one, 0, 0);
        if (status != ISC_R_SUCCESS)
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR,
                "Can't register I/O handle for %s: %s",
                tmp->name, 
                isc_result_totext(status));

            if_deregister_send (tmp);
            if_deregister_receive (tmp);
        }
    }
    
    interface_dereference(&tmp, MDL);
    return tmp;
}

struct interface_info *dhcpserver_locate_interface_by_id(int id)
{
    struct interface_info *ip = NULL;

    for (ip = interfaces; ip; ip = ip -> next) 
    {
        if (id == ip->index)
        {
            return ip;
        }
    }
    
    return NULL;
}

/*******************************************************************************
 * Name:    dhcpserver_locate_interface
 * Purpose: locate interface 
 * Input: 
 *   name: interface name
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
struct interface_info *dhcpserver_locate_interface(char *name)
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
 * Name:    dhcpserver_unregister_interface
 * Purpose: unregister interface from dhcp servere interface list
 * Input: 
 *     name: interface name
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_unregister_interface(char *name)
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
                XFREE(MTYPE_TMP, ip->remote_id);
                ip->remote_id = NULL;
                ip->remote_id_len = 0;
                XFREE(MTYPE_TMP, ip->circuit_id);
                ip->circuit_id = (u_int8_t *)ip->name;
                ip->circuit_id_len = sal_strlen(ip->name);
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
    
    log_sys(M_MOD_DHCPRLY6, E_INFORMATIONAL, "Disable dhcp server function for interface %s", name);
#if 0
    /* add the interface to the dummy_interface list */
    if (dummy_interfaces) 
    {
        interface_reference (&interface -> next, dummy_interfaces, MDL);
        interface_dereference (&dummy_interfaces, MDL);
    }
    interface_reference (&dummy_interfaces, interface, MDL);
#endif
    /* remove the io object */
    omapi_unregister_io_object ((omapi_object_t *)interface);
    if_deregister_send (interface);
    if_deregister_receive (interface);
    interface_dereference (&interface, MDL);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_module_init
 * Purpose: initialize dhcp server module
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : CLI_ERROR
 * Note: 
 ******************************************************************************/
int dhcpserver_module_init(void)
{
#if 0
    int nModId = M_MOD_DHCPRLY6;
    int nCmdMode = 0; /*!DEBUG_MODE_HIDDEN*/
        
    if (g_dhcpserver_initialized)
    {
        return 1;
    }

    g_dhcpserverSwitch = 0x00;

    //DEBUG_REGISTER(dhcp, server, (void *)(&nModId), (void *)(&nCmdMode));
    dhcpserver_intf_hash_init();
    
    g_dhcpserver_initialized = 1;
 #endif

    dhcpserver_pool_hash_init();
 
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_module_free
 * Purpose: release dhcp server module
 * Input:  N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_module_free(void)
{
#if 0
    if (0 == g_dhcpserver_initialized)
    {
        return;
    }

    //hash_iterate(g_dhcpserverIntfhash, dhcpserver_do_unregister_interface, NULL);
    //dhcpserver_intf_hash_finish(g_dhcpserverIntfhash);
    dhcpserver_pool_hash_finish(g_dhcpserverPoolhash);
    dhcpserver_exclude_ip_free_all();
     g_dhcpserver_initialized = 0;
    DEBUG_UNREGISTER(dhcp, server);
#endif
    
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_make 
 * Purpose: get hash key
 * Input: 
 *   intf: interface pointer
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
u_int32_t dhcpserver_intf_hash_make (dhcp_server_interface_t *intf)
{
    u_int32_t val = 0;
    int nLen = sal_strlen(intf->szName);
    int i;
    
    if (NULL != intf)
    {
        for (i = 0; i < nLen; i++)
        {
            val += intf->szName[i];
        }
    }
    
    return val;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_cmp 
 * Purpose: hash compare function
 * Input: 
 *   a: interface pointer
 *   b: interface pointer
 * Output: 
 * Return: 0, 1
 * Note: 
 ******************************************************************************/
bool dhcpserver_intf_hash_cmp (void *a, void *b)
{
    dhcp_server_interface_t *intfa = (dhcp_server_interface_t *) a;
    dhcp_server_interface_t *intfb = (dhcp_server_interface_t *) b;

    if (!sal_strcmp(intfa->szName, intfb->szName))
    {
        return 1;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_calloc
 * Purpose: calloc dhcp server interface 
 * Input: N/A
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
dhcp_server_interface_t *dhcpserver_intf_calloc(void)
{
    dhcp_server_interface_t *pItem = NULL;
    pItem = XCALLOC(MTYPE_TMP, sizeof(dhcp_server_interface_t));
    return pItem;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_free
 * Purpose: free dhcp server interface struct
 * Input: 
 *   pItem: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_intf_free(dhcp_server_interface_t *pItem)
{
    if (NULL == pItem)
    {
        return;
    }

    XFREE(MTYPE_TMP, pItem);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_init
 * Purpose: initialize dhcpserver interface hash
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_intf_hash_init(void)
{
#if 0
    if (NULL != g_dhcpserverIntfhash)
    {
        return 0;
    }

    g_dhcpserverIntfhash = hash_create(dhcpserver_intf_hash_make, dhcpserver_intf_hash_cmp);
    if (NULL == g_dhcpserverIntfhash)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "WARNING: out of memory.");
        return -1;
    }
#endif
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_finish 
 * Purpose: free hash items
 * Input: 
 *   pHash: interface hash
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
/*void dhcpserver_intf_hash_finish (struct hash *pHash)
{
    hash_clean (pHash, (HASH_CLEAN_FUNC)dhcpserver_intf_free);
    hash_free (pHash);
    return;
}*/

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_add
 * Purpose: add interface to hash
 * Input: 
 *   pHash: interface hash
 *   pIntf: intercace item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
/*void dhcpserver_intf_hash_add(struct hash *pHash, dhcp_server_interface_t *pIntf)
{
    hash_get(pHash, pIntf, hash_alloc_intern);
    return;
}*/

/*******************************************************************************
 * Name:    dhcpserver_intf_hash_del
 * Purpose: delete interface from hash
 * Input: 
 *   pHash: interface hash
 *   pIntf: interface item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
/*void dhcpserver_intf_hash_del(struct hash *pHash, dhcp_server_interface_t *pIntf)
{
    hash_release (pHash, pIntf);
    return;
}*/

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_item_calloc
 * Purpose: calloc a dhcp exclude ip item
 * Input:  N/A
 * Output: 
 * Return: 
 *   success: item pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
dhcp_exclude_ip_range_t *dhcpserver_exclude_ip_item_calloc(void)
{
    dhcp_exclude_ip_range_t *pItem = NULL;

    pItem = XCALLOC(MTYPE_TMP, sizeof(dhcp_exclude_ip_range_t));
    return pItem;
}

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_item_free
 * Purpose: free a dhcp exclude ip item
 * Input:  
 *   pItem: exclude ip range item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_exclude_ip_item_free(dhcp_exclude_ip_range_t *pItem)
{
    if (NULL == pItem)
    {
        return;
    }

    XFREE(MTYPE_TMP, pItem);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_item_add
 * Purpose: add exclude ip  item to the list
 * Input:  
 *   pItem: exclude ip range item
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int dhcpserver_exclude_ip_item_add(struct sal_in4_addr startip, struct sal_in4_addr endip)
{
    dhcp_exclude_ip_range_t *pItem = (dhcp_exclude_ip_range_t *)0;
    dhcp_exclude_ip_range_t *p = g_dhcpserverExcludeIpList;
    pItem = dhcpserver_exclude_ip_item_calloc();
    if(NULL == pItem)
        return -1;

    pItem->startip = startip;
    pItem->endip = endip;
    
    if(NULL == p)
    {
        g_dhcpserverExcludeIpList = pItem;
    }
    else
    {
        while(p->next)
        {
            p = p->next;
        }
        p->next = pItem;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_item_delete
 * Purpose: delete exclude ip item
 * Input:  
 *   pItem: exclude ip range item
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1 
 * Note: 
 ******************************************************************************/
int dhcpserver_exclude_ip_item_delete(struct sal_in4_addr startip, struct sal_in4_addr endip)
{
    dhcp_exclude_ip_range_t *p = g_dhcpserverExcludeIpList;
    dhcp_exclude_ip_range_t *pre = (dhcp_exclude_ip_range_t *)0;
    
    while(p)
    {
        if(startip.s_addr == p->startip.s_addr
            && endip.s_addr == p->endip.s_addr)
        {
            break;
        }
        pre = p;
        p = p->next;
    }
    
    if(!p)
    {
        return -1;
    }
    
    if(pre)
    {
        pre->next = p->next;
    }
    else
    {
        g_dhcpserverExcludeIpList = p->next;
    }
    
    dhcpserver_exclude_ip_item_free(p);
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_free_all
 * Purpose: free all exclude ip item
 * Input:  
 *   N/A
 * Output: 
 * Return: 
 *   N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_exclude_ip_free_all()
{
    dhcp_exclude_ip_range_t *p = g_dhcpserverExcludeIpList;
    dhcp_exclude_ip_range_t *next = (dhcp_exclude_ip_range_t *)0;
    
    while(p)
    {
        next = p->next;
        dhcpserver_exclude_ip_item_free(p);
        p = next;
    }
    
    g_dhcpserverExcludeIpList = NULL;
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_exclude_ip_item_lookup
 * Purpose: exclude ip lookup
 * Input:  
 *   startip: exclude start ip
 *   endip: exclude end ip
 * Output: 
 * Return: 
 *   item pointer
 * Note:
 ******************************************************************************/
dhcp_exclude_ip_range_t *dhcpserver_exclude_ip_item_lookup(struct sal_in4_addr startip, struct sal_in4_addr endip)
{
    dhcp_exclude_ip_range_t *p = g_dhcpserverExcludeIpList;
    while(p)
    {
        if(startip.s_addr == p->startip.s_addr 
            && endip.s_addr == p->endip.s_addr)
        {
            break;
        }
        p = p->next;
    }

    return p;
}


/*******************************************************************************
 * Name:    dhcpserver_pool_hash_make 
 * Purpose: get hash key
 * Input: 
 *   intf: pool pointer
 * Output: 
 * Return: 
 *   success: 
 *   failed : 
 * Note: 
 ******************************************************************************/
u_int32_t dhcpserver_pool_hash_make (dhcp_pool_t *pool)
{
    u_int32_t val = 0;
    int nLen = sal_strlen(pool->pool_name);
    int i;
    
    if (NULL != pool)
    {
        for (i = 0; i < nLen; i++)
        {
            val += pool->pool_name[i];
        }
    }
    
    return val;
}

/*******************************************************************************
 * Name:    dhcpserver_poolhash_cmp 
 * Purpose: hash compare function
 * Input: 
 *   a: pool pointer
 *   b: pool pointer
 * Output: 
 * Return: 0, 1
 * Note: 
 ******************************************************************************/
bool dhcpserver_pool_hash_cmp (void *a, void *b)
{
    dhcp_pool_t *poola = (dhcp_pool_t *) a;
    dhcp_pool_t *poolb = (dhcp_pool_t *) b;

    if (!strcmp(poola->pool_name, poolb->pool_name))
    {
        return 1;
    }
    
    return 0;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_calloc
 * Purpose: calloc dhcp server pool 
 * Input: N/A
 * Output: 
 * Return: 
 *   success: pointer
 *   failed : NULL
 * Note: 
 ******************************************************************************/
dhcp_pool_t *dhcpserver_pool_calloc(void)
{
    dhcp_pool_t *pItem = NULL;
    pItem = XCALLOC(MTYPE_TMP, sizeof(dhcp_pool_t));
    return pItem;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_free
 * Purpose: free dhcp server pool struct
 * Input: 
 *   pItem: 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_free(dhcp_pool_t *pItem)
{
    struct dhcp_option_s *option = NULL;
    if (NULL == pItem)
    {
        return;
    }
    
    if(pItem->dhcp_pool_common.bootfile_name)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.dns_server);
    
    if(pItem->dhcp_pool_common.domain_name)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.domain_name);

    if(pItem->dhcp_pool_common.dns_server)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.dns_server);
    
    if(pItem->dhcp_pool_common.gateway)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.gateway);
    
    if(pItem->dhcp_pool_common.netbios_server)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.netbios_server);
    
    if(pItem->dhcp_pool_common.tftp_server_addr)
        XFREE(MTYPE_TMP, pItem->dhcp_pool_common.tftp_server_addr);

    if(pItem->hw_addr)
        XFREE(MTYPE_TMP, pItem->hw_addr);
    
    if(pItem->clientId)
        XFREE(MTYPE_TMP, pItem->clientId);
    
    for(option = pItem->dhcp_pool_common.options; option; option = option->next)
    {
        if(option->buf)
        {
            XFREE(MTYPE_TMP, option->buf);
        }
        
        XFREE(MTYPE_TMP, option);
    }
    XFREE(MTYPE_TMP, pItem);
    return;
}

/*******************************************************************************
 * Name:    dhcpserver_pool_hash_init
 * Purpose: initialize dhcpserver pool hash
 * Input: N/A
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
int dhcpserver_pool_hash_init(void)
{
    if (NULL != g_dhcpserverPoolhash)
    {
        return 0;
    }

    g_dhcpserverPoolhash = ctclib_hash_create(dhcpserver_pool_hash_make, dhcpserver_pool_hash_cmp);
    if (NULL == g_dhcpserverPoolhash)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "WARNING: out of memory.");
        return -1;
    }
    
    return 0;
}

#if 0
/*******************************************************************************
 * Name:    dhcpserver_pool_hash_finish 
 * Purpose: free hash items
 * Input: 
 *   pHash: pool hash
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_hash_finish (struct hash *pHash)
{
    hash_clean (pHash, (HASH_CLEAN_FUNC)dhcpserver_pool_free);
    hash_free (pHash);
    return;
}
#endif

/*******************************************************************************
 * Name:    dhcpserver_pool_hash_add
 * Purpose: add pool to hash
 * Input: 
 *   pHash: pool hash
 *   pIntf: pool item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_hash_add(ctclib_hash_t *pHash, dhcp_pool_t *pItem)
{
    ctclib_hash_get(pHash, pItem, ctclib_hash_alloc_intern);
    return;
}

#if 0
/*******************************************************************************
 * Name:    dhcpserver_pool_hash_del
 * Purpose: delete pool from hash
 * Input: 
 *   pHash: pool hash
 *   pIntf: pool item
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void dhcpserver_pool_hash_del(struct hash *pHash, dhcp_pool_t *pItem)
{
    hash_release (pHash, pItem);
    return;
}
#endif

/*******************************************************************************
 * Name:   dhcpserver_static_pool_confilict_check
 * Purpose: check static pool configuration confilict or not
 * Input: 
 *   ip: binding ip for static pool 
 *   pInPool: the input pool
 * Output: 
 * Return:
 *   success: pool pointer
 *   fail:    NULL
 * Note: 
 ******************************************************************************/
dhcp_pool_t *dhcpserver_static_pool_confilict_check(dhcp_pool_t *pInPool, struct prefix *ip)
{
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t *pTmp = NULL;
    ctclib_hash_t *hash = g_dhcpserverPoolhash;
    ctclib_hash_backet_t *hb = NULL;
    int i;

    if (NULL == hash || NULL == ip)
    {
        return NULL;
    }
        
    for (i = 0; i < hash->size && !pPool; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = (dhcp_pool_t *)hb->data;
            if (staticPool == pTmp->pool_type
                && sal_strcmp(pTmp->pool_name, pInPool->pool_name)
                && !sal_memcmp(&(pTmp->fixIp.u.prefix4), &(ip->u.prefix4), sizeof(struct sal_in4_addr)))
            {
                pPool = pTmp;
                break;
            }
        }
    }

    return pPool;
}

const u_char mask32bit1[][4] =
{
  {0x00, 0x00, 0x00, 0x00},

  {0x80, 0x00, 0x00, 0x00},
  {0xc0, 0x00, 0x00, 0x00},
  {0xe0, 0x00, 0x00, 0x00},
  {0xf0, 0x00, 0x00, 0x00},
  {0xf8, 0x00, 0x00, 0x00},
  {0xfc, 0x00, 0x00, 0x00},
  {0xfe, 0x00, 0x00, 0x00},
  {0xff, 0x00, 0x00, 0x00},

  {0xff, 0x80, 0x00, 0x00},
  {0xff, 0xc0, 0x00, 0x00},
  {0xff, 0xe0, 0x00, 0x00},
  {0xff, 0xf0, 0x00, 0x00},
  {0xff, 0xf8, 0x00, 0x00},
  {0xff, 0xfc, 0x00, 0x00},
  {0xff, 0xfe, 0x00, 0x00},
  {0xff, 0xff, 0x00, 0x00},

  {0xff, 0xff, 0x80, 0x00},
  {0xff, 0xff, 0xc0, 0x00},
  {0xff, 0xff, 0xe0, 0x00},
  {0xff, 0xff, 0xf0, 0x00},
  {0xff, 0xff, 0xf8, 0x00},
  {0xff, 0xff, 0xfc, 0x00},
  {0xff, 0xff, 0xfe, 0x00},
  {0xff, 0xff, 0xff, 0x00},

  {0xff, 0xff, 0xff, 0x80},
  {0xff, 0xff, 0xff, 0xc0},
  {0xff, 0xff, 0xff, 0xe0},
  {0xff, 0xff, 0xff, 0xf0},
  {0xff, 0xff, 0xff, 0xf8},
  {0xff, 0xff, 0xff, 0xfc},
  {0xff, 0xff, 0xff, 0xfe}
};

const uint32 *maskbit321 = (uint32 *)mask32bit1;


static inline int32
prefix_match (struct prefix *n, struct prefix *p)
{
  int32 offset;
  int32 shift;

  /* Set both prefix's head pointer. */
  uint32 *np = (uint32 *)&n->u.prefix;
  uint32 *pp = (uint32 *)&p->u.prefix;

  /* If n's prefix is longer than p's one return 0. */
  if (n->prefixlen > p->prefixlen)
    return 0;

  offset = n->prefixlen / 32;
  shift =  n->prefixlen % 32;

  if (shift)
  {
    if (maskbit321[shift] & (np[offset] ^ pp[offset]))
      return 0;
  }

  while (offset--)
    if (np[offset] != pp[offset])
      return 0;
  return 1;
}

/*******************************************************************************
 * Name:   dhcpserver_dynamic_pool_confilict_check
 * Purpose: check dynamic pool configuration confilict or not
 * Input: 
 *   pInPool: the input pool
 * Output: 
 * Return:
 *   success: pool pointer
 *   fail:    NULL
 * Note: 
 ******************************************************************************/
dhcp_pool_t *dhcpserver_dynamic_pool_confilict_check(struct prefix *network, dhcp_pool_t *pInPool)
{
    dhcp_pool_t *pPool = NULL;
    dhcp_pool_t *pTmp = NULL;
    ctclib_hash_t *hash = g_dhcpserverPoolhash;
    ctclib_hash_backet_t *hb = NULL;
    int i;

    if (NULL == hash)
    {
        return NULL;
    }

    if (NULL == network)
    {
        return NULL;
    }
    
    for (i = 0; i < hash->size && !pPool; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = NULL;
            pTmp = (dhcp_pool_t *)hb->data;
            if (NULL != pTmp
                && dynPool == pTmp->pool_type 
                && sal_strcmp(pTmp->pool_name, pInPool->pool_name)
                && (prefix_match(&(pTmp->network), network) 
                    ||prefix_match(network, &(pTmp->network))))
            {
                pPool = pTmp;
                break;
            }
        }
    }
    
    return pPool;
}

#define	ETHER_ADDR_LEN	6

/*******************************************************************************
 * Name:   dhcpserver_static_pool_config
 * Purpose: configure a static pool
 * Input: 
 *   pInPool: static pool
 *   ip: static pool fix ip addr
 *   hw_addr: static pool hw addr
 *   hw_len: static pool hw addr len
 *   clientId: static pool client id
 *   clientId_len: static pool client id len
 *   clientId_type: static pool client id input type
 * Output: 
 * Return:
 *   success: interface pointer
 *   fail:    NULL
 * Note: 
 ******************************************************************************/
int dhcpserver_static_pool_config(dhcp_pool_t *pInPool,
    struct prefix *ip,
    unsigned char *hw_addr,
    unsigned char *clientId,
    unsigned clientId_len,
    unsigned clientId_type
)
{
    u_int8_t *tmp = NULL;

    if((NULL == pInPool)
        || (NULL == ip)
        || (NULL == hw_addr && NULL == clientId)
        || (NULL != hw_addr && NULL != clientId)
        || (NULL != clientId && clientId_len == 0))
    {
        return -1;
    }
    
    if(hw_addr)
    {
        if(pInPool->hw_addr)
        {
            sal_memcpy(pInPool->hw_addr, hw_addr, ETHER_ADDR_LEN+1);
        }
        else
        {
            tmp = XCALLOC(MTYPE_TMP, ETHER_ADDR_LEN+1);
            if(NULL == tmp)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, "WARNING: out of memory.");
                return -1;
            }
            
            pInPool->hw_addr = tmp;
            sal_memcpy(pInPool->hw_addr, hw_addr, ETHER_ADDR_LEN+1);
            pInPool->hw_len = ETHER_ADDR_LEN+1;
        }

        if(pInPool->clientId)
        {
            XFREE(MTYPE_TMP, pInPool->clientId);
            pInPool->clientId = NULL;
            pInPool->clientId_len = 0;
        }
        pInPool->clientId_type= DHCP_CLENTID_NONE;
    }

    if(clientId)
    {        
        if(pInPool->clientId && pInPool->clientId_len == clientId_len)
        {
            sal_memcpy(pInPool->clientId, clientId, clientId_len);
        }
        else
        {
            tmp = XCALLOC(MTYPE_TMP, clientId_len);
            if(NULL == tmp)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, "WARNING: out of memory.");
                return -1;
            }
            
            if(pInPool->clientId)
            {
                XFREE(MTYPE_TMP, pInPool->clientId);
                pInPool->clientId_len = 0;
                pInPool->clientId = NULL;
            }
            
            pInPool->clientId = tmp;
            sal_memcpy(pInPool->clientId, clientId, clientId_len);
            pInPool->clientId_len = clientId_len;
        }

        if(pInPool->hw_addr)
        {
            XFREE(MTYPE_TMP, pInPool->hw_addr);
            pInPool->hw_addr = NULL;
            pInPool->hw_len = 0;
        }
        
        pInPool->clientId_type= clientId_type;
    }
    
    pInPool->fixIp = *ip;    
    pInPool->pool_type = staticPool;
    return 0;
}

dhcp_option_t * dhcpserver_pool_option_lookup(dhcp_pool_t *pInPool, unsigned code)
{
    dhcp_option_t *cur = (dhcp_option_t *)0;

    if(NULL == pInPool)
    {
        return NULL;
    }

    for(cur = pInPool->dhcp_pool_common.options; cur && (cur->code <= code); cur = cur->next)
    {
        if(cur->code == code)
        {
            return cur;
        }
    }
    
    return NULL;
}

dhcp_option_t *dhcpserver_pool_option_get(unsigned code, 
    u_int8_t* val,
    u_int32_t len,
    enum dhcp_option_Input_type type
)
{
    dhcp_option_t *option = (dhcp_option_t *)0;

    option = XCALLOC(MTYPE_TMP, sizeof(dhcp_option_t));

    if(NULL == option)
    {
        return NULL;
    }

    if(val && 0 != len)
    {
        option->buf = XCALLOC(MTYPE_TMP, len);
        if(NULL == option->buf)
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "WARNING: out of memory.");
            XFREE(MTYPE_TMP, option);
            return NULL;
        }
        sal_memcpy(option->buf, val, len);
    }
    option->data_len = len;
    option->code = code;
    option->type = type;
    return option;
}

int dhcpserver_pool_option_add(dhcp_pool_t *pInPool, dhcp_option_t *option)
{
    dhcp_option_t *pre = (dhcp_option_t *)0;
    dhcp_option_t *cur = (dhcp_option_t *)0;
    
    if(NULL == pInPool || NULL == option)
    {
        return -1;
    }
    
    for(cur = pInPool->dhcp_pool_common.options; cur && cur->code < option->code; cur = cur->next)
    {
        pre = cur;
    }
    
    option->next = cur;
    
    if(pre)
    {
        pre->next = option;
    }
    else
    {
        pInPool->dhcp_pool_common.options = option;
    }
    
    return 0;
}

dhcp_option_t *dhcpserver_pool_option_remove(dhcp_pool_t *pInPool, unsigned code)
{
    dhcp_option_t *pre = (dhcp_option_t *)0;
    dhcp_option_t *cur = (dhcp_option_t *)0;

    if(NULL == pInPool)
    {
        return NULL;
    }

    for(cur = pInPool->dhcp_pool_common.options; cur && cur->code <= code; cur = cur->next)
    {
        if(cur->code == code)
        {
            if(pre)
            {
                pre->next = cur->next;
            }
            else
            {
                pInPool->dhcp_pool_common.options = cur->next;
            }
            cur->next = NULL;
            break;
        }
        pre = cur;
    }
    
    return cur;
}

void dhcpserver_pool_option_free(dhcp_option_t   **option)
{
    if(NULL == option || NULL == *option)
    {
        return;
    }

    if((*option)->buf)
    {
        XFREE(MTYPE_TMP, (*option)->buf);
    }
    
    XFREE(MTYPE_TMP, *option);
    
    *option = NULL;
    return;
}

#if 0
int dhcpserver_encode_pkt_msg(nsm_msg_dhcp_pkt_t *pmsg, u_int8_t **buf, u_int16_t *size,
    int (*pencode_func)(u_int8_t **, u_int16_t *, nsm_msg_dhcp_pkt_t *))
{
    int nbytes;

    if (NULL == pmsg || NULL == buf || NULL == size)
    {
        return -1;
    }

    //pal_assert(pmsg->pktType);
    //pal_assert(NULL != pmsg->clientip);
    //pal_assert(NULL != pmsg->mac);

    sal_memset(&pmsg->cindex, 0x00, sizeof(cindex_t));
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


int dhcpserver_send_ack_msg_to_snooping(struct lease *lease,
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

    if (!lease || lease->state->offer != DHCPACK || !ifp)
    {
        return RESULT_OK;
    }
    nch = nc->async;
    if (! nch || ! nch->up)
    {
        return -1;
    }
    sal_memset(&msg, 0, sizeof msg);
    
    msg.pktType = DHCP_PKT_ACK;
    msg.xid = lease->state->xid;
    msg.vid = atoi(&ifp->name[4]);
    msg.clientip_len = lease->ip_addr.len;
    msg.clientip = (u_char *)sal_malloc(msg.clientip_len);
    sal_memcpy (msg.clientip, lease->ip_addr.iabuf, msg.clientip_len);

    msg.mac_len = lease->hardware_addr.hlen -1;
    msg.mac = (u_int8_t*)sal_malloc(msg.mac_len);
    sal_memcpy (msg.mac, &lease->hardware_addr.hbuf[1], msg.mac_len);

    /* get from option*/
    oc = lookup_option (&dhcp_universe, lease->state ->options,
            DHO_DHCP_LEASE_TIME);
    if (oc)
    {
        sal_memcpy(&msg.lease_time, (u_int32_t *)oc->expression->data.const_data.data, sizeof (msg.lease_time));
        msg.lease_time = sal_ntohl (msg.lease_time);
    } 

    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    //nbytes = dhcpserver_encode_pkt_msg(&msg, &pnt, &size, nsm_encode_dhcp_pkt_msg);
    if (nbytes < 0)
    {
        goto err_out;
    }

    nsm_client_send_message(nch, 0, 0,
            NSM_MSG_DHCP_PKT_SEND, nbytes, &msg_id);

err_out:
    if (msg.clientip)
    {
        sal_free(msg.clientip);
    }

    if (msg.mac)
    {
        sal_free(msg.mac);
    }
 #endif
  
    return 0;        
}
/*for bug 20011 modified by liuyang 2012-8-22*/
int dhcpserver_send_nak_msg_to_snooping(struct dhcp_packet *packet, struct iaddr *cip,
                                                                    struct interface_info *ifp)
{
#if 0
    nsm_msg_dhcp_pkt_t msg;
    struct nsm_client *nc = ZG->nc;
    struct nsm_client_handler *nch;
    u_int32_t msg_id;
    u_char *pnt;
    u_int16_t size;
    int nbytes;
    
    if (!packet || !cip || !ifp)
        return RESULT_ERROR;

    nch = nc->async;
    if (! nch || ! nch->up)
    {
        return -1;
    }
    sal_memset(&msg, 0, sizeof msg);
    
    msg.pktType = DHCP_PKT_NAK;
    msg.clientip_len = cip->len;
    msg.vid = atoi(&ifp->name[4]);
    msg.clientip = (u_char *)sal_malloc(msg.clientip_len);
    sal_memcpy (msg.clientip, cip->iabuf, msg.clientip_len);
    msg.mac_len = packet->hlen;
    msg.mac = (u_int8_t*)sal_malloc(msg.mac_len);
    sal_memcpy (msg.mac, packet->chaddr, msg.mac_len);
    msg.xid = packet->xid;

    pnt = nch->buf + NSM_MSG_HEADER_SIZE;
    size = nch->len - NSM_MSG_HEADER_SIZE;
    //nbytes = dhcpserver_encode_pkt_msg(&msg, &pnt, &size, nsm_encode_dhcp_pkt_msg);
    if (nbytes < 0)
    {
        goto err_out;
    }

    nsm_client_send_message(nch, 0, 0,
            NSM_MSG_DHCP_PKT_SEND, nbytes, &msg_id);

err_out:
    if (msg.clientip)
    {
        sal_free(msg.clientip);
    }

    if (msg.mac)
    {
        sal_free(msg.mac);
    }
#endif
   
    return 0;      
}

void dhcpserver_setup_send_interface(void)
{
    isc_result_t status;
    int sockfd = -1;
    uint8 i = 1;
    struct interface_info *fbi = (struct interface_info *)0;
    
    status = interface_allocate (&ctc_fallback_interface, MDL);
    if (status != ISC_R_SUCCESS)
        log_fatal ("Error allocating fallback interface: %s",
            isc_result_totext (status));
    
    strcpy (ctc_fallback_interface -> name, "ctc_fallback");
    status = interface_reference (&fbi, ctc_fallback_interface, MDL);
    ctc_fallback_interface -> index = -1;
    //interface_stash (ctc_fallback_interface);

    /* Make a socket... */
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0)
    {
        log_fatal("Can't create dhcp socket: %m");
    }

    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &i, sizeof i) < 0)
    {
        log_fatal("Can't set IP_HDRINCL for send interface.");
    }

    fbi->wfdesc = sockfd;

    interface_dereference (&fbi, MDL);
}

int dhcpserver_send_packet(struct interface_info *interface,
                        struct packet *packet, struct dhcp_packet *raw, size_t len,
                        struct in_addr from, struct sockaddr_in *to, struct hardware *hto)
{
    send_packet (interface, packet, raw, len, from, to, hto);

/* modified by liwh for bug 49086, 2018-09-18 */
#if 0
    unsigned ibufp = 0;
    //double hh [16];
    double ih [1536 / sizeof (double)];
    unsigned char *buf = (unsigned char *)ih;
    struct sockaddr sa;
    int result = -1;
   // int fudge;

/* modified by liwh for bug 49086, 2018-09-18 */
    /*if (strcmp (interface -> name, "ctc_fallback"))
        return result;*/
/* liwh end */

    assemble_udp_ip_header (interface, buf, &ibufp, from.s_addr,
                to -> sin_addr.s_addr, to -> sin_port,
                (unsigned char *)raw, len);
    memcpy (buf + ibufp, raw, len);

    /* For some reason, SOCK_PACKET sockets can't be connected,
       so we have to do a sentdo every time. */
    memset (&sa, 0, sizeof sa);
    memcpy((struct sockaddr_in*)&sa, to, sizeof(struct sockaddr_in));    

#ifndef _CENTEC_
       result = sendto (interface -> wfdesc,
             buf , ibufp + len, 0, &sa, sizeof sa);
#else
        result = ipc_pkt_tx_dhcp((uint8 *)( buf), (ibufp + len),
                interface->ifindex, 0, 0);
#endif

    if (result < 0)
        log_error ("dhcpserver_send_packet: %m");
    return result;
#endif
    return 0;    
}

int isc_dhcpserver_shared_network_name_get(char* name, u_int8_t len, struct prefix *network)
{
    struct iaddr iaddr_ip, iaddr_mask;
    struct sal_in4_addr in4_mask;
    char* str = NULL;
    if(NULL == name || NULL == network)
    {
        return -1;
    }
    
    iaddr_ip.len = 4;
    sal_memcpy(iaddr_ip.iabuf, &network->u.prefix4, 4);
    
    sal_masklen2ip(network->prefixlen, &in4_mask);
    iaddr_mask.len = 4;
    sal_memcpy(iaddr_mask.iabuf, &in4_mask, 4);
        
    str = dhcpserver_piaddrmaskv4(iaddr_ip, iaddr_mask);
    sal_memset(name, 0, len);
    
    sal_strcpy(name, str);
    return 0;
}

int isc_dhcpserver_ip_forbid(struct iaddr ip_addr)
{
    dhcp_exclude_ip_range_t *p = g_dhcpserverExcludeIpList;
    u_int32_t addr = sal_ntohl(*(u_int32_t*)(ip_addr.iabuf));
    
    while(p)
    {
        if(0 == sal_ntohl(p->endip.s_addr) && addr == sal_ntohl(p->startip.s_addr))
        {
            return 1;
        }
        else if(addr >= sal_ntohl(p->startip.s_addr) && addr <= sal_ntohl(p->endip.s_addr))
        {
            return 1;
        }
        p = p->next;
    }
    
    return 0;
}

dhcp_pool_t *isc_dhcpserver_ip_static(struct iaddr ip_addr)
{
    dhcp_pool_t *pTmp = NULL;
    ctclib_hash_t *hash = g_dhcpserverPoolhash;
    ctclib_hash_backet_t *hb = NULL;
    int i;
    u_int32_t addr = sal_ntohl(*(u_int32_t*)(ip_addr.iabuf));

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = hb->next)
        {
            pTmp = NULL;
            pTmp = (dhcp_pool_t *)hb->data;
            if (addr == sal_ntohl(pTmp->fixIp.u.prefix4.s_addr))
            {
                   return pTmp;
            }
        }
    }

    return NULL;
}
 
void isc_dhcpserver_lease_into_pool(struct pool *p)
{
    int i;
    struct lease *l;
    struct lease **lptr[RESERVED_LEASES+1];
    
    
    if(NULL == p)
        return;

    
    pool_timer (p);
    
    p -> lease_count = 0;
    p -> free_leases = 0;
    p -> backup_leases = 0;

    lptr [FREE_LEASES] = &p -> free;
    lptr [ACTIVE_LEASES] = &p -> active;
    lptr [EXPIRED_LEASES] = &p -> expired;
    lptr [ABANDONED_LEASES] = &p -> abandoned;
    lptr [BACKUP_LEASES] = &p -> backup;
    lptr [RESERVED_LEASES] = &p->reserved;

    for (i = FREE_LEASES; i <= RESERVED_LEASES; i++)
    {
        for (l = *(lptr [i]); l; l = l -> next)
        {
            p -> lease_count++;
            if (l -> ends <= cur_time)
            {
                if (l->binding_state == FTS_FREE)
                {
                    if (i == FREE_LEASES)
                        p->free_leases++;
                    else if (i != RESERVED_LEASES)
                    {
                        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Impossible case "
                              "at %s:%d.", MDL);
                    }
                }
                else if (l->binding_state == FTS_BACKUP)
                {
                    if (i == BACKUP_LEASES)
                        p->backup_leases++;
                    else if (i != RESERVED_LEASES)
                    {
                        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Impossible case "
                              "at %s:%d.", MDL);
                    }
                }
            }
#if defined (FAILOVER_PROTOCOL)
            if (p -> failover_peer &&
                    l -> tstp > l -> atsfp &&
                    !(l -> flags & ON_UPDATE_QUEUE))
            {
                l -> desired_binding_state = l -> binding_state;
                dhcp_failover_queue_update (l, 1);
            }
#endif
        }
    }

    return;
}

isc_result_t isc_dhcpserver_new_address_range (low, high, subnet, pool)
    struct iaddr low, high;
    struct subnet *subnet;
    struct pool *pool;
{
#if defined (COMPACT_LEASES)
    struct lease *address_range;
#endif
    unsigned min, max, i, cnt;
    char lowbuf [16], highbuf [16], netbuf [16];
    struct shared_network *share = subnet -> shared_network;
    struct lease *lt = (struct lease *)0;
#if !defined(COMPACT_LEASES)
    unsigned j;
    isc_result_t status;
#endif

    /* All subnets should have attached shared network structures. */
    if (!share)
    {
        strcpy (netbuf, piaddr (subnet -> net));
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "No shared network for network %s (%s)",
            netbuf, piaddr (subnet ->netmask));
        return ISC_R_INVALIDARG;
    }

    /* Initialize the hash table if it hasn't been done yet. */
    if (!lease_uid_hash)
    {
        if (!lease_id_new_hash(&lease_uid_hash, LEASE_HASH_SIZE, MDL))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't allocate lease/uid hash");
            return ISC_R_NOMEMORY;
        }
    }
    
    if (!lease_ip_addr_hash)
    {
        if (!lease_ip_new_hash(&lease_ip_addr_hash, LEASE_HASH_SIZE,
                       MDL))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't allocate lease/ip hash");
            return ISC_R_NOMEMORY;
        }
    }
    
    if (!lease_hw_addr_hash)
    {
        if (!lease_id_new_hash(&lease_hw_addr_hash, LEASE_HASH_SIZE,
                       MDL))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't allocate lease/hw hash");
            return ISC_R_NOMEMORY;
        }
    }

    /* Make sure that high and low addresses are in this subnet. */
    if (!addr_eq(subnet->net, subnet_number(low, subnet->netmask)))
    {
        strcpy(lowbuf, piaddr(low));
        strcpy(netbuf, piaddr(subnet->net));
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "bad range, address %s not in subnet %s netmask %s",
              lowbuf, netbuf, piaddr(subnet->netmask));
        return ISC_R_INVALIDARG;
    }

    if (!addr_eq(subnet->net, subnet_number(high, subnet->netmask)))
    {
        strcpy(highbuf, piaddr(high));
        strcpy(netbuf, piaddr(subnet->net));
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "bad range, address %s not in subnet %s netmask %s",
              highbuf, netbuf, piaddr(subnet->netmask));
        return ISC_R_INVALIDARG;
    }

    /* Get the high and low host addresses... */
    max = host_addr (high, subnet -> netmask);
    min = host_addr (low, subnet -> netmask);

    /* Allow range to be specified high-to-low as well as low-to-high. */
    if (min > max)
    {
        max = min;
        min = host_addr (high, subnet -> netmask);
    }
    
    /*memory protect, only support MAX_LEASE_PER_POOL_CNT leases */
    cnt = max - min +1;
        
    if(cnt > MAX_LEASE_PER_POOL_CNT)
    {
        max = min + MAX_LEASE_PER_POOL_CNT -1;
    }
    /* Get a lease structure for each address in the range. */
#if defined (COMPACT_LEASES)
    address_range = new_leases (max - min + 1, MDL);
    if (!address_range)
    {
        strcpy (lowbuf, piaddr (low));
        strcpy (highbuf, piaddr (high));
        log_fatal ("No memory for address range %s-%s.", lowbuf, highbuf);
        return ISC_R_NOMEMORY;
    }
#endif

    /* Fill out the lease structures with some minimal information. 
       ** not include broadcast address and net address by liuyang
       */
    for (i = max - min + 1; i > 0; i--)
    {
        struct lease *lp = (struct lease *)0;
#if defined (COMPACT_LEASES)
        omapi_object_initialize ((omapi_object_t *)&address_range [i],
                     dhcp_type_lease,
                     0, sizeof (struct lease), MDL);
        lease_reference (&lp, &address_range [i], MDL);
#else
        status = lease_allocate (&lp, MDL);
        if (status != ISC_R_SUCCESS)
        {
            j = i;
            log_sys(M_MOD_DHCPRLY6, E_ERROR,"No memory for lease %s",
                piaddr (ip_addr (subnet -> net,
                    subnet -> netmask,
                    i + min)));
            goto error_handle;
        }
#endif
        lp -> ip_addr = ip_addr (subnet -> net,
                     subnet -> netmask, i);
        lp -> starts = MIN_TIME;
        lp -> ends = MIN_TIME;
        subnet_reference (&lp -> subnet, subnet, MDL);
        pool_reference (&lp -> pool, pool, MDL);
        lp -> binding_state = FTS_FREE;
        lp -> next_binding_state = FTS_FREE;
        lp -> flags = 0;

        /* Remember the lease in the IP address hash. */
        if (find_lease_by_ip_addr (&lt, lp -> ip_addr, MDL))
        {
            if (lt -> pool)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR, "lease %s is declared twice!", piaddr (lp -> ip_addr));
            } 
            else
                pool_reference (&lt -> pool, pool, MDL);
            lease_dereference (&lt, MDL);
        }
        else
            lease_ip_hash_add(lease_ip_addr_hash,
                      lp->ip_addr.iabuf, lp->ip_addr.len,
                      lp, MDL);
        
        lease_instantiate(NULL, 0, lp);
        lease_dereference (&lp, MDL);
    }
    
    isc_dhcpserver_lease_into_pool(pool);
    
    strcpy (lowbuf, piaddr (low));
    strcpy (highbuf, piaddr (high));
    log_error ("new address range %s-%s.",lowbuf, highbuf);
    return ISC_R_SUCCESS;
    
#if !defined(COMPACT_LEASES)
error_handle:
        for (i = 0; i < j; i++)
        {
            struct lease temp;
            memset(&temp, 0, sizeof(struct lease));
            temp.ip_addr = ip_addr (subnet -> net,
                         subnet -> netmask, j + min);
            subnet_reference (&lt-> subnet, subnet, MDL);
            pool_reference (&lt-> pool, pool, MDL);
    
            /* Remember the lease in the IP address hash. */
            if (find_lease_by_ip_addr (&lt, temp.ip_addr, MDL))
            {
                if(lt -> subnet)
                    subnet_dereference (&lt -> subnet,MDL);
                if(lt -> pool)
                    pool_reference (&lt -> pool, pool, MDL);
            }
            lease_ip_hash_delete(lease_ip_addr_hash,
                         temp.ip_addr.iabuf, temp.ip_addr.len,
                         MDL);
            lease_dereference(&lt, MDL);
        }
    return status;
#endif
}

int isc_dhcpserver_ipv4_addr_range_get(struct prefix_ipv4 *network, struct sal_in4_addr *low, struct sal_in4_addr *high)
{
              
    struct sal_in4_addr netmask, netaddr, lowaddr, highaddr;
    sal_masklen2ip (network->prefixlen, &netmask);
    
    netmask.s_addr = sal_ntohl(netmask.s_addr);
    netaddr.s_addr = sal_ntohl(network->prefix.s_addr);
    lowaddr.s_addr = (netaddr.s_addr & netmask.s_addr)+1;
    highaddr.s_addr = (netaddr.s_addr | (~netmask.s_addr))-1;
    
    low->s_addr = sal_ntohl(lowaddr.s_addr);
    high->s_addr = sal_ntohl(highaddr.s_addr);
    
    return 0;
}

isc_result_t isc_dhcpserver_pool_declaration(dhcp_pool_t *dhcp_pool, struct group *group)
{
    struct sal_in4_addr low_ip, high_ip;
    struct iaddr low, high;
    struct subnet *subnet= (struct subnet *)0;
    struct shared_network *share = (struct shared_network *)0;
    struct pool *pool= (struct pool *)0;
    struct permit *p = (struct permit *)0;
    isc_result_t status = ISC_R_SUCCESS;

    sal_memset(&low_ip, 0, sizeof(struct sal_in4_addr));
    sal_memset(&high_ip, 0, sizeof(struct sal_in4_addr));
    isc_dhcpserver_ipv4_addr_range_get((struct prefix_ipv4 *)&(dhcp_pool->network),
        &low_ip, 
        &high_ip);

    sal_memset(&low, 0, sizeof(struct iaddr));
    low.len = 4;
    sal_memcpy(low.iabuf, &low_ip, 4);
    
    sal_memset(&high, 0, sizeof(struct iaddr));
    high.len = 4;
    sal_memcpy(high.iabuf, &high_ip, 4);
    
    subnet = group -> subnet;
    share = subnet -> shared_network;

    status = pool_allocate (&pool, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "no memory for ad-hoc pool: %s",
            isc_result_totext (status));
        return ISC_R_NOMEMORY;
    }
    p = new_permit (MDL);
    if (!p)
    {
        pool_dereference(&pool, MDL);
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "no memory for ad-hoc permit.");
        return ISC_R_NOMEMORY;
    }
    p -> type = permit_dynamic_bootp_clients;
    pool -> prohibit_list = p;

    pool_reference (&share -> pools, pool, MDL);
    
    shared_network_reference (&pool -> shared_network,
                  share, MDL);
    if (!clone_group (&pool -> group, share -> group, MDL))
    {
    
        pool_dereference (&share -> pools,MDL);
        shared_network_dereference(&pool -> shared_network, MDL);
        pool_dereference(&pool, MDL);
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "no memory for anon pool group.");
        return ISC_R_NOMEMORY;
    }

    /* Create the new address range... */
    status = isc_dhcpserver_new_address_range (low, high, subnet, pool);
    if (status != ISC_R_SUCCESS)
    {
        group_dereference(&pool -> group->next, MDL);
        group_dereference(&pool -> group, MDL);        
        pool_dereference (&share -> pools,MDL);        
        shared_network_dereference(&pool -> shared_network, MDL);
        pool_dereference(&pool, MDL);
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "Create the new address range for pool fail: %s",
            isc_result_totext (status));
        return status;
    }
    
    pool_dereference(&pool, MDL);
    return ISC_R_SUCCESS;
}

isc_result_t isc_dhcpserver_options_declaration(dhcp_pool_t *dhcp_pool, struct group *group)
{
    dhcp_option_t *option = (dhcp_option_t *)0;
    u_int32_t len = 0;
    int terminated = 0;
    
    if(NULL == dhcp_pool || NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Invalid argument for dhcp option declaration");
        return ISC_R_INVALIDARG;
    }
    
    if(NULL != dhcp_pool->dhcp_pool_common.domain_name)
    {
        len = strlen(dhcp_pool->dhcp_pool_common.domain_name);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_DOMAIN_NAME,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.domain_name,
            len, 1))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding domain name option fail");
        }
    }
    
    if(NULL != dhcp_pool->dhcp_pool_common.bootfile_name)
    {
        len = strlen(dhcp_pool->dhcp_pool_common.bootfile_name);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_BOOTFILE_NAME,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.bootfile_name,
            len, 1))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding bootfile name option fail");
        }
    }

    if(UNKNOWN_NODE != dhcp_pool->dhcp_pool_common.netbios_node_type)
    {
        if(isc_dhcpserver_option_add(group, 
            DHO_NETBIOS_NODE_TYPE,
            &dhcp_pool->dhcp_pool_common.netbios_node_type,
            1, 0))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding netbios nodt type option fail");
        }
    }
    
    if(NULL != dhcp_pool->dhcp_pool_common.dns_server)
    {
        len = dhcp_pool->dhcp_pool_common.n_dns_server*sizeof(struct sal_in4_addr);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_DOMAIN_NAME_SERVERS,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.dns_server,
            len, 0))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding dns server option fail");
        }
    }
    
    if(NULL != dhcp_pool->dhcp_pool_common.gateway)
    {
        len = dhcp_pool->dhcp_pool_common.n_gateway*sizeof(struct sal_in4_addr);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_ROUTERS,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.gateway,
            len, 0))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding gateway option fail");
        }
    }
    
    if(NULL != dhcp_pool->dhcp_pool_common.netbios_server)
    {
        len = dhcp_pool->dhcp_pool_common.n_netbios_server*sizeof(struct sal_in4_addr);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_NETBIOS_NAME_SERVERS,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.netbios_server,
            len, 0))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding netbios server option fail");
        }
    }

    if(NULL != dhcp_pool->dhcp_pool_common.tftp_server_addr)
    {
        len = dhcp_pool->dhcp_pool_common.n_tftp_server_addr*sizeof(struct sal_in4_addr);
        
        if(isc_dhcpserver_option_add(group, 
            DHO_TFTP_SERVER_ADDRS,
            (u_int8_t *)dhcp_pool->dhcp_pool_common.tftp_server_addr,
            len, 0))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding next server option fail");
        }
    }

    for(option = dhcp_pool->dhcp_pool_common.options; option; option = option->next)
    {
        switch(option->type)
        {
            case ASCII_TYPE:
                terminated = 1;
                break;
            case HEX_TYPE:
            case IP_TYPE:
            default:
                terminated = 0;
                break;
        }
        
        if(isc_dhcpserver_option_add(group, 
            option->code,
            (u_int8_t *)option->buf,
            option->data_len, terminated))
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, "Adding option %d fail", option->code);
        }
    }

    return ISC_R_SUCCESS;
}

isc_result_t isc_dhcpserver_common_subnet_declaration(dhcp_pool_t *dhcp_pool, struct shared_network *share, struct subnet *subnet)
{
    isc_result_t status = ISC_R_SUCCESS;

    enter_subnet(subnet);
    
    status = isc_dhcpserver_pool_declaration(dhcp_pool, subnet->group);
    if(ISC_R_SUCCESS != status)
    {
        return status;
    }
    
    status = isc_dhcpserver_options_declaration(dhcp_pool, subnet->group);
    
    if(ISC_R_SUCCESS != status)
    {
        isc_dhcpserver_remove_pool_content(share->pools);
        return status;
    }
    
    /* isc dhcp server do not support option 51(lease), we config it here */
    if(0 != dhcp_pool->dhcp_pool_common.lease_time)
    {
        share->pools->lease_time = (TIME)(dhcp_pool->dhcp_pool_common.lease_time);
    }
    
    subnet_reference(&share->subnets, subnet, MDL);
    
    return ISC_R_SUCCESS;
}

isc_result_t isc_dhcpserver_subnet_declaration(dhcp_pool_t *dhcp_pool, struct shared_network *share)
{
    struct subnet *subnet = (struct subnet *)0;
    struct sal_in4_addr mask;
    isc_result_t status = ISC_R_SUCCESS;

    status = subnet_allocate (&subnet, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "Allocation of new subnet failed: %s",
            isc_result_totext (status));
        return ISC_R_NOMEMORY;
    }
    
    shared_network_reference (&subnet -> shared_network, share, MDL);

    group_reference(&subnet->group, share->group, MDL);
    subnet_reference (&subnet -> group -> subnet, subnet, MDL);

    subnet -> net.len = 4;
    sal_memcpy(subnet -> net.iabuf, &dhcp_pool->network.u.prefix4, 4);
    
    subnet -> netmask.len = 4;
    sal_masklen2ip (dhcp_pool->network.prefixlen, &mask);
    sal_memcpy(subnet -> netmask.iabuf, &mask, 4);

    /* Validate the network number/netmask pair. */
    if (host_addr (subnet -> net, subnet -> netmask))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "bad subnet number/mask combination for subnet %s.",
            piaddr (subnet -> net));
        subnet_dereference (&subnet -> group -> subnet, MDL);
        group_dereference(&subnet->group, MDL);
        shared_network_dereference (&subnet -> shared_network, MDL);
        subnet_dereference (&subnet, MDL);
        return ISC_R_FAILURE;
    }
    
    status = isc_dhcpserver_common_subnet_declaration(dhcp_pool, share, subnet);
    if (status != ISC_R_SUCCESS)
    {
        subnet_dereference (&subnet -> group -> subnet, MDL);
        group_dereference(&subnet->group, MDL);
        shared_network_dereference (&subnet -> shared_network, MDL);
        subnet_dereference (&subnet, MDL);
        return status;
    }
    
    subnet_dereference(&subnet, MDL);
    return ISC_R_SUCCESS;
}

isc_result_t isc_dhcpserver_add_dynamic_pool(dhcp_pool_t *dhcp_pool, struct group *group)
{
    struct shared_network *share;
    isc_result_t status = ISC_R_SUCCESS;
    char *n;
    
    if(NULL == dhcp_pool || dynPool != dhcp_pool->pool_type || NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "invalid argument for adding dynamic pool");
        return ISC_R_INVALIDARG;
    }
    
    share = (struct shared_network*)0;
    
    status = shared_network_allocate(&share, MDL);
    if (status != ISC_R_SUCCESS)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "Can't allocate shared subnet: %s",
            isc_result_totext (status));
        return ISC_R_NOMEMORY;
    }
    
    if (!clone_group (&share -> group, group, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "Can't allocate group for shared net");
        shared_network_dereference (&share, MDL);
        return ISC_R_NOMEMORY;
    }
    
    shared_network_reference (&share -> group -> shared_network,
                  share, MDL);

    share->flags |= SHARED_IMPLICIT;

    status = isc_dhcpserver_subnet_declaration(dhcp_pool, share);
    
    if (status != ISC_R_SUCCESS)
    {        
        shared_network_dereference (&share -> group -> shared_network, MDL);
        group_dereference(&share -> group->next, MDL);
        group_dereference(&share -> group, MDL);
        shared_network_dereference (&share, MDL);
        return status;
    }
    
    /* share -> subnets is the subnet we just parsed. */
    if (share->subnets)
    {
        if(share->subnets->interface)
        {
            interface_reference(&share->interface,
                        share->subnets->interface,
                        MDL);
        }
        /* Make the shared network name from network number. */
        n = dhcpserver_piaddrmaskv4(share->subnets->net,
                   share->subnets->netmask);
        
        if(n != NULL)
        {
            share->name = strdup(n);
        }
        
        if (share->name == NULL)
        {
            if(n != NULL)
            {
                log_sys(M_MOD_DHCPRLY6, E_ERROR,
                    "Out of memory allocating default "
                    "shared network name (\"%s\").", n);
            }
            if(share->subnets->interface)
            {
                interface_dereference(&share->interface, MDL);
            }
            shared_network_dereference (&share -> group -> shared_network, MDL);
            group_dereference(&share -> group->next, MDL);
            group_dereference(&share -> group, MDL);
            shared_network_dereference (&share, MDL);
            return status;
        }
        
        /* Copy the authoritative parameter from the subnet,
           since there is no opportunity to declare it here. */
        share->group->authoritative =
            share->subnets->group->authoritative;
        enter_shared_network(share);
    }
    shared_network_dereference(&share, MDL);
    
    return status;
}

isc_result_t isc_dhcpserver_add_static_pool(dhcp_pool_t *dhcp_pool, struct group *group)
{
    struct host_decl *host = (struct host_decl *)0;
    isc_result_t status = ISC_R_SUCCESS;
    unsigned len = 0;
    
    if(NULL == dhcp_pool 
        || staticPool != dhcp_pool->pool_type
        || NULL == group
        || NULL == dhcp_pool->pool_name
        || (NULL == dhcp_pool->clientId && NULL == dhcp_pool->hw_addr)
        || (dhcp_pool->clientId && dhcp_pool->hw_addr))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "invalid argument for adding static pool");
        return ISC_R_INVALIDARG;
    }
    
    status = host_allocate(&host, MDL);
    
    if (status != ISC_R_SUCCESS)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "can't allocate host decl struct %s: %s",
            dhcp_pool->pool_name, isc_result_totext (status));
        return status;
    }
    
    host -> name = (char *)dmalloc (strlen(dhcp_pool->pool_name) + 1, MDL);
    sal_strcpy(host -> name, dhcp_pool->pool_name);
    
    if (!clone_group (&host -> group, group, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "can't clone group for host %s", dhcp_pool->pool_name);
        host_dereference (&host, MDL);
        return ISC_R_NOMEMORY;
    }
    

    if(dhcp_pool->clientId)
    {
        len = dhcp_pool->clientId_len;
        host -> client_identifier.terminated = 0;
        
        if (!buffer_allocate
            (&host -> client_identifier.buffer,
            len+ host -> client_identifier.terminated, MDL))
        {
            group_dereference (&host->group->next, MDL);
            group_dereference (&host->group, MDL);
            host_dereference (&host, MDL);
            log_sys(M_MOD_DHCPRLY6, E_ERROR,
                "no memory for uid for host %s.", dhcp_pool->pool_name);
            return ISC_R_NOMEMORY;
        }
        
        host -> client_identifier.data = host -> client_identifier.buffer->data;
        host -> client_identifier.len = len;
        memcpy (host -> client_identifier.buffer->data,
            dhcp_pool->clientId,
            len + host -> client_identifier.terminated);
    }
    
    if(dhcp_pool->hw_addr)
    {
        sal_memset(&host -> interface, 0, sizeof(struct hardware));
        host -> interface.hlen = dhcp_pool->hw_len;
        sal_memcpy(&host->interface.hbuf, dhcp_pool->hw_addr, dhcp_pool->hw_len);
    }
    
    status = isc_dhcpserver_fixip_declaration(dhcp_pool, host);
    if (status != ISC_R_SUCCESS)
    {
        group_dereference (&host->group->next, MDL);
        group_dereference (&host->group, MDL);
        host_dereference (&host, MDL);
        if (host -> client_identifier.buffer)
        {
            buffer_dereference(&host -> client_identifier.buffer, MDL);
        }
        return status;
    }
    
    host->flags |= HOST_DECL_STATIC;
    
    isc_dhcpserver_options_declaration(dhcp_pool, host->group);
    
    enter_host (host, 0, 0);
    host_dereference (&host, MDL);
    return ISC_R_SUCCESS;
}

isc_result_t isc_dhcpserver_fixip_declaration(dhcp_pool_t *dhcp_pool, struct host_decl *host)
{
    struct expression *exp = (struct expression *)0;
    struct option_cache *oc = (struct option_cache *)0;
    unsigned char addr [4];
    unsigned len = 0;
    
    if(NULL == host)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "invalid argument for fix ip declaration");
        return ISC_R_INVALIDARG;
    }
    sal_memcpy(addr, &dhcp_pool->fixIp.u.prefix4.s_addr, 4);
    len = 4;
    
    if (!make_const_data (&exp, addr, len, 0, 1, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,  "No memory for option const data");
        return ISC_R_NOMEMORY;
    }
        
    if(!option_cache(&oc, (struct data_string *)0, exp,
            (struct option *)0, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "No memory for option cache");
        buffer_dereference (&exp->data.const_data.buffer, MDL);
        expression_dereference(&exp, MDL);
        return ISC_R_NOMEMORY;
    }
    
    host->fixed_addr = oc;
    return ISC_R_SUCCESS;
}

/* for bug 28374 modified by liuyang 2014-4-28*/
int isc_dhcpserver_ip_lease_config(struct lease *lease, u_int8_t action)
{
    struct lease *lp = (struct lease *)0;
    isc_result_t status = ISC_R_SUCCESS;
    
    if(NULL == lease || NULL == lease->pool)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "invalid argument for lease config");
        return -1;
    }
    
    if(FTS_FREE == lease->binding_state)
    {
        return 0;
    }
    
    switch(action)
    {
        case LEASE_ACTION_RELEASE_ABANDON:
            if(FTS_ABANDONED != lease->binding_state)
                return -1;
            break;
        case LEASE_ACTION_RELEASE_ACTIVE:
            if(FTS_ACTIVE != lease->binding_state)
                return -1;
            break;
        default:
            return -1;
    }
    
    status = lease_allocate (&lp, MDL);
    
    if(ISC_R_SUCCESS != status)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "No memory for lease supersede");
        return -1;
    }
    
    lp ->starts = MIN_TIME;
    lp ->ends = MIN_TIME;
    lp ->binding_state = FTS_FREE;
    lp ->next_binding_state = FTS_FREE;
    lp ->flags = 0;
    lp ->ip_addr = lease ->ip_addr;
    
    if(lease ->subnet)
        subnet_reference (&lp ->subnet, lease ->subnet, MDL);
    
    if(lease ->pool)
        pool_reference (&lp ->pool, lease ->pool, MDL);
    
    if (lease ->state)
    {
        free_lease_state (lease ->state, MDL);
        lease ->state = (struct lease_state *)0;
    
        cancel_timeout (lease_ping_timeout, lease);
        --outstanding_pings;
        log_error (" free lease: %s, cancle ping timer.",
            piaddr (lease ->ip_addr));
    }
    
    supersede_lease (lease, lp, 1, 1, 1);
    lease_dereference(&lp, MDL);
    
    return 0;
}

int isc_dhcpserver_pool_lease_config(struct pool *pool, u_int8_t action)
{
    isc_result_t status = ISC_R_SUCCESS;
    struct lease *lease = (struct lease *)0;;
    struct lease *lp = (struct lease *)0;
    struct lease *next = (struct lease *)0;
    
    if(NULL == pool)
    {
        return -1;
    }

    switch(action)
    {
        case LEASE_ACTION_RELEASE_ABANDON:
            lease = pool ->abandoned;
            break;
        case LEASE_ACTION_RELEASE_ACTIVE:
            lease = pool ->active;
            break;
        default:
            return -1;
    }
    
    status = lease_allocate (&lp, MDL);
    
    if(ISC_R_SUCCESS != status)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "No memory for lease supersede");
        return -1;
    }
    
    next = lease;    
    while(lease)
    {
        next = lease->next;
        lp ->starts = MIN_TIME;
        lp ->ends = MIN_TIME;
        lp ->binding_state = FTS_FREE;
        lp ->next_binding_state = FTS_FREE;
        lp ->flags = 0;
        lp ->ip_addr = lease ->ip_addr;

        if(lp ->subnet)
        {
            subnet_dereference (&lp ->subnet, MDL);
            subnet_reference (&lp ->subnet, lease ->subnet, MDL);
        }

        if(lp ->pool)
        {
            pool_dereference (&lp ->pool, MDL);
            pool_reference (&lp ->pool, lease ->pool, MDL);
        }

        if (lease ->state)
        {
            free_lease_state (lease ->state, MDL);
            lease ->state = (struct lease_state *)0;

            cancel_timeout (lease_ping_timeout, lease);
            --outstanding_pings;
            log_error (" free lease: %s, cancle ping timer.",
                piaddr (lease ->ip_addr));
        }

        supersede_lease (lease, lp, 1, 1, 1);
        lease = next;        
    }

    lease_dereference(&lp, MDL);
    return 0;
}

int isc_dhcpserver_all_lease_config(u_int8_t action)
{
    struct shared_network *s = (struct shared_network *)0;
    struct pool *p = (struct pool *)0;
    
    for (s = shared_networks; s; s = s ->next) 
    {
        for (p = s ->pools; p; p = p ->next) 
        {
            if(isc_dhcpserver_pool_lease_config(p, action))
            {
                return -1;
            }
        }
    }
    
    return 0;
}

void isc_dhcpserver_remove_pool_content(struct pool *pool)
{
    struct lease *cur = (struct lease *)0;
    struct lease *next = (struct lease *)0;
    struct permit *pc = (struct permit *)0;
    struct permit *pn = (struct permit *)0;
    
    if(NULL == pool)
    {
        return;
    }
        
    if (pool -> next)
        pool_dereference (&pool -> next, MDL);
    
    if (pool -> group)
    {
        if (pool -> group->next)
        {
            group_dereference (&pool -> group->next, MDL);
        }
        group_dereference(&pool -> group, MDL);
    }
    
    if (pool -> shared_network)
        shared_network_dereference(&pool -> shared_network, MDL);
    
    if (pool -> active)
    {
        for (cur = pool -> active; cur; cur = next)
        {
            lease_ip_hash_delete(lease_ip_addr_hash, cur->ip_addr.iabuf,
                         cur->ip_addr.len, MDL);
            next = cur->next;
            lease_dereference (&cur, MDL);
        }
    }
    
    if (pool -> expired)
    {
        for (cur = pool -> expired; cur; cur = next)
        {
            lease_ip_hash_delete(lease_ip_addr_hash, cur->ip_addr.iabuf,
                         cur->ip_addr.len, MDL);
            next = cur->next;
            lease_dereference (&cur, MDL);
        }
    }
    
    if (pool -> free)
    {
        for (cur = pool -> free; cur; cur = next)
        {            
            lease_ip_hash_delete(lease_ip_addr_hash, cur->ip_addr.iabuf,
                         cur->ip_addr.len, MDL);
            next = cur->next;
            lease_dereference (&cur, MDL);
        }
    }
    
    if (pool -> backup)
    {
        for (cur = pool -> backup; cur; cur = next)
        {
            lease_ip_hash_delete(lease_ip_addr_hash, cur->ip_addr.iabuf,
                         cur->ip_addr.len, MDL);
            next = cur->next;
            lease_dereference (&cur, MDL);
        }
    }
    
    if (pool -> abandoned)
    {
        for (cur = pool -> abandoned; cur; cur = next)
        {
            lease_ip_hash_delete(lease_ip_addr_hash, cur->ip_addr.iabuf,
                         cur->ip_addr.len, MDL);
            next = cur->next;
            lease_dereference (&cur, MDL);
        }
    }
    
    for (pc = pool -> permit_list; pc; pc = pn)
    {
        pn = pc -> next;
        free_permit (pc, MDL);
    }
    
    pool -> permit_list = (struct permit *)0;

    for (pc = pool -> prohibit_list; pc; pc = pn)
    {
        pn = pc -> next;
        free_permit (pc, MDL);
    }
    return;
}

void isc_dhcpserver_remove_subnet_content(struct subnet *subnet)
{
    struct subnet *tmp = subnets;
    struct subnet *cur = subnets;
    struct subnet *pre = NULL;
    if(NULL == subnet)
    {
        return;
    }
    
    for (cur = subnets; cur; cur = cur -> next_subnet) 
    {
        if (subnet == cur)
        {
            tmp = cur;
            if (pre)
            {
                subnet_dereference (&pre-> next_subnet, MDL);
                if (cur-> next_subnet)
                {
                    subnet_reference (&pre-> next_subnet, cur-> next_subnet, MDL);
                }
            }
            else 
            {
                subnet_dereference (&subnets, MDL);
                if (cur-> next_subnet)
                {
                    subnet_reference (&subnets, cur-> next_subnet, MDL);
                }
            }
            if (cur-> next_subnet)
            {
                subnet_dereference (&cur -> next_subnet, MDL);
            }
            break;
        }
        pre = cur;
    }
    
    if(tmp)
    {
        if(tmp->group)
        {
            if(tmp->group->statements)
            {
                executable_statement_dereference(&tmp->group->statements, MDL);
            }
            group_dereference(&tmp->group, MDL);
        }
        
        if(tmp->shared_network)
        {
            shared_network_dereference (&tmp -> shared_network, MDL);
        }
        
        if(tmp->interface)
        {
             interface_dereference (&tmp -> interface, MDL);
        }
    }
    
    return;
}

void isc_dhcpserver_remove_dynamic_pool (char* shared_network_name)
{
    struct shared_network *cur = (struct shared_network *)0;
    struct shared_network *pre = (struct shared_network *)0;

    if(NULL == shared_network_name)
    {
        return;
    }
    
    for (cur = shared_networks; cur; cur = cur -> next) 
    {
        if (cur->name && !strcmp(cur->name, shared_network_name))
        {
            free(cur->name);
            if(cur->group)
            {
                if(cur->group->next)
                    group_dereference(&cur->group->next, MDL);
                group_dereference(&cur->group, MDL);
            }
            
            if(cur->subnets)
            {
                isc_dhcpserver_remove_subnet_content(cur->subnets);
                subnet_dereference(&cur->subnets, MDL);
            }

            if(cur->pools)
            {
                cancel_timeout(pool_timer, cur->pools);
                isc_dhcpserver_remove_pool_content(cur->pools);
                pool_dereference(&cur->pools, MDL);
            }
            
            if(cur->interface)
            {
                 interface_dereference (&cur -> interface, MDL);
                if(cur->interface->shared_network)
                {
                    shared_network_dereference(&cur->interface->shared_network, MDL);
                }
            }
            
            if (pre)
            {
                shared_network_dereference (&pre-> next, MDL);
                if (cur-> next)
                {
                    shared_network_reference (&pre-> next, cur-> next, MDL);
                }
            } 
            else 
            {
                shared_network_dereference (&shared_networks, MDL);
                if (cur-> next)
                {
                    shared_network_reference(&shared_networks, cur-> next, MDL);
                }
            }
            if (cur-> next)
            {
                shared_network_dereference (&cur -> next, MDL);
            }
            log_error ("network %s:delete", 
                shared_network_name);
            break;
        }
        pre = cur;
    }

    return;
}

void isc_dhcpserver_remove_static_pool (char* host_name)
{
    struct host_decl *hp = (struct host_decl *)0;
    if(NULL == host_name)
    {
        return;
    }
    
    if (host_hash_lookup (&hp, host_name_hash,
                  (u_char *)host_name,strlen (host_name), MDL))
    {
        if (hp -> fixed_addr)
            option_cache_dereference (&hp -> fixed_addr, MDL);
        if(hp->group)
        {
            if(hp->group->statements)
            {
                executable_statement_dereference(&hp->group->statements, MDL);
            }
            if (hp->group->next)
                group_dereference (&hp->group->next, MDL);
            group_dereference (&hp->group, MDL);
        }
        
        delete_host (hp, 0);
        
        if (hp->name)
        {
            dfree (hp->name, MDL);
            hp->name = (char *)0;
        }
        host_dereference (&hp, MDL);
        log_error ("host %s:delete", host_name);
    }
    
    return;
}

int isc_dhcpserver_valid_option_check(unsigned code)
{
    int cnt = 0;
    int i = 0;
    
    cnt = sizeof(dhcp_option_code_support_list)/sizeof(unsigned);
    for(i = 0; i<cnt; i++)
    {
        if(dhcp_option_code_support_list[i] == code)
        {
            return 1;
        }
    }

    return 0;
}

int isc_dhcpserver_dynamic_pool_option_add(char* shared_network_name,
    unsigned code, 
    u_int8_t* val,
    u_int32_t len,
    int terminated
)
{
    struct shared_network *shared = (struct shared_network *)0;
    struct group *group = (struct group *)0;
    if(NULL == shared_network_name)
    {
        return -1;
    }
    
    log_error ("add network %s option %d",
        shared_network_name, code);
    
    for (shared = shared_networks; shared; shared = shared -> next) 
    {
        if (shared->name && !strcmp(shared->name, shared_network_name))
        {
            if(shared->subnets && shared->subnets->group)
            {
                group = shared->subnets->group;
                break;
            }
        }
    }
    
    if(NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, 
            "shared network %s with valid group not found", 
            shared_network_name);
        return -1;
    }
    
    if(DHO_DHCP_LEASE_TIME == code)
    {
        if(NULL != shared->pools)
        {
            shared->pools->lease_time = *((TIME*)val);
            return 0;
        }
        else
        {
            log_sys(M_MOD_DHCPRLY6, E_ERROR, 
                "shared network %s with valid pool not found", 
                shared_network_name);
            return -1;
        }
    }
    
    return isc_dhcpserver_option_add(group, code, val, len, terminated);
}

int isc_dhcpserver_static_pool_option_add(char* host_name,
    unsigned code, 
    u_int8_t* val,
    u_int32_t len,
    int terminated
)
{
    struct host_decl *hp = (struct host_decl *)0;
    struct group *group = (struct group *)0;
    if(NULL == host_name)
    {
        return -1;
    }
    
    log_error ("add host %s option %d",
        host_name, code);
    
    if (host_hash_lookup(&hp, host_name_hash,
                  (u_char *)host_name, strlen(host_name), MDL)) 
    {
        if(hp != NULL && hp->group)
        {
            group = hp->group;
        }
    }

    if(NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR, "host %s with valid group not found", host_name);
        return -1;
    }
    
    host_dereference(&hp, MDL);
    
    return isc_dhcpserver_option_add(group, code, val, len, terminated);
}

int isc_dhcpserver_option_add(struct group *group, 
    unsigned code, 
    u_int8_t* val,
    u_int32_t len,
    int terminated
)
{
    struct expression *exp = (struct expression *)0;
    struct executable_statement *exec= (struct executable_statement *)0;
    struct executable_statement *exec_tmp = (struct executable_statement *)0;
    struct option *option = (struct option *)0;
    
    if(NULL == group)
    {
        return -1;
    }
    
    option_code_hash_lookup(&option, dhcp_universe.code_hash, &code, 0, MDL);
    
    if(NULL == option)
    {
        return -1;
    }
    
    if (!make_const_data (&exp, (const unsigned char *)val, len, terminated, 1, MDL))
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,  "No memory for option const data");
        return -1;
    }
    
    if (!executable_statement_allocate (&exec, MDL))
    {
        buffer_dereference(&exp->data.const_data.buffer, MDL);
        log_sys(M_MOD_DHCPRLY6, E_ERROR,  "no memory for option statement.");
        return -1;
    }
    
    exec->op = supersede_option_statement;
    
    if (!option_cache (&exec->data.option, NULL, exp, option, MDL))
    {
        buffer_dereference(&exp->data.const_data.buffer, MDL);
        expression_dereference (&exp, MDL);
        log_sys(M_MOD_DHCPRLY6, E_ERROR,  "no memory for option cache");
        return -1;
    }
    
    if (group -> statements)
    {
        /* If this set of statements is only referenced
           by this group, just add the current statement
           to the end of the chain. */
        for (exec_tmp = group -> statements; exec_tmp ->next; exec_tmp = exec_tmp->next)
        {
            ;
        }
        executable_statement_reference (&exec_tmp ->next, exec, MDL);
    } else {
        executable_statement_reference (&group -> statements, exec, MDL);
    }
    
    expression_dereference (&exp, MDL);
    executable_statement_dereference(&exec, MDL);
    option_dereference(&option, MDL);    
    log_error ("add option %d", code);
    return 0;
}

int isc_dhcpserver_option_remove(struct group *group, u_int16_t code)
{
    struct executable_statement *cur= (struct executable_statement *)0;
    struct executable_statement *pre = (struct executable_statement *)0;
     
    if(NULL == group)
    {
        return -1;
    }
    
    for (cur = group ->statements; cur; cur = cur->next)
    {
        if(cur ->data.option 
            && cur->data.option->option
            && cur->data.option->option->code == code)
        {
            
            if (pre)
            {
                if (cur->next)
                {
                    executable_statement_reference (&pre->next, cur->next, MDL);
                    executable_statement_dereference(&cur->next, MDL);
                    executable_statement_dereference(&cur, MDL);
                }
                else
                {
                    executable_statement_dereference(&pre->next, MDL);
                }
            }
            else 
            {
                if (cur->next)
                {
                    executable_statement_reference (&group -> statements, cur->next, MDL);
                    executable_statement_dereference(&cur->next, MDL);
                    executable_statement_dereference(&cur, MDL);
                }
                else
                {
                    executable_statement_dereference(&group -> statements, MDL);
                }
            }
            
            log_error ("option %d:delete", code);
            break;
        }
        pre = cur;
    }
    return 0;
}

int isc_dhcpserver_dynamic_pool_option_remove(char* shared_network_name, unsigned code)
{
    struct shared_network *shared = (struct shared_network *)0;
    struct group *group = (struct group *)0;
    if(NULL == shared_network_name)
    {
        return -1;
    }
    
    log_error ("delete network %s option %d",
        shared_network_name, code);
    
    for (shared = shared_networks; shared; shared = shared -> next) 
    {
        if (shared->name && !strcmp(shared->name, shared_network_name))
        {
            if(shared->subnets && shared->subnets->group)
            {
                group = shared->subnets->group;
                break;
            }
        }
    }
    
    if(NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "shared network %s with valid group not found", shared_network_name);
        return -1;
    }
    
    return isc_dhcpserver_option_remove(group, code);
}

int isc_dhcpserver_static_pool_option_remove(char* host_name, unsigned code)
{
    struct host_decl *hp = (struct host_decl *)0;
    struct group *group = (struct group *)0;
    if(NULL == host_name)
    {
        return -1;
    }
    
    log_error ("delete host %s option %d",
        host_name, code);
    if (host_hash_lookup (&hp, host_name_hash,
                  (u_char *)host_name,strlen(host_name), MDL))
    {
        if(hp == NULL && hp->group)
        {
            group = hp->group;
        }
    }

    if(NULL == group)
    {
        log_sys(M_MOD_DHCPRLY6, E_ERROR,
            "host %s with valid group not found", host_name);
        return -1;
    }
    
    host_dereference(&hp, MDL);
    return isc_dhcpserver_option_remove(group, code);
}
