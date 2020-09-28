#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"


static uint32
ssm_opts_decode_dpll_ic_port(uint32 ic_port_index)
{
    if (SSM_DPLL_IC1 == ic_port_index)
    {
        return SSM_DPLL_IC1_TO_IFINDEX;                      
    }
    else if (SSM_DPLL_IC2 == ic_port_index)
    {
        return SSM_DPLL_IC2_TO_IFINDEX;                      
    }
    else if (SSM_DPLL_IC3 == ic_port_index)
    {
        return SSM_DPLL_IC3_TO_IFINDEX;                      
    }
    else if (SSM_DPLL_IC4 == ic_port_index)
    {
        return SSM_DPLL_IC4_TO_IFINDEX;                      
    }
    else if (SSM_DPLL_IC8 == ic_port_index)
    {
        return SSM_DPLL_IC8_TO_IFINDEX;                      
    }
    else
    {
        return SSM_DPLL_INVALID_IFINDEX;
    }
}

uint32
ssm_port_is_up(tbl_ssm_port_t* p_db_ssm_if)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    uint32 is_up = FALSE;
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, p_db_ssm_if->key.name, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (p_db_if)
    {
        is_up = tbl_interface_is_running(p_db_if);
    }
    return is_up;
}

int32
ssm_opts_set_holdover_enable(uint32 holdover_en)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    
    if ((p_ssm_glb->holdover_en != holdover_en) && (!p_ssm_glb->freerun_en))
    {
        PM_E_RETURN(ssm_set_synce_holdover_enable(holdover_en));
        if (holdover_en)
        {
            logid_sys(LOG_SSM_4_STATE_CHANGE, "Lock", "Holdover");
        }
        else
        {
            logid_sys(LOG_SSM_4_STATE_CHANGE, "Holdover", "Lock");
        }
    }
    else
    {
        return PM_E_NONE;
    }

    if (p_ssm_glb->synce_enable && p_ssm_glb->ssm_enable)
    {   
        /*send event pdu*/
        PM_E_RETURN(ssm_pdu_event_send_trigger());
    }
    
    return PM_E_NONE;
}

int32
ssm_opts_selected_ifindex(tbl_ssm_port_t* p_ssm_if_best)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    char ifname_ext[IFNAME_SIZE];
    uint32 freerun_count = 0;
    int32 rc = PM_E_NONE;
    
    if (p_ssm_glb->ssm_enable)
    {
        if (p_ssm_glb->current_quality != _ssm_fsm_get_compared_port_quality(p_ssm_if_best))
        {
            /*modified by yejl for bug 46448, 2012-12-13*/
            p_ssm_glb->current_quality = _ssm_fsm_get_compared_port_quality(p_ssm_if_best);
        }
    }
    /*Added by yejl for bug 46359, 2018-02-05*/
    else
    {
        p_ssm_glb->current_quality = SSM_QL_DNU;
    }
    /*End*/

    /*For preventing send repeated configuration*/
    if (p_ssm_glb->select_ifindex && 
       (p_ssm_glb->select_ifindex == p_ssm_if_best->ifindex) &&
       (p_ssm_glb->select_if_linkup_restore_en == FALSE))
    {
        return PM_E_NONE;
    }
    else
    {
        /*When select a port, should go out freerun*/
        if (p_ssm_glb->freerun_en)
        {
            p_ssm_glb->freerun_en = FALSE;
            freerun_count ++;
        }

        /*modified by yejl for bug 46448, 2012-12-13*/
        if (SSM_E1_IFINDEX != p_ssm_if_best->ifindex)
        {
            PM_E_RETURN(ssm_set_synce_selected_ifindex(p_ssm_if_best->ifindex));
        }
        PM_E_RETURN(ssm_opts_set_holdover_enable(FALSE)); 
    }
    p_ssm_glb->select_ifindex = p_ssm_if_best->ifindex;
    /*add the log when new port is select as the best clock resource*/
    logid_sys(LOG_SSM_5_PORT_SELECT, IFNAME_ETH2FULL(p_ssm_if_best->key.name, ifname_ext));  
    if (freerun_count && !p_ssm_glb->freerun_en && !p_ssm_glb->holdover_en)
    {
        logid_sys(LOG_SSM_4_STATE_CHANGE, "FreeRun", "Lock");
    } 
    
    return PM_E_NONE;
}

int32
ssm_opts_set_holdover_state(uint32 no_holdoff)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    
    if (!p_ssm_glb->freerun_en)
    {
        if (p_ssm_glb->synce_enable && p_ssm_glb->ssm_enable)
        {
            p_ssm_glb->current_quality = (p_ssm_glb->option_mode == SSM_OPTION_MODE_1) ? SSM_QL_EEC1 : SSM_QL_EEC2;
        }
        else
        {
            p_ssm_glb->current_quality = SSM_QL_DNU;
        }
        if (no_holdoff)
        {
            p_ssm_glb->select_ifindex = 0;
            p_ssm_glb->next_selected_ifindex = 0;
        }
        PM_E_RETURN(ssm_opts_set_holdover_enable(TRUE));
    }
    
    return PM_E_NONE;
}

int32
ssm_opts_set_current_quality_master_only(void)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;
    if ((SSM_DEVICE_MAST == p_ssm_glb->device_role) && p_ssm_glb->synce_enable)
    {
        if (p_ssm_glb->ssm_enable && p_ssm_glb->synce_enable)
        {
            p_ssm_glb->current_quality = p_ssm_glb->system_quality;
        }
        else
        {
            p_ssm_glb->current_quality = SSM_QL_DNU;
        }
        
        PM_E_RETURN(ssm_opts_set_holdover_enable(TRUE));
        p_ssm_glb->select_ifindex = 0;
        p_ssm_glb->next_selected_ifindex = 0;
    }
    return PM_E_NONE;
}

int32
ssm_opts_check_all_receive_timer()
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    uint32 timer_exist = FALSE;
    int32 rc = PM_E_NONE;
    
    /*Modified by yejl for bug 46354, 2018-02-05*/
    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && (NULL != p_db_ssm_if->ssm_timeout_timer))
        {
            timer_exist = TRUE;
        }  
    }
    if (timer_exist)
    {
        return PM_E_NONE;
    }
    else
    {
        /*process*/
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (p_db_ssm_if && p_db_ssm_if->synce_enable && ssm_port_is_up(p_db_ssm_if) && 
                p_ssm_glb->synce_enable && p_ssm_glb->ssm_enable)
            {
                if (SSM_QL_INV3 != p_db_ssm_if->force_rx_quality && SSM_QL_DNU != p_db_ssm_if->force_rx_quality)
                {
                    PM_E_RETURN(ssm_fsm_select_clock());
                    return PM_E_NONE;
                }
            }   
        }
    }
    /*End*/
    
    /*set holdover state*/
    if (!p_ssm_glb->freerun_en)
    {
        SSM_EVENT_DEBUG("ssm_opts_check_all_receive_timer: set holdover");
    }
    else
    {
        SSM_EVENT_DEBUG("ssm_opts_check_all_receive_timer: keep freerun");
    }
    PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
    
    return PM_E_NONE;
}

int32
ssm_opts_refresh_property(tbl_ssm_port_t* p_db_ssm_if, uint32 enable)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    uint32 port_is_up = FALSE;
    port_is_up = ssm_port_is_up(p_db_ssm_if);
    
    if (enable)
    {
        if (p_db_ssm_if->synce_enable && port_is_up && 
            p_ssm_glb->synce_enable && p_ssm_glb->ssm_enable)
        {
            if (SSM_PDU_RX == p_db_ssm_if->ssm_pdu_direct)
            {
                CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_send_timer);
                p_db_ssm_if->send_quality = SSM_QL_INV3;
            }
            else if (SSM_PDU_NONE == p_db_ssm_if->ssm_pdu_direct)
            {
                CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_timeout_timer);
                CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_send_timer);    
                p_db_ssm_if->receive_pdu_enable = FALSE;
                p_db_ssm_if->send_quality = SSM_QL_INV3;
                p_db_ssm_if->receive_quality = SSM_QL_INV3;
            }
            else
            {
                p_db_ssm_if->ssm_send_timer = p_db_ssm_if->ssm_send_timer ? p_db_ssm_if->ssm_send_timer
                : ctc_task_add_timer_msec(TASK_PRI_NORMAL, ssm_pdu_information_send_trigger, p_db_ssm_if, SSM_INTERN_INTERVAL_SEND);
                
                if (SSM_DEVICE_MAST == p_ssm_glb->device_role)
                {
                    p_ssm_glb->current_quality = p_ssm_glb->system_quality;
                }
            }
        }
    }
    else
    {
        CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_timeout_timer);
        CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_send_timer);
        
        p_db_ssm_if->receive_pdu_enable = FALSE;
        p_db_ssm_if->send_quality = SSM_QL_INV3;

        /*get the select port quality change or not*/
        if (p_ssm_glb->select_ifindex)
        {
            if ((p_db_ssm_if->ifindex == p_ssm_glb->select_ifindex) && 
                (p_db_ssm_if->receive_quality != SSM_QL_INV3))
            {
                p_ssm_glb->select_if_quality_change_en = TRUE;
            }
        }
        p_db_ssm_if->receive_quality = SSM_QL_INV3;
    }
    return PM_E_NONE;
}

int32
ssm_opts_recheck_device(void)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    int32 rc = PM_E_NONE;

    if (SSM_DEVICE_MAST == p_ssm_glb->device_role)
    {
        PM_E_RETURN(ssm_opts_set_current_quality_master_only());
        return PM_E_NONE;
    }
    
    /*select best quality on synce enable and ssm enable mode*/
    if (p_ssm_glb->synce_enable)
    {
        if (p_ssm_glb->ssm_enable)
        {
            PM_E_RETURN(ssm_opts_check_all_receive_timer());
            PM_E_RETURN(ssm_fsm_select_clock());
        }
    }
    else
    {
        if (!p_ssm_glb->freerun_en)
        {
            SSM_EVENT_DEBUG("ssm_opts_recheck_device: set holdover");
        }
        else
        {
            SSM_EVENT_DEBUG("ssm_opts_recheck_device: keep freerun");
        }
        PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
    }
    return PM_E_NONE;
}

int32
ssm_opts_set_synce_global_enable(uint32 synce_enable)
{    
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && p_db_ssm_if->synce_enable)
        {
            PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_if, synce_enable));
        }  
    }
    return PM_E_NONE;
}

int32
ssm_opts_set_synce_ssm_enable(uint32 ssm_enable)
{
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && p_db_ssm_if->synce_enable)
        {
            PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_if, ssm_enable));
        }  
    }
    return PM_E_NONE;
}

int32
ssm_opts_set_synce_clock_id()
{
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && p_db_ssm_if->synce_enable)
        {
            PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_if, TRUE));
        }  
    }
    return PM_E_NONE;
}

void
ssm_opts_timeout_trigger(void *p_data)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t * p_db_ssm_if = NULL;
    
    p_db_ssm_if = (tbl_ssm_port_t*)p_data;
    if (!p_db_ssm_if)
    {
        SSM_LOG_ERR("Timeout trigger fail because tbl_ssm_port is NULL");
        return;
    }
    if (!ssm_port_is_up(p_db_ssm_if))
    {
        SSM_LOG_ERR("Timeout trigger fail because port is not up");
        return;
    }

    p_db_ssm_if->ssm_timeout_timer = NULL;

    /*check all recieve timer*/
    ssm_opts_check_all_receive_timer();
    
    p_db_ssm_if->receive_pdu_enable = FALSE;

    /*get the select port quality change or not*/
    if (p_ssm_glb->select_ifindex)
    {
        if ((p_ssm_glb->select_ifindex == p_db_ssm_if->ifindex) && 
            (p_db_ssm_if->receive_quality != SSM_QL_INV3))
        {
            p_ssm_glb->select_if_quality_change_en = TRUE;
        }
    }
    p_db_ssm_if->receive_quality = SSM_QL_INV3;
    
}

void
ssm_opts_wtr_select_sync_if_trigger(void *p_data)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t * p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;
    p_db_ssm_if = (tbl_ssm_port_t*)p_data;
    if (!p_db_ssm_if)
    {
        SSM_LOG_ERR("WTR select fail because tbl_ssm_port is NULL");
        return;
    }
    if (!ssm_port_is_up(p_db_ssm_if))
    {
        SSM_LOG_ERR("WTR select fail because port is not up");
        return;
    }
    p_ssm_glb->wtr_send_timer = NULL;

    /*select port*/
    rc = ssm_opts_selected_ifindex(p_db_ssm_if);
    if (rc < 0)
    {   
        SSM_LOG_ERR("WTR select ifindex fail!");
        return;
    }
    p_ssm_glb->select_if_priority_change_en = FALSE;
    p_ssm_glb->select_if_quality_change_en = FALSE;
    return;
}

void
ssm_opts_holdoff_select_sync_if_trigger(void *p_data)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t * p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_ssm_if = (tbl_ssm_port_t*)p_data;
    if (!p_db_ssm_if)
    {
        SSM_LOG_ERR("HOLDOFF select fail because tbl_ssm_port is NULL");
        return;
    }
    if (!ssm_port_is_up(p_db_ssm_if))
    {
        SSM_LOG_ERR("HOLDOFF select fail because port is not up");
        return;
    }
    p_ssm_glb->holdoff_send_timer = NULL;

    rc = ssm_opts_selected_ifindex(p_db_ssm_if);
    if (rc < 0)
    {
        SSM_LOG_ERR("HOLDOFF select ifindex fail!");
        return;
    }
    p_ssm_glb->select_if_priority_change_en = FALSE;
    p_ssm_glb->select_if_quality_change_en = FALSE;
    return;
}


int32
ssm_opts_refresh_send_timer(tbl_ssm_port_t* p_db_ssm_if)
{
    /*close timer*/
    CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_send_timer);

    /*open timer*/
    p_db_ssm_if->ssm_send_timer = p_db_ssm_if->ssm_send_timer ? p_db_ssm_if->ssm_send_timer
    : ctc_task_add_timer_msec(TASK_PRI_NORMAL, ssm_pdu_information_send_trigger, p_db_ssm_if, SSM_INTERN_INTERVAL_SEND);

    return PM_E_NONE;
}

int32
ssm_opts_refresh_timeout_timer(tbl_ssm_port_t* p_db_ssm_if)
{
    /*close timer*/
    CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_timeout_timer);
    
    /*open timer*/
    p_db_ssm_if->ssm_timeout_timer = p_db_ssm_if->ssm_timeout_timer ? p_db_ssm_if->ssm_timeout_timer
    : ctc_task_add_timer_msec(TASK_PRI_NORMAL, ssm_opts_timeout_trigger, p_db_ssm_if, SSM_INTERN_INTERVAL_RECV);
        
    return PM_E_NONE;
}

int32
ssm_opts_receive_clock_status(tbl_ssm_global_t* p_ssm_glb)
{
    uint32 ifindex = 0;
    ifindex = ssm_opts_decode_dpll_ic_port(p_ssm_glb->dpll_selected_ifindex);
    p_ssm_glb->dpll_selected_ifindex = ifindex;
    SSM_EVENT_DEBUG("ssm_opts_receive_clock_status: ", p_ssm_glb->dpll_current_status);
    return PM_E_NONE;
}

