
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "switch.h"
#include "lldp_define.h"
#include "lldp.h"
#include "lldp_api.h"

#define LLDP_MIN(A, B) ((A) < (B) ? (A) : (B))

static void
_lldp_if_tx_timer_fn(void *p_data)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    tbl_lldp_if_t *p_db_lldp_if = (tbl_lldp_if_t*)p_data;

    /*LLDP debug*/
    LLDP_DEBUG(LLDP_TIMER, "LLDP Transmit timer expired\n");
    
    p_db_lldp_if->tx_timer = NULL;

    lldp_pdu_tx(p_db_lldp_if);

    p_db_lldp_if->tx_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lldp_if_tx_timer_fn, p_db_lldp_if, 
        p_db_glb->tx_interval);

    return;
}

int32
lldp_if_tx_init(tbl_lldp_if_t *p_db_lldp_if)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    int32 rc = PM_E_NONE;

    CTC_TASK_STOP_TIMER(p_db_lldp_if->tx_timer);
    p_db_lldp_if->tx_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lldp_if_tx_timer_fn, p_db_lldp_if, 
        p_db_glb->tx_interval);

    return rc;
}

int32
lldp_if_tx_deinit(tbl_lldp_if_t *p_db_lldp_if)
{
    int32 rc = PM_E_NONE;

    CTC_TASK_STOP_TIMER(p_db_lldp_if->tx_timer);

    return rc;
}

int32
lldp_if_update_tx_ttl(tbl_lldp_if_t *p_db_lldp_if)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();

    p_db_lldp_if->tx_ttl = LLDP_MIN(0xFFFF, (p_db_glb->tx_interval * p_db_glb->tx_hold));

    /*LLDP debug*/
    LLDP_DEBUG(LLDP_EVENT, "LLDP update the tx TTL to \n", p_db_lldp_if->tx_ttl);

    return PM_E_NONE;
}

static int32
_lldp_if_set_default(tbl_lldp_if_t *p_lldp_if)
{
    p_lldp_if->tlv_enable_basic = BASIC_TLV_EN_ALL;
    p_lldp_if->tlv_enable_8021  = IEEE_8023_TLV_EN_ALL;
    p_lldp_if->tlv_enable_8023  = IEEE_8023_TLV_EN_ALL;
    p_lldp_if->tlv_enable_med   = MED_TLV_EN_ALL;
    p_lldp_if->med_dev_type = LLDP_MED_DEV_TYPE_DEFAULT;
    p_lldp_if->med_policy_flag = LLDP_MED_NP_DEFAULT_VALUE;
    lldp_if_update_tx_ttl(p_lldp_if);
    
    return PM_E_NONE;
}

int32
_lldp_if_del_neigh(tbl_lldp_if_t *p_db_lldp_if, ds_lldp_neighbour_t *p_db_neigh)
{
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char chassis_id_str[CMD_BUF_32];
    char port_id_str[CMD_BUF_32];
    
    tbl_interface_get_name_by_ifindex(p_db_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);
    lldp_chassis_id_val2str(chassis_id_str, CMD_BUF_32, &p_db_neigh->key.chassis_id);
    lldp_port_id_val2str(port_id_str, CMD_BUF_32, &p_db_neigh->key.port_id);
    logid_sys(LOG_LLDP_4_DEL_NEIGH, chassis_id_str, port_id_str, ifname_ext, "timeout");

    CTC_TASK_STOP_TIMER(p_db_neigh->rx_ttl_timer);
    ds_lldp_neighbour_del_lldp_neighbour(p_db_lldp_if, p_db_neigh);

    return PM_E_NONE;
}

int32
_lldp_if_del_all_neigh(tbl_lldp_if_t *p_db_lldp_if)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    ds_lldp_neighbour_t *p_db_neigh = NULL;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char chassis_id_str[CMD_BUF_32];
    char port_id_str[CMD_BUF_32];
    
    tbl_interface_get_name_by_ifindex(p_db_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);

    CTCLIB_SLIST_LOOP_DEL(p_db_lldp_if->neigh_list.obj_list, p_db_neigh, listnode, next)
    {    
        lldp_chassis_id_val2str(chassis_id_str, CMD_BUF_32, &p_db_neigh->key.chassis_id);
        lldp_port_id_val2str(port_id_str, CMD_BUF_32, &p_db_neigh->key.port_id);
        logid_sys(LOG_LLDP_4_DEL_NEIGH, chassis_id_str, port_id_str, ifname_ext, "administer");

        CTC_TASK_STOP_TIMER(p_db_neigh->rx_ttl_timer);
        ds_lldp_neighbour_del_lldp_neighbour(p_db_lldp_if, p_db_neigh);
    }

    return PM_E_NONE;
}

static void
_lldp_neigh_ttl_timer_fn(void* p_data)
{
    tbl_lldp_if_key_t key;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ds_lldp_neighbour_t *p_db_neigh = (ds_lldp_neighbour_t*)p_data;

    /*LLDP debug*/
    LLDP_DEBUG(LLDP_TIMER, "LLDP Receive TTL timer expired\n");
    
    p_db_neigh->rx_ttl_timer = NULL;

    key.ifindex = p_db_neigh->rx_ifindex;
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
    
    if (NULL == p_db_lldp_if)
    {
        return;
    }
    p_db_lldp_if->aged_count++;
    _lldp_if_del_neigh(p_db_lldp_if, p_db_neigh);

    return;
}

int32
lldp_if_update_neigh(tbl_lldp_if_t *p_db_lldp_if, ds_lldp_neighbour_t *p_neigh)
{
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char chassis_id_str[CMD_BUF_32];
    char port_id_str[CMD_BUF_32];
    ds_lldp_neighbour_t *p_db_neigh = NULL;

    p_db_neigh = ds_lldp_neighbour_get_lldp_neighbour(p_db_lldp_if, p_neigh);
    if (NULL == p_db_neigh)
    {
        /*modified by yejl to fix bug40072, 2016-08-10*/
        if (p_neigh->rx_ttl)
        {
            ds_lldp_neighbour_add_lldp_neighbour(p_db_lldp_if, p_neigh);
            p_db_neigh = ds_lldp_neighbour_get_lldp_neighbour(p_db_lldp_if, p_neigh); 
            if (NULL == p_db_neigh)
            {
                return PM_E_NOT_EXIST;
            }
            p_db_neigh->rx_ttl_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lldp_neigh_ttl_timer_fn, 
                    p_db_neigh, p_db_neigh->rx_ttl);

            tbl_interface_get_name_by_ifindex(p_db_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2FULL(ifname, ifname_ext);
            lldp_chassis_id_val2str(chassis_id_str, CMD_BUF_32, &p_db_neigh->key.chassis_id);
            lldp_port_id_val2str(port_id_str, CMD_BUF_32, &p_db_neigh->key.port_id);
            logid_sys(LOG_LLDP_4_ADD_NEIGH, chassis_id_str, port_id_str, ifname_ext);
        }
    }
    else
    {
        sal_memcpy(p_db_neigh->mac_addr, p_neigh->mac_addr, MAC_ADDR_LEN);
        p_db_neigh->rx_ttl = p_neigh->rx_ttl;
        CTC_TASK_STOP_TIMER(p_db_neigh->rx_ttl_timer);
        sal_memcpy(p_db_neigh, p_neigh, sizeof(ds_lldp_neighbour_t));
        
        /*modified by yejl to fix bug40072, 2016-08-10*/
        if (p_db_neigh->rx_ttl)
        {
            p_db_neigh->rx_ttl_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lldp_neigh_ttl_timer_fn, 
                    p_db_neigh, p_db_neigh->rx_ttl);
        }
        else
        {
            _lldp_if_del_neigh(p_db_lldp_if, p_db_neigh);
        }
    }

    return PM_E_NONE;
}

int32
lldp_if_create(tbl_lldp_if_t *p_lldp_if)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    int32 rc = PM_E_NONE;

    _lldp_if_set_default(p_lldp_if);
    PM_E_RETURN(tbl_lldp_if_add_lldp_if(p_lldp_if));

    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
    if (NULL == p_db_lldp_if)
    {
        return PM_E_FAIL;
    }
    p_db_if = tbl_interface_get_interface_by_ifindex(p_lldp_if->key.ifindex);
    p_db_lldp_if->intf = p_db_if;

    if (LLDP_IF_TXRX == p_lldp_if->mode || LLDP_IF_TXONLY == p_lldp_if->mode)
    {
        /*modified by yejl for bug 53349, 2019-09-30*/
        #ifndef OFPRODUCT
        lldp_if_tx_init(p_db_lldp_if);
        #endif
    }

    return PM_E_NONE;
}

int32
lldp_if_set_mode(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_t *p_db_lldp_if)
{
    uint32 old_tx_enable = FALSE;
    uint32 new_tx_enable = FALSE;
    int32 rc = PM_E_NONE;

    old_tx_enable = (LLDP_IF_TXRX == p_db_lldp_if->mode || LLDP_IF_TXONLY == p_db_lldp_if->mode);
    new_tx_enable = (LLDP_IF_TXRX == p_lldp_if->mode || LLDP_IF_TXONLY == p_lldp_if->mode);
    
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_MODE));

    if (old_tx_enable == new_tx_enable)
    {
        return PM_E_NONE;
    }

    if (new_tx_enable)
    {
        lldp_if_tx_init(p_db_lldp_if);
    }
    else
    {
        lldp_if_tx_deinit(p_db_lldp_if);
    }
    
    return PM_E_NONE;
}

int32
lldp_if_destory(tbl_lldp_if_t *p_db_lldp_if)
{
    int32 rc = PM_E_NONE;

    p_db_lldp_if->intf = NULL;
    PM_E_RETURN(_lldp_if_del_all_neigh(p_db_lldp_if));
    PM_E_RETURN(lldp_if_tx_deinit(p_db_lldp_if));
    PM_E_RETURN(tbl_lldp_if_del_lldp_if(&p_db_lldp_if->key));

    return PM_E_NONE;
}

static int32
_lldp_set_global_default()
{
    tbl_sys_global_t *p_db_sys = tbl_sys_global_get_sys_global();
    tbl_version_t *p_db_version = tbl_version_get_version();
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    p_db_glb->tx_hold       = LLDP_TX_HOLD_DEFAULT;
    p_db_glb->tx_interval   = LLDP_TX_INTERVAL_DEFAULT;
    p_db_glb->tx_delay      = LLDP_TX_DELAY_DEFAULT;
    p_db_glb->reinit_delay  = LLDP_REINIT_DELAY_DEFAULT;
    p_db_glb->system_mac_en = TRUE;
    p_db_glb->enable        = FALSE;
    sal_memcpy(p_db_glb->system_mac, p_db_sys->route_mac, MAC_ADDR_LEN);
    
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_TX_HOLD);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_TX_INTERVAL);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_TX_DELAY);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_REINIT_DELAY);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC_EN);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_MAC);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_ENABLE);
    
    p_db_glb->system_capability = LLDP_SYS_CAP_DEFAULT;
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_CAPABILITY);
    
    sal_snprintf(p_db_glb->system_desc_default, LLDP_DESCRIPTION_SIZE, 
                "%s software, %s, Version %s \n Copyright (C) %s.  All rights reserved.", 
                p_db_version->package, p_db_version->product, p_db_version->version, p_db_version->company);
    tbl_lldp_global_set_lldp_global_field(p_db_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC_DEFAULT);

    return PM_E_NONE;
}

static int32
_lldp_set_if_default()
{
    tbl_interface_master_t *p_if_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t* p_db_if = NULL;
    
    tbl_lldp_if_t lldp_if;
    tbl_lldp_if_t* p_db_lldp_if = NULL;
    
    CTCLIB_SLIST_LOOP(p_if_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        sal_memset(&lldp_if, 0 , sizeof(lldp_if));
        lldp_if.tx_timer = NULL;
        lldp_if.intf = NULL;
        
        lldp_if.key.ifindex = p_db_if->ifindex;
        p_db_lldp_if = tbl_lldp_if_get_lldp_if(&lldp_if.key);
        if (p_db_lldp_if == NULL)
        {
            lldp_if.mode = LLDP_IF_TXRX;
            lldp_if_create(&lldp_if);
        }
    }
    return PM_E_NONE;
}

int32
lldp_init()
{
    _lldp_set_global_default();
    _lldp_set_if_default();

    return PM_E_NONE;
}

int32
lldp_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_LLDP_GLOBAL,            lldp_cmd_process_lldp_global);
    cdb_register_cfg_tbl_cb(TBL_LLDP_IF,                lldp_cmd_process_lldp_if);
    
    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_LOCAL,        lldp_cmd_process_show_local);
    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_NEIGHBOR,     lldp_cmd_process_show_neighbor);
    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_STATISTICS,   lldp_cmd_process_show_statistics);
    cdb_register_cfg_act_cb(ACT_CLEAR_LLDP_STATISTICS,   lldp_cmd_process_clear_statistics);
    
    ipc_register_pkt_rx_fn(GLB_PKT_L2_LLDP,             lldp_pdu_rx);
    
    return PM_E_NONE;
}

int32
lldp_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_LLDP_GLOBAL,            NULL);
    cdb_register_cfg_tbl_cb(TBL_LLDP_IF,                NULL);

    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_LOCAL,        NULL);
    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_NEIGHBOR,     NULL);
    cdb_register_cfg_act_cb(ACT_SHOW_LLDP_STATISTICS,   NULL);
    cdb_register_cfg_act_cb(ACT_CLEAR_LLDP_STATISTICS,   NULL);
    
    ipc_register_pkt_rx_fn(GLB_PKT_L2_LLDP,             NULL);
    
    return PM_E_NONE;
}

