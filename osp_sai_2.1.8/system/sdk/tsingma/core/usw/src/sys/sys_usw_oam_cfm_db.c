#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_cfm_db.c

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
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "ctc_parser.h"
#include "ctc_packet.h"
#include "ctc_register.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"
#include "sys_usw_port.h"
#include "sys_usw_mpls.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_cfm_db.h"
#include "sys_usw_oam_cfm.h"
#include "sys_usw_oam_com.h"

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
**
***************************************************************************/

#define CHAN "Function Begin"


sys_oam_chan_eth_t*
_sys_usw_cfm_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm)
{
    sys_oam_chan_eth_t sys_oam_chan_eth;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_chan_eth, 0, sizeof(sys_oam_chan_eth_t));
    sys_oam_chan_eth.key.com.mep_type = p_key_parm->mep_type;
    sys_oam_chan_eth.key.gport        = p_key_parm->u.eth.gport;
    sys_oam_chan_eth.key.use_fid      = CTC_FLAG_ISSET(p_key_parm->flag, CTC_OAM_KEY_FLAG_L2VPN);
    sys_oam_chan_eth.key.link_oam     = CTC_FLAG_ISSET(p_key_parm->flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    sys_oam_chan_eth.key.l2vpn_oam_id      = p_key_parm->u.eth.l2vpn_oam_id;
    sys_oam_chan_eth.com.mep_type     = p_key_parm->mep_type;

    if ((p_key_parm->u.eth.vlan_id != 0) && (p_key_parm->u.eth.cvlan_id != 0))
    {
        sys_oam_chan_eth.key.vlan_id = p_key_parm->u.eth.vlan_id;
        sys_oam_chan_eth.key.cvlan_id = p_key_parm->u.eth.cvlan_id;
        sys_oam_chan_eth.key.is_cvlan = 0;
    }
    else if((p_key_parm->u.eth.vlan_id != 0) && (p_key_parm->u.eth.cvlan_id == 0))
    {
        sys_oam_chan_eth.key.vlan_id = p_key_parm->u.eth.vlan_id;
        sys_oam_chan_eth.key.cvlan_id = 0;
        sys_oam_chan_eth.key.is_cvlan = 0;
    }
    else if((p_key_parm->u.eth.vlan_id == 0) && (p_key_parm->u.eth.cvlan_id != 0))
    {
        sys_oam_chan_eth.key.vlan_id = p_key_parm->u.eth.cvlan_id;
        sys_oam_chan_eth.key.cvlan_id = 0;
        sys_oam_chan_eth.key.is_cvlan = 1;
    }



    return (sys_oam_chan_eth_t*)_sys_usw_oam_chan_lkup(lchip, &sys_oam_chan_eth.com);
}


sys_oam_chan_eth_t*
_sys_usw_cfm_chan_build_node(uint8 lchip, ctc_oam_lmep_t* p_chan_param)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_eth = (sys_oam_chan_eth_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_chan_eth_t));
    if (NULL != p_sys_chan_eth)
    {
        sal_memset(p_sys_chan_eth, 0, sizeof(sys_oam_chan_eth_t));
        p_sys_chan_eth->com.mep_type = p_chan_param->key.mep_type;
        p_sys_chan_eth->com.master_chipid = p_chan_param->u.y1731_lmep.master_gchip;
        p_sys_chan_eth->key.com.mep_type = p_chan_param->key.mep_type;
        p_sys_chan_eth->key.gport = p_chan_param->key.u.eth.gport;
        p_sys_chan_eth->key.link_oam = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
        p_sys_chan_eth->key.use_fid = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_L2VPN);


        if ((p_chan_param->key.u.eth.vlan_id != 0) && (p_chan_param->key.u.eth.cvlan_id != 0))
        {
            p_sys_chan_eth->key.vlan_id = p_chan_param->key.u.eth.vlan_id;
            p_sys_chan_eth->key.cvlan_id = p_chan_param->key.u.eth.cvlan_id;
            p_sys_chan_eth->key.is_cvlan = 0;
        }
        else if((p_chan_param->key.u.eth.vlan_id != 0) && (p_chan_param->key.u.eth.cvlan_id == 0))
        {
            p_sys_chan_eth->key.vlan_id = p_chan_param->key.u.eth.vlan_id;
            p_sys_chan_eth->key.cvlan_id = 0;
            p_sys_chan_eth->key.is_cvlan = 0;
        }
        else if((p_chan_param->key.u.eth.vlan_id == 0) && (p_chan_param->key.u.eth.cvlan_id != 0))
        {
            p_sys_chan_eth->key.vlan_id = p_chan_param->key.u.eth.cvlan_id;
            p_sys_chan_eth->key.cvlan_id = 0;
            p_sys_chan_eth->key.is_cvlan = 1;
        }


        p_sys_chan_eth->key.l2vpn_oam_id = p_chan_param->key.u.eth.l2vpn_oam_id;
        if (CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_L2VPN)
            && (!CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_VPLS)))
        {
            p_sys_chan_eth->key.is_vpws = 1;

        }

        if (CTC_FLAG_ISSET(p_chan_param->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
        {
            if (CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
            {
                p_sys_chan_eth->link_lm_type     = p_chan_param->u.y1731_lmep.lm_type;
                p_sys_chan_eth->link_lm_cos_type = p_chan_param->u.y1731_lmep.lm_cos_type;
                p_sys_chan_eth->link_lm_cos      = p_chan_param->u.y1731_lmep.lm_cos;
            }
            else
            {
                p_sys_chan_eth->lm_type     = p_chan_param->u.y1731_lmep.lm_type;
            }
        }
        else
        {
            p_sys_chan_eth->lm_type     = CTC_OAM_LM_TYPE_NONE;
        }
    }

    return p_sys_chan_eth;
}

int32
_sys_usw_cfm_chan_free_node(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_chan_eth);
    p_sys_chan_eth = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_chan_build_index(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    DsEgressXcOamEthHashKey_m eth_oam_key;
    uint16 gport        = 0;
    uint8 is_fid = 0;
    uint16 vlan_id        = 0;
    uint16 vpws_oam_id_base = 2<<12;
    uint32 is_cvlan         = 0;
    uint16 cvlan_id         = 0;
    uint32 mode = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_eth);
    gport = p_sys_chan_eth->key.gport;

    if (!CTC_IS_LINKAGG_PORT(p_sys_chan_eth->key.gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_eth->key.gport))))
    {
        return CTC_E_NONE;
    }

    if (!p_sys_chan_eth->key.link_oam)
    {
        /*hash key offset alloc*/
        sal_memset(&eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
        gport =  SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);
        is_fid = p_sys_chan_eth->key.use_fid;
        if (is_fid)
        {
            if (p_sys_chan_eth->key.is_vpws && DRV_IS_DUET2(lchip))
            {
                sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
                if (mode)
                {
                    vpws_oam_id_base += 2048;
                }
                vlan_id = vpws_oam_id_base + p_sys_chan_eth->key.l2vpn_oam_id;
            }
            else
            {
                vlan_id = p_sys_chan_eth->key.l2vpn_oam_id;
            }

            cvlan_id = 0;
            is_cvlan =  0;
        }
        else
        {
            vlan_id = p_sys_chan_eth->key.vlan_id;
            cvlan_id = p_sys_chan_eth->key.cvlan_id;
            is_cvlan =  p_sys_chan_eth->key.is_cvlan;
        }


        SetDsEgressXcOamEthHashKey(V, hashType_f,  &eth_oam_key, EGRESSXCOAMHASHTYPE_ETH);
        SetDsEgressXcOamEthHashKey(V, globalSrcPort_f,   &eth_oam_key, gport);
        SetDsEgressXcOamEthHashKey(V, isFid_f,  &eth_oam_key, is_fid);
        SetDsEgressXcOamEthHashKey(V, vlanId_f,   &eth_oam_key, vlan_id);
        SetDsEgressXcOamEthHashKey(V, isCvlan_f,   &eth_oam_key, is_cvlan);
        SetDsEgressXcOamEthHashKey(V, cvlanId_f,   &eth_oam_key, cvlan_id);
        SetDsEgressXcOamEthHashKey(V, valid_f,   &eth_oam_key, 1);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.global_src_port = 0x%x\n",   gport);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.vlan_id = 0x%x\n",           vlan_id);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.cvlan_id = 0x%x\n",          cvlan_id);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.is_cvlan = 0x%x\n",          is_cvlan);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.is_fid = 0x%x\n",            is_fid);

        CTC_ERROR_RETURN( sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamEthHashKey_t,
                                                        & p_sys_chan_eth->key.com.key_index ,
                                                        &eth_oam_key ));

        p_sys_chan_eth->key.com.key_alloc = SYS_OAM_KEY_HASH;
        p_sys_chan_eth->key.com.key_exit = 1;
    }


    return CTC_E_NONE;

}

int32
_sys_usw_cfm_chan_free_index(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    DsEgressXcOamEthHashKey_m ds_eth_oam_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_chan_eth);

    if (p_sys_chan_eth->key.link_oam)
    {
        return CTC_E_NONE;
    }

    if (!CTC_IS_LINKAGG_PORT(p_sys_chan_eth->key.gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_eth->key.gport))))
    {
        return CTC_E_NONE;
    }

    if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        /*eth oam key delete*/
        sal_memset(&ds_eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
        SetDsEgressXcOamEthHashKey(V, valid_f,                      &ds_eth_oam_key, 0);
        CTC_ERROR_RETURN(sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t,
                                                          p_sys_chan_eth->key.com.key_index ,
                                                          &ds_eth_oam_key));

    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_chan_add_to_db(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_chan_add_to_db(lchip, &p_sys_chan_eth->com));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_chan_del_from_db(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_chan_del_from_db(lchip, &p_sys_chan_eth->com));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_chan_add_to_asic(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    uint16 gport      = 0;
    uint8 is_fid      = 0;
    uint16 vlan_id    = 0;
    uint16 vpws_oam_id_base = (1 << 13);/*8K*/
    DsEgressXcOamEthHashKey_m ds_eth_oam_key;
    uint32 is_cvlan = 0;
    uint16 cvlan_id    = 0;
    uint32 mode = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_eth);

    gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_sys_chan_eth->key.gport);
    is_fid = p_sys_chan_eth->key.use_fid;
    if (is_fid)
    {
        if (p_sys_chan_eth->key.is_vpws && DRV_IS_DUET2(lchip))
        {
            sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
            if (mode)
            {
                vpws_oam_id_base += 2048;
            }
            vlan_id = vpws_oam_id_base + p_sys_chan_eth->key.l2vpn_oam_id;
        }
        else
        {
            vlan_id = p_sys_chan_eth->key.l2vpn_oam_id;
        }
        cvlan_id = 0;
        is_cvlan = 0;
    }
    else
    {
        vlan_id = p_sys_chan_eth->key.vlan_id;
        cvlan_id = p_sys_chan_eth->key.cvlan_id;
        is_cvlan =  p_sys_chan_eth->key.is_cvlan;
    }

    if (!CTC_IS_LINKAGG_PORT(p_sys_chan_eth->key.gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_eth->key.gport))))
    {
        return CTC_E_NONE;
    }

    if (!p_sys_chan_eth->key.link_oam)
    {
        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            /*eth oam key write*/
            sal_memset(&ds_eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
            SetDsEgressXcOamEthHashKey(V, hashType_f,               &ds_eth_oam_key, EGRESSXCOAMHASHTYPE_ETH);
            SetDsEgressXcOamEthHashKey(V, globalSrcPort_f,          &ds_eth_oam_key, gport);
            SetDsEgressXcOamEthHashKey(V, isFid_f,                        &ds_eth_oam_key, is_fid);
            SetDsEgressXcOamEthHashKey(V, vlanId_f,                     &ds_eth_oam_key, vlan_id);
            SetDsEgressXcOamEthHashKey(V, isCvlan_f,                    &ds_eth_oam_key, is_cvlan);
            SetDsEgressXcOamEthHashKey(V, cvlanId_f,                     &ds_eth_oam_key, cvlan_id);
            SetDsEgressXcOamEthHashKey(V, valid_f,                      &ds_eth_oam_key, 1);
            SetDsEgressXcOamEthHashKey(V, oamDestChipId_f,       &ds_eth_oam_key, p_sys_chan_eth->com.master_chipid);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.global_src_port = 0x%x\n",   gport);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.vlan_id = 0x%x\n",           vlan_id);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.cvlan_id = 0x%x\n",          cvlan_id);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.is_cvlan = 0x%x\n",          is_cvlan);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_eth_oam_key.is_fid = 0x%x\n",            is_fid);
            CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t,
                                                                       p_sys_chan_eth->key.com.key_index ,
                                                                       &ds_eth_oam_key));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_chan_eth->key.com.key_index);
        }


    }

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_chan_del_from_asic(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    DsEgressXcOamEthHashKey_m ds_eth_oam_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_chan_eth);

    if (p_sys_chan_eth->key.link_oam)
    {
        return CTC_E_NONE;
    }

    if (!CTC_IS_LINKAGG_PORT(p_sys_chan_eth->key.gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_eth->key.gport))))
    {
        return CTC_E_NONE;
    }

    if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        /*eth oam key delete*/
        sal_memset(&ds_eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
        SetDsEgressXcOamEthHashKey(V, valid_f,                      &ds_eth_oam_key, 0);
        CTC_ERROR_RETURN(sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t,
                                                          p_sys_chan_eth->key.com.key_index ,
                                                          &ds_eth_oam_key));
        p_sys_chan_eth->key.com.key_exit = 0;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_chan_eth->key.com.key_index);
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        if (p_sys_chan_eth->tp_oam_key_valid[0] && DRV_IS_DUET2(lchip))
        {
            sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[0], &ds_eth_oam_key);
            p_sys_chan_eth->tp_oam_key_valid[0] = 0;
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free additional DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_chan_eth->tp_oam_key_index[0]);
        }
        if (p_sys_chan_eth->tp_oam_key_valid[1] && DRV_IS_DUET2(lchip))
        {
            sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[1], &ds_eth_oam_key);
            p_sys_chan_eth->tp_oam_key_valid[1] = 0;
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free additional DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_chan_eth->tp_oam_key_index[1]);
        }

    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_chan_update(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth, bool is_add)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    DsEgressXcOamEthHashKey_m     ds_eth_oam_hash_key;
    DsEthLmProfile_m  ds_eth_lm_profile;
    uint32 mep_index[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint32 lm_index[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8 lm_cos[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8 lm_cos_type[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8  md_level    = 0;
    uint8  level       = 0;
    uint32 index       = 0;
    uint32 cmd = 0;
    uint16 gport       = 0;
    uint16  lport       = 0;
    uint8* p_mep_bitmap  = 0;
    uint8* p_lm_bitmap = 0;
    uint8  lm_en       = 0;
    sys_oam_id_t* p_sys_oamid = NULL;
    int32 ret = 0;
    uint32 vpws_key_index = 0;
    uint8 loop = 0;
    sys_mpls_oam_t mpls_oam_info;

    DsSrcPort_m ds_src_port;
    DsDestPort_m ds_dest_port;
    sal_memset(&ds_src_port, 0, sizeof(DsSrcPort_m));
    sal_memset(&ds_dest_port, 0, sizeof(DsDestPort_m));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_eth = (sys_oam_chan_eth_t*)p_sys_lmep_eth->p_sys_chan;
    md_level    = p_sys_lmep_eth->md_level;
    p_lm_bitmap = &(p_sys_chan_eth->lm_bitmap);

    if (p_sys_lmep_eth->is_up)
    {
        p_mep_bitmap = &(p_sys_chan_eth->up_mep_bitmap);
    }
    else
    {
        p_mep_bitmap = &(p_sys_chan_eth->down_mep_bitmap);
    }

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN) && is_add)
    {
        SET_BIT((*p_lm_bitmap), md_level);
        p_sys_chan_eth->lm_index[md_level] = p_sys_lmep_eth->lm_index_base;
        p_sys_chan_eth->lm_cos[md_level] = p_sys_lmep_eth->lm_cos;
        p_sys_chan_eth->lm_cos_type[md_level] = p_sys_lmep_eth->lm_cos_type;
        lm_en   = 1;
    }
    else
    {
        CLEAR_BIT((*p_lm_bitmap), md_level);
        p_sys_chan_eth->lm_index[md_level] = 0;
        p_sys_chan_eth->lm_cos[md_level] = 0;
        p_sys_chan_eth->lm_cos_type[md_level] = 0;
        lm_en   = 0;
    }

    if (is_add)
    {
        SET_BIT((*p_mep_bitmap), md_level);
        p_sys_chan_eth->mep_index[md_level] = p_sys_lmep_eth->lmep_index;
    }
    else
    {
        CLEAR_BIT((*p_mep_bitmap), md_level);
        p_sys_chan_eth->mep_index[md_level] = 0x1FFF; /*invalid */
    }

    if (!p_sys_chan_eth->key.link_oam)
    {
        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            sal_memset(&ds_eth_oam_hash_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t,
                                                         p_sys_chan_eth->key.com.key_index ,
                                                         &ds_eth_oam_hash_key));
        }


        for (level = 0; level <= SYS_OAM_MAX_MD_LEVEL; level++)
        {
            if ((IS_BIT_SET(p_sys_chan_eth->down_mep_bitmap, level)) ||
                (IS_BIT_SET(p_sys_chan_eth->up_mep_bitmap, level)))
            {
                mep_index[index] = p_sys_chan_eth->mep_index[level];
                if (IS_BIT_SET(p_sys_chan_eth->lm_bitmap, level))
                {
                    lm_index[index] = p_sys_chan_eth->lm_index[level];
                    lm_cos[index] = p_sys_chan_eth->lm_cos[level];
                    lm_cos_type[index] = p_sys_chan_eth->lm_cos_type[level];
                }
                index++;
            }
        }


        if (!CTC_IS_LINKAGG_PORT(p_sys_chan_eth->key.gport)
               && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_eth->key.gport))))
        {
            return CTC_E_NONE;
        }

        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            if (p_sys_lmep_eth->is_up)
            {
                SetDsEgressXcOamEthHashKey(V, mepUpBitmap_f,   &ds_eth_oam_hash_key, (p_sys_chan_eth->up_mep_bitmap));
            }
            else
            {
                SetDsEgressXcOamEthHashKey(V, mepDownBitmap_f,   &ds_eth_oam_hash_key, (p_sys_chan_eth->down_mep_bitmap));
            }

            SetDsEgressXcOamEthHashKey(V, array_0_mepIndex_f,        &ds_eth_oam_hash_key, mep_index[0]);
            SetDsEgressXcOamEthHashKey(V, array_1_mepIndex_f,        &ds_eth_oam_hash_key, mep_index[1]);
            SetDsEgressXcOamEthHashKey(V, array_2_mepIndex_f,         &ds_eth_oam_hash_key, mep_index[2]);
            SetDsEgressXcOamEthHashKey(V, array_3_mepIndex_f,          &ds_eth_oam_hash_key, mep_index[3]);

            SetDsEgressXcOamEthHashKey(V, lmBitmap_f,              &ds_eth_oam_hash_key, ((*p_lm_bitmap) ));
            SetDsEgressXcOamEthHashKey(V, lmProfileIndex_f,       &ds_eth_oam_hash_key, p_sys_chan_eth->com.lm_index_base);
            SetDsEgressXcOamEthHashKey(V, lmType_f,                   &ds_eth_oam_hash_key, p_sys_chan_eth->lm_type);
            SetDsEgressXcOamEthHashKey(V, array_0_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[0]);
            SetDsEgressXcOamEthHashKey(V, array_1_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[1]);
            SetDsEgressXcOamEthHashKey(V, array_2_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[2]);
            SetDsEgressXcOamEthHashKey(V, array_3_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[3]);

        }

        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t,
                                                          p_sys_chan_eth->key.com.key_index ,
                                                          &ds_eth_oam_hash_key));
        }

        if (p_sys_chan_eth->lm_bitmap)
        {
            sal_memset(&ds_eth_lm_profile, 0, sizeof(DsEthLmProfile_m));
            cmd = DRV_IOW(DsSrcEthLmProfile_t, DRV_ENTRY_FLAG);
            SetDsEthLmProfile(V, array_0_lmIndexBase_f, &ds_eth_lm_profile, lm_index[0]);
            SetDsEthLmProfile(V, array_1_lmIndexBase_f, &ds_eth_lm_profile, lm_index[1]);
            SetDsEthLmProfile(V, array_2_lmIndexBase_f, &ds_eth_lm_profile, lm_index[2]);
            SetDsEthLmProfile(V, array_3_lmIndexBase_f, &ds_eth_lm_profile, lm_index[3]);
            SetDsEthLmProfile(V, array_0_lmCos_f, &ds_eth_lm_profile, lm_cos[0]);
            SetDsEthLmProfile(V, array_1_lmCos_f, &ds_eth_lm_profile, lm_cos[1]);
            SetDsEthLmProfile(V, array_2_lmCos_f, &ds_eth_lm_profile, lm_cos[2]);
            SetDsEthLmProfile(V, array_3_lmCos_f, &ds_eth_lm_profile, lm_cos[3]);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_chan_eth->com.lm_index_base, cmd, &ds_eth_lm_profile));

            cmd = DRV_IOW(DsDestEthLmProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_chan_eth->com.lm_index_base, cmd, &ds_eth_lm_profile));

        }

    }
    else
    {
        /*update mepindex in DsSrcPort*/
        gport = p_sys_chan_eth->key.gport;
        if (!CTC_IS_LINKAGG_PORT(gport)
           && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport))))
        {
            return CTC_E_NONE;
        }

        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

        SetDsSrcPort(V, linkOamMepIndex_f,    &ds_src_port,  p_sys_chan_eth->mep_index[md_level]);
        SetDsSrcPort(V, linkOamEn_f,    &ds_src_port,   (is_add ? 1 : 0) );
        SetDsSrcPort(V, linkOamLevel_f,    &ds_src_port,   md_level );

        SetDsDestPort(V, linkOamEn_f,    &ds_dest_port,   (is_add ? 1 : 0) );
        SetDsDestPort(V, linkOamLevel_f,    &ds_dest_port,   md_level );

        if (lm_en)
        {
            SetDsSrcPort(V, linkLmType_f,      &ds_src_port,  p_sys_chan_eth->link_lm_type);
            SetDsSrcPort(V, linkLmCosType_f,     &ds_src_port,  p_sys_chan_eth->link_lm_cos_type);
            SetDsSrcPort(V, linkLmCos_f,     &ds_src_port,  p_sys_chan_eth->link_lm_cos);
            SetDsSrcPort(V, linkLmIndexBase_f,   &ds_src_port,  p_sys_chan_eth->com.link_lm_index_base);

            SetDsDestPort(V, linkLmType_f,      &ds_dest_port,  p_sys_chan_eth->link_lm_type);
            SetDsDestPort(V, linkLmCosType_f,     &ds_dest_port,  p_sys_chan_eth->link_lm_cos_type);
            SetDsDestPort(V, linkLmCos_f,     &ds_dest_port,  p_sys_chan_eth->link_lm_cos);
            SetDsDestPort(V, linkLmIndexBase_f,   &ds_dest_port,  p_sys_chan_eth->com.link_lm_index_base);
        }
        else
        {
            SetDsSrcPort(V, linkLmType_f,   &ds_src_port,  CTC_OAM_LM_TYPE_NONE);
            SetDsDestPort(V, linkLmType_f,   &ds_dest_port,  CTC_OAM_LM_TYPE_NONE);
        }

        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

        cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

    }

    /*alloc vpws additional key or update ad of vpws additional key*/
    if (p_sys_chan_eth->key.is_vpws && p_sys_chan_eth->key.use_fid && !p_sys_chan_eth->key.link_oam && DRV_IS_DUET2(lchip))
    {
        p_sys_oamid = _sys_usw_oam_oamid_lkup(lchip, p_sys_chan_eth->key.gport, p_sys_chan_eth->key.l2vpn_oam_id);
        if (p_sys_oamid && is_add)
        {
            p_sys_oamid->p_sys_chan_eth = p_sys_chan_eth;
        }
        for (loop = 0; p_sys_oamid && loop < 2; loop++)
        {
            if (!p_sys_oamid->label[loop])
            {
                continue;
            }
            mpls_oam_info.label = p_sys_oamid->label[loop];
            mpls_oam_info.spaceid = p_sys_oamid->space_id[loop];
            mpls_oam_info.oam_type = 1;
            ret = sys_usw_mpls_get_oam_info(lchip, &mpls_oam_info);
            if (ret != CTC_E_NONE)
            {
                continue;
            }
            SetDsEgressXcOamEthHashKey(V, vlanId_f, &ds_eth_oam_hash_key, mpls_oam_info.oamid + 8192);
            if (is_add)  /*add lmep need alloc DsEgressXcOamEthHashKey */
            {
                ret = sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamEthHashKey_t, &vpws_key_index, &ds_eth_oam_hash_key);
            }
            else   /*remove lmep need update ad of DsEgressXcOamEthHashKey*/
            {
                ret = sys_usw_oam_key_lookup_io_by_key(lchip, DsEgressXcOamEthHashKey_t, &vpws_key_index, &ds_eth_oam_hash_key);
            }
            ret = ret ? ret : sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t, vpws_key_index, &ds_eth_oam_hash_key);
            if ((ret == CTC_E_NONE) && is_add)
            {
                p_sys_chan_eth->tp_oam_key_valid[loop] = 1;
                p_sys_chan_eth->tp_oam_key_index[loop] = vpws_key_index;
                SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc additional DsEgressXcOamEthHashKey, key_index:%d \n", vpws_key_index);
            }
        }

    }

    return CTC_E_NONE;
}
#if 0
int32
_sys_usw_cfm_chan_update_reomte(uint8 lchip, ctc_oam_lmep_t* p_lmep, sys_oam_chan_eth_t* p_sys_chan_eth, bool is_add)
{
    DsEgressXcOamEthHashKey_m     ds_eth_oam_hash_key;
    uint32 mep_index[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8  md_level    = 0;
    uint8  level       = 0;
    uint32 index       = 0;
    uint32 cmd = 0;
    uint32 gport       = 0;
    uint16  lport       = 0;
    uint8* p_mep_bitmap  = 0;
    uint8* p_lm_bitmap = 0;

    DsSrcPort_m ds_src_port;
    DsDestPort_m ds_dest_port;
    sal_memset(&ds_src_port, 0, sizeof(DsSrcPort_m));
    sal_memset(&ds_dest_port, 0, sizeof(DsDestPort_m));

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    md_level    = p_lmep->key.u.eth.md_level;
    p_lm_bitmap = &(p_sys_chan_eth->lm_bitmap);

    if (CTC_FLAG_ISSET(p_lmep->key.flag, CTC_OAM_KEY_FLAG_UP_MEP))
    {
        p_mep_bitmap = &(p_sys_chan_eth->up_mep_bitmap);
    }
    else
    {
        p_mep_bitmap = &(p_sys_chan_eth->down_mep_bitmap);
    }

    if (CTC_FLAG_ISSET(p_lmep->u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN) && is_add)
    {
        SET_BIT((*p_lm_bitmap), md_level);
    }
    else
    {
        CLEAR_BIT((*p_lm_bitmap), md_level);
    }

    if (is_add)
    {
        SET_BIT((*p_mep_bitmap), md_level);
        p_sys_chan_eth->mep_index[md_level] = p_lmep->lmep_index;
    }
    else
    {
        CLEAR_BIT((*p_mep_bitmap), md_level);
        p_sys_chan_eth->mep_index[md_level] = 0x1FFF; /*invalid */
    }

    if (!p_sys_chan_eth->key.link_oam)
    {
        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            sal_memset(&ds_eth_oam_hash_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t,
                                                         p_sys_chan_eth->key.com.key_index ,
                                                         &ds_eth_oam_hash_key));
        }

        for (level = 1; level <= SYS_OAM_MAX_MD_LEVEL; level++)
        {
            if ((IS_BIT_SET(p_sys_chan_eth->down_mep_bitmap, level)) ||
                (IS_BIT_SET(p_sys_chan_eth->up_mep_bitmap, level)))
            {
                mep_index[index++] = p_sys_chan_eth->mep_index[level];
            }
        }

        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            if (CTC_FLAG_ISSET(p_lmep->key.flag, CTC_OAM_KEY_FLAG_UP_MEP))
            {
                SetDsEgressXcOamEthHashKey(V, mepUpBitmap_f,       &ds_eth_oam_hash_key, (p_sys_chan_eth->up_mep_bitmap));
            }
            else
            {
                 SetDsEgressXcOamEthHashKey(V, mepDownBitmap_f,    &ds_eth_oam_hash_key, (p_sys_chan_eth->down_mep_bitmap));
            }

            SetDsEgressXcOamEthHashKey(V, array_0_mepIndex_f,              &ds_eth_oam_hash_key, mep_index[0]);
            SetDsEgressXcOamEthHashKey(V, array_1_mepIndex_f,            &ds_eth_oam_hash_key, mep_index[1]);
            SetDsEgressXcOamEthHashKey(V, array_2_mepIndex_f,            &ds_eth_oam_hash_key, mep_index[2]);
            SetDsEgressXcOamEthHashKey(V, array_3_mepIndex_f,            &ds_eth_oam_hash_key, mep_index[3]);
        }

        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t,
                                                          p_sys_chan_eth->key.com.key_index ,
                                                          &ds_eth_oam_hash_key));
        }

    }
    else
    {
        /*update mepindex in DsSrcPort*/
        gport = p_sys_chan_eth->key.gport;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));
        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

        SetDsSrcPort(V, linkOamMepIndex_f,    &ds_src_port,  p_sys_chan_eth->mep_index[md_level]);
        SetDsSrcPort(V, linkOamEn_f,    &ds_src_port,   (is_add ? 1 : 0) );
        SetDsSrcPort(V, linkOamLevel_f,    &ds_src_port,   md_level );

        SetDsDestPort(V, linkOamEn_f,    &ds_dest_port,   (is_add ? 1 : 0) );
        SetDsDestPort(V, linkOamLevel_f,    &ds_dest_port,   md_level );


        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));
        cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));


    }

    return CTC_E_NONE;
}
#endif
#define LMEP "Function Begin"

sys_oam_lmep_t*
_sys_usw_cfm_lmep_lkup(uint8 lchip, uint8 md_level, sys_oam_chan_eth_t* p_sys_chan_eth)
{
    sys_oam_lmep_t sys_oam_lmep_eth;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_lmep_eth, 0, sizeof(sys_oam_lmep_t));
    sys_oam_lmep_eth.md_level = md_level;

    return (sys_oam_lmep_t*)_sys_usw_oam_lmep_lkup(lchip, &p_sys_chan_eth->com, &sys_oam_lmep_eth);
}

sys_oam_lmep_t*
_sys_usw_cfm_lmep_build_node(uint8 lchip, ctc_oam_lmep_t* p_lmep_param)
{
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_lmep_eth = (sys_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_lmep_t));
    if (NULL != p_sys_lmep_eth)
    {
        sal_memset(p_sys_lmep_eth, 0, sizeof(sys_oam_lmep_t));
        p_sys_lmep_eth->is_up          = CTC_FLAG_ISSET(p_lmep_param->key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
        p_sys_lmep_eth->vlan_domain    = CTC_FLAG_ISSET(p_lmep_param->key.flag, CTC_OAM_KEY_FLAG_CVLAN_DOMAIN);
        p_sys_lmep_eth->md_level       = p_lmep_param->key.u.eth.md_level;
        p_sys_lmep_eth->flag     = p_lmep_param->u.y1731_lmep.flag;

        p_sys_lmep_eth->mep_on_cpu = CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU);
        p_sys_lmep_eth->tx_csf_type    = p_lmep_param->u.y1731_lmep.tx_csf_type;
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            p_sys_lmep_eth->lmep_index = p_lmep_param->lmep_index;
        }
        if((!CTC_FLAG_ISSET(p_lmep_param->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM))
            &&CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
        {
            p_sys_lmep_eth->lm_cos  = p_lmep_param->u.y1731_lmep.lm_cos;
            p_sys_lmep_eth->lm_cos_type  = p_lmep_param->u.y1731_lmep.lm_cos_type;
        }
    }

    return p_sys_lmep_eth;
}

int32
_sys_usw_cfm_lmep_free_node(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_lmep_eth);
    p_sys_lmep_eth = NULL;
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_build_index(lchip, p_sys_lmep_eth));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_free_index(lchip, p_sys_lmep_eth));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_add_to_db(lchip, p_sys_lmep_eth));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_db(lchip, p_sys_lmep_eth));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_add_to_asic(uint8 lchip, ctc_oam_lmep_t* p_lmep_param,
                                    sys_oam_lmep_t* p_sys_lmep_eth)
{
    ctc_oam_y1731_lmep_t* p_eth_lmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);
    CTC_PTR_VALID_CHECK(p_sys_lmep_eth);

    p_eth_lmep = &(p_lmep_param->u.y1731_lmep);
    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_add_eth_to_asic(lchip, p_eth_lmep, p_sys_lmep_eth));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_asic(lchip, p_sys_lmep_eth));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_update_asic(uint8 lchip, ctc_oam_update_t* p_lmep_param,
                                    sys_oam_lmep_t* p_sys_lmep_eth)
{
    ctc_oam_update_t* p_eth_lmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_lmep_param);
    CTC_PTR_VALID_CHECK(p_sys_lmep_eth);

    p_eth_lmep = p_lmep_param;

    switch (p_eth_lmep->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        if (1 == p_eth_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        CTC_COS_RANGE_CHECK(p_eth_lmep->update_value);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        if (1 == p_eth_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN);
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);

        if (1 == p_eth_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN);
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, CTC_OAM_CSF_TYPE_MAX - 1);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA:
        CTC_MAX_VALUE_CHECK(p_eth_lmep->update_value, 1);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
        if (p_eth_lmep->update_value < SYS_OAM_MIN_CCM_INTERVAL || p_eth_lmep->update_value > SYS_OAM_MAX_CCM_INTERVAL)
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
        if (p_eth_lmep->update_value < SYS_OAM_MIN_MEP_ID || p_eth_lmep->update_value > MCHIP_CAP(SYS_CAP_OAM_MEP_ID))
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_eth_asic(lchip, p_sys_lmep_eth, p_eth_lmep));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_build_lm_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{
    sys_oam_chan_eth_t* sys_oam_chan_eth = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_oam_chan_eth = (sys_oam_chan_eth_t*)p_sys_lmep_eth->p_sys_chan;
    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
    {
        if (!sys_oam_chan_eth->key.link_oam)
        {
            if (p_sys_lmep_eth->p_sys_chan->lm_num >= 3)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  LM entry full in EVC\n");
                return CTC_E_INVALID_CONFIG;
            }
            CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &sys_oam_chan_eth->com, FALSE, p_sys_lmep_eth->md_level));
        }
        else if(!sys_oam_chan_eth->link_lm_index_alloced)
        {
            CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &sys_oam_chan_eth->com, TRUE, p_sys_lmep_eth->md_level));
            sys_oam_chan_eth->link_lm_index_alloced = TRUE;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_lmep_free_lm_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth)
{

    sys_oam_chan_eth_t* sys_oam_chan_eth = NULL;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_oam_chan_eth = (sys_oam_chan_eth_t*)p_sys_lmep_eth->p_sys_chan;

    if (!sys_oam_chan_eth->key.link_oam)
    {
        CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &sys_oam_chan_eth->com, FALSE, p_sys_lmep_eth->md_level));
    }
    else if (sys_oam_chan_eth->link_lm_index_alloced)
    {
        CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &sys_oam_chan_eth->com, TRUE, p_sys_lmep_eth->md_level));
        sys_oam_chan_eth->link_lm_index_alloced= FALSE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_update_lmep_port_status(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;

    sys_oam_lmep_t* p_sys_lmep = NULL;

    ctc_slistnode_t* slist_lmep_node = NULL;
    ctc_slist_t* p_lmep_list = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    p_lmep_list = p_sys_chan_eth->com.lmep_list;
    if (NULL == p_lmep_list)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }


    CTC_SLIST_LOOP(p_lmep_list, slist_lmep_node)
    {
        p_sys_lmep = _ctc_container_of(slist_lmep_node, sys_oam_lmep_t, head);
        p_sys_lmep_eth = p_sys_lmep;

        if (p_sys_lmep_eth->mep_on_cpu)
        {
            continue;
        }

        switch (p_lmep_param->update_value)
        {
            case CTC_OAM_ETH_PS_BLOCKED:
                p_lmep_param->update_value = 1;
                break;

            case CTC_OAM_ETH_PS_UP:
                p_lmep_param->update_value = 2;
                break;

            default:
                p_lmep_param->update_value = 0;
        }

        CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_eth_asic(lchip, p_sys_lmep_eth, p_lmep_param));
    }



    return CTC_E_NONE;
}

int32
_sys_usw_cfm_update_lmep_if_status(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    ctc_oam_update_t* p_eth_lmep_param = p_lmep_param;
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 gport = 0;
    uint8 intf_status = 0;
    uint8 gchip     = 0;
    uint16 lport = 0;
    uint16 mp_port_id = 0;

    gport = p_eth_lmep_param->key.u.eth.gport;
    intf_status = p_eth_lmep_param->update_value;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)


    if (CTC_IS_LINKAGG_PORT(gport))
    {
        lport = CTC_MAP_GPORT_TO_LPORT(gport);
        mp_port_id = lport + 64;
    }
    else
    {
        if (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport)))
        {
            return CTC_E_NONE;
        }

        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
        mp_port_id = lport;
    }

    switch (intf_status)
    {
    case CTC_OAM_ETH_INTF_IS_UP:
        field_value = 1;
        break;

    case CTC_OAM_ETH_INTF_IS_DOWN:
        field_value = 2;
        break;

    case CTC_OAM_ETH_INTF_IS_TESTING:
        field_value = 3;
        break;

    case CTC_OAM_ETH_INTF_IS_UNKNOWN:
        field_value = 4;
        break;

    case CTC_OAM_ETH_INTF_IS_DORMANT:
        field_value = 5;
        break;

    case CTC_OAM_ETH_INTF_IS_NOT_PRESENT:
        field_value = 6;
        break;

    case CTC_OAM_ETH_INTF_IS_LOWER_LAYER_DOWN:
        field_value = 7;
        break;

    default:
        field_value = 0;
        break;
    }

    cmd = DRV_IOW(DsPortProperty_t, DsPortProperty_ifStatus_f);

    if (CTC_IS_LINKAGG_PORT(gport))
    {

        p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
        if (NULL == p_sys_chan_eth)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
			return CTC_E_NOT_EXIST;
        }

        sys_usw_get_gchip_id(lchip, &gchip);
        if (gchip == p_sys_chan_eth->com.master_chipid)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mp_port_id, cmd, &field_value));
        }
    }
    else
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, mp_port_id, cmd, &field_value));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_update_lmep_lm(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    uint32 srcport_cmd      = 0;
    uint32 destport_cmd     = 0;
    uint32 cmd = 0;
    uint32 gport            = 0;
    uint16  lport            = 0;
    uint32 field_value      = 0;
    DsEthLmProfile_m  ds_eth_lm_profile;
    uint32 lm_index[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8 lm_cos[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8 lm_cos_type[SYS_OAM_MAX_MEP_NUM_PER_CHAN] = {0};
    uint8 is_ether_service = 0;
    uint8 index = 0;
    uint8 level = 0;
    uint8 is_link = 0;
    DsEgressXcOamEthHashKey_m   ds_eth_oam_hash_key_tmp;
    uint32 tmp = 0;

    ctc_oam_update_t* p_lmep = (ctc_oam_update_t*)p_lmep_param;

    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    uint8            md_level = 0;

    DsEgressXcOamEthHashKey_m   ds_eth_oam_hash_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!CTC_IS_LINKAGG_PORT(p_lmep->key.u.eth.gport) &&
        FALSE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(p_lmep->key.u.eth.gport)))
    {
        return CTC_E_NONE;
    }

    /*1. lookup chan and check exist*/
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    md_level = p_lmep->key.u.eth.md_level;
    is_link = CTC_FLAG_ISSET(p_lmep->key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    if (CTC_IS_BIT_SET(p_sys_chan_eth->lm_bitmap, md_level))
    {
        if((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE == p_lmep->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS == p_lmep->update_type))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Config is invalid lm is Enable\n");
			return CTC_E_INVALID_CONFIG;
        }
    }

    /*2. lookup lmep and check exist*/
    if ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN == p_lmep->update_type)
        ||(CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE == p_lmep->update_type)
        ||(CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS == p_lmep->update_type))
    {
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
    }

    /*3. lmep db & asic update*/
    switch (p_lmep->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, 1);
        if (1 == p_lmep->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);
            srcport_cmd     = DRV_IOW(DsSrcPort_t,  DsSrcPort_linkLmIndexBase_f);
            destport_cmd    = DRV_IOW(DsDestPort_t, DsDestPort_linkLmIndexBase_f);

            CTC_BIT_SET(p_sys_chan_eth->lm_bitmap, md_level);
            if (!p_sys_chan_eth->link_lm_index_alloced && p_sys_chan_eth->key.link_oam)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &p_sys_chan_eth->com, TRUE, md_level));
                p_sys_chan_eth->link_lm_index_alloced = TRUE;
            }

            if (!p_sys_chan_eth->key.link_oam)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &p_sys_chan_eth->com, FALSE, md_level));
                is_ether_service = 1;
                p_sys_chan_eth->lm_index[md_level] = p_sys_lmep_eth->lm_index_base;
            }
            field_value = p_sys_chan_eth->com.link_lm_index_base;
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN);
            srcport_cmd     = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmType_f);
            destport_cmd    = DRV_IOW(DsDestPort_t,DsDestPort_linkLmType_f);
            field_value     = CTC_OAM_LM_TYPE_NONE;

            CTC_BIT_UNSET(p_sys_chan_eth->lm_bitmap, md_level);

            if (!is_link)
            {
                if (0 == p_sys_chan_eth->lm_bitmap)
                {
                    p_sys_chan_eth->lm_type = CTC_OAM_LM_TYPE_NONE;
                }
                CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &p_sys_chan_eth->com, FALSE, md_level));
                is_ether_service = 1;
                p_sys_chan_eth->lm_index[md_level] = 0;
                p_sys_chan_eth->lm_cos[md_level] = 0;
                p_sys_chan_eth->lm_cos_type[md_level] = 0;
            }
            else
            {
                if(p_sys_chan_eth->link_lm_index_alloced)
                {
                    p_sys_chan_eth->link_lm_type = CTC_OAM_LM_TYPE_NONE;
                    CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &p_sys_chan_eth->com, TRUE, md_level));
                    p_sys_chan_eth->link_lm_index_alloced = FALSE;
                }
            }
        }

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, CTC_OAM_LM_TYPE_MAX - 1);
        p_sys_chan_eth->key.link_oam ?
        (p_sys_chan_eth->link_lm_type = p_lmep->update_value) : (p_sys_chan_eth->lm_type = p_lmep->update_value);

        srcport_cmd     = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmType_f);
        destport_cmd    = DRV_IOW(DsDestPort_t, DsDestPort_linkLmType_f);
        field_value     = p_lmep->update_value;
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE:
        CTC_MAX_VALUE_CHECK(p_lmep->update_value, CTC_OAM_LM_COS_TYPE_MAX - 1);
        if(p_sys_chan_eth->key.link_oam )
        {
            p_sys_chan_eth->link_lm_cos_type = p_lmep->update_value;
        }
        else
        {
            p_sys_chan_eth->lm_cos_type[md_level] = p_lmep->update_value;
            p_sys_lmep_eth->lm_cos_type = p_lmep->update_value;
        }

        srcport_cmd     = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmCosType_f);
        destport_cmd    = DRV_IOW(DsDestPort_t, DsDestPort_linkLmCosType_f);
        field_value     = p_lmep->update_value;
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS:
        CTC_COS_RANGE_CHECK(p_lmep->update_value);
        if (p_sys_chan_eth->key.link_oam )
        {
            p_sys_chan_eth->link_lm_cos = p_lmep->update_value;
        }
        else
        {
            p_sys_chan_eth->lm_cos[md_level] = p_lmep->update_value;
            p_sys_lmep_eth->lm_cos = p_lmep->update_value;
        }

        srcport_cmd     = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmCos_f);
        destport_cmd    = DRV_IOW(DsDestPort_t, DsDestPort_linkLmCos_f);
        field_value     = p_lmep->update_value;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if (!p_sys_chan_eth->key.link_oam)
    {

        for (level = 0; level <= SYS_OAM_MAX_MD_LEVEL; level++)
        {
            if ((IS_BIT_SET(p_sys_chan_eth->down_mep_bitmap, level)) ||
                    (IS_BIT_SET(p_sys_chan_eth->up_mep_bitmap, level)))
            {
                if (IS_BIT_SET(p_sys_chan_eth->lm_bitmap, level))
                {
                    lm_index[index] = p_sys_chan_eth->lm_index[level];
                    lm_cos[index] = p_sys_chan_eth->lm_cos[level];
                    lm_cos_type[index] = p_sys_chan_eth->lm_cos_type[level];
                }
                index++;
            }
        }

        if (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        {
            sal_memset(&ds_eth_oam_hash_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
            CTC_ERROR_RETURN(  sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t,
                                                           p_sys_chan_eth->key.com.key_index ,
                                                           &ds_eth_oam_hash_key));

            SetDsEgressXcOamEthHashKey(V, lmBitmap_f,                 &ds_eth_oam_hash_key, (p_sys_chan_eth->lm_bitmap));
            SetDsEgressXcOamEthHashKey(V, lmProfileIndex_f,            &ds_eth_oam_hash_key, p_sys_chan_eth->com.lm_index_base);
            SetDsEgressXcOamEthHashKey(V, lmType_f,                     &ds_eth_oam_hash_key, p_sys_chan_eth->lm_type);
            SetDsEgressXcOamEthHashKey(V, array_0_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[0]);
            SetDsEgressXcOamEthHashKey(V, array_1_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[1]);
            SetDsEgressXcOamEthHashKey(V, array_2_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[2]);
            SetDsEgressXcOamEthHashKey(V, array_3_lmCosType_f,    &ds_eth_oam_hash_key, lm_cos_type[3]);

            CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t,
                                                          p_sys_chan_eth->key.com.key_index ,
                                                          &ds_eth_oam_hash_key));
        }

        if (is_ether_service)
        {
            sal_memset(&ds_eth_lm_profile, 0, sizeof(DsEthLmProfile_m));
            cmd = DRV_IOW(DsSrcEthLmProfile_t, DRV_ENTRY_FLAG);
            SetDsEthLmProfile(V, array_0_lmIndexBase_f, &ds_eth_lm_profile, lm_index[0]);
            SetDsEthLmProfile(V, array_1_lmIndexBase_f, &ds_eth_lm_profile, lm_index[1]);
            SetDsEthLmProfile(V, array_2_lmIndexBase_f, &ds_eth_lm_profile, lm_index[2]);
            SetDsEthLmProfile(V, array_3_lmIndexBase_f, &ds_eth_lm_profile, lm_index[3]);
            SetDsEthLmProfile(V, array_0_lmCos_f, &ds_eth_lm_profile, lm_cos[0]);
            SetDsEthLmProfile(V, array_1_lmCos_f, &ds_eth_lm_profile, lm_cos[1]);
            SetDsEthLmProfile(V, array_2_lmCos_f, &ds_eth_lm_profile, lm_cos[2]);
            SetDsEthLmProfile(V, array_3_lmCos_f, &ds_eth_lm_profile, lm_cos[3]);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_chan_eth->com.lm_index_base, cmd, &ds_eth_lm_profile));

            cmd = DRV_IOW(DsDestEthLmProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_chan_eth->com.lm_index_base, cmd, &ds_eth_lm_profile));

        }
    }
    else
    {
        /*update mepindex in DsSrcPort*/
        gport = p_sys_chan_eth->key.gport;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, srcport_cmd, &field_value));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, destport_cmd, &field_value));
    }
    /*upadte AD of VPWS duplicate key*/
    if ((!p_sys_chan_eth->key.link_oam) && (p_sys_chan_eth->key.com.key_alloc == SYS_OAM_KEY_HASH)
        && (p_sys_chan_eth->key.is_vpws && p_sys_chan_eth->key.use_fid))
    {
        if (p_sys_chan_eth->tp_oam_key_valid[0])
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[0], &ds_eth_oam_hash_key_tmp));
            tmp = GetDsEgressXcOamEthHashKey(V, vlanId_f, &ds_eth_oam_hash_key_tmp);
            SetDsEgressXcOamEthHashKey(V, vlanId_f, &ds_eth_oam_hash_key, tmp);
            CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[0], &ds_eth_oam_hash_key));
        }
        if (p_sys_chan_eth->tp_oam_key_valid[1])
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[1], &ds_eth_oam_hash_key_tmp));
            tmp = GetDsEgressXcOamEthHashKey(V, vlanId_f, &ds_eth_oam_hash_key_tmp);
            SetDsEgressXcOamEthHashKey(V, vlanId_f, &ds_eth_oam_hash_key, tmp);
            CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_chan_eth->tp_oam_key_index[1], &ds_eth_oam_hash_key));
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_update_lmep_npm(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;

    uint8 md_level = 0;
    AutoGenPktGlbCtl_m glb_ctl;
    uint8 mode = 0;
    uint8 max_session_num = 8;
    uint32 cmd = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    md_level = p_lmep_param->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_sys_lmep_eth->mep_on_cpu)
    {
        return CTC_E_NONE;
    }

    if (0xff != p_lmep_param->update_value)
    {
        cmd = DRV_IOR(AutoGenPktGlbCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_ctl));
        mode = GetAutoGenPktGlbCtl(V, sessionMode_f, &glb_ctl);
        max_session_num = (mode)?((mode == 1)?6:4):8;
        if (p_lmep_param->update_value >= max_session_num)
        {
            p_lmep_param->update_value = 0xff;
        }
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_eth_asic(lchip, p_sys_lmep_eth, p_lmep_param));


    return CTC_E_NONE;
}

#if 0
int32
_sys_usw_cfm_update_lmep_vlan(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;

    uint8 md_level = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  MEP or MIP lookup key not found \n");
        return CTC_E_NOT_EXIST;
    }

    md_level = p_lmep_param->key.u.eth.md_level;
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_sys_lmep_eth->mep_on_cpu)
    {
        return CTC_E_NONE;
    }

    if (p_lmep_param->update_value > CTC_MAX_VLAN_ID)
        {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_eth_asic(lchip, p_sys_lmep_eth, p_lmep_param));



    return CTC_E_NONE;
}
#endif
int32
_sys_usw_cfm_update_master_gchip(uint8 lchip, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_lmep_eth = NULL;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_sys_chan_eth = _sys_usw_cfm_chan_lkup(lchip, &p_lmep_param->key);
    if (NULL == p_sys_chan_eth)
    {
        return CTC_E_OAM_CHAN_ENTRY_NOT_FOUND;
    }
    else if (p_sys_chan_eth->key.link_oam)
    {
        return CTC_E_INVALID_CONFIG;
    }
    p_sys_lmep_eth = _sys_usw_cfm_lmep_lkup(lchip, p_lmep_param->key.u.eth.md_level, p_sys_chan_eth);
    if (NULL == p_sys_lmep_eth)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }
    CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_master_chip(lchip, (sys_oam_chan_com_t*)p_sys_chan_eth, p_lmep_param->update_value));
    return CTC_E_NONE;
}


#define RMEP "Function Begin"

sys_oam_rmep_t*
_sys_usw_cfm_rmep_lkup(uint8 lchip, uint32 rmep_id, sys_oam_lmep_t* p_sys_lmep_eth)
{
    sys_oam_rmep_t sys_oam_rmep_eth;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_rmep_eth, 0, sizeof(sys_oam_rmep_t));
    sys_oam_rmep_eth.rmep_id = rmep_id;

    return (sys_oam_rmep_t*)_sys_usw_oam_rmep_lkup(lchip, p_sys_lmep_eth, &sys_oam_rmep_eth);
}


sys_oam_rmep_t*
_sys_usw_cfm_rmep_build_node(uint8 lchip, ctc_oam_rmep_t* p_rmep_param)
{
    sys_oam_rmep_t* p_sys_rmep_eth = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_rmep_eth = (sys_oam_rmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_rmep_t));
    if (NULL != p_sys_rmep_eth)
    {
        sal_memset(p_sys_rmep_eth, 0, sizeof(sys_oam_rmep_t));
        p_sys_rmep_eth->rmep_id = p_rmep_param->u.y1731_rmep.rmep_id;
        /*p_sys_rmep_eth->is_p2p_mode = 0; */
        p_sys_rmep_eth->rmep_index = p_rmep_param->rmep_index;
    }

    return p_sys_rmep_eth;
}

int32
_sys_usw_cfm_rmep_free_node(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_rmep_eth);
    p_sys_rmep_eth = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth, sys_oam_lmep_t* p_sys_lmep_eth)
{
    int32 ret = 0;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_build_index(lchip, p_sys_rmep_eth));

    if (!CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        sys_oam_lmep_t* p_sys_lmep = NULL;
        DsEgressXcOamRmepHashKey_m  ds_eth_oam_rmep_hash_key;

        p_sys_lmep = p_sys_rmep_eth->p_sys_lmep;
        /*HASH*/
        sal_memset(&ds_eth_oam_rmep_hash_key, 0, sizeof(ds_eth_oam_rmep_hash_key));

        SetDsEgressXcOamRmepHashKey(V, hashType_f,  &ds_eth_oam_rmep_hash_key, EGRESSXCOAMHASHTYPE_RMEP);
        SetDsEgressXcOamRmepHashKey(V, mepIndex_f,   &ds_eth_oam_rmep_hash_key,  p_sys_lmep->lmep_index);
        SetDsEgressXcOamRmepHashKey(V, rmepId_f,  &ds_eth_oam_rmep_hash_key, p_sys_rmep_eth->rmep_id);
        SetDsEgressXcOamRmepHashKey(V, valid_f,   &ds_eth_oam_rmep_hash_key, 1);

        ret = ( sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamRmepHashKey_t,
                                                        &p_sys_rmep_eth->key_index , &ds_eth_oam_rmep_hash_key ));
        if(ret < 0)
        {
            CTC_ERROR_RETURN(_sys_usw_oam_rmep_free_index(lchip, p_sys_rmep_eth)); /*roll back rmep_index*/
            return ret;
        }

    }
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth)
{

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_free_index(lchip, p_sys_rmep_eth));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_add_to_db(lchip, p_sys_rmep_eth));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_db(lchip, p_sys_rmep_eth));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_build_eth_data(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                       sys_oam_rmep_t* p_sys_rmep_eth)
{

    ctc_oam_y1731_rmep_t* p_eth_rmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_eth_rmep = (ctc_oam_y1731_rmep_t*)&p_rmep_param->u.y1731_rmep;

    p_sys_rmep_eth->flag                       = p_eth_rmep->flag;
    p_sys_rmep_eth->rmep_id                    = p_eth_rmep->rmep_id;
    p_sys_rmep_eth->md_level                   = p_rmep_param->key.u.eth.md_level;

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_add_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                    sys_oam_rmep_t* p_sys_rmep_eth)
{
    sys_oam_lmep_t* p_sys_lmep = NULL;
    DsEgressXcOamRmepHashKey_m ds_eth_oam_rmep_hash_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);
    CTC_PTR_VALID_CHECK(p_sys_rmep_eth);

    p_sys_lmep = p_sys_rmep_eth->p_sys_lmep;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_cfm_rmep_build_eth_data(lchip, p_rmep_param, p_sys_rmep_eth));
    CTC_ERROR_RETURN(_sys_usw_oam_rmep_add_eth_to_asic(lchip, p_rmep_param, p_sys_rmep_eth));

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        return CTC_E_NONE;
    }

    sal_memset(&ds_eth_oam_rmep_hash_key, 0, sizeof(ds_eth_oam_rmep_hash_key));

    SetDsEgressXcOamRmepHashKey(V, hashType_f,  &ds_eth_oam_rmep_hash_key, EGRESSXCOAMHASHTYPE_RMEP);
    SetDsEgressXcOamRmepHashKey(V, mepIndex_f,   &ds_eth_oam_rmep_hash_key,  p_sys_lmep->lmep_index);
    SetDsEgressXcOamRmepHashKey(V, rmepId_f,  &ds_eth_oam_rmep_hash_key, p_sys_rmep_eth->rmep_id);
    SetDsEgressXcOamRmepHashKey(V, rmepIndex_f,   &ds_eth_oam_rmep_hash_key,  p_sys_rmep_eth->rmep_index);
    SetDsEgressXcOamRmepHashKey(V, valid_f,   &ds_eth_oam_rmep_hash_key, 1);

    CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, DsEgressXcOamRmepHashKey_t,
                                                 p_sys_rmep_eth->key_index ,
                                                 &ds_eth_oam_rmep_hash_key ));
    SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc DsEgressXcOamRmepHashKey, key_index:%d \n", p_sys_rmep_eth->key_index);


    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_rmep_eth->p_sys_lmep->p_sys_chan->master_chipid))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_asic(lchip, p_sys_rmep_eth));

    if (!CTC_FLAG_ISSET(p_sys_rmep_eth->p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        DsEgressXcOamRmepHashKey_m ds_eth_oam_rmep_hash_key;
        sal_memset(&ds_eth_oam_rmep_hash_key, 0, sizeof(ds_eth_oam_rmep_hash_key));
        CTC_ERROR_RETURN( sys_usw_oam_key_delete_io(lchip, DsEgressXcOamRmepHashKey_t,
                                                      p_sys_rmep_eth->key_index ,
                                                      &ds_eth_oam_rmep_hash_key ));

        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free DsEgressXcOamRmepHashKey, key_index:%d \n", p_sys_rmep_eth->key_index);

    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_rmep_update_asic(uint8 lchip, ctc_oam_update_t* p_rmep_param,
                                    sys_oam_rmep_t* p_sys_rmep_eth)
{
    ctc_oam_update_t* p_eth_rmep = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rmep_param);
    CTC_PTR_VALID_CHECK(p_sys_rmep_eth);

    p_eth_rmep = p_rmep_param;

    switch (p_eth_rmep->update_type)
    {
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        if (1 == p_eth_rmep->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN);
        }

        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_SEQ_FAIL_CLEAR:
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        if (1 == p_eth_rmep->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN);
        }

        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;      /*D2 SF_STATE proc by asic*/

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        if (1 == p_eth_rmep->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN);
        }
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMAC_MIS:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_PORT_INTF_MIS:
        CTC_MAX_VALUE_CHECK(p_eth_rmep->update_value, 1);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS:
        {
            ctc_oam_hw_aps_t* p_hw_aps = NULL;
            p_hw_aps = (ctc_oam_hw_aps_t*)p_eth_rmep->p_update_value;
            CTC_MAX_VALUE_CHECK(p_hw_aps->aps_group_id, MCHIP_CAP(SYS_CAP_APS_GROUP_NUM)-1);
        }
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN:
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_MACSA:
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_update_eth_asic(lchip, p_sys_rmep_eth, p_eth_rmep));

    return CTC_E_NONE;
}

#define COMMON_SET "Function Begin"

int32
_sys_usw_cfm_oam_set_port_tunnel_en(uint8 lchip, uint32 gport, uint8 enable)
{
    uint16 lport = 0;
    uint32 cmd  = 0;
    int32 ret = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* call the sys interface from port module */

    field_value = enable;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_oamTunnelEn_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &field_value);

    return ret;

}

int32
_sys_usw_cfm_oam_set_port_edge_en(uint8 lchip, uint32 gport, uint8 enable)
{

    uint16 lport = 0;
    uint32 cmd  = 0;
    int32 ret = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    field_value = enable;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherOamEdgePort_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &field_value);

    field_value = enable;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherOamEdgePort_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &field_value);

    return ret;

}

int32
_sys_usw_cfm_oam_set_port_oam_en(uint8 lchip, uint32 gport, uint8 dir, uint8 enable)
{

    uint16 lport = 0;
    uint32 cmd  = 0;
    int32 ret = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"gport:%d, dir:%d, enable:%d\n", gport, dir, enable);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* call the sys interface from port module */
    switch (dir)
    {
    case CTC_INGRESS:
        field_value = enable;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherOamValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
        break;

    case CTC_EGRESS:
        field_value = enable;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherOamValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
        break;

    case CTC_BOTH_DIRECTION:
        field_value = enable;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherOamValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherOamValid_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        break;

    default:
        break;
    }

    return ret;

}

int32
_sys_usw_cfm_oam_set_relay_all_to_cpu_en(uint8 lchip, uint8 enable)
{
    uint32 cmd           = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    field_value = enable;
    cmd = DRV_IOW(OamHeaderAdjustCtl_t, OamHeaderAdjustCtl_relayAllToCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_tx_vlan_tpid(uint8 lchip, uint8 tpid_index, uint16 tpid)
{
    uint32 cmd           = 0;
    OamEtherTxCtl_m eth_tx_ctl;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (tpid_index > 3)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&eth_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
    cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));

    DRV_SET_FIELD_V(lchip, OamEtherTxCtl_t, OamEtherTxCtl_tpid0_f + tpid_index , &eth_tx_ctl, tpid);
    cmd = DRV_IOW(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_rx_vlan_tpid(uint8 lchip, uint8 tpid_index, uint16 tpid)
{
    uint32 cmd           = 0;
    OamParserEtherCtl_m par_eth_ctl;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (tpid_index > 3)
    {
        return CTC_E_INVALID_PARAM;
    }


    sal_memset(&par_eth_ctl, 0, sizeof(OamParserEtherCtl_m));
    cmd = DRV_IOR(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_eth_ctl));

    DRV_SET_FIELD_V(lchip, OamParserEtherCtl_t, OamParserEtherCtl_gTpid_0_svlanTpid_f + tpid_index , &par_eth_ctl, tpid);
    cmd = DRV_IOW(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_eth_ctl));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_send_id(uint8 lchip, ctc_oam_eth_senderid_t* p_sender_id)
{
    uint32 cmd      = 0;

    OamEtherSendId_m oam_ether_send_id;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&oam_ether_send_id, 0, sizeof(oam_ether_send_id));
    cmd = DRV_IOW(OamEtherSendId_t, DRV_ENTRY_FLAG);


    DRV_SET_FIELD_V(lchip, OamEtherSendId_t, OamEtherSendId_sendIdLength_f , &oam_ether_send_id,
                        p_sender_id->sender_id_len);
    SetOamEtherSendId(V, sendIdByte0to2_f, &oam_ether_send_id,
                        ( ((p_sender_id->sender_id[0] & 0xFF) << 16) |
                        ((p_sender_id->sender_id[1] & 0xFF) << 8) |
                        ((p_sender_id->sender_id[2] & 0xFF) << 0) ));
    DRV_SET_FIELD_V(lchip, OamEtherSendId_t, OamEtherSendId_sendIdByte3to6_f , &oam_ether_send_id,
                        (((p_sender_id->sender_id[3] & 0xFF) << 24) |
                        ((p_sender_id->sender_id[4] & 0xFF) << 16) |
                        ((p_sender_id->sender_id[5] & 0xFF) << 8) |
                        ((p_sender_id->sender_id[6] & 0xFF) << 0)));
    DRV_SET_FIELD_V(lchip, OamEtherSendId_t, OamEtherSendId_sendIdByte7to10_f , &oam_ether_send_id,
                        (((p_sender_id->sender_id[7] & 0xFF) << 24) |
                        ((p_sender_id->sender_id[8] & 0xFF) << 16) |
                        ((p_sender_id->sender_id[9] & 0xFF) << 8) |
                        ((p_sender_id->sender_id[10] & 0xFF) << 0)));
    DRV_SET_FIELD_V(lchip, OamEtherSendId_t, OamEtherSendId_sendIdByte11to14_f , &oam_ether_send_id,
                        (((p_sender_id->sender_id[11] & 0xFF) << 24) |
                        ((p_sender_id->sender_id[12] & 0xFF) << 16) |
                        ((p_sender_id->sender_id[13] & 0xFF) << 8) |
                        ((p_sender_id->sender_id[14] & 0xFF) << 0)));

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_send_id));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_big_ccm_interval(uint8 lchip, uint8 big_interval)
{
    uint32 cmd      = 0;
    uint32 field_value    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(big_interval, 7);

    field_value = big_interval;

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_minIntervalToCpu_f);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_port_lm_en(uint8 lchip, uint32 gport, uint8 enable)
{
    uint16 lport     = 0;
    uint32 cmd      = 0;
    uint32 field_value    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    field_value = enable;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherLmValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_value));

    field_value = enable;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherLmValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_value));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_bridge_mac(uint8 lchip, mac_addr_t* bridge_mac)
{
    uint32 cmd      = 0;
    OamRxProcEtherCtl_m rx_ether_ctl;
    OamEtherTxCtl_m eth_tx_ctl;
    IpePortMacCtl_m ipe_port_mac;
    EpeL2EditCtl_m epe_l2_edit;
    hw_mac_addr_t   hw_mac = {0};

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)bridge_mac);

    sal_memset(&rx_ether_ctl, 0, sizeof(OamRxProcEtherCtl_m));
    cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_ether_ctl));
    SetOamRxProcEtherCtl(A, bridgeMac_f, &rx_ether_ctl,  hw_mac);
    cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_ether_ctl));

    sal_memset(&eth_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
    cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));
    SetOamEtherTxCtl(A, txBridgeMac_f, &eth_tx_ctl,  hw_mac);
    cmd = DRV_IOW(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));

    cmd = DRV_IOR(IpePortMacCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_port_mac));
    SetIpePortMacCtl(A, y1731ShareMac_f, &ipe_port_mac,  hw_mac);
    cmd = DRV_IOW(IpePortMacCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_port_mac));

    cmd = DRV_IOR(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_l2_edit));
    SetEpeL2EditCtl(A, y1731ShareMac_f, &epe_l2_edit,  hw_mac);
    cmd = DRV_IOW(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_l2_edit));

    return CTC_E_NONE;
}


int32
_sys_usw_cfm_oam_set_lbm_process_by_asic(uint8 lchip, uint8 enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_lbmProcByCpu_f);

    field_value = enable ? 0 : 1;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_set_lmm_process_by_asic(uint8 lchip, uint8 enable)
{

    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_lmmProcByCpu_f);

    field_value = enable ? 0 : 1;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_set_dm_process_by_asic(uint8 lchip, uint8 enable)
{

    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_dmProcByCpu_f);

    field_value = enable ? 0 : 1;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_set_slm_process_by_asic(uint8 lchip, uint8 enable)
{

    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamRxProcEtherCtl_t, OamRxProcEtherCtl_smProcByCpu_f);

    field_value = enable ? 0 : 1;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}


int32
_sys_usw_cfm_oam_set_lbr_sa_use_lbm_da(uint8 lchip, uint8 enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_lbrSaUsingLbmDa_f);

    field_value = enable;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_set_lbr_sa_use_bridge_mac(uint8 lchip, uint8 enable)
{

    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(OamHeaderEditCtl_t, OamHeaderEditCtl_lbrSaType_f);

    field_value = enable;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;

}

int32
 _sys_usw_cfm_set_mip_port_en(uint8 lchip, uint32 gport, uint8 level_bitmap)
{
    uint16  lport       = 0;
    uint32 field = 0;
    uint32 linkOamEn = 0;
    uint32 cmd = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }
    else if (!CTC_IS_LINKAGG_PORT(gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport))))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port, lchip value error \n");
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkOamEn_f);
    DRV_IOCTL(lchip, lport, cmd, &linkOamEn);
    if (!linkOamEn)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_mipEn_f);
        DRV_IOCTL(lchip, lport, cmd, &field);
        field = level_bitmap & 0xFF;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_mipEn_f);
        DRV_IOCTL(lchip, lport, cmd, &field);

        cmd = DRV_IOR(DsDestPort_t, DsDestPort_mipEn_f);
        DRV_IOCTL(lchip, lport, cmd, &field);
        field = level_bitmap & 0xFF;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_mipEn_f);
        DRV_IOCTL(lchip, lport, cmd, &field);
    }
    else
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Port already enable linkoam\n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;


}


int32
 _sys_usw_cfm_set_if_status(uint8 lchip, uint32 gport, uint8 if_status)
{
    uint16  lport       = 0;
    uint32 field = 0;

    uint32 cmd = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    field = if_status & 0xFF;
    cmd = DRV_IOW(DsPortProperty_t, DsPortProperty_ifStatus_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

    return CTC_E_NONE;


}


int32
_sys_usw_cfm_oam_set_ach_channel_type(uint8 lchip, uint16 value)
{
    uint32 field_val = 0;
    uint32 cmd = 0;

    field_val = value;
    cmd = DRV_IOW(ParserLayer4AchCtl_t, ParserLayer4AchCtl_achY1731Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_achY1731ChanType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(OamEtherTxCtl_t, OamEtherTxCtl_achY1731ChanType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}


int32
_sys_usw_cfm_oam_set_rdi_mode(uint8 lchip, uint32 rdi_mode)
{
    uint32 cmd = 0;
    OamRxProcEtherCtl_m oam_rx_proc_ether_ctl;
    cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));
    cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_y1731RdiAutoIndicate_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rdi_mode));

    if (rdi_mode) /*RDI Mode 1*/
    {
        g_oam_master[lchip]->defect_to_rdi_bitmap0 = GetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &oam_rx_proc_ether_ctl);
        g_oam_master[lchip]->defect_to_rdi_bitmap1 = GetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &oam_rx_proc_ether_ctl);
        SetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &oam_rx_proc_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &oam_rx_proc_ether_ctl, 0);
        cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));
    }
    else  /*RDI Mode 0*/
    {
        SetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &oam_rx_proc_ether_ctl, g_oam_master[lchip]->defect_to_rdi_bitmap0);
        SetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &oam_rx_proc_ether_ctl, g_oam_master[lchip]->defect_to_rdi_bitmap1);
        cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_set_ccm_lm_en(uint8 lchip, uint32 enable)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_lmProactiveEtherOamPacket_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &enable));
    cmd = DRV_IOW(EpeOamCtl_t, EpeOamCtl_lmProactiveEtherOamPacket_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &enable));

    return CTC_E_NONE;
}


#define COMMON_GET "Function Begin"

int32
_sys_usw_cfm_oam_get_port_oam_en(uint8 lchip, uint32 gport, uint8 dir, uint8* enable)
{

    uint16 lport = 0;
    uint32 cmd  = 0;
    int32 ret = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* call the sys interface from port module */
    switch (dir)
    {
    case CTC_INGRESS:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_etherOamValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
        *enable = field_value;
        break;

    case CTC_EGRESS:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_etherOamValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
        *enable = field_value;
        break;

    case CTC_BOTH_DIRECTION:
        break;

    default:
        break;
    }

    return ret;

}

int32
_sys_usw_cfm_oam_get_port_tunnel_en(uint8 lchip, uint32 gport, uint8* enable)
{

    uint16 lport = 0;
    uint32 cmd  = 0;
    int32 ret = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* call the sys interface from port module */

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_oamTunnelEn_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &field_value);
    *enable = field_value;

    return ret;

}

int32
_sys_usw_cfm_oam_get_big_ccm_interval(uint8 lchip, uint8* big_interval)
{

    uint32 cmd      = 0;
    uint32 field_value    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    cmd = DRV_IOR(OamRxProcEtherCtl_t, OamRxProcEtherCtl_minIntervalToCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *big_interval = field_value;

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_get_port_lm_en(uint8 lchip, uint32 gport, uint8* enable)
{

    uint16 lport     = 0;
    uint32 cmd      = 0;
    uint32 field_value    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_GLOBAL_PORT_CHECK(gport)

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_etherLmValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_get_bridge_mac(uint8 lchip, mac_addr_t* bridge_mac)
{
    uint32 cmd      = 0;
    mac_addr_t  mac;
    hw_mac_addr_t  hw_mac;
    OamEtherTxCtl_m eth_tx_ctl;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&eth_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
    cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));
    drv_get_field(lchip, OamEtherTxCtl_t, OamEtherTxCtl_txBridgeMac_f, &eth_tx_ctl, (uint32*)&hw_mac);

    SYS_USW_SET_USER_MAC(mac, hw_mac);
    sal_memcpy(bridge_mac, &mac, sizeof(mac_addr_t));

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_get_lbm_process_by_asic(uint8 lchip, uint8* enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(OamRxProcEtherCtl_t, OamRxProcEtherCtl_lbmProcByCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;

}


int32
_sys_usw_cfm_oam_get_lmm_process_by_asic(uint8 lchip, uint8* enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(OamRxProcEtherCtl_t, OamRxProcEtherCtl_lmmProcByCpu_f);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_get_dm_process_by_asic(uint8 lchip, uint8* enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(OamRxProcEtherCtl_t, OamRxProcEtherCtl_dmProcByCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_get_slm_process_by_asic(uint8 lchip, uint8* enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(OamRxProcEtherCtl_t, OamRxProcEtherCtl_smProcByCpu_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;

}


int32
_sys_usw_cfm_oam_get_lbr_sa_use_lbm_da(uint8 lchip, uint8* enable)
{
    uint32 cmd      = 0;
    uint32 field_value = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(OamHeaderEditCtl_t, OamHeaderEditCtl_lbrSaUsingLbmDa_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    *enable = field_value;

    return CTC_E_NONE;
}


int32
_sys_usw_cfm_oam_get_mip_port_en(uint8 lchip, uint32 gport, uint8* level_bitmap)
{

    uint16  lport       = 0;
    uint32 field = 0;
    uint32 cmd = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }
    else if (!CTC_IS_LINKAGG_PORT(gport)
        && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(gport))))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port, lchip value error \n");
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_mipEn_f);
    DRV_IOCTL(lchip, lport, cmd, &field);
    *level_bitmap = field & 0xFF;

    return CTC_E_NONE;

}


int32
_sys_usw_cfm_oam_get_if_status(uint8 lchip, uint32 gport, uint8* if_status)
{

    uint16  lport       = 0;
    uint32 field = 0;
    uint32 cmd = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global port \n");
        return CTC_E_INVALID_PORT;
    }

    cmd = DRV_IOR(DsPortProperty_t, DsPortProperty_ifStatus_f);
    DRV_IOCTL(lchip, lport, cmd, &field);
    *if_status = field & 0xFF;

    return CTC_E_NONE;

}

int32
_sys_usw_cfm_oam_get_ach_channel_type(uint8 lchip, uint16* value)
{
    uint32 field_val = 0;
    uint32 cmd = 0;

    cmd = DRV_IOR(ParserLayer4AchCtl_t, ParserLayer4AchCtl_achY1731Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    *value = field_val;

    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_get_rdi_mode(uint8 lchip, uint32* rdi_mode)
{
    uint32 cmd = 0;
    cmd = DRV_IOR(OamUpdateCtl_t, OamUpdateCtl_y1731RdiAutoIndicate_f);
    DRV_IOCTL(lchip, 0, cmd, rdi_mode);
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_oam_get_ccm_lm_en(uint8 lchip, uint32* enable)
{
    uint32 cmd = 0;
    cmd = DRV_IOR(IpeOamCtl_t, IpeOamCtl_lmProactiveEtherOamPacket_f);
    DRV_IOCTL(lchip, 0, cmd, enable);
    return CTC_E_NONE;
}

int32
_sys_usw_cfm_register_init(uint8 lchip)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint8 gchip_id = 0;
    uint32 nexthop_ptr_bridge = 0;
    uint32 nexthop_ptr_bypass = 0;
    uint8 rdi_index       = 0;
    uint16 mp_port_id = 0;
    uint32 discard_map_val[2];
    uint32 discard_up_lm_en[2];
    uint32 discard_down_lm_en[2];
    uint32 ether_defect_to_rdi0 = 0;
    uint32 ether_defect_to_rdi1  = 0;
    uint8  sub_queue_id = 0;
    mac_addr_t mac;
    uint32 ccm_mac_da[2];
    hw_mac_addr_t   hw_mac = {0};

    uint8 i = 0;
    uint8  defect_type      = 0;
    uint8  defect_sub_type  = 0;
    IpeFwdCtl_m ipe_fwd_ctl;

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip_id));

    /*Parser*/
    field_val = 1;
    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_ethOamTypeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    {
        OamParserLayer2ProtocolCam_m par_l2_cam;
        sal_memset(&par_l2_cam, 0, sizeof(OamParserLayer2ProtocolCam_m));
        SetOamParserLayer2ProtocolCam(V, camValue0_f, &par_l2_cam, 0x408902);
        SetOamParserLayer2ProtocolCam(V, camMask0_f, &par_l2_cam, 0x40FFFF);
        SetOamParserLayer2ProtocolCam(V, additionalOffset0_f, &par_l2_cam, 0);
        SetOamParserLayer2ProtocolCam(V, camLayer3Type0_f, &par_l2_cam, 8);
        cmd = DRV_IOW(OamParserLayer2ProtocolCam_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_l2_cam));
    }

    {
        OamParserLayer2ProtocolCamValid_m par_l2_cam_valid;
        sal_memset(&par_l2_cam_valid, 0, sizeof(OamParserLayer2ProtocolCamValid_m));
        SetOamParserLayer2ProtocolCamValid(V, layer2CamEntryValid_f, &par_l2_cam_valid, 1);
        cmd = DRV_IOW(OamParserLayer2ProtocolCamValid_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_l2_cam_valid));
    }

    {
        OamParserEtherCtl_m par_eth_ctl;
        sal_memset(&par_eth_ctl, 0, sizeof(OamParserEtherCtl_m));
        cmd = DRV_IOR(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_eth_ctl));
        SetOamParserEtherCtl(V, minMepId_f, &par_eth_ctl, 0);
        SetOamParserEtherCtl(V, maxMepId_f, &par_eth_ctl, 8191);
        SetOamParserEtherCtl(V, cfmPduMacDaMdLvlCheckEn_f, &par_eth_ctl, 1);
        SetOamParserEtherCtl(V, tlvLengthCheckEn_f, &par_eth_ctl, 1);
        SetOamParserEtherCtl(V, allowNonZeroOui_f, &par_eth_ctl, 1);
        SetOamParserEtherCtl(V, cfmPduMinLength_f, &par_eth_ctl, 0);
        SetOamParserEtherCtl(V, cfmPduMaxLength_f, &par_eth_ctl, 132);
        SetOamParserEtherCtl(V, firstTlvOffsetChk_f, &par_eth_ctl, 70);
        SetOamParserEtherCtl(V, mdNameLengthChk_f, &par_eth_ctl, 43);
        SetOamParserEtherCtl(V, maIdLengthChk_f, &par_eth_ctl, 48);
        SetOamParserEtherCtl(V, minIfStatusTlvValue_f, &par_eth_ctl, 0);
        SetOamParserEtherCtl(V, maxIfStatusTlvValue_f, &par_eth_ctl, 7);
        SetOamParserEtherCtl(V, minPortStatusTlvValue_f, &par_eth_ctl, 0);
        SetOamParserEtherCtl(V, maxPortStatusTlvValue_f, &par_eth_ctl, 3);
        SetOamParserEtherCtl(V, ignoreEthOamVersion_f, &par_eth_ctl, 1);
        SetOamParserEtherCtl(V, invalidCcmIntervalCheckEn_f, &par_eth_ctl, 1);
        SetOamParserEtherCtl(V, maxLengthField_f, &par_eth_ctl, 1536);
        SetOamParserEtherCtl(V, gTpid_0_svlanTpid_f, &par_eth_ctl, 0x8100);
        SetOamParserEtherCtl(V, gTpid_1_svlanTpid_f, &par_eth_ctl, 0x8100);
        SetOamParserEtherCtl(V, gTpid_2_svlanTpid_f, &par_eth_ctl, 0x9100);
        SetOamParserEtherCtl(V, gTpid_3_svlanTpid_f, &par_eth_ctl, 0x88a8);
        SetOamParserEtherCtl(V, cvlanTpid_f, &par_eth_ctl, 0x8100);
        SetOamParserEtherCtl(V,  y1731MccOpcode_f , &par_eth_ctl, 41);
        SetOamParserEtherCtl(V,  y1731SccOpcode_f , &par_eth_ctl, 50);

/*TBD, Conform for spec v2.2.0*/
#if 0
        SetOamParserEtherCtl(V,  y1731SlmOpcode_f , &par_eth_ctl, 55);
        SetOamParserEtherCtl(V,  y1731SlrOpcode_f , &par_eth_ctl, 54);
#endif
        cmd = DRV_IOW(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &par_eth_ctl));
    }

    /*linkoam*/
    field_val = 0;
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_taggedLinkOamEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*Lookup*/
    field_val = 1;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_oamLookupUserVlanId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
     /*-cmd = DRV_IOW(EpeAclQosCtl_t, EpeAclQosCtl_oamLookupUserVlanId_f);*/
     /*-CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));*/

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_oamFidEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*Ipe/Epe process*/
    field_val = 1;
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_oamTunnelCheckType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_untaggedServiceOamEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = gchip_id;              /*global chipid*/
    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_linkOamDestChipId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_mipEthOamDestChipId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_mipTpOamDestChipId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_selfAdressChkByPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpeOamCtl_t, IpeOamCtl_selfAdressChkByDsMac_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


    field_val = 1;
    cmd = DRV_IOW(EpeOamCtl_t, EpeOamCtl_selfAddressChkByPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(EpeOamCtl_t, EpeOamCtl_selfAddressChkByAcl_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


    field_val = 1;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_oamBypassPolicingOp_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = 1;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_oamBypassStormCtlOp_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    discard_map_val[0] = 0xFFFFFFFF;
    CTC_BIT_UNSET(discard_map_val[0], 26);/*IPEDISCARDTYPE_DS_FWD_DESTID_DIS for mpls bfd*/
    discard_map_val[1] = 0xFFFFFFFF;

    sal_memset(&ipe_fwd_ctl, 0, sizeof(IpeFwdCtl_m));
    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
    SetIpeFwdCtl(A, oamDiscardBitmap_f, &ipe_fwd_ctl, discard_map_val);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));

    if(!DRV_IS_DUET2(lchip))
    {
        IpeAclQosCtl_m ipe_acl_qos_ctl;
        sal_memset(&ipe_acl_qos_ctl,0,sizeof(ipe_acl_qos_ctl));
        cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
        SetIpeAclQosCtl(A, oamDiscardBitmap_f, &ipe_acl_qos_ctl, discard_map_val);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl));
    }

    discard_map_val[0] = 0xFFFFFFFF;
    discard_map_val[1] = 0xFFFFFFFF;
    {
        EpeHeaderEditCtl_m epe_header_edit_ctl;
        sal_memset(&epe_header_edit_ctl,0,sizeof(EpeHeaderEditCtl_m));
        cmd = DRV_IOR(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_header_edit_ctl));
        SetEpeHeaderEditCtl(A, oamDiscardBitmap_f, &epe_header_edit_ctl, discard_map_val);
        cmd = DRV_IOW(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_header_edit_ctl));
    }

    field_val = 1;
    cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_oamIgnorePayloadOperation_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_linkOamDiscardEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*OamEngine Header Adjust*/
    {
        OamHeaderAdjustCtl_m adjust_ctl;
        sal_memset(&adjust_ctl, 0, sizeof(OamHeaderAdjustCtl_m));
        cmd = DRV_IOR(OamHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &adjust_ctl));
        SetOamHeaderAdjustCtl(V, oamPduBypassOamEngine_f, &adjust_ctl, 0);
        SetOamHeaderAdjustCtl(V, relayAllToCpu_f, &adjust_ctl, 0);
        cmd = DRV_IOW(OamHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &adjust_ctl));
    }

    /*OamEngine Rx Process*/
    {
        OamRxProcEtherCtl_m rx_ether_ctl;
        sal_memset(&rx_ether_ctl, 0, sizeof(OamRxProcEtherCtl_m));
        cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_ether_ctl));

         /*-SetOamRxProcEtherCtl(V, oamMacDaChkEn_f, &rx_ether_ctl, 1); //TBD, Confirm for spec v2.2.0*/
        SetOamRxProcEtherCtl(V, macStatusChangeChkEn_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, minIntervalToCpu_f, &rx_ether_ctl, 7);
        /*TBD, Confirm for spec v2.2.0*/
        #if 0
        SetOamRxProcEtherCtl(V, lbmMacdaCheckEn_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, lmmMacdaCheckEn_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, dmMacDaCheckEn_f, &rx_ether_ctl, 1);
        #endif
        SetOamRxProcEtherCtl(V, lbmProcByCpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, lmmProcByCpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, dmProcByCpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, smProcByCpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, twampProcByCpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, ccmWithUnknownTlvTocpu_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, alarmSrcMacMismatch_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, dUnexpPeriodTimerCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, dMegLvlTimerCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, dMismergeTimerCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, dUnexpMepTimerCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, rmepWhileCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, ethCsfLos_f, &rx_ether_ctl, 0);
        SetOamRxProcEtherCtl(V, ethCsfFdi_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, ethCsfRdi_f, &rx_ether_ctl, 2);
        SetOamRxProcEtherCtl(V, ethCsfClear_f, &rx_ether_ctl, 3);
        SetOamRxProcEtherCtl(V, array2_0_rxCsfWhileCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, array2_1_rxCsfWhileCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, array2_2_rxCsfWhileCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, array2_3_rxCsfWhileCfg_f, &rx_ether_ctl, 14);
        SetOamRxProcEtherCtl(V, array_0_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_1_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_2_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_3_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_4_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_5_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_6_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, array_7_csfWhileCfg_f, &rx_ether_ctl, 1);
        SetOamRxProcEtherCtl(V, seqnumFailReportThrd_f, &rx_ether_ctl, 0x1F);
        SetOamRxProcEtherCtl(V, unknownCcmThrd_f, &rx_ether_ctl, 3);
        SetOamRxProcEtherCtl(V, oamDiscardCntEn_f, &rx_ether_ctl, 7);

        ether_defect_to_rdi0 = 0;
        ether_defect_to_rdi1 = 0;
        for (i = 0; i < CTC_USW_OAM_DEFECT_NUM; i++)
        {
            if((CTC_OAM_DEFECT_MAC_STATUS_CHANGE == (1 << i)) || (CTC_OAM_DEFECT_SRC_MAC_MISMATCH == (1 << i)))
            {
                /*CTC_OAM_DEFECT_MAC_STATUS_CHANGE and CTC_OAM_DEFECT_SRC_MAC_MISMATCH not set rdi*/
                continue;
            }
            _sys_usw_oam_get_rdi_defect_type(lchip, (1 << i), &defect_type, &defect_sub_type);
            rdi_index = (defect_type << 3) | defect_sub_type;
            if (rdi_index < 32)
            {
                CTC_BIT_SET(ether_defect_to_rdi0, rdi_index);
            }
            else
            {
                CTC_BIT_SET(ether_defect_to_rdi1, (rdi_index - 32));
            }
        }

        SetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &rx_ether_ctl, ether_defect_to_rdi0);
        SetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &rx_ether_ctl, ether_defect_to_rdi1);

        cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_ether_ctl));
        g_oam_master[lchip]->defect_to_rdi_bitmap0 = ether_defect_to_rdi0;
        g_oam_master[lchip]->defect_to_rdi_bitmap1 = ether_defect_to_rdi1;

    }

    /*OamEngine Header Edit*/
    {
        OamHeaderEditCtl_m hdr_edit_ctl;
        sal_memset(&hdr_edit_ctl, 0, sizeof(OamHeaderEditCtl_m));
        cmd = DRV_IOR(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_edit_ctl));
        SetOamHeaderEditCtl(V, iLoopDestMap_f, &hdr_edit_ctl, SYS_ENCODE_DESTMAP(gchip_id, SYS_RSV_PORT_ILOOP_ID));
        SetOamHeaderEditCtl(V, lbrSaType_f, &hdr_edit_ctl, 0);
        SetOamHeaderEditCtl(V, lbrSaUsingLbmDa_f, &hdr_edit_ctl, 1);
        SetOamHeaderEditCtl(V, localChipId_f, &hdr_edit_ctl, gchip_id);

        CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_OAM + CTC_OAM_EXCP_ALL_TO_CPU, &sub_queue_id));
        SetOamHeaderEditCtl(V, relayAllToCpuDestmap_f, &hdr_edit_ctl, SYS_ENCODE_EXCP_DESTMAP(gchip_id, sub_queue_id));
        SetOamHeaderEditCtl(V, relayAllToCpuNexthopPtr_f, &hdr_edit_ctl, CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_OAM + CTC_OAM_EXCP_ALL_TO_CPU, 0));

        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH,
                                                               &nexthop_ptr_bridge));
        SetOamHeaderEditCtl(V, bypassNextHopPtr_f, &hdr_edit_ctl, nexthop_ptr_bridge);
        SetOamHeaderEditCtl(V, rbdEn_f, &hdr_edit_ctl, 1);
        SetOamHeaderEditCtl(V, redirectBypassDestmap_f, &hdr_edit_ctl, SYS_ENCODE_DESTMAP(gchip_id, SYS_RSV_PORT_DROP_ID));
        SetOamHeaderEditCtl(V, oamPortId_f, &hdr_edit_ctl, MCHIP_CAP(SYS_CAP_CHANID_OAM));
        cmd = DRV_IOW(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hdr_edit_ctl));
    }

    /*OamEngine Tx*/
    {
        OamEtherTxCtl_m eth_tx_ctl;
        sal_memset(&eth_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
        cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));
        SetOamEtherTxCtl(V, iloopChanId_f, &eth_tx_ctl,  SYS_RSV_PORT_ILOOP_ID);
        SetOamEtherTxCtl(V, oamChipId_f, &eth_tx_ctl, gchip_id);
        SetOamEtherTxCtl(V, ccmEtherType_f, &eth_tx_ctl, 0x8902);
        SetOamEtherTxCtl(V, ccmVersion_f, &eth_tx_ctl, 0);
        SetOamEtherTxCtl(V, ccmOpcode_f, &eth_tx_ctl, 1);
        SetOamEtherTxCtl(V, ccmFirstTlvOffset_f, &eth_tx_ctl, 70);
        SetOamEtherTxCtl(V, tpid0_f, &eth_tx_ctl, 0x8100);
        SetOamEtherTxCtl(V, tpid1_f, &eth_tx_ctl, 0x8100);
        SetOamEtherTxCtl(V, tpid2_f, &eth_tx_ctl, 0x9100);
        SetOamEtherTxCtl(V, tpid3_f, &eth_tx_ctl, 0x88a8);
        SetOamEtherTxCtl(V, oamPortId_f, &eth_tx_ctl, MCHIP_CAP(SYS_CAP_CHANID_OAM));
        SetOamEtherTxCtl(V, txMacKnown_f, &eth_tx_ctl, 1);
        SetOamEtherTxCtl(V, ctpid_f, &eth_tx_ctl, 0x8100);


        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH,
                                                              &nexthop_ptr_bridge));
        SetOamEtherTxCtl(V, bridgeNexthopPtr_f, &eth_tx_ctl, nexthop_ptr_bridge);


        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH,
                                                              &nexthop_ptr_bypass));
        SetOamEtherTxCtl(V, bypassNexthopPtr_f, &eth_tx_ctl, nexthop_ptr_bypass);

        /*mcast mac da 0x0180c2000300(last 3bit is level)*/
        ccm_mac_da[1] = 0x30;
        ccm_mac_da[0]= 0x18400006;
        drv_set_field(lchip, OamEtherTxCtl_t, OamEtherTxCtl_cfmMcastAddr_f, &eth_tx_ctl, ccm_mac_da);

        cmd = DRV_IOW(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &eth_tx_ctl));
    }

    mac[0] = 0x0;
    mac[1] = 0x11;
    mac[2] = 0x11;
    mac[3] = 0x11;
    mac[4] = 0x11;
    mac[5] = 0x11;
    _sys_usw_cfm_oam_set_bridge_mac(lchip, &mac);

    {
        DsPortProperty_m ds_port_property;
        sal_memset(&ds_port_property, 0, sizeof(DsPortProperty_m));
        cmd = DRV_IOW(DsPortProperty_t, DRV_ENTRY_FLAG);

        for (mp_port_id = 0; mp_port_id < 192; mp_port_id++)
        {
            mac[0] = 0x0;
            mac[1] = 0x0;
            mac[2] = 0x0;
            mac[3] = 0x22;
            mac[4] = (mp_port_id >> 8);
            mac[5] = (mp_port_id&0xFF);
            SYS_USW_SET_HW_MAC(hw_mac, mac);
            SetDsPortProperty(A, macSaByte_f, &ds_port_property, hw_mac);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mp_port_id, cmd, &ds_port_property));
        }
    }

    /*OamEngine Update*/
     /*#define CHIP_CORE_FRE  450*/
    {
        OamUpdateCtl_m oam_update_ctl;
        sal_memset(&oam_update_ctl, 0, sizeof(OamUpdateCtl_m));
        cmd = DRV_IOR(OamUpdateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
        SetOamUpdateCtl(V, ituDefectClearMode_f, &oam_update_ctl, 1);
        SetOamUpdateCtl(V, cntShiftWhileCfg_f, &oam_update_ctl, 4);
        SetOamUpdateCtl(V, cciWhileCfg_f, &oam_update_ctl, 4);
        SetOamUpdateCtl(V, genRdiByDloc_f, &oam_update_ctl, 1);

        cmd = DRV_IOW(OamUpdateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
    }

    /*ipe oam register*/
    {
        IpeOamCtl_m ipe_oam_ctl;
        sal_memset(&ipe_oam_ctl, 0, sizeof(IpeOamCtl_m));
        cmd = DRV_IOR(IpeOamCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_oam_ctl));
        discard_up_lm_en[0] = 0xFFFFFFFF;
        discard_up_lm_en[1] = 0xFFFFFFFF;
        discard_down_lm_en[0] = 0xFFFFFFFF;
        discard_down_lm_en[1] = 0xFFFFFFFF;
        SetIpeOamCtl(V, lmProactiveEtherOamPacket_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, linkOamDestChipId_f, &ipe_oam_ctl, gchip_id);
        SetIpeOamCtl(V, mipEthOamDestChipId_f, &ipe_oam_ctl, gchip_id);
        SetIpeOamCtl(V, mipTpOamDestChipId_f, &ipe_oam_ctl, gchip_id);
        SetIpeOamCtl(V, exception2DiscardDownLmEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, exception2DiscardUpLmEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, discardLinkLmEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(A, discardDownLmEn_f, &ipe_oam_ctl, discard_up_lm_en);
        SetIpeOamCtl(A, discardUpLmEn_f, &ipe_oam_ctl, discard_down_lm_en);
        SetIpeOamCtl(V, mcMacChkEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, ucMacChkEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, ltmMacChkEn_f, &ipe_oam_ctl, 1);
        SetIpeOamCtl(V, oamFilterDiscardOpCode_f, &ipe_oam_ctl, 0);
        cmd = DRV_IOW(IpeOamCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_oam_ctl));
    }

    /*epe oam register*/
    {
        EpeOamCtl_m epe_oam_ctl;
        sal_memset(&epe_oam_ctl, 0, sizeof(EpeOamCtl_m));
        cmd = DRV_IOR(EpeOamCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_oam_ctl));
        SetEpeOamCtl(V, lmProactiveEtherOamPacket_f, &epe_oam_ctl, 1);
        SetEpeOamCtl(V, discardLinkLmEn_f, &epe_oam_ctl, 1);
        SetEpeOamCtl(V, lmGreenPacket_f, &epe_oam_ctl, 0);
        SetEpeOamCtl(A, discardDownLmEn_f, &epe_oam_ctl, discard_down_lm_en);
        SetEpeOamCtl(A, discardUpLmEn_f, &epe_oam_ctl, discard_up_lm_en);
        SetEpeOamCtl(V, mcMacChkEn_f, &epe_oam_ctl, 1);
        SetEpeOamCtl(V, ucMacChkEn_f, &epe_oam_ctl, 1);
        SetEpeOamCtl(V, ltmMacChkEn_f, &epe_oam_ctl, 1);
        SetEpeOamCtl(V, mipEthOamDestChipId_f, &epe_oam_ctl, gchip_id);
        SetEpeOamCtl(V, oamFilterDiscardOpCode_f, &epe_oam_ctl, 0);
        cmd = DRV_IOW(EpeOamCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_oam_ctl));
    }

    /*for spec bug7441*/
    field_val = 1;
    cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_srcDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_RSV_PORT_DROP_ID, cmd, &field_val));

    return CTC_E_NONE;
}

#endif
