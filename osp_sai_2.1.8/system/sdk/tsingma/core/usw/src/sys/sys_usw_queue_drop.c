/**
 @file sys_usw_queue_drop.c

 @date 2010-01-13

 @version v2.0

*/

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_ftm.h"
#include "sys_usw_qos.h"
#include "sys_usw_dmps.h"
#include "sys_usw_interrupt.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 ****************************************************************************/

#define SYS_MAX_DROP_PROB (32 - 1)
#define SYS_MAX_DROP_FACTOR 10
#define SYS_MAX_FCDL_INTERVAL_TIME 53
#define SYS_MAX_FCDL_MULT_NUM 255

#define SYS_RESRC_IGS_COND(tc_min,total,sc,tc,port,port_tc) \
    ((tc_min<<5)|(total<<4)|(sc<<3)|(tc<<2)|(port <<1)|(port_tc<<0))

#define SYS_RESRC_EGS_COND(que_min,total,sc_pkt,sc,tc,port, grp,queue) \
    ((que_min<<7)|(total<<6)|(sc_pkt<<5)|(sc<<4)|(tc<<3)|(port<<2)|(grp<<1)|(queue<<0))


extern sys_queue_master_t* p_usw_queue_master[CTC_MAX_LOCAL_CHIP_NUM];

/****************************************************************************
 *
 * Function
 *
 ****************************************************************************/

STATIC uint32
_sys_usw_qos_fc_hash_make_profile(sys_qos_fc_profile_t* p_prof)
{
    uint8* data= (uint8*)(p_prof);
    uint8   length = sizeof(sys_qos_fc_profile_t) - sizeof(uint16);

    return ctc_hash_caculate(length, data);

    return CTC_E_NONE;
}

/**
 @brief Queue drop hash comparison hook.
*/
STATIC bool
_sys_usw_qos_fc_hash_cmp_profile(sys_qos_fc_profile_t* p_prof1,
                                           sys_qos_fc_profile_t* p_prof2)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    if (!sal_memcmp((uint8*)(p_prof1), (uint8*)(p_prof2), sizeof(sys_qos_fc_profile_t) - sizeof(uint16)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_qos_fc_alloc_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type  = p_node->type ? p_usw_queue_master[*p_lchip]->opf_type_resrc_fc_dropthrd : p_usw_queue_master[*p_lchip]->opf_type_resrc_fc;
    opf.pool_index = 0;
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_fc_free_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_node->type ? p_usw_queue_master[*p_lchip]->opf_type_resrc_fc_dropthrd : p_usw_queue_master[*p_lchip]->opf_type_resrc_fc;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

int32
_sys_usw_qos_pfc_alloc_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_node->type ? p_usw_queue_master[*p_lchip]->opf_type_resrc_pfc_dropthrd : p_usw_queue_master[*p_lchip]->opf_type_resrc_pfc;
    opf.pool_index = 0;
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_pfc_free_profileId(sys_qos_fc_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_node->type ? p_usw_queue_master[*p_lchip]->opf_type_resrc_pfc_dropthrd : p_usw_queue_master[*p_lchip]->opf_type_resrc_pfc;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

int32
_sys_usw_qos_fc_profile_add_spool(uint8 lchip, uint8 is_pfc, uint8 is_dropthrd, sys_qos_fc_profile_t* p_sys_profile_old,
                                                          sys_qos_fc_profile_t* p_sys_profile_new,
                                                          sys_qos_fc_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_qos_fc_hash_cmp_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    if (is_pfc)
    {
        p_profile_pool = is_dropthrd? p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool : p_usw_queue_master[lchip]->p_pfc_profile_pool;
    }
    else
    {
        p_profile_pool = is_dropthrd? p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool : p_usw_queue_master[lchip]->p_fc_profile_pool;
    }

    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

int32
sys_usw_qos_fc_add_static_profile(uint8 lchip)
{
    uint8 profile = 0;
    uint16 index = 0;
    uint32 cmd = 0;
    uint16 chan_id = 0;
    uint32 field_val = 0;
    uint8 tc = 0;
    sys_qos_fc_profile_t fc_data;
    sys_qos_fc_profile_t *p_fc_profile_get;
    DsIrmPortTcFlowControlThrdProfile_m ds_igr_port_tc_fc_profile;
    DsIrmPortFlowControlThrdProfile_m ds_igr_port_fc_profile;

    uint32 tc_thrd[2][3] =
    {
        {10576, 256, 224},
        {10576, 0x3FFFF, 0x3FFFF}
    };

    uint32 port_thrd[2][3] =
    {
        {10576, 256, 224},
        {10576, 0x3FFFF, 0x3FFFF}
    };

    sal_memset(&fc_data, 0, sizeof(fc_data));
    sal_memset(&ds_igr_port_tc_fc_profile, 0, sizeof(DsIrmPortTcFlowControlThrdProfile_m));
    sal_memset(&ds_igr_port_fc_profile, 0, sizeof(DsIrmPortFlowControlThrdProfile_m));

    for (profile = 0; profile < 2; profile++)   /* here has 2 profile */
    {
        index = (profile << 2 | 0);

        SetDsIrmPortTcFlowControlThrdProfile(V, portTcXoffThrd_f  , &ds_igr_port_tc_fc_profile, tc_thrd[profile][1]);
        SetDsIrmPortTcFlowControlThrdProfile(V, portTcXonThrd_f  , &ds_igr_port_tc_fc_profile, tc_thrd[profile][2]);
        cmd = DRV_IOW(DsIrmPortTcFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_tc_fc_profile));

        fc_data.thrd = tc_thrd[profile][0];
        fc_data.xon = tc_thrd[profile][1];
        fc_data.xoff = tc_thrd[profile][2];
        fc_data.profile_id = profile;

        CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_queue_master[lchip]->p_pfc_profile_pool, &fc_data));

        p_fc_profile_get = ctc_spool_lookup(p_usw_queue_master[lchip]->p_pfc_profile_pool, &fc_data);

        if (profile == 0)
        {
            /* configure portTcThrdProfile ID, precascade */
            for (chan_id = 0; chan_id < 64; chan_id++)
            {
                index = chan_id;
                field_val = 0;
                for (tc = 0; tc < 8; tc++)
                {
                    cmd = DRV_IOW(DsIrmPortStallCfg_t, DsIrmPortStallCfg_u_g1_0_portTcFlowControlThrdProfId_f + tc);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
                    p_usw_queue_master[lchip]->p_pfc_profile[chan_id][tc] = p_fc_profile_get;
                }
            }
        }
    }

    for (profile = 0; profile < 2; profile++)
    {
        index = profile << 2 | 0;
        SetDsIrmPortFlowControlThrdProfile(V, portXoffThrd_f  , &ds_igr_port_fc_profile, port_thrd[profile][1]);
        SetDsIrmPortFlowControlThrdProfile(V, portXonThrd_f  , &ds_igr_port_fc_profile, port_thrd[profile][2]);
        cmd = DRV_IOW(DsIrmPortFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_fc_profile));

        fc_data.thrd = port_thrd[profile][0];
        fc_data.xon = port_thrd[profile][1];
        fc_data.xoff = port_thrd[profile][2];
        fc_data.profile_id = profile;

        CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_queue_master[lchip]->p_fc_profile_pool, &fc_data));

        p_fc_profile_get = ctc_spool_lookup(p_usw_queue_master[lchip]->p_fc_profile_pool, &fc_data);

        if (profile == 0)
        {
            /* port threshold profile ID, 40*2*8=640 profId */
            for (chan_id = 0; chan_id < 64; chan_id ++)
            {
                field_val = 0;
                index = chan_id;
                cmd = DRV_IOW(DsIrmPortStallCfg_t,  DsIrmPortStallCfg_portFlowControlThrdProfId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
                p_usw_queue_master[lchip]->p_fc_profile[chan_id] = p_fc_profile_get;
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_fcdl_detect(uint8 lchip, ctc_qos_resrc_fcdl_detect_t *p_fcdl)
{
    uint8 loop                      = 0;
    uint32 mac_id                   = 0;
    uint8  step                     = 0;
    uint32 tbl_id                   = 0;
    uint32 cmd                      = 0;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;

    CTC_VALUE_RANGE_CHECK(p_fcdl->valid_num, 1, CTC_MAX_FCDL_DETECT_NUM);
    SYS_GLOBAL_PORT_CHECK(p_fcdl->gport);
    for(loop = 0; loop < p_fcdl->valid_num; loop++)
    {
        CTC_MAX_VALUE_CHECK(p_fcdl->priority_class[p_fcdl->valid_num-1], 7);
    }

    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, p_fcdl->gport, SYS_DMPS_PORT_PROP_MAC_ID, (void *)&mac_id));
    if (0xff == mac_id)
    {
        return CTC_E_INVALID_CONFIG;
    }

    step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
    tbl_id = Sgmac0RxPauseCfg0_t + (mac_id / 4) + ((mac_id % 4)*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
    if(p_fcdl->enable)
    {
        for(loop = 0; loop < p_fcdl->valid_num; loop++)
        {
            SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectEn0_f+loop, &rx_pause_cfg, p_fcdl->enable);
            SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectCos0_f+loop, &rx_pause_cfg, p_fcdl->priority_class[loop]);
            SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectTimer0_f+loop, &rx_pause_cfg, p_fcdl->detect_mult);
        }
    }
    else
    {
        SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectEn0_f, &rx_pause_cfg, p_fcdl->enable);
        SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectEn1_f, &rx_pause_cfg, p_fcdl->enable);
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
    return CTC_E_NONE;
}

int32
sys_usw_qos_get_fcdl_detect(uint8 lchip, ctc_qos_resrc_fcdl_detect_t *p_fcdl)
{
    uint32 mac_id                   = 0;
    uint8  step                     = 0;
    uint32 tbl_id                   = 0;
    uint32 cmd                      = 0;
    uint8 value                    = 0;
    uint8 value1                   = 0;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;

    SYS_GLOBAL_PORT_CHECK(p_fcdl->gport);
    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, p_fcdl->gport, SYS_DMPS_PORT_PROP_MAC_ID, (void *)&mac_id));
    if (0xff == mac_id)
    {
        return CTC_E_INVALID_CONFIG;
    }

    step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
    tbl_id = Sgmac0RxPauseCfg0_t + (mac_id / 4) + ((mac_id % 4)*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));

    value = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectEn0_f, &rx_pause_cfg);
    value1 = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectEn1_f, &rx_pause_cfg);
    p_fcdl->enable = (value == 0 && value1 == 0)? 0 : 1;
    if(p_fcdl->enable == 0)
    {
        return CTC_E_NONE;
    }
    p_fcdl->valid_num = (value == 1 && value1 == 1) ? 2 : 1;
    p_fcdl->detect_mult = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectTimer0_f, &rx_pause_cfg);
    p_fcdl->priority_class[0] = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectCos0_f, &rx_pause_cfg);
    p_fcdl->priority_class[1] = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseLockDetectCos1_f, &rx_pause_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
    return CTC_E_NONE;
}

int32
sys_usw_qos_fc_add_profile(uint8 lchip, ctc_qos_resrc_fc_t *p_fc)
{
    sys_qos_fc_profile_t fc_data;
    sys_qos_fc_profile_t fc_dropthrd_data;
    uint8 index = 0;
    uint32 profile_id = 0;
    uint32 dropthrd_profile_id = 0;
    uint32 cmd = 0;
    uint8 chan_id = 0;
    uint32 value = 0;
    uint32 old_profile_id = 0;
    uint16 lport = 0;
    uint32 step = 0;
    uint8 loop = 0;
    sys_qos_fc_profile_t *p_sys_fc_profile = NULL;
    sys_qos_fc_profile_t *p_sys_fc_profile_new = NULL;
    sys_qos_fc_profile_t sys_fc_dropthrd_profile;
    sys_qos_fc_profile_t *p_sys_fc_dropthrd_profile = NULL;
    sys_qos_fc_profile_t *p_sys_fc_dropthrd_profile_new = NULL;
    DsIrmPortTcFlowControlThrdProfile_m ds_igr_port_tc_fc_profile;
    DsIrmPortFlowControlThrdProfile_m ds_igr_port_fc_profile;

    sal_memset(&fc_data, 0, sizeof(fc_data));
    sal_memset(&fc_dropthrd_data, 0, sizeof(fc_dropthrd_data));
    sal_memset(&sys_fc_dropthrd_profile, 0, sizeof(sys_fc_dropthrd_profile));

    SYS_GLOBAL_PORT_CHECK(p_fc->gport);
    CTC_MAX_VALUE_CHECK(p_fc->priority_class, 7);
    CTC_MAX_VALUE_CHECK(p_fc->xon_thrd, MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
    CTC_MAX_VALUE_CHECK(p_fc->xoff_thrd, MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
    CTC_MAX_VALUE_CHECK(p_fc->drop_thrd, MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
    chan_id = SYS_GET_CHANNEL_ID(lchip, p_fc->gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }
    lport = CTC_MAP_GPORT_TO_LPORT(p_fc->gport);
    fc_data.xon = p_fc->xon_thrd;
    fc_data.xoff = p_fc->xoff_thrd;
    fc_dropthrd_data.thrd = p_fc->drop_thrd / 8 * 8;
    fc_dropthrd_data.type = 1;
    if (p_fc->is_pfc)
    {
        p_sys_fc_profile = p_usw_queue_master[lchip]->p_pfc_profile[chan_id][p_fc->priority_class];
        cmd = DRV_IOR(DsIrmPortTcCfg_t, DsIrmPortTcCfg_u_g1_portTcLimitedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (lport << 3) + p_fc->priority_class, cmd, &old_profile_id));
        if(p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][p_fc->priority_class])
        {
            p_sys_fc_dropthrd_profile =  p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][p_fc->priority_class];
        }
    }
    else
    {
        p_sys_fc_profile = p_usw_queue_master[lchip]->p_fc_profile[chan_id];
        cmd = DRV_IOR(DsIrmPortCfg_t, DsIrmPortCfg_portLimitedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &old_profile_id));
        if(p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id])
        {
            p_sys_fc_dropthrd_profile = p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id];
        }
    }

    CTC_ERROR_RETURN(_sys_usw_qos_fc_profile_add_spool(lchip, p_fc->is_pfc, 0, p_sys_fc_profile, &fc_data, &p_sys_fc_profile_new));
    CTC_ERROR_RETURN(_sys_usw_qos_fc_profile_add_spool(lchip, p_fc->is_pfc, 1, p_sys_fc_dropthrd_profile, &fc_dropthrd_data, &p_sys_fc_dropthrd_profile_new));

    profile_id = p_sys_fc_profile_new->profile_id;
    dropthrd_profile_id = p_sys_fc_dropthrd_profile_new->profile_id;
    if (p_fc->is_pfc)
    {
        sal_memset(&ds_igr_port_tc_fc_profile, 0, sizeof(DsIrmPortTcFlowControlThrdProfile_m));

        index = (profile_id << 2);
        SetDsIrmPortTcFlowControlThrdProfile(V, portTcXoffThrd_f  , &ds_igr_port_tc_fc_profile, p_fc->xoff_thrd);
        SetDsIrmPortTcFlowControlThrdProfile(V, portTcXonThrd_f  , &ds_igr_port_tc_fc_profile, p_fc->xon_thrd);
        cmd = DRV_IOW(DsIrmPortTcFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_tc_fc_profile));

        index = chan_id;
        cmd = DRV_IOW(DsIrmPortStallCfg_t, DsIrmPortStallCfg_u_g1_0_portTcFlowControlThrdProfId_f + p_fc->priority_class);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &profile_id));

        p_usw_queue_master[lchip]->p_pfc_profile[chan_id][p_fc->priority_class] = p_sys_fc_profile_new;

        for(loop = 0; loop < 4; loop++)
        {
            step = DsIrmPortTcLimitedThrdProfile_g_1_portTcLimitedThrd_f - DsIrmPortTcLimitedThrdProfile_g_0_portTcLimitedThrd_f;
            cmd = DRV_IOW(DsIrmPortTcLimitedThrdProfile_t, DsIrmPortTcLimitedThrdProfile_g_0_portTcLimitedThrd_f+loop*step);
            value = p_fc->drop_thrd / 8;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, dropthrd_profile_id << 2, cmd, &value));
        }
        if(old_profile_id != 0XF)
        {
            cmd = DRV_IOW(DsIrmPortTcCfg_t, DsIrmPortTcCfg_u_g1_portTcLimitedThrdProfId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (lport << 3) + p_fc->priority_class, cmd, &dropthrd_profile_id));
        }
        p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][p_fc->priority_class] = p_sys_fc_dropthrd_profile_new;
    }
    else
    {
        sal_memset(&ds_igr_port_fc_profile, 0, sizeof(DsIrmPortFlowControlThrdProfile_m));
        index = (profile_id << 2);
        SetDsIrmPortFlowControlThrdProfile(V, portXoffThrd_f  , &ds_igr_port_fc_profile, p_fc->xoff_thrd);
        SetDsIrmPortFlowControlThrdProfile(V, portXonThrd_f  , &ds_igr_port_fc_profile, p_fc->xon_thrd);
        cmd = DRV_IOW(DsIrmPortFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_fc_profile));

        index = chan_id;
        cmd = DRV_IOW(DsIrmPortStallCfg_t,  DsIrmPortStallCfg_portFlowControlThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &profile_id));

        p_usw_queue_master[lchip]->p_fc_profile[chan_id] = p_sys_fc_profile_new;

        for(loop = 0; loop < 4; loop++)
        {
            step = DsIrmPortLimitedThrdProfile_g_1_portLimitedThrd_f - DsIrmPortLimitedThrdProfile_g_0_portLimitedThrd_f;
            cmd = DRV_IOW(DsIrmPortLimitedThrdProfile_t, DsIrmPortLimitedThrdProfile_g_0_portLimitedThrd_f+loop*step);
            value = p_fc->drop_thrd / 8;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, dropthrd_profile_id << 2, cmd, &value));
        }
        if(old_profile_id != 0XF)
        {
            cmd = DRV_IOW(DsIrmPortCfg_t, DsIrmPortCfg_portLimitedThrdProfId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dropthrd_profile_id));
        }
        p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id] = p_sys_fc_dropthrd_profile_new;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_fc_get_profile(uint8 lchip, ctc_qos_resrc_fc_t *p_fc)
{
    uint8 chan_id = 0;
    sys_qos_fc_profile_t *p_sys_fc_profile = NULL;
    sys_qos_fc_profile_t *p_dropthrd_fc_profile = NULL;

    SYS_GLOBAL_PORT_CHECK(p_fc->gport);
    CTC_MAX_VALUE_CHECK(p_fc->priority_class, 7);
    chan_id = SYS_GET_CHANNEL_ID(lchip, p_fc->gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_fc->is_pfc)
    {
        p_sys_fc_profile = p_usw_queue_master[lchip]->p_pfc_profile[chan_id][p_fc->priority_class];
        p_dropthrd_fc_profile = p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][p_fc->priority_class];
    }
    else
    {
        p_sys_fc_profile = p_usw_queue_master[lchip]->p_fc_profile[chan_id];
        p_dropthrd_fc_profile = p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id];
    }
    p_fc->xon_thrd = p_sys_fc_profile->xon;
    p_fc->xoff_thrd = p_sys_fc_profile->xoff;
    p_fc->drop_thrd = MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD);
    if(p_dropthrd_fc_profile)
    {
        p_fc->drop_thrd = p_dropthrd_fc_profile->thrd;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_fc_get_profile_from_asic(uint8 lchip, uint8 is_pfc, sys_qos_fc_profile_t *p_sys_fc_profile)
{
    uint8 index = 0;
    uint32 cmd = 0;
    DsIrmPortTcFlowControlThrdProfile_m ds_igr_port_tc_fc_profile;
    DsIrmPortFlowControlThrdProfile_m ds_igr_port_fc_profile;

    if (is_pfc)
    {
        index = (p_sys_fc_profile->profile_id << 2);
        sal_memset(&ds_igr_port_tc_fc_profile, 0, sizeof(DsIrmPortTcFlowControlThrdProfile_m));
        cmd = DRV_IOR(DsIrmPortTcFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_tc_fc_profile));

        p_sys_fc_profile->xoff = GetDsIrmPortTcFlowControlThrdProfile(V, portTcXoffThrd_f, &ds_igr_port_tc_fc_profile);
        p_sys_fc_profile->xon = GetDsIrmPortTcFlowControlThrdProfile(V, portTcXonThrd_f, &ds_igr_port_tc_fc_profile);
    }
    else
    {
        index = (p_sys_fc_profile->profile_id << 2);
        sal_memset(&ds_igr_port_fc_profile, 0, sizeof(DsIrmPortFlowControlThrdProfile_m));
        cmd = DRV_IOR(DsIrmPortFlowControlThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_igr_port_fc_profile));

        p_sys_fc_profile->xoff = GetDsIrmPortFlowControlThrdProfile(V, portXoffThrd_f, &ds_igr_port_fc_profile);
        p_sys_fc_profile->xon = GetDsIrmPortFlowControlThrdProfile(V, portXonThrd_f, &ds_igr_port_fc_profile);
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_fc_get_dropthrd_profile_from_asic(uint8 lchip, uint8 is_pfc, sys_qos_fc_profile_t *p_sys_fc_profile)
{
    uint8 index = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    if (is_pfc)
    {
        index = (p_sys_fc_profile->profile_id << 2);
        cmd = DRV_IOR(DsIrmPortTcLimitedThrdProfile_t, DsIrmPortTcLimitedThrdProfile_g_0_portTcLimitedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        p_sys_fc_profile->thrd = value * 8;
    }
    else
    {
        index = (p_sys_fc_profile->profile_id << 2);
        cmd = DRV_IOR(DsIrmPortLimitedThrdProfile_t, DsIrmPortLimitedThrdProfile_g_0_portLimitedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        p_sys_fc_profile->thrd = value * 8;
    }
    p_sys_fc_profile->type = 1;

    return CTC_E_NONE;
}


STATIC uint32
_sys_usw_queue_drop_wtd_hash_make_profile(sys_queue_drop_wtd_profile_t* p_prof)
{
    uint8* data= (uint8*)(p_prof) + sizeof(uint32);
    uint8   length = sizeof(sys_queue_drop_wtd_profile_t) - sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

/**
 @brief Queue drop hash comparison hook.
*/
STATIC bool
_sys_usw_queue_drop_wtd_hash_cmp_profile(sys_queue_drop_wtd_profile_t* p_prof1,
                                           sys_queue_drop_wtd_profile_t* p_prof2)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    if (!sal_memcmp((uint8*)(p_prof1) + sizeof(uint32), (uint8*)(p_prof2) + sizeof(uint32), sizeof(sys_queue_drop_wtd_profile_t) - sizeof(uint32)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_queue_drop_wtd_alloc_profileId(sys_queue_drop_wtd_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = p_usw_queue_master[*p_lchip]->opf_type_queue_drop_wtd;
    opf.pool_index = 0;
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_wtd_free_profileId(sys_queue_drop_wtd_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_queue_drop_wtd;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_queue_drop_wred_hash_make_profile(sys_queue_drop_wred_profile_t* p_prof)
{
    uint8* data= (uint8*)(p_prof);
    uint8   length = sizeof(sys_queue_drop_wred_profile_t) - sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

/**
 @brief Queue drop hash comparison hook.
*/
STATIC bool
_sys_usw_queue_drop_wred_hash_cmp_profile(sys_queue_drop_wred_profile_t* p_prof1,
                                           sys_queue_drop_wred_profile_t* p_prof2)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_prof1 || !p_prof2)
    {
        return FALSE;
    }

    if (!sal_memcmp((uint8*)(p_prof1), (uint8*)(p_prof2), sizeof(sys_queue_drop_wred_profile_t) - sizeof(uint32)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_queue_drop_wred_alloc_profileId(sys_queue_drop_wred_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_queue_drop_wred;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
    p_node->profile_id = value_32;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_drop_wred_restore_profileId(sys_queue_drop_wred_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_queue_drop_wred;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_wred_free_profileId(sys_queue_drop_wred_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_queue_master[*p_lchip]->opf_type_queue_drop_wred;
    opf.pool_index = 0;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

int32
_sys_usw_queue_drop_wtd_profile_add_spool(uint8 lchip, sys_queue_drop_wtd_profile_t* p_sys_profile_old,
                                                          sys_queue_drop_wtd_profile_t* p_sys_profile_new,
                                                          sys_queue_drop_wtd_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_drop_wtd_hash_cmp_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_drop_wtd_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

int32
_sys_usw_queue_drop_wred_profile_add_spool(uint8 lchip, sys_queue_drop_wred_profile_t* p_sys_profile_old,
                                                          sys_queue_drop_wred_profile_t* p_sys_profile_new,
                                                          sys_queue_drop_wred_profile_t** pp_sys_profile_get)
{
    ctc_spool_t* p_profile_pool    = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile_new);

    /*if use new date to replace old data, profile_id will not change*/
    if (p_sys_profile_old)
    {
        if (TRUE == _sys_usw_queue_drop_wred_hash_cmp_profile(p_sys_profile_old, p_sys_profile_new))
        {
            *pp_sys_profile_get = p_sys_profile_old;

            return CTC_E_NONE;
        }
    }

    p_profile_pool = p_usw_queue_master[lchip]->p_drop_wred_profile_pool;
    CTC_ERROR_RETURN(ctc_spool_add(p_profile_pool,
                           p_sys_profile_new,
                           p_sys_profile_old,
                           pp_sys_profile_get));

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  " profile_id = %d\n",  (*pp_sys_profile_get)->profile_id);

    return CTC_E_NONE;
}

int32
sys_usw_queue_get_sc_tc(uint8 lchip, sys_queue_node_t* p_sys_queue, uint8* sc, uint8* tc)
{
    uint8 is_mcast = 0;
    uint8 value = 0;
    uint32 cmd = 0;
    uint16 key_index = 0;
    tbl_entry_t   tcam_key;
    uint32 dest_map = 0;
    DsQueueMapTcamKey_m   QueueMapTcamKey;
    DsQueueMapTcamKey_m   QueueMapTcamMask;
    CTC_PTR_VALID_CHECK(p_sys_queue);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&QueueMapTcamKey, 0 , sizeof(QueueMapTcamKey));
    sal_memset(&QueueMapTcamMask, 0 , sizeof(QueueMapTcamMask));
    if(p_sys_queue->queue_id >=  MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT))
    {
        key_index = (p_sys_queue->queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT))/MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP);
        tcam_key.data_entry = (uint32*)(&QueueMapTcamKey);
        tcam_key.mask_entry = (uint32*)(&QueueMapTcamMask);
        cmd = DRV_IOR(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &tcam_key));
        dest_map = GetDsQueueMapTcamKey(V, destMap_f, &QueueMapTcamKey);
        is_mcast = (dest_map &0x40000) ? 1 :0;
    }
    if(DRV_IS_TSINGMA(lchip))
    {
        value = ((SYS_IS_NETWORK_CTL_QUEUE(p_sys_queue->queue_id)) && ((p_sys_queue->queue_id % 4) < 2));
    }
    else
    {
        value = ((SYS_IS_NETWORK_CTL_QUEUE(p_sys_queue->queue_id)) && ((p_sys_queue->queue_id % 4) < 1));
    }
    if(((value)||(p_usw_queue_master[lchip]->enq_mode && is_mcast)) &&
        p_usw_queue_master[lchip]->egs_pool[CTC_QOS_EGS_RESRC_SPAN_POOL].egs_congest_level_num)
    {
        *sc = CTC_QOS_EGS_RESRC_SPAN_POOL;
    }
    else if(SYS_IS_CPU_QUEUE(p_sys_queue->queue_id))
    {
        *sc = CTC_QOS_EGS_RESRC_CONTROL_POOL;
    }
    else
    {
        *sc = CTC_QOS_EGS_RESRC_DEFAULT_POOL;
    }
    *tc = p_sys_queue->offset;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_write_profile_to_asic(uint8 lchip, uint8 wred_mode,uint8 is_dynamic,
                                             sys_queue_node_t* p_sys_queue,
                                             sys_queue_drop_profile_t* p_sys_profile)
{
    uint32 cmd = 0;
    uint8  sc = 0;
    uint8  tc = 0;
    uint8  level_num = 0;
    uint8  cng_level = 0;
    uint8  index = 0;
    uint8  temp_value = 0;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;
    DsErmAqmThrdProfile_m erm_aqm_thrd_profile;
    uint8 drop_factor[11] = {15,14,13,12,11,10,9,0,1,2,3};

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile);

    sys_usw_queue_get_sc_tc(lchip, p_sys_queue, &sc, &tc);

    /* config db */
    if (wred_mode)
    {
        sal_memset(&erm_aqm_thrd_profile, 0, sizeof(DsErmAqmThrdProfile_m));
        SetDsErmAqmThrdProfile(V, g_0_wredAvgMaxLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_max_thrd0 / 8);
        SetDsErmAqmThrdProfile(V, g_1_wredAvgMaxLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_max_thrd1 / 8);
        SetDsErmAqmThrdProfile(V, g_2_wredAvgMaxLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_max_thrd2 / 8);
        SetDsErmAqmThrdProfile(V, g_3_wredAvgMaxLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_max_thrd3 / 8);
        SetDsErmAqmThrdProfile(V, g_0_wredAvgMinLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_min_thrd0 / 8);
        SetDsErmAqmThrdProfile(V, g_1_wredAvgMinLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_min_thrd1 / 8);
        SetDsErmAqmThrdProfile(V, g_2_wredAvgMinLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_min_thrd2 / 8);
        SetDsErmAqmThrdProfile(V, g_3_wredAvgMinLen_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->wred_min_thrd3 / 8);
        SetDsErmAqmThrdProfile(V, g_0_wredProbFactor_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->factor0);
        SetDsErmAqmThrdProfile(V, g_1_wredProbFactor_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->factor1);
        SetDsErmAqmThrdProfile(V, g_2_wredProbFactor_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->factor2);
        SetDsErmAqmThrdProfile(V, g_3_wredProbFactor_f, &erm_aqm_thrd_profile, p_sys_profile->p_drop_wred_profile->factor3);

        cmd = DRV_IOW(DsErmAqmThrdProfile_t, DRV_ENTRY_FLAG);
        index = p_sys_profile->p_drop_wred_profile->profile_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_thrd_profile));
    }
    else if(is_dynamic == 0)
    {
        level_num = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;
        if (level_num > 0)
        {
            sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
            for (cng_level = 0; cng_level < level_num; cng_level++)
            {
                temp_value = (p_usw_queue_master[lchip]->resrc_check_mode != 0) ? (cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-level_num)):0;
                SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedThrd_f, &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd0 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedThrd_f, &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd1 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedThrd_f, &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd2 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedThrd_f, &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd3 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd0 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd1 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd2 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd3 / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedAutoEn_f , &ds_que_thrd_profile, 0);
                SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedAutoEn_f , &ds_que_thrd_profile, 0);
                SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedAutoEn_f , &ds_que_thrd_profile, 0);
                SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedAutoEn_f , &ds_que_thrd_profile, 0);

                cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                index = (p_sys_profile->p_drop_wtd_profile->profile_id << 2) + temp_value;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
            }
        }
    }
    else if(is_dynamic == 1)
    {
        sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
        index = (p_sys_profile->p_drop_wtd_profile->profile_id << 2);
        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

        SetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd0 / 8);
        SetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd1 / 8);
        SetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd2 / 8);
        SetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile, p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd3 / 8);
        SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedFactor_f , &ds_que_thrd_profile, drop_factor[p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor0]);
        SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedFactor_f , &ds_que_thrd_profile, drop_factor[p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor1]);
        SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedFactor_f , &ds_que_thrd_profile, drop_factor[p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor2]);
        SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedFactor_f , &ds_que_thrd_profile, drop_factor[p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor3]);
        SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
        SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
        SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
        SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);

        cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_drop_read_profile_from_asic(uint8 lchip, uint8 wred_mode,
                                             sys_queue_node_t* p_sys_queue,
                                             sys_queue_drop_profile_t* p_sys_profile)
{
    uint32 cmd = 0;
    uint32 sc = 0;
    uint8  level_num = 0;
    uint8  cng_level = 0;
    uint8  index = 0;
    uint8 drop_factor[15] = {7,8,9,10,0,0,0,0,6,5,4,3,2,1,0};
    uint32 value = 0;
    uint8 temp_value = 0;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;
    DsErmAqmThrdProfile_m erm_aqm_thrd_profile;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_profile);

    /* get queue sc */
    /*cmd = DRV_IOR(DsEgrResrcCtl_t, DsEgrResrcCtl_mappedSc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_queue->queue_id, cmd, &sc));
    */
    if(!wred_mode)
    {
        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DsErmQueueLimitedThrdProfile_g_0_queueLimitedAutoEn_f);
        index = (p_sys_profile->p_drop_wtd_profile->profile_id << 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    }
    /* config db */
    if (wred_mode)
    {
        sal_memset(&erm_aqm_thrd_profile, 0, sizeof(DsErmAqmThrdProfile_m));
        cmd = DRV_IOR(DsErmAqmThrdProfile_t, DRV_ENTRY_FLAG);
        index = p_sys_profile->p_drop_wred_profile->profile_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_thrd_profile));

        p_sys_profile->p_drop_wred_profile->wred_max_thrd0 = 8 * GetDsErmAqmThrdProfile(V, g_0_wredAvgMaxLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_max_thrd1 = 8 * GetDsErmAqmThrdProfile(V, g_1_wredAvgMaxLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_max_thrd2 = 8 * GetDsErmAqmThrdProfile(V, g_2_wredAvgMaxLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_max_thrd3 = 8 * GetDsErmAqmThrdProfile(V, g_3_wredAvgMaxLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_min_thrd0 = 8 * GetDsErmAqmThrdProfile(V, g_0_wredAvgMinLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_min_thrd1 = 8 * GetDsErmAqmThrdProfile(V, g_1_wredAvgMinLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_min_thrd2 = 8 * GetDsErmAqmThrdProfile(V, g_2_wredAvgMinLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->wred_min_thrd3 = 8 * GetDsErmAqmThrdProfile(V, g_3_wredAvgMinLen_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->factor0 = GetDsErmAqmThrdProfile(V, g_0_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->factor1 = GetDsErmAqmThrdProfile(V, g_1_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->factor2 = GetDsErmAqmThrdProfile(V, g_2_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_sys_profile->p_drop_wred_profile->factor3 = GetDsErmAqmThrdProfile(V, g_3_wredProbFactor_f, &erm_aqm_thrd_profile);
    }
    else if(value == 0)
    {
        level_num = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;
        if (level_num > 0)
        {
            for (cng_level = 0; cng_level < level_num; cng_level++)
            {
                temp_value = (p_usw_queue_master[lchip]->resrc_check_mode != 0) ? (cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-level_num)):0;
                sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
                cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                index = (p_sys_profile->p_drop_wtd_profile->profile_id << 2) + temp_value;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

                p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd0 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedThrd_f, &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd1 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedThrd_f, &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd2 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedThrd_f, &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].wred_max_thrd3 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedThrd_f, &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd0 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd1 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd2 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile);
                p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd3 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile);
            }
        }
    }
    else if(value == 1)
    {
        sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        index = (p_sys_profile->p_drop_wtd_profile->profile_id << 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

        p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor0 = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor1 = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor2 = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].drop_thrd_factor3 = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd0 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile);
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd1 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile);
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd2 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile);
        p_sys_profile->p_drop_wtd_profile->profile[cng_level].ecn_mark_thrd3 = 8 * GetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_write_asic(uint8 lchip, uint8 wred_mode, uint16 queue_id,
                                                 sys_queue_drop_profile_t* p_sys_profile)
{
    uint32 cmd = 0;
    uint32 index = 0;
    DsErmQueueCfg_m erm_queue_cfg;
    DsErmAqmQueueCfg_m erm_aqm_cfg;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&erm_queue_cfg, 0, sizeof(DsErmQueueCfg_m));
    sal_memset(&erm_aqm_cfg, 0, sizeof(DsErmAqmQueueCfg_m));

    if (wred_mode && (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id)))
    {
        index = (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
                     ?  queue_id : (queue_id - (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT)- MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)));

        cmd = DRV_IOR(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
        SetDsErmAqmQueueCfg(V, queueThrdProfId_f, &erm_aqm_cfg, p_sys_profile->p_drop_wred_profile->profile_id);
        SetDsErmAqmQueueCfg(V, avgQueueLenResetEn_f, &erm_aqm_cfg, 1);
        SetDsErmAqmQueueCfg(V, ewmaWeightFactor_f, &erm_aqm_cfg, 1);
        cmd = DRV_IOW(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
    }
    else if(wred_mode == 0)
    {
        index = queue_id;
        cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
        SetDsErmQueueCfg(V, queueLimitedThrdProfId_f, &erm_queue_cfg, p_sys_profile->p_drop_wtd_profile->profile_id);
        cmd = DRV_IOW(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_clear_asic(uint8 lchip, uint8 wred_mode, uint16 queue_id)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 field_val = 0;
    DsErmQueueCfg_m erm_queue_cfg;
    DsErmAqmQueueCfg_m erm_aqm_cfg;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&erm_queue_cfg, 0, sizeof(DsErmQueueCfg_m));
    sal_memset(&erm_aqm_cfg, 0, sizeof(DsErmAqmQueueCfg_m));

    if (wred_mode)
    {
        index = (queue_id < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
                 ?  queue_id : (queue_id - (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT)- MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC)));
        cmd = DRV_IOR(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
        field_val = MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM);
        SetDsErmAqmQueueCfg(V, queueThrdProfId_f, &erm_aqm_cfg, field_val);
        cmd = DRV_IOW(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
    }
    else
    {
        index = queue_id;
        cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
        field_val = MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WTD_PROFILE_NUM);
        SetDsErmQueueCfg(V, queueLimitedThrdProfId_f, &erm_queue_cfg, field_val);
        cmd = DRV_IOW(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_add_profile(uint8 lchip, uint8 wred_mode, uint8 is_dynamic,ctc_qos_drop_array drop_arraw,
                                      sys_queue_node_t* p_sys_queue,
                                      sys_queue_drop_profile_t** pp_sys_profile_get)
{
    uint8  sc = 0;
    uint8  tc = 0;
    uint8  level_num = 0;
    uint8  cng_level = 0;

    sys_queue_drop_wtd_profile_t* p_sys_wtd_profile_old = NULL;
    sys_queue_drop_wtd_profile_t  sys_wtd_profile_new;
    sys_queue_drop_wred_profile_t* p_sys_wred_profile_old = NULL;
    sys_queue_drop_wred_profile_t  sys_wred_profile_new;
    sys_queue_drop_wtd_profile_t* p_sys_wtd_profile_get = NULL;
    sys_queue_drop_wred_profile_t* p_sys_wred_profile_get = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_queue);
    CTC_PTR_VALID_CHECK(pp_sys_profile_get);

    sal_memset(&sys_wtd_profile_new, 0, sizeof(sys_queue_drop_wtd_profile_t));
    sal_memset(&sys_wred_profile_new, 0, sizeof(sys_queue_drop_wred_profile_t));

    sys_usw_queue_get_sc_tc(lchip, p_sys_queue, &sc, &tc);

    /* config db */
    if (wred_mode)
    {
        sys_wred_profile_new.wred_min_thrd0 = drop_arraw[cng_level].drop.min_th[0] / 8 * 8;
        sys_wred_profile_new.wred_min_thrd1 = drop_arraw[cng_level].drop.min_th[1] / 8 * 8;
        sys_wred_profile_new.wred_min_thrd2 = drop_arraw[cng_level].drop.min_th[2] / 8 * 8;
        sys_wred_profile_new.wred_min_thrd3 = drop_arraw[cng_level].drop.min_th[3] / 8 * 8;
        sys_wred_profile_new.wred_max_thrd0 = drop_arraw[cng_level].drop.max_th[0] / 8 * 8;
        sys_wred_profile_new.wred_max_thrd1 = drop_arraw[cng_level].drop.max_th[1] / 8 * 8;
        sys_wred_profile_new.wred_max_thrd2 = drop_arraw[cng_level].drop.max_th[2] / 8 * 8;
        sys_wred_profile_new.wred_max_thrd3 = drop_arraw[cng_level].drop.max_th[3] / 8 * 8;
        sys_wred_profile_new.factor0 = drop_arraw[cng_level].drop.drop_prob[0];
        sys_wred_profile_new.factor1 = drop_arraw[cng_level].drop.drop_prob[1];
        sys_wred_profile_new.factor2 = drop_arraw[cng_level].drop.drop_prob[2];
        sys_wred_profile_new.factor3 = drop_arraw[cng_level].drop.drop_prob[3];

        p_sys_wred_profile_old = p_sys_queue->drop_profile.p_drop_wred_profile;
        CTC_ERROR_RETURN(_sys_usw_queue_drop_wred_profile_add_spool(lchip, p_sys_wred_profile_old, &sys_wred_profile_new, &p_sys_wred_profile_get));
        p_sys_queue->drop_profile.p_drop_wred_profile = p_sys_wred_profile_get;
    }
    else if(is_dynamic == 0)
    {
        level_num = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;
        if (level_num > 0)
        {
            for (cng_level = 0; cng_level < level_num; cng_level++)
            {
                sys_wtd_profile_new.profile[cng_level].wred_max_thrd0 = drop_arraw[cng_level].drop.max_th[0] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].wred_max_thrd1 = drop_arraw[cng_level].drop.max_th[1] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].wred_max_thrd2 = drop_arraw[cng_level].drop.max_th[2] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].wred_max_thrd3 = drop_arraw[cng_level].drop.max_th[3] / 8 * 8;

                sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd0 = drop_arraw[cng_level].drop.ecn_th[0] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd1 = drop_arraw[cng_level].drop.ecn_th[1] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd2 = drop_arraw[cng_level].drop.ecn_th[2] / 8 * 8;
                sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd3 = drop_arraw[cng_level].drop.ecn_th[3] / 8 * 8;
            }
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        p_sys_wtd_profile_old = p_sys_queue->drop_profile.p_drop_wtd_profile;
        CTC_ERROR_RETURN(_sys_usw_queue_drop_wtd_profile_add_spool(lchip, p_sys_wtd_profile_old, &sys_wtd_profile_new, &p_sys_wtd_profile_get));
        p_sys_queue->drop_profile.p_drop_wtd_profile = p_sys_wtd_profile_get;
    }

    else if(is_dynamic == 1)
    {
        sys_wtd_profile_new.profile[cng_level].drop_thrd_factor0 = drop_arraw[cng_level].drop.drop_factor[0] ;
        sys_wtd_profile_new.profile[cng_level].drop_thrd_factor1 = drop_arraw[cng_level].drop.drop_factor[1] ;
        sys_wtd_profile_new.profile[cng_level].drop_thrd_factor2 = drop_arraw[cng_level].drop.drop_factor[2] ;
        sys_wtd_profile_new.profile[cng_level].drop_thrd_factor3 = drop_arraw[cng_level].drop.drop_factor[3] ;

        sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd0 = drop_arraw[cng_level].drop.ecn_th[0] / 8 * 8;
        sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd1 = drop_arraw[cng_level].drop.ecn_th[1] / 8 * 8;
        sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd2 = drop_arraw[cng_level].drop.ecn_th[2] / 8 * 8;
        sys_wtd_profile_new.profile[cng_level].ecn_mark_thrd3 = drop_arraw[cng_level].drop.ecn_th[3] / 8 * 8;

        p_sys_wtd_profile_old = p_sys_queue->drop_profile.p_drop_wtd_profile;
        CTC_ERROR_RETURN(_sys_usw_queue_drop_wtd_profile_add_spool(lchip, p_sys_wtd_profile_old, &sys_wtd_profile_new, &p_sys_wtd_profile_get));
        p_sys_queue->drop_profile.p_drop_wtd_profile = p_sys_wtd_profile_get;
    }
    *pp_sys_profile_get = &(p_sys_queue->drop_profile);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_drop_remove_profile(uint8 lchip, uint8 wred_mode, sys_queue_node_t* p_sys_queue)
{
    ctc_spool_t* p_profile_pool = NULL;
    sys_queue_drop_wred_profile_t* p_sys_wred_profile_old = NULL;
    sys_queue_drop_wtd_profile_t* p_sys_wtd_profile_old = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_queue);

    p_sys_wred_profile_old = p_sys_queue->drop_profile.p_drop_wred_profile;
    p_sys_wtd_profile_old = p_sys_queue->drop_profile.p_drop_wtd_profile;

    if (wred_mode)
    {
        if (p_sys_wred_profile_old)
        {
            p_profile_pool = p_usw_queue_master[lchip]->p_drop_wred_profile_pool;
            CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_wred_profile_old, NULL));
            p_sys_queue->drop_profile.p_drop_wred_profile = NULL;
        }
    }
    else
    {
        if (p_sys_wtd_profile_old)
        {
            p_profile_pool = p_usw_queue_master[lchip]->p_drop_wtd_profile_pool;
            CTC_ERROR_RETURN(ctc_spool_remove(p_profile_pool, p_sys_wtd_profile_old, NULL));
            p_sys_queue->drop_profile.p_drop_wtd_profile = NULL;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_queue_get_queue_drop(uint8 lchip, sys_queue_node_t* p_sys_queue_node, ctc_qos_drop_array p_drop, uint16 queue_id, uint8 wred_mode)
{
    uint8    profile_id = 0;
    uint32 cmd = 0;
    uint8 level_num = 0;
    uint8 sc = 0;
    uint8 tc = 0;
    uint8 cng_level = 0;
    uint8 is_coexist = 0;
    uint32 index = 0;
    uint8 wred_profile = MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM);
    uint8 drop_factor[16] = {7,8,9,10,0,0,0,0,0,6,5,4,3,2,1,0};
    uint32 is_dynamic = 0;
    uint8 temp_value = 0;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;
    DsErmAqmThrdProfile_m erm_aqm_thrd_profile;
    DsErmAqmQueueCfg_m erm_aqm_queue_cfg;
    DsErmQueueCfg_m erm_queue_cfg;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_drop);

    sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));
    sal_memset(&erm_aqm_thrd_profile, 0, sizeof(DsErmAqmThrdProfile_m));
    sal_memset(&erm_aqm_queue_cfg, 0, sizeof(DsErmAqmQueueCfg_m));
    sal_memset(&erm_queue_cfg, 0, sizeof(DsErmQueueCfg_m));

    sys_usw_queue_get_sc_tc(lchip, p_sys_queue_node, &sc, &tc);
    if (!SYS_IS_NETWORK_BASE_QUEUE(queue_id) && !SYS_IS_EXT_QUEUE(queue_id)&&wred_mode)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id))
    {
        index = (queue_id >= MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT))
                ? ((queue_id - MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXT)) + MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC))
                : queue_id;
        cmd = DRV_IOR(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_queue_cfg));
        wred_profile = GetDsErmAqmQueueCfg(V, queueThrdProfId_f , &erm_aqm_queue_cfg);
    }

    cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id , cmd, &erm_queue_cfg));
    profile_id = GetDsErmQueueCfg(V, queueLimitedThrdProfId_f , &erm_queue_cfg);
    if(wred_profile != MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM)
       && profile_id != MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WTD_PROFILE_NUM))
    {
        is_coexist = 1;
    }
    cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DsErmQueueLimitedThrdProfile_g_0_queueLimitedAutoEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (profile_id << 2), cmd, &is_dynamic));
    p_drop[0].drop.is_dynamic = is_dynamic;
    if(wred_mode)
    {
        cmd = DRV_IOR(DsErmAqmThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, wred_profile, cmd, &erm_aqm_thrd_profile));
        p_drop[cng_level].drop.max_th[0] = GetDsErmAqmThrdProfile(V, g_0_wredAvgMaxLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.max_th[1] = GetDsErmAqmThrdProfile(V, g_1_wredAvgMaxLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.max_th[2] = GetDsErmAqmThrdProfile(V, g_2_wredAvgMaxLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.max_th[3] = GetDsErmAqmThrdProfile(V, g_3_wredAvgMaxLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.min_th[0] = GetDsErmAqmThrdProfile(V, g_0_wredAvgMinLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.min_th[1] = GetDsErmAqmThrdProfile(V, g_1_wredAvgMinLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.min_th[2] = GetDsErmAqmThrdProfile(V, g_2_wredAvgMinLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.min_th[3] = GetDsErmAqmThrdProfile(V, g_3_wredAvgMinLen_f, &erm_aqm_thrd_profile) * 8;
        p_drop[cng_level].drop.drop_prob[0] = GetDsErmAqmThrdProfile(V, g_0_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_drop[cng_level].drop.drop_prob[1] = GetDsErmAqmThrdProfile(V, g_1_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_drop[cng_level].drop.drop_prob[2] = GetDsErmAqmThrdProfile(V, g_2_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_drop[cng_level].drop.drop_prob[3] = GetDsErmAqmThrdProfile(V, g_3_wredProbFactor_f, &erm_aqm_thrd_profile);
        p_drop[cng_level].drop.is_coexist = is_coexist;
    }
    else if(is_dynamic == 0)
    {
        level_num = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;
        if (level_num > 0)
        {
            for (cng_level = 0; cng_level < level_num; cng_level++)
            {
                temp_value = (p_usw_queue_master[lchip]->resrc_check_mode != 0) ? (cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-level_num)):0;
                cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (profile_id << 2) + temp_value, cmd, &ds_que_thrd_profile));
                p_drop[cng_level].drop.max_th[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedThrd_f, &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.max_th[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedThrd_f, &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.max_th[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedThrd_f, &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.max_th[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedThrd_f, &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.ecn_th[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.ecn_th[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.ecn_th[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.ecn_th[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
                p_drop[cng_level].drop.is_coexist = is_coexist;
            }
        }
    }
    else if(is_dynamic == 1)
    {
        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (profile_id << 2), cmd, &ds_que_thrd_profile));
        p_drop[cng_level].drop.drop_factor[0] = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_drop[cng_level].drop.drop_factor[1] = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_drop[cng_level].drop.drop_factor[2] = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_drop[cng_level].drop.drop_factor[3] = drop_factor[GetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedFactor_f, &ds_que_thrd_profile)];
        p_drop[cng_level].drop.ecn_th[0] = GetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
        p_drop[cng_level].drop.ecn_th[1] = GetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
        p_drop[cng_level].drop.ecn_th[2] = GetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
        p_drop[cng_level].drop.ecn_th[3] = GetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile) * 8;
        p_drop[cng_level].drop.is_coexist = is_coexist;
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_queue_drop_add_static_profile(uint8 lchip, ctc_qos_global_cfg_t* p_cfg)
{
    sys_queue_drop_wtd_profile_t sys_profile_new;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;
    uint8  sc = 0;
    uint8  level_num = 0;
    uint8  cng_level = 0;
    uint16 index = 0;
    uint32 cmd = 0;
    uint16 profile_index = 0;
    uint32 field_val = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_profile_new, 0, sizeof(sys_queue_drop_wtd_profile_t));
    field_val = (DRV_IS_DUET2(lchip)) ? 0x7FF : 0xFFF;
    /* sc0 sc1 sc2 sc3 use the same factor*/
    if(p_usw_queue_master[lchip]->resrc_check_mode == 0)
    {
        sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));

        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        index = (profile_index << 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
        SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedFactor_f, &ds_que_thrd_profile, 0);
        SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedFactor_f, &ds_que_thrd_profile, 0);
        SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedFactor_f, &ds_que_thrd_profile, 0);
        SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedFactor_f, &ds_que_thrd_profile, 0);
        SetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        index = (profile_index << 2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

        sys_profile_new.profile_id = profile_index;
        sys_profile_new.profile[cng_level].drop_thrd_factor0 = 7;
        sys_profile_new.profile[cng_level].drop_thrd_factor1 = 7;
        sys_profile_new.profile[cng_level].drop_thrd_factor2 = 7;
        sys_profile_new.profile[cng_level].drop_thrd_factor3 = 7;
        sys_profile_new.profile[cng_level].ecn_mark_thrd0 = field_val * 8;
        sys_profile_new.profile[cng_level].ecn_mark_thrd1 = field_val * 8;
        sys_profile_new.profile[cng_level].ecn_mark_thrd2 = field_val * 8;
        sys_profile_new.profile[cng_level].ecn_mark_thrd3 = field_val * 8;

        CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_queue_master[lchip]->p_drop_wtd_profile_pool, &sys_profile_new));

        return CTC_E_NONE;
    }

    for (sc = 0; sc < 4; sc ++)
    {
        level_num = p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num;
        profile_index = p_usw_queue_master[lchip]->egs_pool[sc].default_profile_id;

        if (level_num > 0)
        {
            for (cng_level = 0; cng_level < level_num; cng_level++)
            {
                sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));

                cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                index = (profile_index << 2) + cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-level_num);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
                SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedThrd_f, &ds_que_thrd_profile, p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[0] / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedThrd_f, &ds_que_thrd_profile, p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[1] / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedThrd_f, &ds_que_thrd_profile, p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[2] / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedThrd_f, &ds_que_thrd_profile, p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[3] / 8);
                SetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
                SetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
                SetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
                SetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
                cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
                index = (profile_index << 2) + cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM)-level_num);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));

                sys_profile_new.profile_id = profile_index;
                sys_profile_new.profile[cng_level].wred_max_thrd0 = p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[0];
                sys_profile_new.profile[cng_level].wred_max_thrd1 = p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[1];
                sys_profile_new.profile[cng_level].wred_max_thrd2 = p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[2];
                sys_profile_new.profile[cng_level].wred_max_thrd3 = p_cfg->resrc_pool.drop_profile[sc].queue_drop[cng_level].max_th[3];
                sys_profile_new.profile[cng_level].ecn_mark_thrd0 = field_val * 8;
                sys_profile_new.profile[cng_level].ecn_mark_thrd1 = field_val * 8;
                sys_profile_new.profile[cng_level].ecn_mark_thrd2 = field_val * 8;
                sys_profile_new.profile[cng_level].ecn_mark_thrd3 = field_val * 8;
            }

            CTC_ERROR_RETURN(ctc_spool_static_add(p_usw_queue_master[lchip]->p_drop_wtd_profile_pool, &sys_profile_new));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_set_drop_enable(uint8 lchip, uint8 wred_mode, ctc_qos_drop_array drop_array,
                                      sys_queue_node_t* p_sys_queue_node)
{
    int32 ret = 0;
    int8 is_dynamic = 0;
    sys_queue_drop_profile_t* p_sys_profile_new = NULL;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_queue_node);
    is_dynamic = drop_array[0].drop.is_dynamic;
    /*add drop prof*/
    CTC_ERROR_GOTO(_sys_usw_queue_drop_add_profile(lchip, wred_mode, is_dynamic,drop_array,
                                                            p_sys_queue_node,
                                                            &p_sys_profile_new), ret, error0);

    /*write drop prof*/
    CTC_ERROR_GOTO(_sys_usw_queue_drop_write_profile_to_asic(lchip, wred_mode,is_dynamic,
                                                                     p_sys_queue_node,
                                                                     p_sys_profile_new), ret, error1);

    /*write drop ctl and count*/
    CTC_ERROR_GOTO(_sys_usw_queue_drop_write_asic(lchip, wred_mode,
                                                           p_sys_queue_node->queue_id,
                                                           p_sys_profile_new), ret, error1);


    return ret;

error1:
    _sys_usw_queue_drop_remove_profile(lchip, wred_mode, p_sys_queue_node);
error0:
    return ret;

}

int32
sys_usw_queue_set_drop_disable(uint8 lchip, uint8 wred_mode, sys_queue_node_t* p_sys_queue_node)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_queue_node);

    CTC_ERROR_RETURN(_sys_usw_queue_drop_clear_asic(lchip, wred_mode, p_sys_queue_node->queue_id));

    CTC_ERROR_RETURN(_sys_usw_queue_drop_remove_profile(lchip, wred_mode, p_sys_queue_node));


    return CTC_E_NONE;
}


int32
sys_usw_queue_set_drop(uint8 lchip, ctc_qos_drop_t* p_drop)
{
    uint16 queue_id = 0;
    uint8  wred_mode = 0;
    ctc_qos_queue_drop_t *p_queue_drop = &p_drop->drop;
    sys_queue_node_t* p_sys_queue_node = NULL;
    ctc_qos_drop_array drop_array;
    uint8 i = 0;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_drop);

    sal_memset(&drop_array, 0, sizeof(drop_array));

    /*get queue_id*/
    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,
                                                       &p_drop->queue,
                                                       &queue_id));
    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    if(p_queue_drop->ecn_mark_th != 0)
    {
        for (i = 0; i < CTC_DROP_PREC_NUM; i++)
        {
            p_queue_drop->ecn_th[i] = p_queue_drop->ecn_mark_th;
        }
    }
    for (i = 0; i < CTC_DROP_PREC_NUM; i++)
    {
        CTC_MAX_VALUE_CHECK(p_queue_drop->min_th[i],    MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
        CTC_MAX_VALUE_CHECK(p_queue_drop->max_th[i],    MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
        CTC_MAX_VALUE_CHECK(p_queue_drop->min_th[i],    p_queue_drop->max_th[i] );
        CTC_MAX_VALUE_CHECK(p_queue_drop->drop_prob[i], SYS_MAX_DROP_PROB );
        CTC_MAX_VALUE_CHECK(p_queue_drop->ecn_th[i],    MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
        CTC_MAX_VALUE_CHECK(p_queue_drop->drop_factor[i],SYS_MAX_DROP_FACTOR);
        if(0 == p_queue_drop->ecn_th[i])
        {
            p_queue_drop->ecn_th[i] = MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) ;
        }
    }

    for (i = 0; i < CTC_RESRC_MAX_CONGEST_LEVEL_NUM; i ++)
    {
        sal_memcpy(&drop_array[i].drop, p_queue_drop, sizeof(ctc_qos_queue_drop_t));
    }

    wred_mode = (p_queue_drop->mode == CTC_QUEUE_DROP_WRED);
    if(wred_mode && (!SYS_IS_NETWORK_BASE_QUEUE(queue_id) && !SYS_IS_EXT_QUEUE(queue_id)))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_queue_set_drop_enable(lchip, wred_mode, drop_array, p_sys_queue_node));
    if (!p_queue_drop->is_coexist && (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id)))
    {
        CTC_ERROR_RETURN(sys_usw_queue_set_drop_disable(lchip, !wred_mode, p_sys_queue_node));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_get_drop(uint8 lchip, ctc_qos_drop_t* p_drop)
{
    uint16 queue_id = 0;
    uint8  wred_mode = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;
    ctc_qos_drop_array drop_array;

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_drop);

    sal_memset(&drop_array, 0, sizeof(drop_array));

    /*get queue_id*/
    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,
                                                       &p_drop->queue,
                                                       &queue_id));
    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    wred_mode = (p_drop->drop.mode == CTC_QUEUE_DROP_WRED);
    drop_array[0].drop.mode = p_drop->drop.mode;
    CTC_ERROR_RETURN(_sys_usw_queue_get_queue_drop(lchip, p_sys_queue_node, drop_array, queue_id, wred_mode));
    sal_memcpy(&p_drop->drop, &drop_array[0].drop, sizeof(ctc_qos_queue_drop_t));

    return CTC_E_NONE;
}

int32
sys_usw_queue_set_cpu_queue_egs_pool_classify(uint8 lchip, uint16 reason_id, uint8 pool)
{
    return CTC_E_NONE;
}

int32
sys_usw_queue_set_port_min(uint8 lchip, uint8 dir, uint32 gport, uint16 thrd)
{
    uint8  profile_id              = 0;
    uint16 lport                   = 0;
    uint16 index                   = 0;
    uint32 cmd                     = 0;
    uint32 field_value             = 0;
    uint16 chan_id                 = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }

    /* write profile to asic */
    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        field_value = thrd;
        cmd = DRV_IOW(DsIrmPortGuaranteedThrdProfile_t, DsIrmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &field_value));

        index = lport;
        field_value = profile_id;
        cmd = DRV_IOW(DsIrmPortCfg_t, DsIrmPortCfg_portGuaranteedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
    }

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        field_value = thrd;
        cmd = DRV_IOW(DsErmPortGuaranteedThrdProfile_t, DsErmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &field_value));

        index = lport;
        field_value = profile_id;
        cmd = DRV_IOW(DsErmPortCfg_t, DsErmPortCfg_portGuaranteedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
    }

    return CTC_E_NONE;
}
int32
sys_usw_queue_get_port_min(uint8 lchip, uint8 dir, uint32 gport, uint16* thrd)
{
    uint8  profile_id              = 0;
    uint16 lport                   = 0;
    uint32 cmd                     = 0;
    uint32 field_value             = 0;
    uint16 chan_id                 = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }

    /* write profile to asic */
    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        cmd = DRV_IOR(DsIrmPortGuaranteedThrdProfile_t, DsIrmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &field_value));
        *thrd = field_value;
    }

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        cmd = DRV_IOR(DsErmPortGuaranteedThrdProfile_t, DsErmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &field_value));
        *thrd = field_value;
    }

    return CTC_E_NONE;
}


int32
sys_usw_qos_set_resrc_classify_pool(uint8 lchip, ctc_qos_resrc_classify_pool_t *p_pool)
{
    uint16 index                    = 0;
    uint32 cmd                      = 0;
    uint32 field_value              = 0;
    uint16 chan_id                  = 0;

    DsIrmPrioScTcMap_m irm_pri_to_sc_tc_map;
    DsErmPrioScTcMap_m erm_prio_sc_tc_map;

    SYS_GLOBAL_PORT_CHECK(p_pool->gport);
    CTC_MAX_VALUE_CHECK(p_pool->pool, 3);
    CTC_MAX_VALUE_CHECK(p_pool->priority, 15);

    if (p_pool->dir == CTC_INGRESS)
    {
        chan_id = SYS_GET_CHANNEL_ID(lchip, p_pool->gport);
        CTC_MAX_VALUE_CHECK(chan_id, 83);

        sal_memset(&irm_pri_to_sc_tc_map, 0, sizeof(DsIrmPrioScTcMap_m));
        if (p_pool->pool == CTC_QOS_IGS_RESRC_NON_DROP_POOL)
        {

            index = p_pool->priority;
            cmd = DRV_IOR(DsIrmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_pri_to_sc_tc_map));
            SetDsIrmPrioScTcMap(V, g1_1_mappedSc_f , &irm_pri_to_sc_tc_map, p_pool->pool);
            cmd = DRV_IOW(DsIrmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_pri_to_sc_tc_map));

            field_value = 1;
        }
        else
        {
            field_value = 0;
        }

        /* all port are mapping profile 0 */
        index = chan_id;
        cmd = DRV_IOW(DsIrmChannel_t, DsIrmChannel_irmProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
    }
    else if (p_pool->dir == CTC_EGRESS)
    {
        if (p_usw_queue_master[lchip]->egs_pool[p_pool->pool].egs_congest_level_num == 0)   /* the sc is not exist */
        {
            return CTC_E_INVALID_PARAM;
        }

        chan_id = SYS_GET_CHANNEL_ID(lchip, p_pool->gport);
        CTC_MAX_VALUE_CHECK(chan_id, 83);

        sal_memset(&erm_prio_sc_tc_map, 0, sizeof(DsErmPrioScTcMap_m));

        if (p_pool->pool == CTC_QOS_EGS_RESRC_SPAN_POOL)
        {
            index = p_pool->priority;
            cmd = DRV_IOR(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
            SetDsErmPrioScTcMap(V, g1_1_mappedSc_f , &erm_prio_sc_tc_map, p_pool->pool);
            cmd = DRV_IOW(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));

            field_value = 1;
        }
        else
        {
            field_value = 0;
        }

        /* default use profile 0, sc use profile 1 */
        index = chan_id;
        cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_ermProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_get_resrc_classify_pool(uint8 lchip, ctc_qos_resrc_classify_pool_t *p_pool)
{
    uint16 index                    = 0;
    uint32 cmd                      = 0;
    uint32 field_value              = 0;
    uint16 chan_id                  = 0;

    DsIrmPrioScTcMap_m irm_pri_to_sc_tc_map;
    DsErmPrioScTcMap_m erm_prio_sc_tc_map;

    SYS_GLOBAL_PORT_CHECK(p_pool->gport);
    CTC_MAX_VALUE_CHECK(p_pool->pool, 3);
    CTC_MAX_VALUE_CHECK(p_pool->priority, 15);

    if (p_pool->dir == CTC_INGRESS)
    {
        chan_id = SYS_GET_CHANNEL_ID(lchip, p_pool->gport);
        CTC_MAX_VALUE_CHECK(chan_id, 83);

        sal_memset(&irm_pri_to_sc_tc_map, 0, sizeof(DsIrmPrioScTcMap_m));

        /* all port are mapping profile 0 */
        index = chan_id;
        cmd = DRV_IOR(DsIrmChannel_t, DsIrmChannel_irmProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
        if(field_value)
        {
            p_pool->pool = CTC_QOS_IGS_RESRC_NON_DROP_POOL;
        }
        else
        {
            p_pool->pool = CTC_QOS_IGS_RESRC_DEFAULT_POOL;
        }
    }
    else if (p_pool->dir == CTC_EGRESS)
    {
        chan_id = SYS_GET_CHANNEL_ID(lchip, p_pool->gport);
        CTC_MAX_VALUE_CHECK(chan_id, 83);

        sal_memset(&erm_prio_sc_tc_map, 0, sizeof(DsErmPrioScTcMap_m));

        /* default use profile 0, sc use profile 1 */
        index = chan_id;
        cmd = DRV_IOR(DsErmChannel_t, DsErmChannel_ermProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_value));
        if(field_value)
        {
            p_pool->pool = CTC_QOS_EGS_RESRC_SPAN_POOL;
        }
        else
        {
            p_pool->pool = CTC_QOS_IGS_RESRC_DEFAULT_POOL;
        }
        if (p_usw_queue_master[lchip]->egs_pool[p_pool->pool].egs_congest_level_num == 0)   /* the sc is not exist */
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_resrc_queue_drop(uint8 lchip, ctc_qos_drop_array p_drop)
{
    uint8  tc                       = 0;
    uint16 index                    = 0;
    uint16 queue_id                 = 0;
    uint8 wred_mode = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;

    if(p_usw_queue_master[lchip]->resrc_check_mode == 0)
    {
        return CTC_E_NOT_SUPPORT;
    }

    /* check para */
    for (index = 0; index < CTC_RESRC_MAX_CONGEST_LEVEL_NUM; index++)
    {
        if(p_drop[index].drop.ecn_mark_th != 0)
        {
            for (tc = 0; tc < 4; tc++)
            {
                p_drop[index].drop.ecn_th[tc] = p_drop[index].drop.ecn_mark_th;
            }
        }
        for (tc = 0; tc < 4; tc++)
        {
            CTC_MAX_VALUE_CHECK(p_drop[index].drop.min_th[tc], MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
            CTC_MAX_VALUE_CHECK(p_drop[index].drop.max_th[tc], MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
            CTC_MAX_VALUE_CHECK(p_drop[index].drop.drop_prob[tc], SYS_MAX_DROP_PROB );
            CTC_MAX_VALUE_CHECK(p_drop[index].drop.ecn_th[tc],MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) );
            CTC_MAX_VALUE_CHECK(p_drop[index].drop.drop_factor[tc],SYS_MAX_DROP_FACTOR);
            if(0 == p_drop[index].drop.ecn_th[tc])
            {
                p_drop[index].drop.ecn_th[tc] = MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD) ;
            }
        }

    }

    /* get queue_id */
    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &(p_drop[0].queue), &queue_id));
    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    wred_mode = (p_drop[0].drop.mode == CTC_QUEUE_DROP_WRED);
    CTC_ERROR_RETURN(sys_usw_queue_set_drop_enable(lchip, wred_mode, p_drop, p_sys_queue_node));
    if (!p_drop[0].drop.is_coexist && (SYS_IS_NETWORK_BASE_QUEUE(queue_id) || SYS_IS_EXT_QUEUE(queue_id)))
    {
        CTC_ERROR_RETURN(sys_usw_queue_set_drop_disable(lchip, !wred_mode, p_sys_queue_node));
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_get_resrc_queue_drop(uint8 lchip, ctc_qos_drop_array p_drop)
{
    uint16 queue_id = 0;
    uint8  wred_mode = 0;
    sys_queue_node_t* p_sys_queue_node = NULL;

    if(p_usw_queue_master[lchip]->resrc_check_mode == 0)
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_drop);

    /*get queue_id*/
    CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip,
                                                       &p_drop[0].queue,
                                                       &queue_id));
    p_sys_queue_node = ctc_vector_get(p_usw_queue_master[lchip]->queue_vec, queue_id);
    if (NULL == p_sys_queue_node)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }
    wred_mode = (p_drop[0].drop.mode == CTC_QUEUE_DROP_WRED);
    CTC_ERROR_RETURN(_sys_usw_queue_get_queue_drop(lchip, p_sys_queue_node, p_drop, queue_id, wred_mode));

    return CTC_E_NONE;
}

int32
sys_usw_queue_set_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_resrc);

    switch (p_resrc->cfg_type)
    {
    case CTC_QOS_RESRC_CFG_POOL_CLASSIFY:
        CTC_ERROR_RETURN(sys_usw_qos_set_resrc_classify_pool(lchip, &p_resrc->u.pool));
        break;

    case CTC_QOS_RESRC_CFG_PORT_MIN:
        CTC_MAX_VALUE_CHECK(p_resrc->u.port_min.threshold, 255);

        CTC_ERROR_RETURN(sys_usw_queue_set_port_min(lchip, p_resrc->u.port_min.dir, p_resrc->u.port_min.gport,
                                              p_resrc->u.port_min.threshold));
        break;

    case CTC_QOS_RESRC_CFG_QUEUE_DROP:
        CTC_ERROR_RETURN(sys_usw_qos_set_resrc_queue_drop(lchip, p_resrc->u.queue_drop));
        break;

    case CTC_QOS_RESRC_CFG_FLOW_CTL:
        CTC_ERROR_RETURN(sys_usw_qos_fc_add_profile(lchip, &p_resrc->u.flow_ctl));
        break;

    case CTC_QOS_RESRC_CFG_FCDL_DETECT:
        CTC_ERROR_RETURN(sys_usw_qos_set_fcdl_detect(lchip, &p_resrc->u.fcdl_detect));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_get_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_resrc);

    switch (p_resrc->cfg_type)
    {
    case CTC_QOS_RESRC_CFG_POOL_CLASSIFY:
        CTC_ERROR_RETURN(sys_usw_qos_get_resrc_classify_pool(lchip, &p_resrc->u.pool));
        break;

    case CTC_QOS_RESRC_CFG_PORT_MIN:
        CTC_ERROR_RETURN(sys_usw_queue_get_port_min(lchip, p_resrc->u.port_min.dir, p_resrc->u.port_min.gport,
                                              &p_resrc->u.port_min.threshold));
        break;

    case CTC_QOS_RESRC_CFG_QUEUE_DROP:
        CTC_ERROR_RETURN(sys_usw_qos_get_resrc_queue_drop(lchip, p_resrc->u.queue_drop));
        break;

    case CTC_QOS_RESRC_CFG_FLOW_CTL:
        CTC_ERROR_RETURN(sys_usw_qos_fc_get_profile(lchip, &p_resrc->u.flow_ctl));
        break;

    case CTC_QOS_RESRC_CFG_FCDL_DETECT:
        CTC_ERROR_RETURN(sys_usw_qos_get_fcdl_detect(lchip, &p_resrc->u.fcdl_detect));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_query_pool_stats(uint8 lchip, ctc_qos_resrc_pool_stats_t* p_stats)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint16 queue_id = 0;
    ctc_qos_queue_id_t queue;
    ctc_qos_queue_id_t queue_temp;

    CTC_PTR_VALID_CHECK(p_stats);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&queue, 0, sizeof(queue));

    switch (p_stats->type)
    {
    case CTC_QOS_RESRC_STATS_IGS_POOL_COUNT:
        CTC_MAX_VALUE_CHECK(p_stats->pool, CTC_QOS_IGS_RESRC_NON_DROP_POOL);
        cmd = DRV_IOR(DsIrmScCnt_t, DsIrmScCnt_g_0_scCnt_f + p_stats->pool);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        p_stats->count = field_value;

        break;

    case CTC_QOS_RESRC_STATS_IGS_TOTAL_COUNT:
        cmd = DRV_IOR(DsIrmMiscCnt_t, DsIrmMiscCnt_totalCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        p_stats->count = field_value;

        break;

    case CTC_QOS_RESRC_STATS_EGS_POOL_COUNT:
        CTC_MAX_VALUE_CHECK(p_stats->pool, CTC_QOS_EGS_RESRC_SPAN_POOL);
        cmd = DRV_IOR(DsErmScCnt_t, DsErmScCnt_g_0_scCnt_f + p_stats->pool);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        p_stats->count = field_value;
        break;

    case CTC_QOS_RESRC_STATS_EGS_TOTAL_COUNT:
        cmd = DRV_IOR(DsErmMiscCnt_t, DsErmMiscCnt_totalCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
        p_stats->count = field_value;
        break;

    case CTC_QOS_RESRC_STATS_QUEUE_COUNT:
        CTC_MAX_VALUE_CHECK(p_stats->priority, 15);

        sal_memset(&queue_temp, 0, sizeof(ctc_qos_queue_id_t));
        if (sal_memcmp(&(p_stats->queue), &queue_temp, sizeof(ctc_qos_queue_id_t)))
        {
            sal_memcpy(&queue, &(p_stats->queue), sizeof(ctc_qos_queue_id_t));
        }
        else
        {
            queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            queue.gport = p_stats->gport;
            queue.queue_id = p_stats->priority / 2;
        }

        CTC_ERROR_RETURN(sys_usw_queue_get_queue_id(lchip, &queue, &queue_id));

        cmd = DRV_IOR(DsErmQueueCnt_t, DsErmQueueCnt_queueCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, queue_id, cmd, &field_value));
        p_stats->count = field_value;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_resrc_mgr_en(uint8 lchip, uint8 enable)
{
    uint32 cmd        = 0;
    uint32 field_val  = 0;
    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(IrmMiscCtl_t, IrmMiscCtl_resourceCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_resourceCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_qos_set_resrc_check_mode(uint8 lchip, uint8 mode)
{
    uint32 cmd = 0;
    uint32 entry_num = 0;
    uint32 index = 0;
    ErmMiscCtl_m erm_misc_ctl;
    IrmMiscCtl_m irm_misc_ctl;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;

    LCHIP_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(mode, 1);

    sal_memset(&erm_misc_ctl, 0, sizeof(ErmMiscCtl_m));
    sal_memset(&irm_misc_ctl, 0, sizeof(IrmMiscCtl_m));
    sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));

    if (mode)
    {
        cmd = DRV_IOR(ErmMiscCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_ctl));
        SetErmMiscCtl(V, resourceCheckMode_f, &erm_misc_ctl, 1);
        cmd = DRV_IOW(ErmMiscCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_ctl));
    }
    else
    {
        cmd = DRV_IOR(ErmMiscCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_ctl));
        SetErmMiscCtl(V, resourceCheckMode_f, &erm_misc_ctl, 0);
        cmd = DRV_IOW(ErmMiscCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_ctl));

        sys_usw_ftm_query_table_entry_num(lchip, DsErmQueueLimitedThrdProfile_t, &entry_num);
        for (index = 0; index < entry_num; index++)
        {
            cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
            SetDsErmQueueLimitedThrdProfile(V, g_0_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
            SetDsErmQueueLimitedThrdProfile(V, g_1_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
            SetDsErmQueueLimitedThrdProfile(V, g_2_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
            SetDsErmQueueLimitedThrdProfile(V, g_3_queueLimitedAutoEn_f , &ds_que_thrd_profile, 1);
            cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
        }
    }

    return CTC_E_NONE;
}

/*
* mode 0 : continuous mode
* mode 1 : discrete mode(congest level)
*/
int32
sys_usw_qos_set_resrc_check_mode(uint8 lchip, uint8 mode)
{
    CTC_ERROR_RETURN(_sys_usw_qos_set_resrc_check_mode(lchip, mode));
    p_usw_queue_master[lchip]->resrc_check_mode = mode;
    return CTC_E_NONE;
}

int32
sys_usw_queue_set_egs_resrc_guarantee(uint8 lchip, uint8 enable)
{
    uint32 cmd;
    uint32 min;

    LCHIP_CHECK(lchip);

    min = enable ? 20 : 0;

    /*default config port min, not config queue min*/
    /*DsErmQueueGuaranteedThrdProfile_m que_min_profile;
    sal_memset(&que_min_profile, 0, sizeof(DsErmQueueGuaranteedThrdProfile_m));
    cmd = DRV_IOR(DsErmQueueGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &que_min_profile));
    SetDsErmQueueGuaranteedThrdProfile(V, queueGuaranteedThrd_f , &que_min_profile, min);
    cmd = DRV_IOW(DsErmQueueGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &que_min_profile));*/

    cmd = DRV_IOW(DsErmPortGuaranteedThrdProfile_t, DsErmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &min));

    return CTC_E_NONE;
}

int32
sys_usw_qos_flow_ctl_profile(uint8 lchip, uint32 gport,
                                    uint8 priority_class,
                                    uint8 is_pfc,
                                    uint8 enable)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 field_id = 0;
    uint32 field_val = 0;
    uint8 chan_id  = 0;
    uint32 port_type = 0;
    uint16 lport = 0;

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));

    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    /* 2. modify XON/XOFF threashold */
    if (is_pfc)
    {

        field_val = enable ? p_usw_queue_master[lchip]->p_pfc_profile[chan_id][priority_class]->profile_id: 1;
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  " profile id = %d\n", field_val);
        field_id = DsIrmPortStallCfg_u_g1_0_portTcFlowControlThrdProfId_f + priority_class;
        index = chan_id;
        cmd = DRV_IOW(DsIrmPortStallCfg_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        if(p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][priority_class] != NULL)
        {
            field_val = enable ? p_usw_queue_master[lchip]->p_dropthrd_pfc_profile[chan_id][priority_class]->profile_id: 0xf;
            cmd = DRV_IOW(DsIrmPortTcCfg_t, DsIrmPortTcCfg_u_g1_portTcLimitedThrdProfId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (lport << 3) + priority_class, cmd, &field_val));
        }
    }
    else
    {
        /*port threshold profile ID*/
        field_val = enable ? p_usw_queue_master[lchip]->p_fc_profile[chan_id]->profile_id : 1;
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  " profile id = %d\n", field_val);
        field_id = DsIrmPortStallCfg_portFlowControlThrdProfId_f;
        index = chan_id;
        cmd = DRV_IOW(DsIrmPortStallCfg_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        if(p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id] != NULL)
        {
            field_val = enable ? p_usw_queue_master[lchip]->p_dropthrd_fc_profile[chan_id]->profile_id: 0xf;
            cmd = DRV_IOW(DsIrmPortCfg_t, DsIrmPortCfg_portLimitedThrdProfId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
        }
    }

    return CTC_E_NONE;
}

/**
 @brief Initialize default queue drop.
*/
STATIC int32
_sys_usw_queue_egs_resrc_mgr_init(uint8 lchip, ctc_qos_global_cfg_t* p_cfg)
{
    uint8 cng_level                = 0;
    uint8 sc                       = 0;
    uint8 level_num                = 0;
    uint8 profile_index            = 0;
    uint16 lport                   = 0;
    uint16 index                   = 0;
    uint32 field_val               = 0;
    uint32 field_id                = 0;
    uint32 cmd                     = 0;
    uint8 priority                 = 0;
    uint8 mcast                    = 0;
    uint8 color                    = 0;
    uint32 entry_num               = 0;
    uint32 pool_size[CTC_QOS_EGS_RESRC_POOL_MAX] = {0};
    uint8 cpu_channel = 0;
    uint8 enable = 0;
    uint8 cutthrough_enable = 0;
    uint8 control_pool_enable = 0;
    uint32 total_pool_size = 0;

    ErmMiscCtl_m erm_misc_ctl;
    DsErmScThrd_m erm_sc_thrd;
    DsErmPrioScTcMap_m erm_prio_sc_tc_map;
    DsErmColorDpMap_m erm_color_dp_map;
    DsErmMiscThrd_m erm_misc_thrd;
    DsErmPortCfg_m erm_port_cfg;
    DsErmAqmQueueCfg_m erm_aqm_cfg;
    ErmAqmQueueScanCtl_m erm_aqm_queue_scan_ctl;
    QWriteCtl_m  q_write_ctl;
    DsErmAqmPortThrd_m aqm_port_thrd;
    DsErmQueueLimitedThrdProfile_m ds_que_thrd_profile;

    sal_memset(&erm_misc_ctl, 0, sizeof(ErmMiscCtl_m));
    sal_memset(&erm_sc_thrd, 0, sizeof(DsErmScThrd_m));
    sal_memset(&erm_prio_sc_tc_map, 0, sizeof(DsErmPrioScTcMap_m));
    sal_memset(&erm_color_dp_map, 0, sizeof(DsErmColorDpMap_m));
    sal_memset(&erm_misc_thrd, 0, sizeof(DsErmMiscThrd_m));
    sal_memset(&erm_port_cfg, 0, sizeof(DsErmPortCfg_m));
    sal_memset(&erm_aqm_cfg, 0, sizeof(DsErmAqmQueueCfg_m));
    sal_memset(&erm_aqm_queue_scan_ctl, 0, sizeof(ErmAqmQueueScanCtl_m));
    sal_memset(&q_write_ctl, 0, sizeof(QWriteCtl_m));
    sal_memset(&aqm_port_thrd, 0, sizeof(DsErmAqmPortThrd_m));
    sal_memset(&ds_que_thrd_profile, 0, sizeof(DsErmQueueLimitedThrdProfile_m));

    /* 1. Buffer assign, total buffer cell is 15K */
    if (p_cfg->resrc_pool.egs_pool_mode == CTC_QOS_RESRC_POOL_DISABLE)
    {
        pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] = (DRV_IS_DUET2(lchip))?13824:14848;
        /* only check sc|total sc */
        pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL]  = 0;
        pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL]      = 0;
        pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL]   = 1536;     /* 1.5K */
        pool_size[CTC_QOS_EGS_RESRC_MIN_POOL]       = 0;
        pool_size[CTC_QOS_EGS_RESRC_C2C_POOL]       = 0;
    }
    else if (p_cfg->resrc_pool.egs_pool_mode == CTC_QOS_RESRC_POOL_MODE1)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.egs_pool_size, sizeof(pool_size));
        if (pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] || pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL])
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else if (p_cfg->resrc_pool.egs_pool_mode == CTC_QOS_RESRC_POOL_MODE2)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.egs_pool_size, sizeof(pool_size));
        if ((pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] == 0)
            || (pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] == 0)
            || (pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL] == 0)
            || (pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL] == 0))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else if (p_cfg->resrc_pool.egs_pool_mode == CTC_QOS_RESRC_POOL_USER_DEFINE)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.egs_pool_size, sizeof(pool_size));
    }
    /* if use 1.6K buffer cells mode ,total pool size should not big than 1.6K*/
    cmd = DRV_IOR(ShareBufferCtl_t, ShareBufferCtl_cfgShareTableEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    total_pool_size =  pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] + pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] +pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL]+
                       pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL] + pool_size[CTC_QOS_EGS_RESRC_MIN_POOL] +pool_size[CTC_QOS_EGS_RESRC_C2C_POOL];
    if (pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] == 0 || (field_val && total_pool_size > 16384)) /* must have default pool */
    {
        return CTC_E_INVALID_PARAM;
    }
    cutthrough_enable = sys_usw_chip_get_cut_through_en(lchip);
    /* 2. configure register fields for egress resource management*/
    SetErmMiscCtl(V, aqmPortUseAvgEn_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, basedOnCellCnt_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, c2cPacketDpEn_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, c2cPacketDp_f, &erm_misc_ctl, 3);
    SetErmMiscCtl(V, continuousPortTcUseScRemain_f, &erm_misc_ctl, ((DRV_IS_DUET2(lchip)&&cutthrough_enable)?0:1));
    SetErmMiscCtl(V, criticalPacketDpEn_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, criticalPacketDp_f, &erm_misc_ctl, 3);
    SetErmMiscCtl(V, ignoreAvqMaxThrdCheck_f, &erm_misc_ctl, 0);
    SetErmMiscCtl(V, microBurstInformEn_f, &erm_misc_ctl, 0);
    SetErmMiscCtl(V, noCareC2cPacket_f, &erm_misc_ctl, 1);/* c2c packet use sc pool*/
    SetErmMiscCtl(V, noCareCriticalPacket_f, &erm_misc_ctl, 0);
    SetErmMiscCtl(V, notEctPacketDpEn_f, &erm_misc_ctl, 0);
    SetErmMiscCtl(V, notEctPacketDp_f, &erm_misc_ctl, 3);
    SetErmMiscCtl(V, resourceCheckEn_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, resourceCheckMode_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, resrcUseCntMin_f, &erm_misc_ctl, 2);
    SetErmMiscCtl(V, glbQueueGuaranteedDis_f, &erm_misc_ctl, 1); /*default use port min, not use queue min*/
    SetErmMiscCtl(V, scExcludeGuarantee_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, aqmExcludeGuarantee_f, &erm_misc_ctl, 1);
    SetErmMiscCtl(V, glbAqmQueueDis_f, &erm_misc_ctl, ((DRV_IS_DUET2(lchip)&&cutthrough_enable)?1:0));

    cmd = DRV_IOW(ErmMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_ctl));

    /* 3. threshold config, total :15k cells (15*1024) */
    cmd = DRV_IOR(DsErmMiscThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_thrd));
    SetDsErmMiscThrd(V, c2cThrd_f, &erm_misc_thrd, 1024);
    SetDsErmMiscThrd(V, criticalThrd_f, &erm_misc_thrd, 1024);
    SetDsErmMiscThrd(V, spanThrd_f, &erm_misc_thrd, 0x400);
    SetDsErmMiscThrd(V, totalThrd_f, &erm_misc_thrd, (pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] +
                                                      pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] +
                                                      pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL] +
                                                      pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL] +
                                                      pool_size[CTC_QOS_EGS_RESRC_MIN_POOL] +
                                                      pool_size[CTC_QOS_EGS_RESRC_C2C_POOL]));

    cmd = DRV_IOW(DsErmMiscThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_misc_thrd));

    /* 4. sc threshold config, use 2 sc, sc0 : default pool; sc1: span pool*/
    cmd = DRV_IOR(DsErmScThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_sc_thrd));
    SetDsErmScThrd(V, g_0_scThrd_f, &erm_sc_thrd, pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL] / 8);
    SetDsErmScThrd(V, g_1_scThrd_f, &erm_sc_thrd, pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] / 8);
    SetDsErmScThrd(V, g_2_scThrd_f, &erm_sc_thrd, pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL] / 8);
    SetDsErmScThrd(V, g_3_scThrd_f, &erm_sc_thrd, (pool_size[CTC_QOS_EGS_RESRC_C2C_POOL]+pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL]) / 8);
    SetDsErmScThrd(V, g_0_scEn_f, &erm_sc_thrd, 1);
    SetDsErmScThrd(V, g_1_scEn_f, &erm_sc_thrd, pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] ? 1 : 0);
    SetDsErmScThrd(V, g_2_scEn_f, &erm_sc_thrd, pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL] ? 1 : 0);
    if(DRV_IS_DUET2(lchip) && cutthrough_enable)
    {
        SetDsErmScThrd(V, g_0_scEn_f, &erm_sc_thrd, 0);
        SetDsErmScThrd(V, g_1_scEn_f, &erm_sc_thrd, 0);
        SetDsErmScThrd(V, g_2_scEn_f, &erm_sc_thrd, 0);
    }
    SetDsErmScThrd(V, g_3_scEn_f, &erm_sc_thrd, (pool_size[CTC_QOS_EGS_RESRC_C2C_POOL]+pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL]) ? 1 : 0);
    SetDsErmScThrd(V, g_0_scCngThrd_f, &erm_sc_thrd, 0x7ff);
    SetDsErmScThrd(V, g_1_scCngThrd_f, &erm_sc_thrd, 0x7ff);
    SetDsErmScThrd(V, g_2_scCngThrd_f, &erm_sc_thrd, 0x7ff);
    SetDsErmScThrd(V, g_3_scCngThrd_f, &erm_sc_thrd, 0x7ff);
    SetDsErmScThrd(V, g_0_scCngEn_f, &erm_sc_thrd, 0);
    SetDsErmScThrd(V, g_1_scCngEn_f, &erm_sc_thrd, 0);
    SetDsErmScThrd(V, g_2_scCngEn_f, &erm_sc_thrd, 0);
    SetDsErmScThrd(V, g_3_scCngEn_f, &erm_sc_thrd, 0);
    cmd = DRV_IOW(DsErmScThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_sc_thrd));

    /* 5. mapping profiles, use profile 0/1: all priority mapped to tc priority/2, default use sc 0*/
    profile_index = 0;
    for (sc = 0; sc < 4; sc ++)
    {
        if (p_cfg->resrc_pool.drop_profile[sc].congest_level_num == 0)
        {
            p_cfg->resrc_pool.drop_profile[sc].congest_level_num = 1;
        }
        level_num = p_cfg->resrc_pool.drop_profile[sc].congest_level_num;

        if (pool_size[sc])
        {
            p_usw_queue_master[lchip]->egs_pool[sc].egs_congest_level_num = p_cfg->resrc_pool.drop_profile[sc].congest_level_num;
            p_usw_queue_master[lchip]->egs_pool[sc].default_profile_id = (p_usw_queue_master[lchip]->resrc_check_mode != 0)?profile_index:0;
            profile_index ++;
        }
    }

    for (mcast = 0; mcast < 2; mcast++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            index = mcast << 4 | priority;
            if (mcast && p_usw_queue_master[lchip]->egs_pool[CTC_QOS_EGS_RESRC_SPAN_POOL].egs_congest_level_num)
            {
                sc = CTC_QOS_EGS_RESRC_SPAN_POOL;
            }
            else
            {
                sc = CTC_QOS_EGS_RESRC_DEFAULT_POOL;
            }
            cmd = DRV_IOR(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
            SetDsErmPrioScTcMap(V, g2_0_mappedTc_f, &erm_prio_sc_tc_map, priority / 2);
            SetDsErmPrioScTcMap(V, g1_0_mappedSc_f , &erm_prio_sc_tc_map, sc);
            cmd = DRV_IOW(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
        }
    }

    for (mcast = 0; mcast < 2; mcast++)
    {
        for (color = 0; color < 4; color++)
        {
            index = mcast << 2 | color;
            field_val = mcast ? 0 : ((color + 3) % 4);
            cmd = DRV_IOR(DsErmColorDpMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_color_dp_map));
            SetDsErmColorDpMap(V, g_0_dropPrecedence_f, &erm_color_dp_map, field_val);
            SetDsErmColorDpMap(V, g_1_dropPrecedence_f, &erm_color_dp_map, field_val);
            SetDsErmColorDpMap(V, g_2_dropPrecedence_f, &erm_color_dp_map, field_val);
            cmd = DRV_IOW(DsErmColorDpMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_color_dp_map));
        }
    }

    for (index = 0; index < SYS_MAX_CHANNEL_NUM; index++)
    {
        field_val = 0;
        cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_ermProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }
    /* cirtical packet use profile 2*/
    control_pool_enable = (p_usw_queue_master[lchip]->egs_pool[CTC_QOS_EGS_RESRC_CONTROL_POOL].egs_congest_level_num)? 2 : 0;
    for (mcast = 0; mcast < control_pool_enable; mcast++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            index = mcast << 4 | priority;
            sc = CTC_QOS_EGS_RESRC_CONTROL_POOL;
            cmd = DRV_IOR(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
            SetDsErmPrioScTcMap(V, g2_2_mappedTc_f, &erm_prio_sc_tc_map, (priority > 7)?(priority-8) : priority);
            SetDsErmPrioScTcMap(V, g1_2_mappedSc_f , &erm_prio_sc_tc_map, sc);
            cmd = DRV_IOW(DsErmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_prio_sc_tc_map));
            field_val = 2;
            cmd = DRV_IOW(DsErmChannel_t, DsErmChannel_ermProfId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0), cmd, &field_val));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1), cmd, &field_val));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2), cmd, &field_val));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3), cmd, &field_val));
            CTC_ERROR_RETURN(sys_usw_get_chip_cpu_eth_en(lchip, &enable, &cpu_channel));
            if(enable)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, cpu_channel, cmd, &field_val));
            }
        }
    }

    /* 6. config congestion level to enable self-tuning threshold */
    for (sc = 0; sc < 4; sc ++)
    {
        if (p_cfg->resrc_pool.drop_profile[sc].congest_level_num == 0)
        {
            p_cfg->resrc_pool.drop_profile[sc].congest_level_num = 1;
        }
        level_num = p_cfg->resrc_pool.drop_profile[sc].congest_level_num - 1;

        for (cng_level = 0; cng_level < level_num; cng_level ++)
        {
            field_val = p_cfg->resrc_pool.drop_profile[sc].congest_threshold[cng_level] / 32;
            field_id = DsErmScCngThrdProfile_g_0_scCngThrd_f + cng_level + (MCHIP_CAP(SYS_CAP_QOS_CONGEST_LEVEL_NUM) - 1 -level_num);
            cmd = DRV_IOW(DsErmScCngThrdProfile_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, sc, cmd, &field_val));
        }
    }
    /* 7. coutthrough mode use port thrd */
    if(DRV_IS_DUET2(lchip) && cutthrough_enable)
    {
        for(index = 0;index < 4;index ++)
        {
            field_val = 128;
            field_id = DsErmPortLimitedThrdProfile_g_0_portLimitedThrd_f + 3*index;
            cmd = DRV_IOW(DsErmPortLimitedThrdProfile_t, field_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 60, cmd, &field_val));
        }
    }
#if 0 /*default use port min, not use queue min*/
    /* 7. config queue minimum guarantee profile 0: 20 cells*/
    DsErmQueueGuaranteedThrdProfile_m que_min_profile;
    sal_memset(&que_min_profile, 0, sizeof(DsErmQueueGuaranteedThrdProfile_m));

    cmd = DRV_IOR(DsErmQueueGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &que_min_profile));
    SetDsErmQueueGuaranteedThrdProfile(V, queueGuaranteedThrd_f , &que_min_profile, 20);
    cmd = DRV_IOW(DsErmQueueGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &que_min_profile));

    for (index = 0; index < 1280; index ++)
    {
        field_val = 0;
        field_id = DsErmQueueCfg_queueGuaranteedThrdProfId_f;
        cmd = DRV_IOW(DsErmQueueCfg_t, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }
#endif

    /* 8. configure port minimum guarantee profile, default all use profile id 0: port Min = 20*/
    field_val = 20;
    cmd = DRV_IOW(DsErmPortGuaranteedThrdProfile_t, DsErmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    for (lport = 0; lport < 64; lport++)
    {
        field_val = 0;
        field_id = DsErmPortCfg_portGuaranteedThrdProfId_f;
        cmd = DRV_IOW(DsErmPortCfg_t, field_id);
        index = lport;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }

    /* 9. disable port limited thrd check, port threshold profile alawy be 0*/
    sys_usw_ftm_query_table_entry_num(lchip, DsErmPortCfg_t, &entry_num);
    for (index = 0; index < entry_num; index++)
    {
        cmd = DRV_IOR(DsErmPortCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_port_cfg));
        SetDsErmPortCfg(V, portLimitedThrdProfId_f, &erm_port_cfg, 0xF);
        cmd = DRV_IOW(DsErmPortCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_port_cfg));
    }

    /* 10, disable wred mode, default wtd mode, wred mode only support base queue(512) and extend queue(256)*/
    sys_usw_ftm_query_table_entry_num(lchip, DsErmAqmQueueCfg_t, &entry_num);
    for (index = 0; index < entry_num; index++)
    {
        cmd = DRV_IOR(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
        SetDsErmAqmQueueCfg(V, queueThrdProfId_f, &erm_aqm_cfg, MCHIP_CAP(SYS_CAP_QOS_QUEUE_DROP_WRED_PROFILE_NUM));
        cmd = DRV_IOW(DsErmAqmQueueCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_aqm_cfg));
    }

    /* 11, config port congestion thrd*/
    cmd = DRV_IOR(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));
    SetQWriteCtl(V, portCongestionThrd_f, &q_write_ctl, 20);
    cmd = DRV_IOW(QWriteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));

    /* 12, diable  port aqm*/
    field_val = (DRV_IS_DUET2(lchip)) ? 0x3FFF : 0xFFFFFF;
    for (index = 0; index < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); index++)
    {
        cmd = DRV_IOR(DsErmAqmPortThrd_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &aqm_port_thrd));
        SetDsErmAqmPortThrd(V, aqmPortThrdHigh_f, &aqm_port_thrd, field_val);
        SetDsErmAqmPortThrd(V, aqmPortThrdLow_f, &aqm_port_thrd, field_val);
        cmd = DRV_IOW(DsErmAqmPortThrd_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &aqm_port_thrd));
    }

    /* 13, enable aqm queue scan*/
    field_val = (DRV_IS_DUET2(lchip)) ? 767 : 3583;
    cmd = DRV_IOR(ErmAqmQueueScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_aqm_queue_scan_ctl));
    SetErmAqmQueueScanCtl(V, aqmQueueScanEn_f , &erm_aqm_queue_scan_ctl, 1);
    SetErmAqmQueueScanCtl(V, aqmQueueScanInterval_f, &erm_aqm_queue_scan_ctl, 10);
    SetErmAqmQueueScanCtl(V, aqmQueueScanPtrMax_f, &erm_aqm_queue_scan_ctl, field_val);
    cmd = DRV_IOW(ErmAqmQueueScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_aqm_queue_scan_ctl));

    /* 14, default disable ecn mark*/
    field_val = (DRV_IS_DUET2(lchip)) ? 0x7FF : 0xFFF;
    for (cng_level = 0; cng_level < 4; cng_level++)
    {
        cmd = DRV_IOR(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        index = (0xF << 2) + cng_level;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
        SetDsErmQueueLimitedThrdProfile(V, g_0_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_1_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_2_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        SetDsErmQueueLimitedThrdProfile(V, g_3_ecnMarkThrd_f , &ds_que_thrd_profile, field_val);
        cmd = DRV_IOW(DsErmQueueLimitedThrdProfile_t, DRV_ENTRY_FLAG);
        index = (0xF << 2) + cng_level;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_que_thrd_profile));
    }
    /* 15, set resrc check mode*/
    CTC_ERROR_RETURN(_sys_usw_qos_set_resrc_check_mode(lchip, p_usw_queue_master[lchip]->resrc_check_mode));

    return CTC_E_NONE;
}

extern int32
_sys_usw_queue_igs_resrc_mgr_init(uint8 lchip, ctc_qos_global_cfg_t* p_cfg)
{
    uint32 cmd                     = 0;
    uint16 lport                   = 0;
    uint8 priority                 = 0;
    uint8 color                    = 0;
    uint16 index                    = 0;
    uint8 cng_level                = 0;
    uint32 field_id                = 0;
    uint32 field_val               = 0;
    uint8 mcast                    = 0;
    uint32 entry_num               = 0;
    uint8 cutthrough_enable        = 0;
    uint32 pool_size[CTC_QOS_IGS_RESRC_POOL_MAX] = {0};

    IrmMiscCtl_m irm_misc_ctl;
    DsIrmMiscThrd_m irm_misc_thrd;
    DsIrmScThrd_m irm_sc_thrd;
    DsIrmPrioScTcMap_m irm_prio_sc_tc_map;
    DsIrmColorDpMap_m irm_color_dp_map;
    DsIrmPortCfg_m irm_port_cfg;
    DsIrmPortTcCfg_m irm_port_tc_cfg;

    sal_memset(&irm_misc_ctl, 0, sizeof(IrmMiscCtl_m));
    sal_memset(&irm_misc_thrd, 0, sizeof(DsIrmMiscThrd_m));
    sal_memset(&irm_sc_thrd, 0, sizeof(DsIrmScThrd_m));
    sal_memset(&irm_prio_sc_tc_map, 0, sizeof(DsIrmPrioScTcMap_m));
    sal_memset(&irm_port_cfg, 0, sizeof(DsIrmPortCfg_m));
    sal_memset(&irm_port_tc_cfg, 0, sizeof(DsIrmPortTcCfg_m));

    /* 1. Buffer assign, total buffer cell is 15K */
    if (p_cfg->resrc_pool.igs_pool_mode == CTC_QOS_RESRC_POOL_DISABLE)
    {
        pool_size[CTC_QOS_IGS_RESRC_DEFAULT_POOL] = (DRV_IS_DUET2(lchip))? 11520:28927;
        pool_size[CTC_QOS_IGS_RESRC_NON_DROP_POOL]  = 0;
        pool_size[CTC_QOS_IGS_RESRC_MIN_POOL]       = 1280;     /*64*20*/
        pool_size[CTC_QOS_IGS_RESRC_C2C_POOL]       = 1280;
        pool_size[CTC_QOS_IGS_RESRC_ROUND_TRIP_POOL]= 0;
        pool_size[CTC_QOS_IGS_RESRC_CONTROL_POOL]   = 1280;
    }
    else if (p_cfg->resrc_pool.igs_pool_mode == CTC_QOS_RESRC_POOL_MODE1)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.igs_pool_size, sizeof(pool_size));
        if (pool_size[CTC_QOS_IGS_RESRC_NON_DROP_POOL])
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else if (p_cfg->resrc_pool.igs_pool_mode == CTC_QOS_RESRC_POOL_MODE2)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.igs_pool_size, sizeof(pool_size));
        if (pool_size[CTC_QOS_IGS_RESRC_NON_DROP_POOL] == 0)
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else if (p_cfg->resrc_pool.igs_pool_mode == CTC_QOS_RESRC_POOL_USER_DEFINE)
    {
        sal_memcpy(pool_size, p_cfg->resrc_pool.igs_pool_size, sizeof(pool_size));
    }

    if (pool_size[CTC_QOS_IGS_RESRC_DEFAULT_POOL] == 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    /* 2. enable ingress resource management*/
    cmd = DRV_IOR(IrmMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_misc_ctl));
    SetIrmMiscCtl(V, c2cPacketDpEn_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, c2cPacketDp_f , &irm_misc_ctl, 3);
    SetIrmMiscCtl(V, continuousPortTcUseScRemain_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, criticalPacketDpEn_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, criticalPacketDp_f , &irm_misc_ctl, 3);
    SetIrmMiscCtl(V, localPhyPortResrcBase_f , &irm_misc_ctl, 0);
    SetIrmMiscCtl(V, localPhyPortResrcEn_f , &irm_misc_ctl, 0);
    SetIrmMiscCtl(V, localPhyPortResrcPriEn_f , &irm_misc_ctl, 0);
    SetIrmMiscCtl(V, noCareC2cPacket_f , &irm_misc_ctl, 0);
    SetIrmMiscCtl(V, noCareCriticalPacket_f , &irm_misc_ctl, 0);
    SetIrmMiscCtl(V, resourceCheckEn_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, resourceCheckMode_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, resrcUseCntMin_f , &irm_misc_ctl, 2);
    SetIrmMiscCtl(V, scExcludeGuarantee_f , &irm_misc_ctl, 1);
    SetIrmMiscCtl(V, glbPortTcGuaranteedDis_f , &irm_misc_ctl, 1);

    cmd = DRV_IOW(IrmMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_misc_ctl));

    /* 3. threshold config, total :15k cells (15*1024) */
    cmd = DRV_IOR(DsIrmMiscThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_misc_thrd));
    SetDsIrmMiscThrd(V, c2cThrd_f, &irm_misc_thrd, pool_size[CTC_QOS_IGS_RESRC_C2C_POOL]);
    SetDsIrmMiscThrd(V, criticalThrd_f, &irm_misc_thrd, pool_size[CTC_QOS_IGS_RESRC_CONTROL_POOL]);
    SetDsIrmMiscThrd(V, totalThrd_f, &irm_misc_thrd, (pool_size[CTC_QOS_IGS_RESRC_DEFAULT_POOL] +
                                                      pool_size[CTC_QOS_IGS_RESRC_NON_DROP_POOL] +
                                                      pool_size[CTC_QOS_IGS_RESRC_MIN_POOL] +
                                                      pool_size[CTC_QOS_IGS_RESRC_C2C_POOL] +
                                                      pool_size[CTC_QOS_IGS_RESRC_ROUND_TRIP_POOL] +
                                                      pool_size[CTC_QOS_IGS_RESRC_CONTROL_POOL]));

    cmd = DRV_IOW(DsIrmMiscThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_misc_thrd));

    /* 4. configure default pool threshold, only use one sc */
    cutthrough_enable = sys_usw_chip_get_cut_through_en(lchip);
    cmd = DRV_IOR(DsIrmScThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_sc_thrd));
    SetDsIrmScThrd(V, scEn_f, &irm_sc_thrd, cutthrough_enable ? 0 : 1);
    SetDsIrmScThrd(V, scThrd_f, &irm_sc_thrd, (pool_size[CTC_QOS_IGS_RESRC_DEFAULT_POOL]+pool_size[CTC_QOS_IGS_RESRC_C2C_POOL]
                   +pool_size[CTC_QOS_IGS_RESRC_CONTROL_POOL])/ 8);
    cmd = DRV_IOW(DsIrmScThrd_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_sc_thrd));

    /* 5. mapping profiles, only use profile 0: all priority mapped to tc priority/2 and sc 0 */
    for (mcast = 0; mcast < 2; mcast++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            index = mcast << 4 | priority;
            cmd = DRV_IOR(DsIrmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_prio_sc_tc_map));
            SetDsIrmPrioScTcMap(V, g2_0_mappedTc_f, &irm_prio_sc_tc_map, priority / 2);
            SetDsIrmPrioScTcMap(V, g1_0_mappedSc_f , &irm_prio_sc_tc_map, CTC_QOS_IGS_RESRC_DEFAULT_POOL);
            cmd = DRV_IOW(DsIrmPrioScTcMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_prio_sc_tc_map));
        }
    }

    for (mcast = 0; mcast < 2; mcast++)
    {
        for (color = 0; color < 4; color++)
        {
            index = mcast << 2 | color;
            field_val = (color + 3) % 4;
            cmd = DRV_IOR(DsIrmColorDpMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_color_dp_map));
            SetDsIrmColorDpMap(V, g_0_dropPrecedence_f, &irm_color_dp_map, field_val);
            cmd = DRV_IOW(DsIrmColorDpMap_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_color_dp_map));
        }
    }

    for (index = 0; index < SYS_MAX_CHANNEL_NUM; index++)
    {
        field_val = 0;
        cmd = DRV_IOW(DsIrmChannel_t, DsIrmChannel_irmProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }

#if 0 /*default use port min, not use portTc min*/
    /* 6. configure portTc minimum guarantee profile, profile 0: portTc Min = 20*/
    DsIrmPortTcGuaranteedThrdProfile_m igr_port_tc_min_profile;
    sal_memset(&igr_port_tc_min_profile, 0, sizeof(DsIrmPortTcGuaranteedThrdProfile_m));
    cmd = DRV_IOR(DsIrmPortTcGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &igr_port_tc_min_profile));
    SetDsIrmPortTcGuaranteedThrdProfile(V, portTcGuaranteedThrd_f, &igr_port_tc_min_profile, 20);
    cmd = DRV_IOW(DsIrmPortTcGuaranteedThrdProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &igr_port_tc_min_profile));

    /* default all use profile id 0 */
    for (lport = 0; lport < 64; lport++)
    {
        for (tc = 0; tc < 8; tc++)
        {
            field_val = 0;
            field_id = DsIrmPortTcCfg_u_g1_portTcGuaranteedThrdProfId_f;
            cmd = DRV_IOW(DsIrmPortTcCfg_t, field_id);
            index = lport << 3 | tc;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        }
    }
#endif
    /* 7. configure port minimum guarantee profile, default all use profile id 0: port Min = 20*/
    field_val = 20;
    cmd = DRV_IOW(DsIrmPortGuaranteedThrdProfile_t, DsIrmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    for (lport = 0; lport < 64; lport++)
    {
        field_val = 0;
        field_id = DsIrmPortCfg_portGuaranteedThrdProfId_f;
        cmd = DRV_IOW(DsIrmPortCfg_t, field_id);
        index = lport;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }

    /* 8. configure SC0 congestion level watermarks, disable ingress congest level */
    for (cng_level = 0; cng_level < 3; cng_level++)
    {
        field_val = (DRV_IS_DUET2(lchip)) ? 0x1FF : 0x3FF; /*aways in congest level 0*/
        cmd = DRV_IOW(DsIrmScCngThrdProfile_t, DsIrmScCngThrdProfile_g_0_scCngThrd_f + cng_level);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    /* 9. disable port limited thrd check*/
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortCfg_t, &entry_num);
    for (index = 0; index < entry_num; index++)
    {
        cmd = DRV_IOR(DsIrmPortCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_port_cfg));
        SetDsIrmPortCfg(V, portLimitedThrdProfId_f, &irm_port_cfg, 0xF);
        cmd = DRV_IOW(DsIrmPortCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_port_cfg));
    }

    /* 10. disable portTc limited thrd check*/
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortTcCfg_t, &entry_num);
    for (index = 0; index < entry_num; index++)
    {
        cmd = DRV_IOR(DsIrmPortTcCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_port_tc_cfg));
        SetDsIrmPortTcCfg(V, u_g1_portTcLimitedThrdProfId_f, &irm_port_tc_cfg, 0xF);
        SetDsIrmPortTcCfg(V, u_g2_portLimitedThrdProfId_f, &irm_port_tc_cfg, 0xF);
        cmd = DRV_IOW(DsIrmPortTcCfg_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &irm_port_tc_cfg));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_get_profile_from_hw(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8  chan_id = 0;
    uint32 table_index = 0;
    uint16  group_id = 0;
    uint8  queue_offset = 0;

    DsQMgrNetChanShpProfile_m net_chan_shp_profile;

    CTC_PTR_VALID_CHECK(p_shp_profile);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    /*Get chan shaping profile*/
    sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));
    cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile));
    p_shp_profile->chan_shp_tokenThrd = GetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile);
    p_shp_profile->chan_shp_tokenThrdShift = GetDsQMgrNetChanShpProfile(V, tokenThrdShift_f, &net_chan_shp_profile);
    if ((MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD) != p_shp_profile->chan_shp_tokenThrd) || (MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT) != p_shp_profile->chan_shp_tokenThrdShift))
    {
        p_usw_queue_master[lchip]->store_chan_shp_en = 1;
    }
    if (DRV_IS_TSINGMA(lchip))
    {
        return CTC_E_NONE;
    }
    /*Get base/ctl queue shaping profile*/
    for (queue_offset = 0; queue_offset < SYS_MAX_QUEUE_OFFSET_PER_NETWORK_CHANNEL; queue_offset++)
    {
        table_index = (queue_offset < 8) ? (chan_id*8 + queue_offset)
        : (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC) + chan_id * 4 + (queue_offset - 8));
        cmd = DRV_IOR(DsQMgrNetQueShpProfId_t, DsQMgrNetQueShpProfId_profId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
        p_shp_profile->queue_shp_profileId[queue_offset] = field_val;
        if (field_val != 0)
        {
            p_usw_queue_master[lchip]->store_que_shp_en = 1;
        }
    }

    /*Get extend group/queue shaping profile*/
    for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
    {
        cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
        if (field_val == chan_id)
        {
            cmd = DRV_IOR(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            p_shp_profile->ext_grp_shp_profileId = field_val;

            for (queue_offset = 0; queue_offset < MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP); queue_offset++)
            {
                cmd = DRV_IOR(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (group_id*MCHIP_CAP(SYS_CAP_QOS_QUEUE_NUM_PER_EXT_GRP) + queue_offset), cmd, &field_val));
                p_shp_profile->ext_queue_shp_profileId[queue_offset] = field_val;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_set_port_drop_en(uint8 lchip, uint32 gport, bool enable, sys_qos_shape_profile_t* p_shp_profile)
{
    uint32 cmd                     = 0;
    uint32 field_val               = 0;
    uint8  chan_id                  = 0;
    uint32 table_index = 0;
    uint16  group_id = 0;
    uint8  queue_offset = 0;
    DsQMgrNetChanShpProfile_m net_chan_shp_profile;
    sal_memset(&net_chan_shp_profile, 0, sizeof(DsQMgrNetChanShpProfile_m));

    CTC_PTR_VALID_CHECK(p_shp_profile);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == chan_id)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    if (enable)
    {
        /*1. Enable Drop incoming channel packets*/
        field_val = 0;
        cmd = DRV_IOW(DsErmPortCfg_t, DsErmPortCfg_portLimitedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
        field_val = 7;
        cmd = DRV_IOW(DsErmPortCfg_t, DsErmPortCfg_portGuaranteedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

        /*2. Disable channel/queue/grp shaping */
        /*Disable chan shaping*/
        if (p_usw_queue_master[lchip]->store_chan_shp_en)
        {
            cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile));
            SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD));
            SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, MCHIP_CAP(SYS_CAP_QOS_SHP_TOKEN_THRD_SHIFT));
            cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile));
        }
        if (DRV_IS_TSINGMA(lchip))
        {
            field_val = 1;
            cmd = DRV_IOW(DsQMgrNetChanMiscCfg_t, DsQMgrNetChanMiscCfg_flushValid_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

            return CTC_E_NONE;
        }
        /*Disable base/ctl queue shaping*/
        if (p_usw_queue_master[lchip]->store_que_shp_en)
        {
            for (queue_offset = 0; queue_offset < SYS_MAX_QUEUE_OFFSET_PER_NETWORK_CHANNEL; queue_offset++)
            {
                field_val = 0;
                table_index = (queue_offset < 8) ? (chan_id * 8 + queue_offset)
                : (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC) + chan_id * 4 + (queue_offset - 8));
                cmd = DRV_IOW(DsQMgrNetQueShpProfId_t, DsQMgrNetQueShpProfId_profId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
            }
        }

        /*Disable extend group/queue shaping*/
        for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
        {
            cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
            if (field_val == chan_id)
            {
                field_val = 0;
                cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));

                for (queue_offset = 0; queue_offset < SYS_MAX_QUEUE_OFFSET_PER_EXT_GROUP; queue_offset++)
                {
                    field_val = 0;
                    cmd = DRV_IOW(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (group_id*4 + queue_offset), cmd, &field_val));
                }
            }
        }
    }
    else
    {
        /*1. Enable channel/queue/grp shaping*/
        /*Enable chan shaping*/
        if (p_usw_queue_master[lchip]->store_chan_shp_en)
        {
            cmd = DRV_IOR(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile));
            SetDsQMgrNetChanShpProfile(V, tokenThrd_f, &net_chan_shp_profile, p_shp_profile->chan_shp_tokenThrd);
            SetDsQMgrNetChanShpProfile(V, tokenThrdShift_f  , &net_chan_shp_profile, p_shp_profile->chan_shp_tokenThrdShift);
            cmd = DRV_IOW(DsQMgrNetChanShpProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &net_chan_shp_profile));
        }
        if (DRV_IS_DUET2(lchip))
        {
            /*Enable base/ctl queue shaping*/
            if (p_usw_queue_master[lchip]->store_que_shp_en)
            {
                for (queue_offset = 0; queue_offset < SYS_MAX_QUEUE_OFFSET_PER_NETWORK_CHANNEL; queue_offset++)
                {
                    field_val = p_shp_profile->queue_shp_profileId[queue_offset];
                    table_index = (queue_offset < 8) ? (chan_id * 8 + queue_offset)
                    : (MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_MISC) + chan_id * 4 + (queue_offset - 8));
                    cmd = DRV_IOW(DsQMgrNetQueShpProfId_t, DsQMgrNetQueShpProfId_profId_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, table_index, cmd, &field_val));
                }
            }

            /*Enable extend group/queue shaping*/
            for (group_id = 0; group_id < MCHIP_CAP(SYS_CAP_QOS_EXT_QUEUE_GRP_NUM); group_id++)
            {
                cmd = DRV_IOR(DsQMgrGrpChanMap_t, DsQMgrGrpChanMap_channelId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));
                if (field_val == chan_id)
                {
                    field_val = p_shp_profile->ext_grp_shp_profileId;
                    cmd = DRV_IOW(DsQMgrExtGrpShpProfId_t, DsQMgrExtGrpShpProfId_profId_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &field_val));

                    for (queue_offset = 0; queue_offset < SYS_MAX_QUEUE_OFFSET_PER_EXT_GROUP; queue_offset++)
                    {
                        field_val = p_shp_profile->ext_queue_shp_profileId[queue_offset];
                        cmd = DRV_IOW(DsQMgrExtQueShpProfId_t, DsQMgrExtQueShpProfId_profId_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (group_id * 4 + queue_offset), cmd, &field_val));
                    }

                }
            }
        }
        else if (DRV_IS_TSINGMA(lchip))
        {
            field_val = 0;
            cmd = DRV_IOW(DsQMgrNetChanMiscCfg_t, DsQMgrNetChanMiscCfg_flushValid_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
        }

        /*2. Disable Drop incoming channel packets*/
        field_val = 0xF;
        cmd = DRV_IOW(DsErmPortCfg_t, DsErmPortCfg_portLimitedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
        field_val = 0;
        cmd = DRV_IOW(DsErmPortCfg_t, DsErmPortCfg_portGuaranteedThrdProfId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_queue_get_port_depth(uint8 lchip, uint32 gport, uint32* p_depth)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8 chan_id = 0;

    CTC_PTR_VALID_CHECK(p_depth);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == chan_id)
    {
        SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    cmd = DRV_IOR(DsErmPortCnt_t, DsErmPortCnt_portCnt_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

    *p_depth = field_val;

    return CTC_E_NONE;
}

int32
sys_usw_qos_drop_dump_status(uint8 lchip)
{
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------Queue Drop-----------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Total WTD drop profile",p_usw_queue_master[lchip]->p_drop_wtd_profile_pool->max_count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n","--Used count", p_usw_queue_master[lchip]->p_drop_wtd_profile_pool->count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Total WRED drop profile", p_usw_queue_master[lchip]->p_drop_wred_profile_pool->max_count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n","--Used count", p_usw_queue_master[lchip]->p_drop_wred_profile_pool->count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s\n", "Reserv profile id");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "--default pool", p_usw_queue_master[lchip]->egs_pool[0].default_profile_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "--non-drop pool", p_usw_queue_master[lchip]->egs_pool[1].default_profile_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "--span pool", p_usw_queue_master[lchip]->egs_pool[2].default_profile_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "--control pool", p_usw_queue_master[lchip]->egs_pool[3].default_profile_id);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------Pause profile-----------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Normal Pause total profile",p_usw_queue_master[lchip]->p_fc_profile_pool->max_count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n","--Used count", p_usw_queue_master[lchip]->p_fc_profile_pool->count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Priority Pause total profile",p_usw_queue_master[lchip]->p_pfc_profile_pool->max_count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n","--Used count", p_usw_queue_master[lchip]->p_pfc_profile_pool->count);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");

    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "-------------------------Resrc status-----------------------\n");
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Ingress resrc mode", p_usw_queue_master[lchip]->igs_resrc_mode);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-30s: %d\n", "Egress  resrc mode", p_usw_queue_master[lchip]->egs_resrc_mode);
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "\n");

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_fc_default_profile(uint8 lchip, uint32 gport)
{
    uint8 priority_class = 0;
    ctc_qos_resrc_fc_t fc;
    uint32 port_type = 0;

    sal_memset(&fc, 0, sizeof(ctc_qos_resrc_fc_t));

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
         return CTC_E_NONE;
    }

    fc.xon_thrd = 240;
    fc.xoff_thrd = 256;
    fc.drop_thrd = MCHIP_CAP(SYS_CAP_QOS_QUEUE_MAX_DROP_THRD);
    fc.gport = gport;
    fc.is_pfc = 0;
    for (priority_class = 0; priority_class < 8; priority_class++)
    {
        fc.priority_class = priority_class;
        CTC_ERROR_RETURN(sys_usw_qos_fc_add_profile(lchip, &fc));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_set_egs_pool_resrc_prof(uint8 lchip, uint8 pool)
{
    uint16 lport = 0;
    uint8 queue_offset = 0;
    uint8  profile_id = 0;
    uint8  gchip = 0;
    uint32 cmd = 0;
    uint32 index = 0;
    DsErmQueueCfg_m erm_queue_cfg;
    sal_memset(&erm_queue_cfg, 0, sizeof(DsErmQueueCfg_m));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    profile_id = p_usw_queue_master[lchip]->egs_pool[pool].default_profile_id;
    if(pool == CTC_QOS_EGS_RESRC_SPAN_POOL)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); lport++)
        {
            for (queue_offset = 0; queue_offset < 2; queue_offset ++)
            {
                index = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC) + (lport * MCHIP_CAP(SYS_CAP_QOS_MISC_QUEUE_NUM)) + queue_offset;;
                cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
                SetDsErmQueueCfg(V, queueLimitedThrdProfId_f, &erm_queue_cfg, profile_id);
                cmd = DRV_IOW(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
            }
        }
    }
    else if(pool == CTC_QOS_EGS_RESRC_CONTROL_POOL)
    {
        for(index = MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_EXCP); index < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_NETWORK_MISC); index ++)
        {
            cmd = DRV_IOR(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
            SetDsErmQueueCfg(V, queueLimitedThrdProfId_f, &erm_queue_cfg, profile_id);
            cmd = DRV_IOW(DsErmQueueCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &erm_queue_cfg));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_fcdl_interval(uint8 lchip, uint32 time)
{
    uint32 value = 0;
    uint32 cmd = DRV_IOW(RefDivSgmacPauseLockPulse_t, RefDivSgmacPauseLockPulse_cfgRefDivSgmacPauseLockPulse_f);

    CTC_VALUE_RANGE_CHECK(time, 1, SYS_MAX_FCDL_INTERVAL_TIME);
    /* time = cfgRefDivSgmacPauseLockPulse[30:8] * 6.4ns */
    value =  (time * 156250) << 8;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_qos_fcdl_state_isr(uint8 lchip, uint8 mac_id)
{
    uint8  gchip = 0;
    CTC_INTERRUPT_EVENT_FUNC event_cb;
    ctc_qos_fcdl_event_t event;
    uint32 cmd = 0;
    uint32 step = 0;
    uint32 tbl_id = 0;
    uint16 lport = 0;
    Sgmac0RxPauseLockDet00_m rx_pause_lockdet;

    sal_memset(&rx_pause_lockdet, 0, sizeof(Sgmac0RxPauseLockDet00_m));
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    sal_memset(&event, 0, sizeof(ctc_qos_fcdl_event_t));
    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_FCDL_DETECT, &event_cb));
    lport = sys_usw_dmps_get_lport_with_mac(lchip, mac_id);
    event.gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    step = Sgmac1RxPauseLockDet00_t - Sgmac0RxPauseLockDet00_t;
    tbl_id = Sgmac0RxPauseLockDet00_t + (mac_id / 4) + ((mac_id % 4)*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_lockdet));
    event.state[0] = GetSgmac0RxPauseLockDet00(V, dbgSgmac0RxPauseLockDetect0_f, &rx_pause_lockdet);
    SetSgmac0RxPauseLockDet00(V, dbgSgmac0RxPauseLockDetect0_f, &rx_pause_lockdet ,0);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_lockdet));
    step = Sgmac1RxPauseLockDet10_t - Sgmac0RxPauseLockDet10_t;
    tbl_id = Sgmac0RxPauseLockDet10_t + (mac_id / 4) + ((mac_id % 4)*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_lockdet));
    event.state[1] = GetSgmac0RxPauseLockDet10(V, dbgSgmac0RxPauseLockDetect1_f, &rx_pause_lockdet);
    SetSgmac0RxPauseLockDet10(V, dbgSgmac0RxPauseLockDetect1_f, &rx_pause_lockdet ,0);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_lockdet));

    if(event_cb)
    {
        CTC_ERROR_RETURN(event_cb(gchip, &event));
    }
    SYS_QOS_QUEUE_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"gport = %d\n,state0 = %d,state1 = %d", event.gport,event.state[0],event.state[1]);

    return CTC_E_NONE;
}

int32
sys_usw_qos_set_ecn_enable(uint8 lchip, uint32 enable)
{
    uint32 ecn_map_action_prof = 0;
    uint16 index = 0;
    uint32 cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_cnActionEn_f);
    uint32 field_val = enable ? 1 : 0;
    DsEcnMappingAction_m ecn_map_action;

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*only TCP and UDP ECN Aware*/
    field_val = enable ? ((1 << CTC_PARSER_L4_TYPE_TCP)|(1<<CTC_PARSER_L4_TYPE_UDP)):0;
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_ecnAware_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(&ecn_map_action, 0, sizeof(DsEcnMappingAction_m));
    sys_usw_ftm_query_table_entry_num(lchip, DsEcnMappingAction_t, &ecn_map_action_prof);
    for (index = 0; index < ecn_map_action_prof; index++)
    {
        SetDsEcnMappingAction(V, array_0_ecnAction_f, &ecn_map_action, CTC_QOS_INTER_CN_DROP);
        SetDsEcnMappingAction(V, array_1_ecnAction_f, &ecn_map_action, enable ? CTC_QOS_INTER_CN_NON_CE : CTC_QOS_INTER_CN_DROP);
        SetDsEcnMappingAction(V, array_2_ecnAction_f, &ecn_map_action, enable ? CTC_QOS_INTER_CN_NON_CE : CTC_QOS_INTER_CN_DROP);
        SetDsEcnMappingAction(V, array_3_ecnAction_f, &ecn_map_action, enable ? CTC_QOS_INTER_CN_CE : CTC_QOS_INTER_CN_DROP);
        cmd = DRV_IOW(DsEcnMappingAction_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ecn_map_action));
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_get_ecn_enable(uint8 lchip, uint32* enable)
{

    uint32 cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_cnActionEn_f);
    uint32 field_val = 0;

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    *enable = field_val;

    return CTC_E_NONE;
}

int32
_sys_usw_queue_drop_ecn_init(uint8 lchip)
{
    uint8 inner_ecn = 0;
    uint8 outer_ecn = 0;
    uint8 ecn_action = 0;
    uint32 cmd      = 0;
    uint16  index = 0;
    uint8  step = 0;

    /** Decap
    +---------+------------------------------------------------+
    |Arriving |            Arriving Outer Header               |
    |   Inner +---------+------------+------------+------------+
    |  Header | Not-ECT | ECT(0)     | ECT(1)     |     CE     |
    +---------+---------+------------+------------+------------+
    | Not-ECT | Not-ECT |Not-ECT(!!!)|Not-ECT(!!!)| <drop>(!!!)|
    |  ECT(0) |  ECT(0) | ECT(0)     | ECT(1)     |     CE     |
    |  ECT(1) |  ECT(1) | ECT(1) (!) | ECT(1)     |     CE     |
    |    CE   |      CE |     CE     |     CE(!!!)|     CE     |
    +---------+---------+------------+------------+------------+
    */

    /** Encap
    +-----------------+-------------------------------+
    | Incoming Header |    Departing Outer Header     |
    | (also equal to  +---------------+---------------+
    | departing Inner | Compatibility |    Normal     |
    |     Header)     | Mode(3168)    |    Mode(4301) |
    +-----------------+---------------+---------------+
    |    Not-ECT      |   Not-ECT     |   Not-ECT     |
    |     ECT(0)      |   Not-ECT     |    ECT(0)     |
    |     ECT(1)      |   Not-ECT     |    ECT(1)     |
    |       CE        |   Not-ECT     |      CE       |
    +-----------------+---------------+---------------+
    */
    sys_queue_drop_ecn_t decap_ecn[4][4] =
    {   {{0,0,0},
         {0,0,1},
         {0,0,2},
         {0,0,3}},

        {{0,0,0},
         {0,0,1},
         {0,0,1},
         {0,0,3}},

        {{0,0,0},
         {0,0,1},
         {0,0,2},
         {0,0,3}},

        {{1,0,0},
         {0,0,3},
         {0,0,3},
         {0,0,3}},
    };

    /** ecn_action map to ecn
    +---------+------------------------------------------------+
    |         |               ecn action                       |
    |Incoming +---------+------------+------------+------------+
    |  Header |   DROP  |  NON_DROP  |   NON_CE   |     CE     |
    +---------+---------+------------+------------+------------+
    | Not-ECT | Not-ECT |   Not-ECT  |  Not-ECT   |   Not-ECT  |
    |  ECT(0) | ECT(0)  |   ECT(0)   |   ECT(0)   |     CE     |
    |  ECT(1) |  ECT(1) |   ECT(1)   |   ECT(1)   |     CE     |
    |    CE   |   CE    |   CE       |     CE     |     CE     |
    +---------+---------+------------+------------+------------+
    */
    uint8 ecnAction2ecn[4][4] =
    {    {0,
          1,
          2,
          3},

         {0,
          1,
          2,
          3},

         {0,
          1,
          2,
          3},

         {0,
          3,
          3,
          3},
    };

    DsTunnelEcnMapping_m tunnel_ecn_map;
    DsIpEcnMapping_m ip_ecn_map;
    DsEcnActionMappingEcn_m ecn_action_map_ecn;
    DsInnerEcnMappingTunnelEcn_m inner_ecn_map_tunnel_ecn;
    sal_memset(&tunnel_ecn_map, 0, sizeof(DsTunnelEcnMapping_m));
    sal_memset(&ip_ecn_map, 0, sizeof(DsIpEcnMapping_m));
    sal_memset(&ecn_action_map_ecn, 0, sizeof(DsEcnActionMappingEcn_m));
    sal_memset(&inner_ecn_map_tunnel_ecn, 0, sizeof(DsInnerEcnMappingTunnelEcn_m));

    /*Decap*/
    for (outer_ecn = 0; outer_ecn < 4; outer_ecn++)
    {
        for (inner_ecn = 0; inner_ecn < 4; inner_ecn++)
        {
            index = outer_ecn;
            step = inner_ecn * 3;
            SetDsTunnelEcnMapping(V, array_0_discard_f + step, &tunnel_ecn_map, decap_ecn[outer_ecn][inner_ecn].discard);
            SetDsTunnelEcnMapping(V, array_0_exceptionEn_f + step, &tunnel_ecn_map, decap_ecn[outer_ecn][inner_ecn].exception_en);
            SetDsTunnelEcnMapping(V, array_0_mappedEcnTunnel_f + step, &tunnel_ecn_map, decap_ecn[outer_ecn][inner_ecn].mapped_ecn);
            cmd = DRV_IOW(DsTunnelEcnMapping_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tunnel_ecn_map));
        }
    }

    for (inner_ecn = 0; inner_ecn < 4; inner_ecn++)
    {
        SetDsIpEcnMapping(V, array_0_mappedEcn_f + inner_ecn, &ip_ecn_map, inner_ecn);
        cmd = DRV_IOW(DsIpEcnMapping_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ip_ecn_map));
    }

    /*ecn_action map to ecn*/
    for (ecn_action = 0; ecn_action < 4; ecn_action++)
    {
        for (outer_ecn = 0; outer_ecn < 4; outer_ecn++)
        {
            index = ecn_action;
            SetDsEcnActionMappingEcn(V, array_0_ecnValue_f + outer_ecn, &ecn_action_map_ecn, ecnAction2ecn[ecn_action][outer_ecn]);
            cmd = DRV_IOW(DsEcnActionMappingEcn_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ecn_action_map_ecn));
        }
    }

    /*encap*/
    for (inner_ecn = 0; inner_ecn < 4; inner_ecn++)
    {
        SetDsInnerEcnMappingTunnelEcn(V, array_0_ecnValue_f + inner_ecn, &inner_ecn_map_tunnel_ecn, inner_ecn);
        cmd = DRV_IOW(DsInnerEcnMappingTunnelEcn_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &inner_ecn_map_tunnel_ecn));
    }

    return CTC_E_NONE;
}

int32
sys_usw_queue_drop_init_profile(uint8 lchip, ctc_qos_global_cfg_t* p_cfg)
{
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint8 profile_used_cnt = 0;
    uint32 que_drop_wtd_profile_num = 0;
    uint32 que_drop_wred_profile_num = 0;
    uint32 fc_profile_num = 0;
    uint32 pfc_profile_num = 0;
    uint32 fc_dropthrd_profile_num = 0;
    uint32 pfc_dropthrd_profile_num = 0;
    ctc_spool_t spool;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    /* init drop profile spool table */
    sys_usw_ftm_query_table_entry_num(lchip, DsErmQueueLimitedThrdProfile_t, &que_drop_wtd_profile_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsErmAqmThrdProfile_t, &que_drop_wred_profile_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortFlowControlThrdProfile_t, &fc_profile_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortTcFlowControlThrdProfile_t, &pfc_profile_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortLimitedThrdProfile_t, &fc_dropthrd_profile_num);
    sys_usw_ftm_query_table_entry_num(lchip, DsIrmPortTcLimitedThrdProfile_t, &pfc_dropthrd_profile_num);

    /* care congest level*/
    que_drop_wtd_profile_num  = que_drop_wtd_profile_num / SYS_RESRC_MAX_CONGEST_LEVEL_NUM;
    fc_profile_num  = fc_profile_num / SYS_RESRC_MAX_CONGEST_LEVEL_NUM;
    pfc_profile_num  = pfc_profile_num / SYS_RESRC_MAX_CONGEST_LEVEL_NUM;
    fc_dropthrd_profile_num  = fc_dropthrd_profile_num / SYS_RESRC_MAX_CONGEST_LEVEL_NUM - 1;
    pfc_dropthrd_profile_num  = pfc_dropthrd_profile_num / SYS_RESRC_MAX_CONGEST_LEVEL_NUM - 1;

    /*create queue drop wtd profile spool*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = que_drop_wtd_profile_num - 1;
    spool.user_data_size = sizeof(sys_queue_drop_wtd_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_drop_wtd_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_drop_wtd_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_queue_drop_wtd_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_queue_drop_wtd_free_profileId;
    p_usw_queue_master[lchip]->p_drop_wtd_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_drop_wtd_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    /*create queue drop wred profile spool*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = que_drop_wred_profile_num - 1;
    spool.user_data_size = sizeof(sys_queue_drop_wred_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_queue_drop_wred_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_queue_drop_wred_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_queue_drop_wred_alloc_profileId;
    spool.spool_free = (spool_free_fn)_sys_usw_queue_drop_wred_free_profileId;
    p_usw_queue_master[lchip]->p_drop_wred_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_drop_wred_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = fc_profile_num;
    spool.user_data_size = sizeof(sys_qos_fc_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_qos_fc_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_qos_fc_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_qos_fc_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_qos_fc_free_profileId;
    p_usw_queue_master[lchip]->p_fc_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_fc_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }

    /*fc and pfc use same struct and hash function*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = pfc_profile_num;
    spool.user_data_size = sizeof(sys_qos_fc_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_qos_fc_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_qos_fc_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_qos_pfc_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_qos_pfc_free_profileId;
    p_usw_queue_master[lchip]->p_pfc_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_pfc_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error3;
    }

    /*fc and pfc dropth use same struct and hash function*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = fc_dropthrd_profile_num;
    spool.user_data_size = sizeof(sys_qos_fc_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_qos_fc_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_qos_fc_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_qos_fc_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_qos_fc_free_profileId;
    p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error4;
    }

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 1;
    spool.max_count = pfc_dropthrd_profile_num;
    spool.user_data_size = sizeof(sys_qos_fc_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_qos_fc_hash_make_profile;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_qos_fc_hash_cmp_profile;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_qos_pfc_alloc_profileId;
    spool.spool_free  = (spool_free_fn)_sys_usw_qos_pfc_free_profileId;
    p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool = ctc_spool_create(&spool);
    if (NULL == p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error5;
    }

    /* add static profile */
    CTC_ERROR_GOTO(_sys_usw_queue_drop_add_static_profile(lchip, p_cfg), ret, error6);
    CTC_ERROR_GOTO(sys_usw_qos_fc_add_static_profile(lchip), ret, error6);


    /*init queue drop wtd profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_queue_drop_wtd,
                                                                    1, "opf-queue-drop-wtd"), ret, error6);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_queue_drop_wtd;
    opf.pool_index = 0;
    profile_used_cnt = p_usw_queue_master[lchip]->p_drop_wtd_profile_pool->count;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, profile_used_cnt, (que_drop_wtd_profile_num - profile_used_cnt - 1)),
                                                                    ret, error7);


    /*init queue drop wred profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_queue_drop_wred,
                                                                    1, "opf-queue-drop-wred"), ret, error7);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_queue_drop_wred;
    opf.pool_index = 0;
    profile_used_cnt = p_usw_queue_master[lchip]->p_drop_wred_profile_pool->count;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, profile_used_cnt, (que_drop_wred_profile_num - profile_used_cnt - 1)),
                                                                    ret, error8);

    /*init fc profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_resrc_fc,
                                                                    1, "opf-resrc-fc"), ret, error8);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_resrc_fc;
    opf.pool_index = 0;
    profile_used_cnt = p_usw_queue_master[lchip]->p_fc_profile_pool->count;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, profile_used_cnt, (fc_profile_num - profile_used_cnt)),
                                                                    ret, error9);

    /*init pfc profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_resrc_pfc,
                                                                    1, "opf-resrc-pfc"), ret, error9);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_resrc_pfc;
    opf.pool_index = 0;
    profile_used_cnt = p_usw_queue_master[lchip]->p_pfc_profile_pool->count;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, profile_used_cnt, (pfc_profile_num - profile_used_cnt)),
                                                                    ret, error10);

    /*init fc dropth profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd,
                                                                    1, "opf-resrc-fc-dropthrd"), ret, error10);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, fc_dropthrd_profile_num ),
                                                                    ret, error11);

    /*init pfc dropth profile opf*/
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd,
                                                                    1, "opf-resrc-pfc-dropthrd"), ret, error11);

    opf.pool_type = p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, pfc_dropthrd_profile_num ),
                                                                    ret, error12);

    return CTC_E_NONE;

error12:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd);

error11:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd);
error10:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc);

error9:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc);

error8:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wred);
error7:
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wtd);
error6:
    ctc_spool_free(p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool);
    p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool = NULL;
error5:
    ctc_spool_free(p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool);
    p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool = NULL;
error4:
    ctc_spool_free(p_usw_queue_master[lchip]->p_pfc_profile_pool);
    p_usw_queue_master[lchip]->p_pfc_profile_pool = NULL;
error3:
    ctc_spool_free(p_usw_queue_master[lchip]->p_fc_profile_pool);
    p_usw_queue_master[lchip]->p_fc_profile_pool = NULL;
error2:
    ctc_spool_free(p_usw_queue_master[lchip]->p_drop_wred_profile_pool);
    p_usw_queue_master[lchip]->p_drop_wred_profile_pool = NULL;
error1:
    ctc_spool_free(p_usw_queue_master[lchip]->p_drop_wtd_profile_pool);
    p_usw_queue_master[lchip]->p_drop_wtd_profile_pool = NULL;
error0:
    return ret;
}

int32
sys_usw_queue_drop_deinit_profile(uint8 lchip)
{
    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc);

    ctc_spool_free(p_usw_queue_master[lchip]->p_pfc_profile_pool);
    p_usw_queue_master[lchip]->p_pfc_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc);

    ctc_spool_free(p_usw_queue_master[lchip]->p_fc_profile_pool);
    p_usw_queue_master[lchip]->p_fc_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_fc_dropthrd);

    ctc_spool_free(p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool);
    p_usw_queue_master[lchip]->p_fc_dropthrd_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_resrc_pfc_dropthrd);

    ctc_spool_free(p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool);
    p_usw_queue_master[lchip]->p_pfc_dropthrd_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wred);

    ctc_spool_free(p_usw_queue_master[lchip]->p_drop_wred_profile_pool);
    p_usw_queue_master[lchip]->p_drop_wred_profile_pool = NULL;

    sys_usw_opf_deinit(lchip, p_usw_queue_master[lchip]->opf_type_queue_drop_wtd);

    ctc_spool_free(p_usw_queue_master[lchip]->p_drop_wtd_profile_pool);
    p_usw_queue_master[lchip]->p_drop_wtd_profile_pool = NULL;

    return CTC_E_NONE;
}

/**
 @brief Queue dropping initialization.
*/
int32
sys_usw_queue_drop_init(uint8 lchip, void *p_glb_parm)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    QWriteCtl_m  q_write_ctl;
    ctc_qos_global_cfg_t * p_glb_cfg = NULL;
    p_glb_cfg = (ctc_qos_global_cfg_t *)p_glb_parm;

    sal_memset(&q_write_ctl, 0, sizeof(QWriteCtl_m));

    p_usw_queue_master[lchip]->resrc_check_mode = 0;
    p_usw_queue_master[lchip]->queue_thrd_factor = 0;
    CTC_ERROR_GOTO(_sys_usw_queue_igs_resrc_mgr_init(lchip, p_glb_cfg), ret, error0);
    CTC_ERROR_GOTO(_sys_usw_queue_egs_resrc_mgr_init(lchip, p_glb_cfg), ret, error0);
    CTC_ERROR_GOTO(sys_usw_queue_drop_init_profile(lchip, p_glb_cfg), ret, error0);
    CTC_ERROR_GOTO(_sys_usw_queue_drop_ecn_init(lchip), ret, error1);

    p_usw_queue_master[lchip]->igs_resrc_mode = p_glb_cfg->resrc_pool.igs_pool_mode;
    p_usw_queue_master[lchip]->egs_resrc_mode = p_glb_cfg->resrc_pool.egs_pool_mode;

    /*Span-Mcast pool*/
    if (p_usw_queue_master[lchip]->egs_pool[CTC_QOS_EGS_RESRC_SPAN_POOL].egs_congest_level_num &&
        (1 == p_usw_queue_master[lchip]->enq_mode || 2 == p_usw_queue_master[lchip]->enq_mode))
    {
        cmd = DRV_IOR(QWriteCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));
        SetQWriteCtl(V, spanPktRewriteScEn_f, &q_write_ctl, 1);
        SetQWriteCtl(V, spanPktRewriteSc_f, &q_write_ctl, 2);
        cmd = DRV_IOW(QWriteCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_write_ctl));
        CTC_ERROR_GOTO(sys_usw_queue_set_egs_pool_resrc_prof(lchip, CTC_QOS_EGS_RESRC_SPAN_POOL), ret, error1);
    }
    if (p_usw_queue_master[lchip]->egs_pool[CTC_QOS_EGS_RESRC_CONTROL_POOL].egs_congest_level_num)
    {
        CTC_ERROR_GOTO(sys_usw_queue_set_egs_pool_resrc_prof(lchip, CTC_QOS_EGS_RESRC_CONTROL_POOL), ret, error1);
    }

    return CTC_E_NONE;

error1:
    (void)sys_usw_queue_drop_deinit_profile(lchip);
error0:
    return ret;
}

int32
sys_usw_queue_drop_deinit(uint8 lchip)
{
    (void)sys_usw_queue_drop_deinit_profile(lchip);

    return CTC_E_NONE;
}

