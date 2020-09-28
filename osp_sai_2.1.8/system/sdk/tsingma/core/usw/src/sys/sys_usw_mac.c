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

#include "sys_usw_common.h"
#include "sys_usw_datapath.h"
#include "sys_usw_mac.h"
#include "sys_usw_chip.h"
#include "sys_usw_peri.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_dmps.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_qos.h"
#include "sys_usw_port.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_mcu.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define CL72_FSM_TR_FAIL         (1<<7)
#define CL72_FSM_TR_ACT          (1<<6)
#define CL72_FSM_TR_COMP         (1<<4)

#define SYS_MAC_GET_HSS_TYPE(id)    (DRV_IS_DUET2(lchip) ? \
    (((id >= 12) && (id <= 15)) || ((id >= 28) && (id <= 31)) || ((id >= 44) && (id <= 47)) || ((id >=60) && (id <= 63))) :\
    (((id >= 44) && (id <= 47)) || ((id >=60) && (id <= 63))))

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

sys_mac_master_t* p_usw_mac_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

extern int32
sys_usw_datapath_dynamic_switch_get_info(uint8 lchip, uint16 lport, uint8 serdes_id, uint8 src_mode, uint8 mode, uint8 flag, sys_datapath_dynamic_switch_attr_t *attr);

extern uint32
sys_usw_datapath_get_serdes_no_random_data_flag(uint8 lchip, uint8 serdes_id);

extern int32
_sys_usw_datapath_set_nettx_portmode(uint8 lchip, uint8 chan_id);

int32
_sys_usw_mac_get_fec_en(uint8 lchip, uint16 lport, uint32* p_value);

int32
sys_usw_mac_set_3ap_training_en(uint8 lchip, uint32 gport, uint32 enable);

extern
int32 sys_usw_qos_set_fc_default_profile(uint8 lchip, uint32 gport);

int32
sys_usw_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void* p_data);

int32
sys_usw_mac_set_link_intr(uint8 lchip, uint32 gport, uint8 enable);

int32
_sys_usw_mac_get_unidir_en(uint8 lchip, uint16 lport, uint32* p_value);

int32
sys_usw_mac_get_fec_en(uint8 lchip, uint32 gport, uint32 *p_value);

STATIC int32
_sys_usw_mac_pcs_rx_rst(uint8 lchip, uint16 lport, uint32 type, uint8 reset);

extern int32
sys_usw_datapath_get_serdes_link_stuck(uint8 lchip, uint8 serdes_id, uint32 *p_link_stuck);

int32
_sys_usw_mac_get_cl73_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value);
/****************************************************************************
 *
* Function
*
*****************************************************************************/

#define  __MAC_FRAME__

int32
_sys_usw_mac_flow_ctrl_cfg(uint8 lchip, uint16 lport)
{
    uint32 cmd             = 0;
    uint32 tbl_id          = 0;
    uint32 field_id        = 0;
    uint32 pause_dec_value = 0;
    uint32 pause_pulse_sel = 0;
    uint8  step            = 0;
    uint8  inner_mac_id    = 0;
    uint8  qm_id           = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;
    Sgmac0TxPauseCfg0_m tx_pause_cfg;

    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    sal_memset(&tx_pause_cfg, 0, sizeof(Sgmac0TxPauseCfg0_m));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &p_port_cap));

    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", p_port_cap->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    switch (p_port_cap->speed_mode)
    {
        case CTC_PORT_SPEED_2G5:
            pause_dec_value = 5;
            pause_pulse_sel = 1;
            break;
        case CTC_PORT_SPEED_10G:
            pause_dec_value = 10;
            break;
        case CTC_PORT_SPEED_20G:
            pause_dec_value = 20;
            break;
        case CTC_PORT_SPEED_40G:
            pause_dec_value = 40;
            break;
        case CTC_PORT_SPEED_100G:
            pause_dec_value = 100;
            break;
        case CTC_PORT_SPEED_5G:
            pause_dec_value = 5;
            break;
        case CTC_PORT_SPEED_25G:
            pause_dec_value = 25;
            break;
        case CTC_PORT_SPEED_50G:
            pause_dec_value = 50;
            break;
        default:
            pause_dec_value = 1;
            break;
    }

    if(DRV_IS_TSINGMA(lchip))
    {
        inner_mac_id = p_port_cap->tbl_id % 4;
        qm_id        = p_port_cap->tbl_id / 4;
    }
    else
    {
        inner_mac_id = p_port_cap->mac_id % 4;
        qm_id        = p_port_cap->mac_id / 4;
    }

    step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
    tbl_id = Sgmac0RxPauseCfg0_t + qm_id + (inner_mac_id*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
    SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPauseTimerDecValue_f, &rx_pause_cfg, pause_dec_value);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));

    step = Sgmac1TxPauseCfg0_t - Sgmac0TxPauseCfg0_t;
    tbl_id = Sgmac0TxPauseCfg0_t + qm_id + (inner_mac_id*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));
    SetSgmac0TxPauseCfg0(V, cfgSgmac0TxPauseTimerDecValue_f, &tx_pause_cfg, pause_dec_value);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));

    if (CTC_PORT_SPEED_2G5 == p_port_cap->speed_mode)
    {
        tbl_id = QuadSgmacCfg0_t + qm_id;
        step = QuadSgmacCfg0_cfgSgmac1PausePulseSel_f - QuadSgmacCfg0_cfgSgmac0PausePulseSel_f;
        field_id = QuadSgmacCfg0_cfgSgmac0PausePulseSel_f + inner_mac_id * step;
        cmd = DRV_IOW(tbl_id, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pause_pulse_sel));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_flow_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8  mac_id  = 0;
    uint32 tbl_id  = 0;
    uint8  step    = 0;
    uint16 lport = 0;
    int32  ret = CTC_E_NONE;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;
    RefDivSgmacPauseTimerPulse_m pause_pulse;

    /*Reset the pulse for pause timer*/
    sal_memset(&pause_pulse, 0, sizeof(RefDivSgmacPauseTimerPulse_m));
    cmd = DRV_IOR(RefDivSgmacPauseTimerPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pause_pulse));
    SetRefDivSgmacPauseTimerPulse(V, cfgRefDivSgmacPauseTimer0Pulse_f, &pause_pulse, 0x4F00);
    SetRefDivSgmacPauseTimerPulse(V, cfgResetDivSgmacPauseTimer0Pulse_f, &pause_pulse, 0);
    SetRefDivSgmacPauseTimerPulse(V, cfgRefDivSgmacPauseTimer1Pulse_f, &pause_pulse, 0x9F00);/*for 2.5G*/
    SetRefDivSgmacPauseTimerPulse(V, cfgResetDivSgmacPauseTimer1Pulse_f, &pause_pulse, 0);
    SetRefDivSgmacPauseTimerPulse(V, cfgRefDivSgmacPauseTimer2Pulse_f, &pause_pulse, 0x4F00);
    SetRefDivSgmacPauseTimerPulse(V, cfgResetDivSgmacPauseTimer2Pulse_f, &pause_pulse, 0);
    SetRefDivSgmacPauseTimerPulse(V, cfgRefDivSgmacPauseTimer3Pulse_f, &pause_pulse, 0x4F00);
    SetRefDivSgmacPauseTimerPulse(V, cfgResetDivSgmacPauseTimer3Pulse_f, &pause_pulse, 0);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pause_pulse));

    /*pause frame length could >=64*/
    cmd = DRV_IOW(NetRxCtl_t, NetRxCtl_cfgNormalAndPfc64Bytes_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*Default disable flow ctl*/
    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    for (mac_id = 0; mac_id < 72; mac_id++)
    {
        step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
        tbl_id = Sgmac0RxPauseCfg0_t + (mac_id / 4) + ((mac_id % 4)*step);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
        SetSgmac0RxPauseCfg0(V, cfgSgmac0RxNormPauseEn_f, &rx_pause_cfg, 0);
        SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPfcPriorityEn_f, &rx_pause_cfg, 0);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
    }

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
    {
        ret = _sys_usw_mac_flow_ctrl_cfg(lchip, lport);
        if (CTC_E_NONE != ret)
        {
            continue;
        }
    }

    return CTC_E_NONE;
}

#define  __MAC__DMPS__

/*this interface is used to get network port capability, lport is sys layer port */
int32
sys_usw_mac_get_port_capability(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t** p_port)
{
    uint16 drv_lport = 0;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* p_port_attr = NULL;

    drv_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
    slice_id = (drv_lport >= MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM))?1:0;
    chip_port = drv_lport - slice_id*MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM);

    p_port_attr = sys_usw_datapath_get_port_capability(lchip, chip_port, slice_id);
    if (p_port_attr == NULL)
    {
        return CTC_E_INVALID_CONFIG;
    }

    *p_port = p_port_attr;
    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_nettxwriteen_en(uint8 lchip, uint16 lport, uint32 enable)
{
    uint32 nettx_write_en[2] = {0};
    uint8  write_en = 0;
    uint32 cmd       = 0;
    NetTxWriteEn_m        nettx_cfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&nettx_cfg, 0, sizeof(NetTxWriteEn_m));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(NetTxWriteEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_cfg));
    GetNetTxWriteEn(A, writeEnPort_f, &nettx_cfg, nettx_write_en);
    if (enable)
    {
        if (port_attr->mac_id < 32)
        {
            write_en = (nettx_write_en[0] >> (port_attr->mac_id)) & 0x1;
            if (!write_en)
            {
                nettx_write_en[0] |= (1 << port_attr->mac_id);
            }
        }
        else
        {
            write_en = (nettx_write_en[1] >> (port_attr->mac_id-32)) & 0x1;
            if (!write_en)
            {
                nettx_write_en[1] |= (1 << (port_attr->mac_id - 32));
            }
        }
    }
    else
    {
        /* NetTxWriteEn */
        GetNetTxWriteEn(A, writeEnPort_f, &nettx_cfg, nettx_write_en);
        if (port_attr->mac_id < 32)
        {
            nettx_write_en[0] &= ~(1 << port_attr->mac_id);
        }
        else
        {
            nettx_write_en[1] &= ~(1 << (port_attr->mac_id - 32));
        }
    }

    SetNetTxWriteEn(A, writeEnPort_f, &nettx_cfg, nettx_write_en);
    cmd = DRV_IOW(NetTxWriteEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_cfg));

    return CTC_E_NONE;
}

#define   __MAC_CONFIG__
STATIC int32
_sys_usw_mac_set_sgmac_xaui_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;
    SharedPcsDsfCfg0_m shared_dsf_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;

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
    value = 2;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Shared Mii*/
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    value = 6;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f + internal_mac_idx, &value, &shared_pcs_cfg);
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
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 8;
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

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_dxaui_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;
    SharedPcsDsfCfg0_m shared_dsf_cfg;

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
    value = 2;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);

    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);

    value = 6;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f + internal_mac_idx, &value, &shared_pcs_cfg);
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
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 8;
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &shared_dsf_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_xfi_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* cfg Quad sgmac*/
    value = 0;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode1_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiRXauiMode0_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);

    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));



    return CTC_E_NONE;
}
STATIC int32
_sys_usw_mac_set_sgmac_xlg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;
    SharedPcsDsfCfg0_m shared_dsf_cfg;

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
    value = 2;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);

    value = 2;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    tbl_id = SharedMii0Cfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    value = 8;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

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
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    value = 31;
    tbl_id = SharedPcsDsfCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsDsfCfg0_cfgDsfDepth0_f, &value, &shared_dsf_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_dsf_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_sgmii_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 speed_mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    speed_mode       = port_attr->speed_mode;
    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    /* cfg Quad sgmac*/
    value = 0;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    if (CTC_PORT_SPEED_2G5 == port_attr->speed_mode)
    {
        value = 0x6;
    }
    else
    {
        value = 0x4;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    if (speed_mode == CTC_PORT_SPEED_2G5)
    {
        value = 3;
    }
    else if (speed_mode == CTC_PORT_SPEED_1G)
    {
        value = 2;
    }
    else if (speed_mode == CTC_PORT_SPEED_100M)
    {
        value = 1;
    }
    else if (speed_mode == CTC_PORT_SPEED_10M)
    {
        value = 0;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);

    if ((speed_mode == CTC_PORT_SPEED_2G5) || (speed_mode == CTC_PORT_SPEED_1G))
    {
        value = 0;
    }
    else if (speed_mode == CTC_PORT_SPEED_100M)
    {
        value = 9;
    }
    else if (speed_mode == CTC_PORT_SPEED_10M)
    {
        value = 99;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
    tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOW(tbl_id, SharedPcsSgmii0Cfg0_ignoreLinkFailure0_f);
    value = 1;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_qsgmii_config(uint8 lchip, uint16 lport, uint8 is_init)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 speed_mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    QsgmiiPcsSoftRst0_m pcs_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    speed_mode       = port_attr->speed_mode;
    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
   /* cfg Quad sgmac*/
    value = 0;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiTxIpgDelInterval_f, &value, &mii_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    /* cfg per Share Mii*/
    value = 0;
    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxEvenIgnore0_f, &value, &mii_per_cfg);
    value = 0x4;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    if (speed_mode == CTC_PORT_SPEED_1G)
    {
        value = 2;
    }
    else if (speed_mode == CTC_PORT_SPEED_100M)
    {
        value = 1;
    }
    else if (speed_mode == CTC_PORT_SPEED_10M)
    {
        value = 0;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);

    if (speed_mode == CTC_PORT_SPEED_1G)
    {
        value = 0;
    }
    else if (speed_mode == CTC_PORT_SPEED_100M)
    {
        value = 9;
    }
    else if (speed_mode == CTC_PORT_SPEED_10M)
    {
        value = 99;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* Toggle PCS Tx and PMA RX reset */
    if (is_init && (0 == port_attr->mac_id%4))
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

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_usxgmii_config(uint8 lchip, uint16 lport, uint8 is_init)
{
    uint32 value    = 0;
    uint32 value1    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 pcs_mode = 0;
    uint8 speed_mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    UsxgmiiPcsXfi0Cfg0_m xfi_cfg;
    UsxgmiiPcsPortNumCtl0_m pcs_cfg;
    UsxgmiiPcsSoftRst0_m    pcs_rst;

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
    value = 0;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);
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
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);

    if (speed_mode == CTC_PORT_SPEED_10G)
    {
        value = 5;
        value1 = 0xb;
    }
    else if (speed_mode == CTC_PORT_SPEED_5G)
    {
        value = 4;
        value1 = 4;
    }
    else if (speed_mode == CTC_PORT_SPEED_2G5)
    {
        value = 3;
        value1 = 2;
    }
    else if (speed_mode == CTC_PORT_SPEED_1G)
    {
        value = 2;
        value1 = 2;
    }
    else if (speed_mode == CTC_PORT_SPEED_100M)
    {
        value = 1;
        value1 = 2;
    }
    else if (speed_mode == CTC_PORT_SPEED_10M)
    {
        value = 0;
        value1 = 2;
    }
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value1, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    if (pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
    {
        if (speed_mode == CTC_PORT_SPEED_10G)
        {
            value = 0;
        }
        else if (speed_mode == CTC_PORT_SPEED_5G)
        {
            value = 1;
        }
        else if (speed_mode == CTC_PORT_SPEED_2G5)
        {
            value = 3;
        }
        else if (speed_mode == CTC_PORT_SPEED_1G)
        {
            value = 9;
        }
        else if (speed_mode == CTC_PORT_SPEED_100M)
        {
            value = 99;
        }
        else if (speed_mode == CTC_PORT_SPEED_10M)
        {
            value = 999;
        }

        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    }
    else if (pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE)
    {
        if (speed_mode == CTC_PORT_SPEED_2G5)
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
        }
        else if (speed_mode == CTC_PORT_SPEED_1G)
        {
            value = 0x21;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
        }
        else if (speed_mode == CTC_PORT_SPEED_100M)
        {
            value = 24;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
        }
        else if (speed_mode == CTC_PORT_SPEED_10M)
        {
            value = 249;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
        }
    }
    else if (pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE)
    {
        if (speed_mode == CTC_PORT_SPEED_5G)
        {
            value = 0;
        }
        else if (speed_mode == CTC_PORT_SPEED_2G5)
        {
            value = 1;
        }
        else if (speed_mode == CTC_PORT_SPEED_1G)
        {
            value = 4;
        }
        else if (speed_mode == CTC_PORT_SPEED_100M)
        {
            value = 49;
        }
        else if (speed_mode == CTC_PORT_SPEED_10M)
        {
            value = 499;
        }
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"error port pcs mode: %d \n", pcs_mode);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    step = UsxgmiiPcsXfi1Cfg0_t - UsxgmiiPcsXfi0Cfg0_t;
    tbl_id = UsxgmiiPcsXfi0Cfg0_t + internal_mac_idx*step + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));
    value = 1;
    field_id = UsxgmiiPcsXfi0Cfg0_usxgmiiEnable0_f;
    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &xfi_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &xfi_cfg));

    /* cfg UsxgmiiPcsPortNumCtl and UsxgmiiPcsXfi0Cfg*/
    tbl_id = UsxgmiiPcsPortNumCtl0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));
    field_id = UsxgmiiPcsPortNumCtl0_portNumCfg_f;
    if (pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
    {
        value = 0;
    }
    else if (pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE)
    {
        value = 3;
    }
    else if (pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE)
    {
        value = 1;
    }
    DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_cfg));

    /* Toggle PCS Tx/RX reset */
    if (is_init && (0 == port_attr->mac_id%4))
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


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_xxvg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d\n", port_attr->slice_id, port_attr->mac_id);

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    /* cfg Quad sgmac*/
    value = 0;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    /* cfg sgmac Rx/Tx*/
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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    value = 7;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f + internal_mac_idx, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f + internal_mac_idx, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_lg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 index    = 0;
    uint8 lg_index = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;
    SharedPcsLgCfg0_m pcs_lg_cfg;
    SharedPcsSerdes0Cfg0_m shared_pcs_serdes_cfg;

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
    value = 7;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);

    value = 7;
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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    value = 9;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 60;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_per_cfg));

    /* cfg SharedPcsCfg */
    value = 0;
    tbl_id = SharedPcsCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx1_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn0_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn1_f, &value, &shared_pcs_cfg);
    }
    else
    {
        lg_index = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xxvgMode3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_fxMode3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeRx3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_sgmiiModeTx3_f, &value, &shared_pcs_cfg);

        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn2_f, &value, &shared_pcs_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_unidirectionEn3_f, &value, &shared_pcs_cfg);
    }

    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_rxauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xauiMode_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_xlgMode_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f + lg_index, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

    if (1 == port_attr->flag)
    {
        tbl_id = SharedPcsLgCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_lg_cfg));
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsLgCfg0_lgPcsLane0And3SwapEn_f, &value, &pcs_lg_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &pcs_lg_cfg));

        tbl_id = SharedPcsSerdes0Cfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value, &shared_pcs_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));

        tbl_id = SharedPcsSerdes2Cfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value, &shared_pcs_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_serdes_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_cg_config(uint8 lchip, uint16 lport)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint8 index    = 0;
    uint8 sgmac_idx = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QuadSgmacCfg0_m mac_cfg;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    Sgmac0TxCfg0_m  mac_tx_cfg;
    SharedMiiCfg0_m mii_cfg;
    SharedMii0Cfg0_m mii_per_cfg;
    SharedPcsCfg0_m shared_pcs_cfg;

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
    value = 2;
    tbl_id = QuadSgmacCfg0_t + sgmac_idx;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacRxBufMode_f, &value, &mac_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, QuadSgmacCfg0_cfgQuadSgmacTxBufMode_f, &value, &mac_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_cfg));

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
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0TxCfg0_cfgSgmac0TxWaitCaptureTs_f, &value, &mac_tx_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_tx_cfg));

    /* cfg Share Mii*/
    value = 0;
    tbl_id = SharedMiiCfg0_t + mii_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mii_cfg));

    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiQsgmiiMode_f, &value, &mii_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgMiiXauiMode_f, &value, &mii_cfg);
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

    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiUsxgmiiEn0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFXMode0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxSampleSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateCnt0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxReplicateSlot0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiAssymMode0_f, &value, &mii_per_cfg);
    value = 10;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiSpeed0_f, &value, &mii_per_cfg);
    value = 0xb;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAFullThrd0_f, &value, &mii_per_cfg);
    value = 0x3FFF;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiTxAmInterval0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiFaultMaskLinkEn0_f, &value, &mii_per_cfg);
    value = 5;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiRxLinkFilterTimer0_f, &value, &mii_per_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_per_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_per_cfg);

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
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode0_f, &value, &shared_pcs_cfg);
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_lgMode1_f, &value, &shared_pcs_cfg);

    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, SharedPcsCfg0_cgMode_f, &value, &shared_pcs_cfg);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &shared_pcs_cfg));

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
_sys_usw_mac_set_mac_config(uint8 lchip, uint16 lport, uint8 is_init)
{
    uint8 gchip;
    uint32 gport;
    sys_datapath_lport_attr_t* port_attr;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    switch (port_attr->pcs_mode)
    {
    case CTC_CHIP_SERDES_SGMII_MODE:
    case CTC_CHIP_SERDES_2DOT5G_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_sgmii_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_QSGMII_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_qsgmii_config(lchip, lport, is_init));
        break;

    case CTC_CHIP_SERDES_USXGMII0_MODE:
    case CTC_CHIP_SERDES_USXGMII1_MODE:
    case CTC_CHIP_SERDES_USXGMII2_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_usxgmii_config(lchip, lport, is_init));
        break;

    case CTC_CHIP_SERDES_XAUI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xaui_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_DXAUI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_dxaui_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_XFI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xfi_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_XLG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xlg_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_XXVG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xxvg_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_LG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_lg_config(lchip, lport));
        break;

    case CTC_CHIP_SERDES_CG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_cg_config(lchip, lport));
        break;

    default:
        break;
    }

    if ((CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        _sys_usw_mac_set_nettxwriteen_en(lchip, lport, FALSE);
    }
    else
    {
        _sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE);
    }
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
    CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_CUT_THROUGH_EN, TRUE));

    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_mac_pre_config(uint8 lchip, uint16 lport)
{
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint8 pcs_idx   = 0;
    uint32 value    = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsLgCfg0_m        pcs_lg_cfg;
    SharedPcsSerdes0Cfg0_m   shared_pcs_serdes_cfg;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    pcs_idx = port_attr->pcs_idx;
    if ((CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) && (port_attr->flag))
    {
        value = 0;
        tbl_id = SharedPcsLgCfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_lg_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsLgCfg0_lgPcsLane0And3SwapEn_f, &value, &pcs_lg_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_lg_cfg));

        tbl_id = SharedPcsSerdes0Cfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value, &shared_pcs_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_serdes_cfg));

        tbl_id = SharedPcsSerdes2Cfg0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value, &shared_pcs_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shared_pcs_serdes_cfg));
    }

    return CTC_E_NONE;
}

#define  __MAC_ENABLE__
STATIC int32
_sys_usw_mac_set_sgmac_sgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_xfi_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_xlg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not xlg mode \n", mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        value = 0;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_xxvg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;

    /* get fec en */
    CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_en));

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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

        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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

        /* Assert Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
_sys_usw_mac_set_sgmac_lg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;

    if ((internal_mac_idx != 0) && (internal_mac_idx != 2))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not lg mode \n", mac_id);
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

    /* get fec en */
    CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_en));

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstXlgRx_f, &value, &pcs_rst);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_rxDeskewSoftRstLg_f, &value, &pcs_rst);
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSoftRst0_softRstLgRx_f, &value, &pcs_rst);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fec_reset));

            if (lg_index == 0)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx0_f, &value, &fec_reset);
                DRV_IOW_FIELD(lchip, tbl_id, ResetCtlSharedFec0_cfgSoftRstFecTx1_f, &value, &fec_reset);
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
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
_sys_usw_mac_set_sgmac_cg_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not cg mode \n", mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* get fec en */
    CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_en));

    if (enable)
    {
        /* Rlease Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstCgRx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Rlease Fec Rx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstCgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Fec Tx Soft Reset if Fec enabled */
        if (fec_en)
        {
            tbl_id = ResetCtlSharedFec0_t + shared_fec_idx;
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
_sys_usw_mac_set_sgmac_xaui_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not xaui mode \n", mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        value = 0;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_dxaui_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx != 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"mac_id: %d is not xaui mode \n", mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        value = 0;
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

        /* Release Sgmac Reset */
        tbl_id = RlmMacCtlReset_t + slice_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        field_id = SharedPcsSoftRst0_softRstXlgTx_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_qsgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QsgmiiPcsSoftRst0_m  pcs_rst;
    SharedMiiResetCfg0_m mii_rst;
    RlmMacCtlReset_m     mac_rst;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    slice_id = port_attr->slice_id;
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst0_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else
    {
        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = QsgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsRxSoftRst0_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = QsgmiiPcsSoftRst0_qsgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_sgmac_usxgmii_en(uint8 lchip, uint16 lport, bool enable)
{
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint8 slice_id;
    uint16 mac_id;
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
    mac_id = port_attr->mac_id;
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"slice_id:%d, mac_id:%d, enable:%d\n", slice_id, mac_id, enable);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (enable)
    {
        value = 0;
        /* Release Pcs Tx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst0_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
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
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
    }
    else
    {
        value = 1;
        /* Assert Pcs Rx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsRxSoftRst0_f + internal_mac_idx;
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
        field_id = RlmMacCtlReset_resetCoreSgmac0_f + mac_id;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mac_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rst));

        /* Assert Pcs Tx Soft Reset */
        tbl_id = UsxgmiiPcsSoftRst0_t + pcs_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
        field_id = UsxgmiiPcsSoftRst0_usxgmiiPcsTxSoftRst0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));

        /* Assert Mii Tx Soft Reset */
        tbl_id = SharedMiiResetCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
        field_id = SharedMiiResetCfg0_cfgSoftRstTx0_f + internal_mac_idx;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_rst);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_rst));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 num = 0;
    uint8 index = 0;
    uint32 tmp_val32 = 0;
    uint32 port_step = 0;
    ctc_chip_serdes_prbs_t prbs;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    sal_memset(&prbs, 0, sizeof(ctc_chip_serdes_prbs_t));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    /* if mac disable, need disable serdes tx and before enable serdes tx, need enable prbs15+ for sgmii*/
    if(!enable)
    {
       for (index = 0; index < num; index++)
       {
           if (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
           {
               prbs.mode = 1;
               prbs.serdes_id = serdes_id+index;
               prbs.polynome_type = CTC_CHIP_SERDES_PRBS15_SUB;
               prbs.value = 1;
               CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_prbs_tx(lchip, &prbs));
               /* at less 10ms before close serdes tx*/
               sal_task_sleep(35);
           }
           if ((CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
            && (CTC_CHIP_SERDES_USXGMII1_MODE != port_attr->pcs_mode)
            && (CTC_CHIP_SERDES_USXGMII2_MODE != port_attr->pcs_mode))
           {
               GET_SERDES(port_attr->mac_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
               CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, real_serdes, FALSE));
           }
       }
    }

    /* PCS/MII/Sgmac/FEC reset or release */
    switch (port_attr->pcs_mode)
    {
    case CTC_CHIP_SERDES_SGMII_MODE:
    case CTC_CHIP_SERDES_2DOT5G_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_sgmii_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_QSGMII_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_qsgmii_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_USXGMII0_MODE:
    case CTC_CHIP_SERDES_USXGMII1_MODE:
    case CTC_CHIP_SERDES_USXGMII2_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_usxgmii_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_XAUI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xaui_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_DXAUI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_dxaui_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_XFI_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xfi_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_XLG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xlg_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_XXVG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_xxvg_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_LG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_lg_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    case CTC_CHIP_SERDES_CG_MODE:
        CTC_ERROR_RETURN(_sys_usw_mac_set_sgmac_cg_en(lchip, lport, enable));
        p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en = ((enable)?TRUE:FALSE);
        break;

    default:
        break;
    }

    port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
    CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, port_step, &tmp_val32));
    if (enable)
    {
        tmp_val32 |= (1 << 0);
    }
    else
    {
        tmp_val32 &= ~(1 << 0);
    }
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, port_step, tmp_val32));

    if(enable)
    {
        /* if mac enable, need enable serdes tx */
        for (index = 0; index < num; index++)
        {
            GET_SERDES(port_attr->mac_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, real_serdes, TRUE));
        }
    }
    else
    {
        /* if mac disable, after reset, need disable prbs15+ for sgmii */
        for (index = 0; index < num; index++)
        {
            if (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
            {
                prbs.mode = 1;
                prbs.serdes_id = serdes_id+index;
                prbs.polynome_type = CTC_CHIP_SERDES_PRBS15_SUB;
                prbs.value = 0;
                CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_prbs_tx(lchip, &prbs));
            }
        }
    }

    CTC_ERROR_RETURN(sys_usw_qos_set_fc_default_profile(lchip, gport));

    return CTC_E_NONE;
}


#define  __GET_LINK_STATUS__
int32
_sys_usw_mac_get_sgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_codeErrCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        *p_value = value;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_qsgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        cmd = DRV_IOR(tb_id, QsgmiiPcsCodeErrCnt0_codeErrCnt0_f+internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        *p_value = value;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_usxgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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

int32
_sys_usw_mac_get_xaui_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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

int32
_sys_usw_mac_get_xfi_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        *p_value = value[0];
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_xlg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_id_sub = 0;
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        tb_id_sub = SharedPcsXfi0Status0_t + pcs_idx;

        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xlg_status));

        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt0_f, &value[0], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt1_f, &value[1], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt2_f, &value[2], &xlg_status);
        DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt3_f, &value[3], &xlg_status);

        cmd = DRV_IOR(tb_id_sub, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[0]));
        cmd = DRV_IOR(tb_id_sub+step, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[1]));
        cmd = DRV_IOR(tb_id_sub+step*2, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[2]));
        cmd = DRV_IOR(tb_id_sub+step*3, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[3]));

        *p_value = (value[0]+value[1]+value[2]+value[3]+value_sub[0]+value_sub[1]+ value_sub[2]+value_sub[3]);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_xxvg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        cmd = DRV_IOR(tb_id, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value[0]));

        *p_value = value[0];
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_lg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_id_sub = 0;
    uint32 value[4] = {0};
    uint32 value_sub[4] = {0};
    uint8 step = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsXlgStatus0_m xlg_status;
    SharedPcsLgStatus0_m lg_status;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        sal_memset(&xlg_status, 0, sizeof(SharedPcsXlgStatus0_m));
        sal_memset(&lg_status, 0, sizeof(SharedPcsLgStatus0_m));

        if (internal_mac_idx == 0)
        {
            tb_id = SharedPcsXlgStatus0_t + pcs_idx;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xlg_status));
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt0_f, &value[0], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt1_f, &value[1], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt2_f, &value[2], &xlg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsXlgStatus0_bipErrCnt3_f, &value[3], &xlg_status);
        }
        else
        {
            tb_id = SharedPcsLgStatus0_t + pcs_idx;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lg_status));
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt0_f, &value[0], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt1_f, &value[1], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt2_f, &value[2], &lg_status);
            DRV_IOR_FIELD(lchip, tb_id, SharedPcsLgStatus0_lgPcs1BipErrCnt3_f, &value[3], &lg_status);
        }

        tb_id_sub = SharedPcsXfi0Status0_t + pcs_idx;
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        cmd = DRV_IOR(tb_id_sub, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[0]));
        cmd = DRV_IOR(tb_id_sub+step, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[1]));
        cmd = DRV_IOR(tb_id_sub+step*2, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[2]));
        cmd = DRV_IOR(tb_id_sub+step*3, SharedPcsXfi0Status0_errBlockCnt0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_sub[3]));

        *p_value = (value[0]+value[1]+value[2]+value[3]+value_sub[0]+value_sub[1]+ value_sub[2]+value_sub[3]);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en)
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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

        *p_value = (value[0]+value[1]+ value[2]+value[3]);
    }

    return CTC_E_NONE;
}

#define  __FEC_CONFIG__

STATIC int32
_sys_usw_mac_set_fec_rs_config(uint8 lchip, uint16 lport)
{
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

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (pcs_idx >= 6)
    {
        shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;
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
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
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
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f + internal_mac_idx, &value, &sharedfec_cfg);
        value = 0;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
        }
        value = 0x13FFF;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx, &value, &sharedfec_cfg);

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
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
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
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
        value = 1;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        }
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
        if (0 == lg_index)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            value = 0x9FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        }
        else
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            value = 0x9FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
        }

        if (1 == port_attr->flag)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecLane0And3SwapEn_f, &value, &sharedfec_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }
    else  /* 100G */
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
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
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

        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_cgfecEn_f, &value, &pcsfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #1.3 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
        value = 0x13FFF;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_fec_baser_config(uint8 lchip, uint16 lport)
{
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
    XgFec0CtlSharedFec0_m xgfec_ctl;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (pcs_idx >= 6)
    {
        shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;
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
        /* #2.1 cfg SharedMii0Cfg tbl */
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
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
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #2.3 cfg XgFec0CtlSharedFec tbl */
        /*
         * 25G lane0--XgFec0; lane1--XgFec2; lane2--XgFec4; lane3--XgFec6
         */
        step = XgFec2CtlSharedFec0_t - XgFec0CtlSharedFec0_t; /* 25G only use XgFec0/2/4/6 */
        tbl_id = XgFec0CtlSharedFec0_t + internal_mac_idx*step + shared_fec_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_ctl));
        field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &xgfec_ctl);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xgfec_ctl));

        /* #2.4 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
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
        value = 0x13FFF;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f + internal_mac_idx, &value, &sharedfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }
    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
        /* #2.1 cfg SharedMii0Cfg tbl */
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
        field_id = SharedMii0Cfg0_cfgMiiTxRsFecEn0_f;
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &mii_per_cfg);
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
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn0_f, &value, &pcsfec_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode0_f, &value, &pcsfec_cfg);
        }
        else
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecEn1_f, &value, &pcsfec_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_lgPcsFecRsMode1_f, &value, &pcsfec_cfg);
        }
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #2.3 cfg XgFec0CtlSharedFec tbl */
        step = XgFec1CtlSharedFec0_t - XgFec0CtlSharedFec0_t;
        if (0 == lg_index)  /* 50G port 0 */
        {
            for (i = 0; i < 4; i++)
            {
                tbl_id = XgFec0CtlSharedFec0_t + i*step + shared_fec_idx;
                field_id = XgFec0CtlSharedFec0_cfgXgFec0TxWidth_f;
                cmd = DRV_IOW(tbl_id, field_id);
                value = 0;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
        }
        else    /* 50G port 2 */
        {
            for (i = 0; i < 4; i++)
            {
                tbl_id = XgFec4CtlSharedFec0_t + i*step + shared_fec_idx;
                field_id = XgFec4CtlSharedFec0_cfgXgFec4TxWidth_f;
                cmd = DRV_IOW(tbl_id, field_id);
                value = 0;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
        }

        /* #2.4 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));

        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec100GPort_f, &value, &sharedfec_cfg);
        value = 1;
        if (0 == lg_index)
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0_f, &value, &sharedfec_cfg);
        }
        else
        {
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1_f, &value, &sharedfec_cfg);
        }
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);

        if (0 == lg_index)
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort0RsMode_f, &value, &sharedfec_cfg);
            value = 0x13FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval0_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval1_f, &value, &sharedfec_cfg);
        }
        else
        {
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec50GPort1RsMode_f, &value, &sharedfec_cfg);
            value = 0x13FFF;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval2_f, &value, &sharedfec_cfg);
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecRxAmInterval3_f, &value, &sharedfec_cfg);
        }

        if (1 == port_attr->flag)
        {
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFecLane0And3SwapEn_f, &value, &sharedfec_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_fec_clear(uint8 lchip, uint16 lport)
{
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
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

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"chip_port:%d, serdes_id:%d, num:%d\n", chip_port, serdes_id, num);

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (pcs_idx >= 6)
    {
        shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;
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
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
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
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f + internal_mac_idx, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #2.4 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
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
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
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
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn0_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn1_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn2_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xfiPcsFecEn3_f, &value, &pcsfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsFecCfg0_xlgPcsFecEn_f, &value, &pcsfec_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcsfec_cfg));

        /* #2.4 cfg GlobalCtlSharedFec tbl */
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
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
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort0RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort1RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort2RsMode_f, &value, &sharedfec_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, GlobalCtlSharedFec0_cfgSharedFec25GPort3RsMode_f, &value, &sharedfec_cfg);
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
    else /* 100G */
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
        tbl_id = GlobalCtlSharedFec0_t + shared_fec_idx;
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

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_fec_type_capability(uint8 pcs_mode, uint32* p_value)
{
    CTC_PTR_VALID_CHECK(p_value);
    if ((CTC_CHIP_SERDES_XXVG_MODE == pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == pcs_mode))
    {
        *p_value = (1 << CTC_PORT_FEC_TYPE_RS);
        *p_value |= (1 << CTC_PORT_FEC_TYPE_BASER);
    }
    else if (CTC_CHIP_SERDES_CG_MODE == pcs_mode)
    {
        *p_value = (1 << CTC_PORT_FEC_TYPE_RS);
    }
    else
    {
        *p_value = (1 << CTC_PORT_FEC_TYPE_NONE);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_fec_config(uint8 lchip, uint16 lport, ctc_port_fec_type_t type)
{
    uint32 fec_value = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if(CTC_PORT_FEC_TYPE_NONE == type)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_set_fec_clear(lchip, lport));
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if(port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }
    _sys_usw_mac_get_fec_type_capability(port_attr->pcs_mode, &fec_value);

    if(!CTC_FLAG_ISSET(fec_value, (1 << type)))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (type)
    {
    case CTC_PORT_FEC_TYPE_RS:
        CTC_ERROR_RETURN(_sys_usw_mac_set_fec_clear(lchip, lport));
        CTC_ERROR_RETURN(_sys_usw_mac_set_fec_rs_config(lchip, lport));
        break;
    case CTC_PORT_FEC_TYPE_BASER:
        CTC_ERROR_RETURN(_sys_usw_mac_set_fec_clear(lchip, lport));
        CTC_ERROR_RETURN(_sys_usw_mac_set_fec_baser_config(lchip, lport));
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_fec_en(uint8 lchip, uint16 lport, ctc_port_fec_type_t type)
{
    uint8 mac_toggle_flag = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if(p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
    {
        mac_toggle_flag = 1;
        CTC_ERROR_RETURN(_sys_usw_mac_set_mac_en(lchip, lport, FALSE));
    }

    CTC_ERROR_RETURN(_sys_usw_mac_set_fec_config(lchip, lport, type));

    if(mac_toggle_flag)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_set_mac_en(lchip, lport, TRUE));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_fec_en(uint8 lchip, uint16 lport, uint32* p_value)
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

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    tbl_id = SharedPcsFecCfg0_t + pcs_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcscfg));

    if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
    {
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

    }
    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
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
    }
    else if (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
    {
        enable = GetSharedPcsFecCfg0(V, cgfecEn_f, &pcscfg);
        if(enable)
        {
            *p_value = CTC_PORT_FEC_TYPE_RS;
        }
        else
        {
            *p_value = CTC_PORT_FEC_TYPE_NONE;
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*jqiu modify for cl73 auto.*/
STATIC int32
_sys_usw_mac_set_dfe_en(uint8 lchip, uint32 gport, uint32 enable)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, dfe enable:%d\n", gport, enable?TRUE:FALSE);

    /* 1.get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    for(cnt=0; cnt<serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        CTC_ERROR_RETURN(sys_usw_datapath_set_dfe_en(lchip, real_serdes, enable));
    }

    return CTC_E_NONE;
}

/* support 802.3ap, Auto-Negotiation for Backplane Ethernet */
int32
_sys_usw_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable)
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

    /* auto-neg already enable, cann't set enable again*/
    if ((enable && p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)
        || ((!enable) && (!p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)))
    {
        return CTC_E_NONE;
    }

    /* if unidir enable, cannot set AN enable */
    CTC_ERROR_RETURN(_sys_usw_mac_get_unidir_en(lchip, lport, &unidir_en));
    if(enable && unidir_en)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Gport %u has enable unidir, cannot enable auto-nego! \n", gport);
        return CTC_E_INVALID_CONFIG;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes %d is in loopback internal mode \n", real_serdes);
            return CTC_E_INVALID_CONFIG;
        }
        lb_param.mode = 1;
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes %d is in loopback external mode \n", real_serdes);
            return CTC_E_INVALID_CONFIG;
        }
    }

    port_attr->is_first = 0;

    if(enable)
    {
        /* do pcs rx reset */
        CTC_ERROR_RETURN(_sys_usw_mac_pcs_rx_rst(lchip, lport, 0, TRUE));
        /* Disable DFE */
        CTC_ERROR_RETURN(_sys_usw_mac_set_dfe_en(lchip, gport, FALSE));
    }
    else
    {
        /* Enable DFE */
        CTC_ERROR_RETURN(_sys_usw_mac_set_dfe_en(lchip, gport, TRUE));
        /* do pcs rx release */
        CTC_ERROR_RETURN(_sys_usw_mac_pcs_rx_rst(lchip, lport, 0, FALSE));
        /* 1. if disable, need do 3ap trainging disable first */
        CTC_ERROR_RETURN(sys_usw_mac_set_3ap_training_en(lchip, gport, 0));
    }

    /* 2. enable 3ap auto-neg */
    for(cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_auto_neg_en(lchip, real_serdes, enable));
        if (!enable)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, real_serdes, 0));
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, real_serdes, 1));
        }
    }

    p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable = enable;

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Exit %s\n", __FUNCTION__);

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cl73_autoneg_ability(uint8 lchip, uint32 gport, uint32 type, sys_datapath_an_ability_t* p_ability)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt = 0;
    sys_datapath_an_ability_t serdes_ability[4];

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d\n", gport);

    /* 1. get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    /* if auto_neg_ok, should get ability */
    if (type)
    {
        for(cnt=0; cnt<serdes_num; cnt++)
        {
            GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_auto_neg_remote_ability(lchip, real_serdes,&(serdes_ability[cnt])));
        }
    }
    else
    {
        for(cnt=0; cnt<serdes_num; cnt++)
        {
            GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_auto_neg_local_ability(lchip, real_serdes, &(serdes_ability[cnt])));
        }
    }
    sal_memset(p_ability, 0, sizeof(sys_datapath_an_ability_t));
    for(cnt=0; cnt<serdes_num; cnt++)
    {
        p_ability->base_ability0 |= serdes_ability[cnt].base_ability0;
        p_ability->base_ability1 |= serdes_ability[cnt].base_ability1;
        p_ability->np0_ability0 |= serdes_ability[cnt].np0_ability0;
        p_ability->np0_ability1 |= serdes_ability[cnt].np0_ability1;
        p_ability->np1_ability0 |= serdes_ability[cnt].np1_ability0;
        p_ability->np1_ability1 |= serdes_ability[cnt].np1_ability1;
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
        "port %s base ability[0]:0x%x,[1] :0x%x, next page0[0]:0x%x,[1]:0x%x, next page1[0]:0x%x,[1]:0x%x\n",
        type?"remote":"local", p_ability->base_ability0, p_ability->base_ability1, p_ability->np0_ability0,
        p_ability->np0_ability1, p_ability->np1_ability0, p_ability->np1_ability1);

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_table_by_mac_id(uint8 lchip, uint8 mac_id, uint32* table, uint32* bitmap)
{
    uint8 offset = 0;
    uint8 base = 0;;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, " %s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " mac id is: %d\n", mac_id);
    if(mac_id < 12)
    {
        *table = RlmHsCtlInterruptFunc_t;
        offset = 0;
        base = 0;
    }
    else if(mac_id < 16)
    {
        *table = RlmCsCtlInterruptFunc_t;
        offset = 12;
        base = 0;
    }
    else if(mac_id < 28)
    {
        *table = RlmHsCtlInterruptFunc_t;
        offset = 16;
        base = 24;
    }
    else if(mac_id < 32)
    {
        *table = RlmCsCtlInterruptFunc_t;
        offset = 28;
        base = 8;
    }
    else if(mac_id < 44)
    {
        *table = RlmHsCtlInterruptFunc_t;
        offset = 32;
        base = 48;
    }
    else if(mac_id < 48)
    {
        *table = RlmCsCtlInterruptFunc_t;
        offset = 44;
        base = 16;
    }
    else if(mac_id < 60)
    {
        *table = RlmHsCtlInterruptFunc_t;
        offset = 48;
        base = 72;
    }
    else
    {
        *table = RlmCsCtlInterruptFunc_t;
        offset = 60;
        base = 24;
    }

    if(bitmap)
    {
        CTC_BMP_SET(bitmap, (mac_id-offset)*2+base);
        CTC_BMP_SET(bitmap, (mac_id-offset)*2+base+1);
    }
    return 0;
}

STATIC int32
_sys_usw_mac_clear_link_intr(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    uint16 mac_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_index = 0;
    uint32 value[4] = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_INIT_CHECK();

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;

    }

    mac_id = port_attr->mac_id;
    tb_index = 1;

    _sys_usw_mac_get_table_by_mac_id(lchip, mac_id, &tb_id, value);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tb_index, cmd, value));
    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cl37_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value)
{
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 field_id  = 0;
    uint16 step      = 0;
    uint8  pcs_idx   = 0;
    uint8  internal_mac_idx = 0;
    uint32 value = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    /* SGMII/QSGMII/USXGMII mode auto neg */
    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        if(CTC_PORT_PROP_AUTO_NEG_EN == type)
        {
            if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
            {
                step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
                tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = SharedPcsSgmii0Cfg0_anEnable0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
            {
                step = QsgmiiPcsAneg1Cfg0_t - QsgmiiPcsAneg0Cfg0_t;
                tbl_id = QsgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = QsgmiiPcsAneg0Cfg0_anEnable0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
            {
                step = UsxgmiiPcsAneg1Cfg0_t - UsxgmiiPcsAneg0Cfg0_t;
                tbl_id = UsxgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = UsxgmiiPcsAneg0Cfg0_anEnable0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
        }
        else if(CTC_PORT_PROP_AUTO_NEG_MODE == type)
        {
            if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
            {
                step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
                tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = SharedPcsSgmii0Cfg0_anegMode0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
            {
                step = QsgmiiPcsAneg1Cfg0_t - QsgmiiPcsAneg0Cfg0_t;
                tbl_id = QsgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = QsgmiiPcsAneg0Cfg0_anegMode0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
            {
                step = UsxgmiiPcsAneg1Cfg0_t - UsxgmiiPcsAneg0Cfg0_t;
                tbl_id = UsxgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                field_id = UsxgmiiPcsAneg0Cfg0_anegMode0_f;
                cmd = DRV_IOR(tbl_id, field_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
            if(0 == value)
            {
                value = CTC_PORT_AUTO_NEG_MODE_1000BASE_X;
            }
            else if (1 == value)
            {
                value = CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER;
            }
            else if (2 == value)
            {
                value = CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER;
            }
        }
        else if (CTC_PORT_PROP_AUTO_NEG_FEC == type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " FEC not supported\n");
            return CTC_E_INVALID_PARAM;
        }
    }
    if (p_value)
    {
        *p_value = value;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cl73_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value)
{
    uint32 value = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        *p_value = 0;
        return CTC_E_NONE;
    }

    if (CTC_PORT_PROP_AUTO_NEG_EN == type)
    {
        value = p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable;
    }
    else if (CTC_PORT_PROP_AUTO_NEG_FEC == type)
    {
        if(DRV_IS_TSINGMA(lchip))
        {
            if ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_XFI_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_XLG_MODE != port_attr->pcs_mode))
            {
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mode %u FEC not supported\n", port_attr->pcs_mode);
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode))
            {
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mode %u FEC not supported\n", port_attr->pcs_mode);
                return CTC_E_INVALID_PARAM;
            }
        }
        value = port_attr->an_fec;
    }
    *p_value = value;

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cl73_capability(uint8 lchip, uint32 gport, uint32* p_capability)
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
                                 CTC_PORT_CSTM_BASER_FEC_REQUESTED|
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            case CTC_CHIP_SERDES_LG_MODE:
                *p_capability = (CTC_PORT_CSTM_50GBASE_KR2 |
                                 CTC_PORT_CSTM_50GBASE_CR2 |
                                 CTC_PORT_CSTM_RS_FEC_ABILITY |
                                 CTC_PORT_CSTM_BASER_FEC_ABILITY |
                                 CTC_PORT_CSTM_RS_FEC_REQUESTED |
                                 CTC_PORT_CSTM_BASER_FEC_REQUESTED|
                                 CTC_PORT_CL73_FEC_ABILITY |
                                 CTC_PORT_CL73_FEC_REQUESTED);
                break;
            case CTC_CHIP_SERDES_CG_MODE:
            case CTC_CHIP_SERDES_XLG_MODE:
                *p_capability = (CTC_PORT_CL73_40GBASE_KR4 |
                                 CTC_PORT_CL73_40GBASE_CR4 |
                                 CTC_PORT_CL73_100GBASE_KR4 |
                                 CTC_PORT_CL73_100GBASE_CR4 |
                                 CTC_PORT_CL73_25G_RS_FEC_REQUESTED);
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
                *p_capability = (CTC_PORT_CL73_10GBASE_KR);
                break;
            case CTC_CHIP_SERDES_XLG_MODE:
                *p_capability = (CTC_PORT_CL73_40GBASE_KR4 |
                                 CTC_PORT_CL73_40GBASE_CR4);
                break;
            default:
                *p_capability = 0;
                break;
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability)
{
    sys_datapath_an_ability_t cl73_ability;

    /* get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    CTC_ERROR_RETURN(_sys_usw_mac_get_cl73_autoneg_ability(lchip, gport, type, &cl73_ability));
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_10GBASE_KR)
    {
        *p_ability |= (CTC_PORT_CL73_10GBASE_KR);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_40GBASE_KR4)
    {
        *p_ability |= (CTC_PORT_CL73_40GBASE_KR4);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_40GBASE_CR4)
    {
        *p_ability |= (CTC_PORT_CL73_40GBASE_CR4);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_100GBASE_KR4)
    {
        *p_ability |= (CTC_PORT_CL73_100GBASE_KR4);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_100GBASE_CR4)
    {
        *p_ability |= (CTC_PORT_CL73_100GBASE_CR4);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_25GBASE_KR_S)
    {
        *p_ability |= (CTC_PORT_CL73_25GBASE_KRS);
    }
    if(cl73_ability.base_ability0 & SYS_PORT_CL73_25GBASE_KR)
    {
        *p_ability |= (CTC_PORT_CL73_25GBASE_KR);
        *p_ability |= (CTC_PORT_CL73_25GBASE_KRS);
    }
    if(cl73_ability.base_ability1 & SYS_PORT_CL73_25G_RS_FEC_REQ)
    {
        *p_ability |= (CTC_PORT_CL73_25G_RS_FEC_REQUESTED);
    }
    if(cl73_ability.base_ability1 & SYS_PORT_CL73_25G_BASER_FEC_REQ)
    {
        *p_ability |= (CTC_PORT_CL73_25G_BASER_FEC_REQUESTED);
    }
    if(cl73_ability.base_ability1 & SYS_PORT_CL73_FEC_SUP)
    {
        *p_ability |= (CTC_PORT_CL73_FEC_ABILITY);
    }
    if(cl73_ability.base_ability1 & SYS_PORT_CL73_FEC_REQ)
    {
        *p_ability |= (CTC_PORT_CL73_FEC_REQUESTED);
    }
    if(cl73_ability.np1_ability0 & SYS_PORT_CSTM_25GBASE_KR1)
    {
        *p_ability |= (CTC_PORT_CSTM_25GBASE_KR1);
    }
    if(cl73_ability.np1_ability0 & SYS_PORT_CSTM_25GBASE_CR1)
    {
        *p_ability |= (CTC_PORT_CSTM_25GBASE_CR1);
    }
    if(cl73_ability.np1_ability0 & SYS_PORT_CSTM_50GBASE_KR2)
    {
        *p_ability |= (CTC_PORT_CSTM_50GBASE_KR2);
    }
    if(cl73_ability.np1_ability0 & SYS_PORT_CSTM_50GBASE_CR2)
    {
        *p_ability |= (CTC_PORT_CSTM_50GBASE_CR2);
    }
    if(cl73_ability.np1_ability1 & SYS_PORT_CSTM_CL91_FEC_SUP)
    {
        *p_ability |= (CTC_PORT_CSTM_RS_FEC_ABILITY);
    }
    if(cl73_ability.np1_ability1 & SYS_PORT_CSTM_CL74_FEC_SUP)
    {
        *p_ability |= (CTC_PORT_CSTM_BASER_FEC_ABILITY);
    }
    if(cl73_ability.np1_ability1 & SYS_PORT_CSTM_CL91_FEC_REQ)
    {
        *p_ability |= (CTC_PORT_CSTM_RS_FEC_REQUESTED);
    }
    if(cl73_ability.np1_ability1 & SYS_PORT_CSTM_CL74_FEC_REQ)
    {
        *p_ability |= (CTC_PORT_CSTM_BASER_FEC_REQUESTED);
    }
    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_3ap_training_en(uint8 lchip, uint16 lport, uint32* p_status)
{
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt;
    uint16 value = 0;
    uint16 result;
    uint8 status[4] = {0};

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Enter %s\n", __FUNCTION__);

    sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    result = 0;
    for(cnt=0; cnt<serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        if(MCHIP_API(lchip)->serdes_get_link_training_status)
        {
            CTC_ERROR_RETURN(MCHIP_API(lchip)->serdes_get_link_training_status(lchip, real_serdes, &value));
        }
        result |= value;
    }

    if(DRV_IS_DUET2(lchip))
    {
        if((result &(CL72_FSM_TR_COMP | CL72_FSM_TR_FAIL | CL72_FSM_TR_ACT))==0)
        {
            *p_status = SYS_PORT_CL72_DISABLE;
        }
        else if(result & CL72_FSM_TR_FAIL)
        {
            *p_status = SYS_PORT_CL72_FAIL;
        }
        else if(result & CL72_FSM_TR_ACT)
        {
            *p_status = SYS_PORT_CL72_PROGRESS;
        }
        else
        {
            *p_status = SYS_PORT_CL72_OK;
        }
    }
    else if(DRV_IS_TSINGMA(lchip))
    {
        /*1 lane mode*/
        if(1 == serdes_num)
        {
            *p_status = result >> (4 * (serdes_id % 4));
        }
        else
        {
            for(cnt=0; cnt<serdes_num; cnt++)
            {
                GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
                status[cnt] = result >> (4 * (real_serdes % 4));
            }
            /*2 lane mode*/
            if(2 == serdes_num)
            {
                /*if all lanes are disable, this port is disable*/
                if((SYS_PORT_CL72_DISABLE == status[0]) && (SYS_PORT_CL72_DISABLE == status[1]))
                {
                    *p_status = SYS_PORT_CL72_DISABLE;
                }
                /*if all lanes are lt OK, this port is OK*/
                else if((SYS_PORT_CL72_OK == status[0]) && (SYS_PORT_CL72_OK == status[1]))
                {
                    *p_status = SYS_PORT_CL72_OK;
                }
                /*if any lane fail, this port is fail*/
                else if((SYS_PORT_CL72_FAIL == status[0]) || (SYS_PORT_CL72_FAIL == status[1]))
                {
                    *p_status = SYS_PORT_CL72_FAIL;
                }
                /*any other state combination means in progress*/
                else
                {
                    *p_status = SYS_PORT_CL72_PROGRESS;
                }
            }
            /*4 lane mode*/
            else
            {
                /*if all lanes are disable, this port is disable*/
                if(0 == result)
                {
                    *p_status = SYS_PORT_CL72_DISABLE;
                }
                /*if all lanes are lt OK, this port is OK*/
                else if(0x3333 == result)
                {
                    *p_status = SYS_PORT_CL72_OK;
                }
                /*if any lane fail, this port is fail*/
                else if((SYS_PORT_CL72_FAIL == status[0]) || (SYS_PORT_CL72_FAIL == status[1]) ||
                        (SYS_PORT_CL72_FAIL == status[2]) || (SYS_PORT_CL72_FAIL == status[3]))
                {
                    *p_status = SYS_PORT_CL72_FAIL;
                }
                /*any other state combination means in progress*/
                else
                {
                    *p_status = SYS_PORT_CL72_PROGRESS;
                }
            }
        }
    }

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"lport:%d, LT status:0x%X\n", lport, *p_status);

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_cl37_an_remote_status(uint8 lchip, uint32 gport, uint32 auto_neg_mode, uint32* p_speed, uint32* p_link)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 fld_id = 0;
    uint32 value = 0;
    uint32 tmp_val32 = 0;
    uint16 step  = 0;
    uint16 lport = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint32 remote_speed = 0;
    uint32 remote_link = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if (NULL == p_usw_port_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    if (p_link)
    {
        *p_link = TRUE;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    if ((CTC_CHIP_SERDES_SGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_2DOT5G_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII0_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII1_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_USXGMII2_MODE != port_attr->pcs_mode))
    {
        return CTC_E_NONE;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    if (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
    {
        step = SharedPcsSgmii1Status0_t - SharedPcsSgmii0Status0_t;
        tb_id = SharedPcsSgmii0Status0_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOR(tb_id, SharedPcsSgmii0Status0_anRxRemoteCfg0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_val32));
        /*
          get sgmii speed
          bit[11:10] --- 0b'10 --- 1000M
                         0b'01 --- 100M
                         0b'00 --- 10M
          */
        value = (tmp_val32 >> 10) & 0x3;
        if (0x0 == value)
        {
            remote_speed = CTC_PORT_SPEED_10M;
        }
        else if (0x1 == value)
        {
            remote_speed = CTC_PORT_SPEED_100M;
        }
        else
        {
            remote_speed = CTC_PORT_SPEED_1G;
        }

        if (CTC_PORT_AUTO_NEG_MODE_1000BASE_X == auto_neg_mode)
        {
            /*
              get 1000BASE-X link status
              bit[13:12] --- 0b'00 --- No error, link OK
                             0b'01 --- Link_Faliure
                             0b'01 --- Offline
                             0b'11 --- Auto-Negotiation_Error
              */
            value = (tmp_val32 >> 12) & 0x3;
            if (value)
            {
                remote_link = FALSE;
            }
            else
            {
                remote_link = TRUE;
            }
        }
        else
        {
            /*
              get SGMII link status
              bit[15] --- 0b'1 --- Link up
                          0b'0 --- Link down
              */
            value = (tmp_val32 >> 15) & 0x1;
            if (value)
            {
                remote_link = TRUE;
            }
            else
            {
                remote_link = FALSE;
            }
        }
    }
    else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        GET_QSGMII_TBL(port_attr->mac_id, pcs_idx, tb_id);
        cmd = DRV_IOR(tb_id, QsgmiiPcsStatus00_anRxRemoteCfg0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_val32));
        /*
          get qsgmii speed
          bit[11:10] --- 0b'10 --- 1000M
                         0b'01 --- 100M
                         0b'00 --- 10M
          */
        value = (tmp_val32 >> 10) & 0x3;
        if (0x0 == value)
        {
            remote_speed = CTC_PORT_SPEED_10M;
        }
        else if (0x1 == value)
        {
            remote_speed = CTC_PORT_SPEED_100M;
        }
        else
        {
            remote_speed = CTC_PORT_SPEED_1G;
        }

        /*
          get QSGMII link status
          bit[15] --- 0b'1 --- Link up
                      0b'0 --- Link down
          */
        value = (tmp_val32 >> 15) & 0x1;
        if (value)
        {
            remote_link = TRUE;
        }
        else
        {
            remote_link = FALSE;
        }
    }
    else
    {
        step = UsxgmiiPcsXfi1Status0_t - UsxgmiiPcsXfi0Status0_t;
        tb_id = UsxgmiiPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
        switch (internal_mac_idx)
        {
        case 0:
            fld_id = UsxgmiiPcsXfi0Status0_usxgmiiAnRxRemoteCfg0_f;
            break;
        case 1:
            fld_id = UsxgmiiPcsXfi1Status0_usxgmiiAnRxRemoteCfg1_f;
            break;
        case 2:
            fld_id = UsxgmiiPcsXfi2Status0_usxgmiiAnRxRemoteCfg2_f;
            break;
        case 3:
            fld_id = UsxgmiiPcsXfi3Status0_usxgmiiAnRxRemoteCfg3_f;
            break;
        }
        cmd = DRV_IOR(tb_id, fld_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_val32));
        /*
          get usxgmii speed
          bit[11:9] ---  0b'000 --- 10M
                         0b'001 --- 100M
                         0b'010 --- 1000M
                         0b'011 --- 10G
                         0b'100 --- 2.5G
                         0b'101 --- 5G
          */
        value = (tmp_val32 >> 9) & 0x7;
        if (0x0 == value)
        {
            remote_speed = CTC_PORT_SPEED_10M;
        }
        else if (0x1 == value)
        {
            remote_speed = CTC_PORT_SPEED_100M;
        }
        else if (0x3 == value)
        {
            remote_speed = CTC_PORT_SPEED_10G;
        }
        else if (0x4 == value)
        {
            remote_speed = CTC_PORT_SPEED_2G5;
        }
        else if (0x5 == value)
        {
            remote_speed = CTC_PORT_SPEED_5G;
        }
        else
        {
            remote_speed = CTC_PORT_SPEED_1G;
        }

        /*
          get USXGMII link status
          bit[15] --- 0b'1 --- Link up
                      0b'0 --- Link down
          */
        value = (tmp_val32 >> 15) & 0x1;
        if (value)
        {
            remote_link = TRUE;
        }
        else
        {
            remote_link = FALSE;
        }
    }

    if (p_speed)
    {
        *p_speed = remote_speed;
    }
    if (p_link)
    {
        *p_link = remote_link;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_unidir_en(uint8 lchip, uint16 lport, bool enable)
{
    uint8 internal_mac_idx = 0;
    uint8  step       = 0;
    uint32 tbl_id     = 0;
    uint8  mii_idx    = 0;
    uint8  pcs_idx    = 0;
    uint32 cmd        = 0;
    uint32 value      = 0;
    uint32 curr_unidir = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedMii0Cfg0_m   mii_cfg;
    uint32 mcuval = 0;
    uint32 port_step = 0;
    uint32 mii_rx_link  = 0;
    /* SYSTEM MODIFIED by tongzb for bug 53301, do not check unidir in sdk */
    //uint32 an_en_stat   = 0;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Lport %u has enable auto-nego, cannot enable unidir! \n", lport);
        return CTC_E_INVALID_CONFIG;
    }
#endif

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    value  = enable?1:0;

    CTC_ERROR_RETURN(_sys_usw_mac_get_unidir_en(lchip, lport, &curr_unidir));
    if (curr_unidir == value)
    {
        return CTC_E_NONE;
    }

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsCfg1_t - SharedPcsCfg0_t;
        tbl_id = SharedPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOW(tbl_id, SharedPcsCfg0_unidirectionEn0_f+internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
    else if(CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        step = QsgmiiPcsCfg1_t - QsgmiiPcsCfg0_t;
        tbl_id = QsgmiiPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOW(tbl_id, QsgmiiPcsCfg0_unidirectionEn0_f+internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
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
        port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, (port_step+0x4), &mcuval));
        if (value)
        {
            mcuval |= (1 << 24);
            CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (port_step+0x4), mcuval));
        }

        switch (port_attr->pcs_mode)
        {
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
            CTC_ERROR_RETURN(_sys_usw_mac_get_xaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &mii_rx_link, TRUE));
            break;
        case CTC_CHIP_SERDES_XFI_MODE:
            CTC_ERROR_RETURN(_sys_usw_mac_get_xfi_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &mii_rx_link, TRUE));
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            CTC_ERROR_RETURN(_sys_usw_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &mii_rx_link, TRUE));
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
            CTC_ERROR_RETURN(_sys_usw_mac_get_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &mii_rx_link, TRUE));
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            CTC_ERROR_RETURN(_sys_usw_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &mii_rx_link, TRUE));
            break;
        default:
            break;
        }
        if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode))
        {
            if ((!value) && (!mii_rx_link))
            {
                /* NetTxWriteEn */
                CTC_ERROR_RETURN(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, FALSE));
            }
        }

        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &value, &mii_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &value, &mii_cfg);
        DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &value, &mii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));

        if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode))
        {
            if (value)
            {
                /* NetTxWriteEn */
                CTC_ERROR_RETURN(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
            }
        }

        if (!value)
        {
            mcuval &= ~(1 << 24);
            CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (port_step+0x4), mcuval));
        }
    }
    else
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    p_usw_mac_master[lchip]->mac_prop[lport].unidir_en = (enable?TRUE:FALSE);

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_unidir_en(uint8 lchip, uint16 lport, uint32* p_value)
{
    uint8 internal_mac_idx = 0;
    uint8  step       = 0;
    uint32 tbl_id     = 0;
    uint8  pcs_idx    = 0;
    uint32 cmd        = 0;
    uint32 value1      = 0;
    QsgmiiPcsCfg0_m q_pcs_cfg;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsCfg1_t - SharedPcsCfg0_t;
        tbl_id = SharedPcsCfg0_t + pcs_idx*step;
        cmd = DRV_IOR(tbl_id, SharedPcsCfg0_unidirectionEn0_f+internal_mac_idx);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));
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

/*
 * PCS rx reset function, reset == 1 : do PCS rx reset;
                          reset == 0 : release PCS rx reset
   type -> sys_port_recover_t, if 0, means not care type
 */
STATIC int32
_sys_usw_mac_pcs_rx_rst(uint8 lchip, uint16 lport, uint32 type, uint8 reset)
{
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8 slice_id  = 0;
    uint16 chip_port = 0;
    uint32 value    = 0;
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint32 field_id = 0;
    uint16 serdes_id = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 shared_fec_idx = 0;
    ctc_chip_serdes_loopback_t lb_param;
    QsgmiiPcsSoftRst0_m   qsgmii_pcs_rst;
    UsxgmiiPcsSoftRst0_m  usxgmii_pcs_rst;
    SharedPcsSoftRst0_m   pcs_rst;
    ResetCtlSharedFec0_m  fec_reset;

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

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;
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
        if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
            || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
        {
            /* Assert Fec Rx Soft Reset */
            if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
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
            lb_param.mode = 0;
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
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
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type))
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
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
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
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type))
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
            || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
        {
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            field_id = SharedPcsSoftRst0_softRstPcsRx0_f + internal_mac_idx;
            cmd = DRV_IOW(tbl_id, field_id);
            value = 0;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            /* Rlease Fec Rx Soft Reset */
            if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
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
            lb_param.mode = 0;
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
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
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type))
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
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
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
                if ((SYS_PORT_CODE_ERROR & type) || (SYS_PORT_NO_RANDOM_FLAG & type))
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
            /* Release Pcs Rx Soft Reset */
            tbl_id = SharedPcsSoftRst0_t + pcs_idx;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
            value = 0;
            field_id = SharedPcsSoftRst0_softRstXlgRx_f + internal_mac_idx;
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &pcs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_rst));
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
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_set_pcs_recover(uint8 lchip, uint16 lport, uint32 type)
{
    uint8  mii_idx = 0;
    uint8  internal_mac_idx = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    uint32 cmd = 0;
    uint8  step = 0;
    uint32 ignore_fault = 0;
    uint32 value = 0;
    int32 ret = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedMii0Cfg0_m      mii_cfg;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    ret = drv_usw_mcu_lock(lchip, DRV_MCU_LOCK_WA_CFG, 0);
    if (ret < 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " CPU get lock failed \n");
        return CTC_E_NONE;
    }

    if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
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
    }

    _sys_usw_mac_pcs_rx_rst(lchip, lport, type, 1);
    _sys_usw_mac_pcs_rx_rst(lchip, lport, type, 0);

    if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
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
    }

    ret = drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_WA_CFG, 0);
    if (ret < 0)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " CPU get lock failed \n");
        return CTC_E_NONE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mac_recover_process(uint8 lchip, uint16 lport, uint32 type)
{
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 index = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    /* #1, serdes dfe reset */
    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num);
    for (index = 0; index < serdes_num; index++)
    {
        /*do dfe reset */
        GET_SERDES(port_attr->mac_id, serdes_id, index, port_attr->flag, port_attr->pcs_mode, real_serdes);
        sys_usw_datapath_set_dfe_reset(lchip, real_serdes, 1);
        sys_usw_datapath_set_dfe_reset(lchip, real_serdes, 0);
    }

    /* #2, delay 20ms */
    sal_task_sleep(20);

    /* #3, pcs(and fec if enable) rx reset */
    _sys_usw_mac_set_pcs_recover(lchip, lport, type);

    return CTC_E_NONE;
}

/* get serdes no random data flag by gport */
int32
_sys_usw_mac_get_no_random_data_flag(uint8 lchip, uint16 lport, uint32* p_value)
{
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt = 0;
    uint32 tmp_val32 = 0;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        *p_value = 0;
        return CTC_E_NONE;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    for (cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        tmp_val32 += sys_usw_datapath_get_serdes_no_random_data_flag(lchip, real_serdes);
    }
    *p_value = (tmp_val32 ? 1 : 0);

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_pcs_status(uint8 lchip, uint16 lport, uint32 *pcs_sta)
{
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 pcs_status = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint8 mii_idx = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint8 shared_fec_idx = 0;
    uint16 step  = 0;
    uint32 fec_cfg  = 0;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    mii_idx          = port_attr->mii_idx;
    pcs_idx          = port_attr->pcs_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if(DRV_IS_TSINGMA(lchip))
    {
        shared_fec_idx = (pcs_idx >= 6) ? (pcs_idx - 6) : 0;
    }
    else
    {
        shared_fec_idx = (mii_idx >=12) ? (mii_idx - 12) : 0;
    }

    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
    {
        step = SharedPcsSgmii1Status0_t - SharedPcsSgmii0Status0_t;
        tbl_id = SharedPcsSgmii0Status0_t + internal_mac_idx*step + pcs_idx;
        fld_id = SharedPcsSgmii0Status0_anLinkStatus0_f;
    }
    else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        step = UsxgmiiPcsXfi1Status0_t - UsxgmiiPcsXfi0Status0_t;
        tbl_id = UsxgmiiPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
        switch (internal_mac_idx)
        {
        case 0:
            fld_id = UsxgmiiPcsXfi0Status0_usxgmiiSyncStatus0_f;
            break;
        case 1:
            fld_id = UsxgmiiPcsXfi1Status0_usxgmiiSyncStatus1_f;
            break;
        case 2:
            fld_id = UsxgmiiPcsXfi2Status0_usxgmiiSyncStatus2_f;
            break;
        case 3:
            fld_id = UsxgmiiPcsXfi3Status0_usxgmiiSyncStatus3_f;
            break;
        }
    }
    else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {
        GET_QSGMII_TBL(port_attr->mac_id, pcs_idx, tbl_id);
        fld_id = QsgmiiPcsStatus00_anLinkStatus0_f;
    }
    else if ((CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        tbl_id = SharedPcsXauiStatus0_t + pcs_idx;
        fld_id = SharedPcsXauiStatus0_xauiEeeAlignStatus_f;
    }
    else if (CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
    {
        step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
        tbl_id = SharedPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
        fld_id = SharedPcsXfi0Status0_xfiSyncStatus0_f;
    }
    else if (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
    {
        tbl_id = SharedPcsXlgStatus0_t + pcs_idx;
        fld_id = SharedPcsXlgStatus0_alignStatus0_f;
    }
    else if (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_cfg));
        if (CTC_PORT_FEC_TYPE_RS == fec_cfg)
        {
            step = RsFec1StatusSharedFec0_t - RsFec0StatusSharedFec0_t;
            tbl_id = RsFec0StatusSharedFec0_t + internal_mac_idx*step + shared_fec_idx;
            fld_id = RsFec0StatusSharedFec0_dbgRsFec0RxReady_f;
        }
        else
        {
            step = SharedPcsXfi1Status0_t - SharedPcsXfi0Status0_t;
            tbl_id = SharedPcsXfi0Status0_t + internal_mac_idx*step + pcs_idx;
            fld_id = SharedPcsXfi0Status0_xfiSyncStatus0_f;
        }
    }
    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_cfg));
        if (CTC_PORT_FEC_TYPE_RS == fec_cfg)
        {
            step = RsFec1StatusSharedFec0_t - RsFec0StatusSharedFec0_t;
            tbl_id = RsFec0StatusSharedFec0_t + internal_mac_idx*step + shared_fec_idx;
            fld_id = RsFec0StatusSharedFec0_dbgRsFec0RxReady_f;
        }
        else
        {
            if (0 == lg_index)
            {
                tbl_id = SharedPcsXlgStatus0_t + pcs_idx;
                fld_id = SharedPcsXlgStatus0_alignStatus0_f;
            }
            else
            {
                tbl_id = SharedPcsLgStatus0_t + pcs_idx;
                fld_id = SharedPcsLgStatus0_lgPcs1AlignStatus_f;
            }
        }
    }
    else if (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_get_fec_en(lchip, lport, &fec_cfg));
        if (CTC_PORT_FEC_TYPE_RS == fec_cfg)
        {
            tbl_id = RsFec0StatusSharedFec0_t + shared_fec_idx;
            fld_id = RsFec0StatusSharedFec0_dbgRsFec0RxReady_f;
        }
        else
        {
            tbl_id = SharedPcsCgStatus0_t + pcs_idx;
            fld_id = SharedPcsCgStatus0_cgPcsAlignStatus_f;
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_status));

    if (pcs_sta)
    {
        *pcs_sta = pcs_status;
    }

    return CTC_E_NONE;
}


int32
_sys_usw_mac_get_code_err(uint8 lchip, uint16 lport, uint32* code_err)
{
    uint32 unidir_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    int32 ret = 0;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    *code_err = 0;
    _sys_usw_mac_get_unidir_en(lchip, lport, &unidir_en);
    if (unidir_en)
    {
        *code_err = 0;
        return CTC_E_NONE;
    }

    switch (port_attr->pcs_mode)
    {
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            ret = _sys_usw_mac_get_sgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_QSGMII_MODE:
            ret = _sys_usw_mac_get_qsgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_USXGMII0_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            ret = _sys_usw_mac_get_usxgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
            ret = _sys_usw_mac_get_xaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XFI_MODE:
            ret = _sys_usw_mac_get_xfi_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XLG_MODE:
            ret = _sys_usw_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
            ret = _sys_usw_mac_get_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_LG_MODE:
            ret = _sys_usw_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        case CTC_CHIP_SERDES_CG_MODE:
            ret = _sys_usw_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_CODE_ERR, lport, code_err, 0);
            break;

        default:
            break;
    }
    return ret;
}

int32
_sys_usw_mac_pcs_link_fault_reset(uint8 lchip, uint32 gport)
{
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_XLG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_XAUI_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_DXAUI_MODE != port_attr->pcs_mode))
    {
        return CTC_E_NONE;
    }

    /* disable interrupt at first */
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, FALSE));
    port_attr->pcs_reset_en = 1;

    CTC_ERROR_RETURN(_sys_usw_mac_set_pcs_recover(lchip, lport, 0));
    sal_task_sleep(60);

    port_attr->pcs_reset_en = 0;
    /* enable interrupt after operation */
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, TRUE));

    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_rx_pause_type(uint8 lchip, uint32 gport, uint32 pasue_type)
{
    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_rx_pause_type(uint8 lchip, uint32 gport, uint32* p_pasue_type)
{
    return CTC_E_NONE;
}







int32
_sys_usw_mac_get_speed_mode_capability(uint8 lchip, uint32 gport, uint32* speed_mode_bitmap)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 tmp_bitmap = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_MAC_GET_HSS_TYPE(port_attr->mac_id))  /* HSS28G */
    {
        if (port_attr->is_serdes_dyn)
        {
            tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_2G5);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_10G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_20G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_40G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_100G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_25G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_50G);
        }
        else
        {
            tmp_bitmap = port_attr->speed_mode;
        }
    }
    else
    {
        if (port_attr->is_serdes_dyn)
        {
            tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_2G5);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_10G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_20G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_40G);
            tmp_bitmap |= (1 << CTC_PORT_SPEED_5G);
        }
        else
        {
            if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
                || (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode))
            {
                tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            }
            else if (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
            {
                tmp_bitmap |= (1 << CTC_PORT_SPEED_10G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_5G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_2G5);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            }
            else if (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode)
            {
                tmp_bitmap |= (1 << CTC_PORT_SPEED_5G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_2G5);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            }
            else if (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
            {
                tmp_bitmap |= (1 << CTC_PORT_SPEED_2G5);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_1G);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_100M);
                tmp_bitmap |= (1 << CTC_PORT_SPEED_10M);
            }
            else
            {
                tmp_bitmap = port_attr->speed_mode;
            }
        }
    }

    *speed_mode_bitmap = tmp_bitmap;

    return CTC_E_NONE;
}

int32
_sys_usw_mac_get_if_type_capability(uint8 lchip, uint32 gport, uint32* if_type_bitmap)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 tmp_if_type_bitmap = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_SGMII);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_2500X);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_QSGMII);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_USXGMII_S);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_USXGMII_M2G5);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_USXGMII_M5G);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_XAUI);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_DXAUI);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_XFI);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_KR);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_CR);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_KR2);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_CR2);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_KR4);
    tmp_if_type_bitmap |= (1 << CTC_PORT_IF_CR4);

    if (SYS_MAC_GET_HSS_TYPE(port_attr->mac_id))  /* HSS28G */
    {
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_QSGMII);
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_USXGMII_S);
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_USXGMII_M2G5);
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_USXGMII_M5G);
    }
    else  /* HSS15G */
    {
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_KR2);
        tmp_if_type_bitmap &= ~(1 << CTC_PORT_IF_CR2);
    }
    *if_type_bitmap = tmp_if_type_bitmap;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_serdes_info_capability(uint8 mac_id, uint16 serdes_id, uint8 serdes_num, uint8 pcs_mode,
                                                    uint8 flag, ctc_port_serdes_info_t* p_serdes_port)
{
    uint8  loop = 0;

    CTC_PTR_VALID_CHECK(p_serdes_port);
    p_serdes_port->serdes_id = serdes_id;
    p_serdes_port->serdes_num = serdes_num;
    p_serdes_port->serdes_mode = pcs_mode;
    for (loop = 0; loop < serdes_num; loop++)
    {
        p_serdes_port->serdes_id_array[loop] = serdes_id + loop;
    }
    if(CTC_CHIP_SERDES_LG_MODE == pcs_mode)
    {
        if(0 == flag)
        {
            p_serdes_port->serdes_id_array[1] = serdes_id + 1;
        }
        else
        {
            p_serdes_port->serdes_id_array[1] = (0 == (mac_id % 4)) ? (serdes_id + 1) : (serdes_id + 3);
        }
    }

    return CTC_E_NONE;
}

/*enable 1:link up txactiv  0:link down, foreoff*/
int32
_sys_usw_mac_restore_led_mode(uint8 lchip, uint32 gport, uint32 enable)
{
    ctc_chip_led_para_t led_para;
    ctc_chip_mac_led_type_t led_type;
    uint8 first_led_mode = 0;
    uint8 sec_led_mode = 0;
    uint8 is_first_need = 0;
    uint8 is_sec_need = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_MAC_NOT_USED;
    }

    first_led_mode = port_attr->first_led_mode;
    sec_led_mode = port_attr->sec_led_mode;
    if ((CTC_CHIP_TXLINK_TXACTIVITY_MODE == first_led_mode) || (CTC_CHIP_RXLINK_BIACTIVITY_MODE == first_led_mode) ||
        (CTC_CHIP_TXACTIVITY_MODE == first_led_mode) || (CTC_CHIP_BIACTIVITY_MODE == first_led_mode))
    {
        is_first_need = 1;
    }

    if ((CTC_CHIP_TXLINK_TXACTIVITY_MODE == first_led_mode) || (CTC_CHIP_RXLINK_BIACTIVITY_MODE == first_led_mode) ||
        (CTC_CHIP_TXACTIVITY_MODE == first_led_mode) || (CTC_CHIP_BIACTIVITY_MODE == first_led_mode))
    {
        is_sec_need = 1;
    }

    if (is_first_need || is_sec_need)
    {
        sal_memset(&led_para, 0, sizeof(ctc_chip_led_para_t));
        led_para.ctl_id = 0;
        led_para.lchip = lchip;
        led_para.port_id = port_attr->mac_id;
        led_para.op_flag |= CTC_CHIP_LED_MODE_SET_OP;
        led_para.first_mode = (is_first_need? (enable?first_led_mode:CTC_CHIP_FORCE_OFF_MODE):first_led_mode);
        led_para.sec_mode = (is_sec_need?(enable?sec_led_mode:CTC_CHIP_FORCE_OFF_MODE):sec_led_mode);
        led_type = (sec_led_mode !=  CTC_CHIP_MAC_LED_MODE)?CTC_CHIP_USING_TWO_LED:CTC_CHIP_USING_ONE_LED;
        CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_mode(lchip, &led_para, led_type, 1));
    }

    return CTC_E_NONE;
}

#define  __SYS_MAC_API__

int32
sys_usw_set_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value)
{
    int32  step = 0;
    uint32 cmd;
    uint32 field = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    if (index > CTC_FRAME_SIZE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((value < SYS_USW_MIN_FRAMESIZE_MIN_VALUE)
       || (value > SYS_USW_MIN_FRAMESIZE_MAX_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(CTC_FRAME_SIZE_7 == index && SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE < value )
    {
        return CTC_E_INVALID_PARAM;
    }

    field = value;
    step = NetRxLenChkCtl_cfgMinLen1_f - NetRxLenChkCtl_cfgMinLen0_f;
    cmd = DRV_IOW(NetRxLenChkCtl_t, NetRxLenChkCtl_cfgMinLen0_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    return CTC_E_NONE;
}

int32
sys_usw_get_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value)
{
    int32  step = 0;
    uint32 cmd;
    uint32 field = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    if (index > CTC_FRAME_SIZE_MAX)
    {
        return CTC_E_INVALID_FRAMESIZE_INDEX;
    }

    CTC_PTR_VALID_CHECK(p_value);

    step = NetRxLenChkCtl_cfgMinLen1_f - NetRxLenChkCtl_cfgMinLen0_f;
    cmd = DRV_IOR(NetRxLenChkCtl_t, NetRxLenChkCtl_cfgMinLen0_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    *p_value = field;

    return CTC_E_NONE;
}

int32
sys_usw_set_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value)
{
    int32  step = 0;
    uint32 cmd;
    uint32 field = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    if (index > CTC_FRAME_SIZE_MAX)
    {
        return CTC_E_INVALID_FRAMESIZE_INDEX;
    }

    if ((value < SYS_USW_MAX_FRAMESIZE_MIN_VALUE) || (value > SYS_USW_MAX_FRAMESIZE_MAX_VALUE))
    {
        return CTC_E_INVALID_PARAM;
    }

    if((CTC_FRAME_SIZE_7 == index) && (SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE > value) )
    {
        return CTC_E_INVALID_PARAM;
    }

    field = value;
    step = NetRxLenChkCtl_cfgMaxLen1_f - NetRxLenChkCtl_cfgMaxLen0_f;
    cmd = DRV_IOW(NetRxLenChkCtl_t, NetRxLenChkCtl_cfgMaxLen0_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    return CTC_E_NONE;
}

int32
sys_usw_get_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value)
{
    int32  step = 0;

    uint32 cmd;
    uint32 field = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    CTC_PTR_VALID_CHECK(p_value);

    step = NetRxLenChkCtl_cfgMaxLen1_f - NetRxLenChkCtl_cfgMaxLen0_f;
    cmd = DRV_IOR(NetRxLenChkCtl_t, NetRxLenChkCtl_cfgMaxLen0_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    *p_value = field;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable)
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

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, enable));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_mac_en(uint8 lchip, uint32 gport, uint32* p_enable)
{
    uint16  lport   = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }
    CTC_PTR_VALID_CHECK(p_enable);

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;

    }

    *p_enable = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en ? TRUE : FALSE;
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_speed(uint8 lchip, uint32 gport, ctc_port_speed_t speed_mode)
{
    uint16 lport      = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_SPEED, speed_mode));
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
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
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
            port_attr->speed_mode = speed_mode;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_config(lchip, lport, FALSE));
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
        }
        else
        {
            port_attr->speed_mode = speed_mode;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_config(lchip, lport, FALSE));
        }
        /* update NetTxPortStaticInfo_portMode_f */
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_datapath_set_nettx_portmode(lchip, port_attr->chan_id));
    }
    else
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " PCS mode %d DO NOT support to config mac speed\n", port_attr->pcs_mode);
        return CTC_E_INVALID_PARAM;
    }
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_speed(uint8 lchip, uint32 gport, ctc_port_speed_t* p_speed_mode)
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

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;

    }
    *p_speed_mode = port_attr->speed_mode;

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_fec_en(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport = 0;
    uint32 curr_fec_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X, fec type:%d\n", gport, value);

    if(value >= CTC_PORT_FEC_TYPE_MAX)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Unsupported FEC type\n");
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_mac_get_fec_en(lchip, gport, &curr_fec_en));
    if (curr_fec_en == value)
    {
        /* same config, do nothing */
        return CTC_E_NONE;
    }

    /* get info from gport */
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode))
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " FEC not supported\n");
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, value));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_fec_en(uint8 lchip, uint32 gport, uint32 *p_value)
{
    uint16 lport = 0;
    uint32 value = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_PTR_VALID_CHECK(p_value);

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode))
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " FEC not supported\n");
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_fec_en(lchip, lport, &value));
    *p_value = value;

    MAC_UNLOCK;

    return CTC_E_NONE;
}

/* support 802.3ap, Auto-Negotiation for Backplane Ethernet */
int32
sys_usw_mac_set_cl73_ability(uint8 lchip, uint32 gport, uint32 ability)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8  serdes_num = 0;
    sys_datapath_an_ability_t cl73_ability;
    uint8  cnt = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, ability:0x%X\n", gport, ability);

    /* get port info from sw table */
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        MAC_UNLOCK;
        return CTC_E_INVALID_CONFIG;

    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    sal_memset(&cl73_ability, 0, sizeof(sys_datapath_an_ability_t));
    /* 1. get ability */
    if(ability & CTC_PORT_CL73_10GBASE_KR)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_10GBASE_KR);
    }
    if(ability & CTC_PORT_CL73_40GBASE_KR4)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_40GBASE_KR4);
    }
    if(ability & CTC_PORT_CL73_40GBASE_CR4)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_40GBASE_CR4);
    }
    if(ability & CTC_PORT_CL73_100GBASE_KR4)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_100GBASE_KR4);
    }
    if(ability & CTC_PORT_CL73_100GBASE_CR4)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_100GBASE_CR4);
    }
    if(ability & CTC_PORT_CL73_25GBASE_KRS)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_25GBASE_KR_S);
    }
    if(ability & CTC_PORT_CL73_25GBASE_KR)
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_25GBASE_KR|SYS_PORT_CL73_25GBASE_KR_S);
    }
    if(ability & CTC_PORT_CL73_25G_RS_FEC_REQUESTED)
    {
        cl73_ability.base_ability1 |= (SYS_PORT_CL73_25G_RS_FEC_REQ);
    }
    if(ability & CTC_PORT_CL73_25G_BASER_FEC_REQUESTED)
    {
        cl73_ability.base_ability1 |= (SYS_PORT_CL73_25G_BASER_FEC_REQ);
    }
    if(ability & CTC_PORT_CL73_FEC_ABILITY)
    {
        cl73_ability.base_ability1 |= (SYS_PORT_CL73_FEC_SUP);
    }
    if(ability & CTC_PORT_CL73_FEC_REQUESTED)
    {
        cl73_ability.base_ability1 |= (SYS_PORT_CL73_FEC_REQ);
    }
    if(ability & CTC_PORT_CSTM_25GBASE_KR1)
    {
        cl73_ability.np1_ability0 |= (SYS_PORT_CSTM_25GBASE_KR1);
    }
    if(ability & CTC_PORT_CSTM_25GBASE_CR1)
    {
        cl73_ability.np1_ability0 |= (SYS_PORT_CSTM_25GBASE_CR1);
    }
    if(ability & CTC_PORT_CSTM_50GBASE_KR2)
    {
        cl73_ability.np1_ability0 |= (SYS_PORT_CSTM_50GBASE_KR2);
    }
    if(ability & CTC_PORT_CSTM_50GBASE_CR2)
    {
        cl73_ability.np1_ability0 |= (SYS_PORT_CSTM_50GBASE_CR2);
    }
    if(ability & CTC_PORT_CSTM_RS_FEC_ABILITY)
    {
        cl73_ability.np1_ability1 |= (SYS_PORT_CSTM_CL91_FEC_SUP);
    }
    if(ability & CTC_PORT_CSTM_BASER_FEC_ABILITY)
    {
        cl73_ability.np1_ability1 |= (SYS_PORT_CSTM_CL74_FEC_SUP);
    }
    if(ability & CTC_PORT_CSTM_RS_FEC_REQUESTED)
    {
        cl73_ability.np1_ability1 |= (SYS_PORT_CSTM_CL91_FEC_REQ);
    }
    if(ability & CTC_PORT_CSTM_BASER_FEC_REQUESTED)
    {
        cl73_ability.np1_ability1 |= (SYS_PORT_CSTM_CL74_FEC_REQ);
    }
    if((cl73_ability.np1_ability0) || (cl73_ability.np1_ability1))
    {
        cl73_ability.base_ability0 |= (SYS_PORT_CL73_NEXT_PAGE);
    }

    /* 2. cfg ability */
    for(cnt=0; cnt<serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_serdes_auto_neg_ability(lchip, real_serdes, &cl73_ability));
    }
    MAC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability)
{
    /* get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl73_ability(lchip, gport, type, p_ability));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32 value)
{
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 field_id  = 0;
    uint16 step      = 0;
    uint32 mode_value= 0;
    uint16 lport     = 0;
    uint32 curr_val  = 0;
    uint32 tmp_val32 = 0;
    uint8 pcs_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSgmii0Cfg0_m sgmii_cfg;
    QsgmiiPcsAneg0Cfg0_m  qsgmii_cfg;
    UsxgmiiPcsAneg0Cfg0_m usxgmii_cfg;
    SharedMii0Cfg0_m      mii_per_cfg;
    SharedPcsCfg0_m       pcs_cfg;
    uint8  mii_idx = 0;
    uint32 enable = 0;
    uint32 port_step = 0;
    uint32 mcu_val   = 0;
    uint32 unidir_en = 0;

    SYS_MAC_INIT_CHECK();

    sal_memset(&sgmii_cfg, 0, sizeof(SharedPcsSgmii0Cfg0_m));
    sal_memset(&qsgmii_cfg, 0, sizeof(QsgmiiPcsAneg0Cfg0_m));
    sal_memset(&usxgmii_cfg, 0, sizeof(UsxgmiiPcsAneg0Cfg0_m));
    sal_memset(&mii_per_cfg, 0, sizeof(SharedMii0Cfg0_m));
    sal_memset(&pcs_cfg, 0, sizeof(SharedPcsCfg0_m));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport) && (CTC_PORT_PROP_AUTO_NEG_EN ==  type))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, value));
    }

    if (CTC_PORT_PROP_AUTO_NEG_EN == type)
    {
        CTC_ERROR_RETURN(sys_usw_mac_get_auto_neg(lchip, gport, type, &curr_val));
        if ((curr_val && value) || ((!curr_val)&&(!value)))
        {
            /* No change */
            return CTC_E_NONE;
        }
    }
    /* if unidir enable, cannot set AN enable */
    CTC_ERROR_RETURN(_sys_usw_mac_get_unidir_en(lchip, lport, &unidir_en));
    if((CTC_PORT_PROP_AUTO_NEG_EN == type) && value && unidir_en)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Gport %u has enable unidir, cannot enable auto-nego! \n", gport);
        return CTC_E_INVALID_CONFIG;
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    mii_idx          = port_attr->mii_idx;
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
            if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode))
            {
                step = SharedPcsSgmii1Cfg0_t - SharedPcsSgmii0Cfg0_t;
                tbl_id = SharedPcsSgmii0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
                field_id = SharedPcsSgmii0Cfg0_anEnable0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &sgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &sgmii_cfg));
            }
            else if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
            {
                step = QsgmiiPcsAneg1Cfg0_t - QsgmiiPcsAneg0Cfg0_t;
                tbl_id = QsgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
                field_id = QsgmiiPcsAneg0Cfg0_anEnable0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &qsgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &qsgmii_cfg));
            }
            else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
            {
                step = UsxgmiiPcsAneg1Cfg0_t - UsxgmiiPcsAneg0Cfg0_t;
                tbl_id = UsxgmiiPcsAneg0Cfg0_t + internal_mac_idx*step + pcs_idx;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
                field_id = UsxgmiiPcsAneg0Cfg0_anEnable0_f;
                DRV_IOW_FIELD(lchip, tbl_id, field_id, &value, &usxgmii_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &usxgmii_cfg));
            }
        }
        /* Cfg auto MODE */
        else if(CTC_PORT_PROP_AUTO_NEG_MODE == type)
        {
            /* sdk only support two mode, 1000Base-X(2'b00), SGMII-Slaver(2'b10) */
            if(CTC_PORT_AUTO_NEG_MODE_1000BASE_X == value)
            {
                mode_value = 0;
            }
            else if(CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER == value)
            {
                mode_value = 1;
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
            enable = (value ? 1 : 0);
            port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mcu_chip_read(lchip, (port_step+0x8), &mcu_val));
            if (enable)
            {
                mcu_val |= (1 << 8);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mcu_chip_write(lchip, (port_step+0x8), mcu_val));

                if (CTC_CHIP_SERDES_CG_MODE != port_attr->pcs_mode)
                {
                    /* IgnoreFault */
                    step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
                    tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
                    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));
                    tmp_val32 = 1;
                    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLocalFault0_f, &tmp_val32, &mii_per_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreRemoteFault0_f, &tmp_val32, &mii_per_cfg);
                    DRV_IOW_FIELD(lchip, tbl_id, SharedMii0Cfg0_cfgMiiIgnoreLintFault0_f, &tmp_val32, &mii_per_cfg);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &mii_per_cfg));

                    /* NetTxWriteEn */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                }
            }
            else
            {
                mcu_val &= ~(1 << 8);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mcu_chip_write(lchip, (port_step+0x8), mcu_val));
            }

            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_cl73_auto_neg_en(lchip, gport, enable));

        }
        else if (CTC_PORT_PROP_AUTO_NEG_FEC == type)
        {
            port_attr->an_fec = 0;
            if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode))
            {
                MAC_UNLOCK;
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " FEC not supported\n");
                return CTC_E_INVALID_PARAM;
            }
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

int32
sys_usw_mac_get_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32* p_value)
{
    uint16 lport     = 0;
    uint32 value     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_INIT_CHECK();

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport) && (CTC_PORT_PROP_AUTO_NEG_EN == type))
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, p_value));
        return CTC_E_NONE;
    }

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode) ||
        (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl37_auto_neg(lchip, lport, type, &value));
    }
    else
    {
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl73_auto_neg(lchip, lport, type, &value));
    }

    *p_value = value;
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_eee_en(uint8 lchip, uint32 gport, bool enable)
{
#ifdef NEVER
    uint16 lport     = 0;
    uint16 tbl_id    = 0;
    uint8  step      = 0;
    uint8  mac_id    = 0;
    uint32 mode_sel  = 0;
    uint32 value[2]  = {0};
    uint32 cmd       = 0;
    uint8  index     = 0;
    uint8  pcs_idx   = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetTxEEECfg_m   eee_cfg;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport: %d, enable: %d\n", gport, enable);
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    sal_memset(&eee_cfg, 0, sizeof(NetTxEEECfg_m));

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_get_mac_mii_pcs_index(lchip, port_attr->mac_id, port_attr->serdes_id,
                                                                   port_attr->pcs_mode, NULL, NULL, &pcs_idx));

    /* Only SGMII/QSGMII/SGMII-2.5G support EEE */
    if ((CTC_CHIP_SERDES_SGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
        && (CTC_CHIP_SERDES_2DOT5G_MODE != port_attr->pcs_mode))
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    /* #1, cfg NetTxEEECfg, NOTE: bitmap for mac_id */
    tbl_id = NetTxEEECfg_t;
    mac_id = port_attr->mac_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &eee_cfg));
    if ((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode))
    {
        mode_sel = 0;
    }
    else
    {
        mode_sel = 1;
    }
    SetNetTxEEECfg(V, eeePortModeSel_f, &eee_cfg, mode_sel);

    if(mac_id < 32)
    {
        if(enable)
        {
            value[0] |= (1<<mac_id);
        }
        else
        {
            value[0] &= (~(1<<mac_id));
        }
    }
    else
    {
        if(enable)
        {
            value[1] |= (1<<(mac_id-32));
        }
        else
        {
            value[1] &= (~(1<<(mac_id-32)));
        }
    }
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac_id: %d, value[0]: 0x%X, value[1]: 0x%X\n", mac_id, value[0], value[1]);
    SetNetTxEEECfg(A, eeeCtlEn_f, &eee_cfg, value);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &eee_cfg));

    /* #2 NetTxEEESleepTimerCfg config */

    /* #3 NetTxEEEWakeupTimerCfg config */

    /* #4, PCS config */
    internal_mac_idx = port_attr->mac_id % 4;
    value[0] = enable ? 1 : 0;

    if (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
    {

    }
    /* cfg UsxgmiiPcsEeeCtl */
    else if ((CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode))
    {
        step = UsxgmiiPcsEeeCtl10_t - UsxgmiiPcsEeeCtl00_t;
        tbl_id = UsxgmiiPcsEeeCtl00_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOW(tbl_id, UsxgmiiPcsEeeCtl00_rxEeeEn0_f);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value[0]));
        cmd = DRV_IOW(tbl_id, UsxgmiiPcsEeeCtl00_txEeeEn0_f);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value[0]));
    }
    /* cfg SharedPcsEeeCtl */
    else
    {
        step = SharedPcsEeeCtl10_t - SharedPcsEeeCtl00_t;
        tbl_id = SharedPcsEeeCtl00_t + internal_mac_idx*step + pcs_idx;
        cmd = DRV_IOW(tbl_id, SharedPcsEeeCtl00_rxEeeEn0_f);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value[0]));
        cmd = DRV_IOW(tbl_id, SharedPcsEeeCtl00_txEeeEn0_f);
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, index, cmd, &value[0]));
    }

    MAC_UNLOCK;

    return CTC_E_NONE;
#else
    return CTC_E_NOT_SUPPORT;
#endif
}

int32
sys_usw_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport      = 0;

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_UNIDIR_EN, enable));
    }
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_unidir_en(lchip, lport, enable));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_unidir_en(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport = 0;
    uint32 value = 0;

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_UNIDIR_EN, p_value));
        return CTC_E_NONE;
    }
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_unidir_en(lchip, lport, &value));
    MAC_UNLOCK;

    *p_value = value;
    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_ipg(uint8 lchip, uint16 lport, uint32 value)
{
    uint16 step   = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint8  mii_idx = 0;
    uint8  lg_index = 0;
    uint32 cmd = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    if((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) ||
       (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
       (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode) ||
       (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode))
    {
        if((4 != value) && (8 != value) && (12 != value))
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MAC-IPG value only support 4/8/12 \n");
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        if((8 != value) && (12 != value))
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MAC-IPG value only support 8/12 \n");
            return CTC_E_INVALID_PARAM;
        }
    }

    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if ((CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        tbl_id = SharedMii0Cfg0_t + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
        step = SharedMii2Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + lg_index*step + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode))
    {
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_ipg(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport     = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, enable:0x%X\n", gport, value);

    /* get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_ipg(lchip, lport, value));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_ipg(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 step   = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint8 mii_idx = 0;
    uint8 internal_mac_idx = 0;
    uint8 lg_index = 0;
    uint32 cmd = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    /* get port info from sw table */
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

    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    if (internal_mac_idx <= 1)
    {
        lg_index = 0;
    }
    else
    {
        lg_index = 1;
    }

    if ((CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
    {
        tbl_id = SharedMii0Cfg0_t + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else if (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode)
    {
        step = SharedMii2Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + lg_index*step + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else if ((CTC_CHIP_SERDES_XFI_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == port_attr->pcs_mode))
    {
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        fld_id = SharedMii0Cfg0_cfgMiiTxIpgLen0_f;
    }
    else
    {
        MAC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(tbl_id, fld_id);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, 0, cmd, p_value));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *fc_prop)
{
    uint16 gport                    = 0;
    uint8 dir                       = 0;
    uint16 lport                    = 0;
    uint8 chan_id                   = 0;
    uint8 lchan_id                  = 0;
    uint8 mac_id                    = 0;
    uint8 lmac_id                   = 0;
    uint32 cmd                      = 0;
    uint32 field_val                = 0;
    uint32 tbl_id                   = 0;
    uint8  step                     = 0;
    uint32 rx_pause_type[2]         = {0};
    uint32 index = 0;

    NetRxPauseReqCtl_m rx_pause_ctl;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;
    Sgmac0TxPauseCfg0_m tx_pause_cfg;
    DsIrmPortStallEn_m  irm_stall_en;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&rx_pause_ctl, 0, sizeof(NetRxPauseReqCtl_m));
    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    sal_memset(&tx_pause_cfg, 0, sizeof(Sgmac0TxPauseCfg0_m));
    sal_memset(&irm_stall_en, 0, sizeof(DsIrmPortStallEn_m));

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X \n", fc_prop->gport);

    gport = fc_prop->gport;
    dir = fc_prop->dir;
    CTC_MAX_VALUE_CHECK(fc_prop->priority_class, 7);

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    chan_id = port_attr->chan_id;
    mac_id  = port_attr->mac_id;
    lchan_id = chan_id&0x3F;
    lmac_id =  mac_id&0x3F;

    MAC_UNLOCK;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "chan_id:%d, lmac_id:%d\n", chan_id, lmac_id);

    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        /*Rx port set pause type: fc/pfc*/
        cmd = DRV_IOR(NetRxPauseReqCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_ctl));
        GetNetRxPauseReqCtl(A, netRxPauseType_f, &rx_pause_ctl, rx_pause_type);
        if (fc_prop->is_pfc && fc_prop->enable)
        {
            CTC_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F));
        }
        else if( !fc_prop->is_pfc && fc_prop->enable)
        {
            CTC_BIT_UNSET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F));
        }

        SetNetRxPauseReqCtl(A, netRxPauseType_f, &rx_pause_ctl, rx_pause_type);
        cmd = DRV_IOW(NetRxPauseReqCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_ctl));

        /*FC/PFC enable/disable*/
        step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
        tbl_id = Sgmac0RxPauseCfg0_t + port_attr->sgmac_idx + ((mac_id % 4)*step);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));

        if (fc_prop->is_pfc)
        {
            field_val = (fc_prop->enable) ? 1 : 0;
            if (fc_prop->enable)
            {
                SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPfcPauseEn_f, &rx_pause_cfg, field_val);
            }
            field_val = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPfcPriorityEn_f, &rx_pause_cfg);
            (fc_prop->enable) ? CTC_BIT_SET(field_val, (fc_prop->priority_class))
                              : CTC_BIT_UNSET(field_val, (fc_prop->priority_class));
            SetSgmac0RxPauseCfg0(V, cfgSgmac0RxPfcPriorityEn_f, &rx_pause_cfg, field_val);
        }
        else
        {
            field_val = (fc_prop->enable) ? 1 : 0;
            SetSgmac0RxPauseCfg0(V, cfgSgmac0RxNormPauseEn_f, &rx_pause_cfg, field_val);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));

    }

    /* egress only global contral */
    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        /*set xon/xoff profile*/
        if (fc_prop->enable)
        {
            CTC_ERROR_RETURN(sys_usw_qos_flow_ctl_profile(lchip, gport,
                                                          fc_prop->priority_class,
                                                          !fc_prop->is_pfc,
                                                          0));
        }

        CTC_ERROR_RETURN(sys_usw_qos_flow_ctl_profile(lchip, gport,
                                                      fc_prop->priority_class,
                                                      fc_prop->is_pfc,
                                                      fc_prop->enable));
        if (!fc_prop->enable)
        {
            do
            {
                if (fc_prop->is_pfc)
                {
                    cmd = DRV_IOR(DsIrmPortStallInfoRec_t, DsIrmPortStallInfoRec_u_g1_portTcStallValid_f);
                }
                else
                {
                    fc_prop->priority_class = 0;
                    cmd = DRV_IOR(DsIrmPortStallInfoRec_t, DsIrmPortStallInfoRec_portStallValid_f);
                }
                DRV_IOCTL(lchip, lchan_id, cmd, &field_val);

                if (!CTC_IS_BIT_SET(field_val, fc_prop->priority_class))
                {
                    break;
                }

                sal_task_sleep(10);
                if ((index++) > 50)
                {
                    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cannot clear stall status\n");
                    return CTC_E_HW_FAIL;
                }
            }
            while (1);
        }

        /*Rx port set pause type: fc/pfc*/
        cmd = DRV_IOR(NetRxPauseReqCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_ctl));
        GetNetRxPauseReqCtl(A, netRxPauseType_f, &rx_pause_ctl, rx_pause_type);
        if (fc_prop->is_pfc)
        {
            if (fc_prop->enable)
            {
                CTC_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F));
            }
        }
        else
        {
            if (fc_prop->enable)
            {
                CTC_BIT_UNSET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F));
            }
        }
        SetNetRxPauseReqCtl(A, netRxPauseType_f, &rx_pause_ctl, rx_pause_type);
        cmd = DRV_IOW(NetRxPauseReqCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_ctl));

        /*Tx port set pause type: fc/pfc*/
        step = Sgmac1TxPauseCfg0_t - Sgmac0TxPauseCfg0_t;
        tbl_id = Sgmac0TxPauseCfg0_t + port_attr->sgmac_idx + ((mac_id % 4)*step);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));

        if (fc_prop->is_pfc)
        {
            if(fc_prop->enable)
            {
                SetSgmac0TxPauseCfg0(V, cfgSgmac0TxPauseType_f, &tx_pause_cfg, 1);
            }
        }
        else
        {
            if (fc_prop->enable)
            {
                SetSgmac0TxPauseCfg0(V, cfgSgmac0TxPauseType_f, &tx_pause_cfg, 0);
            }
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));

        /*Irm FC/PFC enable/disable*/
        cmd = DRV_IOR(DsIrmPortStallEn_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lchan_id, cmd, &irm_stall_en));
        if (fc_prop->is_pfc)
        {
            field_val = GetDsIrmPortStallEn(V, u_g1_portPfcEnVec_f, &irm_stall_en);
            (fc_prop->enable) ? CTC_BIT_SET(field_val, (fc_prop->priority_class))
                              : CTC_BIT_UNSET(field_val, (fc_prop->priority_class));
            SetDsIrmPortStallEn(V, u_g1_portPfcEnVec_f, &irm_stall_en, field_val);
            if (fc_prop->enable)
            {
                SetDsIrmPortStallEn(V, portFcEn_f, &irm_stall_en, 0);
            }
        }
        else if(fc_prop->enable)
        {
            SetDsIrmPortStallEn(V, portFcEn_f, &irm_stall_en, 1);
        }
        else
        {
            SetDsIrmPortStallEn(V, portFcEn_f, &irm_stall_en, 0);
        }

        cmd = DRV_IOW(DsIrmPortStallEn_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lchan_id, cmd, &irm_stall_en));

    }

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *fc_prop)
{
    uint16 gport                    = 0;
    uint8 dir                       = 0;
    uint16 lport                    = 0;
    uint8 chan_id                   = 0;
    uint8 mac_id                    = 0;
    uint8 lmac_id                   = 0;
    uint32 cmd                      = 0;
    uint32 field_val                = 0;
    uint32 tbl_id                   = 0;
    uint8  step                     = 0;
    uint8 lchan_id                  = 0;
    uint32 rx_pause_type[2]         = {0};

    NetRxPauseReqCtl_m  rx_pause_ctl;
    Sgmac0RxPauseCfg0_m rx_pause_cfg;
    Sgmac0TxPauseCfg0_m tx_pause_cfg;
    DsIrmPortStallEn_m  irm_stall_en;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&rx_pause_cfg, 0, sizeof(Sgmac0RxPauseCfg0_m));
    sal_memset(&tx_pause_cfg, 0, sizeof(Sgmac0TxPauseCfg0_m));
    sal_memset(&irm_stall_en, 0, sizeof(DsIrmPortStallEn_m));
    sal_memset(&rx_pause_ctl, 0, sizeof(NetRxPauseReqCtl_m));

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X \n", fc_prop->gport);

    gport = fc_prop->gport;
    dir = fc_prop->dir;
    CTC_MAX_VALUE_CHECK(fc_prop->priority_class, 7);
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    chan_id = port_attr->chan_id;
    mac_id  = port_attr->mac_id;
    lchan_id = chan_id&0x3F;
    lmac_id =  mac_id&0x3F;

    MAC_UNLOCK;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "chan_id:%d, lmac_id:%d\n", chan_id, lmac_id);
    cmd = DRV_IOR(NetRxPauseReqCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_ctl));
    GetNetRxPauseReqCtl(A, netRxPauseType_f, &rx_pause_ctl, rx_pause_type);

    if (CTC_INGRESS == dir)
    {
        /*FC/PFC enable/disable*/
        step = Sgmac1RxPauseCfg0_t - Sgmac0RxPauseCfg0_t;
        tbl_id = Sgmac0RxPauseCfg0_t + port_attr->sgmac_idx + ((mac_id % 4)*step);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_pause_cfg));
        field_val = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxPfcPriorityEn_f, &rx_pause_cfg);

        if (fc_prop->is_pfc && CTC_IS_BIT_SET(field_val, (fc_prop->priority_class)) &&
            CTC_IS_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F)))
        {
            fc_prop->enable = CTC_IS_BIT_SET(field_val, (fc_prop->priority_class));
        }
        else if(!fc_prop->is_pfc && GetSgmac0RxPauseCfg0(V, cfgSgmac0RxNormPauseEn_f, &rx_pause_cfg) &&
            !CTC_IS_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F)))
        {
            fc_prop->enable = GetSgmac0RxPauseCfg0(V, cfgSgmac0RxNormPauseEn_f, &rx_pause_cfg);
        }
    }

    /* egress only global contral */
    if (CTC_EGRESS == dir)
    {
        /*Irm FC/PFC enable/disable*/
        cmd = DRV_IOR(DsIrmPortStallEn_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lchan_id, cmd, &irm_stall_en));
        if (fc_prop->is_pfc && CTC_IS_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F)))
        {
            field_val = GetDsIrmPortStallEn(V, u_g1_portPfcEnVec_f, &irm_stall_en);
            fc_prop->enable = CTC_IS_BIT_SET(field_val, (fc_prop->priority_class));
        }
        else if(!fc_prop->is_pfc && !CTC_IS_BIT_SET(rx_pause_type[lmac_id >> 5], (lmac_id&0x1F)))
        {
            field_val = GetDsIrmPortStallEn(V, portFcEn_f, &irm_stall_en);
            fc_prop->enable = field_val;
        }
    }

    return CTC_E_NONE;
}



int32
sys_usw_mac_get_monitor_rlt(uint8 lchip)
{
    uint16 index = 0;
    sal_time_t tv;
    char* p_time_str = NULL;

    sys_usw_scan_log_t* p_scan_log = NULL;

    LCHIP_CHECK(lchip);
    SYS_MAC_INIT_CHECK();

    p_scan_log = p_usw_mac_master[lchip]->scan_log;

    sal_time(&tv);
    p_time_str = sal_ctime(&tv);

    if(DRV_IS_DUET2(lchip))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"############################################################\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"# 1. Type bitmap:                                          #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  +---+---+---+     bit[0] : Code Error                   #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  | 2 | 1 | 0 |     bit[1] : PCS Not Up                   #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  +---+---+---+     bit[2] : SerDes No Random Flag        #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#                                                          #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"# 2. Type value :                                          #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  1:  Code Error                                          #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  2:  PCS Not Up                                          #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  4:  SerDes No Random Flag                               #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  3:  Code Error + PCS Not Up                             #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  5:  Code Error + SerDes No Random Flag                  #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  6:  PCS Not Up + SerDes No Random Flag                  #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"#  7:  Code Error + PCS Not Up + SerDes No Random Flag     #\n");
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"############################################################\n");
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"TIME: %s", p_time_str);

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"===============================================\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s  %-8s %-50s\n", "Type", "GPORT", "System Time");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------\n");

    for (index = 0; index < SYS_PORT_MAX_LOG_NUM; index++)
    {
        if (p_scan_log[index].valid)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5d  %5d %-50s\n", p_scan_log[index].type,
               p_scan_log[index].gport, p_scan_log[index].time_str);
        }
    }
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"===============================================\n");

    return CTC_E_NONE;
}


int32
sys_usw_mac_get_3ap_training_en(uint8 lchip, uint32 gport, uint32* p_status)
{
    uint16 lport   = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /* 1.get port info from sw table */
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

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_3ap_training_en(lchip, lport, p_status));
    MAC_UNLOCK;

    return CTC_E_NONE;
}


/* support 802.3ap, Auto-Negotiation for Backplane Ethernet */
int32
sys_usw_mac_set_3ap_training_en(uint8 lchip, uint32 gport, uint32 enable)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt = 0;

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d,  cl72 enable flag:%d\n", gport, enable);

    /* 1.get port info from sw table */
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    slice_id = port_attr->slice_id;
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));

    for(cnt=0; cnt<serdes_num; cnt++)
    {
        GET_SERDES(port_attr->mac_id, serdes_id, cnt, port_attr->flag, port_attr->pcs_mode, real_serdes);
        if(MCHIP_API(lchip)->serdes_set_link_training_en)
        {
            CTC_ERROR_RETURN(MCHIP_API(lchip)->serdes_set_link_training_en(lchip, real_serdes, enable));
        }
    }
    return CTC_E_NONE;
}
/*reserved for some special application*/
int32
sys_usw_mac_get_code_error_count(uint8 lchip, uint32 gport, uint32* p_value)
{
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 lport = 0;

    if (NULL == p_usw_port_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_NOT_READY;
    }

    *p_value = port_attr->code_err_count;

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_mac_rx_en(uint8 lchip, uint32 gport, uint32 value)
{
    uint32 cmd      = 0;
    uint32 tbl_id   = 0;
    uint16 step = 0;
    uint8 sgmac_idx = 0;
    uint8 internal_mac_idx = 0;
    Sgmac0RxCfg0_m  mac_rx_cfg;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    value=value?1:0;
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));

    sgmac_idx        = port_attr->sgmac_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    MAC_UNLOCK;

    /* cfg sgmac Rx*/
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxPktEn_f, &value, &mac_rx_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));

    return CTC_E_NONE;
}

int32
sys_usw_mac_link_down_event(uint8 lchip, uint32 gport)
{
    /* SYSTEM modified by tongzb for TM interrupt, do interrupt disable in isr */
#if 0  
    /*disable interrupt*/
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, FALSE));
#endif
    CTC_ERROR_RETURN(_sys_usw_mac_restore_led_mode(lchip, gport, FALSE));

    return CTC_E_NONE;
}

int32
sys_usw_mac_link_up_event(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    uint32 value = 0;
    uint32 auto_neg_en = 0;
    uint32 auto_neg_mode = 0;
    uint32 mac_speed = 0;
    uint32 speed = 0;
    uint32 remote_link = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ctc_chip_serdes_loopback_t ilb_param;
    ctc_chip_serdes_loopback_t elb_param;

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_mac_pcs_link_fault_reset(lchip, gport));

    /*enable interrupt*/
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, TRUE));

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
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&ilb_param));
        elb_param.serdes_id = port_attr->serdes_id;
        elb_param.mode = 1;
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&elb_param));
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

    CTC_ERROR_RETURN(_sys_usw_mac_restore_led_mode(lchip, gport, TRUE));

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_link_intr(uint8 lchip, uint32 gport, uint8 enable)
{
    uint16 lport = 0;
    uint16 mac_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_index = 0;
    ds_t   ds;

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

    mac_id = port_attr->mac_id;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port link interrupt, gport:%d, mac_id:%d, enable:%d\n", \
                     gport, mac_id, enable);
    if (enable)
    {
        tb_index = 3;
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_clear_link_intr(lchip, gport));
    }
    else
    {
        tb_index = 2;
    }
    sal_memset(ds, 0, sizeof(ds));
    _sys_usw_mac_get_table_by_mac_id(lchip, mac_id, &tb_id, ds);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(DRV_IOCTL(lchip, tb_index, cmd, ds));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_link_intr(uint8 lchip, uint32 gport, uint32* enable)
{
    uint16 lport = 0;
    uint16 mac_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_index = 0;
    uint32 value[4] = {0};
    uint32 bitmap[4] = {0};

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
    mac_id = port_attr->mac_id;
    MAC_UNLOCK;

    tb_index = 2;
    _sys_usw_mac_get_table_by_mac_id(lchip, mac_id, &tb_id, bitmap);

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tb_index, cmd, value));
    value[0] &= bitmap[0];
    value[1] &= bitmap[1];
    value[2] &= bitmap[2];
    value[3] &= bitmap[3];

    if(value[0] || value[1] || value[2] || value[3])
    {
        *enable = 0;
    }
    else
    {
        *enable = 1;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_mac_set_internal_property(uint8 lchip, uint16 lport, ctc_port_property_t property, uint32 value)
{
    uint32  tbl_id = 0;
    uint32  field_id = 0;
    uint32  cmd    = 0;
    uint8   step   = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx  = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;
    switch (property)
    {
    case CTC_PORT_PROP_PADING_EN:
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxPadEn_f;
        break;

    case CTC_PORT_PROP_PREAMBLE:
        if((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) ||
           (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode) ||
           (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
           (CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode))
        {
            if(2 > value)
            {
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Preamble len cannot less than 2!\n");
                return CTC_E_INVALID_PARAM;
            }
        }
        else if((CTC_CHIP_SERDES_XLG_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) ||
                (CTC_CHIP_SERDES_CG_MODE == port_attr->pcs_mode))
        {
            if(8 != value)
            {
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Preamble len only support 8!\n");
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if((4 != value) && (8 != value))
            {
                SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_mac_set_internal_property:  value %d out of bound!\n", value);
                return CTC_E_INVALID_PARAM;
            }
        }
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        field_id = SharedMii0Cfg0_cfgMiiTxPreambleLen0_f;
        break;

    case CTC_PORT_PROP_CHK_CRC_EN:
        step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0RxCfg0_cfgSgmac0RxCrcChkEn_f;
        break;

    case CTC_PORT_PROP_STRIP_CRC_EN:
        /* SYSTEM MODIFIED for crc recalulate */
        /* step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxStripCrcEn_f; */
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxKeepTsEn_f;
        cmd = DRV_IOW(tbl_id, field_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        field_id = Sgmac0TxCfg0_cfgSgmac0TxStripCrcEn_f;
        break;

    case CTC_PORT_PROP_APPEND_CRC_EN:
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxAppendCrcEn_f;
        break;

    case CTC_PORT_PROP_APPEND_TOD_EN:
        step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0RxCfg0_cfgSgmac0RxTodAppendEn_f;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(tbl_id, field_id);

    if((CTC_PORT_PROP_PREAMBLE != property) && (1 < value))
    {
        return CTC_E_INVALID_PARAM;
    }
    else
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_internal_property(uint8 lchip, uint32 gport, ctc_port_property_t property, uint32 value)
{
    uint16  lport  = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set mac internal property, gport:0x%04X, property:%d, value:%d\n", \
                     gport, property, value);

    /*Sanity check*/
    SYS_MAC_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_internal_property(lchip, lport, property, value));
    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_internal_property(uint8 lchip, uint32 gport, ctc_port_property_t property, uint32* p_value)
{
    uint32  tbl_id = 0;
    uint32  field_id = 0;
    uint32  cmd    = 0;
    uint8   step   = 0;
    uint16  lport  = 0;
    uint8  sgmac_idx = 0;
    uint8  mii_idx = 0;
    uint8 internal_mac_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get mac internal property, gport:0x%04X, property:%d\n", \
                     gport, property);

    /*Sanity check*/
    SYS_MAC_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
    }
    MAC_UNLOCK;

    sgmac_idx        = port_attr->sgmac_idx;
    mii_idx          = port_attr->mii_idx;
    internal_mac_idx = port_attr->internal_mac_idx;

    switch (property)
    {
    case CTC_PORT_PROP_PADING_EN:
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxPadEn_f;
        break;

    case CTC_PORT_PROP_PREAMBLE:
        step = SharedMii1Cfg0_t - SharedMii0Cfg0_t;
        tbl_id = SharedMii0Cfg0_t + internal_mac_idx*step + mii_idx;
        field_id = SharedMii0Cfg0_cfgMiiTxPreambleLen0_f;
        break;

    case CTC_PORT_PROP_CHK_CRC_EN:
        step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0RxCfg0_cfgSgmac0RxCrcChkEn_f;
        break;

    case CTC_PORT_PROP_STRIP_CRC_EN:
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxStripCrcEn_f;
        break;

    case CTC_PORT_PROP_APPEND_CRC_EN:
        step = Sgmac1TxCfg0_t - Sgmac0TxCfg0_t;
        tbl_id = Sgmac0TxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0TxCfg0_cfgSgmac0TxAppendCrcEn_f;
        break;

    case CTC_PORT_PROP_APPEND_TOD_EN:
        step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        field_id = Sgmac0RxCfg0_cfgSgmac0RxTodAppendEn_f;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(tbl_id, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_value));

    return CTC_E_NONE;
}

int32
sys_usw_mac_set_error_check(uint8 lchip, uint32 gport, bool enable)
{
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_CHK_CRC_EN, (enable ? 1 : 0)));
    /* SYSTEM MODIFIED for crc check and recalulate */
    //CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, (enable ? 1 : 0)));
    //CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, (enable ? 1 : 0)));

    return CTC_E_NONE;
}

/* SYSTEM MODIFIED for crc recalulate */
int32
sys_usw_mac_set_crc_recalc_en(uint8 lchip, uint32 gport, bool enable)
{
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, (enable ? 1 : 0)));
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, (enable ? 1 : 0)));

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_error_check(uint8 lchip, uint32 gport, uint32* enable)
{
    uint32 en[3] = {0};
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_CHK_CRC_EN, &en[0]));
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, &en[1]));
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, &en[2]));

    if(en[0] && en[1] && en[2])
    {
        *enable = 1;
    }
    else
    {
        *enable = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_mac_show_port_maclink(uint8 lchip)
{
    uint8 i = 0;
    uint8 gchip_id = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint32 value_link_up = FALSE;
    uint32 value_mac_en = FALSE;
    uint32 value_32 = 0;
    char value_c[32] = {0};
    char* p_unit[2] = {NULL, NULL};
    int32 ret = 0;
    uint64 rate[CTC_STATS_MAC_STATS_MAX] = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;
    char  str[50] = {0};
    char* bw_unit[] = {"bps", "Kbps", "Mbps", "Gbps", "Tbps"};

    LCHIP_CHECK(lchip);
    SYS_MAC_INIT_CHECK();

    sal_task_sleep(100);

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip_id));
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Show Local Chip %d Port Link:\n", lchip);

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --------------------------------------------------------------------------------------------\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-8s%-6s%-7s%-9s%-7s%-8s%-10s%-13s%-10s%s\n",
                      "GPort", "MAC", "Link", "MAC-EN", "Speed", "Duplex", "Auto-Neg", "Interface", " Rx-Rate", " Tx-Rate");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --------------------------------------------------------------------------------------------\n");

    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip_id, lport);

        ret = sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
        if (ret < 0)
        {
            continue;
        }

        if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
        {
            continue;
        }

        ret = sys_usw_mac_get_mac_en(lchip, gport, &value_mac_en);
        if (ret < 0)
        {
            continue;
        }

        ret = sys_usw_mac_get_link_up(lchip, gport, &value_link_up, 0);
        if (ret < 0)
        {
            continue;
        }

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " 0x%04X  ", gport);
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u", (port_attr->mac_id + 64*port_attr->slice_id));
        if (value_link_up)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s", "up");
        }
        else
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s", "down");
        }

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9s", (value_mac_en ? "TRUE":"FALSE"));

        if(MCHIP_API(lchip)->mac_get_property)
        {
            ret = MCHIP_API(lchip)->mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, (uint32*)&value_32);
        }
        else
        {
            ret = CTC_E_INVALID_PTR;
        }

        if (ret < 0)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s", "-");
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
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s", value_c);
        }

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s", "FD");

        if(MCHIP_API(lchip)->mac_get_property)
        {
            ret = MCHIP_API(lchip)->mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &value_32);
        }
        else
        {
            ret = CTC_E_INVALID_PTR;
        }

        if (ret < 0)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
        }
        else
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", (value_32 ? "TRUE" : "FALSE"));
        }

        if (CTC_PORT_IF_SGMII == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "SGMII");
        }
        else if (CTC_PORT_IF_2500X == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "2500X");
        }
        else if (CTC_PORT_IF_QSGMII == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "QSGMII");
        }
        else if (CTC_PORT_IF_USXGMII_S == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "USXGMII-S");
        }
        else if (CTC_PORT_IF_USXGMII_M2G5 == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "USXGMII-M2G5");
        }
        else if (CTC_PORT_IF_USXGMII_M5G == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "USXGMII-M5G");
        }
        else if (CTC_PORT_IF_XAUI == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "XAUI");
        }
        else if (CTC_PORT_IF_DXAUI == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "DXAUI");
        }
        else if (CTC_PORT_IF_XFI == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "XFI");
        }
        else if (CTC_PORT_IF_KR == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "KR");
        }
        else if (CTC_PORT_IF_CR == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "CR");
        }
        else if (CTC_PORT_IF_KR2 == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "KR2");
        }
        else if (CTC_PORT_IF_CR2 == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "CR2");
        }
        else if (CTC_PORT_IF_KR4 == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "KR4");
        }
        else if (CTC_PORT_IF_CR4 == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "CR4");
        }
        else if (CTC_PORT_IF_FX == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "FX");
        }
        else if (CTC_PORT_IF_NONE == port_attr->interface_type)
        {
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13s", "-");
        }

        sys_usw_mac_stats_get_port_rate(lchip, gport, rate);

        for (i = 0; i < CTC_STATS_MAC_STATS_MAX; i++)
        {
            if (rate[i] < (1000ULL / 8U))
            {
                p_unit[i] = bw_unit[0];
                rate[i] = rate[i] * 8U;
            }
            else if (rate[i] < ((1000ULL * 1000ULL) / 8U))
            {
                p_unit[i] = bw_unit[1];
                rate[i] = rate[i] / (1000ULL / 8U);
            }
            else if (rate[i] < ((1000ULL * 1000ULL * 1000ULL) / 8U))
            {
                p_unit[i] = bw_unit[2];
                rate[i] = rate[i] / ((1000ULL * 1000ULL) / 8U);
            }
            else if (rate[i] < ((1000ULL * 1000ULL * 1000ULL * 1000ULL) / 8U))
            {
                p_unit[i] = bw_unit[3];
                rate[i] = rate[i] / ((1000ULL * 1000ULL * 1000ULL) / 8U);
            }
            else if (rate[i] < ((1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL) / 8U))
            {
                p_unit[i] = bw_unit[4];
                rate[i] = rate[i] / ((1000ULL * 1000ULL * 1000ULL * 1000ULL) / 8U);
            }
            else
            {
                p_unit[i] = bw_unit[0];
                rate[i] = 0;
            }
            sal_sprintf(str, "%3"PRIu64"%s%-6s", rate[i], " ", p_unit[i]);
            SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", str);
        }
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --------------------------------------------------------------------------------------------\n");
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " \n");

    return CTC_E_NONE;
}


#define  __MAC_THREAD__

int32
sys_usw_mac_set_monitor_link_enable(uint8 lchip, uint32 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    enable = enable?1:0;
    LCHIP_CHECK(lchip);
    SYS_MAC_INIT_CHECK();

    p_usw_mac_master[lchip]->polling_status = enable;

    field_val = enable;
    cmd = DRV_IOW(OobFcReserved_t, OobFcReserved_reserved_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_mac_get_monitor_link_enable(uint8 lchip, uint32* p_value)
{
    LCHIP_CHECK(lchip);
    SYS_MAC_INIT_CHECK();
    *p_value = p_usw_mac_master[lchip]->polling_status;

    return CTC_E_NONE;
}

#define  __MAC_ISR__

int32
sys_usw_mac_isr_cl73_complete_isr(uint8 lchip, void* p_data, void* p_data1)
{
    uint32 gport = 0;
    uint16 lport = 0;
    uint8  cl73_ok = 0;
    uint8  serdes_id = 0;
    uint8  gchip = 0;
    sys_datapath_an_ability_t remote_ability;
    sys_datapath_an_ability_t local_ability;
    uint32 ability, cstm_ability;
    uint8  need_training=1;
    uint32 link_stuck = 0;
    uint8  timeout = 0xff;
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
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
		return CTC_E_INVALID_CONFIG;
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
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    /* enable CG MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
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
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                    }
                }
            }
            else if(CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_50GBASE_KR2)
                    || CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_50GBASE_CR2))
            {
                if_mode.speed = CTC_PORT_SPEED_50G;
                if_mode.interface_type = CTC_PORT_IF_CR2;

                if (CTC_CHIP_SERDES_LG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                    /* enable LG MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
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
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL74_FEC_SUP)
                                 && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ)
                                      || CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            &&(CTC_FLAG_ISSET(ability, SYS_PORT_CL73_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
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
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                    /* enable XLG MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }
            }
            else if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR_S)
                    || CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR))
            {
                if_mode.speed = CTC_PORT_SPEED_25G;
                if_mode.interface_type = CTC_PORT_IF_CR;
                if (CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                    /* enable XXVG MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
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
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                        }
                        else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)
                                    ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)))
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                        }
                        else
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                        }
                    }
                    else if (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_25GBASE_CR_S))
                    {
                        if (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)
                            ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_RS_FEC_REQ)
                            ||CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ)
                            ||CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_25G_BASER_FEC_REQ))
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                        }
                        else
                        {
                            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
                        }
                    }
                }
            }
            else if(CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_25GBASE_KR1)
                    || CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_25GBASE_CR1))
            {
                if_mode.speed = CTC_PORT_SPEED_25G;
                if_mode.interface_type = CTC_PORT_IF_CR;
                if (CTC_CHIP_SERDES_XXVG_MODE != port_attr->pcs_mode)
                {
                    /* disable original MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                    /* enable XXVG MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
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
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_RS));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(cstm_ability, SYS_PORT_CSTM_CL74_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.np1_ability1, SYS_PORT_CSTM_CL74_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else if ((port_attr->an_fec & (1<<CTC_PORT_FEC_TYPE_BASER))
                            && (CTC_FLAG_ISSET(ability, SYS_PORT_CL73_FEC_SUP)
                                && (CTC_FLAG_ISSET(local_ability.base_ability1, SYS_PORT_CL73_FEC_REQ)
                                    || CTC_FLAG_ISSET(remote_ability.base_ability1, SYS_PORT_CL73_FEC_REQ))))
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_BASER));
                    }
                    else
                    {
                        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_fec_en(lchip, lport, CTC_PORT_FEC_TYPE_NONE));
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
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, FALSE));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, &if_mode));
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_nettxwriteen_en(lchip, lport, TRUE));
                    /* enable XFI MAC */
                    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_mac_en(lchip, gport, TRUE));
                }
                else
                {
                    port_attr->interface_type = if_mode.interface_type;
                }
            }
            else
            {
                need_training = 0;
            }

            SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                "Local base ability[0]:0x%x,[1]:0x%x, next page0[0]:0x%x,[1]:0x%x,next page1[0]:0x%x,[1]:0x%x\n",
                local_ability.base_ability0,local_ability.base_ability1, local_ability.np0_ability0,
                local_ability.np0_ability1, local_ability.np1_ability0, local_ability.np1_ability1);
            SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                "Remote base ability[0]:0x%x,[1]:0x%x, next page0[0]:0x%x,[1]:0x%x,next page1[0]:0x%x,[1]:0x%x\n",
                remote_ability.base_ability0,remote_ability.base_ability1, remote_ability.np0_ability0,
                remote_ability.np0_ability1, remote_ability.np1_ability0, remote_ability.np1_ability1);

            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_dfe_en(lchip, gport, 1));
            sal_task_sleep(1);
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_link_stuck(lchip, serdes_id, &link_stuck));
            while ((link_stuck) && (timeout--))
            {
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, TRUE));
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, FALSE));
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_link_stuck(lchip, serdes_id, &link_stuck));
                if (!link_stuck)
                {
                    break;
                }
            }
            if (!timeout)
            {
                SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes:%d link stuck occur!\n", serdes_id);
                MAC_UNLOCK;
                return CTC_E_HW_FAIL;
            }

            /* 3. 3ap enable and init */
            if(need_training)
            {
                SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"gport:%d, pcs_mode:%d start training\n", gport, port_attr->pcs_mode);
                /* Link training init */
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_set_3ap_training_en(lchip, gport, 1));
            }

            /* do pcs rx release */
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_pcs_rx_rst(lchip, lport, 0, FALSE));
            port_attr->is_first = 1;
        }
    }

    MAC_UNLOCK;

    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Exit %s\n", __FUNCTION__);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_reset_hw(uint8 lchip, void* p_void)
{
    uint32 gport = 0;
    uint16 lport = 0;
    uint8  gchip_id = 0;
    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    if(NULL == (MCHIP_API(lchip)->mac_set_property))
    {
        return CTC_E_INVALID_PTR;
    }
    sys_usw_get_gchip_id(lchip, &gchip_id);
    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        if (p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
        {
            gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip_id, lport);
            MCHIP_API(lchip)->mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_EN, TRUE);
        }
    }

    return CTC_E_NONE;
}

#define __MAC_WB__
int32
sys_usw_mac_wb_sync(uint8 lchip)
{
    uint16 lport = 0;
    uint16 max_entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_port_mac_prop_t * p_wb_mac_prop = NULL;

    /*syncup datapath matser*/
    wb_data.buffer = mem_malloc(MEM_PORT_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_port_mac_prop_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_MAC_PROP);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);

    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        p_wb_mac_prop = (sys_wb_port_mac_prop_t *)wb_data.buffer + wb_data.valid_cnt;
        p_wb_mac_prop->lport = lport;
        sal_memset(p_wb_mac_prop->rsv, 0, sizeof(uint8) * 2);
        p_wb_mac_prop->port_mac_en = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
        p_wb_mac_prop->speed_mode = p_usw_mac_master[lchip]->mac_prop[lport].speed_mode;
        p_wb_mac_prop->cl73_enable = p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable;
        p_wb_mac_prop->old_cl73_status = p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status;
        p_wb_mac_prop->link_intr_en= p_usw_mac_master[lchip]->mac_prop[lport].link_intr_en;
        p_wb_mac_prop->link_status = p_usw_mac_master[lchip]->mac_prop[lport].link_status;
        p_wb_mac_prop->unidir_en = p_usw_mac_master[lchip]->mac_prop[lport].unidir_en;
        p_wb_mac_prop->rsv1 = 0;

        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
            wb_data.valid_cnt = 0;
        }
    }
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_mac_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_port_mac_prop_t  wb_mac_prop = {0};

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_PORT_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_port_mac_prop_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_MAC_PROP);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_mac_prop, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].port_mac_en = wb_mac_prop.port_mac_en;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].speed_mode = wb_mac_prop.speed_mode;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].cl73_enable = wb_mac_prop.cl73_enable;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].old_cl73_status = wb_mac_prop.old_cl73_status;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].link_intr_en = wb_mac_prop.link_intr_en;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].link_status = wb_mac_prop.link_status;
        p_usw_mac_master[lchip]->mac_prop[wb_mac_prop.lport].unidir_en = wb_mac_prop.unidir_en;
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}


#define  __MAC_INIT__


/**
 @brief initialize the port module
*/
int32
sys_usw_mac_init(uint8 lchip, ctc_port_global_cfg_t* p_port_global_cfg)
{
    int32  ret = CTC_E_NONE;

    if (p_usw_mac_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }

    /*alloc&init DB and mutex*/
    p_usw_mac_master[lchip] = (sys_mac_master_t*)mem_malloc(MEM_PORT_MODULE, sizeof(sys_mac_master_t));

    if (NULL == p_usw_mac_master[lchip])
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_mac_master[lchip], 0, sizeof(sys_mac_master_t));
    ret = sal_mutex_create(&(p_usw_mac_master[lchip]->p_mac_mutex));

    if (ret || !(p_usw_mac_master[lchip]->p_mac_mutex))
    {
        mem_free(p_usw_mac_master[lchip]);

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        return CTC_E_NO_MEMORY;

    }

    p_usw_mac_master[lchip]->mac_prop = (sys_mac_prop_t*)mem_malloc(MEM_PORT_MODULE, SYS_USW_MAX_PORT_NUM_PER_CHIP * sizeof(sys_mac_prop_t));

    if (NULL == p_usw_mac_master[lchip]->mac_prop)
    {
        sal_mutex_destroy(p_usw_mac_master[lchip]->p_mac_mutex);
        mem_free(p_usw_mac_master[lchip]);

        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(p_usw_mac_master[lchip]->mac_prop, 0, sizeof(sys_mac_prop_t) * SYS_USW_MAX_PORT_NUM_PER_CHIP);

    if(MCHIP_API(lchip)->mac_init)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->mac_init(lchip));
    }
    else return CTC_E_INVALID_PTR;

    drv_ser_register_hw_reset_cb(lchip, DRV_SER_HW_RESET_CB_TYPE_MAC, _sys_usw_mac_reset_hw);


    return CTC_E_NONE;
}


int32 sys_usw_mac_deinit(uint8 lchip)
{
    sys_mac_master_t* master_ptr = NULL;
    LCHIP_CHECK(lchip);

    master_ptr = p_usw_mac_master[lchip];

    if(!master_ptr)
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(master_ptr->p_mac_mutex);
    sal_task_destroy(master_ptr->p_monitor_scan);

    if(master_ptr->mac_prop)
    {
        mem_free(master_ptr->mac_prop);
    }

    mem_free(master_ptr);
    p_usw_mac_master[lchip] = NULL;

    return CTC_E_NONE;
}




