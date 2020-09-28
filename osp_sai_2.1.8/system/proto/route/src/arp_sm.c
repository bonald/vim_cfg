#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "route.h"
#include "arp_define.h"
#include "arp_ether.h"
#include "arp_log.h"
#include "arp_debug.h"
#include "arp_api.h"
#include "arp_sm.h"
#include "prefix.h"
#include "nat_api.h"

typedef int32 (*ARP_SM_EVENT_FUNC)(tbl_arp_t *ae_p);

uint8 ARP_RETRY_TIMES   =   3;   /* retry times */
uint8 ARP_PROBE_TIMES   =   6;   /* probe times */


int32 
arp_main_probe(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    int32 rc = PM_E_NONE;

    RT_ARP_SM_DEBUG(ae_p, addr);

    if (ARP_PROBE_TIMES/2 > ae_p->retry_count)
    {
        /* send unicast probe */
        rc = arp_ether_packet_tx(ae_p, ARP_OP_REQUEST, ARP_UNICAST, FALSE);
    }
    else
    {
        /* send broadcast probe */
        rc = arp_ether_packet_tx(ae_p, ARP_OP_REQUEST, ARP_BROADCAST, FALSE);
    }

    rc = arp_main_retry_timer_add(ae_p);
    if (rc)
    {
        ae_p->status = AS_INIT;
    }

    return rc;
}

void
arp_main_retry_timeout(void* p_data)
{
    tbl_arp_t *ae_p = (tbl_arp_t*)p_data;
    uint8 state = AS_INIT;

    state = ae_p->status;
    if (state >= AS_MAX)
    {
        return;
    }
    
    ae_p->retry_timer = NULL;
    arp_sm(ae_p, AE_RETRY_EXP);
    return;
}

void
arp_main_gra_rsp_timeout(void* p_data)
{
    tbl_arp_t   *ae_p = (tbl_arp_t*)p_data;
    uint8       state = AS_INIT;

    state = ae_p->status;
    if (state >= AS_MAX)
    {
        return;
    }
    
    ae_p->gra_rsp_timer = NULL;
    return;
}

int32
arp_main_retry_timer_add(tbl_arp_t *ae_p)
{
    uint32      time = 0;
    int32       rc = PM_E_NONE;
    
    if ((ae_p->route_ref || ae_p->is_bfd_ref) && ae_p->status == AS_INCOMPLETE)
    {
        time = ROUTE_ARP_RETRY_TIME;
    }
    else
    {
        rc = arp_main_get_retry_time(ae_p->ifindex, &time);
    }

    if (rc)/* if time get fail, set to default value */
    {
        time = ARP_RETRY_TIME_DEFAULT;
    }

    /* if not retry, wait for 3 seconds */
    if (0 == time)
    {
        if (0 == ae_p->retry_count)
            time = ARP_RETRY_TIME_DEFAULT*3;
        else
            return PM_E_FAIL;
    }
  
    if (NULL == ae_p->retry_timer)
    {
        ae_p->retry_timer = ctc_task_add_timer(TASK_PRI_NORMAL, arp_main_retry_timeout, (void*)ae_p, time);
    }

    return PM_E_NONE;  
}

int32
arp_main_gra_rsp_timer_add(tbl_arp_t *ae_p)
{
    int32 time = ARP_GRA_RSP_TIME_DEFAULT;
  
    if (NULL == ae_p->gra_rsp_timer)
    {
        arp_ether_packet_tx(ae_p, ARP_OP_REPLY, ARP_BROADCAST, TRUE);
        ae_p->gra_rsp_timer = ctc_task_add_timer(TASK_PRI_NORMAL, arp_main_gra_rsp_timeout, (void*)ae_p, time);
    }

    return PM_E_NONE;  
}

int32
arp_main_get_retry_time(uint32 ifindex, uint32 *time)
{
    tbl_route_if_key_t key;
    tbl_route_if_t *p_rt_if = NULL;
    int32 rc = PM_E_NONE;


    rc = tbl_interface_get_name_by_ifindex(ifindex, key.name, IFNAME_SIZE);
    if (rc < 0)
    {
        return rc;
    }

    p_rt_if = tbl_route_if_get_route_if(&key);
    if (NULL == p_rt_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    *time = p_rt_if->arp_retrans_time;

    return PM_E_NONE;
}

int32
arp_main_get_aging_time(uint32 ifindex, uint32 *time)
{
    tbl_route_if_key_t key;
    tbl_route_if_t *p_rt_if = NULL;
    int32 rc = PM_E_NONE;


    rc = tbl_interface_get_name_by_ifindex(ifindex, key.name, IFNAME_SIZE);
    if (rc < 0)
    {
        return rc;
    }

    p_rt_if = tbl_route_if_get_route_if(&key);
    if (NULL == p_rt_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    *time = p_rt_if->arp_timeout;

    return PM_E_NONE;
}

int32
arp_main_is_need_rep(uint32 ifindex, arp_hdr_t *ah, uint8 is_gratuitous)
{
    tbl_route_if_key_t  key;
    tbl_arp_t           *ae_p = NULL;
    tbl_route_if_t      *p_rt_if = NULL;
    char                *p = NULL;
    addr_ipv4_t         dst_addr;
    tbl_interface_t     *p_db_if = NULL;
    ds_connected_t      *dst_ifc = NULL;
    char buf[CMD_BUF_32];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    /* get the dst prefix */
    sal_memset(buf, 0, sizeof(buf));
    p = (char*)(ah+1);
    p += 2*ah->ar_hln + ah->ar_pln;
    sal_memcpy(&dst_addr, p, sizeof(dst_addr));

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return FALSE;
    }

    /*zhujq for bug 15042 in 2011-06-16*/
    dst_ifc = rt_if_lookup_ifc_ipv4(p_db_if, &dst_addr);
    if (dst_ifc)
    {
        if (TRUE == is_gratuitous)
        {
            ARP_LOG_USER(E_WARNING, ARP_4_DUPLICATE_IP_ADDR, LOG_RATE_LIMIT_TYPE_DEF, 
                cdb_addr_ipv4_val2str(buf, CMD_BUF_32, &dst_addr), IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        return TRUE;
    }

    /* modified by wangjj for arp proxy, 2016-11-11 */
    sal_memset(&key, 0x0, sizeof(tbl_route_if_key_t));
    sal_memcpy(key.name, p_db_if->key.name, sizeof(key.name));
    p_rt_if = tbl_route_if_get_route_if(&key);
    if (NULL == p_rt_if)
    {
        return FALSE;
    }

    if (FALSE == p_rt_if->local_arp_proxy_en
     && FALSE == p_rt_if->arp_proxy_en
     &&(FALSE == nat_api_if_need_proxy_arp(p_rt_if, &dst_addr)))/* modified by taocy for static NAT proxy inside global IP, 2019-06-03*/
    {
        ae_p = arp_db_entry_lookup(&dst_addr);
        if (!ae_p)
        {
            return FALSE;
        }

        /* target ip is not in the same subnet with arp entry [R-ARP-1-0030]*/
        if (ae_p->ifindex != ifindex)
        {
            return FALSE;
        }

        if (TRUE == ae_p->is_router || (TRUE != ae_p->is_router && TRUE == ae_p->is_proxy))
        {
            return TRUE;
        }
    }
    else
    {
        tbl_iproute_node_master_t *p_master = tbl_iproute_node_get_master();
        ctclib_slistnode_t  *listnode = NULL;
        ctclib_slistnode_t  *next = NULL;
        ds_connected_t      *src_ifc = NULL;
        tbl_iproute_node_t  *p_db_rn = NULL;
        uint32              i    = 0;
        prefix_t            q;
        addr_ipv4_t src_addr;
        addr_ipv4_t mask_addr;

        /*get ifc for src ip*/
        p = (char*)(ah+1);
        p += ah->ar_hln;
        sal_memcpy((char*)&src_addr, p, sizeof(src_addr));
        arp_api_get_rt_ifc(p_db_if, &src_addr, &src_ifc);
        sal_masklen2ip(src_ifc->key.address.prefixlen, &mask_addr);

        /*should include remote route and connect route*/
        sal_memset(&q, 0x0, sizeof(q));
        q.family = AF_INET;
        q.prefixlen = IPV4_MAX_PREFIXLEN;
        q.u.prefix4 = dst_addr;

        if (NULL == p_master)
        {
            return FALSE;
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->rn_list, p_db_rn, listnode, next)
        {
            if (!arp_api_fib_match(&q, p_db_rn))
            {
                continue;
            }

            if (!p_db_rn->multipath)
            {
                /*a. src ip and dst ip not in the same subnet and arp proxy is enable*/
                /*for secondary ip address, we follow h3c's implementation: 
                cisco:
                1.dst ifp != src_ifp, must enable arp proxy;
                2.dst ifp ==src ifp, must enable local arp proxy; if(dst ip subnet!=src ip subnet),must also enable
                  normal arp proxy. ((dst_addr.s_addr & mask_addr.s_addr) != (src_ifc->address->u.prefix4.s_addr & mask_addr.s_addr))
                h3c:
                1. if (src_ifp == dst_ifp), then local arp proxy must enabled
                2. if (src ifp != dst ifp), then arp proxy must enabled*/
                if (0 != sal_memcmp(p_db_rn->nh_key.ifname, p_rt_if->key.name, sizeof(p_rt_if->key.name)))
                {
                    if (p_rt_if->arp_proxy_en)
                        return TRUE;
                    else
                        continue;
                }
                /*b. else src ip and dst ip in the same subnet and dst ip is recv interface's self address
                 or src ip and dst ip in the same subnet, and local arp proxy is enable*/
                else if (IPV4_ADDR_SAME(&src_ifc->key.address.u.prefix4, &dst_addr)
                      || p_rt_if->local_arp_proxy_en
                      || nat_api_if_need_proxy_arp(p_rt_if, &dst_addr))/* modified by taocy for static NAT proxy inside global IP, 2019-06-03*/
                {
                    return TRUE;
                }
                /*c. else return FALSE*/
                else
                {
                    continue;
                }
            }
            else
            {
                for (i = 0; i < p_db_rn->nh_group.nh_num; i++)
                {
                    /*a. src ip and dst ip not in the same subnet and arp proxy is enable*/
                    /*for secondary ip address, we follow h3c's implementation:
                    cisco:
                    1.dst ifp != src_ifp, must enable arp proxy;
                    2.dst ifp ==src ifp, must enable local arp proxy; if(dst ip subnet!=src ip subnet),must also enable
                      normal arp proxy. ((dst_addr.s_addr & mask_addr.s_addr) != (src_ifc->address->u.prefix4.s_addr & mask_addr.s_addr))
                    h3c:
                    1. if (src_ifp == dst_ifp), then local arp proxy must enabled
                    2. if (src ifp != dst ifp), then arp proxy must enabled*/
                    if (0 != sal_memcmp(p_db_rn->nh_group.nh[i].ifname, p_rt_if->key.name, sizeof(p_rt_if->key.name)))
                    {
                        if (p_rt_if->arp_proxy_en)
                            return TRUE;
                        else
                            continue;
                    }
                    /*b. else src ip and dst ip in the same subnet and dst ip is recv interface's self address
                     or src ip and dst ip in the same subnet, and local arp proxy is enable*/
                    else if (IPV4_ADDR_SAME(&src_ifc->key.address.u.prefix4, &dst_addr)|| p_rt_if->local_arp_proxy_en)
                    {
                        return TRUE;
                    }
                    /*c. else return FALSE*/
                    else
                    {
                        continue;
                    }
                }
            }
        }
    }

    return FALSE;
}

int32
arp_main_aging_time_decr(tbl_arp_t *p_db_arp, void *p_data, void *p_data1)
{
    if (ARP_TYPE_STATIC == p_db_arp->type)
    {
        return PM_E_NONE;
    }
    
    if (AS_REACHABLE != p_db_arp->status)
    {
        return PM_E_NONE;
    }
    
    if (1 <= p_db_arp->aging_delay)
    {
        p_db_arp->aging_delay--;
    }

    if (0 == p_db_arp->aging_delay)
    {
        /* if aging delay decr to zero, aging timeout */
        p_db_arp->retry_count = 0;
        arp_sm(p_db_arp, AE_AGING_EXP);
    }

    return PM_E_NONE;
}

void
arp_main_aging_timeout(void* p_data)
{
    tbl_route_global_t *p_rt_glb = (tbl_route_global_t*)p_data;

    p_rt_glb->arp_aging_timer = NULL;
    
    arp_api_trav(arp_main_aging_time_decr, NULL, NULL);
    
    p_rt_glb->arp_aging_timer = ctc_task_add_timer(TASK_PRI_NORMAL, arp_main_aging_timeout,
        p_rt_glb, ROUTE_ARP_AGING_SCAN_INTERVAL);
    return;
}

static int32
arp_handle_not_possible(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    
    RT_ARP_SM_DEBUG(ae_p, addr);

    return PM_E_NONE;
}

static int32 
arp_handle_arp_miss(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    int32 rc = PM_E_NONE;

    RT_ARP_SM_DEBUG(ae_p, addr);

    /* send broadcast arp request */
    rc = arp_ether_packet_tx(ae_p, ARP_OP_REQUEST, ARP_BROADCAST, FALSE);

    /* start retry timer */
    rc = arp_main_retry_timer_add(ae_p);
    if (rc)
    {
        ae_p->status = AS_INIT;
    }
    
    return rc;
}

static int32 
arp_handle_gratuitous(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];

    RT_ARP_SM_DEBUG(ae_p, addr);

    arp_ether_packet_tx(ae_p, ARP_OP_REQUEST, ARP_BROADCAST, TRUE);

    return PM_E_NONE;
}

static int32 
arp_handle_do_nothing(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    RT_ARP_SM_DEBUG(ae_p, addr);

    return PM_E_NONE;
}

static int32 
arp_handle_request(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    uint32 time = 0;
    arp_hdr_t *ah = NULL;
    uint8 need_rep = FALSE;
    
    RT_ARP_SM_DEBUG(ae_p, addr);

    SAL_ASSERT(ae_p->rx_buf);
    arp_pkt_move_pld(ae_p->rx_buf, ah);

    /*modified by ychen in 2011-10-08, move from after code arp_ether_lladdr_update() to here*/
    need_rep = arp_main_is_need_rep(ae_p->ifindex, ah, ae_p->is_gratuitous);   
    if (FALSE == need_rep && FALSE == ae_p->is_gratuitous)
    {
        return PM_E_NONE;
    }
    
    if (ARP_TYPE_DYNAMIC == ae_p->type)
    {
        /* arp request is received and stop retry and refresh if necessary */
        if (ae_p->retry_timer)
        {
            ctc_task_delete_timer(ae_p->retry_timer);
            ae_p->retry_timer = NULL;
        }

        ae_p->retry_count = 0;
    
        if (ae_p->refresh_timer)
        {
            ctc_task_delete_timer(ae_p->refresh_timer);
            ae_p->refresh_timer = NULL;
        }
    
        /* if interface aging time retrieve error, use the default value */
        if (arp_main_get_aging_time(ae_p->ifindex, &time))
        {
            time = ARP_AGING_TIME_DEFAULT;
        }
    
        /* if time is invalid, use the default value */
        if (0 == time)
        {
            time = ARP_AGING_TIME_DEFAULT;
        }
    
        ae_p->status = AS_REACHABLE;
        ae_p->aging_delay = time;
    }

    switch (sal_hton16(ah->ar_hrd))
    {
    case ARP_HRD_ETHER:
        /* the static entry should not be updated by dynamic one */
        if (ARP_TYPE_DYNAMIC == ae_p->type && !(ae_p->flags & ARP_DONOT_INSTALL))
        {
            arp_ether_lladdr_update(ae_p, ((arp_eth_t*)ah)->ar_sha);
        }
        break;
        
    default:
        break;
    }   

    /*ychen added in 2011-10-09 for gratuitous arp process*/
    if (FALSE == need_rep)
    {
        return PM_E_NONE;
    }

    /* we will not response to gratuitous arp reponse,but need reply request packet */
    if (TRUE == ae_p->is_gratuitous)
    {
        if (ARP_OP_REQUEST == sal_hton16(ah->ar_op))
        {            
            arp_main_gra_rsp_timer_add(ae_p);
        }
        
        return PM_E_NONE;
    }   
    
    /* send reply */
    switch(sal_hton16(ah->ar_hrd))
    {
    case ARP_HRD_ETHER:
        arp_ether_packet_tx(ae_p, ARP_OP_REPLY, ARP_UNICAST, FALSE);
        break;
        
    default:
        break;
    }

    return PM_E_NONE;
}

static int32 
arp_handle_response(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    uint32 time = 0;
    arp_hdr_t *ah = NULL;

    RT_ARP_SM_DEBUG(ae_p, addr);

    SAL_ASSERT(ae_p->rx_buf);
    arp_pkt_move_pld(ae_p->rx_buf, ah);

    /* arp reply is received and stop retry and refresh */
    if (ae_p->retry_timer)
    {
        ctc_task_delete_timer(ae_p->retry_timer);
        ae_p->retry_timer = NULL;
    }
    
    ae_p->retry_count = 0;

    if (ae_p->refresh_timer)
    {
        ctc_task_delete_timer(ae_p->refresh_timer);
        ae_p->refresh_timer = NULL;
    }
    
    ae_p->status = AS_REACHABLE;
    /* if interface aging time retrieve error, use the default value */
    if (arp_main_get_aging_time(ae_p->ifindex, &time))
    {
        time = ARP_AGING_TIME_DEFAULT;
    }
    
    /* if time is invalid, use the default value */
    if (0 == time)
    {
        time = ARP_AGING_TIME_DEFAULT;
    }

    ae_p->aging_delay = time;

    /* update arp cache */
    switch(sal_hton16(ah->ar_hrd))
    {
    case ARP_HRD_ETHER:
        arp_ether_lladdr_update(ae_p, ((arp_eth_t*)ah)->ar_sha);
        break;
        
    default:
        break;
    }

    return PM_E_NONE;
}

static int32 
arp_handle_retry_timeout_refsh(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    uint32 time = 0;
    int32 rc = PM_E_NONE;

    RT_ARP_SM_DEBUG(ae_p, addr);

    /* increase attempts */
    ae_p->retry_count ++;

    /* retry all times and no response, remove the entry */
    if (ae_p->retry_count >= ARP_PROBE_TIMES)
    {
        ae_p->status = AS_INIT;
        ae_p->retry_count = 0;

        return PM_E_NONE;
    }

    rc = arp_main_get_retry_time(ae_p->ifindex, &time);
    if (rc || 0 == time)/* if time get fail, or set zero, do nothing */
    {
        ae_p->status = AS_INIT;
        ae_p->retry_count = 0;

        return PM_E_NONE;
    }
    
    arp_main_probe(ae_p);

    return rc;
}

static int32 
arp_handle_retry_timeout_incmp(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    uint32 time = 0;
    int32 rc = PM_E_NONE;

    RT_ARP_SM_DEBUG(ae_p, addr);

    /* increase attempts */
    if(ae_p->route_ref == 0 && FALSE == ae_p->is_bfd_ref)
    {
        ae_p->retry_count++;
    }

    /* retry all times and no response, remove the entry */
    if (ae_p->retry_count >= ARP_RETRY_TIMES)
    {
        ae_p->status = AS_INIT;
        ae_p->retry_count = 0;
        return PM_E_NONE;
    }

    if(ae_p->route_ref || ae_p->is_bfd_ref)
    {
        time = ROUTE_ARP_RETRY_TIME;
    }
    else
    {
        rc = arp_main_get_retry_time(ae_p->ifindex, &time);
    }
    
    if (rc || 0 == time)/* if time get fail, or set zero, do nothing */
    {
        ae_p->status = AS_INIT;
        ae_p->retry_count = 0;

        return PM_E_NONE;
    }

    arp_handle_arp_miss(ae_p);

    return rc;
}

static int32
arp_handle_remove_entry(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    
    RT_ARP_SM_DEBUG(ae_p, addr);

    ae_p->status = AS_INIT;

    return PM_E_NONE;
}

static int32
arp_handle_aging_timeout(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    int32 rc = PM_E_NONE;
    
    RT_ARP_SM_DEBUG(ae_p, addr);

    ae_p->status = AS_REFRESH;
    
    rc = arp_main_probe(ae_p);

    return rc;
}

static int32 
arp_handle_refresh_timeout(tbl_arp_t *ae_p)
{
    char addr[INET_ADDRSTRLEN];
    
    RT_ARP_SM_DEBUG(ae_p, addr);

    /* init the state to remove */
    ae_p->status = AS_INIT;

    return PM_E_NONE;
}

/* state machine */
static const ARP_SM_EVENT_FUNC g_arp_sm_dispatch_func[AE_MAX][AS_MAX] =
{
    /*  @@@@@@@@@@@@@@@@@@     AE_TX_ARP_REQ        @@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_not_possible,                    /* 0 AS_INIT       */
        arp_handle_arp_miss,                        /* 1 AS_INCOMPLETE */
        arp_handle_not_possible,                    /* 2 AS_REACHABLE  */
        arp_handle_not_possible,                    /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@@     AE_TX_GARP_REQ       @@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_not_possible,                    /* 0 AS_INIT       */
        arp_handle_not_possible,                    /* 1 AS_INCOMPLETE */
        arp_handle_gratuitous,                      /* 2 AS_REACHABLE  */
        arp_handle_not_possible,                    /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@      AE_RX_ARP_RSP        @@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_do_nothing,                      /* 0 AS_INIT       */
        arp_handle_response,                        /* 1 AS_INCOMPLETE */
        arp_handle_do_nothing,                      /* 2 AS_REACHABLE  */
        arp_handle_response,                        /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@      AE_RX_ARP_REQ        @@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_request,                         /* 0 AS_INIT       */
        arp_handle_request,                         /* 1 AS_INCOMPLETE */
        arp_handle_request,                         /* 2 AS_REACHABLE  */
        arp_handle_request,                         /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@      AE_RETRY_EXP         @@@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_not_possible,                    /* 0 AS_INIT       */
        arp_handle_retry_timeout_incmp,             /* 1 AS_INCOMPLETE */
        arp_handle_not_possible,                    /* 2 AS_REACHABLE  */
        arp_handle_retry_timeout_refsh,             /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@      AE_AGING_EXP         @@@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_do_nothing,                      /* 0 AS_INIT       */
        arp_handle_not_possible,                    /* 1 AS_INCOMPLETE */
        arp_handle_aging_timeout,                   /* 2 AS_REACHABLE  */
        arp_handle_not_possible,                    /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@      AE_REFRESH_EXP       @@@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_not_possible,                    /* 0 AS_INIT       */
        arp_handle_not_possible,                    /* 1 AS_INCOMPLETE */
        arp_handle_not_possible,                    /* 2 AS_REACHABLE  */
        arp_handle_refresh_timeout,                 /* 3 AS_REFRESH    */
    },

    /*  @@@@@@@@@@@@@@@@@@     AE_DELETE_ENTRY      @@@@@@@@@@@@@@@@@@@@@@ */
    {
        arp_handle_remove_entry,                      /* 0 AS_INIT       */
        arp_handle_remove_entry,                      /* 1 AS_INCOMPLETE */
        arp_handle_remove_entry,                      /* 2 AS_REACHABLE  */
        arp_handle_remove_entry,                      /* 3 AS_REFRESH    */
    }
};

int32
arp_sm(tbl_arp_t *ae_p, const arp_event_t event)
{
    uint8 state = AS_INIT;
    int32 rc = PM_E_NONE;

    state = ae_p->status;

    if (state >= AS_MAX)
    {
        return PM_E_FAIL;
    }

    rc = g_arp_sm_dispatch_func[event][state](ae_p);

    if (ae_p->rx_buf)
    {
        ae_p->rx_buf = NULL;
    }
    
    if (AS_INIT == ae_p->status)
    {
        /* stop the timer */
        if (ae_p->refresh_timer)
        {
            ctc_task_delete_timer(ae_p->refresh_timer);
            ae_p->refresh_timer = NULL;
        }
        if (ae_p->retry_timer)
        {
            ctc_task_delete_timer(ae_p->retry_timer);
            ae_p->retry_timer = NULL;
        }
        if (ae_p->gra_rsp_timer)
        {
            ctc_task_delete_timer(ae_p->gra_rsp_timer);
            ae_p->gra_rsp_timer = NULL;
        }
        
        /* clear the garbage arp of kernel */
        arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);

        if (ae_p->route_ref || ae_p->is_bfd_ref)
        {
            ae_p->type = ARP_TYPE_DYNAMIC;
            ae_p->is_router = FALSE;
            sal_memset(&ae_p->mac_addr, 0, MAC_ADDR_LEN);
            ae_p->uptime = ctc_time_boottime_sec(NULL);
            ae_p->status = AS_INCOMPLETE;
            
            arp_sm(ae_p, AE_RETRY_EXP);
        }
        else
        {
            /* Remove the ARP entry from local */            
            arp_db_entry_remove(ae_p);
        }
    }
    
    return rc;
}

