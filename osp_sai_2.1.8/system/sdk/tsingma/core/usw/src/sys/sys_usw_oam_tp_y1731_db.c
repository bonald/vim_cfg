#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_tp_y1731_db.c

 @date 2011-12-11

 @version v2.0

  This file contains oam sys layer function implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "ctc_parser.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"

#include "sys_usw_ftm.h"

#include "sys_usw_nexthop_api.h"
#include "sys_usw_mpls.h"

#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_tp_y1731_db.h"
#include "sys_usw_mpls.h"
#include "sys_usw_oam_cfm_db.h"

#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];
extern int32 sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value);
#define SYS_OAM_TP_DEFAULT_MEPIDX_ON_CPU 0xFFF
#define SYS_OAM_TP_DEFAULT_MEPIDX_MIP (SYS_OAM_TP_DEFAULT_MEPIDX_ON_CPU-1)
/****************************************************************************
*
* Function
*
**
***************************************************************************/

#define CHAN "Function Begin"

int32
_sys_usw_tp_chan_update(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp_y1731, bool is_add)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    DsEgressXcOamMplsSectionHashKey_m mpls_section_hash_key;
    void *p_key = NULL;
    uint32 index       = 0;
    uint32 tbl_id = 0;
    uint32  lm_en       = 0;
    uint32  oam_check_type = SYS_OAM_TP_NONE_TYPE;
    uint32 mep_index_in_key = 0;
    uint32 ret = 0;
    uint32 mode = 0;
    sys_mpls_oam_t mpls_info;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_lmep_tp_y1731->p_sys_chan;

    /*mep index*/
    if (is_add)
    {
        p_sys_chan_tp->mep_index = p_sys_lmep_tp_y1731->lmep_index;
        p_sys_chan_tp->mep_index_in_key = p_sys_lmep_tp_y1731->lmep_index/2;
        oam_check_type = SYS_OAM_TP_MEP_TYPE;
        // mpls tp use label build hash table for mep_index as key!
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Chan: MPLS TP Update mepindex[0x%x] in DsEgressXcOamMplsLabelHashKey_t\n", p_sys_chan_tp->mep_index_in_key);

        if (!CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU)
            ||(p_sys_chan_tp->key.section_oam))
        {
            CTC_ERROR_RETURN(_sys_usw_tp_chan_build_index(lchip, p_sys_chan_tp));
            CTC_ERROR_RETURN(_sys_usw_tp_chan_add_to_asic(lchip, p_sys_chan_tp));
        }
    }
    else
    {
        p_sys_chan_tp->mep_index  = 0x1FFF;
        p_sys_chan_tp->mep_index_in_key = 0x1FFF;
        oam_check_type = SYS_OAM_TP_NONE_TYPE;
    }

    if ((p_sys_chan_tp->key.section_oam)
        && (!g_oam_master[lchip]->tp_section_oam_based_l3if)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id))))
    {
        return CTC_E_NONE;
    }

    mep_index_in_key = p_sys_chan_tp->mep_index_in_key;
    /*lm enable*/
    lm_en = (CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN) && is_add);
    if (p_sys_chan_tp->key.section_oam)
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
    }
    else
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
    }

    index = p_sys_chan_tp->key.com.key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_key));

    if (p_sys_chan_tp->key.section_oam)
    {
        SetDsEgressXcOamMplsSectionHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index)
        if (lm_en)
        {
            SetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f       , p_key, p_sys_chan_tp->com.lm_index_base) ;
            SetDsEgressXcOamMplsSectionHashKey(V, lmCos_f           , p_key, p_sys_chan_tp->lm_cos) ;
            SetDsEgressXcOamMplsSectionHashKey(V, lmType_f            , p_key, p_sys_chan_tp->lm_type) ;
            SetDsEgressXcOamMplsSectionHashKey(V, lmCosType_f         , p_key,   p_sys_chan_tp->lm_cos_type) ;
        }
        else
        {
            SetDsEgressXcOamMplsSectionHashKey(V, lmType_f            , p_key,  CTC_OAM_LM_TYPE_NONE)
        }
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index)
        SetDsEgressXcOamMplsLabelHashKey(V, mplsLmType_f        , p_key, 0)
        if (lm_en)
        {
            SetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f       , p_key, p_sys_chan_tp->com.lm_index_base)
            SetDsEgressXcOamMplsLabelHashKey(V, lmCos_f           , p_key, p_sys_chan_tp->lm_cos) ;
            SetDsEgressXcOamMplsLabelHashKey(V, lmType_f            , p_key, p_sys_chan_tp->lm_type  ) ;
            SetDsEgressXcOamMplsLabelHashKey(V, lmCosType_f         , p_key,  p_sys_chan_tp->lm_cos_type) ;
        }
        else
        {
            SetDsEgressXcOamMplsLabelHashKey(V, lmType_f            , p_key,  CTC_OAM_LM_TYPE_NONE)
        }
    }

    if (!CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU)
        || (p_sys_chan_tp->key.section_oam))
    {
        CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_key));
    }


    if (p_sys_chan_tp->key.section_oam)
    { /*need update port or interface lm enable*/
        uint32 cmd      = 0;
        uint32 field    = 0;
        uint8  gchip    = 0;

        field = CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);
        if (g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            index = p_sys_chan_tp->key.gport_l3if_id;

            cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
        else
        {
            index = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_sys_chan_tp->key.gport_l3if_id);
            gchip = CTC_MAP_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id);
            if (FALSE == sys_usw_chip_is_local(lchip, gchip))
            {
                return CTC_E_NONE;
            }
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
    }
    else
    {
        /* update dsmpls */
        ctc_mpls_property_t mpls_pro_info;
        sys_nh_update_oam_info_t oam_info;

        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
        mpls_pro_info.label = p_sys_chan_tp->key.label;
        mpls_pro_info.space_id = p_sys_chan_tp->key.spaceid;
        if ((!is_add) && DRV_IS_DUET2(lchip))
        {
            mpls_info.label = p_sys_chan_tp->key.label;
            mpls_info.spaceid = p_sys_chan_tp->key.spaceid;
            mpls_info.oam_type = 0; /*lkup vpws oam info*/
            ret = sys_usw_mpls_get_oam_info(lchip, &mpls_info);
            if (CTC_E_NONE == ret)
            {
               sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
               mep_index_in_key = mode ? (mpls_info.oamid + 2048) : mpls_info.oamid; /*When remove TPOAM, restore VPWS info if VPWS exists*/
            }
        }
        mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_IDX;
        mpls_pro_info.value = (void*)(&mep_index_in_key);
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
        mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
        mpls_pro_info.value = (void*)(&oam_check_type);
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
        mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
        mpls_pro_info.value = (void*)(&lm_en);
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));

        /*update nexthop to enbale egress lm*/
        oam_info.lm_en = lm_en;
        oam_info.oam_mep_index = p_sys_chan_tp->mep_index_in_key;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        sys_usw_nh_update_oam_en(lchip, p_sys_lmep_tp_y1731->nhid, &oam_info);

        /*update dsma info to nexthop*/
        oam_info.update_type = 1;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        oam_info.dsma_en = is_add?1:0;
        oam_info.ma_idx = p_sys_lmep_tp_y1731->ma_index;
        oam_info.mep_index = p_sys_lmep_tp_y1731->lmep_index;
        oam_info.mep_type = 1;
        sys_usw_nh_update_oam_en(lchip, p_sys_lmep_tp_y1731->nhid, &oam_info);

        /*update VPWS OAM key by TPOAM*/
        if (DRV_IS_DUET2(lchip))
        {
            mpls_info.label = p_sys_chan_tp->key.label;
            mpls_info.spaceid = p_sys_chan_tp->key.spaceid;
            mpls_info.oam_type = 0; /*lkup vpws oam info*/
            ret = sys_usw_mpls_get_oam_info(lchip, &mpls_info);
            if (CTC_E_NONE == ret)
            {
                _sys_usw_oam_update_vpws_key(lchip, mpls_info.gport, mpls_info.oamid, p_sys_chan_tp, is_add);
            }
        }
    }

    return CTC_E_NONE;
}
#if 0
int32
_sys_usw_tp_chan_update_reomte(uint8 lchip, ctc_oam_lmep_t* p_lmep, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add)
{
    DsEgressXcOamMplsSectionHashKey_m mpls_section_hash_key;
    void *p_key = NULL;
    uint32 index       = 0;
    uint32 tbl_id    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*mep index*/
    if (is_add)
    {
        p_sys_chan_tp->mep_index = p_lmep->lmep_index;
        p_sys_chan_tp->mep_index_in_key = p_lmep->lmep_index/2;
    }
    else
    {
        p_sys_chan_tp->mep_index = 0x1FFF;
        p_sys_chan_tp->mep_index_in_key = 0x1FFF;
    }

    if (p_sys_chan_tp->key.section_oam)
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
    }
    else
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
    }

    index = p_sys_chan_tp->key.com.key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_key));

    if (p_sys_chan_tp->key.section_oam)
    {
        SetDsEgressXcOamMplsSectionHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index) ;
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index) ;
    }
    CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_key));


    return CTC_E_NONE;
}
#endif
int32
_sys_usw_tp_chan_update_mip(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add)
{
    uint32 oam_check_type   = 0;
    uint32 key_index            = 0;
    uint32 tbl_id = 0;

    DsEgressXcOamMplsLabelHashKey_m label_oam_key;
    uint32 chan_lkp_index = 0;
    ctc_mpls_property_t mpls_pro_info;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&label_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
    chan_lkp_index = SYS_OAM_TP_DEFAULT_MEPIDX_MIP;

    SetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &label_oam_key, chan_lkp_index);
    SetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &label_oam_key, EGRESSXCOAMHASHTYPE_MPLSLABEL);
    SetDsEgressXcOamMplsLabelHashKey(V, valid_f, &label_oam_key, 1);

    tbl_id  = DsEgressXcOamMplsLabelHashKey_t;
    CTC_ERROR_RETURN(sys_usw_oam_key_lookup_io_by_key(lchip, tbl_id,  &key_index, &label_oam_key));

    SetDsEgressXcOamMplsLabelHashKey(V, oamDestChipId_f, &label_oam_key, p_sys_chan_tp->com.master_chipid);
    CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, tbl_id,  key_index, &label_oam_key));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Chan: MPLS TP Y1731 Mip Update!!!key_index:%d, destChipId:%d\n", key_index, p_sys_chan_tp->com.master_chipid);

    if (is_add)
    {
        oam_check_type = SYS_OAM_TP_MIP_TYPE;
    }
    else
    {
        oam_check_type = SYS_OAM_TP_NONE_TYPE;
    }

    sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
    mpls_pro_info.label = p_sys_chan_tp->key.label;
    mpls_pro_info.space_id = p_sys_chan_tp->key.spaceid;

    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_IDX;
    mpls_pro_info.value = (void*)(&chan_lkp_index);
    CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
    mpls_pro_info.value = (void*)(&oam_check_type);
    CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));

    return CTC_E_NONE;
}

#define LMEP "Function Begin"

sys_oam_lmep_t*
_sys_usw_tp_y1731_lmep_lkup(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (NULL == p_sys_chan_tp)
    {
        return NULL;
    }

    return (sys_oam_lmep_t*)_sys_usw_oam_lmep_lkup(lchip, &p_sys_chan_tp->com, NULL);
}


sys_oam_lmep_t*
_sys_usw_tp_y1731_lmep_build_node(uint8 lchip, ctc_oam_lmep_t* p_lmep_param)
{
    sys_oam_lmep_t* p_sys_lmep_tp = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_lmep_tp = (sys_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_lmep_t));
    if (NULL != p_sys_lmep_tp)
    {
        sal_memset(p_sys_lmep_tp, 0, sizeof(sys_oam_lmep_t));
        p_sys_lmep_tp->flag           = p_lmep_param->u.y1731_lmep.flag;
        p_sys_lmep_tp->mep_on_cpu = CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU);
        p_sys_lmep_tp->tx_csf_type    = p_lmep_param->u.y1731_lmep.tx_csf_type;
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            p_sys_lmep_tp->lmep_index = p_lmep_param->lmep_index;
        }
    }

    return p_sys_lmep_tp;
}

int32
_sys_usw_tp_y1731_lmep_free_node(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_lmep_tp);
    p_sys_lmep_tp = NULL;
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_build_index(lchip, p_sys_lmep_tp));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_free_index(lchip, p_sys_lmep_tp));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_add_to_db(lchip, p_sys_lmep_tp));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_db(lchip, p_sys_lmep_tp));
    return CTC_E_NONE;
}


int32
_sys_usw_tp_y1731_lmep_add_to_asic(uint8 lchip, ctc_oam_lmep_t* p_lmep_param,
                                         sys_oam_lmep_t* p_sys_lmep_tp)
{
    ctc_oam_y1731_lmep_t* p_tp_y1731_lmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);
    CTC_PTR_VALID_CHECK(p_sys_lmep_tp);

    p_tp_y1731_lmep = &(p_lmep_param->u.y1731_lmep);
    if (CTC_FLAG_ISSET(p_tp_y1731_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    p_sys_lmep_tp->nhid = p_tp_y1731_lmep->nhid;

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_add_tp_y1731_to_asic(lchip, p_tp_y1731_lmep, p_sys_lmep_tp));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_asic(lchip, p_sys_lmep_tp));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_lmep_update_asic(uint8 lchip, ctc_oam_update_t* p_lmep_param,
                                         sys_oam_lmep_t* p_sys_lmep_tp)
{
    ctc_oam_update_t* p_tp_y1731_lmep = NULL;
    uint32 mode = 0;
    uint8  max_session_num = 8;
    uint32 cmd = 0;
    uint32 old_nhid = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);
    CTC_PTR_VALID_CHECK(p_sys_lmep_tp);

    p_tp_y1731_lmep = p_lmep_param;

    switch (p_tp_y1731_lmep->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 1);
        if (1 == p_tp_y1731_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 7);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;            /*D2 SF_STATE proc by ASIC*/

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, 1);
        if (1 == p_tp_y1731_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN);
        }
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_lmep->update_value, CTC_OAM_CSF_TYPE_MAX - 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP:
        old_nhid = p_sys_lmep_tp->nhid;
        p_sys_lmep_tp->nhid        = p_tp_y1731_lmep->update_value;
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL:
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT:
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM:
        if (0xff != p_tp_y1731_lmep->update_value)
        {
            cmd = DRV_IOR(AutoGenPktGlbCtl_t, AutoGenPktGlbCtl_sessionMode_f);
            DRV_IOCTL(lchip, 0, cmd, &mode);
            max_session_num = (mode)?((mode == 1)?6:4):8;
            if (p_lmep_param->update_value >= max_session_num)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL:
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
        if (p_tp_y1731_lmep->update_value < SYS_OAM_MIN_CCM_INTERVAL || p_tp_y1731_lmep->update_value > SYS_OAM_MAX_CCM_INTERVAL)
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
        if (p_tp_y1731_lmep->update_value < SYS_OAM_MIN_MEP_ID || p_tp_y1731_lmep->update_value > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
        break;
    case  CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP:
        {
            CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_master_chip(lchip, p_sys_lmep_tp->p_sys_chan, p_tp_y1731_lmep->update_value));
            return CTC_E_NONE;
        }
        break;
    case  CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_tp_y1731_asic(lchip, p_sys_lmep_tp, p_tp_y1731_lmep));

    if ((p_tp_y1731_lmep->update_type == CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP) && old_nhid)
    {
        sys_nh_update_oam_info_t oam_info;
        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        /*clear old nhid bind info*/
        oam_info.update_type = 1;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        oam_info.dsma_en = 0;
        oam_info.ma_idx = p_sys_lmep_tp->ma_index;
        oam_info.mep_index = p_sys_lmep_tp->lmep_index;
        oam_info.mep_type = 1;
        sys_usw_nh_update_oam_en(lchip, old_nhid, &oam_info);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_update_lmep_lm(uint8 lchip, ctc_oam_update_t* p_lmep_param, sys_oam_lmep_t* p_sys_lmep_tp_y1731)
{
    uint32 index        = 0;
    uint32 tbl_id     = 0;
    uint32 lm_en        = 0;

    ctc_oam_update_t* p_lmep = (ctc_oam_update_t*)p_lmep_param;
    sys_oam_chan_tp_t* p_sys_chan_tp          = NULL;

    DsEgressXcOamMplsSectionHashKey_m mpls_section_hash_key;

    void *p_key = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. lookup chan and check exist*/
    p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_lmep_tp_y1731->p_sys_chan;

    if (CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
    {
        if((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE == p_lmep->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE == p_lmep->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS == p_lmep->update_type))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Config is invalid lm is Enable\n");
			return CTC_E_INVALID_CONFIG;
        }
    }

    /*2. lmep db & asic update*/
    switch (p_lmep->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, 1);
        if (1 == p_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);

            if (!p_sys_chan_tp->lm_index_alloced)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
                p_sys_chan_tp->lm_index_alloced = TRUE;
            }
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);

            if (p_sys_chan_tp->lm_index_alloced)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
                p_sys_chan_tp->lm_index_alloced = FALSE;
            }
            p_sys_chan_tp->lm_type = CTC_OAM_LM_TYPE_NONE;
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, CTC_OAM_LM_TYPE_MAX - 1);
        p_sys_chan_tp->lm_type = p_lmep->update_value;

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, CTC_OAM_LM_COS_TYPE_MAX - 1);
        p_sys_chan_tp->lm_cos_type = p_lmep->update_value;

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, 7);
        p_sys_chan_tp->lm_cos = p_lmep->update_value;

        break;
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK:
        if (p_sys_chan_tp->key.section_oam)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " TP section OAM do not support lock\n");
            return CTC_E_NOT_SUPPORT;
        }
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, 1);
        p_sys_lmep_tp_y1731->lock_en = p_lmep->update_value;
       break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    lm_en = CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);


    if ((p_sys_chan_tp->key.section_oam)
        && (!g_oam_master[lchip]->tp_section_oam_based_l3if)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id))))
    {
        return CTC_E_NONE;
    }


    if (p_sys_chan_tp->key.section_oam)
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
    }
    else
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
    }

    index = p_sys_chan_tp->key.com.key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_key));

   if (p_sys_chan_tp->key.section_oam)
    {
        SetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f       , p_key, p_sys_chan_tp->com.lm_index_base) ;
        SetDsEgressXcOamMplsSectionHashKey(V, lmCos_f             , p_key, p_sys_chan_tp->lm_cos) ;
        SetDsEgressXcOamMplsSectionHashKey(V, lmType_f            , p_key, p_sys_chan_tp->lm_type) ;
        SetDsEgressXcOamMplsSectionHashKey(V, lmCosType_f         , p_key, p_sys_chan_tp->lm_cos_type) ;
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f       , p_key, p_sys_chan_tp->com.lm_index_base) ;
        SetDsEgressXcOamMplsLabelHashKey(V, lmCos_f             , p_key, p_sys_chan_tp->lm_cos) ;
        SetDsEgressXcOamMplsLabelHashKey(V, lmType_f            , p_key, p_sys_chan_tp->lm_type  ) ;
        SetDsEgressXcOamMplsLabelHashKey(V, lmCosType_f         , p_key, p_sys_chan_tp->lm_cos_type) ;
    }

    CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_key));


    if (p_sys_chan_tp->key.section_oam)
    { /*need update port or interface lm enable*/
        uint32 cmd      = 0;
        uint32 field    = 0;

        field = lm_en;
        if (g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            index = p_sys_chan_tp->key.gport_l3if_id;
            cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
        else
        {
            index = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_sys_chan_tp->key.gport_l3if_id);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
    }
    else
    {
        /* update dsmpls */
        ctc_mpls_property_t mpls_pro_info;
        sys_nh_update_oam_info_t oam_info;
        uint32 value = 0;
        int32 ret = 0;

        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
        mpls_pro_info.label = p_sys_chan_tp->key.label;
        mpls_pro_info.space_id = p_sys_chan_tp->key.spaceid;

        mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
        mpls_pro_info.value = (void*)(&lm_en);
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));

        if(CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK == p_lmep->update_type)
        {
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
            value = p_sys_lmep_tp_y1731->lock_en? 5 : 1;
            mpls_pro_info.value = (void*)(&value);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
        }

        oam_info.lm_en = lm_en;
        oam_info.lock_en = p_sys_lmep_tp_y1731->lock_en;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        oam_info.oam_mep_index = p_sys_chan_tp->mep_index_in_key;
        /*update nexthop to enbale egress lm to be add*/
        ret = sys_usw_nh_update_oam_en(lchip, p_sys_lmep_tp_y1731->nhid, &oam_info);
        if (ret && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK == p_lmep->update_type))
        {
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
            value = p_sys_lmep_tp_y1731->lock_en? 1 : 5;
            mpls_pro_info.value = (void*)(&value);
            sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info);
            return ret;
        }

    }

    return CTC_E_NONE;

}

int32
_sys_usw_tp_y1731_update_label(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_tp)
    {
        return CTC_E_OAM_CHAN_ENTRY_NOT_FOUND;
    }
    else if (p_sys_chan_tp->key.section_oam)
    {
        return CTC_E_INVALID_CONFIG;
    }
    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_label(lchip, (sys_oam_chan_com_t*)p_sys_chan_tp, (ctc_oam_tp_key_t*)p_lmep_param->p_update_value));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_update_master_gchip(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_tp)
    {
        return CTC_E_OAM_CHAN_ENTRY_NOT_FOUND;
    }
    else if (p_sys_chan_tp->key.section_oam)
    {
        return CTC_E_INVALID_CONFIG;
    }
    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_master_chip(lchip, (sys_oam_chan_com_t*)p_sys_chan_tp, p_lmep_param->update_value));
    return CTC_E_NONE;
}

#define RMEP "Function Begin"

sys_oam_rmep_t*
_sys_usw_tp_y1731_rmep_lkup(uint8 lchip, uint32 rmep_id, sys_oam_lmep_t* p_sys_lmep_tp_y1731)
{
    sys_oam_rmep_t sys_oam_rmep_tp_y1731;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_rmep_tp_y1731, 0, sizeof(sys_oam_rmep_t));
    sys_oam_rmep_tp_y1731.rmep_id = rmep_id;

    return (sys_oam_rmep_t*)_sys_usw_oam_rmep_lkup(lchip, p_sys_lmep_tp_y1731, &sys_oam_rmep_tp_y1731);
}


sys_oam_rmep_t*
_sys_usw_tp_y1731_rmep_build_node(uint8 lchip, ctc_oam_rmep_t* p_rmep_param)
{
    sys_oam_rmep_t* p_sys_oam_rmep_tp_y1731 = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_oam_rmep_tp_y1731 = (sys_oam_rmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_rmep_t));
    if (NULL != p_sys_oam_rmep_tp_y1731)
    {
        sal_memset(p_sys_oam_rmep_tp_y1731, 0, sizeof(sys_oam_rmep_t));
        p_sys_oam_rmep_tp_y1731->rmep_id        = p_rmep_param->u.y1731_rmep.rmep_id;
        p_sys_oam_rmep_tp_y1731->rmep_index = p_rmep_param->rmep_index;
    }

    return p_sys_oam_rmep_tp_y1731;
}

int32
_sys_usw_tp_y1731_rmep_free_node(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_rmep_tp_y1731);
    p_sys_rmep_tp_y1731 = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_build_index(lchip, p_sys_rmep_tp_y1731));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_free_index(lchip, p_sys_rmep_tp_y1731));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_add_to_db(lchip, p_sys_rmep_tp_y1731));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_db(lchip, p_sys_rmep_tp_y1731));
    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_build_eth_data(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                            sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    ctc_oam_y1731_rmep_t* p_tp_y1731_rmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_tp_y1731_rmep = (ctc_oam_y1731_rmep_t*)&p_rmep_param->u.y1731_rmep;

    p_sys_rmep_tp_y1731->flag                 = p_tp_y1731_rmep->flag;
    p_sys_rmep_tp_y1731->rmep_id                    = p_tp_y1731_rmep->rmep_id;

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_add_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                         sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);
    CTC_PTR_VALID_CHECK(p_sys_rmep_tp_y1731);

    CTC_ERROR_RETURN(_sys_usw_tp_y1731_rmep_build_eth_data(lchip, p_rmep_param, p_sys_rmep_tp_y1731));
    CTC_ERROR_RETURN(_sys_usw_oam_rmep_add_tp_y1731_to_asic(lchip, p_sys_rmep_tp_y1731));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_asic(lchip, p_sys_rmep_tp_y1731));

    return CTC_E_NONE;
}

int32
_sys_usw_tp_y1731_rmep_update_asic(uint8 lchip, ctc_oam_update_t* p_rmep_param,
                                         sys_oam_rmep_t* p_sys_rmep_tp_y1731)
{
    ctc_oam_update_t* p_tp_y1731_rmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);
    CTC_PTR_VALID_CHECK(p_sys_rmep_tp_y1731);

    p_tp_y1731_rmep = p_rmep_param;

    switch (p_tp_y1731_rmep->update_type)
    {
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_rmep->update_value, 1);
        if (1 == p_tp_y1731_rmep->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep_tp_y1731->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep_tp_y1731->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN);
        }

        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_rmep->update_value, 1);
        if (1 == p_tp_y1731_rmep->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep_tp_y1731->flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep_tp_y1731->flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN);
        }

        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_rmep->update_value, 1);
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS:
        {
            ctc_oam_hw_aps_t* p_hw_aps = NULL;
            p_hw_aps = (ctc_oam_hw_aps_t*)p_tp_y1731_rmep->p_update_value;
            CTC_MAX_VALUE_CHECK(p_hw_aps->aps_group_id, MCHIP_CAP(SYS_CAP_APS_GROUP_NUM)-1);
        }
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN:
        CTC_MAX_VALUE_CHECK(p_tp_y1731_rmep->update_value, 1);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_update_tp_y1731_asic(lchip, p_sys_rmep_tp_y1731, p_rmep_param));

    return CTC_E_NONE;
}

#define INIT "Function Begin"
 STATIC int32
_sys_usw_tp_y1731_default_entry(uint8 lchip, uint8 is_add)
{
    uint32 key_index = 0;
    int32  ret = CTC_E_NONE;
    DsEgressXcOamMplsLabelHashKey_m mpls_oam_key;

    /*for TP MEP on CPU*/
    sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
    SetDsEgressXcOamMplsLabelHashKey(V, oamDestChipId_f, &mpls_oam_key, 0);
    SetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &mpls_oam_key, EGRESSXCOAMHASHTYPE_MPLSLABEL);
    SetDsEgressXcOamMplsLabelHashKey(V, valid_f, &mpls_oam_key, 1);
    SetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &mpls_oam_key, SYS_OAM_TP_DEFAULT_MEPIDX_ON_CPU);
    SetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f, &mpls_oam_key, 0x1FFF);
    ret = sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamMplsLabelHashKey_t,  &key_index, (void*)&mpls_oam_key);
    if (ret < 0 && ret != CTC_E_EXIST)
    {
        return ret;
    }

    if (ret != CTC_E_EXIST)
    {
        if (is_add)
        {
            CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, DsEgressXcOamMplsLabelHashKey_t, key_index, (void*)&mpls_oam_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc DsEgressXcOamMplsLabelHashKey, key_index:%d \n", key_index);
        }
        else
        {
            CTC_ERROR_RETURN( sys_usw_oam_key_delete_io(lchip, DsEgressXcOamMplsLabelHashKey_t, key_index, (void*)&mpls_oam_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free DsEgressXcOamMplsLabelHashKey, key_index:%d \n", key_index);
        }
    }
    else if (CTC_E_EXIST == ret)
    {
        if (is_add)
        {
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
        }
        else
        {
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        }
    }

    /*for TP MIP*/
    sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
    SetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &mpls_oam_key, EGRESSXCOAMHASHTYPE_MPLSLABEL);
    SetDsEgressXcOamMplsLabelHashKey(V, valid_f, &mpls_oam_key, 1);
    SetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &mpls_oam_key, SYS_OAM_TP_DEFAULT_MEPIDX_MIP);
    ret = sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamMplsLabelHashKey_t,  &key_index, (void*)&mpls_oam_key);
    if (ret < 0 && ret != CTC_E_EXIST)
    {
        return ret;
    }

    if (ret != CTC_E_EXIST)
    {
        if (is_add)
        {
            CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, DsEgressXcOamMplsLabelHashKey_t, key_index, (void*)&mpls_oam_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc DsEgressXcOamMplsLabelHashKey, key_index:%d \n", key_index);
        }
        else
        {
            CTC_ERROR_RETURN( sys_usw_oam_key_delete_io(lchip, DsEgressXcOamMplsLabelHashKey_t, key_index, (void*)&mpls_oam_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free DsEgressXcOamMplsLabelHashKey, key_index:%d \n", key_index);
        }
    }
    else if (CTC_E_EXIST == ret)
    {
        if (is_add)
        {
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
        }
        else
        {
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_tp_y1731_register_init(uint8 lchip, ctc_oam_global_t* p_tp_y1731_glb)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    OamParserEtherCtl_m  oam_parser_ether_ctl;
    IpeMplsCtl_m          ipe_mpls_ctl;
    EpeAclQosCtl_m      epe_acl_qos_ctl;
    OamEtherTxCtl_m      oam_ether_tx_ctl;

    /*Parser*/
    field_val = p_tp_y1731_glb->tp_y1731_ach_chan_type;
    cmd = DRV_IOW(ParserLayer4AchCtl_t, ParserLayer4AchCtl_achY1731Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(&oam_parser_ether_ctl, 0, sizeof(OamParserEtherCtl_m));
    cmd = DRV_IOR(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_parser_ether_ctl));
    SetOamParserEtherCtl(V, achY1731ChanType_f, &oam_parser_ether_ctl, p_tp_y1731_glb->tp_y1731_ach_chan_type);
    SetOamParserEtherCtl(V,  mplsTpOamAlertLabel_f, &oam_parser_ether_ctl, 13);
    SetOamParserEtherCtl(V,  mplstpLbmTlvOffset_f, &oam_parser_ether_ctl, 4);
    SetOamParserEtherCtl(V,  mplstpLbmTlvOffsetChk_f      , &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V,  mplstpLbmTlvSubTypeChk_f , &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V,  mplstpLbmTlvLenChk_f  , &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V,  mplstpLbmTlvTypeChk_f , &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V,  achChanTypeMcc_f , &oam_parser_ether_ctl, 0x0001);
    SetOamParserEtherCtl(V,  achChanTypeScc_f , &oam_parser_ether_ctl, 0x0002);
    SetOamParserEtherCtl(V,  achChanTypeFm_f , &oam_parser_ether_ctl, 0x0058);
    SetOamParserEtherCtl(V,  tpY1731MdLevel_f , &oam_parser_ether_ctl, 7);


    cmd = DRV_IOW(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_parser_ether_ctl));

    /*ipe lookup*/
    field_val = (g_oam_master[lchip]->tp_section_oam_based_l3if ? 0 : 1);
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_mplsSectionOamUsePort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(&ipe_mpls_ctl, 0, sizeof(IpeMplsCtl_m));
    cmd = DRV_IOR(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_mpls_ctl));

    SetIpeMplsCtl(V,  oamAlertLabel1_f , &ipe_mpls_ctl, 13);
    SetIpeMplsCtl(V,  galSbitCheckEn_f , &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V,  galTtlCheckEn_f , &ipe_mpls_ctl, 1);
    cmd = DRV_IOW(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_mpls_ctl));

    sal_memset(&epe_acl_qos_ctl, 0, sizeof(EpeAclQosCtl_m));
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));
    SetEpeAclQosCtl(V, mplsSectionOamUsePort_f,  &epe_acl_qos_ctl, (g_oam_master[lchip]->tp_section_oam_based_l3if?0:1));
    cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));

    /*ipe/epe oam*/
    field_val = 1;
    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_lmProactiveMplsOamPacket_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(EpeOamCtl_t, EpeOamCtl_lmProactiveMplsOamPacket_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*OAM*/
    field_val = (g_oam_master[lchip]->tp_section_oam_based_l3if ? 0 : 1);
    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_portbasedSectionOam_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_tpLbmCheckLocalMepIdEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(&oam_ether_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
    cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_tx_ctl));
    SetOamEtherTxCtl(V, achHeaderL_f, &oam_ether_tx_ctl, 0x1000);
    SetOamEtherTxCtl(V, achY1731ChanType_f, &oam_ether_tx_ctl,  p_tp_y1731_glb->tp_y1731_ach_chan_type);
    cmd = DRV_IOW(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_tx_ctl));

    /*TP-Y1731 default entry*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_default_entry(lchip, 1));


    sys_usw_ftm_register_rsv_key_cb(lchip, SYS_FTM_HASH_KEY_OAM, _sys_usw_tp_y1731_default_entry);

    return CTC_E_NONE;
}

#endif
