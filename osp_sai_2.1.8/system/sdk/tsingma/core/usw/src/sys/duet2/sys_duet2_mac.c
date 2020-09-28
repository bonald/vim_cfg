/**
 @file sys_duet2_mac.c

 @date 2018-09-12

 @version v1.0
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
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
#include "sys_duet2_mac.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"



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
extern sys_mac_master_t* p_usw_mac_master[CTC_MAX_LOCAL_CHIP_NUM];


static mac_link_status_t map_hs_linkstatus[MAX_HS_MAC_NUM] =
{
    /*uint0*/
    {0, 1},{0, 0},   {1, 1},{1, 0},   {2, 1},{2, 0},   {3, 1},{3, 0},
    {4, 1},{4, 0},   {5, 1},{5, 0},   {6, 1},{6, 0},   {7, 1},{7, 0},

    /*uint1*/
    {8, 1},{8, 0},   {9, 1},{9, 0},   {10, 1},{10, 0}, {11, 1},{11, 0},
    {16, 1},{16,0},  {17, 1},{17,0},  {18, 1},{18, 0}, {19, 1},{19, 0},

    /*uint2*/
    {20, 1},{20, 0}, {21, 1},{21, 0}, {22, 1},{22, 0}, {23, 1},{23, 0},
    {24, 1},{25, 0}, {26, 1},{26, 0}, {27, 1},{27, 0}, {28, 1},{28, 0},

    /*uint3*/
    {32, 1},{32, 0}, {33, 1},{33, 0}, {34, 1},{34, 0}, {35, 1},{35, 0},
    {36, 1},{36, 0}, {37, 1},{37, 0}, {38, 1},{38, 0}, {39, 1},{39, 0},

    /*uint4*/
    {40, 1},{40, 0}, {41, 1},{41, 0}, {42, 1},{42, 0}, {43, 1},{43, 0},
    {48, 1},{48, 0}, {49, 1},{49, 0}, {50, 1},{50, 0}, {51, 1},{51, 0},

    /*uint5*/
    {52, 1},{52, 0}, {53, 1},{53, 0}, {54, 1},{54, 0}, {55, 1},{55, 0},
    {56, 1},{56, 0}, {57, 1},{57, 0}, {58, 1},{58, 0}, {59, 1},{59, 0},
};

static mac_link_status_t map_cs_linkstatus[MAX_CS_MAC_NUM] =
{
    /*uint0*/
    {12, 1},{12, 0}, {13, 1},{13, 0}, {14,1},{14,0}, {15,1},{15,0},

    /*uint1*/
    {28, 1},{28, 0}, {29, 1},{29, 0}, {30,1},{30,0}, {31,1},{31,0},

    /*uint2*/
    {44, 1},{44, 0}, {45, 1},{45, 0}, {46,1},{46,0}, {47,1},{47,0},

    /*uint3*/
    {60, 1},{60, 0}, {61, 1},{61, 0}, {62,1},{62,0}, {63,1},{63,0},
};



extern int32
_sys_usw_mac_set_mac_config(uint8 lchip, uint16 lport, uint8 is_init);

extern int32
_sys_usw_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable);

extern int32
_sys_usw_mac_get_no_random_data_flag(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
_sys_usw_mac_get_code_err(uint8 lchip, uint16 lport, uint32* code_err);

extern int32
_sys_usw_mac_recover_process(uint8 lchip, uint16 lport, uint32 type);

extern int32
_sys_usw_mac_get_table_by_mac_id(uint8 lchip, uint8 mac_id, uint32* table, uint32* bitmap);

extern int32
sys_usw_datapath_dynamic_switch_get_info(uint8 lchip, uint16 lport, uint8 serdes_id,
                 uint8 src_mode, uint8 mode, uint8 portbased_flag, sys_datapath_dynamic_switch_attr_t *attr);

extern int32
_sys_usw_mac_get_cl37_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value);

extern int32
_sys_usw_mac_get_unidir_en(uint8 lchip, uint16 lport, uint32* p_value);

extern int32
_sys_usw_mac_get_sgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_cl37_an_remote_status(uint8 lchip, uint32 gport, uint32 auto_neg_mode, uint32* p_speed, uint32* p_link);

extern int32
_sys_usw_mac_get_qsgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_usxgmii_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_xaui_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_xfi_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_xlg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_xxvg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_lg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_cg_link_status(uint8 lchip, uint8 type, uint16 lport, uint32* p_value, uint32 unidir_en);

extern int32
_sys_usw_mac_get_speed_mode_capability(uint8 lchip, uint32 gport, uint32* speed_mode_bitmap);

extern int32
_sys_usw_mac_get_if_type_capability(uint8 lchip, uint32 gport, uint32* if_type_bitmap);

extern int32
_sys_usw_mac_get_fec_type_capability(uint8 pcs_mode, uint32* p_value);

extern int32
_sys_usw_mac_get_cl73_capability(uint8 lchip, uint32 gport, uint32* p_capability);

extern int32
_sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability);

extern int32
sys_usw_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_set_eee_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_set_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32 value);

extern int32
sys_usw_mac_set_speed(uint8 lchip, uint32 gport, ctc_port_speed_t speed_mode);

extern int32
sys_usw_mac_set_error_check(uint8 lchip, uint32 gport, bool enable);

extern int32
_sys_usw_mac_set_rx_pause_type(uint8 lchip, uint32 gport, uint32 pasue_type);

extern int32
sys_usw_mac_set_link_intr(uint8 lchip, uint32 gport, uint8 enable);

extern int32
sys_usw_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_mac_get_fec_en(uint8 lchip, uint32 gport, uint32 *p_value);

extern int32
sys_usw_mac_get_cl73_ability(uint8 lchip, uint32 gport, uint32 type, uint32* p_ability);

extern int32
sys_usw_mac_get_link_intr(uint8 lchip, uint32 gport, uint32* enable);

extern int32
sys_usw_mac_get_speed(uint8 lchip, uint32 gport, ctc_port_speed_t* p_speed_mode);

extern int32
sys_usw_mac_get_error_check(uint8 lchip, uint32 gport, uint32* enable);

extern int32
_sys_usw_mac_get_rx_pause_type(uint8 lchip, uint32 gport, uint32* p_pasue_type);

extern int32
sys_usw_mac_get_unidir_en(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
sys_usw_mac_get_ipg(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
_sys_usw_mac_flow_ctl_init(uint8 lchip);

extern int32
sys_usw_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void* p_data);

extern int32
sys_usw_mac_wb_restore(uint8 lchip);

extern int32
_sys_usw_mac_get_fec_en(uint8 lchip, uint16 lport, uint32* p_value);

extern int32
_sys_usw_mac_get_3ap_training_en(uint8 lchip, uint16 lport, uint32* p_status);

extern int32
sys_usw_datapath_set_serdes_link_up_cfg(uint8 lchip,
                                        uint16 lport,
                                        uint8 mac_id,
                                        uint8 lg_flag,
                                        uint8 pcs_mode);

extern int32
sys_usw_mac_set_cl73_ability(uint8 lchip, uint32 gport, uint32 ability);
/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_duet2_mac_init_mac_config(uint8 lchip)
{
    uint8 slice_id = 0;
    uint8 mac_id = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
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

        CTC_ERROR_RETURN(_sys_usw_mac_set_mac_config(lchip, lport, TRUE));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_framesize_init(uint8 lchip)
{
    uint8  idx = 0;
    uint32 valueMin = SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE;

    for (idx = 0; idx < CTC_FRAME_SIZE_MAX; idx++)
    {
        if(CTC_FRAME_SIZE_7 == idx )
        {
            valueMin = SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE;
        }
        else
        {
            valueMin = SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE;
        }
        CTC_ERROR_RETURN(sys_usw_set_min_frame_size(lchip, idx, valueMin));
        CTC_ERROR_RETURN(sys_usw_set_max_frame_size(lchip, idx, SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_set_chan_framesize_index(uint8 lchip, uint8 mac_id, uint8 index, sys_port_framesize_type_t type)
{
    uint32 vlan_base = 0;
    uint32 cmd = 0;
    uint32 channelize_en = 0;
    uint32 drv_index = index;
    DsChannelizeMode_m ds_channelize_mode;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set index:%d, type:%d.\n", index, type);

    sal_memset(&ds_channelize_mode, 0, sizeof(DsChannelizeMode_m));
    cmd = DRV_IOR(DsChannelizeMode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mac_id & 0x3F), cmd, &ds_channelize_mode));
    GetDsChannelizeMode(A, channelizeEn_f, &ds_channelize_mode, &channelize_en);

    if (channelize_en)
    {
        if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
        {
            cmd = DRV_IOW(DsChannelizeIngFc_t, DsChannelizeIngFc_chanMinLenSelId_f);
        }
        else
        {
            cmd = DRV_IOW(DsChannelizeIngFc_t, DsChannelizeIngFc_chanMaxLenSelId_f);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mac_id & 0x3F), cmd, &drv_index));
    }
    else
    {
        GetDsChannelizeMode(A, vlanBase_f, &ds_channelize_mode, &vlan_base);
        if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
        {
            vlan_base &= ~(((uint16)0x7) << 5);
            vlan_base |= (index & 0x7) << 5;
        }
        else
        {
            vlan_base &= ~(((uint16)0x7) << 8);
            vlan_base |= (index & 0x7) << 8;
        }
        SetDsChannelizeMode(A, vlanBase_f, &ds_channelize_mode, &vlan_base);
        cmd = DRV_IOW(DsChannelizeMode_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mac_id & 0x3F), cmd, &ds_channelize_mode));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_get_chan_framesize_index(uint8 lchip, uint8 mac_id, uint8* p_index, sys_port_framesize_type_t type)
{
    uint32  index = 0;
    uint32 vlan_base = 0;
    uint32 cmd = 0;
    uint32 channelize_en = 0;
    DsChannelizeMode_m ds_channelize_mode;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ds_channelize_mode, 0, sizeof(DsChannelizeMode_m));
    cmd = DRV_IOR(DsChannelizeMode_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mac_id & 0x3F), cmd, &ds_channelize_mode));
    GetDsChannelizeMode(A, channelizeEn_f, &ds_channelize_mode, &channelize_en);

    if (channelize_en)
    {
        if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
        {
            cmd = DRV_IOR(DsChannelizeIngFc_t, DsChannelizeIngFc_chanMinLenSelId_f);
        }
        else
        {
            cmd = DRV_IOR(DsChannelizeIngFc_t, DsChannelizeIngFc_chanMaxLenSelId_f);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (mac_id & 0x3F), cmd, &index));
    }
    else
    {
        GetDsChannelizeMode(A, vlanBase_f, &ds_channelize_mode, &vlan_base);
        if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
        {
            index = (vlan_base >> 5) & 0x7;
        }
        else
        {
            index = (vlan_base >> 8) & 0x7;
        }
    }
    *p_index = index;

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_chan_init(uint8 lchip)
{
    uint8  index = 0;
    uint16 lport = 0;
    uint32 field = 0;
    uint32 cmd = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;
    NetRxCtl_m net_rx_ctl;
    NetTxMiscCtl_m net_tx_misc_ctl;
    int32 ret = 0;

    CTC_ERROR_RETURN(_sys_duet2_mac_framesize_init(lchip));

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
    {
        ret = (sys_usw_mac_get_port_capability(lchip, lport,&p_port_cap));
        if (ret == CTC_E_INVALID_CONFIG)
        {
            continue;
        }

        if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
        {
            continue;
        }

        CTC_ERROR_RETURN(_sys_duet2_mac_set_chan_framesize_index(lchip, p_port_cap->chan_id, index, SYS_PORT_FRAMESIZE_TYPE_MAX));
        CTC_ERROR_RETURN(_sys_duet2_mac_set_chan_framesize_index(lchip, p_port_cap->chan_id, index, SYS_PORT_FRAMESIZE_TYPE_MIN));

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
_sys_duet2_mac_get_auto_neg_status(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport     = 0;
    uint16 chip_port = 0;
    uint8  slice_id  = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8  serdes_num = 0;
    uint8  cnt = 0;
    uint16 status=0;

    /* get port info from sw table */
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
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_cl73_status(lchip, real_serdes, &status));
        if(status)
        {
            break;
        }
    }
    *p_value = status;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_get_mac_signal_detect(uint8 lchip,
                                uint16 lport,
                                uint8 mac_id,
                                uint8 lg_flag,
                                uint8 pcs_mode,
                                uint32* p_is_detect)
{
    uint16 chip_port = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint16 real_serdes = 0;
    uint8 serdes_num = 0;
    uint8 cnt = 0;
    uint32 tmp_val32[4] = {0};

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num));
    for (cnt = 0; cnt < serdes_num; cnt++)
    {
        GET_SERDES(mac_id, serdes_id, cnt, lg_flag, pcs_mode, real_serdes);
        tmp_val32[cnt] = sys_usw_datapath_get_serdes_sigdet(lchip, real_serdes);
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
sys_usw_mac_get_mac_signal_detect(uint8 lchip, uint32 gport, uint32* p_is_detect)
{
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

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

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_mac_signal_detect(lchip, lport,
        port_attr->mac_id, port_attr->flag, port_attr->pcs_mode, p_is_detect));

    MAC_UNLOCK;

    return CTC_E_NONE;
}

void
_sys_duet2_mac_monitor_thread(void* para)
{
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint32 mac_en = 0;
    uint32 is_up = 0;
    uint32 is_detect = 0;

    uint32 code_err = 0;
    uint32 tmp_code_err[3] = {0};
    uint32 no_random_flag =0;
    uint32 tmp_random_data_flag[3] = {0};
    uint32 pcs_sta = 0;
    uint32 pcs_not_up = 0;
    uint8  i = 0;
    sys_usw_scan_log_t* p_log = NULL;
    char* p_str = NULL;
    sal_time_t tv;
    uint32 type = 0;
    int32  ret = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 cl72_status = 0;
    uint32 port_skip_flag = 0;
    uint8 lchip = (uintptr)para;

    while(1)
    {
        sys_usw_get_gchip_id(lchip, &gchip);

        for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            is_detect = 0;
            mac_en = 0;
            is_up = 0;
            port_skip_flag = 0;
            cl72_status = 0;

            type = 0;
            code_err = 0;
            pcs_sta = 0;
            no_random_flag = 0;
            pcs_not_up = 0;

            if (sys_usw_chip_get_reset_hw_en(lchip))
            {
                continue;
            }
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
            SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
            if (p_usw_mac_master[lchip]->polling_status == 0)
            {
                continue;
            }

            MAC_LOCK;
            /*check mac used */
            ret = sys_usw_mac_get_port_capability(lchip, lport, &port_attr);
            if ((port_attr->port_type != SYS_DMPS_NETWORK_PORT) || ret)
            {
                MAC_UNLOCK;
                continue;
            }

            /*check mac enable */
            mac_en = p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en;
            if ((!mac_en) || ret)
            {
                MAC_UNLOCK;
                continue;
            }

            /* serdes link up cfg */
            sys_usw_mac_get_pcs_status(lchip, lport, &pcs_sta);
            if (pcs_sta)
            {
                sys_usw_datapath_set_serdes_link_up_cfg(lchip, lport, port_attr->mac_id,
                    port_attr->flag, port_attr->pcs_mode);
            }
            else
            {
                pcs_not_up = 1;
            }

            /* restart CL73 AN */
            if (p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)
            {
                port_skip_flag = 1;
                switch (port_attr->pcs_mode)
                {
                case CTC_CHIP_SERDES_XFI_MODE:
                    _sys_usw_mac_get_xfi_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                    break;
                case CTC_CHIP_SERDES_XLG_MODE:
                    _sys_usw_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                    break;
                case CTC_CHIP_SERDES_XXVG_MODE:
                    _sys_usw_mac_get_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                    break;
                case CTC_CHIP_SERDES_LG_MODE:
                    _sys_usw_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                    break;
                case CTC_CHIP_SERDES_CG_MODE:
                    _sys_usw_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, &is_up, FALSE);
                    break;
                }
                if (!is_up)
                {

                    /* old_cl73_status:
                     *   0 -- from auto to training
                     *   1 -- training 1 cycle still not up
                     *   2 -- link filter 1 cycle still not up
                     */
                    _sys_usw_mac_get_3ap_training_en(lchip, lport, &cl72_status);
                    if ((SYS_PORT_CL72_PROGRESS == cl72_status) &&
                        (0 == p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status))
                    {
                        p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status = 1;
                    }
                    else if (1 == p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status)
                    {
                        p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status = 2;
                    }
                    else
                    {
                        _sys_usw_mac_set_cl73_auto_neg_en(lchip, gport, 0);
                        _sys_usw_mac_set_cl73_auto_neg_en(lchip, gport, 1);
                        p_usw_mac_master[lchip]->mac_prop[lport].old_cl73_status = 0;
                    }
                }
            }

            /* CL73 AN enable, but port is not up, skip check code error/random data */
            if (port_skip_flag)
            {
                MAC_UNLOCK;
                continue;
            }

            /*check signal detect */
            ret = _sys_usw_mac_get_mac_signal_detect(lchip, lport, port_attr->mac_id,
                port_attr->flag, port_attr->pcs_mode, &is_detect);
            if ((!is_detect) || ret)
            {
                MAC_UNLOCK;
                continue;
            }

            /*1, check no-random-data flag*/
            if ((CTC_CHIP_SERDES_SGMII_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_2DOT5G_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_QSGMII_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_XAUI_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_DXAUI_MODE != port_attr->pcs_mode))
            {
                for (i = 0; i < PORT_RECOVER_CHECK_ROUND; i++)
                {
                    _sys_usw_mac_get_no_random_data_flag(lchip, gport, &tmp_random_data_flag[i]);
                    sal_task_sleep(1);
                }
                if (tmp_random_data_flag[0] && tmp_random_data_flag[1] && tmp_random_data_flag[2])
                {
                    no_random_flag = 1;
                }
            }

            /*2, check code error*/
            for (i = 0; i < PORT_RECOVER_CHECK_ROUND; i++)
            {
                _sys_usw_mac_get_code_err(lchip, lport, &tmp_code_err[i]);
                sal_task_sleep(1);
            }
            if ((tmp_code_err[0] > 0)
                && (tmp_code_err[1] > 0)
                && (tmp_code_err[2] > 0))
            {
                code_err = 1;
                port_attr->code_err_count++;
            }
            MAC_UNLOCK;

            if (1 == no_random_flag)
            {
                type |= SYS_PORT_NO_RANDOM_FLAG;
            }
            if (1 == code_err)
            {
                type |= SYS_PORT_CODE_ERROR;
                port_attr->code_err_count++;
            }
            if (1 == pcs_not_up)
            {
                type |= SYS_PORT_NOT_UP;
            }

            if (type)
            {
                /*do mac recover*/
                _sys_usw_mac_recover_process(lchip, lport, type);

                /*log event*/
                if (p_usw_mac_master[lchip]->cur_log_index >= SYS_PORT_MAX_LOG_NUM)
                {
                    p_usw_mac_master[lchip]->cur_log_index = 0;
                }

                p_log = &(p_usw_mac_master[lchip]->scan_log[p_usw_mac_master[lchip]->cur_log_index]);
                p_log->valid = 1;
                p_log->type = type;
                p_log->gport = gport;
                sal_time(&tv);
                p_str = sal_ctime(&tv);
                if (p_str)
                {
                    sal_strncpy(p_log->time_str, p_str, 50);
                }
                p_usw_mac_master[lchip]->cur_log_index++;
            }
            sal_task_sleep(10);
        }

        sal_task_sleep(1000);
    }
}

STATIC int32
_sys_duet2_mac_monitor_link(uint8 lchip)
{
    drv_work_platform_type_t platform_type;
    int32 ret = 0;
    uintptr chip_id = lchip;
    uint64 cpu_mask = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;

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
                          SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, SAL_TASK_TYPE_LINK_SCAN,cpu_mask, _sys_duet2_mac_monitor_thread, (void*)chip_id);
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

STATIC int32
sys_duet2_mac_isr_link_status_change_isr(uint8 lchip, uint32 intr, void* p_data)
{
    uint8 link_status = 0;
    uint8 index = 0;
    uint16 mac_id = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 gchip_id = 0;
    uint32* p_bmp = (uint32*)p_data;
    ctc_port_link_status_t port_link_status;
    CTC_INTERRUPT_EVENT_FUNC cb = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint32 max_filed = 0;
    mac_link_status_t* mapped_link_status = NULL;

    switch (intr)
    {
        case SYS_INTR_PCS_LINK_31_0:
            max_filed = MAX_HS_MAC_NUM;
            mapped_link_status = map_hs_linkstatus;
            break;

        case SYS_INTR_PCS_LINK_47_32:
            max_filed = MAX_CS_MAC_NUM;
            mapped_link_status = map_cs_linkstatus;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    for (index = 0; index < max_filed; index++)
    {
        if (CTC_BMP_ISSET(p_bmp, index))
        {
            mac_id = mapped_link_status[index].mac_id;
            link_status = mapped_link_status[index].link_status;

            /*get gport from datapath*/
            lport = sys_usw_dmps_get_lport_with_mac(lchip, mac_id);
            if (lport == SYS_COMMON_USELESS_MAC)
            {
                continue;
            }
            lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
            sys_usw_get_gchip_id(lchip, &gchip_id);
            gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip_id, lport);
            port_link_status.gport = gport;

            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, 0);
            if (NULL == port_attr)
            {
                return CTC_E_INVALID_PARAM;
            }

            if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
            {
                /*check > 10g*/
                if ((port_attr->speed_mode == CTC_PORT_SPEED_40G) || (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                    ||(port_attr->speed_mode == CTC_PORT_SPEED_10G) || (port_attr->speed_mode == CTC_PORT_SPEED_25G)
                    ||(port_attr->speed_mode == CTC_PORT_SPEED_50G))
                {
                    MAC_LOCK;
                    /*check autogen enable and link change to down, need restart auto*/
                    if ((!link_status) && p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable
                        && p_usw_mac_master[lchip]->mac_prop[lport].port_mac_en)
                    {
                        _sys_usw_mac_set_cl73_auto_neg_en(lchip, gport, 0);
                        _sys_usw_mac_set_cl73_auto_neg_en(lchip, gport, 1);
                    }
                    MAC_UNLOCK;
                }
            }

            CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_PORT_LINK_CHANGE, &cb));
            if (cb)
            {
                cb(gchip_id, &port_link_status);
            }

            /*Add task delay to eliminate jitter*/
            sal_task_sleep(10);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_link_intr_init(uint8 lchip)
{
    uint8 mac_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 tb_index = 0;
    ds_t   ds;

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
    {
        tb_index = 2;
        sal_memset(ds, 0, sizeof(ds));
        _sys_usw_mac_get_table_by_mac_id(lchip, mac_id, &tb_id, ds);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tb_index, cmd, ds));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_duet2_get_min_framesize_index(uint8 lchip, uint16 size, uint8* p_index)
{
    uint8  idx = 0;
    uint16 value = 0;

    *p_index = CTC_FRAME_SIZE_MAX;

    for (idx = 0; idx < CTC_FRAME_SIZE_MAX; idx++)
    {
        CTC_ERROR_RETURN(sys_usw_get_min_frame_size(lchip, idx, &value));

        if (size == value)
        {
            *p_index = idx;
            break;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_get_max_framesize_index(uint8 lchip, uint16 size, uint8* p_index)
{
    uint8  idx = 0;
    uint16 value = 0;

    *p_index = CTC_FRAME_SIZE_MAX;

    for (idx = 0; idx < CTC_FRAME_SIZE_MAX; idx++)
    {
        CTC_ERROR_RETURN(sys_usw_get_max_frame_size(lchip, idx, &value));

        if (size == value)
        {
            *p_index = idx;
            break;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_select_chan_framesize_index(uint8 lchip, uint16 value, sys_port_framesize_type_t type, uint8* p_index)
{
    if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
    {
        CTC_ERROR_RETURN(_sys_duet2_get_min_framesize_index(lchip, value, p_index));
        if (CTC_FRAME_SIZE_MAX == *p_index)
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        CTC_ERROR_RETURN(_sys_duet2_get_max_framesize_index(lchip, value, p_index));
        if (CTC_FRAME_SIZE_MAX == *p_index)
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_set_framesize(uint8 lchip, uint32 gport, uint16 value, sys_port_framesize_type_t type)
{
    uint8  index = 0;
    uint16 lport = 0;
    uint32 enable = 0;

    sys_datapath_lport_attr_t* p_port_cap = NULL;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port_cap));

    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", p_port_cap->mac_id);
        return CTC_E_INVALID_CONFIG;

    }

    CTC_ERROR_RETURN(sys_usw_mac_get_mac_en(lchip, gport, &enable));
    if (TRUE == enable)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " To change max/min framesize, the mac must be disable\n");
        return CTC_E_NOT_READY;
    }

    CTC_ERROR_RETURN(_sys_duet2_select_chan_framesize_index(lchip, value, type, &index));
    CTC_ERROR_RETURN(_sys_duet2_mac_set_chan_framesize_index(lchip, p_port_cap->mac_id, index, type));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_get_framesize(uint8 lchip, uint32 gport, uint16* p_value, sys_port_framesize_type_t type)
{

    uint8  index = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port_cap));

    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", p_port_cap->mac_id);
        return CTC_E_INVALID_CONFIG;

    }

    CTC_ERROR_RETURN(_sys_duet2_mac_get_chan_framesize_index(lchip, p_port_cap->mac_id, &index, type));

    if (SYS_PORT_FRAMESIZE_TYPE_MIN == type)
    {
        CTC_ERROR_RETURN(sys_usw_get_min_frame_size(lchip, index, p_value));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_get_max_frame_size(lchip, index, p_value));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_set_max_frame(uint8 lchip, uint32 gport, uint32 value)
{
    CTC_ERROR_RETURN(_sys_duet2_mac_set_framesize(lchip, gport, value, SYS_PORT_FRAMESIZE_TYPE_MAX));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_get_max_frame(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 value = 0;

    CTC_ERROR_RETURN(_sys_duet2_mac_get_framesize(lchip, gport, &value, SYS_PORT_FRAMESIZE_TYPE_MAX));
    *p_value = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_set_min_frame_size(uint8 lchip, uint32 gport, uint8 size)
{
    CTC_ERROR_RETURN(_sys_duet2_mac_set_framesize(lchip, gport, size, SYS_PORT_FRAMESIZE_TYPE_MIN));

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_mac_get_min_frame_size(uint8 lchip, uint32 gport, uint8* p_size)
{
    uint16 size = 0;

    CTC_ERROR_RETURN(_sys_duet2_mac_get_framesize(lchip, gport, &size, SYS_PORT_FRAMESIZE_TYPE_MIN));
    *p_size = size;

    return CTC_E_NONE;
}

int32
sys_duet2_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    uint32 i = 0;
    uint16 lport  = 0;
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 slice_id = 0;
    uint16 serdes_id = 0;
    uint16 chip_port = 0;
    uint8 num = 0;
    uint8 gchip = 0;
    uint8 speed = 0;
    uint8 interface_type = 0;
    uint8 mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr;
    uint32 port_step = 0;
    uint32 tmp_val32 = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MAC_INIT_CHECK();

    switch (if_mode->interface_type)
    {
    case CTC_PORT_IF_SGMII:
        mode = CTC_CHIP_SERDES_SGMII_MODE;
        break;
    case CTC_PORT_IF_2500X:
        mode = CTC_CHIP_SERDES_2DOT5G_MODE;
        break;
    case CTC_PORT_IF_QSGMII:
        mode = CTC_CHIP_SERDES_QSGMII_MODE;
        break;
    case CTC_PORT_IF_USXGMII_S:
        mode = CTC_CHIP_SERDES_USXGMII0_MODE;
        break;
    case CTC_PORT_IF_USXGMII_M2G5:
        mode = CTC_CHIP_SERDES_USXGMII1_MODE;
        break;
    case CTC_PORT_IF_USXGMII_M5G:
        mode = CTC_CHIP_SERDES_USXGMII2_MODE;
        break;
    case CTC_PORT_IF_XAUI:
        mode = CTC_CHIP_SERDES_XAUI_MODE;
        break;
    case CTC_PORT_IF_DXAUI:
        mode = CTC_CHIP_SERDES_DXAUI_MODE;
        break;
    case CTC_PORT_IF_XFI:
        mode = CTC_CHIP_SERDES_XFI_MODE;
        break;
    case CTC_PORT_IF_KR:
    case CTC_PORT_IF_CR:
        if (CTC_PORT_SPEED_25G == if_mode->speed)
        {
            mode = CTC_CHIP_SERDES_XXVG_MODE;
        }
        else if (CTC_PORT_SPEED_10G == if_mode->speed)
        {
            mode = CTC_CHIP_SERDES_XFI_MODE;
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
            mode = CTC_CHIP_SERDES_LG_MODE;
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
            mode = CTC_CHIP_SERDES_XLG_MODE;
        }
        else if (CTC_PORT_SPEED_100G == if_mode->speed)
        {
            mode = CTC_CHIP_SERDES_CG_MODE;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;
    case CTC_PORT_IF_NONE:
    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    sal_memset(&ds_attr, 0, sizeof(sys_datapath_dynamic_switch_attr_t));
    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    MAC_LOCK;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        MAC_UNLOCK;
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC %d is not used \n", port_attr->mac_id);
        return CTC_E_INVALID_CONFIG;
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
        || ((CTC_PORT_SPEED_100G == speed) && (CTC_PORT_IF_KR4 != interface_type) && (CTC_PORT_IF_CR4 != interface_type)))
    {
        MAC_UNLOCK;
        return CTC_E_PARAM_CONFLICT;
    }

    /* check if no change */
    if (port_attr->pcs_mode == mode)
    {
        MAC_UNLOCK;
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_get_gchip_id(lchip, &gchip));

    /* collect all serdes/mac info associated with Dynamic switch process */
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_dynamic_switch_get_info(lchip, lport, serdes_id, port_attr->pcs_mode, mode, 1, &ds_attr));

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
            /* check CL73 AN enable cfg */
            if ((CTC_CHIP_SERDES_XFI_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XLG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_CG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XXVG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_LG_MODE == port_attr_tmp->pcs_mode))
            {
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_set_cl73_auto_neg_en(lchip, tmp_gport, FALSE));
                port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + tmp_lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mcu_chip_read(lchip, (port_step+0x8), &tmp_val32));
                tmp_val32 &= ~(1 << 8);
                CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mcu_chip_write(lchip, (port_step+0x8), tmp_val32));
            }
        }
    }

    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_datapath_set_dynamic_switch(lchip, gport, if_mode));

    MAC_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_duet2_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port)
{
    uint16 lport = 0;
    uint32 unidir_en = 0;
    uint32 remote_link = 1;
    uint32 auto_neg_en = 0;
    uint32 auto_neg_mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetTxWriteEn_m nettx_cfg;
    uint32 nettx_write_en[2] = {0};
    uint8  write_en = 0;
    uint32 cmd = 0;

    SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&nettx_cfg, 0, sizeof(NetTxWriteEn_m));

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
    CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_unidir_en(lchip, lport, &unidir_en));

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
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_sgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
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
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_qsgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
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
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_usxgmii_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
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
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_xaui_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_XFI_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_xfi_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_XLG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_xlg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_xxvg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_LG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_lg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
            break;

        case CTC_CHIP_SERDES_CG_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cg_link_status(lchip, SYS_PORT_MAC_STATUS_TYPE_LINK, lport, p_is_up, unidir_en));
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
        cmd = DRV_IOR(NetTxWriteEn_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &nettx_cfg);
        GetNetTxWriteEn(A, writeEnPort_f, &nettx_cfg, nettx_write_en);
        if (port_attr->mac_id < 32)
        {
            write_en = (nettx_write_en[0] >> (port_attr->mac_id)) & 0x1;
        }
        else
        {
            write_en = (nettx_write_en[1] >> (port_attr->mac_id-32)) & 0x1;
        }
        if (!write_en)
        {
            *p_is_up = 0;
        }
    }

    p_usw_mac_master[lchip]->mac_prop[lport].link_status = (*p_is_up)?1:0;

    MAC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_duet2_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value)
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
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_usw_mac_get_serdes_info_capability(port_attr->mac_id, serdes_id, serdes_num,
                                                     port_attr->pcs_mode, port_attr->flag, (ctc_port_serdes_info_t*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_MAC_ID:
            *(uint32*)p_value = port_attr->mac_id;
            break;
        case CTC_PORT_CAP_TYPE_SPEED_MODE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_speed_mode_capability(lchip, gport, (uint32*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_IF_TYPE:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_if_type_capability(lchip, gport, (uint32*)p_value));
            break;
        case CTC_PORT_CAP_TYPE_FEC_TYPE:
            _sys_usw_mac_get_fec_type_capability(port_attr->pcs_mode, (uint32*)p_value);
            break;
        case CTC_PORT_CAP_TYPE_CL73_ABILITY:
            _sys_usw_mac_get_cl73_capability(lchip, gport, (uint32*)p_value);
            break;
        case CTC_PORT_CAP_TYPE_CL73_REMOTE_ABILITY:
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(_sys_usw_mac_get_cl73_ability(lchip, gport, 1, (uint32*)p_value));
            break;
        default:
            MAC_UNLOCK;
            return CTC_E_INVALID_PARAM;
    }

    MAC_UNLOCK;

    return CTC_E_NONE;
}

/**
@brief   Config port's properties
*/
int32
sys_duet2_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    int32   ret = CTC_E_NONE;

    uint16  lport = 0;
    uint8   value8 = 0;

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
        ret = sys_usw_mac_set_mac_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_EEE_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_usw_mac_set_eee_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_FEC_EN:
        ret = sys_usw_mac_set_fec_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_CL73_ABILITY:
        ret = sys_usw_mac_set_cl73_ability(lchip, gport, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_usw_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_MODE:
        ret = sys_usw_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_FEC:
        ret = sys_usw_mac_set_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, value);
        break;

    case CTC_PORT_PROP_SPEED:
        ret = sys_usw_mac_set_speed(lchip, gport, value);
        break;

    case CTC_PORT_PROP_MAX_FRAME_SIZE:
        ret = _sys_duet2_mac_set_max_frame(lchip, gport, value);
        break;

    case CTC_PORT_PROP_MIN_FRAME_SIZE:
        value8 = value;
        ret = _sys_duet2_mac_set_min_frame_size(lchip, gport, value8);
        break;

    case CTC_PORT_PROP_ERROR_CHECK:
        ret = sys_usw_mac_set_error_check(lchip, gport, value);
        break;

    case CTC_PORT_PROP_RX_PAUSE_TYPE:
        ret = _sys_usw_mac_set_rx_pause_type(lchip, gport, value);
        break;

    case CTC_PORT_PROP_LINKSCAN_EN:
        break;

    case CTC_PORT_PROP_LINK_INTRRUPT_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_usw_mac_set_link_intr(lchip, gport, value);
        p_usw_mac_master[lchip]->mac_prop[lport].link_intr_en = (value)?1:0;

        break;

    case CTC_PORT_PROP_UNIDIR_EN:
        value = (value) ? TRUE : FALSE;
        ret = sys_usw_mac_set_unidir_en(lchip, gport, value);
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
sys_duet2_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value)
{
    uint8  value8 = 0;

    uint8  inverse_value = 0;
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint16 lport = 0;
    ctc_port_speed_t port_speed = CTC_PORT_SPEED_1G;
    uint32 index = 0;
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
        ret = sys_duet2_mac_get_link_up(lchip, gport, p_value, 1);
        break;

    case CTC_PORT_PROP_FEC_EN:
        ret = sys_usw_mac_get_fec_en(lchip, gport, p_value);
        break;
    case CTC_PORT_PROP_CL73_ABILITY:
        ret = sys_usw_mac_get_cl73_ability(lchip, gport, 0, p_value);
        break;

    case CTC_PORT_PROP_SIGNAL_DETECT:
        ret = sys_usw_mac_get_mac_signal_detect(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINK_INTRRUPT_EN:
        ret = sys_usw_mac_get_link_intr(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_EN:
        ret = sys_usw_mac_get_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, p_value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_MODE:
        ret = sys_usw_mac_get_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, p_value);
        break;

    case CTC_PORT_PROP_AUTO_NEG_FEC:
        ret = sys_usw_mac_get_auto_neg(lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, p_value);
        break;

    case CTC_PORT_PROP_SPEED:
        ret = sys_usw_mac_get_speed(lchip, gport, &port_speed);
        if (CTC_E_NONE == ret)
        {
            *p_value = port_speed;
        }
        break;

    case CTC_PORT_PROP_MAX_FRAME_SIZE:
        ret = _sys_duet2_mac_get_max_frame(lchip, gport, (ctc_frame_size_t*)p_value);
        break;

    case CTC_PORT_PROP_MIN_FRAME_SIZE:
        ret = _sys_duet2_mac_get_min_frame_size(lchip, gport, &value8);
        *p_value = value8;
        break;

    case CTC_PORT_PROP_ERROR_CHECK:
        ret = sys_usw_mac_get_error_check(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_RX_PAUSE_TYPE:
        ret = _sys_usw_mac_get_rx_pause_type(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINKSCAN_EN:
        *p_value = 1;
        break;

    case CTC_PORT_PROP_UNIDIR_EN:
        ret = sys_usw_mac_get_unidir_en(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_MAC_TX_IPG:
        ret = sys_usw_mac_get_ipg(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_PREAMBLE:
    case CTC_PORT_PROP_PADING_EN:
    case CTC_PORT_PROP_CHK_CRC_EN:
    case CTC_PORT_PROP_STRIP_CRC_EN:
    case CTC_PORT_PROP_APPEND_CRC_EN:
    case CTC_PORT_PROP_APPEND_TOD_EN:
        ret = sys_usw_mac_get_internal_property(lchip, gport, port_prop, p_value);
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

int32
sys_duet2_mac_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value = 0;

    /* mac config init */
    CTC_ERROR_RETURN(_sys_duet2_mac_init_mac_config(lchip));

    /* Link Filter: 100ms */
    value = 0x2625A00;
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgRefDivCsLinkFilterPulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgRefDivHsLinkFilterPulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 0x3D0900;
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgRefDivHsLinkPulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(RefDivPcsLinkPulse_t, RefDivPcsLinkPulse_cfgRefDivCsLinkPulse_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* during porting phase suppose all port is gport */
    /* !!!!!!!!!!!it should config mac according to datapath later!!!!!!!!!*/
    CTC_ERROR_RETURN(_sys_duet2_mac_chan_init(lchip));

    CTC_ERROR_RETURN(_sys_duet2_mac_monitor_link(lchip));

    CTC_ERROR_RETURN(_sys_usw_mac_flow_ctl_init(lchip));

    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_PCS_LINK_31_0, sys_duet2_mac_isr_link_status_change_isr));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_PCS_LINK_47_32, sys_duet2_mac_isr_link_status_change_isr));
    CTC_ERROR_RETURN(_sys_duet2_mac_link_intr_init(lchip));

    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_CHANGE_0, sys_usw_peri_phy_link_change_isr));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_CHANGE_1, sys_usw_peri_phy_link_change_isr));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_XG_CHANGE_0, sys_usw_peri_phy_link_change_isr));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MDIO_XG_CHANGE_1, sys_usw_peri_phy_link_change_isr));
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_mac_wb_restore(lchip));
    }

    CTC_ERROR_RETURN(sys_usw_mac_mcu_init(lchip));

    return CTC_E_NONE;
}



