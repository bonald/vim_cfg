
#include "proto.h"
#include "route.h"
#include "prefix.h"
#include "libnetfilter_conntrack/libnetfilter_conntrack.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "gen/tbl_nat_proxy_arp_define.h"
#include "gen/tbl_nat_proxy_arp.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"
#include "hsrv_msg.h"
#include "nat_api.h"
#include "nat.h"


bool nat_api_debug_is_on(uint32 flag)
{
    tbl_route_debug_t *p_db_glb = NULL;
    
    p_db_glb = tbl_route_debug_get_route_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->nat, flag))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/***************************************************************************************************
 * Name         : nat_api_set_if_nat 
 * Purpose      : set rt if nat
 * Input        : tbl_route_if_t *p_db_rt_if, uint8 enable_old, uint8 enable_new
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_api_set_if_nat(tbl_route_if_t *p_db_rt_if, uint8 enable_old, uint8 enable_new)
{
    return nat_if_set(p_db_rt_if, enable_old, enable_new);
}

/***************************************************************************************************
 * Name         : nat_api_rtif_del 
 * Purpose      : process rt if delete
 * Input        : tbl_route_if_t *p_db_rt_if
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_api_rtif_del(tbl_route_if_t *p_rt_if)
{
    tbl_route_if_t *p_db_rt_if = NULL;

    p_db_rt_if = tbl_route_if_get_route_if(&p_rt_if->key);
    if (!p_db_rt_if)
    {
        return PM_E_FAIL;
    }

    if (GLB_IF_NAT_DISABLE == p_db_rt_if->nat_enable)
    {
        return PM_E_NONE;
    }

    nat_if_set(p_db_rt_if, p_db_rt_if->nat_enable, GLB_IF_NAT_DISABLE);

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_api_rtif_connected_add
 * Purpose      : process rt if IP add
 * Input        : tbl_route_if_t *p_db_rt_if
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_api_rtif_connected_add(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rt_if, ds_connected_t *p_conn)
{
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL))
    {
        return PM_E_NONE;
    }

    nat_rule_proc_rtif_addr_add(p_db_rt_if, &p_conn->key.address);
    nat_pool_proc_rtif_addr_add(p_db_rt_if, &p_conn->key.address);

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_api_rtif_connected_del 
 * Purpose      : process rt if IP delete
 * Input        : tbl_route_if_t *p_db_rt_if
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_api_rtif_connected_del(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rt_if,   ds_connected_t *p_conn)
{
    if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDR_VIRTUAL))
    {
        return PM_E_NONE;
    }

    nat_rule_proc_rtif_addr_del(p_db_rt_if, &p_conn->key.address);
    nat_pool_proc_rtif_addr_del(p_db_rt_if, &p_conn->key.address);

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_api_if_need_proxy_arp 
 * Purpose      : if the address should be ARP proxy
 * Input        : tbl_route_if_t *p_rtif, addr_ipv4_t *p_addr
 * Output       : N/A                  
 * Return       : TRUE/FLASE
 * Note         : N/A
***************************************************************************************************/
bool
nat_api_if_need_proxy_arp(tbl_route_if_t *p_rtif, addr_ipv4_t *p_addr)
{
    tbl_nat_proxy_arp_key_t proxy_arp_key;
    tbl_nat_proxy_arp_t *p_db_proxy_arp = NULL;

    if (GLB_IF_NAT_OUTSIDE != p_rtif->nat_enable)
    {
        return FALSE;
    }

    sal_memset(&proxy_arp_key, 0, sizeof(proxy_arp_key));
    proxy_arp_key.ifindex = p_rtif->ifindex;
    proxy_arp_key.inside_global_ip.family = AF_INET;
    sal_memcpy(&proxy_arp_key.inside_global_ip.u.prefix4, p_addr, sizeof(addr_ipv4_t));

    p_db_proxy_arp = tbl_nat_proxy_arp_get_nat_proxy_arp(&proxy_arp_key);
    if (p_db_proxy_arp)
    {
        return TRUE;
    }

    return FALSE;
}

int32
nat_api_init(route_master_t *master)
{
#ifdef TSINGMA
#ifndef OFPRODUCT
    tbl_nat_global_t *p_nat_global = NULL;

    /* ====nat global var init===== */
    p_nat_global = tbl_nat_global_get_nat_global();

    sal_memset(p_nat_global, 0, sizeof(tbl_nat_global_t));
    if (0 != ctclib_opb_create(&p_nat_global->rule_id_opb, 0, 100, "nat_rule_id_opb"))
    {
        return PM_E_FAIL;
    }

    if (0 != ctclib_opb_create(&p_nat_global->category_id_opb, 1, 16, "nat_category_id_opb"))
    {
        return PM_E_FAIL;
    }

    if (0 != ctclib_opb_create(&p_nat_global->session_id_opb, 0, NAT_SESSION_SPEC, "nat_session_id_opb"))
    {
        return PM_E_FAIL;
    }

    /* init and use aging timer in hsrv_nat.c, aging_timer handle got by routed will always be zero.*/
    p_nat_global->aging_timer = 0;

    p_nat_global->nft_hdl = 0;
    p_nat_global->rule_cfg_order = 0;

    /* hsrv nat aging notify message process function*/
    ipc_register_hal_rx_fn(ROUTED_FEI_NAT_SESSION_AGING_DEL, nat_session_aging_hook);
    cdb_pm_subscribe_tbl(TBL_NAT_SESSION, NULL, nat_session_tbl_process_after_sync);

    nat_session_global_ttl_init();
    nat_session_spec_init();
    nat_nfct_reg();

    /* ====cmd===== */
    nat_cmd_init();

    /* ====kernel nft table, chains and rules===== */
    nat_nft_init();
#endif
#endif
    return PM_E_NONE;
}

int32
nat_api_deinit()
{
    //tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    // TODO:
    return PM_E_NONE;
}

