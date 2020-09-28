
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
#include "cdb_ds.h"
#include "lib_ioctl.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "rt_fea.h"
#include "route.h"
#include "nd.h"
#include "nd_api.h"
#include "lib_netlink.h"
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "rt_if.h"

void
nd_neigh_count_inc(void)
{
    tbl_route_global_t      *p_route_glb = NULL;

    p_route_glb = tbl_route_global_get_route_global();

    if (p_route_glb)
    {
        p_route_glb->nd_count++;    
    }

    return;
}

void
nd_neigh_count_dec(void)
{
    tbl_route_global_t      *p_route_glb = NULL;

    p_route_glb = tbl_route_global_get_route_global();
    if (p_route_glb)
    {
        p_route_glb->nd_count--;    
    }

    return;
}

int32
nd_fea_add_nbr(tbl_nd_fib_t*   p_nd_fib, tbl_interface_t  *p_db_if)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;
    
    pst_kernel_if = tbl_kernel_if_get_by_name(p_db_if->key.name);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    ND_PTR_CHECK(p_nd_fib);
    ND_PTR_CHECK(p_db_if);

    if (p_nd_fib->ifindex != p_db_if->ifindex)
    {
        sal_memcpy(p_nd_fib->ifname, p_db_if->key.name, IFNAME_SIZE);
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFNAME);
    
        p_nd_fib->ifindex= p_db_if->ifindex;
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFINDEX);
    }

    rt_fea_nexthop_ipv6_add(p_nd_fib->ifname, &p_nd_fib->key.addr.u.prefix6);
    nd_api_del_static_route_from_frr_when_add_nd(p_nd_fib);

    p_nd_fib->flags |= GLB_ND_FLAG_ACTIVE;
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_ACTIVE);
    kernel_ipv6_neigh_entry_add(&p_nd_fib->key.addr.u.prefix6, p_nd_fib->mac_addr, pst_kernel_if->key.ifindex);

    return PM_E_NONE;
}

int32
nd_fea_del_nbr(tbl_nd_fib_t*   p_nd_fib)
{
    tbl_kernel_if_t *pst_kernel_if = NULL;

    ND_PTR_CHECK(p_nd_fib);
    
    pst_kernel_if = tbl_kernel_if_get_by_name(p_nd_fib->ifname);
    if (NULL == pst_kernel_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    ND_PTR_CHECK(p_nd_fib);

    GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_ACTIVE);
    rt_fea_nexthop_ipv6_del(p_nd_fib->ifname, &p_nd_fib->key.addr.u.prefix6);

    kernel_ipv6_neigh_entry_del(&p_nd_fib->key.addr.u.prefix6, p_nd_fib->mac_addr, pst_kernel_if->key.ifindex);
    nd_api_add_static_route_to_frr_when_del_nd(p_nd_fib);
    return PM_E_NONE;
}

int32
nd_fea_update_nbr(tbl_nd_fib_t*   p_nd_fib, tbl_interface_t  *p_db_if, char *p_new_mac, char *p_old_mac)
{
    ND_PTR_CHECK(p_nd_fib);
    ND_PTR_CHECK(p_db_if);
    ND_PTR_CHECK(p_new_mac);
    ND_PTR_CHECK(p_old_mac);

    sal_memcpy(p_nd_fib->mac_addr, p_old_mac, GLB_ETH_ADDR_LEN);
    nd_fea_del_nbr(p_nd_fib);

    sal_memcpy(p_nd_fib->mac_addr, p_new_mac, GLB_ETH_ADDR_LEN);
    nd_fea_add_nbr(p_nd_fib, p_db_if);
    
    return PM_E_NONE;
}

int32
nd_fib_make_key(tbl_nd_fib_key_t *p_fib_key, addr_ipv6_t* p_addr, tbl_interface_t  *p_db_if)
{
    ND_PTR_CHECK(p_fib_key);
    ND_PTR_CHECK(p_addr);

    p_fib_key->addr.family = AF_INET6;
    p_fib_key->addr.prefixlen = IPV6_MAX_BITLEN;
    sal_memcpy(&p_fib_key->addr.u.prefix6, p_addr, sizeof(addr_ipv6_t));
    if (IPV6_PREFIX_IS_LINK_LOCAL(&p_fib_key->addr) && p_db_if)
    {
        p_fib_key->ifindex = p_db_if->ifindex;
    }
    else
    {
         p_fib_key->ifindex = GLB_INVALID_IFINDEX;
    }

    return PM_E_NONE;
}

int32
nd_neigh_add_static_entry(prefix_t* p_addr, uint8 *p_mac, tbl_interface_t  *p_db_if, tbl_route_if_t  *p_db_rtif, cfg_cmd_para_t *para)
{
    uint16            old_state = NES_STATE_MAX;
    char               nbstt[6][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
    bool               add = FALSE;
    bool               remove = FALSE;
    bool               reachable = FALSE;
    tbl_nd_fib_t*   p_nd_fib = NULL;
    tbl_nd_fib_t    nd_fib;
    char               old_mac[GLB_ETH_ADDR_LEN];
    char               buf[CMD_BUF_256];
    tbl_nd_fib_master_t   *p_nd_fib_master = NULL;
     
    sal_memset(old_mac, 0, GLB_ETH_ADDR_LEN);
    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(buf, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_addr->u.prefix6);

    p_nd_fib_master = tbl_nd_fib_get_master();
    if (!p_nd_fib_master)
    {
        ND_LOG_ERR("Nd fib table is not exist\n");
        return PM_E_FAIL;    
    }

    if (p_nd_fib_master->neighbor_hash->count >= nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("IPv6 neighbor cache overflow");    
    }

    /* check interface is running and ipv6 address isn't be used */
    if (p_db_if)
    {
        add = tbl_interface_is_running(p_db_if);
        add = add & if_match_active_ifc_ipv6_num(p_db_rtif, p_addr);
    }
    reachable = add;

    nd_fib_make_key(&nd_fib.key, &p_addr->u.prefix6, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (p_nd_fib)
    {
        sal_memcpy(old_mac, p_nd_fib->mac_addr, GLB_ETH_ADDR_LEN);
        remove = NES_IS_ACTIVE(p_nd_fib); 

        if (NES_IS_ACTIVE(p_nd_fib)  &&  (0 == sal_memcmp (p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN))
            && !GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE))
        {
            /* same as the previos */
            if (remove && add)
            {
                remove = FALSE;
                add = FALSE;
            }
        }

        old_state = p_nd_fib->state;
    }
    else
    {
        old_state = NES_INCOMPLETE;

        nd_fib_make_key(&nd_fib.key, &p_addr->u.prefix6, p_db_if);
        p_nd_fib = nd_neigh_create(&nd_fib, p_db_if);  
        if (!p_nd_fib)
        {
            return PM_E_FAIL;
        }
    }

    /* Set new field */
    sal_memcpy (p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN);
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_MAC_ADDR);
        
    if (reachable)
    {
        p_nd_fib->state = NES_REACHABLE;
    }
    else
    {
        p_nd_fib->state = NES_NONE;
    }
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

    p_nd_fib->flags |= GLB_ND_FLAG_PERMANENT;
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_PERMANENT);

    p_nd_fib->probes = 0;
    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_PROBES);

    CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
        
    if (old_state != p_nd_fib->state)
    {
        ND_EVENT_DEBUG("%s -> %s: %s", nbstt[old_state], nbstt[p_nd_fib->state], buf);
    }

    if (add && remove)
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
        GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE);
        nd_fea_update_nbr(p_nd_fib, p_db_if, (char*)p_mac, old_mac);
    }
    else if (remove)
    {
        GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
        nd_fea_del_nbr(p_nd_fib);
    }
    else if (add)
    {
        if (connected_check_ipv6(p_db_rtif, p_addr))
        {
            return PM_E_NONE;    
        }

        nd_fea_add_nbr(p_nd_fib, p_db_if);
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
    }

    /* set nd fib interface info */
    if (p_db_if)
    {
        sal_memcpy(p_nd_fib->ifname, p_db_if->key.name, IFNAME_SIZE);
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFNAME);
    
        p_nd_fib->ifindex= p_db_if->ifindex;
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFINDEX);
    }
    else
    {
        /* set nd fib interface to invalid */
        p_nd_fib->ifindex= GLB_INVALID_IFINDEX;    
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFINDEX);
        sal_memset(p_nd_fib->ifname, 0, IFNAME_SIZE);
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFNAME);  
    }

    return PM_E_NONE;
}

int32
nd_neigh_del_static_entry(prefix_t* p_addr, tbl_interface_t  *p_db_if, cfg_cmd_para_t *para)
{
    tbl_nd_fib_t*   p_nd_fib = NULL;
    tbl_nd_fib_t     nd_fib;

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));

    nd_fib_make_key(&nd_fib.key, &p_addr->u.prefix6, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (!p_nd_fib)
    {
        CFG_CONFLICT_RET("No such static ND entry!\n");
    }

    if (p_nd_fib && IPV6_PREFIX_IS_LINK_LOCAL(p_addr))
    {
        if (!p_db_if || (p_db_if->ifindex != p_nd_fib->ifindex))
        {
            CFG_CONFLICT_RET("No such static ND entry!\n");
        }
    }

    if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
    {
        CFG_CONFLICT_RET("No such static ND entry!\n");
    }

    nd_neigh_delete(p_nd_fib);

    return PM_E_NONE;
}

void*
nd_neigh_create(void  *p_nd_fib, tbl_interface_t  *p_db_if)
{
    int32                        rc = PM_E_NONE;
    tbl_nd_stats_t          *p_nd_stats = NULL;
    char                         buf[CMD_BUF_256];
    tbl_nd_fib_t              nd_fib;
    tbl_nd_fib_t              *p_new_nd_fib = NULL;
    tbl_nd_fib_master_t   *p_nd_fib_master = tbl_nd_fib_get_master();
    
    sal_memset(buf, 0, CMD_BUF_256);
    sal_memcpy(&nd_fib, (uint8*)p_nd_fib, sizeof(tbl_nd_fib_t));

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &nd_fib.key.addr.u.prefix6);

    p_nd_fib_master = tbl_nd_fib_get_master();
    if (!p_nd_fib_master)
    {
        ND_LOG_ERR("Create nd entry %s failure because Nd fib table is not exist\n", buf);
        return NULL;    
    }

    if (p_nd_fib_master->neighbor_hash->count >= nd_api_count_get_max())
    {
        ND_LOG_ERR("Create nd entry %s failure because IPv6 neighbor cache overflow\n", buf);
        return NULL;
    }

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        ND_LOG_ERR("Create nd entry %s failure because Nd stats table is not exist\n", buf);
        return NULL;    
    }

    rc = tbl_nd_fib_add_nd_fib(&nd_fib);
    if (rc)
    {
        ND_LOG_ERR("Create nd entry %s failure because Nd add fib %s failure\n", buf);
        return NULL;
    }

    //nd_api_del_static_route_from_frr_when_add_nd(&nd_fib);
    
    p_new_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (!p_new_nd_fib)
    {
        ND_LOG_ERR("Create nd entry %s failure because Nd add fib %s failure\n", buf);
        return NULL;
    }

    if (p_db_if)
    {
        p_new_nd_fib->ifindex = p_db_if->ifindex;
        sal_memcpy(p_new_nd_fib->ifname, p_db_if->key.name, IFNAME_SIZE);
    }
    else
    {
        p_new_nd_fib->ifindex = GLB_INVALID_IFINDEX;
        sal_memset(p_new_nd_fib->ifname, 0, IFNAME_SIZE);
    }
    
    p_new_nd_fib->uptime = ctc_time_boottime_sec(NULL);
    
    nd_neigh_count_inc();

    p_nd_stats->allocs++;

    ND_EVENT_DEBUG("DELETE -> INCMP: %s", buf);

    return p_new_nd_fib;
}

int32
nd_neigh_delete(tbl_nd_fib_t  *p_nd_fib)
{
    tbl_interface_t           *p_db_if = NULL;
    tbl_nd_stats_t           *p_nd_stats = NULL;
    char                          nbstt[6][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
    char                          buf[CMD_BUF_256];
    
    ND_PTR_CHECK(p_nd_fib);

    sal_memset(buf, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);

    p_db_if = tbl_interface_get_interface_by_name(p_nd_fib->ifname);

    if (NES_IS_ACTIVE(p_nd_fib))
    {
        if (NULL == p_db_if)
        {
            return PM_E_FAIL;
        }
        
        nd_fea_del_nbr(p_nd_fib);
    }
    
    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_FAIL;    
    }

    if (p_nd_fib->route_ref && p_db_if && tbl_interface_is_running(p_db_if))
    {
        if (p_nd_fib->state < NES_STATE_MAX)
        {
            ND_EVENT_DEBUG("%s -> DELETE: %s", nbstt[p_nd_fib->state], buf);    
        }

        ND_EVENT_DEBUG("DELETE -> INCMP: %s", buf);

        sal_memset(p_nd_fib->mac_addr, 0, GLB_ETH_ADDR_LEN);
        p_nd_fib->state = NES_INCOMPLETE;
        p_nd_fib->probes = 0;
        p_nd_fib->uptime = ctc_time_boottime_sec(NULL);

        if (FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
        {
            GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT);
            tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_PERMANENT);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 0);
        }
        else
        {
            if (p_nd_fib->t_timer)
            {
                ctc_task_delete_timer(p_nd_fib->t_timer);
                p_nd_fib->t_timer = NULL;    
            }
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 0);
        }
    }
    else
    {
        if (p_nd_fib->state < NES_STATE_MAX)
        {
            ND_EVENT_DEBUG("%s -> DELETE: %s", nbstt[p_nd_fib->state], buf);
        }

        CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);

        //nd_api_add_static_route_to_frr_when_del_nd(p_nd_fib);
        tbl_nd_fib_del_nd_fib(&p_nd_fib->key);
        nd_neigh_count_dec();
        p_nd_stats->destroys++;   
    }
    
    return PM_E_NONE;
}

void
nd_neigh_timer_handler(void *p_data)
{
    tbl_nd_fib_t              *p_nd_fib = NULL;
    tbl_interface_t          *p_db_if = NULL;
    uint8                        state = 0;
    char                         buf[CMD_BUF_256];
    tbl_route_if_t            *p_db_rtif = NULL;
    tbl_route_global_t      *p_route_glb = NULL;
    tbl_nd_stats_t           *p_nd_stats = NULL;

    sal_memset(buf, 0, CMD_BUF_256);
    
    p_nd_fib = (tbl_nd_fib_t*)p_data;
    if (!p_nd_fib)
    {
        return;
    }
    p_nd_fib->t_timer = NULL;

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);

    p_route_glb = tbl_route_global_get_route_global();
    if (!p_route_glb)
    {
        return; 
    }
    
    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return;    
    }

    p_db_if = tbl_interface_get_interface_by_name(p_nd_fib->ifname);
    if (NULL == p_db_if)
    {
        return;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t *)p_nd_fib->ifname);
    if (NULL == p_db_rtif)
    {
        return;
    }

    if (!tbl_interface_is_running(p_db_if))
    {
        return;        
    }

    state = p_nd_fib->state;    
    
    switch (state)
    {
        case NES_INCOMPLETE:
            if (p_nd_fib->probes < MAX_MULTICAST_SOLICIT)
            {
                if (0 == p_nd_fib->route_ref)
                {
                    ND_EVENT_DEBUG("Time out for neighbor %s, remain state INCOMPLETE", buf);

                    CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                    p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                        p_db_rtif->nd_retransmit_timer);

                    p_nd_fib->probes++;
                }
                else
                {
                    CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                    p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                        ROUTE_RETRANS_TIMER);
                }
                
                ND_EVENT_DEBUG("Send MCAST probe: %d", p_nd_fib->probes);
                nd_send_ns(p_db_rtif, &p_nd_fib->key.addr.u.prefix6, FALSE, FALSE);
                p_nd_fib->opackets++;
            }
            else
            {
                ND_EVENT_DEBUG("Neighbor resolution failed for %s", buf);
                nd_neigh_delete(p_nd_fib);
            }
            break;

        case NES_REACHABLE:
            p_nd_fib->state = NES_STALE;
            tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

            ND_EVENT_DEBUG("REACH -> STALE: %s", buf);
            /* RFC 4861 7.3.2
                While stale, no action takes place until a packet is sent.
                However, reachability is not actually verified until the entry is actually used.*/
            break;

        case NES_STALE:
            p_nd_fib->state = NES_DELAY;
            tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

            ND_EVENT_DEBUG("STALE -> DELAY: %s", buf);

            CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                DELAY_FIRST_PROBE_TIME);
            p_nd_fib->probes = 0;
            break;

        case NES_DELAY:
            p_nd_fib->state = NES_PROBE;

            ND_EVENT_DEBUG("DELAY -> PROBE: %s", buf);

            CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                p_db_rtif->nd_retransmit_timer);
            goto PROBE;

        case NES_PROBE:
            ND_EVENT_DEBUG("Time out for neighbor %s,remain state PROBE", buf);

            CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                p_db_rtif->nd_retransmit_timer);

PROBE:
            p_nd_fib->probes++;
            if (p_nd_fib->probes <= MAX_UNICAST_SOLICIT)
            {
                ND_EVENT_DEBUG("Send UCAST probe: %d", p_nd_fib->probes);
                nd_send_ns(p_db_rtif, &p_nd_fib->key.addr.u.prefix6, TRUE, FALSE);
            }
            else if (p_nd_fib->probes <= MAX_UNICAST_SOLICIT + MAX_MULTICAST_SOLICIT)
            {
                ND_EVENT_DEBUG("Send MCAST probe: %d", p_nd_fib->probes);
                nd_send_ns(p_db_rtif, &p_nd_fib->key.addr.u.prefix6, FALSE, FALSE);
            }
            else
            {
                ND_EVENT_DEBUG("Neighbor probe failed for %s", buf);
                nd_neigh_delete(p_nd_fib);
            }
            break;

        default:
            p_nd_stats->error++;
            return;
    }

    return;    
}

int32
nd_neigh_update_other(void *p_data, char *p_mac, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, uint8 type)
{
    tbl_nd_fib_t              *p_nd_fib = NULL;
    char                         buf[CMD_BUF_256];
    tbl_nd_stats_t           *p_nd_stats = NULL;
    uint8                        do_update = FALSE;
    char                         old_mac[GLB_ETH_ADDR_LEN];
    char                         buf_type[8]= {0};

    sal_memset(old_mac, 0, GLB_ETH_ADDR_LEN);
    
    ND_PTR_CHECK(p_data);
    ND_PTR_CHECK(p_mac);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_FAIL;    
    }

    sal_memset(buf, 0, CMD_BUF_256);
    
    p_nd_fib = (tbl_nd_fib_t*)p_data;

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);

    if (SAL_ND_NEIGHBOR_SOLICIT == type)
    {
        sal_memcpy(buf_type, "NS", sal_strlen("NS"));
    }
    else if (SAL_ND_ROUTER_ADVERT == type)
    {
        sal_memcpy(buf_type, "RA", sal_strlen("RA"));
    }
    else if (SAL_ND_ROUTER_SOLICIT == type)
    {
        sal_memcpy(buf_type, "RS", sal_strlen("RS"));
    }
    
    ND_EVENT_DEBUG("Neigh update %s for %s", buf, buf_type);
    
    ND_EVENT_DEBUG("Neigh update %s with lladdr %.04hx.%.04hx.%.04hx for %s", buf, sal_ntoh16(((unsigned short *)p_mac)[0]),
        sal_ntoh16(((unsigned short *)p_mac)[1]), sal_ntoh16(((unsigned short *)p_mac)[2]), buf_type);

    if (p_nd_fib->flags & GLB_ND_FLAG_PERMANENT)
    {
        ND_EVENT_DEBUG("Received %s packet with source address %s, but there is static neigh cache entry", buf_type, buf);
        p_nd_stats->rcv_static_conflict++;
        return PM_E_NONE;
    }

    /*
    * newentry olladdr  lladdr   llchange     (*=record)
    *     0          n         n          --            (1)
    *     0          y         n          --            (2)
    *     0          n         y          --            (3) * STALE
    *     0          y         y          n             (4) *
    *     0          y         y          y             (5) * STALE
    *     1         --         n          --            (6)   NOSTATE(= PASSIVE)
    *     1         --         y          --            (7) * STALE
    */
   if  (!NES_IS_VALID(p_nd_fib))
    {
        ND_EVENT_DEBUG("Neighbor %s receives "
            "%s packet with source link-layer address option, change state from "
            "INCOMPLETE to STALE", buf, buf_type);
        do_update = TRUE;
    }
    else if (0 != sal_memcmp(p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN))
    {
        ND_EVENT_DEBUG("Neighbor %s receives "
            "%s packet without source link-layer address option, "
            "no state change", buf, buf_type);
        do_update = TRUE;
    }
    else
    {
        ND_EVENT_DEBUG("Valid neighbor %s receives "
            "%s packet with the same source link-layer address option", buf, buf_type);
    }

    if (do_update)
    {
        /* check old state */
        if (NES_IS_VALID(p_nd_fib))
        {
             GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE);              
             sal_memcpy(old_mac, p_nd_fib->mac_addr, GLB_ETH_ADDR_LEN);   
             nd_fea_update_nbr(p_nd_fib, p_db_if, p_mac, old_mac);
        }
        else
        {
            sal_memcpy(p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN);
            nd_fea_add_nbr(p_nd_fib, p_db_if);
        }

        if (p_nd_fib->state != NES_STALE && p_nd_fib->state < NES_STATE_MAX)
        {
            char nbstt[][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
            ND_EVENT_DEBUG("%s -> STALE: %s", nbstt[p_nd_fib->state], buf);
        }

        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);     
        p_nd_fib->state = NES_STALE;
        tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

        CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
        p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
            DELAY_TIMER);
    }

    /* No cache for NES_NONE dynamic entry */
    if (NES_NONE == p_nd_fib->state)
    {
        nd_neigh_delete(p_nd_fib);
    }

    /* set is_router flags */
    if (SAL_ND_ROUTER_SOLICIT == type)
    {
        /* is_router flag must always be cleared. */
        p_nd_fib->flags &= (~GLB_ND_FLAG_ROUTER);
    }
    else if (SAL_ND_ROUTER_ADVERT == type)
    {
        /* Mark an entry with lladdr as a router. */
        if (NES_IS_VALID(p_nd_fib))
        {
            p_nd_fib->flags |= GLB_ND_FLAG_ROUTER;
        }
    }
    else if (SAL_ND_NEIGHBOR_SOLICIT == type)
    {
        p_nd_fib->ipackets++;
    }

    return PM_E_NONE;
}

int32
nd_neigh_update_na(void *p_data, char *p_mac, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, uint8 router, uint8 solicited, uint8 override)
{
    tbl_nd_fib_t              *p_nd_fib = NULL;
    char                         buf[CMD_BUF_256];
    tbl_nd_stats_t           *p_nd_stats = NULL;
    uint8                        llchange = 0;
    char                         old_mac[GLB_ETH_ADDR_LEN];

    sal_memset(old_mac, 0, GLB_ETH_ADDR_LEN);

    ND_PTR_CHECK(p_data);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_db_if);

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_FAIL;    
    }

    sal_memset(buf, 0, CMD_BUF_256);
    
    p_nd_fib = (tbl_nd_fib_t*)p_data;

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);
    ND_EVENT_DEBUG("Neigh check whether update %s for NA", buf);

    p_nd_fib->ipackets++;

    if (p_nd_fib->flags & GLB_ND_FLAG_PERMANENT)
    {
        ND_EVENT_DEBUG("Received NA packet with source address %s, but there is static neigh cache entry", buf);
        p_nd_stats->rcv_static_conflict++;
        return PM_E_NONE;
    }

    if (NES_INCOMPLETE == p_nd_fib->state)
     {
        /* If no lladdr option came, no update. */
        if (!p_mac)
        {            
            return PM_E_NONE;
        }

        /* Record link-layer address, and update the state. */
        sal_memcpy(p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN);

        if (solicited)
        {
            ND_EVENT_DEBUG("INCMP -> REACH: %s", buf);
            p_nd_fib->state = NES_REACHABLE;
            
            /* This value should be a uniformly-distributed random value
                 between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR
                 times BaseReachableTime milliseconds. A new random value
                 should be calculated when BaseReachableTime changes
                 (due to Router Advertisements) or at least every few hours
                 even if no Router Advertisements are received. */

            if (GLB_ND_UNSPECIFIED == p_db_rtif->nd_base_reachable_time)
            {
                p_db_rtif->nd_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
            }
            else
            {
                 p_db_rtif->nd_reachable_time = p_db_rtif->nd_base_reachable_time;
            }
            
            p_db_rtif->nd_reachable_time = (sal_rand() % p_db_rtif->nd_reachable_time) +
                (p_db_rtif->nd_reachable_time >> 1);

            CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                p_db_rtif->nd_reachable_time);
        }
        else
        {
            ND_EVENT_DEBUG("INCMP -> STALE: %s", buf);
            p_nd_fib->state = NES_STALE;
            tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

            CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
            p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                DELAY_TIMER);
        }

        nd_fea_add_nbr(p_nd_fib, p_db_if);
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
    }
    else
    {
        /* Check if the link-layer address has changed or not. */
        if (NULL != p_mac)
        {
            /* new MAC */
             if (0 != sal_memcmp(p_nd_fib->mac_addr, p_mac, GLB_ETH_ADDR_LEN))
             {
                 llchange = TRUE;
             }
        }

        /*
        * This is very complex.  Look at it with care.
        *
        * override solicit lladdr llchange  action
        *                                  (L: record lladdr)
        *
        *   0         0      n        --      (2c)
        *   0         0      y        n       (2b) L
        *   0         0      y        y       (1)    REACHABLE->STALE
        *   0         1      n        --      (2c)   *->REACHABLE
        *   0         1      y        n       (2b) L *->REACHABLE
        *   0         1      y        y       (1)    REACHABLE->STALE
        *   1         0      n        --      (2a)
        *   1         0      y        n       (2a) L
        *   1         0      y        y       (2a) L *->STALE
        *   1         1      n        --      (2a)   *->REACHABLE
        *   1         1      y        n       (2a) L *->REACHABLE
        *   1         1      y        y       (2a) L *->REACHABLE
        */
        if ((!override) && llchange)
        {
            /* If state is REACHABLE, make it STALE, no other updates should be done. */
            if (NES_REACHABLE == p_nd_fib->state)
            {
                ND_EVENT_DEBUG("REACH -> STALE: %s", buf);
                p_nd_fib->state = NES_STALE;
                tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);

                CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                    DELAY_TIMER);
            }

            return PM_E_NONE;
        }
        else if (override || ((!override) && ((p_mac) && (!llchange))) || (!p_mac))
        {
            char nbstt[][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};

            /* Update link-local address, if any. */
            if ((NULL != p_mac) && (llchange))
            {
                ND_EVENT_DEBUG("Neigh update %s for NA", buf);
                
                GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE);
                
                sal_memcpy(old_mac, p_nd_fib->mac_addr, GLB_ETH_ADDR_LEN);
                nd_fea_update_nbr(p_nd_fib, p_db_if, p_mac, old_mac);
             }

            /* If solicited, make the state REACHABLE.
                If not solicited and the link-layer address was changed, make it STALE. */
            if (solicited)
            {
                if(p_nd_fib->state != NES_REACHABLE && p_nd_fib->state < NES_STATE_MAX)
                {
                    ND_EVENT_DEBUG("%s -> REACH: %s", nbstt[p_nd_fib->state], buf);
                    
                    p_nd_fib->state = NES_REACHABLE;
                    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);
                } 

                /* This value should be a uniformly-distributed random value
                 between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR
                 times BaseReachableTime milliseconds. A new random value
                 should be calculated when BaseReachableTime changes
                 (due to Router Advertisements) or at least every few hours
                 even if no Router Advertisements are received. */

                if (GLB_ND_UNSPECIFIED == p_db_rtif->nd_base_reachable_time)
                {
                    p_db_rtif->nd_reachable_time = GLB_ND_BASE_REACHABLE_TIME;
                }
                else
                {
                     p_db_rtif->nd_reachable_time = p_db_rtif->nd_base_reachable_time;
                }
                
                p_db_rtif->nd_reachable_time = (sal_rand() % p_db_rtif->nd_reachable_time) +
                    (p_db_rtif->nd_reachable_time >> 1);

                CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                    p_db_rtif->nd_reachable_time);
            }
            else if (llchange)
            {
                if(p_nd_fib->state != NES_STALE && p_nd_fib->state < NES_STATE_MAX)
                {
                    ND_EVENT_DEBUG("%s -> STALE: %s", nbstt[p_nd_fib->state], buf);

                    p_nd_fib->state = NES_STALE;
                    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_STATE);
                }

                CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                    DELAY_TIMER);
            }
        }
    }

    p_nd_fib->probes = 0;

    if (router)
    {
        GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ROUTER);
    }
    else
    {
        GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ROUTER);
    }

    return PM_E_NONE;
}

int32
neigh_match_connect (tbl_nd_fib_t   *p_nd_fib, tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc)
{
    prefix_t prefix;

    ND_PTR_CHECK(p_nd_fib);
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);  

    prefix.family = AF_INET6;
    prefix.prefixlen = IPV6_MAX_BITLEN;
    sal_memcpy(&prefix.u.prefix6, &p_nd_fib->key.addr.u.prefix6, sizeof(addr_ipv6_t));
    if (sal_prefix_match(&p_ifc->key.address, &prefix))
    {
        if ((!IN6_IS_ADDR_LINKLOCAL(&p_nd_fib->key.addr.u.prefix6)) ||
            (IN6_IS_ADDR_LINKLOCAL(&p_nd_fib->key.addr.u.prefix6) && p_nd_fib->ifindex == p_db_rtif->ifindex))
        {
            return PM_E_NONE;
        }
    }

    return PM_E_FAIL;
}

int32
neigh_ifc_down(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc, bool delete_addr)
{
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *next = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    tbl_interface_t              *p_db_if = NULL;
    char                             nbstt6[6][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
    char                             buf[CMD_BUF_256];
    char                             ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    sal_memset(buf, 0, CMD_BUF_256);

   IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
                
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    p_master= tbl_nd_fib_get_master();

    CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
    {
        if (0 == neigh_match_connect(p_nd_fib, p_db_rtif, p_ifc))
        { 
            cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);
            
            /* delete from kernel */
            if (NES_IS_ACTIVE(p_nd_fib))
            {
                nd_fea_del_nbr(p_nd_fib);               
                GLB_UNSET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE);
            }
    
            /* delete dynamic entry */
            if (!(p_nd_fib->flags & GLB_ND_FLAG_PERMANENT))
            {
                if(0 == p_nd_fib->route_ref)
                {
                    nd_neigh_delete(p_nd_fib);
                }
                else
                {
                    if (p_nd_fib->state != NES_INCOMPLETE && p_nd_fib->state < NES_STATE_MAX)
                    {
                        ND_EVENT_DEBUG("%s -> INCMP: %s", nbstt6[p_nd_fib->state], buf);
                    }
                    
                    CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                    
                    sal_memset(p_nd_fib->mac_addr, 0, GLB_ETH_ADDR_LEN);
                    p_nd_fib->state = NES_INCOMPLETE;
                }
            }
            else
            {
                if (p_nd_fib->state != NES_NONE && p_nd_fib->state < NES_STATE_MAX)
                {
                    ND_EVENT_DEBUG("%s -> INVLD: %s", nbstt6[p_nd_fib->state], buf);
                }
    
                p_nd_fib->state = NES_NONE;

                if (delete_addr && (!IPV6_PREFIX_IS_LINK_LOCAL(&p_nd_fib->key.addr)))
                {
                    p_nd_fib->ifindex = GLB_INVALID_IFINDEX;
                    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFINDEX);
                    
                    sal_memset(p_nd_fib->ifname, 0, IFNAME_SIZE);
                    tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFNAME);
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int32
neigh_ifc_up(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc)
{
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *next = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    tbl_interface_t              *p_db_if = NULL;
    char                             nbstt6[6][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
    char                             buf[CMD_BUF_256];
    char                             ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    sal_memset(buf, 0, CMD_BUF_256);

    
    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
                
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    p_master= tbl_nd_fib_get_master();

    CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
    {
        if (0 == neigh_match_connect(p_nd_fib, p_db_rtif, p_ifc))
        { 
            /* add to chip and kernel */
            if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT) &&
                (!NES_IS_DETACHED(p_nd_fib)) && (!NES_IS_ACTIVE(p_nd_fib)))
            {
                if (connected_check_ipv6(p_db_rtif, &p_nd_fib->key.addr))
                {
                    continue;    
                }
                
                /* neigh active */
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);
                ND_EVENT_DEBUG("%s -> REACH: %s", nbstt6[p_nd_fib->state], buf);
                p_nd_fib->state = NES_REACHABLE;
                nd_fea_add_nbr(p_nd_fib, p_db_if);
            }

            if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT) && (p_nd_fib->route_ref))
            {
                CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
                p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
                    DELAY_TIMER);
            }
        }
    }

    return PM_E_NONE;
}

int32
neigh_ifc_add(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc)
{
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *next = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    tbl_interface_t              *p_db_if = NULL;
    char                             ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
                
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&p_db_rtif->key);
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    p_master= tbl_nd_fib_get_master();

    CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
    {
        if (0 == neigh_match_connect(p_nd_fib, p_db_rtif, p_ifc))
        { 
            /* add to chip and kernel */
            if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT) )
            {
                sal_memcpy(p_nd_fib->ifname, p_db_if->key.name, IFNAME_SIZE);
                tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFNAME);
            
                p_nd_fib->ifindex= p_db_if->ifindex;
                tbl_nd_fib_set_nd_fib_field(p_nd_fib, TBL_ND_FIB_FLD_IFINDEX);
                
                p_nd_fib->state = NES_NONE;
            }
        }
    }

    return PM_E_NONE;
}

uint32 
get_neigh_linklocal_ifc_num(tbl_route_if_t *p_db_rtif)
{
    ds_connected_v6_t *p_db_conn_v6 = NULL;
    ctclib_slistnode_t    *rt_listnode = NULL;
    uint32                     linklocal_ifc_num = 0;
    
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn_v6, rt_listnode)
    {
        if (IN6_IS_ADDR_LINKLOCAL(&(p_db_conn_v6->key.address.u.prefix6)))
        {
            linklocal_ifc_num++;
        }
    }

    return linklocal_ifc_num;
}

int32
neigh_ifc_del(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc)
{
    struct sal_in6_addr    allnode;
    char                           ifname_ext[IFNAME_EXT_SIZE] = {'\0'};
    
    sal_memset(&allnode, 0, sizeof(struct sal_in6_addr));
    
    ND_PTR_CHECK(p_db_rtif);
    ND_PTR_CHECK(p_ifc);

    if (IN6_IS_ADDR_LINKLOCAL(&(p_ifc->key.address.u.prefix6))
       &&  (1== get_neigh_linklocal_ifc_num(p_db_rtif)))
    {
        if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON))
        {
            /* RFC 2461 section 6.2.8. If a router changes the link-local
              address for one of its interfaces, it SHOULD inform hosts
              of this change.  The router SHOULD multicast a few
              Router Advertisements from the old link-local address with
              the Router Lifetime field set to zero and also multicast a
              few Router Advertisements from the new link-local address.
              The overall effect should be the same as if one interface
              ceases being an advertising interface, and a different one
              starts being an advertising interface.*/

            IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
            ND_EVENT_DEBUG("ND api send RA with router lifetime zero on the interface %s", ifname_ext);
            sal_inet_pton (AF_INET6, ALLNODE, &allnode);
            nd_send_ra (p_db_rtif, &(p_ifc->key.address.u.prefix6), &allnode, FALSE);
        }
    }
    
    if (GLB_FLAG_ISSET (p_ifc->flags, RT_IF_ADDRV6_REAL))
    {
        rt_if_connected_invalid_ipv6(p_db_rtif, p_ifc, TRUE);
    }
    else
    {
        neigh_ifc_down(p_db_rtif, p_ifc, TRUE);  
    }
    
    GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_DAD_IN_PROGRESS);
    GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDRV6_NA_IN_PROGRESS);
    GLB_UNSET_FLAG(p_ifc->flags, RT_IF_ADDR_DUPLICATE);

    p_ifc->dad_ns_send = 0;
    p_ifc->unsolicited_na = 0;

    CTC_TASK_STOP_TIMER(p_ifc->t_timer);

    /* modified by liwh for bug 48611, 2018-09-04 */
    if (IN6_IS_ADDR_LINKLOCAL(&(p_ifc->key.address.u.prefix6))
        &&  (1== get_neigh_linklocal_ifc_num(p_db_rtif)))
    {
        nd_if_stop(p_db_rtif, FALSE);
    }
    /* liwh end */

    return PM_E_NONE;
}

int32
nd_neigh_delete_all_if_entries(uint32 ifindex)
{
    int32                           rc = PM_E_NONE;
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    ctclib_slistnode_t          *next = NULL;

    p_master= tbl_nd_fib_get_master();

    if (!p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
    {
        if ((p_nd_fib->ifindex == ifindex)  && IPV6_PREFIX_IS_LINK_LOCAL(&p_nd_fib->key.addr))
        {
            nd_neigh_delete(p_nd_fib);
        }
    }

    return rc;
}

int32
nd_neigh_route_add(tbl_nexthop_t   *pst_nexthop)
{
    tbl_nd_fib_t        nd_fib;
    tbl_nd_fib_t        *p_nd_fib = NULL;
    tbl_interface_t    *p_db_if = NULL;
    char                   buf[CMD_BUF_256];
    tbl_route_if_t      *p_db_rtif = NULL;
    char                    ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(pst_nexthop);

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(buf, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &pst_nexthop->key.u.ipv6);

    IFNAME_ETH2FULL(pst_nexthop->key.ifname, ifname_ext);
    
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(pst_nexthop->key.ifname));
    if (NULL == p_db_if)
    {
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
    if (!p_db_rtif)
    {
        ND_LOG_ERR("Interface not configured for L3 interface");
        return PM_E_FAIL;
    }
    
    nd_fib_make_key(&nd_fib.key, &pst_nexthop->key.u.ipv6, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (!p_nd_fib)
    {
        p_nd_fib = nd_neigh_create(&nd_fib, p_db_if);  
        if (!p_nd_fib)
        {
            return PM_E_FAIL;
        }
    
        p_nd_fib->state = NES_INCOMPLETE; 

        p_nd_fib->route_ref++;
        p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 
            0);
    }
    /*for bug 48791 by chenc*/
    else
    {
        p_nd_fib->route_ref++;
    }

    return PM_E_NONE;
}

int32
nd_neigh_route_del(tbl_nexthop_t   *pst_nexthop)
{
    tbl_nd_fib_t        nd_fib;
    tbl_nd_fib_t        *p_nd_fib = NULL;
    tbl_interface_t    *p_db_if = NULL;
    char                   buf[CMD_BUF_256];
    /*modified by chenc for bug 53856, 2019-10-18*/
    //tbl_route_if_t      *p_db_rtif = NULL;
    char                    ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    ND_PTR_CHECK(pst_nexthop);

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(buf, 0, CMD_BUF_256);

    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &pst_nexthop->key.u.ipv6);
    
    p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)&(pst_nexthop->key.ifname));
    if (NULL == p_db_if)
    {
        IFNAME_ETH2FULL(pst_nexthop->key.ifname, ifname_ext);
        ND_LOG_ERR("Invalid interface %s", ifname_ext);
        return PM_E_FAIL;
    }

    /*modified by chenc for bug 53856, 2019-10-18*/
    #if 0
    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
    if (!p_db_rtif)
    {
        ND_LOG_ERR("Interface not configured for L3 interface");
        return PM_E_FAIL;
    }
    #endif
    /*chenc end*/
    
    nd_fib_make_key(&nd_fib.key, &pst_nexthop->key.u.ipv6, p_db_if);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (p_nd_fib)
    {
        if(p_nd_fib->route_ref)
        {
            p_nd_fib->route_ref--;
            
            if (!p_nd_fib->route_ref &&
                !GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT) &&
                (p_nd_fib->state == NES_INCOMPLETE))
            {
                nd_neigh_delete(p_nd_fib);
            }
        }
    }

    return PM_E_NONE;
}

int32
nd_api_probe_for_fdb_age(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    addr_ipv6_t        prefix6;
    tbl_nd_fib_t       *p_nd_fib = NULL;
    tbl_nd_fib_t        nd_fib;
    char                   buf[CMD_BUF_256];
    
    ND_PTR_CHECK(p_msg);

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(buf, 0, CMD_BUF_256);

    sal_memcpy(&prefix6, p_msg->data, p_msg->data_len);

    nd_fib_make_key(&nd_fib.key, &prefix6, NULL);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (p_nd_fib) 
    {
        if (!(p_nd_fib->flags & GLB_ND_FLAG_PERMANENT))
        {
            if ((NES_REACHABLE == p_nd_fib->state) || (NES_STALE == p_nd_fib->state)
                || (NES_DELAY == p_nd_fib->state))
            {
                cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);
                
                if (NES_DELAY != p_nd_fib->state)
                {
                    char nbstt[6][6] = {"INVLD", "INCMP", "REACH", "STALE", "DELAY", "PROBE"};
                    ND_EVENT_DEBUG("%s -> DELAY: %s", nbstt[p_nd_fib->state], buf);
                }
                
                GLB_SET_FLAG(p_nd_fib->flags, GLB_ND_FLAG_FDB_AGE);
                p_nd_fib->state = NES_DELAY;
  
                p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 0);
            }    
        }
    }

    return PM_E_NONE;
}

int32
nd_api_kernel_update_nd(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_nd_fib_t       *p_nd_fib = NULL;
    tbl_nd_fib_t        nd_fib;
    char                   buf[CMD_BUF_256];
    
    ND_PTR_CHECK(p_msg);

    sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
    sal_memset(buf, 0, CMD_BUF_256);

    sal_memcpy(&nd_fib.key, p_msg->data, p_msg->data_len);
    p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
    if (p_nd_fib) 
    {
        if (NES_STALE != p_nd_fib->state)
        {
            return  PM_E_NONE;    
        }

        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);
        ND_EVENT_DEBUG("recieve a update from kernel for %s", buf);

        CTC_TASK_STOP_TIMER(p_nd_fib->t_timer);
        p_nd_fib->t_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, nd_neigh_timer_handler, p_nd_fib, 0);
    }

    return PM_E_NONE;
}

