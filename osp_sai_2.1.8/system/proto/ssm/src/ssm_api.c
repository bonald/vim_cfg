#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "glb_synce_define.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"


int32
ssm_set_synce_global_enable(uint32 synce_enable)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    tbl_ssm_global_t ssm_glb;
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.synce_enable = synce_enable;
    
    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE));

    PM_E_RETURN(ssm_opts_set_synce_global_enable(synce_enable));
    PM_E_RETURN(ssm_opts_recheck_device());

    /*run synchronous select porcess under non-synce mode*/
    if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
    }
    
    return rc;
}

int32
ssm_set_synce_local_priority(uint32 local_priority)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    tbl_ssm_global_t ssm_glb;
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.local_priority = local_priority;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY));

    /*run synchronous select porcess under non-synce mode*/
    if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
    }
    
    return rc;
}

int32
ssm_set_synce_system_quality(uint32 option_mode, uint32 system_quality)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    tbl_ssm_global_t ssm_glb;
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.option_mode = option_mode;
    ssm_glb.system_quality = system_quality;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_OPTION_MODE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY));

    /*process*/
    if (SSM_DEVICE_MAST == p_db_ssm_glb->device_role)
    {
        if (p_db_ssm_glb->synce_enable && p_db_ssm_glb->ssm_enable)
        {
            if (p_db_ssm_glb->current_quality != p_db_ssm_glb->system_quality)
            {
                p_db_ssm_glb->current_quality = p_db_ssm_glb->system_quality;
                PM_E_RETURN(ssm_pdu_event_send_trigger());
            }
        }
        else
        {
            if (p_db_ssm_glb->current_quality != SSM_QL_DNU)
            {
                p_db_ssm_glb->current_quality = SSM_QL_DNU;
                PM_E_RETURN(ssm_pdu_event_send_trigger());
            }
        }
    }

    /*added by yejl for bug 46472, 2018-12-07*/
    if (p_db_ssm_glb->holdover_en)
    {
        if (p_db_ssm_glb->synce_enable && p_db_ssm_glb->ssm_enable)
        {
            p_db_ssm_glb->current_quality = (p_db_ssm_glb->option_mode == SSM_OPTION_MODE_1) ? SSM_QL_EEC1 : SSM_QL_EEC2;
        }
        PM_E_RETURN(ssm_pdu_event_send_trigger());
    }
    /*end*/
    
    return rc;
}

int32
ssm_set_synce_device_role(uint32 device_role)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    
    tbl_ssm_global_t ssm_glb;    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.device_role = device_role;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DEVICE_ROLE));

    /*process*/
    if (SSM_DEVICE_MAST == device_role)
    {
        PM_E_RETURN(ssm_opts_set_current_quality_master_only());
        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (p_db_ssm_if && p_db_ssm_if->synce_enable)
            {
                p_db_ssm_if->receive_pdu_enable = FALSE;
                p_db_ssm_if->hop = SSM_DEFAULT_PATH_HOP;
                p_db_ssm_if->receive_quality = SSM_QL_INV3;

                if (NULL == p_db_ssm_if->ssm_send_timer)
                {
                    p_db_ssm_if->send_quality = SSM_QL_INV3;  /*Invalid Flag to show N/A*/
                }
                else
                {
                    p_db_ssm_if->send_quality = p_db_ssm_glb->system_quality;
                }
                CTC_TASK_STOP_TIMER(p_db_ssm_if->ssm_timeout_timer);
                sal_memset(p_db_ssm_if->receive_clock_id, 0x00, SSM_CLOCK_ID_LEN);
            }  
        }
    }
    else
    {
        /*set holdover state*/
        if (!p_db_ssm_glb->freerun_en)
        {
            SSM_EVENT_DEBUG("ssm_set_synce_device_role: set holdover");
        }
        else
        {
            SSM_EVENT_DEBUG("ssm_set_synce_device_role: keep freerun");
        }
        PM_E_RETURN(ssm_opts_set_holdover_state(TRUE));
        p_db_ssm_glb->current_quality = SSM_QL_DNU;

        CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
        {
            if (p_db_ssm_if && p_db_ssm_if->synce_enable)
            {
                p_db_ssm_if->receive_pdu_enable = FALSE;
                p_db_ssm_if->hop = SSM_DEFAULT_PATH_HOP;
                p_db_ssm_if->receive_quality = SSM_QL_INV3;
                p_db_ssm_if->send_quality = SSM_QL_INV3;  /*Invalid Flag to show N/A*/
                sal_memset(p_db_ssm_if->receive_clock_id, 0x00, SSM_CLOCK_ID_LEN);
                
                if (p_db_ssm_glb->synce_enable && p_db_ssm_glb->ssm_enable && p_db_ssm_if->synce_enable && ssm_port_is_up(p_db_ssm_if) &&
                    (SSM_QL_INV3 != p_db_ssm_if->force_rx_quality) && ((0 == p_db_ssm_glb->select_ifindex) || p_db_ssm_glb->holdover_en))
                {
                    PM_E_RETURN(ssm_fsm_select_clock());
                }
            }  
        }
        /*run synchronous select porcess under non-synce mode*/
        if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
        {
            PM_E_RETURN(ssm_fsm_select_non_ssm(device_role));
        }
    }
    
    return rc;
}

int32
ssm_set_synce_ssm_enable(uint32 ssm_enable)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    tbl_ssm_global_t ssm_glb;
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.ssm_enable = ssm_enable;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SSM_ENABLE));

    PM_E_RETURN(ssm_opts_set_synce_ssm_enable(ssm_enable));
    PM_E_RETURN(ssm_opts_recheck_device());

    /*run synchronous select porcess under non-synce mode*/
    if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
    }
    return rc;
} 

int32
ssm_set_synce_ssm_extend_enable(uint32 ssm_extend_enable)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.ssm_extend_enable = ssm_extend_enable;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE));
    return rc;
}   

int32
ssm_set_synce_holdover_enable(uint32 holdover_en)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.holdover_en = holdover_en;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_HOLDOVER_EN));
    return rc;
}

int32
ssm_set_synce_selected_ifindex(uint32 ifindex)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.select_ifindex = ifindex;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX));
    return rc;
}

int32
ssm_set_synce_holdoff_time(uint32 holdoff_time)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.holdoff_time = holdoff_time;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME));

    if (p_db_ssm_glb->holdoff_send_timer)
    {
        ctc_task_delete_timer(p_db_ssm_glb->holdoff_send_timer);
        p_db_ssm_glb->holdoff_send_timer = NULL;
        
        PM_E_RETURN(ssm_opts_recheck_device());
        if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
        {
            PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
        }
    }
    return rc;
}

int32
ssm_set_synce_wait_to_restore_time(uint32 wtr_time)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    ssm_glb.wtr_time = wtr_time;

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_WTR_TIME));

    if (p_db_ssm_glb->wtr_send_timer)
    {
        ctc_task_delete_timer(p_db_ssm_glb->wtr_send_timer);
        p_db_ssm_glb->wtr_send_timer = NULL;
        
        PM_E_RETURN(ssm_opts_recheck_device());
        if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
        {
            PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
        }
    }
    return rc;
}

int32
ssm_set_synce_clock_id(ssm_clock_id_t clock_id)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_global_t ssm_glb;
    
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));
    sal_memcpy(ssm_glb.device_clock_id, clock_id, SSM_CLOCK_ID_LEN);

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DEVICE_CLOCK_ID));

    if (p_db_ssm_glb->ssm_extend_enable)
    {
        PM_E_RETURN(ssm_opts_set_synce_clock_id());    
    }
    
    return rc;
}
    

int32
ssm_set_if_synce_enable(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t *p_db_ssm_port = NULL;
    
    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_SYNCE_ENABLE));

    PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, p_ssm_port->synce_enable));
    PM_E_RETURN(ssm_opts_recheck_device());
    
    /*run synchronous select porcess under non-synce mode*/
    if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
    }
    return rc;
}

int32
ssm_set_if_ssm_pdu_direction(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_SSM_PDU_DIRECT));

    /*process*/
    PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, TRUE));
    PM_E_RETURN(ssm_opts_recheck_device());
    
    return rc;
}

int32
ssm_set_if_priority(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t *p_db_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_PRIORITY));

    /*get the select port priority change or not*/
    if (p_db_ssm_glb->select_ifindex)
    {
        if (p_db_ssm_glb->select_ifindex == p_ssm_port->ifindex)
        {
            p_db_ssm_glb->select_if_priority_change_en = TRUE;
        }
    }
    
    /*run synchronous select porcess under non-synce mode*/
    if (p_db_ssm_glb->synce_enable && !p_db_ssm_glb->ssm_enable)
    {
        PM_E_RETURN(ssm_fsm_select_non_ssm(p_db_ssm_glb->device_role));
    }
    return rc;
}

int32
ssm_set_if_replace_clockid_enable(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN));
    return rc;
}

int32
ssm_set_if_dnu_group_id(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_DNU_GROUP_ID));
    return rc;
}

int32
ssm_set_if_tx_quality_value(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_FORCE_TX_QUALITY));

    /*process*/
    PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, TRUE));
    PM_E_RETURN(ssm_opts_recheck_device());
    
    return rc;
}

int32
ssm_set_if_rx_quality_value(tbl_ssm_port_t* p_ssm_port)
{
    int32 rc = PM_E_NONE;
    tbl_ssm_port_t *p_db_ssm_port = NULL;

    p_db_ssm_port = tbl_ssm_port_get_ssm_port(&p_ssm_port->key);
    if (NULL == p_db_ssm_port)
    {
        return PM_E_NOT_EXIST;
    }

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_port_set_ssm_port_field(p_ssm_port, TBL_SSM_PORT_FLD_FORCE_RX_QUALITY));

    /*process*/
    PM_E_RETURN(ssm_opts_refresh_property(p_db_ssm_port, TRUE));
    PM_E_RETURN(ssm_opts_recheck_device());
    
    return rc;
}

int32
ssm_set_dull_state_process(tbl_ssm_global_t* p_ssm_glb)
{
    int32 rc = PM_E_NONE;
    if (GLB_SYNCE_STATE_FREERUN != p_ssm_glb->dpll_current_status || 
        GLB_SYNCE_STATE_HOLDOVER != p_ssm_glb->dpll_current_status ||
        GLB_SYNCE_STATE_LOCKED != p_ssm_glb->dpll_current_status ||
        GLB_SYNCE_STATE_LOSSOFLOCK != p_ssm_glb->dpll_current_status)
    {
        return PM_E_NONE;
    }
    PM_E_RETURN(ssm_opts_receive_clock_status(p_ssm_glb));
    return PM_E_NONE;
}

int32
ssm_set_field_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_ssm_global_t* p_ssm_glb = NULL;
    int32 rc = PM_E_NONE;

    if(p_ds_node)
    {
        return PM_E_NONE;
    }
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_ssm_glb = (tbl_ssm_global_t*)p_tbl;
        if(NULL == p_ssm_glb)
        {
            return PM_E_INVALID_PARAM;
        }      
        
        switch(field_id)
        {
            case TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS:
                PM_E_RETURN(ssm_set_dull_state_process(p_ssm_glb));
                break;
        }
        break;

    default:
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

int32
ssm_global_init()
{
    int32 rc = PM_E_NONE;
    tbl_ssm_global_t ssm_glb;
    sal_memset(&ssm_glb, 0, sizeof(tbl_ssm_global_t));

    /*set the ssm global field to default value*/
    ssm_glb.device_role = SSM_DEVICE_STAND;
    ssm_glb.ssm_extend_enable = FALSE;
    ssm_glb.holdover_en = FALSE;
    ssm_glb.freerun_en = TRUE;
    ssm_glb.local_priority = SSM_CLOCK_PRIORITY_DEFAUTL;
    ssm_glb.select_ifindex = 0;
    ssm_glb.ssm_enable = FALSE;
    ssm_glb.synce_enable = FALSE;
    ssm_glb.option_mode = SSM_OPTION_MODE_1;
    ssm_glb.system_quality = SSM_QL_DNU;
    ssm_glb.current_quality = SSM_QL_DNU;
    ssm_glb.dpll_current_status = SSM_CLOCK_FREERUN;
    ssm_glb.dpll_selected_ifindex = SSM_DPLL_INVALID_IFINDEX;
    ssm_glb.clock_recovery_select = CLOCK_RECOVERY_SELECT_SYNCE;
    ssm_glb.force_quality_option1_cnt = 0;
    ssm_glb.force_quality_option2_cnt = 0;

    /*Start for holdoff time and wait to restore time*/
    ssm_glb.next_selected_ifindex = 0;
    ssm_glb.select_if_linkup_restore_en = FALSE;
    ssm_glb.select_if_priority_change_en = FALSE;
    ssm_glb.select_if_quality_change_en = FALSE;
    ssm_glb.holdoff_time = SYNCE_HOLDOFF_DEFAULT_TIME;
    ssm_glb.wtr_time = SYNCE_WTR_DEFAULT_TIME;
    ssm_glb.holdoff_send_timer = NULL;
    ssm_glb.wtr_send_timer = NULL;
    /*Ended for holdoff time and wait to restore time*/

    /*Add to DB*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SYNCE_ENABLE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_LOCAL_PRIORITY));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_OPTION_MODE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SYSTEM_QUALITY));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_CURRENT_QUALITY));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DEVICE_ROLE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SSM_ENABLE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SSM_EXTEND_ENABLE));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_HOLDOVER_EN));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_FREERUN_EN));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SELECT_IFINDEX));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DPLL_SELECTED_IFINDEX));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_DPLL_CURRENT_STATUS));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION1_CNT));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_FORCE_QUALITY_OPTION2_CNT));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_CLOCK_RECOVERY_SELECT));

    /*Start for holdoff time and wait to restore time*/
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_NEXT_SELECTED_IFINDEX));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SELECT_IF_LINKUP_RESTORE_EN));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SELECT_IF_PRIORITY_CHANGE_EN));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_SELECT_IF_QUALITY_CHANGE_EN));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_HOLDOFF_TIME));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_WTR_TIME));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_HOLDOFF_SEND_TIMER));
    PM_E_RETURN(tbl_ssm_global_set_ssm_global_field(&ssm_glb, TBL_SSM_GLOBAL_FLD_WTR_SEND_TIMER));
    /*Ended for holdoff time and wait to restore time*/
    
    return rc;
}

