#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_oam.c

 @date 2010-3-9

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
#include "sys_usw_oam_db.h"
#include "sys_usw_oam.h"
#include "sys_usw_oam_cfm.h"
#include "sys_usw_oam_tp_y1731.h"
#include "sys_usw_oam_bfd.h"

#include "sys_usw_mpls.h"
#include "sys_usw_chip.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_ftm.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "usw/include/drv_io.h"

#include "sys_usw_interrupt.h"
/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
/****************************************************************************
*
* Function
*
**
***************************************************************************/

int32
sys_usw_oam_add_maid(uint8 lchip, ctc_oam_maid_t* p_maid)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_maid);
    oam_type = p_maid->mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_ADD));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_ADD))(lchip, (void*)p_maid);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_remove_maid(uint8 lchip, ctc_oam_maid_t* p_maid)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_maid);
    oam_type = p_maid->mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_DEL));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_DEL))(lchip, (void*)p_maid);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_add_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;
    uint32 session_num = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_lmep);
    oam_type = p_lmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    CTC_PTR_VALID_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_ADD));

    OAM_LOCK(lchip);
    ret = sys_usw_oam_get_session_num(lchip, &session_num);
    if ( (CTC_E_NONE != ret ) || (session_num >= SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_OAM)))
    {
        OAM_UNLOCK(lchip);
        return CTC_E_NO_RESOURCE;
    }
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_ADD))(lchip, (void*)p_lmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_remove_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_lmep);
    oam_type = p_lmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_DEL));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_DEL))(lchip, (void*)p_lmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_update_lmep(uint8 lchip, ctc_oam_update_t* p_lmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_lmep);
    oam_type = p_lmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_UPDATE));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_UPDATE))(lchip, (void*)p_lmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_add_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rmep);
    oam_type = p_rmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_ADD));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_ADD))(lchip, (void*)p_rmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_remove_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rmep);
    oam_type = p_rmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_DEL));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_DEL))(lchip, (void*)p_rmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_update_rmep(uint8 lchip, ctc_oam_update_t* p_rmep)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rmep);
    oam_type = p_rmep->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_UPDATE));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_UPDATE))(lchip, (void*)p_rmep);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_add_mip(uint8 lchip, ctc_oam_mip_t* p_mip)
{
    int32 ret       = CTC_E_NONE;
    uint8 oam_type  = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mip);
    oam_type = p_mip->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_ADD));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_ADD))(lchip, (void*)p_mip);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_remove_mip(uint8 lchip, ctc_oam_mip_t* p_mip)
{
    int32 ret       = CTC_E_NONE;
    uint8 oam_type  = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mip);
    oam_type = p_mip->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_DEL));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_DEL))(lchip, (void*)p_mip);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_set_property(uint8 lchip, ctc_oam_property_t* p_prop)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_pro_type = CTC_OAM_PROPERTY_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_prop);
    oam_pro_type = p_prop->oam_pro_type;
    if (oam_pro_type >= CTC_OAM_PROPERTY_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_pro_type, SYS_OAM_GLOB, SYS_OAM_CONFIG));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_pro_type, SYS_OAM_GLOB, SYS_OAM_CONFIG))(lchip, (void*)p_prop);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_get_defect_info(uint8 lchip, void* p_defect_info)
{
    int32 ret      = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_defect_info);
    OAM_LOCK(lchip);
    ret = _sys_usw_oam_get_defect_info(lchip, p_defect_info);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_get_mep_info(uint8 lchip,  void* p_mep_info, uint8 with_key)
{
    int32 ret      = CTC_E_NONE;
    uint8 oam_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mep_info);

    if(with_key)
    {
        oam_type = ((ctc_oam_mep_info_with_key_t*)p_mep_info)->key.mep_type;
        if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_GLOB, SYS_OAM_GET_INFO));
        OAM_LOCK(lchip);
        ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_GLOB, SYS_OAM_GET_INFO))(lchip, (void*)p_mep_info);
        OAM_UNLOCK(lchip);
    }
    else
    {
        OAM_LOCK(lchip);
        ret = _sys_usw_oam_get_mep_info(lchip, (ctc_oam_mep_info_t*)p_mep_info);
        OAM_UNLOCK(lchip);
    }

    return ret;
}

int32
sys_usw_oam_get_stats_info(uint8 lchip, ctc_oam_stats_info_t* p_stat_info)
{

    int32 ret       = CTC_E_NONE;
    uint8 oam_type  = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stat_info);
    oam_type = p_stat_info->key.mep_type;
    if (oam_type >= CTC_OAM_MEP_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_STATS, SYS_OAM_CONFIG));
    OAM_LOCK(lchip);
    ret = (SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_STATS, SYS_OAM_CONFIG))(lchip, (void*)p_stat_info);
    OAM_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_oam_isr_defect(uint8 lchip, uint32 intr, void* p_data)
{
    ctc_oam_defect_info_t  defect_info;
    uint8 gchip = 0;
    ctc_oam_mep_info_t     mep_info;
    ctc_oam_event_t* p_event = NULL;
    uint8 defect_index      = 0;
    uint8 mep_bit_index     = 0;
    uint32 mep_index_bitmap = 0;
    uint32 mep_index_base   = 0;
    uint32 mep_index        = 0;
    CTC_INTERRUPT_EVENT_FUNC error_cache_cb = NULL;
    uint8 do_defect_cache_cnt = 0;
    uint8 do_defect_cache_num = 2;
    int32 ret = CTC_E_NONE;

    sal_memset(&defect_info,    0, sizeof(ctc_oam_defect_info_t));
    sal_memset(&mep_info,       0, sizeof(ctc_oam_mep_info_t));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_OAM_STATUS_UPDATE, &error_cache_cb));
    if (!error_cache_cb)
    {
        return CTC_E_NOT_INIT;
    }
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /* 1. read valid bitmap */
    CTC_ERROR_RETURN(_sys_usw_oam_get_defect_info(lchip, &defect_info));

    p_event = (ctc_oam_event_t*)mem_malloc(MEM_OAM_MODULE, sizeof(ctc_oam_event_t));
    if (NULL == p_event)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_event, 0, sizeof(ctc_oam_event_t));
    OAM_LOCK(lchip);
    while ((do_defect_cache_cnt < do_defect_cache_num) && defect_info.valid_cache_num)
    {
        for (defect_index = 0; defect_index < defect_info.valid_cache_num; defect_index++)
        {
            mep_index_bitmap    = defect_info.mep_index_bitmap[defect_index];
            mep_index_base      = defect_info.mep_index_base[defect_index];

            for (mep_bit_index = 0; mep_bit_index < 32; mep_bit_index++)
            {
                if (!IS_BIT_SET(mep_index_bitmap, mep_bit_index))
                {
                    continue;
                }
                mep_index = mep_index_base * 32 + mep_bit_index;
                mep_info.mep_index = mep_index;
                ret = _sys_usw_oam_get_mep_info(lchip, &mep_info);
                if (CTC_E_NOT_READY == ret)
                {
                    continue;
                }
                _sys_usw_oam_build_defect_event(lchip, &mep_info, p_event);
                if (p_event->valid_entry_num >= CTC_OAM_EVENT_MAX_ENTRY_NUM)
                {
                    OAM_UNLOCK(lchip);
                    CTC_ERROR_GOTO(error_cache_cb(gchip, p_event), ret, ERROR_FREE_MEM);
                    OAM_LOCK(lchip);
                    p_event->valid_entry_num = 0;
                }

            }
        }
        defect_info.valid_cache_num = 0;
        do_defect_cache_cnt++;
        if (do_defect_cache_cnt < do_defect_cache_num)
        {
            CTC_ERROR_GOTO(_sys_usw_oam_get_defect_info(lchip, &defect_info), ret, ERROR_FREE_MEM);
        }
    }

    if (p_event->valid_entry_num > 0)
    {
        OAM_UNLOCK(lchip);
        CTC_ERROR_GOTO(error_cache_cb(gchip, p_event), ret, ERROR_FREE_MEM);
        OAM_LOCK(lchip);
    }

ERROR_FREE_MEM:
    OAM_UNLOCK(lchip);
    mem_free(p_event);
    return ret;
}


int32
sys_usw_oam_init(uint8 lchip, ctc_oam_global_t* p_cfg)
{
    int32 ret      = CTC_E_NONE;
    uint8 work_status = 0;

    CTC_PTR_VALID_CHECK(p_cfg);

    if (0 == SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_OAM))
    {
        return CTC_E_NONE;
    }
       /* check init */
    if (g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    CTC_ERROR_RETURN(_sys_usw_oam_db_init(lchip, p_cfg));

    CTC_ERROR_RETURN(_sys_usw_oam_register_init(lchip, p_cfg));

    CTC_ERROR_RETURN(sys_usw_cfm_init(lchip, CTC_OAM_MEP_TYPE_ETH_1AG, p_cfg));
    CTC_ERROR_RETURN(sys_usw_cfm_init(lchip, CTC_OAM_MEP_TYPE_ETH_Y1731, p_cfg));
    if (_sys_usw_oam_get_mpls_entry_num(lchip))
    {
        CTC_ERROR_RETURN(sys_usw_oam_tp_y1731_init(lchip, CTC_OAM_MEP_TYPE_MPLS_TP_Y1731, p_cfg));
        CTC_ERROR_RETURN(sys_usw_oam_bfd_init(lchip, CTC_OAM_MEP_TYPE_MPLS_BFD, p_cfg));
        CTC_ERROR_RETURN(sys_usw_oam_bfd_init(lchip, CTC_OAM_MEP_TYPE_MPLS_TP_BFD, p_cfg));
    }

    CTC_ERROR_RETURN(sys_usw_oam_bfd_init(lchip, CTC_OAM_MEP_TYPE_IP_BFD, p_cfg));
    CTC_ERROR_RETURN(sys_usw_oam_bfd_init(lchip, CTC_OAM_MEP_TYPE_MICRO_BFD, p_cfg));

     /*register event call back function when initialed*/
    if (!g_oam_master[lchip]->no_mep_resource)
    {
        g_oam_master[lchip]->mep_defect_bitmap  = (uint32*)mem_malloc(MEM_OAM_MODULE, TABLE_MAX_INDEX(lchip, DsEthMep_t)*4);
        if (g_oam_master[lchip]->mep_defect_bitmap)
        {
            sal_memset((uint8*)g_oam_master[lchip]->mep_defect_bitmap, 0 , TABLE_MAX_INDEX(lchip, DsEthMep_t)*4);
            CTC_ERROR_RETURN(sys_usw_interrupt_register_event_cb(lchip, CTC_EVENT_OAM_STATUS_UPDATE, sys_usw_isr_oam_process_isr));
        }
        CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_OAM_DEFECT_CACHE, sys_usw_oam_isr_defect))
    }

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_OAM, sys_usw_oam_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_oam_wb_restore(lchip));
    }

    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_OAM, sys_usw_oam_dump_db));

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return ret;
}


int32
sys_usw_oam_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (0 == SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_OAM))
    {
        return CTC_E_NONE;
    }

    _sys_usw_oam_db_deinit(lchip);

    return CTC_E_NONE;
}

#endif

