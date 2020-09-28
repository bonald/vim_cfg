#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_tp_y1731.c

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

#include "sys_usw_chip.h"

#include "sys_usw_oam_tp_y1731.h"
#include "sys_usw_oam_tp_y1731_db.h"
#include "sys_usw_oam_debug.h"

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

STATIC int32
_sys_usw_tp_y1731_maid_check_param(uint8 lchip, void* p_maid_param, bool is_add)
{
    ctc_oam_maid_t* p_maid = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_maid_param);
    p_maid = (ctc_oam_maid_t*)p_maid_param;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Maid: '%s', len: 0x%x !!\n",
                      p_maid->maid, p_maid->maid_len);

    if (p_maid->maid_len > 16)  /*megid length in Y1731 is 16 bytes*/
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Invalid maid_len: 0x%x\n", p_maid->maid_len);
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_tp_y1731_lmep_check_param(uint8 lchip, ctc_oam_lmep_t* p_lmep_param, bool is_add)
{

    ctc_oam_y1731_lmep_t* p_tp_y1731_lmep = NULL;
    uint32 max_mep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);

    p_tp_y1731_lmep = &(p_lmep_param->u.y1731_lmep);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: label->0x%x, gport_l3ifid->%d, flag->0x%x!!\n",
                      p_lmep_param->key.u.tp.label, p_lmep_param->key.u.tp.gport_or_l3if_id, p_lmep_param->key.flag);

     /*SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: level->%d !!\n", p_lmep_param->key.u.eth.md_level);*/

    if (CTC_FLAG_ISSET(p_lmep_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
    {
        if (!g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            SYS_GLOBAL_PORT_CHECK(p_lmep_param->key.u.tp.gport_or_l3if_id);

            if (CTC_IS_LINKAGG_PORT(p_lmep_param->key.u.tp.gport_or_l3if_id))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
                return CTC_E_INVALID_PORT;
            }
        }
        else
        {
            if ((p_lmep_param->key.u.tp.gport_or_l3if_id < SYS_USW_MIN_CTC_L3IF_ID)
                || (p_lmep_param->key.u.tp.gport_or_l3if_id > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1)))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid interface ID! \n");
                return CTC_E_BADID;
            }
        }
    }

    if (is_add)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: mep id->%d, interval->%d, master gchip->%d!!\n",
                          p_tp_y1731_lmep->mep_id, p_tp_y1731_lmep->ccm_interval, p_tp_y1731_lmep->master_gchip);

        SYS_GLOBAL_CHIPID_CHECK(p_tp_y1731_lmep->master_gchip);

        /* check ccm interval */
        if (p_tp_y1731_lmep->ccm_interval < SYS_OAM_MIN_CCM_INTERVAL
            || p_tp_y1731_lmep->ccm_interval > SYS_OAM_MAX_CCM_INTERVAL)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mep ccm interval is invalid \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check mep id */
        if (p_tp_y1731_lmep->mep_id < SYS_OAM_MIN_MEP_ID
            || p_tp_y1731_lmep->mep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mep id is invalid \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check lm value*/
        if (CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
        {
            if(p_lmep_param->u.y1731_lmep.lm_type >= CTC_OAM_LM_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Lm type is invalid \n");
                return CTC_E_INVALID_PARAM;
            }

            if(p_lmep_param->u.y1731_lmep.lm_cos_type >= CTC_OAM_LM_COS_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Lm cos type is invalid \n");
                return CTC_E_INVALID_PARAM;
            }

            CTC_COS_RANGE_CHECK(p_lmep_param->u.y1731_lmep.lm_cos);
        }

        /* check csf value*/
        if (CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN))
        {
            if(p_lmep_param->u.y1731_lmep.tx_csf_type >= CTC_OAM_CSF_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " CSF type is invalid \n");
                return CTC_E_INVALID_PARAM;
            }
        }

        /* check mep index*/
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            max_mep_index = _sys_usw_oam_get_mep_entry_num(lchip);
            if (max_mep_index && ((p_lmep_param->lmep_index < SYS_OAM_MIN_MEP_INDEX)
                || (p_lmep_param->lmep_index >= max_mep_index)))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam mep index is invalid \n");
                return CTC_E_INVALID_PARAM;
            }
        }

        /*check tx_cos_exp*/
        CTC_COS_RANGE_CHECK(p_lmep_param->u.y1731_lmep.tx_cos_exp);

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_tp_y1731_rmep_check_param(uint8 lchip, ctc_oam_rmep_t* p_rmep_param, bool is_add)
{
    ctc_oam_y1731_rmep_t* p_tp_y1731_rmep = NULL;

    uint32 max_mep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: label->0x%x, gport_l3ifid->%d, flag->0x%x!!\n",
                      p_rmep_param->key.u.tp.label, p_rmep_param->key.u.tp.gport_or_l3if_id, p_rmep_param->key.flag);

    p_tp_y1731_rmep = &(p_rmep_param->u.y1731_rmep);

    if (is_add)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: rmep id->%d !!\n", p_tp_y1731_rmep->rmep_id);

        /* check mep id */
        if (p_tp_y1731_rmep->rmep_id < SYS_OAM_MIN_MEP_ID
            || p_tp_y1731_rmep->rmep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mep id is invalid \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check mep index*/
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            max_mep_index = _sys_usw_oam_get_mep_entry_num(lchip);
            if ((p_rmep_param->rmep_index < SYS_OAM_MIN_MEP_INDEX)
                || (p_rmep_param->rmep_index >= max_mep_index))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam mep index is invalid \n");
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_tp_y1731_mip_check_param(uint8 lchip, ctc_oam_mip_t* p_mip_param, bool is_add)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_mip_param);

    /*check section oam not support mip*/
    if (CTC_FLAG_ISSET(p_mip_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Section oam not support mip \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_tp_y1731_check_update_param(uint8 lchip, ctc_oam_update_t* p_update_param, bool is_local)
{

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_update_param);

    /* check is local or remote*/
    if (p_update_param->is_local != is_local)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: label->0x%x, gport_l3ifid->%d, flag->0x%x!!\n",
                      p_update_param->key.u.tp.label, p_update_param->key.u.tp.gport_or_l3if_id, p_update_param->key.flag);

    if (CTC_FLAG_ISSET(p_update_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
    {
        if (!g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            SYS_GLOBAL_PORT_CHECK(p_update_param->key.u.tp.gport_or_l3if_id);

            if (CTC_IS_LINKAGG_PORT(p_update_param->key.u.tp.gport_or_l3if_id))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
                return CTC_E_INVALID_PORT;
            }

        }
        else
        {
            if ((p_update_param->key.u.tp.gport_or_l3if_id < SYS_USW_MIN_CTC_L3IF_ID)
                || (p_update_param->key.u.tp.gport_or_l3if_id > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1)))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid interface ID! \n");
                return CTC_E_BADID;
            }
        }
    }

    if (!is_local)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: rmep id->%d !!\n", p_update_param->rmep_id);

        /* check mep id */
        if (p_update_param->rmep_id < SYS_OAM_MIN_MEP_ID
            || p_update_param->rmep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mep id is invalid \n");
            return CTC_E_INVALID_PARAM;
        }
    }

    if (p_update_param->is_local)
    {
        switch (p_update_param->update_type)
        {
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_IF_STATUS:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA:
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Config is invalid in this mep type\n");
			return CTC_E_INVALID_CONFIG;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS:
            CTC_COS_RANGE_CHECK(p_update_param->update_value);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL:
            CTC_MAX_VALUE_CHECK(p_update_param->update_value, SYS_OAM_MAX_MD_LEVEL);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP:
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL:
            if ((0 == p_update_param->update_value) || (p_update_param->update_value > 255))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid TTL value\n");
                return CTC_E_INVALID_PARAM;
            }
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP:
            SYS_GLOBAL_CHIPID_CHECK(p_update_param->update_value);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
            CTC_MAX_VALUE_CHECK(p_update_param->update_value, 7);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
            break;
        default:
            break;
        }
    }
    else
    {
        switch (p_update_param->update_type)
        {
        case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_SEQ_FAIL_CLEAR:
        case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_PORT_INTF_MIS:
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Config is invalid in this mep type\n");
			return CTC_E_INVALID_CONFIG;

        default:
            break;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_tp_y1731_add_lmep(uint8 lchip, void* p_lmep)
{
    ctc_oam_lmep_t* p_lmep_param = (ctc_oam_lmep_t*)p_lmep;
    sys_oam_maid_com_t* p_sys_maid_tp = NULL;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731 = NULL;

    int32            ret = CTC_E_NONE;
    uint32          session_type = SYS_OAM_SESSION_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (lmep)*/
    ret = _sys_usw_tp_y1731_lmep_check_param(lchip, p_lmep_param, TRUE);
    if (CTC_E_NONE != ret)
    {
        goto error;
    }

    /*2. check chan exist or add chan */
    ret = _sys_usw_tp_add_chan(lchip, p_lmep_param, (void**)&p_sys_chan_tp);
    if ((CTC_E_NONE != ret) && (CTC_E_EXIST != ret))
    {
        goto error;
    }

    if (NULL == p_sys_chan_tp)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MEP or MIP lookup key not found!!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup maid and check exist*/
    p_lmep_param->maid.mep_type = p_lmep_param->key.mep_type;
    p_sys_maid_tp = _sys_usw_oam_maid_lkup(lchip, &p_lmep_param->maid);
    if ((NULL == p_sys_maid_tp) && (!g_oam_master[lchip]->no_mep_resource))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid entry not found!!\n");
        ret = CTC_E_NOT_EXIST;
        goto STEP_4;
    }

    /*5. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp);
    if (NULL != p_sys_lmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Local mep already exists!!\n");
        ret = CTC_E_EXIST;
        goto error;
    }

    /*6. build lmep sys node*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_build_node(lchip, p_lmep_param);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto STEP_4;
    }

    p_sys_lmep_tp_y1731->p_sys_maid = p_sys_maid_tp;
    p_sys_lmep_tp_y1731->p_sys_chan = &p_sys_chan_tp->com;
    if (p_sys_maid_tp && (!p_sys_lmep_tp_y1731->mep_on_cpu))
    {
        p_sys_maid_tp->ref_cnt++;
    }
    /*7. build lmep index */
    ret = _sys_usw_tp_y1731_lmep_build_index(lchip, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: build index fail!!\n");
        goto STEP_5;
    }

    /*8. add lmep to db */
    ret = _sys_usw_tp_y1731_lmep_add_to_db(lchip, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: add db fail!!\n");
        goto STEP_6;
    }

    /*9. write lmep asic*/
    ret = _sys_usw_tp_y1731_lmep_add_to_asic(lchip, p_lmep_param, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: add asic fail!!\n");
        goto STEP_7;
    }

    /*10. update chan*/
    ret = _sys_usw_tp_chan_update(lchip, p_sys_lmep_tp_y1731, TRUE);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: update chan fail!!\n");
        goto STEP_8;
    }

    sys_usw_oam_get_session_type(lchip, p_sys_lmep_tp_y1731, &session_type);
    SYS_OAM_SESSION_CNT(lchip, session_type, 1);
    /*11. copy mep index*/
    p_lmep_param->lmep_index = p_sys_lmep_tp_y1731->lmep_index;

    goto error;

STEP_8:
    _sys_usw_tp_y1731_lmep_del_from_asic(lchip, p_sys_lmep_tp_y1731);
STEP_7:
    _sys_usw_tp_y1731_lmep_del_from_db(lchip, p_sys_lmep_tp_y1731);
STEP_6:
    _sys_usw_tp_y1731_lmep_free_index(lchip, p_sys_lmep_tp_y1731);
STEP_5:
    if (p_sys_maid_tp && (!p_sys_lmep_tp_y1731->mep_on_cpu))
    {
        p_sys_maid_tp->ref_cnt--;
    }
    _sys_usw_tp_y1731_lmep_free_node(lchip, p_sys_lmep_tp_y1731);
STEP_4:
    _sys_usw_tp_remove_chan(lchip, p_lmep_param);

error:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_remove_lmep(uint8 lchip, void* p_lmep)
{
    ctc_oam_lmep_t* p_lmep_param                = NULL;
    sys_oam_chan_tp_t* p_sys_chan_tp            = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731   = NULL;
    sys_oam_maid_com_t* p_sys_maid              = NULL;
    int32            ret = CTC_E_NONE;
    uint32          session_type = SYS_OAM_SESSION_MAX;
    uint32 mep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_lmep_param = (ctc_oam_lmep_t*)p_lmep;

    /*1. check param valid (lmep)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_lmep_check_param(lchip, p_lmep_param, FALSE));

    /*2. lookup chan and check exist*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_tp)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }
    mep_index = p_sys_lmep_tp_y1731->lmep_index;
    /*4 check if rmep list */
    if (CTC_SLISTCOUNT(p_sys_lmep_tp_y1731->rmep_list) > 0)
    {
        ret = CTC_E_EXIST;
        goto error;
    }

    sys_usw_oam_get_session_type(lchip, p_sys_lmep_tp_y1731, &session_type);
    p_sys_maid = p_sys_lmep_tp_y1731->p_sys_maid;

    /*5. update chan*/
    ret = _sys_usw_tp_chan_update(lchip, p_sys_lmep_tp_y1731, FALSE);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: update chan fail!!\n");
    }

    /*6. remove lmep from asic*/
    ret = _sys_usw_tp_y1731_lmep_del_from_asic(lchip, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: del asic fail!!\n");
    }

    /*7. remove lmep from db*/
    ret = _sys_usw_tp_y1731_lmep_del_from_db(lchip, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: del db fail!!\n");
    }

    /*8. free lmep index*/
    ret = _sys_usw_tp_y1731_lmep_free_index(lchip, p_sys_lmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: free index fail!!\n");
    }

    /*9. free lmep node*/
    if (p_sys_maid && (!p_sys_lmep_tp_y1731->mep_on_cpu))
    {
        p_sys_maid->ref_cnt--;
    }
    _sys_usw_tp_y1731_lmep_free_node(lchip, p_sys_lmep_tp_y1731);

    /*10. update chan exist or remove chan */
    ret = _sys_usw_tp_remove_chan(lchip, p_lmep_param);
    if ((CTC_E_EXIST == ret) || (CTC_E_NOT_EXIST == ret))
    {
        ret = CTC_E_NONE;
    }
    SYS_OAM_SESSION_CNT(lchip, session_type, 0);
    if ((0x1FFF != mep_index) && g_oam_master[lchip]->mep_defect_bitmap[mep_index])
    {
        sal_memset(&g_oam_master[lchip]->mep_defect_bitmap[mep_index], 0, 4);
    }

error:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_update_lmep(uint8 lchip, void* p_lmep)
{

    ctc_oam_update_t* p_lmep_update = (ctc_oam_update_t*)p_lmep;

    sys_oam_chan_tp_t* p_sys_chan_tp          = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731    = NULL;
    int32   ret = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (lmep update)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_check_update_param(lchip, p_lmep_update, TRUE));

    /*2. lookup chan and check exist*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_lmep_update->key);
    if (NULL == p_sys_chan_tp)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid operation on mep configured for cpu \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*4. update lmep db and asic*/
    if (!p_lmep_update->is_local)
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (p_lmep_update->update_type)
    {
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
            ret = _sys_usw_tp_y1731_lmep_update_asic(lchip, p_lmep_update, p_sys_lmep_tp_y1731);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
            CTC_ERROR_RETURN(_sys_usw_tp_y1731_maid_check_param(lchip, (void*)(p_lmep_update->p_update_value), TRUE));
            ret = _sys_usw_tp_y1731_lmep_update_asic(lchip, p_lmep_update, p_sys_lmep_tp_y1731);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK:
            ret = _sys_usw_tp_y1731_update_lmep_lm(lchip, p_lmep_update, p_sys_lmep_tp_y1731);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP:
            ret = _sys_usw_tp_y1731_update_master_gchip(lchip, p_lmep_update);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LABEL:
            ret = _sys_usw_tp_y1731_update_label(lchip, p_lmep_update);
            break;
        default:
            ret = CTC_E_INVALID_PARAM;
    }


    return ret;

}

STATIC int32
_sys_usw_tp_y1731_add_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_rmep_t* p_rmep_param = (ctc_oam_rmep_t*)p_rmep;
    sys_oam_chan_tp_t* p_sys_chan_tp_y1731 = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731 = NULL;
    sys_oam_rmep_t* p_sys_rmep_tp_y1731 = NULL;
    int32 ret           = CTC_E_NONE;
    uint32 rmep_id      = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (rmep)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_rmep_check_param(lchip, p_rmep, TRUE));



    /*2. lookup chan and check exist*/
    p_sys_chan_tp_y1731 = _sys_usw_tp_chan_lkup(lchip, &p_rmep_param->key);
    if (NULL == p_sys_chan_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MEP or MIP lookup key not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp_y1731);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Local mep not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if ((!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        && CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        if (1 != (p_rmep_param->rmep_index- p_sys_lmep_tp_y1731->lmep_index))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam mep index is invalid  \n");
            return CTC_E_INVALID_PARAM;
        }
    }
    if (CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_param->u.y1731_rmep.rmep_id;
    p_sys_rmep_tp_y1731 = _sys_usw_tp_y1731_rmep_lkup(lchip, rmep_id, p_sys_lmep_tp_y1731);
    if (NULL != p_sys_rmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Remote mep already exists \n");
        ret = CTC_E_EXIST;
        goto error;
    }

    if (CTC_SLISTCOUNT(p_sys_lmep_tp_y1731->rmep_list) > 0)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TP-Y1731 only support P2P mode \n");
        ret = CTC_E_EXIST;
        goto error;
    }

    /*5. build rmep sys node*/
    p_sys_rmep_tp_y1731 = _sys_usw_tp_y1731_rmep_build_node(lchip, p_rmep_param);
    if (NULL == p_sys_rmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    p_sys_rmep_tp_y1731->p_sys_lmep = p_sys_lmep_tp_y1731;

    /*6. build rmep index */
    ret = _sys_usw_tp_y1731_rmep_build_index(lchip, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: build index fail!!\n");
        goto STEP_5;
    }

    /*7. add rmep to db */
    ret = _sys_usw_tp_y1731_rmep_add_to_db(lchip, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: add db fail!!\n");
        goto STEP_6;
    }

    /*8. write rmep asic*/
    ret = _sys_usw_tp_y1731_rmep_add_to_asic(lchip, p_rmep_param, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: add asic fail!!\n");
        goto STEP_7;
    }

    /*9. copy mep index*/
    p_rmep_param->rmep_index = p_sys_rmep_tp_y1731->rmep_index;

    goto error;

STEP_7:
    _sys_usw_tp_y1731_rmep_del_from_db(lchip, p_sys_rmep_tp_y1731);
STEP_6:
    _sys_usw_tp_y1731_rmep_free_index(lchip, p_sys_rmep_tp_y1731);
STEP_5:
    _sys_usw_tp_y1731_rmep_free_node(lchip, p_sys_rmep_tp_y1731);

error:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_remove_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_rmep_t* p_rmep_param = (ctc_oam_rmep_t*)p_rmep;
    sys_oam_chan_tp_t* p_sys_chan_tp_y1731 = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731 = NULL;
    sys_oam_rmep_t* p_sys_rmep_tp_y1731 = NULL;
    int32 ret           = CTC_E_NONE;
    uint32 rmep_id  = 0;
    uint32 rmep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (rmep)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_rmep_check_param(lchip, p_rmep, FALSE));

    /*2. lookup chan and check exist*/
    p_sys_chan_tp_y1731 = _sys_usw_tp_chan_lkup(lchip, &p_rmep_param->key);
    if (NULL == p_sys_chan_tp_y1731)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp_y1731);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_param->u.y1731_rmep.rmep_id;
    p_sys_rmep_tp_y1731 = _sys_usw_tp_y1731_rmep_lkup(lchip, rmep_id, p_sys_lmep_tp_y1731);
    if (NULL == p_sys_rmep_tp_y1731)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    /*5. remove rmep from asic*/
    ret = _sys_usw_tp_y1731_rmep_del_from_asic(lchip, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: del asic fail!!\n");
    }

    /*6. remove rmep from db*/
    ret = _sys_usw_tp_y1731_rmep_del_from_db(lchip, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: del db fail!!\n");
    }

    /*7. free rmep index*/
    ret = _sys_usw_tp_y1731_rmep_free_index(lchip, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: free index fail!!\n");
    }

    rmep_index = p_sys_rmep_tp_y1731->rmep_index;
    /*8. free rmep node*/
    _sys_usw_tp_y1731_rmep_free_node(lchip, p_sys_rmep_tp_y1731);

    if (g_oam_master[lchip]->mep_defect_bitmap[rmep_index])
    {
        sal_memset(&g_oam_master[lchip]->mep_defect_bitmap[rmep_index], 0, 4);
    }

RETURN:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_update_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_update_t* p_rmep_param = (ctc_oam_update_t*)p_rmep;

    sys_oam_chan_tp_t* p_sys_chan_tp_y1731 = NULL;
    sys_oam_lmep_t* p_sys_lmep_tp_y1731 = NULL;
    sys_oam_rmep_t* p_sys_rmep_tp_y1731 = NULL;
    uint32 rmep_id  = 0;
    int32 ret = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (rmep update)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_check_update_param(lchip, p_rmep_param, FALSE));



    /*2. lookup chan and check exist*/
    p_sys_chan_tp_y1731 = _sys_usw_tp_chan_lkup(lchip, &p_rmep_param->key);
    if (NULL == p_sys_chan_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MEP or MIP lookup key not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep_tp_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp_y1731);
    if (NULL == p_sys_lmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Local mep not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_tp_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_param->rmep_id;
    p_sys_rmep_tp_y1731 = _sys_usw_tp_y1731_rmep_lkup(lchip, rmep_id, p_sys_lmep_tp_y1731);
    if (NULL == p_sys_rmep_tp_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Remote mep not found!! \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*5. update lmep db and asic*/
    ret = _sys_usw_tp_y1731_rmep_update_asic(lchip, p_rmep_param, p_sys_rmep_tp_y1731);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: update asic fail!!\n");
        goto error;
    }

error:
    return ret;
}

STATIC int32
_sys_usw_tp_y1731_add_mip(uint8 lchip, void* p_mip)
{
    ctc_oam_mip_t* p_mip_param = NULL;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    ctc_oam_lmep_t      lmep_param;

    int32 ret = CTC_E_NONE;

    p_mip_param = p_mip;

    /*1. check param valid (mip)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_mip_check_param(lchip, p_mip, TRUE));


    /*2. check & add chan*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_mip_param->key);
    if (NULL == p_sys_chan_tp)
    {
        sal_memset(&lmep_param, 0, sizeof(ctc_oam_lmep_t));
        sal_memcpy(&lmep_param.key, &p_mip_param->key, sizeof(ctc_oam_key_t));
        lmep_param.u.y1731_lmep.master_gchip = p_mip_param->master_gchip;

        ret = _sys_usw_tp_add_chan(lchip, &lmep_param, (void**)&p_sys_chan_tp);
        if (CTC_E_NONE != ret)
        {
            goto error;
        }
    }

    /*3.update db&asic*/
    p_sys_chan_tp->key.mip_en = 1;
    ret = _sys_usw_tp_chan_update_mip(lchip, p_sys_chan_tp, TRUE);

    if (CTC_E_NONE != ret)
    {
        _sys_usw_tp_chan_update_mip(lchip, p_sys_chan_tp, FALSE);
        _sys_usw_tp_remove_chan(lchip, &lmep_param);
    }

error:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_remove_mip(uint8 lchip, void* p_mip)
{

    ctc_oam_mip_t* p_mip_param = NULL;
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    ctc_oam_lmep_t      lmep_param;

    int32 ret = CTC_E_NONE;

    p_mip_param = p_mip;

    /*1. check param valid (mip)*/
    CTC_ERROR_RETURN(_sys_usw_tp_y1731_mip_check_param(lchip, p_mip, FALSE));


    /*2. check chan*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_mip_param->key);
    if (NULL == p_sys_chan_tp)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    /*3. update db&asic*/
    _sys_usw_tp_chan_update_mip(lchip, p_sys_chan_tp, FALSE);

    sal_memset(&lmep_param, 0, sizeof(ctc_oam_lmep_t));
    sal_memcpy(&lmep_param.key, &p_mip_param->key, sizeof(ctc_oam_key_t));
    lmep_param.u.y1731_lmep.master_gchip = p_mip_param->master_gchip;

    /*4. remove chan*/
    ret = _sys_usw_tp_remove_chan(lchip, &lmep_param);
    if (CTC_E_EXIST == ret)
    {
        ret = CTC_E_NONE;
    }

RETURN:

    return ret;
}

STATIC int32
_sys_usw_tp_y1731_get_stats_info(uint8 lchip, void* p_stat_info)
{

    ctc_oam_stats_info_t* p_sys_stat = NULL;

    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    sys_oam_lmep_t* p_sys_lmep_y1731 = NULL;
    sys_oam_lm_com_t sys_oam_lm_com;
    uint8   lm_type         = 0;
    uint8   lm_cos_type     = 0;
    uint16  lm_base         = 0;
    uint32  lm_index        = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_stat_info);

    p_sys_stat = (ctc_oam_stats_info_t*)p_stat_info;

    /*1. lookup chan and check exist*/
    p_sys_chan_tp = _sys_usw_tp_chan_lkup(lchip, &p_sys_stat->key);
    if (NULL == p_sys_chan_tp)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    /*2. lookup lmep and check exist*/
    p_sys_lmep_y1731 = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan_tp);
    if (NULL == p_sys_lmep_y1731)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid operation on mep configured for cpu \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*3. get mep lm index base, lm type, lm cos type, lm cos*/
    if (!CTC_FLAG_ISSET(p_sys_lmep_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " LM not enbale in MEP\n");
        return CTC_E_NOT_READY;
    }

    lm_cos_type = p_sys_chan_tp->lm_cos_type;
    lm_type     = p_sys_chan_tp->lm_type;
    lm_base     = p_sys_chan_tp->com.lm_index_base;

    if ((CTC_OAM_LM_COS_TYPE_ALL_COS == lm_cos_type)
        || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == lm_cos_type))
    {
        lm_index = lm_base;
    }
    else if (CTC_OAM_LM_COS_TYPE_PER_COS == lm_cos_type)
    {
        lm_index = lm_base;
    }
    sys_oam_lm_com.lm_cos_type  = lm_cos_type;
    sys_oam_lm_com.lm_index     = lm_index;
    sys_oam_lm_com.lm_type      = lm_type;
    _sys_usw_oam_get_stats_info(lchip, &sys_oam_lm_com, p_stat_info);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_tp_y1731_get_mep_info(uint8 lchip, void* p_mep_info)
{
    ctc_oam_mep_info_with_key_t* p_mep_param = (ctc_oam_mep_info_with_key_t*)p_mep_info;

    sys_oam_chan_tp_t*     p_sys_chan  = NULL;
    sys_oam_lmep_t*   p_sys_lmep  = NULL;
    sys_oam_rmep_t*   p_sys_rmep  = NULL;
    int32 ret = CTC_E_NONE;
    ctc_oam_mep_info_t mep_info;
    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);



    /*2. lookup chan and check exist*/
    p_sys_chan = _sys_usw_tp_chan_lkup(lchip, &p_mep_param->key);
    if (NULL == p_sys_chan)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MEP or MIP lookup key not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }


    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_chan->com.master_chipid))
    {
        ret = CTC_E_NOT_EXIST;  /*check master_chipid for Multichip*/
        goto error;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep = _sys_usw_tp_y1731_lmep_lkup(lchip, p_sys_chan);
    if (NULL == p_sys_lmep)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Local mep not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*5. get rmep*/
    p_sys_rmep = _sys_usw_tp_y1731_rmep_lkup(lchip, p_mep_param->rmep_id, p_sys_lmep);
    if (NULL == p_sys_rmep)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Remote mep not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }
    mep_info.mep_index = p_sys_rmep->rmep_index;
    ret = _sys_usw_oam_get_mep_info(lchip, &mep_info);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Get mep info fail!!\n");
        goto error;
    }
    sal_memcpy(&p_mep_param->lmep, &mep_info.lmep, sizeof(ctc_oam_lmep_info_t));
    sal_memcpy(&p_mep_param->rmep, &mep_info.rmep, sizeof(ctc_oam_rmep_info_t));

error:

    return ret;
}

int32
sys_usw_oam_tp_y1731_init(uint8 lchip, uint8 oam_type, ctc_oam_global_t* p_tp_y1731_glb)
{
    /*************************************************
    *init global param
    *************************************************/
    if (NULL == g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*************************************************
    *init callback fun
    *************************************************/

    /* check*/
    SYS_OAM_CHECK_FUNC_TABLE(lchip, oam_type, SYS_OAM_CHECK_MAID)     = _sys_usw_tp_y1731_maid_check_param;

    /* op*/
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_ADD)    = _sys_usw_oam_com_add_maid;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_DEL)    = _sys_usw_oam_com_remove_maid;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_ADD)    = _sys_usw_tp_y1731_add_lmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_DEL)    = _sys_usw_tp_y1731_remove_lmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_UPDATE) = _sys_usw_tp_y1731_update_lmep;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_ADD)    = _sys_usw_tp_y1731_add_rmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_DEL)    = _sys_usw_tp_y1731_remove_rmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_UPDATE) = _sys_usw_tp_y1731_update_rmep;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_ADD)    = _sys_usw_tp_y1731_add_mip;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_DEL)    = _sys_usw_tp_y1731_remove_mip;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_STATS, SYS_OAM_CONFIG)    = _sys_usw_tp_y1731_get_stats_info;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_GLOB, SYS_OAM_GET_INFO) = _sys_usw_tp_y1731_get_mep_info;

    return CTC_E_NONE;

}

#endif
