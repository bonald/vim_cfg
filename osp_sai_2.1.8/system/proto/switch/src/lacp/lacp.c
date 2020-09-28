#include "proto.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "switch.h"
#include "if_mgr.h"
#include "if_agg.h"
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "lacp_sm.h"
#include "switch_api.h"


static int32
_lacp_add_if_lacp_init(tbl_interface_t *p_db_if)
{
    ds_lag_t *p_db_lag = p_db_if->lag;
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    p_db_lacp->actor_port_number = p_db_if->ifindex;
    p_db_lacp->actor_port_priority = LACP_DEFAULT_PORT_PRIORITY;
    p_db_lacp->actor_oper_port_key = p_db_lag->lag_id;
    
    p_db_lacp->lacp_enabled = (GLB_DUPLEX_FULL == p_db_if->oper_duplex) ? TRUE : FALSE;
    GLB_SET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_AGGREGATION);
    GLB_SET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_DEFAULTED);

    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_AGGREGATION);
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DEFAULTED);

    if (GLB_AGG_MODE_DYNAMIC_ACTIVE == p_db_lag->mode)
    {
        GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_LACP_ACTIVITY);
    }
    else
    {
        GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_LACP_ACTIVITY);
    }

    return PM_E_NONE;
}

static int32
_lacp_add_if_lacp_deinit(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    /* notify partner before deleted */
    p_db_lacp->ntt = TRUE;
    p_db_lacp->actor_oper_port_state = 0;
    lacp_sm_tx_lacpdu(p_db_if);

    if (NULL == p_db_lacp)
    {
        return PM_E_NONE;
    }

    CTC_TASK_STOP_TIMER(p_db_lacp->current_while_timer);
    CTC_TASK_STOP_TIMER(p_db_lacp->periodic_timer);
    CTC_TASK_STOP_TIMER(p_db_lacp->wait_while_timer);

    if (p_db_lacp->pdu)
    {
        XFREE(MEM_LACP_PDU, p_db_lacp->pdu);
        p_db_lacp->pdu = NULL;
    }
    
    return PM_E_NONE;
}

/* send LACPDU after configuration is changed */
static int32
_lacp_cfg_change_process(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->ntt = TRUE;
    return lacp_sm_tx_lacpdu(p_db_if);
}

static int32
_lacp_compare_system_id_and_priority(tbl_lag_global_t* p_lag_global, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu)
{
#ifdef CONFIG_MLAG
    /* MLAG modification */
    if (p_db_lacp->mlag_id)
    {
        if ((0 == sal_memcmp(p_lag_global->mlag_system_id, p_pdu->partner.system, MAC_ADDR_LEN))
         && (p_lag_global->mlag_system_priority == p_pdu->partner.system_priority))
        {
            return 0;
        }
    }
    else
#endif
    {
        if ((0 == sal_memcmp(p_lag_global->system_id, p_pdu->partner.system, MAC_ADDR_LEN))
         && (p_lag_global->system_priority == p_pdu->partner.system_priority))
        {
            return 0;
        }
    }

    return -1;
}

int32
lacp_add_if_lacp(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem)
{
    ds_lacp_t lacp;
    ds_lacp_t *p_db_lacp = NULL;
    int32 rc = PM_E_NONE;
    
    sal_memset(&lacp, 0, sizeof(lacp));
    lacp.mlag_id = p_db_if_agg->mlag_id;
    PM_E_RETURN(ds_lacp_add_lacp(p_db_if_mem, &lacp));

    _lacp_add_if_lacp_init(p_db_if_mem);
    p_db_lacp = ds_lacp_get_lacp(p_db_if_mem);
    if (NULL == p_db_lacp)
    {
        return PM_E_NO_MEMORY;
    }
    
    ds_lacp_set_lacp_field(p_db_if_mem, p_db_lacp, DS_LACP_FLD_MAX);
    lacp_sm_receive_process_event(p_db_if_mem, RX_EVENT_BEGIN);
    lacp_sm_mux_process_event(p_db_if_mem, MUX_EVENT_BEGIN);

    lacp_sm_pertx_cfg_update_state(p_db_if_mem);
    lacp_sm_receive_cfg_update_state(p_db_if_mem);
    lacp_sm_mux_cfg_update_state(p_db_if_mem);

    PM_E_RETURN(if_agg_aggregator_bw_update(p_db_if_agg));
    PM_E_RETURN(_if_agg_attach_member_sync_prop(p_db_if_agg, p_db_if_mem));

    return PM_E_NONE;
}

int32
lacp_del_if_lacp(tbl_interface_t *p_db_if_mem)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(_lacp_add_if_lacp_deinit(p_db_if_mem));
    PM_E_RETURN(ds_lacp_del_lacp(p_db_if_mem));

    return PM_E_NONE;
}

int32
lacp_set_port_priority(tbl_interface_t *p_db_if, uint16 port_priority)
{
    ds_lacp_t *p_db_lacp = NULL;
    int32 rc = PM_E_NONE;

    p_db_lacp = p_db_if->lacp;
    if (NULL == p_db_lacp)
    {
        return PM_E_NOT_EXIST;
    }
    
    /* 1. process */
    p_db_lacp->actor_port_priority = port_priority;

    /* 2. update local CDB */
    PM_E_RETURN(ds_lacp_set_lacp_field(p_db_if, p_db_lacp, DS_LACP_FLD_ACTOR_PORT_PRIORITY));

    _lacp_cfg_change_process(p_db_if, p_db_lacp);
    return rc;
}

int32
lacp_set_port_short_timeout(tbl_interface_t *p_db_if, uint32 enable)
{
    ds_lacp_t *p_db_lacp = NULL;
    int32 rc = PM_E_NONE;

    p_db_lacp = p_db_if->lacp;
    if (NULL == p_db_lacp)
    {
        return PM_E_NOT_EXIST;
    }
    
    /* 1. process */
    p_db_lacp->short_timeout = enable;
    if (enable)
    {
        GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_LACP_TIMEOUT);
    }
    else
    {
        GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_LACP_TIMEOUT);
    }

    /* 2. update local CDB */
    PM_E_RETURN(ds_lacp_set_lacp_field(p_db_if, p_db_lacp, DS_LACP_FLD_SHORT_TIMEOUT));

    _lacp_cfg_change_process(p_db_if, p_db_lacp);
    return rc;
}

int32
lacp_set_system_priority(uint16 system_priority)
{
    tbl_lag_global_t* p_lag_global = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lacp_t *p_db_lacp = NULL;
    
    p_lag_global = tbl_lag_global_get_lag_global();
    p_lag_global->system_priority = system_priority;
    tbl_lag_global_set_lag_global_field(p_lag_global, TBL_LAG_GLOBAL_FLD_SYSTEM_PRIORITY);

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        p_db_lacp = p_db_if->lacp;
        if (p_db_lacp)
        {
            _lacp_cfg_change_process(p_db_if, p_db_lacp);
        }
    }
    swth_set_lacp_priority(system_priority);

    return PM_E_NONE;
}
    
int32
lacp_set_system_id(const uint8 *system_id)
{
    tbl_lag_global_t* p_lag_global = NULL;
    p_lag_global = tbl_lag_global_get_lag_global();

    sal_memcpy(p_lag_global->system_id, system_id, MAC_ADDR_LEN);
    tbl_lag_global_set_lag_global_field(p_lag_global, TBL_LAG_GLOBAL_FLD_SYSTEM_ID);

    /* system id should not be changed */
    
    return PM_E_NONE;
}

int32
lacp_set_hash_arithmetic(const uint8 hash_arithmetic_type)
{
    int32 rc = PM_E_NONE;
    tbl_lag_global_t *p_lag_global=NULL;
    p_lag_global=tbl_lag_global_get_lag_global();
    
    p_lag_global->hash_arithmetic = hash_arithmetic_type;
    tbl_lag_global_set_lag_global_field(p_lag_global,TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC);
    
    return rc;
}

int32
lacp_set_load_balance_inner_field_en(const uint32 load_balance_inner_field_en)
{
    int32 rc = PM_E_NONE;
    tbl_lag_global_t *p_lag_global=NULL;
    p_lag_global=tbl_lag_global_get_lag_global();
    
    p_lag_global->load_balance_inner_field_en = load_balance_inner_field_en;
    tbl_lag_global_set_lag_global_field(p_lag_global,TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN);
    
    return rc;
}

int32
lacp_set_load_balance_mode(uint8 channel_group_id,uint8 load_balance_mode)
{
    int32 rc = PM_E_NONE;
    tbl_lag_global_t* p_lag_global = NULL;
    p_lag_global = tbl_lag_global_get_lag_global();
    
    p_lag_global->load_balance_mode[channel_group_id] = load_balance_mode;
    tbl_lag_global_set_lag_global_field(p_lag_global,TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE);
    
    return rc;
}

int32
lacp_set_load_balance(uint32 load_balance_flag)
{
    int32 rc = PM_E_NONE;
    tbl_lag_global_t* p_lag_global = NULL;
    p_lag_global = tbl_lag_global_get_lag_global();

    p_lag_global->load_balance = load_balance_flag;
    //PM_E_RETURN(fei_agg_set_load_balance(load_balance_flag));
    tbl_lag_global_set_lag_global_field(p_lag_global, TBL_LAG_GLOBAL_FLD_LOAD_BALANCE);    

    return rc;
}

int32
lacp_clear_counters(uint32 lag_id)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if->lacp)
        {
            if (0 == lag_id || p_db_if->lag->lag_id == lag_id)
            {
                p_db_if->lacp->tx_lacp_count = 0;
                p_db_if->lacp->tx_error_count = 0;
                p_db_if->lacp->rx_lacp_count = 0;
                p_db_if->lacp->rx_error_count = 0;
            }
        }
    }

    return PM_E_NONE;
}

int32
lacp_init_default_config(uint8 *system_id)
{
    lacp_set_system_priority(LACP_DEFAULT_SYSTEM_PRIORITY);
#ifndef TSINGMA
    lacp_set_load_balance(GLB_AGG_LOAD_BALANCE_CTC_DEFAULT);
#endif

#ifdef TAPPRODUCT
    if(check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
    {
        lacp_set_load_balance_inner_field_en(GLB_AGG_LOAD_BALANCE_INNER_FIELD_EN);
    }
#endif
    lacp_set_system_id(system_id);

    return PM_E_NONE;    
}

int32
lacp_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_LAG_GLOBAL, lacp_cmd_process_lag_global);
    ipc_register_pkt_rx_fn(GLB_PKT_LACP, lacp_pdu_rx);
    cdb_register_cfg_act_cb(ACT_CLEAR_LACP_COUNTERS, lacp_cmd_process_clear_counters);
    return PM_E_NONE;
}

int32
lacp_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_LAG_GLOBAL, NULL);
    cdb_register_cfg_act_cb(ACT_CLEAR_LACP_COUNTERS, NULL);
    return PM_E_NONE;
}

/* called when if UP/RUNNING changes */
int32
lacp_if_change_process(tbl_interface_t *p_db_if)
{
    if (p_db_if->lacp)
    {
        p_db_if->lacp->lacp_enabled = (GLB_DUPLEX_FULL == p_db_if->oper_duplex) ? TRUE : FALSE;
        lacp_sm_pertx_cfg_update_state(p_db_if);
        lacp_sm_receive_cfg_update_state(p_db_if);
    }
    
    return PM_E_NONE;
}

int32
lacp_set_port_unselected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->selected = SELECTED_UNSELECTED;
    
    if (MUX_WAITING == p_db_lacp->mux_state 
     || MUX_ATTACHED == p_db_lacp->mux_state
     || MUX_COLLECTING_DISTRIBUTING == p_db_lacp->mux_state)
    {
        lacp_sm_mux_process_event(p_db_if, MUX_EVENT_UNSELECTED);
    }
    
    return PM_E_NONE;
}

/* 43.4.9 recordPDU */
int32
lacp_record_pdu(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu)
{
    tbl_lag_global_t *p_lag_glb = tbl_lag_global_get_lag_global();
    uint32 sync = FALSE;
    uint32 maintaining = FALSE;
    int32 system_id_and_pri_cmp = 0;

    system_id_and_pri_cmp = _lacp_compare_system_id_and_priority(p_lag_glb, p_db_lacp, p_pdu);

    /* 43.4.9 recordPDU paragraph 5 */
    maintaining = (LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_pdu->actor.state)
        || (LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state) && LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_pdu->partner.state)));
  
    /* recordPDU paragraph 1 */
    p_db_lacp->partner_oper_port_number = p_pdu->actor.port;
    p_db_lacp->partner_oper_port_priority = p_pdu->actor.port_priority;
    sal_memcpy(p_db_lacp->partner_oper_system, p_pdu->actor.system, MAC_ADDR_LEN);
    p_db_lacp->partner_oper_system_priority = p_pdu->actor.system_priority;
    p_db_lacp->partner_oper_key = p_pdu->actor.key;
    /* Partner_Oper_Port_State variables with the exception of Synchronization */
    p_db_lacp->partner_oper_port_state = p_pdu->actor.state;
    
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DEFAULTED);

    /* 43.4.9 recordPDU paragraph 2 */
    /* Modified by kcao for bug 49513 2018-10-19 allow partner oper key is 0 */
    sync = ((p_db_lacp->actor_port_number == p_pdu->partner.port)
        && (p_db_lacp->actor_port_priority == p_pdu->partner.port_priority)
        && (0 == system_id_and_pri_cmp)
        && ((p_db_lacp->actor_oper_port_key == p_pdu->partner.key))        
        && (GET_AGGREGATION(p_db_lacp->actor_oper_port_state) == GET_AGGREGATION(p_pdu->partner.state))
        && (LACP_IN_SYNC == GET_SYNCHRONIZATION(p_pdu->actor.state))
        && maintaining)
        /* 43.4.9 recordPDU paragraph 3 */
          || ((GET_AGGREGATION(p_pdu->actor.state) == LACP_INDIVIDUAL)
        && (GET_SYNCHRONIZATION (p_pdu->actor.state) == LACP_IN_SYNC) 
        && maintaining);

    if (sync)
    {
        GLB_SET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_SYNCHRONIZATION);
    }
    else
    {
        GLB_UNSET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_SYNCHRONIZATION);
    }
    
    return PM_E_NONE;
}

/* 43.4.9 recordDefault */
int32
lacp_record_default(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->partner_oper_port_number = p_db_lacp->partner_admin_port_number;
    p_db_lacp->partner_oper_port_priority = p_db_lacp->partner_admin_port_priority;
    sal_memcpy(p_db_lacp->partner_oper_system, p_db_lacp->partner_admin_system, MAC_ADDR_LEN);
    p_db_lacp->partner_oper_system_priority = p_db_lacp->partner_admin_system_priority;
    p_db_lacp->partner_oper_key = p_db_lacp->partner_admin_key;
    p_db_lacp->partner_oper_port_state = p_db_lacp->partner_admin_port_state;
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DEFAULTED);

    return PM_E_NONE;
}

/* 43.4.9 update_Selected */
int32
lacp_update_selected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu)
{
    if ((p_db_lacp->partner_oper_port_number != p_pdu->actor.port)
     || (p_db_lacp->partner_oper_port_priority != p_pdu->actor.port_priority)
     || (sal_memcmp(p_db_lacp->partner_oper_system, p_pdu->actor.system, MAC_ADDR_LEN))
     || (p_db_lacp->partner_oper_system_priority != p_pdu->actor.system_priority)
     || (p_db_lacp->partner_oper_key != p_pdu->actor.key)
     || (GET_AGGREGATION(p_db_lacp->partner_oper_port_state) != GET_AGGREGATION(p_pdu->actor.state)))
    {
        lacp_set_port_unselected(p_db_if, p_db_lacp);
    }
    
    return PM_E_NONE;
}

/* 43.4.9 update_Default_Selected */
int32
lacp_update_default_selected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    if ((p_db_lacp->partner_oper_port_number != p_db_lacp->partner_admin_port_number)
     || (p_db_lacp->partner_oper_port_priority != p_db_lacp->partner_admin_port_priority)
     || (sal_memcmp(p_db_lacp->partner_oper_system, p_db_lacp->partner_admin_system, MAC_ADDR_LEN))
     || (p_db_lacp->partner_oper_system_priority != p_db_lacp->partner_admin_system_priority)
     || (p_db_lacp->partner_oper_key != p_db_lacp->partner_admin_key)
     || (GLB_FLAG_ISSET(p_db_lacp->partner_oper_port_state, STATE_FLAG_AGGREGATION) != GLB_FLAG_ISSET(p_db_lacp->partner_admin_port_state, STATE_FLAG_AGGREGATION)))
    {
        lacp_set_port_unselected(p_db_if, p_db_lacp);
    }

    return PM_E_NONE;
}
    
/* 43.4.9 update_NTT */
int32
lacp_update_ntt(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu)
{
    tbl_lag_global_t* p_lag_global = NULL;
    p_lag_global = tbl_lag_global_get_lag_global();
    int32 system_id_and_pri_cmp = 0;

    system_id_and_pri_cmp = _lacp_compare_system_id_and_priority(p_lag_global, p_db_lacp, p_pdu);
    if ((p_db_lacp->actor_port_number != p_pdu->partner.port)
     || (p_db_lacp->actor_port_priority != p_pdu->partner.port_priority)
     || system_id_and_pri_cmp
     || (p_db_lacp->actor_oper_port_key != p_pdu->partner.key)
     || (GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state) != GET_LACP_ACTIVITY(p_pdu->partner.state))
     || (GET_LACP_TIMEOUT(p_db_lacp->actor_oper_port_state) != GET_LACP_TIMEOUT(p_pdu->partner.state))
     || (GET_SYNCHRONIZATION(p_db_lacp->actor_oper_port_state) != GET_SYNCHRONIZATION(p_pdu->partner.state))
     || (GET_AGGREGATION(p_db_lacp->actor_oper_port_state) != GET_AGGREGATION(p_pdu->partner.state)))
    {
        p_db_lacp->ntt = TRUE;
        lacp_sm_tx_lacpdu(p_db_if);
    }
    
    return PM_E_NONE;
}

/* 43.4.9 Attach_Mux_To_Aggregator */
int32
lacp_attach_mux_to_aggregator(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    if (SELECTED_UNSELECTED == p_db_lacp->selected || SELECTED_STANDBY == p_db_lacp->selected)
    {
        lacp_sm_mux_process_event(p_db_if, MUX_EVENT_UNSELECTED);
    }

    return PM_E_NONE;
}

/* 43.4.9 Detach_Mux_From_Aggregator */
int32
lacp_detach_mux_from_aggregator(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    /* clear partner info */
    p_db_lacp->partner_oper_system_priority = 0;    
    sal_memset(p_db_lacp->partner_oper_system, 0, MAC_ADDR_LEN);
    p_db_lacp->partner_oper_key = 0;
    p_db_lacp->partner_oper_port_priority = 0;
    p_db_lacp->partner_oper_port_number = 0;
    p_db_lacp->partner_oper_port_state = 0;
    
    return PM_E_NONE;
}

/* 43.4.9 Enable_Collecting_Distributing : will call HAL API to set Chip */
int32
lacp_enable_collecting_distributing(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_if_agg = NULL;
    ds_lag_t *p_mem_lag = p_db_if->lag;
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_mem_lag->lag_id);
    p_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_if_agg || NULL == p_if_agg->lag)
    {
        return PM_E_NOT_EXIST;
    }

    PM_E_RETURN(if_agg_attach_mux_to_aggregator(p_if_agg, p_db_if));
    return PM_E_NONE;
}

/* 43.4.9 Disable_Collecting_Distributing : will call HAL API to set Chip */
int32
lacp_disable_collecting_distributing(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_if_agg = NULL;
    ds_lag_t *p_mem_lag = p_db_if->lag;
    int32 rc = PM_E_NONE;
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_mem_lag->lag_id);
    p_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_if_agg || NULL == p_if_agg->lag)
    {
        return PM_E_NOT_EXIST;
    }

    PM_E_RETURN(if_agg_detach_mux_from_aggregator(p_if_agg, p_db_if));
    return PM_E_NONE;
}

int32
lacp_set_mlag_sysid(uint8 *mlag_sysid)
{
    tbl_lag_global_t *p_lag_glb = tbl_lag_global_get_lag_global();

    sal_memcpy(p_lag_glb->mlag_system_id, mlag_sysid, MAC_ADDR_LEN);
    tbl_lag_global_set_lag_global_field(p_lag_glb, TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_ID);
    
    return PM_E_NONE;
}

int32
lacp_set_mlag_syspri(uint16 mlag_syspri)
{
    tbl_lag_global_t *p_lag_glb = tbl_lag_global_get_lag_global();

    p_lag_glb->mlag_system_priority = mlag_syspri;
    tbl_lag_global_set_lag_global_field(p_lag_glb, TBL_LAG_GLOBAL_FLD_MLAG_SYSTEM_PRIORITY);
    
    return PM_E_NONE;
}

int32
lacp_set_mlag_id(tbl_interface_t *p_db_if_agg, uint32 mlag_id)
{
    tbl_interface_t *p_db_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ds_lag_t *p_lag = NULL;

    p_lag = p_db_if_agg->lag;
    if (NULL == p_lag)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_db_if_mem, listnode)
    {
        if (p_db_if_mem->lacp)
        {
            p_db_if_mem->lacp->mlag_id = mlag_id;
            ds_lacp_set_lacp_field(p_db_if_mem, p_db_if_mem->lacp, DS_LACP_FLD_MLAG_ID);
            if (mlag_id)
            {
                p_db_if_mem->lacp->actor_oper_port_key = (LACP_MLAG_FLAG | mlag_id);
            }
            else
            {
                p_db_if_mem->lacp->actor_oper_port_key = p_lag->lag_id;
            }
            ds_lacp_set_lacp_field(p_db_if_mem, p_db_if_mem->lacp, DS_LACP_FLD_ACTOR_OPER_PORT_KEY);
        }
    }
    
    return PM_E_NONE;
}

