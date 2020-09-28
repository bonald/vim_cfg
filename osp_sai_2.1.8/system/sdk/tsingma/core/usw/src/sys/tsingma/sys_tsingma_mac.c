/**
 @file sys_usw_port.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_interrupt.h"
#include "ctc_warmboot.h"
 /*#include "../../../../dkits/ctc_dkit.h"*/

#include "sys_usw_common.h"
#include "sys_tsingma_datapath.h"
#include "sys_tsingma_mac.h"
#include "sys_usw_chip.h"
#include "sys_usw_peri.h"
#include "sys_tsingma_peri.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_dmps.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_qos.h"
#include "sys_usw_port.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_mcu.h"
#include "sys_tsingma_mcu.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(op, lport) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            _sys_tsingma_mac_mutex_flag_unlock(lchip, lport); \
            return (rv); \
        } \
    } while (0)

sys_datapath_lport_xpipe_mapping_t g_tsingma_xpipe_mac_mapping[SYS_USW_XPIPE_PORT_NUM] =
{
    /* HSS12G MAC */
    { 0,  0,  4},
    { 1,  1,  5},
    { 2,  2,  6},
    { 3,  3,  7},
    {20, 20, 16},
    {21, 21, 17},
    {22, 22, 18},
    {23, 23, 19},
    {12, 12,  8},
    {13, 13,  9},
    {14, 14, 10},
    {15, 15, 11},
    {28, 28, 24},
    {29, 29, 25},
    {30, 30, 26},
    {31, 31, 27},

    /* HSS28G MAC */
    {44, 44, 40},
    {45, 45, 41},
    {46, 46, 42},
    {47, 47, 43},
    {60, 60, 56},
    {61, 61, 57},
    {62, 62, 58},
    {63, 63, 59},
};

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
extern sys_mac_master_t* p_usw_mac_master[CTC_MAX_LOCAL_CHIP_NUM];
extern sys_datapath_master_t* p_usw_datapath_master[CTC_MAX_LOCAL_CHIP_NUM];

extern int32
sys_tsingma_datapath_dynamic_switch_get_info(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode,
sys_datapath_dynamic_switch_attr_t *target, uint8 qm_chg_flag, uint8 serdes_id_raw);

extern int32
drv_usw_chip_write(uint8 lchip, uint32 offset, uint32 value);

extern int32
drv_tsingma_mcu_lock(uint8 lchip, uint32 lock_id, uint8 core_id);

extern int32
drv_tsingma_mcu_unlock(uint8 lchip, uint32 lock_id, uint8 core_id);

extern int32
sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability);

extern int32
_sys_usw_mac_get_speed_mode_capability(uint8 lchip, uint32 gport, uint32* speed_mode_bitmap);

extern int32
_sys_usw_mac_get_if_type_capability(uint8 lchip, uint32 gport, uint32* if_type_bitmap);

extern int32
_sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability);

extern int32
_sys_usw_mac_set_nettxwriteen_en(uint8 lchip, uint16 lport, uint32 enable);

extern int32
sys_usw_mac_set_error_check(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_set_crc_recalc_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_get_error_check(uint8 lchip, uint32 gport, uint32* enable);

extern int32
sys_usw_mac_set_ipg(uint8 lchip, uint32 gport, uint32 value);

extern int32
sys_usw_mac_get_ipg(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
_sys_usw_mac_get_cl37_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value);

extern int32
_sys_usw_mac_get_cl37_an_remote_status(uint8 lchip, uint32 gport, uint32 auto_neg_mode, uint32* p_speed, uint32* p_link);

extern int32
sys_usw_mac_wb_restore(uint8 lchip);

extern int32
sys_usw_mac_wb_sync(uint8 lchip);

extern int32
_sys_usw_mac_flow_ctl_init(uint8 lchip);

extern void*
sys_usw_datapath_get_port_capability(uint8 lchip, uint16 lport, uint8 slice_id);

extern int32
sys_usw_qos_set_fc_default_profile(uint8 lchip, uint32 gport);

extern int32
_sys_tsingma_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable);

extern int32 sys_usw_mac_set_3ap_training_en(uint8 lchip, uint16 gport, uint32 enable);

extern int32
sys_usw_mac_set_cl73_ability(uint8 lchip, uint32 gport, uint32 ability);

extern int32
_sys_usw_mac_get_3ap_training_en(uint8 lchip, uint16 lport, uint32* p_status);

extern int32
_sys_usw_mac_get_cl73_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value);



STATIC int32
_sys_tsingma_mac_set_chan_framesize(uint8 lchip, uint8, uint32, sys_port_framesize_type_t);

int32
_sys_tsingma_mac_set_link_intr(uint8 lchip, uint32 gport, uint8 enable);

STATIC int32
sys_tsingma_mac_isr_pcs_dispatch(uint8 lchip, uint32 intr, void* p_data);

int32
_sys_tsingma_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable);

int32
sys_tsingma_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);

int32
sys_tsingma_mac_get_code_err(uint8 lchip, uint32 gport, uint32* code_err);


STATIC int32
sys_tsingma_mac_get_mac_signal_detect(uint8 lchip, uint32 gport, uint32* p_is_detect);

int32
_sys_tsingma_mac_set_fec_en(uint8 lchip, uint16 lport, uint32 type);

int32
_sys_tsingma_mac_get_fec_en(uint8 lchip, uint32 lport, uint32* p_value);

STATIC int32
_sys_tsingma_mac_init_mac_config(uint8 lchip);

int32
_sys_tsingma_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable, uint8 restart_flag);

int32
_sys_tsingma_mac_get_unidir_en(uint8 lchip, uint16 lport, uint32* p_value);

extern int32
sys_tsingma_datapath_check_xpipe_param(uint8 lchip, uint8 slice_id, uint16 lport, uint8 enable);

STATIC int32
_sys_tsingma_mac_set_sgmac_sgmii_en(uint8 lchip, uint16 lport, bool enable);

extern int32
sys_tsingma_datapath_set_serdes_prbs31_en(uint8 lchip, uint16 serdes_id, uint8 enable);

extern int32
sys_tsingma_datapath_get_serdes_prbs31_en(uint8 lchip, uint16 serdes_id, uint8* enable);

extern int32
sys_tsingma_datapath_get_serdes_prbs31_errcnt(uint8 lchip, uint16 serdes_id, uint32* errcnt);

extern int32
sys_tsingma_datapath_set_serdes_sm_reset(uint8 lchip, uint16 serdes_id);

extern int32
sys_tsingma_datapath_get_serdes_no_random_flag(uint8 lchip, uint16 serdes_id, uint8 *flag);

extern int32
sys_tsingma_datapath_set_serdes_sm_reset(uint8 lchip, uint16 serdes_id);

extern int32
sys_usw_datapath_get_serdes_cl73_status(uint8 lchip, uint16 serdes_id, uint16* p_value);

/****************************************************************************
 *
* Function
*
*****************************************************************************/
/*
Write multi-elements in single table.
The caller must ensure:
-- TRUE == (lengthof(field_id) == field_num && lengthof(value) == field_num)
-- val_stru points to a valid space
-- field_id[i] and value[i] are strictly matched
*/
int32
sys_tsingma_mac_write_table_elem(uint8 lchip, tbls_id_t tbl_id, uint32 field_num, fld_id_t field_id[], uint32 value[], void* val_stru)
{
    uint32 cmd = 0;
    uint32 field_cnt;

    if(NULL == field_id || NULL == value || NULL == val_stru)
    {
        return CTC_E_INVALID_PTR;
    }

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, val_stru));
    for(field_cnt = 0; field_cnt < field_num; field_cnt++)
    {
        DRV_IOW_FIELD(lchip, tbl_id, field_id[field_cnt], &(value[field_cnt]), val_stru);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, val_stru));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_chan_framesize(uint8 lchip, uint8 chan_id, uint32 value, sys_port_framesize_type_t type)
{
    uint32 cmd = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set mac_id:%d, type:%d.\n", chan_id, value);

    if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
    {
        cmd = DRV_IOW(DsChannelizeMode_t, DsChannelizeMode_portMinLen_f);
    }
    else if (SYS_PORT_FRAMESIZE_TYPE_MAX == type)
    {
        cmd = DRV_IOW(DsChannelizeMode_t, DsChannelizeMode_portMaxLen_f);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (chan_id & 0x3F), cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_chan_framesize(uint8 lchip, uint8 chan_id, uint32* p_value, sys_port_framesize_type_t type)
{
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
    {
        cmd = DRV_IOR(DsChannelizeMode_t, DsChannelizeMode_portMinLen_f);
    }
    else if (SYS_PORT_FRAMESIZE_TYPE_MAX == type)
    {
        cmd = DRV_IOR(DsChannelizeMode_t, DsChannelizeMode_portMaxLen_f);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (chan_id & 0x3F), cmd, &value));
    *p_value = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_framesize(uint8 lchip, uint32 gport, uint32 value, sys_port_framesize_type_t type)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port_cap));

    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_chan_framesize(lchip, p_port_cap->chan_id, value, type));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_framesize(uint8 lchip, uint32 gport, uint32* p_value, sys_port_framesize_type_t type)
{
    uint16 lport = 0;
    uint32 value = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port_cap));

    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_get_chan_framesize(lchip, p_port_cap->chan_id, &value, type));
    *p_value = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_max_frame(uint8 lchip, uint32 gport, uint32 value)
{
    if ((value < SYS_USW_MAX_FRAMESIZE_MIN_VALUE) || (value > SYS_USW_MAX_FRAMESIZE_MAX_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_framesize(lchip, gport, value, SYS_PORT_FRAMESIZE_TYPE_MAX));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_max_frame(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint32 value = 0;

    CTC_ERROR_RETURN(_sys_tsingma_mac_get_framesize(lchip, gport, &value, SYS_PORT_FRAMESIZE_TYPE_MAX));
    *p_value = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_min_frame_size(uint8 lchip, uint32 gport, uint32 size)
{

    if ((size < SYS_USW_MIN_FRAMESIZE_MIN_VALUE)
       || (size > SYS_USW_MIN_FRAMESIZE_MAX_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_framesize(lchip, gport, size, SYS_PORT_FRAMESIZE_TYPE_MIN));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_min_frame_size(uint8 lchip, uint32 gport, uint8* p_size)
{
    uint32 size = 0;

    CTC_ERROR_RETURN(_sys_tsingma_mac_get_framesize(lchip, gport, &size, SYS_PORT_FRAMESIZE_TYPE_MIN));
    *p_size = size;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_framesize_init(uint8 lchip)
{
    uint32 cmd;
    uint32 valueMin = SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE;
    uint32 valueMax = SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE;
    cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &valueMin));
    cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &valueMax));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_chan_init(uint8 lchip)
{
    uint16 lport = 0;
    uint32 field = 0;
    uint32 cmd = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;
    NetRxCtl_m net_rx_ctl;
    NetTxMiscCtl_m net_tx_misc_ctl;
    int32 ret = 0;

    CTC_ERROR_RETURN(_sys_tsingma_mac_framesize_init(lchip));

    for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
    {
        ret = (sys_usw_mac_get_port_capability(lchip, lport,&p_port_cap));
        if (ret == CTC_E_INVALID_CONFIG)
        {
            continue;
        }

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_chan_framesize(lchip, p_port_cap->chan_id, SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE, SYS_PORT_FRAMESIZE_TYPE_MAX));
        CTC_ERROR_RETURN(_sys_tsingma_mac_set_chan_framesize(lchip, p_port_cap->chan_id, SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE, SYS_PORT_FRAMESIZE_TYPE_MIN));

        field = lport & 0x3F;
        cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_port_cap->chan_id), cmd, &field));
    }

    sal_memset(&net_rx_ctl, 0, sizeof(NetRxCtl_m));
    cmd = DRV_IOR(NetRxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_rx_ctl));

    SetNetRxCtl(V, underLenChkEn_f, &net_rx_ctl, 1);
    SetNetRxCtl(V, cfgOverLenWithErr_f, &net_rx_ctl, 1);
    SetNetRxCtl(V, bpduChkEn_f, &net_rx_ctl, 0);

    cmd = DRV_IOW(NetRxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_rx_ctl));

    sal_memset(&net_tx_misc_ctl, 0, sizeof(NetTxMiscCtl_m));
    cmd = DRV_IOR(NetTxMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_misc_ctl));

    SetNetTxMiscCtl(V, runtDropEn_f, &net_tx_misc_ctl, 1);
    /* RTL logic is discard packet, when packet length <= cfgMinPktLen */
    SetNetTxMiscCtl(V, cfgMinPktLen0_f, &net_tx_misc_ctl, (SYS_USW_MIN_FRAMESIZE_MIN_VALUE - 1));

    cmd = DRV_IOW(NetTxMiscCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &net_tx_misc_ctl));

    /* RTL logic is discard packet, when egressHeaderAdjust output packet's length <= minPktLen */
    field = 5;
    cmd = DRV_IOW(EpeHdrAdjustMiscCtl_t, EpeHdrAdjustMiscCtl_minPktLen_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    return CTC_E_NONE;
}


int32
_sys_tsingma_mac_set_quadsgmaccfg(uint8 lchip, uint32 value, uint8 sgmac_idx)
{
    uint8 index   = 0;
    uint32 cmd    = 0;
    uint32 tbl_id = QuadSgmacCfg0_t + sgmac_idx;
    QuadSgmacCfg0_m mac_cfg;

    /* cfg Quad sgmac*/
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xaui_dxaui_config(uint8 lchip, uint16 lport)
{
    uint32 value     = 0;
    uint32 value_inc = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint16 step      = 0;
    uint8  index     = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  cnt       = 0;
    uint8  tmp_pcs_idx = 0;
    uint8  hss_idx     = 0;
    uint8  field_id    = 0;
    uint8  inner_serdes = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    Sgmac0RxCfg0_m         mac_rx_cfg;
    Sgmac0TxCfg0_m         mac_tx_cfg;
    SharedMiiCfg0_m        mii_cfg;
    SharedMii0Cfg0_m       mii_per_cfg;
    SharedPcsCfg0_m        shared_pcs_cfg;
    SharedPcsDsfCfg0_m     shared_dsf_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not xaui mode \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 2, sgmac_idx));

    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        value = 1;
    }
    else
    {
        value = 2;
    }
    /* cfg sgmac Rx/Tx*/
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Shared Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Shared Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    if(CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)  /*for XAUI mode, cfg SharedMii0Cfg.cfgMiiSpeed0 = 5*/
    {
        value = 5;
    }
    else  /*for DXAUI mode, cfg SharedMii0Cfg.cfgMiiSpeed0 = 6*/
    {
        value = 6;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);

    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        value = 1;
        value_inc = 0;
    }
    else
    {
        value = 5;
        if(CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)  /*for XAUI mode, cfg SharedMii0Cfg.cfgMiiTxPaceIncValue0 = 11*/
        {
            value_inc = 11;
        }
        else  /*for DXAUI mode, cfg SharedMii0Cfg.cfgMiiTxPaceIncValue0 = 3*/
        {
            value_inc = 3;
        }
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value_inc, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    value = 0x3fff;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn3_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 7;
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &shared_dsf_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    value = 80;
    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    for(cnt = 0; cnt < 4; cnt++)
    {
        tbl_id = SharedPcsFx0Cfg0_t + cnt * step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xfi_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step     = 0;
    uint8 index     = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx   = 0;
    uint8 pcs_idx   = 0;
    uint8 hss_idx   = 0;
    uint8 field_id  = 0;
    uint8 inner_serdes = 0;
    uint8 tmp_pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m mac_cfg;*/
    Sgmac0RxCfg0_m   mac_rx_cfg;
    Sgmac0TxCfg0_m   mac_tx_cfg;
    SharedMiiCfg0_m  mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m  shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 0, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        value = 2;
    }
    else
    {
        if (internal_mac_idx == 0)
        {
            value = 2;
        }
        else if (internal_mac_idx == 2)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }
    }

    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);

    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 17;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 16;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));


    value = 80;
    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xlg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint8 index     = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx   = 0;
    uint8 pcs_idx   = 0;
    uint8 internal_mac_idx = 0;
    uint8  tmp_pcs_idx = 0;
    uint8  hss_idx     = 0;
    uint8  field_id    = 0;
    uint8  inner_serdes = 0;
    uint8  step = 0;
    uint8  cnt       = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m mac_cfg;*/
    Sgmac0RxCfg0_m   mac_rx_cfg;
    Sgmac0TxCfg0_m   mac_tx_cfg;
    SharedMiiCfg0_m  mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m  shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not xlg mode \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 2, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    tbl_id = Sgmac0RxCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    tbl_id = Sgmac0TxCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    tbl_id = SharedMii0Cfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    value = 8;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 16;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 17;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn3_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    tbl_id = SharedPcsSerdes0Cfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    value = 80;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    for(cnt = 0; cnt < 4; cnt++)
    {
        tbl_id = SharedPcsFx0Cfg0_t + cnt * step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_tsingma_mac_set_sgmac_sgmii_config(uint8 lchip, uint16 lport)
{
    uint32 value      = 0;
    uint32 value1     = 0;
    uint32 cmd        = 0;
    uint32 tbl_id     = 0;
    uint16 step       = 0;
    uint8  index      = 0;
    uint8  sgmac_idx  = 0;
    uint8  mii_idx    = 0;
    uint8  pcs_idx    = 0;
    uint8 tmp_pcs_idx = 0;
    uint8  speed_mode = 0;
    uint8  hss_idx   = 0;
    uint8  field_id  = 0;
    uint8  inner_serdes = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m mac_cfg;*/
    Sgmac0RxCfg0_m   mac_rx_cfg;
    Sgmac0TxCfg0_m   mac_tx_cfg;
    SharedMiiCfg0_m  mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m  shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    speed_mode       = port_attr->speed_mode;
    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 0, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        value = 2;
    }
    else
    {
        if (internal_mac_idx == 0)
        {
            value = 2;
        }
        else if (internal_mac_idx == 2)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }
    }
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));
    value = 3;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);

    switch(speed_mode)
    {
        case CTC_PORT_SPEED_2G5:
            value = 3;
            value1 = 0;
            break;
        case CTC_PORT_SPEED_1G:
            value = 2;
            value1 = 0;
            break;
        case CTC_PORT_SPEED_100M:
            value = 1;
            value1 = 9;
            break;
        case CTC_PORT_SPEED_10M:
            value = 0;
            value1 = 99;
            break;
        default:
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"error port speed mode: %d \n", speed_mode);
            return CTC_E_INVALID_PARAM;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value1, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value1, &mii_per_cfg);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        if(CTC_PORT_SPEED_2G5 == speed_mode)
        {
            value = 5;
        }
        else
        {
            value = 1;
        }
        value1 = 3;
    }
    else
    {
        value = 1;
        value1 = 0;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value1, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    if((CTC_PORT_SPEED_2G5 == speed_mode) && (15 > mii_idx))
    {
        value = 0x6;
    }
    else
    {
        value = 0x4;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 80;
    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check*/
    ///TODO: Does parallel detect need enable?
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_qsgmii_config(uint8 lchip, uint16 lport, uint8 is_init)
{
    uint32 value            = 0;
    uint32 value_txrep      = 0;
    uint32 value_rxsmp      = 0;
    uint32 cmd              = 0;
    uint32 tbl_id           = 0;
    uint32 slice_id         = 0;
    uint16 step             = 0;
    uint16 chip_port        = 0;
    uint16 serdes_id        = 0;
    uint8  serdes_num       = 0;
    uint8  index            = 0;
    uint8  sgmac_idx        = 0;
    uint8  mii_idx          = 0;
    uint8  pcs_idx          = 0;
    uint8  speed_mode       = 0;
    uint8  tmp_pcs_idx      = 0;
    uint8  hss_idx          = 0;
    uint8  field_id         = 0;
    uint8  inner_serdes     = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m mac_cfg;*/
    Sgmac0TxCfg0_m          mac_tx_cfg;
    Sgmac0RxCfg0_m          mac_rx_cfg;
    SharedMiiCfg0_m         mii_cfg;
    SharedMii0Cfg0_m        mii_per_cfg;
    SharedPcsCfg0_m         shared_pcs_cfg;
    SharedPcsDsfCfg0_m      dsf_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    QsgmiiPcsSoftRst0_m     pcs_rst;
    Hss12GUnitReset0_m      hss_rst;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;
    SharedPcsSerdes0Cfg0_m  shared_pcs_serdes_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    speed_mode       = port_attr->speed_mode;
    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    if(CTC_E_NONE != sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num))
    {
        return CTC_E_INVALID_CONFIG;
    }

    /*set & reset core qsgmii pcs, after serdes rxtx clock bringup done, qsgmii only    20181113 by asic simulation*/
    if((is_init) && (0 == internal_mac_idx))
    {
        if(!SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(serdes_id))
        {
            return CTC_E_INVALID_CONFIG;
        }
        tbl_id   = Hss12GUnitReset0_t + serdes_id/8;
        field_id = Hss12GUnitReset0_resetCoreQsgmiiPcs0_f + serdes_id%8;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_rst));
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_rst));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_rst));
    }

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 0, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    value = 3;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 3;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);

    switch(speed_mode)
    {
        case CTC_PORT_SPEED_10M:
            value = 0;
            value_txrep = 99;
            value_rxsmp = 99;
            break;
        case CTC_PORT_SPEED_100M:
            value = 1;
            value_txrep = 9;
            value_rxsmp = 9;
            break;
        case CTC_PORT_SPEED_1G:
            value = 2;
            value_txrep = 0;
            value_rxsmp = 0;
            break;
        default:
            break;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value_txrep, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value_rxsmp, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    tbl_id = SharedPcsCfg0_t + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + serdes_id%4, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + serdes_id%4, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + serdes_id%4, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + serdes_id%4, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + serdes_id%4, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg per SharedPcsSerdes */
    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx * step + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    value = 80;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /* Toggle PCS Tx and PMA RX reset */
    if (is_init && (0 == internal_mac_idx))
    {
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = 1;   /* reset */
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst_f, &value, &pcs_rst);
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = 0;   /* release */
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst_f, &value, &pcs_rst);
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_usxgmii_config(uint8 lchip, uint16 lport, uint8 is_init)
{
                                      /*USXGMII0_MODE      USXGMII1_MODE         USXGMII2_MODE*/
    uint32 value_tbl_smii[6][3][6] = {{{5,0,0,0,0,1},     {0xff},               {0xff}},  /*10G*/
                                      {{4,1,1,0,0,1},     {0xff},               {4,0,0,0,0,0}},  /*5G*/
                                      {{3,3,3,0,0,1},     {3,0,0,0,0,0},        {3,1,1,0,0,0}},  /*2.5G*/
                                      {{2,9,9,0,0,1},     {2,0x21,0x21,1,1,0}, {2,4,4,0,0,0}},  /*1G*/
                                      {{1,99,99,0,0,1},   {1,24,24,0,0,0},     {1,49,49,0,0,0}},  /*100M*/
                                      {{0,999,999,0,0,1}, {0,249,249,0,0,0},   {0,499,499,0,0,0}}};  /*10M*/
    uint32 value         = 0;
    uint32 value2        = 0;
    uint32 spd_mode      = 0;
    uint32 usx_mode      = 0;
    uint32 cmd           = 0;
    uint32 tbl_id        = 0;
    uint32 field_id      = 0;
    uint16 step          = 0;
    uint8  index         = 0;
    uint8  sgmac_idx     = 0;
    uint8  mii_idx       = 0;
    uint8  pcs_idx       = 0;
    uint8  tmp_pcs_idx   = 0;
    uint8  inner_serdes  = 0;
    uint8  pcs_mode      = 0;
    uint8  speed_mode    = 0;
    uint8  hss_macro_idx = 0;
    uint8  serdes_lane   = 0;
    uint8  mode_valid_flag  = 1;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m mac_cfg;*/
    Sgmac0RxCfg0_m          mac_rx_cfg;
    Sgmac0TxCfg0_m          mac_tx_cfg;
    SharedMiiCfg0_m         mii_cfg;
    SharedMii0Cfg0_m        mii_per_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsCfg0_m         shared_pcs;
    SharedPcsDsfCfg0_m      dsf_cfg;
    SharedPcsSerdes0Cfg0_m  serdes_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    UsxgmiiPcsSoftRst0_m    pcs_rst;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    pcs_mode         = port_attr->pcs_mode;
    speed_mode       = port_attr->speed_mode;
    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 0, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    value = 2;
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 0x11;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    switch(speed_mode)
    {
        case CTC_PORT_SPEED_10G:
            spd_mode = 0;
            break;
        case CTC_PORT_SPEED_5G:
            spd_mode = 1;
            break;
        case CTC_PORT_SPEED_2G5:
            spd_mode = 2;
            break;
        case CTC_PORT_SPEED_1G:
            spd_mode = 3;
            break;
        case CTC_PORT_SPEED_100M:
            spd_mode = 4;
            break;
        case CTC_PORT_SPEED_10M:
            spd_mode = 5;
            break;
        default:
            mode_valid_flag = 0;
            break;
    }

    if(CTC_CHIP_SERDES_USXGMII2_MODE < pcs_mode)
    {
        mode_valid_flag = 0;
    }
    usx_mode = pcs_mode - CTC_CHIP_SERDES_USXGMII0_MODE;

    if((0 == mode_valid_flag) || (0xff == value_tbl_smii[spd_mode][usx_mode][0]))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"error port pcs mode: %d, value_tbl_smii[%d][%d][0] %d\n", pcs_mode, spd_mode, usx_mode, value_tbl_smii[spd_mode][usx_mode][0]);
        return CTC_E_NONE;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &(value_tbl_smii[spd_mode][usx_mode][0]), &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &(value_tbl_smii[spd_mode][usx_mode][1]), &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &(value_tbl_smii[spd_mode][usx_mode][2]), &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &(value_tbl_smii[spd_mode][usx_mode][3]), &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &(value_tbl_smii[spd_mode][usx_mode][4]), &mii_per_cfg);
    value2 = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value2, &mii_per_cfg);
    /*config link filter*/
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tmp_pcs_idx = port_attr->serdes_id / 4;
    inner_serdes = port_attr->serdes_id % 4;
    tbl_id = SharedPcsCfg0_t + tmp_pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + inner_serdes, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + inner_serdes, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + inner_serdes, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + inner_serdes, &value, &shared_pcs);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + inner_serdes, &value, &shared_pcs);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs));

    tbl_id = SharedPcsDsfCfg0_t + tmp_pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + inner_serdes * step + tmp_pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &serdes_cfg));
    value = 80;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &serdes_cfg));

    /* cfg Hss12GMacrocfg */
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_macro_idx = port_attr->serdes_id / 8;
        serdes_lane = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_macro_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * serdes_lane;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &(value_tbl_smii[spd_mode][usx_mode][5]), &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
    tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
    value2 = 1;
    field_id = UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f;
    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value2, &xfi_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

    /* cfg UsxgmiiPcsPortNumCtl and UsxgmiiPcsXfi0Cfg*/
    switch(pcs_mode)
    {
        case CTC_CHIP_SERDES_USXGMII0_MODE:
            value2 = 0;
            break;
        case CTC_CHIP_SERDES_USXGMII1_MODE:
            value2 = 3;
            break;
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            value2 = 1;
            break;
        default:
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"error port pcs mode: %d \n", pcs_mode);
            return CTC_E_INVALID_PARAM;
    }
    tbl_id = UsxgmiiPcsPortNumCtl0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    field_id = UsxgmiiPcsPortNumCtl0_portNumCfg_f;
    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value2, &pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /* Toggle PCS Tx/RX reset */
    if (is_init && (0 == internal_mac_idx))
    {
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        value = 1;  /* reset */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        value = 0;  /* release */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx/4;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xxvg_config(uint8 lchip, uint16 lport)
{
    uint32 value     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 tmp_val   = 0;
    uint16 step      = 0;
    uint8  index     = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m  mac_cfg;
    Sgmac0RxCfg0_m   mac_rx_cfg;
    Sgmac0TxCfg0_m   mac_tx_cfg;
    SharedMiiCfg0_m  mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m  shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* cfg Quad sgmac*/
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));

    /* to support 25/50G mix config inner qm */
    DRV_IOR_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &tmp_val, &mac_cfg);
    value = ~(internal_mac_idx < 2 ? 1 : 2);
    value &= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);

    DRV_IOR_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &tmp_val, &mac_cfg);
    value = ~(internal_mac_idx < 2 ? 1 : 2);
    value &= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));


    /* cfg sgmac Rx/Tx*/
    switch(internal_mac_idx)
    {
        case 0:
            value = 2;
            break;
        case 2:
            value = 1;
            break;
        default:
            value = 0;
            break;
    }
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);

    tmp_val = 0;
    DRV_IOR_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &tmp_val, &mii_cfg);
    value = ~(internal_mac_idx < 2 ? 1 : 2);
    value &= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg Share Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    value = 7;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);

    value = 32;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    if (internal_mac_idx <= 1)
    {
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    }
    else
    {
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_lg_per_sharedmii_cfg(uint8 lchip, uint32 tbl_id, uint8  internal_mac_idx, uint8  mii_idx)
{
    uint32 value      = 0;
    uint32 cmd        = 0;
    uint8  index      = 0;
    SharedMii0Cfg0_m  mii_per_cfg;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    value = 9;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 16;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 17;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_lg_config(uint8 lchip, uint16 lport)
{
    uint32 value     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 tmp_val   = 0;
    uint16 step      = 0;
    uint8  index     = 0;
    uint8  lg_index  = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  cnt       = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m   mac_cfg;
    Sgmac0RxCfg0_m    mac_rx_cfg;
    Sgmac0TxCfg0_m    mac_tx_cfg;
    SharedMiiCfg0_m   mii_cfg;
    SharedPcsCfg0_m   shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if ((internal_mac_idx != 0) && (internal_mac_idx != 2))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not lg mode \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* cfg Quad sgmac*/
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));

    /* to support 25/50G mix config inner qm */
    DRV_IOR_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &tmp_val, &mac_cfg);
    value = internal_mac_idx < 2 ? 5 : 6;
    value |= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);

    DRV_IOR_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &tmp_val, &mac_cfg);
    value = internal_mac_idx < 2 ? 5 : 6;
    value |= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));


    /* cfg sgmac Rx/Tx*/
    if (internal_mac_idx == 0)
    {
        value = 1;
    }
    else
    {
        value = 0;
    }
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);

    DRV_IOR_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &tmp_val, &mii_cfg);
    value = internal_mac_idx < 2 ? 5 : 6;
    value |= tmp_val;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_lg_per_sharedmii_cfg(lchip, tbl_id, internal_mac_idx, mii_idx));
    tbl_id = SharedMii0Cfg0_t + (internal_mac_idx+1)*step + mii_idx;
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_lg_per_sharedmii_cfg(lchip, tbl_id, internal_mac_idx, mii_idx));

    /* cfg SharedPcsCfg */
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 0;
    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    }
    else
    {
        lg_index = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    }

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f + lg_index, &value, &shared_pcs_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    for(cnt = 0; cnt < 2; cnt++)
    {
        if(internal_mac_idx != 0)
        {
            cnt += 2;
        }
        tbl_id = SharedPcsFx0Cfg0_t + cnt * step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_cg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint8 index     = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx   = 0;
    uint8 pcs_idx   = 0;
    uint8 step      = 0;
    uint8 cnt       = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
     /*QuadSgmacCfg0_m  mac_cfg;*/
    Sgmac0RxCfg0_m   mac_rx_cfg;
    Sgmac0TxCfg0_m   mac_tx_cfg;
    SharedMiiCfg0_m  mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m  shared_pcs_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;
    SharedPcsDsfCfg0_m dsf_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;
    SharedPcsFx0Cfg0_m      pcs_fx_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not cg mode \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 2, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    value = 0;
    tbl_id = Sgmac0RxCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    tbl_id = Sgmac0TxCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    tbl_id = SharedMii0Cfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);

    value = 10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 32;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    value = 100;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &shared_pcs_serdes_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    /*additional 100BaseFx config 0*/
    value = 0;
    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    for(cnt = 0; cnt < 4; cnt++)
    {
        tbl_id = SharedPcsFx0Cfg0_t + cnt * step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_100_basefx_config(uint8 lchip, uint16 lport)
{
    uint32 value     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint16 step      = 0;
    uint8  index     = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  internal_mac_idx = 0;
    uint8  tmp_pcs_idx = 0;
    uint8  hss_idx   = 0;
    uint8  field_id  = 0;
    uint8  inner_serdes = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    Sgmac0RxCfg0_m         mac_rx_cfg;
    Sgmac0TxCfg0_m         mac_tx_cfg;
    SharedMiiCfg0_m        mii_cfg;
    SharedMii0Cfg0_m       mii_per_cfg;
    SharedPcsCfg0_m        shared_pcs_cfg;
    SharedPcsFx0Cfg0_m     pcs_fx_cfg;
    SharedPcsSerdes0Cfg0_m pcs_sd_cfg;
    SharedPcsDsfCfg0_m      dsf_cfg;
    UsxgmiiPcsXfi0Cfg0_m    xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    Hss12GMacroCfg0_m       hss_cfg;
    SharedPcsSgmii0Cfg0_m   sgmii_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        return CTC_E_NONE;
    }

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* cfg Quad sgmac*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, 0, sgmac_idx));

    /* cfg sgmac Rx/Tx*/
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx * step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxInputWidth_f, &value, &mac_rx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_rx_cfg));

    step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
    tbl_id = Sgmac0TxCfg0_t + internal_mac_idx * step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));
    value = 3;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxOutputWidth_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx * step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceDecValue0_f, &value, &mii_per_cfg);
    value = 9;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPaceIncValue0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    /*config link filter*/
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f, &value, &mii_per_cfg);
    value = 0x10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* additional integrity config */
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));
    value = 31;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &dsf_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsf_cfg));

    if(12 > port_attr->serdes_id)
    {
        tmp_pcs_idx = port_attr->serdes_id;
        step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
        tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f, &value, &xfi_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

        tbl_id = UsxgmiiPcsPortNumCtl0_t + tmp_pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
        value = 3;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsPortNumCtl0_portNumCfg_f, &value, &pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    }
    step = Hss12GMacroCfg0_cfgHssL1UsxgmiiMode_f - Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f;
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        hss_idx = port_attr->serdes_id / 8;
        inner_serdes = port_attr->serdes_id % 8;
        tbl_id = Hss12GMacroCfg0_t + hss_idx;
        field_id = Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f + step * inner_serdes;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &hss_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hss_cfg));
    }

    /* cfg SharedPcsCfg */
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
    tbl_id = SharedPcsFx0Cfg0_t + internal_mac_idx * step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value, &pcs_fx_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxLinkEnable0_f, &value, &pcs_fx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_fx_cfg));

    step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
    tbl_id = SharedPcsSerdes0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_sd_cfg));
    value = 80;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_rxPopCntCfg0_f, &value, &pcs_sd_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_sd_cfg));

    /*parallelizing check disable*/
    value = 0;
    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_anParallelDetectEn0_f, &value, &sgmii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f, &value, &sgmii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &sgmii_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_pre_config(uint8 lchip, uint16 lport)
{
    uint8 slice_id   = 0;
    uint32 cmd       = 0;
    uint32 tmp_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetRxChannelMap_m chan_map;
    EpeScheduleNetPortToChanRa_m port_to_chan;
    EpeScheduleNetChanToPortRa_m chan_to_port;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        return CTC_E_NONE;
    }
    cmd = DRV_IOR((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->mac_id, cmd, &chan_map);
    tmp_val32 = port_attr->chan_id;
    DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgPortToChanMapping_f, &tmp_val32, &chan_map);
    cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->mac_id, cmd, &chan_map);

    cmd = DRV_IOR((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->chan_id, cmd, &chan_map);
    tmp_val32 = port_attr->mac_id;
    DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgChanToPortMapping_f, &tmp_val32, &chan_map);
    cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->chan_id, cmd, &chan_map);

    /*mapping mac_id and channel_id*/
    tmp_val32 = port_attr->chan_id;
    DRV_IOW_FIELD(lchip, (EpeScheduleNetPortToChanRa_t+slice_id), EpeScheduleNetPortToChanRa_internalChanId_f,
        &tmp_val32, &port_to_chan);
    cmd = DRV_IOW((EpeScheduleNetPortToChanRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->mac_id, cmd, &port_to_chan);

    tmp_val32 = port_attr->mac_id;
    DRV_IOW_FIELD(lchip, (EpeScheduleNetChanToPortRa_t+slice_id), EpeScheduleNetChanToPortRa_physicalPortId_f,
        &tmp_val32, &chan_to_port);
    cmd = DRV_IOW((EpeScheduleNetChanToPortRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, port_attr->chan_id, cmd, &chan_to_port);

    return CTC_E_NONE;
}


/*
 * 3 calls:
 *   1) init
 *   2) dynamic switch
 *   3) user API call
 *  if 1)/2) and Qsgmii/Usxgmii, need do quadreset.
 */
 int32
_sys_tsingma_mac_set_mac_config(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr, uint8 is_init)
{
    uint8 gchip      = 0;
    uint32 gport     = 0;

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_pre_config(lchip, lport));

    switch (port_attr->pcs_mode)
    {
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_sgmii_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_QSGMII_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_qsgmii_config(lchip, lport, is_init));
            break;
        case CTC_CHIP_SERDES_USXGMII0_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_usxgmii_config(lchip, lport, is_init));
            break;
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xaui_dxaui_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_XFI_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xfi_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xlg_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xxvg_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_lg_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_cg_config(lchip, lport));
            break;
        case CTC_CHIP_SERDES_100BASEFX_MODE:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_100_basefx_config(lchip, lport));
            break;
        default:
            break;
    }

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_CUT_THROUGH_EN, TRUE));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_comp_init_config(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr)
{
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint32 cmd       = 0;
    uint32 tmp_val32 = 0;
    uint8  i         = 0;
    QsgmiiPcsCfg0_m qpcs_cfg;

    tmp_val32 = 1;
    for (i = 0; i < 12; i++)
    {
        tbl_id = QsgmiiPcsCfg0_t + i;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qpcs_cfg));
        fld_id = QsgmiiPcsCfg0_reAlignEachEn_f;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &qpcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qpcs_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_init_mac_config(uint8 lchip)
{
    uint8 slice_id = 0;
    uint8 mac_id = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    for (mac_id = 0; mac_id < TM_MAC_TBL_NUM_PER_SLICE; mac_id++)
    {

        lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, mac_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            /* SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not use\n", mac_id); */
            continue;
        }

        CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
        if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
            return CTC_E_NONE;
        }

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_mac_config(lchip, lport, port_attr, TRUE));
    }

    /* complementary init config */
    CTC_ERROR_RETURN(_sys_tsingma_mac_comp_init_config(lchip, lport, port_attr));

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_mac_set_hss28g_tx_reset(uint8 lchip, uint16 lport, uint8 lane_num, uint32 val, uint8 internal_mac_idx, uint8 flag)
{
    uint8 hss_idx  = 0;
    uint8 idx = 0;
    uint8 offset = 0;
    fld_id_t field_id[4] = {0};
    uint32 value[4] = {0};
    Hss28GUnitReset_m hss28g_rst;

    if(!SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport))
    {
        return CTC_E_NONE;
    }
    if(lane_num > 4)
    {
        return CTC_E_INVALID_PARAM;
    }
    hss_idx = lport/60;

    /*reset lane consider LG lane swap*/
    if (1 == flag)
    {
        if (1 == lane_num)
        {
            switch (internal_mac_idx)
            {
            case 0:
                field_id[0] = Hss28GUnitReset_resetHssL2Tx_f;
                break;
            case 1:
                field_id[0] = Hss28GUnitReset_resetHssL1Tx_f;
                break;
            case 2:
                field_id[0] = Hss28GUnitReset_resetHssL0Tx_f;
                break;
            case 3:
                field_id[0] = Hss28GUnitReset_resetHssL3Tx_f;
                break;
            }
            field_id[0] += (hss_idx ? 4:0);
            value[0] = val;
        }
        else if(2 == lane_num)
        {
            switch (internal_mac_idx)
            {
            case 0:
                field_id[0] = Hss28GUnitReset_resetHssL2Tx_f;
                field_id[1] = Hss28GUnitReset_resetHssL1Tx_f;
                break;
            case 2:
                field_id[0] = Hss28GUnitReset_resetHssL0Tx_f;
                field_id[1] = Hss28GUnitReset_resetHssL3Tx_f;
                break;
            }
            field_id[0] += (hss_idx ? 4:0);
            field_id[1] += (hss_idx ? 4:0);
            value[0] = val;
            value[1] = val;
        }
        else if(4 == lane_num)
        {
            field_id[0] = Hss28GUnitReset_resetHssL2Tx_f;
            field_id[1] = Hss28GUnitReset_resetHssL1Tx_f;
            field_id[2] = Hss28GUnitReset_resetHssL0Tx_f;
            field_id[3] = Hss28GUnitReset_resetHssL3Tx_f;
            field_id[0] += (hss_idx ? 4:0);
            field_id[1] += (hss_idx ? 4:0);
            field_id[2] += (hss_idx ? 4:0);
            field_id[3] += (hss_idx ? 4:0);
            value[0] = val;
            value[1] = val;
            value[2] = val;
            value[3] = val;
        }
    }
    else
    {
        offset = (4*hss_idx) + (internal_mac_idx%4);

        for(; idx < lane_num; idx++)
        {
            field_id[idx] = Hss28GUnitReset_resetHssL0Tx_f + idx + offset;
            value[idx] = val;
        }
    }

    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, Hss28GUnitReset_t, lane_num, field_id, value, &hss28g_rst));

    return CTC_E_NONE;
}


int32
sys_tsingma_mac_fec_hss12g_xfi_xlg_cfg(uint8 lchip, uint32 value, uint8 internal_mac_idx, uint8 shared_fec_idx, uint8 pcs_mode, uint8 rx_flag)
{
    uint32 cmd           = 0;
    uint32 tbl_id        = 0;
    uint32 field_id      = 0;
    uint32 field_id_base = 0;
    uint32 step          = 0;
    uint8  port_cnt      = 0;
    SharedPcsXgFecCfg0_m fec_cfg;

    tbl_id = SharedPcsXgFecCfg0_t + shared_fec_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_cfg));

    step = SharedPcsXgFecCfg0_cfgXgFec1TxRst_f - SharedPcsXgFecCfg0_cfgXgFec0TxRst_f;
    field_id_base = rx_flag ? SharedPcsXgFecCfg0_cfgXgFec0RxRst_f : SharedPcsXgFecCfg0_cfgXgFec0TxRst_f;
    if(CTC_CHIP_SERDES_XFI_MODE == pcs_mode)
    {
        field_id = field_id_base + step*internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_cfg);
    }
    else if(CTC_CHIP_SERDES_XLG_MODE == pcs_mode)
    {
        for(; port_cnt < 4; port_cnt++)
        {
            field_id = field_id_base + step*port_cnt;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_cfg);
        }
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_cfg));

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_fec_hss28g_xfi_xlg_cfg(uint8 lchip, uint32 value, uint8 internal_mac_idx, uint8 shared_fec_idx, uint8 pcs_mode, uint8 rx_flag)
{
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint32 step     = 0;
    uint8  port_cnt = 0;
    uint32 field_id_base = 0;
    ResetCtlSharedFec0_m rst_fec;

    tbl_id = ResetCtlSharedFec0_t + (shared_fec_idx - 6);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst_fec));

    step = ResetCtlSharedFec0_cfgSoftRstFecRx1_f - ResetCtlSharedFec0_cfgSoftRstFecRx0_f;
    field_id_base = rx_flag ? ResetCtlSharedFec0_cfgSoftRstFecRx0_f : ResetCtlSharedFec0_cfgSoftRstFecTx0_f;
    if(CTC_CHIP_SERDES_XFI_MODE == pcs_mode)
    {
        field_id = field_id_base + step*internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &rst_fec);
    }
    else if(CTC_CHIP_SERDES_XLG_MODE == pcs_mode)
    {
        for(; port_cnt < 4; port_cnt++)
        {
            field_id = field_id_base + step*port_cnt;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &rst_fec);
        }
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst_fec));

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_en_fec_xfi_xlg_cfg(uint8 lchip, uint32 value, uint8 internal_mac_idx, uint8 shared_fec_idx, uint8 pcs_mode, uint8 pcs_idx, uint8 rx_flag)
{
    if(SYS_MAC_JUDGE_HSS28G_BY_PCS_IDX(pcs_idx))
    {
        CTC_ERROR_RETURN(sys_tsingma_mac_fec_hss28g_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, pcs_mode, rx_flag));
    }
    else
    {
        CTC_ERROR_RETURN(sys_tsingma_mac_fec_hss12g_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, pcs_mode, rx_flag));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_sgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)  /* release reset */
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, 0));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else     /* Assert reset */
    {
        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, 0));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xfi_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint32 fec_en = 0;
    uint8 shared_fec_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* get fec en */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &fec_en));

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);

    if (enable)  /* release reset */
    {
        value = 0;
        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XFI_MODE, pcs_idx, FALSE));
        }
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, port_attr->flag));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        /*if rx rst is on hold, do not set reset 0*/
        if(0 == p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold)
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }

        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XFI_MODE, pcs_idx, TRUE));
        }
    }
    else     /* Assert reset */
    {
        value = 1;
        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XFI_MODE, pcs_idx, TRUE));
        }
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, port_attr->flag));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XFI_MODE, pcs_idx, FALSE));
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xlg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint32 fec_en = 0;
    uint8 shared_fec_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* get fec en */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &fec_en));

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);
    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_tbl_id: %d is not xlg mode \n", mac_tbl_id);
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        value = 0;
        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XLG_MODE, pcs_idx, FALSE));
        }
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstXlgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, port_attr->flag));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        /*if rx rst is on hold, do not set reset 0*/
        if(0 == p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold)
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }

        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XLG_MODE, pcs_idx, TRUE));
        }
    }
    else
    {
        value = 1;
        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XLG_MODE, pcs_idx, TRUE));
        }
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, port_attr->flag));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstXlgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        if (fec_en)
        {
            CTC_ERROR_RETURN(sys_tsingma_mac_en_fec_xfi_xlg_cfg(lchip, value, internal_mac_idx, shared_fec_idx, CTC_CHIP_SERDES_XLG_MODE, pcs_idx, FALSE));
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xxvg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 shared_fec_idx = 0;
    uint32 fec_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ResetCtlSharedFec0_m fec_reset;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);

    /* get fec en */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &fec_en));

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecTx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
            value = 1;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }

        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, port_attr->flag));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        /*if rx rst is on hold, do not set reset 0*/
        if(0 == p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold)
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }

        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecTx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
            value = 0;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
    }
    else
    {
        /* Assert Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecTx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
            value = 1;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }

        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 1, value, internal_mac_idx, port_attr->flag));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 1;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecTx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
            value = 1;
            field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_lg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 lg_index = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 shared_fec_idx = 0;
    uint32 fec_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ResetCtlSharedFec0_m fec_reset;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);

    if ((internal_mac_idx != 0) && (internal_mac_idx != 2))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_tbl_id: %d is not lg mode \n", mac_tbl_id);
        return CTC_E_INVALID_PARAM;
    }

    /* get fec en */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &fec_en));

    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
        }

        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgTx_f, &value, &pcs_rst);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgTx_f, &value, &pcs_rst);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 2, value, internal_mac_idx, port_attr->flag));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRst_f, &value, &pcs_rst);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRstLg_f, &value, &pcs_rst);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        /*if rx rst is on hold, do not set reset 0*/
        if(0 == p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold)
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgRx_f, &value, &pcs_rst);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgRx_f, &value, &pcs_rst);
            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }

        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 0;
                 /*DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);*/
                 /*DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);*/
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 0;
                 /*DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);*/
                 /*DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);*/
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
        }
    }
    else
    {
        /* Assert Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
        }

        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRst_f, &value, &pcs_rst);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgRx_f, &value, &pcs_rst);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRstLg_f, &value, &pcs_rst);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgRx_f, &value, &pcs_rst);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 2, value, internal_mac_idx, port_attr->flag));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgTx_f, &value, &pcs_rst);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgTx_f, &value, &pcs_rst);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_cg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 shared_fec_idx = 0;
    uint32 fec_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ResetCtlSharedFec0_m fec_reset;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_tbl_id: %d is not cg mode \n", mac_tbl_id);
        return CTC_E_INVALID_PARAM;
    }

    /* get fec en */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &fec_en));

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
        /* Release Pcs Tx Soft Reset */
        value = 0;
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstCgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, port_attr->flag));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        /*if rx rst is on hold, do not set reset 0*/
        if(0 == p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold)
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            field_id = SharedPcsSoftRst0_softRstCgRx_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }
        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
    }
    else
    {
        /* Assert Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }

        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = SharedPcsSoftRst0_softRstCgRx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, port_attr->flag));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstCgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx - 6;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx3_f, &value, &fec_reset);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_set_sgmac_xaui_dxaui_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_tbl_id: %d is not xaui mode \n", mac_tbl_id);
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        for(idx = 0; idx < 4; idx++)
        {
            field_id = SharedPcsSoftRst0_softRstPcsTx0_f + idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        }
        field_id = SharedPcsSoftRst0_softRstXlgTx_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, 0));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        for(idx = 0; idx < 4; idx++)
        {
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        }
        field_id = SharedPcsSoftRst0_softRstXlgRx_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else
    {
        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_rxDeskewSoftRst_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        for(idx = 0; idx < 4; idx++)
        {
            field_id = SharedPcsSoftRst0_softRstPcsTx0_f + idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        }
        field_id = SharedPcsSoftRst0_softRstXlgRx_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        CTC_ERROR_RETURN(_sys_tsingma_mac_set_hss28g_tx_reset(lchip, lport, 4, value, internal_mac_idx, 0));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        for(idx = 0; idx < 4; idx++)
        {
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        }
        field_id = SharedPcsSoftRst0_softRstXlgTx_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }


    return CTC_E_NONE;
}

 /*/TODO: DRV_IOCTL need CTC_ERROR_RETURN*/
STATIC int32
_sys_tsingma_mac_set_sgmac_qsgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 field_id  = 0;
    uint8  slice_id  = 0;
    uint16 mac_tbl_id    = 0;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QsgmiiPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else
    {
        /* Assert Pcs Rx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = 0;
        field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        value = 1;
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        value = 0;
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    return CTC_E_NONE;
}

 /*/TODO: DRV_IOCTL need CTC_ERROR_RETURN*/
STATIC int32
_sys_tsingma_mac_set_sgmac_usxgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_tbl_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    UsxgmiiPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);

        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);

        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else
    {
        /* Assert Pcs Rx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        value = 0;
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);

        value = 1;
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = 0;
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);

        value = 1;
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_mac_set_sgmac_100basefx_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value     = 0;
    uint32 value1    = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 field_id  = 0;
    uint8  slice_id;
    uint16 mac_tbl_id;
    uint8  mii_idx   = 0;
    uint8  pcs_idx   = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;
    SharedPcsCfg0_m      shared_pcs_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        return CTC_E_NONE;
    }

    slice_id = port_attr->slice_id;
    mac_tbl_id = port_attr->tbl_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_tbl_id:%d, enable:%d\n", slice_id, mac_tbl_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)  /* release reset */
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Release Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Release Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Release Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else     /* Assert reset */
    {
        /*close pcs fxMode before disable*/
        value1 = 0;
        tbl_id = SharedPcsCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value1, &shared_pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_cfg));

        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Rx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstRx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Sgmac Reset */
        tbl_id = RlmMacCtlReset_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_tbl_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = SharedPcsSoftRst0_softRstPcsTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /*recover pcs fxMode after disable*/
        value1 = 1;
        tbl_id = SharedPcsCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value1, &shared_pcs_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_cfg));
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint8 num = 0;
    uint8 index = 0;
    ctc_chip_serdes_prbs_t prbs;
    uint16 real_serdes = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    sal_memset(&prbs, 0, sizeof(ctc_chip_serdes_prbs_t));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    for (index = 0; index < num; index++)
    {
        GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
        if (!enable &&
            (CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
            && (CTC_CHIP_SERDES_USXGMII0_MODE != port_attr->pcs_mode)
            && (CTC_CHIP_SERDES_USXGMII1_MODE != port_attr->pcs_mode)
            && (CTC_CHIP_SERDES_USXGMII2_MODE != port_attr->pcs_mode))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, TRUE));
            p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 0;
        }
    }

    /* PCS/MII/Sgmac/FEC reset or release */
    if (1 != SDK_WORK_PLATFORM)
    {
        switch (port_attr->pcs_mode)
        {
            case CTC_CHIP_SERDES_SGMII_MODE:
            case CTC_CHIP_SERDES_2DOT5G_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_sgmii_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_QSGMII_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_qsgmii_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_USXGMII0_MODE:
            case CTC_CHIP_SERDES_USXGMII1_MODE:
            case CTC_CHIP_SERDES_USXGMII2_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_usxgmii_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_XAUI_MODE:
            case CTC_CHIP_SERDES_DXAUI_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xaui_dxaui_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_XFI_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xfi_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_XLG_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xlg_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_XXVG_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_xxvg_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_LG_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_lg_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_CG_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_cg_en(lchip, lport, enable));
                break;
            case CTC_CHIP_SERDES_100BASEFX_MODE:
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_sgmac_100basefx_en(lchip, lport, enable));
                break;
            default:
                break;
        }
    }
    p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);

    for (index = 0; index < num; index++)
    {
        GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
        if (enable && (real_serdes < 24))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, FALSE));
        }
    }

    CTC_ERROR_RETURN(sys_usw_qos_set_fc_default_profile(lchip, gport));

    return CTC_E_NONE;
}

/*
SGMII 100BASEFX
*/
STATIC int32
_sys_tsingma_mac_get_sgmii_100basefx_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value = 0;
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /*need not read dbgMiiRxFaultType0 when port less than 10G*/
    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        *p_value = (value)?TRUE:FALSE;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        step = SharedPcsSgmii1Status0_t - SharedPcsSgmii0Status0_t;
        tb_id = SharedPcsSgmii0Status0_t + internal_mac_idx*step + pcs_idx;
        /* read 3 times */
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_codeErrCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        *p_value = value;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_qsgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value = 0;
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /*need not read dbgMiiRxFaultType0 when port less than 10G*/
    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        *p_value = (value)?TRUE:FALSE;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        tb_id = QsgmiiPcsCodeErrCnt0_t + pcs_idx;
        /* read 3 times */
        cmd = DRV_IOR(tb_id, QsgmiiPcsCodeErrCnt0_codeErrCnt0_f+internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        *p_value = value;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_usxgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value[3] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    UsxgmiiPcsXfi0Status0_m pcs_status;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"lport %d not use!\n", lport);
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /*need not read dbgMiiRxFaultType0 when port less than 10G*/
    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        if (unidir_en)
        {
            *p_value = value[0]?TRUE:FALSE;
        }
        else
        {
            cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));
            *p_value = (value[0])?TRUE:FALSE;
        }
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        sal_memset(&pcs_status, 0, sizeof(UsxgmiiPcsXfi0Status0_m));

        step = UsxgmiiPcsXfi1Status0_t - UsxgmiiPcsXfi0Status0_t;
        tb_id = UsxgmiiPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_status));

        DRV_IOR_FIELD(lchip, tb_id, UsxgmiiPcsXfi0Status0_errBlockCnt0_f, &value[0], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, UsxgmiiPcsXfi0Status0_badBerCnt0_f, &value[1], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, UsxgmiiPcsXfi0Status0_hiBer0_f, &value[2], &pcs_status);

        *p_value = (value[0]+value[1]+value[2]);
    }

    return CTC_E_NONE;
}

/*
XAUI DXAUI
*/
STATIC int32
_sys_tsingma_mac_get_xaui_dxaui_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value[4] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsXauiStatus0_m pcs_status;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    if (internal_mac_idx != 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        /*check link fault state*/
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));

        *p_value = (value[0])?(value[1]?FALSE:TRUE):FALSE;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        sal_memset(&pcs_status, 0, sizeof(SharedPcsXauiStatus0_m));
        tb_id = SharedPcsXauiStatus0_t + pcs_idx;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_status));

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt0_f, &value[0], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt1_f, &value[1], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt2_f, &value[2], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt3_f, &value[3], &pcs_status);

        *p_value = (value[0]+value[1]+value[2]+value[3]);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_xfi_xxvg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value[2] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        if (unidir_en)
        {
            *p_value = value[0]?TRUE:FALSE;
        }
        else
        {
            /*check link fault state*/
            cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));
            *p_value = (value[0])?(value[1]?FALSE:TRUE):FALSE;
        }
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        tb_id = SharedPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_badBerCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_hiBer0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        *p_value = value[0];
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_xlg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
     /*uint32 tb_id_sub = 0;*/
    uint32 value[4] = {0};
    uint32 value_sub[4] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsXlgStatus0_m xlg_status;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        if (unidir_en)
        {
            *p_value = value[0]?TRUE:FALSE;
        }
        else
        {
            /*check link fault state*/
            cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));
            *p_value = (value[0])?(value[1]?FALSE:TRUE):FALSE;
        }
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        sal_memset(&xlg_status, 0, sizeof(SharedPcsXlgStatus0_m));

        tb_id = SharedPcsXlgStatus0_t + pcs_idx;
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
         /*tb_id_sub = SharedPcsXfi0Status0_t + pcs_idx;*/

        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xlg_status));

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt0_f, &value[0], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt1_f, &value[1], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt2_f, &value[2], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt3_f, &value[3], &xlg_status);

         /*cmd = DRV_IOR(tb_id_sub, SharedPcsXfi0Status0_errBlockCnt0_f);*/
         /*CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[0]));*/
         /*cmd = DRV_IOR(tb_id_sub+step, SharedPcsXfi0Status0_errBlockCnt0_f);*/
         /*CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[1]));*/
         /*cmd = DRV_IOR(tb_id_sub+step*2, SharedPcsXfi0Status0_errBlockCnt0_f);*/
         /*CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[2]));*/
         /*cmd = DRV_IOR(tb_id_sub+step*3, SharedPcsXfi0Status0_errBlockCnt0_f);*/
         /*CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[3]));*/

        *p_value = (value[0]+value[1]+value[2]+value[3]+value_sub[0]+value_sub[1]+ value_sub[2]+value_sub[3]);
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_mac_get_lg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_id_sub = 0;
    uint32 field_id = 0;
    uint32 value[4] = {0};
    uint32 value_sub[4] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    if ((internal_mac_idx != 0) && (internal_mac_idx != 2))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        if (unidir_en)
        {
            *p_value = value[0]?TRUE:FALSE;
        }
        else
        {
            /*check link fault state*/
            cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));
            *p_value = (value[0])?(value[1]?FALSE:TRUE):FALSE;
        }
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        /*First Lg use xlg status, second Lg use lg status*/
        if (internal_mac_idx == 0)
        {
            tb_id = SharedPcsXlgStatus0_t + pcs_idx;
            field_id = SharedPcsXlgStatus0_bipErrCnt0_f;
        }
        else
        {
            tb_id = SharedPcsLgStatus0_t + pcs_idx;
            field_id = SharedPcsLgStatus0_lgPcs1BipErrCnt0_f;
        }
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        tb_id_sub = SharedPcsXfi0Status0_t + pcs_idx;
        cmd = DRV_IOR(tb_id, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));
        cmd = DRV_IOR(tb_id_sub, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[0]));

        cmd = DRV_IOR(tb_id, field_id+1);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));
        cmd = DRV_IOR(tb_id_sub+step, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[1]));

        cmd = DRV_IOR(tb_id, field_id+2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[2]));
        cmd = DRV_IOR(tb_id_sub+step*2, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[2]));

        cmd = DRV_IOR(tb_id, field_id+3);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[3]));
        cmd = DRV_IOR(tb_id_sub+step*3, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[3]));

        *p_value = (value[0]+value[1]+value[2]+value[3]+value_sub[0]+value_sub[1]+ value_sub[2]+value_sub[3]);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_cg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value[4] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_LINK)
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + internal_mac_idx*step + mii_idx;

        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        if (unidir_en)
        {
            *p_value = value[0]?TRUE:FALSE;
        }
        else
        {
            /*check link fault state*/
            cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));
            *p_value = (value[0])?(value[1]?FALSE:TRUE):FALSE;
        }
    }

    if (type == SYS_PORT_MAC_STATUS_TYPE_CODE_ERR)
    {
        tb_id = SharedPcsXfi0Status0_t + pcs_idx;
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;

        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        cmd = DRV_IOR(tb_id+step, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[1]));

        cmd = DRV_IOR(tb_id+step*2, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[2]));

        cmd = DRV_IOR(tb_id+step*3, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[3]));

        *p_value += (value[0]+value[1]+ value[2]+value[3]);
    }

    return CTC_E_NONE;
}

/*
Set RS FEC config, including XXVG/LG/CG
*/
STATIC int32
_sys_tsingma_mac_set_fec_rs_config(uint8 lchip, uint32 gport)
{
    uint16 lport     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 field_id = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 shared_fec_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 step = 0;
    uint32 value;
    uint32 i;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint8 num = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsFecCfg0_m pcsfec_cfg;
    GlobalCtlSharedFec0_m sharedfec_cfg;
    SharedMii0Cfg0_m mii_per_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X\n", gport);

    /* get info from gport */
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if(SYS_MAC_JUDGE_HSS28G_BY_PCS_IDX(pcs_idx))
    {
        shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mii index: %d pcs index: %d \n", mii_idx, pcs_idx);
        return CTC_E_INVALID_PARAM;
    }

    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
    {
        /* #1.1 cfg SharedMii0Cfg tbl */
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxRsFecEn0_f, &value, &mii_per_cfg);
        value = 0x4fff;
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

        /* #1.2 cfg SharedPcsFecCfg tbl */
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
        }
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 0x13FFF;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx, &value, &sharedfec_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f + internal_mac_idx, &value, &sharedfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        }
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }

    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
        /* #1.1 cfg SharedMii0Cfg tbl */
        step = SharedMii2Cfg0_t - SharedMii0Cfg0_t;  /* step is 2 */
        tbl_id = SharedMii0Cfg0_t + lg_index*step + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
        field_id = SharedMii0Cfg0_cfgMiiTxRsFecEn0_f;  /* for 50G, only config 0/2 */
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_per_cfg);
        field_id = SharedMii0Cfg0_cfgMiiTxAmInterval0_f;
        value = 0x4FFF;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_per_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

        /* #1.2 cfg SharedPcsFecCfg tbl */
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        value = 0;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        }

        value = 1;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
        }
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        if (0 == lg_index)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            value = 0x9FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        }
        else
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            value = 0x9FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
        }

        value = 0;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
        }

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }

    else if(CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
    {
        /* #1.1 cfg SharedMii0Cfg tbl */
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        for (i = 0; i < 4; i++)
        {
            tbl_id = SharedMii0Cfg0_t + i*step + mii_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
            field_id = SharedMii0Cfg0_cfgMiiTxRsFecEn0_f;
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_per_cfg);
            field_id = SharedMii0Cfg0_cfgMiiTxAmInterval0_f;
            value = 0x3fff;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_per_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
        }

        /* #1.2 cfg SharedPcsFecCfg tbl */
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 0x13FFF;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }

    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Rs pcs mode invalid! pcs_mode %d, serdes id %d, mii index %d, pcs index %d\n", port_attr->pcs_mode, port_attr->serdes_id, mii_idx, pcs_idx);
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*
BASE-R FEC common config of xxvg & lg
    SharedMiiCfg:
    SharedMii0Cfg.cfgMiiTxRsFecEn0/1/2/3 = 0
    SharedMii0Cfg.cfgMiiTxAminInterval0/1/2/3 = 16383
*/
int32
sys_tsingma_mac_fec_sharedmii_cfg(uint8 lchip, uint8 internal_mac_idx, uint8 mii_idx)
{
    uint8    step     = 0;
    uint32   tbl_id   = 0;
    uint32   value[2]  = {0, 16383};
    fld_id_t field[2]  = {SharedMii0Cfg0_cfgMiiTxRsFecEn0_f, SharedMii0Cfg0_cfgMiiTxAmInterval0_f};
    SharedMii0Cfg0_m      mii_per_cfg   = {{0}};

    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 2, field, value, &mii_per_cfg));
    return CTC_E_NONE;
}

/*
BASE-R FEC common config of xxvg & lg
*/
int32
sys_tsingma_mac_fec_baser_xxvg_lg_common_cfg(uint8 lchip, uint8 internal_mac_idx, uint8 mii_idx, uint8 pcs_idx, uint8 lg_index, uint8 shared_fec_idx, uint8 lg_flag)
{
     /*uint8  step     = 0;*/
    uint32 tbl_id   = 0;
     /*uint32 field_id = 0;*/
    uint32 cmd      = 0;
    uint32 value    = 0;
    SharedPcsFecCfg0_m    pcsfec_cfg    = {{0}};
    GlobalCtlSharedFec0_m sharedfec_cfg = {{0}};

    /* 1. cfg SharedMii0Cfg tbl */
    CTC_ERROR_RETURN(sys_tsingma_mac_fec_sharedmii_cfg(lchip, internal_mac_idx, mii_idx));

    /* 2. cfg SharedPcsFecCfg tbl */
    tbl_id = SharedPcsFecCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
    if (0 == lg_index)
    {
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
    }
    else
    {
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
    }
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

    /* 3. cfg GlobalCtlSharedFec */
    tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
    if (0 == lg_index)
    {
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
    }
    else
    {
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
    }
    DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f + internal_mac_idx, &value, &sharedfec_cfg);

    value = 0x13FFF;
    if(lg_flag)
    {
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
        }
    }
    else
    {
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx, &value, &sharedfec_cfg);
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    return CTC_E_NONE;
}

/*
Set BASE-R FEC config, including XXVG/LG/XFI/XLG
*/
STATIC int32
_sys_tsingma_mac_set_fec_baser_config(uint8 lchip, uint32 gport)
{
    uint16 lport     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 tbl_id_base = 0;
    uint32 field_id = 0;
    fld_id_t field_id_row[4] = {0};
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 shared_fec_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 step = 0;
    uint32 value;
    uint32 value_row[4];
    uint32 i;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint8 num = 0;
    uint8 hss28g_flag = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsFecCfg0_m pcsfec_cfg;
    GlobalCtlSharedFec0_m sharedfec_cfg;
     /*SharedMii0Cfg0_m mii_per_cfg;*/
    XgFec0CtlSharedFec0_m xgfec_ctl;
    SharedPcsXgFecCfg0_m shared_xgfec;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X\n", gport);

    /* get info from gport */
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    hss28g_flag      = SYS_MAC_JUDGE_HSS28G_BY_PCS_IDX(pcs_idx);
    shared_fec_idx   = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);
    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }
    tbl_id_base = ((pcs_idx == 6) ? XgFec0CtlSharedFec0_t : XgFec0CtlSharedFec1_t);

    if((CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode) && (TRUE == hss28g_flag))
    {
        /* 0. common cfg */
        CTC_ERROR_RETURN(sys_tsingma_mac_fec_baser_xxvg_lg_common_cfg(lchip, internal_mac_idx, mii_idx, pcs_idx, lg_index, shared_fec_idx, FALSE));

        /* 1. cfg SharedPcsFecCfg tbl */
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* 2. cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f + internal_mac_idx, &value, &sharedfec_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        /* 3. cfg XgFec0CtlSharedFec tbl */
        step = XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t; /* 25G only use XgFec0/2/4/6 */
        tbl_id = XgFec0CtlSharedFec0_t + internal_mac_idx*step + shared_fec_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_ctl));
        field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &xgfec_ctl);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_ctl));
    }

    else if((CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) && (TRUE == hss28g_flag))
    {
        /* 0. common cfg */
        CTC_ERROR_RETURN(sys_tsingma_mac_fec_baser_xxvg_lg_common_cfg(lchip, internal_mac_idx, mii_idx, pcs_idx, lg_index, shared_fec_idx, TRUE));

        /* 1. cfg SharedPcsFecCfg tbl */
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        value = 0;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        }

        value = 1;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* 2. cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 0;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
        }

        value = 1;
        if(0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        /* 3. cfg XgFec0CtlSharedFec tbl */
        step = XgFec1CtlSharedFec0_t - XgFec0CtlSharedFec0_t;
        value = 0;
        field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
         /*if (0 == lg_index)*/
         /*{*/
         /*    tbl_id = tbl_id_base + step*internal_mac_idx;*/
         /*}*/
         /*else*/
         /*{*/
         /*    tbl_id = tbl_id_base + step*(internal_mac_idx+4);*/
         /*}*/
        for (i = 0; i < 4; i++)
        {
            tbl_id = tbl_id_base + (i+lg_index*4)*step;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
        }
    }

    else if(CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
    {
        if(hss28g_flag)
        {
            tbl_id = GlobalCtlSharedFec0_t + (pcs_idx-6);
            field_id = GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx;
            value = 0x13fff;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &sharedfec_cfg));
        }
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        field_id = SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx;
        value = 1;
        CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &pcsfec_cfg));
        if(hss28g_flag)
        {
            tbl_id = tbl_id_base + internal_mac_idx*(XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t);
            field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
            value = 0;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
        }
    }

    else if(CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
    {
        if(hss28g_flag)
        {
            tbl_id = GlobalCtlSharedFec0_t + (pcs_idx-6);
            field_id = GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx;
            value = 0x13fff;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &sharedfec_cfg));

            tbl_id = GlobalCtlSharedFec0_t + (pcs_idx-6);
            field_id = GlobalCtlSharedFec0_cfgSharedFec40GPort_f;
            value = 1;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &sharedfec_cfg));
        }
        tbl_id = SharedPcsFecCfg0_t + pcs_idx;
        field_id = SharedPcsFecCfg0_xlgPcsFecEn_f + internal_mac_idx;
        value = 1;
        CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &pcsfec_cfg));
        if(hss28g_flag)
        {
            field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
            value = 0;
            for(i = 0; i < 4; i++)
            {
                tbl_id = tbl_id_base + i*(XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t);
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
            }
        }
        else
        {
            tbl_id = SharedPcsXgFecCfg0_t + pcs_idx;
            step = SharedPcsXgFecCfg0_cfgXgFec1TxWidth_f - SharedPcsXgFecCfg0_cfgXgFec0TxWidth_f;
            sal_memset(value_row, 1, 4*sizeof(uint32));
            for(i = 0; i < 4; i++)
            {
                field_id_row[i] = SharedPcsXgFecCfg0_cfgXgFec0TxWidth_f + step*i;
            }
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 4, field_id_row, value_row, &shared_xgfec));
        }
    }

    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Baser pcs mode invalid! pcs_mode %d, serdes id %d, mii index %d, pcs index %d\n", port_attr->pcs_mode, port_attr->serdes_id, mii_idx, pcs_idx);
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*
Set FEC clear config
*/
STATIC int32
_sys_tsingma_mac_set_fec_clear(uint8 lchip, uint32 gport)
{
    uint16 lport     = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 tbl_id_base = 0;
    uint32 field_id = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 shared_fec_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 step = 0;
    uint32 value;
    uint32 i;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint8 num = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsFecCfg0_m pcsfec_cfg = {{0}};
    GlobalCtlSharedFec0_m sharedfec_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    XgFec0CtlSharedFec0_m xgfec_ctl;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X\n", gport);

    /* get info from gport */
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx);
    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }
    tbl_id_base = ((pcs_idx == 6) ? XgFec0CtlSharedFec0_t : XgFec0CtlSharedFec1_t);

    /* HSS28G FEC CONFIG */
    if(SYS_MAC_JUDGE_HSS28G_BY_PCS_IDX(pcs_idx))
    {
        if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode || CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        {
            step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
            tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxRsFecEn0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

            /* #2.2 cfg SharedPcsFecCfg tbl */
            tbl_id = SharedPcsFecCfg0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
            }
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            /* #2.4 cfg GlobalCtlSharedFec tbl */
            tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
            }
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f + internal_mac_idx, &value, &sharedfec_cfg);
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            }
            value = 0x3fff;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f+internal_mac_idx, &value, &sharedfec_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

            /*XgFec0CtlSharedFec0 set to default value*/
            tbl_id = tbl_id_base + internal_mac_idx*(XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t);
            field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
            value = 1;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
        }
        else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        {
            step = SharedMii2Cfg0_t - SharedMii0Cfg0_t;  /* step is 2 */
            tbl_id = SharedMii0Cfg0_t + lg_index*step + mii_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxRsFecEn0_f, &value, &mii_per_cfg);
            value = 16383;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

            /* #2.2 cfg SharedPcsFecCfg tbl */
            tbl_id = SharedPcsFecCfg0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
            }
            value = 0;
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
            }
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            /* #2.4 cfg GlobalCtlSharedFec tbl */
            tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
            }
            value = 0;
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
            }

            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            }
            value = 0x3fff;
            if(0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
                DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

            /*XgFec0CtlSharedFec0 set to default value*/
            step = XgFec1CtlSharedFec0_t - XgFec0CtlSharedFec0_t;
            value = 1;
            field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
            for (i = 0; i < 4; i++)
            {
                tbl_id = tbl_id_base + (i+lg_index*4)*step;
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
            }
        }
        else if (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode || CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)/* 100G 40G */
        {
            step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
            for (i = 0; i < 4; i++)
            {
                tbl_id = SharedMii0Cfg0_t + i*step + mii_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxRsFecEn0_f, &value, &mii_per_cfg);
                value = 16383;
                DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
            }

            /* #2.2 cfg SharedPcsFecCfg tbl */
            tbl_id = SharedPcsFecCfg0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

            /* #2.4 cfg GlobalCtlSharedFec tbl */
            tbl_id = GlobalCtlSharedFec0_t + (shared_fec_idx-6);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec40GPort_f, &value, &sharedfec_cfg);
            value = 0x3fff;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

            /*XgFec0CtlSharedFec0 set to default value*/
            field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
            value = 1;
            for(i = 0; i < 4; i++)
            {
                tbl_id = tbl_id_base + i*(XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t);
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t*)(&field_id), &value, &xgfec_ctl));
            }
        }
        else
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"28G CLEAR pcs mode invalid! pcs_mode %d, serdes id %d, mii index %d, pcs index %d\n", port_attr->pcs_mode, port_attr->serdes_id, mii_idx, pcs_idx);
            return CTC_E_INVALID_PARAM;
        }
    }

    /* HSS12G FEC CONFIG */
    else
    {
        if(CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        {
            tbl_id = SharedPcsFecCfg0_t + pcs_idx;
            field_id = SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx;
            value = 0;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t *)(&field_id), &value, &pcsfec_cfg));
        }
        else if(CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        {
            tbl_id = SharedPcsFecCfg0_t + pcs_idx;
            field_id = SharedPcsFecCfg0_xlgPcsFecEn_f + internal_mac_idx;
            value = 0;
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, tbl_id, 1, (fld_id_t *)(&field_id), &value, &pcsfec_cfg));
        }
        else
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"12G CLEAR pcs mode invalid! pcs_mode %d, serdes id %d, mii index %d, pcs index %d\n", port_attr->pcs_mode, port_attr->serdes_id, mii_idx, pcs_idx);
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_get_fec_type_capability(uint8 pcs_mode, uint32* p_value)
{
    CTC_PTR_VALID_CHECK(p_value);

    switch(pcs_mode)
    {
        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_LG_MODE:
            *p_value = (1 << CTC_PORT_FEC_TYPE_RS);
            *p_value |= (1 << CTC_PORT_FEC_TYPE_BASER);
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            *p_value = (1 << CTC_PORT_FEC_TYPE_RS);
            break;
        case CTC_CHIP_SERDES_XFI_MODE:
        case CTC_CHIP_SERDES_XLG_MODE:
            *p_value = (1 << CTC_PORT_FEC_TYPE_BASER);
            break;
        default:
            *p_value = (1 << CTC_PORT_FEC_TYPE_NONE);
            break;
    }
    return CTC_E_NONE;
}

/*
Set FEC config.
For TM, the relationships of port mode and FEC mode are as follows:
RS
BASE-R
 +---------+
 |RS|BASE-R|
 +--+------+
 | XXVG    |
 | LG      |
 |CG| XFI  |
 |  | XLG  |
 +---------+
*/
STATIC int32
_sys_tsingma_mac_set_fec_config(uint8 lchip, uint16 lport, ctc_port_fec_type_t type)
{
    uint32 fec_value = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;


    if(CTC_PORT_FEC_TYPE_NONE == type)
    {
        CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_clear(lchip, lport));
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }
    _sys_tsingma_mac_get_fec_type_capability(port_attr->pcs_mode, &fec_value);

    if(!CTC_FLAG_ISSET(fec_value, (1 << type)))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch(type)
    {
        case CTC_PORT_FEC_TYPE_RS:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_clear(lchip, lport));
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_rs_config(lchip, lport));
            break;
        case CTC_PORT_FEC_TYPE_BASER:
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_clear(lchip, lport));
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_baser_config(lchip, lport));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_fec_en(uint8 lchip, uint16 lport, uint32 type)
{
    uint8  gchip      = 0;
    uint32 gport      = 0;
    uint8 mac_toggle_flag = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

    if(p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
    {
        mac_toggle_flag = 1;
        CTC_ERROR_RETURN(_sys_tsingma_mac_set_mac_en(lchip, gport, FALSE));
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_config(lchip, lport, type));

    if(mac_toggle_flag)
    {
        CTC_ERROR_RETURN(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_set_fec_en(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport = 0;
    uint32 curr_fec_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X, fec type:%d\n", gport, value);

    if(value >= CTC_PORT_FEC_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_fec_en(lchip, lport, &curr_fec_en));
    if (curr_fec_en == value)
    {
        /* same config, do nothing */
        return CTC_E_NONE;
    }

    /* get info from gport */
    MAC_LOCK;
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    if((CTC_PORT_FEC_TYPE_BASER == value) && ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                                          && (CTC_CHIP_SERDES_LG_MODE    != port_attr->pcs_mode)
                                          && (CTC_CHIP_SERDES_XFI_MODE   != port_attr->pcs_mode)
                                          && (CTC_CHIP_SERDES_XLG_MODE   != port_attr->pcs_mode)))
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }
    if((CTC_PORT_FEC_TYPE_RS == value)     && ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                                           && (CTC_CHIP_SERDES_LG_MODE   != port_attr->pcs_mode)
                                           && (CTC_CHIP_SERDES_CG_MODE   != port_attr->pcs_mode)))
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_en(lchip, lport, value));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

/*
Get FEC status by global port id
*/
int32
_sys_tsingma_mac_get_fec_en(uint8 lchip, uint32 lport, uint32* p_value)
{
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint8 internal_mac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 step = 0;
    uint32 enable = 0;
    uint32 rs_fec = 0;
    SharedPcsFecCfg0_m  pcscfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_PTR_VALID_CHECK(p_value);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    tbl_id = SharedPcsFecCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcscfg));

    switch(port_attr->pcs_mode)
    {
        case CTC_CHIP_SERDES_XXVG_MODE:
            if (0 == internal_mac_idx % 4)
            enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn0_f, &pcscfg);
            else if (1 == internal_mac_idx % 4)
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn1_f, &pcscfg);
            else if (2 == internal_mac_idx % 4)
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn2_f, &pcscfg);
            else
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn3_f, &pcscfg);
            if(enable)
            {
                step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
                tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
                cmd = DRV_IOR(tbl_id, SharedMii0Cfg0_cfgMiiTxRsFecEn0_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rs_fec));
                if(rs_fec)
                {
                    *p_value = CTC_PORT_FEC_TYPE_RS;
                }
                else
                {
                    *p_value = CTC_PORT_FEC_TYPE_BASER;
                }
            }
            else
            {
                *p_value = CTC_PORT_FEC_TYPE_NONE;
            }
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            if (0 == internal_mac_idx % 4)
            {
                enable = GetSharedPcsFecCfg0(V, lgPcsFecEn0_f, &pcscfg);
                rs_fec = GetSharedPcsFecCfg0(V, lgPcsFecRsMode0_f, &pcscfg);
            }
            else
            {
                enable = GetSharedPcsFecCfg0(V, lgPcsFecEn1_f, &pcscfg);
                rs_fec = GetSharedPcsFecCfg0(V, lgPcsFecRsMode1_f, &pcscfg);
            }
            if(enable)
            {
                if(rs_fec)
                {
                    *p_value = CTC_PORT_FEC_TYPE_RS;
                }
                else
                {
                    *p_value = CTC_PORT_FEC_TYPE_BASER;
                }
            }
            else
            {
                *p_value = CTC_PORT_FEC_TYPE_NONE;
            }
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            enable = GetSharedPcsFecCfg0(V, cgfecEn_f, &pcscfg);
            if(enable)
            {
                *p_value = CTC_PORT_FEC_TYPE_RS;
            }
            else
            {
                *p_value = CTC_PORT_FEC_TYPE_NONE;
            }
            break;
        case CTC_CHIP_SERDES_XFI_MODE:
            if (0 == internal_mac_idx % 4)
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn0_f, &pcscfg);
            else if (1 == internal_mac_idx % 4)
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn1_f, &pcscfg);
            else if (2 == internal_mac_idx % 4)
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn2_f, &pcscfg);
            else
                enable = GetSharedPcsFecCfg0(V, xfiPcsFecEn3_f, &pcscfg);
            if(enable)
            {
                *p_value = CTC_PORT_FEC_TYPE_BASER;
            }
            else
            {
                *p_value = CTC_PORT_FEC_TYPE_NONE;
            }
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            enable = GetSharedPcsFecCfg0(V, xlgPcsFecEn_f, &pcscfg);
            if(enable)
            {
                *p_value = CTC_PORT_FEC_TYPE_BASER;
            }
            else
            {
                *p_value = CTC_PORT_FEC_TYPE_NONE;
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*
 * PCS rx reset function, reset == 1 : do PCS rx reset;
                          reset == 0 : release PCS rx reset
   type -> sys_port_recover_t, if 0, means not care type

   Used by AN CL73 to control link status
 */
STATIC int32
_sys_tsingma_mac_pcs_rx_rst(uint8 lchip, uint16 lport, uint32 type, uint8 reset)
{
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8 slice_id  = 0;
    uint16 chip_port = 0;
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint16 serdes_id = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 shared_fec_idx = 0;
    ctc_chip_serdes_loopback_t lb_param;
    QsgmiiPcsSoftRst0_m   qsgmii_pcs_rst;
    UsxgmiiPcsSoftRst0_m  usxgmii_pcs_rst;
    SharedPcsSoftRst0_m   pcs_rst;
    ResetCtlSharedFec0_m  fec_reset;
    SharedPcsXgFecCfg0_m  xgfec_cfg;

    if (NULL == p_usw_mac_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    sal_memset(&qsgmii_pcs_rst, 0, sizeof(QsgmiiPcsSoftRst0_m));
    sal_memset(&usxgmii_pcs_rst, 0, sizeof(UsxgmiiPcsSoftRst0_m));
    sal_memset(&pcs_rst, 0, sizeof(SharedPcsSoftRst0_m));
    sal_memset(&fec_reset, 0, sizeof(ResetCtlSharedFec0_m));

    sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = (SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport) ? (pcs_idx - 6) : pcs_idx);
    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if (reset)
    {
        p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold = 1;

        if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode))
        {
            /* Assert Fec Rx Soft Reset */
            if((CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode))
            {
                if(SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport))
                {
                    tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                    value = 1;
                    field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                }
                else
                {
                    tbl_id = SharedPcsXgFecCfg0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                    value = 1;
                    field_id = SharedPcsXgFecCfg0_cfgXgFec0RxRst_f +
                               internal_mac_idx*(SharedPcsXgFecCfg0_cfgXgFec1RxRst_f - SharedPcsXgFecCfg0_cfgXgFec0RxRst_f);
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &xgfec_cfg);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                }
            }

            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
            cmd = DRV_IOW(tbl_id, field_id);
            value = 1;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        }
        else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        {
            /* Assert Pcs Rx Soft Reset */
            tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
            /*
              if serdes loopback internal, assert pma soft reset
              if serdes no loopback, but has code error or no random flag, assert pma soft reset
                                     if only not link up, DO NOT assert pma soft reset
                                     The same as USXGMII
              when reset, do ONE IO
              when release, first release PMA, then PCS
             */

            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, NULL));
            sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
            lb_param.serdes_id = serdes_id;
            lb_param.mode = 0; /* 0--LM1 (inner loopback, 12G only) */
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
            if (lb_param.enable) /* serdes_loopback */
            {
                value = 1;
                field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst1_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst2_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst3_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
            }
            else
            {
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type)) /* For TM it is not used currently */
                {
                    value = 1;
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst1_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst2_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst3_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                }
                else
                {
                    value = 1;
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                }
            }
        }
        else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
        {
            /* Assert Pcs Rx Soft Reset */
            tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));

            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, NULL));
            sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
            lb_param.serdes_id = serdes_id;
            lb_param.mode = 0;
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
            if (lb_param.enable) /* serdes_loopback */
            {
                value = 1;
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst1_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst2_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst3_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
            }
            else
            {
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type)) /* For TM it is not used currently */
                {
                    value = 1;
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst1_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst2_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst3_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                }
                else
                {
                    value = 1;
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                }
            }
        }
        else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        {
            /* Assert Fec Rx Soft Reset */
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }

            /* Assert Pcs Rx Soft Reset */
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            value = 1;
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgRx_f, &value, &pcs_rst);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgRx_f, &value, &pcs_rst);
            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }
        else if ((CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
        {
            if(CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            {
                if(SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport))
                {
                    tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                    value = 1;
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                }
                else
                {
                    tbl_id = SharedPcsXgFecCfg0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                    value = 1;
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec0RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec1RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec2RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec3RxRst_f, &value, &xgfec_cfg);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                }

                /* Assert Pcs Rx Soft Reset */
                tbl_id = SharedPcsSoftRst0_t + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
                value = 1;
                field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            }
            /*XAUI DXAUI*/
            else
            {
                tbl_id = SharedPcsSoftRst0_t + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
                value = 1;
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx0_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx1_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx2_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx3_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRst_f, &value, &pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            }
        }
        else if (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        {
            /* Assert Fec Rx Soft Reset */
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            /* Assert Pcs Rx Soft Reset */
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            value = 1;
            field_id = SharedPcsSoftRst0_softRstCgRx_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        }
    }
    else
    {
        if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode))
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
            cmd = DRV_IOW(tbl_id, field_id);
            value = 0;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            /* Rlease Fec Rx Soft Reset */
            if((CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode))
            {
                if(SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport))
                {
                    tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                    value = 0;
                    field_id = ResetCtlSharedFec0_cfgSoftRstFecRx0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &fec_reset);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                }
                else
                {
                    tbl_id = SharedPcsXgFecCfg0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                    value = 0;
                    field_id = SharedPcsXgFecCfg0_cfgXgFec0RxRst_f +
                               internal_mac_idx*(SharedPcsXgFecCfg0_cfgXgFec1RxRst_f - SharedPcsXgFecCfg0_cfgXgFec0RxRst_f);
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &xgfec_cfg);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                }
            }
        }
        else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        {
            /* Release Pcs Rx Soft Reset */
            tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;

            /*
              if serdes loopback internal, assert pma soft reset
              if serdes no loopback, but has code error or no random flag, assert pma soft reset
                                     if only not link up, DO NOT assert pma soft reset
                                     The same as USXGMII
             */

            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, NULL));
            sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
            lb_param.serdes_id = serdes_id;
            lb_param.mode = 0; /* 0--LM1 (inner loopback, 12G only) */
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
            if (lb_param.enable) /* serdes_loopback */
            {
                value = 0;

                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));

                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst1_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst2_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst3_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
            }
            else
            {
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type)) /* For TM it is not used currently */
                {
                    value = 0;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPmaRxSoftRst_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));

                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst1_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst2_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst3_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                }
                else
                {
                    value = 0;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                    field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_pcs_rst));
                }
            }
        }
        else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
        {
            /* Release Pcs Rx Soft Reset */
            tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;

            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, NULL));
            sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
            lb_param.serdes_id = serdes_id;
            lb_param.mode = 0;
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
            if (lb_param.enable) /* serdes_loopback */
            {
                value = 0;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));

                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst1_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst2_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst3_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
            }
            else
            {
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type)) /* For TM it is not used currently */
                {
                    value = 0;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));

                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst1_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst2_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst3_f;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                }
                else
                {
                    value = 0;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                    field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
                    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_pcs_rst);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_pcs_rst));
                }
            }
        }
        else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        {
           /* Release Pcs Rx Soft Reset */
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            value = 0;
            if (0 == lg_index)
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgRx_f, &value, &pcs_rst);
            }
            else
            {
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgRx_f, &value, &pcs_rst);
            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

            /* Rlease Fec Rx Soft Reset */
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
            else
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
            }
        }
        else if ((CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
        {

            if(CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            {
                /* Release Pcs Rx Soft Reset */
                tbl_id = SharedPcsSoftRst0_t + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
                value = 0;
                field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

                if(SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(lport))
                {
                    tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                    value = 0;
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
                    DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
                }
                else
                {
                    tbl_id = SharedPcsXgFecCfg0_t + shared_fec_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                    value = 0;
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec0RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec1RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec2RxRst_f, &value, &xgfec_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsXgFecCfg0_cfgXgFec3RxRst_f, &value, &xgfec_cfg);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_cfg));
                }
            }
            /*XAUI DXAUI*/
            else
            {
                tbl_id = SharedPcsSoftRst0_t + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx0_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx1_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx2_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstPcsRx3_f,   &value, &pcs_rst);
                DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRst_f, &value, &pcs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            }
        }
        else if (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        {
            /* Release Pcs Rx Soft Reset */
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            value = 0;
            field_id = SharedPcsSoftRst0_softRstCgRx_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

            /* Rlease Fec Rx Soft Reset */
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx0_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx1_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx2_f, &value, &fec_reset);
            DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecRx3_f, &value, &fec_reset);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));
        }
        p_usw_mac_master[lchip]->mac_prop[lport].rx_rst_hold = 0;
    }

    return CTC_E_NONE;
}

/* support 802.3ap, Auto-Negotiation for Backplane Ethernet */
int32
_sys_tsingma_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable, uint8 restart_flag)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8  serdes_num = 0;
    uint8  cnt = 0;
    uint32 unidir_en = 0;
    ctc_chip_serdes_loopback_t lb_param;

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Enter %s\n", __FUNCTION__);
    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, cl73 auto enable:0x%X\n", gport, enable);

    /* get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    /* if unidir enable, cannot set AN enable */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_unidir_en(lchip, lport, &unidir_en));
    if(enable && unidir_en)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Gport %u has enable unidir, cannot enable auto-nego! \n", gport);
        return CTC_E_INVALID_CONFIG;
    }

    /* auto-neg already enable, cann't set enable again*/
    if ((enable && p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)
        || ((!enable) && (!p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    /* if serdes loopback enable, cannot set enable */
    for(cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
        lb_param.serdes_id = real_serdes;
        lb_param.mode = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes %d is in loopback internal mode \n", real_serdes);
            return CTC_E_INVALID_CONFIG;
        }
        lb_param.mode = 1;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes %d is in loopback external mode \n", real_serdes);
            return CTC_E_INVALID_CONFIG;
        }
    }

    port_attr->is_first = 0;

    if(enable)
    {
        if(!restart_flag)
        {
            CTC_ERROR_RETURN(_sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 1));
        }
    }
    else
    {
        p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable = enable;
        if(!restart_flag)
        {
            CTC_ERROR_RETURN(_sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 0));
        }

        /* if disable, need do 3ap training disable first */
        CTC_ERROR_RETURN(sys_usw_mac_set_3ap_training_en(lchip, gport, FALSE));
    }

    /* 2. enable 3ap auto-neg */
    for(cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(real_serdes))
        {
            /*Disable DFE before AN-LT. DFE will be enabled after LT if eye height < 90 (done by MCU)*/
            sys_tsingma_datapath_set_dfe_en(lchip, real_serdes, !enable);
        }

        if (enable)
        {
            /*disable PRBS*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, FALSE));
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_auto_neg_en(lchip, real_serdes, enable));
        if ((!enable) && (!restart_flag))
        {
            if (SYS_DATAPATH_SERDES_IS_HSS28G(real_serdes))
            {
                /* enable PRBS */
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, TRUE));
                p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 0;
            }
            else
            {
                if (!p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
                {
                    /* enable PRBS */
                    CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, TRUE));
                }
            }
        }
    }

    if(enable)
    {
        p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable = enable;
    }


    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Exit %s\n", __FUNCTION__);

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_intr_get_table_bitmap(uint8 lchip, sys_datapath_lport_attr_t* port_attr, uint32* table, uint32* bitmap)
{
    uint8  mii_idx = 0;
    uint8  inner_port_id = 0;
    uint32 tmp_table = 0;
    uint32 link_bitmap = 0;

    mii_idx          = port_attr->mii_idx;
    inner_port_id = port_attr->internal_mac_idx;

    /*HSS12G0*/
    if(8 > mii_idx)
    {
        CTC_BMP_SET(&link_bitmap, (4 * mii_idx + inner_port_id));
        tmp_table = Hss12GUnitWrapperInterruptFunc0_t;
    }
    /*HSS12G1*/
    else if(13 > mii_idx)
    {
        if(12 == mii_idx)
        {
            CTC_BMP_SET(&link_bitmap, (4 * 4 + inner_port_id));
        }
        else
        {
            CTC_BMP_SET(&link_bitmap, (4 * (mii_idx - 8) + inner_port_id));
        }
        tmp_table = Hss12GUnitWrapperInterruptFunc1_t;
    }
    /*HSS12G2*/
    else if(15 > mii_idx)
    {
        CTC_BMP_SET(&link_bitmap, (4 * (mii_idx - 13) + inner_port_id));
        tmp_table = Hss12GUnitWrapperInterruptFunc2_t;
    }
    /*HSS28G*/
    else
    {
        CTC_BMP_SET(&link_bitmap, (4 * (mii_idx - 15) + inner_port_id));
        tmp_table = Hss28GUnitWrapperInterruptFunc_t;
    }

    if(bitmap)
    {
        (*bitmap) |= link_bitmap;
    }
    if(table)
    {
        (*table)  |= tmp_table;
    }

    return CTC_E_NONE;
}


int32
_sys_tsingma_mac_get_cl73_capability(uint8 lchip, uint32 gport, uint32* p_capability)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if(SYS_DATAPATH_MAC_IN_HSS28G(lchip, lport))
    {
        switch(port_attr->pcs_mode)
        {
            case CTC_CHIP_SERDES_XXVG_MODE:
            case CTC_CHIP_SERDES_XFI_MODE:
                *p_capability = (CTC_PORT_CL73_10GBASE_KR |
                                 CTC_PORT_CL73_25GBASE_KRS |
                                 CTC_PORT_CL73_25GBASE_CRS |
                                 CTC_PORT_CL73_25GBASE_KR |
                                 CTC_PORT_CL73_25GBASE_CR |
                                 CTC_PORT_CL73_25G_RS_FEC_REQUESTED |
                                 CTC_PORT_CL73_25G_BASER_FEC_REQUESTED |
                                 CTC_PORT_CSTM_25GBASE_KR1 |
                                 CTC_PORT_CSTM_25GBASE_CR1 |
                                 CTC_PORT_CSTM_RS_FEC_ABILITY |
                                 CTC_PORT_CSTM_BASER_FEC_ABILITY |
                                 CTC_PORT_CSTM_RS_FEC_REQUESTED |
                                 CTC_PORT_CSTM_BASER_FEC_REQUESTED |
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            case CTC_CHIP_SERDES_LG_MODE:
                *p_capability = (CTC_PORT_CSTM_50GBASE_KR2 |
                                 CTC_PORT_CSTM_50GBASE_CR2 |
                                 CTC_PORT_CSTM_RS_FEC_ABILITY |
                                 CTC_PORT_CSTM_BASER_FEC_ABILITY |
                                 CTC_PORT_CSTM_RS_FEC_REQUESTED |
                                 CTC_PORT_CSTM_BASER_FEC_REQUESTED |
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            case CTC_CHIP_SERDES_CG_MODE:
            case CTC_CHIP_SERDES_XLG_MODE:
                *p_capability = (CTC_PORT_CL73_40GBASE_KR4 |
                                 CTC_PORT_CL73_40GBASE_CR4 |
                                 CTC_PORT_CL73_100GBASE_KR4 |
                                 CTC_PORT_CL73_100GBASE_CR4 |
                                 CTC_PORT_CL73_25G_RS_FEC_REQUESTED|
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            default:
                *p_capability = 0;
                break;
        }
    }
    else
    {
        switch(port_attr->pcs_mode)
        {
            case CTC_CHIP_SERDES_XFI_MODE:
                *p_capability = (CTC_PORT_CL73_10GBASE_KR|
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            case CTC_CHIP_SERDES_XLG_MODE:
                *p_capability = (CTC_PORT_CL73_40GBASE_KR4 |
                                 CTC_PORT_CL73_40GBASE_CR4|
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            default:
                *p_capability = 0;
                break;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_mutex_flag_lock(uint8 lchip, uint16 lport)
{
    uint32 mutex_time = 100;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(NULL == port_attr)
    {
        return CTC_E_INVALID_PTR;
    }

    while((TRUE == port_attr->mutex_flag) && (0 != mutex_time))
    {
        mutex_time--;
        sal_task_sleep(10);
    }
    if(0 == mutex_time)
    {
        return CTC_E_HW_TIME_OUT;
    }

    port_attr->mutex_flag = TRUE;
    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_mutex_flag_unlock(uint8 lchip, uint16 lport)
{
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(NULL == port_attr)
    {
        return CTC_E_INVALID_PTR;
    }

    port_attr->mutex_flag = FALSE;
    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable)
{
    uint8 internal_mac_idx = 0;
    uint8  step       = 0;
    uint32 tbl_id     = 0;
    uint8  mii_idx    = 0;
    uint8  pcs_idx    = 0;
    uint32 cmd        = 0;
    uint32 value      = 0;
    uint32 curr_unidir = 0;
    /* SYSTEM MODIFIED by tongzb for bug 53301, do not check unidir in sdk */
    //uint32 an_en_stat = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedMii0Cfg0_m   mii_cfg;
    uint16 lport = 0;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 num = 0;
    uint8 index = 0;
    uint8 prbs_en = 0;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_UNIDIR_EN, enable));
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

#if 0  /* SYSTEM MODIFIED by tongzb for bug 53301, do not check unidir in sdk */
    /*if auto-neg enable, cannot set unidir enable*/
    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        CTC_ERROR_RETURN(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &an_en_stat));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_mac_get_cl73_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &an_en_stat));
    }
    if(enable && an_en_stat)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Gport %u has enable auto-nego, cannot enable unidir! \n", gport);
        return CTC_E_INVALID_CONFIG;
    }
#endif
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    value  = enable?1:0;

    CTC_ERROR_RETURN(_sys_tsingma_mac_get_unidir_en(lchip, lport, &curr_unidir));
    if (curr_unidir == value)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_mutex_flag_lock(lchip, lport));

    if (SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) && (!enable))
    {
        for (index = 0; index < num; index++)
        {
            GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(sys_tsingma_datapath_get_serdes_prbs31_en(lchip, real_serdes, &prbs_en), lport);
            if (!prbs_en)
            {
                /*disable prbs*/
                CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, TRUE), lport);
                p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 0;
            }
        }
    }

    if(CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode)
    {
        step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
        tbl_id = SharedPcsFx0Cfg0_t + step*internal_mac_idx + pcs_idx;
        cmd = DRV_IOW(tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f);
        CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &value), lport);
    }
    else if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsCfg1_t - SharedPcsCfg0_t;
        tbl_id = SharedPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOW(tbl_id, SharedPcsCfg0_unidirectionEn0_f+internal_mac_idx);
        CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &value), lport);
    }
    else if(CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        step = QsgmiiPcsCfg1_t - QsgmiiPcsCfg0_t;
        tbl_id = QsgmiiPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOW(tbl_id, QsgmiiPcsCfg0_unidirectionEn0_f+internal_mac_idx);
        CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &value), lport);
    }
    else if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode))
    {
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &mii_cfg), lport);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &mii_cfg), lport);
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) && (enable))
    {
        for (index = 0; index < num; index++)
        {
            GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(sys_tsingma_datapath_get_serdes_prbs31_en(lchip, real_serdes, &prbs_en), lport);
            if (prbs_en)
            {
                /*sm reset*/
                sys_tsingma_datapath_set_serdes_sm_reset(lchip, real_serdes);
                /*disable prbs*/
                CTC_ERROR_RETURN_LPORT_MUTEX_UNLOCK(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, FALSE), lport);
            }
        }
    }

    p_usw_mac_master[lchip]->mac_prop[lport].unidir_en = (enable?TRUE:FALSE);

    CTC_ERROR_RETURN(_sys_tsingma_mac_mutex_flag_unlock(lchip, lport));

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_get_unidir_en(uint8 lchip, uint16 lport, uint32* p_value)
{
    uint8 internal_mac_idx = 0;
    uint8  step       = 0;
    uint32 tbl_id     = 0;
    uint8  pcs_idx    = 0;
    uint32 cmd        = 0;
    uint32 value1      = 0;
    SharedPcsCfg0_m pcs_cfg;
    QsgmiiPcsCfg0_m q_pcs_cfg;
    SharedPcsFx0Cfg0_m fx_cfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&pcs_cfg, 0, sizeof(SharedPcsCfg0_m));
    sal_memset(&fx_cfg, 0, sizeof(SharedPcsFx0Cfg0_m));
    sal_memset(&q_pcs_cfg, 0, sizeof(QsgmiiPcsCfg0_m));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_UNIDIR_EN, p_value));
        return CTC_E_NONE;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if(CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode)
    {
        step = SharedPcsFx1Cfg0_t - SharedPcsFx0Cfg0_t;
        tbl_id = SharedPcsFx0Cfg0_t + step*internal_mac_idx + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fx_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, SharedPcsFx0Cfg0_cfgFxTxForceLinkStatus0_f, &value1, &fx_cfg);
        if (value1)
        {
            *p_value = TRUE;
        }
        else
        {
            *p_value = FALSE;
        }
    }
    else if(CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        step = QsgmiiPcsCfg1_t - QsgmiiPcsCfg0_t;
        tbl_id = QsgmiiPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &q_pcs_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, QsgmiiPcsCfg0_unidirectionEn0_f+internal_mac_idx, &value1, &q_pcs_cfg);
        if (value1)
        {
            *p_value = TRUE;
        }
        else
        {
            *p_value = FALSE;
        }
    }
    else if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsCfg1_t - SharedPcsCfg0_t;
        tbl_id = SharedPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f+internal_mac_idx, &value1, &pcs_cfg);
        if (value1)
        {
            *p_value = TRUE;
        }
        else
        {
            *p_value = FALSE;
        }
    }
    else if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode))
    {
        *p_value = p_usw_mac_master[lchip]->mac_prop[lport].unidir_en;
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_get_code_err(uint8 lchip, uint32 gport, uint32* code_err)
{
    uint32 unidir_en = 0;
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    int32 ret = 0;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    *code_err = 0;
    _sys_tsingma_mac_get_unidir_en(lchip, lport, &unidir_en);
    if (unidir_en)
    {
        *code_err = 0;
        return CTC_E_NONE;
    }

    switch (port_attr->pcs_mode)
    {
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
        case CTC_CHIP_SERDES_100BASEFX_MODE:
            ret = _sys_tsingma_mac_get_sgmii_100basefx_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_QSGMII_MODE:
            ret = _sys_tsingma_mac_get_qsgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_USXGMII0_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            ret = _sys_tsingma_mac_get_usxgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
            ret = _sys_tsingma_mac_get_xaui_dxaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_XFI_MODE:
            ret = _sys_tsingma_mac_get_xfi_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XLG_MODE:
            ret = _sys_tsingma_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_LG_MODE:
            ret = _sys_tsingma_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_CG_MODE:
            ret = _sys_tsingma_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        default:
            break;
    }
    return ret;
}

int32
_sys_tsingma_mac_set_rx_pause_type(uint8 lchip, uint32 gport, uint32 pasue_type)
{
    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_get_rx_pause_type(uint8 lchip, uint32 gport, uint32* p_pasue_type)
{
    return CTC_E_NONE;
}

int32
sys_tsingma_mac_get_mode_by_port_if(ctc_port_if_mode_t* if_mode, uint8* mode)
{
    switch (if_mode->interface_type)
    {
    case CTC_PORT_IF_SGMII:
        *mode = CTC_CHIP_SERDES_SGMII_MODE;
        break;
    case CTC_PORT_IF_2500X:
        *mode = CTC_CHIP_SERDES_2DOT5G_MODE;
        break;
    case CTC_PORT_IF_QSGMII:
        *mode = CTC_CHIP_SERDES_QSGMII_MODE;
        break;
    case CTC_PORT_IF_USXGMII_S:
        *mode = CTC_CHIP_SERDES_USXGMII0_MODE;
        break;
    case CTC_PORT_IF_USXGMII_M2G5:
        *mode = CTC_CHIP_SERDES_USXGMII1_MODE;
        break;
    case CTC_PORT_IF_USXGMII_M5G:
        *mode = CTC_CHIP_SERDES_USXGMII2_MODE;
        break;
    case CTC_PORT_IF_XAUI:
        *mode = CTC_CHIP_SERDES_XAUI_MODE;
        break;
    case CTC_PORT_IF_DXAUI:
        *mode = CTC_CHIP_SERDES_DXAUI_MODE;
        break;
    case CTC_PORT_IF_XFI:
        *mode = CTC_CHIP_SERDES_XFI_MODE;
        break;
    case CTC_PORT_IF_KR:
    case CTC_PORT_IF_CR:
        if (CTC_PORT_SPEED_25G == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_XXVG_MODE;
        }
        else if (CTC_PORT_SPEED_10G == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_XFI_MODE;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_PORT_IF_KR2:
    case CTC_PORT_IF_CR2:
        if (CTC_PORT_SPEED_50G == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_LG_MODE;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_PORT_IF_KR4:
    case CTC_PORT_IF_CR4:
        if (CTC_PORT_SPEED_40G == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_XLG_MODE;
        }
        else if (CTC_PORT_SPEED_100G == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_CG_MODE;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_PORT_IF_FX:
        if (CTC_PORT_SPEED_100M == if_mode->speed)
        {
            *mode = CTC_CHIP_SERDES_100BASEFX_MODE;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_PORT_IF_NONE:
    default:
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
sys_tsingma_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    uint32 i = 0;
    uint16 lport      = 0;
    uint8 gchip = 0;
    uint8 speed = 0;
    uint8 interface_type = 0;
    uint8 mode = 0;
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(if_mode);

    sal_memset(&ds_attr, 0, sizeof(sys_datapath_dynamic_switch_attr_t));
    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_tsingma_mac_get_mode_by_port_if(if_mode, &mode));

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    if (port_attr->xpipe_en)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    /* check if no change */
    if(mode == port_attr->pcs_mode)
    {
        MAC_UNLOCK;
        return CTC_E_NONE;
    }

    speed = if_mode->speed;
    interface_type = if_mode->interface_type;

    if (((CTC_PORT_SPEED_1G == speed) && (CTC_PORT_IF_SGMII != interface_type) && (CTC_PORT_IF_QSGMII != interface_type))
        || ((CTC_PORT_SPEED_2G5 == speed) && (CTC_PORT_IF_2500X != interface_type) && (CTC_PORT_IF_USXGMII_M2G5 != interface_type))
        || ((CTC_PORT_SPEED_5G == speed) && (CTC_PORT_IF_USXGMII_M5G != interface_type))
        || ((CTC_PORT_SPEED_10G == speed) && (CTC_PORT_IF_USXGMII_S != interface_type) \
            && (CTC_PORT_IF_XAUI != interface_type) && (CTC_PORT_IF_XFI != interface_type) \
            && (CTC_PORT_IF_KR != interface_type) && (CTC_PORT_IF_CR != interface_type))
        || ((CTC_PORT_SPEED_20G == speed) && (CTC_PORT_IF_DXAUI != interface_type))
        || ((CTC_PORT_SPEED_25G == speed) && (CTC_PORT_IF_KR != interface_type) && (CTC_PORT_IF_CR != interface_type))
        || ((CTC_PORT_SPEED_40G == speed) && (CTC_PORT_IF_KR4 != interface_type) && (CTC_PORT_IF_CR4 != interface_type))
        || ((CTC_PORT_SPEED_50G == speed) && (CTC_PORT_IF_KR2 != interface_type) && (CTC_PORT_IF_CR2 != interface_type))
        || ((CTC_PORT_SPEED_100G == speed) && (CTC_PORT_IF_KR4 != interface_type) && (CTC_PORT_IF_CR4 != interface_type))
        || ((CTC_PORT_SPEED_100M == speed) && (CTC_PORT_IF_FX != interface_type)))
    {
        MAC_UNLOCK;
        return CTC_E_PARAM_CONFLICT;
    }

    /* collect all serdes/mac info associated with Dynamic switch process */
    if(((SYS_DATAPATH_IS_SERDES_1_PORT_1(port_attr->pcs_mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(mode)) ||
         (SYS_DATAPATH_IS_SERDES_1_PORT_1(mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(port_attr->pcs_mode))) &&
       (0 != (port_attr->serdes_id % 4)))
    {}
    else
    {
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_dynamic_switch_get_info(lchip, port_attr, mode, &ds_attr, FALSE, 0xff));
    }

    /* check mac enable cfg */
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_get_gchip_id(lchip, &gchip));

    /* check CL73 AN enable cfg */
    for (i = 0; i < ds_attr.mac_num; i++)
    {
        if (1 != ds_attr.m[i].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr.m[i].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                continue;
            }
            tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_cl73_auto_neg_en(lchip, tmp_gport, FALSE, FALSE));
        }
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, if_mode));

    MAC_UNLOCK;
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_mac_set_speed(uint8 lchip, uint32 gport, ctc_port_speed_t speed_mode)
{
    uint16 lport      = 0;
    uint32 auto_neg_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &auto_neg_en));
    if ((CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport)) && (0 == auto_neg_en))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_SPEED, speed_mode));
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    if ( ((port_attr->pcs_mode == CTC_CHIP_SERDES_SGMII_MODE) && (speed_mode == CTC_PORT_SPEED_1G || speed_mode == CTC_PORT_SPEED_100M || speed_mode == CTC_PORT_SPEED_10M))
        ||((port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE) && (speed_mode == CTC_PORT_SPEED_1G || speed_mode == CTC_PORT_SPEED_100M || speed_mode == CTC_PORT_SPEED_10M))
        ||((port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE) && (speed_mode == CTC_PORT_SPEED_10G || speed_mode == CTC_PORT_SPEED_5G || speed_mode == CTC_PORT_SPEED_2G5 || speed_mode == CTC_PORT_SPEED_1G || speed_mode == CTC_PORT_SPEED_100M || speed_mode == CTC_PORT_SPEED_10M))
        ||((port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE) && (speed_mode == CTC_PORT_SPEED_2G5 || speed_mode == CTC_PORT_SPEED_1G || speed_mode == CTC_PORT_SPEED_100M || speed_mode == CTC_PORT_SPEED_10M))
        ||((port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE) && (speed_mode == CTC_PORT_SPEED_5G || speed_mode == CTC_PORT_SPEED_2G5 || speed_mode == CTC_PORT_SPEED_1G || speed_mode == CTC_PORT_SPEED_100M || speed_mode == CTC_PORT_SPEED_10M)) )
    {
        if (port_attr->speed_mode == speed_mode)
        {
            MAC_UNLOCK;
            return CTC_E_NONE;
        }

        /* if mac enable: first mac disable, then mac enable again */
        if (p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
        {
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, FALSE));
            port_attr->speed_mode = speed_mode;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_config(lchip, lport, port_attr, FALSE));
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
        }
        else
        {
            port_attr->speed_mode = speed_mode;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_config(lchip, lport, port_attr, FALSE));
        }
    }
    else
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " PCS mode %u DO NOT support to config mac speed %u\n", port_attr->pcs_mode, speed_mode);
        return CTC_E_INVALID_PARAM;
    }
    MAC_UNLOCK;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_get_speed(uint8 lchip, uint32 gport, ctc_port_speed_t* p_speed_mode)
{
    uint16 lport      = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 value = 0;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_SPEED, &value));
        *p_speed_mode = value;
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;

    }

    *p_speed_mode = port_attr->speed_mode;

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_cl37_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable)
{
    uint16 lport      = 0;
    uint16 step       = 0;
    uint32 cmd        = 0;
    uint32 tbl_id     = 0;
    uint32 field_id   = 0;
    uint32 curr_en    = 0;
    uint32 value      = 0;
    uint8  pcs_idx    = 0;
    uint8  internal_mac_idx = 0;
    uint32 unidir_en  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSgmii0Cfg0_m sgmii_cfg;
    QsgmiiPcsAneg0Cfg0_m  qsgmii_cfg;
    UsxgmiiPcsAneg0Cfg0_m usxgmii_cfg;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &curr_en));
    if (curr_en == enable)
    {
        /* No change */
        return CTC_E_NONE;
    }

    /* if unidir enable, cannot set AN enable */
    CTC_ERROR_RETURN(_sys_tsingma_mac_get_unidir_en(lchip, lport, &unidir_en));
    if(enable && unidir_en)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Gport %u has enable unidir, cannot enable auto-nego! \n", gport);
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(port_attr == NULL)
    {
        return CTC_E_INVALID_PTR;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    value = enable ? 1 : 0;
    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
        tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
        field_id = SharedPcsSgmii0Cfg0_anEnable0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &sgmii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
    }
    else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        step = QsgmiiPcsAneg1Cfg0_t - QsgmiiPcsAneg0Cfg0_t;
        tbl_id = QsgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
        field_id = QsgmiiPcsAneg0Cfg0_anEnable0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
    }
    else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||
             (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        step = UsxgmiiPcsAneg1Cfg0_t - UsxgmiiPcsAneg0Cfg0_t;
        tbl_id = UsxgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
        field_id = UsxgmiiPcsAneg0Cfg0_anEnable0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_set_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32 value)
{
    uint32 cmd        = 0;
    uint32 tbl_id     = 0;
    uint32 field_id   = 0;
    uint16 step       = 0;
    uint32 mode_value = 0;
    uint16 lport      = 0;
    uint32 curr_val   = 0;
    uint8  pcs_idx    = 0;
    uint8  internal_mac_idx = 0;
    SharedPcsSgmii0Cfg0_m sgmii_cfg;
    QsgmiiPcsAneg0Cfg0_m  qsgmii_cfg;
    UsxgmiiPcsAneg0Cfg0_m usxgmii_cfg;
    SharedPcsCfg0_m       pcs_cfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_INIT_CHECK();

    sal_memset(&sgmii_cfg, 0, sizeof(SharedPcsSgmii0Cfg0_m));
    sal_memset(&qsgmii_cfg, 0, sizeof(QsgmiiPcsAneg0Cfg0_m));
    sal_memset(&usxgmii_cfg, 0, sizeof(UsxgmiiPcsAneg0Cfg0_m));
    sal_memset(&pcs_cfg, 0, sizeof(SharedPcsCfg0_m));

    if (CTC_PORT_PROP_AUTO_NEG_EN == type)
    {
        CTC_ERROR_RETURN(sys_usw_mac_get_auto_neg(lchip, gport, type, &curr_val));
        if ((curr_val && value) || ((!curr_val)&&(!value)))
        {
            /* No change */
            return CTC_E_NONE;
        }
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, value));
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if ((port_attr->port_type != SYS_DMPS_NETWORK_PORT) ||
        (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    /* SGMII/QSGMII/USXGMII mode auto neg */
    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        /* Cfg auto neg enable/disable */
        if(CTC_PORT_PROP_AUTO_NEG_EN == type)
        {
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_cl37_auto_neg_en(lchip, gport, value));
        }
        /* Cfg auto MODE */
        else if(CTC_PORT_PROP_AUTO_NEG_MODE == type)
        {
            /* sdk only support two mode, 1000Base-X(2'b00), SGMII-Slaver(2'b10) */
            if(CTC_PORT_AUTO_NEG_MODE_1000BASE_X == value)
            {
                mode_value = 0;
            }
            else if(CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER == value)
            {
                mode_value = 2;
            }

            if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
            {
                step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
                tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
                field_id = SharedPcsSgmii0Cfg0_anegMode0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &mode_value, &sgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
            }
            else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
            {
                step = QsgmiiPcsAneg1Cfg0_t - QsgmiiPcsAneg0Cfg0_t;
                tbl_id = QsgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
                field_id = QsgmiiPcsAneg0Cfg0_anegMode0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &mode_value, &qsgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
            }
            else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
            {
                if ((CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER == value) || (CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER == value))
                {
                    MAC_UNLOCK;
                    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Unsupported Auto-Neg mode.\n");
                    return CTC_E_INVALID_PARAM;
                }
                step = UsxgmiiPcsAneg1Cfg0_t - UsxgmiiPcsAneg0Cfg0_t;
                tbl_id = UsxgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
                field_id = UsxgmiiPcsAneg0Cfg0_anegMode0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &mode_value, &usxgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
            }
        }
        else if (CTC_PORT_PROP_AUTO_NEG_FEC == type)
        {
            MAC_UNLOCK;
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " FEC not supported\n");
            return CTC_E_INVALID_PARAM;
        }
    }
    /* 802.3 cl73 auto neg */
    else if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode))
    {
        /* Cfg auto neg enable/disable */
        if (CTC_PORT_PROP_AUTO_NEG_EN == type)
        {
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_cl73_auto_neg_en(lchip, gport, value, FALSE));
        }
        else if (CTC_PORT_PROP_AUTO_NEG_FEC == type)
        {
            port_attr->an_fec = 0;
            if (CTC_PORT_FEC_TYPE_NONE != value)
            {
                if (value & (1 << CTC_PORT_FEC_TYPE_RS))
                {
                    port_attr->an_fec |= (1 << CTC_PORT_FEC_TYPE_RS);
                }
                if (value & (1 << CTC_PORT_FEC_TYPE_BASER))
                {
                    port_attr->an_fec |= (1 << CTC_PORT_FEC_TYPE_BASER);
                }
            }
        }
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"port 0x%x mode %d do not support Auto-Nego \n", gport, port_attr->pcs_mode);
        MAC_UNLOCK;
        return CTC_E_INVALID_CONFIG;
    }

    MAC_UNLOCK;

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_mac_set_eee_en(uint8 lchip, uint32 gport, bool enable)
{
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
    return CTC_E_NOT_SUPPORT;
}

int32
sys_tsingma_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    /* if mac already is enabled, no need to do enable again */
    if ((!sys_usw_chip_get_reset_hw_en(lchip)) && ((p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en && enable)
        || (!(p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en) && !enable)))
    {
        MAC_UNLOCK;
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, enable));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

static int32
_sys_tsingma_mac_get_mac_signal_detect(uint8 lchip,
                                                    uint16 lport,
                                                    uint8 mac_id,
                                                    uint8 lg_flag,
                                                    uint8 pcs_mode,
                                                    uint32* p_is_detect)
{
    uint8  cnt          = 0;
    uint8  slice_id     = 0;
    uint8  serdes_num   = 0;
    uint16 chip_port    = 0;
    uint16 serdes_id    = 0;
    uint16 real_serdes  = 0;
    uint32 tmp_val32[4] = {0};

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));
    for (cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(mac_id, serdes_id, cnt, lg_flag, pcs_mode, real_serdes);
        tmp_val32[cnt] = sys_tsingma_datapath_get_serdes_sigdet(lchip, real_serdes);
    }

    *p_is_detect = 1;
    for (cnt = 0; cnt < serdes_num; cnt++)
    {
        if (tmp_val32[cnt] == 0)
        {
            *p_is_detect = 0;
            break;
        }
    }

    return CTC_E_NONE;
}

/* get serdes signal detect by gport, used for port enable and disable */
int32
sys_tsingma_mac_get_mac_signal_detect(uint8 lchip, uint32 gport, uint32* p_is_detect)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if(NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_mac_signal_detect(lchip, lport, port_attr->mac_id,
                                     port_attr->flag, port_attr->pcs_mode, p_is_detect));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port)
{
    uint16 lport = 0;
    uint32 unidir_en = 0;
    uint32 remote_link = 1;
    uint32 auto_neg_en = 0;
    uint32 auto_neg_mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_is_up);
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_LINK_UP, p_is_up));
        return CTC_E_NONE;
    }

    MAC_LOCK;

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    if (((port_attr->pcs_mode == CTC_CHIP_SERDES_XLG_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_CG_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_LG_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_XAUI_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_DXAUI_MODE))
        && (port_attr->pcs_reset_en == 1))
    {
        *p_is_up = TRUE;
        MAC_UNLOCK;
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &auto_neg_en));
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_MODE, &auto_neg_mode));
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_unidir_en(lchip, lport, &unidir_en));

    if (is_port
        && (CTC_CHIP_SERDES_SGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_2DOT5G_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII0_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII1_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII2_MODE != port_attr->pcs_mode))
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Unsupported PCS mode \n");
        return CTC_E_INVALID_PARAM;
    }

    switch (port_attr->pcs_mode)
    {
        case CTC_CHIP_SERDES_100BASEFX_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_sgmii_100basefx_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_sgmii_100basefx_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            if (is_port && auto_neg_en)
            {
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_an_remote_status(lchip, gport, auto_neg_mode, NULL, &remote_link));
                if (FALSE == remote_link)
                {
                    *p_is_up = FALSE;
                }
            }
            break;

        case CTC_CHIP_SERDES_QSGMII_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_qsgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            if (is_port && auto_neg_en)
            {
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_an_remote_status(lchip, gport, auto_neg_mode, NULL, &remote_link));
                if (FALSE == remote_link)
                {
                    *p_is_up = FALSE;
                }
            }
            break;

        case CTC_CHIP_SERDES_USXGMII0_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_usxgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            if (is_port && auto_neg_en)
            {
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_an_remote_status(lchip, gport, auto_neg_mode, NULL, &remote_link));
                if (FALSE == remote_link)
                {
                    *p_is_up = FALSE;
                }
            }
            break;

        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_xaui_dxaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_XFI_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_xfi_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_XLG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_LG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_CG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        default:
            break;
    }
    p_usw_mac_master[lchip]->mac_prop[lport].link_status = (*p_is_up)?1:0;

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_pcs_link_fault_reset(uint8 lchip, uint32 gport)
{
    uint16 lport     = 0;
    uint8  internal_mac_idx = 0;
    uint16 step      = 0;
    uint8  mii_idx   = 0;
    uint32 tbl_id    = 0;
    uint32 field_id  = 0;
    uint32 cmd       = 0;
    uint32 ignore_fault = 0;
    uint32 value = 0;
    int32  ret   = 0;
    SharedMii0Cfg0_m mii_cfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if(port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if((CTC_CHIP_SERDES_CG_MODE    != port_attr->pcs_mode) &&
       (CTC_CHIP_SERDES_XLG_MODE   != port_attr->pcs_mode) &&
       (CTC_CHIP_SERDES_LG_MODE    != port_attr->pcs_mode) &&
       (CTC_CHIP_SERDES_XAUI_MODE  != port_attr->pcs_mode) &&
       (CTC_CHIP_SERDES_DXAUI_MODE != port_attr->pcs_mode))
    {
        return CTC_E_NONE;
    }

    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    /* disable interrupt at first */
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, FALSE));

    CTC_ERROR_RETURN(_sys_tsingma_mac_mutex_flag_lock(lchip, lport));

    port_attr->pcs_reset_en = 1;

    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + step*internal_mac_idx + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &mii_cfg);

    field_id = SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f;
    DRV_IOR_FIELD(lchip, tbl_id, field_id, &ignore_fault, &mii_cfg);
    if (0 == ignore_fault)  /* care fault */
    {
        value = 1;   /* not care */
        field_id = SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        field_id = SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        field_id = SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &mii_cfg);
    }

    /* do rx reset */
    MAC_LOCK;
    ret = _sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 1);
    if(CTC_E_NONE != ret)
    {
        MAC_UNLOCK;
        _sys_tsingma_mac_mutex_flag_unlock(lchip, lport);
        return ret;
    }
    ret = _sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 0);
    if(CTC_E_NONE != ret)
    {
        MAC_UNLOCK;
        _sys_tsingma_mac_mutex_flag_unlock(lchip, lport);
        return ret;
    }
    MAC_UNLOCK;

    sal_task_sleep(110);

    if (0 == ignore_fault)
    {
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + step*internal_mac_idx + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &mii_cfg);

        value = 0;   /* resume */
        field_id = SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        field_id = SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        field_id = SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &mii_cfg);
    }

    port_attr->pcs_reset_en = 0;
    CTC_ERROR_RETURN(_sys_tsingma_mac_mutex_flag_unlock(lchip, lport));
    /* enable interrupt after operation */
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, TRUE));

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_link_up_event(uint8 lchip, uint32 gport)
{
    uint16 lport         = 0;
    uint32 value         = 0;
    uint32 auto_neg_en   = 0;
    uint32 auto_neg_mode = 0;
    uint32 mac_speed     = 0;
    uint32 speed         = 0;
    uint32 remote_link   = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ctc_chip_serdes_loopback_t ilb_param;
    ctc_chip_serdes_loopback_t elb_param;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_tsingma_mac_pcs_link_fault_reset(lchip, gport));

    /* SYSTEM modified by tongzb for ODCC authentication 2019-07-11 */
    /*enable interrupt*/
    //CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, TRUE));

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        /* if serdes loopback, do nothing */
        sal_memset(&ilb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
        sal_memset(&elb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
        ilb_param.serdes_id = port_attr->serdes_id;
        ilb_param.mode = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&ilb_param));
        elb_param.serdes_id = port_attr->serdes_id;
        elb_param.mode = 1;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&elb_param));
        if (ilb_param.enable || elb_param.enable)
        {
            return CTC_E_NONE;
        }

        CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, &value));
        CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &auto_neg_en));
        if (auto_neg_en)
        {
            if (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
            {
                CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, &auto_neg_mode));
                if (CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER != auto_neg_mode)
                {
                    return CTC_E_NONE;
                }
            }
            CTC_ERROR_RETURN(_sys_usw_mac_get_cl37_an_remote_status(lchip, gport, CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER, &speed, &remote_link));
            if (remote_link)
            {
                CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, &mac_speed));
                if (speed != mac_speed)
                {
                    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_SPEED, speed));
                }
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_link_intr(uint8 lchip, uint32 gport, uint8 enable)
{
    uint16 lport       = 0;
    uint32 cmd         = 0;
    uint32 tb_id       = 0;
    uint32 tb_index    = 0;
    uint32 link_bitmap = 0;
    uint32 value[4]    = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_intr_get_table_bitmap(lchip, port_attr, &tb_id, &link_bitmap));

    if(Hss28GUnitWrapperInterruptFunc_t == tb_id)
    {
        value[0] = link_bitmap | (link_bitmap << 8);
    }
    else
    {
        value[0] = link_bitmap;
        value[1] = link_bitmap;
    }

    if(enable)
    {
        /* SYSTEM modified by tongzb for clear link intr 2019-08-16 */
        /*clear link intr*/
        tb_index = 1;
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, tb_index, cmd, value));
        tb_index = 3;
    }
    else
    {
        tb_index = 2;
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, tb_index, cmd, value));

    MAC_UNLOCK;
    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_get_link_intr(uint8 lchip, uint32 gport, uint32* enable)
{
    uint16 lport        = 0;
    uint32 cmd          = 0;
    uint32 tb_id        = 0;
    uint32 tb_index     = 2;
    uint32 link_bitmap  = 0;
    uint32 value[4]     = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_intr_get_table_bitmap(lchip, port_attr, &tb_id, &link_bitmap));

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, tb_index, cmd, value));

    if(Hss28GUnitWrapperInterruptFunc_t == tb_id)
    {
        if(CTC_FLAG_ISSET(value[0], (link_bitmap | (link_bitmap << 8))))
        {
            *enable = 0;
        }
        else
        {
            *enable = 1;
        }
    }
    else
    {
        if(CTC_FLAG_ISSET(value[0], link_bitmap) && CTC_FLAG_ISSET(value[1], link_bitmap))
        {
            *enable = 0;
        }
        else
        {
            *enable = 1;
        }
    }

    MAC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_tsingma_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value)
{
    uint16 lport = 0;
    uint8  slice_id = 0;
    uint16 chip_port = 0;
    uint16 serdes_id = 0;
    uint8  serdes_num = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port capability, gport:0x%04X, type:%d!\n", gport, type);

    SYS_MAC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    switch (type)
    {
        case CTC_PORT_CAP_TYPE_SERDES_INFO:
            /*same as usw*/
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_serdes_info_capability(port_attr->mac_id, serdes_id, serdes_num,
                                                     port_attr->pcs_mode, port_attr->flag, (ctc_port_serdes_info_t*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_MAC_ID:
            /*same as usw*/
            *(uint32*)p_value = port_attr->mac_id;
            break;
        case CTC_PORT_CAP_TYPE_SPEED_MODE:
            /*same as usw*/
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_speed_mode_capability(lchip, gport, (uint32*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_IF_TYPE:
            /*same as usw*/
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_if_type_capability(lchip, gport, (uint32*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_FEC_TYPE:
            _sys_tsingma_mac_get_fec_type_capability(port_attr->pcs_mode, (uint32*)p_value);
            break;
        case CTC_PORT_CAP_TYPE_CL73_ABILITY:
            _sys_tsingma_mac_get_cl73_capability(lchip, gport, (uint32*)p_value);
            break;
        case CTC_PORT_CAP_TYPE_CL73_REMOTE_ABILITY:
            /*same as usw*/
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl73_ability(lchip, gport, 1, (uint32*)p_value));
            break;
        default:
            MAC_UNLOCK;
            return CTC_E_INVALID_PARAM;
    }

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_set_xpipe_en(uint8 lchip, uint16 lport, uint8 enable)
{
    uint8 slice_id   = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 tmp_val32 = 0;
    uint8  step      = 0;
    int32  ret       = CTC_E_NONE;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedMii0Cfg0_m mii_cfg;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    ret= sys_tsingma_datapath_check_xpipe_param(lchip, slice_id, lport, enable);
    if(ret)
    {
        return ret;
    }
    ret = sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
    if(ret)
    {
        return ret;
    }
    port_attr->xpipe_en = enable;

    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + port_attr->internal_mac_idx*step + port_attr->mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
    tmp_val32 = enable;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxPmacSfdEn0_f, &tmp_val32, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxPmacSfdEn0_f, &tmp_val32, &mii_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &mii_cfg);
    if(ret)
    {
        return ret;
    }

    ret = sys_tsingma_datapath_set_xpipe_en(lchip, lport, enable);
    if(ret)
    {
        return ret;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_set_xpipe_en(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport     = 0;
    uint8 enable     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    enable = value ? TRUE : FALSE;
    /* if mac already is enabled, no need to do enable again */
    if ((port_attr->xpipe_en && enable)
        || (!port_attr->xpipe_en && !enable))
    {
        MAC_UNLOCK;
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_xpipe_en(lchip, lport, enable));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_get_xpipe_en(uint8 lchip, uint32 gport, uint32 *p_value)
{
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    if (p_value)
    {
        *p_value = port_attr->xpipe_en;
    }
    MAC_UNLOCK;

    return CTC_E_NONE;
}

/**
@brief   Config port's properties
*/
int32
sys_tsingma_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    int32   ret = CTC_E_NONE;

    uint16  lport = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property, gport:0x%04X, property:%d, value:%d\n", \
                     gport, port_prop, value);

    /*Sanity check*/
    SYS_MAC_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    switch (port_prop)
    {
    case CTC_PORT_PROP_MAC_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_tsingma_mac_set_mac_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_EEE_EN:
        value = (value) ? TRUE : FALSE;
        ret = _sys_tsingma_mac_set_eee_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_FEC_EN:
        ret = sys_tsingma_mac_set_fec_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_CL73_ABILITY:
        ret = sys_usw_mac_set_cl73_ability(lchip, gport, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_tsingma_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_MODE:
        ret = sys_tsingma_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_FEC:
        ret = sys_tsingma_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, value);
        break;

    case CTC_PORT_PROP_SPEED:
        ret = _sys_tsingma_mac_set_speed(lchip, gport, value);
        break;

    case CTC_PORT_PROP_MAX_FRAME_SIZE:
        ret = _sys_tsingma_mac_set_max_frame(lchip, gport, value);
        break;

    case CTC_PORT_PROP_MIN_FRAME_SIZE:
        ret = _sys_tsingma_mac_set_min_frame_size(lchip, gport, value);
        break;

    case CTC_PORT_PROP_ERROR_CHECK:
        ret = sys_usw_mac_set_error_check(lchip, gport, value);
        break;

    case CTC_PORT_PROP_CRC_RECALC_EN:
        ret = sys_usw_mac_set_crc_recalc_en(lchip, gport, value);
        break;
    
    case CTC_PORT_PROP_RX_PAUSE_TYPE:
        ret = _sys_tsingma_mac_set_rx_pause_type(lchip, gport, value);
        break;

    case CTC_PORT_PROP_LINKSCAN_EN:
        break;

    case CTC_PORT_PROP_LINK_INTRRUPT_EN:
        value = (value) ? TRUE : FALSE;
        ret = _sys_tsingma_mac_set_link_intr(lchip, gport, value);
        p_usw_mac_master[lchip]->mac_prop[lport].link_intr_en = (value)?1:0;

        break;

    case CTC_PORT_PROP_UNIDIR_EN:
        value = (value) ? 1 : 0;
        ret = _sys_tsingma_mac_set_unidir_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_MAC_TX_IPG:
        ret = sys_usw_mac_set_ipg(lchip, gport, value);
        break;

    case CTC_PORT_PROP_PREAMBLE:
    case CTC_PORT_PROP_PADING_EN:
    case CTC_PORT_PROP_CHK_CRC_EN:
    case CTC_PORT_PROP_STRIP_CRC_EN:
    case CTC_PORT_PROP_APPEND_CRC_EN:
    case CTC_PORT_PROP_APPEND_TOD_EN:
        ret = sys_usw_mac_set_internal_property(lchip, gport, port_prop, value);
        break;

    case CTC_PORT_PROP_XPIPE_EN:
        ret = sys_tsingma_mac_set_xpipe_en(lchip, gport, value);
        break;

    default:
        return CTC_E_NOT_SUPPORT;
    }

    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    return CTC_E_NONE;
}

/**
@brief    Get port's properties according to gport id
*/
int32
sys_tsingma_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value)
{
    uint8  value8 = 0;

    uint8  inverse_value = 0;
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint16 lport = 0;
    ctc_port_speed_t port_speed = CTC_PORT_SPEED_1G;
    uint32 index = 0;
     /*uint32 value = 0;*/
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port property, gport:0x%04X, property:%d!\n", gport, port_prop);

    /*Sanity check*/
    SYS_MAC_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    /* zero value */
    *p_value = 0;
    index = lport;

    switch (port_prop)
    {
    case CTC_PORT_PROP_MAC_EN:
        ret = sys_usw_mac_get_mac_en(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINK_UP:
        ret = sys_tsingma_mac_get_link_up(lchip, gport, p_value, 1);
        break;

    case CTC_PORT_PROP_FEC_EN:
        MAC_LOCK;
        ret = _sys_tsingma_mac_get_fec_en(lchip, lport, p_value);
        MAC_UNLOCK;
        break;
    case CTC_PORT_PROP_CL73_ABILITY:
        /*same as usw*/
        ret = sys_usw_mac_get_cl73_ability(lchip, gport, 0, p_value);
        break;

    case CTC_PORT_PROP_SIGNAL_DETECT:
        ret = sys_tsingma_mac_get_mac_signal_detect(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINK_INTRRUPT_EN:
        ret = _sys_tsingma_mac_get_link_intr(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_EN:
        ret = sys_usw_mac_get_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, p_value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_MODE:
        ret = sys_usw_mac_get_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, p_value);
        break;

    case CTC_PORT_PROP_SPEED:
        ret = _sys_tsingma_mac_get_speed(lchip, gport, &port_speed);
        if (CTC_E_NONE == ret)
        {
            *p_value = port_speed;
        }
        break;

    case CTC_PORT_PROP_MAX_FRAME_SIZE:
        ret = _sys_tsingma_mac_get_max_frame(lchip, gport, (ctc_frame_size_t*)p_value);
        break;

    case CTC_PORT_PROP_PREAMBLE:
    case CTC_PORT_PROP_PADING_EN:
    case CTC_PORT_PROP_CHK_CRC_EN:
    case CTC_PORT_PROP_STRIP_CRC_EN:
    case CTC_PORT_PROP_APPEND_CRC_EN:
    case CTC_PORT_PROP_APPEND_TOD_EN:
        ret = sys_usw_mac_get_internal_property(lchip, gport, port_prop, p_value);
        break;

    case CTC_PORT_PROP_MIN_FRAME_SIZE:
        ret = _sys_tsingma_mac_get_min_frame_size(lchip, gport, &value8);
        *p_value = value8;
        break;

    case CTC_PORT_PROP_ERROR_CHECK:
        ret = sys_usw_mac_get_error_check(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_RX_PAUSE_TYPE:
        ret = _sys_tsingma_mac_get_rx_pause_type(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINKSCAN_EN:
        *p_value = 1;
        break;

    case CTC_PORT_PROP_UNIDIR_EN:
        ret = _sys_tsingma_mac_get_unidir_en(lchip, lport, p_value);
        break;

    case CTC_PORT_PROP_MAC_TX_IPG:
        ret = sys_usw_mac_get_ipg(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_XPIPE_EN:
        ret = sys_tsingma_mac_get_xpipe_en(lchip, gport, p_value);
        break;

    default:
        return CTC_E_NOT_SUPPORT;
    }

    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (cmd == 0)
    {
        /*when cmd == 0, get value from  software memory*/
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));

    *p_value = inverse_value ? (!(*p_value)) : (*p_value);

    return CTC_E_NONE;
}

 /*/TODO: How to deal with XFI/XLG BASER FEC ability?*/
int32
sys_tsingma_mac_isr_cl73_complete_isr(uint8 lchip, void* p_data, void* p_data1)
{
    uint32 gport = 0;
    uint16 lport = 0;
    uint8  cl73_ok = 0;
    uint8  serdes_id = 0;
    uint8  lane = 0;
    uint8  gchip = 0;
    uint8  enable = 0;
    uint8  mac_en[4] = {0};
    uint8  mac_idx;
    sys_datapath_an_ability_t remote_ability;
    sys_datapath_an_ability_t local_ability;
    uint32 ability, cstm_ability;
    uint8  post_proc_flag = TRUE;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ctc_port_if_mode_t if_mode;

    cl73_ok = *((uint8*)p_data1);
    serdes_id = *((uint8*)p_data);

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Enter %s\n", __FUNCTION__);
    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, cl73_ok:%d\n", serdes_id, cl73_ok);

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    sal_memset(&if_mode, 0, sizeof(ctc_port_if_mode_t));

    /* get port info from sw table */
    sys_usw_get_gchip_id(lchip, &gchip);
    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, serdes_id, &gport));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }
    if((CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not support \n");
        return CTC_E_NOT_SUPPORT;
    }

    MAC_LOCK;
    if(cl73_ok)
    {
        if (0 == port_attr->is_first)
        {
            /* 1. recover bypass, get ability */
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_auto_neg_local_ability(lchip, serdes_id, &local_ability));
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_auto_neg_remote_ability(lchip, serdes_id, &remote_ability));
            ability = local_ability.base_ability0 & remote_ability.base_ability0;
            cstm_ability = local_ability.np1_ability0 & remote_ability.np1_ability0;

            if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_100GBASE_KR4)
                || CTC_FLAG_ISSET(ability, SYS_PORT_CL73_100GBASE_CR4))
            {
                if_mode.speed = CTC_PORT_SPEED_100G;
                if_mode.interface_type = CTC_PORT_IF_CR4;

                if (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    /* enable CG MAC */
                    if(enable)
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }

                if (port_attr->an_fec)
                {
                    if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_RS))
                        && ((CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ))
                            ||(CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                    }
                }
            }
            else if(CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_50GBASE_KR2)
                    || CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_50GBASE_CR2))
            {
                if_mode.speed = CTC_PORT_SPEED_50G;
                if_mode.interface_type = CTC_PORT_IF_CR2;

                 /*Switch to 50G*/
                if (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    /* enable LG MAC */
                    if(enable)
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }

                ability = local_ability.base_ability1 & remote_ability.base_ability1;
                cstm_ability = local_ability.np1_ability1 & remote_ability.np1_ability1;

                /*25/50G Ethernet Consortium Draft 3.2.5.2*/
                if (port_attr->an_fec)
                {
                    if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_RS))
                        && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL91_FEC_SUP)
                            && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL91_FEC_REQ)
                                ||CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL91_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL74_FEC_SUP)
                                 && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ)
                                      || CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            &&(CTC_FLAG_ISSET(ability, SYS_PORT_CL73_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                    }
                }
            }
            else if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_40GBASE_CR4)
                    || CTC_FLAG_ISSET(ability, SYS_PORT_CL73_40GBASE_KR4))
            {
                if_mode.speed = CTC_PORT_SPEED_40G;
                if_mode.interface_type = CTC_PORT_IF_CR4;

                if (CTC_CHIP_SERDES_XLG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    /* enable XLG MAC */
                    if(enable)
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }

                /*TM support 10G/40G BASE-R FEC*/
                if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                    &&((CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)) || (CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)))
                    &&(CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_SUP))
                    &&(CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_SUP)))
                {
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                }
                else
                {
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                }
            }
            else if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR_S)
                    || CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR))
            {
                if_mode.speed = CTC_PORT_SPEED_25G;
                if_mode.interface_type = CTC_PORT_IF_CR;
                 /*Switch to 25G*/
                if (CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    if(SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) && (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode))
                    {
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            mac_en[mac_idx] = p_usw_mac_master[lchip]->mac_prop[lport/4*4 + mac_idx].port_mac_en;
                        }
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XXVG MAC */
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            if(mac_en[mac_idx])
                            {
                                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, (gport/4*4 + mac_idx), TRUE));
                            }
                        }
                    }
                    else
                    {
                        enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XXVG MAC */
                        if(enable)
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                        }
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }
                /*CL73.6.5.1*/
                if (port_attr->an_fec)
                {
                    if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR))
                    {
                        if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_RS))
                            && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)
                                ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)))
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                        }
                        else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)
                                    ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)))
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                        }
                        else
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                        }
                    }
                    else if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR_S))
                    {
                        if (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)
                            ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)
                            ||CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)
                            ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ))
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                        }
                        else
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                        }
                    }
                }
            }
            else if(CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_25GBASE_KR1)
                    || CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_25GBASE_CR1))
            {
                if_mode.speed = CTC_PORT_SPEED_25G;
                if_mode.interface_type = CTC_PORT_IF_CR;
                 /*Switch to 25G*/
                if (CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    if(SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) && (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode))
                    {
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            mac_en[mac_idx] = p_usw_mac_master[lchip]->mac_prop[lport/4*4 + mac_idx].port_mac_en;
                        }
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XXVG MAC */
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            if(mac_en[mac_idx])
                            {
                                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, (gport/4*4 + mac_idx), TRUE));
                            }
                        }
                    }
                    else
                    {
                        enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XXVG MAC */
                        if(enable)
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                        }
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }
                ability = local_ability.base_ability1 & remote_ability.base_ability1;
                cstm_ability = local_ability.np1_ability1 & remote_ability.np1_ability1;
                /*25/50G Ethernet Consortium Draft 3.2.5.2*/
                if (port_attr->an_fec)
                {
                    if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_RS))
                        && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL91_FEC_SUP)
                            && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL91_FEC_REQ)
                                || CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL91_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL74_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                    }
                }
            }
            else if(CTC_FLAG_ISSET(ability, SYS_PORT_CL73_10GBASE_KR))
            {
                if_mode.speed = CTC_PORT_SPEED_10G;
                if_mode.interface_type = CTC_PORT_IF_XFI;

                if (CTC_CHIP_SERDES_XFI_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    if(SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) && (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode))
                    {
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            mac_en[mac_idx] = p_usw_mac_master[lchip]->mac_prop[lport/4*4 + mac_idx].port_mac_en;
                        }
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XFI MAC */
                        for(mac_idx = 0; mac_idx < 4; mac_idx++)
                        {
                            if(mac_en[mac_idx])
                            {
                                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, (gport/4*4 + mac_idx), TRUE));
                            }
                        }
                    }
                    else
                    {
                        enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                        /* enable XFI MAC */
                        if(enable)
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_mac_en(lchip, gport, TRUE));
                        }
                    }
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }

                /*TM support 10G/40G BASE-R FEC*/
                if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                    &&((CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)) || (CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)))
                    &&(CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_SUP))
                    &&(CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_SUP)))
                {
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                }
                else
                {
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                }
            }
            else
            {
                post_proc_flag = FALSE;
            }

            SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                "Lp base ability[0]:0x%x,[1]:0x%x, next page0[0]:0x%x,[1]:0x%x,next page1[0]:0x%x,[1]:0x%x\n",
                remote_ability.base_ability0,remote_ability.base_ability1, remote_ability.np0_ability0,
                remote_ability.np0_ability1, remote_ability.np1_ability0, remote_ability.np1_ability1);

            /*CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_set_dfe_en(lchip, gport, 1));*/

            /* 3. AN OK post process */
            if(post_proc_flag)
            {
                SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, pcs_mode:%d start post process\n", gport, port_attr->pcs_mode);

                /*for 28G serdes in 10G/40G mode, do sm reset before LT*/
                if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
                {
                    if(CTC_PORT_SPEED_10G == if_mode.speed)
                    {
                        sys_tsingma_datapath_set_serdes_sm_reset(lchip, serdes_id);
                    }
                    else if(CTC_PORT_SPEED_40G == if_mode.speed)
                    {
                        for(lane = (serdes_id/4*4); lane < (serdes_id/4*4+4); lane++)
                        {
                            sys_tsingma_datapath_set_serdes_sm_reset(lchip, lane);
                        }
                    }
                }

                /*3.1  if multi serdes-1 port, disable AN in other serdes*/
                if((CTC_PORT_SPEED_100G == if_mode.speed) || (CTC_PORT_SPEED_40G == if_mode.speed))
                {
                    for(lane = (serdes_id/4*4); lane < (serdes_id/4*4+4); lane++)
                    {
                        if(lane == serdes_id)
                        {
                            continue;
                        }
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_auto_neg_en(lchip, lane, FALSE));
                    }
                }
                else if(CTC_PORT_SPEED_50G == if_mode.speed)
                {
                    if(1 == port_attr->flag)
                    {
                        switch(serdes_id % 4)
                        {
                            case 0:
                                lane = serdes_id + 3;
                                break;
                            case 1:
                                lane = serdes_id + 1;
                                break;
                            case 2:
                                lane = serdes_id - 1;
                                break;
                            case 3:
                            default:
                                lane = serdes_id - 3;
                                break;
                        }
                    }
                    else
                    {
                        lane = (0 == serdes_id%2) ? (serdes_id+1) : (serdes_id-1);
                    }
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_auto_neg_en(lchip, lane, FALSE));
                }

                /*3.2  enable Link training*/
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_set_3ap_training_en(lchip, gport, TRUE));

                /*3.3  if public ability exists, set PCS RX rst to 0*/
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 0));
            }

            port_attr->is_first = 1;
        }
    }

    MAC_UNLOCK;

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Exit %s\n", __FUNCTION__);

    return CTC_E_NONE;
}


int32
_sys_tsingma_mac_isr_feature_handler(uint8 lchip, uint8 hss_id, sys_func_intr_feature_t ft_id, uint8 inner_serdes_id, uint8 hss12g_flag)
{
    uint8  cl73_ok   = TRUE;
    uint8  serdes_id = hss12g_flag ? (hss_id * HSS12G_LANE_NUM + inner_serdes_id) : (SYS_DATAPATH_SERDES_28G_START_ID + inner_serdes_id);

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Pcs interrupt happened! serdes_id %u, cl73_ok %u, ft_id %u\n", serdes_id, cl73_ok, ft_id);
    switch(ft_id)
    {
        case FUNC_INTR_AN_LINK_GOOD:  /*12G or 28G*/
            CTC_ERROR_RETURN(sys_tsingma_mac_isr_cl73_complete_isr(lchip, (void*)(&serdes_id), (void*)(&cl73_ok)));
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_isr_hss12g_feature_dispatch(uint8 lchip, uint8 hss_id, sys_func_intr_feature_t ft_id, uint32 word_2_val, uint32 word_3_val)
{
    uint32 word_2_mask = 0;
    uint32 word_3_mask = 0;
    uint32 temp_val = 0;
    uint8  word_2_fix_bit = 0;
    uint8  word_3_fix_bit = 0;
    uint8  lane_num_fix = 5;
    uint8  cnt;

    switch(ft_id)
    {
        case FUNC_INTR_AN_LINK_GOOD:
            word_2_mask = 0x82082082;
            word_3_mask = 0x820;
            word_2_fix_bit = 1;
            word_3_fix_bit = 5;
            lane_num_fix = 6;
            break;
        case FUNC_INTR_TRAIN_FAIL:
            word_2_mask = 0x10410410;
            word_3_mask = 0x4104;
            word_2_fix_bit = 4;
            word_3_fix_bit = 2;
            break;
        case FUNC_INTR_TRAIN_OK:
            word_2_mask = 0x20820820;
            word_3_mask = 0x8208;
            word_2_fix_bit = 5;
            word_3_fix_bit = 3;
            break;
        default:
            return CTC_E_NONE;
    }

    //word 2
    if(0 != (word_2_mask & word_2_val))
    {
        for(cnt = 0; cnt < HSS12G_LANE_NUM; cnt++)
        {
            temp_val = word_2_val >> (6*cnt + word_2_fix_bit);
            if(0 == temp_val)
            {
                break;
            }
            if(CTC_FLAG_ISSET(temp_val, 0x1))
            {
                /*
                Call feature process to handle intr.
                cnt - inner serdes id  ft_id - feature process
                */
                _sys_tsingma_mac_isr_feature_handler(lchip, hss_id, ft_id, cnt, TRUE);
            }
        }
    }

    //word 3
    if(0 != (word_3_mask & word_3_val))
    {
        for(cnt = 0; cnt < HSS12G_LANE_NUM - lane_num_fix; cnt++)
        {
            temp_val = word_3_val >> (6*cnt + word_3_fix_bit);
            if(0 == temp_val)
            {
                break;
            }
            if(CTC_FLAG_ISSET(temp_val, 0x1))
            {
                /*
                Call feature process to handle intr.
                cnt+lane_num_fix - inner serdes id  ft_id - feature process
                */
                _sys_tsingma_mac_isr_feature_handler(lchip, hss_id, ft_id, cnt+lane_num_fix, TRUE);
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_isr_hss28g_feature_dispatch(uint8 lchip, sys_func_intr_feature_t ft_id, uint32 word_0_val, uint32 word_1_val)
{
    uint32 word_0_mask = 0;
    uint32 word_1_mask = 0;
    uint32 temp_val = 0;
    uint8  word_0_fix_bit = 0;
    uint8  word_1_fix_bit = 0;
    uint8  lane_num_fix = 5;
    uint8  cnt;

    switch(ft_id)
    {
        case FUNC_INTR_AN_LINK_GOOD:
            word_0_mask = 0x24920000;
            word_1_mask = 0x49;
            word_0_fix_bit = 17;
            word_1_fix_bit = 0;
            break;
        default:
            return CTC_E_NONE;
    }

    //word 0
    if(0 != (word_0_mask & word_0_val))
    {
        for(cnt = 0; cnt < 2*HSS28G_LANE_NUM; cnt++)
        {
            temp_val = word_0_val >> (3*cnt + word_0_fix_bit);
            if(0 == temp_val)
            {
                break;
            }
            if(CTC_FLAG_ISSET(temp_val, 0x1))
            {
                /*
                Call feature process to handle intr.
                cnt - inner serdes id  ft_id - feature process
                */
                _sys_tsingma_mac_isr_feature_handler(lchip, 0, ft_id, cnt, FALSE);
            }
        }
    }

    //word 1
    if(0 != (word_1_mask & word_1_val))
    {
        for(cnt = 0; cnt < 2*HSS28G_LANE_NUM - lane_num_fix; cnt++)
        {
            temp_val = word_1_val >> (3*cnt + word_1_fix_bit);
            if(0 == temp_val)
            {
                break;
            }
            if(CTC_FLAG_ISSET(temp_val, 0x1))
            {
                /*
                Call feature process to handle intr.
                cnt+lane_num_fix - inner serdes id  ft_id - feature process
                */
                _sys_tsingma_mac_isr_feature_handler(lchip, 0, ft_id, cnt+lane_num_fix, FALSE);
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_isr_linkstat_handler(uint8 lchip, uint16 lport, uint32 link_stat)
{
    ctc_port_link_status_t     port_link_status;
    CTC_INTERRUPT_EVENT_FUNC   cb        = NULL;
    uint8  gchip_id  = 0;
    uint32 gport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sys_usw_get_gchip_id(lchip, &gchip_id);
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip_id, lport);
    port_link_status.gport = gport;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lport %u, link_stat %u\n", lport, link_stat);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (NULL == port_attr)
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    /*check autoneg enable and link change to down, need restart autoneg*/
    if((port_attr->port_type == SYS_DMPS_NETWORK_PORT) &&
       (0 == link_stat) && (0 != p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable))
    {
        _sys_tsingma_mac_set_cl73_auto_neg_en(lchip, gport, FALSE, TRUE);
        _sys_tsingma_mac_set_cl73_auto_neg_en(lchip, gport, TRUE, TRUE);
    }
    MAC_UNLOCK;

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PORT_LINK_CHANGE, &cb))
    if (cb)
    {
        cb(gchip_id, &port_link_status);
    }

    /*Add task delay to eliminate jitter*/
    /*sal_task_sleep(10);*/

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_isr_linkstat(uint8 lchip, uint8 hss_id, uint32 word_0_val, uint32 word_1_val)
{
    uint8   inner_mii;
    uint8   up_flag   = 0;  /*1-up*/
    uint8   down_flag = 0;  /*1-down*/
    uint32  link_stat = 0;  /*1-up  0-down*/
    uint8   gchip     = 0;
    uint16  lport     = 0;
    uint32  gport     = 0;

    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    if(3 < hss_id)
    {
        return CTC_E_INVALID_PARAM;
    }

    /*Retrieve lport and link stat to call handler*/
    for(inner_mii = 0; inner_mii < 32; inner_mii++)
    {
        up_flag   = 0;
        down_flag = 0;
        link_stat = 0;

        lport = p_usw_datapath_master[lchip]->mii_lport_map[hss_id][inner_mii];
        if(SYS_DATAPATH_USELESS_MAC == lport)
        {
            continue;
        }

        if(3 == hss_id)
        {
            if((word_0_val>>inner_mii) & 1) //link up status update
            {
                up_flag = 1;
            }
            if((word_0_val>>(inner_mii+8)) & 1) //link down status update
            {
                down_flag = 1;
            }
        }
        else
        {
            if((word_0_val>>inner_mii) & 1) //link up status update
            {
                up_flag = 1;
            }
            if((word_1_val>>inner_mii) & 1) //link down status update
            {
                down_flag = 1;
            }
        }

        if((!up_flag) && (!down_flag))
        {
            continue;
        }

        if(up_flag && down_flag)
        {
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
            CTC_ERROR_RETURN(sys_tsingma_mac_get_link_up(lchip, gport, &link_stat, 0));
        }
        else
        {
            link_stat = up_flag ? 1 : 0;
        }

        CTC_ERROR_RETURN(sys_tsingma_mcu_set_port_link_stat(lchip, lport, link_stat));
        /* SYSTEM modified by tongzb, disable link up interrupt, for bug 51149 */
        if((down_flag)&&(!link_stat))
        {
            CTC_ERROR_RETURN(_sys_tsingma_mac_isr_linkstat_handler(lchip, lport, link_stat));
        }
    }
    return CTC_E_NONE;
}

/*
pcs interrupt dispatcher
*/
STATIC int32
sys_tsingma_mac_isr_pcs_dispatch(uint8 lchip, uint32 intr, void* p_data)
{
    uint32* p_bmp = (uint32*)p_data;
    uint32  tbl_id = intr;
    uint8  hss_id = 0;
    sys_func_intr_feature_t  ft_id;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tbl_id %u, 0x%x, 0x%x, 0x%x, 0x%x\n", tbl_id, p_bmp[0], p_bmp[1], p_bmp[2], p_bmp[3]);
    if((0 == p_bmp[0]) && (0 == p_bmp[1]) && (0 == p_bmp[2]) && (0 == p_bmp[3]))
    {
        return CTC_E_NONE;
    }

    if(Hss28GUnitWrapperInterruptFunc_t == tbl_id)
    {
        for(ft_id = FUNC_INTR_AN_COMPLETE; ft_id < FUNC_INTR_RX_UPDATE_REQ; ft_id++)
        {
            _sys_tsingma_mac_isr_hss28g_feature_dispatch(lchip, ft_id, p_bmp[0], p_bmp[1]);
        }
        _sys_tsingma_mac_isr_linkstat(lchip, 3, p_bmp[0], 0);
    }
    else if((Hss12GUnitWrapperInterruptFunc0_t <= tbl_id) && (Hss12GUnitWrapperInterruptFunc2_t >= tbl_id))
    {
        hss_id = tbl_id - Hss12GUnitWrapperInterruptFunc0_t;
        for(ft_id = FUNC_INTR_AN_COMPLETE; ft_id < FUNC_INTR_BUTT; ft_id++)
        {
            _sys_tsingma_mac_isr_hss12g_feature_dispatch(lchip, hss_id, ft_id, p_bmp[2], p_bmp[3]);
        }
        _sys_tsingma_mac_isr_linkstat(lchip, hss_id, p_bmp[0], p_bmp[1]);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_get_hss28g_phy_ready(uint8 lchip, uint16 start_serdes_id, uint8 serdes_num, uint8* p_ready_flag)
{
    uint16 lane_id  = 0;
    uint16 lane_id_start  = start_serdes_id - SYS_DATAPATH_SERDES_28G_START_ID;
    uint16 lane_id_end  = lane_id_start + serdes_num;
    uint8  step     = Hss28GMon_monHssL1RxPhyReady_f - Hss28GMon_monHssL0RxPhyReady_f;
    uint32 cmd      = 0;
    uint32 value    = 0;
    uint32 fld_id[HSS28G_LANE_NUM*2] = {0};
    Hss28GMon_m hss_mon;

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(start_serdes_id))
    {
        return CTC_E_INVALID_PARAM;
    }

    for(lane_id = 0; lane_id < HSS28G_LANE_NUM*2; lane_id++)
    {
        fld_id[lane_id] = Hss28GMon_monHssL0RxPhyReady_f + step*lane_id;
    }

    cmd = DRV_IOR(Hss28GMon_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_mon));

    for(lane_id = lane_id_start; lane_id < lane_id_end; lane_id++)
    {
        DRV_IOR_FIELD(lchip, Hss28GMon_t, fld_id[lane_id], &value, &hss_mon);
        if(0 == value)
        {
            *p_ready_flag = FALSE;
            return CTC_E_NONE;
        }
    }

    *p_ready_flag = TRUE;
    return CTC_E_NONE;
}

int32
_sys_tsingma_mac_poll_hss28g_port(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr)
{
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 num = 0;
    uint8 index = 0;
    uint8 prbs_en = 0;
    uint8 nr_flag_lane = 0;
    uint8 nr_flag = 0;
    uint32 fld_id = 0;
    uint32 step = 0;
    uint32 cmd = 0;
    uint32 phy_ready = 0;
    uint8 i = 0;
    uint32 pcs_sta  = 0;
    Hss28GMon_m mon;

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        return CTC_E_NONE;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    CTC_ERROR_RETURN(sys_usw_mac_get_pcs_status(lchip, lport, &pcs_sta));
    if (!pcs_sta)
    {
        for (index = 0; index < num; index++)
        {
            GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, TRUE));
        }
        p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 0;
    }

    for (index = 0; index < num; index++)
    {
        cmd = DRV_IOR(Hss28GMon_t, DRV_ENTRY_FLAG);
        for (i = 0; i < 3; i++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mon));
            step = Hss28GMon_monHssL1RxPhyReady_f - Hss28GMon_monHssL0RxPhyReady_f;
            fld_id = Hss28GMon_monHssL0RxPhyReady_f + step * (serdes_id - 24);
            DRV_IOR_FIELD(lchip, Hss28GMon_t, fld_id, &phy_ready, &mon);
            if (0 == phy_ready)
            {
                p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 0;
                return CTC_E_NONE;
            }
            sal_task_sleep(10);
        }
    }

    if (0 != p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready)
    {
        return CTC_E_NONE;
    }

    for (index = 0; index < num; index++)
    {
        GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
        for (i = 0; i < 3; i++)
        {
            sys_tsingma_datapath_get_serdes_no_random_flag(lchip, real_serdes, &nr_flag_lane);
            nr_flag += nr_flag_lane;
        }
    }

    if (0 == nr_flag)
    {
        for (index = 0; index < num; index++)
        {
            GET_SERDES(port_attr->tbl_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_prbs31_en(lchip, real_serdes, &prbs_en));
            if (prbs_en)
            {
                /*when pcs down->up, do sm reset*/
                sys_tsingma_datapath_set_serdes_sm_reset(lchip, real_serdes);
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, real_serdes, FALSE));
            }
        }
        p_usw_mac_master[lchip]->mac_prop[lport].old_phy_ready = 1;
    }

    return CTC_E_NONE;
}

void
_sys_tsingma_mac_monitor_thread(void* para)
{
    uint8  lchip       = (uintptr)para;
    uint8  gchip       = 0;
    uint16 lport       = 0;
    uint16 chip_port   = 0;
    int32  ret         = 0;
    uint32 is_up       = 0;
    uint32 gport       = 0;
    uint32 cl72_status = 0;
    uint16 serdes_id   = 0;
    uint8  i           = 0;
    uint8  serdes_num  = 0;
    uint32 port_skip_flag = 0;

    uint32 is_detect = 0;
    uint8  phy_rdy_28g = FALSE;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 tmp_cl73_status[4] = {0};
    uint16 cl73_status     = 0;
    uint32 pcs_sta = 0;
    uint16 real_serdes = 0;

    uint8 an_restart_flag1 = 0;
    uint8 an_restart_flag2 = 0;

    sys_usw_get_gchip_id(lchip, &gchip);

    while(1)
    {
        for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            port_skip_flag = 0;
            is_detect = 0;
            tmp_cl73_status[0] = 0;
            tmp_cl73_status[1] = 0;
            tmp_cl73_status[2] = 0;
            tmp_cl73_status[3] = 0;
            an_restart_flag1   = 0;
            an_restart_flag2   = 0;
            cl73_status        = 0;
            pcs_sta            = 0;

            SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
            if(p_usw_mac_master[lchip]->polling_status == 0)
            {
                continue;
            }

            gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

            MAC_LOCK;
            /*check mac used */
            ret = sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
            if((port_attr->port_type != SYS_DMPS_NETWORK_PORT) || ret)
            {
                MAC_UNLOCK;
                continue;
            }

            /*check mac enable */
            if(FALSE == p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
            {
                MAC_UNLOCK;
                continue;
            }

            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, 0, &serdes_id, &serdes_num);

            for (i = 0; i < serdes_num; i++)
            {
                GET_SERDES(port_attr->tbl_id, serdes_id, i, port_attr->flag, port_attr->pcs_mode, real_serdes);
                sys_usw_datapath_get_serdes_cl73_status(lchip, real_serdes, &tmp_cl73_status[i]);
                cl73_status += tmp_cl73_status[i];
            }

            an_restart_flag1 = p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable;
            /* restart CL73 AN */
            if (an_restart_flag1)
            {
                port_skip_flag = 1;
                if (serdes_id < SYS_DATAPATH_SERDES_28G_START_ID)
                {
                    switch(port_attr->pcs_mode)
                    {
                        case CTC_CHIP_SERDES_XFI_MODE:
                        case CTC_CHIP_SERDES_XXVG_MODE:
                            _sys_tsingma_mac_get_xfi_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                            break;
                        case CTC_CHIP_SERDES_XLG_MODE:
                            _sys_tsingma_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                            break;
                        case CTC_CHIP_SERDES_LG_MODE:
                            _sys_tsingma_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                            break;
                        case CTC_CHIP_SERDES_CG_MODE:
                            _sys_tsingma_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                            break;
                    }
                    an_restart_flag2 = (!is_up);
                }
                else
                {
                    sys_usw_mac_get_pcs_status(lchip, lport, &pcs_sta);
                    an_restart_flag2 = (!pcs_sta);
                }

                if (an_restart_flag2)
                {
                    /* old_cl73_status:
                     *   0 -- from auto to training
                     *   1 -- training 1 cycle still not up
                     */
                    _sys_usw_mac_get_3ap_training_en(lchip, lport, &cl72_status);
                    if(((SYS_PORT_CL72_PROGRESS == cl72_status) || (SYS_PORT_CL72_OK == cl72_status)) &&
                       (0 == p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status))
                    {
                        p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status = 1;
                    }
                    else
                    {
                        _sys_tsingma_mac_set_cl73_auto_neg_en(lchip, gport, FALSE, TRUE);
                        _sys_tsingma_mac_set_cl73_auto_neg_en(lchip, gport, TRUE, TRUE);
                        p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status = 0;
                    }
                }
            }

            if (port_skip_flag)
            {
                MAC_UNLOCK;
                continue;
            }

            /*for 28G XAUI/DXAUI mode, if link is down (mac enabled) and phy_ready is 1, do rx reset to recover link status*/
            if(SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id) &&
               ((CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode)))
            {
                is_up       = FALSE;
                phy_rdy_28g = FALSE;
                _sys_tsingma_mac_get_xaui_dxaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                if(!is_up)
                {
                    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - (port_attr->slice_id)*SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, port_attr->slice_id, &serdes_id, &serdes_num);
                    ret = _sys_tsingma_mac_get_hss28g_phy_ready(lchip, serdes_id, serdes_num, &phy_rdy_28g);
                    if((CTC_E_NONE == ret) && (TRUE == phy_rdy_28g))
                    {
                        _sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 1);
                        _sys_tsingma_mac_pcs_rx_rst(lchip, lport, 0, 0);
                    }
                }
            }

            /*check signal detect */
            ret = _sys_tsingma_mac_get_mac_signal_detect(lchip, lport, port_attr->mac_id,
                port_attr->flag, port_attr->pcs_mode, &is_detect);
            if ((!is_detect) || ret)
            {
                MAC_UNLOCK;
                continue;
            }

            ret = _sys_tsingma_mac_poll_hss28g_port(lchip, lport, port_attr);
            if(ret)
            {
                MAC_UNLOCK;
                continue;
            }

            MAC_UNLOCK;
            sal_task_sleep(10);
        }

        sal_task_sleep(1000);
    }
}

STATIC int32
_sys_tsingma_mac_monitor_link(uint8 lchip)
{
    drv_work_platform_type_t platform_type;
    int32 ret = 0;
    uintptr chip_id = lchip;
    uint64 cpu_mask = 0;
    uint32 field_val = 0;
    uint32 cmd = 0;

    if (sys_usw_chip_get_reset_hw_en(lchip))
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));
    if (platform_type != HW_PLATFORM)
    {
        return CTC_E_NONE;
    }

    sal_memset(p_usw_mac_master[lchip]->scan_log, 0, sizeof(sys_usw_scan_log_t)*SYS_PORT_MAX_LOG_NUM);

    cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
    ret = sys_usw_task_create(lchip,&(p_usw_mac_master[lchip]->p_monitor_scan), "ctclnkMon",
                          SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, SAL_TASK_TYPE_LINK_SCAN,cpu_mask, _sys_tsingma_mac_monitor_thread, (void*)chip_id);
    if (ret < 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_usw_mac_master[lchip]->polling_status = 1;
    field_val = 1;
    cmd = DRV_IOW(OobFcReserved_t, OobFcReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

/**
 @brief initialize the port module
*/
int32
sys_tsingma_mac_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 lport = 0;
    uint8  gchip = 0;
    uint16 tmp_port = 0;
    uint32 val_flt = 0x2625A00;
    RefDivHsPcsLinkPulse_m hs_pulse;
    RefDivCsPcsLinkPulse_m cs_pulse;
    sys_datapath_lport_attr_t* port_attr = NULL;


    /* Enlarge link Filter */
    cmd = DRV_IOR(RefDivHsPcsLinkPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_pulse));
    DRV_IOW_FIELD(lchip, RefDivHsPcsLinkPulse_t, RefDivHsPcsLinkPulse_cfgRefDivHsLinkFilterPulse_f, &val_flt, &hs_pulse);
    cmd = DRV_IOW(RefDivHsPcsLinkPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_pulse));

    cmd = DRV_IOR(RefDivCsPcsLinkPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cs_pulse));
    DRV_IOW_FIELD(lchip, RefDivCsPcsLinkPulse_t, RefDivCsPcsLinkPulse_cfgRefDivCsLinkFilterPulse_f, &val_flt, &cs_pulse);
    cmd = DRV_IOW(RefDivCsPcsLinkPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cs_pulse));

    /* mac config init */
    CTC_ERROR_RETURN(_sys_tsingma_mac_init_mac_config(lchip));

    /* during porting phase suppose all port is gport */
    /* !!!!!!!!!!!it should config mac according to datapath later!!!!!!!!!*/
    CTC_ERROR_RETURN(_sys_tsingma_mac_chan_init(lchip));

    CTC_ERROR_RETURN(_sys_tsingma_mac_monitor_link(lchip));

    CTC_ERROR_RETURN(_sys_usw_mac_flow_ctl_init(lchip));

    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_PCS_LINK_31_0, sys_tsingma_mac_isr_pcs_dispatch));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_PCS_LINK_47_32, sys_tsingma_mac_isr_pcs_dispatch));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_XG_CHANGE_0, sys_usw_peri_phy_link_change_isr));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_XG_CHANGE_1, sys_usw_peri_phy_link_change_isr));

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_mac_wb_restore(lchip));
    }
    if (0 == sys_usw_chip_get_reset_hw_en(lchip))
    {
        CTC_ERROR_RETURN(sys_tsingma_mcu_init(lchip));
    }

    /* init x-pipe */
    value = 0;
    cmd = DRV_IOW(QWriteGuaranteeCtl_t, QWriteGuaranteeCtl_mode_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (0 == SDK_WORK_PLATFORM)
    {
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, 0);
            if ((NULL == port_attr) || (SYS_DMPS_NETWORK_PORT != port_attr->port_type))
            {
                continue;
            }
            for (tmp_port = 0; tmp_port < SYS_USW_XPIPE_PORT_NUM; tmp_port++)
            {
                if (lport == g_tsingma_xpipe_mac_mapping[tmp_port].emac_id)
                {
                    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
                    CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, lport), CTC_PORT_PROP_XPIPE_EN, port_attr->xpipe_en));
                }
            }
        }
    }

    /* unmask linkstat interrupt */
    /* Hss12GUnitWrapperInterruptFunc[0~2] entry 3 bit 0~31 (up) 32~63 (down) */
    /* Hss28GUnitWrapperInterruptFunc entry 3 bit 0~7 (up) 8~15 (down) */
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x483170, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x483174, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x501170, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x501174, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x584170, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x584174, 0xffffffff));
    CTC_ERROR_RETURN(drv_usw_chip_write(lchip, 0x283098, 0x0000ffff));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_module_reset_check(uint8 lchip, sys_datapath_lport_attr_t* port_attr)
{
    uint32 value            = 0;
    uint32 value1           = 0;
    uint32 value2           = 0;
    uint32 value3           = 0;
    uint32 value4           = 0;
    uint32 value5           = 0;
    uint32 cmd              = 0;
    uint32 tbl_id           = 0;
    char* p_table_id        = NULL;
    char* p_table_id1       = NULL;
    char* p_table_id2       = NULL;
    char* p_table_id3       = NULL;
    char* p_table_id4       = NULL;
    char* p_table_id5       = NULL;
    uint8 idx               = 0;
    uint8 lg_index          = 0;
    SharedPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    QsgmiiPcsSoftRst0_m  pcs_rst1;
    UsxgmiiPcsSoftRst0_m  pcs_rst2;
    sal_memset(&pcs_rst, 0, sizeof(SharedPcsSoftRst0_m));
    sal_memset(&mii_rst, 0, sizeof(SharedMiiResetCfg0_m));
    sal_memset(&pcs_rst1, 0, sizeof(QsgmiiPcsSoftRst0_m));
    sal_memset(&pcs_rst2, 0, sizeof(UsxgmiiPcsSoftRst0_m));

    lg_index = (port_attr->internal_mac_idx <= 1) ? 0:1;
    if(port_attr->pcs_mode == CTC_CHIP_SERDES_SGMII_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_2DOT5G_MODE
       || port_attr->pcs_mode == CTC_CHIP_SERDES_XFI_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_XXVG_MODE
       || port_attr->pcs_mode == CTC_CHIP_SERDES_100BASEFX_MODE)
    {
        /* Release Pcs Tx Rx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = GetSharedPcsSoftRst0(V, softRstPcsTx0_f+port_attr->internal_mac_idx, &pcs_rst);
        value1 = GetSharedPcsSoftRst0(V, softRstPcsRx0_f+port_attr->internal_mac_idx, &pcs_rst);
        p_table_id = value ? "SharedPcsSoftRst.softRstPcsTx" : NULL;
        p_table_id1 = value1 ? "SharedPcsSoftRst.softRstPcsRx" : NULL;
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
    {
        tbl_id = QsgmiiPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst1));
        value = GetQsgmiiPcsSoftRst0(V, qsgmiiPcsTxSoftRst_f, &pcs_rst1);
        value1 = GetQsgmiiPcsSoftRst0(V, qsgmiiPcsTxSoftRst0_f+port_attr->internal_mac_idx, &pcs_rst1);
        value = (value == 0 && value1 == 0) ? 0 : 1;
        value2 = GetQsgmiiPcsSoftRst0(V, qsgmiiPmaRxSoftRst_f, &pcs_rst1);
        value3 = GetQsgmiiPcsSoftRst0(V, qsgmiiPcsRxSoftRst0_f+port_attr->internal_mac_idx, &pcs_rst1);
        value1 = (value2 == 0 && value3 == 0) ? 0 : 1;
        p_table_id = value ? "QsgmiiPcsSoftRst.qsgmiiPcsTxSoftRst" : NULL;
        p_table_id1 = value1 ? "QsgmiiPcsSoftRst.qsgmiiPcsRxSoftRst" : NULL;
        value2 = 0;
        value3 = 0;
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE ||port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE)
    {
        tbl_id = UsxgmiiPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst2));
        value = GetUsxgmiiPcsSoftRst0(V, usxgmiiPcsTxSoftRst_f, &pcs_rst2);
        value1 = GetUsxgmiiPcsSoftRst0(V, usxgmiiPcsTxSoftRst0_f+port_attr->internal_mac_idx, &pcs_rst2);
        value = (value == 0 && value1 == 0) ? 0 : 1;
        value2 = GetUsxgmiiPcsSoftRst0(V, usxgmiiPcsRxSoftRst_f, &pcs_rst2);
        value3 = GetUsxgmiiPcsSoftRst0(V, usxgmiiPcsRxSoftRst0_f+port_attr->internal_mac_idx, &pcs_rst2);
        value1 = (value2 == 0 && value3 == 0) ? 0 : 1;
        p_table_id = value ? "UsxgmiiPcsSoftRst.usxgmiiPcsTxSoftRst" : NULL;
        p_table_id1 = value1 ? "UsxgmiiPcsSoftRst.usxgmiiPcsTxSoftRst" : NULL;
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_XAUI_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_DXAUI_MODE)
    {
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        for(idx = 0; idx < 4; idx++)
        {
            value = GetSharedPcsSoftRst0(V, softRstPcsTx0_f+idx, &pcs_rst);
            if(value != 0)
            {
                break;
            }
        }
        value = (idx == 4) ? 0 :1;
        for(idx = 0; idx < 4; idx++)
        {
            value1 = GetSharedPcsSoftRst0(V, softRstPcsRx0_f+idx, &pcs_rst);
            if(value1 != 0)
            {
                break;
            }
        }
        value1 = (idx == 4) ? 0 :1;
        value2 = GetSharedPcsSoftRst0(V, softRstXlgTx_f, &pcs_rst);
        value3 = GetSharedPcsSoftRst0(V, softRstXlgRx_f, &pcs_rst);
        p_table_id = value ? "SharedPcsSoftRst.softRstPcsTx" : NULL;
        p_table_id1 = value1 ? "SharedPcsSoftRst.softRstPcsRx" : NULL;
        p_table_id2 = value2 ? "SharedPcsSoftRst.softRstXlgTx" : NULL;
        p_table_id3 = value3 ? "SharedPcsSoftRst.softRstXlgRx" : NULL;
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_XLG_MODE)
    {
        tbl_id = SharedPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = GetSharedPcsSoftRst0(V, softRstXlgTx_f, &pcs_rst);
        value1 = GetSharedPcsSoftRst0(V, softRstXlgRx_f, &pcs_rst);
        value2 = GetSharedPcsSoftRst0(V, rxDeskewSoftRst_f, &pcs_rst);
        p_table_id = value ? "SharedPcsSoftRst.softRstXlgTx" : NULL;
        p_table_id1 = value1 ? "SharedPcsSoftRst.softRstXlgRx" : NULL;
        p_table_id2 = value2 ? "SharedPcsSoftRst.rxDeskewSoftRst" : NULL;
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_LG_MODE)
    {
        /* Release Pcs Tx Soft Reset */
        tbl_id = SharedPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        if (0 == lg_index)
        {
            value = GetSharedPcsSoftRst0(V, softRstXlgTx_f, &pcs_rst);
            value1 = GetSharedPcsSoftRst0(V, softRstXlgRx_f, &pcs_rst);
            value2 = GetSharedPcsSoftRst0(V, rxDeskewSoftRst_f, &pcs_rst);
            p_table_id = value ? "SharedPcsSoftRst.softRstXlgTx" : NULL;
            p_table_id1 = value1 ? "SharedPcsSoftRst.softRstXlgRx" : NULL;
            p_table_id2 = value2 ? "SharedPcsSoftRst.rxDeskewSoftRst" : NULL;
        }
        else
        {
            value = GetSharedPcsSoftRst0(V, softRstLgTx_f, &pcs_rst);
            value1 = GetSharedPcsSoftRst0(V, softRstLgRx_f, &pcs_rst);
            value2 = GetSharedPcsSoftRst0(V, rxDeskewSoftRstLg_f, &pcs_rst);
            p_table_id = value ? "SharedPcsSoftRst.softRstLgTx" : NULL;
            p_table_id1 = value1 ? "SharedPcsSoftRst.softRstLgRx" : NULL;
            p_table_id2 = value2 ? "SharedPcsSoftRst.rxDeskewSoftRstLg" : NULL;
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_CG_MODE)
    {
        tbl_id = SharedPcsSoftRst0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        value = GetSharedPcsSoftRst0(V, softRstCgTx_f, &pcs_rst);
        value1 = GetSharedPcsSoftRst0(V, softRstCgRx_f, &pcs_rst);
        value2 = GetSharedPcsSoftRst0(V, rxDeskewSoftRst_f, &pcs_rst);
        p_table_id = value ? "SharedPcsSoftRst.softRstCgTx" : NULL;
        p_table_id1 = value1 ? "SharedPcsSoftRst.softRstCgRx" : NULL;
        p_table_id2 = value2 ? "SharedPcsSoftRst.rxDeskewSoftRst" : NULL;
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Module Reset check    --------------   Fail\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Pcs mode is error\n");
        return CTC_E_NONE;
    }
    /* Release Mii Tx Rx Soft Reset */
    tbl_id = SharedMiiResetCfg0_t + port_attr->mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
    value4 = GetSharedMiiResetCfg0(V, cfgSoftRstTx0_f+port_attr->internal_mac_idx, &mii_rst);
    value5 = GetSharedMiiResetCfg0(V, cfgSoftRstRx0_f+port_attr->internal_mac_idx, &mii_rst);
    p_table_id4 = value4 ? "SharedMiiResetCfg.cfgSoftRstTx" : NULL;
    p_table_id5 = value5 ? "SharedMiiResetCfg.cfgSoftRstRx" : NULL;

    if(value1 == 0 && value == 0 && value2 == 0 && value3 == 0 && value4 == 0 && value5 == 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Module Reset check    --------------   Pass\n");
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Module Reset check    --------------   Fail\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Correlative Status List:\n");
        if(value)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id,value);
        }
        if(value1)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id1,value1);
        }
        if(value2)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id2,value2);
        }
        if(value3)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id3,value3);
        }
        if(value4)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id4,value4);
        }
        if(value5)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s:%d\n",p_table_id5,value5);
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_hss_cs_macrco_check(uint8 lchip, uint8* lane_id,uint8 hss_id,uint8 hss_type, uint8 serdes_num)
{
    Hss12GLaneMiscCfg0_m hs12_misccfg;
    Hss12GCmuMon0_m hs12_cmumon;
    Hss12GLaneMon0_m hs12_lanemon;
    Hss12GMacroMon0_m hs12_macromon;
    Hss28GMon_m hs28_mon;
    uint32 tb_id = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    char* p_table_id[11] = {0};
    uint8 flag[11] = {0};
    uint8 step[12] = {0};
    uint8 loop = 0;
    uint8 cmu = 0;
    uint8 loop_num = 0;
    uint8 flag1 = 0;

    if(hss_type == SYS_DATAPATH_HSS_TYPE_15G)
    {
        tb_id = Hss12GLaneMiscCfg0_t + hss_id;
        step[0] = Hss12GLaneMiscCfg0_cfgHssL1AuxTxCkSel_f - Hss12GLaneMiscCfg0_cfgHssL0AuxTxCkSel_f;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs12_misccfg);
        value = GetHss12GLaneMiscCfg0(V,cfgHssL0AuxTxCkSel_f + step[0] * lane_id[0], &hs12_misccfg);
        if((lane_id[0]<=3) && (value == 0))
        {
            cmu = 0;
        }
        else if ((lane_id[0]<=3 && value == 1) || (lane_id[0]>=4 && lane_id[0]<=7 && value == 0))
        {
            cmu = 1;
        }
        else if (lane_id[0]>=4 && lane_id[0]<=7 && value == 1)
        {
            cmu = 2;
        }
        tb_id = Hss12GCmuMon0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs12_cmumon);
        step[0] = Hss12GCmuMon0_monHssCmu1Lol_f - Hss12GCmuMon0_monHssCmu0Lol_f;
        value = GetHss12GCmuMon0(V,monHssCmu0Lol_f+cmu*step[0], &hs12_cmumon);
        p_table_id[0] =  "Hss12GCmuMon.monHssCmu0Lol:1";
        flag[0] = (value == 1) ? 1 : 0;

        step[1] = Hss12GLaneMon0_monHssL1Lol_f - Hss12GLaneMon0_monHssL0Lol_f;
        step[2] = Hss12GLaneMon0_monHssL1LolUdl_f - Hss12GLaneMon0_monHssL0LolUdl_f;
        step[3] = Hss12GLaneMon0_monHssL1RxEiFiltered_f - Hss12GLaneMon0_monHssL0RxEiFiltered_f;
        step[5] = Hss12GLaneMon0_monHssL1Pma2PcsTxDetRxN_f - Hss12GLaneMon0_monHssL0Pma2PcsTxDetRxN_f;
        step[6] = Hss12GLaneMon0_monHssL1Pma2PcsTxDetRxP_f - Hss12GLaneMon0_monHssL0Pma2PcsTxDetRxP_f;
        step[7] = Hss12GLaneMon0_monHssL1PmaRstDone_f - Hss12GLaneMon0_monHssL0PmaRstDone_f;
        step[8] = Hss12GLaneMon0_monHssL1DfeRstDone_f - Hss12GLaneMon0_monHssL0DfeRstDone_f;
        step[9] = Hss12GMacroMon0_monHssL1AnethTxEn_f - Hss12GMacroMon0_monHssL0AnethTxEn_f;
        step[10] = Hss12GMacroMon0_monHssL1PcsRxReady_f - Hss12GMacroMon0_monHssL0PcsRxReady_f;
        step[11] = Hss12GMacroMon0_monHssL1PcsTxReady_f - Hss12GMacroMon0_monHssL0PcsTxReady_f;

        tb_id = Hss12GLaneMon0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs12_lanemon);
        tb_id = Hss12GMacroMon0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs12_macromon);
        for(loop = 0; loop < serdes_num; loop ++)
        {
            value = GetHss12GLaneMon0(V,monHssL0Lol_f + step[1]*lane_id[loop], &hs12_lanemon);
            if(value == 1)
            {
                flag[1] = 1;
                p_table_id[1] =  "Hss12GLaneMon.monHssL0Lol:1";
            }
            value = GetHss12GLaneMon0(V,monHssL0LolUdl_f + step[2]*lane_id[loop], &hs12_lanemon);
            if(value == 1)
            {
                flag[2] = 1;
                p_table_id[2] =  "Hss12GLaneMon.monHssL0LolUdl:1";
            }
            value = GetHss12GLaneMon0(V,monHssL0RxEiFiltered_f + step[3]*lane_id[loop], &hs12_lanemon);
            if(value != 1)
            {
                flag[3] = 1;
                p_table_id[3] =  "Hss12GLaneMon.monHssL0RxEiFiltered:0";
            }
            value = GetHss12GLaneMon0(V,monHssL0Pma2PcsTxDetRxN_f + step[5]*lane_id[loop], &hs12_lanemon);
            if(value != 1)
            {
                flag[5] = 1;
                p_table_id[5] =  "Hss12GLaneMon.monHssL0Pma2PcsTxDetRxN:0";
            }
            value = GetHss12GLaneMon0(V,monHssL0Pma2PcsTxDetRxP_f + step[6]*lane_id[loop], &hs12_lanemon);
            if(value != 1)
            {
                flag[6] = 1;
                p_table_id[6] =  "Hss12GLaneMon.monHssL0Pma2PcsTxDetRxP:0";
            }
            value = GetHss12GLaneMon0(V,monHssL0PmaRstDone_f + step[7]*lane_id[loop], &hs12_lanemon);
            if(value != 1)
            {
                flag[7] = 1;
                p_table_id[7] =  "Hss12GLaneMon.monHssL0PmaRstDone:0";
            }
            value = GetHss12GLaneMon0(V,monHssL0DfeRstDone_f + step[8]*lane_id[loop], &hs12_lanemon);
            if(value != 1)
            {
                flag[8] = 1;
                p_table_id[8] =  "Hss12GLaneMon.monHssL0DfeRstDone:0";
            }
            value = GetHss12GMacroMon0(V,monHssL0AnethTxEn_f + step[9]*lane_id[loop], &hs12_macromon);
            if(value == 1)
            {
                value = GetHss12GMacroMon0(V,monHssL0PcsRxReady_f + step[10]*lane_id[loop], &hs12_macromon);
                if(value != 1)
                {
                    flag[9] = 1;
                    p_table_id[9] =  "Hss12GMacroMon.monHssL0PcsRxReady:0";
                }
                value = GetHss12GMacroMon0(V,monHssL0PcsTxReady_f + step[11]*lane_id[loop], &hs12_macromon);
                if(value != 1)
                {
                    flag[10] = 1;
                    p_table_id[10] =  "Hss12GMacroMon.monHssL0PcsTxReady:0";
                }
            }
        }
    }
    else
    {
        tb_id = Hss28GMon_t;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs28_mon);

        step[0] = Hss28GMon_monHss1PllLockTx_f - Hss28GMon_monHss0PllLockTx_f;
        step[1] = Hss28GMon_monHss1PllLockRx_f - Hss28GMon_monHss0PllLockRx_f;
        step[2] = Hss28GMon_monHss1McuRamEccSbeCnt_f - Hss28GMon_monHss0McuRamEccSbeCnt_f;
        step[3] = Hss28GMon_monHssL1RxSigDet_f - Hss28GMon_monHssL0RxSigDet_f;
        step[4] = Hss28GMon_monHssL1RxPhyReady_f - Hss28GMon_monHssL0RxPhyReady_f;
        value = GetHss28GMon(V,monHss0PllLockTx_f + step[0]*hss_id, &hs28_mon);
        if(value == 0)
        {
            flag[0] = 1;
            p_table_id[0] =  "Hss28GMon_monHss0PllLockTx:0";
        }
        value = GetHss28GMon(V,monHss0PllLockRx_f + step[1]*hss_id, &hs28_mon);
        if(value == 0)
        {
            flag[1] = 1;
            p_table_id[1] =  "Hss28GMon_monHss0PllLockRx:0";
        }
        value = GetHss28GMon(V,monHss1McuRamEccSbeCnt_f + step[2]*hss_id, &hs28_mon);
        if(value != 0)
        {
            flag[2] = 1;
            p_table_id[2] =  "Hss28GMon_monHss0McuRamEccSbeCnt:1";
        }
        for(loop = 0; loop < serdes_num; loop ++)
        {
            value = GetHss28GMon(V,monHssL0RxSigDet_f + step[3]*(lane_id[loop]+4*hss_id), &hs28_mon);
            if(value == 0)
            {
                flag[3] = 1;
                p_table_id[3] =  "Hss28GMon_monHssL0RxSigDet:0";
            }
            value = GetHss28GMon(V,monHssL0RxPhyReady_f + step[3]*(lane_id[loop]+4*hss_id), &hs28_mon);
            if(value == 0)
            {
                flag[4] = 1;
                p_table_id[4] =  "Hss28GMon_monHssL0RxPhyReady:0";
            }
        }
    }

    loop_num = (hss_type == SYS_DATAPATH_HSS_TYPE_15G) ? 10 : 4;
    for(loop = 0; loop < loop_num; loop ++)
    {
        if(flag[loop] == 1 && flag1 == 0)
        {
            flag1 = 1;
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HSS/CS Macrco check   --------------   Fail\n");
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Correlative Status List:\n");
        }
        if(flag[loop] == 1)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s\n",p_table_id[loop]);
        }
    }
    if(flag1 == 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HSS/CS Macrco check   --------------   Pass\n");
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_mac_pcs_status_check(uint8 lchip, sys_datapath_lport_attr_t* port_attr,uint32 lport, uint8 hss_type, uint8 auto_neg_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 value_arr1[20]= {0};
    uint32 value_arr2[20]= {0};
    uint32 value2 = 0;
    uint8 loop = 0;
    uint8 loop1 = 0;
    uint8 flag1 = 0;
    uint8 flag2 = 0;
    uint8 flag[9] = {0};
    uint32 enable = 0;
    char* p_table_id[9] = {0};
    SharedPcsXauiStatus0_m pcs_status;
    SharedPcsXlgStatus0_m xlg_status;
    SharedPcsLgStatus0_m lg_status;
    uint8 step = 0;

    _sys_tsingma_mac_get_fec_en(lchip, lport, &enable);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "port_attr->mac_id=%d,port_attr->pcs_idx=%d,port_attr->mac_id=%d\n",port_attr->mac_id, port_attr->pcs_idx,port_attr->internal_mac_idx);
    if(port_attr->pcs_mode == CTC_CHIP_SERDES_XFI_MODE)
    {
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        tb_id = SharedPcsXfi0Status0_t + port_attr->internal_mac_idx*step + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[0] = value == 0 ? 0 : 1;
        p_table_id[0] = "SharedPcsXfi0Status_errBlockCnt:1";
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_badBerCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_badBerCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[1] = value == 0 ? 0 : 1;
        p_table_id[1] = "SharedPcsXfi0Status_badBerCnt:1";
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_hiBer0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[2] = value == 0 ? 0 : 1;
        p_table_id[2] = "SharedPcsXfi0Status_hiBer:1";
        if(enable == 0)
        {
            cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_rxBlockLock0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[3] = value == 1 ? 0 : 1;
            p_table_id[3] = "SharedPcsXfi0Status_rxBlockLock:0";
        }
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_xfiSyncStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[4] = value == 1 ? 0 : 1;
        p_table_id[4] = "SharedPcsXfi0Status_xfiSyncStatus0:0";

        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + port_attr->internal_mac_idx*step + port_attr->mii_idx;
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[5] = value == 0 ? 0 : 1;
        p_table_id[5] = "SharedMii0Status_dbgMiiRxFaultType:1";
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultState0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[6] = value == 0 ? 0 : 1;
        p_table_id[6] = "SharedMii0Status_dbgMiiRxFaultState:1";
        if(hss_type == SYS_DATAPATH_HSS_TYPE_15G && enable)
        {
            step = SharedPcsXgFecMon0_dbgXgFec1RxBlockLock_f - SharedPcsXgFecMon0_dbgXgFec0RxBlockLock_f;
            tb_id = SharedPcsXgFecMon0_t + port_attr->pcs_idx;
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0RxBlockLock_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[7] = value == 1 ? 0 : 1;
            p_table_id[7] = "SharedPcsXgFecMon0_dbgXgFec0RxBlockLock:0";

            step = SharedPcsXgFecMon0_dbgXgFec1UncoBlkCnt_f - SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f;
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));
            flag[8] = value1 == 0 ? 0 : 1;
            p_table_id[8] = "SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt:1";
        }
        else if(hss_type == SYS_DATAPATH_HSS_TYPE_28G && enable)
        {
            step = XgFec2StatusSharedFec0_t - XgFec0StatusSharedFec0_t;
            tb_id = XgFec0StatusSharedFec0_t + (port_attr->pcs_idx - 6) + step * (port_attr->internal_mac_idx%4);
            cmd = DRV_IOR(tb_id, XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[7] = value == 1 ? 0 : 1;
            p_table_id[7] = "XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock:0";
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_CG_MODE)
    {
        tb_id = SharedPcsCgStatus0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsCgStatus0_cgPcsAlignStatus_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[0] = value == 1 ? 0 : 1;
        p_table_id[0] = "SharedPcsCgStatus0_cgPcsAlignStatus:0";

        for(loop = 0; loop < 20; loop ++)
        {
            step = SharedPcsCgStatus0_cgLane1BipErrCnt_f - SharedPcsCgStatus0_cgLane0BipErrCnt_f;
            cmd = DRV_IOR(tb_id, SharedPcsCgStatus0_cgLane0BipErrCnt_f+loop*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_arr1[loop]));
        }
        sal_task_sleep(1000);
        for(loop = 0; loop < 20; loop ++)
        {
            step = SharedPcsCgStatus0_cgLane1BipErrCnt_f - SharedPcsCgStatus0_cgLane0BipErrCnt_f;
            cmd = DRV_IOR(tb_id, SharedPcsCgStatus0_cgLane0BipErrCnt_f+loop*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_arr2[loop]));
        }
        for(loop = 0; loop < 20; loop ++)
        {
            value = value_arr2[loop] -value_arr1[loop];
            if(value != 0)
            {
                flag[1] = 1;
                p_table_id[1] = "SharedPcsCgStatus0_cgLane0BipErrCnt:1";
                break;
            }
        }
        if(enable)
        {
            for(loop = 0; loop < 4; loop ++)
            {
                tb_id = GlobalStatusSharedFec0_t + (port_attr->pcs_idx-6);
                cmd = DRV_IOR(tb_id, GlobalStatusSharedFec0_dbgSharedFecAmLock0_f + loop);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                if(value != 1)
                {
                    flag[2] = 1;
                    p_table_id[2] = "GlobalStatusSharedFec0_dbgSharedFecAmLock0:0";
                }
                cmd = DRV_IOR(tb_id, GlobalStatusSharedFec0_dbgSharedFecAlignStatus0_f + loop);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                if(value != 1)
                {
                    flag[3] = 1;
                    p_table_id[3] = "GlobalStatusSharedFec0_dbgSharedFecAlignStatus0:0";
                }
            }
            tb_id = RsFec0StatusSharedFec0_t + (port_attr->pcs_idx-6);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value2));
            flag[4] = value2 == 0 ? 0 : 1;
            p_table_id[4] = "RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount:1";
        }

        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + port_attr->internal_mac_idx*step + port_attr->mii_idx;
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[6] = value == 0 ? 0 : 1;
        p_table_id[6] = "SharedMii0Status_dbgMiiRxFaultType:1";
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultState0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[7] = value == 0 ? 0 : 1;
        p_table_id[7] = "SharedMii0Status_dbgMiiRxFaultState:1";
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_SGMII_MODE)
    {
        step = SharedPcsSgmii1Status0_t - SharedPcsSgmii0Status0_t;
        tb_id = SharedPcsSgmii0Status0_t + port_attr->internal_mac_idx*step + port_attr->pcs_idx;

        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_codeErrCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[0] = value == 0 ? 0 : 1;
        p_table_id[0] = "SharedPcsSgmiiStatus.codeErrCnt:1";
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_sgmiiSyncStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[1] = value == 1 ? 0 : 1;
        p_table_id[1] = "SharedPcsSgmiiStatus.sgmiiSyncStatus:0";
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_anLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[2] = value == 1 ? 0 : 1;
        p_table_id[2] = "SharedPcsSgmiiStatus.anLinkStatus:0";
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_anegState0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        if(auto_neg_en && value != 6)
        {
            flag[3] = 1;
            p_table_id[3] = "SharedPcsSgmiiStatus.anegState:0";
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_100BASEFX_MODE)
    {
        step = SharedPcsFx1Status0_t - SharedPcsFx0Status0_t;
        tb_id = SharedPcsFx0Status0_t + port_attr->internal_mac_idx * step + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsFx0Status0_dbgFxLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[0] = value == 1 ? 0 : 1;
        p_table_id[0] = "SharedPcsFxStatus.dbgFxLinkStatus:0";
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE||port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE)
    {
        step = UsxgmiiPcsXfi1Status0_t - UsxgmiiPcsXfi0Status0_t;
        tb_id = UsxgmiiPcsXfi0Status0_t + port_attr->internal_mac_idx*step + port_attr->pcs_idx;
        if(port_attr->internal_mac_idx == 0)
        {
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_errBlockCnt0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[0] = value == 0 ? 0 : 1;
            p_table_id[0] = "UsxgmiiPcsXfiStatus.errBlockCnt:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_badBerCnt0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[1] = value == 0 ? 0 : 1;
            p_table_id[1] = "UsxgmiiPcsXfiStatus.badBerCnt:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_usxgmiiSyncStatus0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[2] = value == 1 ? 0 : 1;
            p_table_id[2] = "UsxgmiiPcsXfiStatus.usxgmiiSyncStatus:0";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_hiBer0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[3] = value == 0 ? 0 : 1;
            p_table_id[3] = "UsxgmiiPcsXfiStatus.hiBer:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_rxBlockLock0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[4] = value == 1 ? 0 : 1;
            p_table_id[4] = "UsxgmiiPcsXfiStatus.rxBlockLock:0";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_usxgmiiAnegState0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 6)
            {
                flag[5] = 1;
                p_table_id[5] = "UsxgmiiPcsXfiStatus.usxgmiiAnegState0:0";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_usxgmiiAnBlockLock0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 1)
            {
                flag[6] = 1;
                p_table_id[6] = "UsxgmiiPcsXfiStatus.usxgmiiAnBlockLock0:0";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_usxgmiiAnegErr0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 0)
            {
                flag[7] = 1;
                p_table_id[7] = "UsxgmiiPcsXfiStatus.usxgmiiAnegErr0:1";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi0Status0_usxgmiiLinkFailure0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 0)
            {
                flag[8] = 1;
                p_table_id[8] = "UsxgmiiPcsXfiStatus.usxgmiiLinkFailure0:1";
            }
        }
        else
        {
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_errBlockCnt1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[0] = value == 0 ? 0 : 1;
            p_table_id[0] = "UsxgmiiPcsXfiStatus.errBlockCnt:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_badBerCnt1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[1] = value == 0 ? 0 : 1;
            p_table_id[1] = "UsxgmiiPcsXfiStatus.badBerCnt:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_usxgmiiSyncStatus1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[2] = value == 1 ? 0 : 1;
            p_table_id[2] = "UsxgmiiPcsXfiStatus.usxgmiiSyncStatus:0";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_hiBer1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[3] = value == 0 ? 0 : 1;
            p_table_id[3] = "UsxgmiiPcsXfiStatus.hiBer:1";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_rxBlockLock1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[4] = value == 1 ? 0 : 1;
            p_table_id[4] = "UsxgmiiPcsXfiStatus.rxBlockLock:0";
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_usxgmiiAnegState1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 6)
            {
                flag[5] = 1;
                p_table_id[5] = "UsxgmiiPcsXfiStatus.usxgmiiAnegState0:0";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_usxgmiiAnBlockLock1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 1)
            {
                flag[6] = 1;
                p_table_id[6] = "UsxgmiiPcsXfiStatus.usxgmiiAnBlockLock0:0";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_usxgmiiAnegErr1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 0)
            {
                flag[7] = 1;
                p_table_id[7] = "UsxgmiiPcsXfiStatus.usxgmiiAnegErr0:1";
            }
            cmd = DRV_IOR(tb_id, UsxgmiiPcsXfi1Status0_usxgmiiLinkFailure1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            if(auto_neg_en && value != 0)
            {
                flag[8] = 1;
                p_table_id[8] = "UsxgmiiPcsXfiStatus.usxgmiiLinkFailure0:1";
            }
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
    {
        GET_QSGMII_TBL(port_attr->mac_id, port_attr->pcs_idx, tb_id);
        if(port_attr->mac_id == 0)
        {
            cmd = DRV_IOR(tb_id, QsgmiiPcsStatus00_syncStatus0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        }
        else
        {
            cmd = DRV_IOR(tb_id, QsgmiiPcsStatus20_syncStatus2_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        }
        flag[0] = value == 1 ? 0 : 1;
        p_table_id[0] = "QsgmiiPcsStatus.syncStatus:0";
        cmd = DRV_IOR(tb_id, QsgmiiPcsStatus00_anegState0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        if(auto_neg_en && value != 6)
        {
            flag[1] = 1;
            p_table_id[1] = "QsgmiiPcsStatus.anegState:0";
        }
        cmd = DRV_IOR(tb_id, QsgmiiPcsStatus00_anLinkFailure0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        if(auto_neg_en && value != 0)
        {
            flag[2] = 1;
            p_table_id[2] = "QsgmiiPcsStatus.anLinkFailure:1";
        }
        cmd = DRV_IOR(tb_id, QsgmiiPcsStatus00_anLinkStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        if(auto_neg_en && value != 1)
        {
            flag[3] = 1;
            p_table_id[3] = "QsgmiiPcsStatus.anLinkStatus:0";
        }
        tb_id = QsgmiiPcsCodeErrCnt0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, QsgmiiPcsCodeErrCnt0_codeErrCnt0_f+port_attr->internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[4] = value == 0 ? 0 : 1;
        p_table_id[4] = "QsgmiiPcsCodeErrCnt.codeErrCnt:1";
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_XAUI_MODE || port_attr->pcs_mode == CTC_CHIP_SERDES_DXAUI_MODE)
    {
        sal_memset(&pcs_status, 0, sizeof(SharedPcsXauiStatus0_m));
        tb_id = SharedPcsXauiStatus0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_status));
        sal_task_sleep(1000);
        sal_memset(&pcs_status, 0, sizeof(SharedPcsXauiStatus0_m));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_status));
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt0_f, &value_arr1[0], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt1_f, &value_arr1[1], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt2_f, &value_arr1[2], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_codeErrorCnt3_f, &value_arr1[3], &pcs_status);
        value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
        flag[0] = value == 0 ? 0 : 1;
        p_table_id[0] = "SharedPcsXauiStatus.codeErrorCnt:1";

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_xauiSyncStatus0_f, &value_arr1[0], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_xauiSyncStatus1_f, &value_arr1[1], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_xauiSyncStatus2_f, &value_arr1[2], &pcs_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_xauiSyncStatus3_f, &value_arr1[3], &pcs_status);
        value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
        flag[1] = value == 4 ? 0 : 1;
        p_table_id[1] = "SharedPcsXauiStatus.xauiSyncStatus:0";

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXauiStatus0_xauiEeeAlignStatus_f, &value, &pcs_status);
        flag[2] = value == 1 ? 0 : 1;
        p_table_id[2] = "SharedPcsXauiStatus.xauiEeeAlignStatus:0";

    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_XLG_MODE)
    {
        sal_memset(&xlg_status, 0, sizeof(SharedPcsXlgStatus0_m));
        tb_id = SharedPcsXlgStatus0_t + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xlg_status));
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt0_f, &value_arr1[0], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt1_f, &value_arr1[1], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt2_f, &value_arr1[2], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt3_f, &value_arr1[3], &xlg_status);
        value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
        flag[0] = value == 0 ? 0 : 1;
        p_table_id[0] = "SharedPcsXlgStatus.bipErrCnt:1";

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock0_f, &value_arr1[0], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock1_f, &value_arr1[1], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock2_f, &value_arr1[2], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock3_f, &value_arr1[3], &xlg_status);
        value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
        flag[1] = value == 4 ? 0 : 1;
        p_table_id[1] = "SharedPcsXlgStatus.rxAmLock:0";

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_alignStatus0_f, &value, &xlg_status);
        flag[2] = value == 1 ? 0 : 1;
        p_table_id[2] = "SharedPcsXlgStatus.alignStatus:0";

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane0_f, &value_arr1[0], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane1_f, &value_arr1[1], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane2_f, &value_arr1[2], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane3_f, &value_arr1[3], &xlg_status);
        for(loop = 0; loop < 4; loop ++)
        {
            for(loop1 = 0; loop1 < 4; loop1 ++)
            {
                if(loop == value_arr1[loop1])
                {
                    break;
                }
                if(loop1 == 3)
                {
                    flag2 = 1;
                }
            }
        }
        flag[3] = flag2 == 0 ? 0 : 1;
        p_table_id[3] = "SharedPcsXlgStatus.rxAmLockLane:0";

        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + port_attr->internal_mac_idx*step + port_attr->mii_idx;
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[5] = value == 0 ? 0 : 1;
        p_table_id[5] = "SharedMii0Status_dbgMiiRxFaultType:1";
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultState0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[6] = value == 0 ? 0 : 1;
        p_table_id[6] = "SharedMii0Status_dbgMiiRxFaultState:1";
        if(hss_type == SYS_DATAPATH_HSS_TYPE_15G && enable)
        {
            step = SharedPcsXgFecMon0_dbgXgFec1RxBlockLock_f - SharedPcsXgFecMon0_dbgXgFec0RxBlockLock_f;
            tb_id = SharedPcsXgFecMon0_t + port_attr->pcs_idx;
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0RxBlockLock_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[7] = value == 1 ? 0 : 1;
            p_table_id[7] = "SharedPcsXgFecMon0_dbgXgFec0RxBlockLock:0";

            step = SharedPcsXgFecMon0_dbgXgFec1UncoBlkCnt_f - SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f;
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt_f + (port_attr->internal_mac_idx%4)*step);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));
            flag[8] = value1 == 0 ? 0 : 1;
            p_table_id[8] = "SharedPcsXgFecMon0_dbgXgFec0UncoBlkCnt:1";
        }
        else if(hss_type == SYS_DATAPATH_HSS_TYPE_28G && enable)
        {
            step = XgFec2StatusSharedFec0_t - XgFec0StatusSharedFec0_t;
            tb_id = XgFec0StatusSharedFec0_t + (port_attr->pcs_idx - 6) + step * (port_attr->internal_mac_idx%4);
            cmd = DRV_IOR(tb_id, XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[7] = value == 1 ? 0 : 1;
            p_table_id[7] = "XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock:0";
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_XXVG_MODE)
    {
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        tb_id = SharedPcsXfi0Status0_t + port_attr->internal_mac_idx*step + port_attr->pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[0] = value == 0 ? 0 : 1;
        p_table_id[0] = "SharedPcsXfi0Status_errBlockCnt:1";
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_badBerCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        sal_task_sleep(1000);
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_badBerCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[1] = value == 0 ? 0 : 1;
        p_table_id[1] = "SharedPcsXfi0Status_badBerCnt:1";
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_hiBer0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[2] = value == 0 ? 0 : 1;
        p_table_id[2] = "SharedPcsXfi0Status_hiBer:1";
        if(enable == 0)
        {
            cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_rxBlockLock0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[3] = value == 1 ? 0 : 1;
            p_table_id[3] = "SharedPcsXfi0Status_rxBlockLock:0";
        }
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_xfiSyncStatus0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        flag[4] = value == 1 ? 0 : 1;
        p_table_id[4] = "SharedPcsXfi0Status_xfiSyncStatus0:0";
        if(enable)
        {
            tb_id = RsFec0StatusSharedFec0_t + (port_attr->pcs_idx-6);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value2));
            flag[5] = value2 == 0 ? 0 : 1;
            p_table_id[5] = "RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount:1";
        }
        if(hss_type == SYS_DATAPATH_HSS_TYPE_28G && enable)
        {
            step = XgFec2StatusSharedFec0_t - XgFec0StatusSharedFec0_t;
            tb_id = XgFec0StatusSharedFec0_t + (port_attr->pcs_idx - 6) + step * (port_attr->internal_mac_idx%4);
            cmd = DRV_IOR(tb_id, XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            flag[7] = value == 1 ? 0 : 1;
            p_table_id[7] = "XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock:0";
        }
    }
    else if(port_attr->pcs_mode == CTC_CHIP_SERDES_LG_MODE)
    {
        if (port_attr->internal_mac_idx == 0)
        {
            tb_id = SharedPcsXlgStatus0_t + port_attr->pcs_idx;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xlg_status));
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt0_f, &value_arr1[0], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt1_f, &value_arr1[1], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt2_f, &value_arr1[2], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt3_f, &value_arr1[3], &xlg_status);
            value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
            flag[0] = value == 0 ? 0 : 1;
            p_table_id[0] = "SharedPcsXlgStatus.bipErrCnt:1";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock0_f, &value_arr1[0], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock1_f, &value_arr1[1], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock2_f, &value_arr1[2], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLock3_f, &value_arr1[3], &xlg_status);
            value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
            flag[1] = value == 4 ? 0 : 1;
            p_table_id[1] = "SharedPcsXlgStatus.rxAmLock:0";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_alignStatus0_f, &value, &xlg_status);
            flag[2] = value == 1 ? 0 : 1;
            p_table_id[2] = "SharedPcsXlgStatus.alignStatus:0";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane0_f, &value_arr1[0], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane1_f, &value_arr1[1], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane2_f, &value_arr1[2], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_rxAmLockLane3_f, &value_arr1[3], &xlg_status);
        }
        else
        {
            tb_id = SharedPcsLgStatus0_t + port_attr->pcs_idx;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lg_status));
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxBlockLock0_f, &value_arr1[0], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxBlockLock1_f, &value_arr1[1], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxBlockLock2_f, &value_arr1[2], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxBlockLock3_f, &value_arr1[3], &lg_status);
            value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
            flag[0] = value == 4 ? 0 : 1;
            p_table_id[0] = "SharedPcsLgStatus.lgPcs1RxBlockLock:0";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt0_f, &value_arr1[0], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt1_f, &value_arr1[1], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt2_f, &value_arr1[2], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt3_f, &value_arr1[3], &lg_status);
            value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
            flag[1] = value == 0 ? 0 : 1;
            p_table_id[1] = "SharedPcsLgStatus.lgPcs1BipErrCnt:1";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLock0_f, &value_arr1[0], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLock1_f, &value_arr1[1], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLock2_f, &value_arr1[2], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLock3_f, &value_arr1[3], &lg_status);
            value = value_arr1[0]+value_arr1[1]+value_arr1[2]+value_arr1[3];
            flag[2] = value == 4 ? 0 : 1;
            p_table_id[2] = "SharedPcsLgStatus.lgPcs1RxAmLock:0";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1AlignStatus_f, &value, &lg_status);
            flag[3] = value == 1 ? 0 : 1;
            p_table_id[3] = "SharedPcsLgStatus.lgPcs1AlignStatus:0";

            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLockLane0_f, &value_arr1[0], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLockLane1_f, &value_arr1[1], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLockLane2_f, &value_arr1[2], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1RxAmLockLane3_f, &value_arr1[3], &lg_status);
        }
        for(loop = 0; loop < 4; loop ++)
        {
            for(loop1 = 0; loop1 < 4; loop1 ++)
            {
                if(loop == value_arr1[loop1])
                {
                    break;
                }
                if(loop1 == 3)
                {
                    flag2 = 1;
                }
            }
        }
        flag[4] = flag2 == 0 ? 0 : 1;
        p_table_id[4] = port_attr->internal_mac_idx == 0 ? "SharedPcsXlgStatus.rxAmLockLane:0" : "SharedPcsLgStatus.lgPcs1RxAmLockLane:0";
        if(enable)
        {
            value1 = port_attr->internal_mac_idx == 0 ? 0 : 2;
            step = RsFec1StatusSharedFec0_t - RsFec0StatusSharedFec0_t;
            tb_id = RsFec0StatusSharedFec0_t + (port_attr->pcs_idx-6) + step*value1;
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value2));
            flag[5] = value2 == 0 ? 0 : 1;
            p_table_id[5] = "RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount:1";

            tb_id = RsFec0StatusSharedFec0_t + (port_attr->pcs_idx-6) + step*(value1+1);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            sal_task_sleep(1000);
            cmd = DRV_IOR(tb_id, RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value2));
            flag[6] = value2 == 0 ? 0 : 1;
            p_table_id[6] = "RsFec0StatusSharedFec0_dbgRsFec0UnCoCwCount:1";
        }
        else if(hss_type == SYS_DATAPATH_HSS_TYPE_28G && enable)
        {
            for(loop = 0; loop < 4; loop ++)
            {
                value1 = port_attr->internal_mac_idx == 0 ? 0 : 4;
                step = XgFec1StatusSharedFec0_t - XgFec0StatusSharedFec0_t;
                tb_id = XgFec0StatusSharedFec0_t + (port_attr->pcs_idx - 6) + step * (value1+loop);
                cmd = DRV_IOR(tb_id, XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                flag[5+loop] = value == 1 ? 0 : 1;
                p_table_id[5+loop] = "XgFec0StatusSharedFec0_dbgXgFec0RxBlockLock:0";
            }
        }
    }
    for(loop = 0; loop < 9; loop ++)
    {
        if(flag[loop] == 1 && flag1 == 0)
        {
            flag1 = 1;
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"PCS Status check      --------------   Fail\n");
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Correlative Status List:\n");
        }
        if(flag[loop] == 1)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %s\n",p_table_id[loop]);
        }
    }
    if(flag1 == 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"PCS Status check      --------------   Pass\n");
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_mac_self_checking(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    char* mac_en = NULL;
    char* auto_neg = NULL;
    char* link_up = "-";
    uint32 value_link_up = FALSE;
    uint32 value_mac_en = FALSE;
    uint32 value_32 = 0;
    uint32 value_64 = 0;
    char value_c[32] = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;
    char* p_fault = NULL;
    char* auto_neg_mod[] = {"1000BASE-X", "SGMII master", "SGMII slaver"};
    char* ffe_mod[] = {"3AP  ", "user define  "};
    ctc_chip_serdes_loopback_t loopback_inter = {0};
    ctc_chip_serdes_loopback_t loopback_exter0 = {0};
    char* p_loopback_mode = NULL;
    uint32 temperature = 0;
    uint16 coefficient[SYS_USW_CHIP_FFE_PARAM_NUM] = {0};
    uint8 status = 0;
    uint8 channel = 0;
    uint16 sigdet = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    int32 ret = 0;
    uint8 enable[4] = {0};
    uint16 chip_port;
    uint8 serdes_num = 0;
    uint16 serdes_id = 0;
    uint8 serdes_num_tmp = 0;
    uint8 hss_id = 0;
    uint8 hss_type = 0;
    uint8 lane_id[4] = {0};
    uint8 step = 0;
    uint8 auto_neg_en = 0;
    ctc_mac_stats_t mac_stats;

    SYS_MAC_INIT_CHECK();
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Mac Information:\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"------------------------------------------------------------------------------------------\n");
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    /* get serdes num */
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - port_attr->slice_id*256;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, port_attr->slice_id, &serdes_id, &serdes_num));
    /* get mac en */
    CTC_ERROR_RETURN(sys_usw_mac_get_mac_en(lchip, gport, &value_mac_en));
    mac_en = value_mac_en ? "Enable" : "Disable";
    /* get mac link up status */
    CTC_ERROR_RETURN(sys_tsingma_mac_get_link_up(lchip, gport, &value_link_up, 0));
    link_up = value_link_up ? "Up" : "Down";
    /* get loopback status */
    loopback_inter.serdes_id = serdes_id;
    CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, &loopback_inter));
    loopback_exter0.serdes_id = serdes_id;
    loopback_exter0.mode = 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, &loopback_exter0));

    if(loopback_inter.enable)
    {
        p_loopback_mode = "internal";
    }
    else if(loopback_exter0.enable)
    {
        p_loopback_mode = "external";
    }
    else
    {
        p_loopback_mode = "-";
    }
    /* get asic temperature */
    CTC_ERROR_RETURN(sys_tsingma_peri_get_chip_sensor(lchip, CTC_CHIP_SENSOR_TEMP, &temperature));

    CTC_ERROR_RETURN(sys_usw_get_channel_by_port(lchip, gport, &channel));
    /* get port speed */
    ret = sys_tsingma_mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, (uint32*)&value_32);
    if (ret < 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-7s", "-");
    }
    else
    {
        sal_memset(value_c, 0, sizeof(value_c));
        if (CTC_PORT_SPEED_1G == value_32)
        {
            sal_memcpy(value_c, "1G", sal_strlen("1G"));
        }
        else if (CTC_PORT_SPEED_100M == value_32)
        {
            sal_memcpy(value_c, "100M", sal_strlen("100M"));
        }
        else if (CTC_PORT_SPEED_10M == value_32)
        {
            sal_memcpy(value_c, "10M", sal_strlen("10M"));
        }
        else if (CTC_PORT_SPEED_2G5 == value_32)
        {
            sal_memcpy(value_c, "2.5G", sal_strlen("2.5G"));
        }
        else if (CTC_PORT_SPEED_10G == value_32)
        {
            sal_memcpy(value_c, "10G", sal_strlen("10G"));
        }
        else if (CTC_PORT_SPEED_20G == value_32)
        {
            sal_memcpy(value_c, "20G", sal_strlen("20G"));
        }
        else if (CTC_PORT_SPEED_40G == value_32)
        {
            sal_memcpy(value_c, "40G", sal_strlen("10G"));
        }
        else if (CTC_PORT_SPEED_100G == value_32)
        {
            sal_memcpy(value_c, "100G", sal_strlen("100G"));
        }
        else if (CTC_PORT_SPEED_5G == value_32)
        {
            sal_memcpy(value_c, "5G", sal_strlen("5G"));
        }
        else if (CTC_PORT_SPEED_25G == value_32)
        {
            sal_memcpy(value_c, "25G", sal_strlen("25G"));
        }
        else if (CTC_PORT_SPEED_50G == value_32)
        {
            sal_memcpy(value_c, "50G", sal_strlen("50G"));
        }
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %-12d %s %-15s %s %-16s %s %-12d\n",
                          "Mac ID   :",(port_attr->mac_id+port_attr->slice_id*64), "Mac-En  :",mac_en, "Link    :",link_up, "Channel ID :",channel);
    /* get Auto-Neg mode */
    ret = sys_tsingma_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &value_32);
    auto_neg_en = value_32;
    if (ret < 0)
    {
        auto_neg = "-";
    }
    else if(value_32)
    {
        sys_tsingma_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, &value_32);
        auto_neg =  ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode))?
                     auto_neg_mod[value_32] : "TURE";
    }
    else
    {
        auto_neg = "FALSE";
    }
    if(serdes_num == 1)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %-12d %s %-15s %s %-16s %s %-12s\n",
                          "Serdes ID:",serdes_id, "Auto-Neg:",auto_neg, "Loopback:",p_loopback_mode, "Speed      :",value_c);
    }
    else if(serdes_num == 4)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %d/%d/%d/%-3d %s %-15s %s %-16s %s %-12s\n",
                          "Serdes ID:",serdes_id,serdes_id+1,serdes_id+2,serdes_id+3, "Auto-Neg:",auto_neg,"Loopback:",p_loopback_mode, "Speed      :",value_c);
    }
    else if(serdes_num == 2)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %d/%-6d %s %-15s %s %-16s %s %-12s\n",
                          "Serdes ID:",serdes_id,serdes_id+1, "Auto-Neg:",auto_neg,"Loopback:",p_loopback_mode, "Speed      :",value_c);
    }
    /* get serdes sigdet */
    sigdet = sys_tsingma_datapath_get_serdes_sigdet(lchip, serdes_id);
    /* get phrb_status */
    for(serdes_num_tmp = 0; serdes_num_tmp < serdes_num; serdes_num_tmp ++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_prbs_status(lchip, serdes_id+serdes_num_tmp,&enable[serdes_num_tmp],&hss_id,&hss_type,&lane_id[serdes_num_tmp]));
    }

    if(port_attr->pcs_mode != CTC_CHIP_SERDES_SGMII_MODE && port_attr->pcs_mode != CTC_CHIP_SERDES_QSGMII_MODE &&
       port_attr->pcs_mode != CTC_CHIP_SERDES_2DOT5G_MODE && port_attr->pcs_mode != CTC_CHIP_SERDES_100BASEFX_MODE )
    {
        step = SharedMii1Status0_t - SharedMii0Status0_t;
        tb_id = SharedMii0Status0_t + port_attr->internal_mac_idx*step + port_attr->mii_idx;
        cmd = DRV_IOR(tb_id, SharedMii0Status0_dbgMiiRxFaultType0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        p_fault = (value == 0) ? "No":"Yes";
    }
    else
    {
        p_fault = "-";
    }
    if(serdes_num == 1)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %-12d %s %-15d %s %-16s %s %-12d\n",
                          "Sigdet   :",sigdet, "PRBS    :",enable[0], "Fault   :",p_fault, "Temperature:",temperature);
    }
    else if(serdes_num == 4)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %-12d %s %d/%d/%d/%-9d %s %-16s %s %-12d\n",
                          "Sigdet   :",sigdet, "PRBS    :",enable[0],enable[1],enable[2],enable[3], "Fault   :",p_fault, "Temperature:",temperature);
    }
    else if(serdes_num == 2)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %-12d %s %d/%-9d %s %-16s %s %-12d\n",
                          "Sigdet   :",sigdet, "PRBS    :",enable[0],enable[1], "Fault   :",p_fault, "Temperature:",temperature);
    }
    /* get ffe mode and value */
    for(serdes_num_tmp = 0; serdes_num_tmp < serdes_num; serdes_num_tmp ++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe(lchip, serdes_id +serdes_num_tmp,coefficient,CTC_CHIP_SERDES_FFE_MODE_3AP,&status));
        if(status == 1 && serdes_num_tmp == 0)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %s/%d/%d/%d/%d",
                          "FFE       :",ffe_mod[0], coefficient[0],coefficient[1], coefficient[2],coefficient[3]);
        }
        else if(status == 1)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %d/%d/%d/%d", coefficient[0],coefficient[1], coefficient[2],coefficient[3]);
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe(lchip, serdes_id +serdes_num_tmp,coefficient,CTC_CHIP_SERDES_FFE_MODE_DEFINE,&status));
        if(status == 1 && serdes_num_tmp == 0)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s %s/%d/%d/%d/%d",
                          "FFE      :",ffe_mod[1], coefficient[0],coefficient[1], coefficient[2],coefficient[3]);
        }
        else if(status == 1)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  %d/%d/%d/%d", coefficient[0],coefficient[1], coefficient[2],coefficient[3]);
        }
    }
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"------------------------------------------------------------------------------------------\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Check  Process:\n");
    CTC_ERROR_RETURN(_sys_tsingma_mac_module_reset_check(lchip, port_attr));
    CTC_ERROR_RETURN(_sys_tsingma_mac_hss_cs_macrco_check(lchip, lane_id, hss_id,hss_type,serdes_num));
    CTC_ERROR_RETURN(_sys_tsingma_mac_pcs_status_check(lchip, port_attr, lport,hss_type,auto_neg_en));

    sal_memset(&mac_stats, 0, sizeof(ctc_mac_stats_t));
    mac_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    CTC_ERROR_RETURN(sys_usw_mac_stats_get_rx_stats(lchip, gport, &mac_stats));
    if(mac_stats.u.stats_detail.stats.rx_stats.fcs_error_bytes == 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RX Status check       --------------   Pass\n");
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"RX Status check       --------------   Fail\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Crc Error,SI problem\n");
    }

    sal_memset(&mac_stats, 0, sizeof(ctc_mac_stats_t));
    mac_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    CTC_ERROR_RETURN(sys_usw_mac_stats_get_tx_stats(lchip, gport, &mac_stats));
    value_64 = mac_stats.u.stats_detail.stats.tx_stats.good_bcast_bytes + mac_stats.u.stats_detail.stats.tx_stats.good_mcast_bytes
               + mac_stats.u.stats_detail.stats.tx_stats.good_ucast_bytes + mac_stats.u.stats_detail.stats.tx_stats.good_pause_bytes
               + mac_stats.u.stats_detail.stats.tx_stats.good_control_bytes + mac_stats.u.stats_detail.stats.tx_stats.bytes_63
               + mac_stats.u.stats_detail.stats.tx_stats.bytes_64 + mac_stats.u.stats_detail.stats.tx_stats.bytes_65_to_127
               + mac_stats.u.stats_detail.stats.tx_stats.bytes_128_to_255 + mac_stats.u.stats_detail.stats.tx_stats.bytes_256_to_511
               + mac_stats.u.stats_detail.stats.tx_stats.bytes_512_to_1023 + mac_stats.u.stats_detail.stats.tx_stats.bytes_1024_to_1518
               + mac_stats.u.stats_detail.stats.tx_stats.bytes_1519 + mac_stats.u.stats_detail.stats.tx_stats.jumbo_bytes
               + mac_stats.u.stats_detail.stats.tx_stats.fcs_error_bytes + mac_stats.u.stats_detail.stats.tx_stats.bytes_1519_to_2047;
    if(mac_stats.u.stats_detail.stats.tx_stats.mac_underrun_bytes != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"TX Status check       --------------   Fail\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  There is MAC underrun packet\n");
    }
    else if(value_64 == 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"TX Status check       --------------   Fail\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  NetTx send no packet to Mac\n");
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"TX Status check       --------------   Pass\n");
    }
    return CTC_E_NONE;
}



