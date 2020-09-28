#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "glb_g8131_define.h"
#include "glb_tp_oam.h"
#include "cdb_ds.h"
#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "g8131_api.h"
#include "g8131_fsm.h"
#include "g8131_pdu.h"
#include "fei.h"
#include "fei_g8131.h"

void
g8131_debug_aps_map_str_to_state(char *str, uint32 *state, uint32 *working_active)
{
    if (sal_strncmp (str, "nr-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_NR_W;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "nr-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_NR_P;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "lo", 2) == 0)
    {
        *state = G8131_FSM_STATE_LO;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "fs", 2) == 0)
    {
        *state = G8131_FSM_STATE_FS;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "sf-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_SF_W;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "sf-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_SF_P;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "sd-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_SD_W;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "sd-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_SD_P;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "ms-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_MS_P;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "ms-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_MS_W;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "wtr", 3) == 0)
    {
        *state = G8131_FSM_STATE_WTR;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "dnr", 3) == 0)
    {
        *state = G8131_FSM_STATE_DNR;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "ex-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_EXER_W;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "ex-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_EXER_P;
        *working_active = FALSE;
    }
    else if (sal_strncmp (str, "rr-w", 4) == 0)
    {
        *state = G8131_FSM_STATE_RR_W;
        *working_active = TRUE;
    }
    else if (sal_strncmp (str, "rr-p", 4) == 0)
    {
        *state = G8131_FSM_STATE_RR_P;
        *working_active = FALSE;
    }
    else
    {
        *state = G8131_FSM_STATE_MAX;
    }

    return;
}

void 
g8131_debug_aps_map_str_to_event(char *str, uint32 *event)
{
    if (sal_strncmp (str, "lo", 2) == 0)
    {
        *event = G8131_FSM_L_EVENT_LO;
    }
    else if (sal_strncmp (str, "fs", 2) == 0)
    {
        *event = G8131_FSM_L_EVENT_FS;
    }
    else if (sal_strncmp (str, "sf-w", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_SF_W;
    }
    else if (sal_strncmp (str, "wrsf", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF;
    }
    else if (sal_strncmp (str, "sf-p", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_SF_P;
    }
    else if (sal_strncmp (str, "prsf", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF;
    }
    else if (sal_strncmp (str, "sd-w", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_SD_W;
    }
    else if (sal_strncmp (str, "wrsd", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_W_RCOVERS_FROM_SD;
    }
    else if (sal_strncmp (str, "sd-p", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_SD_P;
    }
    else if (sal_strncmp (str, "prsd", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_P_RCOVERS_FROM_SD;
    }
    else if (sal_strncmp (str, "ms-p", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_MS_P;
    }
    else if (sal_strncmp (str, "ms-w", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_MS_W;
    }
    else if (sal_strncmp (str, "clear", 5) == 0)
    {
        *event = G8131_FSM_L_EVENT_CLEAR;
    }
    else if (sal_strncmp (str, "exer", 4) == 0)
    {
        *event = G8131_FSM_L_EVENT_EXER;
    }
    else if (sal_strncmp (str, "wtr", 3) == 0)
    {
        *event = G8131_FSM_L_EVENT_WTR;
    }
    else
    {
        *event = G8131_FSM_EVENT_MAX;
    }

    return;
}

void 
g8131_debug_aps_map_str_to_far_event(char *str, uint32 *event)
{
    if (sal_strncmp (str, "lo", 2) == 0)
    {
        *event = G8131_FSM_R_EVENT_LO;
    }
    else if (sal_strncmp (str, "sf-p", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_SF_P;
    }
    else if (sal_strncmp (str, "fs", 2) == 0)
    {
        *event = G8131_FSM_R_EVENT_FS;
    }
    else if (sal_strncmp (str, "sf-w", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_SF_W;
    }
    else if (sal_strncmp (str, "sd-w", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_SD_W;
    }
    else if (sal_strncmp (str, "sd-p", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_SD_P;
    }
    else if (sal_strncmp (str, "ms-p", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_MS_P;
    }
    else if (sal_strncmp (str, "ms-w", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_MS_W;
    }
    else if (sal_strncmp (str, "wtr", 3) == 0)
    {
        *event = G8131_FSM_R_EVENT_WTR;
    }
    else if (sal_strncmp (str, "ex-w", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_EXER_W;
    }
    else if (sal_strncmp (str, "ex-p", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_EXER_P;
    }
    else if (sal_strncmp (str, "rr-w", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_RR_W;
    }
    else if (sal_strncmp (str, "rr-p", 4) == 0)
    {
        *event = G8131_FSM_R_EVENT_RR_P;
    }
    else if (sal_strncmp (str, "nr-null", 5) == 0)
    {
        *event = G8131_FSM_R_EVENT_NR_NULL;
    }
    else if (sal_strncmp (str, "nr-normal", 5) == 0)
    {
        *event = G8131_FSM_R_EVENT_NR_NORMAL;
    }
    else if (sal_strncmp (str, "dnr", 3) == 0)
    {
        *event = G8131_FSM_R_EVENT_DNR;
    }
    else
    {
        *event = G8131_FSM_EVENT_MAX;
    }

    return;
}


void
g8131_aps_map_str_to_event(char *str, uint32 *current_event)
{
    if (0 == sal_strcmp(str, "clear")) 
    {
        *current_event = G8131_FSM_L_EVENT_CLEAR;
    }
    else if (0 == sal_strcmp(str, "exercise"))
    {
        *current_event = G8131_FSM_L_EVENT_EXER;
    }
    else if (0 == sal_strcmp(str, "force")) 
    {
        *current_event = G8131_FSM_L_EVENT_FS;
    }
    else if (0 == sal_strcmp(str, "lockout"))
    {
        *current_event = G8131_FSM_L_EVENT_LO;
    }
    else if (0 == sal_strcmp(str, "manual-switch-to-protection")) 
    {
        *current_event = G8131_FSM_L_EVENT_MS_P;
    }
    else if (0 == sal_strcmp(str, "manual-switch-to-working"))
    {
        *current_event = G8131_FSM_L_EVENT_MS_W;
    }
    return;
}

static int32
g8131_pw_aps_get_lsp_aps(tbl_aps_group_info_t * p_db_ag_info, uint32* lsp_aps_group1, uint32* lsp_aps_group2)
{
    tbl_pw_tpe_t* p_db_pw_tpe = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;
    
    if (G8131_APS_GROUP_TYPE_PW != p_db_ag_info->aps_group_type)
    {
        *lsp_aps_group1 = 0;
        *lsp_aps_group2 = 0;
    }

    sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key)); 
    sal_strncpy(pw_tpe_key.name, p_db_ag_info->working_name, LSPNAME_SIZE);
    p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
    if (p_db_pw_tpe)
    {
        *lsp_aps_group1 = p_db_pw_tpe->lsp_aps_group_id;  
    }
    else
    {
        *lsp_aps_group1 = 0;
    }

    sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key)); 
    sal_strncpy(pw_tpe_key.name, p_db_ag_info->protection_name, LSPNAME_SIZE);
    p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
    if (p_db_pw_tpe)
    {
        *lsp_aps_group2 = p_db_pw_tpe->lsp_aps_group_id;
    }
    else
    {
        *lsp_aps_group2 = 0;
    }
    return PM_E_NONE;
}

int32
g8131_lsp_aps_group_before_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_lsp_aps_group_t *p_lsp_aps_group = NULL; 
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    tbl_aps_group_info_t aps_group_info;
    tbl_aps_group_info_key_t aps_group_info_key;

    switch (oper)
    {
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }        
        p_lsp_aps_group = (tbl_lsp_aps_group_t*)p_tbl;
        if (NULL == p_lsp_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
        aps_group_info_key.aps_group_id = p_lsp_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
        if (NULL == p_db_ag_info)
        {   
            sal_memset(&aps_group_info, 0, sizeof(aps_group_info));
            aps_group_info.key.aps_group_id = p_lsp_aps_group->key.group_id;
            aps_group_info.aps_group_type = G8131_APS_GROUP_TYPE_LSP;
            PM_E_RETURN(tbl_aps_group_info_add_aps_group_info(&aps_group_info));
            
            /*init aps group info*/
            PM_E_RETURN(g8131_aps_group_info_initialize(&aps_group_info, G8131_REVERTIVE_SWITCHING, 
                                                          G8131_NO_PERMANENT_BRIDGE, G8131_BIDIRECTIONAL_SWITCHING));
        }
        break;
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }        
        p_lsp_aps_group = (tbl_lsp_aps_group_t*)p_tbl;
        if (NULL == p_lsp_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
        aps_group_info_key.aps_group_id = p_lsp_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
        if (p_db_ag_info)
        {   
            /*modified by yejl for bug 47196, 2018-05-15*/
            CTC_TASK_STOP_TIMER(p_db_ag_info->wtr_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sf_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sfp_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sd_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sdp_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sc_out_aps_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->aps_tx_timer);
            PM_E_RETURN(tbl_aps_group_info_del_aps_group_info(&aps_group_info_key)); 
        }
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        } 
        p_lsp_aps_group = (tbl_lsp_aps_group_t*)p_tbl;
        if (NULL == p_lsp_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info, 0, sizeof(aps_group_info));
        aps_group_info.key.aps_group_id = p_lsp_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&(aps_group_info.key));
        if (NULL == p_db_ag_info)
        {
            return PM_E_NOT_EXIST;
        }
        switch (field_id)
        {
        case TBL_LSP_APS_GROUP_FLD_WORKING_LSP_NAME:
            sal_memcpy(aps_group_info.working_name, p_lsp_aps_group->working_lsp_name, LSPNAME_SIZE);
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_WORKING_NAME));
            break;
            
        case TBL_LSP_APS_GROUP_FLD_PROTECTION_LSP_NAME:
            sal_memcpy(aps_group_info.protection_name, p_lsp_aps_group->protection_lsp_name, LSPNAME_SIZE);
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME));
            break;
            
        case TBL_LSP_APS_GROUP_FLD_IS_REVERTIVE:
            aps_group_info.is_revertive = p_lsp_aps_group->is_revertive;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE));

            /*init aps group info*/
            PM_E_RETURN(g8131_aps_group_info_initialize(&aps_group_info, p_lsp_aps_group->is_revertive, 
                                                        G8131_NO_PERMANENT_BRIDGE, G8131_BIDIRECTIONAL_SWITCHING));
            /*reset aps group info*/
            PM_E_RETURN(g8131_reset_aps_group_info(&aps_group_info));
            break;

        case TBL_LSP_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME:
            aps_group_info.wait_to_restore_time = p_lsp_aps_group->wait_to_restore_time;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME));
            break;

        case TBL_LSP_APS_GROUP_FLD_HOLD_OFF_TIME:
            aps_group_info.hold_off_time = p_lsp_aps_group->hold_off_time;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME));

            /*process hold off time fucntion*/
            if (G8131_ACTIVE_NONE != p_db_ag_info->active_path)
            {
                if ((G8131_FSM_STATE_LO != p_db_ag_info->current_state) &&
                    (G8131_HOLDOFF_TIMER_MIN_VALUE == p_lsp_aps_group->hold_off_time))
                {
                    G8131_EVENT_DEBUG("SET HOLD-OFF-TIME: lsp aps group %u fast aps true", p_db_ag_info->key.aps_group_id);
                    aps_group_info.fast_aps_en = TRUE;
                    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
                    PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, TRUE));
                }
                else
                {
                    G8131_EVENT_DEBUG("SET HOLD-OFF-TIME: lsp aps group %u fast aps false", p_db_ag_info->key.aps_group_id);
                    aps_group_info.fast_aps_en = FALSE;
                    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
                    PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, FALSE));
                }
            }
            break;

        case TBL_LSP_APS_GROUP_FLD_BRIDGE_PERMANENT:
            aps_group_info.bridge_permanent = p_lsp_aps_group->bridge_permanent;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT));
            break;

        case TBL_LSP_APS_GROUP_FLD_FLOW_FLAG:
            if (GLB_FLAG_ISSET(p_lsp_aps_group->flow_flag, G8131_APS_GROUP_FLAG_ADD_BY_FLOW))
            {
                PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, p_db_ag_info->fast_aps_en));    
            }
            
            break;
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

int32
g8131_pw_aps_group_before_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_pw_aps_group_t *p_pw_aps_group = NULL; 
    tbl_aps_group_info_t * p_db_ag_info = NULL;
    tbl_aps_group_info_t aps_group_info;
    tbl_aps_group_info_key_t aps_group_info_key;

    switch (oper)
    {
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }        
        p_pw_aps_group = (tbl_pw_aps_group_t*)p_tbl;
        if (NULL == p_pw_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
        aps_group_info_key.aps_group_id = p_pw_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
        if (NULL == p_db_ag_info)
        {   
            sal_memset(&aps_group_info, 0, sizeof(aps_group_info));
            aps_group_info.key.aps_group_id = p_pw_aps_group->key.group_id;
            aps_group_info.aps_group_type = G8131_APS_GROUP_TYPE_PW;
            PM_E_RETURN(tbl_aps_group_info_add_aps_group_info(&aps_group_info)); 
            
            /*init aps group info*/
            PM_E_RETURN(g8131_aps_group_info_initialize(&aps_group_info, G8131_REVERTIVE_SWITCHING, 
                                                        G8131_NO_PERMANENT_BRIDGE, G8131_BIDIRECTIONAL_SWITCHING));
        }
        break;
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        }        
        p_pw_aps_group = (tbl_pw_aps_group_t*)p_tbl;
        if (NULL == p_pw_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
        aps_group_info_key.aps_group_id = p_pw_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
        if (p_db_ag_info)
        {   
            /*modified by yejl for bug 47196, 2018-05-15*/
            CTC_TASK_STOP_TIMER(p_db_ag_info->wtr_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sf_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sfp_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sd_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sdp_hold_off_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->sc_out_aps_timer);
            CTC_TASK_STOP_TIMER(p_db_ag_info->aps_tx_timer);
            PM_E_RETURN(tbl_aps_group_info_del_aps_group_info(&aps_group_info_key)); 
        }
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* del sub DS */
            break;
        } 
        p_pw_aps_group = (tbl_pw_aps_group_t*)p_tbl;
        if (NULL == p_pw_aps_group)
        {
            return PM_E_NONE;
        }
        sal_memset(&aps_group_info, 0, sizeof(aps_group_info));
        aps_group_info.key.aps_group_id = p_pw_aps_group->key.group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&(aps_group_info.key));
        if (NULL == p_db_ag_info)
        {
            return PM_E_NOT_EXIST;
        }
        switch (field_id)
        {
        case TBL_PW_APS_GROUP_FLD_WORKING_PW_NAME:
            sal_memcpy(aps_group_info.working_name, p_pw_aps_group->working_pw_name, LSPNAME_SIZE);
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_WORKING_NAME));
            break;
            
        case TBL_PW_APS_GROUP_FLD_PROTECTION_PW_NAME:
            sal_memcpy(aps_group_info.protection_name, p_pw_aps_group->protection_pw_name, LSPNAME_SIZE);
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_PROTECTION_NAME));
            break;
            
        case TBL_PW_APS_GROUP_FLD_IS_REVERTIVE:
            aps_group_info.is_revertive = p_pw_aps_group->is_revertive;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_IS_REVERTIVE));

            /*init aps group info*/
            PM_E_RETURN(g8131_aps_group_info_initialize(&aps_group_info, p_pw_aps_group->is_revertive, 
                                                        G8131_NO_PERMANENT_BRIDGE, G8131_BIDIRECTIONAL_SWITCHING));
            /*reset aps group info*/
            PM_E_RETURN(g8131_reset_aps_group_info(&aps_group_info));
            break;

        case TBL_PW_APS_GROUP_FLD_WAIT_TO_RESTORE_TIME:
            aps_group_info.wait_to_restore_time = p_pw_aps_group->wait_to_restore_time;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_WAIT_TO_RESTORE_TIME));
            break;

        case TBL_PW_APS_GROUP_FLD_HOLD_OFF_TIME:
            aps_group_info.hold_off_time = p_pw_aps_group->hold_off_time;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_HOLD_OFF_TIME));

            /*process hold off time fucntion*/
            if (G8131_ACTIVE_NONE != p_db_ag_info->active_path)
            {
                if ((G8131_FSM_STATE_LO != p_db_ag_info->current_state) &&
                    (G8131_HOLDOFF_TIMER_MIN_VALUE == p_pw_aps_group->hold_off_time))
                {
                    G8131_EVENT_DEBUG("SET HOLD-OFF-TIME: pw aps group %u fast aps true", p_db_ag_info->key.aps_group_id);
                    aps_group_info.fast_aps_en = TRUE;
                    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
                    PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, TRUE));
                }
                else
                {
                    G8131_EVENT_DEBUG("SET HOLD-OFF-TIME: pw aps group %u fast aps false", p_db_ag_info->key.aps_group_id);
                    aps_group_info.fast_aps_en = FALSE;
                    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
                    PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, FALSE));
                }
            }
            break;

        case TBL_PW_APS_GROUP_FLD_BRIDGE_PERMANENT:
            aps_group_info.bridge_permanent = p_pw_aps_group->bridge_permanent;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(&aps_group_info, TBL_APS_GROUP_INFO_FLD_BRIDGE_PERMANENT));
            break;
        case TBL_PW_APS_GROUP_FLD_FLOW_FLAG:
            if (GLB_FLAG_ISSET(p_pw_aps_group->flow_flag, G8131_APS_GROUP_FLAG_ADD_BY_FLOW))
            {
                PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, p_db_ag_info->fast_aps_en));    
            }
            break;
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

static int32
g8131_aps_group_info_cmd_get_aps_group_info(uint32 aps_group_id, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == aps_group_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_aps_group_info_dump_one, &args);
    }
    else
    {
        sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
        aps_group_info_key.aps_group_id = aps_group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
        if (!p_db_ag_info)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("Aps group %u does not exist", aps_group_id);
        }        
        tbl_aps_group_info_dump_one(p_db_ag_info, &args);
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

static int32 
g8131_aps_group_info_cmd_set_current_event(tbl_aps_group_info_t* p_aps_group_info, tbl_aps_group_info_t* p_db_ag_info, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        CFG_CONFLICT_RET("G8131 is not in active state.");
    }
    
    if (G8131_FSM_L_EVENT_CLEAR == p_aps_group_info->current_event) 
    {
        /*TODO*/
    }
    else if (G8131_FSM_L_EVENT_EXER == p_aps_group_info->current_event)
    {
        if ((p_db_ag_info->current_state == G8131_FSM_STATE_NR_P) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_LO) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_FS) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_P) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_P) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_MS_P) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_MS_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_WTR))
        {
            CFG_CONFLICT_RET("Current state priority is equal or higher than Exercise");
        }
        if (p_db_ag_info->current_state == G8131_FSM_STATE_EXER_W ||
            p_db_ag_info->current_state == G8131_FSM_STATE_EXER_P)
        {
            CFG_CONFLICT_RET("Already in Exercise state");
        }
    }
    else if (G8131_FSM_L_EVENT_FS == p_aps_group_info->current_event) 
    {
        if (p_db_ag_info->current_state == G8131_FSM_STATE_LO ||
            p_db_ag_info->current_state == G8131_FSM_STATE_SF_P)
        {
            CFG_CONFLICT_RET("Current state priority is equal or higher than Forced Switch(FS)");
        }
        if (p_db_ag_info->current_state == G8131_FSM_STATE_FS)
        {
            CFG_CONFLICT_RET("Already in Forced Switch(FS) state");
        }
    }
    else if (G8131_FSM_L_EVENT_LO == p_aps_group_info->current_event)
    {
        if (p_db_ag_info->current_state == G8131_FSM_STATE_LO)
        {
            CFG_CONFLICT_RET("Already in Lockout of protection(LO) state");
        }    
    }
    else if (G8131_FSM_L_EVENT_MS_P == p_aps_group_info->current_event) 
    {
        if ((p_db_ag_info->current_state == G8131_FSM_STATE_LO) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_FS) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_P) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_P) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_MS_W))
        {
            CFG_CONFLICT_RET("Current state priority is equal or higher than Manual Switch to Protection(MS-P)");
        }
        if (p_db_ag_info->current_state == G8131_FSM_STATE_MS_P)
        {
            CFG_CONFLICT_RET("Already in Manual Switch to Protection(MS-P) state");
        }
    }
    else if (G8131_FSM_L_EVENT_MS_W == p_aps_group_info->current_event)
    {
        if ((p_db_ag_info->current_state == G8131_FSM_STATE_LO) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_FS) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_P) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SF_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_P) || 
            (p_db_ag_info->current_state == G8131_FSM_STATE_SD_W) ||
            (p_db_ag_info->current_state == G8131_FSM_STATE_MS_P))
        {
            CFG_CONFLICT_RET("Current state priority is equal or higher than Manual Switch to Working(MS-W)");
        }
        if (p_db_ag_info->current_state == G8131_FSM_STATE_MS_W)
        {
            CFG_CONFLICT_RET("Already in Manual Switch to Working(MS-W) state");
        }
    }

    /* 2. check duplicate config */
    if (p_aps_group_info->current_event == p_db_ag_info->current_event)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(g8131_aps_group_info_set_current_event(p_aps_group_info));
    return rc;
}

int32
g8131_debug_aps_group_info_cmd_set_event_change(tbl_aps_group_info_t* p_aps_group_info, tbl_aps_group_info_t* p_db_ag_info, 
   cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        CFG_CONFLICT_RET("G8131 is not in active state.");
    }

    if (p_aps_group_info->current_event == p_db_ag_info->current_event)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(g8131_debug_aps_group_info_set_event_change(p_aps_group_info));
    return rc;
}

int32
g8131_debug_aps_group_info_cmd_set_state_change(tbl_aps_group_info_t* p_aps_group_info, tbl_aps_group_info_t* p_db_ag_info, 
        uint32 working_active, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        CFG_CONFLICT_RET("G8131 is not in active state.");
    }
    
    if (p_aps_group_info->current_state == p_db_ag_info->current_state)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(g8131_debug_aps_group_info_set_state_change(p_aps_group_info, working_active));
    return rc;
}

static int32
g8131_aps_group_check( tbl_aps_group_info_t* p_db_ag_info, char* name, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    if (0 == sal_strcmp(name, "lsp-aps-group"))
    {
        if (G8131_APS_GROUP_TYPE_PW == p_db_ag_info->aps_group_type)
        {
            CFG_CONFLICT_RET("LSP aps group %u does not exist", p_db_ag_info->key.aps_group_id);
        }
    }
    else if (0 == sal_strcmp(name, "pw-aps-group"))
    {
        if (G8131_APS_GROUP_TYPE_LSP == p_db_ag_info->aps_group_type)
        {
            CFG_CONFLICT_RET("PW aps group %u does not exist", p_db_ag_info->key.aps_group_id);
        }
    }
    return rc;
}

static int32
g8131_aps_group_info_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 field_id = 0;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    p_node = cdb_get_tbl(TBL_APS_GROUP_INFO);

    tbl_aps_group_info_t aps_group_info;
    tbl_aps_group_info_t* p_db_ag_info = NULL;

    uint32 aps_group_id = 0;
    uint32 event = 0;
    uint32 state = 0;
    uint32 working_active = FALSE;
    
    CFG_DBG_FUNC();
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        aps_group_id = cdb_uint_str2val(argv[0], &rc);
        if (rc || (aps_group_id > G8131_APS_MAXIMUM))
        {
            CFG_CONFLICT_RET("Invalid aps group value");
        }
        
        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        sal_memset(&aps_group_info, 0, sizeof(aps_group_info));
        aps_group_info.key.aps_group_id = aps_group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&(aps_group_info.key));  
        if (NULL == p_db_ag_info)
        {
            CFG_CONFLICT_RET("Aps group %u does not exist", aps_group_id);
        } 
        
        switch (field_id)
        {
        case TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            rc = g8131_aps_group_check(p_db_ag_info, argv[4], para);
            if (rc < 0)
            {
                return rc;
            }
            if (0 == sal_strcmp(argv[2], "local_event")) 
            {
                g8131_aps_map_str_to_event(argv[3], &event);
                aps_group_info.current_event = event;
                rc = g8131_aps_group_info_cmd_set_current_event(&aps_group_info, p_db_ag_info, para);
            }
            else if (0 == sal_strcmp(argv[2], "local_change"))
            {
                g8131_debug_aps_map_str_to_event(argv[3], &event);
                aps_group_info.current_event = event;
                rc = g8131_debug_aps_group_info_cmd_set_event_change(&aps_group_info, p_db_ag_info, para);
            }
            break;

        case TBL_APS_GROUP_INFO_FLD_CURRENT_STATE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            rc = g8131_aps_group_check(p_db_ag_info, argv[3], para);
            if (rc < 0)
            {
                return rc;
            }
            g8131_debug_aps_map_str_to_state(argv[2], &state, &working_active);
            aps_group_info.current_state = state;
            rc = g8131_debug_aps_group_info_cmd_set_state_change(&aps_group_info, p_db_ag_info, working_active, para);
            break;
            
        case TBL_APS_GROUP_INFO_FLD_FAR_END_LAST_EVENT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            rc = g8131_aps_group_check(p_db_ag_info, argv[3], para);
            if (rc < 0)
            {
                return rc;
            }
            g8131_debug_aps_map_str_to_far_event(argv[2], &event);
            aps_group_info.current_event = event;
            rc = g8131_debug_aps_group_info_cmd_set_event_change(&aps_group_info, p_db_ag_info, para);
            break;
        }
        break;
        
    case CDB_OPER_GET:
        if (0 == argc)
        {
            aps_group_id = 0;
        }
        else if (1 == argc)
        {
            
            aps_group_id = cdb_uint_str2val(argv[0], &rc);
            if (rc || (aps_group_id > G8131_APS_MAXIMUM) || (aps_group_id <= 0))
            {
                CFG_INVALID_PARAM_RET("Invalid aps group value");
            }
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = g8131_aps_group_info_cmd_get_aps_group_info(aps_group_id, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
g8131_aps_group_info_show_brief(tbl_aps_group_info_t* p_db_ag_info, FILE *fp)
{
    if (G8131_APS_GROUP_TYPE_LSP == p_db_ag_info->aps_group_type)
    {   
        sal_fprintf(fp, "%-13u%-10s", p_db_ag_info->key.aps_group_id, "LSP");
    }
    else if (G8131_APS_GROUP_TYPE_PW == p_db_ag_info->aps_group_type)
    {
        sal_fprintf(fp, "%-13u%-10s", p_db_ag_info->key.aps_group_id, "PW");
    }

    if (G8131_ACTIVE_WORKING == p_db_ag_info->active_path)
    {
        sal_fprintf(fp, "Working        ");
    }
    else if (G8131_ACTIVE_PROTECTING == p_db_ag_info->active_path)
    {
        sal_fprintf(fp, "Protection     ");
    }
    else
    {
        sal_fprintf(fp, "None           ");
    }
    sal_fprintf(fp, "%s\n", (p_db_ag_info->active_path == G8131_ACTIVE_NONE) ? "N/A" : G8131_FSM_STATE_STR_SHORT(p_db_ag_info->current_state));

    return PM_E_NONE;
}

int32
g8131_aps_group_info_show_detail(tbl_aps_group_info_t* p_db_ag_info, FILE *fp)
{
    tbl_lsp_pe_t* p_db_lsp_pe = NULL;
    tbl_lsp_pe_key_t lsp_pe_key;
    tbl_pw_tpe_t* p_db_pw_tpe = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;
    
    uint32 wtr_timeleft = 0;
    uint32 holdoff_timeleft = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_SIZE];
    int32 rc = PM_E_NONE;
    
    
    sal_fprintf(fp, "===========================================================================\n");
    sal_fprintf(fp, " %-8s %-8s %-8s %-8s %-6s %-5s %-5s %-5s %-5s %-5s\n", "CS", "LS", "CE(L/R)", "FE(L/R)", "RS", "A", "B", "D", "R", "T");
    sal_fprintf(fp, "----------+--------+--------+--------+------+-----+-----+-----+-----+------\n");

    sal_fprintf(fp, " %-8s %-8s %-8s %-8s %-6s %-5s %-5s %-5s %-5s %-5s\n",
        (G8131_ACTIVE_NONE == p_db_ag_info->active_path)? "N/A" : G8131_FSM_STATE_STR_SHORT(p_db_ag_info->current_state),
        (G8131_ACTIVE_NONE == p_db_ag_info->active_path)? "N/A" : G8131_FSM_STATE_STR_SHORT(p_db_ag_info->last_state),
        (G8131_ACTIVE_NONE == p_db_ag_info->active_path)? "N/A" : G8131_FSM_EVENT_STR_SHORT(p_db_ag_info->current_event),
        (G8131_ACTIVE_NONE == p_db_ag_info->active_path)? "N/A" : G8131_FSM_EVENT_STR_SHORT(p_db_ag_info->far_end_last_event),
        (G8131_ACTIVE_NONE == p_db_ag_info->active_path)? "N/A" : ((p_db_ag_info->requested_signal)?"NORMAL":"NULL"),
        ((p_db_ag_info->aps_channel)?"APS":"N-APS"), ((p_db_ag_info->bridge_permanent)?"1+1":"1:1"), ((p_db_ag_info->direction)?"BI":"UNI"), 
        ((p_db_ag_info->is_revertive)?"REV":"N-REV"), ((p_db_ag_info->bridge_type)?"BR":"SL"));

    if (G8131_APS_GROUP_TYPE_LSP == p_db_ag_info->aps_group_type)
    {
        sal_fprintf(fp, "LSP Group ID     : %u\n", p_db_ag_info->key.aps_group_id);
        
        sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key)); 
        sal_strncpy(lsp_pe_key.name, p_db_ag_info->working_name, LSPNAME_SIZE);
        p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
        if (p_db_lsp_pe)
        {
            /*Modified by yejl for bug 47264, 2018-05-16*/
            rc = tbl_interface_get_name_by_ifindex(p_db_lsp_pe->ifindex, ifname, IFNAME_SIZE);
            if (rc)
            {
                 return PM_E_NONE;
            }
            IFNAME_ETH2FULL(ifname, ifname_ext);
            sal_fprintf(fp, "Working Info     : lsp_outlabel = %u  port = %s\n", p_db_lsp_pe->outlabel, ifname_ext);
        }
        else
        {
            sal_fprintf(fp, "Working Info     : N/A\n");
        }
        
        sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key)); 
        sal_strncpy(lsp_pe_key.name, p_db_ag_info->protection_name, LSPNAME_SIZE);
        p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
        if (p_db_lsp_pe)
        {
            rc = tbl_interface_get_name_by_ifindex(p_db_lsp_pe->ifindex, ifname, IFNAME_SIZE);
            if (rc)
            {
                 return PM_E_NONE;
            }
            IFNAME_ETH2FULL(ifname, ifname_ext);
            sal_fprintf(fp, "Protection Info  : lsp_outlabel = %u  port = %s\n", p_db_lsp_pe->outlabel, ifname_ext);
        }
        else
        {
            sal_fprintf(fp, "Protection Info  : N/A\n");
        }
    }
    else if (G8131_APS_GROUP_TYPE_PW == p_db_ag_info->aps_group_type)
    {
        sal_fprintf(fp, "PW Group ID      : %u\n", p_db_ag_info->key.aps_group_id);
            
        sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key)); 
        sal_strncpy(pw_tpe_key.name, p_db_ag_info->working_name, LSPNAME_SIZE);
        p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
        if (p_db_pw_tpe)
        {
            if (p_db_pw_tpe->lsp_aps_group_id)
            {
                sal_fprintf(fp, "Working Info     : pw_outlabel = %u  lsp_aps_group = %u\n", p_db_pw_tpe->outlabel, p_db_pw_tpe->lsp_aps_group_id);
            }
            else if (p_db_pw_tpe->lsp_name)
            {
                sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key)); 
                sal_strncpy(lsp_pe_key.name, p_db_pw_tpe->lsp_name, LSPNAME_SIZE);
                p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
                if (p_db_lsp_pe)
                {
                    rc = tbl_interface_get_name_by_ifindex(p_db_lsp_pe->ifindex, ifname, IFNAME_SIZE);
                    if (rc)
                    {
                         return PM_E_NONE;
                    }
                    IFNAME_ETH2FULL(ifname, ifname_ext);
                    sal_fprintf(fp, "Working Info     : pw_outlabel = %u  lsp_outlabel = %u  port = %s\n", p_db_pw_tpe->outlabel, p_db_lsp_pe->outlabel, ifname_ext);
                }
            }   
        }
        else
        {
            sal_fprintf(fp, "Working Info     : N/A\n");
        }
        
        sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key)); 
        sal_strncpy(pw_tpe_key.name, p_db_ag_info->protection_name, LSPNAME_SIZE);
        p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
        if (p_db_pw_tpe)
        {
            if (p_db_pw_tpe->lsp_aps_group_id)
            {
                sal_fprintf(fp, "Protection Info  : pw_outlabel = %u  lsp_aps_group = %u\n", p_db_pw_tpe->outlabel, p_db_pw_tpe->lsp_aps_group_id);
            }
            else if (p_db_pw_tpe->lsp_name)
            {
                sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key)); 
                sal_strncpy(lsp_pe_key.name, p_db_pw_tpe->lsp_name, LSPNAME_SIZE);
                p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
                if (p_db_lsp_pe)
                {
                    rc = tbl_interface_get_name_by_ifindex(p_db_lsp_pe->ifindex, ifname, IFNAME_SIZE);
                    if (rc)
                    {
                         return PM_E_NONE;
                    }
                    IFNAME_ETH2FULL(ifname, ifname_ext);
                    sal_fprintf(fp, "Protection Info  : pw_outlabel = %u  lsp_outlabel = %u  port = %s\n", p_db_pw_tpe->outlabel, p_db_lsp_pe->outlabel, ifname_ext);
                }
            }   
        }
        else
        {
            sal_fprintf(fp, "Protection Info  : N/A\n");
        }
    }

    if (G8131_ACTIVE_WORKING == p_db_ag_info->active_path)
    {
        sal_fprintf(fp, "Active-Path      : Working\n");
    }
    else if (G8131_ACTIVE_PROTECTING == p_db_ag_info->active_path)
    {
        sal_fprintf(fp, "Active-Path      : Protection\n");
    }
    else
    {
        sal_fprintf(fp, "Active-Path      : None\n");
    }

    /*
    time proecss
    */

    if (p_db_ag_info->wtr_timer)
    {
        wtr_timeleft = ctc_timer_get_remain_sec(p_db_ag_info->wtr_timer);
        sal_fprintf (fp, "WTR-Timer        : %u(s)/%u(s)\n", wtr_timeleft, p_db_ag_info->wait_to_restore_time);
    }
    else
    {
        if (TRUE == p_db_ag_info->is_revertive)
        {
            sal_fprintf (fp, "WTR-Timer        : -/%u(s)\n", p_db_ag_info->wait_to_restore_time);
        }
        else
        {
            sal_fprintf (fp, "WTR-Timer        : -/-\n");
        }
    }

    if (p_db_ag_info->sf_hold_off_timer)
    {
        holdoff_timeleft = ctc_timer_get_remain_msec(p_db_ag_info->sf_hold_off_timer);
        sal_fprintf (fp, "SF HOLD OFF-Timer: %u(ms)/%u(ms)\n", holdoff_timeleft, p_db_ag_info->hold_off_time* 100);
    }
    if (p_db_ag_info->sfp_hold_off_timer)
    {
        /*Modified by yejl for bug 47431, 2018-05-26*/
        holdoff_timeleft = ctc_timer_get_remain_msec(p_db_ag_info->sfp_hold_off_timer);
        sal_fprintf (fp, "SFP HOLD OFF-Timer: %u(ms)/%u(ms)\n", holdoff_timeleft, p_db_ag_info->hold_off_time* 100);
    }
    if (p_db_ag_info->sd_hold_off_timer)
    {
        holdoff_timeleft = ctc_timer_get_remain_msec(p_db_ag_info->sd_hold_off_timer);
        sal_fprintf (fp, "SD HOLD OFF-Timer: %u(ms)/%u(ms)\n", holdoff_timeleft, p_db_ag_info->hold_off_time* 100);
    }
    if (p_db_ag_info->sdp_hold_off_timer)
    {
        holdoff_timeleft = ctc_timer_get_remain_msec(p_db_ag_info->sdp_hold_off_timer);
        sal_fprintf (fp, "SDP HOLD OFF-Timer : %u(ms)/%u(ms)\n", holdoff_timeleft, p_db_ag_info->hold_off_time * 100);
    }

    if (!p_db_ag_info->sf_hold_off_timer && !p_db_ag_info->sfp_hold_off_timer &&
        !p_db_ag_info->sd_hold_off_timer && !p_db_ag_info->sdp_hold_off_timer)
    {
        if (G8131_HOLDOFF_TIMER_MIN_VALUE != p_db_ag_info->hold_off_time)
        {
            sal_fprintf (fp, "HOLD OFF-Timer   : -/%u(ms)\n", p_db_ag_info->hold_off_time * 100);
        }   
        else
        {
            sal_fprintf (fp, "HOLD OFF-Timer   : -/0(ms)\n");
        }
    }
    
    sal_fprintf (fp, "DFOP State       : %s\n", G8131_DEFECT_STR(p_db_ag_info->dfop_reason));

    return PM_E_NONE;
}

static int32
g8131_aps_group_info_show_counters(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_aps_group_info_master_t *p_master = tbl_aps_group_info_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_aps_group_info_t* p_db_ag_info = NULL;

    uint32 lsp_count = 0;
    uint32 pw_count = 0;

    sal_fprintf(fp, " %-10s %-10s %-10s\n", "LSP", "PW", "Total");
    sal_fprintf(fp, "------------+----------+----------\n");

    CTCLIB_SLIST_LOOP(p_master->aps_group_info_list, p_db_ag_info, listnode)
    {
        if (p_db_ag_info && p_db_ag_info->aps_group_type == G8131_APS_GROUP_TYPE_LSP)
        {
            lsp_count++;    
        } 
        else if (p_db_ag_info && p_db_ag_info->aps_group_type == G8131_APS_GROUP_TYPE_PW)
        {
            pw_count++;    
        }
    }
    sal_fprintf(fp, " %-10u %-10u %-10u\n", lsp_count, pw_count, lsp_count + pw_count);

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
_g8131_aps_group_info_show_brief_banner( FILE *fp)
{
    sal_fprintf(fp, "Group-Id     Type      Active-Path    Current-State\n");
    sal_fprintf(fp, "-------------+---------+--------------+--------------\n");
    
    return PM_E_NONE;
}

int32
_g8131_aps_group_info_show_datail_banner( FILE *fp)
{
    /*Modified by yejl for bug 47379 for the optimizationo of show g8131, 2018-05-23*/
    sal_fprintf(fp, "CS - Current State, LS - Last State, CE - Current Event, (L/R) - Local/Remote,\n");
    sal_fprintf(fp, "FE - Far end last Event, RS - Request Signal, WRSF - Working recovers from SF,\n");
    sal_fprintf(fp, "PRSF - Protecting recovers from SF, DFOP - Failure of protocol defects\n");
    sal_fprintf(fp, "A - APS protocol type (No APS Channel, APS Channel)\n");
    sal_fprintf(fp, "B - Local protection architecture type (1+1, 1:1)\n");
    sal_fprintf(fp, "D - Local protection switching type (Unidirectional, Bidirectional)\n");
    sal_fprintf(fp, "R - Local protection operation type (Non-revertive, Revertive)\n");
    sal_fprintf(fp, "T - Local Bridge Type (Selector, Broadcast)\n");
    
    return PM_E_NONE;
}

int32
g8131_aps_group_info_show_all_detail(uint32 is_lsp, uint32 is_pw, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_aps_group_info_master_t *p_master1 = tbl_aps_group_info_get_master();
    tbl_aps_group_info_master_t *p_master2 = tbl_aps_group_info_get_master();
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *listnode2 = NULL;
    tbl_aps_group_info_t* p_db_ag_info1 = NULL;
    tbl_aps_group_info_t* p_db_ag_info2 = NULL;

    _g8131_aps_group_info_show_datail_banner(fp);

    if (is_lsp)
    {
        CTCLIB_SLIST_LOOP(p_master1->aps_group_info_list, p_db_ag_info1, listnode1)
        {
            if (p_db_ag_info1 && p_db_ag_info1->aps_group_type == G8131_APS_GROUP_TYPE_LSP)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info1, fp);
                sal_fprintf(fp, "\n");
            }  
        }
    }

    if (is_pw)
    {
        CTCLIB_SLIST_LOOP(p_master2->aps_group_info_list, p_db_ag_info2, listnode2)
        {
            if (p_db_ag_info2 && p_db_ag_info2->aps_group_type == G8131_APS_GROUP_TYPE_PW)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info2, fp);
                sal_fprintf(fp, "\n");
            }  
        }
    }
   
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
g8131_aps_group_info_show_all_brief(uint32 is_lsp, uint32 is_pw, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_aps_group_info_master_t *p_master1 = tbl_aps_group_info_get_master();
    tbl_aps_group_info_master_t *p_master2 = tbl_aps_group_info_get_master();
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *listnode2 = NULL;
    tbl_aps_group_info_t* p_db_ag_info1 = NULL;
    tbl_aps_group_info_t* p_db_ag_info2 = NULL;

    _g8131_aps_group_info_show_brief_banner(fp);

    if (is_lsp)
    {
        CTCLIB_SLIST_LOOP(p_master1->aps_group_info_list, p_db_ag_info1, listnode1)
        {
            if (p_db_ag_info1 && p_db_ag_info1->aps_group_type == G8131_APS_GROUP_TYPE_LSP)
            {
                g8131_aps_group_info_show_brief(p_db_ag_info1, fp);
            }  
        }
    }

    if (is_pw)
    {
        CTCLIB_SLIST_LOOP(p_master2->aps_group_info_list, p_db_ag_info2, listnode2)
        {
            if (p_db_ag_info2 && p_db_ag_info2->aps_group_type == G8131_APS_GROUP_TYPE_PW)
            {
                g8131_aps_group_info_show_brief(p_db_ag_info2, fp);
            }  
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
g8131_aps_group_info_show_one_group(tbl_aps_group_info_t* p_db_ag_info, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_aps_group_info_t * p_db_ag_info1 = NULL;
    tbl_aps_group_info_t * p_db_ag_info2 = NULL;
    tbl_aps_group_info_key_t ag_info_key1;
    tbl_aps_group_info_key_t ag_info_key2;
    uint32 lsp_aps_group1 = 0;
    uint32 lsp_aps_group2 = 0;

    _g8131_aps_group_info_show_datail_banner(fp);
    g8131_aps_group_info_show_detail(p_db_ag_info, fp);

    /*Modified by yejl for bug47390, 2018-05-24*/
    g8131_pw_aps_get_lsp_aps(p_db_ag_info, &lsp_aps_group1, &lsp_aps_group2);
    if (lsp_aps_group1 || lsp_aps_group2)
    {
        sal_memset(&ag_info_key1, 0, sizeof(ag_info_key1));
        ag_info_key1.aps_group_id = lsp_aps_group1;
        p_db_ag_info1 = tbl_aps_group_info_get_aps_group_info(&ag_info_key1);
        sal_memset(&ag_info_key2, 0, sizeof(ag_info_key2));
        ag_info_key2.aps_group_id = lsp_aps_group2;
        p_db_ag_info2 = tbl_aps_group_info_get_aps_group_info(&ag_info_key2);
        
        if (lsp_aps_group1 < lsp_aps_group2)
        {
            if (p_db_ag_info1)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info1, fp);
            }
            if (p_db_ag_info2)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info2, fp);
            }
        }
        else if (lsp_aps_group1 > lsp_aps_group2)
        {
            if (p_db_ag_info2)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info2, fp);
            }
            if (p_db_ag_info1)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info1, fp);
            }
        }
        else
        {
            if (p_db_ag_info1)
            {
                g8131_aps_group_info_show_detail(p_db_ag_info1, fp);
            }
        }
    }
    /*end by yejl*/
    
    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

static int32
g8131_aps_group_info_cmd_show_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
    uint32 aps_group_id = 0;
    int32  rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            g8131_aps_group_info_show_all_detail(TRUE, TRUE, para);
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "brief"))
            {
                g8131_aps_group_info_show_all_brief(TRUE, TRUE, para);
            }
            else if (0 == sal_strcmp(argv[0], "counters"))
            {
                g8131_aps_group_info_show_counters(para);   
            }
            else if (0 == sal_strcmp(argv[0], "lsp-aps-group"))
            {
                g8131_aps_group_info_show_all_detail(TRUE, FALSE, para);
            }
            else if (0 == sal_strcmp(argv[0], "pw-aps-group"))
            {
                g8131_aps_group_info_show_all_detail(FALSE, TRUE, para);
            }
        }
        else if (2 == argc)
        {   
            if (0 == sal_strcmp(argv[0], "lsp-aps-group"))
            {
                if (0 == sal_strcmp(argv[1], "brief"))
                {
                    g8131_aps_group_info_show_all_brief(TRUE, FALSE, para);
                }
                else
                {
                    aps_group_id = cdb_uint_str2val(argv[1], &rc);
                    if (rc || (aps_group_id > G8131_APS_MAXIMUM))
                    {
                        CFG_CONFLICT_RET("Invalid aps group value");
                    }
                    
                    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
                    ag_info_key.aps_group_id = aps_group_id;
                    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
                    if (NULL == p_db_ag_info)
                    {
                        CFG_INVALID_PARAM_RET("LSP group id %u does not exist", aps_group_id);
                    }
                    else if (G8131_APS_GROUP_TYPE_LSP != p_db_ag_info->aps_group_type)
                    {
                        CFG_INVALID_PARAM_RET("LSP group id %u does not exist", aps_group_id);
                    }
                    g8131_aps_group_info_show_one_group(p_db_ag_info, para);
                }
            }
            else if (0 == sal_strcmp(argv[0], "pw-aps-group"))
            {
                if (0 == sal_strcmp(argv[1], "brief"))
                {
                    g8131_aps_group_info_show_all_brief(FALSE, TRUE, para);
                }
                else
                {
                    aps_group_id = cdb_uint_str2val(argv[1], &rc);
                    if (rc || (aps_group_id > G8131_APS_MAXIMUM))
                    {
                        CFG_CONFLICT_RET("Invalid aps group value");
                    }
                    
                    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
                    ag_info_key.aps_group_id = aps_group_id;
                    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
                    if (NULL == p_db_ag_info)
                    {
                        CFG_INVALID_PARAM_RET("PW group id %u does not exist", aps_group_id);
                    }
                    else if (G8131_APS_GROUP_TYPE_PW != p_db_ag_info->aps_group_type)
                    {
                        CFG_INVALID_PARAM_RET("PW group id %u does not exist", aps_group_id);
                    }
                    g8131_aps_group_info_show_one_group(p_db_ag_info, para);
                }
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
  
    return PM_E_NONE;
}

int32
g8131_start(void)
{
    cdb_pm_subscribe_tbl(TBL_LSP_APS_GROUP, g8131_lsp_aps_group_before_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_PW_APS_GROUP, g8131_pw_aps_group_before_sync, NULL);

    cdb_register_cfg_tbl_cb(TBL_APS_GROUP_INFO, g8131_aps_group_info_cmd_process);
    cdb_register_cfg_act_cb(ACT_SHOW_G8131, g8131_aps_group_info_cmd_show_process);
    
    return PM_E_NONE;
}

int32
g8131_stop(void)
{
    cdb_register_cfg_tbl_cb(TBL_APS_GROUP_INFO, NULL);

    return PM_E_NONE;
}

