
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "cdb_ds.h"
#include "lib_ioctl.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "rt_fea.h"
#include "route.h"
#include "nd.h"
#include "nd_api.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "gen/ds_connected_v6_define.h"
#include "gen/ds_connected_v6.h"
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "rt_if.h"
#include "lib_sysctl.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"

bool nd_debug_is_on(uint32 flag)
{
    tbl_route_debug_t *p_db_glb = NULL;
    
    p_db_glb = tbl_route_debug_get_route_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->nd, flag))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

int32
nd_api_add_static_route_to_frr_when_del_nd(tbl_nd_fib_t *p_db_nd_fib)
{
    int32 rc = PM_E_NONE;
    char buf1[CMD_BUF_64] = {0}; 
    char buf2[CMD_BUF_64] = {0}; 
    char buf3[CMD_BUF_64] = {0}; 
    
    if(!p_db_nd_fib->is_static_route_dup)
    {
        return PM_E_NONE;
    }

    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;

    sal_memset(&static_route, 0x00, sizeof(static_route));
    static_route.key.p.family = AF_INET6;
    static_route.key.p.prefixlen = 128;
    if (IN6_IS_ADDR_LINKLOCAL(&p_db_nd_fib->staic_route_dup_ipv6_nh))
    {
        static_route.key.ifindex_v6 = p_db_nd_fib->ifindex;
    }
    else
    {
        static_route.key.ifindex_v6 = 0;
    }
    sal_memcpy(&static_route.key.nh_addr.ipv6, &p_db_nd_fib->staic_route_dup_ipv6_nh, sizeof(addr_ipv6_t));
    sal_memcpy(&static_route.key.p.u.prefix6, &p_db_nd_fib->key.addr.u.prefix6, sizeof(addr_ipv6_t));
    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    if (NULL == pst_static_route)
    {
        return PM_E_NONE;
    }

    log_sys(M_MOD_ROUTE, E_INFORMATIONAL, "Add static route %s/%d %s rib for del nd %s\n", 
                cdb_addr_ipv6_val2str(buf1, CMD_BUF_64, &pst_static_route->key.p.u.prefix6),
                pst_static_route->key.p.prefixlen, 
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_64, &pst_static_route->key.nh_addr.ipv6),
                cdb_addr_ipv6_val2str(buf3, CMD_BUF_64, &p_db_nd_fib->key.addr.u.prefix6));
                
    rc = rt_if_add_static_route_v6_by_syscmd(&pst_static_route->key.p.u.prefix6, &pst_static_route->key.nh_addr.ipv6, 
        pst_static_route->key.p.prefixlen, pst_static_route->distance, pst_static_route->key.ifindex_v6);

    return rc;
}

int32
nd_api_del_static_route_from_frr_when_add_nd(tbl_nd_fib_t *p_nd_fib)
{
    tbl_nd_fib_t *p_db_nd_fib = NULL;
    char buf1[CMD_BUF_64] = {0}; 
    char buf2[CMD_BUF_64] = {0}; 
    char buf3[CMD_BUF_64] = {0}; 

    p_db_nd_fib = tbl_nd_fib_get_nd_fib(&p_nd_fib->key);
    if (NULL == p_db_nd_fib)
    {
        return PM_E_NONE;
    }
    
    if(!p_db_nd_fib->is_static_route_dup)
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
        if (AF_INET6 == p_db_rst_cfg->key.p.family
            && 0 == sal_memcmp(&p_db_rst_cfg->key.p.u.prefix6, &p_db_nd_fib->key.addr.u.prefix6, sizeof(addr_ipv6_t)))
        {
            log_sys(M_MOD_ROUTE, E_INFORMATIONAL, "Delete static route %s/%d %s rib for add nd %s\n", 
                cdb_addr_ipv6_val2str(buf1, CMD_BUF_64, &p_db_rst_cfg->key.p.u.prefix6),
                p_db_rst_cfg->key.p.prefixlen, 
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_64, &p_db_rst_cfg->key.nh_addr.ipv6),
                cdb_addr_ipv6_val2str(buf3, CMD_BUF_64, &p_db_nd_fib->key.addr.u.prefix6));
                
            rt_if_del_static_route_v6_by_syscmd(&p_db_rst_cfg->key.p.u.prefix6, &p_db_rst_cfg->key.nh_addr.ipv6, 
                p_db_rst_cfg->key.p.prefixlen, p_db_rst_cfg->key.ifindex_v6);

            p_db_nd_fib->is_static_route_dup = TRUE;
            sal_memcpy(&p_db_nd_fib->staic_route_dup_ipv6_nh, &p_db_rst_cfg->key.nh_addr.ipv6, sizeof(addr_ipv6_t));
            tbl_nd_fib_set_nd_fib_field(p_db_nd_fib, TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP);
            tbl_nd_fib_set_nd_fib_field(p_db_nd_fib, TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH);
            
            return PM_E_NONE;
        }
    }

    return PM_E_NONE;
}

int32 
nd_check_interface(tbl_route_if_t *p_db_rtif, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        CFG_INVALID_PARAM_RET("Invalid interface");
    }

    if (INTERFACE_IS_LOOPBACK(p_db_if))
    {
        CFG_INVALID_PARAM_RET("Cannot configured for loopback interface");
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_INVALID_PARAM_RET("Cannot configure aggregator member");
    }
    
    return rc;
}

void
nd_init_para_for_add_if(tbl_route_if_t *p_rt_if)
{
    p_rt_if->nd_ra_interval_max = GLB_ND_TRT_ADV_INTERVAL;
    p_rt_if->nd_ra_interval_min = RTR_ADV_INTERVAL_FACTOR * p_rt_if->nd_ra_interval_max;
    p_rt_if->nd_ra_min_delay = MIN_DELAY_BETWEEN_RAS;
    p_rt_if->nd_router_lifetime = GLB_ND_ADV_DEFAULT_LIFETIME;
    p_rt_if->nd_ra_currrent_hop_limit = GLB_ND_DEFAULT_RA_CUR_HOP_LIMIT;
    p_rt_if->nd_retransmit_timer = GLB_ND_RETRANS_TIMER;
    p_rt_if->nd_base_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
    p_rt_if->nd_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
    p_rt_if->nd_delay_probe_time = DELAY_FIRST_PROBE_TIME;
    p_rt_if->nd_dad_attempts = GLB_ND_DUP_ADDR_DETECT_TRANSMITS;
    p_rt_if->nd_ra_advertise_mtu = TRUE;

    /* Set destination address to the all node multicast address as default. */
    sal_inet_pton (AF_INET6, ALLNODE, &p_rt_if->nd_dst.u.prefix6);

    p_rt_if->nd_last_ra_unsolicited = -MIN_DELAY_BETWEEN_RAS;
    p_rt_if->nd_ra_prefix_default_vlifetime = GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME;
    p_rt_if->nd_ra_prefix_default_plifetime = GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME;
    p_rt_if->nd_ra_prefix_default_onlink = TRUE;
    p_rt_if->nd_ra_prefix_default_auto = TRUE;
    
    return;
}

int32
nd_dad_successed(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_ifc)
{
    int32                       rc = PM_E_NONE;
    char                        buf[CMD_BUF_256];
    ds_connected_v6_t   *p_ifc_tmp = NULL;
    ctclib_slistnode_t     *listnode = NULL;
    tbl_interface_t         *p_db_if = NULL;
    tbl_interface_t         *p_if_mem = NULL;
    ctclib_slistnode_t     *listnode_mem = NULL;
    tbl_route_if_t           *p_db_member_rtif = NULL;
    char                         ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
 
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    sal_memset(buf, 0, CMD_BUF_256);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_NONE;
    }

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_ifc->key.address.u.prefix6);  
    ND_EVENT_DEBUG("DAD: %s is unique", buf);

    p_ifc->dad_ns_send = 0;
    GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);

    /* delay for address to become effective */
    sal_udelay(MIN_DAD_DELAY_TIME * 1000);

    rt_if_connected_valid_ipv6 (p_db_rtif, p_ifc);
    
    /* send unsolicited NA to let ohter know me */
    GLB_SET_FLAG(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);

    if (p_ifc->t_timer)
    {
        ctc_task_delete_timer(p_ifc->t_timer);
        p_ifc->t_timer = NULL;
    }
    p_ifc->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_ifc_timer_handler, p_ifc, 0);

    /* IPv6 function start after link-local address is verified */
    if (IPV6_PREFIX_IS_LINK_LOCAL(&p_ifc->key.address))
    {
        /* Enable IPv6 routing */
        GLB_SET_FLAG(p_db_rtif->nd_flags, ND_IF_ENABLE);

        ND_EVENT_DEBUG("ND enable unicast routing on the interface %s", ifname_ext);
        
        hal_l3if_set_ipv6_ucast_en(p_db_rtif, TRUE);
        if (INTERFACE_IS_LAG_GROUP(p_db_if) && p_db_if->lag)
        {
            CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_if_mem, listnode_mem)
            {
                p_db_member_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_if_mem->key.name);
                if (p_db_member_rtif)
                {
                    hal_l3if_set_ipv6_ucast_en(p_db_member_rtif, TRUE);
                }
            }    
        }

        /* start Router advertisement */
        if (p_db_rtif->nd_ra_advertise)
        {
            nd_if_rtadv_start(p_db_rtif);
        }

        /* start all other addresses' DAD */
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc_tmp, listnode)
        {
            if ((p_ifc_tmp != p_ifc) &&
                (!GLB_FLAG_ISSET(p_ifc_tmp->flags, RT_IF_ADDRV6_REAL)) && 
                (!GLB_FLAG_ISSET(p_ifc_tmp->flags, RT_IF_ADDR_DUPLICATE)) && 
                (!GLB_FLAG_ISSET(p_ifc_tmp->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS)))
            {
                nd_address_install(p_db_rtif, (void*)p_ifc_tmp);
            }
        }
    }

    return rc;
}

void
nd_ifc_timer_handler(void *p_data)
{
    ds_connected_v6_t   *p_ifc = NULL;
    tbl_route_if_t           rtif;
    tbl_route_if_t           *p_db_rtif;
    char                        buf[CMD_BUF_256];
    struct sal_in6_addr   dst;
    tbl_nd_stats_t         *p_nd_stats = NULL;
    char                        ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    sal_memset(buf, 0, CMD_BUF_256);
    sal_memset(&rtif, 0, sizeof(tbl_route_if_t));

    p_ifc = (ds_connected_v6_t*)p_data;
    if (!p_ifc)
    {
        return;
    }
    p_ifc->t_timer = NULL;

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return;    
    }

    sal_memcpy(&rtif.key.name, p_ifc->ifname, IFNAME_SIZE);
    p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
    if (!p_db_rtif)
    {
        IFNAME_ETH2FULL(p_ifc->ifname, ifname_ext);
        ND_EVENT_DEBUG("Invalid interface %s, %s:%d", ifname_ext, __FUNCTION__, __LINE__);
        return;        
    }

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_ifc->key.address.u.prefix6);  
    if (GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS))
    {
        if (p_ifc->dad_ns_send < p_db_rtif->nd_dad_attempts)
        {
            p_nd_stats->snd_dad_ns++;
            p_ifc->dad_ns_send++;

            ND_EVENT_DEBUG("Time out for DAD NS, target:%s", buf);
            ND_EVENT_DEBUG("Send DAD NS:%d", p_ifc->dad_ns_send);

            nd_send_ns(p_db_rtif, &(p_ifc->key.address.u.prefix6), FALSE, TRUE);
    
            p_ifc->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_ifc_timer_handler, p_ifc, p_db_rtif->nd_retransmit_timer);  
        }
        else
        {
            nd_dad_successed(p_db_rtif, p_ifc);
        }
    }
    else if (GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS))
    {
        p_nd_stats->snd_unsolicited_na++;
        p_ifc->unsolicited_na++;

        ND_EVENT_DEBUG("Time out for unsolicited NA, target:%s", buf);
        ND_EVENT_DEBUG("Send unsolicited NA:%d", p_ifc->unsolicited_na);
        sal_inet_pton (AF_INET6, ALLNODE, &dst);
        nd_send_na(p_db_rtif, &dst, &(p_ifc->key.address.u.prefix6), TRUE, FALSE, TRUE, TRUE);

        if (p_ifc->unsolicited_na < ND_UNSOLICITED_NA)
        {    
            p_ifc->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_ifc_timer_handler, p_ifc, p_db_rtif->nd_retransmit_timer);  
        }
        else
        {
            p_ifc->unsolicited_na = 0;
            GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);
        }
    }

    return;
}

int32
nd_dad_start(tbl_route_if_t *p_db_rtif, ds_connected_v6_t* p_ifc)
{
    int32                    rc = PM_E_NONE;
    char                     buf[CMD_BUF_256];
    uint32                  delay = 0;

    sal_memset(buf, 0, CMD_BUF_256);

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_ifc->key.address.u.prefix6);
    ND_EVENT_DEBUG("ND DAD start for %s/%d", buf, p_ifc->key.address.prefixlen);

    if (IPV6_PREFIX_IS_LINK_LOCAL(&p_ifc->key.address) )
    {
        /* set ns interval to kernel */
        lib_sysctl_nd_set_if_ns_interval(p_db_rtif->key.name, (p_db_rtif->nd_retransmit_timer * MAX_MULTICAST_SOLICIT));

        /* set ns interval to kernel */
        lib_sysctl_nd_set_if_reachable_time(p_db_rtif->key.name, p_db_rtif->nd_base_reachable_time);
    }

    if (0 != p_db_rtif->nd_dad_attempts)
    {
        GLB_SET_FLAG (p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);

        delay = sal_rand() % MAX_RTR_SOLICITATION_DELAY;
        if (delay <= MIN_DAD_DELAY_TIME)
        {
            delay = MIN_DAD_DELAY_TIME;
        }

        if (p_ifc->t_timer)
        {
            ctc_task_delete_timer(p_ifc->t_timer);
            p_ifc->t_timer = NULL;
        }

        p_ifc->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_ifc_timer_handler, p_ifc, delay);
    }
    else
    {
        nd_dad_successed(p_db_rtif, p_ifc);
    }

    return rc;
}

int32
nd_if_stop(tbl_route_if_t *p_db_rtif, bool duplicate)
{
    ds_connected_v6_t     *p_ifc  = NULL;
    ctclib_slistnode_t        *listnode   = NULL;
    tbl_interface_t            *p_db_if = NULL;
    tbl_interface_t            *p_if_mem = NULL;
    ctclib_slistnode_t        *listnode_mem = NULL;
    char                           ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    tbl_route_if_t             *p_db_member_rtif = NULL;
    
    ND_PTR_CHECK(p_db_rtif);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_NONE;
    }

    /* stop RA */
    nd_if_rtadv_stop(p_db_rtif);

    /* delete linklocal ifc */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc, listnode)
    {
        if (IN6_IS_ADDR_LINKLOCAL(&(p_ifc->key.address.u.prefix6)))
        {
            continue;
        }

        if (GLB_FLAG_ISSET (p_ifc->flags, RT_IF_ADDRV6_REAL))
        {
            rt_if_connected_invalid_ipv6(p_db_rtif, p_ifc, FALSE);
        }     

        GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
        GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);

        p_ifc->dad_ns_send = 0;
        p_ifc->unsolicited_na = 0;

        CTC_TASK_STOP_TIMER(p_ifc->t_timer);

        if (!duplicate)
        {
            GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDR_DUPLICATE);
        }
    }

    /* delete linklocal ifc */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc, listnode)
    {
        if (!IN6_IS_ADDR_LINKLOCAL(&(p_ifc->key.address.u.prefix6)))
        {
            continue;
        }

        if (GLB_FLAG_ISSET (p_ifc->flags, RT_IF_ADDRV6_REAL))
        {
            rt_if_connected_invalid_ipv6(p_db_rtif, p_ifc, FALSE);
        }

        GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
        GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);
        p_ifc->dad_ns_send = 0;
        p_ifc->unsolicited_na = 0;

        CTC_TASK_STOP_TIMER(p_ifc->t_timer);

        if (!duplicate)
        {
            GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDR_DUPLICATE);
        }
    }

    /* disable l3 if unicast routing */
    if (p_db_rtif->unicast_en_v6)
    {
        ND_EVENT_DEBUG("ND disable unicast routing on the interface %s", ifname_ext);
        
        hal_l3if_set_ipv6_ucast_en(p_db_rtif, FALSE);
    
        if (INTERFACE_IS_LAG_GROUP(p_db_if) && p_db_if->lag)
        {
            CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_if_mem, listnode_mem)
            {
                p_db_member_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_if_mem->key.name);
                if (p_db_member_rtif)
                {
                    hal_l3if_set_ipv6_ucast_en(p_db_member_rtif, FALSE);
                }
            }    
        }
    }
    
    GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_ENABLE);

    return PM_E_NONE;
}

int32
nd_dad_failed(ds_connected_v6_t  *p_ifc, tbl_route_if_t *p_db_rtif)
{
    char                         buf[CMD_BUF_256];
    char                         ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_ifc);
    ND_PTR_CHECK(p_db_rtif);

    sal_memset(buf, 0, CMD_BUF_256);
    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_ifc->key.address.u.prefix6);

    IFNAME_ETH2FULL(p_ifc->ifname, ifname_ext);
    ND_EVENT_DEBUG("Duplicate address %s on %s", buf, ifname_ext);

    GLB_SET_FLAG(p_ifc->flags, RT_IF_ADDR_DUPLICATE);
    ds_connected_v6_set_connected_v6_field(p_db_rtif, p_ifc, DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS);

    if (IN6_IS_ADDR_LINKLOCAL(&(p_ifc->key.address.u.prefix6)))
    {
        ND_EVENT_DEBUG("DAD: duplicate link-local %s on %s, interface stalled", buf, ifname_ext);     
        nd_if_stop(p_db_rtif, TRUE);
    }
    else
    {
        rt_fea_ipv6_del_connect_route_by_link(DEFAULT_VRF_ID, &p_ifc->key.address.u.prefix6, p_ifc->key.address.prefixlen, p_ifc->ifname);
        p_ifc->dad_ns_send = 0;
        GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
        CTC_TASK_STOP_TIMER(p_ifc->t_timer);
    }
      
    return PM_E_NONE;
}

int32
nd_address_install(tbl_route_if_t *p_db_rtif, void* ifc_v6)
{
    int32                       rc = PM_E_NONE;
    tbl_interface_t          *p_db_if = NULL;
    char                         buf[CMD_BUF_256];
    ds_connected_v6_t    *p_ifc = NULL;
    char                          ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    sal_memset(buf, 0, CMD_BUF_256);

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(ifc_v6);

    p_ifc = (ds_connected_v6_t*)ifc_v6;
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (NULL == p_db_if)
    {
        sal_printf("Invalid interface, %s:%d", __FUNCTION__, __LINE__);
        return PM_E_INVALID_PARAM;
    }

    /* update nd fib */
    neigh_ifc_add(p_db_rtif, p_ifc);

    if (IPV6_PREFIX_IS_LINK_LOCAL(&p_ifc->key.address) 
        || ((!IPV6_PREFIX_IS_LINK_LOCAL(&p_ifc->key.address)) && GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_ENABLE)))
    {
        /* Interface must be up, address can't be duplicate, 
            and address isn't already in DAD progress  */
        if (tbl_interface_is_running(p_db_if) && (!GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDR_DUPLICATE))
            && (!GLB_FLAG_ISSET(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS)))
        {
            if (rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable))
            {
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_ifc->key.address.u.prefix6);

                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                ND_EVENT_DEBUG("Adding prefix %s/%d to %s", buf, p_ifc->key.address.prefixlen, ifname_ext);

                if (!GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_ENABLE)
                    && !IPV6_PREFIX_IS_LINK_LOCAL(&p_ifc->key.address))
                {
                     usleep(20000);  
                }

                /* start DAD check */
                nd_dad_start(p_db_rtif, ifc_v6);
            }
        }
    }

    return rc;
}

int32
nd_api_check_prefix_same_with_ifaddr(tbl_route_if_t  *p_db_rt_if, addr_ipv6_t *prefix)
{
    ctclib_slistnode_t   *listnode  = NULL;
    ds_connected_t      *p_db_conn = NULL;
    prefix_t                 *p = NULL;
 
    CTCLIB_SLIST_LOOP(p_db_rt_if->ifc_ipv6.obj_list, p_db_conn, listnode)
    {
        p = &p_db_conn->key.address;
        if (0 == sal_memcmp(&p->u.prefix6, prefix, sizeof(addr_ipv6_t)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
nd_trigger_request(addr_ipv6_t *ipv6_addr, tbl_interface_t  *p_db_if, tbl_route_if_t *p_db_rt_if)
{
    char                         buf[CMD_BUF_256];
    tbl_nd_stats_t          *p_nd_stats = NULL;
    tbl_nd_fib_t              *p_nd_fib = NULL;
    tbl_nd_fib_t              nd_fib;
    prefix_t                    p;
    char                         ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    sal_memset(buf, 0, CMD_BUF_256);
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(&p, 0, sizeof(prefix_t));

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_FAIL;    
    }

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, ipv6_addr);
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    ND_EVENT_DEBUG("ND nbr miss for %s interface %s", buf, ifname_ext);

    p_nd_stats->rcv_miss++;

    p.family = AF_INET6;
    p.prefixlen = IPV6_MAX_BITLEN;
    sal_memcpy(&p.u.prefix6, ipv6_addr, sizeof(addr_ipv6_t));
    if(if_match_active_ifc_ipv6_num(p_db_rt_if, &p) == 0)
    {
        ND_EVENT_DEBUG("Trigger NS to %s, but interafce has no active address with same subnet", buf);
        p_nd_stats->rcv_miss_conflict++;
        return PM_E_NONE;
    }

    nd_fib_make_key(&nd_fib.key, ipv6_addr, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib((tbl_nd_fib_key_t*)&nd_fib.key);
    if (p_nd_fib)
    {
        /* can't overwrite static entry */
        if (p_nd_fib->flags & GLB_ND_FLAG_PERMANENT)
        {
            ND_EVENT_DEBUG("Trigger NS to %s, but there is static neigh cache entry", buf);
            p_nd_stats->rcv_static_conflict++;
            return PM_E_NONE;
        }

        if (NES_IS_VALID(p_nd_fib))
        {
            ND_EVENT_DEBUG("Trigger NS to %s, but there is already a valid neigh cache entry", buf);
            p_nd_stats->rcv_miss_conflict++;
            return PM_E_NONE;
        }

        if (NES_INCOMPLETE == p_nd_fib->state)
        {
            ND_EVENT_DEBUG("ND %s is in progress", buf);
            return PM_E_NONE;
        }    
    }
    else
    {
        p_nd_fib = nd_neigh_create(&nd_fib, p_db_if);
        if (!p_nd_fib)
        {
            return PM_E_FAIL;
        }

        p_nd_fib->state = NES_INCOMPLETE;   
    }

    p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
        p_db_rt_if->nd_retransmit_timer);

    p_nd_fib->probes++;
    ND_EVENT_DEBUG("Send MCAST probe: %d", p_nd_fib->probes);

    nd_send_ns(p_db_rt_if, ipv6_addr, FALSE, FALSE);

    p_nd_fib->opackets++;
    
    return PM_E_NONE;
}

int32
nd_api_miss_trigger(addr_ipv6_t *ipv6_addr, uint32 kernel_ifindex)
{
    int32                       rc = PM_E_NONE;
    tbl_interface_t          *p_db_if = NULL;
    tbl_kernel_if_t          *pst_kernel_if = NULL;
    tbl_kernel_if_key_t    kernel_if_key;
    int32                        is_ifaddr = FALSE;
    tbl_route_if_t            *p_db_rt_if = NULL;
    tbl_route_if_key_t      rtif_key;
    tbl_nd_stats_t          *p_nd_stats = NULL;
    
    ND_PTR_CHECK(ipv6_addr);

    sal_memset(&kernel_if_key, 0, sizeof(kernel_if_key));   
    
    kernel_if_key.ifindex = kernel_ifindex;
    pst_kernel_if = tbl_kernel_if_get_kernel_if(&kernel_if_key);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_if = tbl_interface_get_interface_by_name(pst_kernel_if->name);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, pst_kernel_if->name, sizeof(rtif_key.name));
    p_db_rt_if = tbl_route_if_get_route_if(&rtif_key);
    if (NULL == p_db_rt_if)
    {
        p_nd_stats->intf_invalid++;
        return PM_E_NOT_EXIST;
    }

    is_ifaddr = nd_api_check_prefix_same_with_ifaddr(p_db_rt_if, ipv6_addr);
    if (tbl_interface_is_running(p_db_if) && !is_ifaddr)
    {
        /* do request */
        rc = nd_trigger_request(ipv6_addr, p_db_if, p_db_rt_if);
    }

    return rc;
}

void
nd_rtadv_if_timer(void *p_data)
{
    tbl_route_if_t           *p_db_rtif = NULL;
    ds_connected_v6_t    *p_ifc = NULL;
    struct sal_in6_addr    allnode;

    if (!p_data)
    {
        return;    
    }
    
    sal_memset(&allnode, 0, sizeof(struct sal_in6_addr));
    
    p_db_rtif = (tbl_route_if_t*)p_data;
    p_db_rtif->t_ra_unsolicited = NULL;

    p_ifc = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
    if (!p_ifc)
    {
        return;        
    }

    /* Send to the all node multicast address. */
    sal_inet_pton (AF_INET6, ALLNODE, &allnode);
    nd_send_ra (p_db_rtif, &(p_ifc->key.address.u.prefix6), &allnode, TRUE);
    p_db_rtif->nd_ra_init_unsolicited_ocount++;

    p_db_rtif->nd_ra_interval = ND_IF_RA_INTERVAL_RANDOM(p_db_rtif);
    if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL) && 
        (p_db_rtif->nd_ra_init_unsolicited_ocount < MAX_INITIAL_RTR_ADVERTISEMETNS))
    {
        if (p_db_rtif->nd_ra_interval > MAX_INITIAL_RTR_ADVERT_INTERVAL)
        {
            p_db_rtif->nd_ra_interval = MAX_INITIAL_RTR_ADVERT_INTERVAL;
        }
    }
    else
    {
        p_db_rtif->nd_ra_init_unsolicited_ocount = 0;
        GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL);
    }

    CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
    p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, p_db_rtif->nd_ra_interval);

    return;
}

void 
nd_rtadv_if_ra_solicited(void *p_data)
{
    tbl_route_if_t           *p_db_rtif = NULL;
    ds_connected_v6_t    *p_ifc = NULL;
    struct sal_in6_addr    allnode;

    if (!p_data)
    {
        return;    
    }
    
    sal_memset(&allnode, 0, sizeof(struct sal_in6_addr));
    
    p_db_rtif = (tbl_route_if_t*)p_data;
    p_db_rtif->t_ra_solicited = NULL;

    p_ifc = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
    if (!p_ifc)
    {
        return;        
    }

    p_db_rtif->nd_last_ra_solicited = ctc_time_boottime_msec(NULL);
    
    if (IN6_IS_ADDR_UNSPECIFIED(&p_db_rtif->nd_rs_dest_addr))
    {
        /* Send to the all node multicast address. */
        sal_inet_pton (AF_INET6, ALLNODE, &allnode);
        nd_send_ra(p_db_rtif, &(p_ifc->key.address.u.prefix6), &allnode, TRUE);
    }
    else
    {
        nd_send_ra(p_db_rtif, &(p_ifc->key.address.u.prefix6), &(p_db_rtif->nd_rs_dest_addr.u.prefix6), TRUE);
    }

    return;
}

int32
nd_if_rtadv_start(tbl_route_if_t *p_db_rtif)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif); 

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("ND start Router Advertisement on the interface %s", ifname_ext);

    if (!GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON))
    {
        GLB_SET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_ON);
        GLB_SET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL);
        p_db_rtif->nd_last_ra_unsolicited = 0;

        /* For the first few advertisements (up to 
            MAX_INITIAL_RTR_ADVERTISEMENTS) sent from an interface when it
            becomes an advertising interface, if the randomly chosen interval is
            greater than MAX_INITIAL_RTR_ADVERT_INTERVAL, the timer 
            SHOULD be set  to MAX_INITIAL_RTR_ADVERT_INTERVAL instead. */
            
        p_db_rtif->nd_ra_interval = ND_IF_RA_INTERVAL_RANDOM(p_db_rtif);
        if (p_db_rtif->nd_ra_interval > MAX_INITIAL_RTR_ADVERT_INTERVAL)
        {
            p_db_rtif->nd_ra_interval = MAX_INITIAL_RTR_ADVERT_INTERVAL;
        }

        CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
        p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, p_db_rtif->nd_ra_interval);
    }
    else
    {
        ND_EVENT_DEBUG("ND Router Advertisement has started on the interface %s", ifname_ext);
    }

    return PM_E_NONE;
}

int32
nd_if_rtadv_stop(tbl_route_if_t *p_db_rtif)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif); 

    if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON))
    {
        IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
        ND_EVENT_DEBUG("ND stop Router Advertisement on the interface %s", ifname_ext); 
    
        CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_solicited);
        CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);

        GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_ON);
        GLB_UNSET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL);

        p_db_rtif->nd_ra_init_unsolicited_ocount = 0;
        p_db_rtif->nd_last_ra_unsolicited = 0;
    }

    return PM_E_NONE;
}

int32
nd_if_rtadv_reset(tbl_route_if_t *p_db_rtif)
{
    uint32       interval = 0;
    uint32       remained = 0;
    char          ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("ND reset Router Advertisement on the interface %s", ifname_ext);

    if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON)) 
    {
        GLB_SET_FLAG(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL);
        p_db_rtif->nd_ra_init_unsolicited_ocount = 0;

        interval = ND_IF_RA_INTERVAL_RANDOM(p_db_rtif);
        if (interval > MAX_INITIAL_RTR_ADVERT_INTERVAL)
        {
            interval = MAX_INITIAL_RTR_ADVERT_INTERVAL;
        }

        if (p_db_rtif->t_ra_unsolicited)
        {
            remained =ctc_timer_get_remain_msec(p_db_rtif->t_ra_unsolicited);
        }
        
        if (interval < remained)
        {
            /* send earlier as possible */
            p_db_rtif->nd_ra_interval = interval + (p_db_rtif->nd_ra_interval - remained);

            CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
            p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, interval);
        }
    }

    return PM_E_NONE;
}

int32
nd_if_rtadv_chage_ra_interval(tbl_route_if_t *p_db_rtif)
{
    uint32  interval = 0;
    uint32  remained = 0;
    char     ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    ND_PTR_CHECK(p_db_rtif);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("ND change Router Advertisement interval on the interface %s", ifname_ext);

    if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON) && 
        (!GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_INITIAL)))
    {
        interval = ND_IF_RA_INTERVAL_RANDOM(p_db_rtif);
        if (p_db_rtif->t_ra_unsolicited)
        {
            remained =ctc_timer_get_remain_msec(p_db_rtif->t_ra_unsolicited);
        }

        if (interval < remained)
        {
            /* send earlier as possible */
            p_db_rtif->nd_ra_interval = interval + (p_db_rtif->nd_ra_interval - remained);

            CTC_TASK_STOP_TIMER(p_db_rtif->t_ra_unsolicited);
            p_db_rtif->t_ra_unsolicited = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_rtadv_if_timer, p_db_rtif, interval);
        }
    }
    
    return PM_E_NONE;
}

int32
nd_api_if_show(FILE *fp, tbl_route_if_t *p_db_rtif)
{
    uint32  remained = 0;
    
    ND_PTR_CHECK(fp);
    ND_PTR_CHECK(p_db_rtif);

    /* DAD attempts */
    if (p_db_rtif->nd_dad_attempts > 0)
    {
        sal_fprintf(fp, "  ND DAD is enabled, number of DAD attempts: %u\n",
            p_db_rtif->nd_dad_attempts);
    }
    else
    {
        sal_fprintf(fp, "  ND DAD is disabled\n");
    }

    /* router advertisement */
    if (p_db_rtif->nd_ra_advertise)
    {
        sal_fprintf(fp, "  ND router advertisement is enabled\n");
    }
    else
    {
        sal_fprintf(fp, "  ND router advertisement is disabled\n");
    }

    /* reachable time */
    sal_fprintf(fp, "  ND reachable time is %u milliseconds\n", p_db_rtif->nd_base_reachable_time);

    if (GLB_ND_BASE_REACHABLE_TIME != p_db_rtif->nd_base_reachable_time)
    {
        sal_fprintf(fp, "  ND advertised reachable time is %u milliseconds\n", p_db_rtif->nd_base_reachable_time);
    }
    else
    {
        sal_fprintf(fp, "  ND advertised reachable time is %u milliseconds\n", GLB_ND_UNSPECIFIED);
    }
   
    /* retransmit interval */
    sal_fprintf(fp, "  ND retransmit interval is %u milliseconds\n", p_db_rtif->nd_retransmit_timer);

    if (GLB_ND_RETRANS_TIMER == p_db_rtif->nd_retransmit_timer)
    {
        sal_fprintf(fp, "  ND advertised retransmit interval is %u milliseconds\n", GLB_ND_UNSPECIFIED);
    }
    else
    {   
        sal_fprintf(fp, "  ND advertised retransmit interval is %u milliseconds\n", p_db_rtif->nd_retransmit_timer);
    }

    /* advertisements interval */
    sal_fprintf(fp, "  ND router advertisements max interval: %u secs\n",
        (p_db_rtif->nd_ra_interval_max + 500) / 1000);

    sal_fprintf(fp, "  ND router advertisements min interval: %u secs\n",
        ((p_db_rtif->nd_ra_interval_min + 500) / 1000));

    if (NULL != p_db_rtif->t_ra_unsolicited)
    {
        if (p_db_rtif->t_ra_unsolicited)
        {
            remained =ctc_timer_get_remain_msec(p_db_rtif->t_ra_unsolicited);
        }
        sal_fprintf(fp, "  ND next router advertisement due in %u secs\n", (remained/1000));
    }

    /* router lifetime */
    if (GLB_ND_UNSPECIFIED == p_db_rtif->nd_router_lifetime)
    {
         sal_fprintf(fp, "  ND is not advertising as a default router\n");
    }
    else
    {
        sal_fprintf(fp, "  ND router advertisements live for %u seconds\n", p_db_rtif->nd_router_lifetime);
    }
    
    /* cur hop-limit */
    sal_fprintf(fp, "  ND router advertisements hop-limit is %u\n", p_db_rtif->nd_ra_currrent_hop_limit);

    /* managed address config flag */
    if (p_db_rtif->nd_managed_config)
    {
        sal_fprintf(fp, "  Hosts use DHCP to obtain routable addresses\n");
    }
    else
    {
        sal_fprintf(fp, "  Hosts use stateless autoconfig for addresses\n");
    }

    /* other statefull config flag */
    if (p_db_rtif->nd_other_config)
    {
        sal_fprintf(fp, "  Hosts use DHCP to obtain other configuration\n");
    }

    return PM_E_NONE;
}

int32
nd_api_delete_rtif_config(tbl_route_if_t *p_db_rtif)
{
    int32  rc = PM_E_NONE;
    
    p_db_rtif->nd_ra_currrent_hop_limit = GLB_ND_DEFAULT_RA_CUR_HOP_LIMIT;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_CURRRENT_HOP_LIMIT));

    p_db_rtif->nd_ra_advertise = FALSE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE));

    p_db_rtif->nd_ra_advertise_mtu = TRUE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE_MTU));

    p_db_rtif->nd_ra_interval_max = GLB_ND_TRT_ADV_INTERVAL;
    p_db_rtif->nd_ra_interval_min = RTR_ADV_INTERVAL_FACTOR * p_db_rtif->nd_ra_interval_max;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_INTERVAL_MAX));
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_INTERVAL_MIN));

    p_db_rtif->nd_router_lifetime = GLB_ND_ADV_DEFAULT_LIFETIME;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_ROUTER_LIFETIME));

    p_db_rtif->nd_dad_attempts = GLB_ND_DUP_ADDR_DETECT_TRANSMITS;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_DAD_ATTEMPTS));

    /* set ns interval to kernel */
   p_db_rtif->nd_retransmit_timer = GLB_ND_RETRANS_TIMER;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RETRANSMIT_TIMER));

    /* set ns interval to kernel */
    p_db_rtif->nd_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_REACHABLE_TIME));
    p_db_rtif->nd_base_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_BASE_REACHABLE_TIME));

    p_db_rtif->nd_managed_config = FALSE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_MANAGED_CONFIG));

    p_db_rtif->nd_other_config = FALSE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_OTHER_CONFIG));

   p_db_rtif->nd_ra_prefix_default_vlifetime = GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_VLIFETIME));

    p_db_rtif->nd_ra_prefix_default_plifetime = GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_PLIFETIME));

    p_db_rtif->nd_ra_prefix_default_onlink = TRUE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_ONLINK));

    p_db_rtif->nd_ra_prefix_default_auto = TRUE;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_AUTO));
    
    return  rc;
}

int32 
nd_clear_nd_entries_and_config(tbl_route_if_t *p_db_rtif)
{
    /* delete dynamic nd entries */
    nd_if_stop(p_db_rtif, FALSE);
    
    /* delete all nd prefix */
    nd_prefix_delete_all_entry(p_db_rtif);

    /* set all nd if config to default */
    nd_api_delete_rtif_config(p_db_rtif);

    /* delete all static nd prefix entries*/
    nd_neigh_delete_all_if_entries(p_db_rtif->ifindex);
 
    return  PM_E_NONE;
}

int32
nd_api_if_up_pre_unset(tbl_route_if_t *p_db_rtif)
{
    ds_connected_v6_t    *p_ifc = NULL;
    struct sal_in6_addr    allnode;
    tbl_interface_t           *p_db_if = NULL;
    char                           ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    
    sal_memset(&allnode, 0, sizeof(struct sal_in6_addr));

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    ND_EVENT_DEBUG("ND api pre-unset interface %s", ifname_ext);

    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(p_db_rtif->key));
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        ND_EVENT_DEBUG("Cannot configure aggregator member");
        return  PM_E_NONE;
    }

    p_ifc = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
    if (p_ifc && GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON))
    {
        /* Send to the all node multicast address. */
        ND_EVENT_DEBUG("ND api send RA with router lifetime zero on the interface %s", ifname_ext);
        sal_inet_pton (AF_INET6, ALLNODE, &allnode);
        nd_send_ra (p_db_rtif, &(p_ifc->key.address.u.prefix6), &allnode, FALSE);
    }

    return  PM_E_NONE;
}

uint32
nd_api_count_get_max()
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
        if (GLB_STM_MODE_IPUCV6 == type)
        {
            p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
            if(NULL != p_sys_spec) {
                return p_sys_spec->host_v6_routes;
            }
        }
    }
            
    return 0;
}
