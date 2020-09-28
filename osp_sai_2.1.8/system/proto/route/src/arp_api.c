
#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "route.h"
#include "prefix.h"
#include "rt_fea.h"
#include "lib_ioctl.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_api.h"
#include "arp_ether.h"
#include "arp_sm.h"
#include "lib_netlink.h"
#include "hsrv_msg.h"
#include "rt_if.h"
#include "glb_openflow_define.h"

ctc_task_t *arp_numberlimit_attacktimer = NULL;
#define ARP_PORT_LIMIT_ATTACK_LOG_TIME 30

int32
rt_api_add_static_route_to_frr_when_del_arp(tbl_arp_t *p_db_arp)
{
    int32 rc = PM_E_NONE;
    char buf1[CMD_BUF_16] = {0}; 
    char buf2[CMD_BUF_16] = {0}; 
    char buf3[CMD_BUF_16] = {0}; 

    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t  *nextnode = NULL;
    ds_static_route_dup_ipv4_nh_t *p_db_static_route_dup_ipv4_nh = NULL;
    
    if(!p_db_arp->is_static_route_dup)
    {
        return PM_E_NONE;
    }

    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;

    sal_memset(&static_route, 0x00, sizeof(static_route));
    static_route.key.p.family = AF_INET;
    static_route.key.p.prefixlen = 32;

    sal_memcpy(&static_route.key.p.u.prefix4, &p_db_arp->key.ip, sizeof(addr_ipv4_t));
    CTCLIB_SLIST_LOOP_DEL(p_db_arp->static_route_dup_ipv4_nh.obj_list, p_db_static_route_dup_ipv4_nh, listnode, nextnode)
    {
        sal_memset(&static_route.key.nh_addr.ipv4, 0x00, sizeof(addr_ipv4_t));
        sal_memcpy(&static_route.key.nh_addr.ipv4, &p_db_static_route_dup_ipv4_nh->key.address, sizeof(addr_ipv4_t));
        pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
        if (NULL == pst_static_route)
        {
            ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh(p_db_arp, p_db_static_route_dup_ipv4_nh);
            continue;
        }

        DEBUG_ROUTE_IPV4("Add static route %s/%d %s rib for del arp %s\n", 
                    cdb_addr_ipv4_val2str(buf1, CMD_BUF_16, &pst_static_route->key.p.u.prefix4),
                    pst_static_route->key.p.prefixlen, 
                    cdb_addr_ipv4_val2str(buf2, CMD_BUF_16, &pst_static_route->key.nh_addr.ipv4),
                    cdb_addr_ipv4_val2str(buf3, CMD_BUF_16, &p_db_arp->key.ip));
                    
        rc = rt_if_add_static_route_by_syscmd(&pst_static_route->key.p.u.prefix4, &pst_static_route->key.nh_addr.ipv4, 
            pst_static_route->key.p.prefixlen, pst_static_route->distance);

        ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh(p_db_arp, p_db_static_route_dup_ipv4_nh);
    }

    p_db_arp->is_static_route_dup = FALSE;
    tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_IS_STATIC_ROUTE_DUP);
    SAL_ASSERT(CTCLIB_SLIST_ISEMPTY(p_db_arp->static_route_dup_ipv4_nh.obj_list));
    return rc;
}

int32
rt_api_del_static_route_from_frr_when_add_arp(tbl_arp_t *p_arp)
{
    tbl_arp_t *p_db_arp = NULL;
    char buf1[CMD_BUF_16] = {0}; 
    char buf2[CMD_BUF_16] = {0}; 
    char buf3[CMD_BUF_16] = {0}; 
    ds_static_route_dup_ipv4_nh_t ds_static_route_dup_ipv4_nh;
    ds_static_route_dup_ipv4_nh_t *p_db_ds_static_route_dup_ipv4_nh = NULL;

    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NONE;
    }
    
    if(!p_db_arp->is_static_route_dup)
    {
        //return PM_E_NONE;
    }

    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        if (AF_INET == p_db_rst_cfg->key.p.family
            && 0 == sal_memcmp(&p_db_rst_cfg->key.p.u.prefix4, &p_db_arp->key.ip, sizeof(addr_ipv4_t)))
        {
            DEBUG_ROUTE_IPV4("Delete static route %s/%d %s rib for add arp %s\n", 
                cdb_addr_ipv4_val2str(buf1, CMD_BUF_16, &p_db_rst_cfg->key.p.u.prefix4),
                p_db_rst_cfg->key.p.prefixlen, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_16, &p_db_rst_cfg->key.nh_addr.ipv4),
                cdb_addr_ipv4_val2str(buf3, CMD_BUF_16, &p_db_arp->key.ip));
                
            rt_if_del_static_route_by_syscmd(&p_db_rst_cfg->key.p.u.prefix4, &p_db_rst_cfg->key.nh_addr.ipv4, 
                p_db_rst_cfg->key.p.prefixlen);

            p_db_arp->is_static_route_dup = TRUE;
            sal_memset(&ds_static_route_dup_ipv4_nh, 0x00, sizeof(ds_static_route_dup_ipv4_nh));
            sal_memcpy(&ds_static_route_dup_ipv4_nh.key.address, &p_db_rst_cfg->key.nh_addr.ipv4, sizeof(addr_ipv4_t));
            p_db_ds_static_route_dup_ipv4_nh = ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(p_db_arp, &ds_static_route_dup_ipv4_nh);
            if (p_db_ds_static_route_dup_ipv4_nh)
            {
                
            }
            else
            {
                ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh(p_db_arp, &ds_static_route_dup_ipv4_nh);
                tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_IS_STATIC_ROUTE_DUP);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_arp_api_probe_for_fdb_age(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    uint32 *ip_addr = NULL;    
    tbl_arp_t *ae_p = NULL;
    addr_ipv4_t ip;

    ip_addr = (uint32 *)p_msg->data;
    ip.s_addr = *ip_addr;
    ip.s_addr = sal_ntoh32(ip.s_addr);
    
    ae_p = arp_db_entry_lookup(&ip);
    if (NULL != ae_p && ARP_TYPE_DYNAMIC == ae_p->type)
    {
        ae_p->aging_delay = 0;    
        GLB_SET_FLAG(ae_p->flags, ARP_SYN_FDB_AGE);
    }

    return PM_E_NONE;
}

int32
arp_api_ifc_match(addr_ipv4_t *addr, tbl_kernel_if_t *pst_kernel_if)
{
    prefix_t q;
    prefix_t p;

    sal_memset(&q, 0, sizeof(prefix_t));
    sal_memset(&p, 0, sizeof(prefix_t));    
    p.prefixlen = IPV4_MAX_PREFIXLEN;
    p.family = AF_INET;
    sal_memcpy(&p.u.prefix4, addr, sizeof(addr_ipv4_t));

    if (NULL != pst_kernel_if)
    {
        q.prefixlen = pst_kernel_if->masklen;
        q.family = AF_INET;
        sal_memcpy(&q.u.prefix4, &pst_kernel_if->ip_addr, sizeof(addr_ipv4_t));
        ip_apply_mask(&q);
    }

    if (prefix_compare_net(&q, &p))
    {
        return TRUE;
    }

    return FALSE;
}

int32
arp_api_rt_ifc_match(addr_ipv4_t *addr, ds_connected_t *ifc)
{
    prefix_t q;
    prefix_t p;

    sal_memset(&q, 0, sizeof(prefix_t));
    sal_memset(&p, 0, sizeof(prefix_t));    
    p.prefixlen = IPV4_MAX_PREFIXLEN;
    p.family = AF_INET;
    sal_memcpy(&p.u.prefix4, addr, sizeof(addr_ipv4_t));

    if (NULL != ifc)
    {
        ip_prefix_copy(&q, &ifc->key.address);
        ip_apply_mask(&q);
    }

    if (prefix_compare_net(&q, &p))
    {
        return TRUE;
    }

    return FALSE;
}

static int32
_arp_api_ifc_add(tbl_arp_t *ae_p, void *p_data, void *p_data1)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data1;
    //tbl_kernel_if_t *pst_kernel_if = (tbl_kernel_if_t*)p_data;

    tbl_route_if_t *p_db_rtif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ds_connected_t *ifc = (ds_connected_t*)p_data;
    ds_connected_t *ifc_tmp = NULL;
    
    addr_ipv4_t mask_addr;
    addr_ipv4_t host_addr;
    int32       is_routemac = 0;    
    int32       rc = PM_E_NONE;

    if (NULL == ifc || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    
    /*when mac is route mac, we need not tracv this arp entry*/
    if (!ae_p->is_router)
    {        
        rc = arp_api_check_is_routemac(p_db_if, ae_p->mac_addr, &is_routemac);
    }
    (void)rc;

    if (!is_routemac)
    {
        if (ARP_TYPE_STATIC == ae_p->type && GLB_INVALID_IFINDEX == ae_p->ifindex)
        {
            /* for static arp which equal to ifc address, just assign ifindex */
            if (ae_p->key.ip.s_addr == ifc->key.address.u.prefix4.s_addr)
            {
                ae_p->ifindex = p_db_if->ifindex;
            }
            
            /* if ip does not match ifc, return */
            if (FALSE == arp_api_rt_ifc_match(&ae_p->key.ip, ifc))
            {
                return PM_E_NONE;
            }

            /* Filter subnet broadcast and network address */
            addr_mask_len2ip_v4(ifc->key.address.prefixlen, &mask_addr);
            host_addr.s_addr = ae_p->key.ip.s_addr & ~(mask_addr.s_addr);
            if( host_addr.s_addr == ~(mask_addr.s_addr) || host_addr.s_addr == 0)
            {
                return PM_E_NONE;
            }

            /* matched */
            ae_p->ifindex = p_db_if->ifindex;
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);
            sal_memcpy(ae_p->ifname, p_db_if->key.name, sizeof(ae_p->ifname));
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);
        }
    }

    if (ARP_TYPE_STATIC == ae_p->type
                && p_db_if->ifindex == ae_p->ifindex
                && ae_p->is_router == FALSE)
    {
        /* for static arp which equal to ifc address, delete it from os */
        if (ae_p->key.ip.s_addr == ifc->key.address.u.prefix4.s_addr)
        {
            arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);

            /* set invalid ifindex */
            ae_p->ifindex = GLB_INVALID_IFINDEX;
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);

            sal_memset(ae_p->ifname, 0, sizeof(ae_p->ifname));
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);

            return PM_E_NONE;
        }

        /*added by ychen in 2013-09-12 for bug 24603, check other address*/
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
        {
            /*modified by ychen for bug 24999 in 2013-09-16*/
            if (ifc_tmp != ifc && ae_p->key.ip.s_addr == ifc_tmp->key.address.u.prefix4.s_addr)
            {
                arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);

                /* set invalid ifindex */
                ae_p->ifindex = GLB_INVALID_IFINDEX;
                tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);

                sal_memset(ae_p->ifname, 0, sizeof(ae_p->ifname));
                tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);
            }
        }
        /*ychen end*/
    }
    else if (ARP_TYPE_DYNAMIC == ae_p->type)
    {
        /* matched */
        if (TRUE == arp_api_rt_ifc_match(&ae_p->key.ip, ifc))
        {
            ae_p->ifindex = p_db_if->ifindex;
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);
            sal_memcpy(ae_p->ifname, p_db_if->key.name, sizeof(ae_p->ifname));
            tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);
        }
    }

    return PM_E_NONE;
}

static int32
_arp_api_ifc_del(tbl_arp_t *ae_p, void *p_data, void *p_data1)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data1;
    //tbl_kernel_if_t *pst_kernel_if = (tbl_kernel_if_t*)p_data;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *ifc = (ds_connected_t*)p_data;
    ds_connected_t *ifc_tmp = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32           rc = PM_E_NONE;

    if (NULL == ifc || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    #if 0
    if (0 != sal_strncmp(ae_p->ifname, pst_kernel_if->name, sizeof(pst_kernel_if->name)))
    {
        return PM_E_NONE;
    }
    #endif

    if (TRUE != arp_api_rt_ifc_match(&ae_p->key.ip, ifc))
    {
        return PM_E_NONE;
    }

    /* add the arp whoes address same as ifc and match other ifc on same ifp to os */
    if (ae_p->key.ip.s_addr == ifc->key.address.u.prefix4.s_addr &&
       ARP_TYPE_STATIC == ae_p->type && 
       GLB_INVALID_IFINDEX == ae_p->ifindex)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
        {
            if (ifc_tmp != ifc && arp_api_rt_ifc_match(&ae_p->key.ip, ifc_tmp))
            {   
                ae_p->ifindex = p_db_if->ifindex;
                tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);
                sal_memcpy(ae_p->ifname, p_db_if->key.name, sizeof(ae_p->ifname));
                tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);

                arp_db_synch_hw_os(ae_p, ARP_SYN_ADD);
                return PM_E_NONE;
            }
        }
    }

    if (ARP_TYPE_STATIC == ae_p->type && ae_p->is_router) 
    {
        if (ae_p->key.ip.s_addr == ifc->key.address.u.prefix4.s_addr)
        {
            rc = arp_db_entry_remove(ae_p);
            return PM_E_NONE;
        }
    }
    else if (ARP_TYPE_STATIC == ae_p->type)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
        {
            if (ifc_tmp != ifc && arp_api_rt_ifc_match(&ae_p->key.ip, ifc_tmp))
            {
                return PM_E_NONE;
            }
        }
        
        /* set invalid ifindex */
        ae_p->ifindex = GLB_INVALID_IFINDEX;
        tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);

        sal_memset(ae_p->ifname, 0, sizeof(ae_p->ifname));
        tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);
        
        /* Added by Jeffrey for bug 4545 */
        arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);

        return PM_E_NONE;
    }
    
    /* Remove dynamic arp which no matched address on the interface */
    if (tbl_interface_is_running(p_db_if))
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
        {
            if (ifc_tmp != ifc && arp_api_rt_ifc_match(&ae_p->key.ip, ifc_tmp))
            {
                return PM_E_NONE;
            }
        }
    }
    
    if (ARP_TYPE_DYNAMIC == ae_p->type) 
    {
        arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);    
        arp_sm(ae_p, AE_DELETE_ENTRY);
    }

    (void)rc;
    return PM_E_NONE;
}

static int32
_arp_api_ifc_up(tbl_arp_t *p_db_arp, void *p_data, void *p_data1)
{
    int32 rc = PM_E_NONE;
    
    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
        if (TRUE != p_db_arp->is_router)
        {
            rc = arp_db_synch_hw_os(p_db_arp, ARP_SYN_ADD);
        }
    }
    else
    {
        /* here not possible
         * if dynamic arp entry existed, error!
         */
    }

    return rc;
}

int32
_arp_api_ifc_down(tbl_arp_t *p_db_arp, void *p_data, void *p_data1)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data1;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *ifc = (ds_connected_t*)p_data;
    ds_connected_t *ifc_tmp = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if (NULL == ifc || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
        /* Here for ifc addr check */
        if (tbl_interface_is_running(p_db_if))
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
            {
                if (ifc_tmp != ifc && arp_api_rt_ifc_match(&p_db_arp->key.ip, ifc_tmp))
                {
                    return PM_E_NONE;
                }
            }
        }
        
        if (TRUE != p_db_arp->is_router)
        {
            rc = arp_db_synch_hw_os(p_db_arp, ARP_SYN_DEL);
        }
        
        return rc;
    }
    else
    {
        /* Here for ifc addr check */
        if (tbl_interface_is_running(p_db_if))
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
            {
                if (ifc_tmp != ifc && arp_api_rt_ifc_match(&p_db_arp->key.ip, ifc_tmp))
                {
                    return PM_E_NONE;
                }
            }
        }
        
        rc = arp_db_synch_hw_os(p_db_arp, ARP_SYN_DEL);
        arp_sm(p_db_arp, AE_DELETE_ENTRY);
     }
   
    return PM_E_NONE;
}

static int32
_arp_api_synch_add_os(tbl_arp_t *p_db_arp)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    int32 rc = PM_E_NONE;

    pst_kernel_if = tbl_kernel_if_get_by_name(p_db_arp->ifname);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    rc = kernel_neigh_entry_add(&p_db_arp->key.ip, p_db_arp->mac_addr, pst_kernel_if->key.ifindex, p_db_arp->is_proxy);
    if (rc >= 0)/*fix bug 47331 by chenc*/
    //if (!rc)
    {
        GLB_SET_FLAG(p_db_arp->flags, ARP_SYN_OS);
    }

    return rc;
}

static int32
_arp_api_synch_add_hw(tbl_arp_t *p_db_arp)
{
    prefix_t p;
    int32 rc = PM_E_NONE;

    sal_memset(&p, 0, sizeof(prefix_t));
    p.family = AF_INET;
    p.prefixlen = IPV4_MAX_PREFIXLEN;
    sal_memcpy(&p.u.prefix4, &p_db_arp->key.ip, sizeof(p_db_arp->key.ip));

    rc = rt_fea_ipv4_add_neighbor(GLB_DEF_VRFID, &p_db_arp->key.ip, p_db_arp->mac_addr, 
        p_db_arp->ifindex, (ARP_TYPE_STATIC == p_db_arp->type), p_db_arp->ifname);
    
    if (!rc)
    {
        GLB_SET_FLAG(p_db_arp->flags, ARP_SYN_HW);
        rt_api_del_static_route_from_frr_when_add_arp(p_db_arp);
    }

    return rc;
}

static int32
_arp_api_synch_del_os(tbl_arp_t *p_db_arp)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    int32 rc = PM_E_NONE;

    if (!GLB_FLAG_ISSET(p_db_arp->flags, ARP_SYN_OS))
    {
        return rc;
    }

    pst_kernel_if = tbl_kernel_if_get_by_name(p_db_arp->ifname);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    rc = kernel_neigh_entry_del(&p_db_arp->key.ip, p_db_arp->mac_addr, pst_kernel_if->key.ifindex);
    if (!rc)
    {
        GLB_SET_FLAG(p_db_arp->flags, ARP_SYN_OS);
    }

    return rc;
}

static int32
_arp_api_synch_del_hw(tbl_arp_t *p_arp)
{
    int32 rc = PM_E_NONE;
    tbl_arp_t *p_db_arp = NULL;

    p_db_arp = tbl_arp_get_arp(&p_arp->key);
    if (NULL == p_db_arp)
    {
        return PM_E_NONE;
    }

    if (!GLB_FLAG_ISSET(p_db_arp->flags, ARP_SYN_HW))
    {
        return rc;
    }

    rc = rt_fea_ipv4_del_neighbor(GLB_DEF_VRFID, &p_db_arp->key.ip, (ARP_TYPE_STATIC == p_db_arp->type));
    if (!rc)
    {
        GLB_UNSET_FLAG(p_db_arp->flags, ARP_SYN_HW);
        rt_api_add_static_route_to_frr_when_del_arp(p_db_arp);
    }
    
    return rc;
}

int32
arp_db_synch_hw_os(tbl_arp_t *ae_p, arp_syn_op_t op)
{
    char addr[INET_ADDRSTRLEN];
    int32 rc = PM_E_NONE;

    if (ARP_TYPE_STATIC == ae_p->type)
    {
        ARP_EVENT_DEBUG("%s, static arp IP is %s, operation is %s",
            __FUNCTION__, sal_inet_ntop(AF_INET, &ae_p->key.ip, addr, INET_ADDRSTRLEN),
            (ARP_SYN_ADD == op) ? "add" : "del");
    }
    else
    {
        ARP_EVENT_DEBUG("%s, dynamic arp IP is %s, state is %s, operation is %s",
            __FUNCTION__, sal_inet_ntop(AF_INET, &ae_p->key.ip, addr, INET_ADDRSTRLEN),
            cdb_enum_val2str(as_strs, AS_MAX, ae_p->status), 
            (ARP_SYN_ADD == op) ? "add" : "del");
    }

    switch(op)
    {
    case ARP_SYN_ADD:
        rc = _arp_api_synch_add_hw(ae_p);
        if(!rc)
            rc = _arp_api_synch_add_os(ae_p);
        break;

    case ARP_SYN_DEL:
        rc = _arp_api_synch_del_hw(ae_p);
        if(!rc)
        {
            rc = _arp_api_synch_del_os(ae_p);
        }
        break;

    default:
        break;
    }

    return rc;
}

int32
arp_db_count_inc(tbl_arp_t *p_arp)
{
    if (p_arp->is_router)
    {
        return PM_E_NONE;
    }
    
    rt_neigh_counter_inc(GLB_DEF_VRFID);
    return PM_E_NONE;
}

int32
arp_db_count_dec(tbl_arp_t *p_arp)
{
    if (p_arp->is_router)
    {
        return PM_E_NONE;
    }
    
    rt_neigh_counter_dec(GLB_DEF_VRFID);
    return PM_E_NONE;
}

uint32
arp_api_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) { 
        return 0;
    }

    type = p_chassis->cur_stm_mode;    
    if (type < GLB_STM_MODE_MAX) {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) {
            return p_sys_spec->host_routes;
        }
    }
            
    return 0;
}

uint32
arp_db_cache_overflow()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    //if (p_rt_glb->current_host_routes  >= GLB_ARP_CACHE_MAX)
    if (p_rt_glb->current_host_routes  >= arp_api_count_get_max())
    {
        return TRUE;
    }
    
    return FALSE;
}

static tbl_arp_t*
_arp_api_get_gra_arp(tbl_interface_t *p_db_if, ds_connected_t *ifc, tbl_arp_t *ae_tmp)
{
    tbl_arp_t   *ae_p = NULL;

    ae_p = arp_db_entry_lookup(&ifc->key.address.u.prefix4);
    if ((NULL == ae_p) || (FALSE == ae_p->is_router))
    {
        sal_memcpy(ae_tmp->mac_addr, p_db_if->mac_addr, MAC_ADDR_LEN);
        sal_memcpy(&ae_tmp->key.ip, &ifc->key.address.u.prefix4.s_addr, sizeof(addr_ipv4_t));
        ae_tmp->is_router = TRUE;
        ae_tmp->status = AS_REACHABLE;
        ae_tmp->ifindex = p_db_if->ifindex;
        ae_tmp->vrf_id = GLB_DEF_VRFID;
        ae_p = ae_tmp;
    }

    return ae_p;
}

static int32
_arp_api_gra_trigger(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    tbl_arp_t *ae_p = NULL;
    tbl_arp_t ae_tmp;
    ctclib_slistnode_t *listnode = NULL;
    ds_connected_t *ifc = NULL;
    
    sal_memset(&ae_tmp, 0, sizeof(ae_tmp));
    /* send gratuitous for specific address */
    if (p_conn)
    {
        ifc = p_conn;
        ae_p = _arp_api_get_gra_arp(p_db_if, ifc, &ae_tmp);
        arp_sm(ae_p, AE_TX_GARP_REQ);
        return PM_E_NONE;
    }

    /* send gratuitous arp for all ipv4 address */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc, listnode)
    {
        ae_p = _arp_api_get_gra_arp(p_db_if, ifc, &ae_tmp);
        arp_sm(ae_p, AE_TX_GARP_REQ);
    }
    
    return PM_E_NONE;    
}

static int32
_arp_api_trigger_request(addr_ipv4_t *ip_addr, tbl_interface_t *p_db_if)
{
    tbl_arp_t *p_db_arp = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_arp = arp_db_entry_lookup(ip_addr);
    if (NULL == p_db_arp)
    {
        /* create a new entry */
        rc = arp_db_entry_create(p_db_if, ip_addr, NULL, ARP_TYPE_DYNAMIC, FALSE, FALSE, &p_db_arp);
        if (rc < 0)
        {
            return rc;
        }
    }
    else
    {
        switch(p_db_arp->status)
        {
        case AS_REACHABLE:
            /* for AS_REACHABLE, synchronize it to os and hw */
            arp_db_synch_hw_os(p_db_arp, ARP_SYN_ADD);
            return PM_E_NONE;
            
        case AS_INIT:
            /* just change the state */
            break;

        case AS_INCOMPLETE:
        case AS_REFRESH:
            /* request is send out and wait for reply */
            return PM_E_NONE;
            
        default:
            return PM_E_INVALID_PARAM;
        }
    }

    p_db_arp->status = AS_INCOMPLETE;

    arp_sm(p_db_arp, AE_TX_ARP_REQ);

    return PM_E_NONE;
}

int32
arp_db_entry_create(tbl_interface_t *p_db_if, addr_ipv4_t *ip, 
             uint8 *ll_addr, arp_type_t type, 
             uint8 is_proxy, uint8 is_router, tbl_arp_t **entry)
{
    tbl_route_if_t* p_route_if = NULL;
    tbl_route_if_key_t route_if_key;
    addr_ipv4_t     mask_addr;
    addr_ipv4_t     host_addr;
    uint32          ifindex = 0;
    tbl_arp_t       arp;
    tbl_arp_t           *p_db_arp = NULL;
    ds_connected_t      *ifc = NULL;
    tbl_route_if_t      *p_rt_if = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    int32               rc = PM_E_NONE;

    sal_memset(&mask_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&host_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&arp, 0, sizeof(arp));
    
    if (!is_router && arp_db_cache_overflow())
    {
        arp_debug_cache_overflow();
        return PM_E_EXCEED_MAX_NUM;
    }

    p_db_arp = arp_db_entry_lookup(ip);
    if (NULL != p_db_arp)
    {
        *entry = NULL;
        *entry = p_db_arp;
        
        return PM_E_EXIST;
    }

    p_rt_if = arp_api_check_addr_with_rt_if_exist(p_db_if, ip);
    if (NULL == p_rt_if)
    {
        ifindex = GLB_INVALID_IFINDEX;
        sal_memset(arp.ifname, 0, sizeof(arp.ifname));
    }
    else
    {
        ifindex = p_db_if->ifindex;
        if (GLB_IF_MODE_L2 == p_db_if->mode)
        {
            sal_memcpy(arp.ifname, p_rt_if->key.name, sizeof(arp.ifname));
        }
        else if (GLB_IF_MODE_L3 == p_db_if->mode)
        {
            sal_memcpy(arp.ifname, p_db_if->key.name, sizeof(arp.ifname));
        }

        /* modified by wangjj for fix bug 42288, 2017-01-09 */    
        if (FALSE == is_router)
        {
            CTCLIB_SLIST_LOOP(p_rt_if->ifc_ipv4.obj_list, p_tmp_conn, listnode)
            {
                if (p_tmp_conn->key.address.u.prefix4.s_addr == ip->s_addr)
                {
                    ifindex = GLB_INVALID_IFINDEX;
                    sal_memset(arp.ifname, 0, sizeof(arp.ifname));
                }
            }
        }      
    }

    if (FALSE == is_router)
    {        
        /* eventhough the static arp match non interfce, it can existed */
        if (p_db_if)
        {
            if (arp_api_get_rt_ifc(p_db_if, ip, &ifc) || !ifc)
            {
                return PM_E_FAIL;
            }
        }

         /* Filter subnet broadcast and network address */
        if (ifc)
        {
            addr_mask_len2ip_v4(ifc->key.address.prefixlen, &mask_addr);
            host_addr.s_addr = ip->s_addr & ~(mask_addr.s_addr);
            if ( host_addr.s_addr == ~(mask_addr.s_addr) || host_addr.s_addr == 0)
            {
                ifindex = GLB_INVALID_IFINDEX;
                sal_memset(arp.ifname, 0, sizeof(arp.ifname));
            }

            if (ip->s_addr == ifc->key.address.u.prefix4.s_addr)
            {
                ifindex = GLB_INVALID_IFINDEX;
                sal_memset(arp.ifname, 0, sizeof(arp.ifname));
            }
        }

        /*get the current number of arp if ip arp limit is enable*/
        if (NULL != p_db_if)
        {
            sal_memset(&route_if_key, 0, sizeof(route_if_key));
            sal_memcpy(route_if_key.name, p_db_if->key.name, sizeof(route_if_key.name));
            p_route_if = tbl_route_if_get_route_if(&route_if_key);
            if (p_route_if && p_db_if->arp_numberlimit_enable)
            {
                if (type == ARP_TYPE_DYNAMIC)
                {
                    p_route_if->arp_curr_dynamic++;
                }
                if (p_route_if->arp_curr_dynamic > p_db_if->arp_numberlimit_number)
                {
                    p_route_if->arp_curr_dynamic--;
                    p_route_if->arp_attack_number++;
                    return PM_E_EXCEED_ARP_LIMIT_MAX_NUM;
                }
            }
        }
    }

    sal_memcpy(&arp.key.ip, ip, sizeof(addr_ipv4_t));
    arp.ifindex = ifindex;
    arp.type = type;
    arp.is_router = is_router;    
    arp.is_proxy = is_proxy;
    arp.vrf_id = GLB_DEF_VRFID;
    if (is_router)
    {
        arp.status = AS_REACHABLE;
    }
    if (ll_addr)
    {
        sal_memcpy(arp.mac_addr, ll_addr, MAC_ADDR_LEN);
    }

    arp.uptime = ctc_time_boottime_sec(NULL);
    rc = tbl_arp_add_arp(&arp);
    (void)rc;

    //rt_api_del_static_route_from_frr_when_add_arp(&arp);
    
    if (!is_router)    
    {
        arp_db_count_inc(&arp);
    }

    /* debug */
    arp_debug_entry_trace(&arp, ARP_ENTRY_ADD);

    *entry = NULL;
    *entry = tbl_arp_get_arp(&arp.key);

    return PM_E_NONE;
}

int32
arp_db_entry_remove(tbl_arp_t *p_db_arp)
{
    tbl_route_if_t* p_route_if = NULL;
    tbl_route_if_key_t route_if_key;
    tbl_interface_t*        p_db_if = NULL;
    tbl_interface_key_t     if_key;

    tbl_arp_t *p_db_arp_tmp = NULL;

    p_db_arp_tmp = tbl_arp_get_arp(&p_db_arp->key);
    
    /* debug */
    arp_debug_entry_trace(p_db_arp, ARP_ENTRY_DEL);
    
    /* router arp should not be counted */
    if (!p_db_arp->is_router)
    {
        /*route if*/
        sal_memset(&route_if_key, 0, sizeof(route_if_key));
        sal_memcpy(route_if_key.name, p_db_arp->ifname, sizeof(route_if_key.name));
        p_route_if = tbl_route_if_get_route_if(&route_if_key);
        if (p_route_if)
        {
            sal_memcpy(if_key.name, p_db_arp->ifname, sizeof(if_key.name));
            p_db_if = tbl_interface_get_interface(&if_key);
            if ((p_db_if != NULL) && p_db_if->arp_numberlimit_enable)
            {
                if (p_db_arp->type == ARP_TYPE_DYNAMIC)
                {
                    if (p_route_if->arp_curr_dynamic)
                    {
                        p_route_if->arp_curr_dynamic--;
                    }
                }
            }
        }
        /*modified by chenc for bug 51816, 2019-04-28*/
        if (p_db_arp_tmp) 
        {
            arp_db_count_dec(p_db_arp);
        }
    }
    
    /* stop the timer */
    if (p_db_arp->refresh_timer)
    {
        ctc_task_delete_timer(p_db_arp->refresh_timer);
        p_db_arp->refresh_timer = NULL;
    }
    if (p_db_arp->retry_timer)
    {
        ctc_task_delete_timer(p_db_arp->retry_timer);
        p_db_arp->retry_timer = NULL;
    }
    if (p_db_arp->gra_rsp_timer)
    {
        ctc_task_delete_timer(p_db_arp->gra_rsp_timer);
        p_db_arp->gra_rsp_timer = NULL;
    }

    //rt_api_add_static_route_to_frr_when_del_arp(p_db_arp);
    return tbl_arp_del_arp(&p_db_arp->key);
}

tbl_arp_t*
arp_db_entry_lookup(addr_ipv4_t *ip)
{
    tbl_arp_key_t key;
    key.ip = *ip;
    return tbl_arp_get_arp(&key);
}

int32
arp_api_connected_add(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, uint32 add_not_up, 
    uint32 new_up_running, 
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    mac_addr_t vmac)
    /* liwh end */
{
    tbl_arp_t           *p_db_arp       = NULL;
    tbl_interface_t     *pst_interface  = NULL;
    tbl_interface_key_t p_if_key;
    tbl_iter_args_t     args;
    addr_ipv4_t         ip_addr; 
    tbl_arp_t           arp_entry;
    uint32              ref_cnt     = 0;
    int32               rc          = PM_E_NONE;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    mac_addr_t          vmac_zero;
    /* liwh end */

    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&arp_entry, 0, sizeof(tbl_arp_t));
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    sal_memset(vmac_zero, 0, sizeof(mac_addr_t));
    /* liwh end */
    
    if (NULL == p_db_rtif || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    if (ip_addr.s_addr != p_conn->key.address.u.prefix4.s_addr)
    {
        sal_memcpy(&ip_addr, &p_conn->key.address.u.prefix4.s_addr, sizeof(addr_ipv4_t));
        sal_memset(&p_if_key, 0, sizeof(p_if_key));
        sal_strcpy(p_if_key.name, p_db_if->key.name);
        pst_interface = tbl_interface_get_interface(&p_if_key);
        if (NULL != pst_interface)
        {
            sal_memset(&args, 0, sizeof(args));
            args.argv[0] = p_conn;
            args.argv[1] = &add_not_up;
            args.argv[2] = p_db_if;
            tbl_arp_iterate((TBL_ITER_CB_FUNC)rt_fea_arp_add_static_by_loop, &args);
        }
    }
    else
    {
        return PM_E_NONE;
    }
    
    if (new_up_running)
    {
        /* delete dynamic arp whose address is same as ifc */
        p_db_arp = arp_db_entry_lookup(&p_conn->key.address.u.prefix4);
        if (NULL != p_db_arp && ((ARP_TYPE_DYNAMIC == p_db_arp->type)
            /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
            || GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL)))
            /* liwh end */
        {
            /* modified by liwh for bug 51067, 2019-02-20 */
            if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL))
            {
                arp_db_synch_hw_os(p_db_arp, ARP_SYN_DEL); 
            }
            /* liwh end */
            
            ref_cnt = p_db_arp->route_ref;
            rc = arp_db_entry_remove(p_db_arp);
            p_db_arp = NULL;
        }
        
        if (NULL == p_db_arp)
        {
            /* create router arp */
            if (!GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL)
                /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
                || (0 == sal_memcmp(vmac_zero, vmac, GLB_ETH_ADDR_LEN)))
                /* liwh end */
            {
                rc = arp_db_entry_create(p_db_if, &p_conn->key.address.u.prefix4, p_db_if->mac_addr, 
                        ARP_TYPE_STATIC, FALSE, TRUE, &p_db_arp);
            }
            /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
            else
            {
                rc = arp_db_entry_create(p_db_if, &p_conn->key.address.u.prefix4, vmac, 
                        ARP_TYPE_STATIC, FALSE, TRUE, &p_db_arp);
            }
            /* liwh end */

            p_db_arp->status = AS_REACHABLE;
            p_db_arp->is_router = TRUE;
            p_db_arp->route_ref = ref_cnt;
       }
    }

    /* update arp cache */
    arp_api_trav(_arp_api_ifc_add, (void*)p_conn, (void*)p_db_if);

    /* trigger gratuitous arp */
    rc = _arp_api_gra_trigger(p_db_if, p_db_rtif, p_conn);
    (void)rc;
    
    return PM_E_NONE;
}

int32
arp_api_connected_del(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    if (NULL == p_db_rtif || NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    /* since arp whoes address same as ifc if no ifindex asssigned, so trav all the cache */
    arp_api_trav(_arp_api_ifc_del, (void*)p_conn, (void*)p_db_if);

    return PM_E_NONE;
}

int32
arp_api_connected_up(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, uint32 new_up_running)
{
    tbl_interface_t     *pst_interface = NULL;
    tbl_interface_key_t p_if_key ;
    tbl_iter_args_t     args;
    tbl_arp_t       *p_db_arp = NULL;
    tbl_arp_t       arp_entry;
    addr_ipv4_t     ip_addr; 
    uint32          ref_cnt = 0;
    uint32          add_not_up = 0;
    int32           rc = PM_E_NONE;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    mac_addr_t      vmac;
    /* liwh end */
    
    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&arp_entry, 0, sizeof(tbl_arp_t));
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    sal_memset(&vmac, 0, sizeof(mac_addr_t));
    /* liwh end */
    
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       add vmac by liwh */
    rc = arp_api_connected_add(p_db_if, p_db_rtif, p_conn, 0, new_up_running, vmac);
    /* liwh end */
    if (INTERFACE_IS_LOOPBACK(p_db_if))
    {
        return PM_E_INVALID_PARAM;
    }

    if (ip_addr.s_addr != p_conn->key.address.u.prefix4.s_addr)
    {
        sal_memcpy(&ip_addr, &p_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memset(&p_if_key, 0, sizeof(p_if_key));
        sal_strcpy(p_if_key.name, p_db_rtif->key.name);

        pst_interface = tbl_interface_get_interface(&p_if_key);
        if (NULL != pst_interface)
        {
            sal_memset(&args, 0, sizeof(args));
            args.argv[0] = p_conn;
            args.argv[1] = &add_not_up;
            args.argv[2] = p_db_if;
            tbl_arp_iterate((TBL_ITER_CB_FUNC)rt_fea_arp_add_static_by_loop, &args);
        }
        
        return PM_E_NONE;
    }
    
    /* should check running beside this function */
    if (0)
    {
        p_db_arp = arp_db_entry_lookup(&ip_addr);
        if (p_db_arp && ARP_TYPE_DYNAMIC == p_db_arp->type)
        {
            ref_cnt = p_db_arp->route_ref;
            arp_db_entry_remove(p_db_arp);
            p_db_arp = NULL;
        }
        
        if (NULL == p_db_arp)
        {
            PM_E_RETURN(arp_db_entry_create(p_db_if, &ip_addr, p_db_if->mac_addr, 
                        ARP_TYPE_STATIC, FALSE, TRUE, &p_db_arp));
            if (NULL != p_db_arp)
            {
                p_db_arp->status = AS_REACHABLE;
                p_db_arp->route_ref = ref_cnt;
            }
        }
    }

    /* update arp cache */
    arp_api_trav_ifindex(p_db_if->ifindex, _arp_api_ifc_up, (void*)p_conn, (void*)p_db_if);

    /* trigger gratuitous arp */
    //rc = _arp_api_gra_trigger(p_db_if, pst_kernel_if);

    return rc;
}

int32
arp_api_connected_down(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn)
{
    tbl_arp_t       *p_db_arp = NULL;
    tbl_arp_key_t   key;
    int32           rc = PM_E_NONE;
    
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    /* clear all dynamic arp entries */
    rc = arp_api_trav_ifindex(p_db_if->ifindex, _arp_api_ifc_down, (void*)p_conn, (void*)p_db_if);

    sal_memset(&key, 0, sizeof(key));
    sal_memcpy(&key.ip, &p_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
    p_db_arp = tbl_arp_get_arp(&key);
    if (NULL != p_db_arp && p_db_arp->is_router)
    {
        if (IPV4_MAX_PREFIXLEN != p_conn->key.address.prefixlen)
        {
            p_db_arp->type = ARP_TYPE_DYNAMIC;
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_TYPE);

            /* remove router arp */
            arp_sm(p_db_arp, AE_DELETE_ENTRY);
        }
    }

    return rc;
}

int32
arp_api_add_static_arp(tbl_arp_t *p_arp, tbl_interface_t *p_db_if)
{
    tbl_arp_t   *p_db_arp = NULL;
    int32       rc = PM_E_NONE;

    rc = arp_db_entry_create(p_db_if, &p_arp->key.ip, p_arp->mac_addr, 
                    ARP_TYPE_STATIC, FALSE, p_arp->is_router, &p_db_arp);
    if (0 == rc && p_db_arp)
    {
        if (GLB_INVALID_IFINDEX != p_db_arp->ifindex)
        {
            if (NULL != p_db_if && tbl_interface_is_running(p_db_if))
            {
                arp_db_synch_hw_os(p_db_arp, ARP_SYN_ADD);
            }
        }
        
        p_db_arp->status = AS_REACHABLE;
        p_db_arp->route_ref = p_arp->route_ref;
    }

    return rc;
}

int32
arp_api_del_static_arp(tbl_arp_t *p_arp)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_arp_t       *p_db_arp = NULL;
    int32           rc = PM_E_NONE;

    p_db_arp = arp_db_entry_lookup(&p_arp->key.ip);

    if (NULL != p_db_arp && (ARP_TYPE_STATIC == p_db_arp->type || ARP_TYPE_DYNAMIC == p_db_arp->type))
    {
        if (TRUE == p_db_arp->is_router)
        {
            return PM_E_CFG_CONFLICT;
        }

        if (GLB_INVALID_IFINDEX != p_db_arp->ifindex)
        {
            rc = arp_db_synch_hw_os(p_db_arp, ARP_SYN_DEL);
        }
        else
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_arp->ifname);
            if (NULL != p_db_if && GLB_IF_TYPE_LOOPBACK_IF != p_db_if->hw_type)
            {
                p_db_arp->ifindex = p_db_if->ifindex;
                p_db_arp->is_router = TRUE;
                sal_memcpy(p_db_arp->mac_addr, p_db_if->mac_addr, MAC_ADDR_LEN);
                tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_MAC_ADDR);
                return PM_E_NONE;
            }
        }

        if (p_db_arp->route_ref)
        {
            p_db_arp->type = ARP_TYPE_DYNAMIC;
            p_db_arp->status = AS_INCOMPLETE;
            p_db_arp->uptime = ctc_time_boottime_sec(NULL);
            p_db_arp->flags = 0;
            sal_memset(p_db_arp->mac_addr, 0, MAC_ADDR_LEN);

            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_TYPE);
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_STATUS);
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_FLAGS);
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_UPTIME);
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_MAC_ADDR);
            arp_sm(p_db_arp, AE_RETRY_EXP);
        }
        else
        {
            p_db_arp->type = ARP_TYPE_DYNAMIC;
            tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_TYPE);
            arp_sm(p_db_arp, AE_DELETE_ENTRY);
        }
    }
    
    return rc;
}

int32
arp_api_update_static_arp(tbl_arp_t *p_db_arp, tbl_arp_t *p_arp, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    p_arp->status = AS_REACHABLE;
    p_arp->type = ARP_TYPE_STATIC;
    
    if ((0 == sal_memcmp(p_db_arp->mac_addr, p_arp->mac_addr, MAC_ADDR_LEN))
             && p_db_arp->ifindex == p_arp->ifindex)
    {
        /* same entry, stop timer */
        if (p_db_arp->retry_timer)
        {
            ctc_task_delete_timer(p_db_arp->retry_timer);
            p_db_arp->retry_timer = NULL;
        }
        if (p_db_arp->refresh_timer)
        {
            ctc_task_delete_timer(p_db_arp->refresh_timer);
            p_db_arp->refresh_timer = NULL;
        }
        if (p_db_arp->gra_rsp_timer)
        {
            ctc_task_delete_timer(p_db_arp->gra_rsp_timer);
            p_db_arp->gra_rsp_timer = NULL;
        }
        
        tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_TYPE);
        tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_IS_ROUTER);
        tbl_arp_set_arp_field(p_arp, TBL_ARP_FLD_STATUS);

        return PM_E_NONE;
    }
    else
    {
        /* not same entry, remove old entry */
        p_arp->route_ref = p_db_arp->route_ref;
        p_db_arp->route_ref = 0;
        p_db_arp->status = AS_INIT;
        arp_sm(p_db_arp, AE_DELETE_ENTRY);
    }

    rc = arp_api_add_static_arp(p_arp, p_db_if);

    return rc;
}

int32
arp_api_miss_trigger(addr_ipv4_t *ip_addr, uint32 kernel_ifindex)
{
    tbl_interface_t     *p_db_if = NULL;
    tbl_kernel_if_t     *pst_kernel_if = NULL;
    tbl_kernel_if_key_t kernel_if_key;
    addr_ipv4_t         mask_addr;
    addr_ipv4_t         host_addr;
    int32               is_ifaddr = FALSE;
    int32               rc = PM_E_NONE;

    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));
    kernel_if_key.ifindex = kernel_ifindex;
    pst_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }

    if (0 == sal_memcmp(&pst_kernel_if->ip_addr, ip_addr, sizeof(addr_ipv4_t)))
    {
        return PM_E_NOT_EXIST;
    }

    /* Filter subnet broadcast and network address */
    addr_mask_len2ip_v4(pst_kernel_if->masklen, &mask_addr);
    host_addr.s_addr = ip_addr->s_addr & ~(mask_addr.s_addr);
    if (host_addr.s_addr == ~(mask_addr.s_addr) || host_addr.s_addr == 0)
    {
        return PM_E_NONE;
    }

    p_db_if = tbl_interface_get_interface_by_name(pst_kernel_if->name);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    is_ifaddr = arp_api_check_prefix_same_with_ifaddr(pst_kernel_if->name, ip_addr);
    if (tbl_interface_is_running(p_db_if) && !is_ifaddr)
    {
        /* do request */
        rc = _arp_api_trigger_request(ip_addr, p_db_if);
    }

    return rc;
}

int32
arp_api_check_is_routemac(tbl_interface_t *p_db_if, uint8 *mac, int32 *is_routemac)
{
    tbl_sys_global_t         *p_sys_glb = tbl_sys_global_get_sys_global();
    /* support vrrp modified by liwh for bug 45215, 2017-09-26 */
    tbl_vrrp_vmac_t          vrrp_vmac;
    tbl_vrrp_vmac_t          *p_db_vrrp_vmac = NULL;
    /* liwh end */

    sal_memset(&vrrp_vmac, 0, sizeof(tbl_vrrp_vmac_t));

    *is_routemac = FALSE;

    if (NULL == mac || NULL == p_sys_glb)
    {
        return PM_E_INVALID_PARAM;
    } 

    if (0 == sal_memcmp(mac, p_sys_glb->route_mac, MAC_ADDR_LEN))
    {
        *is_routemac = TRUE;
        return PM_E_NONE; 
    }

    /* support vrrp modified by liwh for bug 45215, 2017-09-26 */
    /* check whether the mac is vrrp mac*/
    sal_memcpy(vrrp_vmac.key.mac, mac, GLB_ETH_ADDR_LEN);
    p_db_vrrp_vmac = tbl_vrrp_vmac_get_vrrp_vmac(&vrrp_vmac.key);
    if (p_db_vrrp_vmac)
    {
        *is_routemac = TRUE;
        return PM_E_NONE;    
    }
    /* liwh end */

    return PM_E_NONE;
}

tbl_kernel_if_t *
arp_api_check_addr_with_kernel_if_exist(addr_ipv4_t *addr_ip)
{
    ctclib_slistnode_t      *listnode    = NULL;    
    tbl_kernel_if_t         *p_db_kernel_if = NULL;
    addr_ipv4_t             ip_addr_check;
    addr_ipv4_t             ip_addr_check2;
    uint32                  mask = 0;
    tbl_kernel_if_master_t  *p_master = tbl_kernel_if_get_master();

    sal_memset(&ip_addr_check, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_check2, 0, sizeof(addr_ipv4_t));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            IPV4_LEN_TO_MASK(mask, p_db_kernel_if->masklen);
            ip_addr_check.s_addr = ip_addr_check.s_addr & mask;

            ip_addr_check2.s_addr = sal_htonl(addr_ip->s_addr);
            ip_addr_check2.s_addr = ip_addr_check2.s_addr & mask;
            if (ip_addr_check.s_addr && (0 == sal_memcmp(&ip_addr_check, &ip_addr_check2, sizeof(addr_ipv4_t))))
            {
                return p_db_kernel_if;
            }
        }
    }

    return NULL;
}

int32
arp_api_check_prefix_same_with_ifaddr(char *ifname, addr_ipv4_t *prefix)
{
    ctclib_slistnode_t  *listnode  = NULL;
    ds_connected_t      *p_db_conn = NULL;
    prefix_t            *p = NULL;
    tbl_route_if_t      *p_db_rt_if = NULL;
    tbl_route_if_key_t  rtif_key;

    if (NULL == ifname || NULL == prefix)
    {
        return FALSE;
    }

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, ifname, sizeof(rtif_key.name));
    p_db_rt_if = tbl_route_if_get_route_if(&rtif_key);
    if (NULL == p_db_rt_if)
    {
        return FALSE;
    }

    CTCLIB_SLIST_LOOP(p_db_rt_if->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (0 == sal_memcmp(&p->u.prefix4, prefix, sizeof(addr_ipv4_t)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

tbl_route_if_t *
arp_api_check_addr_with_rt_if_exist(tbl_interface_t *p_db_if, addr_ipv4_t *addr_ip)
{
    ctclib_slistnode_t  *listnode  = NULL;
    ds_connected_t      *p_db_conn = NULL;
    tbl_route_if_t      *p_db_rtif = NULL;
    prefix_t            *p = NULL;
    prefix_t            q;
   
    if (NULL == p_db_if)
    {
        return NULL;
    }

    sal_memset(&q, 0, sizeof(q));
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return NULL;
    }
    
    sal_memset(&q, 0, sizeof q);
  
    q.family = AF_INET;
    q.u.prefix4 = *addr_ip;
    q.prefixlen = IPV4_MAX_BITLEN;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (prefix_compare_net(p, &q)) 
        {
            return p_db_rtif;
        }
    }

    return NULL;
}

tbl_route_if_t *
arp_api_check_addr_with_rt_if_exist_advance(addr_ipv4_t *addr_ip)
{
    ctclib_slistnode_t  *listnode  = NULL;
    ctclib_slistnode_t  *listnode_sub  = NULL;
    ds_connected_t      *p_db_conn = NULL;
    tbl_route_if_t      *p_db_rtif = NULL;
    prefix_t            *p = NULL;
    prefix_t            q;
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();

    if (NULL == p_master)
    {
        SAL_ASSERT(0);
    }
    
    sal_memset(&q, 0, sizeof(q));
    
    q.family = AF_INET;
    q.u.prefix4 = *addr_ip;
    q.prefixlen = IPV4_MAX_BITLEN;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
    {
        if (NULL != p_db_rtif)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode_sub)
            {
                p = &p_db_conn->key.address;
                if (prefix_compare_net(p, &q)) 
                {
                    return p_db_rtif;
                }
            }
        }
    }

    return NULL;
}

int32
arp_api_get_kernel_if_by_tip(tbl_interface_t *p_db_if, addr_ipv4_t *tar_ip, tbl_kernel_if_t **pst_kernel_if)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = tbl_kernel_if_get_master();
    addr_ipv4_t ip_addr_check;
    addr_ipv4_t ip_addr_check2;    

    if (NULL == p_db_if || NULL == p_master)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&ip_addr_check, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_check2, 0, sizeof(addr_ipv4_t));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            ip_addr_check2.s_addr = sal_htonl(tar_ip->s_addr);
            if (ip_addr_check.s_addr && (ip_addr_check2.s_addr == ip_addr_check.s_addr))
            {
                *pst_kernel_if = p_db_kernel_if;
                return PM_E_NONE;
            }                
        }
    }

    return PM_E_NOT_EXIST;
}

int32
arp_api_get_rt_if_by_tip(tbl_interface_t *p_db_if, addr_ipv4_t *tar_ip, ds_connected_t **pp_conn)
{
    ctclib_slistnode_t  *listnode    = NULL;
    ds_connected_t      *p_db_conn = NULL;
    tbl_route_if_t      *p_db_rtif = NULL;
    prefix_t            *p = NULL;
    prefix_t            q;
   
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(&q, 0, sizeof(q));
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memset(&q, 0, sizeof q);
  
    q.family = AF_INET;
    q.u.prefix4 = *tar_ip;
    q.prefixlen = IPV4_MAX_BITLEN;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (prefix_compare_net(p, &q)) 
        {
            *pp_conn = p_db_conn;
            return PM_E_NONE;
        }
    }

    return PM_E_NOT_EXIST;
}

int32
arp_api_get_ifc(tbl_interface_t *p_db_if, addr_ipv4_t *addr, tbl_kernel_if_t **pst_kernel_if)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = tbl_kernel_if_get_master();
    addr_ipv4_t ip_addr_check;
    addr_ipv4_t ip_addr_check2;    
    uint32      mask = 0;

    if (NULL == p_db_if || NULL == p_master || NULL == addr)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&ip_addr_check, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_check2, 0, sizeof(addr_ipv4_t));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            IPV4_LEN_TO_MASK(mask, p_db_kernel_if->masklen);
            ip_addr_check.s_addr = ip_addr_check.s_addr & mask;
            ip_addr_check2.s_addr = sal_htonl(addr->s_addr);
            ip_addr_check2.s_addr = ip_addr_check2.s_addr & mask;
            if (ip_addr_check.s_addr && (ip_addr_check2.s_addr == ip_addr_check.s_addr))
            {
                *pst_kernel_if = p_db_kernel_if;
                return PM_E_NONE;
            }                
        }
    }

    return PM_E_FAIL;
}

int32
arp_api_get_rt_ifc(tbl_interface_t *p_db_if, addr_ipv4_t *addr, ds_connected_t **pp_conn)
{
    prefix_t *p = NULL;
    prefix_t q;
    ctclib_slistnode_t *listnode = NULL;
    tbl_route_if_t  *p_db_rtif = NULL;
    ds_connected_t  *p_db_conn = NULL;
    uint8           best_prefixlen = 0;

    *pp_conn = NULL;

    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memset(&q, 0, sizeof(q));
    best_prefixlen = 0;
  
    q.family = AF_INET;
    q.u.prefix4 = *addr;
    q.prefixlen = IPV4_MAX_BITLEN;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (prefix_compare_net(p, &q) && p->prefixlen > best_prefixlen) 
        {           
            best_prefixlen = p->prefixlen;
            *pp_conn = p_db_conn;
            return PM_E_NONE;
        }
    }

    return PM_E_FAIL;
}

int32
arp_api_get_ifc_for_shortest_prefix(tbl_interface_t *p_db_if, addr_ipv4_t *addr, tbl_kernel_if_t *pst_kernel_if)
{
    ctclib_slistnode_t  *listnode    = NULL;    
    tbl_kernel_if_t     *p_db_kernel_if = NULL;
    addr_ipv4_t         ip_addr_check;
    addr_ipv4_t         ip_addr_check2;    
    uint32              best_prefixlen = 0;    
    tbl_kernel_if_master_t *pst_master = tbl_kernel_if_get_master();

    sal_memset(&ip_addr_check, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_check2, 0, sizeof(addr_ipv4_t));  
    
    sal_memset(&ip_addr_check, 0, sizeof(ip_addr_check));
    CTCLIB_SLIST_LOOP(pst_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            ip_addr_check2.s_addr = sal_htonl(addr->s_addr);
            if (ip_addr_check.s_addr && (p_db_kernel_if->masklen < best_prefixlen) 
                        && (0 == sal_memcmp(&ip_addr_check, &ip_addr_check2, sizeof(addr_ipv4_t))))
            {
                best_prefixlen = p_db_kernel_if->masklen;
                pst_kernel_if = p_db_kernel_if;

                return PM_E_NONE;
            }                
        }
    }

    return PM_E_NOT_EXIST;
}

int32
arp_api_get_rt_ifc_for_shortest_prefix(tbl_interface_t *p_db_if, addr_ipv4_t *addr, ds_connected_t **pp_conn)
{
    prefix_t *p = NULL;
    prefix_t q;
    uint8 best_prefixlen = 0;
    ctclib_slistnode_t *listnode = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;

    *pp_conn = NULL;
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(&q, 0, sizeof q);
    best_prefixlen = IPV4_MAX_BITLEN + 1;
  
    q.family = AF_INET;
    q.u.prefix4 = *addr;
    q.prefixlen = IPV4_MAX_BITLEN;

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (prefix_compare_net(p, &q) && p->prefixlen < best_prefixlen) 
        {           
            best_prefixlen = p->prefixlen;
            *pp_conn = p_db_conn;
            return PM_E_NONE;
        }
    }

    return PM_E_NOT_EXIST;
}

/***************************************************************************************************
 * Name         : rt_if_lookup_ifc_ipv4
 * Purpose      : find the conneted addr of related route_if by the prefix and interface           
 * Input        : interface, addr
 * Output       : N/A                  
 * Return       : ds_connected
 * Note         : N/A
***************************************************************************************************/
ds_connected_t*
rt_if_lookup_ifc_ipv4(tbl_interface_t *p_db_if, addr_ipv4_t *addr)
{
    tbl_route_if_t      *p_db_rtif = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *ifc = NULL;

    if (NULL == p_db_if || NULL == addr)
    {
        return NULL;
    }
    
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)(&p_db_if->key));
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc, listnode)
    {
        if (ifc->key.address.u.prefix4.s_addr == addr->s_addr)
        {
            return ifc;
        }
    }

    return NULL;
}

int32
arp_api_fib_match(prefix_t *p, tbl_iproute_node_t *pst_iproute_node)
{
    int32       is_match = FALSE;
    prefix_t    q;

    sal_memset(&q, 0, sizeof(prefix_t));
    if (NULL == pst_iproute_node || NULL == p)
    {
        return is_match;
    }

    if (AF_INET == p->family)
    {
        q.prefixlen = pst_iproute_node->key.p.prefixlen;
        q.family = AF_INET;
        sal_memcpy(&q.u.prefix4, &pst_iproute_node->key.p.u.prefix4, sizeof(addr_ipv4_t));
        ip_apply_mask(&q);
        if (prefix_compare_net(&q, p))
        {
            is_match = TRUE;
        }
    }
    else
    {
        /* TO support for IPv6 */
        if (0 == sal_memcmp(&pst_iproute_node->key.p.u.prefix6, &p->u.prefix6, sizeof(addr_ipv6_t)))
        {
            is_match = TRUE;
        }
    }

    return is_match;
}

int32
arp_api_del_arp_by_mirror(tbl_arp_t *ae_p, void *p_data, void *p_data1)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data1;
    tbl_route_if_t  *pst_rt_if = (tbl_route_if_t*)p_data;
    addr_ipv4_t     ip_addr;

    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));

    if (NULL == pst_rt_if || NULL == p_db_if || NULL == ae_p)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    if (!GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        return PM_E_FAIL;
    }

    if (0 != sal_strncmp(ae_p->ifname, pst_rt_if->key.name, sizeof(ae_p->ifname)))
    {
        return PM_E_NONE;
    }

    if (ARP_TYPE_STATIC == ae_p->type && TRUE == ae_p->is_router)
    {
        arp_sm(ae_p, AE_DELETE_ENTRY);

        return PM_E_NONE;
    }
    else if (ARP_TYPE_STATIC == ae_p->type)
    {
        /* set invalid ifindex */
        ae_p->ifindex = GLB_INVALID_IFINDEX;
        tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFINDEX);
        sal_memset(ae_p->ifname, 0, sizeof(ae_p->ifname));
        tbl_arp_set_arp_field(ae_p, TBL_ARP_FLD_IFNAME);

        arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);
        return PM_E_NONE;
    }

    arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);
    arp_sm(ae_p, AE_DELETE_ENTRY);

    return PM_E_NONE;
}

int32
_arp_api_if_set_mirror_update(tbl_interface_t *pst_interface, char *ifname)
{
    tbl_route_if_t      *p_db_route_if = NULL;
    tbl_route_if_key_t  rtif_key;

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, ifname, sizeof(rtif_key.name));
    p_db_route_if = tbl_route_if_get_route_if(&rtif_key);
    if (NULL == p_db_route_if)
    {
        return PM_E_FAIL;
    }

    /* 3. update cache entries */
    arp_api_trav(arp_api_del_arp_by_mirror, (void*)p_db_route_if, (void*)pst_interface);

    return PM_E_NONE;
}

int32
arp_api_if_set_mirror_update(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
        uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_t     *pst_interface = NULL;
    tbl_interface_t     *p_db_interface = NULL;
    int32               rc = PM_E_NONE;
    
    if (NULL == p_tbl)
    {
        return PM_E_FAIL; 
    }

    /* process tbl*/  
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        pst_interface = (tbl_interface_t*)p_tbl;
        p_db_interface = tbl_interface_get_interface(&pst_interface->key);
        if (NULL == p_db_interface)
        {
            return PM_E_FAIL;
        }
        
        switch (field_id)
        {
            case TBL_INTERFACE_FLD_MIRROR_ENABLE:
                break;

            case TBL_INTERFACE_FLD_MIRROR_DEST_ID:
                rc = _arp_api_if_set_mirror_update(pst_interface, p_db_interface->key.name);
                break;
                
            default:
                break;
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    (void)rc;
    return PM_E_NONE;
}

int32
_rt_if_set_arp_ageing_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout)
{
    int32           interval = 0;
    uint32          ifindex = 0;
    
    /* 1. check duplicate config */
    if (p_db_rtif->arp_timeout == timeout)
    {
        return PM_E_NONE;
    }
    
    /* 2. process */
    interval = p_db_rtif->arp_timeout - timeout;
    ifindex = tbl_interface_get_ifindex_by_name(p_db_rtif->key.name);

    /* 3. update cache entries */
    arp_api_trav_ifindex(ifindex, arp_api_aging_time_update, (void*)&interval, NULL);

    return PM_E_NONE;
}

int32
rt_if_set_arp_ageing_timeout(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
        uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_route_if_t  *pst_rif= NULL;
    tbl_route_if_t  *p_db_rif= NULL;
    int32           rc = PM_E_NONE;
    
    if (NULL == p_tbl)
    {
        return PM_E_FAIL; 
    }

    /* process tbl*/  
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        pst_rif = (tbl_route_if_t*)p_tbl;
        p_db_rif = tbl_route_if_get_route_if(&pst_rif->key);
        if (NULL == p_db_rif)
        {
            return PM_E_FAIL;
        }
        
        switch (field_id)
        {
            case TBL_ROUTE_IF_FLD_MTU:
                break;

            case TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME:
                break;

            case TBL_ROUTE_IF_FLD_ARP_TIMEOUT:
                rc = _rt_if_set_arp_ageing_timeout(p_db_rif, pst_rif->arp_timeout);
                break;
                
            default:
                break;
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    (void)rc;
    return PM_E_NONE;
}

int32
arp_api_aging_time_update(tbl_arp_t *p_db_arp, void *p_data, void *p_data1)
{
    int32 interval = *((int32*)p_data);
    int32 time = 0;

    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
        return PM_E_NONE;
    }
    if (AS_REACHABLE != p_db_arp->status)
    {
        return PM_E_NONE;
    }
    
    time = p_db_arp->aging_delay - interval;
    p_db_arp->aging_delay = time > 0 ? time : 0;

    tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_AGING_DELAY);

    return PM_E_NONE;
}

int32
arp_api_aging_time_init(tbl_arp_t *p_db_arp)
{
    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
        return PM_E_NONE;
    }
    
    if (AS_REACHABLE != p_db_arp->status)
    {
        return PM_E_NONE;
    }

    p_db_arp->aging_delay = 0;
    tbl_arp_set_arp_field(p_db_arp , TBL_ARP_FLD_AGING_DELAY);

    return PM_E_NONE;
}

int32
arp_api_route_add_nh_arp(tbl_nexthop_t *pst_nexthop)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_arp_t       *ae_p = NULL;
    int32           rc = PM_E_NONE;

    ae_p = arp_db_entry_lookup(&pst_nexthop->key.u.ipv4);
    if (ae_p != NULL)
    {
        ae_p->route_ref++; /*modified by ychen for bug 7351 for invalid read*/
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_name(pst_nexthop->key.ifname);
        rc = arp_db_entry_create(p_db_if, &pst_nexthop->key.u.ipv4, NULL, ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
        if (rc < 0)
        {
            return PM_E_NONE;
        }
        
        ae_p->route_ref++;
        ae_p->status = AS_INCOMPLETE;
        arp_sm(ae_p, AE_RETRY_EXP);
    }
    
    /*delete log by chenc for bug 45602*/
/*
    if (ae_p)
    {
        ARP_PROTO_DEBUG( "arp_api_route_add_nh, IP ip is %s, ref_cnt = %d",
            sal_inet_ntop(AF_INET, &ae_p->key.ip, addr, INET_ADDRSTRLEN), ae_p->route_ref);
    }
*/
    return PM_E_NONE;

}

int32
arp_api_route_del_nh_arp_by_nexthop_key(nexthop_key_t *pst_nexthop)
{
    tbl_arp_t   *ae_p = NULL;
    char        addr[INET_ADDRSTRLEN];

    sal_memset(addr, 0, sizeof(addr));
    ae_p = arp_db_entry_lookup(&pst_nexthop->u.ipv4);
    if (NULL == ae_p)
    {
        return PM_E_NONE;
    }
    
    /*delete log by chenc for bug 45602*/
/*
    ARP_PROTO_DEBUG("arp_api_route_del_nh, arp IP is %s, ref_cnt = %d", 
        sal_inet_ntop(AF_INET, &ae_p->key.ip, addr, INET_ADDRSTRLEN), ae_p->route_ref);
*/
    if (ae_p->route_ref)
    {
        ae_p->route_ref--;
#ifdef TODO     /* modified by huangxt for bug 14539, 2011-04-02 */
        if (ae_p->route_ref == 0 && FALSE == ae_p->ae_bfd_ref)
#else
        if (ae_p->route_ref == 0)
#endif /*_CENTEC_*/
        {
            if (ae_p->status == AS_INCOMPLETE)
            {
                ae_p->status = AS_INIT;
                
                /* Modified by Jeffrey for bug 4391 use boot time as update time */
                /* ae_p->ae_uptime = pal_time_current(NULL); */
                ae_p->uptime = ctc_time_boottime_sec(NULL);
                arp_sm(ae_p, AE_DELETE_ENTRY);
            }
        }
    }

    return PM_E_NONE;

}

int32
arp_api_route_del_nh_arp(tbl_nexthop_t *pst_nexthop)
{
    tbl_arp_t   *ae_p = NULL;
    char        addr[INET_ADDRSTRLEN];

    sal_memset(addr, 0, sizeof(addr));
    ae_p = arp_db_entry_lookup(&pst_nexthop->key.u.ipv4);
    if (NULL == ae_p)
    {
        return PM_E_NONE;
    }
    
    /*delete log by chenc for bug 45602*/
/*
    ARP_PROTO_DEBUG("arp_api_route_del_nh, arp IP is %s, ref_cnt = %d", 
        sal_inet_ntop(AF_INET, &ae_p->key.ip, addr, INET_ADDRSTRLEN), ae_p->route_ref);
*/
    if (ae_p->route_ref)
    {
        ae_p->route_ref--;
#ifdef TODO     /* modified by huangxt for bug 14539, 2011-04-02 */
        if (ae_p->route_ref == 0 && FALSE == ae_p->ae_bfd_ref)
#else
        if (ae_p->route_ref == 0)
#endif /*_CENTEC_*/
        {
            if (ae_p->status == AS_INCOMPLETE)
            {
                ae_p->status = AS_INIT;
                
                /* Modified by Jeffrey for bug 4391 use boot time as update time */
                /* ae_p->ae_uptime = pal_time_current(NULL); */
                ae_p->uptime = ctc_time_boottime_sec(NULL);
                arp_sm(ae_p, AE_DELETE_ENTRY);
            }
        }
    }

    return PM_E_NONE;

}

int32
arp_api_for_update_dynamic_tunnel(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    glb_openflow_dynamic_tunnel_msg_t *req = NULL;
    tbl_nexthop_t nexthop;
    
    req = (glb_openflow_dynamic_tunnel_msg_t *)p_msg->data;
    sal_memset(&nexthop, 0x0, sizeof(tbl_nexthop_t));

    sal_memcpy(&(nexthop.key.u.ipv4), &(req->ipv4), sizeof(req->ipv4));
    sal_strncpy(nexthop.key.ifname, req->ifname, GLB_IFNAME_SZ);
    
    if(req->is_add)
    {
        arp_api_route_add_nh_arp(&nexthop);
    }
    else
    {
        arp_api_route_del_nh_arp(&nexthop);
    }

    return 0;
}

int32
arp_api_route_add(tbl_nexthop_t *p_db_nh)
{
    if (NULL == p_db_nh)
    {
        return PM_E_FAIL;
    }

    arp_api_route_add_nh_arp(p_db_nh);

    return PM_E_NONE;
}

int32
arp_api_route_del(tbl_nexthop_t *p_db_nh)
{
    if (NULL == p_db_nh)
    {
        return PM_E_FAIL;
    }

    arp_api_route_del_nh_arp(p_db_nh);

    return PM_E_NONE; 
}

uint32
arp_api_entry_match(addr_ipv4_t *ip)
{
    tbl_arp_t *p_db_arp = NULL;
    
    p_db_arp = arp_db_entry_lookup(ip);
    /*modified by dingyi for bug 4239, 2008-02-01*/
    if (p_db_arp)
    {
        if (GLB_FLAG_ISSET(p_db_arp->flags, ARP_SYN_HW) || p_db_arp->is_router)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
arp_api_del_by_ifindex(tbl_arp_t *p_db_arp, void *p_data, void *p_data1)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data;

    if (p_db_if->ifindex != p_db_arp->ifindex)
    {
        return PM_E_NONE;
    }

    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
    }
    else
    {
        arp_db_synch_hw_os(p_db_arp, ARP_SYN_DEL);
        arp_sm(p_db_arp, AE_DELETE_ENTRY);
    }
    
    return PM_E_NONE;
}

tbl_kernel_if_t*
rt_if_match_ipv4_address(addr_ipv4_t *addr, vrf_id_t vrf_id)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;

    pst_kernel_if = tbl_kernel_if_check_by_list_loop(addr);

    return pst_kernel_if;
}

int32
arp_api_flush_statistics()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    p_rt_glb->arp_pkt_rx_count = 0;
    p_rt_glb->arp_pkt_tx_count = 0;
    p_rt_glb->arp_pkt_discard_count = 0;

    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT);
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT);
    tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT);

    return PM_E_NONE;
}

static int32
_arp_api_trav_ifindex(tbl_arp_t *p_db_arp, tbl_iter_args_t *pargs)
{
    ARP_ENTRY_ITER_FUNC fn = (ARP_ENTRY_ITER_FUNC)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];
    void *p_data = (void*)pargs->argv[2];
    void *p_data1 = (void*)pargs->argv[3];
    
    if (GLB_INVALID_IFINDEX == p_db_arp->ifindex)
    {
        return PM_E_NONE;
    }

    if (ifindex != p_db_arp->ifindex)
    {
        return PM_E_NONE;
    }

    fn(p_db_arp, p_data, p_data1);
    
    return PM_E_NONE;
}

static int32
_arp_api_trav(tbl_arp_t *p_db_arp, tbl_iter_args_t *pargs)
{
    ARP_ENTRY_ITER_FUNC fn = (ARP_ENTRY_ITER_FUNC)pargs->argv[0];
    void *p_data = (void*)pargs->argv[2];
    void *p_data1 = (void*)pargs->argv[3];

    fn(p_db_arp, p_data, p_data1);
    
    return PM_E_NONE;
}

int32
arp_api_trav_ifindex(uint32 ifindex, ARP_ENTRY_ITER_FUNC func, void *p_data, void *p_data1)
{
    tbl_iter_args_t args;

    args.argv[0] = func;
    args.argv[1] = &ifindex;
    args.argv[2] = p_data;
    args.argv[3] = p_data1;

    tbl_arp_iterate((TBL_ITER_CB_FUNC)_arp_api_trav_ifindex, &args);

    return PM_E_NONE;
}

int32
arp_api_trav(ARP_ENTRY_ITER_FUNC func, void *p_data, void *p_data1)
{
    tbl_iter_args_t args;

    args.argv[0] = func;
    args.argv[2] = p_data;
    args.argv[3] = p_data1;

    tbl_arp_iterate((TBL_ITER_CB_FUNC)_arp_api_trav, &args);

    return PM_E_NONE;
}

int32
arp_api_process_flusharp_statistics(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_SET:
        arp_api_flush_statistics();
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return PM_E_NONE;
}

int32
_arp_api_if_set_arp_numberlimit_enable(tbl_interface_t* p_db_if, uint32 arp_numberlimit_enable)
{
    rt_if_del_all_arp(p_db_if);
    return PM_E_NONE;
}

int32
_arp_api_if_set_arp_numberlimit_number(tbl_interface_t* p_db_if, uint32 arp_numberlimit_number)
{
    if (p_db_if->arp_numberlimit_enable)
    {
        rt_if_del_all_arp(p_db_if);
    }
    return PM_E_NONE;
}

/*added by yejl to fix bug 44113, 2017-05-25*/
void
_arp_numberlimit_attack_logtimer(void *p_argv)
{
    tbl_route_if_master_t   *p_master   = tbl_route_if_get_master();
    ctclib_slistnode_t      *listnode   = NULL;
    tbl_route_if_t          *p_route_if = NULL;
    char                    ifname_ext[IFNAME_SIZE];
    
    arp_numberlimit_attacktimer = NULL;
    CTCLIB_SLIST_LOOP(p_master->if_list, p_route_if, listnode)
    {
        if (NULL != p_route_if)
        {
            if (p_route_if->arp_attack_number)
            {
                IFNAME_ETH2FULL(p_route_if->key.name, ifname_ext);
                logid_sys(LOG_ARPNUMBERLIMIT_4_ABNORMAL, ifname_ext, p_route_if->arp_attack_number);    
                p_route_if->arp_attack_number = 0;
            }
        }
    }
    arp_numberlimit_attacktimer = CTC_TASK_ADD_TIME(_arp_numberlimit_attack_logtimer, NULL, ARP_PORT_LIMIT_ATTACK_LOG_TIME);
    return;
}

int32
arp_numberlimit_attack_log_time_init()
{
    if (CTC_TASK_IS_RUNNING(arp_numberlimit_attacktimer))
    {
        CTC_TASK_STOP_TIMER(arp_numberlimit_attacktimer);
    }
    arp_numberlimit_attacktimer = CTC_TASK_ADD_TIME(_arp_numberlimit_attack_logtimer, NULL, ARP_PORT_LIMIT_ATTACK_LOG_TIME);
    return PM_E_NONE;
}
/*ended by yejl*/

int32
arp_api_init()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    
    cdb_register_cfg_tbl_cb(TBL_ARP, arp_cmd_process_arp);
    cdb_register_cfg_act_cb(ACT_FLUSH_ARP, arp_cmd_process_flusharp);
    cdb_register_cfg_act_cb(ACT_FLUSH_ARP_STATISTICS, arp_api_process_flusharp_statistics);
    ipc_register_pkt_rx_fn(GLB_PKT_ARP, arp_ether_packet_rx);
    ipc_register_hal_rx_fn(ROUTED_FEI_FDB_AGE_ARP_NOTIFY, _arp_api_probe_for_fdb_age);
    ipc_register_hal_rx_fn(ROUTED_FEI_DYNAMIC_TUNNEL_UPDATE_NOTIFY, arp_api_for_update_dynamic_tunnel);

    arp_numberlimit_attack_log_time_init();  
    p_rt_glb->arp_aging_timer = ctc_task_add_timer(TASK_PRI_NORMAL, arp_main_aging_timeout,
        p_rt_glb, ROUTE_ARP_AGING_SCAN_INTERVAL);
    return PM_E_NONE;
}

int32
arp_api_deinit()
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();

    if (p_rt_glb->arp_aging_timer)
    {
        ctc_task_delete_timer(p_rt_glb->arp_aging_timer);
        p_rt_glb->arp_aging_timer = NULL;
    }
    
    cdb_register_cfg_tbl_cb(TBL_ARP, NULL);
    cdb_register_cfg_act_cb(ACT_FLUSH_ARP, NULL);
    cdb_register_cfg_act_cb(ACT_FLUSH_ARP_STATISTICS, NULL);
    ipc_register_pkt_rx_fn(GLB_PKT_ARP, NULL);
    //ipc_register_hal_rx_fn(ROUTED_HAL_FDB_AGE_ARP_NOTIFY, NULL);
    return PM_E_NONE;
}

int32
arp_update_vip_dynamic_arp_fib(addr_ipv4_t *dest)
{
    tbl_arp_fib_t   *p_arp_fib = NULL;
    /* deleted arp and nexthop relation by liwh for bug 45444, 2017-10-26 */
    //tbl_nexthop_t   *pst_nexthop = NULL;
    /* liwh end */
    tbl_arp_fib_t   arp_fib;
    char            arp_ifname[IFNAME_SIZE];
    int32           rc = PM_E_NONE;
    tbl_arp_t       *entry = NULL;
    tbl_interface_t *p_db_if = NULL;

    sal_memset(&arp_fib, 0, sizeof(arp_fib));
    sal_memcpy(&arp_fib.key.ip, dest, sizeof(addr_ipv4_t));
    sal_memset(&arp_ifname, 0, sizeof(arp_ifname));

    /* get arp fib info */
    p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib.key);
    if ((NULL != p_arp_fib) && (ARP_TYPE_DYNAMIC == p_arp_fib->type))
    { 
        p_db_if = tbl_interface_get_interface_by_name(p_arp_fib->ifname);
        if (!p_db_if)
        {
            return PM_E_NOT_EXIST;
        }

        /* when arp_fib dynamic entry for vip is exist, add arp entry for vip */
        rc = arp_db_entry_create(p_db_if, &p_arp_fib->key.ip, p_db_if->mac_addr, 
                        p_arp_fib->type, FALSE, FALSE, &entry);
        if (entry)
        {
            entry->status = AS_REACHABLE;
        }
    }

    return rc;
}

