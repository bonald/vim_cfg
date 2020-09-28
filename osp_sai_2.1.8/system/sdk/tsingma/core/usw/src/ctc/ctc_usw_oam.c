#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_oam.c

 @date 2010-3-9

 @version v2.0

  This file contains  OAM (Ethernet OAM/MPLS OAM/PBX OAM and EFM OAM) associated APIs's implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_usw_oam.h"
#include "sys_usw_oam.h"
#include "sys_usw_common.h"
#include "sys_usw_npm.h"

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

/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32
ctc_usw_oam_add_maid(uint8 lchip, ctc_oam_maid_t* p_maid)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN( CTC_E_EXIST,
                                  ret,
                                  sys_usw_oam_add_maid(lchip, p_maid));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_oam_remove_maid(lchip, p_maid);
    }

    return ret;
}

int32
ctc_usw_oam_remove_maid(uint8 lchip, ctc_oam_maid_t* p_maid)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_remove_maid(lchip, p_maid));
    }

    return ret;
}

int32
ctc_usw_oam_add_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
                                  ret,
                                  sys_usw_oam_add_lmep(lchip, p_lmep));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_oam_remove_lmep(lchip, p_lmep);
    }

    return ret;

}

int32
ctc_usw_oam_remove_lmep(uint8 lchip, ctc_oam_lmep_t* p_lmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_remove_lmep(lchip, p_lmep));

    }

    return ret;
}

int32
ctc_usw_oam_update_lmep(uint8 lchip, ctc_oam_update_t* p_lmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_update_lmep(lchip, p_lmep));
    }

    return ret;
}

int32
ctc_usw_oam_add_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_add_rmep(lchip, p_rmep));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_oam_remove_rmep(lchip, p_rmep);
    }

    return ret;
}

int32
ctc_usw_oam_remove_rmep(uint8 lchip, ctc_oam_rmep_t* p_rmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_remove_rmep(lchip, p_rmep));
    }

    return ret;
}

int32
ctc_usw_oam_update_rmep(uint8 lchip, ctc_oam_update_t* p_rmep)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_update_rmep(lchip, p_rmep));
    }

    return ret;
}

int32
ctc_usw_oam_add_mip(uint8 lchip, ctc_oam_mip_t* p_mip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
                                  ret,
                                  sys_usw_oam_add_mip(lchip, p_mip));
    }

    /*rollback if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_oam_remove_mip(lchip, p_mip);
    }

    return ret;
}

int32
ctc_usw_oam_remove_mip(uint8 lchip, ctc_oam_mip_t* p_mip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_remove_mip(lchip, p_mip));
    }

    return ret;
}

int32
ctc_usw_oam_set_property(uint8 lchip, ctc_oam_property_t* p_prop)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_chip                 = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_prop);
    if (CTC_OAM_PROPERTY_TYPE_COMMON == p_prop->oam_pro_type)
    {
        all_chip = 1;
    }
    else if(CTC_OAM_PROPERTY_TYPE_Y1731 == p_prop->oam_pro_type)
    {
        switch(p_prop->u.y1731.cfg_type)
        {
        case CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN:
        case CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN:
        case CTC_OAM_Y1731_CFG_TYPE_ADD_EDGE_PORT:
        case CTC_OAM_Y1731_CFG_TYPE_REMOVE_EDGE_PORT:
        case CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN:
        case CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE:
        case CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN:
            SYS_MAP_GPORT_TO_LCHIP(p_prop->u.y1731.gport, lchip);
            break;

        case CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU:
        case CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC:
        case CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC:
        case CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC:
        case CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC:
        case CTC_OAM_Y1731_CFG_TYPE_ALL_TO_CPU:
        case CTC_OAM_Y1731_CFG_TYPE_TX_VLAN_TPID:
        case CTC_OAM_Y1731_CFG_TYPE_RX_VLAN_TPID:
        case CTC_OAM_Y1731_CFG_TYPE_SENDER_ID:
        case CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC:
        case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA:
        case CTC_OAM_Y1731_CFG_TYPE_LBR_SA_SHARE_MAC:
        default:
            all_chip = 1;
        }
    }
    else if(CTC_OAM_PROPERTY_TYPE_BFD == p_prop->oam_pro_type)
    {
        all_chip = 1;
    }


    lchip_start = lchip;
    lchip_end   = lchip + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_chip)
    {
        CTC_ERROR_RETURN(sys_usw_oam_set_property(lchip, p_prop));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_oam_get_defect_info(uint8 lchip, void* p_defect_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_oam_get_defect_info(lchip, p_defect_info));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_oam_get_mep_info(uint8 lchip, ctc_oam_mep_info_t* p_mep_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_oam_get_mep_info(lchip, p_mep_info, 0));
    }

    return CTC_E_NONE;
}


int32
ctc_usw_oam_get_mep_info_with_key(uint8 lchip, ctc_oam_mep_info_with_key_t* p_mep_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_get_mep_info(lchip, p_mep_info, 1));
    }

    return ret;
}

int32
ctc_usw_oam_get_stats(uint8 lchip, ctc_oam_stats_info_t* p_stat_info)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_EXIST,
                                  ret,
                                  sys_usw_oam_get_stats_info(lchip, p_stat_info));
    }

    return ret;
}

int32
ctc_usw_oam_set_trpt_cfg(uint8 lchip, ctc_oam_trpt_t* p_trpt)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    ctc_npm_cfg_t cfg;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    CTC_PTR_VALID_CHECK(p_trpt);
    LCHIP_CHECK(lchip);

    sal_memset(&cfg, 0, sizeof(ctc_npm_cfg_t));
    if (CTC_OAM_TX_TYPE_PACKET == p_trpt->tx_mode)
    {
        cfg.tx_mode = CTC_NPM_TX_MODE_PACKET_NUM;
    }
    else if (CTC_OAM_TX_TYPE_CONTINUOUS == p_trpt->tx_mode)
    {
        cfg.tx_mode = CTC_NPM_TX_MODE_CONTINUOUS;
    }
    else
    {
        cfg.tx_mode = p_trpt->tx_mode;
    }
    cfg.session_id = p_trpt->session_id;
    cfg.vlan_id = p_trpt->vlan_id;
    cfg.dest_gport = p_trpt->gport;
    if(p_trpt->nhid)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_MCAST;
    }
    if (CTC_INGRESS == p_trpt->direction)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_ILOOP;
    }
    if (p_trpt->tx_seq_en)
    {
        cfg.flag |= CTC_NPM_CFG_FLAG_SEQ_EN;
    }
    cfg.rate = p_trpt->rate;
    cfg.tx_period = p_trpt->tx_period;
    cfg.packet_num = p_trpt->packet_num;
    cfg.pkt_format.frame_size = p_trpt->size;
    cfg.pkt_format.header_len = p_trpt->header_len;
    cfg.pkt_format.seq_num_offset = p_trpt->seq_num_offset;
    cfg.pkt_format.repeat_pattern = p_trpt->repeat_pattern;
    cfg.pkt_format.pattern_type = p_trpt->pattern_type;
    cfg.pkt_format.pkt_header = p_trpt->pkt_header;
    cfg.pkt_format.ipg = 20;

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_npm_set_config(lchip, &cfg));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_oam_set_trpt_en(uint8 lchip, uint8 gchip, uint8 session_id, uint8 enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip);
    CTC_ERROR_RETURN(sys_usw_npm_set_transmit_en(lchip, session_id, enable));
    return CTC_E_NONE;
}

int32
ctc_usw_oam_get_trpt_stats(uint8 lchip, uint8 gchip, uint8 session_id,  ctc_oam_trpt_stats_t* p_trpt_stats)
{
    ctc_npm_stats_t stats;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    CTC_PTR_VALID_CHECK(p_trpt_stats);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip);

    sal_memset(&stats, 0, sizeof(ctc_npm_stats_t));
    CTC_ERROR_RETURN(sys_usw_npm_get_stats(lchip, session_id, &stats));
    p_trpt_stats->rx_oct = stats.rx_bytes;
    p_trpt_stats->rx_pkt = stats.rx_pkts;
    p_trpt_stats->tx_oct = stats.tx_bytes;
    p_trpt_stats->tx_pkt = stats.tx_pkts;
    p_trpt_stats->tx_fcf = stats.tx_fcf;
    p_trpt_stats->tx_fcb = stats.tx_fcb;
    p_trpt_stats->rx_fcl = stats.rx_fcl;

    return CTC_E_NONE;
}

int32
ctc_usw_oam_clear_trpt_stats(uint8 lchip, uint8 gchip, uint8 session_id)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_OAM);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip);
    CTC_ERROR_RETURN(sys_usw_npm_clear_stats(lchip, session_id));
    return CTC_E_NONE;
}

int32
ctc_usw_oam_init(uint8 lchip, ctc_oam_global_t* p_cfg)
{
    ctc_oam_global_t oam_global;
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);
    if (NULL == p_cfg)
    {
        sal_memset(&oam_global, 0, sizeof(ctc_oam_global_t));

        /*eth oam global param*/
        oam_global.mep_index_alloc_by_sdk       = 1;

        /*TP Y.1731 OAM*/
        oam_global.tp_y1731_ach_chan_type       = 0x8902;
        oam_global.tp_section_oam_based_l3if    = 0;

        oam_global.tp_csf_ach_chan_type = 9;
        oam_global.tp_csf_los = 0x7;
        oam_global.tp_csf_fdi = 0x1;
        oam_global.tp_csf_rdi = 0x2;
        oam_global.tp_csf_clear = 0x0;
    }
    else
    {
        sal_memcpy(&oam_global, p_cfg, sizeof(ctc_oam_global_t));
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_oam_init(lchip, &oam_global));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_oam_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_oam_deinit(lchip));
    }

    return CTC_E_NONE;
}

#endif

