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
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"

#include "tpoam_list.h"

#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_api.h"
#include "tpoam_event.h"
#include "tpoam_debug.h"
#include "tpoam_error.h"
#include "tpoam_lm.h"
#include "tpoam_lb.h"
#include "tpoam_dm.h"
#include "tpoam_cmd.h"
#include "g8131_api.h"

#include "fei_openflow_tpoam.h"

#define TPOAM_CHECK_MIP(MP_TYPE) \
do { \
    if (((MP_TYPE) == (TPOAM_MP_TYPE_MIP))) \
    { \
        return 0;\
    } \
} while (0)

#define TPOAM_MEP_ID_CHECK(_mep_id_)                \
    if ((_mep_id_) < TPOAM_MIN_MEP_ID               \
        || (_mep_id_) > TPOAM_MAX_MEP_ID)           \
    {                                                   \
        return TPOAM_E_INVALID_MEP_ID;              \
    }    

#define TPOAM_CC_INTERVAL_CHECK(_interval_)         \
    if ((_interval_) < TPOAM_MIN_CC_INTERVAL        \
        || (_interval_) > TPOAM_MAX_CC_INTERVAL)    \
    {                                                   \
        return TPOAM_E_INVALID_CCM_INTERVAL;        \
    }


/* Trap pending cache size */
#define TPOAM_TRAP_CACHE_SIZE_MAX   512

int32
tpoam_clear_session_all_timer(tbl_oam_session_t* pst_oam)
{
    tpoam_util_cancel_timer(&pst_oam->p_t_send_dm_timer);

    return TPOAM_E_NONE;
}

int32
tpoam_clear_session_all_cache_list(tbl_oam_session_t* pst_oam)
{
    tpoam_lm_destroy_list_dual_lm(pst_oam);
    tpoam_dm_destroy_list_dmm(pst_oam);

    return TPOAM_E_NONE;
}

int32
tpoam_api_add_rmep(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_tpoam_global = tbl_tpoam_global_get_tpoam_global();
    
    /* 1. Check Param */
    TPOAM_MEP_ID_CHECK(pst_oam->rmepid);
    
    /* 2. Do check */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }
    
    #if 0
    if (TPOAM_INVALID_MEP_ID != pst_oam->rmepid)
    {
        return TPOAM_E_RMEP_EXIST;
    }

    
    if (pst_oam->mepid == rmep_id)
    {
        return TPOAM_E_SAME_TO_LMEP_ID;
    }
    #endif
    
    /* 3. Add to db */
    p_tpoam_global->resouce_count_curr.rmep_cnt++;

    /* 4. Set GLB_TPOAM_EVT_WAIT_1ST_CC */
    tpoam_event_add_rmep_set(pst_oam);

    return TPOAM_E_NONE;
}


int32
tpoam_api_del_rmep(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_tpoam_global = tbl_tpoam_global_get_tpoam_global();
    
    //TPOAM_DBG_API("delete MEG %s RMEP %d", pst_oam->megid_str, pst_oam->rmepid);
    
    /* 1. Do check */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }

    if (TPOAM_INVALID_MEP_ID == pst_oam->rmepid)
    {
        return TPOAM_E_RMEP_NOT_EXIST;
    }

    /* 3. Clear RMEP related events */
    tpoam_event_del_rmep_clear(pst_oam);
    
    /* 4. Delete from db */
    pst_oam->rmepid = TPOAM_INVALID_MEP_ID;
    p_tpoam_global->resouce_count_curr.rmep_cnt--;

    TPOAM_OP_ERR_RET(tpoam_event_update_signal_status(pst_oam)); /* modified by yaom for CRC trigger AIS 20140919 */
    return TPOAM_E_NONE;
}

int32
tpoam_api_add_lmep(tbl_oam_session_t* pst_oam/*, uint16 mep_id, uint8 cc_interval, uint8 exp, uint32 tunnel_id*/)
{
    //uint32 lck_by_srv = FALSE;

    tbl_tpoam_global_t *p_tpoam_global = tbl_tpoam_global_get_tpoam_global();
    
    //TPOAM_DBG_API("add MEG %s LMEP %d, CC Interval %s EXP %d", 
    //    pst_oam->megid_str, pst_oam->mepid, tpoam_api_cc_interval_desc(pst_oam->cc_interval), pst_oam->exp);
    
    /* 1. Check Param */
    TPOAM_MEP_ID_CHECK(pst_oam->mepid);
    TPOAM_CC_INTERVAL_CHECK(pst_oam->cc_interval);
    
    /* 2. Do check */
    if (pst_oam->mepid)
    {
        //return TPOAM_E_LMEP_EXIST;
    }

    /* Check resource count */
    if (p_tpoam_global->resouce_count_curr.lmep_cnt >= p_tpoam_global->resouce_count_max.lmep_cnt)
    {
        //return TPOAM_E_LMEP_FULL;
    }
    
    //TPOAM_OP_ERR_RET(nsm_tpoam_api_get_lck_by_srv_service(pst_oam, NSM_TPOAM_DIR_MAX, &lck_by_srv));

    /* 4. Add pw-t-pe nh */
    //TPOAM_OP_ERR_RET(_nsm_tpoam_api_set_lmep_nh_tpe(pst_oam, TPOAM_OP_ADD));
        
    /* 5. Build data & add to db */
    //pst_oam->mepid = mep_id;
    //pst_oam->dm_enable = TRUE;
    //pst_oam->active = lck_by_srv ? FALSE : TRUE;
    //pst_oam->cc_interval = cc_interval;
    //pst_oam->rmepid = TPOAM_INVALID_MEP_ID;
    //pst_oam->ttl = TPOAM_TTL_DEF;
    //pst_oam->exp = exp;
    /* modified by yaom for y1731 tp oam adaption for GreatBelt 20130204*/
    //pst_oam->internal_tunnel_id = tunnel_id;
    //p_tpoam_global->resouce_count_curr.lmep_cnt++;

    return TPOAM_E_NONE;
}


int32
tpoam_api_del_lmep(tbl_oam_session_t* pst_oam)
{
    tbl_tpoam_global_t *p_tpoam_global = tbl_tpoam_global_get_tpoam_global();
    
    //TPOAM_DBG_API("delete MEG %s LMEP %d", pst_oam->megid_str, pst_oam->mepid);

    /* 1. Do check */
    if (!pst_oam->mepid)
    {
        return TPOAM_E_LMEP_NOT_EXIST;
    }

    if (TPOAM_INVALID_MEP_ID != pst_oam->rmepid)
    {
        //return TPOAM_E_RMEP_EXIST;
    }

    /* 6. Stop session based on the LMEP */
    //TPOAM_OP_ERR_RET(tpoam_lm_stop(pst_oam));
    //TPOAM_OP_ERR_RET(tpoam_lb_stop_lb(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_dm_stop(pst_oam));

    /* 2. Disable LM/CCM */
    if (TPOAM_LM_TYPE_DUAL == pst_oam->oam_lm.lm_type)
    {
        //tmp delete
        //tpoam_api_set_lm_dual_en(pst_oam, FALSE, NULL);
    }
    else if (TPOAM_LM_TYPE_SINGLE == pst_oam->oam_lm.lm_type)
    {
        //tpoam_api_set_lm_single_en(pst_oam, FALSE, NULL);
    }
    
    if (pst_oam->cc_enable)
    {
        //tmp delete
        //tpoam_api_set_cc_enable(FALSE, pst_oam);
    }
    
    /* 4. Del pw-t-pe nh */
    //TPOAM_OP_ERR_RET(_tpoam_api_set_lmep_nh_tpe(pst_oam, TPOAM_OP_DEL));

    /* 5. Clear LMEP related events */
    TPOAM_OP_ERR_RET(tpoam_event_del_lmep_clear(pst_oam));

    /* 7. Delete from db */
    p_tpoam_global->resouce_count_curr.lmep_cnt--;

    pst_oam->mepid = TPOAM_INVALID_MEP_ID;
   
    return TPOAM_E_NONE;
}

int32
tpoam_api_lmep_deinit(tbl_oam_session_t* pst_oam)
{
    if (!pst_oam->mepid)
    {
        return TPOAM_E_NONE;
    }
    
    if (TPOAM_INVALID_MEP_ID != pst_oam->rmepid)
    {
        //TPOAM_OP_ERR_RET(tpoam_api_del_rmep(pst_oam));
    }
    
    TPOAM_OP_ERR_RET(tpoam_api_del_lmep(pst_oam));
    return TPOAM_E_NONE;
}

int32
tpoam_api_meg_init(tbl_oam_session_t* pst_oam)
{
    //pst_oam->pst_service = NULL;
    pst_oam->oam_status = 0;
    //pst_oam->mip_en = FALSE;   
    //TPOAM_OP_ERR_RET(tpoam_lm_init(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_lb_init(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_event_init(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_dm_init(pst_oam));

    return TPOAM_E_NONE;
}

int32
tpoam_api_meg_deinit(tbl_oam_session_t* pst_oam)
{
    TPOAM_OP_ERR_RET(tpoam_api_lmep_deinit(pst_oam));
    //TPOAM_OP_ERR_RET(tpoam_lm_deinit(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_lb_deinit(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_dm_deinit(pst_oam));
    TPOAM_OP_ERR_RET(tpoam_event_deinit(pst_oam));
    //TPOAM_OP_ERR_RET(tpoam_api_set_mip_en(pst_oam, FALSE));    
    pst_oam->oam_status = 0;
    pst_oam->service_ifindex = 0;
    sal_memset(pst_oam->service_name, 0x00, sizeof(pst_oam->service_name));
 
    return TPOAM_E_NONE;
}

int32 
_tpoam_sync_mep(tbl_oam_session_t* pst_oam)
{
    int32 rc = PM_E_NONE;
    
    if (0 == pst_oam->mepid)
    {
        rc += tpoam_api_del_lmep(pst_oam);
    }
    else
    {
        rc += tpoam_api_add_lmep(pst_oam);
    }

    return rc;
}


int32 
_tpoam_sync_rmep(tbl_oam_session_t* pst_oam)
{
    int32 rc = PM_E_NONE;
    
    if (0 == pst_oam->rmepid)
    {
        rc += tpoam_api_del_rmep(pst_oam);
    }
    else
    {
        rc += tpoam_api_add_rmep(pst_oam);
    }

    return rc;
}

static int32
_tpoam_process_sync_oam_session_after_cb_flag_add_flow(tbl_oam_session_t *pst_oam)
{
    int32 rc = PM_E_NONE;

    if (pst_oam->flag_add_flow)
    {
        TPOAM_CHECK_MIP(pst_oam->mp_type);

        rc += tpoam_api_meg_init(pst_oam);
        
        rc += tpoam_api_add_rmep(pst_oam);
        rc += tpoam_api_add_lmep(pst_oam);
        
        if (pst_oam->dm_enable)
        {
            //tpoam_dm_para_t dm_para = {0};
            //tpoam_dm_para_init(&dm_para);
            rc = tpoam_dm_start(pst_oam, &pst_oam->oam_dm.para, FALSE);
        }

        if (pst_oam->lm_enable)
        {
            //rc = tpoam_api_set_lm_dual_en(pst_oam);
            if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
            {
                pst_oam->oam_list.pst_dual_lm_cache_list = list_new();
            }
            rc = tpoam_lm_start_calc();
        }
    }
    else
    {
        /*modified by chenc for bug 52663, 2019-06-27*/
        /*delete oam flow when do on demand test but oam session configuration is not default*/
        tpoam_lb_clean(pst_oam);
        /*chenc end*/
        
        //rc = tpoam_api_set_lm_dual_en(pst_oam);
        tpoam_cmd_clear_statistics_one(pst_oam, NULL);
        
        TPOAM_CHECK_MIP(pst_oam->mp_type);

        tpoam_dm_deinit_dm_session(pst_oam);
        rc = tpoam_dm_stop(pst_oam);
        rc = tpoam_lm_stop_calc();
        tpoam_clear_session_all_cache_list(pst_oam);
    }
    
    return rc;
}

static int32
_tpoam_process_sync_oam_session_after_cb_set_lm_enable(tbl_oam_session_t *pst_oam)
{
    int32 rc = PM_E_NONE;

    //if ((pst_oam->lm_enable && 0 != pst_oam->bind_oam_flow_id) || FALSE == pst_oam->lm_enable)
    {
        rc = tpoam_api_set_lm_dual_en(pst_oam);
    }
    
    if (pst_oam->lm_enable)
    {
        rc = tpoam_lm_start_calc();
    }
    else
    {
        rc = tpoam_lm_stop_calc();
    }
                
    return rc;
}

static int32
_tpoam_process_sync_oam_session_after_cb_set_dm_enable(tbl_oam_session_t *pst_oam)
{
    int32 rc = PM_E_NONE;
    
     if (pst_oam->dm_enable)
    {
        rc = tpoam_dm_start(pst_oam, &pst_oam->oam_dm.para, FALSE);
    }
    else
    {
        tpoam_dm_deinit_dm_session(pst_oam);
        rc = tpoam_dm_stop(pst_oam);
    }
    
    return rc;
}

/*added by yejl for bug 52508, 2019-06-24*/
static int32
_tpoam_process_sync_oam_session_after_cb_set_dm_threshold(tbl_oam_session_t *pst_oam)
{
    int32 rc = PM_E_NONE;
    tbl_oam_session_upload_t * pst_oam_upload = NULL;
    tbl_tpoam_global_t* p_tpoam_glb = tbl_tpoam_global_get_tpoam_global();
    
    if (pst_oam == NULL)
    {
        return PM_E_NONE;
    }
    tpoam_dm_t* pst_oam_dm = &pst_oam->oam_dm;
    if (pst_oam_dm && pst_oam_dm->delay_threshold == 0)
    {
        if (pst_oam_dm->is_event_occur)
        {
            pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
            if (pst_oam_upload)
            {
                if (p_tpoam_glb->dm_delay_threshold == 0)
                {
                    _tpoam_dm_event_clear(pst_oam, pst_oam_upload);    
                }
                else if (p_tpoam_glb->dm_delay_threshold > pst_oam_upload->upload_dm_event.delay)
                {
                    _tpoam_dm_event_clear(pst_oam, pst_oam_upload);
                }
            }
        }
    }
    return rc;
}
/*ended by yejl*/

static int32
_tpoam_process_sync_oam_session_after_cb_set_bind_oam_flow_id(tbl_oam_session_t *pst_oam)
{
    int32 rc = PM_E_NONE;
    tbl_oam_session_upload_t *pst_oam_upload = NULL;

    pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload((tbl_oam_session_upload_key_t *)&(pst_oam->key));
    if (pst_oam_upload)
    {
        pst_oam_upload->bind_oam_flow_id = pst_oam->bind_oam_flow_id;
        tbl_oam_session_upload_set_oam_session_upload_field(pst_oam_upload, TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID);
    }
    else
    {
        return TPOAM_E_FAIL;
    }
    
    return rc;
}

static int32
_tpoam_process_sync_oam_session_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_oam_session_t *pst_oam = NULL;
    tbl_oam_session_key_t *p_oam_session_key;
    tbl_oam_session_master_t *p_oam_session_master =  tbl_oam_session_get_master();
    
    tbl_oam_session_upload_t *pst_oam_upload = NULL;
    tbl_oam_session_upload_key_t *p_oam_session_upload_key = NULL;
    tbl_oam_session_upload_t oam_upload;

    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
     
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_session_key = (tbl_oam_session_key_t *)p_tbl;
        pst_oam = tbl_oam_session_get_oam_session(p_oam_session_key);
        if (NULL != pst_oam)
        {
            TPOAM_CHECK_MIP(pst_oam->mp_type);
            
            rc += tpoam_api_meg_init(pst_oam);
            if (0 != pst_oam->mepid && 0 != pst_oam->rmepid)
            {
                rc += tpoam_api_add_rmep(pst_oam);
                rc += tpoam_api_add_lmep(pst_oam);
            }

            if (pst_oam->creat_by_flow)
            {
                if (pst_oam->dm_enable)
                {
                    //tpoam_dm_para_t dm_para = {0};
                    //tpoam_dm_para_init(&dm_para);
                    rc = tpoam_dm_start(pst_oam, &pst_oam->oam_dm.para, FALSE);
                }

                if (pst_oam->lm_enable)
                {
                    if (NULL == pst_oam->oam_list.pst_dual_lm_cache_list)
                    {
                        pst_oam->oam_list.pst_dual_lm_cache_list = list_new();
                    }
                    rc = tpoam_lm_start_calc();
                }
            }
        }
        else
        {
            return PM_E_NONE;
        }
        
        /*prepare upload*/
        p_oam_session_upload_key = (tbl_oam_session_upload_key_t *)p_tbl;
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload(p_oam_session_upload_key);
        if (NULL == pst_oam_upload)
        {
            sal_memset(&oam_upload, 0x00, sizeof(oam_upload));
            oam_upload.key.session_id = p_oam_session_upload_key->session_id;
            oam_upload.bind_oam_flow_id = pst_oam->bind_oam_flow_id;
            tbl_oam_session_upload_add_oam_session_upload(&oam_upload);
        }
        
        /*added by yejl*/
        if (pst_oam->aps_group_id)
        {
            sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
            ag_info_key.aps_group_id = pst_oam->aps_group_id; 
            p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
            if (p_db_ag_info)
            {
                g8131_reset_aps_group_info(p_db_ag_info);
            }
        }
        
        /*end by yejl*/
        break;
    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        
        p_oam_session_key = (tbl_oam_session_key_t *)p_tbl;
        pst_oam = tbl_oam_session_get_oam_session(p_oam_session_key);
        if (NULL != pst_oam)
        {
            switch (field_id)
            {
            case TBL_OAM_SESSION_FLD_MEPID:
                //rc = _tpoam_sync_mep(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_RMEPID:
                //rc = _tpoam_sync_rmep(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_FLAG_ADD_FLOW:
                rc = _tpoam_process_sync_oam_session_after_cb_flag_add_flow(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_BIND_OAM_FLOW_ID:
                TPOAM_CHECK_MIP(pst_oam->mp_type);
                rc = _tpoam_process_sync_oam_session_after_cb_set_bind_oam_flow_id(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_LM_ENABLE:
                TPOAM_CHECK_MIP(pst_oam->mp_type);
                rc = _tpoam_process_sync_oam_session_after_cb_set_lm_enable(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_DM_ENABLE:
                TPOAM_CHECK_MIP(pst_oam->mp_type);
                rc = _tpoam_process_sync_oam_session_after_cb_set_dm_enable(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_OAM_DM:
                TPOAM_CHECK_MIP(pst_oam->mp_type);
                rc = _tpoam_process_sync_oam_session_after_cb_set_dm_threshold(pst_oam);
                break;
            case TBL_OAM_SESSION_FLD_APS_GROUP_ID:
                TPOAM_CHECK_MIP(pst_oam->mp_type);
                 /*added by yejl*/
                if (pst_oam->bind_oam_flow_id && pst_oam->aps_group_id)
                {
                    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
                    ag_info_key.aps_group_id = pst_oam->aps_group_id; 
                    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
                    if (p_db_ag_info)
                    {
                        g8131_reset_aps_group_info(p_db_ag_info);
                    }
                }
                break;
                /*end by yejl*/
            }
        }
        break;
    case CDB_OPER_DEL:
        if (0 == p_oam_session_master->oam_session_hash->count)
        {
            rc = tpoam_lm_stop_calc();
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}


static int32
_tpoam_process_sync_oam_session_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_oam_session_t*pst_oam = NULL;
    tbl_oam_session_key_t *p_oam_session_key;

    tbl_oam_session_upload_t*pst_oam_upload = NULL;
    tbl_oam_session_upload_key_t *p_oam_session_upload_key;

    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
     
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        p_oam_session_key = (tbl_oam_session_key_t *)p_tbl;
        pst_oam = tbl_oam_session_get_oam_session(p_oam_session_key);
        if (NULL != pst_oam)
        {
            /*added by yejl*/
            switch (field_id)
            {
            case TBL_OAM_SESSION_FLD_APS_GROUP_ID: 
                if (!pst_oam->bind_oam_flow_id && pst_oam->aps_group_id)
                {
                    sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
                    ag_info_key.aps_group_id = pst_oam->aps_group_id; 
                    p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
                    if (p_db_ag_info)
                    {
                        g8131_reset_aps_group_info(p_db_ag_info);
                    }
                }
                break;
            default:
                break;
            }
            /*end by yejl*/
        }
        break;
    case CDB_OPER_DEL:
        p_oam_session_key = (tbl_oam_session_key_t *)p_tbl;
        pst_oam = tbl_oam_session_get_oam_session(p_oam_session_key);
        if (NULL != pst_oam)
        {
            //TPOAM_CHECK_MIP(pst_oam->mp_type);
            if (pst_oam->mp_type == TPOAM_MP_TYPE_MIP)
            { 
                /*delete upload*/
                p_oam_session_upload_key = (tbl_oam_session_upload_key_t *)p_tbl;
                pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload(p_oam_session_upload_key);
                if (NULL != pst_oam_upload)
                {
                    tbl_oam_session_upload_del_oam_session_upload(p_oam_session_upload_key);
                }
                
                return 0;
            } 

            /*delete oam flow when do on demand test*/
            tpoam_lb_clean(pst_oam);
            
            tpoam_clear_session_all_timer(pst_oam);
            rc += tpoam_api_meg_deinit(pst_oam);
            rc += tpoam_api_del_rmep(pst_oam);
            rc += tpoam_api_del_lmep(pst_oam);
            if (pst_oam->creat_by_flow)
            {
                rc = tpoam_dm_stop(pst_oam);

                tpoam_lm_destroy_list_dual_lm(pst_oam);
            }
            
            tpoam_clear_session_all_cache_list(pst_oam);
        }
        else
        {
            return PM_E_NONE;
        }

        /*delete upload*/
        p_oam_session_upload_key = (tbl_oam_session_upload_key_t *)p_tbl;
        pst_oam_upload = tbl_oam_session_upload_get_oam_session_upload(p_oam_session_upload_key);
        if (NULL != pst_oam_upload)
        {
            tbl_oam_session_upload_del_oam_session_upload(p_oam_session_upload_key);
        }
        
        /*added by yejl*/
        if (pst_oam->aps_group_id)
        {
            sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
            ag_info_key.aps_group_id = pst_oam->aps_group_id; 
            p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
            if (p_db_ag_info)
            {
                g8131_reset_aps_group_info(p_db_ag_info);
            }
        }
        /*end by yejl*/
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

int32
tpoam_global_init()
{
    tbl_tpoam_global_t *p_tpoam_global = tbl_tpoam_global_get_tpoam_global();

    p_tpoam_global->channel_type = GLB_MPLS_OAM_ACH_CHAN_TYPE_Y1731;
    p_tpoam_global->resouce_count_max.section_cnt = 64;
    p_tpoam_global->resouce_count_max.megid_cnt = 1000; /* changed equal to MEP max count */
    p_tpoam_global->resouce_count_max.lmep_cnt = 1000;  /* (DRV_FOAM_MEP_MAX_ENTRY_NUM/2 - 1) index 0 is reserved */
    p_tpoam_global->resouce_count_max.rmep_cnt = 1000;
    p_tpoam_global->resouce_count_max.mip_cnt = 1000;   /* changed equal to MEP max count */
    p_tpoam_global->resouce_count_max.csf_cnt = 64;
    p_tpoam_global->dm_enable_global = TRUE;
    p_tpoam_global->dm_calc_interval = GLB_TPOAM_DM_CALCULATE_INTERVAL_DEFAULT;
    p_tpoam_global->dm_delay_threshold = GLB_TPOAM_DM_DELAY_THRESHOLD_DEFAULT;
    p_tpoam_global->lm_enable_global = TRUE;
    p_tpoam_global->lm_calc_interval = GLB_TPOAM_LM_CALCULATE_INTERVAL_DEFAULT;

    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_CHANNEL_TYPE);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_RESOUCE_COUNT_MAX);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_DM_ENABLE_GLOBAL);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_DM_CALC_INTERVAL);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_DM_DELAY_THRESHOLD);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_LM_ENABLE_GLOBAL);
    tbl_tpoam_global_set_tpoam_global_field(p_tpoam_global, TBL_TPOAM_GLOBAL_FLD_LM_CALC_INTERVAL);
    
    p_tpoam_global->trap.param.token_interval = 16;   /* 64*1024 us */
    p_tpoam_global->trap.param.token_rate = 1;
    p_tpoam_global->trap.param.token_max = 2;

    p_tpoam_global->log.param.token_interval = 16;    /* 64*1024 us */
    p_tpoam_global->log.param.token_rate = 6;
    p_tpoam_global->log.param.token_max = 12;

    p_tpoam_global->trap_cache.pst_cache_list = list_new();
    p_tpoam_global->trap_cache.max_count = TPOAM_TRAP_CACHE_SIZE_MAX;

    tpoam_api_set_fast_aps_enable(TRUE);
    //tpoam_api_set_rdi_reason(TPOAM_RDI_RSN_ALL, FALSE);
    tpoam_api_set_rdi_reason(TPOAM_RDI_RSN_ALL, TRUE);
    tpoam_api_set_sf_reason(TPOAM_SF_RSN_DEF);
    //tpoam_api_set_sf_reason(TPOAM_SF_RSN_ALL);
    /* modified by yaom for CRC trigger AIS 20140919 */
    tpoam_api_set_sd_reason(TPOAM_SD_RSN_DEF);
    tpoam_api_set_event_rule_map(TPOAM_EVT_LOG_DEF, TPOAM_EVT_DEST_LOG);
    tpoam_api_set_event_rule_map(TPOAM_EVT_TRAP_DEF, TPOAM_EVT_DEST_TRAP);
    tpoam_api_set_event_rule_map(TPOAM_EVT_CACHE_DEF, TPOAM_EVT_DEST_CACHE);
    
    return PM_E_NONE;
}

int32
tpoam_start(void)
{
    /*cmd process*/
    cdb_register_cfg_tbl_cb(TBL_TPOAM_GLOBAL,  tpoam_cmd_process_tpoam_global);
    cdb_register_cfg_tbl_cb(TBL_TPOAM_EXEC_GLOBAL,  tpoam_cmd_process_tpoam_exec_global);
    cdb_register_cfg_tbl_cb(TBL_OAM_SESSION_UPLOAD,  tpoam_cmd_process_oam_session_upload);

    cdb_register_cfg_act_cb(ACT_SHOW_TPOAM, tpoam_cmd_process_show);
    cdb_register_cfg_act_cb(ACT_CLEAR_TPOAM, tpoam_cmd_process_clear);
    cdb_register_cfg_act_cb(ACT_EXEC_TPOAM, tpoam_cmd_process_exec);

    /*process msg from hsrv*/
    ipc_register_hal_rx_fn(TPOAM_HAL_MSG_TYPE_TPOAM_EVENT, tpoam_event_handle_hsrv_msg);
    ipc_register_hal_rx_fn(TPOAM_HAL_MSG_TYPE_LM_DUAL_COUNTERS, tpoam_lm_dual_counters_handle_hsrv_msg);
    //ipc_register_hal_rx_fn(TPOAM_HAL_MSG_TYPE_LM_SD, tpoam_lm_dual_counters_handle_hsrv_msg);

    cdb_pm_subscribe_tbl(TBL_OAM_SESSION, _tpoam_process_sync_oam_session_before_cb, _tpoam_process_sync_oam_session_after_cb);

    tpoam_global_init();

    ipc_register_pkt_rx_fn(GLB_PKT_TPOAM, tpoam_packet_rx);
    
    return PM_E_NONE;
}

int32
tpoam_stop(void)
{

    return PM_E_NONE;
}

