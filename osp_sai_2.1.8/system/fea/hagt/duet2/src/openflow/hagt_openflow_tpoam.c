#include "ctc_api.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_chip.h"
#include "hagt_openflow.h"
#include "glb_tp_oam.h"
#include "ctc_task.h"
#include "ctclib_vector.h"

#include "hagt_openflow_tpoam.h"

extern hagt_master_t *g_pst_hagt_master;

#define CTC_OAM_OAM_CCM_SIM_INTERVAL   1000
#define CTC_OAM_OAM_DUAL_LM_INTERVAL   100
//#define CTC_OAM_OAM_DUAL_LM_INTERVAL   1000
static hagt_tpoam_t *g_pst_hagt_tpoam_master = NULL;

#define HAGT_TPOAM_DBG_COMMON(fmt, args...) \
do { \
    if (g_pst_hagt_tpoam_master->hagt_tpoam_debug)\
    HAGT_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define FLAG_ISSET(V,F)        (((V) & (F)) == (F))

extern int32
ex_cplusplus_cm_oam_update_ccm_timer(uint32 chip_id, uint32 min_ptr, uint32 max_ptr, uint32 update_times);

extern int32
hsrv_tpoam_get_aps_group_fast_aps_en(uint32 mep_index, uint32 event_bmp, uint32* p_fast_aps_en, uint32* p_aps_group, uint32* p_aps_group_offset, uint32* p_protection_en);

extern int32
hsrv_tpoam_update_pw_mep_nh_by_groupid(uint32   lsp_aps_group_id);

char*
tpoam_api_event_ext_desc(uint32 event)
{
    static char event_str[256] = {0};
    int len = 0;

    sal_sprintf(event_str + len, "TPOAM EVENT: ");
    len = sal_strlen(event_str);

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_NONE))
    {
        sal_sprintf(event_str + len, "None. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LOC))
    {
        sal_sprintf(event_str + len, "LOC. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_UNEXP_MEP))
    {
        sal_sprintf(event_str + len, "Unexp RMEPID. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_UNEXP_PERIOD))
    {
        sal_sprintf(event_str + len, "Unexp Interval. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_MISMERGE))
    {
        sal_sprintf(event_str + len, "Unexp MEGID. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_MEG_LVL))
    {
        sal_sprintf(event_str + len, "Unexp Level. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_RDI_RX))
    {
        sal_sprintf(event_str + len, "RDI RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_RDI_TX))
    {
        sal_sprintf(event_str + len, "RDI TX. ");
        len = sal_strlen(event_str);
    }

#ifdef support_many_event
    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_RX))
    {
        sal_sprintf(event_str + len, "AIS RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_TX))
    {
        sal_sprintf(event_str + len, "AIS TX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_AIS_DEFECT))
    {
        sal_sprintf(event_str + len, "AIS Defect. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_CSF_DEFECT))
    {
        sal_sprintf(event_str + len, "CSF Defect. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_RX))
    {
        sal_sprintf(event_str + len, "LCK RX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_TX))
    {
        sal_sprintf(event_str + len, "LCK TX. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_LCK_DEFECT))
    {
        sal_sprintf(event_str + len, "LCK Defect. ");
        len = sal_strlen(event_str);
    }
    
    if (FLAG_ISSET(event, GLB_TPOAM_EVT_EVT_CACHE_FULL))
    {
        sal_sprintf(event_str + len, "Event Full. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_1DM_CACHE_FULL))
    {
        sal_sprintf(event_str + len, "1DM Full. ");
        len = sal_strlen(event_str);
    }
#endif

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_WAIT_1ST_CC))
    {
        sal_sprintf(event_str + len, "Wait 1st CCM. ");
        len = sal_strlen(event_str);
    }

    if (FLAG_ISSET(event, GLB_TPOAM_EVT_CRC_DEFECT))
    {
        sal_sprintf(event_str + len, "CRC RX. ");
        len = sal_strlen(event_str);
    }

    return event_str;
}

static int32 _hagt_tpoam_update_ccm_cos_by_dual_lm(hagt_tpoam_lm_dual_t* pst_lmep_lm_dual)
{
    ctc_oam_update_t lmep_update;
    ctc_oam_update_t* pst_lmep_update = &lmep_update;
    
    sal_memset(&lmep_update, 0, sizeof(ctc_oam_update_t));
                
    /* set local mep parameters */
    pst_lmep_update->key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    switch (pst_lmep_lm_dual->oam_type)
    {
        case GLB_OAM_TYPE_MPLS_LSP:
        case GLB_OAM_TYPE_MPLS_PW:
            pst_lmep_update->key.u.tp.label = pst_lmep_lm_dual->key_id;
            break;
        case GLB_OAM_TYPE_MPLS_SECTION:
            pst_lmep_update->key.u.tp.gport_or_l3if_id = pst_lmep_lm_dual->key_id;
            GLB_SET_FLAG(pst_lmep_update->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);        
            break;
        default:
            HAGT_LOG_ERR("Invalid OAM type %hhu when build LMEP Key.", pst_lmep_lm_dual->oam_type);
            return HAGT_E_INVALID_PARAM;
    }

    lmep_update.is_local = TRUE;
    lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP;
    lmep_update.update_value = pst_lmep_lm_dual->lm_cos_value;

    HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
    return HAGT_E_NONE;
}

int32
_hagt_tpoam_rm_lm_dual(uint16  mep_index)
{
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual = NULL;
    hagt_tpoam_lm_dual_t  lm_dual;
    int32   ret = HAGT_E_NONE;

    sal_memset(&lm_dual, 0, sizeof(hagt_tpoam_lm_dual_t));
    lm_dual.mep_index = mep_index;
    
    /* delete dual lm information */
    pst_lmep_lm_dual = ctclib_hash_lookup(g_pst_hagt_tpoam_master->pst_lm_dual_hash, &lm_dual);
    if (pst_lmep_lm_dual)
    {
        /* update ccm cos to max when dual lm of per-cos mode is disabled */
        if ((CTC_OAM_LM_COS_TYPE_PER_COS == pst_lmep_lm_dual->lm_cos_type)
            || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type))
        {
            pst_lmep_lm_dual->lm_cos_value = CTC_MAX_COS - 1;
            
            ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
            if (ret)
            {
                HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam-type %d key_id %d mep_index %d"
                               " failure, ret is %d",
                               pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                               ret);
            }
        }
    
        ctclib_hash_release(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual);  
        HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
    }

    return HAGT_E_NONE;
}


static int32 
_hagt_tpoam_set_lm_disable(ctc_oam_update_t* pst_lmep_update, uint32 lm_type, uint32 mepindex)
{
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual = NULL;
    hagt_tpoam_lm_dual_t  lm_dual;
    int32   ret = HAGT_E_NONE;

    HAGT_TPOAM_DBG_COMMON("Enter into %s.", __FUNCTION__);

    sal_memset(&lm_dual, 0, sizeof(hagt_tpoam_lm_dual_t));
    lm_dual.mep_index = mepindex;
    lm_dual.lm_type = lm_type;

    pst_lmep_update->is_local = TRUE;
    
    {        
        /* set lm disable */
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
        pst_lmep_update->update_value = FALSE;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }

    /* for single lm, don't reserve lm information */
    if (CTC_OAM_LM_TYPE_DUAL != lm_dual.lm_type)
    {
        return HAGT_E_NONE;
    }
    
    {
        /* delete dual lm information */
        pst_lmep_lm_dual = ctclib_hash_lookup(g_pst_hagt_tpoam_master->pst_lm_dual_hash, &lm_dual);
        if (pst_lmep_lm_dual)
        {
            /* update ccm cos to max when dual lm of per-cos mode is disabled */
            if ((CTC_OAM_LM_COS_TYPE_PER_COS == pst_lmep_lm_dual->lm_cos_type)
                || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type))
            {
                pst_lmep_lm_dual->lm_cos_value = CTC_MAX_COS - 1;
                
                ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
                if (ret)
                {
                    HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam-type %d key_id %d mep_index %d"
                                   " failure, ret is %d",
                                   pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                                   ret);
                }
            }
        
            ctclib_hash_release(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual);  
            HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
        }
    }
    
    return HAGT_E_NONE;
}

static int32 
_hagt_tpoam_set_lm_enable(ctc_oam_update_t* pst_lmep_update, HalMsgTpoamAddOamReq_t* p_msg)
{
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual = NULL;
    hagt_tpoam_lm_dual_t  lm_dual;
    void* p_data1 = NULL;
    int32   ret = HAGT_E_NONE;

    HAGT_TPOAM_DBG_COMMON("Enter into %s.", __FUNCTION__);

    sal_memset(&lm_dual, 0, sizeof(hagt_tpoam_lm_dual_t));

    lm_dual.oam_type = p_msg->key.oamType;
    lm_dual.key_id = p_msg->key.id;
    lm_dual.mep_index = p_msg->mepIndex;
    lm_dual.lm_enable = p_msg->lmEnable;
    lm_dual.lm_type = p_msg->lmType;
    lm_dual.lm_cos_type = p_msg->lmCosType;
    lm_dual.lm_cos_value = p_msg->lmCosValue;
    lm_dual.stats_interval = p_msg->lmStatsInterval;
    lm_dual.timer_counter = lm_dual.stats_interval;

    pst_lmep_update->is_local = TRUE;
        
    if (lm_dual.lm_enable)
    {
        /* set lm type */
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE;
        pst_lmep_update->update_value = lm_dual.lm_type;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));

        /* set lm cos type */
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE;
        pst_lmep_update->update_value = lm_dual.lm_cos_type;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));

        /* set lm cos */
        if (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == lm_dual.lm_cos_type)
        {
            pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS;
            pst_lmep_update->update_value = lm_dual.lm_cos_value;
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
        }

        /* set lm enable */
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
        pst_lmep_update->update_value = lm_dual.lm_enable;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }
    else
    {        
        /* set lm disable */
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
        pst_lmep_update->update_value = lm_dual.lm_enable;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }

    /* for single lm, don't reserve lm information */
    if (CTC_OAM_LM_TYPE_DUAL != lm_dual.lm_type)
    {
        return HAGT_E_NONE;
    }
    
    if (lm_dual.lm_enable)
    {
        /* reserve dual lm information */
        
        /* create and reserve dual lm node */
        pst_lmep_lm_dual = HAGT_CALLOC(CTCLIB_MEM_TPOAM_MODULE, sizeof(hagt_tpoam_lm_dual_t));
        if (!pst_lmep_lm_dual)
        {
            /* set lm disable */
            pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
            //pst_lmep_update->update_value = lm_dual.lm_enable;
            pst_lmep_update->update_value = FALSE;
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
            return HAGT_E_NO_MEMORY;
        }

        sal_memcpy(pst_lmep_lm_dual, &lm_dual, sizeof(hagt_tpoam_lm_dual_t));
        
        p_data1 = ctclib_hash_get(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual, 
                                     ctclib_hash_alloc_intern);
        if (!p_data1)
        {
            /* set lm disable */
            pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
            //pst_lmep_update->update_value = lm_dual.lm_enable;
            pst_lmep_update->update_value = FALSE;
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
            
            HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
            return HAGT_E_NO_MEMORY;
        }

        /* update ccm cos to cos value when dual lm of per-cos mode is disabled */
        if (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type)    
        {
            ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
            if (ret)
            {
                HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam_type %d key_id_id %d mep_index %d"
                               " failure, ret is %d",
                               pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                               ret);
            }
        }

        if (g_pst_hagt_tpoam_master->pst_lm_dual_hash->count > HAGT_TPOAM_LM_DUAL_UPDATE_TIME)
        {
            g_pst_hagt_tpoam_master->lm_dual_hash_count_per_time 
                = g_pst_hagt_tpoam_master->pst_lm_dual_hash->count / HAGT_TPOAM_LM_DUAL_UPDATE_TIME;
        }
        else
        {
            g_pst_hagt_tpoam_master->lm_dual_hash_count_per_time = HAGT_TPOAM_LM_DUAL_UPDATE_PER_TIME_MIN;
        }
    }
    else
    {
        /* delete dual lm information */
        pst_lmep_lm_dual = ctclib_hash_lookup(g_pst_hagt_tpoam_master->pst_lm_dual_hash, &lm_dual);
        if (pst_lmep_lm_dual)
        {
            /* update ccm cos to max when dual lm of per-cos mode is disabled */
            if ((CTC_OAM_LM_COS_TYPE_PER_COS == pst_lmep_lm_dual->lm_cos_type)
                || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type))
            {
                pst_lmep_lm_dual->lm_cos_value = CTC_MAX_COS - 1;
                
                ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
                if (ret)
                {
                    HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam-type %d key_id %d mep_index %d"
                                   " failure, ret is %d",
                                   pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                                   ret);
                }
            }
        
            ctclib_hash_release(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual);  
            HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
        }
    }
    
    return HAGT_E_NONE;
}

static void
_hagt_tpoam_update_timer_handler (void *p_data)
{
#ifdef _GLB_UML_SYSTEM_
    uint8 lchip_num = 0;
    uint8 lchip;

   ctc_get_local_chip_num(&lchip_num);
    for (lchip = 0; lchip < lchip_num; lchip ++)
    { 
        ex_cplusplus_cm_oam_update_ccm_timer(lchip, 2, 1050, 1);
        //cm_oam_scan_defect_timer(lchip, 1);
    }
#endif /* _GLB_UML_SYSTEM_ */

    g_pst_hagt_tpoam_master->pst_oam_timer = CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_update_timer_handler, NULL, CTC_OAM_OAM_CCM_SIM_INTERVAL);
    
    if (!g_pst_hagt_tpoam_master->pst_oam_timer)
    {
        HAGT_LOG_ERR("Update MPLS-TP OAM engine failed.\n");
        return;
    }

    return;
}

static int32 
_hagt_tpoam_build_lmep_key(HalMsgTpoamLMepKey_t* pst_key, ctc_oam_lmep_t* pst_lmep)
{
    /* 1. Channel */
    pst_lmep->key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    switch (pst_key->oamType)
    {
    case GLB_OAM_TYPE_MPLS_LSP:
    case GLB_OAM_TYPE_MPLS_PW:
        pst_lmep->key.u.tp.label = pst_key->id;
        break;
    case GLB_OAM_TYPE_MPLS_SECTION:
        pst_lmep->key.u.tp.gport_or_l3if_id = pst_key->id;
        GLB_SET_FLAG(pst_lmep->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);        
        break;
    default:
        HAGT_LOG_ERR("Invalid OAM type %ld when build LMEP Key.", pst_key->oamType);
        return HAGT_E_INVALID_PARAM;
    }

    /* 2. MAID */
    pst_lmep->maid.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    pst_lmep->maid.maid_len = pst_key->megid.size;
    sal_memcpy(pst_lmep->maid.maid, pst_key->megid.buf, pst_lmep->maid.maid_len);

    /* 3. MEP ID */    
    pst_lmep->u.y1731_lmep.mep_id = pst_key->mepid;
    return HAGT_E_NONE;
}

int32
hagt_tpoam_add_megid(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgTpoamMegidReq_t*     p_msg = NULL;
    ctc_oam_maid_t                   maid;

    sal_memset(&maid, 0, sizeof(maid));
    
    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamAddMegid;

    /* add maid */
    sal_memset(&maid, 0, sizeof(ctc_oam_maid_t));
    maid.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    maid.maid_len = p_msg->id.size;
    sal_memcpy(maid.maid, p_msg->id.buf, maid.maid_len);
 
    HAGT_IF_ERROR_RETURN(ctc_oam_add_maid(&maid));
    
    return HAGT_E_NONE;
}

int32
hagt_tpoam_del_megid(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgTpoamMegidReq_t*     p_msg = NULL;
    ctc_oam_maid_t                   maid;

    sal_memset(&maid, 0, sizeof(maid));
    
    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamAddMegid;

    /* add maid */
    sal_memset(&maid, 0, sizeof(ctc_oam_maid_t));
    maid.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    maid.maid_len = p_msg->id.size;
    sal_memcpy(maid.maid, p_msg->id.buf, maid.maid_len);
 
    HAGT_IF_ERROR_RETURN(ctc_oam_remove_maid(&maid));
    
    return HAGT_E_NONE;
}

int32
hagt_tpoam_add_oam_lmep_rmep(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgTpoamAddOamReq_t* p_msg = NULL;
    ctc_oam_lmep_t                   lmep;
    ctc_oam_lmep_t*                 pst_lmep = &lmep;
    ctc_oam_update_t                lmep_update;
    ctc_oam_update_t*              pst_lmep_update = &lmep_update;
    ctc_oam_rmep_t                  rmep;
    ctc_oam_rmep_t*                 pst_rmep = &rmep;
    ctc_oam_update_t                rmep_update;
    ctc_oam_update_t*              pst_rmep_update = &rmep_update;
    int32                                  rc = HAGT_E_NONE;
    
    sal_memset(&lmep, 0, sizeof(lmep));
    sal_memset(&rmep, 0, sizeof(rmep));
    
    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamAddOam;

    /* add local mep */
    HAGT_IF_ERROR_RETURN(_hagt_tpoam_build_lmep_key(&p_msg->key, pst_lmep));
    pst_lmep->u.y1731_lmep.nhid = p_msg->nhid;
    pst_lmep->u.y1731_lmep.ccm_interval = p_msg->interval;
    pst_lmep->u.y1731_lmep.mpls_ttl = p_msg->ttl;
    pst_lmep->u.y1731_lmep.tx_cos_exp = p_msg->exp;
    pst_lmep->lmep_index = p_msg->mepIndex;

    if (p_msg->withoutGal)
    {
        pst_lmep->u.y1731_lmep.flag |= CTC_OAM_Y1731_LMEP_FLAG_WITHOUT_GAL;
    }

    if (p_msg->isProtection)
    {
        pst_lmep->u.y1731_lmep.flag |= CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH;
    }

    HAGT_IF_ERROR_RETURN(ctc_oam_add_lmep(pst_lmep));

    sal_memcpy(&pst_lmep_update->key, &pst_lmep->key, sizeof(ctc_oam_key_t));

    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL;
    pst_lmep_update->update_value = p_msg->level;
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP;
    pst_lmep_update->update_value = p_msg->exp;
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    if (p_msg->isProtection)
    {
        pst_lmep_update->is_local = TRUE;
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP;
        pst_lmep_update->update_value = p_msg->nhid;
        rc = ctc_oam_update_lmep(pst_lmep_update);
        if (rc)
        {
            ctc_oam_remove_lmep(pst_lmep);
            return rc;
        }
    }

    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN;
    //pst_lmep_update->update_value = p_msg->dmEnable;
    pst_lmep_update->update_value = TRUE;
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    #if 1
    if (p_msg->lmEnable)
    {
        rc = _hagt_tpoam_set_lm_enable(pst_lmep_update, p_msg);
        if (rc)
        {
            _hagt_tpoam_rm_lm_dual(pst_lmep->lmep_index);
            ctc_oam_remove_lmep(pst_lmep);
            return rc;
        }
    }
    #else
    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
    pst_lmep_update->update_value = p_msg->lmEnable;
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }
    #endif

    /* add remote mep */
    sal_memcpy(&pst_rmep->key, &pst_lmep->key, sizeof(ctc_oam_key_t));

    pst_rmep->u.y1731_rmep.rmep_id = p_msg->rmepid;
    pst_rmep->rmep_index = p_msg->mepIndex + 1;
    rc = ctc_oam_add_rmep(pst_rmep);
    if (rc)
    {
        _hagt_tpoam_rm_lm_dual(pst_lmep->lmep_index);
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    sal_memcpy(&pst_rmep_update->key, &pst_lmep->key, sizeof(ctc_oam_key_t));
    pst_rmep_update->is_local = FALSE;
    pst_rmep_update->update_type = CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN;
    pst_rmep_update->update_value = p_msg->active;
    pst_rmep_update->rmep_id = p_msg->rmepid;
    
    rc = ctc_oam_update_rmep(pst_rmep_update);
    if (rc)
    {
        ctc_oam_remove_rmep(pst_rmep);
        _hagt_tpoam_rm_lm_dual(pst_lmep->lmep_index);
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN;
    pst_lmep_update->update_value = TRUE;
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_rmep(pst_rmep);
        _hagt_tpoam_rm_lm_dual(pst_lmep->lmep_index);
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    pst_lmep_update->is_local = TRUE;
    pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN;
    pst_lmep_update->update_value = p_msg->ccEnable;
    
    rc = ctc_oam_update_lmep(pst_lmep_update);
    if (rc)
    {
        ctc_oam_remove_rmep(pst_rmep);
        _hagt_tpoam_rm_lm_dual(pst_lmep->lmep_index);
        ctc_oam_remove_lmep(pst_lmep);
        return rc;
    }

    if (!g_pst_hagt_tpoam_master->pst_oam_timer && p_msg->firstMep)
    {
        _hagt_tpoam_update_timer_handler ((void *)g_pst_hagt_tpoam_master->pst_oam_timer);
    }
        
    return HAGT_E_NONE;
}

int32
hagt_tpoam_del_oam_lmep_rmep(Hsrv2AgtMsg_t* p_msg_data)
{
    ctc_oam_lmep_t lmep;
    ctc_oam_lmep_t* pst_lmep = &lmep;
    HalMsgTpoamDelOamReq_t* p_msg = NULL;
    ctc_oam_rmep_t rmep;
    ctc_oam_rmep_t* pst_rmep = &rmep;
    ctc_oam_update_t                lmep_update;
    ctc_oam_update_t*              pst_lmep_update = &lmep_update;
    
    sal_memset(&lmep, 0, sizeof(lmep));
    sal_memset(&rmep, 0, sizeof(rmep));
    
    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamDelOam;

    HAGT_IF_ERROR_RETURN(_hagt_tpoam_build_lmep_key(&p_msg->key, pst_lmep));

    sal_memcpy(&pst_lmep_update->key, &pst_lmep->key, sizeof(ctc_oam_key_t));
    _hagt_tpoam_set_lm_disable(pst_lmep_update, CTC_OAM_LM_TYPE_DUAL, p_msg->mepIndex);
    
    /* remove remote mep */
    sal_memcpy(&pst_rmep->key, &pst_lmep->key, sizeof(ctc_oam_key_t));
    pst_rmep->u.y1731_rmep.rmep_id = p_msg->rmepid;
    pst_rmep->rmep_index = p_msg->mepIndex + 1;
    ctc_oam_remove_rmep(pst_rmep);

    /* remove local mep */
    pst_lmep->lmep_index = p_msg->mepIndex;
    ctc_oam_remove_lmep(pst_lmep);

    if (g_pst_hagt_tpoam_master->pst_oam_timer && p_msg->lastMep)
    {
        CTC_TASK_DEL_TIME (g_pst_hagt_tpoam_master->pst_oam_timer);
    }
 
    return HAGT_E_NONE;
}

int32
hagt_tpoam_update_oam_lmep(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgTpoamAddOamReq_t* p_msg = NULL;
    ctc_oam_lmep_t lmep;
    ctc_oam_lmep_t* pst_lmep = &lmep;
    ctc_oam_update_t lmep_update;
    ctc_oam_update_t* pst_lmep_update = &lmep_update;

    sal_memset(&lmep, 0, sizeof(lmep));
    
    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamUpdateOam;

    HAGT_IF_ERROR_RETURN(_hagt_tpoam_build_lmep_key(&p_msg->key, pst_lmep));
   
    sal_memcpy(&pst_lmep_update->key, &pst_lmep->key, sizeof(ctc_oam_key_t));

    /*cmd enable cc*/
    if (FLAG_ISSET(p_msg->flag, GLB_TPOAM_UPDATE_CC_EN))
    {
        HAGT_TPOAM_DBG_COMMON("Set LMEP type %ld MEPID %ld cc enable %ld.", 
        p_msg->key.oamType, p_msg->key.mepid, p_msg->ccEnable);
    
        pst_lmep_update->is_local = TRUE;
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN;
        pst_lmep_update->update_value = p_msg->ccEnable;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }

    /*event set rdi*/
    if (FLAG_ISSET(p_msg->flag, GLB_TPOAM_UPDATE_RDI))
    {
        HAGT_TPOAM_DBG_COMMON("Set LMEP type %ld MEPID %ld set RDI %ld.", 
            p_msg->key.oamType, p_msg->key.mepid, p_msg->rdiEnable);
    
        pst_lmep_update->is_local = TRUE;
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI;
        pst_lmep_update->update_value = p_msg->rdiEnable;
        /*fix bug 47553 by chenc*/
        //HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
        ctc_oam_update_lmep(pst_lmep_update);
    }

    /*lm enable*/
    if (FLAG_ISSET(p_msg->flag, GLB_TPOAM_UPDATE_LM_EN))
    {
        _hagt_tpoam_set_lm_enable(pst_lmep_update, p_msg);
    }
    
    if (FLAG_ISSET(p_msg->flag, GLB_TPOAM_UPDATE_TX_COS_EXP))
    {
        HAGT_TPOAM_DBG_COMMON("Set LMEP type %ld MEPID %ld exp value %ld.", 
            p_msg->key.oamType, p_msg->key.mepid, p_msg->exp);
    
        pst_lmep_update->is_local = TRUE;
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP;
        pst_lmep_update->update_value = p_msg->exp;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }

    if (FLAG_ISSET(p_msg->flag, GLB_TPOAM_UPDATE_NHOP))
    {
        HAGT_TPOAM_DBG_COMMON("Set LMEP type %ld MEPID %ld nhid %ld.", 
            p_msg->key.oamType, p_msg->key.mepid, p_msg->nhid);
    
        pst_lmep_update->is_local = TRUE;
        pst_lmep_update->update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP;
        pst_lmep_update->update_value = p_msg->nhid;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(pst_lmep_update));
    }

    return HAGT_E_NONE;
}


int32
hagt_tpoam_add_oam_mip(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgEthOamCfmMipReq_t* p_msg = NULL;
    ctc_oam_mip_t mip;

    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgEthOamCfmAddMip;

    HAGT_TPOAM_DBG_COMMON("Add MIP for label %ld %ld.", p_msg->vlanid, p_msg->glPort); 

    sal_memset(&mip, 0, sizeof(mip));
    mip.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    mip.key.u.tp.label = p_msg->vlanid;

    HAGT_IF_ERROR_RETURN(ctc_oam_add_mip(&mip));

    sal_memset(&mip, 0, sizeof(mip));
    mip.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    mip.key.u.tp.label = p_msg->glPort;

    HAGT_IF_ERROR_RETURN(ctc_oam_add_mip(&mip));
    
    return HAGT_E_NONE;
}

int32
hagt_tpoam_del_oam_mip(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgEthOamCfmMipReq_t* p_msg = NULL;
    ctc_oam_mip_t mip;

    p_msg = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgEthOamCfmAddMip;

    HAGT_TPOAM_DBG_COMMON("Del MIP for label %ld %ld.", p_msg->vlanid, p_msg->glPort); 

    sal_memset(&mip, 0, sizeof(mip));
    mip.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    mip.key.u.tp.label = p_msg->vlanid;

    HAGT_IF_ERROR_RETURN(ctc_oam_remove_mip(&mip));

    sal_memset(&mip, 0, sizeof(mip));
    mip.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    mip.key.u.tp.label = p_msg->glPort;

    HAGT_IF_ERROR_RETURN(ctc_oam_remove_mip(&mip));
    
    return HAGT_E_NONE;
}


#ifndef tpoam_event
static int32 
_hagt_tpoam_send_event(uint32 count, glb_tpoam_event_entry_t* pst_events)
{
    uint16 slot_number = 0;
    Hagt2SrvMsg_t msg;
    int32 ret = HAGT_E_NONE;
    
    sal_memset(&msg, 0, sizeof(msg));

    /* Send message to HSRV, report OAM event */
    msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgTpoamEvent;
    msg.opHagt2Srv.hagt2SrvMsgTpoamEvent.count = count;
    msg.opHagt2Srv.hagt2SrvMsgTpoamEvent.rawData.buf = (uint8*)pst_events;
    msg.opHagt2Srv.hagt2SrvMsgTpoamEvent.rawData.size = sizeof(glb_tpoam_event_entry_t)*count;
    HAGT_IF_ERROR_RETURN(hagt_get_phy_slotid(&slot_number));  /*Get line card slot number*/
    GLB_SET_BIT(msg.agentId, slot_number);
    
    ret = HAGT2SRV_MSG_SEND(&msg);
    if (ret < 0)
    {
        return ret;
    }

    return HAGT_E_NONE;
}

/*ctc event convert to tpoam event*/
static int32
_hagt_tpoam_map_oam_event(uint8 gchip, ctc_oam_event_entry_t* pst_src_event, glb_tpoam_event_entry_t* pst_dst_event)
{
    ctc_oam_mep_info_t mep_info;
    ctc_oam_mep_info_t* p_mep_info = &mep_info;
    uint8 lchip = hagt_chip_glb2local(gchip);
    int32 ret;

    sal_memset(pst_dst_event, 0, sizeof(glb_tpoam_event_entry_t));
    sal_memset(p_mep_info, 0 , sizeof(ctc_oam_mep_info_t));

    /* set default wait first cc flag*/
    GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC);
    HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_WAIT_1ST_CC\n");
    
    pst_dst_event->lmep_index = pst_src_event->lmep_index;
    /* status on LMEP */
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_UNEXPECTED_MEP))
    {
        GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_UNEXP_MEP);
        HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_UNEXP_MEP\n");
    }
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_MISMERGE))
    {
        GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_MISMERGE);
        HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_MISMERGE\n");
    }
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_UNEXPECTED_LEVEL))
    {
        GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_MEG_LVL);
        HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_MEG_LVL\n");
    }
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_EVENT_RDI_TX))
    {
        GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_RDI_TX);
        HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_RDI_TX\n");
    }
    
    if (pst_src_event->is_remote)
    {
        pst_dst_event->rmep_index = pst_src_event->rmep_index;
        /* status on RMEP */
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT))
        {
            GLB_UNSET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC);/*same with GLB_TPOAM_EVT_WAIT_1ST_CC*/
            HAGT_TPOAM_DBG_COMMON("mapping oam event unset GLB_TPOAM_EVT_WAIT_1ST_CC\n");
        }
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_DLOC))
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_LOC);
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_LOC\n");
        }
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_UNEXPECTED_PERIOD))
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_UNEXP_PERIOD);
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_UNEXP_PERIOD\n");
        }
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_EVENT_RDI_RX))
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_RDI_RX);
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_RDI_RX\n");
        }
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_CSF))
        {
            //GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_CSF_RX);
        }
    }
    else
    {
        /* need to get remote mep status */
        p_mep_info->mep_index = pst_src_event->lmep_index + 1;
        ret = ctc_oam_get_mep_info(lchip, p_mep_info);
        if (CTC_E_NONE != ret)
        {
            /* rmep is not configured */
            return HAGT_E_NONE;
        }

        if (!p_mep_info->rmep.y1731_rmep.active)
        {
            /* rmep is not configured */
            return HAGT_E_NONE;
        }

        pst_dst_event->rmep_index = pst_src_event->lmep_index + 1;

        if (p_mep_info->rmep.y1731_rmep.first_pkt_rx)
        {
            GLB_UNSET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_WAIT_1ST_CC);/*same with is_remote->GLB_TPOAM_EVT_WAIT_1ST_CC*/
            HAGT_TPOAM_DBG_COMMON("mapping oam event unset GLB_TPOAM_EVT_WAIT_1ST_CC\n");
        }
        if (p_mep_info->rmep.y1731_rmep.d_loc)
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_LOC);/*same with is_remote->GLB_TPOAM_EVT_LOC*/
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_LOC\n");
        }
        
        if (p_mep_info->rmep.y1731_rmep.d_unexp_period)
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_UNEXP_PERIOD);/*same with is_remote->GLB_TPOAM_EVT_UNEXP_PERIOD*/
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_UNEXP_PERIOD\n");
        }

        if (p_mep_info->rmep.y1731_rmep.last_rdi)
        {
            GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_RDI_RX);/*same with is_remote->GLB_TPOAM_EVT_RDI_RX*/
            HAGT_TPOAM_DBG_COMMON("mapping oam event set GLB_TPOAM_EVT_RDI_RX\n");
        }
        if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_CSF))/*same with is_remote->CTC_OAM_DEFECT_CSF*/
        {
            //GLB_SET_FLAG(pst_dst_event->event_bmp, GLB_TPOAM_EVT_CSF_RX);
        }
    }

    /*todo*/
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_NONE))
    {
        HAGT_TPOAM_DBG_COMMON("unmapping oam event CTC_OAM_DEFECT_NONE\n");
    }
    
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_MAC_STATUS_CHANGE))
    {
        HAGT_TPOAM_DBG_COMMON("unmapping oam event CTC_OAM_DEFECT_MAC_STATUS_CHANGE\n");
    }
    
    if (GLB_FLAG_ISSET(pst_src_event->event_bmp, CTC_OAM_DEFECT_SRC_MAC_MISMATCH))
    {
        HAGT_TPOAM_DBG_COMMON("unmapping oam event CTC_OAM_DEFECT_SRC_MAC_MISMATCH\n");
    }

    //if (0)
    {
        //follow not mapping
        //GLB_TPOAM_EVT_NONE
        //SW etc
    }
        
    return HAGT_E_NONE;
}

/*sys_goldengate_isr_oam_process_isr*/
int32                                                                                                                                          
hagt_tpoam_isr_callback(uint8 gchip, void* p_data)                                                                                             
{                                                                                                                                              
    glb_tpoam_event_entry_t* p_glb_events = NULL;                                                                                              
    glb_tpoam_event_entry_t* p_tmp_events = NULL;                                                                                              
    ctc_oam_event_t* p_event = (ctc_oam_event_t*)p_data;                                                                                       
    uint32 index = 0;      
    uint32 cache_index = 0;    
    uint32 cache_total = 0;                                                                                                                    
    uint32 i = 0;                                                                                                                              
    
    if (!p_data)                                                                                                                               
    {                                                                                                                                          
        return HAGT_E_NONE;                                                                                                                    
    }
                                                                                                                                               
    p_glb_events = (glb_tpoam_event_entry_t*)HAGT_CALLOC(CTCLIB_MEM_TPOAM_MODULE, CTC_OAM_EVENT_MAX_ENTRY_NUM*sizeof(glb_tpoam_event_entry_t));
    if (!p_glb_events)                                                                                                                         
    {                                                                                                                                          
        return HAGT_E_NO_MEMORY;                                                                                                               
    }                                                                                                                                          
                                                                                                                                               
    for (i = 0; i < p_event->valid_entry_num; i++)                                                                                             
    {
        if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 != p_event->oam_event_entry[i].mep_type)                                                            
        {                                                                                                                                      
            continue;                                                                                                                          
        }                                                                                                                                      
                                                                                                                                               
        p_tmp_events = p_glb_events + index;                                                                                             
        _hagt_tpoam_map_oam_event(gchip, &p_event->oam_event_entry[i], p_tmp_events);                                                          
                                                                                                                                               
        if (g_pst_hagt_tpoam_master->fast_aps_enable)  
        {
            uint32  fast_aps_en = 0;
            uint32 aps_group_offset = 0;
            uint32 aps_group_id = 0;
            uint16 aps_group_offset_id = 0;
            uint32 protection_en = FALSE;
            int32  rc = HAGT_E_NONE;
            
#ifndef TTPPRODUCT
            hsrv_tpoam_get_aps_group_fast_aps_en(p_tmp_events->lmep_index, p_tmp_events->event_bmp, &fast_aps_en, &aps_group_id, &aps_group_offset, &protection_en);
#endif 
            if (fast_aps_en)
            {
                aps_group_offset_id = aps_group_offset;
                rc = ctc_aps_set_aps_bridge(aps_group_offset_id, protection_en);
                if (rc)
                {
                    HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", rc, __FUNCTION__, __LINE__);
                }
                
#ifndef TTPPRODUCT
                hsrv_tpoam_update_pw_mep_nh_by_groupid(aps_group_id);
#endif

                /*modified by yejl for bug 47179, 2018-05-15*/ 
                /*modified by yejl for bug 47303, 2018-05-21*/
                HAGT_TPOAM_DBG_COMMON("APS group id %u fast change to %s link", aps_group_id, (protection_en == 1 ? "protection" : "working"));
            }
        }
                                                                                                                                               
        HAGT_TPOAM_DBG_COMMON("ISR mep_index %d, rmep_index %d, bmp %x %s.",                                                                      
                  p_tmp_events->lmep_index, p_tmp_events->rmep_index, p_tmp_events->event_bmp, tpoam_api_event_ext_desc(p_tmp_events->event_bmp));                                                
                                                                                                                                               
        cache_index++;
        index++;
        if (CTC_OAM_EVENT_MAX_ENTRY_NUM <= cache_index)
        {
            HAGT_TPOAM_DBG_COMMON("cache_index more than CTC_OAM_EVENT_MAX_ENTRY_NUM 512\n");
            cache_total = cache_index;
            for (cache_index = 0; (cache_index + GLB_TPOAM_EVENT_CACHE_SIZE) <= cache_total; cache_index += GLB_TPOAM_EVENT_CACHE_SIZE)                
            {                                                                                                                                          
                _hagt_tpoam_send_event(GLB_TPOAM_EVENT_CACHE_SIZE, p_glb_events + cache_index);                                                        
            }
            cache_index = 0;
        }
    }                                                                                                                                          
                                                                                                                                               
    cache_total = cache_index;                                                                                                                 
                                                                                                                                               
    for (cache_index = 0; (cache_index + GLB_TPOAM_EVENT_CACHE_SIZE) <= cache_total; cache_index += GLB_TPOAM_EVENT_CACHE_SIZE)                
    {                                                                                                                                          
        _hagt_tpoam_send_event(GLB_TPOAM_EVENT_CACHE_SIZE, p_glb_events + cache_index);                                                        
    }                                                                                                                                          
                                                                                                                                               
    if (cache_total > cache_index)                                                                                                             
    {                                                                                                                                          
        _hagt_tpoam_send_event((cache_total - cache_index), p_glb_events + cache_index);                                                       
    }                                                                                                                                          
                                                                                                                                               
    HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, p_glb_events);                                                                                          
    return HAGT_E_NONE;                                                                                                                        
}         
#endif

#ifndef lm_dual
static int32 
_hagt_tpoam_lm_dual_report(int32 count, glb_tpoam_lm_dual_counter_t* pst_lm_dual_counter)
{
    uint16 slot_number = 0;
    Hagt2SrvMsg_t msg;

    HAGT_CHECK_PTR(pst_lm_dual_counter);
    
    sal_memset(&msg, 0, sizeof(msg));

    /*Send msg to hal server to report tpoam lm */    
    msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgEthOamCfmLmDualReq;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmDualReq.count = count;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmDualReq.rawData.buf = (uint8*)pst_lm_dual_counter;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmDualReq.rawData.size = sizeof(pst_lm_dual_counter) * count;
    HAGT_IF_ERROR_RETURN(hagt_get_phy_slotid(&slot_number));  /*Get line card slot number*/
    GLB_SET_BIT(msg.agentId, slot_number);
    
    return HAGT2SRV_MSG_SEND(&msg); 
}

void _hagt_tpoam_update_lm_dual_status(void)
{
    uint16 index = 0;
    ctclib_hash_backet_t* pst_hb = NULL;
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual = NULL;
    glb_tpoam_lm_dual_counter_cache_t lm_dual_counter;
    ctc_oam_stats_info_t lm_stat_info;
    uint16  lm_dual_num = 0;
    int32   ret = HAGT_E_NONE;

    sal_memset(&lm_dual_counter, 0, sizeof(glb_tpoam_lm_dual_counter_cache_t));
    
    /*enable lm oam session count is 0, means not oam session enable dual lm*/
    if (0 == g_pst_hagt_tpoam_master->pst_lm_dual_hash->count)
    {
        return;
    }
    
    for (index = g_pst_hagt_tpoam_master->lm_dual_hash_index; index < g_pst_hagt_tpoam_master->pst_lm_dual_hash->size; index++)
    {   
        for (pst_hb = g_pst_hagt_tpoam_master->pst_lm_dual_hash->index[index]; pst_hb; pst_hb = pst_hb->next)
        {
            pst_lmep_lm_dual = pst_hb->data;

            pst_lmep_lm_dual->timer_counter--;
            if (0 == pst_lmep_lm_dual->timer_counter)
            {
                /* update counter*/
                pst_lmep_lm_dual->timer_counter = pst_lmep_lm_dual->stats_interval;
                
                /* read lm dual counters */
                sal_memset(&lm_stat_info, 0, sizeof(ctc_oam_stats_info_t));

                lm_stat_info.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

                switch (pst_lmep_lm_dual->oam_type)
                {
                    case GLB_OAM_TYPE_MPLS_LSP:
                    case GLB_OAM_TYPE_MPLS_PW:
                        lm_stat_info.key.u.tp.label = pst_lmep_lm_dual->key_id;
                        break;
                    case GLB_OAM_TYPE_MPLS_SECTION:
                        lm_stat_info.key.u.tp.gport_or_l3if_id = pst_lmep_lm_dual->key_id;
                        GLB_SET_FLAG(lm_stat_info.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);        
                        break;
                    default:
                        HAGT_LOG_ERR("Invalid OAM type %hhu when build LMEP Key.", pst_lmep_lm_dual->oam_type);
                        return;
                }
                
                lm_stat_info.lm_type = pst_lmep_lm_dual->lm_type;
                lm_stat_info.lm_cos_type = pst_lmep_lm_dual->lm_cos_type;
        
                ret = ctc_oam_get_stats(&lm_stat_info);
                if (ret)
                {
                    HAGT_LOG_ERR(" Update dual lm failure for oam_type %d key_id %d,"
                                   " ret is %d",
                                   pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, ret);
                }
                
                /* reserve lm dual counters */
                if (CTC_OAM_LM_COS_TYPE_PER_COS != pst_lmep_lm_dual->lm_cos_type)
                {
                    lm_dual_counter.entry[lm_dual_counter.num].oam_type = pst_lmep_lm_dual->oam_type;
                    lm_dual_counter.entry[lm_dual_counter.num].key_id = pst_lmep_lm_dual->key_id;
                    lm_dual_counter.entry[lm_dual_counter.num].mep_index = pst_lmep_lm_dual->mep_index;
                    lm_dual_counter.entry[lm_dual_counter.num].ccm_cos_value = pst_lmep_lm_dual->lm_cos_value;
                    lm_dual_counter.entry[lm_dual_counter.num].TxFCf = lm_stat_info.lm_info[0].tx_fcf; 
                    lm_dual_counter.entry[lm_dual_counter.num].RxFCb = lm_stat_info.lm_info[0].rx_fcb;
                    lm_dual_counter.entry[lm_dual_counter.num].TxFCb = lm_stat_info.lm_info[0].tx_fcb;
                    lm_dual_counter.entry[lm_dual_counter.num].RxFCl = lm_stat_info.lm_info[0].rx_fcl;
                    HAGT_TPOAM_DBG_COMMON("LMEP index %d cos %d TxFCf %u RxFCb %u TxFCb %u RxFCl %u.", 
                        pst_lmep_lm_dual->mep_index, pst_lmep_lm_dual->lm_cos_value,
                        lm_stat_info.lm_info[0].tx_fcf, lm_stat_info.lm_info[0].rx_fcb, lm_stat_info.lm_info[0].tx_fcb, lm_stat_info.lm_info[0].rx_fcl);
                }
                else
                {
                    lm_dual_counter.entry[lm_dual_counter.num].oam_type = pst_lmep_lm_dual->oam_type;
                    lm_dual_counter.entry[lm_dual_counter.num].key_id = pst_lmep_lm_dual->key_id;
                    lm_dual_counter.entry[lm_dual_counter.num].mep_index = pst_lmep_lm_dual->mep_index;
                    lm_dual_counter.entry[lm_dual_counter.num].ccm_cos_value = pst_lmep_lm_dual->lm_cos_value;
                    lm_dual_counter.entry[lm_dual_counter.num].TxFCf 
                        = lm_stat_info.lm_info[pst_lmep_lm_dual->lm_cos_value].tx_fcf; 
                    lm_dual_counter.entry[lm_dual_counter.num].RxFCb 
                        = lm_stat_info.lm_info[pst_lmep_lm_dual->lm_cos_value].rx_fcb;
                    lm_dual_counter.entry[lm_dual_counter.num].TxFCb 
                        = lm_stat_info.lm_info[pst_lmep_lm_dual->lm_cos_value].tx_fcb;
                    lm_dual_counter.entry[lm_dual_counter.num].RxFCl 
                        = lm_stat_info.lm_info[pst_lmep_lm_dual->lm_cos_value].rx_fcl;
                }
                lm_dual_counter.num++;
                

                /* if cache num is no less than 10, send to onmd */
                if (GLB_TPOAM_MAX_LM_DUAL_COUNTER_CACHE_NUM == lm_dual_counter.num)
                {
                    ret = _hagt_tpoam_lm_dual_report(lm_dual_counter.num, lm_dual_counter.entry);
                    if (ret)
                    {
                        HAGT_LOG_ERR("Report lm dual conters failure, ret is %d\n", ret); 
                    }
                    lm_dual_counter.num = 0;
                }

                /* change ccm cos for lm dual per cos */          
                if (CTC_OAM_LM_COS_TYPE_PER_COS == pst_lmep_lm_dual->lm_cos_type)
                {
                    pst_lmep_lm_dual->lm_cos_value++; 
                    if (pst_lmep_lm_dual->lm_cos_value >= CTC_MAX_COS)
                    {
                        pst_lmep_lm_dual->lm_cos_value = CTC_MIN_COS;
                    }

                    ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
                    if (ret)
                    {
                        HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam_type %d key_id %d,"
                                       " ret is %d",
                                       pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, ret);
                    }
                }
            }

            lm_dual_num++;
            g_pst_hagt_tpoam_master->lm_dual_hash_total_update_count++;
        }

        /* every time, update 1/10 nodes, 
           but for the last time, all left nodes should be updated */
        if ((lm_dual_num > g_pst_hagt_tpoam_master->lm_dual_hash_count_per_time)
            && (g_pst_hagt_tpoam_master->lm_dual_time < HAGT_TPOAM_LM_DUAL_UPDATE_TIME - 1))
        {
            break;
        }

        /* if all valid nodes are updated, don't loop again */
        if (g_pst_hagt_tpoam_master->lm_dual_hash_total_update_count >= g_pst_hagt_tpoam_master->pst_lm_dual_hash->count)
        {
            break;
        }  
    }
    
    if (lm_dual_counter.num)
    {
        ret = _hagt_tpoam_lm_dual_report(lm_dual_counter.num, lm_dual_counter.entry);
        if (ret)
        {
            HAGT_LOG_ERR("Report lm dual conters failure, ret is %d\n", ret); 
        }
        lm_dual_counter.num = 0;
    }   

    /* update from next index for next time */
    g_pst_hagt_tpoam_master->lm_dual_hash_index = index + 1;

    /* if update time is 10, start from beginning next time */
    g_pst_hagt_tpoam_master->lm_dual_time++;
    if (g_pst_hagt_tpoam_master->lm_dual_time >= HAGT_TPOAM_LM_DUAL_UPDATE_TIME)
    {
        g_pst_hagt_tpoam_master->lm_dual_time = 0;
        g_pst_hagt_tpoam_master->lm_dual_hash_index = 0;
        g_pst_hagt_tpoam_master->lm_dual_hash_total_update_count = 0;
    }

    HAGT_TPOAM_DBG_COMMON("Lm Dual update time is %d, hash index is %d", 
        g_pst_hagt_tpoam_master->lm_dual_time, g_pst_hagt_tpoam_master->lm_dual_hash_index);  

    return;
}

static void
_hagt_tpoam_update_lm_timer_handler (void *p_data)
{
    _hagt_tpoam_update_lm_dual_status();

    g_pst_hagt_tpoam_master->pst_lm_timer = 
        CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_update_lm_timer_handler, NULL, CTC_OAM_OAM_DUAL_LM_INTERVAL);
    if (!g_pst_hagt_tpoam_master->pst_lm_timer)
    {
        HAGT_LOG_ERR("Update MPLS-TP OAM LM timer failed.\n");
        return;
    }

    return;
}

static int32 
_hagt_tpoam_set_lm_enable_(Hsrv2AgtMsg_t* pst_hsrv_msg)
{    
    ctc_oam_update_t lmep_update;
    ctc_oam_update_t* pst_lmep_update = &lmep_update;
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual = NULL;
    hagt_tpoam_lm_dual_t  lm_dual;
    void* p_data1 = NULL;
    int32   ret = HAGT_E_NONE;

    HAGT_CHECK_PTR(pst_hsrv_msg); 

    HAGT_TPOAM_DBG_COMMON("Enter into %s.", __FUNCTION__);

    sal_memset(&lmep_update, 0, sizeof(lmep_update));
    sal_memset(&lm_dual, 0, sizeof(hagt_tpoam_lm_dual_t));

    lm_dual.oam_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.oamType;
    lm_dual.key_id = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.keyId;
    lm_dual.mep_index = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.mepIndex;
    lm_dual.lm_enable = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.enable;
    lm_dual.lm_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.lmType;
    lm_dual.lm_cos_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.cosType;
    lm_dual.lm_cos_value = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.cosValue;
    lm_dual.stats_interval = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmEnable.statsInterval;
    lm_dual.timer_counter = lm_dual.stats_interval;

    /* set local mep parameters */
    pst_lmep_update->key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
    switch (lm_dual.oam_type)
    {
        case GLB_OAM_TYPE_MPLS_LSP:
        case GLB_OAM_TYPE_MPLS_PW:
            pst_lmep_update->key.u.tp.label = lm_dual.key_id;
            break;
        case GLB_OAM_TYPE_MPLS_SECTION:
            pst_lmep_update->key.u.tp.gport_or_l3if_id = lm_dual.key_id;
            GLB_SET_FLAG(pst_lmep_update->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);        
            break;
        default:
            HAGT_LOG_ERR("Invalid OAM type %hhu when build LMEP Key.", lm_dual.oam_type);
            return HAGT_E_INVALID_PARAM;
    }

    lmep_update.is_local = TRUE;
        
    if (lm_dual.lm_enable)
    {
        /* set lm type */
        lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE;
        lmep_update.update_value = lm_dual.lm_type;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));

        /* set lm cos type */
        lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE;
        lmep_update.update_value = lm_dual.lm_cos_type;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));

        /* set lm cos */
        if (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == lm_dual.lm_cos_type)
        {
            lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS;
            lmep_update.update_value = lm_dual.lm_cos_value;
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
        }

        /* set lm enable */
        lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
        lmep_update.update_value = lm_dual.lm_enable;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
    }
    else
    {        
        /* set lm disable */
        lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
        lmep_update.update_value = lm_dual.lm_enable;
        HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
    }

    /* for single lm, don't reserve lm information */
    if (CTC_OAM_LM_TYPE_DUAL != lm_dual.lm_type)
    {
        return HAGT_E_NONE;
    }
    
    if (lm_dual.lm_enable)
    {
        /* reserve dual lm information */
        
        /* create and reserve dual lm node */
        pst_lmep_lm_dual = HAGT_CALLOC(CTCLIB_MEM_TPOAM_MODULE, sizeof(hagt_tpoam_lm_dual_t));
        if (!pst_lmep_lm_dual)
        {
            /* set lm disable */
            lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
            lmep_update.update_value = lm_dual.lm_enable;      
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
            return HAGT_E_NO_MEMORY;
        }

        sal_memcpy(pst_lmep_lm_dual, &lm_dual, sizeof(hagt_tpoam_lm_dual_t));
        
        p_data1 = ctclib_hash_get(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual, 
                                     ctclib_hash_alloc_intern);
        if (!p_data1)
        {
            /* set lm disable */
            lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN;
            lmep_update.update_value = lm_dual.lm_enable;
            HAGT_IF_ERROR_RETURN(ctc_oam_update_lmep(&lmep_update));
            
            HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
            return HAGT_E_NO_MEMORY;
        }

        /* update ccm cos to cos value when dual lm of per-cos mode is disabled */
        if (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type)    
        {
            ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
            if (ret)
            {
                HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam_type %d key_id_id %d mep_index %d"
                               " failure, ret is %d",
                               pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                               ret);
            }
        }

        if (g_pst_hagt_tpoam_master->pst_lm_dual_hash->count > HAGT_TPOAM_LM_DUAL_UPDATE_TIME)
        {
            g_pst_hagt_tpoam_master->lm_dual_hash_count_per_time 
                = g_pst_hagt_tpoam_master->pst_lm_dual_hash->count / HAGT_TPOAM_LM_DUAL_UPDATE_TIME;
        }
        else
        {
            g_pst_hagt_tpoam_master->lm_dual_hash_count_per_time = HAGT_TPOAM_LM_DUAL_UPDATE_PER_TIME_MIN;
        }
    }
    else
    {
        /* delete dual lm information */
        pst_lmep_lm_dual = ctclib_hash_lookup(g_pst_hagt_tpoam_master->pst_lm_dual_hash, &lm_dual);
        if (pst_lmep_lm_dual)
        {
            /* update ccm cos to max when dual lm of per-cos mode is disabled */
            if ((CTC_OAM_LM_COS_TYPE_PER_COS == pst_lmep_lm_dual->lm_cos_type)
                || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == pst_lmep_lm_dual->lm_cos_type))
            {
                pst_lmep_lm_dual->lm_cos_value = CTC_MAX_COS - 1;
                
                ret = _hagt_tpoam_update_ccm_cos_by_dual_lm(pst_lmep_lm_dual);
                if (ret)
                {
                    HAGT_LOG_ERR(" Update ccm cos by  dual lm failure for oam-type %d key_id %d mep_index %d"
                                   " failure, ret is %d",
                                   pst_lmep_lm_dual->oam_type, pst_lmep_lm_dual->key_id, pst_lmep_lm_dual->mep_index,
                                   ret);
                }
            }
        
            ctclib_hash_release(g_pst_hagt_tpoam_master->pst_lm_dual_hash, (void*)pst_lmep_lm_dual);  
            HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_dual);
        }
    }
    
    return HAGT_E_NONE;
}

static inline uint32
_hagt_tpoam_lm_dual_hash_key (hagt_tpoam_lm_dual_t* pst_lmep_lm_dual)
{
    uint32 key = 0;
    
    HAGT_CHECK_PTR(pst_lmep_lm_dual);

    key = pst_lmep_lm_dual->mep_index;

    return key;
}

static bool
_hagt_tpoam_lm_dual_hash_cmp(void* pva, void* pvb)
{
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual_a = NULL;
    hagt_tpoam_lm_dual_t* pst_lmep_lm_dual_b = NULL;

    HAGT_CHECK_PTR(pva);
    HAGT_CHECK_PTR(pvb);

    pst_lmep_lm_dual_a = (hagt_tpoam_lm_dual_t*)pva;
    pst_lmep_lm_dual_b = (hagt_tpoam_lm_dual_t*)pvb;

    if (pst_lmep_lm_dual_a->mep_index == pst_lmep_lm_dual_b->mep_index)
    {
        return TRUE;
    }

    return FALSE;
}

#endif

#ifndef lm_sd
static int32 
_hagt_tpoam_lm_sd_report(int32 count, glb_tpoam_lm_sd_t* pst_lm_sd)
{
    uint16 slot_number = 0;
    Hagt2SrvMsg_t msg;

    HAGT_CHECK_PTR(pst_lm_sd);
    
    sal_memset(&msg, 0, sizeof(msg));

    /*Send msg to hal server to report tpoam lm */    
    msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgEthOamCfmLmSDReq;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmSDReq.count = count;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmSDReq.rawData.buf = (uint8*)pst_lm_sd;
    msg.opHagt2Srv.hagt2SrvMsgEthOamCfmLmSDReq.rawData.size = sizeof(pst_lm_sd) * count;
    HAGT_IF_ERROR_RETURN(hagt_get_phy_slotid(&slot_number));  /*Get line card slot number*/
    GLB_SET_BIT(msg.agentId, slot_number);
    
    return HAGT2SRV_MSG_SEND(&msg); 
}

u_int32_t hagt_tpoam_lm_calculate(u_int32_t new_counter, u_int32_t old_counter)
{
    u_int32_t counter_diff = 0 ;

    if (new_counter >= old_counter)
    {
        counter_diff = new_counter - old_counter;
    }
    /* if old counter > new counter, the counter should be greater than u_int32_t */
    else
    {
        counter_diff = 0xFFFFFFFF - old_counter + new_counter + 1;
    }

    return counter_diff;
}

bool hagt_tpoam_check_lm_loss_ratio(hagt_tpoam_lm_sd_t* pst_lmep_lm_sd, ctc_oam_stats_info_t* pst_lm_stat_info, uint8 cos)
{
    uint32  TxFCf_diff = 0;
    uint32  TxFCb_diff = 0;
    uint32  RxFCb_diff = 0;
    uint32  RxFCl_diff = 0;
    uint64  frame_loss = 0;
    uint32  lm_far_end_frame_loss = 0; 
    uint32  lm_far_end_frame_loss_ratio = 0; 
    uint32  lm_near_end_frame_loss = 0;
    uint32  lm_near_end_frame_loss_ratio = 0;

    TxFCf_diff = hagt_tpoam_lm_calculate(pst_lmep_lm_sd->lm_info[cos].tx_fcf, pst_lm_stat_info->lm_info[cos].tx_fcf);
    TxFCb_diff = hagt_tpoam_lm_calculate(pst_lmep_lm_sd->lm_info[cos].tx_fcb, pst_lm_stat_info->lm_info[cos].tx_fcb);
    RxFCb_diff = hagt_tpoam_lm_calculate(pst_lmep_lm_sd->lm_info[cos].rx_fcb, pst_lm_stat_info->lm_info[cos].rx_fcb);
    RxFCl_diff = hagt_tpoam_lm_calculate(pst_lmep_lm_sd->lm_info[cos].rx_fcl, pst_lm_stat_info->lm_info[cos].rx_fcl);
    
    /* Frame Loss (far-end) = |TxFCb[tc] - TxFCb[tp]| - |RxFCb[tc] - RxFCb[tp]| */
    if (TxFCb_diff >= RxFCb_diff)
    {
        lm_far_end_frame_loss = TxFCb_diff - RxFCb_diff;
    }
    else
    {
        lm_far_end_frame_loss = RxFCb_diff - TxFCb_diff;    
    }

    frame_loss = lm_far_end_frame_loss * 1000;
    if (TxFCb_diff)
    {
        lm_far_end_frame_loss_ratio = frame_loss / TxFCb_diff;
    }

    if ((lm_far_end_frame_loss_ratio >= pst_lmep_lm_sd->ratio_limit)
        && (pst_lmep_lm_sd->lm_far_end_frame_loss_ratio[cos] < pst_lmep_lm_sd->ratio_limit))
    {
        pst_lmep_lm_sd->lm_far_end_frame_loss_ratio[cos] = lm_far_end_frame_loss_ratio;
        return TRUE;
    }
    pst_lmep_lm_sd->lm_far_end_frame_loss_ratio[cos] = lm_far_end_frame_loss_ratio;

    /* Frame Loss (near-end) = |TxFCf[tc] - TxFCf[tp]| - |RxFCl[tc] - RxFCl[tp]| */
    if (TxFCf_diff >= RxFCl_diff)
    {
        lm_near_end_frame_loss = TxFCf_diff - RxFCl_diff;
    }
    else
    {
        lm_near_end_frame_loss = RxFCl_diff - TxFCf_diff;    
    }

    frame_loss = lm_near_end_frame_loss * 1000;

    if (TxFCf_diff)
    {
        lm_near_end_frame_loss_ratio = frame_loss / TxFCf_diff;
    }

    if ((lm_near_end_frame_loss_ratio >= pst_lmep_lm_sd->ratio_limit)
        && (pst_lmep_lm_sd->lm_near_end_frame_loss_ratio[cos] < pst_lmep_lm_sd->ratio_limit))
    {
        pst_lmep_lm_sd->lm_near_end_frame_loss_ratio[cos] = lm_near_end_frame_loss_ratio;
        return TRUE;
    }

    pst_lmep_lm_sd->lm_near_end_frame_loss_ratio[cos] = lm_near_end_frame_loss_ratio;

    return FALSE;
}

void _hagt_tpoam_update_lm_sd_status(void)
{
    uint16 index = 0;
    ctclib_hash_backet_t* pst_hb = NULL;
    hagt_tpoam_lm_sd_t* pst_lmep_lm_sd = NULL;
    ctc_oam_stats_info_t lm_stat_info;
    int32   ret = HAGT_E_NONE;
    bool    loss_ratio_exceed_limit = FALSE;
    glb_tpoam_lm_sd_cache_t    lm_sd_cache;
    uint16  index_cos = 0;

    sal_memset(&lm_sd_cache, 0, sizeof(glb_tpoam_lm_sd_cache_t));
    
    for (index = 0; index < g_pst_hagt_tpoam_master->pst_lm_sd_hash->size; index++)
    {   
        for (pst_hb = g_pst_hagt_tpoam_master->pst_lm_sd_hash->index[index]; pst_hb; pst_hb = pst_hb->next)
        {
            pst_lmep_lm_sd = pst_hb->data;

            if (pst_lmep_lm_sd->ratio_limit > 0)
            {
                pst_lmep_lm_sd->ratio_limit = 0;
            }
            else
            {
                pst_lmep_lm_sd->ratio_limit = 1;
            }

            /* read lm dual counters */
            sal_memset(&lm_stat_info, 0, sizeof(ctc_oam_stats_info_t));
            loss_ratio_exceed_limit = FALSE;

                lm_stat_info.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

            switch (pst_lmep_lm_sd->oam_type)
            {
                case GLB_OAM_TYPE_MPLS_LSP:
                case GLB_OAM_TYPE_MPLS_PW:
                    lm_stat_info.key.u.tp.label = pst_lmep_lm_sd->key_id;
                    break;
                case GLB_OAM_TYPE_MPLS_SECTION:
                    lm_stat_info.key.u.tp.gport_or_l3if_id = pst_lmep_lm_sd->key_id;
                    GLB_SET_FLAG(lm_stat_info.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);        
                    break;
                default:
                    HAGT_LOG_ERR("Invalid OAM type %hhu when build LMEP Key.", pst_lmep_lm_sd->oam_type);
                    return;
            }
            
            lm_stat_info.lm_type = pst_lmep_lm_sd->lm_type;
            lm_stat_info.lm_cos_type = pst_lmep_lm_sd->lm_cos_type;
    
            ret = ctc_oam_get_stats(&lm_stat_info);
            if (ret)
            {
                HAGT_LOG_ERR(" Update dual lm failure for oam_type %d key_id %d,"
                               " ret is %d",
                               pst_lmep_lm_sd->oam_type, pst_lmep_lm_sd->key_id,
                               ret);
            }

            /* reserve lm dual counters */
            if (CTC_OAM_LM_COS_TYPE_PER_COS != pst_lmep_lm_sd->lm_cos_type)
            {
                /* check for cos 0 */
                if (hagt_tpoam_check_lm_loss_ratio(pst_lmep_lm_sd, &lm_stat_info, CTC_MIN_COS))
                {
                    loss_ratio_exceed_limit = TRUE;
                }
            }
            else
            {
                /* check lm loss ratio for every cos */
                for (index_cos = 0; index_cos < CTC_MAX_COS; index_cos++)
                {
                    if (hagt_tpoam_check_lm_loss_ratio(pst_lmep_lm_sd, &lm_stat_info, index_cos))
                    {
                        loss_ratio_exceed_limit = TRUE;
                        break;
                    }
                }
            }

            /* send level/vlan/mep_id to hsrv*/
            if (loss_ratio_exceed_limit)
            {
                lm_sd_cache.entry[lm_sd_cache.num].oam_type = pst_lmep_lm_sd->oam_type;
                lm_sd_cache.entry[lm_sd_cache.num].key_id = pst_lmep_lm_sd->key_id;
                lm_sd_cache.entry[lm_sd_cache.num].mep_Index = pst_lmep_lm_sd->mep_index;
                lm_sd_cache.num++;

                if (GLB_TPOAM_MAX_LM_SD_CACHE_NUM == lm_sd_cache.num)
                {
                    ret = _hagt_tpoam_lm_sd_report(lm_sd_cache.num, lm_sd_cache.entry);
                    if (ret)
                    {
                        HAGT_LOG_ERR("Report lm sd failure, ret is %d\n", ret); 
                    }
                    lm_sd_cache.num = 0;
                }  
            }
        }
    }

    if (lm_sd_cache.num)
    {
        ret = _hagt_tpoam_lm_sd_report(lm_sd_cache.num, lm_sd_cache.entry);
        if (ret)
        {
            HAGT_LOG_ERR("Report lm sd failure, ret is %d\n", ret); 
        }
        lm_sd_cache.num = 0;
    }  
    
    return;
}

// TODO:timer have not delete
void
_hagt_tpoam_lm_sd_update_timer_handler (void *p_data)
{
    _hagt_tpoam_update_lm_sd_status();

    HAGT_CHECK_PTR_RETURN_VOID(p_data);
    
    if (g_pst_hagt_tpoam_master->pst_lm_sd_hash->count)
    {
        CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_lm_sd_update_timer_handler, (void*)(g_pst_hagt_tpoam_master), HAGT_TPOAM_LM_SD_TIMER_INTERVAL);   
    }
    
    return;
}

static void
_hagt_tpoam_lm_sd_run_update_timer(bool enable)
{
    CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_lm_sd_update_timer_handler, 
                                    (void*)(g_pst_hagt_tpoam_master), HAGT_TPOAM_LM_SD_TIMER_INTERVAL);

    return;    
}

static int32 
_hagt_tpoam_set_lm_sd_enable(Hsrv2AgtMsg_t* pst_hsrv_msg)
{    
    hagt_tpoam_lm_sd_t* pst_lmep_lm_sd = NULL;
    hagt_tpoam_lm_sd_t  lm_sd;
    void* p_data1 = NULL;

    HAGT_CHECK_PTR(pst_hsrv_msg); 

    HAGT_TPOAM_DBG_COMMON("Enter into %s.", __FUNCTION__);

    sal_memset(&lm_sd, 0, sizeof(hagt_tpoam_lm_sd_t));

    lm_sd.oam_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.oamType;
    lm_sd.key_id = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.keyId;
    lm_sd.mep_index = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.mepIndex;
    lm_sd.ratio_limit = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.ratioLimit;
    lm_sd.lm_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.lmType;
    lm_sd.lm_cos_type = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.cosType;
    lm_sd.lm_cos_value = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.cosValue;
    
    if (pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetLmSDEnable.enable)
    {
       if (0 == g_pst_hagt_tpoam_master->pst_lm_sd_hash->count)
       {
           _hagt_tpoam_lm_sd_run_update_timer(TRUE);
       }
       
       pst_lmep_lm_sd = HAGT_CALLOC(CTCLIB_MEM_TPOAM_MODULE, sizeof(hagt_tpoam_lm_sd_t));
       if (!pst_lmep_lm_sd)
       {
           return HAGT_E_NO_MEMORY;
       }
       
       sal_memcpy(pst_lmep_lm_sd, &lm_sd, sizeof(hagt_tpoam_lm_sd_t));
       
       p_data1 = ctclib_hash_get(g_pst_hagt_tpoam_master->pst_lm_sd_hash, (void*)pst_lmep_lm_sd, 
                                    ctclib_hash_alloc_intern);
       if (!p_data1)
       {
           HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_sd);
           return HAGT_E_NO_MEMORY;
       }
    }
    else
    {
        pst_lmep_lm_sd = ctclib_hash_lookup(g_pst_hagt_tpoam_master->pst_lm_sd_hash, &lm_sd);     
        ctclib_hash_release(g_pst_hagt_tpoam_master->pst_lm_sd_hash, (void*)pst_lmep_lm_sd);  
        HAGT_FREE(CTCLIB_MEM_TPOAM_MODULE, pst_lmep_lm_sd);
    }
    
    return HAGT_E_NONE;
}

static inline uint32
_hagt_tpoam_lm_sd_hash_key (hagt_tpoam_lm_sd_t* pst_lmep_lm_sd)
{
    uint32 key = 0;
    
    HAGT_CHECK_PTR(pst_lmep_lm_sd);

    key = pst_lmep_lm_sd->mep_index;

    return key;
}

static bool
_hagt_tpoam_lm_sd_hash_cmp(void* pva, void* pvb)
{
    hagt_tpoam_lm_sd_t* pst_lmep_lm_sd_a = NULL;
    hagt_tpoam_lm_sd_t* pst_lmep_lm_sd_b = NULL;

    HAGT_CHECK_PTR(pva);
    HAGT_CHECK_PTR(pvb);

    pst_lmep_lm_sd_a = (hagt_tpoam_lm_sd_t*)pva;
    pst_lmep_lm_sd_b = (hagt_tpoam_lm_sd_t*)pvb;

    if (pst_lmep_lm_sd_a->mep_index == pst_lmep_lm_sd_b->mep_index)
    {
        return TRUE;
    }

    return FALSE;
}

#endif

/*init timer*/
int32
_hagt_tpoam_run_update_timer(bool enable)
{
    if (enable)
    {
        if (!g_pst_hagt_tpoam_master->pst_oam_timer)
        {
            g_pst_hagt_tpoam_master->pst_oam_timer = CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_update_timer_handler, NULL, CTC_OAM_OAM_CCM_SIM_INTERVAL);   
            if (!g_pst_hagt_tpoam_master->pst_oam_timer)
            {
                HAGT_LOG_ERR("Update MPLS-TP OAM engine failed.\n");
                return HAGT_E_NOT_INIT;
            }
        }

        if (!g_pst_hagt_tpoam_master->pst_lm_timer)
        {
            g_pst_hagt_tpoam_master->pst_lm_timer = CTC_TASK_ADD_TIME_MSEC(_hagt_tpoam_update_lm_timer_handler, NULL, CTC_OAM_OAM_DUAL_LM_INTERVAL);   
            if (!g_pst_hagt_tpoam_master->pst_lm_timer)
            {
                HAGT_LOG_ERR("Update MPLS-TP OAM LM timer failed.\n");
                return HAGT_E_NOT_INIT;
            }
        }
    }
    else
    {
        if (g_pst_hagt_tpoam_master->pst_oam_timer)
        {
            CTC_TASK_DEL_TIME(g_pst_hagt_tpoam_master->pst_oam_timer); 
        }

        if (g_pst_hagt_tpoam_master->pst_lm_timer)
        {
            CTC_TASK_DEL_TIME(g_pst_hagt_tpoam_master->pst_lm_timer); 
        }
    }

    return HAGT_E_NONE;
}


int32
hagt_tpoam_aps_group_protecting_en(Hsrv2AgtMsg_t* p_msg_data)
{
    uint16  aps_group_id = 0;
    bool  protecting_en = 0;
    int rc = HAGT_E_NONE;

    /*1. get values and dump*/
    HAGT_CHECK_PTR(p_msg_data);
    
    /*2. get msg info*/
    aps_group_id = p_msg_data->opHsrv2Agt.hsrv2AgtMsgMplsApsProtectingEnReq.apsgroupid;
    protecting_en = p_msg_data->opHsrv2Agt.hsrv2AgtMsgMplsApsProtectingEnReq.protectingen;
    
    /*3. protect group withsdk*/
    rc = ctc_aps_set_aps_bridge(aps_group_id, protecting_en);
    if (rc)
    {
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", rc, __FUNCTION__, __LINE__);
    }

    return rc;
}

int32
hagt_tpoam_set_pw_mode(Hsrv2AgtMsg_t* p_msg_data)
{
    int rc = HAGT_E_NONE;

    HAGT_CHECK_PTR(p_msg_data);
    
    /* set pw mode */
    g_pst_hagt_tpoam_master->pw_not_standard_mode = p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamSetPwMode.enable;

    return rc;
}

uint32
hagt_tpoam_get_pw_mode(void)
{
    uint32  pw_is_standard = 0;

    if (0 == g_pst_hagt_tpoam_master->pw_not_standard_mode)
    {
        pw_is_standard = TRUE;
    }
    
    return pw_is_standard;
}

int32
_hagt_tpoam_set_global_fast_aps_en(Hsrv2AgtMsg_t* p_msg_data)
{
    int rc = HAGT_E_NONE;

    /*1. get values and dump*/
    HAGT_CHECK_PTR(p_msg_data);
    
    /*2. get msg info*/
    g_pst_hagt_tpoam_master->fast_aps_enable = p_msg_data->opHsrv2Agt.hsrv2AgtMsgTpoamSetHagtAPS.enable;
    
    return rc;
}

#ifndef glb_config_set
static uint32 
_hagt_tpoam_rdi_reason_glb2ctc(uint8 glb_rdi_reason)
{
    uint32 ctc_rdi_reason = 0;
    if (GLB_FLAG_ISSET(glb_rdi_reason, GLB_TPOAM_RDI_RSN_LOC))
    {
        GLB_SET_FLAG(ctc_rdi_reason, CTC_OAM_DEFECT_DLOC);
    }
    if (GLB_FLAG_ISSET(glb_rdi_reason, GLB_TPOAM_RDI_RSN_CC_ERR))
    {
        GLB_SET_FLAG(ctc_rdi_reason, CTC_OAM_DEFECT_UNEXPECTED_MEP);
        GLB_SET_FLAG(ctc_rdi_reason, CTC_OAM_DEFECT_UNEXPECTED_PERIOD);        
    }
    if (GLB_FLAG_ISSET(glb_rdi_reason, GLB_TPOAM_RDI_RSN_CC_MISMERGE))
    {
        GLB_SET_FLAG(ctc_rdi_reason, CTC_OAM_DEFECT_UNEXPECTED_LEVEL);
        GLB_SET_FLAG(ctc_rdi_reason, CTC_OAM_DEFECT_MISMERGE);
    }  
    return ctc_rdi_reason;
}

static int32 
_hagt_tpoam_set_rdi_reason(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    ctc_oam_property_t oam_prop;
    HalMsgTpoamRdiReasonReq_t* p_msg = NULL;
    uint8 glb_rdi_reason = 0;
    uint32 ctc_rdi_reason = 0;
    
    p_msg = &pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamRdiReason;
    glb_rdi_reason = p_msg->rdiReason;
    ctc_rdi_reason = _hagt_tpoam_rdi_reason_glb2ctc(glb_rdi_reason);
    HAGT_TPOAM_DBG_COMMON("Set RDI reason global 0x%02X, ctc 0x%02X.", glb_rdi_reason, ctc_rdi_reason);

    sal_memset(&oam_prop, 0 ,sizeof(ctc_oam_property_t));
    oam_prop.oam_pro_type       = CTC_OAM_PROPERTY_TYPE_COMMON;
    oam_prop.u.common.cfg_type = CTC_OAM_COM_PRO_TYPE_DEFECT_TO_RDI;
    oam_prop.u.common.value  =  ctc_rdi_reason;

    HAGT_IF_ERROR_RETURN(ctc_oam_set_property(&oam_prop));
    return HAGT_E_NONE;
}

static int32 
_hagt_tpoam_set_sf_reason(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    HalMsgTpoamSFReasonReq_t* p_msg = NULL;
    
    p_msg = &pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgTpoamSetSFReason;
    g_pst_hagt_tpoam_master->sf_reason = p_msg->sfReason;
    
    HAGT_TPOAM_DBG_COMMON("Set SF reason 0x%X.", g_pst_hagt_tpoam_master->sf_reason);
    return HAGT_E_NONE;
}
#endif

int32 hagt_tpoam_init()
{
/*init hagt tpoam, add by chenc*/
    g_pst_hagt_tpoam_master = HAGT_OPENFLOW_CALLOC(sizeof(hagt_tpoam_t));
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_tpoam_master);

    //g_pst_openflow_tpoam->pst_mep_opf = ctclib_opf_init(1);
    //ctclib_opf_init_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, GLB_TPOAM_MEP_INDEX_MIN, 
    //    GLB_TPOAM_MEP_INDEX_NUM);

    /*start uml*/

    /*start timer*/
    _hagt_tpoam_run_update_timer(TRUE);

    /* HASH init MEP */
    g_pst_hagt_tpoam_master->pst_lm_dual_hash = ctclib_hash_create_size(HAGT_TPOAM_LM_DUAL_HASH_SIZE,
        _hagt_tpoam_lm_dual_hash_key, _hagt_tpoam_lm_dual_hash_cmp);
    
    g_pst_hagt_tpoam_master->pst_lm_sd_hash = ctclib_hash_create_size(HAGT_TPOAM_LM_SD_HASH_SIZE, 
        _hagt_tpoam_lm_sd_hash_key, _hagt_tpoam_lm_sd_hash_cmp);
    
    g_pst_hagt_tpoam_master->p_mep = ctclib_vector_init(2, HAGT_TPOAM_FAST_APS_MAX_MEP_NUM); /* max 8192 mep data */
    if (!g_pst_hagt_tpoam_master->p_mep)
    {
        HAGT_LOG_ERR("init tpoam mep vector failed.");
    }

    g_pst_hagt_tpoam_master->p_aps = ctclib_vector_init(1, HAGT_TPOAM_FAST_APS_MAX_APS_NUM); /* max 4096 aps data */
    if (!g_pst_hagt_tpoam_master->p_aps)
    {
        HAGT_LOG_ERR("init tpoam mep vector failed.");
    }

    /*chenc end*/

    g_pst_hagt_tpoam_master->fast_aps_enable = TRUE;

    return HAGT_E_NONE;
}

int32
hagt_openflow_tpoam_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamAddMegid, hagt_tpoam_add_megid);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamDelMegid, hagt_tpoam_del_megid);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamAddOam, hagt_tpoam_add_oam_lmep_rmep);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamDelOam,  hagt_tpoam_del_oam_lmep_rmep);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamUpdateOam, hagt_tpoam_update_oam_lmep);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgMplsApsProtectingEnReq, hagt_tpoam_aps_group_protecting_en);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetPwMode, hagt_tpoam_set_pw_mode);
        
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetLmEnable, _hagt_tpoam_set_lm_enable_);  
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetLmSDEnable, _hagt_tpoam_set_lm_sd_enable);
    
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetSFReason, _hagt_tpoam_set_sf_reason);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamRdiReason, _hagt_tpoam_set_rdi_reason);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetHagtAPS, _hagt_tpoam_set_global_fast_aps_en);
    
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgEthOamCfmAddMip, hagt_tpoam_add_oam_mip);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgEthOamCfmDelMip, hagt_tpoam_del_oam_mip);

#undef Reg    

#ifndef TTPPRODUCT
    ctc_interrupt_register_event_cb(CTC_EVENT_OAM_STATUS_UPDATE, hagt_tpoam_isr_callback);
#endif

    return HAGT_E_NONE;
}

