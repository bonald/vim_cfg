
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "switch.h"
#include "lacp_define.h"
#include "lacp_sm.h"
#include "lacp_pdu.h"
#include "lacp.h"


uint32 g_lacp_debug_flag = TRUE;

static int32
_lacp_sm_selection_logic(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);


#define __43_4_12_RECEIVE_MACHINE_

uint32 lacp_sm_receive_state_table[RX_EVENT_MAX][RX_STATE_MAX] = 
{
  /* RX_INITIALIZE     RX_PORT_DISABLED  RX_LACP_DISABLED  RX_EXPIRED        RX_DEFAULTED      RX_CURRENT */
    {RX_INITIALIZE,    RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX},     /* RX_EVENT_BEGIN */
    {RX_STATE_MAX,     RX_INITIALIZE,    RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX},     /* RX_EVENT_PORT_MOVED */
    {RX_STATE_MAX,     RX_EXPIRED,       RX_EXPIRED,       RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX},     /* RX_EVENT_PORTENABLED_LACPENABLED */
    {RX_STATE_MAX,     RX_LACP_DISABLED, RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX},     /* RX_EVENT_PORTENABLED_LACPDISABLED */
    {RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_DEFAULTED,     RX_STATE_MAX,     RX_EXPIRED},       /* RX_EVENT_CURRWHILE_TIMER_EXPIRED */
    {RX_STATE_MAX,     RX_STATE_MAX,     RX_STATE_MAX,     RX_CURRENT,       RX_CURRENT,       RX_CURRENT},       /* RX_EVENT_PDU_RX */
    {RX_PORT_DISABLED, RX_PORT_DISABLED, RX_PORT_DISABLED, RX_PORT_DISABLED, RX_PORT_DISABLED, RX_PORT_DISABLED}, /* RX_EVENT_PORT_DISABLED */
};

static void
_lacp_sm_receive_current_while_timer_cb(void* p_data)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data;
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    if (p_db_lacp)
    {
        p_db_lacp->current_while_timer = NULL;
    }

    lacp_sm_receive_process_event(p_db_if, RX_EVENT_CURRWHILE_TIMER_EXPIRED);
    return;
}

static int32
_lacp_sm_receive_update_event(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 port_enabled = FALSE;

    port_enabled = tbl_interface_is_running(p_db_if);
    
    if (!p_db_lacp->lacp_enabled) 
    {      
        if (PERTX_NO_PERIODIC != p_db_lacp->periodic_tx_state) 
        {
            lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_DISABLE);
        }
    }
    else if (!port_enabled)
    {
        if (PERTX_NO_PERIODIC != p_db_lacp->periodic_tx_state)
        {
            lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_DISABLE);
        }
    }
    else if (LACP_PASSIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state)
        && LACP_PASSIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->partner_oper_port_state))
    {
        if(PERTX_NO_PERIODIC != p_db_lacp->periodic_tx_state)
        {
            lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_DISABLE);
        }
    }
    else if (LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state)
        || LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->partner_oper_port_state))
    {
        if (PERTX_NO_PERIODIC == p_db_lacp->periodic_tx_state)
        {
            lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_ENABLE);
        }
    }
    
    if (LACP_IN_SYNC == GET_SYNCHRONIZATION(p_db_lacp->partner_oper_port_state))
    {
        if (MUX_ATTACHED == p_db_lacp->mux_state)
        {
            if (SELECTED_SELECTED == p_db_lacp->selected)
            {
                lacp_sm_mux_process_event(p_db_if, MUX_EVENT_SELECTED_PARTNER_INSYNC);
            }
        }
    } 
    else 
    {
        if (MUX_COLLECTING_DISTRIBUTING == p_db_lacp->mux_state)
        {
            lacp_sm_mux_process_event(p_db_if, MUX_EVENT_PARTNER_OUTOFSYNC);
        }
    }

    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_initialize(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->receive_state = RX_INITIALIZE;

    lacp_record_default(p_db_if, p_db_lacp);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_EXPIRED);
    p_db_lacp->port_moved = FALSE;

    lacp_set_port_unselected(p_db_if, p_db_lacp);

    /* move to port disable state */
    lacp_sm_receive_process_event(p_db_if, RX_EVENT_PORT_DISABLED);

    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_port_disabled(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{  
    p_db_lacp->receive_state = RX_PORT_DISABLED;

    GLB_UNSET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_SYNCHRONIZATION);

    if (p_db_lacp->port_moved)
    {
        lacp_sm_receive_process_event(p_db_if, RX_EVENT_PORT_MOVED);
    }

    _lacp_sm_receive_update_event(p_db_if, p_db_lacp);

    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_lacp_disabled(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{  
    p_db_lacp->receive_state = RX_LACP_DISABLED;

    lacp_record_default(p_db_if, p_db_lacp);
    GLB_UNSET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_AGGREGATION);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_EXPIRED);

    lacp_set_port_unselected(p_db_if, p_db_lacp);
        
    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_expired(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 restart_timer = FALSE;
    p_db_lacp->receive_state = RX_EXPIRED;

    GLB_UNSET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_SYNCHRONIZATION);
    GLB_SET_FLAG(p_db_lacp->partner_oper_port_state, STATE_FLAG_LACP_TIMEOUT);

    if (p_db_lacp->current_while_timer)
    {
        restart_timer = TRUE;
        CTC_TASK_STOP_TIMER(p_db_lacp->current_while_timer);
    }
    
    p_db_lacp->current_while_timer = ctc_task_add_timer(TASK_PRI_NORMAL, 
        _lacp_sm_receive_current_while_timer_cb, p_db_if, SHORT_TIMEOUT_TIME);
    
    LACP_DEBUG(LACP_TIMER, "Interface %s %s timer %s with %u seconds", p_db_if->key.name,
            restart_str(restart_timer),
            "current_while_timer",
            SHORT_TIMEOUT_TIME);
    
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_EXPIRED);

    _lacp_sm_receive_update_event(p_db_if, p_db_lacp);

    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_defaulted(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{  
    p_db_lacp->receive_state = RX_DEFAULTED;

    lacp_update_default_selected(p_db_if, p_db_lacp);
    lacp_record_default(p_db_if, p_db_lacp);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_EXPIRED);
    
    _lacp_sm_receive_update_event(p_db_if, p_db_lacp);

    return PM_E_NONE;
}

static int32
_lacp_sm_receive_state_current(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 restart_timer = FALSE;
    lacp_pdu_t *p_pdu = NULL;
    uint32 timeout = 0;
    
    p_db_lacp->receive_state = RX_CURRENT;
    p_pdu = (lacp_pdu_t*)p_db_lacp->pdu;
    
    lacp_update_selected(p_db_if, p_db_lacp, p_pdu);
    lacp_update_ntt(p_db_if, p_db_lacp, p_pdu);
    lacp_record_pdu(p_db_if, p_db_lacp, p_pdu);

    /* the first packet will result in change to UNSELECTED state */
    if (SELECTED_UNSELECTED == p_db_lacp->selected)
    {
        _lacp_sm_selection_logic(p_db_if, p_db_lacp);
    }
  
    timeout = (LACP_SHORT_TIMEOUT == GET_LACP_TIMEOUT(p_db_lacp->actor_oper_port_state)
        ? SHORT_TIMEOUT_TIME : LONG_TIMEOUT_TIME);
         
    if (p_db_lacp->current_while_timer)
    {
        restart_timer = TRUE;
        CTC_TASK_STOP_TIMER(p_db_lacp->current_while_timer);
    }
    
    p_db_lacp->current_while_timer = ctc_task_add_timer(TASK_PRI_NORMAL, 
        _lacp_sm_receive_current_while_timer_cb, p_db_if, timeout);

    LACP_DEBUG(LACP_TIMER, "Interface %s %s timer %s with %u seconds", p_db_if->key.name,
            restart_str(restart_timer),
            "current_while_timer",
            timeout);

    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_EXPIRED);

    _lacp_sm_receive_update_event(p_db_if, p_db_lacp);

    return PM_E_NONE;
}

int32
lacp_sm_receive_process_event(tbl_interface_t *p_db_if, lacp_rx_event_t event)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    lacp_rx_state_t next_state = 0;

    if (event >= RX_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  lacp_sm_receive_state_table[event][p_db_lacp->receive_state];
    LACP_DEBUG(LACP_PROTO, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "Receive SM",
            p_db_if->key.name,
            cdb_enum_val2str(lacp_rx_event_strs, RX_EVENT_MAX, event),
            cdb_enum_val2str(lacp_rx_state_strs, RX_STATE_MAX, p_db_lacp->receive_state),
            cdb_enum_val2str(lacp_rx_state_strs, RX_STATE_MAX, next_state));

    switch (next_state)
    {
    case RX_INITIALIZE:
        _lacp_sm_receive_state_initialize(p_db_if, p_db_lacp);
        break;
    case RX_PORT_DISABLED:
        _lacp_sm_receive_state_port_disabled(p_db_if, p_db_lacp);
        break;
    case RX_LACP_DISABLED:
        _lacp_sm_receive_state_lacp_disabled(p_db_if, p_db_lacp);
        break;
    case RX_EXPIRED:
        _lacp_sm_receive_state_expired(p_db_if, p_db_lacp);
        break;
    case RX_DEFAULTED:
        _lacp_sm_receive_state_defaulted(p_db_if, p_db_lacp);
        break;
    case RX_CURRENT:
        _lacp_sm_receive_state_current(p_db_if, p_db_lacp);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

int32
lacp_sm_receive_cfg_update_state(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    uint32 port_enabled = FALSE;
    lacp_rx_event_t rx_event = 0;
    
    port_enabled = tbl_interface_is_running(p_db_if);
    if (port_enabled)
    {
        if (RX_PORT_DISABLED == p_db_lacp->receive_state)
        {
            if (p_db_lacp->lacp_enabled)
            {
                rx_event = RX_EVENT_PORTENABLED_LACPENABLED;
            }
            else
            {
                rx_event = RX_EVENT_PORTENABLED_LACPDISABLED;
            }
            lacp_sm_receive_process_event(p_db_if, rx_event);
        }
        
        /* RX_LACP_DISABLED is dead in 43.4.12, so add a condition for this state */
        if (RX_LACP_DISABLED == p_db_lacp->receive_state)
        {
            if (p_db_lacp->lacp_enabled)
            {
                rx_event = RX_EVENT_PORTENABLED_LACPENABLED;
            }
            lacp_sm_receive_process_event(p_db_if, rx_event);
        }
    }
    else
    {
        if (RX_PORT_DISABLED != p_db_lacp->receive_state)
        {
            rx_event = RX_EVENT_PORT_DISABLED;
            lacp_sm_receive_process_event(p_db_if, rx_event);
        }

        if (SELECTED_SELECTED == p_db_lacp->selected)
        {
            lacp_set_port_unselected(p_db_if, p_db_lacp);
            lacp_record_default(p_db_if, p_db_lacp);
        }
    }
    
    return PM_E_NONE;
}

#define _43_4_13_PERIODIC_TRANSMISSION_MACHINE_

uint32 lacp_sm_pertx_state_table[PERTX_EVENT_MAX][PERTX_STATE_MAX] = 
{
  /* PERTX_NO_PERIODIC    PERTX_FAST_PERIODIC  PERTX_SLOW_PERIODIC PERTX_PERIODIC_TX */
    {PERTX_NO_PERIODIC,   PERTX_NO_PERIODIC,   PERTX_NO_PERIODIC,  PERTX_NO_PERIODIC},   /* PERTX_EVENT_DISABLE */
    {PERTX_FAST_PERIODIC, PERTX_STATE_MAX,     PERTX_STATE_MAX,    PERTX_STATE_MAX},     /* PERTX_EVENT_ENABLE */
    {PERTX_STATE_MAX,     PERTX_STATE_MAX,     PERTX_PERIODIC_TX,  PERTX_FAST_PERIODIC}, /* PERTX_EVENT_SHORT_TIMEOUT */
    {PERTX_STATE_MAX,     PERTX_SLOW_PERIODIC, PERTX_STATE_MAX,    PERTX_SLOW_PERIODIC}, /* PERTX_EVENT_LONG_TIMEOUT */
    {PERTX_STATE_MAX,     PERTX_PERIODIC_TX,   PERTX_PERIODIC_TX,  PERTX_STATE_MAX},     /* PERTX_EVENT_PERIODIC_TIME_EXPIRED */
};

static void
_lacp_sm_pertx_timer_cb(void* p_data)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data;
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    if (p_db_lacp)
    {
        p_db_lacp->periodic_timer = NULL;
    }

    lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_PERIODIC_TIME_EXPIRED);
    return;
}

static int32
_lacp_sm_pertx_state_fast_periodic(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 restart_timer = FALSE;
    p_db_lacp->periodic_tx_state = PERTX_FAST_PERIODIC;

    LACP_DEBUG(LACP_PROTO, "%s enter fast periodic %u seconds", p_db_if->key.name, FAST_PERIODIC_TIME);

    if (p_db_lacp->periodic_timer)
    {
        restart_timer = TRUE;
        CTC_TASK_STOP_TIMER(p_db_lacp->periodic_timer);
    }
    p_db_lacp->periodic_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lacp_sm_pertx_timer_cb, p_db_if, FAST_PERIODIC_TIME);

    LACP_DEBUG(LACP_TIMER, "Interface %s %s timer %s with %u seconds", p_db_if->key.name,
            restart_str(restart_timer),
            "periodic_timer",
            FAST_PERIODIC_TIME);

    return PM_E_NONE;
}

static int32
_lacp_sm_pertx_state_slow_periodic(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 restart_timer = FALSE;
    p_db_lacp->periodic_tx_state = PERTX_SLOW_PERIODIC;

    LACP_DEBUG(LACP_PROTO, "%s enter slow periodic %u seconds", p_db_if->key.name, SLOW_PERIODIC_TIME);

    if (p_db_lacp->periodic_timer)
    {
        restart_timer = TRUE;
        CTC_TASK_STOP_TIMER(p_db_lacp->periodic_timer);
    }
    p_db_lacp->periodic_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lacp_sm_pertx_timer_cb, p_db_if, SLOW_PERIODIC_TIME);

    LACP_DEBUG(LACP_TIMER, "Interface %s %s timer %s with %u seconds", p_db_if->key.name,
            restart_str(restart_timer),
            "periodic_timer",
            SLOW_PERIODIC_TIME);
    
    return PM_E_NONE;
}

static int32
_lacp_sm_pertx_state_no_periodic(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->periodic_tx_state = PERTX_NO_PERIODIC;

    LACP_DEBUG(LACP_PROTO, "%s enter no periodic", p_db_if->key.name);

    CTC_TASK_STOP_TIMER(p_db_lacp->periodic_timer);

    return PM_E_NONE;
}

static int32
_lacp_sm_pertx_state_periodic_tx(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 short_timeout = FALSE;
    
    p_db_lacp->periodic_tx_state = PERTX_PERIODIC_TX;
    short_timeout = (LACP_SHORT_TIMEOUT == GET_LACP_TIMEOUT(p_db_lacp->partner_oper_port_state));

    LACP_DEBUG(LACP_PROTO, "%s enter periodic TX, timeout is %s", p_db_if->key.name, 
        short_timeout ? "short" : "long");
    
    p_db_lacp->ntt = TRUE;
    lacp_sm_tx_lacpdu(p_db_if);
    
    if (short_timeout)
    {
        lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_SHORT_TIMEOUT);
    }
    else
    {
        lacp_sm_pertx_process_event(p_db_if, PERTX_EVENT_LONG_TIMEOUT);
    }

    return PM_E_NONE;
}

int32
lacp_sm_pertx_process_event(tbl_interface_t *p_db_if, lacp_pertx_event_t event)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    lacp_pertx_state_t next_state = 0;

    if (event >= PERTX_EVENT_MAX)
    {
        return PM_E_FAIL;
    }

    next_state =  lacp_sm_pertx_state_table[event][p_db_lacp->periodic_tx_state];
    LACP_DEBUG(LACP_PROTO, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "PeriodicTX SM",
            p_db_if->key.name,
            cdb_enum_val2str(lacp_pertx_event_strs, PERTX_EVENT_MAX, event),
            cdb_enum_val2str(lacp_pertx_state_strs, PERTX_STATE_MAX, p_db_lacp->periodic_tx_state),
            cdb_enum_val2str(lacp_pertx_state_strs, PERTX_STATE_MAX, next_state));
    
    switch (next_state)
    {
    case PERTX_NO_PERIODIC:
        _lacp_sm_pertx_state_no_periodic(p_db_if, p_db_lacp);
        break;
    case PERTX_FAST_PERIODIC:
        _lacp_sm_pertx_state_fast_periodic(p_db_if, p_db_lacp);
        break;
    case PERTX_SLOW_PERIODIC:
        _lacp_sm_pertx_state_slow_periodic(p_db_if, p_db_lacp);
        break;
    case PERTX_PERIODIC_TX:
        _lacp_sm_pertx_state_periodic_tx(p_db_if, p_db_lacp);
        break;
    default:
        break;
    }

    return PM_E_NONE;
}

int32
lacp_sm_pertx_cfg_update_state(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    uint32 port_enabled = FALSE;
    lacp_pertx_event_t event = PERTX_EVENT_ENABLE;
    
    port_enabled = tbl_interface_is_running(p_db_if);
    if (!p_db_lacp->lacp_enabled || !port_enabled)
    {
        event = PERTX_EVENT_DISABLE;
    }
    else if (LACP_PASSIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state)
        && LACP_PASSIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->partner_oper_port_state))
    {
        event = PERTX_EVENT_DISABLE;
    }
    else if (LACP_ACTIVE_LACP == GET_LACP_ACTIVITY(p_db_lacp->actor_oper_port_state))
    {
        event = PERTX_EVENT_ENABLE;
    }

    lacp_sm_pertx_process_event(p_db_if, event);
    
    return PM_E_NONE;
}

#define _43_4_14_SELECTION_LOGIC_

static int32
_lacp_sm_check_selection_logic_match(ds_lacp_t *p_selected_lacp, ds_lacp_t *p_new_lacp)
{
    if (p_selected_lacp->actor_oper_port_key != p_new_lacp->actor_oper_port_key)
    {
        return PM_E_FAIL;
    }

    if (p_selected_lacp->partner_oper_system_priority != p_new_lacp->partner_oper_system_priority)
    {
        return PM_E_FAIL;
    }
    if (0 != sal_memcmp(p_selected_lacp->partner_oper_system, p_new_lacp->partner_oper_system, MAC_ADDR_LEN))
    {
        return PM_E_FAIL;
    }
    if (p_selected_lacp->partner_oper_key != p_new_lacp->partner_oper_key)
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;
}

static int32
_lacp_sm_selection_logic(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_if_agg = NULL;
    ds_lag_t *p_agg_lag = NULL;
    ds_lag_t *p_mem_lag = p_db_if->lag;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ds_lacp_t *p_selected_lacp = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_mem_lag->lag_id);
    p_if_agg = tbl_interface_get_interface(&if_key);
    if (NULL == p_if_agg || NULL == p_if_agg->lag)
    {
        return PM_E_NOT_EXIST;
    }

#if 1
    /* Deleted by kcao for bug 49513 2018-10-19 allow partner oper key is 0 */
    /* not receive any LACPDU, should not be selected */
    if (0 == p_db_lacp->partner_oper_key)
    {
        return PM_E_NONE;
    }
#endif
    
    p_agg_lag = p_if_agg->lag;
    CTCLIB_SLIST_LOOP(p_agg_lag->member_ports.obj_list, p_if_mem, listnode)
    {
        p_selected_lacp = p_if_mem->lacp;
        if (NULL == p_selected_lacp)
        {
            continue;
        }
        
        if (SELECTED_SELECTED == p_selected_lacp->selected)
        {
            rc = _lacp_sm_check_selection_logic_match(p_selected_lacp, p_db_lacp);
            if (rc < 0)
            {
                /* not match with previous selected member */
                return PM_E_NONE;
            }
        }
    }

    /* for LACP_MAX_ATTACH_LINKS equal to GLB_LINKAGG_MAX_MEMBER, not implement select_count
     * the STANDBY state will not happen
     */
    if (p_agg_lag->bundle_ports_count <= LACP_MAX_ATTACH_LINKS)
    {
        p_db_lacp->selected = SELECTED_SELECTED;
        lacp_sm_mux_process_event(p_db_if, MUX_EVENT_SELECTED);
    }
    else
    {
        p_db_lacp->selected = SELECTED_STANDBY;
        lacp_sm_mux_process_event(p_db_if, MUX_EVENT_STANDBY);
    }

    return PM_E_NONE;
}

#define __43_4_15_MUX_MACHINE_

uint32 lacp_sm_mux_state_table[MUX_EVENT_MAX][MUX_STATE_MAX] = 
{
  /* MUX_DETACHED    MUX_WAITING     MUX_ATTACHED                 MUX_COLLECTING_DISTRIBUTING */
    {MUX_DETACHED,   MUX_STATE_MAX,  MUX_STATE_MAX,               MUX_STATE_MAX},   /* MUX_EVENT_BEGIN */
    {MUX_WAITING,    MUX_STATE_MAX,  MUX_STATE_MAX,               MUX_STATE_MAX},   /* MUX_EVENT_SELECTED */
    {MUX_WAITING,    MUX_STATE_MAX,  MUX_DETACHED,                MUX_ATTACHED},    /* MUX_EVENT_STANDBY */
    {MUX_STATE_MAX,  MUX_DETACHED,   MUX_DETACHED,                MUX_ATTACHED},    /* MUX_EVENT_UNSELECTED */
    {MUX_STATE_MAX,  MUX_ATTACHED,   MUX_STATE_MAX,               MUX_STATE_MAX},   /* MUX_EVENT_SELECTED_READYTRUE */
    {MUX_STATE_MAX,  MUX_STATE_MAX,  MUX_COLLECTING_DISTRIBUTING, MUX_STATE_MAX},   /* MUX_EVENT_SELECTED_PARTNER_INSYNC */
    {MUX_STATE_MAX,  MUX_STATE_MAX,  MUX_STATE_MAX,               MUX_ATTACHED},    /* MUX_EVENT_PARTNER_OUTOFSYNC */
};

static void
_lacp_sm_wait_while_timer_cb(void* p_data)
{
    tbl_interface_t *p_db_if = (tbl_interface_t*)p_data;
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    if (p_db_lacp)
    {
        p_db_lacp->wait_while_timer = NULL;
    }

    if (MUX_WAITING == p_db_lacp->mux_state && SELECTED_SELECTED == p_db_lacp->selected)
    {
        p_db_lacp->ready_n = TRUE;
        p_db_lacp->ntt = TRUE;
        lacp_sm_mux_process_event(p_db_if, MUX_EVENT_SELECTED_READYTRUE);
    }

    return;
}

static int32
_lacp_sm_mux_state_detached(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{  
    p_db_lacp->mux_state = MUX_DETACHED;

    lacp_detach_mux_from_aggregator(p_db_if, p_db_lacp);
    lacp_disable_collecting_distributing(p_db_if, p_db_lacp);

    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_SYNCHRONIZATION);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_COLLECTING);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DISTRIBUTING);
    p_db_lacp->ntt = TRUE;
    lacp_sm_tx_lacpdu(p_db_if);
    
    return PM_E_NONE;
}

static int32
_lacp_sm_mux_state_waiting(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    uint32 restart_timer = FALSE;
    p_db_lacp->mux_state = MUX_WAITING;

    if (p_db_lacp->wait_while_timer)
    {
        restart_timer = TRUE;
        CTC_TASK_STOP_TIMER(p_db_lacp->wait_while_timer);
    }
    
    p_db_lacp->wait_while_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _lacp_sm_wait_while_timer_cb, p_db_if, AGGREGATE_WAIT_TIME);

    LACP_DEBUG(LACP_TIMER, "Interface %s %s timer %s with %u seconds", p_db_if->key.name,
            restart_str(restart_timer),
            "wait_while_timer",
            AGGREGATE_WAIT_TIME);

    return PM_E_NONE;
}

static int32
_lacp_sm_mux_state_attached(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->mux_state = MUX_ATTACHED;

    lacp_attach_mux_to_aggregator(p_db_if, p_db_lacp);
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_SYNCHRONIZATION);

    lacp_disable_collecting_distributing(p_db_if, p_db_lacp);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_COLLECTING);
    GLB_UNSET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DISTRIBUTING);
    p_db_lacp->ntt = TRUE;
    lacp_sm_tx_lacpdu(p_db_if);
    
    return PM_E_NONE;
}

static int32
_lacp_sm_mux_state_collecting_distributing(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp)
{
    p_db_lacp->mux_state = MUX_COLLECTING_DISTRIBUTING;

    lacp_enable_collecting_distributing(p_db_if, p_db_lacp);
    
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_COLLECTING);
    GLB_SET_FLAG(p_db_lacp->actor_oper_port_state, STATE_FLAG_DISTRIBUTING);
    p_db_lacp->ntt = TRUE;
    lacp_sm_tx_lacpdu(p_db_if);
    
    return PM_E_NONE;
}

int32
lacp_sm_mux_process_event(tbl_interface_t *p_db_if, lacp_mux_event_t event)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    lacp_mux_event_t next_state = 0;

    if (event >= MUX_EVENT_MAX)
    {
        return PM_E_FAIL;
    }
    
    next_state =  lacp_sm_mux_state_table[event][p_db_lacp->mux_state];
    LACP_DEBUG(LACP_PROTO, "[%s] Interface %s recv event: %s, state: %s -> %s", 
            "MUX SM",
            p_db_if->key.name,
            cdb_enum_val2str(lacp_mux_event_strs, MUX_EVENT_MAX, event),
            cdb_enum_val2str(lacp_mux_state_strs, MUX_STATE_MAX, p_db_lacp->mux_state),
            cdb_enum_val2str(lacp_mux_state_strs, MUX_STATE_MAX, next_state));
    
    switch (next_state)
    {
    case MUX_DETACHED:
        _lacp_sm_mux_state_detached(p_db_if, p_db_lacp);
        break;
    case MUX_WAITING:
        _lacp_sm_mux_state_waiting(p_db_if, p_db_lacp);
        break;
    case MUX_ATTACHED:
        _lacp_sm_mux_state_attached(p_db_if, p_db_lacp);
        break;
    case MUX_COLLECTING_DISTRIBUTING:
        _lacp_sm_mux_state_collecting_distributing(p_db_if, p_db_lacp);
        break;
    default:
        break;
    }
    
    return PM_E_NONE;
}

int32
lacp_sm_mux_cfg_update_state(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;

    if ((MUX_DETACHED == p_db_lacp->mux_state && SELECTED_UNSELECTED == p_db_lacp->selected)
      || SELECTED_STANDBY == p_db_lacp->selected)
    {
        _lacp_sm_selection_logic(p_db_if, p_db_lacp);
        if (p_db_lacp->ntt)
        {
            lacp_sm_tx_lacpdu(p_db_if);
        }
    }

    return PM_E_NONE;
}

#define _43_4_16_TRANSMIT_MACHINE_

int32
lacp_sm_tx_lacpdu(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    int32 rc = PM_E_NONE;

    if (!p_db_lacp->ntt)
    {
        return PM_E_NONE;
    }

    /* When the Periodic machine is in the NO_PERIODIC state, the Transmit machine shall
       Not transmit any LACPDUs, and Set the value of NTT to FALSE. */
    if (p_db_lacp->periodic_tx_state == PERTX_NO_PERIODIC) 
    {
        p_db_lacp->ntt = FALSE;
        return PM_E_NONE;
    }

    /* When the LACP_Enabled variable is FALSE, the Transmit machine shall 
       not transmit any LACPDUs and shall set the value of NTT to FALSE. */
    if (!p_db_lacp->lacp_enabled)
    {
        p_db_lacp->ntt = FALSE;
        return PM_E_NONE;
    }

    rc = lacp_pdu_tx_lacp(p_db_if);
    if (rc < 0)
    {
        p_db_lacp->tx_error_count++;
    }
    else
    {
        p_db_lacp->tx_lacp_count++;
    }
    p_db_lacp->ntt = FALSE;
  
    return PM_E_NONE;
}

