
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
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "rt_fea.h"
#include "route.h"
#include "nd.h"
#include "nd_api.h"
#include "rt_if.h"
#include "lib_sysctl.h"
#include "hsrv_msg.h"

static int32
_nd_if_set_hop_limit(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    uint32                      hop_limit = 0;
    tbl_route_if_t            *p_db_rtif = NULL;
    
    if (!p_db_if)
    {
        return PM_E_NONE;
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return PM_E_NONE;
    }

    p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
    if (!p_db_rtif)
    {   
        return PM_E_NONE;
    }

    /* set interface hop limit to kernel */
    hop_limit = *(uint32*)pargs->argv[0];
    lib_sysctl_nd_set_if_hop_limit((char*)p_db_rtif->key.name, hop_limit);
    
    return PM_E_NONE;
}
            
int32 nd_hop_limit_set(uint32 hop_limit, cfg_cmd_para_t *para)
{
    tbl_route_global_t   *p_db_rt_glb = tbl_route_global_get_route_global();
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));

    /* set default hop limit to kernel */
    lib_sysctl_nd_set_default_hop_limit(hop_limit);

    if (p_db_rt_glb)
    {
        p_db_rt_glb->nd_hop_limit = hop_limit;
        tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT);
    }

    /* set hop limit to all interface */
    args.argv[0] = &hop_limit;
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_nd_if_set_hop_limit, &args);

    return PM_E_NONE;
}

int32
nd_ra_hop_limit_set(tbl_route_if_t *p_db_rtif, uint32 current_hop_limit, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_ra_currrent_hop_limit != current_hop_limit)
    {
        p_db_rtif->nd_ra_currrent_hop_limit = current_hop_limit;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_CURRRENT_HOP_LIMIT));

        /* send ra again */
        nd_if_rtadv_reset(p_db_rtif);
    }
  
    return rc;
}

int32
nd_ra_advertise_set(tbl_route_if_t *p_db_rtif, uint32 advertise, cfg_cmd_para_t *para)
{
    int32                         rc = PM_E_NONE;
    ds_connected_v6_t    *p_ifc = NULL; 
    struct sal_in6_addr    allnode;
    char                           ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    sal_memset(&allnode, 0, sizeof(struct sal_in6_addr));

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }
    
    if (p_db_rtif->nd_ra_advertise != advertise)
    {
        if (advertise)
        {
            if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_ENABLE))
            {
                nd_if_rtadv_start(p_db_rtif);
            }
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_rtif->nd_flags, ND_IF_RTADV_ON))
            {
                p_ifc = if_lookup_ifc_ipv6_linklocal(p_db_rtif);
                if (p_ifc)
                {
                    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
                    ND_EVENT_DEBUG("ND api send RA with router lifetime zero on the interface %s", ifname_ext);
        
                    sal_inet_pton (AF_INET6, ALLNODE, &allnode);
                    nd_send_ra(p_db_rtif, &(p_ifc->key.address.u.prefix6), &allnode, FALSE);
                }
            }
        
            nd_if_rtadv_stop(p_db_rtif); 
        }

        p_db_rtif->nd_ra_advertise = advertise;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE));
    }
  
    return rc;
}

int32
nd_ra_advertise_mtu_set(tbl_route_if_t *p_db_rtif, uint32 advertise_mtu, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_ra_advertise_mtu != advertise_mtu)
    {
        p_db_rtif->nd_ra_advertise_mtu = advertise_mtu;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_ADVERTISE_MTU));
    }
  
    return rc;
}

int32
nd_ra_interval_set(tbl_route_if_t *p_db_rtif, uint32 ra_interval_max, uint32 ra_interval_min, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;
    uint32         max_ms = 0;
    uint32         min_ms = 0;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    max_ms = ra_interval_max * 1000;
    if (0 != ra_interval_min)
    {
        min_ms = ra_interval_min * 1000;
        if (ra_interval_max >= RTR_ADV_INTERVAL_THRESHOLE)
        {
            if (min_ms > RTR_ADV_INTERVAL_FACTOR_MAX * max_ms)
            {
                CFG_INVALID_PARAM_RET("RA min-interval can't be greater than 0.75 * max-interval");
            }
        }
        else
        {
            if (min_ms > max_ms)
            {
                CFG_INVALID_PARAM_RET("RA min-interval can't be greater than max-interval");
            }
        }
    }
    else
    {
        min_ms = (ra_interval_max >= RTR_ADV_INTERVAL_THRESHOLE) ? max_ms * RTR_ADV_INTERVAL_FACTOR : max_ms;
    }

    if ((p_db_rtif->nd_ra_interval_max != max_ms) 
        || ((0 != ra_interval_min) &&  (p_db_rtif->nd_ra_interval_min != ra_interval_min)))
    {
        if ((0 != p_db_rtif->nd_router_lifetime) && (p_db_rtif->nd_router_lifetime * 1000 < max_ms)
            && ((p_db_rtif->nd_ra_interval_max * 3 /1000) != p_db_rtif->nd_router_lifetime))
        {
            CFG_INVALID_PARAM_RET("RA lifetime can't be less than RA interval");
        }

        if ((p_db_rtif->nd_ra_interval_max * 3 /1000) == p_db_rtif->nd_router_lifetime)
        {
            p_db_rtif->nd_router_lifetime = max_ms * 3 / 1000;
            PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_ROUTER_LIFETIME));
        }

        p_db_rtif->nd_ra_interval_max = max_ms;
        p_db_rtif->nd_ra_interval_min = min_ms;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_INTERVAL_MAX));
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_INTERVAL_MIN));
        
        nd_if_rtadv_chage_ra_interval(p_db_rtif);
    }
    
    return rc;
}

int32
nd_ra_lifetime_set(tbl_route_if_t *p_db_rtif, uint32 lifetime, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_router_lifetime != lifetime) 
    {
        if ((0 != lifetime) && (lifetime * 1000 < p_db_rtif->nd_ra_interval_max))
        {
            CFG_INVALID_PARAM_RET("RA lifetime can't be less than RA interval");
        }

        p_db_rtif->nd_router_lifetime = lifetime;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_ROUTER_LIFETIME));
        
        nd_if_rtadv_reset(p_db_rtif);
    }
    
    return rc;
}

int32
nd_dad_attempts_set(tbl_route_if_t *p_db_rtif, uint32 dad_attempts, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_dad_attempts != dad_attempts) 
    {
        lib_sysctl_nd_set_if_dad_attempts(p_db_rtif->key.name, dad_attempts);
            
        p_db_rtif->nd_dad_attempts = dad_attempts;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_DAD_ATTEMPTS));
    }
    
    return rc;
}

int32
nd_ns_interval_set(tbl_route_if_t *p_db_rtif, uint32 ns_interval, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_retransmit_timer != ns_interval) 
    {
        /* set ns interval to kernel */
        lib_sysctl_nd_set_if_ns_interval(p_db_rtif->key.name, ns_interval);

        /* reserver retransmit */
        p_db_rtif->nd_retransmit_timer = ns_interval;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RETRANSMIT_TIMER));

        /* send ra again */
        nd_if_rtadv_reset(p_db_rtif);
    }
    
    return rc;
}

int32
nd_reachable_time_set(tbl_route_if_t *p_db_rtif, uint32 config_reachable_time, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;
    uint32         reachable_time = 0;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (GLB_ND_UNSPECIFIED == config_reachable_time)
    {
        reachable_time = GLB_ND_BASE_REACHABLE_TIME;
    }
    else
    {
        reachable_time = config_reachable_time;
    }

    if (p_db_rtif->nd_reachable_time != reachable_time) 
    {
        /* set ns interval to kernel */
        lib_sysctl_nd_set_if_reachable_time(p_db_rtif->key.name, reachable_time);
        
        p_db_rtif->nd_reachable_time = reachable_time;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_REACHABLE_TIME));

        p_db_rtif->nd_base_reachable_time = reachable_time;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_BASE_REACHABLE_TIME));

        /* send ra again */
        nd_if_rtadv_reset(p_db_rtif);
   }
    
    return rc;
}

int32
nd_managed_config_set(tbl_route_if_t *p_db_rtif, uint32 managed_config, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    if (p_db_rtif->nd_managed_config != managed_config) 
    {
        p_db_rtif->nd_managed_config = managed_config;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_MANAGED_CONFIG));

        /* expedite RA */
        nd_if_rtadv_reset(p_db_rtif);
   }
    
    return rc;
}

int32
nd_other_config_set(tbl_route_if_t *p_db_rtif, uint32 other_config, cfg_cmd_para_t *para)
{
    int32           rc = PM_E_NONE;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }
    
    if (p_db_rtif->nd_other_config != other_config) 
    {
        p_db_rtif->nd_other_config = other_config;
        PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_OTHER_CONFIG));

        /* expedite RA */
        nd_if_rtadv_reset(p_db_rtif);
   }
    
    return rc;
}

int32
nd_prefix_default_set(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                          rc = PM_E_NONE;
    uint8                           i = 0;
    uint32                         vlifetime = GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME;
    uint32                         plifetime = GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME;
    uint32                         onlink = TRUE;
    uint32                         auto_is_set = TRUE;
    ctclib_slistnode_t         *rt_listnode = NULL;
    ds_nd_prefix_t             *p_nd_prefix = NULL;

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    for (i = 1;  i < argc; i++)
    {
        if (!sal_strcmp(argv[i], "vlifetime"))
        {
            if (!sal_strcmp(argv[i+1], "infinite"))
            {
                vlifetime = 0xffffffff;
            }
            else
            {
                PM_CFG_GET_UINT(vlifetime, argv[i+1], para->p_rs);
            }
            
            i++;
        }
        else if (!sal_strcmp(argv[i], "vpreferredtime"))
        {
            if (!sal_strcmp(argv[i+1], "infinite"))
            {
                plifetime = 0xffffffff;
            }
            else
            {
                PM_CFG_GET_UINT(plifetime, argv[i+1], para->p_rs);
            }

            i++;
        }
        else if (!sal_strcmp(argv[i], "offlink"))
        {
            onlink = FALSE;
        }
        else if (!sal_strcmp(argv[i], "noautoconfig"))
        {
            auto_is_set = FALSE;
        }  
    }

    if(plifetime > vlifetime)
    {
        CFG_INVALID_PARAM_RET("Preferred lifetime must not exceed valid lifetime\n");
    }

    p_db_rtif->nd_ra_prefix_default_vlifetime = vlifetime;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_VLIFETIME));

    p_db_rtif->nd_ra_prefix_default_plifetime = plifetime;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_PLIFETIME));

    p_db_rtif->nd_ra_prefix_default_onlink = onlink;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_ONLINK));

    p_db_rtif->nd_ra_prefix_default_auto = auto_is_set;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ND_RA_PREFIX_DEFAULT_AUTO));

    if (p_db_rtif->nd_prefix.obj_list && p_db_rtif->nd_prefix.obj_list->count)
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->nd_prefix.obj_list, p_nd_prefix, rt_listnode)
        {
            if (!p_nd_prefix->config)
            {
                p_nd_prefix->vlifetime = vlifetime;
                p_nd_prefix->plifetime = plifetime;
                p_nd_prefix->onlink = onlink;
                p_nd_prefix->autoconfig = auto_is_set;
            }
        }
    }

    /* expedite RA */
    nd_if_rtadv_reset(p_db_rtif);

    return rc;
}

int32
nd_prefix_delete_all_entry(tbl_route_if_t *p_db_rtif)
{
    ds_nd_prefix_t              *p_db_nd_prefix = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_db_rtif->nd_prefix.obj_list, p_db_nd_prefix, listnode, next)
    {
        ds_nd_prefix_del_nd_prefix(p_db_rtif, p_db_nd_prefix);
    }

    return PM_E_NONE;
}

int32
nd_prefix_add_entry(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;
    uint8                    i = 0;
    uint32                  vlifetime = GLB_ND_DEFAULT_PREFIX_VALID_LIFETIME;
    uint32                  plifetime = GLB_ND_DEFAULT_PREFIX_PREFERED_LIFETIME;
    uint32                  onlink = TRUE;
    uint32                  auto_is_set = TRUE;
    prefix_t                addr;
    ds_nd_prefix_t      nd_prefix;
     ds_nd_prefix_t     *p_nd_prefix = NULL;

    sal_memset(&addr, 0, sizeof(prefix_t));
    sal_memset(&nd_prefix, 0, sizeof(ds_nd_prefix_t));

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    /* get IPv6 address */
    rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("Invalid IPv6 address");
    }    
    addr.family = AF_INET6;
    PM_CFG_GET_UINT(addr.prefixlen, argv[i+1], para->p_rs);

    /* IPv6 address check */
    if (IPV6_PREFIX_IS_LINK_LOCAL(&addr))
    {
        CFG_INVALID_PARAM_RET("Can't specify link-local prefix");
    }

    for (i = 2;  i < argc; i++)
    {
        if (!sal_strcmp(argv[i], "vlifetime"))
        {
            if (!sal_strcmp(argv[i+1], "infinite"))
            {
                vlifetime = 0xffffffff;
            }
            else
            {
                PM_CFG_GET_UINT(vlifetime, argv[i+1], para->p_rs);
            }
            
            i++;
        }
        else if (!sal_strcmp(argv[i], "vpreferredtime"))
        {
            if (!sal_strcmp(argv[i+1], "infinite"))
            {
                plifetime = 0xffffffff;
            }
            else
            {
                PM_CFG_GET_UINT(plifetime, argv[i+1], para->p_rs);
            }

            i++;
        }
        else if (!sal_strcmp(argv[i], "offlink"))
        {
            onlink = FALSE;
        }
        else if (!sal_strcmp(argv[i], "noautoconfig"))
        {
            auto_is_set = FALSE;
        }  
    }

    if (plifetime > vlifetime)
    {
        CFG_INVALID_PARAM_RET("Preferred lifetime must not exceed valid lifetime\n");
    }

    /* mask address*/
    ip_apply_mask(&addr);

    sal_memcpy(&nd_prefix.key.addr, &addr, sizeof(prefix_t));
    if (argc > 2)
    {
        nd_prefix.config = TRUE;
    }
    else
    {
        /* modified by liwh for bug 48609, 2018-09-03 */
        vlifetime = p_db_rtif->nd_ra_prefix_default_vlifetime;
        plifetime = p_db_rtif->nd_ra_prefix_default_plifetime;
        onlink = p_db_rtif->nd_ra_prefix_default_onlink;
        auto_is_set = p_db_rtif->nd_ra_prefix_default_auto;
        /* liwh end */
    }
    
    nd_prefix.key.ifindex = p_db_rtif->ifindex;
    nd_prefix.vlifetime = vlifetime;
    nd_prefix.plifetime = plifetime;
    nd_prefix.onlink = onlink;
    nd_prefix.autoconfig = auto_is_set;

    p_nd_prefix = ds_nd_prefix_get_nd_prefix(p_db_rtif, &nd_prefix);
    if (!p_nd_prefix)
    {
        if (p_db_rtif->nd_prefix.obj_list->count >= MAX_ADV_PREFIX_NUM)
        {
            CFG_INVALID_PARAM_RET("Total number of prefixes for one interface should not exceed 9");  
        }

        ds_nd_prefix_add_nd_prefix(p_db_rtif, &nd_prefix);
    }
    else
    {
        p_nd_prefix->vlifetime = nd_prefix.vlifetime;
        ds_nd_prefix_set_nd_prefix_field(p_db_rtif, p_nd_prefix, DS_ND_PREFIX_FLD_VLIFETIME);
            
        p_nd_prefix->plifetime = nd_prefix.plifetime;
        ds_nd_prefix_set_nd_prefix_field(p_db_rtif, p_nd_prefix, DS_ND_PREFIX_FLD_PLIFETIME);
        
        p_nd_prefix->onlink = nd_prefix.onlink;
        ds_nd_prefix_set_nd_prefix_field(p_db_rtif, p_nd_prefix, DS_ND_PREFIX_FLD_ONLINK);
        
        p_nd_prefix->autoconfig = nd_prefix.autoconfig;
        ds_nd_prefix_set_nd_prefix_field(p_db_rtif, p_nd_prefix, DS_ND_PREFIX_FLD_AUTOCONFIG);
        
        p_nd_prefix->config = nd_prefix.config;
        ds_nd_prefix_set_nd_prefix_field(p_db_rtif, p_nd_prefix, DS_ND_PREFIX_FLD_CONFIG);
    }

    /* expedite RA */
    nd_if_rtadv_reset(p_db_rtif);

    return rc;
}

int32
nd_prefix_del_entry(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;
    prefix_t                addr;
    ds_nd_prefix_t      nd_prefix;
    ds_nd_prefix_t      *p_nd_prefix = NULL;

    sal_memset(&addr, 0, sizeof(prefix_t));
    sal_memset(&nd_prefix, 0, sizeof(ds_nd_prefix_t));

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    rc = nd_check_interface(p_db_rtif, para);
    if (rc)
    {
        return rc;    
    }

    /* get IPv6 address */
    rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("Invalid IPv6 address");
    }    
    addr.family = AF_INET6;
    PM_CFG_GET_UINT(addr.prefixlen, argv[1], para->p_rs);

    /* IPv6 address check */
    if (IPV6_PREFIX_IS_LINK_LOCAL(&addr))
    {
        CFG_INVALID_PARAM_RET("Can't specify link-local prefix");
    }

    /* mask address*/
    ip_apply_mask(&addr);

    sal_memcpy(&nd_prefix.key.addr, &addr, sizeof(prefix_t));
    nd_prefix.key.ifindex = p_db_rtif->ifindex;    
    p_nd_prefix = ds_nd_prefix_get_nd_prefix(p_db_rtif, &nd_prefix);
    if (!p_nd_prefix)
    {
        CFG_INVALID_PARAM_RET("ND prefix entry is not exist");  
        
    }

    ds_nd_prefix_del_nd_prefix(p_db_rtif, &nd_prefix);

    /* expedite RA */
    nd_if_rtadv_reset(p_db_rtif);
    
    return rc;
}

int32
nd_cmd_mac_addr_str2val(uint8 *mac, const char *str, cfg_cmd_para_t *para)
{
    char    *mac_str    = NULL;
    char    buf[64];
    int32   rc = PM_E_NONE;

    sal_memset(buf, 0, sizeof(buf));
    sal_strncpy(buf, str, 64);
    mac_str = buf;

    rc = cdb_arp_mac_address_is_valid(mac_str);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("Mac address is invalid!");
    }
    
    if (3 != sal_sscanf(mac_str, "%4hx.%4hx.%4hx", (uint16*)&mac[0], (uint16*)&mac[2], (uint16*)&mac[4]))
    {
        CFG_INVALID_PARAM_RET("Mac address is invalid!");
    }
    
    *(uint16*)&mac[0] = sal_hton16(*(uint16*)&mac[0]);
    *(uint16*)&mac[2] = sal_hton16(*(uint16*)&mac[2]);
    *(uint16*)&mac[4] = sal_hton16(*(uint16*)&mac[4]);
    
    return PM_E_NONE;
}

static int32
_nd_fib_cmd_get_entry(prefix_t  *p_addr, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE                          *fp = NULL;
    tbl_nd_fib_t               *p_nd_fib = NULL;
    tbl_iter_args_t            args;
    char                           buf[CMD_BUF_256];
    tbl_nd_fib_master_t     *p_master = NULL;
    ctclib_slistnode_t         *listnode = NULL;

    sal_memset(buf, 0, CMD_BUF_256);

    p_master= tbl_nd_fib_get_master();
    if (!p_master)
    {
        return PM_E_FAIL;    
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == p_addr->prefixlen)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nd_fib_dump_one, &args);
    }
    else
    {       
        p_master= tbl_nd_fib_get_master();
        if (!p_master)
        {
            return PM_E_FAIL;    
        }

        CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
        {
            if (0 == sal_memcmp(&p_nd_fib->key.addr, p_addr, sizeof(prefix_t)))
            {
                tbl_nd_fib_dump_one(p_nd_fib, &args);
            }
        }  
    }
    
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
nd_cmd_process_nd_fib(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                    rc = PM_E_NONE;
    prefix_t                addr;
    tbl_sys_global_t    *p_glb = tbl_sys_global_get_sys_global();
    uint8                    mac_addr[GLB_ETH_ADDR_LEN];
    tbl_interface_t       *p_db_if = NULL;
    tbl_interface_t       intf;
    cdb_node_t           *p_node = NULL;
    field_parse_t         field;
    tbl_route_if_t        *p_db_rtif = NULL;

    sal_memset(&addr, 0, sizeof(prefix_t));
    sal_memset(&field, 0, sizeof(field_parse_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t)); 

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        /* get IPv6 address */
        rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid IPv6 address");
        }    
        addr.family = AF_INET6;
        addr.prefixlen = IPV6_MAX_BITLEN;

        /* IPv6 address check */
        if (IPV6_PREFIX_IS_MULTICAST(&addr) || IPV6_PREFIX_IS_LOOPBACK(&addr) ||
            IPV6_PREFIX_IS_UNSPECIFIED(&addr))
        {
            CFG_INVALID_PARAM_RET("Invalid IPv6 address %s\n", argv[0]);
        }

        /* MAC address check */
        rc = nd_cmd_mac_addr_str2val(mac_addr, argv[1], para);
        if (rc)
        {
            return rc;
        }
        
        if (GLB_IS_ZERO_MAC(mac_addr) || GLB_IS_BCAST_MAC(mac_addr) || GLB_IS_MCAST_MAC(mac_addr))
        {
            CFG_INVALID_PARAM_RET("Invalid MAC address");
        }
    
        if (0 == sal_memcmp(mac_addr, p_glb->route_mac, MAC_ADDR_LEN))
        {
            CFG_INVALID_PARAM_RET("Cannot set route MAC address");
        }

        /* interface check */
        if (IPV6_PREFIX_IS_LINK_LOCAL(&addr))
        {
            if (argc < 3)
            {
                CFG_INVALID_PARAM_RET("Interface must be specified for link-local address\n");
            }
            
            sal_strncpy(intf.key.name, argv[2], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                CFG_INVALID_PARAM_RET("Interface does not exist");
            }

            p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
            if (!p_db_rtif)
            {
                CFG_CONFLICT_RET("Interface not configured for L3 interface");
            }
    
            rc = nd_check_interface(p_db_rtif, para);
            if (rc)
            {
                return rc;    
            }
        }
        else
        {
            if (argc > 2)
            {
                CFG_INVALID_PARAM_RET("Interface can't be specified for non link-local address");
            }
    
            p_db_rtif = if_match_all_by_ipv6_address(&addr.u.prefix6);

            if (p_db_rtif)
            {
                rc = nd_check_interface(p_db_rtif, para);
                if (rc)
                {
                    return rc;    
                }
                
                p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)p_db_rtif->key.name);
            }
        }

        /* add neighbor */
        rc = nd_neigh_add_static_entry(&addr, mac_addr, p_db_if, p_db_rtif, para);
        break;

    case CDB_OPER_DEL:      
        /* get IPv6 address */
        rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid IPv6 address");
        }    
        addr.family = AF_INET6;
        addr.prefixlen = IPV6_MAX_BITLEN;

        /* IPv6 address check */
        if (IPV6_PREFIX_IS_MULTICAST(&addr) || IPV6_PREFIX_IS_LOOPBACK(&addr) ||
            IPV6_PREFIX_IS_UNSPECIFIED(&addr))
        {
            CFG_INVALID_PARAM_RET("Invalid IPv6 address %s\n", argv[0]);
        }

        /* interface check */
        if (IPV6_PREFIX_IS_LINK_LOCAL(&addr))
        {
            if (argc < 2)
            {
                CFG_INVALID_PARAM_RET("Interface must be specified for link-local address\n");
            }

            sal_memset(&intf, 0, sizeof(tbl_interface_t));    
            sal_strncpy(intf.key.name, argv[1], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                CFG_INVALID_PARAM_RET("Interface does not exist");
            }
    
            p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)p_db_if->key.name);
            if (NULL == p_db_rtif)
            {
                CFG_CONFLICT_RET("No such static ND entry!\n");
            }
        }
        else
        {
            if (argc > 1)
            {
                CFG_INVALID_PARAM_RET("Interface can't be specified for non link-local address\n");
            }
        }

        /* add neighbor */
        rc = nd_neigh_del_static_entry(&addr, p_db_if, para);  
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        p_node = cdb_get_tbl(TBL_ND_FIB);
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));

        if (argc >= 1)
        {
            rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid IPv6 address");
            }    
            addr.family = AF_INET6;
            addr.prefixlen = IPV6_MAX_BITLEN;
        }
        
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _nd_fib_cmd_get_entry(&addr, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
nd_show_one_nd_entry(tbl_nd_fib_t   *p_nd_fib, FILE  *fp)
{
    char                    buf[CMD_BUF_256];
    sal_time_t           now;
    tbl_interface_t     *p_db_if = NULL;
    char                    ifname_ext[IFNAME_EXT_SIZE];

    ND_PTR_CHECK(p_nd_fib);
    ND_PTR_CHECK(fp);

    sal_memset(buf, 0, CMD_BUF_256);
    cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &p_nd_fib->key.addr.u.prefix6);

    sal_fprintf(fp, "%-39s ", buf);

    now = ctc_time_boottime_sec(NULL);

    if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
    {
        sal_fprintf(fp, "%-8s ", "-");
    }
    else
    {
        sal_fprintf(fp, "%-8lu ", (now - p_nd_fib->uptime)/60);
    }

    sal_fprintf(fp, "%.04hx.%.04hx.%.04hx  ",
    sal_ntoh16(((unsigned short *)(p_nd_fib->mac_addr))[0]),
    sal_ntoh16(((unsigned short *)(p_nd_fib->mac_addr))[1]),
    sal_ntoh16(((unsigned short *)(p_nd_fib->mac_addr))[2]));

    if (NES_INCOMPLETE == p_nd_fib->state)
    {
        sal_fprintf(fp, "%-5s ", "INCMP");
    }
    else if (NES_STALE == p_nd_fib->state)
    {
        sal_fprintf(fp, "%-5s ", "STALE");
    }
    else if (NES_DELAY == p_nd_fib->state)
    {
        sal_fprintf(fp, "%-5s ", "DELAY");
    }
    else if (NES_PROBE == p_nd_fib->state)
    {
        sal_fprintf(fp, "%-5s ", "PROBE");
    }
    else if (NES_REACHABLE == p_nd_fib->state)
    {
        p_db_if = tbl_interface_get_interface((tbl_interface_key_t*)p_nd_fib->ifname);
        if (p_db_if && tbl_interface_is_running(p_db_if) &&
            GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE))
        {
            sal_fprintf(fp, "%-5s ", "REACH");
        }
        else
        {
            sal_fprintf(fp, "%-5s ", "INVLD");
        }
    }
    else
    {
        sal_fprintf(fp, "%-5s ", "INVLD");
    }

    if (p_nd_fib->ifindex)
    {
        IFNAME_ETH2FULL(p_nd_fib->ifname, ifname_ext);
        sal_fprintf(fp, "%-s\n", ifname_ext);
    }
    else
    {
        sal_fprintf(fp, "%-s\n", "N/A");
    }
    
    return PM_E_NONE;
}

int32
nd_show_neighbor_statistics(tbl_nd_fib_master_t   *p_master, FILE  *fp)
{
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    uint32                           neistat[NES_STATE_MAX + 2] = {0};
    tbl_nd_stats_t               *p_nd_stats = NULL;
    ctclib_slistnode_t           *listnode = NULL;
    
    ND_PTR_CHECK(p_master);
    ND_PTR_CHECK(fp);

    p_nd_stats = tbl_nd_stats_get_nd_stats();
    if (!p_nd_stats)
    {
        return PM_E_FAIL;    
    }
    
    CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
    {
        if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
        {
            neistat[NES_STATE_MAX]++;
        }
        else
        {
            neistat[NES_STATE_MAX + 1]++;
        }

        if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT) &&
            !GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_ACTIVE))
        {
            neistat[NES_NONE]++;
        }
        else
        {
            neistat[p_nd_fib->state]++;
        }
    }
    
    sal_fprintf(fp, "Total IPv6 neighbors: %u\n", neistat[NES_STATE_MAX] + neistat[NES_STATE_MAX + 1]);
    sal_fprintf(fp, "  Static  %3u    Dynamic  %3u\n",
             neistat[NES_STATE_MAX], neistat[NES_STATE_MAX + 1]);
    sal_fprintf(fp, "  Entry States\n");
    sal_fprintf(fp, "    INVLD %3u  INCMP %3u  REACH %3u  STALE %3u  DELAY %3u  PROBE %3u\n",
            neistat[NES_NONE], neistat[NES_INCOMPLETE], neistat[NES_REACHABLE],
            neistat[NES_STALE], neistat[NES_DELAY], neistat[NES_PROBE]);

    ND_EVENT_DEBUG("rcv: %llu rcv_rs: %llu rcv_ra: %llu rcv_ns: %llu rcv_na: %llu rcv_redirect: %llu", 
        p_nd_stats->rcv, p_nd_stats->rcv_rs, p_nd_stats->rcv_ra, p_nd_stats->rcv_ns, p_nd_stats->rcv_na, p_nd_stats->rcv_redirect);
    ND_EVENT_DEBUG("rcv_error: %llu rcv_tooshort: %llu rcv_invalid: %llu rcv_inconsistent: %llu", 
        p_nd_stats->rcv_error, p_nd_stats->rcv_tooshort, p_nd_stats->rcv_invalid, p_nd_stats->rcv_inconsistent);
    ND_EVENT_DEBUG("Neighbor entry allocs: %llu destroys: %llu", p_nd_stats->allocs, p_nd_stats->destroys);

    /*ND_EVENT_DEBUG("Allocs: %llu", p_nd_stats->allocs);
    ND_EVENT_DEBUG("Destroys: %llu", p_nd_stats->destroys);
    ND_EVENT_DEBUG("rcv_error: %llu", p_nd_stats->rcv);
    ND_EVENT_DEBUG("Rcv error: %llu", p_nd_stats->rcv_error);
    ND_EVENT_DEBUG("rcv_tooshort: %llu", p_nd_stats->rcv_tooshort);
    ND_EVENT_DEBUG("rcv_invalid: %llu", p_nd_stats->rcv_invalid);
    ND_EVENT_DEBUG("rcv_inconsistent: %llu", p_nd_stats->rcv_inconsistent);
    ND_EVENT_DEBUG("rcv_rs: %llu", p_nd_stats->rcv_rs);
    ND_EVENT_DEBUG("rcv_ra: %llu", p_nd_stats->rcv_ra);
    ND_EVENT_DEBUG("rcv_ns: %llu", p_nd_stats->rcv_ns);
    ND_EVENT_DEBUG("rcv_na: %llu", p_nd_stats->rcv_na);
    ND_EVENT_DEBUG("rcv_redirect: %llu", p_nd_stats->rcv_redirect);
    ND_EVENT_DEBUG("overflow_failed: %llu", p_nd_stats->overflow_failed);
    ND_EVENT_DEBUG("rcv_miss: %llu", p_nd_stats->rcv_miss);
    ND_EVENT_DEBUG("rcv_miss_conflict: %llu", p_nd_stats->rcv_miss_conflict);
    ND_EVENT_DEBUG("rcv_static_conflict: %llu", p_nd_stats->rcv_static_conflict);
    ND_EVENT_DEBUG("error: %llu", p_nd_stats->error);
    ND_EVENT_DEBUG("res_failed: %llu", p_nd_stats->res_failed);
    ND_EVENT_DEBUG("rcv_probes_mcast: %llu", p_nd_stats->rcv_probes_mcast);
    ND_EVENT_DEBUG("rcv_probes_ucast: %llu", p_nd_stats->rcv_probes_ucast);
    ND_EVENT_DEBUG("intf_invalid: %llu", p_nd_stats->intf_invalid);*/
    
    return PM_E_NONE;
}

int32
nd_show_neighbor_prefix(tbl_route_if_t    *p_db_rtif, FILE  *fp)
{
    const char prefix_show_header[] =
        "Codes for 1st column:\n"
        "       A - Address, P - Prefix-Advertisement\n"
        "Codes for 2nd column and above:\n"
        "       D - Default, N - Not advertised\n"
        "Codes for RA prefix option flag:\n"
        "       L - On link, A - Auto-config\n";
    char nd_line[256];
    char nd_tmp[256];
    char dftstr[70] = "\0";
    ds_connected_v6_t       *p_ifc  = NULL;
    ctclib_slistnode_t          *listnode   = NULL;
    prefix_t                        p;
    ds_nd_prefix_t              *p_nd_prefix = NULL;
    ctclib_slistnode_t           *rt_listnode = NULL;
    uint8                             blank = 0;
    char                              ifname_ext[IFNAME_EXT_SIZE] = {'\0'};

    IFNAME_ETH2FULL(p_db_rtif->key.name, ifname_ext);
    sal_fprintf(fp, "IPv6 Prefix Advertisements %s\n", ifname_ext);
    sal_fprintf(fp, "%s", prefix_show_header);

    sal_sprintf(nd_line, "     default ");
    sal_strncat(dftstr, "[", sal_strlen("["));

    if (p_db_rtif->nd_ra_prefix_default_onlink)
    {
        sal_strncat(dftstr, "L", sal_strlen("L"));
    }

    if (p_db_rtif->nd_ra_prefix_default_auto)
    {
        sal_strncat(dftstr, "A", sal_strlen("A"));
    }

    sal_strncat(dftstr, "] Valid lifetime ", sal_strlen("] Valid lifetime "));
    if (0xffffffff == p_db_rtif->nd_ra_prefix_default_vlifetime)
    {
        sal_strncat(dftstr, "infinite", sal_strlen("infinite"));
    }
    else
    {
        sal_sprintf(nd_tmp, "%u sec", p_db_rtif->nd_ra_prefix_default_vlifetime); 
        sal_strncat(dftstr, nd_tmp, sal_strlen(nd_tmp));
    }

    sal_strncat(dftstr, ", preferred lifetime ", sal_strlen(", preferred lifetime "));
    if (0xffffffff == p_db_rtif->nd_ra_prefix_default_plifetime)
    {
        sal_strncat(dftstr, "infinite", sal_strlen("infinite"));
    }
    else
    {
        sal_sprintf(nd_tmp, "%u sec", p_db_rtif->nd_ra_prefix_default_plifetime); 
        sal_strncat(dftstr, nd_tmp, sal_strlen(nd_tmp));
    }

    sal_strncat(nd_line, dftstr, sal_strlen(dftstr));
    sal_fprintf(fp, "%s\n", nd_line);

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_ifc, listnode)
    {
        ip_prefix_copy (&p, &p_ifc->key.address);
        
        if (IPV6_PREFIX_IS_UNSPECIFIED (&p_ifc->key.address)
            || IPV6_PREFIX_IS_LOOPBACK  (&p_ifc->key.address)
            || IPV6_PREFIX_IS_MULTICAST  (&p_ifc->key.address)
            || IPV6_PREFIX_IS_LINK_LOCAL (&p_ifc->key.address))
        {
            continue;
        }

        if ((p_db_rtif->nd_prefix.obj_list && p_db_rtif->nd_prefix.obj_list->count)
            || (0 == p_db_rtif->nd_ra_prefix_default_vlifetime))
        {
            sal_sprintf(nd_line, "ADN  ");
        }
        else
        {
            sal_sprintf(nd_line, "AD   ");    
        }

        ip_apply_mask (&p);

        cdb_addr_ipv6_val2str(nd_tmp, CMD_BUF_256, &p.u.prefix6);
        sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

        sal_sprintf(nd_tmp, "/%d ", p.prefixlen);
        sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

        sal_strncat(nd_line, dftstr, sal_strlen(dftstr));
        sal_fprintf(fp, "%s\n", nd_line);
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->nd_prefix.obj_list, p_nd_prefix, rt_listnode)
    {
        sal_sprintf(nd_line, "P");

        if (p_nd_prefix->config)
        {
            blank++;
        }
        else
        {
            sal_strncat(nd_line, "D", sal_strlen("D"));    
        }

        if (0 == p_nd_prefix->vlifetime)
        {
            sal_strncat(nd_line, "N", sal_strlen("N"));
        }
        else
        {
            blank++;
        }

        sal_strcat(nd_line, "  ");
        while(blank > 0)
        {
            sal_strcat(nd_line, " ");
            blank--;
        }

        ip_prefix_copy (&p, &p_nd_prefix->key.addr);
        cdb_addr_ipv6_val2str(nd_tmp, CMD_BUF_256, &p.u.prefix6);
        sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
        
        sal_sprintf(nd_tmp, "/%d ", p.prefixlen);
        sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));

        if (p_nd_prefix->config)
        {
            sal_strncat(nd_line, "[", sal_strlen("["));

            if (p_nd_prefix->onlink)
            {
                sal_strncat(nd_line, "L", sal_strlen("L"));
            }

            if (p_nd_prefix->autoconfig)
            {
                sal_strncat(nd_line, "A", sal_strlen("A"));
            }

            sal_strncat(nd_line, "] Valid lifetime ", sal_strlen("] Valid lifetime "));
            if (0xffffffff == p_nd_prefix->vlifetime)
            {
                sal_strncat(nd_line, "infinite", sal_strlen("infinite"));
            }
            else
            {
                sal_sprintf(nd_tmp, "%u sec", p_nd_prefix->vlifetime); 
                sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
            }

            sal_strncat(nd_line, ", preferred lifetime ", sal_strlen(", preferred lifetime "));
            if (0xffffffff == p_nd_prefix->plifetime)
            {
                sal_strncat(nd_line, "infinite", sal_strlen("infinite"));
            }
            else
            {
                sal_sprintf(nd_tmp, "%u sec", p_nd_prefix->plifetime); 
                sal_strncat(nd_line, nd_tmp, sal_strlen(nd_tmp));
            }
        }
        else
        {
            sal_strncat(nd_line, dftstr, sal_strlen(dftstr));    
        }

        sal_fprintf(fp, "%s\n", nd_line);      
    }

    return PM_E_NONE;
}

int32
nd_cmd_show_nd(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    FILE                            *fp = NULL;
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    prefix_t                        addr;
    tbl_interface_t               intf;
    tbl_interface_t               *p_db_if = NULL;
    tbl_route_if_t                *p_db_rtif = NULL;
    tbl_route_if_t                rtif;
   
    sal_memset(&addr, 0, sizeof(prefix_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));
    sal_memset(&rtif, 0, sizeof(tbl_route_if_t));

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    
    p_master= tbl_nd_fib_get_master();
    if (!p_master)
    {
        return PM_E_FAIL;    
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if  ((1 == argc) && (0 == sal_memcmp(argv[0], "statistics", sal_strlen("statistics")))) 
    {
        nd_show_neighbor_statistics(p_master, fp);
    }
    else if  ((3 == argc) && (0 == sal_memcmp(argv[0], "prefix", sal_strlen("prefix")))
        && (0 == sal_memcmp(argv[1], "interface", sal_strlen("interface"))))
    {
        sal_strncpy(intf.key.name, argv[2], IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_INVALID_PARAM_RET("Interface does not exist");
        }
        
        sal_strncpy(rtif.key.name, argv[2], IFNAME_SIZE);
        p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
        if (!p_db_rtif)
        {
            sal_fclose(fp);
            fp = NULL;
            CFG_CONFLICT_RET("Interface not configured for L3 interface");
        }

        nd_show_neighbor_prefix(p_db_rtif, fp);
    }
    else
    {   
        sal_fprintf(fp, "%-39s %-8s %-15s %-5s %s\n","IPv6 address", "Age",
            "Link-Layer Addr", "State", "Interface");
    
        if (0 == argc)
        {
            CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
            {
                nd_show_one_nd_entry(p_nd_fib, fp);
            }    
        }
        else if (1 == argc)
        {
            if (0 == sal_memcmp(argv[0], "dynamic", sal_strlen("dynamic"))) 
            {
                CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
                {
                    if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
                    {
                        nd_show_one_nd_entry(p_nd_fib, fp);
                    }
                }    
            }
            else if (0 == sal_memcmp(argv[0], "static", sal_strlen("static"))) 
            {
                CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
                {
                    if (GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
                    {
                        nd_show_one_nd_entry(p_nd_fib, fp);
                    }
                }    
            }
            else
            {
                addr.family = AF_INET6;
                addr.prefixlen = IPV6_MAX_BITLEN;
                rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
                if (rc < 0)
                {
                    sal_fclose(fp);
                    fp = NULL;
                    CFG_INVALID_PARAM_RET("Invalid IPv6 address");
                }    
    
                CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
                {
                    if (0 == sal_memcmp(&p_nd_fib->key.addr, &addr, sizeof(prefix_t)))
                    {
                        nd_show_one_nd_entry(p_nd_fib, fp);
                    }
                }   
            }
        }
        else if (2 == argc)
        {
            if (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))) 
            {
                sal_strncpy(intf.key.name, argv[1], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&intf.key);
                if (!p_db_if)
                {
                    sal_fclose(fp);
                    fp = NULL;
                    CFG_INVALID_PARAM_RET("Interface does not exist");
                }
    
                CTCLIB_SLIST_LOOP(p_master->neighbor_list, p_nd_fib, listnode)
                {
                    if (p_nd_fib->ifindex == p_db_if->ifindex)
                    {
                        nd_show_one_nd_entry(p_nd_fib, fp);
                    }
                }  
            }
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
nd_cmd_clear_nd(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    tbl_nd_fib_master_t      *p_master = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    tbl_nd_fib_t                  *p_nd_fib = NULL;
    ctclib_slistnode_t          *next = NULL;
    prefix_t                        addr;
    tbl_interface_t               intf;
    tbl_interface_t               *p_db_if = NULL;

    sal_memset(&addr, 0, sizeof(prefix_t));
    sal_memset(&intf, 0, sizeof(tbl_interface_t));

    if (0 == nd_api_count_get_max())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    p_master= tbl_nd_fib_get_master();
    if (!p_master)
    {
        return PM_E_FAIL;    
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
        {
            if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT))
            {
                nd_neigh_delete(p_nd_fib);
            }
        }    
    }
    else if (1 == argc)
    {
        addr.family = AF_INET6;
        addr.prefixlen = IPV6_MAX_BITLEN;
        rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid IPv6 address");
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
        {
            if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT)
               &&(0 == sal_memcmp(&p_nd_fib->key.addr, &addr, sizeof(prefix_t))))
            {
                nd_neigh_delete(p_nd_fib);
            }
        } 
    }
    else if (2 == argc)
    {
        if (0 == sal_memcmp(argv[0], "interface", sal_strlen("interface"))) 
        {
            sal_strncpy(intf.key.name, argv[1], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&intf.key);
            if (!p_db_if)
            {
                CFG_INVALID_PARAM_RET("Interface does not exist");
            }

            CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
            {
                if ((p_nd_fib->ifindex == p_db_if->ifindex)
                    && (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT)))
                {
                    nd_neigh_delete(p_nd_fib);
                }
            }  
        }
        else
        {
            addr.family = AF_INET6;
            addr.prefixlen = IPV6_MAX_BITLEN;
            rc = sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid IPv6 address");
            }

            if (!IPV6_PREFIX_IS_LINK_LOCAL(&addr))
            {
                CFG_INVALID_PARAM_RET("Interface can't be specified for non link-local address");    
            }
    
            CTCLIB_SLIST_LOOP_DEL(p_master->neighbor_list, p_nd_fib, listnode, next)
            {
                if (!GLB_FLAG_ISSET(p_nd_fib->flags, GLB_ND_FLAG_PERMANENT)
                   &&(0 == sal_memcmp(&p_nd_fib->key.addr, &addr, sizeof(prefix_t)))
                   && (0 == sal_memcmp(p_nd_fib->ifname, argv[1], sal_strlen(p_nd_fib->ifname))))
                {
                    nd_neigh_delete(p_nd_fib);
                }
            } 
        }
    }
    
    return rc;
}

int32
nd_start()
{
    cdb_register_cfg_tbl_cb(TBL_ND_FIB, nd_cmd_process_nd_fib);
    cdb_register_cfg_act_cb(ACT_SHOW_ND, nd_cmd_show_nd);  
    cdb_register_cfg_act_cb(ACT_CLEAR_ND, nd_cmd_clear_nd);

    ipc_register_pkt_rx_fn(GLB_PKT_ND, nd_pdu_rx);

    ipc_register_hal_rx_fn(ROUTED_FEI_FDB_AGE_ND_NOTIFY, nd_api_probe_for_fdb_age);
    ipc_register_hal_rx_fn(ROUTED_FEI_KERNEL_UPDATE_ND_NOTIFY, nd_api_kernel_update_nd);

    return PM_E_NONE;
}
