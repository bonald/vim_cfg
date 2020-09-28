#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_ns_route_ip_define.h"
#include "gen/tbl_ns_route_ip.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "arp_define.h"
#include "arp_api.h"
#include "arpratelimit_api.h"
#include "arp_sm.h"
#include "prefix.h"
#include "route.h"
#include "rt_if.h"
#include "rt_fea.h"
#include "vrrp.h"
#include "vrrp_api.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "nd_api.h"
#include "nd.h"
#include "nat_api.h"

extern int32
_rt_if_cmd_add_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, cfg_cmd_para_t *para);

static int
addrconf_ifid_eui48 (u_int8_t *eui, tbl_interface_t *p_if)
{
    sal_memcpy (eui, p_if->mac_addr, 3);
    sal_memcpy (eui + 5, p_if->mac_addr + 3, 3);

    eui[3] = 0xFF;
    eui[4] = 0xFE;
    eui[0] ^= 2;

    #if 0
    if (( ETH_ALEN != p_if->hw_addr_len && (NSM_IF_PHY_TYPE_TUNNEL_IF != p_if->phy_type))||
        (4 != p_if->hw_addr_len && (NSM_IF_PHY_TYPE_TUNNEL_IF == p_if->phy_type))) /*for tunnel. added by zhangwf */
    {
        return -1;
    }
    #endif

    return 0;
}

int32
rt_if_l3if_loopback_generate_eui64(u_int8_t *eui)
{
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    if (!p_sys_glb)
    {
        return -1;
    }
    
    sal_memcpy (eui, p_sys_glb->route_mac, 3); 
    sal_memcpy (eui + 5, p_sys_glb->route_mac + 3, 3);

    eui[3] = 0xFF;
    eui[4] = 0xFE;
    eui[0] ^= 2;  

    return 0;
}

int32
ipv6_generate_eui64 (u_int8_t *eui, tbl_interface_t *p_if)
{
    switch (p_if->hw_type)
    {
        case GLB_IF_TYPE_PORT_IF:
        case GLB_IF_TYPE_VLAN_IF:
        case GLB_IF_TYPE_LINKAGG_IF:
#if 0
#ifdef HAVE_TUNNEL
        case IF_TYPE_IPIP:
        case IF_TYPE_GREIP:
        case IF_TYPE_IPV6IP:
        case IF_TYPE_6TO4:
        case IF_TYPE_ISATAP:
#endif
#endif
            return (addrconf_ifid_eui48(eui, p_if));

        default:
            return -1;
    }
}

int32
ipv6_generate_eui64_by_route_if (u_int8_t *eui, tbl_route_if_t *p_rt_if)
{
    tbl_interface_t *p_if = tbl_interface_get_interface_by_name(p_rt_if->key.name);
    if (NULL == p_if)
    {
        return -1;
    }
    switch (p_if->hw_type)
    {
        case GLB_IF_TYPE_PORT_IF:
        case GLB_IF_TYPE_VLAN_IF:
        case GLB_IF_TYPE_LINKAGG_IF:
#if 0
#ifdef HAVE_TUNNEL
        case IF_TYPE_IPIP:
        case IF_TYPE_GREIP:
        case IF_TYPE_IPV6IP:
        case IF_TYPE_6TO4:
        case IF_TYPE_ISATAP:
#endif
#endif
            return (addrconf_ifid_eui48(eui, p_if));

        default:
            return -1;
    }
}

void
_rt_if_generate_and_apply_eui_for_eui_and_linklocal(tbl_interface_t * p_db_if, ds_connected_v6_t *conn_cp)
{
    uint8 eui[8] = {0};
    
    /*auto address on loopback*/
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        rt_if_l3if_loopback_generate_eui64 (eui);
    }
    else
    {
        ipv6_generate_eui64 (eui, p_db_if);
    }

    if(conn_cp->key.address.prefixlen > (IPV6_MAX_BITLEN - EUI64_HOST_BITS))
    {
        int euibytes = (IPV6_MAX_BITLEN - conn_cp->key.address.prefixlen) / 8;
        int prebit = conn_cp->key.address.prefixlen % 8;

        sal_memcpy (&(conn_cp->key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - euibytes]),
                     &eui[EUI64_HOST_BYTES - euibytes], euibytes);

        if(prebit != 0)
        {
            uint8 maskbit[] = {0x00, 0x80, 0xc0, 0xe0,
                                  0xf0, 0xf8, 0xfc, 0xfe};

            conn_cp->key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - euibytes - 1] &= maskbit[prebit];
            conn_cp->key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - euibytes - 1] |= (eui[EUI64_HOST_BYTES - euibytes - 1] & (~maskbit[prebit]));
        }
    }
    else
    {
        sal_memcpy (&(conn_cp->key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - EUI64_HOST_BYTES]),
                     eui, EUI64_HOST_BYTES);
    }

    return;
}

int32
rt_ipv4_route_addr_check(prefix_t *p, char *err_str, uint32 len)
{
    addr_ipv4_t *p_addr = &p->u.prefix4;
    addr_ipv4_t addr;
    addr_ipv4_t mask;
    char        addr_str[INET_ADDRSTRLEN];
    uint32      ntoh_s_addr = 0;

    sal_memset(addr_str, 0, sizeof(addr_str));
    sal_inet_ntop(AF_INET,&p->u.prefix4, addr_str, INET_ADDRSTRLEN);
    
    addr_mask_len2ip_v4(p->prefixlen, &mask);

    /* network is 0. */
    addr.s_addr = p_addr->s_addr & mask.s_addr;
    ntoh_s_addr = sal_ntoh32(addr.s_addr);
    
    /* Check route IP address */
    if (IN_EXPERIMENTAL(ntoh_s_addr)
     || IN_LOOPBACK(ntoh_s_addr))
    {
        sal_snprintf(err_str, len, "Invalid IPv4 address");
        return PM_E_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

int32
rt_ipv6_addr_check(prefix_t *p, tbl_interface_t *p_db_if, char *err_str, uint32 len)
{
    char        addr_str[256] = {0};
    
    /*broadcast interface and tunnel not support*/
    #if 0
    /* 128 bit address prefix is disallowed only on broadcast interface */
    if (ifp && if_is_broadcast (ifp) && p->prefixlen == IPV6_MAX_PREFIXLEN)
        return 0;

    /* 128 bit address prefix is disallowed only on broadcast interface */
      if (ifp && if_is_tunnel(ifp) && p->prefixlen == IPV6_MAX_PREFIXLEN)
          return 0;
    #endif

    if (0 == p->prefixlen)
    {
        sal_snprintf(err_str, len, "Invalid prefix length 0");
        return PM_E_INVALID_PARAM;
    }
    
    /* Check IP address */
    if (IN6_IS_ADDR_MULTICAST(&p->u.prefix6) 
     || IN6_IS_ADDR_LOOPBACK(&p->u.prefix6)
     || IN6_IS_ADDR_UNSPECIFIED(&p->u.prefix6))
    {
        sal_snprintf(err_str, len, "Invalid IPv6 address");
        return PM_E_INVALID_PARAM;
    }

    // same to up
    /* 128 bit address prefix is disallowed only on broadcast interface */
    if (GLB_IF_TYPE_LOOPBACK_IF != p_db_if->hw_type && p->prefixlen == IPV6_MAX_BITLEN)
    {
        sal_snprintf(err_str, len, "Bad mask /%d for address %s", 
            p->prefixlen, cdb_addr_ipv6_val2str(addr_str, sizeof (addr_str), &p->u.prefix6));
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

int32
rt_if_ipv6_addr_check(prefix_t *p, tbl_interface_t *p_db_if, char *err_str, uint32 len)
{
    /* ifp can be NULL */
    if (NULL == p_db_if)
    {
        sal_snprintf(err_str, len, "Interface not exist");
        return PM_E_CFG_CONFLICT;
    }

    /*only 128 bit mask can be used on loopback interface*/
    if (INTERFACE_IS_LOOPBACK(p_db_if) && (IPV6_MAX_BITLEN != p->prefixlen))
    {
        sal_snprintf(err_str, len, "Only 128-bit masks are supported on Loopback interfaces");
        return PM_E_ROUTE_IF_INVALID_PREFIX_LEN_FOR_LB_IF;
    }

    return rt_ipv6_addr_check(p, p_db_if, err_str, len);
}

int32
rt_ipv4_addr_check(prefix_t *p, tbl_interface_t *p_db_if, char *err_str, uint32 len)
{
    addr_ipv4_t *p_addr = &p->u.prefix4;
    addr_ipv4_t addr;
    addr_ipv4_t mask;
    char        addr_str[INET_ADDRSTRLEN];
    uint32      ntoh_s_addr = 0;

    sal_memset(addr_str, 0, sizeof(addr_str));
    sal_inet_ntop(AF_INET,&p->u.prefix4, addr_str, INET_ADDRSTRLEN);
    
    addr_mask_len2ip_v4(p->prefixlen, &mask);

    /* network is 0. */
    addr.s_addr = p_addr->s_addr & mask.s_addr;
    ntoh_s_addr = sal_ntoh32(addr.s_addr);

    /* Check IP address */
    if (IN_EXPERIMENTAL(ntoh_s_addr) 
     || IN_CLASSD(ntoh_s_addr)
     || IN_LOOPBACK(ntoh_s_addr))
    {
        sal_snprintf(err_str, len, "Invalid IPv4 address");
        return PM_E_INVALID_PARAM;
    }

    /* check interface ipv4 addr */
    if (p_db_if)
    {
        if (IPV4_NET0(ntoh_s_addr))
        {
            sal_snprintf(err_str, len, "Invalid IPv4 address");
            return PM_E_INVALID_PARAM;
        }
        
        /* Added by tanker for bug1812, to check net id can not be 0 */
        if (addr.s_addr == 0)
        {
            sal_snprintf(err_str, len, "Bad mask /%d for address %s", p->prefixlen, addr_str);
            return PM_E_INVALID_PARAM;
        }
    
        /* For 32-bit prefixes no need to check for network or broadcast addresses */
        if (p->prefixlen != IPV4_MAX_BITLEN)
        {
            /* This is network. */
            if (addr.s_addr == p_addr->s_addr)
            {
                sal_snprintf(err_str, len, "Bad mask /%d for address %s", p->prefixlen, addr_str);
                return PM_E_INVALID_PARAM;
            }

            /* This is broadcast. */
            addr.s_addr = p_addr->s_addr | (~mask.s_addr);
            if (addr.s_addr == p_addr->s_addr)
            {
                sal_snprintf(err_str, len, "Bad mask /%d for address %s", p->prefixlen, addr_str);
                return PM_E_INVALID_PARAM;
            }
        }
        else if (GLB_IF_TYPE_LOOPBACK_IF != p_db_if->hw_type)
        {
            sal_snprintf(err_str, len, "Bad mask /%d for address %s", p->prefixlen, addr_str);
            return PM_E_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}

int32
rt_if_ipv4_addr_check(prefix_t *p, tbl_interface_t *p_db_if, char *err_str, uint32 len)
{
    /* ifp can be NULL */
    if (NULL == p_db_if)
    {
        sal_snprintf(err_str, len, "Interface not exist");
        return PM_E_CFG_CONFLICT;
    }

    /*Modified by xgu, only 32 bit mask can be used on loopback interface*/
    if (INTERFACE_IS_LOOPBACK(p_db_if) && (IPV4_MAX_BITLEN != p->prefixlen))
    {
        sal_snprintf(err_str, len, "Only 32-bit masks are supported on Loopback interfaces");
        return PM_E_INVALID_PARAM;
    }

    return rt_ipv4_addr_check(p, p_db_if, err_str, len);
}

ds_connected_v6_t*
rt_if_cmd_ipv6_addr_check_overlap_for_diff_interface(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, tbl_route_if_t **pp_overlap_rtif)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    tbl_route_if_t      *p_db_rtif_node = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *listnode_ds = NULL;
    ds_connected_v6_t      *p_db_conn = NULL;
    //uint32              secondary = FALSE;
    int32               rc = PM_E_NONE;

    *pp_overlap_rtif = NULL;
    //secondary = (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)) ? TRUE : FALSE;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif_node, listnode)
    {
        if (p_db_rtif == p_db_rtif_node)
        {
            /*huangxt for 19814, do not overlap on the same interface*/
            //continue;
        }

        if (CTCLIB_SLIST_ISEMPTY(p_db_rtif_node->ifc_ipv6.obj_list))
        {
            continue;
        }

        CTCLIB_SLIST_LOOP(p_db_rtif_node->ifc_ipv6.obj_list, p_db_conn, listnode_ds)
        {
            if (IN6_IS_ADDR_LINKLOCAL(&p_db_conn->key.address.u.prefix6))
            {
                continue;
            }

            if (p_conn->key.address.prefixlen < p_db_conn->key.address.prefixlen)
            {
                rc = prefix_compare_net(&p_conn->key.address, &p_db_conn->key.address);
            }
            else
            {
                rc = prefix_compare_net(&p_db_conn->key.address, &p_conn->key.address);
            }
        
            if (!rc)
            {
                continue;
            }

            *pp_overlap_rtif = p_db_rtif_node;
            if (0 != sal_memcmp(p_db_rtif_node->key.name, p_db_rtif->key.name, sizeof(p_db_rtif->key.name)))
            {
                return p_db_conn;
            }
        }
    }    

    return NULL;
}

/*check address is same net between interface*/
ds_connected_v6_t*
rt_if_cmd_ipv6_addr_check_overlap(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, tbl_route_if_t **pp_overlap_rtif)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    tbl_route_if_t      *p_db_rtif_node = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *listnode_ds = NULL;
    ds_connected_v6_t      *p_db_conn = NULL;
    //uint32              secondary = FALSE;
    int32               rc = PM_E_NONE;

    *pp_overlap_rtif = NULL;
    //secondary = (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)) ? TRUE : FALSE;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif_node, listnode)
    {
        if (p_db_rtif == p_db_rtif_node)
        {
            /*huangxt for 19814, do not overlap on the same interface*/
            //continue;
        }

        if (CTCLIB_SLIST_ISEMPTY(p_db_rtif_node->ifc_ipv6.obj_list))
        {
            continue;
        }

        CTCLIB_SLIST_LOOP(p_db_rtif_node->ifc_ipv6.obj_list, p_db_conn, listnode_ds)
        {
            if (IN6_IS_ADDR_LINKLOCAL(&p_db_conn->key.address.u.prefix6))
            {
                continue;
            }

            if (p_conn->key.address.prefixlen < p_db_conn->key.address.prefixlen)
            {
                rc = prefix_compare_net(&p_conn->key.address, &p_db_conn->key.address);
            }
            else
            {
                rc = prefix_compare_net(&p_db_conn->key.address, &p_conn->key.address);
            }
        
            if (!rc)
            {
                continue;
            }

            *pp_overlap_rtif = p_db_rtif_node;
            return p_db_conn;
        }
    }    

    return NULL;
}

/*check address is same net between interface*/
ds_connected_t*
rt_if_cmd_ipv4_addr_check_overlap(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, tbl_route_if_t **pp_overlap_rtif)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    tbl_route_if_t      *p_db_rtif_node = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *listnode_ds = NULL;
    ds_connected_t      *p_db_conn = NULL;
    uint32              secondary = FALSE;
    int32               rc = PM_E_NONE;

    *pp_overlap_rtif = NULL;
    secondary = (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)) ? TRUE : FALSE;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif_node, listnode)
    {
        if (p_db_rtif == p_db_rtif_node)
        {
            continue;
        }

        if (CTCLIB_SLIST_ISEMPTY(p_db_rtif_node->ifc_ipv4.obj_list))
        {
            continue;
        }

        CTCLIB_SLIST_LOOP(p_db_rtif_node->ifc_ipv4.obj_list, p_db_conn, listnode_ds)
        {
#ifdef TODO
            if (!secondary)
                if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
                    if (p_db_conn->ifp == p_db_rtif)
                        continue;
#else
            (void)secondary;
#endif
            if (p_conn->key.address.prefixlen < p_db_conn->key.address.prefixlen)
            {
                rc = prefix_compare_net(&p_conn->key.address, &p_db_conn->key.address);
            }
            else
            {
                rc = prefix_compare_net(&p_db_conn->key.address, &p_conn->key.address);
            }
        
            if (!rc)
            {
                continue;
            }

            *pp_overlap_rtif = p_db_rtif_node;
            return p_db_conn;
        }
    }    

    return NULL;
}

static int32
_rt_if_cmd_get_if(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_route_if_key_t key;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_iter_args_t args;
    char ifname_ext[IFNAME_EXT_SIZE];
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == ifname)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_route_if_dump_one, &args);
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.name, ifname, IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&key);
        if (!p_db_rtif)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Interface %s is not L3 interface", IFNAME_ETH2FULL(ifname, ifname_ext));
        }
        
        tbl_route_if_dump_one(p_db_rtif, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
rt_if_cmd_ipv4_addr_check_outbind(ds_connected_t *p_conn, tbl_ns_route_ip_t **pp_db_rtip)
{
    prefix_t prefix;
    tbl_ns_route_ip_master_t *p_master = tbl_ns_route_ip_get_master();
    tbl_ns_route_ip_t        *p_db_rtip = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    int32               rc = PM_E_NONE;

    sal_memset(&prefix, 0, sizeof(prefix));
    prefix.family = AF_INET;
    prefix.prefixlen = IPV4_MAX_PREFIXLEN;

    *pp_db_rtip = NULL;

    CTCLIB_SLIST_LOOP(p_master->ns_route_ip_list, p_db_rtip, listnode)
    {
        if (p_db_rtip->is_inband)
        {
            continue;
        }

        prefix.u.prefix4 = p_db_rtip->key.ip.u.prefix4;
        rc = prefix_compare_net(&p_conn->key.address, &prefix);
        if (!rc)
        {
            continue;
        }
        *pp_db_rtip = p_db_rtip;
        return PM_E_FAIL;
    }    

    return PM_E_NONE;
}

int32 
_rt_if_add_ipv6_auto_by_global_address(tbl_route_if_t *p_rt_if, ds_connected_v6_t *p_conn, cfg_cmd_para_t *para)
{
    ds_connected_v6_t      *p_conn_linklocal = NULL;
    ds_connected_v6_t      conn_cp;
    tbl_route_if_t      *p_overlap_rtif = NULL;
    ds_connected_v6_t *p_conn_tmp = NULL;
    char buf1[128] = {0};
    char                ifname_ext[IFNAME_EXT_SIZE];
    u_int8_t eui[8]={0};
    int32 rc = PM_E_NONE;

    sal_memset(&conn_cp, 0, sizeof(conn_cp));
    p_conn_linklocal = if_lookup_ifc_ipv6_linklocal(p_rt_if);
    if (NULL == p_conn_linklocal && !IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6))
    {
        conn_cp.key.address.prefixlen = p_conn->key.address.prefixlen;
        conn_cp.key.address.family = AF_INET6;
        sal_memcpy(&conn_cp.key.address.u.prefix6, &p_conn->key.address.u.prefix6, sizeof(p_conn->key.address.u.prefix6));
        p_conn_tmp = rt_if_cmd_ipv6_addr_check_overlap(p_rt_if, &conn_cp, &p_overlap_rtif);
        if (p_conn_tmp)
        {
            if (0 == sal_memcmp(p_overlap_rtif->key.name, p_rt_if->key.name, sizeof(p_rt_if->key.name)))
            {
                //delete ip address
                
            }
            else
            {
                CFG_CONFLICT_RET("%s overlaps with interface %s", 
                    cdb_addr_ipv6_val2str(buf1, sizeof(buf1), &p_conn->key.address.u.prefix6), IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext));
            }
        }

        MY_PRINTF("IPv6 address auto-generate link-local address for interface %s", IFNAME_ETH2FULL(p_rt_if->key.name, ifname_ext));

        conn_cp.key.address.prefixlen = LINK_LOCAL_PREFIX_LEN;
        conn_cp.key.address.u.prefix6.s6_addr32[0] = sal_hton32(0xFE800000);
        conn_cp.key.address.u.prefix6.s6_addr32[1] = sal_hton32(0x0);

        if (TRUE == rt_if_is_loopback(p_rt_if->key.name))
        {
            rt_if_l3if_loopback_generate_eui64 (eui);
        }
        else
        {
            ipv6_generate_eui64_by_route_if(eui, p_rt_if);
        }

        sal_memcpy (&(conn_cp.key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - EUI64_HOST_BYTES]),
            eui, EUI64_HOST_BYTES);

        /*nsm_ipv6_address_install (vr_id, ifname, &(cp.prefix), cp.prefixlen,
            NULL, NULL, RT_IF_ADDRV6_LINKLOCAL);*/
        //nsm_ipv6_address_install (vr_id, ifname, &(cp.prefix), cp.prefixlen,
        //    NULL, NULL, 0);
        rc = _rt_if_cmd_add_ipv6(p_rt_if, &conn_cp, para);
        if (rc != PM_E_NONE)
        {
            return rc;
        }
    }
    
    return PM_E_NONE;
}

void 
_rt_if_fill_0_for_EUI_64_address(ds_connected_v6_t *p_conn, uint32 prefixlen)
{
    uint32 i = 0;
    
    if (prefixlen < IPV6_MAX_BITLEN - EUI64_HOST_BITS)
    {
        if (prefixlen < 32 + 1)
        {
            p_conn->key.address.u.prefix6.s6_addr32[1] = 0;

            p_conn->key.address.u.prefix6.s6_addr32[0] = sal_ntoh32(p_conn->key.address.u.prefix6.s6_addr32[0]);
            for (i = 0; i < 32 - prefixlen; i++)
            {
                GLB_UNSET_FLAG(p_conn->key.address.u.prefix6.s6_addr32[0], 1 << i);
            }
            p_conn->key.address.u.prefix6.s6_addr32[0] = sal_hton32(p_conn->key.address.u.prefix6.s6_addr32[0]);
        }
        else
        {
            p_conn->key.address.u.prefix6.s6_addr32[1] = sal_ntoh32(p_conn->key.address.u.prefix6.s6_addr32[1]);
            for (i = 0; i < IPV6_MAX_BITLEN - EUI64_HOST_BITS - prefixlen; i++)
            {
                GLB_UNSET_FLAG(p_conn->key.address.u.prefix6.s6_addr32[1], 1<< i);
            }
            p_conn->key.address.u.prefix6.s6_addr32[1] = sal_hton32(p_conn->key.address.u.prefix6.s6_addr32[1]);
        }
    }
}

int32
_rt_if_cmd_del_linux_link_local(tbl_route_if_t *p_rt_if)
{
    //ds_connected_v6_t      *p_conn_linklocal = NULL;
    ds_connected_v6_t      conn_cp;
    char buf[CMD_BUF_64] = {0};
    char                ifname_ext[IFNAME_EXT_SIZE];
    u_int8_t eui[8]={0};
    tbl_interface_t     *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface_by_name(p_rt_if->key.name);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(&conn_cp, 0, sizeof(conn_cp));
    //p_conn_linklocal = if_lookup_ifc_ipv6_linklocal(p_rt_if);
    //if (NULL == p_conn_linklocal)
    {
        conn_cp.key.address.prefixlen = LINK_LOCAL_PREFIX_LEN_LINUX_DEFAULT;
        conn_cp.key.address.family = AF_INET6;
        conn_cp.key.address.u.prefix6.s6_addr32[0] = sal_hton32(0xFE800000);
        conn_cp.key.address.u.prefix6.s6_addr32[1] = sal_hton32(0x0);

        if (TRUE == rt_if_is_loopback(p_rt_if->key.name))
        {
            rt_if_l3if_loopback_generate_eui64 (eui);
        }
        else
        {
            ipv6_generate_eui64_by_route_if(eui, p_rt_if);
        }

        sal_memcpy (&(conn_cp.key.address.u.prefix6.s6_addr[IPV6_MAX_BYTELEN - EUI64_HOST_BYTES]),
            eui, EUI64_HOST_BYTES);

        MY_PRINTF("IPv6 address delete linux auto-generate link-local address %s for interface %s",
            prefix_val2str(buf, sizeof (buf), &conn_cp.key.address, TRUE), IFNAME_ETH2FULL(p_rt_if->key.name, ifname_ext));
            
        rt_cmd_del_if_addr_v6_by_syscmd(p_rt_if->key.name, &conn_cp.key.address.u.prefix6, conn_cp.key.address.prefixlen);

        //rt_if_indirect_ip_route_dec(p_db_if, 0, IN6_IS_ADDR_LINKLOCAL(&conn_cp.key.address.u.prefix6));
    }

    return PM_E_NONE;
}

/*TRUE: update, although num is full, but can update it
  FALSE: not update, num is full*/
int32
_rt_if_cmd_add_ipv6_update_address_when_address_num_is_full(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, char * ifname_ext)
{
    tbl_route_if_t      *p_overlap_rtif = NULL;
    ds_connected_v6_t *p_conn_tmp = NULL;
    
    /* Check overlapping address. */
    if (NULL != (p_conn_tmp = rt_if_cmd_ipv6_addr_check_overlap(p_db_rtif, p_conn, &p_overlap_rtif)))
    {
        if (0 == sal_memcmp(p_overlap_rtif->key.name, p_db_rtif->key.name, sizeof(p_db_rtif->key.name)))
        {
            /*the new address is same network with exist address of it's address*/
            /*update address*/
            return TRUE;
        }
        else
        {
            IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext);
            return PM_E_FAIL;
            
            //CFG_CONFLICT_RET("%s overlaps with interface %s", 
            //    prefix_val2str(addr_str, CMD_BUF_64, &conn_cp.key.address, TRUE), ifname_ext);
        }
    }

    return FALSE;
}

int32
_rt_if_cmd_add_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, cfg_cmd_para_t *para)
{
    tbl_interface_t     *p_db_if = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t      *p_tmp_conn = NULL;
    tbl_route_if_t      *p_overlap_rtif = NULL;
    ds_connected_v6_t      *p_db_conn = NULL;
    addr_ipv6_t         ip_addr_cmd;    
    char                ifname_ext[IFNAME_EXT_SIZE] = {0};
    char                addr_str[CMD_BUF_64];
//    uint32              mask_len    = 0;
    int32               rc = PM_E_NONE;
//    tbl_ns_route_ip_t   *p_db_rtip = NULL;
    mac_addr_t          vmac;
    ds_connected_v6_t *p_conn_tmp = NULL;
    
    sal_memset(&vmac, 0, sizeof(mac_addr_t));   

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    if (p_db_if->mtu < GLB_IPV6_MIN_MTU)
    {
        CFG_CONFLICT_RET("Set ipv6 address failed, please adjust the MTU to at least %u", GLB_IPV6_MIN_MTU);
    }

    /*if overlaps with mgmt-if address*/
    #if 0
    if (rt_if_cmd_ipv4_addr_check_outbind(p_conn, &p_db_rtip))
    {
        addr_str[0] = CMD_ZERO_CHAR;
        cdb_addr_ipv4_val2str(addr_str, CMD_BUF_64, &p_db_rtip->key.ip);
        CFG_CONFLICT_RET("Overlaps with %s in mgmt-if namespace", addr_str);
    }
    #endif

#if 0
    /*set link-local address is modify, so if address is link-local, it will not increase the address num*/
    /*global ipv6 address max is 8, then add 1 link-local address, so is 8+1*/
    if ((!IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6)) && 
        CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv6.obj_list) >= RT_IF_ADDRESS_V6_MAX + 1)
    {
        sal_memset(ifname_ext, 0, sizeof(ifname_ext));
        rc = _rt_if_cmd_add_ipv6_update_address_when_address_num_is_full(p_db_rtif, &conn_cp, ifname_ext);
        if (FALSE == rc)
        {
            CFG_CONFLICT_RET("Total number of global IPv6 address for one interface should not exceed %u", RT_IF_ADDRESS_V6_MAX);
        }
        else if(PM_E_FAIL == rc)
        {
            CFG_CONFLICT_RET("%s overlaps with interface %s", 
                prefix_val2str(addr_str, CMD_BUF_64, &conn_cp.key.address, TRUE), ifname_ext);
        }
        else
        {
            /*although num is full, but can update it*/
        }
    }
#endif

    /*default linux generate a link-local address, then first configure link-local address, linux will exist 2 link-local, 
        so delete the linux link-local address first*/
    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDRV6_LINKLOCAL) && (CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv6.obj_list) == 0))
    {
        //_rt_if_cmd_del_linux_link_local(p_db_rtif);
    }

    /* install the auto-generated link-local address if no link-local
        address exist when adding non link-local address */
    rc = _rt_if_add_ipv6_auto_by_global_address(p_db_rtif, p_conn, para);
    if (rc != PM_E_NONE)
    {
        return rc;
    }

    /* construct the ipv6 prefix */
    ds_connected_v6_t      conn_cp;
    sal_memset(&conn_cp, 0, sizeof (conn_cp));
    conn_cp.key.address.family = AF_INET6;
    conn_cp.key.address.prefixlen = 
        IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6) ? LINK_LOCAL_PREFIX_LEN : p_conn->key.address.prefixlen;
    sal_memcpy(&conn_cp.key.address.u.prefix6, &p_conn->key.address.u.prefix6, sizeof (p_conn->key.address.u.prefix6));

    if (p_conn->flags & (RT_IF_ADDRV6_GLOBAL_EUI64 | RT_IF_ADDRV6_AUTO_LINKLOCAL))
    {
        _rt_if_generate_and_apply_eui_for_eui_and_linklocal(p_db_if, &conn_cp);
    }

    /* fill 0 for EUI-64 address */
    if (GLB_FLAG_ISSET (p_conn->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
    {
        _rt_if_fill_0_for_EUI_64_address(&conn_cp, p_conn->key.address.prefixlen);
    }

    ds_connected_v6_t      *p_conn_linklocal = NULL;
    int32 need_delete = 0;
    int32 aflags = 0;
    uint32_t ipv4_addr;
    int32_t is_ipv4_addr = 1;

    /* link-local address already exists, check whether overwriting is needed  */
    p_conn_linklocal = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
    if (NULL != p_conn_linklocal)
    {
        /* if link-local address exists, again enable IPv6, we only tag it */
        if (GLB_FLAG_ISSET (p_conn->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
        {
            MY_PRINTF("IPv6 address tag enabled link-local "
                "address for interface %s", p_db_rtif->key.name);
            GLB_SET_FLAG(p_conn_linklocal->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
            ds_connected_v6_set_connected_v6_field(p_db_rtif, p_conn_linklocal, DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL);
            return PM_E_NONE;
        }
        else if (GLB_FLAG_ISSET (p_conn->flags, RT_IF_ADDRV6_LINKLOCAL))
        {
            /* overwrite the previous link-local address if different */
            if (0 != IPV6_ADDR_CMP(&(conn_cp.key.address.u.prefix6), &(p_conn_linklocal->key.address.u.prefix6)))
            {
                MY_PRINTF("IPv6 address overwrite the "
                    "previous link-local address for interface %s", p_db_rtif->key.name);
                need_delete = 1;
                
                /* modified by liwh for bug 49003, 2018-09-25
                     when change link-local address, should send ra */
                nd_api_if_up_pre_unset(p_db_rtif);
                /* liwh end */

                if (GLB_FLAG_ISSET (p_conn_linklocal->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
                {
                    aflags = RT_IF_ADDRV6_AUTO_LINKLOCAL;
                }
            }
            else
            {
                MY_PRINTF("IPv6 address tag link-local "
                    "address for interface %s, because the same address exists",
                    p_db_rtif->key.name);
               GLB_SET_FLAG (p_conn_linklocal->flags, RT_IF_ADDRV6_LINKLOCAL);
               ds_connected_v6_set_connected_v6_field(p_db_rtif, p_conn_linklocal, DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL);
                return PM_E_NONE;
            }
        }
    }

    /*if ipv6 address is valid*/
    if (IN6_IS_ADDR_V4COMPAT(&(conn_cp.key.address.u.prefix6)) 
        || IN6_IS_ADDR_V4MAPPED(&(conn_cp.key.address.u.prefix6)))
    {
        ipv4_addr = sal_ntoh32(conn_cp.key.address.u.prefix6.s6_addr32[3]);
    }
    else
    {
        is_ipv4_addr = 0;
    }

    if (is_ipv4_addr && (IN_EXPERIMENTAL (ipv4_addr)
        || IN_CLASSD (ipv4_addr)
        || IN_LOOPBACK(ipv4_addr)
        || IPV4_NET0 (ipv4_addr)))
    {
        //return NSM_API_SET_ERR_INVALID_IPV6_ADDRESS;
        CFG_INVALID_PARAM_RET ("Invalid address\n");
    }

    p_tmp_conn = connected_check_ipv6(p_db_rtif, &conn_cp.key.address);
    if ((NULL != p_tmp_conn) && (conn_cp.key.address.prefixlen != p_tmp_conn->key.address.prefixlen))
    {
        /* same IPv6 address, but different prefix length */
        MY_PRINTF("IPv6 address overwrite the "
            "previous address prefix length for interface %s", p_db_rtif->key.name);

        /* support nd modified by liwh for bug 47547, 2018-07-24 */
        neigh_ifc_down(p_db_rtif, p_tmp_conn, FALSE);
        /* liwh end */
    
        ipv6_address_del(p_db_rtif, p_tmp_conn, TRUE);
        p_tmp_conn = NULL;
    }
    
    /* 2. check duplicate config */
    p_db_conn = ds_connected_v6_get_connected_v6(p_db_rtif, &conn_cp);
    //p_db_conn = ds_connected_v6_get_connected_v6(p_db_rtif, p_conn);
    if (NULL != p_db_conn)
    {
        /* same configuration */
        MY_PRINTF("maybe no need return");
        return PM_E_NONE;
        // TODO:cc maybe not return
    }

    /*create the new address and install*/
    if (p_tmp_conn == NULL)
    {
        #if 0
        /* limit that the number of non link-local address must be less that
            MAX_INTERFACE_IPV6_ADDRESS_NUM */
        if ((!IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6)) &&
            (if_get_ifc_ipv6_num(p_db_rtif) >= MAX_INTERFACE_IPV6_ADDRESS_NUM))
        {
            //return (NSM_API_SET_IPV6_ADDRESS_OVERFLOW);
            CFG_CONFLICT_RET("Total number of global IPv6 address for one interface should not exceed %u", 
                MAX_INTERFACE_IPV6_ADDRESS_NUM);
        }
        #endif

        /*set link-local address is modify, so if address is link-local, it will not increase the address num*/
        /*global ipv6 address max is 8, then add 1 link-local address, so is 8+1*/
        if ((!IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6)) && 
            CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv6.obj_list) >= RT_IF_ADDRESS_V6_MAX + 1)
        {
            sal_memset(ifname_ext, 0, sizeof(ifname_ext));
            rc = _rt_if_cmd_add_ipv6_update_address_when_address_num_is_full(p_db_rtif, &conn_cp, ifname_ext);
            if (FALSE == rc)
            {
                CFG_CONFLICT_RET("Total number of global IPv6 address for one interface should not exceed %u", RT_IF_ADDRESS_V6_MAX);
            }
            else if(PM_E_FAIL == rc)
            {
                CFG_CONFLICT_RET("%s overlaps with interface %s", 
                    prefix_val2str(addr_str, CMD_BUF_64, &conn_cp.key.address, TRUE), ifname_ext);
            }
            else
            {
                /*although num is full, but can update it*/
            }
        }
        
        /* Check overlapping address. */
        sal_memset(ifname_ext, 0, sizeof(ifname_ext));
        /*fix bug 49425 by chenc*/
        if (NULL != rt_if_cmd_ipv6_addr_check_overlap_for_diff_interface(p_db_rtif, &conn_cp, &p_overlap_rtif))
        {
            IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext);
            CFG_CONFLICT_RET("%s overlaps with interface %s", 
                prefix_val2str(addr_str, CMD_BUF_64, &conn_cp.key.address, TRUE), ifname_ext);
        }

        p_overlap_rtif = NULL;
        if (NULL != (p_conn_tmp = rt_if_cmd_ipv6_addr_check_overlap(p_db_rtif, &conn_cp, &p_overlap_rtif)))
        {
            if (0 == sal_memcmp(p_overlap_rtif->key.name, p_db_rtif->key.name, sizeof(p_db_rtif->key.name)))
            {
                /*for 19814. do not overlap*/
                ipv6_address_del(p_db_rtif, p_conn_tmp, TRUE);

                /*for bug 48648 by chenc*/
                p_conn_tmp = NULL;
                while(NULL != (p_conn_tmp = rt_if_cmd_ipv6_addr_check_overlap(p_db_rtif, &conn_cp, &p_overlap_rtif)))
                {
                    if (0 == sal_memcmp(p_overlap_rtif->key.name, p_db_rtif->key.name, sizeof(p_db_rtif->key.name)))
                    {
                        /*for 19814. do not overlap*/
                        ipv6_address_del(p_db_rtif, p_conn_tmp, TRUE);
                    }
                    p_conn_tmp = NULL;
                }
            }
            else
            {
                /*fix bug 48726:  3333::1/64 's auto generated address conflict with f000::1/1*/
                if ((IN6_IS_ADDR_LINKLOCAL(&conn_cp.key.address.u.prefix6) && !IN6_IS_ADDR_LINKLOCAL(&p_conn_tmp->key.address.u.prefix6)))
                {
                }
                else
                {
                    IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext);
                    CFG_CONFLICT_RET("%s overlaps with interface %s", 
                        prefix_val2str(addr_str, CMD_BUF_64, &conn_cp.key.address, TRUE), ifname_ext);
                }
            }
        }

        /*if ipv6 addr is the same network, replace it, or add it*/

        /* set destination */
        sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
        //p_conn->destination = p_conn->key.address;
        //sal_memcpy(&ip_addr_cmd, &p_conn->key.address.u.prefix6, sizeof(ip_addr_cmd));
        //mask_len = p_conn->destination.prefixlen;

        conn_cp.destination = conn_cp.key.address;
        sal_memcpy(&ip_addr_cmd, &conn_cp.key.address.u.prefix6, sizeof(ip_addr_cmd));
        //mask_len = conn_cp.destination.prefixlen;
        #if 0
        addr_mask_len2ip_v4(p_conn->destination.prefixlen, &mask);
        p_conn->destination.u.prefix6.s_addr |= ~mask.s_addr;
        #else
        //assert(ip_apply_mask(&p_conn->destination) == PM_E_NONE);
        assert(ip_apply_mask(&conn_cp.destination) == PM_E_NONE);
        #endif

        //p_conn->ifindex = p_db_rtif->ifindex;
        //sal_memcpy(p_conn->ifname, p_db_rtif->key.name, IFNAME_SIZE);

        conn_cp.ifindex = p_db_rtif->ifindex;
        sal_memcpy(conn_cp.ifname, p_db_rtif->key.name, IFNAME_SIZE);

        GLB_SET_FLAG(p_conn->flags, RT_IF_ADDRV6_CONFIGURED);
        p_conn->flags |= aflags;

        conn_cp.flags = p_conn->flags;
        
        /* 3. call API */
        uint32 add_count_num = 2;
        if (INTERFACE_IS_LOOPBACK(p_db_if))
        {
            add_count_num = 1;
        }

        if (!IN6_IS_ADDR_LINKLOCAL(&conn_cp.key.address.u.prefix6) && rt_indirect_ipv6_route_count_max_check(add_count_num))
        {
            CFG_INVALID_PARAM_RET("IPv6 indirect route entry num is more than maximum %u", rt_fea_indirect_route_v6_count_get_max());
        }
        //rc = rt_if_add_ipv6(p_db_rtif, p_conn, vmac);
        rc = rt_if_add_ipv6(p_db_rtif, &conn_cp, vmac);
        usleep(100000);
        //rc = rt_cmd_add_if_addr_v6_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len);
    }
    
    /* support nd modified by liwh for bug 47547, 2018-07-03 */
    p_db_conn = ds_connected_v6_get_connected_v6(p_db_rtif, &conn_cp);
    if (p_db_conn)
    {
        nd_address_install(p_db_rtif, p_db_conn);
    }
    /* liwh end */


    if (need_delete)
    {
        /* when change link-local address, should unset the if-enable flag, by tanker for bug2629 */
        //struct nsm_if *nif = (struct nsm_if *)(ifp->info);
        //if (!nif)
        //    return NSM_FAILURE;

        /*bug 19814. if link local address is changed, DAD should be reproduced.*/
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_tmp_conn, listnode)
        {
            if (IN6_IS_ADDR_LINKLOCAL (&p_tmp_conn->key.address.u.prefix6)) /*user configured link local need not change*/
            {
                continue;
            }

            if (GLB_FLAG_ISSET (p_tmp_conn->flags, RT_IF_ADDRV6_REAL))
            {
                rt_if_connected_invalid_ipv6(p_db_rtif, p_tmp_conn, TRUE);
            }     

            GLB_UNSET_FLAG(p_tmp_conn->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
            GLB_UNSET_FLAG(p_tmp_conn->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);
            GLB_UNSET_FLAG(p_tmp_conn->flags, RT_IF_ADDR_DUPLICATE);

            p_tmp_conn->dad_ns_send = 0;
            p_tmp_conn->unsolicited_na = 0;
    
            CTC_TASK_STOP_TIMER(p_tmp_conn->t_timer);
         }

        ipv6_address_del (p_db_rtif, p_conn_linklocal, TRUE);

        if(p_tmp_conn && GLB_FLAG_ISSET(p_tmp_conn->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS))
        {
            GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_ENABLE);
        }
        /* Ended added by tanker */
    } 
    
    return rc;
}

ds_connected_v6_t *
ifc_get_ipv6 (ds_connected_v6_t * p_conn, addr_ipv6_t *addr, uint32 prefixlen,
	      tbl_route_if_t *ifp, uint32 add_ifindex_name)
{
    tbl_interface_t *p_db_if = NULL;
    
    p_conn->key.address.family = AF_INET6;
    sal_memcpy(&p_conn->key.address.u.prefix6, addr, sizeof (addr_ipv6_t));
    p_conn->key.address.prefixlen = prefixlen;

    if (add_ifindex_name)
    {
        p_conn->ifindex = ifp->ifindex;
        sal_memcpy(p_conn->ifname, ifp->key.name, IFNAME_SIZE);

        p_db_if = tbl_interface_get_interface_by_name(p_conn->ifname);
        if (NULL == p_db_if)
        {
            return NULL;
        }
    }
    
    ds_connected_v6_add_connected_v6(ifp, p_conn);
    rt_if_indirect_ip_route_inc(p_db_if, FALSE, IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6));
    
  return ds_connected_v6_get_connected_v6(ifp, p_conn);
}

int32
_rt_if_check_whether_add_auto_generated_linklocal_address
    (tbl_route_if_t *ifp, ds_connected_v6_t *ifc, ds_connected_v6_t * p_conn, uint32 delete_all, int32 disable)
{
    addr_ipv6_t in6_addr;
    tbl_interface_t * p_db_if = NULL;
    uint8 eui[8] = {0};
    char                ifname_ext[IFNAME_EXT_SIZE] = {0};
    ds_connected_v6_t conn_tmp;
    ds_connected_v6_t *ifc_tmp = NULL;

    p_db_if = tbl_interface_get_interface_by_name(ifp->key.name);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }
    
    if ((!delete_all && (GLB_FLAG_ISSET(ifc->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL) ||(if_get_ifc_ipv6_num(ifp) > 1)))
        || (delete_all && (GLB_FLAG_ISSET(ifc->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL)) && (!disable)))
    {
        MY_PRINTF("IPv6 address must exist a "
            "link-local address for interface %s, because ipv6 enable or "
            "other address configurated", IFNAME_ETH2FULL(ifp->key.name, ifname_ext));

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
            to the user configurated link-local address */
        if (0 != IPV6_ADDR_CMP(&in6_addr, &p_conn->key.address.u.prefix6))
        {
            /*not same*/
            MY_PRINTF("IPv6 address install auto-generated "
                "link-local address for interface %s", IFNAME_ETH2FULL(ifp->key.name, ifname_ext));

            /* If old one isn't auto generated link-local ,
                 install auto generated link-local address */
             // TODO: cc
            //if (GLB_FLAG_ISSET (ifc->conf, RT_IF_ADDRV6_REAL))
            //{
                //nsm_nd_api_if_change_link_local_addr(ifp, ifc);
            //}

            sal_memset(&conn_tmp, 0, sizeof(conn_tmp));
            GLB_UNSET_FLAG (conn_tmp.flags, RT_IF_ADDRV6_LINKLOCAL);
            if (GLB_FLAG_ISSET(ifc->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
            {
                GLB_SET_FLAG (conn_tmp.flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
            }

            /* This address is configured from nsm. */
            GLB_SET_FLAG (conn_tmp.flags, RT_IF_ADDRV6_CONFIGURED);

            /* Add to linked list. */
            ifc_tmp = ifc_get_ipv6 (&conn_tmp, &in6_addr, LINK_LOCAL_PREFIX_LEN, ifp, TRUE);
            if (NULL == ifc_tmp)
            {
                //return NSM_API_SET_ARP_GENERAL_ERR;
                return PM_E_FAIL;
            }
            
            nd_address_install(ifp, ifc_tmp);
        }
        else
        {
            /*same*/
            MY_PRINTF("IPv6 address return for "
                "interface %s, because the deleted link-local address is equal "
                "to the auto-generated address", IFNAME_ETH2FULL(ifp->key.name, ifname_ext));

            GLB_UNSET_FLAG (ifc->flags, RT_IF_ADDRV6_LINKLOCAL);
            ds_connected_v6_set_connected_v6_field(ifp, ifc, DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL);

            /* If old one is is auto generated link-local address,
                nothing need to be done */

            //return (NSM_API_SET_SUCCESS);
            /*return PM_E_FAIL to return all process*/
            return PM_E_FAIL;
        }
    }

    return PM_E_NONE;
}

int32
_rt_if_del_ipv6_check_for_no_auto_linklocal(tbl_route_if_t *p_db_rtif)
{
    ds_connected_v6_t      *p_conn_linklocal = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE] = {0};

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    
    if (NULL != (p_conn_linklocal = if_lookup_ifc_ipv6_linklocal(p_db_rtif)))
    {
        if (GLB_FLAG_ISSET(p_conn_linklocal->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
        {
            GLB_UNSET_FLAG(p_conn_linklocal->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
            ds_connected_v6_set_connected_v6_field(p_db_rtif, p_conn_linklocal, DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL);

            if (GLB_FLAG_ISSET(p_conn_linklocal->flags, RT_IF_ADDRV6_LINKLOCAL))
            {
                MY_PRINTF("IPv6 address untag enabled "
                    "link-local address for interface %s, because link-local "
                    "address has configurated", ifname_ext);
                /*return PM_E_FAIL to return all process*/
                return PM_E_FAIL;
            }
            else if (if_get_ifc_ipv6_num(p_db_rtif) > 1)
            {
                /* non link-local address configurated, link-local address
                    can't be deleted */
                MY_PRINTF("IPv6 address untag enabled "
                    "link-local address for interface %s, because no link-local "
                    "address has configurated", ifname_ext);
                /*return PM_E_FAIL to return all process*/
                return PM_E_FAIL;
            }
            else
            {
                MY_PRINTF("IPv6 address delete enabled "
                    "link-local address for interface %s, because no other "
                    "address exists", ifname_ext);
            }
        }
        else
        {
            MY_PRINTF("IPv6 address return for "
                "interface %s, because no ipv6 enable auto link-local", ifname_ext);
            /*return PM_E_FAIL to return all process*/
            return PM_E_FAIL;
        }
    }
    else
    {
        MY_PRINTF("IPv6 address return for "
            "interface %s, because no address configurated", ifname_ext);
        /*return PM_E_FAIL to return all process*/
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
_rt_if_cmd_del_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, cfg_cmd_para_t *para, uint32 delete_all, int32 disable)
{
    ds_connected_v6_t *p_db_conn = NULL;
    ds_connected_v6_t conn_cp;
    int32 rc = PM_E_NONE;
    tbl_interface_t * p_db_if = NULL;
    int32 is_link_local = FALSE;
    ds_connected_v6_t      *p_conn_linklocal = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE] = {0};
    char buf[128] = {0};

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
        
    /* 1. config check */
    // TODO: cc
    /* process command of "no ipv6 enable" */
    /*no auto linklocal*/
    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL))
    {
        if (PM_E_NONE != _rt_if_del_ipv6_check_for_no_auto_linklocal(p_db_rtif))
        {
            return PM_E_NONE;
        }
    }
    
    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(&conn_cp, 0, sizeof (conn_cp));
    conn_cp.key.address.family = AF_INET6;
    conn_cp.key.address.prefixlen = 
        IN6_IS_ADDR_LINKLOCAL(&p_conn->key.address.u.prefix6) ? LINK_LOCAL_PREFIX_LEN : p_conn->key.address.prefixlen;
    sal_memcpy(&conn_cp.key.address.u.prefix6, &p_conn->key.address.u.prefix6, sizeof (p_conn->key.address.u.prefix6));

    if (p_conn->flags & (RT_IF_ADDRV6_GLOBAL_EUI64 | RT_IF_ADDRV6_AUTO_LINKLOCAL))
    {
        _rt_if_generate_and_apply_eui_for_eui_and_linklocal(p_db_if, &conn_cp);
    }

    /* fill 0 for EUI-64 address */
    if (GLB_FLAG_ISSET (p_conn->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
    {
        _rt_if_fill_0_for_EUI_64_address(&conn_cp, p_conn->key.address.prefixlen);
    }

    /*conn_cp is more good than p_conn*/
    /* Check current interface address. */
    //p_db_conn = connected_check_ipv6(p_db_rtif, &conn_cp);
    p_db_conn = ds_connected_v6_get_connected_v6(p_db_rtif, &conn_cp);
    if (NULL == p_db_conn || p_db_conn->key.address.prefixlen != conn_cp.key.address.prefixlen)
    {
        if (para)
        {
            CFG_CONFLICT_RET("Can't find IPv6 address %s", prefix_val2str(buf, sizeof(buf), &conn_cp.key.address, TRUE));
        }
    }

    /* support nd modified by liwh for bug 47547, 2018-07-18 */
    neigh_ifc_del(p_db_rtif, p_db_conn);
    /* liwh end */

    is_link_local = IN6_IS_ADDR_LINKLOCAL(&(p_db_conn->key.address.u.prefix6));
    
    /*if normal address and link-local all configured, 
        then delete link-local, it should auto generated link-local address*/
    /* check whether add auto generated link-local address */
    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDRV6_LINKLOCAL))
    //    || (delete_all && GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL)))
    {
        /*add*/
        rc = _rt_if_check_whether_add_auto_generated_linklocal_address(p_db_rtif, p_db_conn, p_conn, delete_all, disable);
        if (PM_E_NONE != rc)
        {
            return rc;
        }
    }

    /* 3. call API */
    //ipv6_address_del(p_db_rtif, p_conn);
    rc = ipv6_address_del(p_db_rtif, p_db_conn, FALSE);
    if (PM_E_NONE != rc)
    {
        MY_PRINTF("ipv6_address_del rc %u", rc);
    }

    /*if the address need to be delete 
        is not link-local address and address num is 0 or 1 
        and the last is link-local
        and auto link-local is not configured
        and link-local is not configured
        should delete the last address
        */
    if ((!is_link_local) && (if_get_ifc_ipv6_num(p_db_rtif) < 2) &&
        (NULL != (p_conn_linklocal = if_lookup_ifc_ipv6_linklocal(p_db_rtif))) &&
        (!GLB_FLAG_ISSET(p_conn_linklocal->flags, RT_IF_ADDRV6_AUTO_LINKLOCAL)) &&
        (!GLB_FLAG_ISSET(p_conn_linklocal->flags, RT_IF_ADDRV6_LINKLOCAL)))
    {
        MY_PRINTF("IPv6 address delete auto-generate "
            "link-locak address for interface %s, because the last non link-local "
            "address is deleted", IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext));
        
        rc = ipv6_address_del (p_db_rtif, p_conn_linklocal, TRUE);
    }

    // TODO: cc nd
    /* delete all link-local neighbor when no address, by tanker for bug2582 */
    #if 0
    if(if_get_ifc_ipv6_num(ifp) == 0)
    {
        nif = (struct nsm_if *)(ifp->info);
        if((NULL != nif) && (NULL != nif->nd_if))
        {
            struct neigh_match_if_param match;
            u_int32_t num;

            nm = nif->nd_if->top->nm;

            match.ifp = ifp;
            match.flags = NDF_PERMANENT;
            neigh_walk(nm, neigh_match_if_link_local, (void *)&match,
            neigh_delete, nm, (int*)&num);
        }
    }
    #endif

#if 0
    /*if link local address is changed, DAD should be reproduced.*/
    if(IN6_IS_ADDR_LINKLOCAL(p_conn->key.address.u.prefix6))
    {
        for (ifc = ifp->ifc_ipv6; ifc != NULL; ifc = ifc->next)
        {
            if (IN6_IS_ADDR_LINKLOCAL (&ifc->address->u.prefix6))
            {
                continue;
            }
            
            nsm_fea_if_ipv6_address_delete(ifp, ifc);
            nsm_connected_down_ipv6(ifp, ifc);
            GLB_UNSET_FLAG(ifc->conf, RT_IF_ADDRV6_REAL); /*RT_IF_ADDRV6_REAL|NSM_IFC_ACTIVE*/
        }
    }
#endif
    return rc;
}

static int32
_rt_if_cmd_del_all_ipv6(tbl_route_if_t *p_db_rtif, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    MY_PRINTF("IPv6 address delete all IPv6 address of interface %s", p_db_rtif->key.name);
    
    /* 1. config check */
    /* 2. check duplicate config */

    /* 3. call API */
    rc = rt_if_del_all_ipv6(p_db_rtif, FALSE, TRUE);

    return rc;
}

int32
rt_add_arp_for_ptp_udp_dest_ip(tbl_route_if_t *p_db_rtif)
{
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_arp_t       *ae_p = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_ptp_port = tbl_ptp_port_get_ptp_port_by_name(p_db_rtif->key.name);
    if (NULL == p_db_ptp_port)
    {
        return PM_E_NONE;
    }

    if (p_db_ptp_port->port_addr.dest_ip_addr.s_addr != 0 && p_db_ptp_port->port_addr.protocol != PTP_NET_PROTO_IEEE_802_3)
    {
        /*add arp*/
        ae_p = arp_db_entry_lookup(&(p_db_ptp_port->port_addr.dest_ip_addr));
        if (ae_p != NULL)
        {
            ae_p->route_ref++; /*modified by ychen for bug 7351 for invalid read*/
        }
        else
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(p_db_ptp_port->ifindex);
            rc = arp_db_entry_create(p_db_if, &(p_db_ptp_port->port_addr.dest_ip_addr), NULL, ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
            if (rc < 0)
            {
                return PM_E_NONE;
            }
            
            ae_p->route_ref++;
            ae_p->status = AS_INCOMPLETE;
            arp_sm(ae_p, AE_RETRY_EXP);
        }
    }

    return PM_E_NONE;
}

static int32
_rt_if_cmd_add_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, cfg_cmd_para_t *para)
{
    tbl_interface_t     *p_db_if = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    tbl_route_if_t      *p_overlap_rtif = NULL;
    ds_connected_t      *p_db_conn = NULL;
    addr_ipv4_t         ip_addr_cmd;    
    addr_ipv4_t         mask;
    char                ifname_ext[IFNAME_EXT_SIZE];
    char                addr_str[CMD_BUF_64];
    uint32              mask_len    = 0;
    int32               rc = PM_E_NONE;
    tbl_ns_route_ip_t   *p_db_rtip = NULL;
    mac_addr_t          vmac;

    sal_memset(&vmac, 0, sizeof(mac_addr_t));   

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /*if overlaps with mgmt-if address*/
    if (rt_if_cmd_ipv4_addr_check_outbind(p_conn, &p_db_rtip))
    {
        addr_str[0] = CMD_ZERO_CHAR;
        cdb_addr_ipv4_val2str(addr_str, CMD_BUF_64, &p_db_rtip->key.ip.u.prefix4);
        CFG_CONFLICT_RET("Overlaps with %s in mgmt-if namespace", addr_str);
    }

    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY))
    {
        if (CTCLIB_SLIST_ISEMPTY(p_db_rtif->ifc_ipv4.obj_list))
        {
            CFG_CONFLICT_RET("Configure primary first");
        }

        if (CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv4.obj_list) >= RT_IF_ADDRESS_MAX)
        {
            CFG_CONFLICT_RET("Exceeding maximum address limit");
        }
    }

    /* 2. check duplicate config */
    p_db_conn = ds_connected_get_connected(p_db_rtif, p_conn);
    if (NULL != p_db_conn)
    {
        if (GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY) && !GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            CFG_CONFLICT_RET("Primary can't be same as secondary");
        }
        else if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY) && GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            CFG_CONFLICT_RET("Secondary can't be same as primary");
        }

        /* same configuration */
        return PM_E_NONE;
    }

    /* Check overlapping address. */
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
    if (rt_if_cmd_ipv4_addr_check_overlap(p_db_rtif, p_conn, &p_overlap_rtif))
    {
        IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext);
        CFG_CONFLICT_RET("%s overlaps with interface %s", 
            prefix_val2str(addr_str, CMD_BUF_64, &p_conn->key.address, TRUE), ifname_ext);
    }

    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY))
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_tmp_conn, listnode)
        {
            if (p_tmp_conn->key.address.u.prefix4.s_addr == p_conn->key.address.u.prefix4.s_addr)
            {
                if (GLB_FLAG_ISSET(p_tmp_conn->flags, RT_IF_ADDR_SECONDARY))
                {
                    CFG_CONFLICT_RET("Secondary can't be same as other secondary");
                }
                else
                {
                    CFG_CONFLICT_RET("Secondary can't be same as primary");
                }
            }
        }
    }
    else
    {
        /* clear the related arp entry before */
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_tmp_conn, listnode)
        {
            if (!GLB_FLAG_ISSET(p_tmp_conn->flags, RT_IF_ADDR_SECONDARY))
            {
                //tbl_iter_args_t     args;
                //args.argv[0] = p_db_rtif->key.name;
                //tbl_iproute_node_iterate(rt_fea_del_route_by_ifname, &args);

                //rc = arp_api_connected_del(p_db_if, p_db_rtif, p_tmp_conn);
                sal_memset(&ip_addr_cmd, 0, sizeof(ip_addr_cmd));
                mask_len = p_tmp_conn->destination.prefixlen;
                sal_memcpy(&ip_addr_cmd, &p_tmp_conn->key.address.u.prefix4, sizeof(ip_addr_cmd));
                rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);

                rt_if_indirect_ip_route_dec(p_db_if, TRUE, FALSE);
            }
        }
    }

    uint32 add_count_num = 2;
    if (INTERFACE_IS_LOOPBACK(p_db_if))
    {
        add_count_num = 1;
    }
        
    if (rt_indirect_ipv4_route_count_max_check(add_count_num))
    {
        CFG_CONFLICT_RET("IPv4 indirect route entry num is more than maximum %u", rt_fea_indirect_route_count_get_max());
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
    usleep(100000);
    rc = rt_cmd_add_if_addr_by_syscmd(p_db_rtif->key.name, &ip_addr_cmd, mask_len, TRUE);

    rt_if_indirect_ip_route_inc(p_db_if, TRUE, FALSE);

    rt_add_arp_for_ptp_udp_dest_ip(p_db_rtif);
    return rc;
}

static int32
_rt_if_cmd_del_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, cfg_cmd_para_t *para)
{
    ds_connected_t *p_db_conn = NULL;
    int32 rc = PM_E_NONE;
    tbl_interface_t     *p_db_if = NULL;

    /* 1. config check */
    if (!GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY))
    {
        if (CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv4.obj_list) > 1)
        {
            CFG_CONFLICT_RET("Must delete secondary before deleting primary");
        }
    }

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
  
    /* 2. check duplicate config */
    p_db_conn = ds_connected_get_connected(p_db_rtif, p_conn);
    if (NULL == p_db_conn)
    {
        CFG_CONFLICT_RET("Can't find address");
    }
    
    if ((GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_SECONDARY)) 
            && (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY)))
    {
        CFG_CONFLICT_RET("Secondary can't be same as primary");
    }

    ds_connected_t conn_backup;
    sal_memset(&conn_backup, 0, sizeof(conn_backup));
    sal_memcpy(&conn_backup, p_db_conn, sizeof(conn_backup));
    
    /* 3. call API */
    rc = rt_if_del_ipv4(p_db_rtif, p_conn);
    rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &conn_backup.key.address.u.prefix4, conn_backup.destination.prefixlen, TRUE);

    /*added by chenc for bug 52266, 2019-06-04*/
    /*update secondary address*/
    if (GLB_FLAG_ISSET(conn_backup.flags, RT_IF_ADDR_SECONDARY))
    {
        ds_connected_t *p_db_conn_tmp = NULL;
        ctclib_slistnode_t  *listnode   = NULL;
        ctclib_slistnode_t  *next       = NULL;
        CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, p_db_conn_tmp, listnode, next)
        {
            if (!GLB_FLAG_ISSET(p_db_conn_tmp->flags, RT_IF_ADDR_SECONDARY))
            {
                continue;
            }

            /*update*/
            if (0 == sal_memcmp(p_db_conn_tmp, &conn_backup, sizeof(ds_connected_t)))
            {
                rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &p_db_conn_tmp->key.address.u.prefix4, p_db_conn_tmp->destination.prefixlen, TRUE);
            }
            else
            {
                rc = rt_cmd_del_if_addr_by_syscmd(p_db_rtif->key.name, &p_db_conn_tmp->key.address.u.prefix4, p_db_conn_tmp->destination.prefixlen, FALSE);
            }
        }

        CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, p_db_conn_tmp, listnode, next)
        {
            if (!GLB_FLAG_ISSET(p_db_conn_tmp->flags, RT_IF_ADDR_SECONDARY))
            {
                continue;
            }

            /*not add itself*/
            if (0 == sal_memcmp(p_db_conn_tmp, &conn_backup, sizeof(ds_connected_t)))
            {
                continue;
            }

            rc = rt_cmd_add_if_addr_by_syscmd(p_db_rtif->key.name, &p_db_conn_tmp->key.address.u.prefix4, p_db_conn_tmp->destination.prefixlen, FALSE);
        }
    }
    /*chenc end*/

    rt_if_indirect_ip_route_dec(p_db_if, TRUE, FALSE);
    
    /*modified by chenc for remove patch function, 2019-05-11*/
    #ifndef USE_FPM_IPUC
    rc = rt_if_rt_entry_by_ifaddr(p_db_rtif, &conn_backup.key.address.u.prefix4);
    #endif
    return rc;
}

static int32
_rt_if_cmd_del_all_ipv4(tbl_route_if_t *p_db_rtif, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    /* 2. check duplicate config */

    /* 3. call API */
    rc = rt_if_del_all_ipv4(p_db_rtif);

    return rc;
}

static int32
_rt_if_cmd_del_all_ipv4_secondary(tbl_route_if_t *p_db_rtif, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    /* 2. check duplicate config */

    /* 3. call API */
    rc = rt_if_del_all_ipv4_secondary(p_db_rtif);

    return rc;
}

int32
rt_if_cmd_check_l3_cmd(const char *ifname, cfg_cmd_para_t *para, tbl_interface_t **pp_if, tbl_route_if_t **pp_rt_if)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_interface_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, sizeof(key.name));
    *pp_rt_if = NULL;
    
    /* check exist */
    p_db_if = tbl_interface_get_interface(&key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s is not exist", IFNAME_ETH2FULL(key.name, ifname_ext));
    }
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&key);
    if (!p_db_rtif)
    {
        CFG_CONFLICT_RET("Interface %s is not L3 interface", IFNAME_ETH2FULL(key.name, ifname_ext));
    }

    *pp_if = p_db_if;
    *pp_rt_if = p_db_rtif;

    return PM_E_NONE;
}

static int32
_rt_if_cmd_set_arp_ageing_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_rtif->arp_timeout == timeout)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_cmd_set_arp_ageing_timeout(p_db_rtif, timeout));
    return rc;
}

static int32
_rt_if_cmd_set_ucast_rpf_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is tunnel interface", p_db_if->key.name);
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

#ifdef TODO
    if (!GLB_FLAG_ISSET(nm->sys_ctrl, GLB_CTRL_URPF_EN))
    {
        return NSM_L3IF_ERROR_URPF_GLOBAL_DISABLED;  
    }
#endif

    /* 2. check duplicate config */
    if (p_db_rtif->unicast_rpf_en == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_ucast_rpf_en(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_ucast_rpf_en_v6(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is tunnel interface", p_db_if->key.name);
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

#ifdef TODO
    if (!GLB_FLAG_ISSET(nm->sys_ctrl, GLB_CTRL_URPF_EN))
    {
        return NSM_L3IF_ERROR_URPF_GLOBAL_DISABLED;  
    }
#endif

    /* 2. check duplicate config */
    if (p_db_rtif->unicast_rpf_en_v6 == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_ucast_rpf_en_v6(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_rip_recv_version(tbl_route_if_t *p_db_rtif, uint8 rip_recv_version, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_recv_version == rip_recv_version)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_recv_version(p_db_rtif, rip_recv_version));
    return rc;
}

static int32
_rt_if_cmd_set_rip_send_version(tbl_route_if_t *p_db_rtif, uint8 rip_send_version, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_send_version == rip_send_version)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_send_version(p_db_rtif, rip_send_version));
    return rc;
}

static int32
_rt_if_cmd_set_rip_v2_broadcast(tbl_route_if_t *p_db_rtif, uint8 rip_v2_broadcast, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_v2_broadcast== rip_v2_broadcast)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_v2_broadcast(p_db_rtif, rip_v2_broadcast));
    return rc;
}

static int32
_rt_if_cmd_set_rip_split_horizon(tbl_route_if_t *p_db_rtif, uint8 rip_split_horizon, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_split_horizon== rip_split_horizon)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_split_horizon(p_db_rtif, rip_split_horizon));
    return rc;
}

static int32
_rt_if_cmd_set_ripng_split_horizon(tbl_route_if_t *p_db_rtif, uint8 ripng_split_horizon, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ripng_split_horizon== ripng_split_horizon)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ripng_split_horizon(p_db_rtif, ripng_split_horizon));
    return rc;
}


static int32
_rt_if_cmd_set_rip_poisoned_reverse(tbl_route_if_t *p_db_rtif, uint8 rip_poisoned_reverse, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_poisoned_reverse== rip_poisoned_reverse)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_poisoned_reverse(p_db_rtif, rip_poisoned_reverse));
    return rc;
}

static int32
_rt_if_cmd_set_ripng_poisoned_reverse(tbl_route_if_t *p_db_rtif, uint8 ripng_poisoned_reverse, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ripng_poisoned_reverse== ripng_poisoned_reverse)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ripng_poisoned_reverse(p_db_rtif, ripng_poisoned_reverse));
    return rc;
}

static int32
_rt_if_cmd_set_rip_auth_mode(tbl_route_if_t *p_db_rtif, uint8 rip_auth_mode, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->rip_auth_mode== rip_auth_mode)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_auth_mode(p_db_rtif, rip_auth_mode));
    return rc;
}

static int32
_rt_if_cmd_set_rip_auth_string(tbl_route_if_t *p_db_rtif, char* rip_auth_string, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_db_rtif->rip_auth_string,rip_auth_string,sizeof(p_db_rtif->rip_auth_string)))
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_rip_auth_string(p_db_rtif,rip_auth_string));
    return rc;
}

static int32
_rt_if_cmd_set_ip_unreachable_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ip_unreachable_en == enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ip_unreachable_en(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_ospf_mtu_ignore_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_mtu_ignore == enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_mtu_ignore_en(p_db_rtif, enable));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    if (TRUE == enable)
    {
        sal_sprintf(cmd_str, "ip ospf mtu-ignore");
    }
    else
    {
        sal_sprintf(cmd_str, "no ip ospf mtu-ignore");
    }
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    return rc;
}

static int32
_rt_if_cmd_set_ospf6_mtu_ignore_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_mtu_ignore == enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_mtu_ignore_en(p_db_rtif, enable));
    
    return rc;
}


static int32
_rt_if_cmd_set_ospf_hello_interval(tbl_route_if_t *p_db_rtif, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 43976, 2017-08-07 */
    bool      update_dead_interval = FALSE;
    uint32    dead_timer_interval = 0;
    /* liwh end */
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_timer_hello == interval)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 43976, 2017-08-07 */
    if ((p_db_rtif->ospf_timer_dead 
        && (p_db_rtif->ospf_timer_dead == (p_db_rtif->ospf_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO)))
        || (0 == p_db_rtif->ospf_timer_dead))
    {
        update_dead_interval = TRUE;
    }
    /* liwh end */

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_hello_interval(p_db_rtif, interval));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "ip ospf hello-interval %u", interval);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    /* modified by liwh for bug 43976, 2017-08-07 */
    if (update_dead_interval)
    {
        /* 4. check dead interval duplicate config */
        dead_timer_interval = GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO * interval;
        if (p_db_rtif->ospf_timer_dead == dead_timer_interval)
        {
            return PM_E_NONE;
        }
    
        /* 5. call API */
        PM_E_RETURN(rt_if_set_ospf_dead_interval(p_db_rtif, dead_timer_interval));
    }
    /* liwh end */

    return rc;
}

static int32
_rt_if_cmd_set_ospf_dead_interval(tbl_route_if_t *p_db_rtif, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 43976, 2017-08-07 */
    uint32    dead_timer_interval = 0;
    /* liwh end */
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* modified by liwh for bug 43976, 2017-08-07 */
    if (0 == interval)
    {
        dead_timer_interval = p_db_rtif->ospf_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO;
    }
    else
    {
        dead_timer_interval = interval;    
    }
    /* liwh end */

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_timer_dead == dead_timer_interval)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_dead_interval(p_db_rtif, dead_timer_interval));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "ip ospf dead-interval %u", dead_timer_interval);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    return rc;
}

static int32
_rt_if_cmd_set_ospf6_dead_interval(tbl_route_if_t *p_db_rtif, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 43976, 2017-08-07 */
    uint32    dead_timer_interval = 0;
    /* liwh end */
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* modified by liwh for bug 43976, 2017-08-07 */
    if (0 == interval)
    {
        dead_timer_interval = p_db_rtif->ospf6_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO;
    }
    else
    {
        dead_timer_interval = interval;    
    }
    /* liwh end */

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_timer_dead == dead_timer_interval)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_dead_interval(p_db_rtif, dead_timer_interval));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "ipv6 ospf6 dead-interval %u", dead_timer_interval);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    return rc;
}

static int32
_rt_if_cmd_set_ospf6_hello_interval(tbl_route_if_t *p_db_rtif, uint32 interval, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 43976, 2017-08-07 */
    bool      update_dead_interval = FALSE;
    uint32    dead_timer_interval = 0;
    /* liwh end */
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_timer_hello == interval)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 43976, 2017-08-07 */
    if ((p_db_rtif->ospf6_timer_dead 
        && (p_db_rtif->ospf6_timer_dead == (p_db_rtif->ospf6_timer_hello * GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO)))
        || (0 == p_db_rtif->ospf6_timer_dead))
    {
        update_dead_interval = TRUE;
    }
    /* liwh end */

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_hello_interval(p_db_rtif, interval));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "ipv6 ospf6 hello-interval %u", interval);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    /* modified by liwh for bug 43976, 2017-08-07 */
    if (update_dead_interval)
    {
        /* 4. check dead interval duplicate config */
        dead_timer_interval = GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO * interval;
        if (p_db_rtif->ospf6_timer_dead == dead_timer_interval)
        {
            return PM_E_NONE;
        }
    
        /* 5. call API */
        PM_E_RETURN(rt_if_set_ospf6_dead_interval(p_db_rtif, dead_timer_interval));
    }
    /* liwh end */

    return rc;
}


/* modified by liwh for bug 51257, 2019-03-18 */
void
rt_ospf_cost_timer(void *p_data)
{
    tbl_route_if_t           *p_db_rtif = NULL;

    if (!p_data)
    {
        return;    
    }
     
    p_db_rtif = (tbl_route_if_t*)p_data;
    p_db_rtif->t_ospf_cost_timer = NULL;

    return;
}
/* liwh end */

static int32
_rt_if_cmd_set_ospf_cost(tbl_route_if_t *p_db_rtif, uint32 cost_ospf, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char cmd_str[128] = {0};
    int32   remained = 0; /* modified by liwh for bug 51257, 2019-03-18 */

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_cost == cost_ospf)
    {
        return PM_E_NONE;
    }

    /* modified by liwh for bug 51257, 2019-03-18 */
    if (!p_db_rtif->t_ospf_cost_timer)
    {
        p_db_rtif->t_ospf_cost_timer = ctc_task_add_timer(TASK_PRI_NORMAL, rt_ospf_cost_timer, p_db_rtif, GLB_OSPF_COST_TIME);
    }
    else
    {
        remained =ctc_timer_get_remain_sec(p_db_rtif->t_ospf_cost_timer);
        CFG_INVALID_PARAM_RET("Please wait %d seconds when change ospf cost again", remained);
    }
    /* liwh end */
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_cost(p_db_rtif, cost_ospf));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    if (0 != cost_ospf)
    {
        sal_sprintf(cmd_str, "ip ospf cost %u", cost_ospf);
    }
    else
    {
        sal_sprintf(cmd_str, "no ip ospf cost");
    }
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    return rc;
}

static int32
_rt_if_cmd_set_ospf6_cost(tbl_route_if_t *p_db_rtif, uint32 cost_ospf, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    int32   remained = 0; /* modified by liwh for bug 51257, 2019-03-18 */

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_cost == cost_ospf)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_cost(p_db_rtif, cost_ospf));
    
    return rc;
}


static int32
_rt_if_cmd_set_ospf_priority(tbl_route_if_t *p_db_rtif, uint32 priority, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_priority== priority)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_priority(p_db_rtif, priority));
    return rc;
}

static int32
_rt_if_cmd_set_ospf6_priority(tbl_route_if_t *p_db_rtif, uint32 priority, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_priority== priority)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_priority(p_db_rtif, priority));
    return rc;
}


static int32
_rt_if_cmd_set_ospf_auth_type(tbl_route_if_t *p_db_rtif, uint32 auth_type, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_auth_type == auth_type)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_auth_type(p_db_rtif, auth_type));

    if (OSPF_RIF_AUTH_TYPE_MD5 == auth_type)
    {
        sal_sprintf(cmd_str, "ip ospf authentication message-digest");
    }
    else
    {
        sal_sprintf(cmd_str, "no ip ospf authentication");
    }
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);
    
    return rc;
}

static int32
_rt_if_cmd_set_ospf_network_type(tbl_route_if_t *p_db_rtif, uint32 network_type, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char cmd_str[128];

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf_network_type == network_type)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_network_type(p_db_rtif, network_type));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    if (OSPF_NETWORK_TYPE_POINT_TO_POINT == network_type)
    {
        sal_sprintf(cmd_str, "ip ospf network point-to-point");
    }
    else
    {
        sal_sprintf(cmd_str, "no ip ospf network");
    }
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);

    return rc;
}

static int32
_rt_if_cmd_set_ospf6_network_type(tbl_route_if_t *p_db_rtif, uint32 network_type, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->ospf6_network_type == network_type)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf6_network_type(p_db_rtif, network_type));
    
    return rc;
}


static int32
_rt_if_cmd_set_ospf_auth_key(tbl_route_if_t *p_db_rtif, char *auth_key, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_db_rtif->ospf_auth_key, auth_key, sizeof(p_db_rtif->ospf_auth_key)))
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_ospf_auth_key(p_db_rtif, auth_key));
    return rc;
}

#define OSPF_IF_MAX_AUTH_KET_CNT 10

static int32
_rt_if_cmd_add_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    ds_ospf_auth_t *p_db_auth_md5 = NULL;
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    p_db_auth_md5 = ds_ospf_auth_get_ospf_auth(p_db_rtif, p_auth);
    if (NULL != p_db_auth_md5)
    {
        CFG_FUNC_ERR_RET("Key %u already exists", p_auth->key.id);
        return PM_E_NONE;
    }

    /* 2. check of max ospf auth key num */
    if (OSPF_IF_MAX_AUTH_KET_CNT <= p_db_rtif->ospf_md5_key.obj_list->count)
    {
        CFG_FUNC_ERR_RET("The max num ospf auth key of interface %s is %u", p_db_rtif->key.name, OSPF_IF_MAX_AUTH_KET_CNT);
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_add_ospf_auth_md5_key(p_db_rtif, p_auth));

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "ip ospf message-digest-key %u md5 %s", p_auth->key.id, p_auth->ospf_auth_md5_key);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);

    return rc;
}

static int32
_rt_if_cmd_del_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    ds_ospf_auth_t* p_db_auth = NULL;
    char cmd_str[128] = {0};

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    p_db_auth = ds_ospf_auth_get_ospf_auth(p_db_rtif, p_auth);
    if (NULL == p_db_auth)
    {
        /* system modified by liwh & lil for bug 47162, 2018-05-16 */
        CFG_FUNC_ERR_RET("Key %d does not exist", p_auth->key.id);
        /* liwh end */
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_del_ospf_auth_md5_key(p_db_rtif, p_auth));
    
    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "no ip ospf message-digest-key %u", p_auth->key.id);
    rt_vty_if_cmd(p_db_rtif->key.name, cmd_str);

    return rc;
}

static int32
_rt_if_cmd_set_local_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (INTERFACE_IS_LOOPBACK(p_db_if))
    {
        CFG_INVALID_PARAM_RET("ARP proxy should not be enabled on loopback interface.");
    }

    /* 2. check duplicate config */
    if (p_db_rtif->local_arp_proxy_en == enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_local_arp_proxy(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (INTERFACE_IS_LOOPBACK(p_db_if))
    {
        CFG_INVALID_PARAM_RET("ARP proxy should not be enabled on loopback interface.");
    }

    /* 2. check duplicate config */
    if (p_db_rtif->arp_proxy_en == enable)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_arp_proxy(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_dhcp_relay_option_trust(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_rtif->dhcp_relay_option_trust == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_dhcp_relay_option_trust(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_dhcp_server_group(tbl_route_if_t *p_db_rtif, uint8 index, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (index > GLB_MAX_DHCSRVGRP_NUM) {
        CFG_CONFLICT_RET("Invalid dhcp server group index %d", index);;
    }

    if (p_db_rtif->dhcp_client_enable && (0 != index))
    {
        CFG_CONFLICT_RET("Conflict with dhcp client feature enabled on this interface");
    }
    
    /* 2. check duplicate config */
    if (p_db_rtif->dhcp_server_group == index)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_dhcp_server_group(p_db_rtif, index));
    return rc;
}

static int32
_rt_if_cmd_set_ip_redirects_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    if (p_db_rtif->local_arp_proxy_en && enable)
    {
        CFG_INVALID_PARAM_RET("Can not enable IP ICMP redirect because of local arp proxy enabled");
    }

    /* 2. check duplicate config */
    if (p_db_rtif->ip_redirects_en == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_ip_redirects_en(p_db_rtif, enable));
    return rc;
}

/*arp rate limit start*/
static int32
_rt_if_cmd_set_arpratelimit_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (NULL == p_db_rtif || NULL == para)
    {
        CFG_FUNC_ERR_RET("Input parameter is NULL!");
    }
    
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    
    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not exist", ifname_ext);
    }
    /*member port cannot cfg*/
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    rc = arpratelimit_is_arpratelimit_l3_port(p_db_if);
    if (FALSE == rc)
    {
        CFG_CONFLICT_RET("Interface %s is not routed port or LAG or vlanif", ifname_ext);
    }

    /* 2. check duplicate config */
    if (p_db_rtif->arp_rate_limit_en == enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(arpratelimit_set_enable(p_db_rtif, enable));
    return rc;
}

static int32
_rt_if_cmd_set_arpratelimit_pkt_max(tbl_route_if_t *p_db_rtif, uint32 pktmax, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (NULL == p_db_rtif || NULL == para)
    {
        CFG_FUNC_ERR_RET("Input parameter is NULL!");
    }
    
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not exist", ifname_ext);
    }
    /*member port cannot cfg*/
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    rc = arpratelimit_is_arpratelimit_l3_port(p_db_if);
    if (FALSE == rc)
    {
        CFG_CONFLICT_RET("Interface %s is not routed port or LAG or vlanif", ifname_ext);
    }

    /* 2. check duplicate config */
    if (p_db_rtif->arp_rate_limit_pkt_max == pktmax)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(arpratelimit_set_pkt_max(p_db_rtif, pktmax));
    
    /*after set ok, clear curr pkt num and flag*/
    p_db_rtif->arp_rate_limit_pkt_curr = 0;
    p_db_rtif->arp_rate_limit_port_abnormal_flag = ARP_RATE_LIMIT_ABNORMAL_FALSE;
    
    return rc;
}

static int32
_rt_if_cmd_set_arpratelimit_exceed_action(tbl_route_if_t *p_db_rtif, uint8 action, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (NULL == p_db_rtif || NULL == para)
    {
        CFG_FUNC_ERR_RET("Input parameter is NULL!");
    }
    
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not exist", ifname_ext);
    }
    
    /*member port cannot cfg*/
    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    rc = arpratelimit_is_arpratelimit_l3_port(p_db_if);
    if (FALSE == rc)
    {
        CFG_CONFLICT_RET("Interface %s is not routed port or LAG or vlanif", ifname_ext);
    }

    /* 2. check duplicate config */
    if (p_db_rtif->arp_rate_limit_violation == action)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(arpratelimit_set_exceed_action(p_db_rtif, action));

    /*after set ok, clear curr pkt num and flag*/
    p_db_rtif->arp_rate_limit_pkt_curr = 0;
    p_db_rtif->arp_rate_limit_port_abnormal_flag = ARP_RATE_LIMIT_ABNORMAL_FALSE;

    return rc;
}
/*arp rate limit end*/

static int32
_rt_if_cmd_set_arp_retry_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_rtif->arp_retry_timeout == timeout)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_arp_retry_timeout(p_db_rtif, timeout));
    return rc;
}

static int32
_rt_if_cmd_set_dhcp_client_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* physical, agg, vlan support only*/
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
    && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
    && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type
    )
    {
        CFG_CONFLICT_RET("Function not supported on this interface");
    }

    if (p_db_rtif->dhcp_server_group && enable)
    {
        CFG_CONFLICT_RET("Conflict with dhcp relay/server feature enabled on this interface");
    }

    /*1. delete ip */
    _rt_if_cmd_del_all_ipv4(p_db_rtif, para);

    /* 3. call API */
    PM_E_RETURN(rt_if_set_dhcp_client_en(p_db_rtif, enable));

/* modified by liwh for bug 51205, 2019-03-28 */
#ifdef OFPRODUCT
    if (enable)
    {
        /* modified by chenc for bug 51931, 2019-06-19 */
        #ifndef USW
        CFG_PROMPT_RET("Warning: dhcp packets can not match flow table when dhcp client/relay is enabled.");
        #endif
    }
#endif
/* liwh end */

    return rc;
}

/* modified by liwh for bug 49365, 2018-11-27 */
int32 
rt_if_cmd_dhcpv6_set_auto_linklocal_address(cfg_cmd_para_t * para, tbl_route_if_t *p_db_rtif, bool is_add)
{
    int32 rc = PM_E_NONE;
    ds_connected_v6_t ds_conn_v6;
    prefix_t *p_prefix = &ds_conn_v6.key.address;

    sal_memset(&ds_conn_v6, 0, sizeof(ds_conn_v6));

    GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
        
    p_prefix->family = AF_INET6;
    p_prefix->prefixlen = LINK_LOCAL_PREFIX_LEN;
    p_prefix->u.prefix6.s6_addr[0] = 0xfe;
    p_prefix->u.prefix6.s6_addr[1] = 0x80;
    
    if (is_add)
    {
        rc = _rt_if_cmd_add_ipv6(p_db_rtif, &ds_conn_v6, para);
    }
    else
    {
        rc = _rt_if_cmd_del_ipv6(p_db_rtif, &ds_conn_v6, para, FALSE, FALSE);
    }

    return rc;
}

static int32
_rt_if_cmd_set_dhcpv6_client_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    if (FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* physical, agg, vlan support only*/
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
        && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
        && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type
        )
    {
        CFG_CONFLICT_RET("Function not supported on this interface");
    }

     if (enable == p_db_rtif->dhcpv6_client_enable)
    {
        if (enable)
        {
            CFG_CONFLICT_RET("DHCPv6 client is already enabled");
        }
        else
        {
            CFG_CONFLICT_RET("DHCPv6 client is already disabled");
        }
    } 

    if (p_db_rtif->ifc_ipv6.obj_list && p_db_rtif->ifc_ipv6.obj_list->count && enable)
    {
        CFG_CONFLICT_RET("DHCPv6 client conflicts with ipv6 address");    
    }

    if (enable)
    {
        rt_if_cmd_dhcpv6_set_auto_linklocal_address(para, p_db_rtif, TRUE);
        usleep(100000);
    }
    
    /* 3. call API */
    PM_E_RETURN(rt_if_set_dhcpv6_client_en(p_db_rtif, enable));

    if (!enable)
    {
        rt_if_cmd_dhcpv6_set_auto_linklocal_address(para, p_db_rtif, FALSE);    
    }
    
    return rc;
}
/*liwh end */

/* begin added by taocy for NAT, 2019/06/04*/
static int32
_rt_if_cmd_set_nat_en(tbl_route_if_t *p_db_rtif, uint8 enable, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* physical, agg, vlan support only*/
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
        && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
        && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type
        )
    {
        CFG_CONFLICT_RET("Function not supported on this interface.");
    }

    //printf("\r\n _rt_if_cmd_set_nat_en, old:%u, new:%u", p_db_rtif->nat_enable, enable);
    if (p_db_rtif->nat_enable == enable)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(nat_api_set_if_nat(p_db_rtif, p_db_rtif->nat_enable, enable));

    PM_E_RETURN(rt_if_set_nat_en(p_db_rtif, enable));

    return PM_E_NONE;
}
/* end added by taocy for NAT, 2019/06/04*/

/*add by zhw for mc*/
static int32
_rt_if_cmd_set_pim_mode(tbl_route_if_t *p_db_rtif, uint8 pim_mode, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->pim_mode== pim_mode)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_pim_mode(p_db_rtif, pim_mode));
    return rc;
}

static int32
_rt_if_cmd_set_multicast(tbl_route_if_t *p_db_rtif, uint8 mode, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->multicast== mode)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(rt_if_set_multicast(p_db_rtif, mode));
    return rc;
}
/*end add zhw*/

int
rt_if_check_simple_key(char *key, cfg_cmd_para_t *para)
{
    char              *p = key;
    int32             rc = PM_E_NONE;

    if (sal_strlen(p) > OSPF_AUTH_SIMPLE_MAX || sal_strlen(p) < 1)
    {
        CFG_FUNC_ERR_RET("Invalid ospf key! its length range should be [1, 8]");
    }

    /* check the chars */
    while ('\0' != *p)
    {
        if (*p == '%')
        {
            CFG_FUNC_ERR_RET("The key can include 0-9A-Za-z~`!@#$^&*()_+-={}[]|:;<>,.");
        }
       
        p++;
    }

    return rc;
}

int
rt_if_check_rip_string(char *key, cfg_cmd_para_t *para)
{
    char              *p = key;
    int32             rc = PM_E_NONE;

    if (sal_strlen(p) > STRING_SIZE || sal_strlen(p) < 1)
    {
        CFG_FUNC_ERR_RET("Invalid rip string! its length range should be [1, 15]");
    }

    /* check the chars */
    while ('\0' != *p)
    {
        if (*p == '%')
        {
            CFG_FUNC_ERR_RET("The key can include 0-9A-Za-z~`!@#$^&*()_+-={}[]|:;<>,.");
        }
       
        p++;
    }

    return rc;
}

int
rt_if_check_md5_key(char *key, cfg_cmd_para_t *para)
{
    char              *p = key;
    int32             rc = PM_E_NONE;

    if (sal_strlen(p) > OSPF_AUTH_MD5_MAX || sal_strlen(p) < 1)
    {
        CFG_FUNC_ERR_RET("Invalid ospf key! its length range should be [1, 16]");
    }

    /* check the chars */
    while ('\0' != *p)
    {
        if (*p == '%')
        {
            CFG_FUNC_ERR_RET("The key can include 0-9A-Za-z~`!@#$^&*()_+-={}[]|:;<>,.");
        }

        p++;
    }

    return rc;
}

int32
_rt_if_cmd_dhclient_process_add_ip(int32 result, tbl_route_if_t *p_db_rtif)
{
    int32 rc = PM_E_NONE;

    if (PM_E_NONE == result || FALSE == p_db_rtif->dhcp_client_enable)
    {
        return rc;
    }

    p_db_rtif->dhcp_client_flags = DHC_NTF_UNBOUND_DONE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_CLIENT_FLAGS));

    return rc;
}

int32
_rt_if_cmd_dhclient_process_del_ip(tbl_route_if_t *p_db_rtif)
{
    int32 rc = PM_E_NONE;

    if (p_db_rtif->dhcp_client_enable && p_db_rtif->dhcp_client_flags != 0)
    {
        p_db_rtif->dhcp_client_flags = 0;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_CLIENT_FLAGS));
    }

    return rc;
}

int 
prefix_is_subnet_router_anycast(prefix_t *p)
{
    prefix_t tmp;

    sal_memset(&tmp, 0, sizeof(tmp));

    if ((IN6_IS_ADDR_MULTICAST(&(p->u.prefix6))) || 
        (IPV6_MAX_BITLEN == p->prefixlen))
    {
        return (0);
    }

    ip_prefix_copy ((prefix_t *)(&tmp), (prefix_t *)p);
    ip_apply_mask_ipv6(&tmp);
    if (IPV6_ADDR_SAME(&(tmp.u.prefix6), &(p->u.prefix6)))
    {
        return (1);
    }
    else
    {
        return (0);
    }        
}

/*       0          1          2             3                     4   5               6*/
/*/${iface}/ifc_ipv6/add/${address_type}/${ip_prefix}/${eui-64}*/
/*/${iface}/ifc_ipv6/add/${address_type}/${ip}*/
/*/${iface}/ifc_ipv6/add/${address_type}*/

/*/${iface}/ifc_ipv6/del/${address_type}/${ip_prefix}/${eui-64}*/
/*/${iface}/ifc_ipv6/del/${address_type}/${ip}*/
/*/${iface}/ifc_ipv6/del/${address_type}*/

/*       0           1         2       34         5*/
/*/${iface}/ifc_ipv4/add/${ip}/${secondary}*/
/*/${iface}/ifc_ipv4/del/${ip}*/
/*/${iface}/ifc_ipv4/del/${ip}/${secondary}*/
int32
rt_if_cmd_process_route_if_ipv6_address(char * * argv, int32 argc, cfg_cmd_para_t * para, tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif)
{
    int32 rc = PM_E_NONE;
    uint32 is_add = FALSE;
    ds_connected_v6_t ds_conn_v6;
    prefix_t *p_prefix = &ds_conn_v6.key.address;
    uint32 is_del_all = FALSE;
    char        buf1[CMD_BUF_256] = {0};

    _cmd_print_para(argv, argc);
    
    sal_memset(&ds_conn_v6, 0, sizeof(ds_conn_v6));
    if (0 == sal_strcmp(argv[2], "add"))
    {
        is_add = TRUE;
    }
    else if (0 == sal_strcmp(argv[2], "del"))
    {
        is_add = FALSE;
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid action %s", argv[2]);
    }

    /*global unicast address*/
    if (0 == sal_strcmp(argv[3], "ip_prefix"))
    {
        GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_GLOBAL);

        /*eui-64*/
        if ((argc == 7) && 0 == sal_strcmp(argv[6], "eui-64"))
        {
            GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_GLOBAL_EUI64);
        }
        
        CFG_PARSER_PREFIX(argv[4], argv[5], p_prefix, para->p_rs);

        rc = rt_if_ipv6_addr_check(p_prefix, p_db_if, para->p_rs->desc, MAX_CMD_RET_LEN);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            /*add or (delete and not linklocal)*/
            if (is_add || (!is_add && !IN6_IS_ADDR_LINKLOCAL(&(p_prefix->u.prefix6))))
            {
                if (p_prefix->prefixlen == IPV6_MAX_PREFIXLEN || p_prefix->prefixlen == 0)
                {
                    CFG_INVALID_PARAM_RET ("Bad mask /%d for address %s", 
                        p_prefix->prefixlen, cdb_addr_ipv6_val2str(buf1, sizeof (buf1), &p_prefix->u.prefix6));
                }
                else if(rc == PM_E_ROUTE_IF_INVALID_PREFIX_LEN_FOR_LB_IF)
                {
                    return rc;
                }
                else
                {
                    CFG_INVALID_PARAM_RET ("Invalid address");
                }
            }
            return rc;
        }
        
        if (IN6_IS_ADDR_LINKLOCAL(&p_prefix->u.prefix6))
        {
            CFG_INVALID_PARAM_RET("Prefix-length should not be configured for link-local address");
        }

        if (argc >= 7 && 0 == sal_strcmp(argv[6], "eui-64"))
        {
            if (p_prefix->prefixlen > IPV6_MAX_PREFIXLEN - EUI64_HOST_BITS)
            {
                CFG_INVALID_PARAM_RET("Prefix-length should not be greater than %u", 
                    IPV6_MAX_PREFIXLEN - EUI64_HOST_BITS);
            }

            GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_GLOBAL_EUI64);
        }
        else if (is_add && prefix_is_subnet_router_anycast(p_prefix))
        {
            CFG_INVALID_PARAM_RET("Subnet router anycast address can't be configured");
        }
    }

    /*link-local address*/
    if (0 == sal_strcmp(argv[3], "ip"))
    {
        GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_LINKLOCAL);

        CFG_PARSER_IPV6_ADDR(argv[4], &p_prefix->u.prefix6, para->p_rs);
        p_prefix->family = AF_INET6;
        p_prefix->prefixlen = LINK_LOCAL_PREFIX_LEN;
        if (!IN6_IS_ADDR_LINKLOCAL (&p_prefix->u.prefix6))
        {
            CFG_INVALID_PARAM_RET ("Invalid link-local address");
        }
        else if (prefix_is_subnet_router_anycast(p_prefix))
        {
            CFG_INVALID_PARAM_RET ("Subnet router anycast address can't be configured");
        }
    }

    /*auto link-local*/
    if (0 == sal_strcmp(argv[3], "auto"))
    {
        GLB_SET_FLAG(ds_conn_v6.flags, RT_IF_ADDRV6_AUTO_LINKLOCAL);
        
        p_prefix->family = AF_INET6;
        p_prefix->prefixlen = LINK_LOCAL_PREFIX_LEN;
        p_prefix->u.prefix6.s6_addr[0] = 0xfe;
        p_prefix->u.prefix6.s6_addr[1] = 0x80;
        //p_prefix->u.prefix6.s6_addr32[0] = sal_hton32(0xfe800000);;
    }

    if (0 == sal_strcmp(argv[3], "all"))
    {
        is_del_all = TRUE;
    }

    /*fix bug 48663*/
    if (!is_del_all && IN6_IS_ADDR_6TO4(&ds_conn_v6.key.address.u.prefix6))
    {
        CFG_INVALID_PARAM_RET ("6to4 address is invalid");
    }

    /*fix bug 49378*/
    if (!is_del_all && IN6_IS_ADDR_V4MAPPED(&ds_conn_v6.key.address.u.prefix6))
    {
        CFG_INVALID_PARAM_RET ("IPv4-mapped IPv6 address is invalid");
    }
    
    if (is_add)
    {
        rc = _rt_if_cmd_add_ipv6(p_db_rtif, &ds_conn_v6, para);
        /* is dhclient enable ? we need to check rc and notify dhcp-server we fail to use this ip */
       // _rt_if_cmd_dhclient_process_add_ip(rc, p_db_rtif);
    }
    else
    {
        if (is_del_all)
        {
            rc = _rt_if_cmd_del_all_ipv6(p_db_rtif, para);
        }
        else
        {
            rc = _rt_if_cmd_del_ipv6(p_db_rtif, &ds_conn_v6, para, FALSE, FALSE);
        }
        /* is dhlient enable ? */
        //_rt_if_cmd_dhclient_process_del_ip(p_db_rtif);
    }

    return rc;
}

static int32
_rt_if_cmd_set_igmp(tbl_route_if_t *p_db_rtif, uint8 mode, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. config check */
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);

    /* 2. check duplicate config */
    if (p_db_rtif->igmp == mode)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
	p_db_rtif->igmp = mode;
	tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_IGMP);
    return rc;
}


int32
rt_if_cmd_process_route_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    tbl_route_if_key_t rtif_key;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    char *ifname = NULL;
    field_parse_t field;
    uint32 value = 0;
    uint32 ospf_mtu_enable = 0;
	uint32 ospf6_mtu_enable = 0;
    char auth_key[256];
    int32 rc = PM_E_NONE;
    tbl_interface_t* pst_interface = NULL;
    tbl_interface_key_t if_key;
    ds_connected_t ds_conn;
    prefix_t *p_prefix = &ds_conn.key.address;
    tbl_interface_t *p_db_if = NULL;

    uint32 is_add = FALSE;
    uint32 is_del_all = FALSE;
    uint32 del_all_sec = FALSE;
    ds_ospf_auth_t ospf_auth_md5;
	char rip_auth_string[STRING_SIZE];
    uint32 key_id = 0;
    char auth_md5_key[OSPF_AUTH_MD5_MAX + 1];

    uint32 ra_interval_max = 0;
    uint32 ra_interval_min = 0;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_ROUTE_IF);
    sal_memset(&if_key, 0, sizeof(if_key));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        RT_IF_CMD_CHECK_RT_IF(argv[0], para, &p_db_if, &p_db_rtif);
        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_strcpy(rtif_key.name, argv[0]);
        p_db_rtif = tbl_route_if_get_route_if(&rtif_key);

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_ROUTE_IF_FLD_ND_PREFIX:
            nd_prefix_add_entry(p_db_rtif, &argv[2], (argc - 2), para);
            break;
        }
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        RT_IF_CMD_CHECK_RT_IF(argv[0], para, &p_db_if, &p_db_rtif);
        sal_memset(&rtif_key, 0, sizeof(rtif_key));
        sal_strcpy(rtif_key.name, argv[0]);
        p_db_rtif = tbl_route_if_get_route_if(&rtif_key);

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_ROUTE_IF_FLD_ND_PREFIX:
            nd_prefix_del_entry(p_db_rtif, &argv[2], (argc - 2), para);
            break;
        }
        break;
        
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }

        /*add by chenc for bug 50407, 2018-12-20*/
        /*for switchport config switchport cmd start*/
        if (field_id != TBL_ROUTE_IF_FLD_MODE_SET_BEFORE)
        {
            RT_IF_CMD_CHECK_RT_IF(argv[0], para, &p_db_if, &p_db_rtif);
            sal_memset(&rtif_key, 0, sizeof(rtif_key));
            sal_strcpy(rtif_key.name, argv[0]);
            p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
        }
        
        if (p_db_rtif == NULL)
        {
            return PM_E_NONE;
        }
        /*for switchport config switchport cmd end*/

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
		case TBL_ROUTE_IF_FLD_RIP_RECV_VERSION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_rip_recv_version(p_db_rtif, value, para);
			
			break;
			

		case TBL_ROUTE_IF_FLD_RIP_SEND_VERSION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_rip_send_version(p_db_rtif, value, para);

			break;

		case TBL_ROUTE_IF_FLD_RIP_V2_BROADCAST:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "add"))
				{
					value = 1;
					_rt_if_cmd_set_rip_v2_broadcast(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[2], "del"))
				{
					value = 0;
					_rt_if_cmd_set_rip_v2_broadcast(p_db_rtif, value, para);
				}
			}
			break;

		case TBL_ROUTE_IF_FLD_RIP_SPLIT_HORIZON:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "add"))
				{
					_rt_if_cmd_set_rip_split_horizon(p_db_rtif, RIP_SPLIT_ON, para);
					_rt_if_cmd_set_rip_poisoned_reverse(p_db_rtif, RIP_POISONED_OFF, para);
				}
				else if (0 == sal_strcmp(argv[2], "del"))
				{
					value = 1;
					_rt_if_cmd_set_rip_split_horizon(p_db_rtif, RIP_SPLIT_OFF, para);
					_rt_if_cmd_set_rip_poisoned_reverse(p_db_rtif, RIP_POISONED_OFF, para);
				}
			}
			break;

		case TBL_ROUTE_IF_FLD_RIP_POISONED_REVERSE:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "add"))
				{
					value = 1;
					_rt_if_cmd_set_rip_split_horizon(p_db_rtif, RIP_SPLIT_ON, para);
					_rt_if_cmd_set_rip_poisoned_reverse(p_db_rtif, RIP_POISONED_ON, para);
				}
				else if (0 == sal_strcmp(argv[2], "del"))
				{
					value = 0;
					_rt_if_cmd_set_rip_split_horizon(p_db_rtif, RIP_SPLIT_ON, para);
					_rt_if_cmd_set_rip_poisoned_reverse(p_db_rtif, RIP_POISONED_OFF, para);
				}
			}
			break;
		/*add by zhw for ripng*/
		case TBL_ROUTE_IF_FLD_RIPNG_SPLIT_HORIZON:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "add"))
				{
					_rt_if_cmd_set_ripng_split_horizon(p_db_rtif, RIPNG_SPLIT_ON, para);
					_rt_if_cmd_set_ripng_poisoned_reverse(p_db_rtif, RIPNG_POISONED_OFF, para);
				}
				else if (0 == sal_strcmp(argv[2], "del"))
				{
					value = 1;
					_rt_if_cmd_set_ripng_split_horizon(p_db_rtif, RIPNG_SPLIT_OFF, para);
					_rt_if_cmd_set_ripng_poisoned_reverse(p_db_rtif, RIPNG_POISONED_OFF, para);
				}
			}
			break;

		case TBL_ROUTE_IF_FLD_RIPNG_POISONED_REVERSE:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "add"))
				{
					value = 1;
					_rt_if_cmd_set_ripng_split_horizon(p_db_rtif, RIPNG_SPLIT_ON, para);
					_rt_if_cmd_set_ripng_poisoned_reverse(p_db_rtif, RIPNG_POISONED_ON, para);
				}
				else if (0 == sal_strcmp(argv[2], "del"))
				{
					value = 0;
					_rt_if_cmd_set_ripng_split_horizon(p_db_rtif, RIPNG_SPLIT_ON, para);
					_rt_if_cmd_set_ripng_poisoned_reverse(p_db_rtif, RIPNG_POISONED_OFF, para);
				}
			}
			break;
		/*end add*/
		case TBL_ROUTE_IF_FLD_RIP_AUTH_MODE:
			if (4 == argc && 0 == sal_strcmp(argv[2], "add"))
			{
				if (0 == sal_strcmp(argv[3], "text"))
				{
					value = RIP_AUTH_TEXT;
					_rt_if_cmd_set_rip_auth_mode(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[3], "md5"))
				{
					value = RIP_AUTH_MD5;
					_rt_if_cmd_set_rip_auth_mode(p_db_rtif, value, para);
				}
			}
			else if (3 == argc && 0 == sal_strcmp(argv[2], "del"))
			{
				value = RIP_AUTH_NONE;
				_rt_if_cmd_set_rip_auth_mode(p_db_rtif, value, para);
			}
			break;

		case TBL_ROUTE_IF_FLD_RIP_AUTH_STRING:
			sal_memset(rip_auth_string, 0, sizeof(rip_auth_string));
			if (4 == argc && 0 == sal_strcmp(argv[2], "add"))
			{
				if (rt_if_check_rip_string(argv[3], para))
                {
                    return rc;
                }
				sal_memcpy(rip_auth_string,argv[3],STRING_SIZE);
				_rt_if_cmd_set_rip_auth_string(p_db_rtif, rip_auth_string, para);
				
			}
			else if (3 == argc && 0 == sal_strcmp(argv[2], "del"))
			{
				_rt_if_cmd_set_rip_auth_string(p_db_rtif, rip_auth_string, para);
			}
			break;

        /*support ipv6 address start*/
        case TBL_ROUTE_IF_FLD_IFC_IPV6:
// TODO:CC
#if 0
#if 1
            if (p_db_if && p_db_if->inband_snat_type)
            {
                CFG_INVALID_PARAM_RET("'%s' already assigned to %s",
                                      p_db_if->key.name,
                                      INBAND_SNAT_TYPE_TO_STRING(p_db_if->inband_snat_type));
            }
#endif

            /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
            if (vrrp_ip_address_check(p_db_rtif))
            {
                CFG_CONFLICT_RET("Interface IP address can't be changed because vrrp virtual ip is configured for this"
                    " interface");
            }
            /* liwh end */
#endif
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }

            if (p_db_rtif->dhcpv6_client_enable)
            {
                /* if not dhcpv6 add ipv6 address, should return conflict */
                if ((argc < 7) || ((argc >= 7) && sal_strcmp (argv[6], "dhcpv6")))
                {
                    CFG_CONFLICT_RET("IPv6 address conflicts with DHCPv6 client");
                }
            }

            rt_if_cmd_process_route_if_ipv6_address(argv, argc, para, p_db_if, p_db_rtif);
            break;
/*support ipv6 address end*/

        case TBL_ROUTE_IF_FLD_IFC_IPV4:
#if 1
            if (p_db_if && p_db_if->inband_snat_type)
            {
                CFG_INVALID_PARAM_RET("'%s' already assigned to %s",
                                      p_db_if->key.name,
                                      INBAND_SNAT_TYPE_TO_STRING(p_db_if->inband_snat_type));
            }
#endif

            /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
            if (vrrp_ip_address_check(p_db_rtif))
            {
                CFG_CONFLICT_RET("Interface IP address can't be changed because vrrp virtual ip is configured for this"
                    " interface");
            }
            /* liwh end */
            
            sal_memset(&ds_conn, 0, sizeof(ds_conn));
            if (0 == sal_strcmp(argv[2], "add"))
            {
                PM_CFG_CHECK_EXPECT_2PARAMS(argc, 5, 6, para->p_rs);
                is_add = TRUE;
            }
            else if (0 == sal_strcmp(argv[2], "del"))
            {
                is_add = FALSE;
                if (4 == argc)
                {
                    if (0 == sal_strcmp(argv[3], "all"))
                    {
                        is_del_all = TRUE;
                    }
                    else
                    {
                        CFG_INVALID_PARAM_RET("Invalid delete parameter %s", argv[3]);
                    }
                }
                else if (5 == argc)
                {
                    if (0 == sal_strcmp(argv[4], "secondary"))
                    {
                        del_all_sec = TRUE;
                    }
                }
                else
                {
                    PM_CFG_CHECK_EXPECT_2PARAMS(argc, 5, 6, para->p_rs);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid action %s", argv[1]);
            }

            if (argc > 4 && !del_all_sec)
            {
                if (argc == 6)
                {
                    GLB_SET_FLAG(ds_conn.flags, RT_IF_ADDR_SECONDARY);
                }
                
                CFG_PARSER_PREFIX(argv[3], argv[4], p_prefix, para->p_rs);

                rc = rt_if_ipv4_addr_check(p_prefix, p_db_if, para->p_rs->desc, MAX_CMD_RET_LEN);
                if (rc < 0)
                {
                    para->p_rs->rc = rc;
                    return rc;
                }
            }
            
            if (is_add)
            {
                rc = _rt_if_cmd_add_ipv4(p_db_rtif, &ds_conn, para);
                /* is dhclient enable ? we need to check rc and notify dhcp-server we fail to use this ip */
                _rt_if_cmd_dhclient_process_add_ip(rc, p_db_rtif);
            }
            else
            {
                if (del_all_sec)
                {
                    rc = _rt_if_cmd_del_all_ipv4_secondary(p_db_rtif, para);
                }
                else
                {
                    if (is_del_all)
                    {
                        rc = _rt_if_cmd_del_all_ipv4(p_db_rtif, para);
                    }
                    else
                    {
                        rc = _rt_if_cmd_del_ipv4(p_db_rtif, &ds_conn, para);
                    }
                }
                /* is dhlient enable ? */
                _rt_if_cmd_dhclient_process_del_ip(p_db_rtif);
            }
            break;
            
        case TBL_ROUTE_IF_FLD_OSPF_MD5_KEY:
            sal_memset(auth_md5_key, 0, sizeof(auth_md5_key));
            sal_memset(&ospf_auth_md5, 0, sizeof(ospf_auth_md5));
            if (5 == argc)
            {
                if (0 == sal_strcmp(argv[2], "add-md5"))
                {
                    if (rt_if_check_md5_key(argv[4], para))
                    {
                        return rc;
                    }
                    key_id = sal_atoi(argv[3]);
                    sal_strncpy(auth_md5_key, argv[4], OSPF_AUTH_MD5_MAX);
                }

                ospf_auth_md5.key.id = key_id;
                sal_strncpy(ospf_auth_md5.ospf_auth_md5_key, argv[4], OSPF_AUTH_MD5_MAX);
                _rt_if_cmd_add_ospf_auth_md5_key(p_db_rtif, &ospf_auth_md5, para);
            }
            else if (4 == argc)
            {
                if (0 == sal_strcmp(argv[2], "del-md5"))
                {
                    key_id = sal_atoi(argv[3]);
                }

                ospf_auth_md5.key.id = key_id;
                _rt_if_cmd_del_ospf_auth_md5_key(p_db_rtif, &ospf_auth_md5, para);
            }
            break;

        case TBL_ROUTE_IF_FLD_OSPF_COST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf_cost(p_db_rtif, value, para);
            break;

		case TBL_ROUTE_IF_FLD_OSPF6_COST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf6_cost(p_db_rtif, value, para);
            break;
			
		case TBL_ROUTE_IF_FLD_OSPF_PRIORITY:
			PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
			value++;//0-255 ->1-256
            _rt_if_cmd_set_ospf_priority(p_db_rtif, value, para);
			break;

		case TBL_ROUTE_IF_FLD_OSPF6_PRIORITY:
			PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
			value++;//0-255 ->1-256
            _rt_if_cmd_set_ospf6_priority(p_db_rtif, value, para);
			break;

        case TBL_ROUTE_IF_FLD_MTU:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("MTU can not set on layer 2 interface");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);

            /*1 check if the route interface is agg */
            sal_strcpy(if_key.name, argv[0]);
            pst_interface = tbl_interface_get_interface(&if_key);
            if (NULL == pst_interface)
            {
                return PM_E_NOT_EXIST;
            }

            if (GLB_IF_TYPE_PORT_IF == pst_interface->hw_type && (pst_interface->lag || pst_interface->lacp))
            {
                CFG_INVALID_PARAM_RET("Cannot configure aggregator member");
            }

            if (GLB_IF_TYPE_LINKAGG_IF == pst_interface->hw_type)
            {
                rc = rt_if_mtu_update_agg_member(pst_interface, value);
            }
            
            rt_if_set_mtu(argv[0], value);
            break;
        /*add by chenc for bug 50407, 2018-12-20*/
        case TBL_ROUTE_IF_FLD_MODE_SET_BEFORE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_ENUM(value, glb_if_mode_strs, GLB_IF_MODE_MAX, argv[2], para->p_rs);
            p_db_rtif->mode_set_before = value;
            rc = rt_if_set_mode_set_before(value, p_db_rtif, para);
            break;

        case TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("Arp retry-interval can not set on layer 2 interface");
            }

            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rt_if_set_arp_retry_interval(argv[0], value);
            break;

        case TBL_ROUTE_IF_FLD_ARP_TIMEOUT:
            if (NULL == p_db_rtif)
            {
                CFG_INVALID_PARAM_RET("Arp timeout can not set on layer 2 interface");
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_arp_ageing_timeout(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ARP_RETRY_TIMEOUT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_arp_retry_timeout(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_UNICAST_RPF_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
        /*Modified by zhush for bug 52864, 2019-07-25*/
            if (0 != p_db_rtif->nat_enable)
            {
               CFG_FUNC_ERR_RET("RPF enable failed, NAT is enable on this interface"); 
            }
            else
            {
                rc = _rt_if_cmd_set_ucast_rpf_en(p_db_rtif, value, para);
            }
        /*end zhush*/   
            break;

        case TBL_ROUTE_IF_FLD_UNICAST_RPF_EN_V6:
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }
            
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_ucast_rpf_en_v6(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_IP_UNREACHABLE_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_ip_unreachable_en(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_LOCAL_ARP_PROXY_EN:
    #ifdef OFPRODUCT
            CFG_UNSUPPORT_PRODUCT_RET();
    #endif
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_local_arp_proxy(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ARP_PROXY_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_arp_proxy(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_IP_REDIRECTS_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_ip_redirects_en(p_db_rtif, value, para);
            break;

            /*arp rate limit start*/
        case TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_arpratelimit_en(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_PKT_MAX:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_arpratelimit_pkt_max(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ARP_RATE_LIMIT_VIOLATION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_arpratelimit_exceed_action(p_db_rtif, value, para);
            break;
            /*arp rate limit end*/

        case TBL_ROUTE_IF_FLD_DHCP_RELAY_OPTION_TRUST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_dhcp_relay_option_trust(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_DHCP_SERVER_GROUP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_dhcp_server_group(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_OSPF_MTU_IGNORE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            if (value) {
                ospf_mtu_enable = TRUE;
            } else {
                ospf_mtu_enable = FALSE;
            }
            _rt_if_cmd_set_ospf_mtu_ignore_en(p_db_rtif, ospf_mtu_enable, para);
            break;
			
		case TBL_ROUTE_IF_FLD_OSPF6_MTU_IGNORE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            if (value) {
                ospf6_mtu_enable = TRUE;
            } else {
                ospf6_mtu_enable = FALSE;
            }
            _rt_if_cmd_set_ospf6_mtu_ignore_en(p_db_rtif, ospf6_mtu_enable, para);
            break;
			
        case TBL_ROUTE_IF_FLD_OSPF_TIMER_HELLO:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf_hello_interval(p_db_rtif, value, para);
            break;

		case TBL_ROUTE_IF_FLD_OSPF6_TIMER_HELLO:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf6_hello_interval(p_db_rtif, value, para);
            break;
			
        case TBL_ROUTE_IF_FLD_OSPF_TIMER_DEAD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf_dead_interval(p_db_rtif, value, para);
            break;

	    case TBL_ROUTE_IF_FLD_OSPF6_TIMER_DEAD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_ospf6_dead_interval(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_OSPF_AUTH_TYPE:
			if(0 == sal_strcmp(argv[2], "message-digest"))
                value = OSPF_RIF_AUTH_TYPE_MD5;
            else if(0 == sal_strcmp(argv[2], "simple"))
                value = OSPF_RIF_AUTH_TYPE_SIMPLE;
            else if(0 == sal_strcmp(argv[2], "default"))
                value = OSPF_RIF_AUTH_TYPE_DEFAULT;
            else if((0 == sal_strcmp(argv[2], "no-message-digest")) && 
                    (p_db_rtif->ospf_auth_type == OSPF_RIF_AUTH_TYPE_MD5))
                value = OSPF_RIF_AUTH_TYPE_DEFAULT;
            else
                value = p_db_rtif->ospf_auth_type; /* no change */
            
            _rt_if_cmd_set_ospf_auth_type(p_db_rtif, value, para);	
            break;

        case TBL_ROUTE_IF_FLD_OSPF_AUTH_KEY:
            sal_memset(auth_key, 0, sizeof(auth_key));
            if (3 == argc)
            {
                if (rt_if_check_simple_key(argv[2], para))
                {
                    return rc;
                }

                sal_strncpy(auth_key, argv[2], OSPF_AUTH_SIMPLE_MAX);
                _rt_if_cmd_set_ospf_auth_key(p_db_rtif, auth_key, para);
            }
            else if (2 == argc)
            {
                _rt_if_cmd_set_ospf_auth_key(p_db_rtif, auth_key, para);
            }
            break;
        case TBL_ROUTE_IF_FLD_OSPF_NETWORK_TYPE:
			if (4 == argc && 0 == sal_strcmp(argv[2], "add"))
			{
				if (0 == sal_strcmp(argv[3], "broadcast"))
				{
					value = OSPF_TYPE_BROADCAST;
					_rt_if_cmd_set_ospf_network_type(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[3], "non-broadcast"))
				{
					value = OSPF_TYPE_NBMA;
					_rt_if_cmd_set_ospf_network_type(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[3], "point-to-multipoint"))
				{
					value = OSPF_TYPE_P2M;
					_rt_if_cmd_set_ospf_network_type(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[3], "point-to-point"))
				{
					value = OSPF_TYPE_P2P;
					_rt_if_cmd_set_ospf_network_type(p_db_rtif, value, para);
				}
			}
			else if (3 == argc && 0 == sal_strcmp(argv[2], "del"))
			{
				value = OSPF_TYPE_NONE;
				_rt_if_cmd_set_ospf_network_type(p_db_rtif, value, para);
			}
            break;
		case TBL_ROUTE_IF_FLD_OSPF6_NETWORK_TYPE:
			if (4 == argc && 0 == sal_strcmp(argv[2], "add"))
			{
				if (0 == sal_strcmp(argv[3], "broadcast"))
				{
					value = OSPF_TYPE_BROADCAST;
					_rt_if_cmd_set_ospf6_network_type(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[3], "point-to-point"))
				{
					value = OSPF_TYPE_P2P;
					_rt_if_cmd_set_ospf6_network_type(p_db_rtif, value, para);
				}
			}
			else if (3 == argc && 0 == sal_strcmp(argv[2], "del"))
			{
				value = OSPF_TYPE_NONE;
				_rt_if_cmd_set_ospf6_network_type(p_db_rtif, value, para);
			}
            break;
        case TBL_ROUTE_IF_FLD_DHCP_CLIENT_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            if (0 == value && 0 == p_db_rtif->dhcp_client_enable)
            {
                return PM_E_NONE;
            }

            /* modified by chenc for bug 52470, 2019-06-24 */
            if (1 == value && 1 == p_db_rtif->dhcp_client_enable)
            {
                CFG_INVALID_PARAM_RET("DHCP client has already been enabled");
            }
            
            /*2. enable dhcp */
            _rt_if_cmd_set_dhcp_client_en(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_DHCP_CLIENT_IPV4:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            _rt_if_cmd_set_dhcp_client_en(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_RA_CURRRENT_HOP_LIMIT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_ra_hop_limit_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_ra_advertise_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE_MTU:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_ra_advertise_mtu_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_RA_INTERVAL_MAX:
            PM_CFG_GET_UINT(ra_interval_max, argv[2], para->p_rs);
            if (argc > 3)
            {
                PM_CFG_GET_UINT(ra_interval_min, argv[3], para->p_rs);    
            }
            nd_ra_interval_set(p_db_rtif, ra_interval_max, ra_interval_min, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_ROUTER_LIFETIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

            if (0 == sal_strcmp(argv[2], "default"))
            {
                value = p_db_rtif->nd_ra_interval_max * 3 /1000;
            }
            else
            {
                PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            }
            nd_ra_lifetime_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_DAD_ATTEMPTS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_dad_attempts_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_RETRANSMIT_TIMER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_ns_interval_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_REACHABLE_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_reachable_time_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_MANAGED_CONFIG:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_managed_config_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_OTHER_CONFIG:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            nd_other_config_set(p_db_rtif, value, para);
            break;

        case TBL_ROUTE_IF_FLD_ND_PREFIX:
            if ((argc >= 3) &&  (!sal_strcmp(argv[2], "default")))
            {
                nd_prefix_default_set(p_db_rtif, &argv[2], (argc - 2), para);
            }
            break;

        /* modified by liwh for bug 49365, 2018-11-27 */
        case TBL_ROUTE_IF_FLD_DHCPV6_CLIENT_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
 #ifdef GOLDENGATE /* modified by wus for bug 53734, 2019-10-26 */
            CFG_FUNC_ERR_RET("Dhcpv6 client function is not supported on this device");
 #endif
            /* enable dhcpv6 */
            _rt_if_cmd_set_dhcpv6_client_en(p_db_rtif, value, para);
            break;
        /* liwh end */

        case TBL_ROUTE_IF_FLD_NAT_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
        /*Modified by zhush for bug 52864, 2019-07-25*/
            if (1 == p_db_rtif->unicast_rpf_en)
            {
                CFG_FUNC_ERR_RET("NAT enable failed, RPF verify is enable on this interface");
            }
            else
            {
                rc = _rt_if_cmd_set_nat_en(p_db_rtif, value, para);
                if (PM_E_RESOURCE_FULL == rc)
                {
                    if (GLB_IF_NAT_OUTSIDE == value)
                    {
                        CFG_FUNC_ERR_RET("The number of NAT outside interfaces beyond limit %u.", GLB_NAT_OUTSIDE_IF_NUM);
                    }
                    else if (GLB_IF_NAT_INSIDE == value) 
                    {
                        CFG_FUNC_ERR_RET("The number of NAT inside interfaces beyond limit %u.", GLB_NAT_INSIDE_IF_NUM);
                    }
                }
            }
        /*end zhush*/
            break;
		/*add by zhw for mc*/
		case TBL_ROUTE_IF_FLD_PIM_MODE:
			if (3 == argc)
			{
				if (0 == sal_strcmp(argv[2], "dense-mode"))
				{
					value = PIM_MODE_DM;
					_rt_if_cmd_set_pim_mode(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[2], "sparse-mode"))
				{
					value = PIM_MODE_SM;
					_rt_if_cmd_set_pim_mode(p_db_rtif, value, para);
				}
				else if (0 == sal_strcmp(argv[2], "0"))
				{
					value = PIM_MODE_NONE;
					_rt_if_cmd_set_pim_mode(p_db_rtif, value, para);
				}
			}
			break;

		case TBL_ROUTE_IF_FLD_MULTICAST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
            rc = _rt_if_cmd_set_multicast(p_db_rtif, value, para);
			break;
			
		case TBL_ROUTE_IF_FLD_IGMP:
			if (3 == argc && 0 == sal_strcmp(argv[1], "igmp"))
	        {
	            PM_CFG_GET_UINT(value, argv[2], para->p_rs);
				rc = _rt_if_cmd_set_igmp(p_db_rtif, value, para);
			}
			break;
        case TBL_ROUTE_IF_FLD_IGMP_VERSION:
			if (3 == argc && 0 == sal_strcmp(argv[1], "igmp_version"))
	        {
	            sal_strcpy(p_db_rtif->igmp_version, argv[2]);
	            tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_IGMP_VERSION);
			}
			break;
		case TBL_ROUTE_IF_FLD_IGMP_INTF_LIMIT_NUMBER:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "igmp_intf_limit_number"))
			 {
				 PM_CFG_GET_UINT(p_db_rtif->igmp_intf_limit_number, argv[2], para->p_rs);
				 tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_IGMP_INTF_LIMIT_NUMBER);
			 }
			 break;
		/*end add zhw*/
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            ifname = NULL;
        }
        else if (1 <= argc)
        {
            if (!sal_strcmp(argv[0], "all"))
            {
                ifname = NULL;
            }
            else
            {
                ifname = argv[0];
            }
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _rt_if_cmd_get_if(ifname, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
_rt_if_cmd_set_dhcp_ip_sync(tbl_route_if_t* p_db_rt_if)
{
    int32 rc = PM_E_NONE;
    int32 ipv4_len = 0, mask_len = 0;
    ds_connected_t ds_conn;
    prefix_t *p_prefix = &ds_conn.key.address;
    cfg_cmd_para_t para;
    cfg_result_t p_rs;
    tbl_interface_t *p_db_interf_if = NULL;
    tbl_interface_key_t if_key;

    para.p_rs = (cfg_result_t *)&p_rs;
    sal_memset(&ds_conn, 0, sizeof(ds_conn));
    ipv4_len = sal_strlen(p_db_rt_if->dhcp_client_ipv4);
    mask_len = sal_strlen(p_db_rt_if->dhcp_client_ipv4_mask);

    sal_strncpy(if_key.name, p_db_rt_if->key.name, IFNAME_SIZE);
    p_db_interf_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_interf_if)
    {
        return PM_E_INVALID_IFNAME;
    }

    CFG_PARSER_PREFIX(p_db_rt_if->dhcp_client_ipv4, p_db_rt_if->dhcp_client_ipv4_mask, p_prefix, para.p_rs);
    rc = rt_if_ipv4_addr_check(p_prefix, p_db_interf_if, para.p_rs->desc, MAX_CMD_RET_LEN);
    if (rc < 0)
    {
        return rc;
    }

    /* add dhcp ip*/
    if  (ipv4_len && mask_len)
    {
        rc = _rt_if_cmd_add_ipv4(p_db_rt_if, &ds_conn, &para);
    }
    else
    {
        /* remove dhcp ip*/
        rc = _rt_if_cmd_del_ipv4(p_db_rt_if, &ds_conn, &para);
    }
    
    return rc;
}

int32
rt_if_cmd_process_route_if_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_route_if_t* p_sys_if = NULL;
    tbl_route_if_t * p_db_rt_if = NULL;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:        
        p_sys_if = (tbl_route_if_t*)p_tbl;
        if(NULL == p_sys_if)
        {
            return PM_E_INVALID_PARAM;
        }
        p_db_rt_if = tbl_route_if_get_route_if(&p_sys_if->key);
        if(NULL == p_db_rt_if)
        {
            return PM_E_NOT_EXIST;
        }     
        
        switch(field_id)
        {
            case TBL_ROUTE_IF_FLD_DHCP_CLIENT_IPV4:
                rc = _rt_if_cmd_set_dhcp_ip_sync(p_db_rt_if);
                break;
        }

        break;

    default:
        break;
    }

    return rc;
}

