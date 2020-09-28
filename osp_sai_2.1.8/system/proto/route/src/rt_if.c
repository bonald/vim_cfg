
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"
#include "prefix.h"
#include "rt_if.h"
#include "rt_fea.h"
#include "arp_api.h"
#include "arpratelimit_api.h"
#include "lib_ioctl.h"
#include "lib_sysctl.h"
#include "lib_netlink.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "vrrp.h" 
#include "vrrp_api.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "ipsla_api.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "nd_api.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "nat_api.h"

extern int32
_rt_if_cmd_del_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, cfg_cmd_para_t *para, uint32 delete_all, int32 disable);

extern int32
rt_cmd_del_static_route_by_syscmd(prefix_t *dest, prefix_t *gate, uint8 prefixlen, uint32 ifindex, uint32 is_blackhole);

extern int32
rt_cmd_static_rt_cnt_del(vrf_id_t vrf_id, prefix_t *dest, uint8 prefixlen, uint32 distance, uint32 is_ipv4, prefix_t *gate);

extern int32
ipmc_update_ipmc_intf_delete(char *interface_name);

extern void
nd_init_para_for_add_if(tbl_route_if_t *p_rt_if);

extern int32
rt_if_l3if_loopback_generate_eui64(u_int8_t *eui);

extern int32
ipv6_generate_eui64 (u_int8_t *eui, tbl_interface_t *p_if);

extern ds_connected_v6_t *
ifc_get_ipv6 (ds_connected_v6_t * p_conn, addr_ipv6_t *addr, uint32 prefixlen,
	      tbl_route_if_t *ifp, uint32 add_ifindex_name);

int32
if_get_ifc_ipv6_num(tbl_route_if_t * p_db_rtif)
{
    return p_db_rtif->ifc_ipv6.obj_list->count;
}

ds_connected_v6_t *
connected_check_ipv6 (tbl_route_if_t *p_db_rtif, prefix_t *p)
{
    ds_connected_v6_t      *p_tmp_conn = NULL;
    ctclib_slistnode_t  *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_tmp_conn, listnode)
    {
        if ((0 == IPV6_ADDR_CMP(&(p_tmp_conn->key.address.u.prefix6), &(p->u.prefix6))))
        {
            return p_tmp_conn;
        }
    }

    return NULL;
}

int32
if_match_active_ifc_ipv6_num (tbl_route_if_t *p_db_rtif, prefix_t *p)
{
    ds_connected_v6_t      *p_tmp_conn = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    prefix_t *q;
    int num = 0;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_tmp_conn, listnode)
    {
        if((q = &p_tmp_conn->key.address)
            && ! IN6_IS_ADDR_LOOPBACK (&q->u.prefix6)
            && ! IN6_IS_ADDR_UNSPECIFIED (&q->u.prefix6)
            && GLB_FLAG_ISSET(p_tmp_conn->flags, RT_IF_ADDRV6_REAL)
            && !GLB_FLAG_ISSET (p_tmp_conn->flags, RT_IF_ADDR_DUPLICATE))
        {
            #if 0
            if (if_is_pointopoint(p_db_rtif))
            {
                if((q = p_tmp_conn->destination) && sal_prefix_same (p, q))
                {
                    num++;
                }
            }
            else
            #endif
            {
                if(sal_prefix_match (q, p))
                {
                    num++;
                }
            }
        }
    }

    return num;
}

int32 
rt_if_is_loopback(char *ifname)
{
    tbl_interface_t * p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (p_db_if && p_db_if->hw_type == GLB_IF_TYPE_LOOPBACK_IF)
    {
        return TRUE;
    }

    return FALSE;
}

tbl_route_if_t *
if_match_all_by_ipv6_address (addr_ipv6_t *addr_ipv6)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode_connect = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    
    tbl_route_if_t *match = NULL;
    ds_connected_v6_t *ifc = NULL;
    //struct route_node *rn;
    prefix_t *p = NULL;
    prefix_t q;
    uint8 best_prefixlen = 0;

    sal_memset (&q, 0, sizeof(q));
    best_prefixlen = 0;

    q.family = AF_INET6;
    sal_memcpy(&q.u.prefix6, addr_ipv6, sizeof(q.u.prefix6));
    q.prefixlen = IPV6_MAX_BITLEN;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_rtif, listnode, next)
    {
        if (p_db_rtif != NULL)
        {
             CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, ifc, listnode_connect)	
            {
                if ((p = &ifc->key.address))						
                {
                    //if (if_is_pointopoint (p_db_rtif) && p->prefixlen == IPV6_MAX_BITLEN)
                    if (p->prefixlen == IPV6_MAX_BITLEN) 
                    {								
                        if ((p = &ifc->destination))	
                        {
                            if (IPV6_ADDR_SAME (&p->u.prefix6, addr_ipv6))		
                            {
                                //route_unlock_node (rn);
                                return p_db_rtif;
                            }
                        }
                    }
                    else
                    {
                        if (sal_prefix_match (p, &q) && p->prefixlen > best_prefixlen)
                        {
                            best_prefixlen = p->prefixlen;
                            match = p_db_rtif;
                        }
                    }
                }		
            }
        }
    }

  return match;
}

tbl_route_if_t*
rt_interface_check_rif_exist(tbl_interface_t *p_db_if)
{
    tbl_route_if_t      *p_route_if = NULL;
    tbl_route_if_key_t  rtif_key;

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, p_db_if->key.name, sizeof(rtif_key.name));
    p_route_if = tbl_route_if_get_route_if(&rtif_key);

    return p_route_if;
}

int32
rt_if_add_if(char *ifname)
{
    tbl_sys_global_t    *p_sys_glb  = tbl_sys_global_get_sys_global();
    tbl_interface_t     *p_db_if    = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    tbl_route_if_t      rt_if;
    int32               rc          = PM_E_NONE;

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(&rt_if, 0, sizeof(rt_if));
    sal_strcpy(rt_if.key.name, ifname);
    rt_if.mtu = p_db_if->mtu;
    sal_memcpy(rt_if.mac, p_sys_glb->route_mac, MAC_ADDR_LEN);
    rt_if.arp_retrans_time = GLB_ARP_DEFAULT_RETRANS_TIME;
    rt_if.arp_timeout = GLB_ARP_DEFAULT_TIMEOUT;
    rt_if.ip_redirects_en = TRUE;
    rt_if.ifindex = p_db_if->ifindex;
    rt_if.unicast_rpf_en = p_db_if->unicast_rpf_en;
    rt_if.ospf_timer_dead = OSPF_DEAD_INTERVAL_DEFAULT;
    rt_if.ospf_timer_hello = OSPF_HELLO_INTERVAL_DEFAULT;
    rt_if.nat_enable = GLB_IF_NAT_DISABLE; /* added by taocy for NAT, 2019-06-03*/
    rt_if.nat_category_id = GLB_IF_NAT_CATEGORY_ID_INVALID; /* added by taocy for NAT, 2019-06-03*/

	rt_if.ospf6_timer_dead = OSPF_DEAD_INTERVAL_DEFAULT;		/*add by zhw for ospf6*/
    rt_if.ospf6_timer_hello = OSPF_HELLO_INTERVAL_DEFAULT;		/*add by zhw for ospf6*/
	
    arpratelimit_init_para_for_add_if(&rt_if);

    nd_init_para_for_add_if(&rt_if);
    
    rc = tbl_route_if_add_route_if(&rt_if);
    if (rc < 0)
    {
        return rc;
    }

    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* set CDB default value */
    rt_if_set_ip_unreachable_en(p_db_rtif, TRUE);
    rt_if_set_ip_redirects_en(p_db_rtif, TRUE);
    rt_if_set_ospf_bandwidth(p_db_if, p_db_if->oper_bandwidth);

    return rc;
}

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   set vmac to fea and chip */
int32
rt_if_set_vmac(char *ifname, mac_addr_t vmac, bool add)
{
    int32               rc          = PM_E_NONE;
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;
    mac_addr_t          vmac_zero;

    sal_memset(&rt_if, 0, sizeof(tbl_route_if_t));
    sal_strncpy(rt_if.key.name, ifname, IFNAME_SIZE);
    sal_memset(vmac_zero, 0, sizeof(mac_addr_t));
        
    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    if (add)
    {
        if ((0 == sal_memcmp(p_db_rtif->vmac1, vmac, GLB_ETH_ADDR_LEN))
            || (0 == sal_memcmp(p_db_rtif->vmac2, vmac, GLB_ETH_ADDR_LEN))
            || (0 == sal_memcmp(p_db_rtif->vmac3, vmac, GLB_ETH_ADDR_LEN))) 
        {
            return rc;
        }

        if (0 == sal_memcmp(p_db_rtif->vmac1, vmac_zero, GLB_ETH_ADDR_LEN))
        {
            sal_memcpy(p_db_rtif->vmac1, vmac, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC1));
        }
        else if (0 == sal_memcmp(p_db_rtif->vmac2, vmac_zero, GLB_ETH_ADDR_LEN))
        {
            sal_memcpy(p_db_rtif->vmac2, vmac, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC2));
        }
        else if (0 == sal_memcmp(p_db_rtif->vmac3, vmac_zero, GLB_ETH_ADDR_LEN))
        {
            sal_memcpy(p_db_rtif->vmac3, vmac, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC3));
        }  
    }
    else
    {
        if (0 == sal_memcmp(p_db_rtif->vmac1, vmac, GLB_ETH_ADDR_LEN))
        {
            sal_memset(p_db_rtif->vmac1, 0, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC1));
        }
        else if (0 == sal_memcmp(p_db_rtif->vmac2, vmac, GLB_ETH_ADDR_LEN))
        {
            sal_memset(p_db_rtif->vmac2, 0, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC2));
        }
        else if (0 == sal_memcmp(p_db_rtif->vmac3, vmac, GLB_ETH_ADDR_LEN))
        {
            sal_memset(p_db_rtif->vmac3, 0, MAC_ADDR_LEN);
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_VMAC3));
        }
    }
    
    return rc;
}
/* liwh end */

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   add vip and arp to kernel */
int32
rt_if_add_vip_and_arp(char *ifname, mac_addr_t vmac, ds_connected_t *p_conn)
{
    addr_ipv4_t     ip_addr_cmd;    
    addr_ipv4_t     mask;
    uint32          mask_len = 0;
    int32           rc = PM_E_NONE;
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;

    sal_memset(&rt_if, 0, sizeof(tbl_route_if_t));
    sal_strncpy(rt_if.key.name, ifname, IFNAME_SIZE);
        
    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    /* set destination */
    sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
    p_conn->destination = p_conn->key.address;
    sal_memcpy(&ip_addr_cmd, &p_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
    mask_len = p_conn->destination.prefixlen;
    addr_mask_len2ip_v4(p_conn->destination.prefixlen, &mask);
    p_conn->destination.u.prefix4.s_addr |= ~mask.s_addr;

    /* 3. call API */
    rc = rt_if_add_ipv4(p_db_rtif, p_conn, vmac);
    usleep(20000);
    rc = rt_cmd_add_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);

    return rc;
}
/* liwh end */

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   delete vip and arp from kernel */
int32
rt_if_del_vip_and_arp(char *ifname, ds_connected_t *p_conn)
{
    ds_connected_t      *p_db_conn = NULL;
    addr_ipv4_t         ip_addr_cmd;    
    uint32              mask_len    = 0;
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;
    int32               rc = PM_E_NONE;

    sal_memset(&rt_if, 0, sizeof(tbl_route_if_t));
    sal_strncpy(rt_if.key.name, ifname, IFNAME_SIZE);
        
    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    /* check duplicate config */
    p_db_conn = ds_connected_get_connected(p_db_rtif, p_conn);
    if (NULL == p_db_conn)
    {
        return PM_E_FAIL;
    }

    sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
    sal_memcpy(&ip_addr_cmd, &p_db_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
    mask_len = p_db_conn->destination.prefixlen;

    /* 3. call API */
    rc = rt_if_del_ipv4(p_db_rtif, p_conn);
    rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);
    /*modified by chenc for remove patch function, 2019-05-11*/
    #ifndef USE_FPM_IPUC
    rc = rt_if_rt_entry_by_ifaddr(p_db_rtif, &ip_addr_cmd);
    #endif

    /* update dynamic arp by arp fib */
    arp_update_vip_dynamic_arp_fib(&ip_addr_cmd);

    return rc;
}
/* liwh end */

int32
rt_if_del_if_addr(char *ifname)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    char cmd_str[128];
    char buf[CMD_BUF_256];

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    p_db_kernel_if = tbl_kernel_if_get_by_name(ifname);
    if ((NULL != p_db_kernel_if) && (0x0 != p_db_kernel_if->ip_addr.s_addr))
    {
        sal_sprintf(cmd_str, "no ip address %s/%d",
                    cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_kernel_if->ip_addr),
                    p_db_kernel_if->masklen);
        rt_vty_if_cmd(ifname, cmd_str);
    }

    return PM_E_NONE;

}

int32
rt_if_del_if(char *ifname)
{
    tbl_interface_t *p_db_if    = NULL;
    tbl_route_if_t  *p_db_rtif  = NULL;
    tbl_route_if_key_t rt_if_key;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, ifname);

    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_rtif || NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    ipsla_update_src_intf_delete(p_db_if);
    ipmc_update_ipmc_intf_delete(p_db_if->key.name);
    nat_api_rtif_del(p_db_rtif);/* added by taocy for NAT, 2019-06-03*/

    rt_if_clear_if(p_db_if);
    CTC_TASK_STOP_TIMER(p_db_rtif->t_delete_all_v6_addr_timer);
    CTC_TASK_STOP_TIMER(p_db_rtif->t_send_arp_timer);
    return tbl_route_if_del_route_if(&rt_if_key);
}

int32
rt_if_del_all_arp(tbl_interface_t *p_db_if)
{
    arp_api_trav_ifindex(p_db_if->ifindex, arp_api_del_by_ifindex, (void*)p_db_if, NULL);
    return PM_E_NONE;
}

static int32
_rt_if_destroy(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif)
{
    /* support vrrp modified by liwh for bug 45215, 2017-09-20 */
    vrrp_ip_address_interface_del(p_db_rtif);
    /* liwh end */

    /* modified by wangjj for fix BHM reload system when no interface clear ospf related, 2017-03-16 */
    ipc_pm_to_bhm_keepalive();
    rt_if_del_all_ipv4(p_db_rtif);
    rt_if_del_all_ipv6(p_db_rtif, TRUE, FALSE);
    
    /* support nd modified by liwh for bug 47547, 2018-07-18 */
    nd_clear_nd_entries_and_config(p_db_rtif);
    /* liwh end */

    /* modified by liwh for bug 51257, 2019-03-18 */
    CTC_TASK_STOP_TIMER(p_db_rtif->t_ospf_cost_timer);
    /* liwh end */
    
    if (GLB_IF_TYPE_LOOPBACK_IF != p_db_if->hw_type)
    {
        rt_if_clear_rtif_urpf_en(p_db_rtif);
        rt_if_clear_rtif_urpf_en_v6(p_db_rtif);
    }
    rt_if_clear_ospf_if(p_db_rtif);
	rt_if_clear_ospf6_if(p_db_rtif);
    rt_if_del_all_ospf_auth_md5(p_db_rtif);
    /* modified by wangjj for fix bug 43802, 2017-04-25 */
    #ifdef _CENTEC_
    /*modified by chenc for remove patch function, 2019-05-11*/
    #ifndef USE_FPM_IPUC
    rt_if_ipv4_del_related_route_by_link(p_db_if, p_db_rtif, FALSE);
    rt_if_ipv6_del_related_route_by_link(p_db_if, p_db_rtif, FALSE);
    #endif
    #endif

    /* clear ospf kernel if related to vlanif, 2017-05-22 */
    char if_name[IFNAME_SIZE];
    tbl_iter_args_t args;

    sal_memset(if_name, 0, sizeof(if_name));
    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        sal_memcpy(if_name, p_db_if->key.name, sizeof(p_db_if->key.name));
        args.argv[0] = if_name;
        tbl_kernel_if_iterate(rt_if_clear_kernel_if_by_ifname, &args);
    }

    /* disable dhclient */
    if (TRUE == p_db_rtif->dhcp_client_enable )
    {
        rt_if_set_dhcp_client_en(p_db_rtif, FALSE);
    }

    /* modified by liwh for bug 49365, 2018-12-04 */
    /* disable dhcpv6 client */
    if (TRUE == p_db_rtif->dhcpv6_client_enable )
    {
        rt_if_set_dhcpv6_client_en(p_db_rtif, FALSE);
    }
    /* liwh end */

    if (GLB_IF_NAT_DISABLE != p_db_rtif->nat_enable )
    {
        rt_if_set_nat_en(p_db_rtif, GLB_IF_NAT_DISABLE);
    }

    return PM_E_NONE;
}

int32
rt_if_clear_if(tbl_interface_t *p_db_if)
{
    tbl_route_if_t *p_db_rtif = NULL;

    p_db_rtif = rt_interface_check_rif_exist(p_db_if);
    if (p_db_rtif)
    {
        _rt_if_destroy(p_db_if, p_db_rtif);
    }

    return PM_E_NONE;
}

/*current only support ipv6*/
int32
static_delete_by_if (afi_t afi, char *ifname)
{
    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    char ifname_tmp[IFNAME_SIZE] = {0};
    int32 rc = 0;
    uint32 distance = 0;
    
    //struct ptree_node *rn;
    //union nsm_nexthop *gate = NULL;
    //prefix_t p;

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        if (NULL != p_db_rst_cfg)
        {
            if (p_db_rst_cfg->nexthop_type_v6 == NEXTHOP_TYPE_IPV6_IFINDEX)
            {
                rc = tbl_interface_get_name_by_ifindex(p_db_rst_cfg->key.ifindex_v6, ifname_tmp, sizeof(ifname_tmp));
                if (PM_E_NONE != rc)
                {
                    continue;
                }
                
                if (sal_strcmp (ifname_tmp, ifname) == 0)
                {
                    #if 0
                    if (p_db_rst_cfg->nexthop_type_v6 == NEXTHOP_TYPE_IPV4_IFINDEX)
                    {
                        gate = &p_db_rst_cfg->key.nh_addr.ipv4;
                    }
                    else if (p_db_rst_cfg->nexthop_type_v6 == NEXTHOP_TYPE_IPV6_IFINDEX)
                    {
                        gate = &p_db_rst_cfg->key.nh_addr.ipv6;
                    }
                    
                    sal_memset(&p, 0, sizeof(prefix_t));
                    p.prefixlen = rn->key_len;
                    if (afi == AFI_IP)
                    {
                        p.family = AF_INET;
                        NSM_PTREE_KEY_COPY (&p.u.prefix4, rn);
                    }
                    else if (afi == AFI_IP6)
                    {
                        p.family = AF_INET6;
                        NSM_PTREE_KEY_COPY (&p.u.prefix6, rn);
                    }

                    //p.pad1 = NSM_ROUTE_STATIC;
                    //rt_delete_static_route (vrf, afi, &p, gate, ifname, p_db_rst_cfg->distance);

                    tbl_static_route_cfg_t  *pst_static_route = NULL;
                    tbl_static_route_cfg_t  static_route = {0};

                    sal_memset(&static_route, 0, sizeof(static_route));
                    static_route.key.vrf_id = 0;
                    static_route.key.p.family = p.family;
                    static_route.key.p.prefixlen = prefixlen;
                    sal_memcpy(static_route.key.p.u.prefix6, &p.u.prefix6, sizeof(p.u.prefix6));
                    
                    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
                    if (NULL != pst_static_route)
                    {
                        #if 0
                        /* process cancel track static route first */
                        /*add family check for no ipv6 route*/
                        if (argc > 5 && AF_INET == dest->family)
                        {
                            track_ipv4_route_unset(&static_route.key, para);
                            rc = rt_cmd_add_static_route_by_syscmd(dest, gate, prefixlen, pst_static_route->distance, ifindex);
                            return rc;
                        }
                        #endif

                        rc = tbl_static_route_cfg_del_static_route_cfg(&static_route.key);
                        rt_cmd_static_rt_cnt_del(vrf_id, dest, prefixlen);

                    }
                    else
                    {
                        //CFG_INVALID_PARAM_RET ("No matching route to delete");
                        MY_PRINTF("No matching route to delete");
                        return PM_E_NONE;
                    }
                    #else
                    tbl_static_route_cfg_t static_route_cfg;
                    
                    sal_memset(&static_route_cfg, 0, sizeof(static_route_cfg));
                    sal_memcpy(&static_route_cfg, p_db_rst_cfg, sizeof(static_route_cfg));
                    distance = p_db_rst_cfg->distance;
                    
                    rc = tbl_static_route_cfg_del_static_route_cfg(&static_route_cfg.key);
                    #endif

                    prefix_t gate;
                    sal_memset(&gate, 0, sizeof(gate));
                    gate.family = AF_INET6;
                    /*gate no need prefixlen*/
                    //gate.prefixlen = ;
                    sal_memcpy(&gate.u.prefix6, &static_route_cfg.key.nh_addr.ipv6, sizeof(static_route_cfg.key.nh_addr.ipv6));
                    
                    rt_cmd_static_rt_cnt_del(static_route_cfg.key.vrf_id, &static_route_cfg.key.p, static_route_cfg.key.p.prefixlen, distance, FALSE, &gate);
                    rc = rt_cmd_del_static_route_by_syscmd(&static_route_cfg.key.p, 
                        &gate, static_route_cfg.key.p.prefixlen, static_route_cfg.key.ifindex_v6, FALSE);
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int32
rt_if_set_mode(char *ifname, uint32 mode)
{
    int32 rc = PM_E_NONE;
    if (GLB_IF_MODE_L3 == mode)
    {
        rc = rt_if_add_if(ifname);
    }
    else
    {    
        /*delete ipv6 static route by if*/
        static_delete_by_if (AFI_IP6, ifname);
        rc = rt_if_del_if(ifname);
    }

    return rc;
}

int32
rt_if_set_mtu(char *ifname, uint32 mtu)
{
    tbl_route_if_t      *p_db_rtif  = NULL;
    tbl_route_if_key_t  rt_if_key;
    int32               rc          = PM_E_NONE;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, ifname);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    p_db_rtif->mtu = mtu;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_MTU);
    return rc;
}

/*add by chenc for bug 50407, 2018-12-20*/
int32
rt_if_mgr_set_mode(tbl_route_if_t *p_db_route_if, uint32 mode_set_before)
{
    int32 rc = PM_E_NONE;

    p_db_route_if->mode_set_before = mode_set_before;
    tbl_route_if_set_route_if_field(p_db_route_if, TBL_ROUTE_IF_FLD_MODE_SET_BEFORE);

    /* delete ipv6 address */
    /* delete ipv6 static route by if */
    static_delete_by_if (AFI_IP6, p_db_route_if->key.name);
    rc = rt_if_del_if(p_db_route_if->key.name);
    
    return rc;
}

/*add by chenc for bug 50407, 2018-12-20*/
int32
rt_if_set_mode_set_before(uint8 mode_set_before, tbl_route_if_t *p_db_route_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;

    if (mode_set_before != GLB_IF_MODE_L2)
    {
        p_db_route_if->mode_set_before = mode_set_before;
        tbl_route_if_set_route_if_field(p_db_route_if, TBL_ROUTE_IF_FLD_MODE_SET_BEFORE);
        return PM_E_NONE;
    }
    
    p_db_if = tbl_interface_get_interface_by_name(p_db_route_if->key.name);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", p_db_route_if->key.name);
    }
    
    /* 1. config check */
#if 1
    if (GLB_IF_MODE_L2 == mode_set_before && GLB_IF_MODE_L3 == p_db_if->mode && p_db_if->inband_snat_type)
    {
        CFG_INVALID_PARAM_RET("'%s' already assigned to %s",
                              p_db_if->key.name,
                              INBAND_SNAT_TYPE_TO_STRING(p_db_if->inband_snat_type));
    }
#endif

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("This command is forbidden on LAG interface");
    }

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    if (p_db_if && p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("Cannot configure for MLAG peer-link has been configured");
    }
    
    if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
    {
        CFG_CONFLICT_RET("Cannot configure mirror destination port");
    }

    if ((p_db_if->erps_enable) && (GLB_IF_MODE_L2 != mode_set_before))
    {
        CFG_CONFLICT_RET("This interface is already used by ERPS");
    }

    /* modified by liwh for bug 40053, 2016-08-10 */
    if (p_db_if && p_db_if->dot1x_enable)
    {
        CFG_CONFLICT_RET("Conflict detected! dot1x is enabled on this interface!");
    }
    /* liwh end */

    /* 2. check duplicate config */
    if (p_db_if->mode == mode_set_before)
    {
        return PM_E_NONE;
    }
 
    /* 3. call API */
    PM_E_RETURN(rt_if_mgr_set_mode(p_db_route_if, mode_set_before));
    return rc;
}

ds_connected_t *
rt_if_get_primary_connected(tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ds_connected_t      *p_db_conn  = NULL;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            return p_db_conn;
        }
    }

    return NULL;
}

static int32
_rt_if_running_set_process(char *ifname, uint32 new_running)
{
    tbl_kernel_if_t     *p_kernel_if    = NULL;
    int32               rc              = PM_E_NONE;

    p_kernel_if = tbl_kernel_if_get_by_name(ifname);
    if (NULL != p_kernel_if)
    {
        rc = lib_netlink_link_set(p_kernel_if->key.ifindex, new_running);
        //rc = lib_ioctl_if_set_up_running(p_kernel_if->name, new_running);
    }

    return rc;
}

int32
rt_if_up_set_process(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, uint32 new_running)
{
    int32 rc = PM_E_NONE;

    rc = lib_ioctl_if_set_up(p_db_rtif->key.name, new_running);

    return rc;
}

int32
rt_fib_connected_update_ipv6(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_db_conn, uint32 new_up_running)
{
    addr_ipv6_t     dest_addr;
    addr_ipv6_t     dest_addr_cp;
    addr_ipv6_t     dest_host;
    vrf_id_t        vrf_id;
    //uint32          mask        = 0;
    uint8           prefixlen   = 0;
    int32           rc          = PM_E_NONE;
    char buf1[128] = {0}; 

    if (IN6_IS_ADDR_LINKLOCAL(&p_db_conn->key.address.u.prefix6))
    {
        MY_PRINTF("Linklocal address %s, return", prefix_val2str(buf1, sizeof(buf1), &p_db_conn->key.address, TRUE));
        return 0;
    }

    sal_memset(&dest_addr, 0, sizeof(dest_addr));
    sal_memset(&dest_host, 0, sizeof(dest_host));
    sal_memset(&dest_addr_cp, 0, sizeof(dest_addr_cp));

    vrf_id = DEFAULT_VRF_ID;
    prefixlen = p_db_conn->key.address.prefixlen;

    sal_memcpy(&dest_addr_cp, &p_db_conn->key.address.u.prefix6, sizeof(addr_ipv6_t));
    sal_memcpy(&dest_host, &p_db_conn->key.address.u.prefix6, sizeof(addr_ipv6_t));

    #if 0
    dest_addr_cp.s_addr = sal_htonl(dest_addr_cp.s_addr);
    IPV4_LEN_TO_MASK(mask, prefixlen);

    dest_addr_cp.s_addr = dest_addr_cp.s_addr & mask;
    dest_addr_cp.s_addr = sal_htonl(dest_addr_cp.s_addr);
    #else
    assert(apply_mask_ipv6_addr(&dest_addr_cp, prefixlen) == PM_E_NONE);
    #endif
    sal_memcpy(&dest_addr, &dest_addr_cp, sizeof(addr_ipv6_t));

    /* 1. For host route 2. Check the interface is not up && running */
    if (!new_up_running)
    {
        rc = rt_fea_ipv6_del_connect_route_by_link(vrf_id, &dest_addr, prefixlen, p_db_rtif->key.name);
        rc = rt_fea_ipv6_del_connect_route_by_link(vrf_id, &dest_host, 128, p_db_rtif->key.name);
    }
    else
    {
        rc = rt_fea_ipv6_add_connect_route_by_link(vrf_id, &dest_addr, prefixlen, p_db_rtif->key.name);
        rc = rt_fea_ipv6_add_connect_route_by_link(vrf_id, &dest_host, 128, p_db_rtif->key.name);
    }

    return rc;
}

int32
rt_fib_connected_update_ipv4(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_db_conn, uint32 new_up_running)
{
    addr_ipv4_t     dest_addr;
    addr_ipv4_t     dest_addr_cp;
    addr_ipv4_t     dest_host;
    vrf_id_t        vrf_id;
    uint32          mask        = 0;
    uint8           prefixlen   = 0;
    int32           rc          = PM_E_NONE;

    sal_memset(&dest_addr, 0, sizeof(dest_addr));
    sal_memset(&dest_host, 0, sizeof(dest_host));
    sal_memset(&dest_addr_cp, 0, sizeof(dest_addr_cp));

    vrf_id = DEFAULT_VRF_ID;
    prefixlen = p_db_conn->key.address.prefixlen;

    sal_memcpy(&dest_addr_cp, &p_db_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
    sal_memcpy(&dest_host, &p_db_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));

    dest_addr_cp.s_addr = sal_htonl(dest_addr_cp.s_addr);
    IPV4_LEN_TO_MASK(mask, prefixlen);

    dest_addr_cp.s_addr = dest_addr_cp.s_addr & mask;
    dest_addr_cp.s_addr = sal_htonl(dest_addr_cp.s_addr);
    sal_memcpy(&dest_addr, &dest_addr_cp, sizeof(addr_ipv4_t));

    /* 1. For host route 2. Check the interface is not up && running */
    if (!new_up_running)
    {
        rc = rt_fea_ipv4_del_connect_route_by_link(vrf_id, &dest_addr, prefixlen, p_db_rtif->key.name);
        rc = rt_fea_ipv4_del_connect_route_by_link(vrf_id, &dest_host, 32, p_db_rtif->key.name);
    }
    else
    {
        rc = rt_fea_ipv4_add_connect_route_by_link(vrf_id, &dest_addr, prefixlen, p_db_rtif->key.name);
        rc = rt_fea_ipv4_add_connect_route_by_link(vrf_id, &dest_host, 32, p_db_rtif->key.name);
    }

    return rc;
}

int32
rt_if_ipv6_route_del_by_nexthop(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_t  *pst_iproute_node   = obj;
    tbl_interface_t     *p_db_if            = pargs->argv[0];
    tbl_route_if_t      *p_db_rtif          = pargs->argv[1];
    vrf_id_t            vrf_id;
    addr_ipv6_t         dest;
    addr_ipv6_t         gate;
    uint8               prefixlen   = 0;
    uint32              ifindex     = 0;
    uint32              distance    = 0;
    uint32              i    = 0, j =0;
    int                 ret = PM_E_NONE;
    addr_ipv6_t         gate_addr;
    char                ifname[IFNAME_SIZE];
    nexthop_ecmp_t       nh_group;

    //log_sys(M_MOD_ROUTE, E_ERROR, "%s \n", __func__);

    if (NULL == pst_iproute_node || NULL == p_db_if || NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    if (AF_INET6 != pst_iproute_node->key.p.family)
    {
        return PM_E_NONE;
    }

    sal_memset(&gate_addr, 0, sizeof(gate_addr));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memcpy(ifname, p_db_rtif->key.name, sizeof(ifname));
    ifindex = p_db_rtif->kernel_ifindex;

    if (0 == pst_iproute_node->multipath)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath is 0, dest %x\n", __func__, 
          //  pst_iproute_node->key.p.u.prefix4.s_addr);   
       // log_sys(M_MOD_ROUTE, E_ERROR, "%s type %d, ifname %s, db_if name %s\n", __func__,
           // pst_iproute_node->nh_key.type, pst_iproute_node->nh_key.ifname, p_db_if->key.name);   
        if (NH_TYPE_IPV6 == pst_iproute_node->nh_key.type || NH_TYPE_IPV6_IFINDEX == pst_iproute_node->nh_key.type)
        {
            if (0 == sal_memcmp(pst_iproute_node->nh_key.ifname, p_db_if->key.name, sizeof(pst_iproute_node->nh_key.ifname)))
            {
                vrf_id = pst_iproute_node->key.vrf_id;
                sal_memcpy(&dest, &pst_iproute_node->key.p.u.prefix6, sizeof(pst_iproute_node->key.p.u.prefix6));
                sal_memcpy(&gate, &pst_iproute_node->nh_key.u.ipv6, sizeof(pst_iproute_node->nh_key.u.ipv6));
                //dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
                //gate.s_addr  = pst_iproute_node->nh_key.u.ipv4.s_addr;
                prefixlen = pst_iproute_node->key.p.prefixlen;
                distance = pst_iproute_node->distance;
                #define RTN_LOCAL 2
                ret = rt_fea_ipv6_del_route_nl(vrf_id, &dest, &gate, prefixlen, ifindex, distance, RTN_LOCAL);
                //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
            }
        }
    }

    else if (1 == pst_iproute_node->multipath)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath is 1\n", __func__);    
        sal_memset(&nh_group, 0, sizeof(nh_group));
        
        for (i = 0; i < pst_iproute_node->nh_group.nh_num; i++)
        {
            if (sal_memcmp(pst_iproute_node->nh_group.nh[i].ifname, ifname, sizeof(ifname)))
            {
                /*copy the diff nh member to tmp nh_group*/
                sal_memcpy(&nh_group.nh[j], &pst_iproute_node->nh_group.nh[i], sizeof(nexthop_key_t));
                j++;
            }
        }
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath i =%d, j = %d\n", __func__, i, j);    

        /*all nexthop is this if*/
        if (j == 0) /*del this route*/
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            //dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            sal_memcpy(&dest, &pst_iproute_node->key.p.u.prefix6, sizeof(dest));
            
            prefixlen = pst_iproute_node->key.p.prefixlen;
            distance = pst_iproute_node->distance;
            ret = rt_fea_ipv6_del_route_multi_nl(vrf_id, &dest, NULL,
                                pst_iproute_node->nh_group.nh_num, prefixlen, distance);
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
        }
        else if(j < i)/*update this route*/
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            //dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            sal_memcpy(&dest, &pst_iproute_node->key.p.u.prefix, sizeof(dest));
            prefixlen = pst_iproute_node->key.p.prefixlen;
            distance = pst_iproute_node->distance;
            ret = rt_fea_ipv6_add_route_multi_nl(vrf_id, &dest, nh_group.nh,
                                j, prefixlen, distance);  
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
        }
        else if(j == i) /*do nothing*/
        {
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, do nothing \n");    
            
        }
    }
    
    return ret;
}

int32
rt_if_ipv4_route_del_by_nexthop(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_t  *pst_iproute_node   = obj;
    tbl_interface_t     *p_db_if            = pargs->argv[0];
    tbl_route_if_t      *p_db_rtif          = pargs->argv[1];
    vrf_id_t            vrf_id;
    addr_ipv4_t         dest;
    addr_ipv4_t         gate;
    uint8               prefixlen   = 0;
    uint32              ifindex     = 0;
    uint32              distance    = 0;
    uint32              i    = 0, j =0;
    int                 ret = PM_E_NONE;
    addr_ipv4_t         gate_addr;
    char                ifname[IFNAME_SIZE];
    nexthop_ecmp_t       nh_group;

    //log_sys(M_MOD_ROUTE, E_ERROR, "%s \n", __func__);

    if (NULL == pst_iproute_node || NULL == p_db_if || NULL == p_db_rtif)
    {
        return PM_E_FAIL;
    }

    if (AF_INET6 == pst_iproute_node->key.p.family)
    {
        return PM_E_NONE;
    }

    sal_memset(&gate_addr, 0, sizeof(gate_addr));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memcpy(ifname, p_db_rtif->key.name, sizeof(ifname));
    ifindex = p_db_rtif->kernel_ifindex;

    if (0 == pst_iproute_node->multipath)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath is 0, dest %x\n", __func__, 
          //  pst_iproute_node->key.p.u.prefix4.s_addr);   
       // log_sys(M_MOD_ROUTE, E_ERROR, "%s type %d, ifname %s, db_if name %s\n", __func__, 
           // pst_iproute_node->nh_key.type, pst_iproute_node->nh_key.ifname, p_db_if->key.name);   
        if (NH_TYPE_IPV4 == pst_iproute_node->nh_key.type)
        {
            if (0 == sal_memcmp(pst_iproute_node->nh_key.ifname, p_db_if->key.name, sizeof(pst_iproute_node->nh_key.ifname)))
            {
                vrf_id = pst_iproute_node->key.vrf_id;
                dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
                gate.s_addr  = pst_iproute_node->nh_key.u.ipv4.s_addr;
                prefixlen = pst_iproute_node->key.p.prefixlen;
                distance = pst_iproute_node->distance;
                #define RTN_LOCAL 2
                ret = rt_fea_ipv4_del_route_nl(vrf_id, &dest, &gate, prefixlen, ifindex, distance, RTN_LOCAL, ZEBRA_ROUTE_MAX);
                //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
            }
        }
    }

    else if (1 == pst_iproute_node->multipath)
    {
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath is 1\n", __func__);    
        sal_memset(&nh_group, 0, sizeof(nh_group));
        
        for (i = 0; i < pst_iproute_node->nh_group.nh_num; i++)
        {
            if (sal_memcmp(pst_iproute_node->nh_group.nh[i].ifname, ifname, sizeof(ifname)))
            {
                /*copy the diff nh member to tmp nh_group*/
                sal_memcpy(&nh_group.nh[j], &pst_iproute_node->nh_group.nh[i], sizeof(nexthop_key_t));
                j++;
            }
        }
        //log_sys(M_MOD_ROUTE, E_ERROR, "%s multipath i =%d, j = %d\n", __func__, i, j);    

        if (j == 0) /*del this route*/
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            
            prefixlen = pst_iproute_node->key.p.prefixlen;
            distance = pst_iproute_node->distance;
            ret = rt_fea_ipv4_del_route_multi_nl(vrf_id, &dest, NULL,
                                pst_iproute_node->nh_group.nh_num, prefixlen, distance, ZEBRA_ROUTE_MAX);
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
        }
        else if(j < i)/*update this route*/
        {
            vrf_id = pst_iproute_node->key.vrf_id;
            dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
            prefixlen = pst_iproute_node->key.p.prefixlen;
            distance = pst_iproute_node->distance;
            ret = rt_fea_ipv4_add_route_multi_nl(vrf_id, &dest, nh_group.nh,
                                j, prefixlen, distance, ZEBRA_ROUTE_MAX);  
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, dest %x, gate %x\n", dest.s_addr, gate.s_addr);    
        }
        else if(j == i) /*do nothing*/
        {
            //log_sys(M_MOD_ROUTE, E_ERROR, "rt_fea_ipv4_del_route_nl, do nothing \n");    
            
        }
    }
    
    return ret;
}

int32
rt_if_ipv4_del_related_route_by_link(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, uint32 new_up_running)
{
    tbl_iter_args_t args;

    args.argv[0] = p_db_if;
    args.argv[1] = p_db_rtif;
    if (!new_up_running)
    {
        tbl_iproute_node_iterate(rt_if_ipv4_route_del_by_nexthop, &args);
    }

    return PM_E_NONE;
}

int32
rt_if_ipv6_del_related_route_by_link(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, uint32 new_up_running)
{
    tbl_iter_args_t args;

    args.argv[0] = p_db_if;
    args.argv[1] = p_db_rtif;
    if (!new_up_running)
    {
        tbl_iproute_node_iterate(rt_if_ipv6_route_del_by_nexthop, &args);
    }

    return PM_E_NONE;
}

int32
rt_if_running_update_v6(char *ifname, uint32 new_up_running)
{
    tbl_interface_t     *p_db_if    = NULL;
    ds_connected_v6_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    int32               rc = PM_E_NONE;
    tbl_interface_t     interface_running;
    //uint32                  prefixlen = 0;

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 3. If the interface is up and running */
    sal_memset(&interface_running, 0, sizeof(interface_running));
    sal_memcpy(&interface_running, p_db_if, sizeof(tbl_interface_t));
    /*modified by chenc for remove patch function, 2019-05-11*/
    #ifndef USE_FPM_IPUC
    rc = rt_if_ipv6_del_related_route_by_link(p_db_if, p_db_rtif, new_up_running);
    #endif

    if (!new_up_running)
    {
        //nd_api_if_up_pre_unset(p_db_rtif);
        nd_if_stop(p_db_rtif, FALSE);
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, listnode)
    {
    #if 0
        if (!new_up_running)
        {
            if (tbl_interface_is_running(p_db_if))
            {
                GLB_UNSET_FLAG(interface_running.flags, GLB_IFF_RUNNING);
            }

            /* delete all neighbor entries from chip and kernel */
            neigh_ifc_down(p_db_rtif, p_db_conn);

            rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_db_conn, new_up_running);

            prefixlen = p_db_conn->key.address.prefixlen;
            rt_cmd_del_if_addr_v6_by_syscmd(p_db_if->key.name, &p_db_conn->key.address.u.prefix6, prefixlen);
        }
        else
            #endif
        if (new_up_running)   
        {
            /* set interface running, otherwise can't start nd dad check */
            p_db_if->flags = p_db_if->flags | (GLB_IFF_UP|GLB_IFF_RUNNING);
 
            /* start nd dad check and install ipv6 address*/
            nd_address_install(p_db_rtif, p_db_conn);
        }
    }

    return rc;
}

int32
rt_if_up_update(char *ifname, uint32 new_up)
{
    tbl_interface_t     *p_db_if    = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    int32               rc          = PM_E_NONE;

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 3. If the interface is up and running */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_db_conn, new_up);
    }

    /*for bug 49488 by chenc*/
    #if 0
    /* 3. If the interface is up and running */
    ds_connected_v6_t      *p_db_conn_v6  = NULL;
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn_v6, listnode)
    {
        rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_db_conn_v6, new_up);
    }
    #else
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        rc = rt_if_running_update_v6(ifname, new_up);
    }
    #endif

    return rc;
}

int32
rt_if_indirect_ip_route_inc(tbl_interface_t *p_db_if, uint32 is_ipv4, uint32 is_link_local)
{
    int32 ret = PM_E_NONE;
    
    /*for interface address*/
    if (p_db_if)
    {
        if (INTERFACE_IS_LOOPBACK(p_db_if))
        {
            if (is_ipv4)
            {
                ret = rt_indirect_ipv4_route_inc(0, 1);
            }
            else
            {
                if (!is_link_local)
                {
                    rt_indirect_ipv6_route_inc(0, 1);
                }
            }
        }
        else
        {
            if (is_ipv4)
            {
                ret = rt_indirect_ipv4_route_inc(0, 2);
            }
            else
            {
                if (!is_link_local)
                {
                    rt_indirect_ipv6_route_inc(0, 2);
                }
            }
        }
    }
    /*for static route*/
    else
    {
        if (is_ipv4)
        {
            ret = rt_indirect_ipv4_route_inc(0, 1);
        }
        else
        {
            rt_indirect_ipv6_route_inc(0, 1);
        }
    }

    return ret;
}

void
rt_if_indirect_ip_route_dec(tbl_interface_t *p_db_if, uint32 is_ipv4, uint32 is_link_local)
{
    /*for interface address*/
    if (p_db_if)
    {
        if (INTERFACE_IS_LOOPBACK(p_db_if))
        {
            if (is_ipv4)
            {
                rt_indirect_ipv4_route_dec(0, 1);
            }
            else
            {
                if (!is_link_local)
                {
                    rt_indirect_ipv6_route_dec(0, 1);
                }
            }
        }
        else
        {
            if (is_ipv4)
            {
                rt_indirect_ipv4_route_dec(0, 2);
            }
            else
            {
                if (!is_link_local)
                {
                    rt_indirect_ipv6_route_dec(0, 2);
                }
            }
        }
    }
    else
    {
        if (is_ipv4)
        {
            rt_indirect_ipv4_route_dec(0, 1);
        }
        else
        {
            rt_indirect_ipv6_route_dec(0, 1);
        }
    }
}

int32
rt_if_connected_valid_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_ifc)
{
    int32               rc = PM_E_NONE;
    tbl_interface_t     *p_db_if    = NULL;
    uint32                  prefixlen = 0;

    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_ifc, TRUE);

    prefixlen = p_ifc->key.address.prefixlen;
    
    rt_cmd_add_if_addr_v6_by_syscmd(p_db_rtif->key.name, &p_ifc->key.address.u.prefix6, prefixlen);

    //rt_if_indirect_ip_route_inc(p_db_if, 0, IN6_IS_ADDR_LINKLOCAL(&p_ifc->key.address.u.prefix6));

    /* add static neighbor entries to chip and kernel*/
    neigh_ifc_up(p_db_rtif, p_ifc);

    GLB_SET_FLAG (p_ifc->flags, RT_IF_ADDRV6_REAL);
            
    return rc;
}

int32
rt_if_connected_invalid_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_ifc, bool delete_addr)
{
    int32                    rc = PM_E_NONE;
    uint32                  prefixlen = 0;
    tbl_interface_t       *p_db_if    = NULL;

    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* delete all neighbor entries from chip and kernel */
    neigh_ifc_down(p_db_rtif, p_ifc, delete_addr);

    rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_ifc, FALSE);

    prefixlen = p_ifc->key.address.prefixlen;
    rt_cmd_del_if_addr_v6_by_syscmd(p_db_if->key.name, &p_ifc->key.address.u.prefix6, prefixlen);

    //rt_if_indirect_ip_route_dec(p_db_if, 0, IN6_IS_ADDR_LINKLOCAL(&p_ifc->key.address.u.prefix6));

    GLB_UNSET_FLAG (p_ifc->flags, RT_IF_ADDRV6_REAL);

    return rc;
}

int32
rt_if_running_update(char *ifname, uint32 new_up_running)
{
    tbl_interface_t     *p_db_if    = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    int32               rc          = PM_E_NONE;
    tbl_interface_t     interface_running;

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 3. If the interface is up and running */
    sal_memset(&interface_running, 0, sizeof(interface_running));
    sal_memcpy(&interface_running, p_db_if, sizeof(tbl_interface_t));
    /*modified by chenc for remove patch function, 2019-05-11*/
    #ifndef USE_FPM_IPUC
    rc = rt_if_ipv4_del_related_route_by_link(p_db_if, p_db_rtif, new_up_running);
    #endif

    if (p_db_rtif->is_up_running != new_up_running)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
        {
            rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_db_conn, new_up_running);
            if (!new_up_running)
            {
                if (tbl_interface_is_running(p_db_if))
                {
                    GLB_UNSET_FLAG(interface_running.flags, GLB_IFF_RUNNING);
                }
                rc = arp_api_connected_down(&interface_running, p_db_rtif, p_db_conn);
            }
            else
            {
               rc = arp_api_connected_up(p_db_if, p_db_rtif, p_db_conn, new_up_running);
            }
        }

        p_db_rtif->is_up_running = new_up_running;
    }

    return rc;
}

/*added by chenc for bug 52215, 2019-06-11*/
static void
_rt_if_send_arp_on_timer(void* p_data)
{
    tbl_route_if_t* p_db_rtif = p_data ? (tbl_route_if_t*)p_data : NULL;

    p_db_rtif->t_send_arp_timer = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    uint32 new_up_running = 1;
    tbl_interface_t     *p_db_if    = NULL;
    
    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)
    {
        return;
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        /*modified by chenc for bug 52708, 2019-06-26*/
        //rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_db_conn, new_up_running);
        /*chenc end*/
        //if (!new_up_running)
        //{
        //    if (tbl_interface_is_running(p_db_if))
        //    {
        //        GLB_UNSET_FLAG(interface_running.flags, GLB_IFF_RUNNING);
        //    }
        //    rc = arp_api_connected_down(&interface_running, p_db_rtif, p_db_conn);
        //}
        //else
        {
           arp_api_connected_up(p_db_if, p_db_rtif, p_db_conn, new_up_running);
        }
    }
    //p_db_rtif->is_up_running = new_up_running;

    return;
}
/*chenc end*/

/*for bug 51460 by chenc, 2019-03-28*/
int32
rt_if_running_update_arp(char *ifname, uint32 new_up_running)
{
    tbl_interface_t     *p_db_if    = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    int32               rc          = PM_E_NONE;
    tbl_interface_t     interface_running;

    p_db_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* 3. If the interface is up and running */
    sal_memset(&interface_running, 0, sizeof(interface_running));
    sal_memcpy(&interface_running, p_db_if, sizeof(tbl_interface_t));

    if (p_db_rtif->is_up_running != new_up_running)
    {
        /*modified by chenc for bug 52215, 2019-06-11*/
        uint32 sleep_time;
        sleep_time = 100;
        
        if (sleep_time && new_up_running)
        {
            /*modified by chenc for bug 52708, 2019-06-26*/
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
            {
                rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_db_conn, new_up_running);
            }
            /*chenc end*/
    
            p_db_rtif->t_send_arp_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, _rt_if_send_arp_on_timer, p_db_rtif, sleep_time);
            p_db_rtif->is_up_running = new_up_running;
            return PM_E_NONE;
        }

        if (!new_up_running)
        {
            CTC_TASK_STOP_TIMER(p_db_rtif->t_send_arp_timer);
        }
        /*chenc ended*/
        
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
        {
            rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_db_conn, new_up_running);
            if (!new_up_running)
            {
                if (tbl_interface_is_running(p_db_if))
                {
                    GLB_UNSET_FLAG(interface_running.flags, GLB_IFF_RUNNING);
                }
                rc = arp_api_connected_down(&interface_running, p_db_rtif, p_db_conn);
            }
            else
            {
               rc = arp_api_connected_up(p_db_if, p_db_rtif, p_db_conn, new_up_running);
            }
        }
        p_db_rtif->is_up_running = new_up_running;
    }

    return rc;
}
/*chenc end*/

int32
rt_if_running_change(tbl_interface_t *p_db_if, uint32 old_running, uint32 new_running)
{
    tbl_route_if_t *p_db_rtif   = NULL;
    int32           rc          = PM_E_NONE;

    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        rc = _rt_if_running_set_process(p_db_if->key.name, new_running);
    }
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&p_db_if->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }
    
    p_db_rtif->ipv6_link_local_had_delete = 0;

    rc = _rt_if_running_set_process(p_db_if->key.name, new_running);
    //rc = rt_if_running_update(p_db_if->key.name, new_running);
    /*for bug 51460 by chenc, 2019-03-28*/
    rc = rt_if_running_update_arp(p_db_if->key.name, new_running);
    /*chenc end*/
    rc = rt_if_running_update_v6(p_db_if->key.name, new_running);

    return rc;
}

int32
rt_if_add_static_route_v6_by_syscmd(addr_ipv6_t *dest, addr_ipv6_t *gate,
                        uint8 prefixlen, uint32 distance, uint32 ifindex)
{
    char        buf[CMD_BUF_256];
    char        buf2[CMD_BUF_256];
    char        cmd_str[128];
    char ifname[IFNAME_EXT_SIZE] = {0};

    if (0 != ifindex)
    {
        if (0 != tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_EXT_SIZE))
        {
            MY_PRINTF("get ifname by ifindex %u failed", ifindex);
        }
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    if (1 != distance)
    {
        sal_sprintf(cmd_str, "ipv6 route %s/%d %s %s %d",
                        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, dest),
                        prefixlen,
                        cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate),
                        ifname,
                        distance);
    }
    else
    {
        sal_sprintf(cmd_str, "ipv6 route %s/%d %s %s",
                    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, dest),
                    prefixlen,
                    cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate),
                    ifname);
    }
    rt_vty_config_cmd(cmd_str);

    return PM_E_NONE;
}

int32
rt_if_del_static_route_v6_by_syscmd(addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen, uint32 ifindex)
{
    char        buf[CMD_BUF_256];
    char        buf2[CMD_BUF_256];
    char        cmd_str[128];
    char ifname[IFNAME_EXT_SIZE] = {0};

    if (0 != ifindex)
    {
        if (0 != tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_EXT_SIZE))
        {
            MY_PRINTF("get ifname by ifindex %u failed", ifindex);
        }
    }
    
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "no ipv6 route %s/%d %s %s",
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, dest),
                prefixlen,
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, gate),
                ifname);
    rt_vty_config_cmd(cmd_str);

    return PM_E_NONE;
}

int32
rt_if_add_static_route_by_syscmd(addr_ipv4_t *dest, addr_ipv4_t *gate,
                        uint8 prefixlen, uint32 distance)
{
    char        buf[CMD_BUF_256];
    char        buf2[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t     tmp_zero;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));

    if (1 != distance)
    {
        if (0 != sal_memcmp(&tmp_zero, gate, sizeof(tmp_zero)))
        {
            sal_sprintf(cmd_str, "ip route %s/%d %s %d",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen,
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate),
                        distance);
        }
        else
        {
            sal_sprintf(cmd_str, "ip route %s/%d null0 %d",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen,
                        distance);
        }
    }
    else
    {
        if (0 != sal_memcmp(&tmp_zero, gate, sizeof(tmp_zero)))
        {
            sal_sprintf(cmd_str, "ip route %s/%d %s",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen,
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate));
        }
        else
        {
            sal_sprintf(cmd_str, "ip route %s/%d null0",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen);
        }
    }
    rt_vty_config_cmd(cmd_str);

    return PM_E_NONE;
}

int32
rt_if_del_static_route_by_syscmd(addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen)
{
    char        buf[CMD_BUF_256];
    char        buf2[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t     tmp_zero;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));

    if (0 != sal_memcmp(&tmp_zero, gate, sizeof(tmp_zero)))
    {
        sal_sprintf(cmd_str, "no ip route %s/%d %s",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen,
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, gate));
    }
    else
    {
        sal_sprintf(cmd_str, "no ip route %s/%d null0",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, dest),
                        prefixlen);
    }

    rt_vty_config_cmd(cmd_str);

    return PM_E_NONE;
}


int32
rt_if_del_route_v6_by_ifaddr(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_t  *pst_iproute_node = obj;
    tbl_route_if_t      *p_db_rtif  = pargs->argv[0];
    addr_ipv6_t         *nhaddr     = pargs->argv[1];
    addr_ipv6_t         gate_addr;
    addr_ipv6_t         dest;
    addr_ipv6_t         gate;
    vrf_id_t            vrf_id;
    char                ifname[IFNAME_SIZE];
    char                buf[CMD_BUF_64];
    uint8               prefixlen   = 0;
    uint32              distance    = 0;
    int                 ret         = PM_E_NONE;

    if (NULL == pst_iproute_node || NULL == p_db_rtif || NULL == nhaddr)
    {
        return PM_E_FAIL;
    }

    sal_memset(&gate_addr, 0, sizeof(gate_addr));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(buf, 0, sizeof(buf));
    sal_memcpy(ifname, p_db_rtif->key.name, sizeof(ifname));

    vrf_id      = pst_iproute_node->key.vrf_id;
    sal_memcpy(&dest, &pst_iproute_node->key.p.u.prefix6, sizeof(addr_ipv6_t));
    sal_memcpy(&gate, &pst_iproute_node->nh_key.u.ipv6, sizeof(addr_ipv6_t));
    prefixlen   = pst_iproute_node->key.p.prefixlen;
    distance    = pst_iproute_node->distance;

    /* get static route */
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;

    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = AF_INET6;
    static_route.key.p.prefixlen = prefixlen;
    sal_memcpy(&static_route.key.nh_addr.ipv6, &gate, sizeof(addr_ipv6_t));
    sal_memcpy (&static_route.key.p.u.prefix6, &dest, sizeof(addr_ipv6_t));

    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    if (NULL != pst_static_route)
    {
        if (0 == pst_iproute_node->multipath && NH_TYPE_IPV6 == pst_iproute_node->nh_key.type)
        {
            if (0 == sal_memcmp(&pst_iproute_node->nh_key.u.ipv6, nhaddr, sizeof(addr_ipv6_t))
                    || 0 == sal_strcmp(pst_iproute_node->nh_key.ifname, ifname))
            {
                ret = rt_if_del_static_route_v6_by_syscmd(&dest, &gate, prefixlen, pst_static_route->key.ifindex_v6);
                ret = rt_if_add_static_route_v6_by_syscmd(&dest, &gate, prefixlen, distance, pst_static_route->key.ifindex_v6);
            }
        }
    }

    return ret;
}

int32
rt_if_rt_entry_v6_delete_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv6_t *nh_addr)
{
    tbl_iter_args_t     args;
    int32               ret         = PM_E_NONE;

    if (NULL == p_db_rtif || NULL == nh_addr)
    {
        return PM_E_NONE;
    }

    args.argv[0] = p_db_rtif;
    args.argv[1] = nh_addr;
    ret = tbl_iproute_node_iterate(rt_if_del_route_v6_by_ifaddr, &args);

    return ret;
}

int32
rt_if_del_route_by_ifaddr(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_t  *pst_iproute_node = obj;
    tbl_route_if_t      *p_db_rtif  = pargs->argv[0];
    addr_ipv4_t         *nhaddr     = pargs->argv[1];
    addr_ipv4_t         gate_addr;
    addr_ipv4_t         dest;
    addr_ipv4_t         gate;
    vrf_id_t            vrf_id;
    char                ifname[IFNAME_SIZE];
    char                buf[CMD_BUF_64];
    uint8               prefixlen   = 0;
    uint32              distance    = 0;
    int                 ret         = PM_E_NONE;

    if (NULL == pst_iproute_node || NULL == p_db_rtif || NULL == nhaddr)
    {
        return PM_E_FAIL;
    }

    sal_memset(&gate_addr, 0, sizeof(gate_addr));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(buf, 0, sizeof(buf));
    sal_memcpy(ifname, p_db_rtif->key.name, sizeof(ifname));

    vrf_id      = pst_iproute_node->key.vrf_id;
    dest.s_addr = pst_iproute_node->key.p.u.prefix4.s_addr;
    gate.s_addr = pst_iproute_node->nh_key.u.ipv4.s_addr;
    prefixlen   = pst_iproute_node->key.p.prefixlen;
    distance    = pst_iproute_node->distance;

    /* get static route */
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;

    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = AF_INET;
    static_route.key.p.prefixlen = prefixlen;
    sal_memcpy(&static_route.key.nh_addr.ipv4, &gate, sizeof(addr_ipv4_t));
    sal_memcpy (&static_route.key.p.u.prefix4, &dest, sizeof(addr_ipv4_t));

    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    if (NULL != pst_static_route)
    {
        if (0 == pst_iproute_node->multipath && NH_TYPE_IPV4 == pst_iproute_node->nh_key.type)
        {
            if (0 == sal_memcmp(&pst_iproute_node->nh_key.u.ipv4, nhaddr, sizeof(addr_ipv4_t))
                    || 0 == sal_strcmp(pst_iproute_node->nh_key.ifname, ifname))
            {
                ret = rt_if_del_static_route_by_syscmd(&dest, &gate, prefixlen);
                ret = rt_if_add_static_route_by_syscmd(&dest, &gate, prefixlen, distance);
            }
        }
    }

    return ret;
}

int32
rt_if_rt_entry_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv4_t *nh_addr)
{
    tbl_iter_args_t     args;
    int32               ret         = PM_E_NONE;

    if (NULL == p_db_rtif || NULL == nh_addr)
    {
        return PM_E_NONE;
    }

    args.argv[0] = p_db_rtif;
    args.argv[1] = nh_addr;
    ret = tbl_iproute_node_iterate(rt_if_del_route_by_ifaddr, &args);

    return ret;
}

int32
rt_if_up_change(tbl_interface_t *p_db_if, uint32 old_up, uint32 new_up)
{
    tbl_route_if_t  *p_db_rtif = NULL;
    int32           rc = PM_E_NONE;

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&p_db_if->key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        rc = rt_if_up_update(p_db_if->key.name, new_up);
    }
    rc = rt_if_up_set_process(p_db_if, p_db_rtif, new_up);

    return rc;
}

int32
rt_if_connected_down_ipv6(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_db_conn)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ds_connected_v6_t      *p_tmp_conn = NULL;
    uint32              is_running  = 0;
    int32               rc          = PM_E_NONE;

    if (NULL == p_db_rtif || NULL == p_db_if || NULL == p_db_conn)
    {
        return PM_E_FAIL;
    }

    is_running = tbl_interface_is_running(p_db_if);
    if (is_running)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_tmp_conn, listnode)
        {
            if (0 == sal_memcmp(p_db_conn, p_tmp_conn, sizeof(ds_connected_v6_t)))
            {
                continue;
            }
            else
            {
                rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_tmp_conn, TRUE);
            }
        }
    }

    return rc;
}

int32
rt_if_connected_down_ipv4(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_db_conn)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    uint32              is_running  = 0;
    int32               rc          = PM_E_NONE;

    if (NULL == p_db_rtif || NULL == p_db_if || NULL == p_db_conn)
    {
        return PM_E_FAIL;
    }

    is_running = tbl_interface_is_running(p_db_if);
    if (is_running)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_tmp_conn, listnode)
        {
            if (0 == sal_memcmp(p_db_conn, p_tmp_conn, sizeof(ds_connected_t)))
            {
                continue;
            }
            else
            {
                rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_tmp_conn, TRUE);
            }
        }
    }

    return rc;
}

static int32
_rt_if_del_ipv6_check_up_process(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn)
{
    char buf[CMD_BUF_64];
    int32 rc = PM_E_NONE;

    /* noted by kcao, 2015-01-05, when set shutdown, switchport quickly
     * maybe up flag is clear, but running flag is not clear when changed to switchport
     * then, only check the running flag here
     */

    if (rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable))
    {
        rc = (rt_fea_ipv6_del_addr(GLB_DEF_VRFID, p_conn, p_db_if->ifindex));
        if (rc < 0)
        {
            log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete FEA addr %s failed, rc = %d\n",
                p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
            return rc;
        }
    }
    
    return rc;
}

static int32
_rt_if_add_ipv4_check_up_process(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    ds_connected_t  *p_db_conn = NULL;
    int32           rc = PM_E_NONE;

    if (tbl_interface_is_running(p_db_if) && rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable))
    {
        p_db_conn = ds_connected_get_connected(p_db_rtif, p_conn);
        PM_E_RETURN(rt_fea_ipv4_add_addr(GLB_DEF_VRFID, p_db_conn, p_db_if->ifindex));
    }

    return rc;
}

static int32
_rt_if_del_ipv4_check_up_process(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    char buf[CMD_BUF_64];
    int32 rc = PM_E_NONE;

    /* noted by kcao, 2015-01-05, when set shutdown, switchport quickly
     * maybe up flag is clear, but running flag is not clear when changed to switchport
     * then, only check the running flag here
     */

    if (rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable))
    {
        rc = (rt_fea_ipv4_del_addr(GLB_DEF_VRFID, p_conn, p_db_if->ifindex));
        if (rc < 0)
        {
            log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete FEA addr %s failed, rc = %d\n",
                p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
            return rc;
        }
    }

    return rc;
}

int32
rt_if_addr_v6_del_update(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *ifc)
{
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t      *ifc_tmp = NULL;
    int32               add_del_update = TRUE;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, ifc_tmp, listnode)
    {
        if (IPV6_ADDR_SAME(&ifc->key.address.u.prefix, &ifc_tmp->key.address.u.prefix))
        {
            if (prefix_compare_net(&ifc->key.address, &ifc_tmp->key.address) 
                || prefix_compare_net(&ifc_tmp->key.address, &ifc->key.address))
            {
                add_del_update = FALSE;
            }
        }
    }

    return add_del_update;
}

int32
rt_if_addr_del_update(tbl_route_if_t *p_db_rtif, ds_connected_t *ifc)
{
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *ifc_tmp = NULL;
    int32               add_del_update = TRUE;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
    {
        if (ifc->key.address.u.prefix4.s_addr != ifc_tmp->key.address.u.prefix4.s_addr)
        {
            /*compare net addr*/
            if (prefix_compare_net(&ifc->key.address, &ifc_tmp->key.address) 
                || prefix_compare_net(&ifc_tmp->key.address, &ifc->key.address))
            {
                add_del_update = FALSE;
            }
        }
    }

    return add_del_update;
}

ds_connected_v6_t *
if_match_ifc_ipv6_direct (tbl_route_if_t *ifp, prefix_t *p)
{
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t *ifc = NULL;
    prefix_t *q;

    CTCLIB_SLIST_LOOP(ifp->ifc_ipv6.obj_list, ifc, listnode)
    {
        if ((q = &ifc->key.address)
            && ! IN6_IS_ADDR_LINKLOCAL (&q->u.prefix6)
            && ! IN6_IS_ADDR_LOOPBACK (&q->u.prefix6)
            && ! IN6_IS_ADDR_UNSPECIFIED (&q->u.prefix6))
        {
            /*not sopport p2p if*/
            #if 0
            if (if_is_pointopoint (ifp))
            {
                if ((q = ifc->destination)
                    && sal_prefix_same (p, q))
                {
                    break;
                }
            }
            else
            #endif
            {
                if (sal_prefix_match (q, p))
                {
                    break;
                }
            }
        }
    }

  return ifc;
}

ds_connected_v6_t *
if_lookup_ifc_ipv6_linklocal (tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t *p_conn;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_conn, listnode)
    {
        if (IN6_IS_ADDR_LINKLOCAL (&p_conn->key.address.u.prefix6))
        {
            return p_conn;
        }
    }

    return NULL;
}

int32
hal_l3if_set_ipv6_ucast_en(tbl_route_if_t * p_route_if, uint32 ipv6_unicast_en)
{
    int32 rc = PM_E_NONE;

    p_route_if->unicast_en_v6 = ipv6_unicast_en;

    /*sync to fea*/
    rc = tbl_route_if_set_route_if_field(p_route_if, TBL_ROUTE_IF_FLD_UNICAST_EN_V6);

    return rc;
}

/*first config static route, then add addr is static route's nexthop for interface, it will delete the static route from frr*/
/*rt_if_rt_entry_v6_delete_by_ifaddr*/
int32
rt_if_v6_delete_static_route_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv6_t *addr)
{
    if (NULL == p_db_rtif || NULL == addr)
    {
        return PM_E_NONE;
    }

    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_static_route = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_static_route, listnode, next)
    {
        if (0 == sal_memcmp(&p_db_static_route->key.nh_addr.ipv6, addr, sizeof(addr_ipv6_t)))
        {
            rc = rt_if_del_static_route_v6_by_syscmd(&p_db_static_route->key.p.u.prefix6, addr, p_db_static_route->key.p.prefixlen, p_db_static_route->key.ifindex_v6);
        }
    
    }
    return rc;
}

/*first config static route, then add addr is static route's nexthop for interface, then delete addr, it will add the static route to frr*/
int32
rt_if_v6_add_static_route_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv6_t *addr)
{
    if (NULL == p_db_rtif || NULL == addr)
    {
        return PM_E_NONE;
    }

    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_static_route = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_static_route, listnode, next)
    {
        if (0 == sal_memcmp(&p_db_static_route->key.nh_addr.ipv6, addr, sizeof(addr_ipv6_t)))
        {
            rc = rt_if_add_static_route_v6_by_syscmd(&p_db_static_route->key.p.u.prefix6, addr, p_db_static_route->key.p.prefixlen, p_db_static_route->distance, p_db_static_route->key.ifindex_v6);
        }
    
    }
    return rc;
}

int32
rt_if_add_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, mac_addr_t vmac)
{
    tbl_interface_t *p_db_if = NULL;
    //ds_connected_v6_t  *p_db_conn_primary = NULL;
    //uint32          is_running = 0;
    int32           rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    //is_running = tbl_interface_is_running(p_db_if);

    /*config ipv6 addr, not need to delete addr*/
    #if 0
    if (!GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)
        /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
        && !GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL))
        /* liwh end */
    {
        if (!CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list))
        {
            //p_db_conn_primary = rt_if_get_primary_connected(p_db_rtif);
        }
    }

    if (p_db_conn_primary)
    {
        PM_E_RETURN(rt_if_del_ipv6(p_db_rtif, p_db_conn_primary));
    }
    #endif

    PM_E_RETURN(ds_connected_v6_add_connected_v6(p_db_rtif, p_conn));
    rt_if_indirect_ip_route_inc(p_db_if, FALSE, IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6));
    //rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_conn, tbl_interface_is_running(p_db_if));
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       add vmac by liwh */

    // TODO:ndp
    //rc = arp_api_connected_add(p_db_if, p_db_rtif, p_conn, 1, is_running, vmac);
    /*useless*/
    //PM_E_RETURN(_rt_if_add_ipv4_check_up_process(p_db_if, p_db_rtif, p_conn));

    //rt_if_v6_delete_static_route_by_ifaddr(p_db_rtif, &p_conn->key.address.u.prefix6);
    return rc;
}

int32
rt_if_del_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn)
{
    tbl_interface_t *p_db_if = NULL;
    ds_connected_v6_t  *p_db_conn = NULL;
    char            buf[CMD_BUF_64];
    int32           need_update = TRUE;
    int32           rc = PM_E_NONE;

    p_db_conn = ds_connected_v6_get_connected_v6(p_db_rtif, p_conn);
    if (NULL == p_db_conn)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    rc = (_rt_if_del_ipv6_check_up_process(p_db_if, p_db_rtif, p_conn));
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete addr %s failed, rc = %d\n",
            p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
        return rc;
    }

    need_update = rt_if_addr_v6_del_update(p_db_rtif, p_conn);
    if (need_update)
    {
        // TODO: cc maybe args add ipv6 addr, after delete ip address, if delete route?
        /*modified by chenc for remove patch function, 2019-05-11*/
        #ifndef USE_FPM_IPUC
        tbl_iter_args_t args;
        args.argv[0] = p_db_rtif;
        tbl_iproute_node_iterate(rt_fea_del_route_by_ifname, &args);
        #endif
    }

    rc = rt_fib_connected_update_ipv6(p_db_if, p_db_rtif, p_conn, FALSE);
    rt_if_connected_down_ipv6(p_db_if, p_db_rtif, p_db_conn);

    // TODO: cc
    rt_if_indirect_ip_route_dec(p_db_if, FALSE, IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6));
    //rc = arp_api_connected_del(p_db_if, p_db_rtif, p_db_conn);
    rc = (ds_connected_v6_del_connected_v6(p_db_rtif, p_conn));
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete connected addr %s failed, rc = %d\n",
            p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
        return rc;
    }

    return rc;
}

int32
rt_if_add_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn,
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    mac_addr_t vmac)
    /* liwh end */
{
    tbl_interface_t *p_db_if = NULL;
    ds_connected_t  *p_db_conn_primary = NULL;
    uint32          is_running = 0;
    int32           rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    is_running = tbl_interface_is_running(p_db_if);
    if (!GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)
        /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
        && !GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL))
        /* liwh end */
    {
        if (!CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list))
        {
            p_db_conn_primary = rt_if_get_primary_connected(p_db_rtif);
        }
    }

    if (p_db_conn_primary)
    {
        PM_E_RETURN(rt_if_del_ipv4(p_db_rtif, p_db_conn_primary));
    }

    PM_E_RETURN(ds_connected_add_connected(p_db_rtif, p_conn));
    rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_conn, tbl_interface_is_running(p_db_if));
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       add vmac by liwh */
    rc = arp_api_connected_add(p_db_if, p_db_rtif, p_conn, 1, is_running, vmac);
    PM_E_RETURN(_rt_if_add_ipv4_check_up_process(p_db_if, p_db_rtif, p_conn));

    nat_api_rtif_connected_add(p_db_if, p_db_rtif, p_conn); /* added by taocy for NAT, 2019/06/03*/
    return rc;
}

int32
rt_if_del_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    tbl_interface_t *p_db_if = NULL;
    ds_connected_t  *p_db_conn = NULL;
    char            buf[CMD_BUF_64];
    int32           need_update = TRUE;
    int32           rc = PM_E_NONE;

    p_db_conn = ds_connected_get_connected(p_db_rtif, p_conn);
    if (NULL == p_db_conn)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    rc = (_rt_if_del_ipv4_check_up_process(p_db_if, p_db_rtif, p_conn));
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete addr %s failed, rc = %d\n",
            p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
        return rc;
    }

    need_update = rt_if_addr_del_update(p_db_rtif, p_conn);
    if (need_update)
    {
        /*modified by chenc for remove patch function, 2019-05-11*/
        #ifndef USE_FPM_IPUC
        tbl_iter_args_t args;
        args.argv[0] = p_db_rtif;
        tbl_iproute_node_iterate(rt_fea_del_route_by_ifname, &args);
        #endif
    }

    rc = rt_fib_connected_update_ipv4(p_db_if, p_db_rtif, p_conn, FALSE);
    rt_if_connected_down_ipv4(p_db_if, p_db_rtif, p_db_conn);

    rc = arp_api_connected_del(p_db_if, p_db_rtif, p_db_conn);
    nat_api_rtif_connected_del(p_db_if, p_db_rtif, p_db_conn); /* added by taocy for NAT, 2019/06/03*/
    rc = (ds_connected_del_connected(p_db_rtif, p_conn));
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Interface %s delete connected addr %s failed, rc = %d\n",
            p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE), rc);
        return rc;
    }

    return rc;
}

int32
ipv6_address_del(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, uint32 commit_frr)
{
    int32 rc = PM_E_NONE;
    //uint32          mask_len    = 0;
    addr_ipv6_t     ip_addr_cmd;
    char        buf[CMD_BUF_64] = {0};

    MY_PRINTF("delete interface %s ipv6 address %s", 
        p_db_rtif->key.name, prefix_val2str(buf, CMD_BUF_64, &p_conn->key.address, TRUE));
    
    sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
    sal_memcpy(&ip_addr_cmd, &p_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
    /*add normal ipv6 address, add link-local address, del link-local address, del normal ipv6 address, 
    p_conn->destination.prefixlen is 0*/
    //mask_len = p_conn->destination.prefixlen;
    //mask_len = p_conn->key.address.prefixlen;

    /* delete neighbor entry for ipv6 address from chip and kernel */
    neigh_ifc_del(p_db_rtif, p_conn);
    /* liwh end */
    
    rc = rt_if_del_ipv6(p_db_rtif, p_conn);

    /*when delete ipv6 address, neigh_ifc_del->rt_if_connected_invalid_ipv6->rt_cmd_del_if_addr_v6_by_syscmd will delete first, 
    so here cannot find it and delete*/
    if (commit_frr)
    {
        //rc += rt_cmd_del_if_addr_v6_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len);
    }
    
    /*add static route then add if addr, then delete if addr, then add if addr, route not active, so delete it*/
    //rc += rt_if_rt_entry_v6_delete_by_ifaddr(p_db_rtif, &ip_addr_cmd);

    return rc;
}

int32
rt_if_clear_kernel_if_by_ifname(void *obj, tbl_iter_args_t *pargs)
{
    char *kifname = (char *)pargs->argv[0];
    tbl_kernel_if_t     *pst_kernel_if  = obj;

    if (NULL != pst_kernel_if)
    {
        if (0 == sal_memcmp(pst_kernel_if->name, kifname, sizeof(pst_kernel_if->name)))
        {
            tbl_kernel_if_del_kernel_if(&pst_kernel_if->key);
        }
    }

    return PM_E_NONE;
}

int32
rt_if_clear_rtif_urpf_en(tbl_route_if_t *p_db_rtif)
{
    p_db_rtif->unicast_rpf_en = 0;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_UNICAST_RPF_EN);
    
    return PM_E_NONE;
}

int32
rt_if_clear_rtif_urpf_en_v6(tbl_route_if_t *p_db_rtif)
{
    p_db_rtif->unicast_rpf_en_v6 = 0;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_UNICAST_RPF_EN_V6);
    
    return PM_E_NONE;
}

int32
rt_if_clear_ospf_if(tbl_route_if_t *p_db_rtif)
{
    char cmd_str[128];
    char auth_key[256];
    ctclib_slistnode_t  *rt_listnode    = NULL;
    ds_ospf_auth_t      *p_db_auth_md5  = NULL;

    if (OSPF_DEAD_INTERVAL_DEFAULT != p_db_rtif->ospf_timer_dead)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf dead-interval");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_timer_dead = OSPF_DEAD_INTERVAL_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_TIMER_DEAD);
    }

    if (OSPF_HELLO_INTERVAL_DEFAULT != p_db_rtif->ospf_timer_hello)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf hello-interval");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_timer_hello = OSPF_HELLO_INTERVAL_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_TIMER_HELLO);
    }
	
    if (p_db_rtif->ospf_mtu_ignore)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf mtu-ignore");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);

        p_db_rtif->ospf_mtu_ignore = FALSE;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_MTU_IGNORE);        
    }

    if (p_db_rtif->ospf_cost)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf cost");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_cost = 0;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_COST);        
    }

    sal_memset(auth_key, 0, sizeof(auth_key));
    if (sal_memcmp(p_db_rtif->ospf_auth_key, auth_key, sizeof(auth_key)))
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf authentication-key");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        sal_memset(p_db_rtif->ospf_auth_key, 0, sizeof(auth_key));
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_KEY);
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ospf_md5_key.obj_list, p_db_auth_md5, rt_listnode)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf message-digest-key %u", p_db_auth_md5->key.id);
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    }

    if((OSPF_RIF_AUTH_TYPE_MD5 == p_db_rtif->ospf_auth_type) || (OSPF_RIF_AUTH_TYPE_SIMPLE == p_db_rtif->ospf_auth_type))
	{
        sal_sprintf(cmd_str, "no ip ospf authentication");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_auth_type = OSPF_RIF_AUTH_TYPE_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_TYPE);
    }

    if (OSPF_NETWORK_TYPE_DEFAULT != p_db_rtif->ospf_network_type)
    {
        sal_sprintf(cmd_str, "no ip ospf network");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_network_type = OSPF_NETWORK_TYPE_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_NETWORK_TYPE);
    }

    return PM_E_NONE;
}

int32
rt_if_clear_ospf6_if(tbl_route_if_t *p_db_rtif)
{
    char cmd_str[128];
    char auth_key[256];
    ctclib_slistnode_t  *rt_listnode    = NULL;
    ds_ospf_auth_t      *p_db_auth_md5  = NULL;

    if (OSPF_DEAD_INTERVAL_DEFAULT != p_db_rtif->ospf6_timer_dead)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ipv6 ospf6 dead-interval");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf6_timer_dead = OSPF_DEAD_INTERVAL_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_TIMER_DEAD);
    }

    if (OSPF_HELLO_INTERVAL_DEFAULT != p_db_rtif->ospf6_timer_hello)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ipv6 ospf6 hello-interval");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf6_timer_hello = OSPF_HELLO_INTERVAL_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_TIMER_HELLO);
    }
#if 0	
    if (p_db_rtif->ospf_mtu_ignore)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf mtu-ignore");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);

        p_db_rtif->ospf_mtu_ignore = FALSE;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_MTU_IGNORE);        
    }

    if (p_db_rtif->ospf_cost)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf cost");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_cost = 0;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_COST);        
    }

    sal_memset(auth_key, 0, sizeof(auth_key));
    if (sal_memcmp(p_db_rtif->ospf_auth_key, auth_key, sizeof(auth_key)))
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf authentication-key");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        sal_memset(p_db_rtif->ospf_auth_key, 0, sizeof(auth_key));
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_KEY);
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ospf_md5_key.obj_list, p_db_auth_md5, rt_listnode)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip ospf message-digest-key %u", p_db_auth_md5->key.id);
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    }

    if((OSPF_RIF_AUTH_TYPE_MD5 == p_db_rtif->ospf_auth_type) || (OSPF_RIF_AUTH_TYPE_SIMPLE == p_db_rtif->ospf_auth_type))
	{
        sal_sprintf(cmd_str, "no ip ospf authentication");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_auth_type = OSPF_RIF_AUTH_TYPE_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_TYPE);
    }

    if (OSPF_NETWORK_TYPE_DEFAULT != p_db_rtif->ospf_network_type)
    {
        sal_sprintf(cmd_str, "no ip ospf network");
        rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
        p_db_rtif->ospf_network_type = OSPF_NETWORK_TYPE_DEFAULT;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_NETWORK_TYPE);
    }
#endif
    return PM_E_NONE;
}


// TODO: cc auto linklocal add first, then delete?
int32
rt_if_del_all_ipv6_linklocal(tbl_route_if_t *p_db_rtif, int32 disable, uint32 commit_frr)
{
    ds_connected_v6_t      *p_db_conn  = NULL;
    addr_ipv6_t in6_addr;

    sal_memset(&in6_addr, 0, sizeof(in6_addr));
    p_db_conn = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
    if (NULL != p_db_conn)
    {
        /*fix bug 48653 by chenc*/
        /*config link-local address is same as auto link-local address, then delete all ipv6 address, start*/
        if (GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL) && !disable)
        {
            tbl_interface_t * p_db_if = NULL;
            uint8 eui[8] = {0};
            
            p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
            if (!p_db_if)
            {
                return PM_E_FAIL;
            }
    
            sal_memset(&in6_addr, 0, sizeof(in6_addr));

            /* link-local address must exist, use auto-generated link-local
                address. "ipv6 enable" also means auto-generated address */
            in6_addr.s6_addr32[0] = sal_hton32(0xfe800000);
            in6_addr.s6_addr32[1] = sal_hton32(0x0);
            /*huangxt for 19814. auto address on loopback*/
            if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
            {
                rt_if_l3if_loopback_generate_eui64 (eui);
            }
            else
            {
                ipv6_generate_eui64 (eui, p_db_if);
            }
            
            sal_memcpy (&(in6_addr.s6_addr[IPV6_MAX_BYTELEN - EUI64_HOST_BYTES]),
                eui, EUI64_HOST_BYTES);

            /* check whether the auto-generated link-local address is equal
            to the installed link-local address */
            if (0 == IPV6_ADDR_CMP(&in6_addr, &p_db_conn->key.address.u.prefix6))
            {
                GLB_UNSET_FLAG(p_db_conn->flags, RT_IF_ADDRV6_LINKLOCAL);
                ds_connected_v6_set_connected_v6_field(p_db_rtif, p_db_conn, DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL);
                return PM_E_NONE;
            }
        }
        /*end*/
        
        ds_connected_v6_t conn_for_no_all;
        sal_memset(&conn_for_no_all, 0, sizeof(conn_for_no_all));
        sal_memcpy(&conn_for_no_all, p_db_conn, sizeof(conn_for_no_all));

        /*if link-local is set, the link-local address must is configured*/
        if (GLB_FLAG_ISSET(conn_for_no_all.flags, RT_IF_ADDRV6_LINKLOCAL))
        {
            GLB_UNSET_FLAG(conn_for_no_all.flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
        }
        /*if link-local is not set, auto link-local is set, the link-local address must is auto generated*/
        else if (GLB_FLAG_ISSET(conn_for_no_all.flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
        {
            /*auto link-local address not delete when no ipv6 address*/
            if (!disable)
            {
                return PM_E_NONE;
            }
            /*auto link-local address not delete when switchport*/
            else
            {
                
            }
        }
        /*link-local address is generated by normal, not delete here, 
            it delete at the end of _rt_if_cmd_del_ipv6 when delete normal adress */
        else
        {
            return PM_E_NONE;
        }
        
        _rt_if_cmd_del_ipv6(p_db_rtif, &conn_for_no_all, NULL, TRUE, disable);
    }

    return PM_E_NONE;
}

static void
_rt_if_delete_all_linklocal_age_entry(void* p_data)
{
    tbl_route_if_t* p_db_rtif = p_data ? (tbl_route_if_t*)p_data : NULL;

    p_db_rtif->t_delete_all_v6_addr_timer = NULL;
    MY_PRINTF("time is up, delete link-local address");
    rt_if_del_all_ipv6_linklocal(p_db_rtif, FALSE, TRUE);

    return;
}

/*this func has been called by cmd set switchport or no ipv6 address all, 
when config switchport, it will 
static_delete_by_if->nd_clear_nd_entries_and_config->nd_if_stop->
rt_if_connected_invalid_ipv6->rt_cmd_del_if_addr_v6_by_syscmd
delete first, here cannot find the address, frr will print cannot find address*/
int32
rt_if_del_all_ipv6(tbl_route_if_t *p_db_rtif, int32 disable, uint32 delete_by_cmd)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ctclib_slistnode_t  *next       = NULL;
    ds_connected_v6_t      *p_db_conn  = NULL;
//    addr_ipv6_t         ip_addr_cmd;
//    uint32              mask_len    = 0;
    int32               rc          = PM_E_NONE;
    uint32 commit_frr = TRUE;

    /*when switchport, the address has been delete by nd, so not commit to frr*/
    if (!delete_by_cmd)
    {
        commit_frr = FALSE;
    }

    /*for bug 48656 by chenc*/
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, listnode, next)
    {
        /*fix bug 48563, global ipv6 unicast address is at first, link-local is the second
        when delete the global address, if there is not other address, it will delete link-local
        address auto, so link-local address pointer become invalid, then second delete 
        link-local address will generate core*/
        if (p_db_rtif->ifc_ipv6.obj_list->count == 0)
        {
            continue;
        }
        
        if (IN6_IS_ADDR_LINKLOCAL(&(p_db_conn->key.address.u.prefix6)))
        {
            continue;
        }

        /* Skip the loopback address.  */
        if (TRUE == rt_if_is_loopback (p_db_rtif->key.name))
        {
            if (IN6_IS_ADDR_LOOPBACK (&p_db_conn->key.address.u.prefix6))
            {
                continue;
            }
        }

        //rc = ipv6_address_del (p_db_rtif, p_db_conn, commit_frr);
        _rt_if_cmd_del_ipv6(p_db_rtif, p_db_conn, NULL, TRUE, disable);
    }

    /*fix bug 48792 by chenc*/
    if (delete_by_cmd)
    {
        CTC_TASK_STOP_TIMER(p_db_rtif->t_delete_all_v6_addr_timer);
        p_db_rtif->t_delete_all_v6_addr_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, _rt_if_delete_all_linklocal_age_entry, p_db_rtif, 50);
    }
    else
    {
        rt_if_del_all_ipv6_linklocal(p_db_rtif, disable, commit_frr);

        p_db_conn = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
        if (NULL != p_db_conn)
        {
            if ((GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL)) && (!disable))
            {
                /*if auto link-local is enable, auto link-local will not delete, so connected_v6 count is 1*/
                
                if (ctclib_slist_count(p_db_rtif->ifc_ipv6.obj_list) != 1)
                {
                    log_sys(M_MOD_ROUTE, E_ERROR, "Delete all IPv6 address failed for IPv6 address count is not 1\n");
                    return PM_E_FAIL;
                }
                else
                {
                    return rc;
                }
            }
        }
        
        if (! CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv6.obj_list))
        {
            log_sys(M_MOD_ROUTE, E_ERROR, "Delete all IPv6 address failed for IPv6 address count is not 0\n");
            return PM_E_FAIL;
        }
    }
    return rc;
}

int32
rt_if_del_all_ipv4(tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ctclib_slistnode_t  *next       = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    addr_ipv4_t         ip_addr_cmd;
    uint32              mask_len    = 0;
    int32               rc          = PM_E_NONE;
    tbl_interface_t     *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode, next)
    {
        sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
        sal_memcpy(&ip_addr_cmd, &p_db_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
        mask_len = p_db_conn->destination.prefixlen;

        /* 3. call API */
        rt_if_del_ipv4(p_db_rtif, p_db_conn);
        rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);

        rt_if_indirect_ip_route_dec(p_db_if, TRUE, FALSE);
    }

    SAL_ASSERT(CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list));
    return rc;
}

int32
rt_if_del_all_ipv4_secondary(tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ctclib_slistnode_t  *next       = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    addr_ipv4_t         ip_addr_cmd;
    uint32              mask_len    = 0;
    int32               rc          = PM_E_NONE;
    tbl_interface_t     *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode, next)
    {
        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            continue;
        }

        sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
        sal_memcpy(&ip_addr_cmd, &p_db_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
        mask_len = p_db_conn->destination.prefixlen;

        /* 3. call API */
        rt_if_del_ipv4(p_db_rtif, p_db_conn);
        rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);

        rt_if_indirect_ip_route_dec(p_db_if, TRUE, FALSE);
        
        /*modified by chenc for remove patch function, 2019-05-11*/
        #ifndef USE_FPM_IPUC
        rt_if_rt_entry_by_ifaddr(p_db_rtif, &ip_addr_cmd);
        #endif
    }

    return rc;
}


int32
rt_if_set_arp_retry_interval(char *ifname, uint32 retry_interval)
{
    tbl_route_if_t      *p_db_rtif = NULL;
    tbl_route_if_key_t  rt_if_key;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, ifname);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    p_db_rtif->arp_retrans_time = retry_interval;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME);
    return PM_E_NONE;
}

int32
rt_if_cmd_set_arp_ageing_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout)
{
    uint32 ifindex  = 0;
    int32 interval  = 0;
    int32 rc        = PM_E_NONE;

    /* 1. process */
    interval = p_db_rtif->arp_timeout - timeout;

    /* update cache entries */
    ifindex = tbl_interface_get_ifindex_by_name(p_db_rtif->key.name);
    arp_api_trav_ifindex(ifindex, arp_api_aging_time_update, (void*)&interval, NULL);

    /* 2. update local CDB */
    p_db_rtif->arp_timeout = timeout;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_TIMEOUT));
    return rc;
}

int32
rt_if_set_arp_retry_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    rt_if_set_nei_retrans(p_db_rtif, timeout);

    /* 2. update local CDB */
    p_db_rtif->arp_retry_timeout = timeout;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RETRY_TIMEOUT));
    return rc;
}

int32
rt_if_set_arp_timeout(char *ifname, uint32 timeout)
{
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_route_if_key_t rt_if_key;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, ifname);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    p_db_rtif->arp_timeout = timeout;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_TIMEOUT);
    return PM_E_NONE;
}

/*add by zhw for mc*/
int32
rt_if_set_pim_mode(tbl_route_if_t *p_db_rtif, uint8 pim_mode)
{
	int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->pim_mode = pim_mode;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_PIM_MODE));
    return rc;
}

int32
rt_if_set_multicast(tbl_route_if_t *p_db_rtif, uint8 mode)
{
	int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->multicast= mode;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_MULTICAST));
    return rc;
}
/*end add zhw*/

int32
rt_if_set_ucast_rpf_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->unicast_rpf_en = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_UNICAST_RPF_EN));
    return rc;
}

int32
rt_if_set_ucast_rpf_en_v6(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->unicast_rpf_en_v6 = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_UNICAST_RPF_EN_V6));
    return rc;
}

int32
rt_if_set_rip_recv_version(tbl_route_if_t *p_db_rtif, uint8 recv_version)
{
	int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->rip_recv_version = recv_version;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_RECV_VERSION));
    return rc;
}

int32
rt_if_set_rip_send_version(tbl_route_if_t *p_db_rtif, uint8 send_version)
{
	int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->rip_send_version = send_version;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_SEND_VERSION));
    return rc;
}

int32
rt_if_set_rip_v2_broadcast(tbl_route_if_t *p_db_rtif, uint8 v2_broadcast)
{
	int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->rip_v2_broadcast = v2_broadcast;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_V2_BROADCAST));
    return rc;
}

int32
rt_if_set_rip_split_horizon(tbl_route_if_t *p_db_rtif, uint8 split_horizon)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	p_db_rtif->rip_split_horizon = split_horizon;
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_SPLIT_HORIZON));
	return rc;
}

int32
rt_if_set_ripng_split_horizon(tbl_route_if_t *p_db_rtif, uint8 split_horizon)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	p_db_rtif->ripng_split_horizon = split_horizon;
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIPNG_SPLIT_HORIZON));
	return rc;
}

int32
rt_if_set_rip_poisoned_reverse(tbl_route_if_t *p_db_rtif, uint8 poisoned_reverse)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	p_db_rtif->rip_poisoned_reverse = poisoned_reverse;
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_POISONED_REVERSE));
	return rc;
}

int32
rt_if_set_ripng_poisoned_reverse(tbl_route_if_t *p_db_rtif, uint8 poisoned_reverse)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	p_db_rtif->ripng_poisoned_reverse = poisoned_reverse;
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIPNG_POISONED_REVERSE));
	return rc;
}


int32
rt_if_set_rip_auth_mode(tbl_route_if_t *p_db_rtif, uint8 auth_mode)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	p_db_rtif->rip_auth_mode = auth_mode;
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_AUTH_MODE));
	return rc;
}

int32
rt_if_set_rip_auth_string(tbl_route_if_t *p_db_rtif, char* auth_string)
{
	int32 rc = PM_E_NONE;

	/* 1. process */
	/* 2. update local CDB */
	sal_memcpy(p_db_rtif->rip_auth_string,auth_string,sizeof(p_db_rtif->rip_auth_string));
	PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_RIP_AUTH_STRING));
	return rc;
}

int32
rt_if_set_ip_unreachable_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* only when rtif has configured IP address, could call sysctl, otherwise kernel will report error */
    p_db_kernel_if = tbl_kernel_if_get_by_name(p_db_rtif->key.name);
    if (NULL != p_db_kernel_if && !CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list))
    {
        PM_E_RETURN(lib_sysctl_set_intf_ip_unreachable_en(GLB_DEF_VRFID, p_db_rtif->key.name, enable));
    }

    /* 2. update local CDB */
    p_db_rtif->ip_unreachable_en = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_IP_UNREACHABLE_EN));
    return rc;
}

int32
rt_if_set_ospf_mtu_ignore_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */

    if (enable)
    {
        p_db_rtif->ospf_mtu_ignore = TRUE;
    }
    else
    {
        p_db_rtif->ospf_mtu_ignore = FALSE;

    }
    
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_MTU_IGNORE));
    return rc;
}

int32
rt_if_set_ospf6_mtu_ignore_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */

    if (enable)
    {
        p_db_rtif->ospf6_mtu_ignore = TRUE;
    }
    else
    {
        p_db_rtif->ospf6_mtu_ignore = FALSE;

    }
    
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_MTU_IGNORE));
    return rc;
}


int32
rt_if_set_ospf_hello_interval(tbl_route_if_t *p_db_rtif, uint32 interval)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_timer_hello = interval;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_TIMER_HELLO));
    return rc;
}

int32
rt_if_set_ospf6_hello_interval(tbl_route_if_t *p_db_rtif, uint32 interval)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf6_timer_hello = interval;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_TIMER_HELLO));
    return rc;
}


int32
rt_if_set_ospf_dead_interval(tbl_route_if_t *p_db_rtif, uint32 interval)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_timer_dead = interval;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_TIMER_DEAD));
    return rc;
}

int32
rt_if_set_ospf6_dead_interval(tbl_route_if_t *p_db_rtif, uint32 interval)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf6_timer_dead = interval;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_TIMER_DEAD));
    return rc;
}

int32
rt_if_set_ospf_cost(tbl_route_if_t *p_db_rtif, uint32 cost)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_cost = cost;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_COST));
    return rc;
}

int32
rt_if_set_ospf6_cost(tbl_route_if_t *p_db_rtif, uint32 cost)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf6_cost = cost;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_COST));
    return rc;
}


int32
rt_if_set_ospf_priority(tbl_route_if_t *p_db_rtif, uint32 priority)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_priority= priority;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_PRIORITY));
    return rc;
}

int32
rt_if_set_ospf6_priority(tbl_route_if_t *p_db_rtif, uint32 priority)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf6_priority= priority;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_PRIORITY));
    return rc;
}


int32
rt_if_set_ospf_auth_type(tbl_route_if_t *p_db_rtif, uint32 type)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_auth_type = type;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_TYPE));
    return rc;
}

int32
rt_if_set_ospf_network_type(tbl_route_if_t *p_db_rtif, uint32 type)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf_network_type = type;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_NETWORK_TYPE));
    return rc;
}

int32
rt_if_set_ospf6_network_type(tbl_route_if_t *p_db_rtif, uint32 type)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    p_db_rtif->ospf6_network_type = type;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF6_NETWORK_TYPE));
    return rc;
}


int32
rt_if_set_ospf_auth_key(tbl_route_if_t *p_db_rtif, char *key)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* 2. update local CDB */
    sal_memcpy(p_db_rtif->ospf_auth_key, key, sizeof(p_db_rtif->ospf_auth_key));
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_OSPF_AUTH_KEY));
    return rc;
}

int32
rt_if_add_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth)
{
    int32 rc = PM_E_NONE;

    rc = ds_ospf_auth_add_ospf_auth(p_db_rtif, p_auth);
    return rc;
}

int32
rt_if_del_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth)
{
    int32 rc = PM_E_NONE;

    rc = ds_ospf_auth_del_ospf_auth(p_db_rtif, p_auth);

    return rc;
}

int32
rt_if_del_all_ospf_auth_md5(tbl_route_if_t *p_db_rtif)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ctclib_slistnode_t  *next       = NULL;
    ds_ospf_auth_t      *p_db_auth  = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ospf_md5_key.obj_list, p_db_auth, listnode, next)
    {
        rt_if_del_ospf_auth_md5_key(p_db_rtif, p_db_auth);
    }

    SAL_ASSERT(CTCLIB_SLIST_ISEMPTY(p_db_rtif->ospf_md5_key.obj_list));
    return PM_E_NONE;
}

int32
rt_if_set_ip_redirects_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    int32           rc              = PM_E_NONE;

    /* 1. process */
    /* only when rtif has configured IP address, could call sysctl, otherwise kernel will report error */
    p_db_kernel_if = tbl_kernel_if_get_by_name(p_db_rtif->key.name);
    if (NULL != p_db_kernel_if && !CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list))
    {
        PM_E_RETURN(lib_sysctl_set_intf_ip_redirects_en(GLB_DEF_VRFID, p_db_rtif->key.name, enable));
    }

    /* 2. update local CDB */
    p_db_rtif->ip_redirects_en = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_IP_REDIRECTS_EN));
    return rc;
}

int32
rt_if_set_ospf_bandwidth(tbl_interface_t * p_db_if, uint32 ospf_bandwith)
{
    int32 rc = PM_E_NONE;
#ifndef TAPPRODUCT
    char cmd_str[128];
    uint32 bandwidth = 0;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    bandwidth = ospf_bandwith/RT_BANDWIDTH_DIVIDE;
    bandwidth = (bandwidth) ? bandwidth : 1;    /* bandwidth should be 1 at least */
    sal_sprintf(cmd_str, "bandwidth %u", bandwidth);
    rt_vty_if_cmd(p_db_if->key.name, cmd_str);

#endif
    return rc;
}

int32
rt_if_set_local_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /*disable ip redirect when local arp proxy is enabled*/
    rt_if_set_ip_redirects_en(p_db_rtif, !enable);

    /* 2. update local CDB */
    p_db_rtif->local_arp_proxy_en = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_LOCAL_ARP_PROXY_EN));
    return rc;
}

int32
rt_if_set_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->arp_proxy_en= enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_PROXY_EN));
    return rc;
}

int32
rt_if_set_dhcp_relay_option_trust(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->dhcp_relay_option_trust = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_RELAY_OPTION_TRUST));
    return rc;
}

int32
rt_if_set_dhcp_server_group(tbl_route_if_t *p_db_rtif, uint8 index)
{
    int32 rc = PM_E_NONE;
    uint8_t enabled = (0 != index);
    tbl_sys_global_t *pGlobal = tbl_sys_global_get_sys_global();

    /* 1. update local CDB */
    p_db_rtif->dhcp_server_group = index;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_SERVER_GROUP));

    /* 2. process */
    if (p_db_rtif->dhcp_pdu_enabled != enabled && pGlobal
            && pGlobal->dhcp_service_enable && pGlobal->dhcp_relay_enable)
    {
        p_db_rtif->dhcp_pdu_enabled = enabled;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
    }
    return rc;
}

int32
rt_if_set_dhcp_client_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->dhcp_client_enable= enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_CLIENT_ENABLE));
    return rc;
}

/* modified by liwh for bug 49365, 2018-11-27 */
int32
rt_if_set_dhcpv6_client_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->dhcpv6_client_enable= enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCPV6_CLIENT_ENABLE));
    return rc;
}
/*liwh end */

int32
rt_if_set_nat_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;
    uint32 category_id = GLB_IF_NAT_CATEGORY_ID_INVALID;
    tbl_nat_global_t * p_nat_global = NULL;

    /* 1. process */
    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;
    }

    if (GLB_IF_NAT_OUTSIDE == enable)
    {
        if (ctclib_opb_alloc_offset(&p_nat_global->category_id_opb, &category_id))
        {
            return PM_E_RESOURCE_FULL;
        }

        p_db_rtif->nat_category_id = category_id;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_NAT_CATEGORY_ID));
    }
    else if (p_db_rtif->nat_category_id != GLB_IF_NAT_CATEGORY_ID_INVALID)
    {
        category_id = p_db_rtif->nat_category_id;
        ctclib_opb_free_offset(&p_nat_global->category_id_opb, category_id);

        p_db_rtif->nat_category_id = GLB_IF_NAT_CATEGORY_ID_INVALID;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_NAT_CATEGORY_ID));
    }

    /* 2. update local CDB */
    p_db_rtif->nat_enable = enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_NAT_ENABLE));
    return rc;
}

int32
rt_if_set_nei_retrans(tbl_route_if_t *p_db_rtif, uint8 timelen)
{
    return lib_sysctl_set_nei_retrans(GLB_DEF_VRFID, p_db_rtif->key.name, timelen);
}

int32
rt_if_mtu_update_agg_member(tbl_interface_t *pst_interface, uint32 mtu)
{
    tbl_interface_t     *pst_intf_agg   = NULL;
    ctclib_slistnode_t  *listnode       = NULL;
    tbl_route_if_t      *p_db_rtif      = NULL;
    tbl_route_if_key_t  rt_if_key;

    /* no need to check first member */
    if (0 == pst_interface->lag->member_ports.obj_list->count)
    {
        return PM_E_NONE;
    }

    /* update agg member mtu */
    CTCLIB_SLIST_LOOP(pst_interface->lag->member_ports.obj_list, pst_intf_agg, listnode)
    {
        sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
        sal_strcpy(rt_if_key.name, pst_intf_agg->key.name);
        p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
        if (NULL == p_db_rtif)
        {
            return PM_E_NOT_EXIST;
        }

        p_db_rtif->mtu = mtu;
        tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_MTU);
    }

    return PM_E_NONE;
}

int32
rt_if_is_nonmirror_running(uint32 flags, uint32 mirror_enable)
{
    uint32 running = GLB_FLAG_ISSET(flags, GLB_IFF_RUNNING);
    uint32 no_mirror_dest = !GLB_FLAG_ISSET(mirror_enable, 0x01); // MIRROR_DEST_FLAG

    return running & no_mirror_dest;
}

