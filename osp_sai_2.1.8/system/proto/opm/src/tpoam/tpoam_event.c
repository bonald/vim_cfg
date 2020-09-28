#include "proto.h"
#include "glb_openflow_define.h"

#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"

#include "tpoam_list.h"

#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_lm.h"
#include "tpoam_event.h"
#include "tpoam_debug.h"
#include "tpoam_api.h"
#include "tpoam_error.h"

#include "fei_openflow_tpoam.h"

extern int32
g8131_tpoam_notify_status(tbl_oam_session_t* p_oam_session, uint32 status, uint32 old_status);

enum glb_mplstp_mode_e
{
    GLB_MPLSTP_MODE_INVALID = 0,
    GLB_MPLSTP_MODE_IP,
    GLB_MPLSTP_MODE_NON_IP
};
typedef enum glb_mplstp_mode_e glb_mplstp_mode_t;

int32
hal_tpoam_set_rdi(tbl_oam_session_t* pst_oam, uint8 enable)
{
    glb_openflow_tpoam_t  oam_flow;
    int32 rc = PM_E_NONE;
    
    rc = tpoam_api_get_oam_flow(&oam_flow, pst_oam);
    if (PM_E_NONE != rc)
    {
        return PM_E_FAIL;
    }

    GLB_SET_FLAG(oam_flow.flag, GLB_TPOAM_UPDATE_RDI);
    oam_flow.mepid = pst_oam->mepid;
    oam_flow.rdi_enable = enable;
    
    fei_flow_update_tpoam_flow(&oam_flow);

    return TPOAM_E_NONE;
}


static int32
_tpoam_event_log(tbl_oam_session_t* pst_oam, u_int32_t is_set, u_int32_t event, 
    struct sal_timeval* pst_tv)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    tpoam_ratelimit_t* pst_limit = &p_db_tpoam_glb->log;
    tpoam_ratelimit_param_t* pst_param = &pst_limit->param;
    tpoam_ratelimit_stats_t* pst_stats = &pst_limit->stats;
    uint32 new_slice = TPOAM_TV_TO_SLICE(pst_tv, pst_param->token_interval);

    /* 1. alloc token based on slice */
    pst_stats->count++;    
    if (new_slice != pst_limit->slice)
    {
        pst_limit->slice = new_slice;
        pst_limit->token += pst_param->token_rate;
        if (pst_limit->token > pst_param->token_max)
        {
            pst_limit->token = pst_param->token_max;
        }
    }

    /* 2. not token, not do log */
    if (pst_limit->token == 0)
    {
        pst_stats->limited++;
        return TPOAM_E_NONE;
    }

    /* 3. has token, do log directly */
    pst_limit->token--;

    // TODO: log format
    //TPOAM_LOG_USER(E_WARNING, TPOAM_4_LOG_EVENT, pst_oam->megid, tpoam_api_event_desc(event),
    //    is_set ? "occur" : "clear");

    return TPOAM_E_NONE;
}


static int32
_tpoam_event_trap(tbl_oam_session_t* pst_oam, u_int32_t is_set, u_int32_t event, 
    struct sal_timeval* pst_tv)
{
    #if 0
    tpoam_trap_entry_t entry;
    tpoam_trap_entry_t* pst_entry = &entry;
    tpoam_ratelimit_t* pst_limit = &g_p_tpoam_master->trap;
    tpoam_ratelimit_param_t* pst_param = &pst_limit->param;
    tpoam_ratelimit_stats_t* pst_stats = &pst_limit->stats;
    uint32 new_slice = TPOAM_TV_TO_SLICE(pst_tv, pst_param->token_interval);

    if (!pst_oam)
    {
        return TPOAM_E_FAIL;
    }

    /* 1. bulid a entry */    
    pst_entry->type = pst_oam->type;
    pst_entry->pw_type = pst_oam->pw_type;
    pst_entry->is_set = is_set;
    pst_entry->mep_id = (pst_oam->pst_lmep) ? pst_oam->pst_lmep->mep_id : 0;
    pst_entry->event = event;
    pst_entry->timestamp = pst_tv->tv_sec;

    /* system_modified by liwh for bug 18048 , 2012-03-07 */
    pal_mem_cpy(pst_entry->megid, pst_oam->megid, GLB_TPOAM_MEGID_STR_LEN);                

    /* 2. alloc token based on slice */
    pst_stats->count++;    
    if (new_slice != pst_limit->slice)
    {
        pst_limit->slice = new_slice;
        pst_limit->token += pst_param->token_rate;
        if (pst_limit->token > pst_param->token_max)
        {
            pst_limit->token = pst_param->token_max;
        }
    }

    /* 3. not token, add entry to pending list */
    if (pst_limit->token == 0)
    {
        _tpoam_event_add_trap_cache(pst_entry);
        pst_stats->limited++;
        return TPOAM_E_NONE;
    }

    /* 4. has token, send trap entry directly */
    pst_limit->token--;
    nsm_mpls_tp_oam_snmp_trap(pst_entry);
    g_p_tpoam_master->trap_cache.tx_count++;
    #endif
    return TPOAM_E_NONE;
}

static int32
_tpoam_event_add_event_cache(tbl_oam_session_t* pst_oam, uint32 event, uint32 is_set)
{
    tpoam_event_t* pst_event = &pst_oam->oam_event;
    tpoam_event_entry_t* pst_entry = NULL;
    tpoam_event_entry_t* pst_oldest_entry = NULL;    
    struct listnode* pst_oldest_node = NULL;
    
    pst_entry = XCALLOC(MEM_PM_TPOAM_EVENT_ENTRY, sizeof(tpoam_event_entry_t));
    if (!pst_entry)
    {
        return TPOAM_E_MEMORY_ALLOC_FAIL;
    }
    
    pst_entry->event = event;
    pst_entry->is_set = is_set;
    sal_time(&pst_entry->time);

    if (NULL == pst_event->pst_event_cache_list)
    {
        pst_event->pst_event_cache_list = list_new();
    }

    /*add to list*/
    if (LISTCOUNT(pst_event->pst_event_cache_list) < pst_event->max_count)
    {
        listnode_add(pst_event->pst_event_cache_list, pst_entry);
    }
    else if (LISTCOUNT(pst_event->pst_event_cache_list) == pst_event->max_count)
    {
        #ifdef support_many_event
        uint32 event_bmp = 0;
        GLB_SET_FLAG(event_bmp, GLB_TPOAM_EVT_EVT_CACHE_FULL);
        TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_EVT_CACHE_FULL, NULL));
        #endif
        /* delete oldest entry */
        pst_oldest_node = LISTHEAD(pst_event->pst_event_cache_list);
        pst_oldest_entry = GETDATA(pst_oldest_node);
        XFREE(MEM_PM_TPOAM_EVENT_ENTRY, pst_oldest_entry);
        list_delete_node(pst_event->pst_event_cache_list, pst_oldest_node);
        
        /* add new entry */
        listnode_add(pst_event->pst_event_cache_list, pst_entry);
    }
    else
    {
        TPOAM_ASSERT;
    }
    return TPOAM_E_NONE;
}


/*trigger 8131 to change*/
static int32
_tpoam_event_notify_signal_status(tbl_oam_session_t* pst_oam, tpoam_status_t signal_status)
{
    TPOAM_DBG_EVENT("Oam session %u, MEG %s, new signal status %s, trigger g8131 to change\n", 
        pst_oam->key.session_id, pst_oam->megid_str, tpoam_api_signal_status_desc(signal_status));
    tpoam_status_t old_signal_status = pst_oam->signal_status;

    pst_oam->signal_status = signal_status;
    tbl_oam_session_set_oam_session_field(pst_oam, TBL_OAM_SESSION_FLD_SIGNAL_STATUS);
    switch (pst_oam->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_LSP:
        g8131_tpoam_notify_status(pst_oam, signal_status, old_signal_status);    
        break;
     
    case GLB_OAM_TYPE_MPLS_PW:
        g8131_tpoam_notify_status(pst_oam, signal_status, old_signal_status);
        break;

    case GLB_OAM_TYPE_MPLS_SECTION:
        //nsm_mpls_ring_tunnel_notify_status(pst_oam, signal_status, old_signal_status);
        break;
        
    default:
        break;
    }

    //TPOAM_DBG_EVENT(EVENT_APS, "MEG %s switch from %s to %s", pst_oam->megid, 
    //    tpoam_api_signal_status_desc(old_signal_status), tpoam_api_signal_status_desc(signal_status));
    
    return TPOAM_E_NONE;
}

int32
_tpoam_event_upload_ccm_status(tbl_oam_session_t* pst_oam, tpoam_status_t new_signal_status)
{
    tbl_oam_session_upload_t * pst_oam_upload;
    pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
    if (pst_oam_upload)
    {
        pst_oam_upload->upload_ccm.ccm_status = new_signal_status;
        tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM);
    }
    else
    {
        TPOAM_LOG_ERR("Upload ccm status, but not find the tbl %u, error\n", pst_oam->key.session_id);
        return TPOAM_E_FAIL;
    }

    return TPOAM_E_NONE;
}

/*This function used to update SF status*/
int32
tpoam_event_update_signal_status(tbl_oam_session_t* pst_oam)
{        
    tpoam_status_t new_signal_status = 0;
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (TPOAM_INVALID_MEP_ID == pst_oam->mepid || TPOAM_INVALID_MEP_ID == pst_oam->rmepid)
    {
        /* not have rmep */
        new_signal_status = TPOAM_STATUS_INIT;
    }
    else
    {
        /* priority of sf is higher than sd */
        /*(sf is set, and not ais event) || (sf is set, and include ais event and is clent sf or server sf)*/
        if (pst_oam->oam_status & p_db_tpoam_glb->sf_reason_mask)
        {
            new_signal_status =TPOAM_STATUS_FAIL;
        }
        /*same with sf*/
        else if (pst_oam->oam_status & p_db_tpoam_glb->sd_reason_mask)
        {
            new_signal_status = TPOAM_STATUS_DEGRADE;
        }
        else
        {
            if (GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_WAIT_1ST_CC))
            {
                new_signal_status = TPOAM_STATUS_INIT;
            }
            else
            {
                new_signal_status = TPOAM_STATUS_OK;
            }
        }
    }

    TPOAM_DBG_EVENT("Oam session %u, MEG %s, new signal status %s, old signal status %s\n", 
        pst_oam->key.session_id, pst_oam->megid_str, tpoam_api_signal_status_desc(new_signal_status), tpoam_api_signal_status_desc(pst_oam->signal_status));

    if (pst_oam->signal_status != new_signal_status)
    {
        logid_sys(LOG_OAM_4_TPOAM_CCM_STATE_CHANGE, pst_oam->key.session_id, 
            tpoam_api_signal_status_desc(pst_oam->signal_status), tpoam_api_signal_status_desc(new_signal_status));
            
        TPOAM_OP_ERR_RET(_tpoam_event_notify_signal_status(pst_oam, new_signal_status));
        
        _tpoam_event_upload_ccm_status(pst_oam, new_signal_status);
    }
    return TPOAM_E_NONE;
}


/*event_bmp is the new bmp, it is include the config value;*/
/*event is a flag, means use which bit of event_bmp;*/
int32
tpoam_event_set_oam_status(tbl_oam_session_t* pst_oam, uint32 event_bmp, uint32 event, uint32* p_rdi_clr)
{
    if (p_rdi_clr)
    {
        TPOAM_DBG_EVENT("Oam session %u, MEG %s, all event bmp 0x%04X, %s event 0x%04X %s, rdi clear %u\n", 
            pst_oam->key.session_id, pst_oam->megid_str, event_bmp, tpoam_api_event_ext_desc(event_bmp), event, tpoam_api_event_desc(event), *p_rdi_clr);
    }
    else
    {
        TPOAM_DBG_EVENT("Oam session %u, MEG %s, all event bmp 0x%04X, %s event 0x%04X %s\n", 
            pst_oam->key.session_id, pst_oam->megid_str, event_bmp, tpoam_api_event_ext_desc(event_bmp), event, tpoam_api_event_desc(event));
    }
    uint32 do_log = FALSE;
    uint32 do_trap = FALSE;
    uint32 do_cache = FALSE;
    uint32 is_set = FALSE;
    struct sal_timeval tv;
    
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();

    /*judge for this event, do log or trap or cache ?*/
    if (GLB_FLAG_ISSET(p_db_tpoam_glb->event_rule_map[TPOAM_EVT_DEST_LOG], event))
    {
        do_log = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_tpoam_glb->event_rule_map[TPOAM_EVT_DEST_TRAP], event))
    {
        do_trap = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_tpoam_glb->event_rule_map[TPOAM_EVT_DEST_CACHE], event))
    {
        do_cache = TRUE;
    }

    /* only changes need do log, trap, cache */
    /*status is the old data, event_bmp is the new data, compare the event bit*/
    if (GLB_FLAG_ISSET(pst_oam->oam_status, event) != GLB_FLAG_ISSET(event_bmp, event))
    {
        /*set operate*/
        if (GLB_FLAG_ISSET(event_bmp, event))    
        {
            GLB_SET_FLAG(pst_oam->oam_status, event);
            is_set = TRUE;
        }
        /*clear operate*/
        else
        {
            GLB_UNSET_FLAG(pst_oam->oam_status, event);
            is_set = FALSE;
            if (p_rdi_clr)
            {
                *p_rdi_clr = TRUE;
            }
        }

        TPOAM_DBG_EVENT("Status changes, oam session %u, MEG %s, event %s, status %u, set %u\n", 
            pst_oam->key.session_id, pst_oam->megid_str, tpoam_api_event_desc(event), tpoam_api_event_desc(pst_oam->oam_status), is_set);
        
        /* modified by yaom for CRC trigger AIS 20140919 */
        /*if is wait 1st cc or event is one of sf or sd, update signal status*/
        if ((GLB_TPOAM_EVT_WAIT_1ST_CC == event) 
            || (p_db_tpoam_glb->sf_reason_mask & event)
            || (p_db_tpoam_glb->sd_reason_mask & event))
        {
            tpoam_event_update_signal_status(pst_oam);
        }

        sal_gettimeofday(&tv);
    
        /* do log */
        if (do_log)
        {
            _tpoam_event_log(pst_oam, is_set, event, &tv);
        }

        /* do trap */
        if (do_trap)
        {
            _tpoam_event_trap(pst_oam, is_set, event, &tv);
        }
        
        /* do cache */
        if (do_cache)
        {
            _tpoam_event_add_event_cache(pst_oam, event, is_set);
        }
    }
    return TPOAM_E_NONE;
}


static int32
_tpoam_event_set_rdi_tx(tbl_oam_session_t* pst_oam, uint32 enable)
{
    //uint32 label_or_section = 0;
    uint32 event_bmp = 0;
    //glb_mplstp_mode_t mode = GLB_MPLSTP_MODE_INVALID;
    //int32 ret = TPOAM_E_NONE;

    tbl_section_oam_t *pst_section = NULL;
    tbl_section_oam_key_t section_key;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_lsp_pe_key_t lsp_pe_key;
    //tbl_lsp_p_t *pst_lsp_p = NULL;
    //tbl_lsp_p_key_t lsp_p_key;
    tbl_pw_tpe_t * pst_tpe = NULL;
    tbl_pw_tpe_key_t  pw_tpe_key;
    
    /* No reason to trigger RDI */
    switch (pst_oam->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        section_key.ifindex = pst_oam->service_ifindex;
        pst_section = tbl_section_oam_get_section_oam(&section_key);
        if (NULL != pst_section)
        {
            //mode = GLB_MPLSTP_MODE_NON_IP;
            {
                //label_or_section = pst_section->key.ifindex;
            }
        }
        break;
    case GLB_OAM_TYPE_MPLS_LSP:
        sal_memset(&lsp_pe_key, 0x00, sizeof(lsp_pe_key));
        sal_strcpy(lsp_pe_key.name, pst_oam->service_name);
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
        if (NULL != pst_lsp_pe)
        {
            //maybe not need
            //mode = pst_lsp_pe->lsp_mode;
            //mode = ;
            //label_or_section = pst_lsp_pe->inlabel;
        }
        break;
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pst_oam->pw_type)
        {
        case TPOAM_PW_TYPE_TPE:
            sal_memset(&pw_tpe_key, 0x00, sizeof(pw_tpe_key));
            sal_strcpy(pw_tpe_key.name, pst_oam->service_name);
            pst_tpe = tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
            if (NULL != pst_tpe)
            {
                //label_or_section = pst_tpe->inlabel;
            }
            break;
        default:
            return TPOAM_E_INVALID_TYPE;
        }
        break;
    default:
        return TPOAM_E_INVALID_TYPE;
    }

    //hal_tpoam_set_rdi(pst_oam->oam_type, mode, label_or_section, pst_oam->mepid, enable);
    hal_tpoam_set_rdi(pst_oam, enable);
    GLB_UNSET_FLAG(event_bmp, GLB_TPOAM_EVT_RDI_TX);
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_RDI_TX, NULL));

    return TPOAM_E_NONE;            
}


static int32
_tpoam_event_update_rdi_status(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (0 == (pst_oam->oam_status & (p_db_tpoam_glb->rdi_reason_mask)))
    {
        TPOAM_OP_ERR_NO_RET(_tpoam_event_set_rdi_tx(pst_oam, FALSE));
    }
    return TPOAM_E_NONE;
}

int32
tpoam_event_update_oam_status(tbl_oam_session_t* pst_oam, hal_tpoam_event_entry_t* pst_event)
{
    TPOAM_DBG_EVENT("Update oam status session id %d, MEG %s, all event bmp 0x%04X %s", 
        pst_oam->key.session_id, pst_oam->megid_str, pst_event->event_bmp, tpoam_api_event_ext_desc(pst_event->event_bmp));
        
    uint32 rdi_rsn_clr = FALSE;
    uint32 sw_rdi = 0;
    uint32 hw_rdi = 0;
    //uint32 prev_status = 0;
    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();

    /* those reason maybe affect clear GLB_TPOAM_DEFECT_FLAG_TX_RDI */

    if (!pst_oam->mepid)
    {
        return TPOAM_E_NONE;
    }

    //prev_status = pst_oam->oam_status;
    /* update 5 RDI reason event */
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_LOC, &rdi_rsn_clr));
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_UNEXP_MEP, &rdi_rsn_clr));
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_UNEXP_PERIOD, &rdi_rsn_clr));
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_MISMERGE, &rdi_rsn_clr));
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_MEG_LVL, &rdi_rsn_clr));

    /* those reason not affect clear GLB_TPOAM_DEFECT_FLAG_TX_RDI */
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_RDI_RX, NULL));
    sw_rdi = (pst_event->event_bmp & (p_db_tpoam_glb->rdi_reason_mask)) ? GLB_TPOAM_EVT_RDI_TX : 0;
    hw_rdi = (pst_event->event_bmp & GLB_TPOAM_EVT_RDI_TX);
    if (sw_rdi != hw_rdi)
    {
        /*
        TPOAM_DBG_EVENT(EVENT_RX, "MEG %s RDI TX mismatch, RDI (SW 0x%04X, HW 0x%04X), status (old 0x%04X, new 0x%04X)", 
            pst_oam->megid, sw_rdi, hw_rdi, prev_status, pst_event->event_bmp);
        */
    }

    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, sw_rdi, GLB_TPOAM_EVT_RDI_TX, NULL));
    /* modified by yaom for CRC trigger AIS 20140919 */
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_CRC_DEFECT, NULL));
    TPOAM_OP_ERR_NO_RET(tpoam_event_set_oam_status(pst_oam, pst_event->event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC, NULL));

    if (rdi_rsn_clr)
    {
        TPOAM_OP_ERR_RET(_tpoam_event_update_rdi_status(pst_oam));
    }

    return TPOAM_E_NONE;
}

/*opm receive hsrv event msg to set oam status*/
int32
tpoam_event_handle_hsrv_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    hal_tpoam_event_t* pst_event = NULL;
    tbl_oam_session_t* pst_oam = NULL;
    uint32 i = 0;

    pst_event = (hal_tpoam_event_t*)(p_msg->data);
    
    for (i = 0; i < pst_event->count; i++)
    {
        pst_oam = tbl_oam_session_get_oam_session_by_session_id(pst_event->events[i].oam_session_id);
        if (NULL == pst_oam)
        {
            /* modified by yangl for bug 53815 2019-11-05 */
            TPOAM_LOG_DEBUG("Not found oam session by oam session id %u", pst_event->events[i].oam_session_id);
            /* yangl end */
            continue;
        }

        /* modified by yaom for CRC trigger AIS 20140919 */
        /* should keep crc event */
        if (GLB_FLAG_ISSET(pst_oam->oam_status, GLB_TPOAM_EVT_CRC_DEFECT))
        {
            pst_event->events[i].event_bmp |= GLB_TPOAM_EVT_CRC_DEFECT;
        }
        
        tpoam_event_update_oam_status(pst_oam, &pst_event->events[i]);
    }
    
    return PM_E_NONE;
}

int32
tpoam_event_add_rmep_set(tbl_oam_session_t* pst_oam)
{
    uint32 event_bmp = 0;
    /* modified by yaom for CRC trigger AIS 20140919 */
    uint32 ifindex = 0;
    uint32 oif_ix = 0;
    //uint32 mem_oif_ix = 0;
    tbl_interface_t *p_db_if = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;

    GLB_SET_FLAG(event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC);
    TPOAM_OP_ERR_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC, NULL));

    if (GLB_OAM_TYPE_MPLS_SECTION == pst_oam->oam_type)
    {
        ifindex = pst_oam->service_ifindex;
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == pst_oam->oam_type)
    {
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_oam->service_name);
        if (NULL != pst_lsp_pe)
        {
            {
                oif_ix = pst_lsp_pe->ifindex;
                //mem_oif_ix = pst_lsp->u.lsp_pe.mem_ifindex;
                //ifindex = mem_oif_ix ? mem_oif_ix : oif_ix;
                ifindex = oif_ix;
            }
        }
    }

    if (ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        //if (p_db_if && (GLB_IF_CRC_MONITOR_DISABLE != ifp->crc_monitor_type) && ifp->crc_alarm)
        if (p_db_if)
        {
            //GLB_SET_FLAG(event_bmp, GLB_TPOAM_EVT_CRC_DEFECT);
            //TPOAM_OP_ERR_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_CRC_DEFECT, NULL));
        }
    }

    return TPOAM_E_NONE;
}


int32
tpoam_event_del_rmep_clear(tbl_oam_session_t* pst_oam)
{
    uint32 clr_events = GLB_TPOAM_EVT_LOC 
                | GLB_TPOAM_EVT_RDI_RX 
                | GLB_TPOAM_EVT_UNEXP_PERIOD 
                | GLB_TPOAM_EVT_WAIT_1ST_CC
                | GLB_TPOAM_EVT_CRC_DEFECT; /* modified by yaom for CRC trigger AIS 20140919 */
    
    GLB_UNSET_FLAG(pst_oam->oam_status, clr_events);
    
    TPOAM_OP_ERR_RET(_tpoam_event_update_rdi_status(pst_oam));
    return TPOAM_E_NONE;
}

int32
tpoam_event_del_lmep_clear(tbl_oam_session_t* pst_oam)
{
    #ifdef tpoam_event_del_lmep_clear
    uint32 clr_events = GLB_TPOAM_EVT_LOC 
                | GLB_TPOAM_EVT_UNEXP_MEP 
                | GLB_TPOAM_EVT_UNEXP_PERIOD 
                | GLB_TPOAM_EVT_MISMERGE
                | GLB_TPOAM_EVT_MEG_LVL
                | GLB_TPOAM_EVT_RDI_RX
                | GLB_TPOAM_EVT_RDI_TX
                | GLB_TPOAM_EVT_WAIT_1ST_CC
                | GLB_TPOAM_EVT_AIS_RX
                | GLB_TPOAM_EVT_AIS_TX
                | GLB_TPOAM_EVT_AIS_DEFECT
                /* modified by yaom for CRC trigger AIS 20140919 */
                | GLB_TPOAM_EVT_CRC_DEFECT;
    #else
    uint32 clr_events = GLB_TPOAM_EVT_LOC 
                | GLB_TPOAM_EVT_UNEXP_MEP 
                | GLB_TPOAM_EVT_UNEXP_PERIOD 
                | GLB_TPOAM_EVT_MISMERGE
                | GLB_TPOAM_EVT_MEG_LVL
                | GLB_TPOAM_EVT_RDI_RX
                | GLB_TPOAM_EVT_RDI_TX
                | GLB_TPOAM_EVT_WAIT_1ST_CC
                /* modified by yaom for CRC trigger AIS 20140919 */
                | GLB_TPOAM_EVT_CRC_DEFECT;
    #endif
    GLB_UNSET_FLAG(pst_oam->oam_status, clr_events);

    /* need not update AIS for remove MEP will distroy AIS timer */

    /* need not update RDI for MEP is remove in HW */
    
    return TPOAM_E_NONE;
}

int32
tpoam_event_clear_oam_event(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t* bb)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_event_entry_t* pst_entry = NULL;
    tpoam_event_t* pst_event = &pst_oam->oam_event;
    
    LIST_LOOP_DEL(pst_event->pst_event_cache_list, pst_entry, pst_node, pst_next_node)
    {
        listnode_delete(pst_event->pst_event_cache_list, pst_entry);
        XFREE(MEM_PM_TPOAM_EVENT_ENTRY, pst_entry);
    }

    #ifdef support_many_event
    uint32 event_bmp = 0;
    GLB_UNSET_FLAG(event_bmp, GLB_TPOAM_EVT_EVT_CACHE_FULL);
    TPOAM_OP_ERR_RET(tpoam_event_set_oam_status(pst_oam, event_bmp, GLB_TPOAM_EVT_EVT_CACHE_FULL, NULL));
    #endif
    return TPOAM_E_NONE;    
}

int32
tpoam_event_init(tbl_oam_session_t* pst_oam)
{
    tpoam_event_t* pst_event = &pst_oam->oam_event;
    if (NULL == pst_event->pst_event_cache_list)
    {
        pst_event->pst_event_cache_list = list_new();
    }
    pst_event->max_count = TPOAM_EVT_CACHE_SIZE_DEF;
    
    return TPOAM_E_NONE;
}

int32
tpoam_event_deinit(tbl_oam_session_t* pst_oam)
{
    struct listnode* pst_next_node = NULL;
    struct listnode* pst_node = NULL;
    tpoam_event_entry_t* pst_entry = NULL;
    tpoam_event_t* pst_event = &pst_oam->oam_event;
    
    if (pst_event->pst_event_cache_list)
    {
        LIST_LOOP_DEL(pst_event->pst_event_cache_list, pst_entry, pst_node, pst_next_node)
        {
            listnode_delete(pst_event->pst_event_cache_list, pst_entry);
            XFREE(MEM_PM_TPOAM_EVENT_ENTRY, pst_entry);
        }
        list_free(pst_event->pst_event_cache_list);
    }
    pst_event->pst_event_cache_list = NULL;    
    return TPOAM_E_NONE;
}


