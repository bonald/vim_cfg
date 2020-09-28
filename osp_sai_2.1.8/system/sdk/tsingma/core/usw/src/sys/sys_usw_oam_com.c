#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_com.c

 @date 2013-01-18

 @version v2.1

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

#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"
#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_oam_debug.h"

#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];

/****************************************************************************
*
* Function
*
***************************************************************************/
 int32
sys_usw_oam_key_write_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data)
{
    drv_acc_in_t  lookup_info;
    drv_acc_out_t lookup_result;

    sal_memset(&lookup_info, 0, sizeof(lookup_info));
    sal_memset(&lookup_result, 0, sizeof(lookup_result));

    lookup_info.type = DRV_ACC_TYPE_ADD;
    lookup_info.op_type = DRV_ACC_OP_BY_INDEX;
    lookup_info.data = p_data;
    lookup_info.tbl_id = tbl_id;
    lookup_info.index = index;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &lookup_info, &lookup_result));

    return CTC_E_NONE;
}

 int32
sys_usw_oam_key_delete_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data)
{
    drv_acc_in_t  lookup_info;
    drv_acc_out_t lookup_result;

    sal_memset(&lookup_info, 0, sizeof(lookup_info));
    sal_memset(&lookup_result, 0, sizeof(lookup_result));

    lookup_info.type = DRV_ACC_TYPE_DEL;
    lookup_info.op_type = DRV_ACC_OP_BY_INDEX;
    lookup_info.data = p_data;
    lookup_info.tbl_id = tbl_id;
    lookup_info.index = index;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &lookup_info, &lookup_result));
    return CTC_E_NONE;
}


int32
sys_usw_oam_key_read_io(uint8 lchip, uint32 tbl_id, uint32 index, void *p_data)
{
    drv_acc_in_t  lookup_info;
    drv_acc_out_t lookup_result;

    CTC_PTR_VALID_CHECK(p_data);
    sal_memset(&lookup_info, 0, sizeof(lookup_info));
    sal_memset(&lookup_result, 0, sizeof(lookup_result));

    lookup_info.type = DRV_ACC_TYPE_LOOKUP;
    lookup_info.op_type = DRV_ACC_OP_BY_INDEX;
    lookup_info.tbl_id = tbl_id;
    lookup_info.index = index;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &lookup_info, &lookup_result));
    sal_memcpy(p_data, lookup_result.data, sizeof(DsEgressXcOamEthHashKey_m));

    return CTC_E_NONE;
}

int32
sys_usw_oam_key_lookup_io(uint8 lchip, uint32 tbl_id, uint32 *index, void *p_data)
{
    drv_acc_in_t  lookup_info;
    drv_acc_out_t lookup_result;


    sal_memset(&lookup_info, 0, sizeof(lookup_info));
    sal_memset(&lookup_result, 0, sizeof(lookup_result));

    lookup_info.type = DRV_ACC_TYPE_ALLOC;
    lookup_info.op_type = DRV_ACC_OP_BY_KEY;
    lookup_info.data = p_data;
    lookup_info.tbl_id = tbl_id;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &lookup_info, &lookup_result));
    *index = lookup_result.key_index;

    if (!lookup_result.is_hit && !lookup_result.is_conflict)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"tbl_id:%d, index->0x%x\n", tbl_id,  lookup_result.key_index);
    }
    else if (lookup_result.is_conflict)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
        return CTC_E_HASH_CONFLICT;
    }
    else if (lookup_result.is_hit)
    {

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
		return CTC_E_EXIST;
    }

    return CTC_E_NONE;
}

int32
sys_usw_oam_key_lookup_io_by_key(uint8 lchip, uint32 tbl_id, uint32 *index, void *p_data)
{
    drv_acc_in_t  lookup_info;
    drv_acc_out_t lookup_result;


    sal_memset(&lookup_info, 0, sizeof(lookup_info));
    sal_memset(&lookup_result, 0, sizeof(lookup_result));

    lookup_info.type = DRV_ACC_TYPE_LOOKUP;
    lookup_info.op_type = DRV_ACC_OP_BY_KEY;
    lookup_info.data = p_data;
    lookup_info.tbl_id = tbl_id;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &lookup_info, &lookup_result));

    if (lookup_result.is_hit)
    {
        *index = lookup_result.key_index;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"tbl_id:%d, index->0x%x\n", tbl_id,  lookup_result.key_index);
    }
    else
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hash lookup confict,\n");
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}



int32
_sys_usw_oam_com_add_maid(uint8 lchip, void* p_maid)
{
    ctc_oam_maid_t* p_maid_param = (ctc_oam_maid_t*)p_maid;
    sys_oam_maid_com_t* p_sys_maid = NULL;
    int32 ret                = CTC_E_NONE;
    uint8 mep_type           = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check maid param valid*/
    CTC_PTR_VALID_CHECK(p_maid_param);
    mep_type = p_maid_param->mep_type;
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_CHECK_FUNC_TABLE(lchip, mep_type, SYS_OAM_CHECK_MAID));
    CTC_ERROR_RETURN((SYS_OAM_CHECK_FUNC_TABLE(lchip, mep_type, SYS_OAM_CHECK_MAID))(lchip, (void*)p_maid_param, TRUE));

    /*2. lookup maid and check exist*/
    p_sys_maid = _sys_usw_oam_maid_lkup(lchip, p_maid_param);
    if (NULL != p_sys_maid)
    {
        ret = CTC_E_EXIST;
        goto RETURN;
    }

    /*3. build maid sys node*/
    p_sys_maid = _sys_usw_oam_maid_build_node(lchip, p_maid_param);
    if (NULL == p_sys_maid)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: build_node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto RETURN;
    }

    /*4. build maid index*/
    ret = _sys_usw_oam_maid_build_index(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: build index fail!!\n");
        goto STEP_3;
    }

    /*5 write chan to db*/
    ret = _sys_usw_oam_maid_add_to_db(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: add db fail!!\n");
        goto STEP_4;
    }

    /*6. write key/chan to asic*/
    ret = _sys_usw_oam_maid_add_to_asic(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: add asic fail!!\n");
        goto STEP_5;
    }

    goto RETURN;

STEP_5:
    _sys_usw_oam_maid_del_from_db(lchip, p_sys_maid);
STEP_4:
    _sys_usw_oam_maid_free_index(lchip, p_sys_maid);
STEP_3:
    _sys_usw_oam_maid_free_node(lchip, p_sys_maid);

RETURN:
    return ret;
}


int32
_sys_usw_oam_com_remove_maid(uint8 lchip, void* p_maid)
{
    ctc_oam_maid_t* p_maid_param = (ctc_oam_maid_t*)p_maid;
    sys_oam_maid_com_t* p_sys_maid = NULL;
    int32 ret                = CTC_E_NONE;
    uint8 mep_type           = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check maid param valid*/
    CTC_PTR_VALID_CHECK(p_maid_param);
    mep_type = p_maid_param->mep_type;
    SYS_OAM_FUNC_TABLE_CHECK(SYS_OAM_CHECK_FUNC_TABLE(lchip, mep_type, SYS_OAM_CHECK_MAID));
    CTC_ERROR_RETURN((SYS_OAM_CHECK_FUNC_TABLE(lchip, mep_type, SYS_OAM_CHECK_MAID))(lchip, (void*)p_maid_param, FALSE));


    /*2. lookup maid and check exist*/
    p_sys_maid = _sys_usw_oam_maid_lkup(lchip, p_maid_param);
    if (NULL == p_sys_maid)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (p_sys_maid->ref_cnt)
    {
        ret = CTC_E_IN_USE;
        goto error;
    }

    /*3. delete maid from asic*/
    ret = _sys_usw_oam_maid_del_from_asic(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: del asic fail!!\n");
        goto error;
    }

    /*4. delete maid from db*/
    ret = _sys_usw_oam_maid_del_from_db(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: del db fail!!\n");
        goto error;
    }

    /*5. free maid index*/
    ret = _sys_usw_oam_maid_free_index(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: free index fail!!\n");
        goto error;
    }

    /*6. free maid sys node*/
    ret = _sys_usw_oam_maid_free_node(lchip, p_sys_maid);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid: free node fail!!\n");
        goto error;
    }

error:
    return ret;
}

#define TP_OAM_CHAN  "FUCNTION"

sys_oam_chan_tp_t*
_sys_usw_tp_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm)
{
    sys_oam_chan_tp_t sys_oam_chan;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_chan, 0, sizeof(sys_oam_chan_tp_t));
    sys_oam_chan.key.com.mep_type   = p_key_parm->mep_type;
    sys_oam_chan.key.label          = p_key_parm->u.tp.label;
    sys_oam_chan.key.spaceid        = p_key_parm->u.tp.mpls_spaceid;
    sys_oam_chan.key.gport_l3if_id  = p_key_parm->u.tp.gport_or_l3if_id;
    sys_oam_chan.key.section_oam    = CTC_FLAG_ISSET(p_key_parm->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    sys_oam_chan.com.mep_type       = p_key_parm->mep_type;

    return (sys_oam_chan_tp_t*)_sys_usw_oam_chan_lkup(lchip, &sys_oam_chan.com);
}


sys_oam_chan_tp_t*
_sys_usw_tp_chan_build_node(uint8 lchip, ctc_oam_lmep_t* p_chan_param)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    uint8 mep_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mep_type = p_chan_param->key.mep_type;

    p_sys_chan_tp = (sys_oam_chan_tp_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_chan_tp_t));
    if (NULL != p_sys_chan_tp)
    {
        sal_memset(p_sys_chan_tp, 0, sizeof(sys_oam_chan_tp_t));
        p_sys_chan_tp->com.mep_type = p_chan_param->key.mep_type;

        if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type)
        {
            p_sys_chan_tp->com.master_chipid = p_chan_param->u.y1731_lmep.master_gchip;
        }
        else if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        {
            p_sys_chan_tp->com.master_chipid = p_chan_param->u.bfd_lmep.master_gchip;
        }
        p_sys_chan_tp->key.com.mep_type     = p_chan_param->key.mep_type;
        p_sys_chan_tp->key.label            = p_chan_param->key.u.tp.label;
        p_sys_chan_tp->key.spaceid          = p_chan_param->key.u.tp.mpls_spaceid;
        p_sys_chan_tp->key.gport_l3if_id    = p_chan_param->key.u.tp.gport_or_l3if_id;
        p_sys_chan_tp->key.section_oam      = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        p_sys_chan_tp->mep_on_cpu = CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU);

        if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type)
        {
            if (CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
            {
                p_sys_chan_tp->lm_type     = p_chan_param->u.y1731_lmep.lm_type;
                p_sys_chan_tp->lm_cos_type = p_chan_param->u.y1731_lmep.lm_cos_type;
                p_sys_chan_tp->lm_cos      = p_chan_param->u.y1731_lmep.lm_cos;
            }
            else
            {
                p_sys_chan_tp->lm_type     = CTC_OAM_LM_TYPE_NONE;
            }
        }
        else if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        {
            if (CTC_FLAG_ISSET(p_chan_param->u.bfd_lmep.flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN))
            {
                p_sys_chan_tp->lm_type     = CTC_OAM_LM_TYPE_SINGLE;
                p_sys_chan_tp->lm_cos_type = p_chan_param->u.bfd_lmep.lm_cos_type;
                p_sys_chan_tp->lm_cos      = p_chan_param->u.bfd_lmep.lm_cos;
            }
            else
            {
                p_sys_chan_tp->lm_type     = CTC_OAM_LM_TYPE_NONE;
            }
        }


    }

    return p_sys_chan_tp;
}

int32
_sys_usw_tp_chan_free_node(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_chan_tp);
    p_sys_chan_tp = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_tp_chan_build_index(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{
    DsEgressXcOamMplsLabelHashKey_m label_oam_key;
    DsEgressXcOamMplsSectionHashKey_m section_oam_key;
    void *p_key=NULL;

    uint16 gport        = 0;

    uint32 tbl_id = 0;
    uint32 mpls_key     = 0;
    uint8 section_lchip = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_tp);

    if (p_sys_chan_tp->key.section_oam && (!g_oam_master[lchip]->tp_section_oam_based_l3if))
    {
        gport = p_sys_chan_tp->key.gport_l3if_id;
    }

    sal_memset(&label_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
    sal_memset(&section_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));

    if (p_sys_chan_tp->key.section_oam)
    {
        if (g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            mpls_key = p_sys_chan_tp->key.gport_l3if_id;
        }
        else
        {
            gport = p_sys_chan_tp->key.gport_l3if_id;
            mpls_key = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
        }
    }

    if (p_sys_chan_tp->key.section_oam)
    {

        SetDsEgressXcOamMplsSectionHashKey(V, interfaceId_f, &section_oam_key, mpls_key);
        SetDsEgressXcOamMplsSectionHashKey(V, hashType_f, &section_oam_key, EGRESSXCOAMHASHTYPE_MPLSSECTION);
        SetDsEgressXcOamMplsSectionHashKey(V, valid_f, &section_oam_key, 1);
        p_key = &section_oam_key;
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"CHAN(Key): DsEgressXcOamMplsSectionHashKey use mpls_key[0x%x] \n", mpls_key);
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &label_oam_key, p_sys_chan_tp->mep_index_in_key);
        SetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &label_oam_key, EGRESSXCOAMHASHTYPE_MPLSLABEL);
        SetDsEgressXcOamMplsLabelHashKey(V, valid_f, &label_oam_key, 1);
        p_key = &label_oam_key;
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"CHAN(Key): DsEgressXcOamMplsLabelHashKey use mpls_key[0x%x](mep_index) \n", p_sys_chan_tp->mep_index_in_key);
    }

    if ((p_sys_chan_tp->key.section_oam)
        && (!g_oam_master[lchip]->tp_section_oam_based_l3if)
    && (!CTC_IS_LINKAGG_PORT(p_sys_chan_tp->key.gport_l3if_id))
    && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id))))
    {
        CTC_ERROR_RETURN(sys_usw_get_local_chip_id(SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id), &section_lchip));
        CTC_ERROR_RETURN(sys_usw_oam_key_lookup_io(section_lchip, tbl_id,  &p_sys_chan_tp->key.com.key_index, p_key));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_oam_key_lookup_io(lchip, tbl_id,  &p_sys_chan_tp->key.com.key_index, p_key));
    }
    p_sys_chan_tp->key.com.key_alloc = SYS_OAM_KEY_HASH;
    p_sys_chan_tp->key.com.key_exit = 1;

    if ((CTC_OAM_LM_TYPE_NONE != p_sys_chan_tp->lm_type) && (!p_sys_chan_tp->lm_index_alloced))
    {
        CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
        p_sys_chan_tp->lm_index_alloced = TRUE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_tp_chan_free_index(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_tp);

    /*1.free chan index*/

    /*2.free lm index*/
    if (TRUE == p_sys_chan_tp->lm_index_alloced)
    {
        CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
        p_sys_chan_tp->lm_index_alloced = FALSE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_tp_chan_add_to_asic(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{
    uint32 key_index  = 0;
    uint32 tbl_id = 0;
    uint16 gport      = 0;
    uint32 mpls_key   = 0;
    uint32 hash_type  = 0;
    void *p_key = NULL;

    DsEgressXcOamMplsLabelHashKey_m mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m section_oam_key;


    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_tp);


    if (p_sys_chan_tp->key.section_oam)
    {
        if (g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            mpls_key = p_sys_chan_tp->key.gport_l3if_id;
        }
        else
        {
            gport = p_sys_chan_tp->key.gport_l3if_id;
            mpls_key =  SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

            if (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id)))
            {
                return CTC_E_NONE;
            }
        }
    }
    else
    {
    /*
        if (!p_sys_chan_tp->mep_index)
        {
            return CTC_E_NONE;
        }
        */
    }

    hash_type = p_sys_chan_tp->key.section_oam ? EGRESSXCOAMHASHTYPE_MPLSSECTION :
                                                                                     EGRESSXCOAMHASHTYPE_MPLSLABEL;

    if (p_sys_chan_tp->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        if (p_sys_chan_tp->key.section_oam)
        {
            sal_memset(&section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
            SetDsEgressXcOamMplsSectionHashKey(V, valid_f, &section_oam_key, 1);
            SetDsEgressXcOamMplsSectionHashKey(V, hashType_f, &section_oam_key, hash_type);
            SetDsEgressXcOamMplsSectionHashKey(V, interfaceId_f, &section_oam_key, mpls_key);
            SetDsEgressXcOamMplsSectionHashKey(V, oamDestChipId_f, &section_oam_key, (p_sys_chan_tp->com.master_chipid));
            p_key = &section_oam_key;
            tbl_id = DsEgressXcOamMplsSectionHashKey_t;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: DsEgressXcOamMplsSectionHashKey.mpls_key = 0x%x\n", mpls_key);
        }
        else
        {
            sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
            SetDsEgressXcOamMplsLabelHashKey(V, oamDestChipId_f, &mpls_oam_key, (p_sys_chan_tp->com.master_chipid));
            SetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &mpls_oam_key, hash_type);
            SetDsEgressXcOamMplsLabelHashKey(V, valid_f, &mpls_oam_key, 1);
            SetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &mpls_oam_key, p_sys_chan_tp->mep_index_in_key);
            p_key = &mpls_oam_key;
            tbl_id = DsEgressXcOamMplsLabelHashKey_t;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: DsEgressXcOamMplsLabelHashKey_mplsOamIndex_f = 0x%x\n", p_sys_chan_tp->mep_index_in_key);
        }
        key_index = p_sys_chan_tp->key.com.key_index;
        CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, tbl_id, key_index, p_key));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_tp_chan_del_from_asic(uint8 lchip, sys_oam_chan_tp_t* p_sys_chan_tp)
{
    DsEgressXcOamMplsLabelHashKey_m mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m section_oam_key;
    uint32 key_index      = 0;
    void *p_key = NULL;
    uint32 tbl_id = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_tp);

    if (0 == p_sys_chan_tp->key.com.key_exit)
    {
        return CTC_E_NONE;
    }

    key_index = p_sys_chan_tp->key.com.key_index;

    if ((p_sys_chan_tp->key.section_oam)
        && (!g_oam_master[lchip]->tp_section_oam_based_l3if)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id))))
    {
        return CTC_E_NONE;
    }

    if (p_sys_chan_tp->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        if (p_sys_chan_tp->key.section_oam)
        {
            sal_memset(&section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
            p_key = &section_oam_key;
            tbl_id = DsEgressXcOamMplsSectionHashKey_t;
        }
        else
        {
            sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
            p_key = &mpls_oam_key;
            tbl_id = DsEgressXcOamMplsLabelHashKey_t;
        }
        if (!(p_sys_chan_tp->mep_on_cpu) || (p_sys_chan_tp->key.section_oam))
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_delete_io(lchip, tbl_id, key_index, p_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        }
    }

    return CTC_E_NONE;
}



int32
_sys_usw_tp_add_chan(uint8 lchip, void* p_chan, void** p_sys_chan)
{
    ctc_oam_lmep_t* p_chan_param   = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    int32 ret = CTC_E_NONE;

    /*1. lookup chan and check exist*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_chan_param->key);
    if (NULL != p_sys_chan_tp)
    {
        ret = CTC_E_EXIST;
        (*p_sys_chan) = p_sys_chan_tp;
        goto error;
    }

    /*2. build chan sys node*/
    p_sys_chan_tp = _sys_usw_tp_chan_build_node(lchip, p_chan_param);
    if (NULL == p_sys_chan_tp)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    /*4. add chan to db*/
    ret = _sys_usw_oam_chan_add_to_db(lchip, &p_sys_chan_tp->com);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: add db fail!!\n");
        goto STEP_3;
    }

    (*p_sys_chan) = p_sys_chan_tp;

    goto error;

STEP_3:
    _sys_usw_tp_chan_free_node(lchip, p_sys_chan_tp);

error:
    return ret;
}


int32
_sys_usw_tp_remove_chan(uint8 lchip, void* p_chan)
{
    ctc_oam_lmep_t* p_chan_param = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    int32 ret = CTC_E_NONE;

    /*1. lookup chan and check exist*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_chan_param->key);
    if (NULL == p_sys_chan_tp)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    if (CTC_SLISTCOUNT(p_sys_chan_tp->com.lmep_list) > 0)
    { /*need consider mip*/
        ret = CTC_E_EXIST;
        goto RETURN;
    }

    /*2. remove key/chan from asic*/
    if (0 == p_sys_chan_tp->key.mip_en)
    {
        ret = _sys_usw_tp_chan_del_from_asic(lchip, p_sys_chan_tp);
        if (CTC_E_NONE != ret)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del asic fail!!\n");
            goto RETURN;
        }
    }

    /*3. remove chan from db*/
    ret = _sys_usw_oam_chan_del_from_db(lchip, &p_sys_chan_tp->com);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del db fail!!\n");
        goto RETURN;
    }

    /*4. free chan index*/
    ret = _sys_usw_tp_chan_free_index(lchip, p_sys_chan_tp);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: free index fail!!\n");
        goto RETURN;
    }

    /*5. free chan node*/
    ret = _sys_usw_tp_chan_free_node(lchip, p_sys_chan_tp);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: free index fail!!\n");
        goto RETURN;
    }

RETURN:
    return ret;
}

#endif
