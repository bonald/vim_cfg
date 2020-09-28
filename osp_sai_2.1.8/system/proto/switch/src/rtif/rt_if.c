
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "prefix.h"
#include "rt_if.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"


int32
rt_if_set_arp_retry_interval(char *ifname, uint32 retry_interval)
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

    p_db_rtif->arp_retrans_time = retry_interval;
    tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_ARP_RETRANS_TIME);
    return PM_E_NONE;
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

int32 rt_if_check_before_add_agg_member(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    tbl_route_if_t *p_db_rtif     = NULL;
    tbl_route_if_t *p_db_agg_rtif = NULL;
    tbl_route_if_key_t rt_if_key;
    cfg_cmd_para_t *para = (cfg_cmd_para_t*)p_param;

    if ((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return PM_E_NONE;
    }

    /* no need to check first member */
    if ((NULL != p_db_agg_if->lag))
    {
        if (0 == p_db_agg_if->lag->member_ports.obj_list->count)
        {
            return PM_E_NONE;
        }
    }

    /* update mtu */
    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, p_db_mem_if->key.name);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, p_db_agg_if->key.name);
    p_db_agg_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_agg_rtif)
    {
        return PM_E_NONE;
    }

    if (p_db_rtif->mtu != p_db_agg_rtif->mtu)
    {
        CFG_CONFLICT_RET("MTU mismatch, %s's MTU is %u, %s's MTU is %u",
            p_db_agg_rtif->key.name, p_db_agg_rtif->mtu,
            p_db_rtif->key.name, p_db_rtif->mtu);
    }

    return PM_E_NONE;
}

int32 rt_if_after_add_agg_member(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if ((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return PM_E_NONE;
    }

    /* first member */
    if (1 != p_db_agg_if->lag->member_ports.obj_list->count)
    {
        return PM_E_NONE;
    }

    /* create router interface */
    if (GLB_IF_MODE_L3 == p_db_agg_if->mode)
    {
        tbl_interface_set_interface_field(p_db_agg_if, TBL_INTERFACE_FLD_MODE);
        //rt_if_add_if(p_db_agg_if->key.name);
    }

    /* update mtu */

    #if 0
    tbl_route_if_t *p_db_rtif     = NULL;
    tbl_route_if_t *p_db_agg_rtif = NULL;
    tbl_route_if_key_t rt_if_key;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, p_db_mem_if->key.name);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return PM_E_NONE;
    }

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, p_db_agg_if->key.name);
    p_db_agg_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_agg_rtif)
    {
        return PM_E_NONE;
    }
    #endif
    
    /* agg use first member interface's mtu */
    if (p_db_agg_if->mtu != p_db_mem_if->mtu)
    {
        //rt_if_set_mtu(p_db_agg_rtif->key.name, p_db_rtif->mtu);

        tbl_interface_set_interface_field(p_db_agg_if, TBL_INTERFACE_FLD_MTU);        
    }

    return PM_E_NONE;
}

int32 rt_if_before_destroy_agg(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    tbl_route_if_t *p_db_agg_rtif = NULL;
    tbl_route_if_key_t rt_if_key;

    sal_memset(rt_if_key.name, 0, IFNAME_SIZE);
    sal_strcpy(rt_if_key.name, p_db_agg_if->key.name);
    p_db_agg_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_agg_rtif)
    {
        return PM_E_NONE;
    }

    tbl_interface_set_interface_field(p_db_agg_if, TBL_INTERFACE_FLD_MODE);
    //rt_if_del_if(p_db_agg_if->key.name);
    return PM_E_NONE;
}

