#include "proto.h"
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
#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "g8131_api.h"
#include "g8131_fsm.h"
#include "g8131_pdu.h"
#include "fei.h"
#include "fei_g8131.h"


char *
g8131_get_aps_type(tbl_aps_group_info_t* p_db_ag_info)
{
    if (G8131_APS_GROUP_TYPE_PW == p_db_ag_info->aps_group_type)
    {
        return "pw";
    }
    else 
    {
        return "lsp";
    }
    
}


tbl_oam_session_t*
g8131_get_oam_by_aps_group(char* name, uint32 type)
{
    tbl_lsp_pe_key_t lsp_pe_key;
    tbl_lsp_pe_t* p_db_lsp_pe = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;
    tbl_pw_tpe_t* p_db_pw_tpe = NULL;
    tbl_oam_session_key_t oam_session_key;
    tbl_oam_session_t* p_db_oam_session = NULL;

    if (G8131_APS_GROUP_TYPE_LSP == type)
    {
        sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key));
        sal_strncpy(lsp_pe_key.name, name, LSPNAME_SIZE);
        p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
        if (p_db_lsp_pe)
        {
            oam_session_key.session_id = p_db_lsp_pe->oam_session_id;
            p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
            if (p_db_oam_session)
            {
                return p_db_oam_session;
            }
            else
            {
                return NULL;
            }    
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key));
        sal_strncpy(pw_tpe_key.name, name, LSPNAME_SIZE);
        p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
        if (p_db_pw_tpe)
        {
            oam_session_key.session_id = p_db_pw_tpe->oam_session_id;
            p_db_oam_session = tbl_oam_session_get_oam_session(&oam_session_key);
            if (p_db_oam_session)
            {
                return p_db_oam_session;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }    
}

tbl_aps_group_info_t*
g8131_get_aps_group_by_oam(tbl_oam_session_t* p_oam_session)
{
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;

    if (NULL == p_oam_session)
    {
        return NULL;
    }
    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
    ag_info_key.aps_group_id = p_oam_session->aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
    if (p_db_ag_info)
    {
        return p_db_ag_info;
    }
    else
    {
        return NULL;
    }
}


int32
g8131_get_link_state (char* name, uint32 type)
{
    tbl_oam_session_t* p_db_oam_session = NULL;
    p_db_oam_session = g8131_get_oam_by_aps_group(name, type);
    if (p_db_oam_session)
    {
        return p_db_oam_session->signal_status;
    }
    else
    {
        return TPOAM_STATUS_INIT;
    }
}

int32
g8131_is_sf_or_sfp_reasserted_by_oam(char* name, uint32 type)
{
    tbl_oam_session_t* p_db_oam_session = NULL;
    p_db_oam_session = g8131_get_oam_by_aps_group(name, type);
    if (p_db_oam_session)
    {
        if (TPOAM_STATUS_FAIL == p_db_oam_session->signal_status)
        {
            return TRUE;
        }
    }
    else
    {
        return TRUE;
    }
    return FALSE;
}

int32
g8131_is_sd_or_sdp_reasserted_by_oam(char* name, uint32 type)
{
    tbl_oam_session_t* p_db_oam_session = NULL;
    p_db_oam_session = g8131_get_oam_by_aps_group(name, type);
    if (p_db_oam_session)
    {
        if (TPOAM_STATUS_DEGRADE == p_db_oam_session->signal_status)
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
    return FALSE;
}

int32
g8131_is_sfp_reasserted(tbl_aps_group_info_t* p_db_ag_info)
{
    if (NULL == p_db_ag_info)
    {
        return FALSE;
    }
    return ((g8131_is_sf_or_sfp_reasserted_by_oam(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE)));
}

int32
g8131_is_sf_reasserted(tbl_aps_group_info_t* p_db_ag_info)
{
    if (NULL == p_db_ag_info)
    {
        return FALSE;
    }
    return ((g8131_is_sf_or_sfp_reasserted_by_oam(p_db_ag_info->working_name, p_db_ag_info->aps_group_type))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE)));
}

int32
g8131_is_sdp_reasserted(tbl_aps_group_info_t* p_db_ag_info)
{
    if (NULL == p_db_ag_info)
    {
        return FALSE;
    }
    return ((g8131_is_sd_or_sdp_reasserted_by_oam(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE)));
}

int32
g8131_is_sd_reasserted(tbl_aps_group_info_t* p_db_ag_info)
{
    if (NULL == p_db_ag_info)
    {
        return FALSE;
    }
    return ((g8131_is_sd_or_sdp_reasserted_by_oam(p_db_ag_info->working_name, p_db_ag_info->aps_group_type))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE))
            &&(!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE)));
}

int32
g8131_tpoam_notify_status(tbl_oam_session_t* p_oam_session, uint32 status, uint32 old_status)
{
    int32 rc = PM_E_NONE;
    tbl_aps_group_info_t* p_db_ag_info = NULL;

    p_db_ag_info = g8131_get_aps_group_by_oam(p_oam_session);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NONE;
    }

    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        PM_E_RETURN(g8131_reset_aps_group_info(p_db_ag_info));
        return PM_E_NONE;
    }

    if (TPOAM_STATUS_INIT == status)
    {
        PM_E_RETURN(g8131_reset_aps_group_info(p_db_ag_info));
    }
    else if (TPOAM_STATUS_FAIL == status)
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_oam_session->service_name))
        {
            G8131_EVENT_DEBUG("Aps group %u: Local Signal fail on working\n", p_db_ag_info->key.aps_group_id);
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_W));
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_oam_session->service_name))
        {
            G8131_EVENT_DEBUG("Aps group %u: Local Signal fail on protection\n", p_db_ag_info->key.aps_group_id);
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_P));
        }
    }
    else if (TPOAM_STATUS_DEGRADE == status)
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_oam_session->service_name))
        {
            if (TPOAM_STATUS_FAIL == old_status)
            {
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF));
            }
            else
            {
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_W));
            }
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_oam_session->service_name))
        {
            if (TPOAM_STATUS_FAIL == old_status)
            {
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF));
            }
            else
            {
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_P));
            }
        }
    }
    else if (TPOAM_STATUS_OK == status)
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_oam_session->service_name))
        {
            if (TPOAM_STATUS_FAIL == old_status)
            {
                G8131_EVENT_DEBUG("Aps group %u: Local Signal Recovery from SF on working\n", p_db_ag_info->key.aps_group_id);
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF));
            }
            else
            {
                G8131_EVENT_DEBUG("Aps group %u: Local Signal Recovery from SD on working\n", p_db_ag_info->key.aps_group_id);
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_W_RCOVERS_FROM_SD));
            }
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_oam_session->service_name))
        {
            if (TPOAM_STATUS_FAIL == old_status)
            {
                G8131_EVENT_DEBUG("Aps group %u: Local Signal Recovery from SF on protection\n", p_db_ag_info->key.aps_group_id);
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF));
            }
            else
            {
                G8131_EVENT_DEBUG("Aps group %u: Local Signal Recovery from SD on protection\n", p_db_ag_info->key.aps_group_id);
                PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_P_RCOVERS_FROM_SD));
            }
        }
    }
    return rc;
}

int32
g8131_lsp_notify_status(tbl_lsp_pe_t* p_lsp_pe, uint32 status)
{
    int32 rc = PM_E_NONE;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;

    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
    ag_info_key.aps_group_id = p_lsp_pe->lsp_aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NONE;
    }
    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        return PM_E_NONE;
    }
    if (TPOAM_STATUS_FAIL == status)
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_lsp_pe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_W));
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_lsp_pe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_P));
        }
    }
    else if ((TPOAM_STATUS_OK == status)&&
        (TPOAM_STATUS_OK == g8131_get_link_state(p_lsp_pe->key.name, G8131_APS_GROUP_TYPE_LSP)))
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_lsp_pe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF));
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_lsp_pe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF));
        }
    }
    return rc;
}

int32
g8131_pw_notify_status(tbl_pw_tpe_t* p_pw_tpe, uint32 status)
{
    int32 rc = PM_E_NONE;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key; 
    
    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
    ag_info_key.aps_group_id = p_pw_tpe->pw_aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NONE;
    }
    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        return PM_E_NONE;
    }
    if (TPOAM_STATUS_FAIL == status)
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_pw_tpe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_W));
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_pw_tpe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_P));
        }
    }
    else if ((TPOAM_STATUS_OK == status) && 
        (TPOAM_STATUS_OK == g8131_get_link_state(p_pw_tpe->key.name, G8131_APS_GROUP_TYPE_PW)))
    {
        if (0 == sal_strcmp(p_db_ag_info->working_name, p_pw_tpe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF));
        }
        else if (0 == sal_strcmp(p_db_ag_info->protection_name, p_pw_tpe->key.name))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF));
        }
    }
    return rc;
}

int32 
g8131_pw_tpe_update_lsp_pe(tbl_lsp_pe_t *p_db_lsp_pe, uint32 lsp_status_ok)
{
    int32 rc = PM_E_NONE;
    tbl_pw_tpe_master_t* p_master = tbl_pw_tpe_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_pw_tpe_t* p_pw_tpe = NULL;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
        
    if (NULL == p_db_lsp_pe)
    {
        return PM_E_NONE;
    }
    CTCLIB_SLIST_LOOP(p_master->pw_tpe_list, p_pw_tpe, listnode)
    {
        if (NULL != p_pw_tpe)
        {
            continue;
        }
        if ((0 != sal_strcmp(p_pw_tpe->lsp_name, p_db_lsp_pe->key.name))
           &&(p_pw_tpe->lsp_aps_group_id != p_db_lsp_pe->lsp_aps_group_id))
        {
            continue;
        }
        if (lsp_status_ok)
        {
            PM_E_RETURN(g8131_pw_notify_status(p_pw_tpe, TPOAM_STATUS_OK));
        }
        else
        {
            PM_E_RETURN(g8131_pw_notify_status(p_pw_tpe, TPOAM_STATUS_FAIL));
        }
        sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
        ag_info_key.aps_group_id = p_pw_tpe->pw_aps_group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
        if (p_db_ag_info)
        {
            PM_E_RETURN(g8131_pw_aps_group_update_protection(p_db_ag_info));
        }
    }
    return rc;
}

int32
g8131_lsp_aps_group_update_protection(tbl_aps_group_info_t* p_db_ag_info)
{
    uint32 protection_en = FALSE;
    int32 rc = PM_E_NONE;
    
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe_pre = NULL;
    tbl_lsp_pe_key_t lsp_pe_key;
    tbl_lsp_pe_key_t lsp_pe_pre_key;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    /*get the previous select lsp pe table*/
    sal_memset(&lsp_pe_pre_key, 0, sizeof(lsp_pe_pre_key));
    sal_memcpy(lsp_pe_pre_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);
    p_db_lsp_pe_pre = tbl_lsp_pe_get_lsp_pe(&lsp_pe_pre_key);

    /*set the select name to null*/
    sal_memset(&(p_db_ag_info->select_name), 0, sizeof(p_db_ag_info->select_name));

    /*working active: protection_en is false; protection active: protection_en is true*/
    if (G8131_ACTIVE_NONE != p_db_ag_info->active_path)
    {
        if (G8131_ACTIVE_WORKING == p_db_ag_info->active_path)
        {
            sal_memcpy(p_db_ag_info->select_name, p_db_ag_info->working_name, LSPNAME_SIZE);
            protection_en = FALSE;
        }
        else
        {
            sal_memcpy(p_db_ag_info->select_name, p_db_ag_info->protection_name, LSPNAME_SIZE); 
            protection_en = TRUE;
        }
    }
    /*get the current select lsp pe table*/
    sal_memset(&lsp_pe_key, 0, sizeof(lsp_pe_key));
    sal_memcpy(lsp_pe_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);
    p_db_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);

    /*if previous lsp table is not equal to current lsp table, protection function precess*/
    if (p_db_lsp_pe_pre != p_db_lsp_pe)
    {
        if (p_db_lsp_pe)
        {
            /*modified by yejl for bug 47179, 2018-05-15*/
            logid_sys(LOG_OAM_4_G8131_LINK_CHANGE, "LSP", p_db_ag_info->key.aps_group_id, protection_en ? "protection" : "working");
            PM_E_RETURN(fei_g8131_mpls_protection_en(G8131_APS_GROUP_TYPE_LSP, p_db_ag_info->key.aps_group_id, protection_en));
        }
        if ((!p_db_lsp_pe_pre && p_db_lsp_pe) || (p_db_lsp_pe_pre && !p_db_lsp_pe))
        {
            if (p_db_lsp_pe_pre)
            {
                /*update the pw depend on this lsp */
                PM_E_RETURN(g8131_pw_tpe_update_lsp_pe(p_db_lsp_pe_pre, FALSE));
            }
            else if (p_db_lsp_pe)
            {
                /*update the pw depend on this lsp */
                PM_E_RETURN(g8131_pw_tpe_update_lsp_pe(p_db_lsp_pe, TRUE));
            }
        }
    }
    return rc;
}

int32
g8131_pw_aps_group_update_protection(tbl_aps_group_info_t* p_db_ag_info)
{
    int32 rc = PM_E_NONE;
    uint32 protection_en = FALSE;
    
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;
    tbl_pw_tpe_t *p_db_pw_tpe_pre = NULL;
    tbl_pw_tpe_key_t pw_tpe_key;
    tbl_pw_tpe_key_t pw_tpe_pre_key;

    /*get the previous select lsp pe table*/
    sal_memset(&pw_tpe_pre_key, 0, sizeof(pw_tpe_pre_key));
    sal_memcpy(pw_tpe_pre_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);
    p_db_pw_tpe_pre = tbl_pw_tpe_get_pw_tpe(&pw_tpe_pre_key);

    /*set the select name to null*/
    sal_memset(&(p_db_ag_info->select_name), 0, sizeof(p_db_ag_info->select_name));

    /*working active: protection_en is false; protection active: protection_en is true*/
    if (G8131_ACTIVE_NONE != p_db_ag_info->active_path)
    {
        if (G8131_ACTIVE_WORKING == p_db_ag_info->active_path)
        {
            sal_memcpy(p_db_ag_info->select_name, p_db_ag_info->working_name, LSPNAME_SIZE);
            protection_en = FALSE;
        }
        else
        {
            sal_memcpy(p_db_ag_info->select_name, p_db_ag_info->protection_name, LSPNAME_SIZE); 
            protection_en = TRUE;
        }
    }
    /*get the current select lsp pe table*/
    sal_memset(&pw_tpe_key, 0, sizeof(pw_tpe_key));
    sal_memcpy(pw_tpe_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);
    p_db_pw_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);

    /*if previous lsp table is not equal to current lsp table, protection function precess*/
    if (p_db_pw_tpe_pre != p_db_pw_tpe)
    {
        if (p_db_pw_tpe)
        {
            /*modified by yejl for bug 47179, 2018-05-15*/
            logid_sys(LOG_OAM_4_G8131_LINK_CHANGE, "PW", p_db_ag_info->key.aps_group_id, protection_en ? "protection" : "working");
            PM_E_RETURN(fei_g8131_mpls_protection_en(G8131_APS_GROUP_TYPE_PW, p_db_ag_info->key.aps_group_id, protection_en));
        }
    }
    return rc;
    
}


int32
g8131_lsp_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en)
{
    int32 rc = PM_E_NONE;
    
    tbl_lsp_pe_t *p_db_lsp_pe_w = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe_p = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe = NULL;
    tbl_lsp_pe_t *p_db_lsp_pe_pre = NULL;
    tbl_lsp_pe_key_t lsp_pe_w_key;
    tbl_lsp_pe_key_t lsp_pe_p_key;
    tbl_lsp_pe_key_t lsp_pe_pre_key;

    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memset(&lsp_pe_w_key, 0, sizeof(lsp_pe_w_key));
    sal_memset(&lsp_pe_p_key, 0, sizeof(lsp_pe_p_key));
    sal_memset(&lsp_pe_pre_key, 0, sizeof(lsp_pe_pre_key));
    sal_memcpy(lsp_pe_w_key.name, p_db_ag_info->working_name, LSPNAME_SIZE);
    sal_memcpy(lsp_pe_p_key.name, p_db_ag_info->protection_name, LSPNAME_SIZE);
    sal_memcpy(lsp_pe_pre_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);

    p_db_lsp_pe_w = tbl_lsp_pe_get_lsp_pe(&lsp_pe_w_key);
    p_db_lsp_pe_p = tbl_lsp_pe_get_lsp_pe(&lsp_pe_p_key);
    p_db_lsp_pe_pre = tbl_lsp_pe_get_lsp_pe(&lsp_pe_pre_key);
    
    p_db_lsp_pe = protection_en ? p_db_lsp_pe_p : p_db_lsp_pe_w;
    if (p_db_lsp_pe)
    {
        sal_memcpy(p_db_ag_info->select_name, p_db_lsp_pe->key.name, LSPNAME_SIZE);
    }

    if (p_db_lsp_pe_pre != p_db_lsp_pe)
    {
        if (NULL != p_db_lsp_pe)
        {
            /*modified by yejl for bug 47179, 2018-05-15*/
            logid_sys(LOG_OAM_4_G8131_LINK_CHANGE, "LSP", p_db_ag_info->key.aps_group_id, protection_en ? "protection" : "working");
            PM_E_RETURN(fei_g8131_mpls_protection_en(G8131_APS_GROUP_TYPE_LSP, p_db_ag_info->key.aps_group_id, protection_en));
        }
        if ((!p_db_lsp_pe_pre && p_db_lsp_pe) || (p_db_lsp_pe_pre && !p_db_lsp_pe))
        {
            if (p_db_lsp_pe_pre)
            {
                /*update the pw depend on this lsp */
                PM_E_RETURN(g8131_pw_tpe_update_lsp_pe(p_db_lsp_pe_pre, FALSE));
            }
            else if (p_db_lsp_pe)
            {
                /*update the pw depend on this lsp */
                PM_E_RETURN(g8131_pw_tpe_update_lsp_pe(p_db_lsp_pe, TRUE));
            }
        }
    }    
    return PM_E_NONE;
}

int32
g8131_pw_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en)
{
    int32 rc = PM_E_NONE;
    
    tbl_pw_tpe_t *p_db_pw_tpe_w = NULL;
    tbl_pw_tpe_t *p_db_pw_tpe_p = NULL;
    tbl_pw_tpe_t *p_db_pw_tpe = NULL;
    tbl_pw_tpe_t *p_db_pw_tpe_pre = NULL;
    tbl_pw_tpe_key_t pw_tpe_w_key;
    tbl_pw_tpe_key_t pw_tpe_p_key;
    tbl_pw_tpe_key_t pw_tpe_pre_key;

    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memset(&pw_tpe_w_key, 0, sizeof(pw_tpe_w_key));
    sal_memset(&pw_tpe_p_key, 0, sizeof(pw_tpe_p_key));
    sal_memset(&pw_tpe_pre_key, 0, sizeof(pw_tpe_pre_key));
    sal_memcpy(pw_tpe_w_key.name, p_db_ag_info->working_name, LSPNAME_SIZE);
    sal_memcpy(pw_tpe_p_key.name, p_db_ag_info->protection_name, LSPNAME_SIZE);
    sal_memcpy(pw_tpe_pre_key.name, p_db_ag_info->select_name, LSPNAME_SIZE);

    p_db_pw_tpe_w = tbl_pw_tpe_get_pw_tpe(&pw_tpe_w_key);
    p_db_pw_tpe_p = tbl_pw_tpe_get_pw_tpe(&pw_tpe_p_key);
    p_db_pw_tpe_pre = tbl_pw_tpe_get_pw_tpe(&pw_tpe_pre_key);
    
    p_db_pw_tpe = protection_en ? p_db_pw_tpe_p : p_db_pw_tpe_w;
    if (p_db_pw_tpe)
    {   
        sal_memcpy(p_db_ag_info->select_name, p_db_pw_tpe->key.name, LSPNAME_SIZE);
    }
    
    if (p_db_pw_tpe_pre != p_db_pw_tpe)
    {
        if (NULL != p_db_pw_tpe)
        {
            /*modified by yejl for bug 47179, 2018-05-15*/
            logid_sys(LOG_OAM_4_G8131_LINK_CHANGE, "PW", p_db_ag_info->key.aps_group_id, protection_en ? "protection" : "working");
            PM_E_RETURN(fei_g8131_mpls_protection_en(G8131_APS_GROUP_TYPE_PW, p_db_ag_info->key.aps_group_id, protection_en));
        }
    }
        
    return PM_E_NONE;
}

int32
g8131_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    switch (p_db_ag_info->aps_group_type)
    {
    case G8131_APS_GROUP_TYPE_LSP:
        g8131_lsp_aps_group_protection_en(p_db_ag_info, protection_en);
        break;
    case G8131_APS_GROUP_TYPE_PW:
        g8131_pw_aps_group_protection_en(p_db_ag_info, protection_en);
        break;
    default:
        break;
    }
    return PM_E_NONE;
}

int32
g8131_aps_group_info_set_current_event(tbl_aps_group_info_t* p_aps_group_info)
{
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
    aps_group_info_key.aps_group_id = p_aps_group_info->key.aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_aps_group_info, TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT));

    /*If current event is lockout and holdoff time is 0, fast aps en is false*/
    if (G8131_FSM_L_EVENT_LO == p_aps_group_info->current_event)
    {
        if (G8131_HOLDOFF_TIMER_MIN_VALUE == p_db_ag_info->hold_off_time)
        {
            G8131_EVENT_DEBUG("SET CURRENT EVENT: %s aps group %u fast aps false",
                               g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id);

            p_db_ag_info->fast_aps_en = FALSE;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
            PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, FALSE));
        }
    }
    /*If current event is Clear and holdoff time is 0 and current state is Lockout, fast aps en is true*/
    if (G8131_FSM_L_EVENT_CLEAR == p_aps_group_info->current_event)
    {
        if ((G8131_HOLDOFF_TIMER_MIN_VALUE == p_db_ag_info->hold_off_time) && 
            (G8131_FSM_STATE_LO == p_db_ag_info->current_state))
        {
            G8131_EVENT_DEBUG("SET CURRENT EVENT: %s aps group %u fast aps true",
                               g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id);

            p_db_ag_info->fast_aps_en = TRUE;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
            PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, TRUE));
        }
    }
    /* Invoke FSM*/
    PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, p_aps_group_info->current_event));
    return rc;
}

int32
g8131_debug_aps_group_info_set_state_change(tbl_aps_group_info_t* p_aps_group_info, uint32 working_active)
{
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
    aps_group_info_key.aps_group_id = p_aps_group_info->key.aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_aps_group_info, TBL_APS_GROUP_INFO_FLD_CURRENT_STATE));

    PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, working_active));

    PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, p_aps_group_info->current_state));
    return rc;
}

int32
g8131_debug_aps_group_info_set_event_change(tbl_aps_group_info_t* p_aps_group_info)
{
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
    aps_group_info_key.aps_group_id = p_aps_group_info->key.aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_aps_group_info, TBL_APS_GROUP_INFO_FLD_CURRENT_EVENT));

    /* Invoke FSM*/
    PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, p_aps_group_info->current_event));
    return rc;
}


int32
g8131_aps_group_info_initialize(tbl_aps_group_info_t* p_aps_group_info, uint32 revertive, uint32 bridge_mode, uint32 direction)
{
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == p_aps_group_info)
    {
        return PM_E_NONE;
    }
    sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
    aps_group_info_key.aps_group_id = p_aps_group_info->key.aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_ag_info->last_state = G8131_FSM_STATE_NR_W;
    p_db_ag_info->current_state = G8131_FSM_STATE_NR_W;
    p_db_ag_info->hold_off_expired = 0;
    p_db_ag_info->sc_aps_intervel = 1;
    p_db_ag_info->active_path = G8131_ACTIVE_NONE;
    p_db_ag_info->current_event = G8131_FSM_EVENT_MAX;
    p_db_ag_info->far_end_last_event = G8131_FSM_EVENT_MAX;
    p_db_ag_info->dfop_reason = G8131_SUCCESS;
    p_db_ag_info->bridge_type = G8131_BROADCAST_BRIDGE;

    CTC_TASK_STOP_TIMER(p_db_ag_info->wtr_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sf_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sfp_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sd_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sdp_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sc_out_aps_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->aps_tx_timer);

    if (revertive)
    {
        p_db_ag_info->is_revertive = TRUE;
        p_db_ag_info->wait_to_restore_time = G8131_DEFAULT_WTR_TIMEOUT;
    }
    else
    {
        p_db_ag_info->is_revertive = FALSE;
        p_db_ag_info->wait_to_restore_time = 0;
    }
    if (!bridge_mode)
    {
        p_db_ag_info->bridge_permanent = TRUE;
    }
    else
    {
        p_db_ag_info->bridge_permanent = FALSE;
    }   
    p_db_ag_info->direction = direction;
    p_db_ag_info->aps_channel = G8131_APS_CHANNEL;
    p_db_ag_info->requested_signal = G8131_NULL_SIGNAL; /* data will be flowing on working path initially */
        
    return rc;
}

int32
g8131_reset_aps_group_info(tbl_aps_group_info_t* p_aps_group_info)
{
    tbl_aps_group_info_key_t aps_group_info_key;
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == p_aps_group_info)
    {
        return PM_E_NONE;
    }
    sal_memset(&aps_group_info_key, 0, sizeof(aps_group_info_key));
    aps_group_info_key.aps_group_id = p_aps_group_info->key.aps_group_id;
    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&aps_group_info_key);
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_ag_info->last_state = G8131_FSM_STATE_NR_W;
    p_db_ag_info->current_state = G8131_FSM_STATE_NR_W;
    p_db_ag_info->hold_off_expired = 0;
    p_db_ag_info->dfop_reason = G8131_SUCCESS;
    p_db_ag_info->current_event = G8131_FSM_EVENT_MAX;
    p_db_ag_info->far_end_last_event = G8131_FSM_EVENT_MAX;
    p_db_ag_info->active_path = G8131_ACTIVE_NONE;
    
    CTC_TASK_STOP_TIMER(p_db_ag_info->wtr_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sf_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sfp_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sd_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sdp_hold_off_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->sc_out_aps_timer);
    CTC_TASK_STOP_TIMER(p_db_ag_info->aps_tx_timer);

    p_db_ag_info->requested_signal = G8131_NULL_SIGNAL; /* data will be flowing on working path initially */ 
    
    /*do not fast aps en*/
//    G8131_EVENT_DEBUG("RESET APS GROUP INFO: %s aps group %d fast aps false",
//                       g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id); 
    p_db_ag_info->fast_aps_en = FALSE;
    PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
    PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, FALSE));

    /*aps tx timer*/
    p_db_ag_info->aps_tx_timer = p_db_ag_info->aps_tx_timer ? p_db_ag_info->aps_tx_timer:
        ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_aps_tx_timer_handler, p_db_ag_info, G8131_DEFAULT_APS_TX_INTERVAL);

    if ((NULL != p_db_ag_info->working_name) && (NULL != p_db_ag_info->protection_name) 
        && (TPOAM_STATUS_INIT != g8131_get_link_state(p_db_ag_info->working_name, p_db_ag_info->aps_group_type))
        && (TPOAM_STATUS_INIT != g8131_get_link_state(p_db_ag_info->protection_name, p_db_ag_info->aps_group_type)))
    {
        if ((G8131_HOLDOFF_TIMER_MIN_VALUE == p_db_ag_info->hold_off_time) && 
            (G8131_FSM_STATE_LO != p_db_ag_info->current_state))
        {
            G8131_EVENT_DEBUG("RESET APS GROUP INFO: %s aps group %u fast aps true", 
                               g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id);
            p_db_ag_info->fast_aps_en = TRUE;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
            PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, TRUE));
        }
        p_db_ag_info->active_path = G8131_ACTIVE_WORKING;
        PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH));
        
        if (g8131_is_sfp_reasserted(p_db_ag_info))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_P));
        }
        else if (g8131_is_sf_reasserted(p_db_ag_info))
        {
            p_db_ag_info->active_path = G8131_ACTIVE_PROTECTING;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH));
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SF_W));
        }

        /*for CRC trigger AIS 20140919 */    
        if (g8131_is_sdp_reasserted(p_db_ag_info))
        {
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_P));
        }
        else if (g8131_is_sd_reasserted(p_db_ag_info))
        {
            p_db_ag_info->active_path = G8131_ACTIVE_PROTECTING;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH));
            PM_E_RETURN(g8131_fsm_execute(p_db_ag_info, G8131_FSM_L_EVENT_SD_W));
        }
    }

    if (G8131_APS_GROUP_TYPE_LSP == p_db_ag_info->aps_group_type)
    {
        PM_E_RETURN(g8131_lsp_aps_group_update_protection(p_db_ag_info));
    }
    else if (G8131_APS_GROUP_TYPE_PW == p_db_ag_info->aps_group_type)
    {
        PM_E_RETURN(g8131_pw_aps_group_update_protection(p_db_ag_info));
    }
    
    return rc;
}

