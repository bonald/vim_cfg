#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_oam_cfm.c

 @date 2011-11-9

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
#include "ctc_parser.h"
#include "ctc_register.h"
#include "sys_usw_chip.h"

#include "sys_usw_oam.h"
#include "sys_usw_oam_com.h"
#include "sys_usw_oam_cfm_db.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_cfm.h"
#include "sys_usw_ftm.h"

#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];
extern int32 sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value);
/****************************************************************************
*
* Function
*
***************************************************************************/

STATIC int32
_sys_usw_cfm_maid_check_param(uint8 lchip, void* p_maid_param, bool is_add)
{
    ctc_oam_maid_t* p_eth_maid = NULL;
    uint8 maid_entry_num = 0;

    CTC_PTR_VALID_CHECK(p_maid_param);
    p_eth_maid = (ctc_oam_maid_t*)p_maid_param;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Maid: '%s', len: 0x%x !!\n",
                      p_eth_maid->maid, p_eth_maid->maid_len);

    /* get the entry number and check the length and length type */
    switch (g_oam_master[lchip]->maid_len_format)
    {
    case CTC_OAM_MAID_LEN_16BYTES:
        maid_entry_num = 2;
        break;

    case CTC_OAM_MAID_LEN_32BYTES:
        maid_entry_num = 4;
        break;

    case CTC_OAM_MAID_LEN_48BYTES:
        maid_entry_num = 6;
        break;

    default:
        break;
    }

    if (0 == g_oam_master[lchip]->maid_len_format)
    {
        if ((p_eth_maid->maid_len == 0) || (p_eth_maid->maid_len > CTC_OAM_MAX_MAID_LEN))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Invalid maid_len: 0x%x\n", p_eth_maid->maid_len);
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        if (p_eth_maid->maid_len > maid_entry_num * 8)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Invalid maid_len: 0x%x\n", p_eth_maid->maid_len);
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_chan_check_param(uint8 lchip, ctc_oam_lmep_t* p_chan_param, bool is_add)
{
    uint32 gport       = 0;
    uint8  is_up       = 0;
    uint8  is_link_oam = 0;
    uint8 is_l2vpn_oam = 0;
    uint32 entry_num = 0;
    uint32 mode = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_chan_param);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Chan: gport->0x%x, vlan->%d, flag->0x%x!!\n",
                      p_chan_param->key.u.eth.gport, p_chan_param->key.u.eth.vlan_id, p_chan_param->key.flag);

    gport = p_chan_param->key.u.eth.gport;
    is_link_oam = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    is_up = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    is_l2vpn_oam = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_L2VPN);

    SYS_GLOBAL_PORT_CHECK(gport);

    if (is_link_oam)
    {
        /* gport must be local physical port */
        if (CTC_IS_LINKAGG_PORT(gport))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
            return CTC_E_INVALID_PORT;
        }

        if (is_up)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Lookup channel isnt down mep channel \n");
            return CTC_E_INVALID_CONFIG;
        }
    }
    else
    {   /* check vlan id if not link oam */
        if (p_chan_param->key.u.eth.vlan_id)
        {
            CTC_VLAN_RANGE_CHECK(p_chan_param->key.u.eth.vlan_id);
        }

        if (p_chan_param->key.u.eth.cvlan_id)
        {
            CTC_VLAN_RANGE_CHECK(p_chan_param->key.u.eth.cvlan_id);
        }
    }

    if (is_l2vpn_oam)
    {

        if (CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_VPLS))
        {
            if (p_chan_param->key.u.eth.l2vpn_oam_id > 0x1FFF || (!p_chan_param->key.u.eth.l2vpn_oam_id))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid VPLS fid \n");
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            sys_usw_ftm_query_table_entry_num(0, DsMac_t, &entry_num);

            if (entry_num < 0x3000)/*12K*/
            {
                return CTC_E_NOT_SUPPORT;
            }
            if (p_chan_param->key.u.eth.l2vpn_oam_id > 0xFFF || (!p_chan_param->key.u.eth.l2vpn_oam_id))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid VPWS fid \n");
                return CTC_E_INVALID_PARAM;
            }
            sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
            if (mode && DRV_IS_DUET2(lchip) && p_chan_param->key.u.eth.l2vpn_oam_id > 0x7FF)
            {
                return CTC_E_INVALID_PARAM;
            }
        }

    }

    if (is_add)
    {
        SYS_GLOBAL_CHIPID_CHECK(p_chan_param->u.y1731_lmep.master_gchip);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_lmep_check_param(uint8 lchip, ctc_oam_lmep_t* p_lmep_param, bool is_add)
{
    ctc_oam_y1731_lmep_t* p_eth_lmep = NULL;
    uint32 max_mep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);

    p_eth_lmep = &(p_lmep_param->u.y1731_lmep);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"Lmep: gport->0x%x, vlan->%d, flag->0x%x!!\n",
                      p_lmep_param->key.u.eth.gport, p_lmep_param->key.u.eth.vlan_id, p_lmep_param->key.flag);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"Lmep: level->%d !!\n", p_lmep_param->key.u.eth.md_level);

    SYS_GLOBAL_PORT_CHECK(p_lmep_param->key.u.eth.gport)
    /* check md level */
    if (p_lmep_param->key.u.eth.md_level > SYS_OAM_MAX_MD_LEVEL)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Md level is invalid  \n");
        return CTC_E_INVALID_PARAM;
    }


    if (is_add)
    {

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: mep id->%d, interval->%d, ethTypeIndex->%d!!\n",
                          p_eth_lmep->mep_id, p_eth_lmep->ccm_interval, p_eth_lmep->tpid_index);

        /* check tipid */
        if (p_eth_lmep->tpid_index < CTC_PARSER_L2_TPID_SVLAN_TPID_0
            || p_eth_lmep->tpid_index > CTC_PARSER_L2_TPID_SVLAN_TPID_3)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Mep tpid index is invalid \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check ccm interval */
        if (p_eth_lmep->ccm_interval < SYS_OAM_MIN_CCM_INTERVAL
            || p_eth_lmep->ccm_interval > SYS_OAM_MAX_CCM_INTERVAL)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Mep ccm interval is invalid  \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check mep id */
        if (p_eth_lmep->mep_id < SYS_OAM_MIN_MEP_ID
            || p_eth_lmep->mep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Mep id is invalid  \n");
            return CTC_E_INVALID_PARAM;
        }

        /* check lm value*/
        if (CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
        {
            if(p_lmep_param->u.y1731_lmep.lm_type >= CTC_OAM_LM_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Lm type is invalid  \n");
                return CTC_E_INVALID_PARAM;
            }

            if(p_lmep_param->u.y1731_lmep.lm_cos_type >= CTC_OAM_LM_COS_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Lm cos type is invalid  \n");
                return CTC_E_INVALID_PARAM;
            }

            CTC_COS_RANGE_CHECK(p_lmep_param->u.y1731_lmep.lm_cos);
        }

        /* check csf value*/
        if (CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN))
        {
            if(p_lmep_param->u.y1731_lmep.tx_csf_type >= CTC_OAM_CSF_TYPE_MAX)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  CSF type is invalid  \n");
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
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Oam mep index is invalid  \n");
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_rmep_check_param(uint8 lchip, ctc_oam_rmep_t* p_rmep_param, bool is_add)
{
    ctc_oam_y1731_rmep_t* p_eth_rmep = NULL;
    uint32 max_mep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: gport->0x%x, vlan->%d, flag->0x%x!!\n",
                      p_rmep_param->key.u.eth.gport, p_rmep_param->key.u.eth.vlan_id, p_rmep_param->key.flag);

    p_eth_rmep = &(p_rmep_param->u.y1731_rmep);
    SYS_GLOBAL_PORT_CHECK(p_rmep_param->key.u.eth.gport);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: level->%d !!\n", p_rmep_param->key.u.eth.md_level);

    /* check md level */
    if (p_rmep_param->key.u.eth.md_level > SYS_OAM_MAX_MD_LEVEL)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Md level is invalid\n");
        return CTC_E_INVALID_PARAM;
    }


    if (is_add)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: rmep id->%d !!\n", p_eth_rmep->rmep_id);

        /* check mep id */
        if (p_eth_rmep->rmep_id < SYS_OAM_MIN_MEP_ID
            || p_eth_rmep->rmep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Mep id is invalid\n");
            return CTC_E_INVALID_PARAM;
        }

        /* check mep index*/
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            max_mep_index = _sys_usw_oam_get_mep_entry_num(lchip);
            if ((p_rmep_param->rmep_index < SYS_OAM_MIN_MEP_INDEX)
                || (p_rmep_param->rmep_index >= max_mep_index))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Oam mep index is invalid \n");
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_check_update_param(uint8 lchip, ctc_oam_update_t* p_update_param, bool is_local)
{
    ctc_oam_update_t* p_eth_update = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_update_param);

    p_eth_update = p_update_param;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Lmep: gport->0x%x, vlan->%d, flag->0x%x!!\n",
                      p_eth_update->key.u.eth.gport, p_eth_update->key.u.eth.vlan_id, p_eth_update->key.flag);

    SYS_GLOBAL_PORT_CHECK(p_eth_update->key.u.eth.gport);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: level->%d !!\n", p_eth_update->key.u.eth.md_level);

    /* check is local or remote*/
    if (p_eth_update->is_local != is_local)
    {
        return CTC_E_INVALID_PARAM;
    }

    /* check md level */
    if (p_eth_update->is_local && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS != p_eth_update->update_type)
        && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_IF_STATUS != p_eth_update->update_type)
        && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE != p_eth_update->update_type)
        && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE != p_eth_update->update_type)
        && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS != p_eth_update->update_type)
        && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT != p_eth_update->update_type))
    {
        if (p_eth_update->key.u.eth.md_level > SYS_OAM_MAX_MD_LEVEL)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Md level is invalid\n");
            return CTC_E_INVALID_PARAM;
        }
    }

    if (!is_local)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Rmep: rmep id->%d !!\n", p_eth_update->rmep_id);

        /* check mep id */
        if (p_eth_update->rmep_id < SYS_OAM_MIN_MEP_ID
            || p_eth_update->rmep_id > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Mep id is invalid \n");
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        if (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP == p_eth_update->update_type)
        {
            SYS_GLOBAL_CHIPID_CHECK(p_eth_update->update_value);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_mip_check_param(uint8 lchip, ctc_oam_mip_t* p_mip_param, bool is_add)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_mip_param);

    SYS_GLOBAL_PORT_CHECK(p_mip_param->key.u.eth.gport);

    /* check vlan id if not link oam */
    CTC_VLAN_RANGE_CHECK(p_mip_param->key.u.eth.vlan_id);

    if (p_mip_param->key.u.eth.md_level > SYS_OAM_MAX_MD_LEVEL)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Md level is invalid\n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_cfm_add_chan(uint8 lchip, void* p_chan, void** p_sys_chan)
{
    ctc_oam_lmep_t* p_chan_param   = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    int32 ret = CTC_E_NONE;

    /*1. check chan param valid*/
    CTC_ERROR_RETURN(_sys_usw_cfm_chan_check_param(lchip, p_chan_param, TRUE));

    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_chan_param->key);
    if (NULL != p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: MEP or MIP lookup key exists!!\n");
        ret = CTC_E_EXIST;
        *p_sys_chan = p_sys_chan_eth;
        goto error;
    }

    /*3. build chan sys node*/
    p_sys_chan_eth = _sys_usw_cfm_chan_build_node(lchip, p_chan_param);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    /*4. build chan index*/
    ret = (!CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)) ?
         _sys_usw_cfm_chan_build_index(lchip, p_sys_chan_eth) : ret;
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: build index fail!!\n");
        goto STEP_3;
    }


    /*5. add chan to db*/
    ret = _sys_usw_cfm_chan_add_to_db(lchip, p_sys_chan_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: add db fail!!\n");
        goto STEP_4;
    }

    /*6. add key/chan to asic*/
    ret = (!CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)) ?
        _sys_usw_cfm_chan_add_to_asic(lchip, p_sys_chan_eth) : ret;
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: add asic fail!!\n");
        goto STEP_5;
    }

    *p_sys_chan = p_sys_chan_eth;

    goto error;

STEP_5:
    _sys_usw_cfm_chan_del_from_db(lchip, p_sys_chan_eth);
STEP_4:
    _sys_usw_cfm_chan_free_index(lchip, p_sys_chan_eth);
STEP_3:
    _sys_usw_cfm_chan_free_node(lchip, p_sys_chan_eth);

error:
    return ret;
}

STATIC int32
_sys_usw_cfm_remove_chan(uint8 lchip, void* p_chan)
{
    ctc_oam_lmep_t* p_chan_param = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    int32 ret = CTC_E_NONE;

    /*1. check param valid*/
    CTC_ERROR_RETURN(_sys_usw_cfm_chan_check_param(lchip, p_chan_param, FALSE));

    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_chan_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: MEP or MIP lookup key not found!!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if ((CTC_SLISTCOUNT(p_sys_chan_eth->com.lmep_list) > 0) || p_sys_chan_eth->mip_bitmap
        || p_sys_chan_eth->down_mep_bitmap || p_sys_chan_eth->up_mep_bitmap)
    {
        ret = CTC_E_EXIST;
        goto error;
    }

    /*3. remove key/chan from asic*/
    ret = (!CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)) ?
        _sys_usw_cfm_chan_del_from_asic(lchip, p_sys_chan_eth) : ret;
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del asic fail!!\n");
    }

    /*4. remove chan from db*/
    ret = _sys_usw_cfm_chan_del_from_db(lchip, p_sys_chan_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del db fail!!\n");
    }

    /*6. free chan node*/
    ret = _sys_usw_cfm_chan_free_node(lchip, p_sys_chan_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: free index fail!!\n");
    }

error:
    return ret;
}

STATIC int32
_sys_usw_cfm_add_lmep(uint8 lchip, void* p_lmep)
{
    ctc_oam_lmep_t* p_lmep_param = (ctc_oam_lmep_t*)p_lmep;
    sys_oam_maid_com_t* p_sys_maid_eth = NULL;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    int32            ret = CTC_E_NONE;
    uint8            md_level = 0;
    uint32          session_type = SYS_OAM_SESSION_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (lmep)*/

    ret = _sys_usw_cfm_lmep_check_param(lchip, p_lmep_param, TRUE);
    if (CTC_E_NONE != ret)
    {
        goto error;
    }

    /*1. check chan exist or add chan */
    ret = _sys_usw_cfm_add_chan(lchip, p_lmep_param, (void**)&p_sys_chan_eth);
    if ((CTC_E_NONE != ret) && (CTC_E_EXIST != ret))
    {
        goto error;
    }

    if (NULL == p_sys_chan_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*2. lookup maid and check exist*/
    p_lmep_param->maid.mep_type = p_lmep_param->key.mep_type;
    p_sys_maid_eth = _sys_usw_oam_maid_lkup(lchip, &p_lmep_param->maid);
    if ((NULL == p_sys_maid_eth) && (!g_oam_master[lchip]->no_mep_resource))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Maid entry not found!!\n");
        ret = CTC_E_NOT_EXIST;
        goto STEP_4;
    }

    /*3. lookup chan and check exist*/
    md_level = p_lmep_param->key.u.eth.md_level;
    if (IS_BIT_SET((p_sys_chan_eth->down_mep_bitmap), md_level) ||
        IS_BIT_SET((p_sys_chan_eth->up_mep_bitmap), md_level) ||
        IS_BIT_SET((p_sys_chan_eth->mip_bitmap), md_level))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MD level already config other!!\n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    if ((CTC_SLISTCOUNT(p_sys_chan_eth->com.lmep_list) >= SYS_OAM_MAX_MEP_NUM_PER_CHAN)
        ||((CTC_FLAG_ISSET(p_lmep_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))&&(CTC_SLISTCOUNT(p_sys_chan_eth->com.lmep_list) >= 1)))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lookup key entry full in EVC!!\n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup lmep and check exist*/

    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL != p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Local mep already exists!!\n");
        ret = CTC_E_EXIST;
        goto error;
    }

    /*5. build lmep sys node*/
    p_sys_lmep_eth = _sys_usw_cfm_lmep_build_node(lchip, p_lmep_param);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto STEP_4;
    }

    p_sys_lmep_eth->p_sys_maid = p_sys_maid_eth;
    p_sys_lmep_eth->p_sys_chan = &p_sys_chan_eth->com;
    if (p_sys_maid_eth && (!p_sys_lmep_eth->mep_on_cpu))
    {
        p_sys_maid_eth->ref_cnt++;
    }
    /*6. build lmep index */
    ret = _sys_usw_cfm_lmep_build_index(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: build index fail!!\n");
        goto STEP_5;
    }

    /*7. add lmep to db */
    ret = _sys_usw_cfm_lmep_add_to_db(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: add db fail!!\n");
        goto STEP_6;
    }

    /*8. write lmep asic*/
    ret = _sys_usw_cfm_lmep_add_to_asic(lchip, p_lmep_param, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: add asic fail!!\n");
        goto STEP_7;
    }

    /*9. build lm index*/
    ret = _sys_usw_cfm_lmep_build_lm_index(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: build lm index fail!!\n");
        goto STEP_8;
    }

    /*10. update chan*/
    ret = (!CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)) ?
        _sys_usw_cfm_chan_update(lchip, p_sys_lmep_eth, TRUE) : ret;
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: update chan fail!!\n");
        goto STEP_9;
    }

    sys_usw_oam_get_session_type(lchip, p_sys_lmep_eth, &session_type);
    SYS_OAM_SESSION_CNT(lchip, session_type, 1);

    /*11. copy mep index*/
    p_lmep_param->lmep_index = p_sys_lmep_eth->lmep_index;

    goto error;

STEP_9:
    _sys_usw_cfm_lmep_free_lm_index(lchip, p_sys_lmep_eth);
STEP_8:
    _sys_usw_cfm_lmep_del_from_asic(lchip, p_sys_lmep_eth);
STEP_7:
    _sys_usw_cfm_lmep_del_from_db(lchip, p_sys_lmep_eth);
STEP_6:
    _sys_usw_cfm_lmep_free_index(lchip, p_sys_lmep_eth);
STEP_5:
    if (p_sys_maid_eth && (!p_sys_lmep_eth->mep_on_cpu))
    {
        p_sys_maid_eth->ref_cnt--;
    }
    _sys_usw_cfm_lmep_free_node(lchip, p_sys_lmep_eth);
STEP_4:
     _sys_usw_cfm_remove_chan(lchip, p_lmep_param);

error:

    return ret;
}

STATIC int32
_sys_usw_cfm_remove_lmep(uint8 lchip, void* p_lmep)
{
    ctc_oam_lmep_t* p_lmep_param = (ctc_oam_lmep_t*)p_lmep;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    sys_oam_maid_com_t* p_sys_maid     = NULL;
    int32           ret = CTC_E_NONE;
    uint8           md_level = 0;
    uint32          lmep_index = 0;
    uint32          session_type = SYS_OAM_SESSION_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (lmep)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_lmep_check_param(lchip, p_lmep_param, FALSE));

    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    md_level = p_lmep_param->key.u.eth.md_level;

    if (!IS_BIT_SET((p_sys_chan_eth->down_mep_bitmap), md_level) &&
        !IS_BIT_SET((p_sys_chan_eth->up_mep_bitmap), md_level) &&
        !IS_BIT_SET((p_sys_chan_eth->mip_bitmap), md_level) &&
        !CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MD level not exist !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/

    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Local mep not found !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*4. check if rmep list */
    if (CTC_SLISTCOUNT(p_sys_lmep_eth->rmep_list) > 0)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Remote mep already exist !!\n");
        ret = CTC_E_EXIST;
        goto error;
    }

    sys_usw_oam_get_session_type(lchip, p_sys_lmep_eth, &session_type);
    p_sys_maid = p_sys_lmep_eth->p_sys_maid;
    lmep_index = p_sys_lmep_eth->lmep_index;


    /*5. free lm index */
    ret = _sys_usw_cfm_lmep_free_lm_index(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: free lm index fail!!\n");
    }


    /*6. update chan*/
    ret = (!CTC_FLAG_ISSET(p_lmep_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)) ?
        _sys_usw_cfm_chan_update(lchip, p_sys_lmep_eth, FALSE) : ret;
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: update chan fail!!\n");
    }

    /*7. remove lmep from asic*/
    ret = _sys_usw_cfm_lmep_del_from_asic(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: del asic fail!!\n");
    }

    /*8. remove lmep from db*/
    ret = _sys_usw_cfm_lmep_del_from_db(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: del db fail!!\n");
    }

    /*9. free lmep index*/
    ret = _sys_usw_cfm_lmep_free_index(lchip, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Lmep: free index fail!!\n");
    }

    /*10. free lmep node*/
    if (p_sys_maid && (!p_sys_lmep_eth->mep_on_cpu))
    {
        p_sys_maid->ref_cnt--;
    }
    _sys_usw_cfm_lmep_free_node(lchip, p_sys_lmep_eth);

    /*11. update chan exist or remove chan */
    ret = _sys_usw_cfm_remove_chan(lchip, p_lmep_param);
    if ((CTC_E_EXIST == ret) || (CTC_E_NOT_EXIST == ret))
    {
        ret = CTC_E_NONE;
    }

    p_lmep_param->lmep_index = lmep_index;

    SYS_OAM_SESSION_CNT(lchip, session_type, 0);
    if ((0x1FFF != lmep_index) && g_oam_master[lchip]->mep_defect_bitmap[lmep_index])
    {
        sal_memset(&g_oam_master[lchip]->mep_defect_bitmap[lmep_index], 0, 4);
    }

error:

    return ret;
}

STATIC int32
_sys_usw_cfm_update_lmep_assigned_level(uint8 lchip, void* p_lmep)
{
    ctc_oam_update_t* p_lmep_param = (ctc_oam_update_t*)p_lmep;

    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    uint8            md_level = 0;
    int32            ret = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    /*2. lookup lmep and check exist*/
    md_level = p_lmep_param->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
        return CTC_E_NOT_EXIST;

    }

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on mep configured for cpu \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*3. check p2p mode not support update */
    if ((!CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
        && ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA == p_lmep_param->update_type)))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on p2p mep configured\n");
        return CTC_E_INVALID_CONFIG;
    }

    /*4. update lmep db and asic*/
    ret = _sys_usw_cfm_lmep_update_asic(lchip, p_lmep_param, p_sys_lmep_eth);

    return ret;
}

STATIC int32
_sys_usw_cfm_update_lmep(uint8 lchip, void* p_lmep)
{
    ctc_oam_update_t* p_lmep_update = (ctc_oam_update_t*)p_lmep;

    int32  ret = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_cfm_check_update_param(lchip, p_lmep_update, TRUE));

    if (!p_lmep_update->is_local)
    {
        ret = CTC_E_INVALID_PARAM;
    }

    switch (p_lmep_update->update_type)
    {
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
            ret = _sys_usw_cfm_update_lmep_assigned_level(lchip, (void*)p_lmep_update);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
            CTC_ERROR_RETURN(_sys_usw_cfm_maid_check_param(lchip, (void*)(p_lmep_update->p_update_value), TRUE));
            ret = _sys_usw_cfm_update_lmep_assigned_level(lchip, (void*)p_lmep_update);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS:
            CTC_MAX_VALUE_CHECK(p_lmep_update->update_value, 2);
            ret = _sys_usw_cfm_update_lmep_port_status(lchip, p_lmep_update);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_IF_STATUS:
            CTC_MAX_VALUE_CHECK(p_lmep_update->update_value, 7);
            ret = _sys_usw_cfm_update_lmep_if_status(lchip, p_lmep_update);
            break;

        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS:
            ret = _sys_usw_cfm_update_lmep_lm(lchip, p_lmep_update);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT:
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM:
            ret = _sys_usw_cfm_update_lmep_npm(lchip, p_lmep_update);
            break;
        case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP:
            ret = _sys_usw_cfm_update_master_gchip(lchip, p_lmep_update);
            break;

        default:
            ret = CTC_E_INVALID_PARAM;
    }

    return ret;
}

STATIC int32
_sys_usw_cfm_add_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_rmep_t* p_rmep_param = (ctc_oam_rmep_t*)p_rmep;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    sys_oam_rmep_t* p_sys_rmep_eth = NULL;
    int32 ret           = CTC_E_NONE;
    uint8 md_level      = 0;
    uint32 rmep_id      = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (rmep)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_rmep_check_param(lchip, p_rmep_param, TRUE));



    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_rmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MEP or MIP lookup key not found !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    md_level = p_rmep_param->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Local mep not found !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if ((!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        && CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        if (1 != (p_rmep_param->rmep_index - p_sys_lmep_eth->lmep_index))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Oam mep index is invalid \n");
            return CTC_E_INVALID_PARAM;
        }
    }
    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU)
        || CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_param->u.y1731_rmep.rmep_id;
    p_sys_rmep_eth = _sys_usw_cfm_rmep_lkup(lchip, rmep_id, p_sys_lmep_eth);
    if (NULL != p_sys_rmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Remote mep already exists !!\n");
        ret = CTC_E_EXIST;
        goto error;
    }

    if (((CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)) || (p_sys_chan_eth->key.link_oam))
        &&(CTC_SLISTCOUNT(p_sys_lmep_eth->rmep_list) > 0))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "P2P mode or linkoam can't have more than one rmep\n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*5. build rmep sys node*/
    p_sys_rmep_eth = _sys_usw_cfm_rmep_build_node(lchip, p_rmep_param);
    if (NULL == p_sys_rmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }
    p_sys_rmep_eth->p_sys_lmep = p_sys_lmep_eth;

    /*6. build rmep index */
    ret = _sys_usw_cfm_rmep_build_index(lchip, p_sys_rmep_eth, p_sys_lmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: build index fail!!\n");
        goto STEP_5;
    }

    /*7. add rmep to db */
    ret = _sys_usw_cfm_rmep_add_to_db(lchip, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: add db fail!!\n");
        goto STEP_6;
    }
    /*8. write rmep asic*/
    ret = _sys_usw_cfm_rmep_add_to_asic(lchip, p_rmep_param, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: add asic fail!!\n");
        goto STEP_7;
    }

    /*9. copy mep index*/
    p_rmep_param->rmep_index = p_sys_rmep_eth->rmep_index;

    goto error;

STEP_7:
    _sys_usw_cfm_rmep_del_from_db(lchip, p_sys_rmep_eth);
STEP_6:
    _sys_usw_cfm_rmep_free_index(lchip, p_sys_rmep_eth);
STEP_5:
    _sys_usw_cfm_rmep_free_node(lchip, p_sys_rmep_eth);

error:

    return ret;
}

STATIC int32
_sys_usw_cfm_remove_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_rmep_t* p_rmep_param = (ctc_oam_rmep_t*)p_rmep;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    sys_oam_rmep_t* p_sys_rmep_eth = NULL;
    int32 ret           = CTC_E_NONE;
    uint8 md_level  = 0;
    uint32 rmep_id  = 0;
    uint32 rmep_index = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (rmep)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_rmep_check_param(lchip, p_rmep_param, FALSE));



    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_rmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    md_level = p_rmep_param->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_param->u.y1731_rmep.rmep_id;
    p_sys_rmep_eth = _sys_usw_cfm_rmep_lkup(lchip, rmep_id, p_sys_lmep_eth);
    if (NULL == p_sys_rmep_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    rmep_index = p_sys_rmep_eth->rmep_index;
    /*5. remove rmep from asic*/
    ret = _sys_usw_cfm_rmep_del_from_asic(lchip, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: del asic fail!!\n");
    }

    /*6. remove rmep from db*/
    ret = _sys_usw_cfm_rmep_del_from_db(lchip, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: del db fail!!\n");
    }

    /*7. free rmep index*/
    ret = _sys_usw_cfm_rmep_free_index(lchip, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: free index fail!!\n");
    }

    /*8. free rmep node*/
    _sys_usw_cfm_rmep_free_node(lchip, p_sys_rmep_eth);

    if (g_oam_master[lchip]->mep_defect_bitmap[rmep_index])
    {
        sal_memset(&g_oam_master[lchip]->mep_defect_bitmap[rmep_index], 0, 4);
    }

error:
    p_rmep_param->rmep_index = rmep_index;

    return ret;
}

STATIC int32
_sys_usw_cfm_update_rmep(uint8 lchip, void* p_rmep)
{
    ctc_oam_update_t* p_rmep_update = (ctc_oam_update_t*)p_rmep;

    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    sys_oam_rmep_t* p_sys_rmep_eth = NULL;
    uint8 md_level  = 0;
    uint32 rmep_id  = 0;
    int32 ret = CTC_E_NONE;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. check param valid (chan)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_check_update_param(lchip, p_rmep_update, FALSE));



    /*2. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_rmep_update->key);
    if (NULL == p_sys_chan_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3. lookup lmep and check exist*/
    md_level = p_rmep_update->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    if ((!(CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)))
        && ((CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN == p_rmep_update->update_type)
            || (CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF == p_rmep_update->update_type)))
    {
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    rmep_id = p_rmep_update->rmep_id;
    p_sys_rmep_eth = _sys_usw_cfm_rmep_lkup(lchip, rmep_id, p_sys_lmep_eth);
    if (NULL == p_sys_rmep_eth)
    {
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*5. update rmep db and asic*/
    ret = _sys_usw_cfm_rmep_update_asic(lchip, p_rmep_update, p_sys_rmep_eth);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Rmep: update asic fail!!\n");
        goto error;
    }

error:

    return ret;
}

STATIC int32
_sys_usw_cfm_add_mip(uint8 lchip, void* p_mip)
{
    ctc_oam_mip_t* p_mip_param = NULL;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    ctc_oam_lmep_t      chan_param;

    uint8 md_level      = 0;
    int32 ret = CTC_E_NONE;

    p_mip_param = p_mip;

    if (!DRV_IS_DUET2(lchip))
    {
        /*TM mip do not need config mip key*/
        return CTC_E_NONE;
    }
    sal_memset(&chan_param, 0, sizeof(ctc_oam_lmep_t));
    sal_memcpy(&chan_param.key, &p_mip_param->key, sizeof(ctc_oam_key_t));
    chan_param.u.y1731_lmep.master_gchip = p_mip_param->master_gchip;

    md_level = p_mip_param->key.u.eth.md_level;
    /*1. check param valid (mip)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_mip_check_param(lchip, p_mip_param, TRUE));


    /*2. check & add chan*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_mip_param->key);
    if (NULL == p_sys_chan_eth)
    {
        ret = _sys_usw_cfm_add_chan(lchip, &chan_param, (void**)&p_sys_chan_eth);
        if (CTC_E_NONE != ret)
        {
            goto error;
        }
    }

    if (IS_BIT_SET((p_sys_chan_eth->down_mep_bitmap), md_level) ||
        IS_BIT_SET((p_sys_chan_eth->up_mep_bitmap), md_level) ||
        IS_BIT_SET((p_sys_chan_eth->mip_bitmap), md_level))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MD level already config other !!\n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*3.update db*/
    SET_BIT((p_sys_chan_eth->mip_bitmap), md_level);

error:
    return ret;
}

STATIC int32
_sys_usw_cfm_remove_mip(uint8 lchip, void* p_mip)
{
    ctc_oam_mip_t* p_mip_param = NULL;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    ctc_oam_lmep_t      chan_param;
    uint8 md_level      = 0;

    int32 ret = CTC_E_NONE;

    p_mip_param = p_mip;
    if (!DRV_IS_DUET2(lchip))
    {
        /*TM mip do not need config mip key*/
        return CTC_E_NONE;
    }
    sal_memset(&chan_param, 0, sizeof(ctc_oam_lmep_t));
    sal_memcpy(&chan_param.key, &p_mip_param->key, sizeof(ctc_oam_key_t));
    chan_param.u.y1731_lmep.master_gchip = p_mip_param->master_gchip;

    md_level = p_mip_param->key.u.eth.md_level;
    /*1. check param valid (mip)*/
    CTC_ERROR_RETURN(_sys_usw_cfm_mip_check_param(lchip, p_mip_param, FALSE));


    /*2. check chan*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_mip_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MEP or MIP lookup key not found !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (!IS_BIT_SET((p_sys_chan_eth->down_mep_bitmap), md_level) &&
        !IS_BIT_SET((p_sys_chan_eth->up_mep_bitmap), md_level) &&
        !IS_BIT_SET((p_sys_chan_eth->mip_bitmap), md_level))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"MD level not exist !!\n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    /*3.update db*/
    CLEAR_BIT((p_sys_chan_eth->mip_bitmap), md_level);

    /*4. remove chan*/
    ret = _sys_usw_cfm_remove_chan(lchip, &chan_param);
    if (CTC_E_EXIST == ret)
    {
        ret = CTC_E_NONE;
    }

error:

    return ret;

}

STATIC int32
_sys_usw_cfm_set_property(uint8 lchip, void* p_prop)
{
    ctc_oam_property_t* p_prop_param = (ctc_oam_property_t*)p_prop;
    ctc_oam_y1731_prop_t* p_eth_prop   = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_prop);
    p_eth_prop   = &p_prop_param->u.y1731;

    switch (p_eth_prop->cfg_type)
    {
    case CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_port_oam_en(lchip, p_eth_prop->gport, p_eth_prop->dir, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_port_tunnel_en(lchip, p_eth_prop->gport, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_ALL_TO_CPU:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_relay_all_to_cpu_en(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_ADD_EDGE_PORT:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_port_edge_en(lchip, p_eth_prop->gport, 1));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_REMOVE_EDGE_PORT:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_port_edge_en(lchip, p_eth_prop->gport, 0));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_TX_VLAN_TPID:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_tx_vlan_tpid(lchip, (p_eth_prop->value >> 16) & 0xFFFF, p_eth_prop->value & 0xFFFF));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_RX_VLAN_TPID:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_rx_vlan_tpid(lchip, (p_eth_prop->value >> 16) & 0xFFFF, p_eth_prop->value & 0xFFFF));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_SENDER_ID:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_send_id(lchip, (ctc_oam_eth_senderid_t*)p_eth_prop->p_value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_big_ccm_interval(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_port_lm_en(lchip, p_eth_prop->gport, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_bridge_mac(lchip, (mac_addr_t*)p_eth_prop->p_value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_lbm_process_by_asic(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_lbr_sa_use_lbm_da(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_SHARE_MAC:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_lbr_sa_use_bridge_mac(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_lmm_process_by_asic(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_dm_process_by_asic(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_slm_process_by_asic(lchip, p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE:
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 0xFF); /*level bitmap*/
        CTC_ERROR_RETURN(
            _sys_usw_cfm_set_mip_port_en(lchip, p_eth_prop->gport, p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_IF_STATUS:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 7);
        CTC_ERROR_RETURN(
            _sys_usw_cfm_set_if_status(lchip, p_eth_prop->gport, p_eth_prop->value));

        break;

    case CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_set_ach_channel_type(lchip, (uint16)p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_RDI_MODE:
        CTC_MAX_VALUE_CHECK(p_eth_prop->value, 1);
        CTC_ERROR_RETURN(_sys_usw_cfm_oam_set_rdi_mode(lchip, p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN:
        CTC_ERROR_RETURN(_sys_usw_cfm_oam_set_ccm_lm_en(lchip, p_eth_prop->value));
        break;
    default:
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
		return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_get_property(uint8 lchip, void* p_prop)
{
    ctc_oam_property_t* p_prop_param = (ctc_oam_property_t*)p_prop;
    ctc_oam_y1731_prop_t* p_eth_prop   = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_prop);
    p_eth_prop   = &p_prop_param->u.y1731;

    switch (p_eth_prop->cfg_type)
    {
    case CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_port_oam_en(lchip, p_eth_prop->gport, p_eth_prop->dir, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_port_tunnel_en(lchip, p_eth_prop->gport, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_big_ccm_interval(lchip, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_port_lm_en(lchip, p_eth_prop->gport, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_lbm_process_by_asic(lchip, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_lmm_process_by_asic(lchip, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_dm_process_by_asic(lchip, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_slm_process_by_asic(lchip, (uint8*)&p_eth_prop->value));
        break;

    case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_lbr_sa_use_lbm_da(lchip, (uint8*)&p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_mip_port_en(lchip, p_eth_prop->gport, (uint8*)&p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE:
        break;
    case CTC_OAM_Y1731_CFG_TYPE_IF_STATUS:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_if_status(lchip, p_eth_prop->gport, (uint8*)&p_eth_prop->value));
         break;
    case CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE:
        CTC_ERROR_RETURN(
        _sys_usw_cfm_oam_get_ach_channel_type(lchip, (uint16*)&p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_RDI_MODE:
        CTC_ERROR_RETURN(
            _sys_usw_cfm_oam_get_rdi_mode(lchip, &p_eth_prop->value));
        break;
    case CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN:
        CTC_ERROR_RETURN(_sys_usw_cfm_oam_get_ccm_lm_en(lchip, (uint32*)&p_eth_prop->value));
        break;
    default:
        break;

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_cfm_get_stats_info(uint8 lchip, void* p_stat_info)
{
    ctc_oam_stats_info_t* p_sys_stat = NULL;

    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    uint8            md_level = 0;
    sys_oam_lm_com_t sys_oam_lm_com;
    uint8   lm_type         = 0;
    uint8   lm_cos_type     = 0;
    uint32  lm_index        = 0;
    uint8   cnt             = 0;
    uint8   lvl             = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_stat_info);

    p_sys_stat = (ctc_oam_stats_info_t*)p_stat_info;

    /*1. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_sys_stat->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
		return CTC_E_NOT_EXIST;
    }

    /*2. lookup lmep and check exist*/
    md_level = p_sys_stat->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
		return CTC_E_NOT_EXIST;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on mep configured for cpu \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*3. get mep lm index base, lm type, lm cos type, lm cos*/
    if (!CTC_IS_BIT_SET(p_sys_chan_eth->lm_bitmap, md_level))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  LM not enbale in MEP\n");
	    return CTC_E_NOT_READY;
    }

    if (CTC_FLAG_ISSET(p_sys_stat->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
    {
        lm_cos_type = p_sys_chan_eth->link_lm_cos_type;
        lm_type     = p_sys_chan_eth->link_lm_type;
        lm_index    = p_sys_chan_eth->com.link_lm_index_base;
    }
    else
    {
        lm_cos_type = p_sys_chan_eth->lm_cos_type[md_level];
        lm_type     = p_sys_chan_eth->lm_type;
        lm_index     = p_sys_chan_eth->lm_index[md_level];

        for (lvl = 1; lvl <= md_level; lvl++)
        {
            if ((CTC_IS_BIT_SET(p_sys_chan_eth->down_mep_bitmap, lvl))
                ||(CTC_IS_BIT_SET(p_sys_chan_eth->up_mep_bitmap, lvl)))
            {
                cnt++;
            }
        }

        if (cnt > 3)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  LM entry full in EVC\n");
			return CTC_E_INVALID_CONFIG;
        }
    }

    sys_oam_lm_com.lm_cos_type  = lm_cos_type;
    sys_oam_lm_com.lm_index     = lm_index;
    sys_oam_lm_com.lm_type      = lm_type;
    _sys_usw_oam_get_stats_info(lchip, &sys_oam_lm_com, p_stat_info);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_cfm_get_mep_info(uint8 lchip, void* p_mep_info)
{
    ctc_oam_mep_info_with_key_t* p_mep_param = (ctc_oam_mep_info_with_key_t*)p_mep_info;

    sys_oam_chan_eth_t*     p_sys_chan  = NULL;
    sys_oam_lmep_t*   p_sys_lmep  = NULL;
    sys_oam_rmep_t*   p_sys_rmep  = NULL;
    int32 ret = CTC_E_NONE;
    ctc_oam_mep_info_t mep_info;
    sal_memset(&mep_info, 0, sizeof(ctc_oam_mep_info_t));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);



    /*2. lookup chan and check exist*/
    p_sys_chan = _sys_usw_cfm_chan_lkup(lchip, &p_mep_param->key);
    if (NULL == p_sys_chan)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_chan->com.master_chipid))
    {
        ret = CTC_E_NOT_EXIST;  /*check master_chipid for Multichip*/
        goto error;
    }

    /*3. lookup lmep and check exist*/
    p_sys_lmep = _sys_usw_cfm_lmep_lkup(lchip, p_mep_param->key.u.eth.md_level, p_sys_chan);
    if (NULL == p_sys_lmep)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
        ret = CTC_E_NOT_EXIST;
        goto error;
    }

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Invalid operation on mep configured for cpu \n");
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    /*4. lookup rmep and check exist*/
    p_sys_rmep = _sys_usw_cfm_rmep_lkup(lchip, p_mep_param->rmep_id, p_sys_lmep);
    if (NULL == p_sys_rmep)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Remote mep not found \n");
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
sys_usw_cfm_init(uint8 lchip, uint8 oam_type, ctc_oam_global_t* p_eth_glb)
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
    /*check*/
    SYS_OAM_CHECK_FUNC_TABLE(lchip, oam_type, SYS_OAM_CHECK_MAID)     = _sys_usw_cfm_maid_check_param;

    /*op*/
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_ADD)    = _sys_usw_oam_com_add_maid;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MAID, SYS_OAM_DEL)    = _sys_usw_oam_com_remove_maid;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_ADD)    = _sys_usw_cfm_add_lmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_DEL)    = _sys_usw_cfm_remove_lmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_LMEP, SYS_OAM_UPDATE) = _sys_usw_cfm_update_lmep;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_ADD)    = _sys_usw_cfm_add_rmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_DEL)    = _sys_usw_cfm_remove_rmep;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_RMEP, SYS_OAM_UPDATE) = _sys_usw_cfm_update_rmep;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_ADD)    = _sys_usw_cfm_add_mip;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_MIP, SYS_OAM_DEL)    = _sys_usw_cfm_remove_mip;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_STATS, SYS_OAM_CONFIG)    = _sys_usw_cfm_get_stats_info;
    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_GLOB, SYS_OAM_GET_INFO)    = _sys_usw_cfm_get_mep_info;

    SYS_OAM_FUNC_TABLE(lchip, CTC_OAM_PROPERTY_TYPE_Y1731, SYS_OAM_GLOB, SYS_OAM_CONFIG) = _sys_usw_cfm_set_property;


    return CTC_E_NONE;
}

#endif
