/**
 @file sys_usw_datapath.c

 @date 2014-10-28

 @version v3.0


*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_port.h"
#include "sys_usw_chip.h"
#include "sys_usw_datapath.h"
#include "sys_usw_mac.h"
#include "sys_usw_peri.h"
#include "sys_usw_dmps.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_mcu.h"
#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"

uint8 g_usw_mac_port_map[64] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63
};

sys_datapath_master_t* p_usw_datapath_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define NETTX_MEM_MAX_DEPTH_PER_TXQM 256
#define BUFRETRV_MEM_MAX_DEPTH 320
#define EPE_BODY_MEM_MAX_DEPTH 304
#define EPE_SOP_MEM_MAX_DEPTH 608

#define SYS_DATAPATH_MAP_HSSID_TO_HSSIDX(id, type)  \
    (SYS_DATAPATH_HSS_TYPE_15G == type) ? (id) : (id + HSS15G_NUM_PER_SLICE)

#define SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% SERDES_NUM_PER_SLICE) - 24)/HSS28G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*HSS28G_NUM_PER_SLICE) \
    :(((serdes_id)% SERDES_NUM_PER_SLICE)/HSS15G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*HSS15G_NUM_PER_SLICE)

#define SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% SERDES_NUM_PER_SLICE) - 24)/HSS28G_LANE_NUM + (((serdes_id)/SERDES_NUM_PER_SLICE)?3:3)) \
    :(((serdes_id)% SERDES_NUM_PER_SLICE)/HSS15G_LANE_NUM + ((serdes_id)/SERDES_NUM_PER_SLICE)*7)

#define SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?(((serdes_id% SERDES_NUM_PER_SLICE) - 24)%HSS28G_LANE_NUM) \
    :((serdes_id% SERDES_NUM_PER_SLICE)%HSS15G_LANE_NUM)

#define SYS_DATAPATH_IS_HSS28G(id) \
    (((id >= 12) && (id <= 15)) || ((id >= 28) && (id <= 31)) || \
    ((id >= 44) && (id <= 47)) || ((id >=60) && (id <= 63)))

#define SYS_DATAPATH_HSS_STEP(serdes_id, step)                     \
    if(DRV_IS_DUET2(lchip))                                        \
    {                                                              \
        if(SYS_DATAPATH_SERDES_IS_HSS28G((serdes_id)))             \
        {                                                          \
            step = ((serdes_id)/4 - 6) + (3 - (serdes_id)%4)*4;    \
        }                                                          \
        else                                                       \
        {                                                          \
            step = (serdes_id)/8 + (7 - (serdes_id)%8)*3 + 16;     \
        }                                                          \
    }                                                              \
    else                                                           \
    {                                                              \
        step = serdes_id;                                          \
    }


/*Hss15G internal lane: A, E, F, B, C, G, H, D*/
uint8 g_usw_hss15g_lane_map[HSS15G_LANE_NUM] = {0, 4, 5, 1, 2, 6, 7, 3};

/*for hss28g , E,F,G,H is not used */
uint8 g_usw_hss_tx_addr_map[HSS15G_LANE_NUM] =
{
    DRV_HSS_TX_LINKA_ADDR, DRV_HSS_TX_LINKB_ADDR, DRV_HSS_TX_LINKC_ADDR, DRV_HSS_TX_LINKD_ADDR,
    DRV_HSS_TX_LINKE_ADDR, DRV_HSS_TX_LINKF_ADDR, DRV_HSS_TX_LINKG_ADDR, DRV_HSS_TX_LINKH_ADDR
};

/*for hss28g , E,F,G,H is not used */
uint8 g_usw_hss_rx_addr_map[HSS15G_LANE_NUM] =
{
    DRV_HSS_RX_LINKA_ADDR, DRV_HSS_RX_LINKB_ADDR, DRV_HSS_RX_LINKC_ADDR, DRV_HSS_RX_LINKD_ADDR,
    DRV_HSS_RX_LINKE_ADDR, DRV_HSS_RX_LINKF_ADDR, DRV_HSS_RX_LINKG_ADDR, DRV_HSS_RX_LINKH_ADDR
};

uint32 g_nettx_cn[SYS_PHY_PORT_NUM_PER_SLICE] =
{
    24, 12, 12, 12,  8,  8,  4,  4, 24, 12, 12, 12, 60, 12, 24, 12,
     8,  8,  4,  4, 24, 12, 12, 12,  8,  8,  4,  4, 60, 12, 24, 12,
    24, 12, 12, 12,  8,  8,  4,  4, 24, 12, 12, 12, 60, 12, 24, 12,
     8,  8,  4,  4, 24, 12, 12, 12,  8,  8,  4,  4, 60, 12, 24, 12,
};

/* MsgEntryNum */
uint8 g_bufretrv_alloc[84] =
{
     4,  3,  3,  3,  3,  3,  3,  3,  4,  3,  3,  3,
    10,  3,  3,  3,  3,  3,  3,  3,  4,  3,  3,  3,
     3,  3,  3,  3, 10,  3,  3,  3,  4,  3,  3,  3,
     3,  3,  3,  3,  4,  3,  3,  3, 10,  3,  3,  3,
     3,  3,  3,  3,  4,  3,  3,  3,  3,  3,  3,  3,
    10,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
     3, 10, 10, 10,  0, 10,  3,  3,  3,  3,  3,  0,
};

sys_datapath_bufsz_step_t g_bufsz_step[] =
{
/*   sop body  br_credit_cfg */
    {80, 40,  0},   /* 100G */
    {40, 20,  1},   /* 50G, use 40G credit */
    {32, 16,  1},   /* 40G */
    {20, 10,  2},   /* 25G */
    { 8,  4,  3},   /* 10G */
    { 6,  2,  4},   /* 5G */
    { 3,  1,  5},   /* 10M/100M/1G/2.5G */
    {60, 30,  6},   /* Misc channel 100G: MacSec/Eloop*/
    { 6,  3,  7},   /* Misc channel 10G: Wlan*/
    { 0, 24,  -1},   /* Invalid port */
};

sys_datapath_serdes_group_mac_info_t g_serdes_group_mac_attr[] =
{
    /* serdes / mac mapping in one serdes group(4 continuous serdes from lane 0/4 in each HS/CS macro) */
       /*                                                           MAC   LIST                          MAC   LIST
           serdes mode           serdes_num   mac_num      serdes 0          serdes 1           serdes 2          serdes 3
       */
    {  CTC_CHIP_SERDES_NONE_MODE,      1,        0,  {{-1, -1, -1, -1}, {-1, -1, -1, -1}, {-1, -1, -1, -1}, {-1, -1, -1, -1}} },

    {  CTC_CHIP_SERDES_XFI_MODE,       4,        4,  {{ 0, -1, -1, -1}, { 1, -1, -1, -1}, { 2, -1, -1, -1}, { 3, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_SGMII_MODE,     4,        4,  {{ 0, -1, -1, -1}, { 1, -1, -1, -1}, { 2, -1, -1, -1}, { 3, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_2DOT5G_MODE,    4,        4,  {{ 0, -1, -1, -1}, { 1, -1, -1, -1}, { 2, -1, -1, -1}, { 3, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_XXVG_MODE,      4,        4,  {{ 0, -1, -1, -1}, { 1, -1, -1, -1}, { 2, -1, -1, -1}, { 3, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_USXGMII0_MODE,  4,        4,  {{ 0, -1, -1, -1}, { 1, -1, -1, -1}, { 2, -1, -1, -1}, { 3, -1, -1, -1}} },

    {  CTC_CHIP_SERDES_XAUI_MODE,      4,        1,  {{ 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_DXAUI_MODE,     4,        1,  {{ 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_XLG_MODE,       4,        1,  {{ 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_CG_MODE,        4,        1,  {{ 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}} },

    {  CTC_CHIP_SERDES_QSGMII_MODE,    1,        4,  {{ 0,  1,  2,  3}, { 4,  5,  6,  7}, {-1, -1, -1, -1}, {-1, -1, -1, -1}} },
    {  CTC_CHIP_SERDES_USXGMII1_MODE,  1,        4,  {{ 0,  1,  2,  3}, { 4,  5,  6,  7}, {-1, -1, -1, -1}, {-1, -1, -1, -1}} },

    {  CTC_CHIP_SERDES_USXGMII2_MODE,  1,        2,  {{ 0,  1, -1, -1}, { 4,  5, -1, -1}, {-1, -1, -1, -1}, {-1, -1, -1, -1}} },

    {  CTC_CHIP_SERDES_LG_MODE,        2,        1,  {{ 0, -1, -1, -1}, { 0, -1, -1, -1}, { 2, -1, -1, -1}, { 2, -1, -1, -1}} },
    /* {  CTC_CHIP_SERDES_LG_MODE,        2,        1,  {{ 2, -1, -1, -1}, { 0, -1, -1, -1}, { 0, -1, -1, -1}, { 2, -1, -1, -1}} }, */
};
#define SYS_DATAPATH_GET_BASE_MAC_BY_SERDES(serdes, mode, mac) \
{                                                              \
    if ( (CTC_CHIP_SERDES_QSGMII_MODE == mode)                 \
        || (CTC_CHIP_SERDES_USXGMII1_MODE == mode)             \
        || (CTC_CHIP_SERDES_USXGMII2_MODE == mode))            \
    {                                                          \
        switch (serdes)                                        \
        {                                                      \
        case 0:                                                \
        case 1:                                                \
        case 2:                                                \
        case 3:                                                \
            mac = 0;                                           \
            break;                                             \
        case 4:                                                \
        case 5:                                                \
        case 6:                                                \
        case 7:                                                \
            mac = 8;                                           \
            break;                                             \
        case 8:                                                \
        case 9:                                                \
        case 10:                                                \
        case 11:                                                \
            mac = 20;                                          \
            break;                                             \
        case 12:                                               \
        case 13:                                               \
        case 14:                                                \
        case 15:                                                \
            mac = 32;                                          \
            break;                                             \
        case 16:                                               \
        case 17:                                               \
        case 18:                                                \
        case 19:                                                \
            mac = 40;                                          \
            break;                                             \
        case 20:                                               \
        case 21:                                               \
        case 22:                                                \
        case 23:                                                \
            mac = 52;                                          \
            break;                                             \
        }                                                      \
    }                                                          \
    else                                                       \
    {                                                          \
        switch (serdes / 4)                                    \
        {                                                      \
        case 0:                                                \
            mac = 0;                                           \
            break;                                             \
        case 1:                                                \
            mac = 8;                                           \
            break;                                             \
        case 2:                                                \
            mac = 20;                                          \
            break;                                             \
        case 3:                                                \
            mac = 32;                                          \
            break;                                             \
        case 4:                                                \
            mac = 40;                                          \
            break;                                             \
        case 5:                                                \
            mac = 52;                                          \
            break;                                             \
        case 6:                                                \
            mac = 12;                                          \
            break;                                             \
        case 7:                                                \
            mac = 28;                                          \
            break;                                             \
        case 8:                                                \
            mac = 44;                                          \
            break;                                             \
        case 9:                                                \
            mac = 60;                                          \
            break;                                             \
        }                                                      \
    }                                                          \
}


#define SYS_DATAPATH_GET_HW_MAC_BY_SERDES(serdes, mode, mac, mac_num) \
{                                                              \
    if ( (CTC_CHIP_SERDES_QSGMII_MODE == mode)                 \
        || (CTC_CHIP_SERDES_USXGMII1_MODE == mode)             \
        || (CTC_CHIP_SERDES_USXGMII2_MODE == mode))            \
    {                                                          \
        if (CTC_CHIP_SERDES_USXGMII2_MODE == mode)             \
            mac_num = 2;                                       \
        else                                                   \
            mac_num = 4;                                       \
        switch (serdes)                                        \
        {                                                      \
        case 0:                                                \
            mac = 0;                                           \
            break;                                             \
        case 1:                                                \
            mac = 4;                                           \
            break;                                             \
        case 2:                                                \
            mac = 2;                                           \
            break;                                             \
        case 3:                                                \
            mac = 3;                                           \
            break;                                             \
        case 4:                                                \
            mac = 8;                                           \
            break;                                             \
        case 5:                                                \
            mac = 16;                                          \
            break;                                             \
        case 6:                                                \
            mac = 10;                                           \
            break;                                             \
        case 7:                                                \
            mac = 11;                                           \
            break;                                             \
        case 8:                                                \
            mac = 20;                                          \
            break;                                             \
        case 9:                                                \
            mac = 24;                                          \
            break;                                             \
        case 10:                                                \
            mac = 22;                                           \
            break;                                             \
        case 11:                                                \
            mac = 23;                                           \
            break;                                             \
        case 12:                                               \
            mac = 32;                                          \
            break;                                             \
        case 13:                                               \
            mac = 36;                                          \
            break;                                             \
        case 14:                                                \
            mac = 34;                                           \
            break;                                             \
        case 15:                                                \
            mac = 35;                                           \
            break;                                             \
        case 16:                                               \
            mac = 40;                                          \
            break;                                             \
        case 17:                                               \
            mac = 48;                                          \
            break;                                             \
        case 18:                                                \
            mac = 42;                                           \
            break;                                             \
        case 19:                                                \
            mac = 43;                                           \
            break;                                             \
        case 20:                                               \
            mac = 52;                                          \
            break;                                             \
        case 21:                                               \
            mac = 56;                                          \
            break;                                             \
        case 22:                                                \
            mac = 54;                                           \
            break;                                             \
        case 23:                                                \
            mac = 55;                                           \
            break;                                             \
        }                                                      \
    }                                                          \
    else                                                       \
    {                                                          \
        mac_num = 1;                                           \
        mac = serdes;                                          \
        mac += serdes/4*4;                                     \
        if ((serdes >=8) && (serdes <= 11))                    \
            mac += 4;                                          \
        else if ((serdes >=12) && (serdes <= 19))\
            mac += 8;                    \
        else if ((serdes >=20) && (serdes <= 23))\
            mac += 12;                    \
        else if ((serdes >=24) && (serdes <= 27))\
            mac = 12 + (serdes-24);                    \
        else if ((serdes >=28) && (serdes <= 31))\
            mac = 28 + (serdes-28);                    \
        else if ((serdes >=32) && (serdes <= 35))\
            mac = 44 + (serdes-32);                    \
        else if ((serdes >=36) && (serdes <= 39))\
            mac = 60 + (serdes-36);                    \
    }                                                   \
}


#define SYS_DATAPATH_CHAN_IS_STUB(chan) \
    ((0 == chan) || (8 == chan) || (20 == chan) || (32 == chan) || \
     (40 == chan) || (52 == chan) || (12 == chan) || (28 == chan) || \
     (44 == chan) || (60 == chan))
#define SYS_DATAPATH_STUB_CHAN_NUM  10

struct sys_datapath_chan_epe_mem_s
{
    uint8 stub_chan_id;
    uint32 body_s_pointer;
    uint32 sop_s_pointer;
};
struct sys_datapath_chan_epe_mem_s g_epe_mem_info[SYS_DATAPATH_STUB_CHAN_NUM] = {{0,0,0}};

extern int32 _sys_usw_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable);
extern int32 sys_usw_mac_get_mac_en(uint8 lchip, uint32 gport, uint32* p_enable);
extern int32 _sys_usw_mac_set_fec_en(uint8 lchip, uint16 lport, uint32 value);
extern int32 _sys_usw_mac_set_mac_pre_config(uint8 lchip, uint16 lport);
extern int32 _sys_usw_mac_set_mac_config(uint8 lchip, uint16 lport, uint8 is_init);
extern int32 _sys_usw_mac_flow_ctrl_cfg(uint8 lchip, uint16 lport);
extern int32 _sys_usw_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable);
extern int32 _sys_usw_mac_set_unidir_en(uint8 lchip, uint16 lport, bool enable);
extern uint16 sys_usw_datapath_get_lport_with_chan(uint8 lchip, uint8 slice_id, uint8 chan_id);
extern int32 _sys_usw_mac_get_cl73_auto_neg(uint8 lchip, uint16 lport, uint32 type, uint32* p_value);
extern int32 sys_usw_peri_set_phy_scan_cfg(uint8 lchip);
extern int32 sys_usw_mac_set_mac_rx_en(uint8 lchip, uint32 gport, uint32 value);
extern int32 sys_usw_ftm_misc_config(uint8 lchip);
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define __DEBUG__

STATIC int32
_sys_usw_datapath_show_calendar(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 txqm = 0;
    uint8 index = 0;
    uint8 sub_idx = 0;
    uint8 slice_id = 0;
    uint8 is_back[4] = {0};
    uint8 walkend[4] = {0};
    uint8 mac_id[8] = {0xff};
    char* txqm_str[4] = {"Txqm0", "Txqm1", "Txqm2", "Txqm3"};
    NetTxCal_m cal_entry;
    NetTxCalCtl_m cal_ctl;

    DATAPATH_INIT_CHECK();

    cmd = DRV_IOR((NetTxCalCtl_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cal_ctl);

    for (txqm = 0; txqm < 4; txqm++)
    {
        is_back[txqm] = GetNetTxCalCtl(V, calEntry0Sel_f + txqm, &cal_ctl);

        if (!is_back[txqm])
        {
            walkend[txqm] = GetNetTxCalCtl(V, walkerEnd0_f + txqm*2, &cal_ctl);
        }
        else
        {
            walkend[txqm] = GetNetTxCalCtl(V, walkerEnd0Bak_f + txqm*2, &cal_ctl);
        }
    }

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (txqm = 0; txqm < 4; txqm++)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s\twalkend: %d\n", txqm_str[txqm], walkend[txqm]);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------- \n");

            for (index = txqm*64; index < txqm*64 + walkend[txqm]; index++)
            {
                if ((index%8) == 0)
                {
                    for (sub_idx = 0; sub_idx < 8; sub_idx++)
                    {
                        if (!is_back[txqm])
                        {
                            cmd = DRV_IOR((NetTxCal_t+slice_id), DRV_ENTRY_FLAG);
                        }
                        else
                        {
                            cmd = DRV_IOR((NetTxCalBak_t+slice_id), DRV_ENTRY_FLAG);
                        }
                        DRV_IOCTL(lchip, index+sub_idx, cmd, &cal_entry);

                        mac_id[sub_idx] = GetNetTxCal(V, calEntry_f, &cal_entry);
                        if (index + sub_idx >= txqm*64 + walkend[txqm])
                        {
                            mac_id[sub_idx] = 127;
                        }
                    }

                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d \n", "Entry index:", index, index+1,
                        index+2, index+3, index+4, index+5, index+6, index+7);
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-12s %-4d %-4d %-4d %-4d %-4d %-4d %-4d %-4d \n", "Mac Id:",mac_id[0], mac_id[1],
                        mac_id[2], mac_id[3], mac_id[4], mac_id[5], mac_id[6], mac_id[7]);
                }
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------- \n");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_show_clktree(uint8 lchip, uint32 start, uint32 end, uint8 is_all)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    HsClkTreeCfg0_m hs_clk_tree;
    CsClkTreeCfg0_m cs_clk_tree;
    uint32 TxIntfClkSel = 0;
    uint32 TxCoreClkSel = 0;
    uint32 CoreA_sel = 0;
    uint32 CoreB_sel = 0;
    uint32 intf_sel = 0;
    uint32 fld_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 start_idx = 0;
    uint8 end_idx = 0;
    uint8 lane_idx = 0;
    uint8 index = 0;
    uint8 internal_idx = 0;
    char* pll_str[SYS_DATAPATH_HSSCLK_MAX] = {"Disable","515.625M","500M", "644.53125M","625M"};
    char* div_str[5] = {"Disable","FULL","HALF", "QUAD","EIGHTH"};
    char* idle = "-";
    char* tm_width[6] = {"-", "10bit", "16bit", "20bit", "32bit", "64bit"};
    uint32 tm_rate_div[6] = {0, 1, 2, 4, 8, 16};

    DATAPATH_INIT_CHECK();

    if (is_all)
    {
        start_idx = 0;
        end_idx = SYS_DATAPATH_HSS_NUM;
    }
    else
    {
        if((end < start) || (SYS_DATAPATH_HSS_NUM < start) || (SYS_DATAPATH_HSS_NUM < end))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Illegal start or end index! start_idx=%u, end_idx=%u.\n", start, end);
            return 0;
        }
        start_idx = start;
        end_idx = end+1;
    }

    for (index = start_idx; index < end_idx; index++)
    {
        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, index);
        if (NULL == p_hss)
        {
            continue;
        }

        if(DRV_IS_TSINGMA(lchip))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "HSSID: %d  Type: %-8s PLL ref clock: 156.25M\n", p_hss->hss_id, (p_hss->hss_type)?"HSS28G":"HSS12G");
            if(0 == p_hss->hss_type) //HSS12G
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SerdesRate(VCO) = PLL ref clock * Multiply factor\n");
            }
            else //HSS28G
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SerdesRate(VCO) = PLL ref clock * Multiply factor * 4\n");
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FinalSpeed = SerdesRate * RateSel\n");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------\n");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-4s %-6s %-10s %-8s \n", "Lane", "CMUId", "DataWidth", "RateSel");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------\n");
            for(lane_idx = 0; lane_idx < ((p_hss->hss_type)?HSS28G_LANE_NUM:HSS12G_LANE_NUM); lane_idx++)
            {
                if(p_hss->serdes_info[lane_idx].mode == CTC_CHIP_SERDES_NONE_MODE)
                {
                     if(0 != p_hss->serdes_info[lane_idx].is_dyn)
                     {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4u %-6s %-10s %-8s \n", lane_idx, idle, idle, idle);
                     }
                     continue;
                }
                else
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4u ", lane_idx);
                }
                if(0 == p_hss->serdes_info[lane_idx].pll_sel)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s ", idle);
                    continue;
                }
                else
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u ", (p_hss->serdes_info[lane_idx].pll_sel - 1));
                }
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s ", tm_width[p_hss->serdes_info[lane_idx].bit_width]);
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"1/%-6u \n", tm_rate_div[p_hss->serdes_info[lane_idx].rate_div]);
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------\n\n");
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hssid  %d  Type:%-8s \n", p_hss->hss_id, (p_hss->hss_type)?"HSS28G":"HSS15G");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HssClkA:%-8s A0InfDiv:%-8s A0CoreDiv:%-8s A1CoreDiv:%-8s\n", pll_str[p_hss->plla_mode], div_str[p_hss->intf_div_a],
                                div_str[p_hss->core_div_a[0]], div_str[p_hss->core_div_a[1]]);

            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HssClkB:%-8s B0InfDiv:%-8s B0CoreDiv:%-8s B1CoreDiv:%-8s\n", pll_str[p_hss->pllb_mode], div_str[p_hss->intf_div_b],
                                div_str[p_hss->core_div_b[0]], div_str[p_hss->core_div_b[1]]);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------------------------------------\n");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4s %-16s %-16s %-8s %-8s %-8s\n", "LANE", "TxIntfSel", "TxCoreSel", "CoreASel", "CoreBSel", "IntfSel");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------------------------------------\n");
            for (lane_idx = 0; lane_idx < ((p_hss->hss_type)?HSS28G_LANE_NUM:HSS15G_LANE_NUM); lane_idx++)
            {
                internal_idx = lane_idx;
                if (p_hss->serdes_info[lane_idx].mode == CTC_CHIP_SERDES_NONE_MODE)
                {
                     SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4d %-8s\n",  lane_idx, "Disable");
                }
                else
                {
                    if (p_hss->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
                    {
                        tb_id = HsClkTreeCfg0_t + p_hss->hss_id;
                        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
                        DRV_IOCTL(lchip, 0, cmd, &hs_clk_tree);
                        fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*internal_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &TxIntfClkSel, &hs_clk_tree);
                        fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*internal_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &TxCoreClkSel, &hs_clk_tree);
                        fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*internal_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &CoreA_sel, &hs_clk_tree);
                        fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*internal_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &CoreB_sel, &hs_clk_tree);
                        fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*internal_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &intf_sel, &hs_clk_tree);
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4d %-16s %-16s %-8s %-8s %-8s\n",  lane_idx, TxIntfClkSel?"CoreClk":"IntfClk",
                            TxCoreClkSel?"CoreB":"CoreA", CoreA_sel?"A0":"A1", CoreB_sel?"B0":"B1", intf_sel?"A0":"B0");
                    }
                    else
                    {
                        tb_id = CsClkTreeCfg0_t + p_hss->hss_id;
                        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
                        DRV_IOCTL(lchip, 0, cmd, &cs_clk_tree);
                        fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*lane_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &TxIntfClkSel, &cs_clk_tree);
                        fld_id = CsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*lane_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &TxCoreClkSel, &cs_clk_tree);
                        fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &CoreA_sel, &cs_clk_tree);
                        fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &CoreB_sel, &cs_clk_tree);
                        fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*lane_idx;
                        DRV_IOR_FIELD(lchip, tb_id, fld_id, &intf_sel, &cs_clk_tree);
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4d %-16s %-16s %-8s %-8s %-8s\n",  lane_idx, TxIntfClkSel?"CoreClk":"IntfClk",
                            TxCoreClkSel?"CoreB":"CoreA", CoreA_sel?"A0":"A1", CoreB_sel?"B0":"B1", intf_sel?"A0":"B0");
                    }
                }
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------------------\n");
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_show_hss(uint8 lchip, uint32 start, uint32 end, uint8 is_all)
{
    uint8 start_idx = 0;
    uint8 end_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint32 multa = 0;
    uint32 multb = 0;
    uint8 index = 0;
    char* pll_str[SYS_DATAPATH_HSSCLK_MAX] = {"Disable","515.625M","500M", "644.53125M","625M", "567.1875M"};
    char* vco_str[2][SYS_DATAPATH_HSSCLK_MAX] =
    {
        {"Disable","10.3125G","10G", "12.890625G","12.5G", "11.34375G"},
        {"Disable","20.625G","20G", "25.78125G","25G", "Not Support"}
    };
    uint8 tm_mult_12g[SYS_DATAPATH_CMU_MAX] = {0, 66, 64, 80, 80};
    uint8 tm_mult_28g[SYS_DATAPATH_28G_CMU_MAX] = {0, 165, 33, 32, 40, 180, 192, 40};
    uint8 tm_div4_28g[SYS_DATAPATH_28G_CMU_MAX] = {0, 1, 0, 0, 0, 1, 1, 0};
    char* tm_vco_28g[SYS_DATAPATH_28G_CMU_MAX] = {"Disable", "23.2~25.1Gbps(2)", "19.4~20.5Gbps(5)", "19.4~20.5Gbps(5)",
                                                  "23.2~25.1Gbps(2)", "27.3~30.1Gbps(0)", "27.3~30.1Gbps(0)", "23.2~25.1Gbps(2)"};

    if(DRV_IS_TSINGMA(lchip))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4s %-6s %-7s %-6s %-6s %-6s %-7s %-10s \n", "IDX", "HSSID", "TYPE", "MULTA", "MULTB", "MULTC", "DIV4EN", "VCO RANGE");
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4s %-8s %-8s %-12s %-12s %-12s %-12s %-8s %-8s\n", "IDX", "HSSID", "TYPE", "PLLA", "PLLB", "VCOA", "VCOB", "MULTA", "MULTB");
    }
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"---------------------------------------------------------------------------------------\n");

    DATAPATH_INIT_CHECK();

    if (is_all)
    {
        start_idx = 0;
        end_idx = SYS_DATAPATH_HSS_NUM;
    }
    else
    {
        if((end < start) || (SYS_DATAPATH_HSS_NUM < start) || (SYS_DATAPATH_HSS_NUM < end))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Illegal start or end index! start=%u, end=%u.\n", start, end);
            return 0;
        }
        start_idx = start;
        end_idx = end+1;
    }

    for (index = start_idx; index < end_idx; index++)
    {
        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, index);
        if (NULL == p_hss)
        {
            continue;
        }

        multa = 0;
        multb = 0;

        if(DRV_IS_TSINGMA(lchip))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-4u %-6u %-7s ", index, p_hss->hss_id, (p_hss->hss_type?"HSS28G":"HSS12G"));
            if(!p_hss->hss_type) //hss12g
            {
                if((SYS_DATAPATH_CMU_MAX > p_hss->plla_mode) || (SYS_DATAPATH_CMU_IDLE < p_hss->plla_mode))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u ", tm_mult_12g[p_hss->plla_mode]);
                }
                else
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s ", "-");
                }

                if((SYS_DATAPATH_CMU_MAX > p_hss->pllb_mode) || (SYS_DATAPATH_CMU_IDLE < p_hss->pllb_mode))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u ", tm_mult_12g[p_hss->pllb_mode]);
                }
                else
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s ", "-");
                }

                if((SYS_DATAPATH_CMU_MAX > p_hss->pllc_mode) || (SYS_DATAPATH_CMU_IDLE < p_hss->pllc_mode))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6u ", tm_mult_12g[p_hss->pllc_mode]);
                }
                else
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-6s ", "-");
                }
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s %-10s \n", "-", "-");
            }
            else //hss28g
            {
                if((SYS_DATAPATH_28G_CMU_MAX > p_hss->plla_mode) || (SYS_DATAPATH_28G_CMU_IDLE < p_hss->plla_mode))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6u %-6s %-6s %-7u %-10s \n", tm_mult_28g[p_hss->plla_mode],
                                         "-", "-", tm_div4_28g[p_hss->plla_mode], tm_vco_28g[p_hss->plla_mode]);
                }
                else if((SYS_DATAPATH_28G_CMU_MAX > p_hss->pllb_mode) || (SYS_DATAPATH_28G_CMU_IDLE < p_hss->pllb_mode))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s %-6u %-6s %-7u %-10s \n", "-", tm_mult_28g[p_hss->pllb_mode],
                                         "-", tm_div4_28g[p_hss->pllb_mode], tm_vco_28g[p_hss->pllb_mode]);
                }
            }
        }
        else
        {
            if (p_hss->plla_mode)
            {
                if (!p_hss->hss_type)
                {
                    SYS_DATAPATH_GET_HSS15G_MULT(p_hss->plla_mode, multa);
                }
                else
                {
                    SYS_DATAPATH_GET_HSS28G_MULT(p_hss->plla_mode, multa);
                }
            }

            if (p_hss->pllb_mode)
            {
                if (!p_hss->hss_type)
                {
                    SYS_DATAPATH_GET_HSS15G_MULT(p_hss->pllb_mode, multb);
                }
                else
                {
                    SYS_DATAPATH_GET_HSS28G_MULT(p_hss->pllb_mode, multb);
                }
            }

            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-4d %-8d %-8s %-12s %-12s %-12s %-12s 0x%-8x 0x%-8x\n", index, p_hss->hss_id, (p_hss->hss_type?"HSS28G":"HSS15G"),
            pll_str[p_hss->plla_mode], pll_str[p_hss->pllb_mode], vco_str[p_hss->hss_type][p_hss->plla_mode], vco_str[p_hss->hss_type][p_hss->pllb_mode],
            multa, multb);
        }
    }
    return 0;
}

STATIC int32
_sys_usw_datapath_show_serdes(uint8 lchip, uint32 start, uint32 end, uint8 is_all)
{
    char* mode[CTC_CHIP_MAX_SERDES_MODE] = {"None", "Xfi", "Sgmii", "Not support", "Qsgmii", "Xaui", "D-xaui", "Xlg", "Cg","2.5G", "Usxgmii0", "Usxgmii1", "Usxgmii2", "Xxvg", "Lg", "100BaseFX"};
    char* bw[4] = {"8BITS", "10BITS", "20BITS", "40BITS"};
    char* div[4] = {"FULL", "HALF", "QUAD", "EIGHTH"};
    char* tm_bw[6] = {"NULL", "10BITS", "16BITS", "20BITS", "32BITS", "64BITS"};
    char* tm_div[6] = {"NULL", "FULL", "HALF", "QUAD", "EIGHTH", "HEXL"};
    uint8 start_idx = 0;
    uint8 end_idx = 0;
    uint32 hss_idx = 0;
    uint8 lane_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_lport_attr_t* p_lport_attr = NULL;
    uint8 index = 0;
    uint8 slice_id = 0;
    uint8 gchip = 0;

    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8s %-8s %-9s %-8s %-8s %-8s %-5s %-5s %-5s   %-5s %-9s\n", "SERDESID", "SLICE", "HSSID", "MODE", "BW", "RS", "PLLSEL", "CHAN", "MAC", "GPORT", "DYN", "OVERCLOCK");
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------------------------------------------\n");


    sys_usw_get_gchip_id(lchip, &gchip);

    if (is_all)
    {
        start_idx = 0;
        end_idx = SERDES_NUM_PER_SLICE;
    }
    else
    {
        if((end < start) || (SERDES_NUM_PER_SLICE < start) || (SERDES_NUM_PER_SLICE < end))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Illegal start or end index! start=%u, end=%u.\n", start, end);
            return 0;
        }
        start_idx = start;
        end_idx = end+1;
    }

    for (index = start_idx; index < end_idx; index++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(index);
        lane_idx = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(index);

        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss)
        {
            continue;
        }

        slice_id = 0;
        p_serdes = &p_hss->serdes_info[lane_idx];

        if (CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode)
        {
            if(0 != p_serdes->is_dyn)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8d %-8d %-8d %-9s %-8s %-8s %-8s %-5s %-5s %-5s   %-5d %-9s\n", index, slice_id, p_hss->hss_id, mode[p_serdes->mode],
                "-", "-", "-","-", "-", "-", p_serdes->is_dyn, "-");
            }
            continue;
        }

        p_lport_attr = (sys_datapath_lport_attr_t*)(&(p_usw_datapath_master[lchip]->port_attr[slice_id][p_serdes->lport]));
        if(DRV_IS_TSINGMA(lchip))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8d %-8d %-8d %-9s %-8s %-8s %-8s %-5d %-5d 0x%04X  %-5d %-9d\n", index, slice_id, p_hss->hss_id, mode[p_serdes->mode],
            tm_bw[p_serdes->bit_width], tm_div[p_serdes->rate_div], ((p_serdes->pll_sel==2) ?"PLLB":(p_serdes->pll_sel==1 ? "PLLA":(p_serdes->pll_sel==3 ? "PLLC":"NONE"))),
            p_lport_attr->chan_id, p_lport_attr->mac_id, CTC_MAP_LPORT_TO_GPORT(gchip, p_serdes->lport), p_serdes->is_dyn, p_serdes->overclocking_speed);
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8d %-8d %-8d %-9s %-8s %-8s %-8s %-5d %-5d 0x%04X  %-5d %-9d\n", index, slice_id, p_hss->hss_id, mode[p_serdes->mode],
            bw[p_serdes->bit_width], div[p_serdes->rate_div], ((p_serdes->pll_sel ==SYS_DATAPATH_PLL_SEL_PLLB) ?"PLLB":"PLLA"),p_lport_attr->chan_id, p_lport_attr->mac_id,
            CTC_MAP_LPORT_TO_GPORT(gchip, p_serdes->lport), p_serdes->is_dyn, p_serdes->overclocking_speed);
        }
    }

    return CTC_E_NONE;
}


int32
sys_usw_datapath_show_info(uint8 lchip, uint8 type, uint32 start, uint32 end, uint8 is_all)
{
    DATAPATH_INIT_CHECK();

    if (type == SYS_DATAPATH_DEBUG_HSS)
    {
        _sys_usw_datapath_show_hss(lchip, start, end, is_all);
    }
    else if (type == SYS_DATAPATH_DEBUG_SERDES)
    {
        _sys_usw_datapath_show_serdes(lchip, start, end, is_all);
    }
    else if (type == SYS_DATAPATH_DEBUG_CLKTREE)
    {
        _sys_usw_datapath_show_clktree(lchip, start, end, is_all);
    }
    else if (type == SYS_DATAPATH_DEBUG_CALENDAR)
    {
        _sys_usw_datapath_show_calendar(lchip);
    }
    return CTC_E_NONE;
}

int32
sys_usw_datapath_show_status(uint8 lchip)
{
    uint8 total_cnt = 0;
    uint8 sgmii_cnt = 0;
    uint8 qsgmii_cnt = 0;
    uint8 usxgmii_cnt = 0;
    uint8 xfi_cnt = 0;
    uint8 xlg_cnt = 0;
    uint8 dot25_cnt = 0;
    uint8 xaui_cnt = 0;
    uint8 dxaui_cnt = 0;
    uint8 xxvg_cnt = 0;
    uint8 lg_cnt = 0;
    uint8 cg_cnt = 0;
    uint8 fx_cnt = 0;
    uint8 index = 0;
    uint8 hss_idx = 0;
    uint8 lane_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    if(DRV_IS_TSINGMA(lchip))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Feature not support!\n");
        return CTC_E_NOT_SUPPORT;
    }

    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "Core FrequencyA", p_usw_datapath_master[lchip]->core_plla);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "Core FrequencyB", p_usw_datapath_master[lchip]->core_pllb);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    for (index = 0; index < SERDES_NUM_PER_SLICE; index++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(index);
        lane_idx = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(index);

        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss)
        {
            continue;
        }

        p_serdes = &p_hss->serdes_info[lane_idx];
        if (p_serdes->mode != CTC_CHIP_SERDES_NONE_MODE)
        {
            total_cnt++;
        }

        if (p_serdes->mode == CTC_CHIP_SERDES_XFI_MODE)
        {
            xfi_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE)
        {
            sgmii_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_QSGMII_MODE)
        {
            qsgmii_cnt++;
        }
        else if ((p_serdes->mode == CTC_CHIP_SERDES_USXGMII0_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_USXGMII1_MODE) ||
            (p_serdes->mode == CTC_CHIP_SERDES_USXGMII2_MODE))
        {
            usxgmii_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE)
        {
            xaui_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE)
        {
            dxaui_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE)
        {
            xlg_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_XXVG_MODE)
        {
            xxvg_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_LG_MODE)
        {
            lg_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE)
        {
            cg_cnt++;
        }
        else if (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)
        {
            dot25_cnt++;
        }
        else if(DRV_IS_TSINGMA(lchip) && (p_serdes->mode == CTC_CHIP_SERDES_100BASEFX_MODE))
        {
            fx_cnt++;
        }
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "Total Serdes Count", total_cnt);
    if (sgmii_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--Sgmii Mode Count", sgmii_cnt);
    }
    if (qsgmii_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--Qsgmii Mode Count", qsgmii_cnt);
    }
    if (dot25_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--2.5G Mode Count", dot25_cnt);
    }
    if (usxgmii_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--USXGMII Mode Count", usxgmii_cnt);
    }
    if (xfi_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--XFI Mode Count", xfi_cnt);
    }
    if (xaui_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--Xaui Mode Count", xaui_cnt);
    }
    if (dxaui_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--D-Xaui Mode Count", dxaui_cnt);
    }
    if (xlg_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--XLG Mode Count", xlg_cnt);
    }
    if (xxvg_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--XXVG Mode Count", xxvg_cnt);
    }
    if (lg_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--LG Mode Count", lg_cnt);
    }
    if (cg_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--CG Mode Count", cg_cnt);
    }
    if(DRV_IS_TSINGMA(lchip) && fx_cnt)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s: %2d\n", "--100BsFx Mode Count", fx_cnt);
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s\n", "Internal Port Status");
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8s %-8s\n", "Type", "PortId", "ChanId");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------\n");
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8d %-8d\n", "OAM", SYS_RSV_PORT_OAM_CPU_ID, p_usw_datapath_master[lchip]->oam_chan);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8s %-8d\n", "DMA", "-", p_usw_datapath_master[lchip]->dma_chan);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8d %-8d\n", "ILOOP", SYS_RSV_PORT_ILOOP_ID, p_usw_datapath_master[lchip]->iloop_chan);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-8s %-8d %-8d\n", "ELOOP", SYS_RSV_PORT_ELOOP_ID, p_usw_datapath_master[lchip]->eloop_chan);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s\n", "ChipPort-mac-chan mapping(S0 means Slice0)");
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"------------------------------------------------------\n");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s:   |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s|\n",
        "ChipPort", "S0 0~11", "S0 12~15", "S0 16~27", "S0 28~31", "S0 32~43", "S0 44~47", "S0 48~59", "S0 60~63");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s:   |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s|\n",
        "Channel", "S0 0~11", "S0 12~15", "S0 16~27", "S0 28~31", "S0 32~43", "S0 44~47", "S0 48~59", "S0 60~63");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s:   |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s|\n",
        "MacId", "S0 0~11", "S0 12~15", "S0 16~27", "S0 28~31", "S0 32~43", "S0 44~47", "S0 48~59", "S0 60~63");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s:   |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s| |%-8s|\n",
        "SharedPcs/Usxgmii", "0~1/0~2", "6/None", "2/3~5", "7/None", "3~4/6~8", "8/None", "5/9~11", "9/None");

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"---------------------------------------------------------------------\n");

    return CTC_E_NONE;
}

#define __CLKTREE__

STATIC int32
_sys_usw_datapath_get_rate(uint8 lchip, uint8 mode, uint8 type, uint8* p_hssclk,
                            uint8* p_width, uint8* p_div, uint8* p_core_div, uint8* p_intf_div, uint16 overclocking_speed)
{
    switch (mode)
    {
        case CTC_CHIP_SERDES_NONE_MODE:
            *p_hssclk = SYS_DATAPATH_HSSCLK_DISABLE;
            break;

        case CTC_CHIP_SERDES_XFI_MODE:
        case CTC_CHIP_SERDES_XLG_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_11_06G == overclocking_speed)
            {
                *p_hssclk = SYS_DATAPATH_HSSCLK_567DOT1875;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_58G == overclocking_speed)
            {
                *p_hssclk = SYS_DATAPATH_HSSCLK_644DOT53125;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_12G == overclocking_speed)
            {
                *p_hssclk = SYS_DATAPATH_HSSCLK_625;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_NONE == overclocking_speed)
            {
                *p_hssclk = SYS_DATAPATH_HSSCLK_515DOT625;
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = (type == SYS_DATAPATH_HSS_TYPE_15G)?SYS_DATAPATH_SERDES_DIV_FULL: SYS_DATAPATH_SERDES_DIV_HALF;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_FULL;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_USELESS;
            break;

        case CTC_CHIP_SERDES_USXGMII0_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_515DOT625;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = (type == SYS_DATAPATH_HSS_TYPE_15G)?SYS_DATAPATH_SERDES_DIV_FULL: SYS_DATAPATH_SERDES_DIV_HALF;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_FULL;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_USELESS;
            break;

        case CTC_CHIP_SERDES_SGMII_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_500;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_EIGHTH;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_QUAD;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_EIGHTH;
            break;

        case CTC_CHIP_SERDES_XAUI_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_625;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_QUAD;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_QUAD;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_USELESS;
            break;

        case CTC_CHIP_SERDES_DXAUI_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_625;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_HALF;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_HALF;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_USELESS;
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_LG_MODE:
        case CTC_CHIP_SERDES_CG_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_644DOT53125;
            *p_width = SYS_DATAPATH_BIT_WIDTH_40BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_FULL;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_FULL;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_USELESS;
            break;

        case CTC_CHIP_SERDES_2DOT5G_MODE:
        case CTC_CHIP_MAX_SERDES_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_625;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_QUAD;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_HALF;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_QUAD;
            break;

        case CTC_CHIP_SERDES_QSGMII_MODE:
            if (CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
            {
                return CTC_E_INVALID_PARAM;
            }
            *p_hssclk = SYS_DATAPATH_HSSCLK_500;
            *p_width = SYS_DATAPATH_BIT_WIDTH_20BIT;
            *p_div = SYS_DATAPATH_SERDES_DIV_HALF;
            *p_core_div = SYS_DATAPATH_CLKTREE_DIV_FULL;
            *p_intf_div = SYS_DATAPATH_CLKTREE_DIV_HALF;
            break;

        default:
            break;
            /*not support*/
             /*-return CTC_E_INVALID_PARAM;*/
    }

    return CTC_E_NONE;
}

/*
 get mac id/lport/chan id from serdes id
*/
STATIC int32
_sys_usw_datapath_get_serdes_mapping(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 serdes_grp_flag,
                        uint16* p_lport, uint8* p_mac, uint8* p_chan, uint8* p_speed, uint8* p_port_num)
{
    uint8 mac_id = 0;
    uint8 speed = 0;
    uint8 port_num = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    switch(mode)
    {
        case CTC_CHIP_SERDES_XFI_MODE:
            speed = CTC_PORT_SPEED_10G;
            break;

        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_QSGMII_MODE:
            speed = CTC_PORT_SPEED_1G;
            break;

        case CTC_CHIP_SERDES_USXGMII0_MODE:
            speed = CTC_PORT_SPEED_10G;
            break;

        case CTC_CHIP_SERDES_USXGMII1_MODE:
            speed = CTC_PORT_SPEED_2G5;
            break;

        case CTC_CHIP_SERDES_USXGMII2_MODE:
            speed = CTC_PORT_SPEED_5G;
            break;

        case CTC_CHIP_SERDES_XAUI_MODE:
            speed = CTC_PORT_SPEED_10G;
            break;

        case CTC_CHIP_SERDES_DXAUI_MODE:
            speed = CTC_PORT_SPEED_20G;
            break;

        case CTC_CHIP_SERDES_XXVG_MODE:
            speed = CTC_PORT_SPEED_25G;
            break;

        case CTC_CHIP_SERDES_XLG_MODE:
            speed = CTC_PORT_SPEED_40G;
            break;

        case CTC_CHIP_SERDES_LG_MODE:
            speed = CTC_PORT_SPEED_50G;
            break;

        case CTC_CHIP_SERDES_CG_MODE:
            speed = CTC_PORT_SPEED_100G;
            break;

        case CTC_CHIP_SERDES_2DOT5G_MODE:
            speed = CTC_PORT_SPEED_2G5;
            break;

        default:
            break;
    }

    /* serdes/port = 1/(4 |2) */
    if ((mode == CTC_CHIP_SERDES_QSGMII_MODE) || (mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        if ((lane_id == 2) || (lane_id == 3) || (lane_id == 6) || (lane_id == 7))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"serdes %d don't support mode %d \n", serdes_id, mode);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid configuration \n");
            return CTC_E_NONE;

        }

        /* serdes/port = 1/4 */
        if (mode != CTC_CHIP_SERDES_USXGMII2_MODE)
        {
            port_num = 4;
        }/* serdes/port = 1/2 */
        else
        {
            port_num = 2;
        }

        if (serdes_id != 4 && serdes_id != 17)
        {
            if (lane_id <= 1)
            {
                mac_id = (serdes_id - hss_id * 4) * 4 + hss_id * 4;
            }
            else
            {
                mac_id = (serdes_id - (hss_id * 2 + 1) * 2) * 4 + (hss_id + 1) * 4;
            }
        }
        else
        {
            if (serdes_id == 4)
            {
                mac_id = (serdes_id - (hss_id * 2 + 1) * 2) * 4 + (hss_id + 1) * 4 - 4;
            }
            else
            {
                mac_id = (serdes_id - hss_id * 4) * 4 + hss_id * 4 + 4;
            }
        }
    }/* serdes/port = 1/1 */
    else if ((mode == CTC_CHIP_SERDES_SGMII_MODE) || (mode == CTC_CHIP_SERDES_2DOT5G_MODE) ||
        (mode == CTC_CHIP_SERDES_XFI_MODE) ||(mode == CTC_CHIP_SERDES_USXGMII0_MODE) || (mode == CTC_CHIP_SERDES_XXVG_MODE))
    {
        port_num = 1;
        if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            if (serdes_id >= 4 && serdes_id <= 7)
            {
                mac_id = serdes_id + 4;
            }
            else if (lane_id <= 3)
            {
                mac_id = serdes_id % 4 + serdes_id / 4 * 8 + hss_id * 4;
            }
            else
            {
                mac_id = serdes_id % 4 + serdes_id / 4 * 8 + (hss_id + 1) * 4;
            }
        }
        else
        {
            mac_id = serdes_id - 24 + (hss_id + 1) * 3 * 4;
        }
    }/* serdes/port = 2/1 */
    else if (mode == CTC_CHIP_SERDES_LG_MODE)
    {
        port_num = 1;
        if (serdes_grp_flag)
        {
            if ((1 == serdes_id%4) || (2 == serdes_id%4))
            {
                mac_id = serdes_id/4*4 - 24 + (hss_id + 1) * 3 * 4;
            }
            else
            {
                mac_id = serdes_id/4*4 - 24 + (hss_id + 1) * 3 * 4 + 2;
            }
        }
        else
        {
            serdes_id = serdes_id / 2 * 2;
            mac_id = serdes_id - 24 + (hss_id + 1) * 3 * 4;
        }
    }/* serdes/port = 4/1 */
    else if ((mode == CTC_CHIP_SERDES_XAUI_MODE) || (mode == CTC_CHIP_SERDES_DXAUI_MODE) ||
        (mode == CTC_CHIP_SERDES_XLG_MODE) || (mode == CTC_CHIP_SERDES_CG_MODE))
    {
        port_num = 1;
        serdes_id = serdes_id / 4 * 4;
        if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            if (serdes_id >= 4 && serdes_id <= 7)
            {
                mac_id = serdes_id + 4;
            }
            else if (lane_id <= 3)
            {
                mac_id = serdes_id % 4 + serdes_id / 4 * 8 + hss_id * 4;
            }
            else
            {
                mac_id = serdes_id % 4 + serdes_id / 4 * 8 + (hss_id + 1) * 4;
            }
        }
        else
        {
            mac_id = serdes_id - 24 + (hss_id + 1) * 3 * 4;
        }
    }

#if(SDK_WORK_PLATFORM == 1)
        mac_id = serdes_id;
        *p_mac = mac_id;
        *p_chan = mac_id;
        *p_lport = mac_id;
        *p_speed = speed;
        *p_port_num = 1;

        return CTC_E_NONE;
#endif

    *p_mac = mac_id;
    *p_chan = mac_id;
    *p_speed = speed;
    *p_port_num = port_num;
    *p_lport = g_usw_mac_port_map[mac_id];
    if (mode == CTC_CHIP_SERDES_NONE_MODE)
    {
        *p_lport = SYS_COMMON_USELESS_MAC;
    }

    return CTC_E_NONE;
}

/* type:hss15g or hss28g, dir:0-tx,1-rx */
STATIC int32
_sys_usw_datapath_encode_serdes_cfg(uint8 lchip, uint8 type, uint8 dir, uint16* p_value, sys_datapath_serdes_info_t* p_serdes)
{
    uint32 temp = 0;
    uint8 pll_sel = 0;

    if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
    {
        /*Disable serdes*/
        return CTC_E_NONE;
    }

    if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
    {
        pll_sel = 0;
    }
    else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
    {
        pll_sel = 1;
    }

    if (SYS_DATAPATH_HSS_TYPE_15G == type)
    {
       if (p_serdes->bit_width != SYS_DATAPATH_BIT_WIDTH_20BIT)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss15g only suport BW 20bits!\n");
            return CTC_E_INVALID_PARAM;
        }

        if (dir == 0)
        {
            /*tx*/
            *p_value |= ((1 << SYS_DATAPATH_HSS_LINK_EN_BIT) | (1 << SYS_DATAPATH_HSS_WR_MODE_BIT));
            *p_value |= (0x3 << SYS_DATAPATH_HSS_BW_SEL_BIT);
            *p_value |= (p_serdes->rate_div << SYS_DATAPATH_HSS_RATE_SEL_BIT);
            *p_value |= (pll_sel << SYS_DATAPATH_HSS_PLL_SEL_BIT);
        }
        else
        {
            /*rx*/
            *p_value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT) | (1 << SYS_DATAPATH_HSS_WR_MODE_BIT);
            *p_value |= (0x3 << SYS_DATAPATH_HSS_BW_SEL_BIT);
            *p_value |= (p_serdes->rate_div << SYS_DATAPATH_HSS_RATE_SEL_BIT);
            *p_value |= (pll_sel << SYS_DATAPATH_HSS_PLL_SEL_BIT);

            /*for rate_div not equal 1/4 or 1/8, select DFE12*/
            if (p_serdes->rate_div < 2)
            {
                *p_value |= 0x30;
            }
        }
    }
    else
    {
        if ((p_serdes->bit_width == SYS_DATAPATH_BIT_WIDTH_20BIT) && (p_serdes->rate_div == SYS_DATAPATH_SERDES_DIV_FULL))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HSS28G cannot support full divider and 20bits BW!!\n");
            return CTC_E_INVALID_PARAM;
        }

        if (p_serdes->bit_width == SYS_DATAPATH_BIT_WIDTH_20BIT)
        {
            temp = 0x1;
        }
        else if (p_serdes->bit_width == SYS_DATAPATH_BIT_WIDTH_40BIT)
        {
            temp = 0x3;
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"HSS28G only support 20bit and 40bits BW, now bw is %d!!\n", p_serdes->bit_width);
            return CTC_E_INVALID_PARAM;
        }

        if (dir == 0)
        {
            /*tx*/
            *p_value |= ((1 << SYS_DATAPATH_HSS_LINK_EN_BIT) | (1 << SYS_DATAPATH_HSS_WR_MODE_BIT));
            *p_value |= (temp << SYS_DATAPATH_HSS_BW_SEL_BIT);
            *p_value |= (p_serdes->rate_div << SYS_DATAPATH_HSS_RATE_SEL_BIT);
            *p_value |= (pll_sel << SYS_DATAPATH_HSS_PLL_SEL_BIT);
        }
        else
        {
            /*rx*/
            *p_value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT) | (1 << SYS_DATAPATH_HSS_WR_MODE_BIT);
            *p_value |= (temp << SYS_DATAPATH_HSS_BW_SEL_BIT);
            *p_value |= (p_serdes->rate_div << SYS_DATAPATH_HSS_RATE_SEL_BIT);
            *p_value |= (pll_sel << SYS_DATAPATH_HSS_PLL_SEL_BIT);

            /*for rate_div not equal 1/4 or 1/8, select DFE15*/
            if (p_serdes->rate_div < 2)
            {
                *p_value |= 0x30;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_hss15g_cfg_lane_clktree(uint8 lchip, uint8 idx, sys_datapath_hss_attribute_t* p_hss)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    HsClkTreeCfg0_m clk_tree;
    HsMacroCfg0_m hs_macro;
    uint32 tmp_val = 0;
    uint32 fld_id = 0;
    uint8 get_valid_div = 0;
    uint32 out_mode = 0;
    uint8 lane_id = 0;
    uint8 tmp_lane  = 0;

    lane_id = idx;

    p_serdes = &p_hss->serdes_info[idx];

    if (p_serdes->mode == CTC_CHIP_SERDES_NONE_MODE)
    {
        return CTC_E_NONE;
    }

    sal_memset(&clk_tree, 0, sizeof(HsClkTreeCfg0_m));
    sal_memset(&hs_macro, 0, sizeof(HsMacroCfg0_m));

    tb_id = HsClkTreeCfg0_t + p_hss->hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE) ||
        (p_serdes->mode == CTC_CHIP_SERDES_QSGMII_MODE))
    {
        if (p_serdes->mode != CTC_CHIP_SERDES_QSGMII_MODE)
        {
            out_mode = 0;
        }
        else
        {
            out_mode = 0x3;
        }

        /*Must using intf divider*/
        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->intf_div_b == SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"SGMII/QSGMII mode missed interface divider!!, please check! \n");
                return CTC_E_INVALID_PARAM;
            }

            /*core b source select, using b0 core*/
            tmp_val = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f;
                break;
            case 1:
                fld_id = HsClkTreeCfg0_cfgClockHssTxBCoreSB0IntSel_f;
                break;
            case 2:
                fld_id = HsClkTreeCfg0_cfgClockHssTxCCoreSB0IntSel_f;
                break;
            case 3:
                fld_id = HsClkTreeCfg0_cfgClockHssTxDCoreSB0IntSel_f;
                break;
            case 4:
                fld_id = HsClkTreeCfg0_cfgClockHssTxECoreSB0IntSel_f;
                break;
            case 5:
                fld_id = HsClkTreeCfg0_cfgClockHssTxFCoreSB0IntSel_f;
                break;
            case 6:
                fld_id = HsClkTreeCfg0_cfgClockHssTxGCoreSB0IntSel_f;
                break;
            case 7:
                fld_id = HsClkTreeCfg0_cfgClockHssTxHCoreSB0IntSel_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            if ((0 == idx%4) && (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 2;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 3;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->intf_div_a == SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"SGMII/QSGMII mode missed interface divider!!, please check! \n");
                return CTC_E_INVALID_PARAM;
            }

            /*core a source select, using a0 core*/
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            if ((0 == idx%4) && (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 2;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 3;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
        }

        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->core_div_b[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core b source select, if b0 match using b0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_id;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_b[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }

                /*core b source select, if b1 match using b1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_id;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->core_div_a[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core a source select, if a0 match using a0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_a[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }

                /*core a source select, if a1 match using a1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }
        }

        /*interface source select, plla using A0Intf, pllb using B0Intf*/
        fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*lane_id;
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?0:1;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        if ((0 == idx%4) && (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 2;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 3;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }

        /*core source select, plla acore, pllb using bcore*/
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?1:0;
        fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*lane_id;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        if ((0 == idx%4) && (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 2;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 3;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }

        /*tx clock select, using interface clock */
        tmp_val = 0;
        fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*lane_id;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        if ((0 == idx%4) && (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 2;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 3;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
    }
    else if ((p_serdes->mode == CTC_CHIP_SERDES_XFI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE) ||
        (p_serdes->mode == CTC_CHIP_SERDES_USXGMII0_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_USXGMII1_MODE) ||
        (p_serdes->mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        if ((p_serdes->mode == CTC_CHIP_SERDES_XFI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE))
        {
            out_mode = 0x2;
        }
        else
        {
            out_mode = 0x4;
        }

        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            /*core b source select, using b0 core*/
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 2;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 3;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
            else if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
        }
        else  if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            /*core a source select, using a0 core*/
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 2;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                tmp_lane = lane_id + 3;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
            else if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
            {
                tmp_lane = lane_id + 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*tmp_lane;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }
        }

        /*core source select, using acore or bcore*/
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?1:0;
        fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*lane_id;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 2;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 3;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }

        /*tx clock select, using core clock */
        tmp_val = 1;
        fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*lane_id;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 2;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            tmp_lane = lane_id + 3;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if ((0 == idx%4) && (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
        {
            tmp_lane = lane_id + 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*tmp_lane;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
    }
    else
    {
        out_mode = 0x01;   /* XAUI */

        /*XAUI,DXAUI*/
        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->core_div_b[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core b source select, if b0 match using b0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_id;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_b[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }

                /*core b source select, if b1 match using b1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*lane_id;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }

            if (!get_valid_div)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss15G Can not get valid core divider!! hssid:%d, lane:%d\n", p_hss->hss_id, idx);
                return CTC_E_INVALID_PARAM;
            }

            /*core source select, using b0 or b1*/
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

            /*tx clock select, using core clock */
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->core_div_a[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core a source select, if a0 match using a0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_a[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }

                /*core a source select, if a1 match using a1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = HsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*lane_id;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }

            if (!get_valid_div)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss15G Can not get valid core divider!! hssid:%d, lane:%d\n", p_hss->hss_id, idx);
                return CTC_E_INVALID_PARAM;
            }

            /*core source select, using a0 or a1*/
            tmp_val = 0;
            fld_id = HsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

            /*tx clock select, using core clock */
            tmp_val = 1;
            fld_id = HsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*lane_id;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    tb_id = HsMacroCfg0_t + p_hss->hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_macro);
    switch(lane_id)
    {
        case 0:
            fld_id = HsMacroCfg0_cfgHssTxADataOutSel_f;
            break;
        case 1:
            fld_id = HsMacroCfg0_cfgHssTxBDataOutSel_f;
            break;
        case 2:
            fld_id = HsMacroCfg0_cfgHssTxCDataOutSel_f;
            break;
        case 3:
            fld_id = HsMacroCfg0_cfgHssTxDDataOutSel_f;
            break;
        case 4:
            fld_id = HsMacroCfg0_cfgHssTxEDataOutSel_f;
            break;
        case 5:
            fld_id = HsMacroCfg0_cfgHssTxFDataOutSel_f;
            break;
        case 6:
            fld_id = HsMacroCfg0_cfgHssTxGDataOutSel_f;
            break;
        case 7:
            fld_id = HsMacroCfg0_cfgHssTxHDataOutSel_f;
            break;
    }
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &out_mode, &hs_macro);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_macro);

    if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_QSGMII_MODE))
    {
        tb_id = HsMacroCfg0_t + p_hss->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);
        switch (lane_id)
        {
        case 0:
            fld_id = HsMacroCfg0_cfgResetHssTxA_f;
            break;
        case 1:
            fld_id = HsMacroCfg0_cfgResetHssTxB_f;
            break;
        case 2:
            fld_id = HsMacroCfg0_cfgResetHssTxC_f;
            break;
        case 3:
            fld_id = HsMacroCfg0_cfgResetHssTxD_f;
            break;
        case 4:
            fld_id = HsMacroCfg0_cfgResetHssTxE_f;
            break;
        case 5:
            fld_id = HsMacroCfg0_cfgResetHssTxF_f;
            break;
        case 6:
            fld_id = HsMacroCfg0_cfgResetHssTxG_f;
            break;
        case 7:
            fld_id = HsMacroCfg0_cfgResetHssTxH_f;
            break;
        default:
            fld_id = 0;
            break;
        }
        tmp_val = 1;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);

        sal_task_sleep(1);

        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);
        switch (lane_id)
        {
        case 0:
            fld_id = HsMacroCfg0_cfgResetHssTxA_f;
            break;
        case 1:
            fld_id = HsMacroCfg0_cfgResetHssTxB_f;
            break;
        case 2:
            fld_id = HsMacroCfg0_cfgResetHssTxC_f;
            break;
        case 3:
            fld_id = HsMacroCfg0_cfgResetHssTxD_f;
            break;
        case 4:
            fld_id = HsMacroCfg0_cfgResetHssTxE_f;
            break;
        case 5:
            fld_id = HsMacroCfg0_cfgResetHssTxF_f;
            break;
        case 6:
            fld_id = HsMacroCfg0_cfgResetHssTxG_f;
            break;
        case 7:
            fld_id = HsMacroCfg0_cfgResetHssTxH_f;
            break;
        default:
            fld_id = 0;
            break;
        }
        tmp_val = 0;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);
    }

    tb_id = HsMacroCfg0_t + p_hss->hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_macro);
    switch (lane_id)
    {
    case 0:
        fld_id = HsMacroCfg0_cfgLaneAUsxgmiiUse4Lane_f;
        break;
    case 1:
        fld_id = HsMacroCfg0_cfgLaneBUsxgmiiUse4Lane_f;
        break;
    case 2:
        fld_id = HsMacroCfg0_cfgLaneCUsxgmiiUse4Lane_f;
        break;
    case 3:
        fld_id = HsMacroCfg0_cfgLaneDUsxgmiiUse4Lane_f;
        break;
    case 4:
        fld_id = HsMacroCfg0_cfgLaneEUsxgmiiUse4Lane_f;
        break;
    case 5:
        fld_id = HsMacroCfg0_cfgLaneFUsxgmiiUse4Lane_f;
        break;
    case 6:
        fld_id = HsMacroCfg0_cfgLaneGUsxgmiiUse4Lane_f;
        break;
    case 7:
        fld_id = HsMacroCfg0_cfgLaneHUsxgmiiUse4Lane_f;
        break;
    }
    tmp_val = 1;
    if ((CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
    {
        tmp_val = 0;
    }
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_macro);

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_hss28g_cfg_lane_clktree(uint8 lchip, uint8 idx, sys_datapath_hss_attribute_t* p_hss)
{
    uint32 cmd = 0;
    uint32 tb_id = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    CsClkTreeCfg0_m clk_tree;
    CsMacroCfg0_m cs_macro;
    uint32 tmp_val = 0;
    uint32 fld_id = 0;
    uint8 get_valid_div = 0;
    uint32 out_mode = 0;

    p_serdes = &p_hss->serdes_info[idx];

    if (p_serdes->mode == CTC_CHIP_SERDES_NONE_MODE)
    {
        return CTC_E_NONE;
    }

    sal_memset(&clk_tree, 0, sizeof(CsClkTreeCfg0_m));
    sal_memset(&cs_macro, 0, sizeof(CsMacroCfg0_m));

    tb_id = CsClkTreeCfg0_t + p_hss->hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
    {
        out_mode = 1;

        /*Must using intf divider*/
        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->intf_div_b == SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"SGMII/QSGMII mode missed interface divider!!, please check! \n");
                return CTC_E_INVALID_PARAM;
            }

            /*select core b source, using b0core*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->intf_div_a == SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"SGMII/QSGMII mode missed interface divider!!, please check! \n");
                return CTC_E_INVALID_PARAM;
            }

            /*select core a source, using a0core*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }

        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->core_div_b[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core b source select, if b0 match using b0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_b[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }

                /*core b source select, if b1 match using b1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->core_div_a[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*core a source select, if a0 match using a0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_a[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
                {
                    get_valid_div = 1;
                }

                /*core a source select, if a1 match using a1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }
        }

        /*select interface clock source, plla using A0Intf, pllb using B0Intf*/
        fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfSA0IntSel_f + 5*idx;
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?0:1;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

        /*select core clock source, plla using ACore, pllb using BCore*/
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?1:0;
        fld_id = CsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*idx;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

        /*select tx clock source, using interface clock*/
        tmp_val = 0;
        fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*idx;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
    }
    else if ((p_serdes->mode == CTC_CHIP_SERDES_XFI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_XXVG_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_LG_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE))
    {
        out_mode = 0x1;

        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            /*select core b source, using b0core*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            /*select core a source, using a0core*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }

        /*select core clock source, plla using ACore, pllb using BCore*/
        tmp_val = (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)?1:0;
        fld_id = CsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*idx;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

        /*select tx clock source, using core clock*/
        tmp_val = 1;
        fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*idx;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
    }
    else if ((p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
    {
        out_mode = 0x0;

        /*XAUI,DXAUI*/
        if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
        {
            if (p_hss->core_div_b[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_b[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*select core b source, if b0 match using b0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_b[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_b[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }

                /*select core b source, if b1 match using b1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSB0IntSel_f + 5*idx;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }

            if (!get_valid_div)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss28G Can not get valid core divider!! hssid:%d, lane:%d\n", p_hss->hss_id, idx);
                return CTC_E_INVALID_PARAM;
            }

            /*select core clock source, plla using ACore, pllb using BCore*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

            /*select tx clock source, using core clock*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
        else if (p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
        {
            if (p_hss->core_div_a[0] != SYS_DATAPATH_CLKTREE_DIV_USELESS)
            {
                if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_a[0] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }
            }

            /*select core a source, if a0 match using a0core*/
            if (get_valid_div)
            {
                tmp_val = 1;
                fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*idx;
                DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
            }

            if ((p_hss->core_div_a[1] != SYS_DATAPATH_CLKTREE_DIV_USELESS) && (!get_valid_div))
            {
                if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_QUAD) && (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE))
                {
                    get_valid_div = 1;
                }
                else if ((p_hss->core_div_a[1] == SYS_DATAPATH_CLKTREE_DIV_HALF) && (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
                {
                    get_valid_div = 1;
                }

                /*select core a source, if a1 match using a1core*/
                if (get_valid_div)
                {
                    tmp_val = 0;
                    fld_id = CsClkTreeCfg0_cfgClockHssTxACoreSA0IntSel_f + 5*idx;
                    DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
                }
            }

            if (!get_valid_div)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss28G Can not get valid core divider!! hssid:%d, lane:%d\n", p_hss->hss_id, idx);
                return CTC_E_INVALID_PARAM;
            }

            /*select core clock source, plla using ACore, pllb using BCore*/
            tmp_val = 0;
            fld_id = CsClkTreeCfg0_cfgClockHssTxACoreS1IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);

            /*select tx clock source, using core clock*/
            tmp_val = 1;
            fld_id = CsClkTreeCfg0_cfgClockHssTxAIntfS2IntSel_f + 5*idx;
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &clk_tree);
        }
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Datapath]Unsupported serdes mode \n");
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    tb_id = CsMacroCfg0_t + p_hss->hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_macro);
    if((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
    {
        fld_id = CsMacroCfg0_cfgHssTxADataSgmiiSel_f + 4*idx;
        if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
        {
            out_mode = 1;
        }
        else
        {
            out_mode = 0;
        }
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &out_mode, &cs_macro);
    }

    if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_XFI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_XXVG_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_LG_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE))
    {
        fld_id = CsMacroCfg0_cfgHssTxADataXfiSel_f + 4*idx;
        if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)
            || (p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE))
        {
            out_mode = 0;
        }
        else
        {
            out_mode = 1;
        }
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &out_mode, &cs_macro);
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_macro);

    if ((p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE))
    {
        tb_id = CsMacroCfg0_t + p_hss->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        fld_id = CsMacroCfg0_cfgResetHssTxA_f + idx;
        tmp_val = 1;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);

        sal_task_sleep(1);

        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        fld_id = CsMacroCfg0_cfgResetHssTxA_f + idx;
        tmp_val = 0;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);
    }

    tb_id = CsMacroCfg0_t + p_hss->hss_id;
    fld_id = CsMacroCfg0_cfgHssTxADataCgmacSel_f + 4*idx;
    tmp_val = (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE)?1:0;
    cmd = DRV_IOW(tb_id, fld_id);
    DRV_IOCTL(lchip, 0, cmd, &tmp_val);

    return CTC_E_NONE;
}

/*
serdes tx direction clock cfg, datapath init and dynamic switch change pll cfg will use the interface.
Notice: Before Using the interface must using sys_usw_datapath_build_serdes_info to set serdes info
pll_type: refer to sys_datapath_pll_sel_t
*/
int32
sys_usw_datapath_hss15g_cfg_clktree(uint8 lchip, uint8 hss_idx, uint8 pll_type)
{
    sys_datapath_hss_attribute_t* p_hss = NULL;
    HsClkTreeCfg0_m clk_tree;
    uint8 hss_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value = 0;

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss %d ClkTree Op BitMap:0x%x\n", hss_idx, p_hss->clktree_bitmap);

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_15G)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss parameter error, not match, hss_idx:%d \n", hss_idx);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&clk_tree, 0, sizeof(HsClkTreeCfg0_m));

    hss_id = p_hss->hss_id;

    tb_id = HsClkTreeCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    if ((pll_type == SYS_DATAPATH_PLL_SEL_PLLA) || (pll_type == SYS_DATAPATH_PLL_SEL_BOTH))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss PllA Parameter, mode:%d, intf_div:%d, Core0_div:%d,  Core1_div:%d \n",
            p_hss->plla_mode, p_hss->intf_div_a, p_hss->core_div_a[0], p_hss->core_div_a[1] );

        /*1. cfg plla intercafe divider and core divider*/
        if (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)
        {
            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->intf_div_a);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20A0Intf_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_a[0]);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20A0Core_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_a[1]);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20A1Core_f, &value, &clk_tree);
            }
        }
    }

    if ((pll_type == SYS_DATAPATH_PLL_SEL_PLLB) || (pll_type == SYS_DATAPATH_PLL_SEL_BOTH))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss PllB Parameter, mode:%d, intf_div:%d, Core0_div:%d,  Core1_div:%d \n",
            p_hss->pllb_mode, p_hss->intf_div_b, p_hss->core_div_b[0], p_hss->core_div_b[1] );

        /*2. cfg pllb intercafe divider and core divider*/
        if (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
        {
            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->intf_div_b);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20B0Intf_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_b[0]);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20B0Core_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_b[1]);
                DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkDividerHssTxClk20B1Core_f, &value, &clk_tree);
            }
        }
    }

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    /*3. do clktree reset */
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);
    value = 1;
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A0Intf_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B0Intf_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A0Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A1Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B0Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B1Core_f, &value, &clk_tree);
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    sal_task_sleep(5);

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);
    value = 0;
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A0Intf_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B0Intf_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A0Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20A1Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B0Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
    {
        DRV_IOW_FIELD(lchip, tb_id, HsClkTreeCfg0_cfgClkResetHssTxClk20B1Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1);
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    return CTC_E_NONE;
}

/*
serdes tx direction clock cfg, datapath init and dynamic switch change pll cfg will use the interface.
Notice: Before Using the interface must using sys_usw_datapath_build_serdes_info to set serdes info
pll_type: refer to sys_datapath_pll_sel_t
*/
int32
sys_usw_datapath_hss28g_cfg_clktree(uint8 lchip, uint8 hss_idx, uint8 pll_type)
{
    sys_datapath_hss_attribute_t* p_hss = NULL;
    CsClkTreeCfg0_m clk_tree;
    uint8 hss_id = 0;
    uint32 cmd = 0;
    uint32 tb_id = 0;
    uint32 value = 0;

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_28G)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss parameter error, not match, hss_idx:%d \n", hss_idx);
        return CTC_E_INVALID_PARAM;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss %d ClkTree Op BitMap:0x%x\n", hss_idx, p_hss->clktree_bitmap);

    sal_memset(&clk_tree, 0, sizeof(CsClkTreeCfg0_m));

    hss_id = p_hss->hss_id;

    tb_id = CsClkTreeCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    if ((pll_type == SYS_DATAPATH_PLL_SEL_PLLA) || (pll_type == SYS_DATAPATH_PLL_SEL_BOTH))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss PllA Parameter, mode:%d, intf_div:%d, Core0_div:%d,  Core1_div:%d \n",
            p_hss->plla_mode, p_hss->intf_div_a, p_hss->core_div_a[0], p_hss->core_div_a[1] );

        /*1. cfg plla intercafe divider and core divider*/
        if (p_hss->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)
        {
            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->intf_div_a);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40A0Intf_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_a[0]);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40A0Core_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_a[1]);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40A1Core_f, &value, &clk_tree);
            }
        }
    }

    if ((pll_type == SYS_DATAPATH_PLL_SEL_PLLB) || (pll_type == SYS_DATAPATH_PLL_SEL_BOTH))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Hss PllB Parameter, mode:%d, intf_div:%d, Core0_div:%d,  Core1_div:%d \n",
            p_hss->pllb_mode, p_hss->intf_div_b, p_hss->core_div_b[0], p_hss->core_div_b[1] );
        /*2. cfg pllb intercafe divider and core divider*/
        if (p_hss->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
        {
            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->intf_div_b);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40B0Intf_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_b[0]);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40B0Core_f, &value, &clk_tree);
            }

            if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
            {
                value = SYS_DATAPATH_GET_CLKTREE_DIV(p_hss->core_div_b[1]);
                DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkDividerHssTxClk40B1Core_f, &value, &clk_tree);
            }
        }
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    /*3. do clktree reset */
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);
    value = 1;
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A0Intf_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B0Intf_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A0Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A1Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B0Core_f, &value, &clk_tree);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B1Core_f, &value, &clk_tree);
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    sal_task_sleep(5);

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);
    value = 0;
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A0Intf_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B0Intf_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A0Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40A1Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B0Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
    }
    if (CTC_IS_BIT_SET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1))
    {
        DRV_IOW_FIELD(lchip, tb_id, CsClkTreeCfg0_cfgClkResetHssTxClk40B1Core_f, &value, &clk_tree);
        CTC_BIT_UNSET(p_hss->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1);
    }
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &clk_tree);

    return CTC_E_NONE;
}

/*
   Used to change hss15g plla/pllb cfg, using for dynamic switch
   Notice: Before using the function, must confirm pll is not used by any serdes
   pll_type: refer to sys_datapath_pll_sel_t
   pll_clk:   refer to sys_datapath_hssclk_t
*/
int32
sys_usw_datapath_hss15g_pll_cfg(uint8 lchip, uint8 hss_idx, uint8 pll_type, uint8 pll_clk)
{
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint32 mult = 0;
    uint32 tb_id = 0;
    HsCfg0_m hs_cfg;
    uint32 cmd = 0;
    uint32 value = 0;
    HsMacroMiscMon0_m hs_mon;
    uint32 timeout = 0x4000;
    uint8 pll_ready = 0;

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss %d not used\n", hss_idx);
        return CTC_E_NONE;
    }

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_15G)
    {
        return CTC_E_INVALID_PARAM;
    }

    tb_id = HsCfg0_t + p_hss->hss_id;
    if (pll_type == SYS_DATAPATH_PLL_SEL_PLLA)
    {
        if (pll_clk == SYS_DATAPATH_HSSCLK_DISABLE)
        {
             /*cannot disable ref clock, because pll still have lane, if set disable will cause serdes cannot op*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidA_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
        }
        else
        {
            /*1. enable ref clock */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidA_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            sal_task_sleep(1);

            /*2. change pll config */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            SYS_DATAPATH_GET_HSS15G_MULT(pll_clk, mult);
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssDivSelA_f, &mult, &hs_cfg);

            if ((pll_clk == SYS_DATAPATH_HSSCLK_515DOT625) || (pll_clk == SYS_DATAPATH_HSSCLK_500))
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelA_f, &value, &hs_cfg);
            }
            else
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelA_f, &value, &hs_cfg);
            }
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            /*3. set pll recalculate*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRecCalA_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            sal_task_sleep(1);

            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRecCalA_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
        }

    }
    else if (pll_type == SYS_DATAPATH_PLL_SEL_PLLB)
    {
        if (pll_clk == SYS_DATAPATH_HSSCLK_DISABLE)
        {
             /*cannot disable ref clock, because pll still have lane, if set disable will cause serdes cannot op*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidB_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
        }
        else
        {
            /*1. enable ref clock */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidB_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            sal_task_sleep(1);

            /*2. change pll config */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            SYS_DATAPATH_GET_HSS15G_MULT(pll_clk, mult);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssDivSelB_f, &mult, &hs_cfg);
            if (pll_clk == SYS_DATAPATH_HSSCLK_644DOT53125)
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelB_f, &value, &hs_cfg);
            }
            else
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssVcoSelB_f, &value, &hs_cfg);
            }
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            /*3. set pll recalculate*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRecCalB_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            sal_task_sleep(1);

            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRecCalB_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
        }
    }

    sal_task_sleep(10);

    /*5. wait hssptrready*/
    if (pll_clk != SYS_DATAPATH_HSSCLK_DISABLE)
    {
        tb_id = HsMacroMiscMon0_t + p_hss->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_mon);

        if (0 == SDK_WORK_PLATFORM)
        {
            while(--timeout)
            {
                if (pll_type == SYS_DATAPATH_PLL_SEL_PLLA)
                {
                    if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyA_f, &hs_mon))
                    {
                        pll_ready = 1;
                    }
                }
                else if (pll_type == SYS_DATAPATH_PLL_SEL_PLLB)
                {
                    if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyB_f, &hs_mon))
                    {
                        pll_ready = 1;
                    }
                }

                if (pll_ready)
                {
                    break;
                }
            }

            /*check ready*/
            if (!pll_ready)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;

            }
        }
    }

    return CTC_E_NONE;
}

/*
   Used to change hss28g plla/pllb cfg, using for dynamic switch
   Notice: Before using the function, must confirm pll is not used by any serdes
   pll_type: refer to sys_datapath_pll_sel_t
   pll_clk:   refer to sys_datapath_hssclk_t
*/
int32
sys_usw_datapath_hss28g_pll_cfg(uint8 lchip, uint8 hss_idx, uint8 pll_type, uint8 pll_clk)
{
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint32 mult = 0;
    uint32 tb_id = 0;
    CsCfg0_m cs_cfg;
    uint32 cmd = 0;
    uint32 value = 0;
    CsMacroMiscMon0_m cs_mon;
    uint32 timeout = 0x4000;
    uint8 pll_ready = 0;

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss %d not used\n", hss_idx);
        return CTC_E_NONE;
    }

    if (p_hss->hss_type != SYS_DATAPATH_HSS_TYPE_28G)
    {
        return CTC_E_INVALID_PARAM;
    }

    tb_id = CsCfg0_t + p_hss->hss_id;
    if (pll_type == SYS_DATAPATH_PLL_SEL_PLLA)
    {
        if (pll_clk == SYS_DATAPATH_HSSCLK_DISABLE)
        {
            /*cannot disable ref clock, because pll still have lane, if set disable will cause serdes cannot op*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidA_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
        }
        else
        {
            /*1. Enable ref clock */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidA_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            sal_task_sleep(1);

            /*2. change pll config */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            SYS_DATAPATH_GET_HSS28G_MULT(pll_clk, mult);
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssDivSelA_f, &mult, &cs_cfg);

            /*plla just using VCO:20G,20.625G*/
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelA_f, &value, &cs_cfg);

            if (pll_clk == SYS_DATAPATH_HSSCLK_500)
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2A_f, &value, &cs_cfg);
            }
            else
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2A_f, &value, &cs_cfg);
            }
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            /*3. set pll recalculate*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRecCalA_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            sal_task_sleep(1);

            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRecCalA_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
        }

    }
    else if (pll_type == SYS_DATAPATH_PLL_SEL_PLLB)
    {
        if (pll_clk == SYS_DATAPATH_HSSCLK_DISABLE)
        {
             /*cannot disable ref clock, because pll still have lane, if set disable will cause serdes cannot op*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidB_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
        }
        else
        {
            /*1. enable ref clock */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidB_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            sal_task_sleep(1);

            /*2. change pll config */
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            SYS_DATAPATH_GET_HSS28G_MULT(pll_clk, mult);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssDivSelB_f, &mult, &cs_cfg);

            if (pll_clk == SYS_DATAPATH_HSSCLK_625)
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
            }
            else if (pll_clk == SYS_DATAPATH_HSSCLK_500)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
            }
            else if (pll_clk == SYS_DATAPATH_HSSCLK_515DOT625)
            {
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
            }
            else
            {
                value = 1;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssVcoSelB_f, &value, &cs_cfg);
                value = 0;
                DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPllDiv2B_f, &value, &cs_cfg);
            }
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            /*3. set pll recalculate*/
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRecCalB_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            sal_task_sleep(1);

            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRecCalB_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
        }
    }

    sal_task_sleep(10);

    /*4. wait hssptrready*/
    if (pll_clk != SYS_DATAPATH_HSSCLK_DISABLE)
    {
        tb_id = CsMacroMiscMon0_t + p_hss->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_mon);

        if (0 == SDK_WORK_PLATFORM)
        {
            while(--timeout)
            {
                if (pll_type == SYS_DATAPATH_PLL_SEL_PLLA)
                {
                    if (GetCsMacroMiscMon0(V,monHss28gHssPrtReadyA_f, &cs_mon))
                    {
                        pll_ready = 1;
                    }
                }
                else if (pll_type == SYS_DATAPATH_PLL_SEL_PLLB)
                {
                    if (GetCsMacroMiscMon0(V,monHss28gHssPrtReadyB_f, &cs_mon))
                    {
                        pll_ready = 1;
                    }
                }

                if (pll_ready)
                {
                    break;
                }
            }

            /*check ready*/
            if (!pll_ready)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_store_mcu_data_mem(uint8 lchip, uint16 network_lport, uint8 drop_flag)
{
    uint8 slice_id = 0;
    uint16 chip_port = 0;
    uint8  num = 0;
    uint32 tmp_val32 = 0;
    uint32 port_step = 0;
    uint16 serdes_id = 0;
    uint16 serdes_id2 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    uint8 mcu_mac_id = 0;
    uint8 mcu_serdes_num = 0;
    uint8 mcu_serdes_mode = 0;
    uint8 mcu_flag = 0;
    uint8 mcu_serdes_id = 0;
    uint8 mcu_serdes_id2 = 0;
    uint8 mcu_mac_en = 0;

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }

    port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + network_lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
    port_attr = sys_usw_datapath_get_port_capability(lchip, network_lport, slice_id);

    if (1 == drop_flag)
    {
        mcu_mac_id = 0;
        mcu_serdes_num = 0;
        mcu_serdes_mode = 0;
        mcu_flag = 0;
        mcu_serdes_id = 0;
        mcu_serdes_id2 = 0;
        mcu_mac_en = 0;
    }
    else
    {
        mcu_mac_id = port_attr->mac_id;
        mcu_serdes_num = port_attr->serdes_num;
        mcu_serdes_mode = port_attr->pcs_mode;
        mcu_flag = port_attr->flag;

        chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(network_lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
        sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num);

        if ((CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) && (1 == port_attr->flag))
        {
            if (0 == port_attr->mac_id % 4)
            {
                serdes_id2 = serdes_id + 1;
            }
            else
            {
                serdes_id2 = serdes_id + 3;
            }
        }
        mcu_serdes_id = serdes_id;
        mcu_serdes_id2 = serdes_id2;
        mcu_mac_en = 0;
    }


    tmp_val32 = 0;
    tmp_val32 |= (mcu_mac_en              );
    tmp_val32 |= (mcu_mac_id         <<  8);
    tmp_val32 |= (mcu_serdes_id      << 16);
    tmp_val32 |= (mcu_serdes_id2     << 24);
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, port_step, tmp_val32));

    tmp_val32 = 0;
    tmp_val32 |= (mcu_serdes_num          );
    tmp_val32 |= (mcu_serdes_mode    <<  8);
    tmp_val32 |= (mcu_flag           << 16);
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (port_step+0x4), tmp_val32));

    return CTC_E_NONE;
}

/**
 @brief get register index of Mac Mii and SharedPcs
*/
int32
_sys_usw_get_mac_mii_pcs_index(uint8 lchip, uint8 mac_id, uint8 serdes_id, uint8 pcs_mode, uint8* sgmac_idx,
                                            uint8* mii_idx, uint8* pcs_idx, uint8* internal_mac_idx)
{
    uint8 sgmac_id;
    uint8 mii_id;
    uint8 pcs_id;
    uint8 max_pcs_idx = 0;

    CTC_MAX_VALUE_CHECK(mac_id, SYS_PHY_PORT_NUM_PER_SLICE - 1);

    if((CTC_CHIP_SERDES_QSGMII_MODE   == pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == pcs_mode) ||
       (CTC_CHIP_SERDES_USXGMII1_MODE == pcs_mode) || (CTC_CHIP_SERDES_USXGMII2_MODE == pcs_mode))
    {
        max_pcs_idx = 12;
    }
    else
    {
        max_pcs_idx = 10;
    }

    sgmac_id = mac_id / 4;

    if ((pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE) || (pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE) ||
        (pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE) || (pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            return CTC_E_INVALID_PARAM;
        }

        if (pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
        {
            mii_id = serdes_id / 4 * 2;
            pcs_id = serdes_id / 4 * 2;
        }
        else
        {
            if (serdes_id % 4 == 0)
            {
                mii_id = serdes_id / 2;
                pcs_id = serdes_id / 2;
            }
            else if (serdes_id % 4 == 1)
            {
                mii_id = (serdes_id - 1) / 2 + 1;
                pcs_id = (serdes_id - 1) / 2 + 1;
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    else
    {
        if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            mii_id = (serdes_id / 4) * 2;
        }
        else
        {
            mii_id = 12 + (serdes_id - 24) / 4;
        }

        pcs_id = serdes_id / 4;
    }

    if((sgmac_id >= 16) || (mii_id >= 16) || (pcs_id >= max_pcs_idx))
    {
        SYS_MAC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Get index error! mac_id %u, sgmac_idx %u, mii_idx %u, pcs_idx %u\n",
                        mac_id, sgmac_id, mii_id, pcs_id);
        return CTC_E_INVALID_PARAM;
    }

    SYS_USW_VALID_PTR_WRITE(sgmac_idx,        sgmac_id);
    SYS_USW_VALID_PTR_WRITE(mii_idx,          mii_id);
    SYS_USW_VALID_PTR_WRITE(pcs_idx,          pcs_id);
    SYS_USW_VALID_PTR_WRITE(internal_mac_idx, (mac_id % 4));

    return CTC_E_NONE;
}

/*
   Used to build serdes info softeware, prepare for set serdes mode
*/
int32
sys_usw_datapath_build_serdes_info(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 group, uint8 is_dyn, uint16 overclocking_speed)
{
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8  hss_idx          = 0;
    uint8  hss_id           = 0;
    uint8  lane_id          = 0;
    uint8  hssclk           = 0;
    uint8  rate_div         = 0;
    uint8  bit_width        = 0;
    uint8  core_div         = 0;
    uint8  intf_div         = 0;
    uint8  index            = 0;
    uint16 lport            = 0;
    uint8  port_num         = 0;
    uint8  chan_id          = 0;
    uint8  mac_id           = 0;
    uint8  need_add         = 0;
    uint8  speed_mode       = 0;
    uint8  slice_id         = 0;
    uint8  serdes_grp_flag  = 0;
    uint8  sgmac_idx        = 0;
    uint8  mii_idx          = 0;
    uint8  pcs_idx          = 0;
    uint8  internal_mac_idx = 0;

    if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        if ((mode == CTC_CHIP_SERDES_XXVG_MODE) || (mode == CTC_CHIP_SERDES_LG_MODE) ||
        (mode == CTC_CHIP_SERDES_CG_MODE))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

        }
    }
    else
    {
        if ((mode == CTC_CHIP_SERDES_USXGMII0_MODE) || (mode == CTC_CHIP_SERDES_USXGMII1_MODE) ||
        (mode == CTC_CHIP_SERDES_USXGMII2_MODE))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

        }
    }

    /*hss_idx used to index vector, hss_id is actual id*/
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_get_rate(lchip, mode,SYS_DATAPATH_HSS_TYPE_28G,
            &hssclk, &bit_width, &rate_div, &core_div, &intf_div, overclocking_speed));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_get_rate(lchip, mode,SYS_DATAPATH_HSS_TYPE_15G,
            &hssclk, &bit_width, &rate_div, &core_div, &intf_div, overclocking_speed));
    }

    need_add = 0;
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        /*first usefull serdes in hss*/
        p_hss_vec = (sys_datapath_hss_attribute_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_datapath_hss_attribute_t));
        if (NULL == p_hss_vec)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;

        }

        sal_memset(p_hss_vec, 0, sizeof(sys_datapath_hss_attribute_t));

        p_hss_vec->hss_id = hss_id;
        p_hss_vec->hss_type = SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)?1:0;

        /*using plla first, if serdes is cg must using pllb*/
        if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            if ((SYS_DATAPATH_HSSCLK_625 == hssclk) || (SYS_DATAPATH_HSSCLK_644DOT53125 == hssclk))
            {
                p_hss_vec->pllb_mode = hssclk;
                p_hss_vec->intf_div_b = intf_div;
                p_hss_vec->core_div_b[0] = core_div;
                p_hss_vec->pllb_ref_cnt++;
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
            }
            else
            {
                if (SYS_DATAPATH_HSSCLK_644DOT53125 == hssclk)
                {
                    p_hss_vec->pllb_mode = hssclk;
                    p_hss_vec->intf_div_b = intf_div;
                    p_hss_vec->core_div_b[0] = core_div;
                    p_hss_vec->pllb_ref_cnt++;
                    CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                    CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
                }
                else
                {
                    p_hss_vec->plla_mode = hssclk;
                    p_hss_vec->intf_div_a = intf_div;
                    p_hss_vec->core_div_a[0] = core_div;
                    p_hss_vec->plla_ref_cnt++;
                    CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
                    CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
                }
            }
        }
        else
        {
            if (SYS_DATAPATH_HSSCLK_644DOT53125 == hssclk)
            {
                p_hss_vec->pllb_mode = hssclk;
                p_hss_vec->intf_div_b = intf_div;
                p_hss_vec->core_div_b[0] = core_div;
                p_hss_vec->pllb_ref_cnt++;
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
            }
            else
            {
                p_hss_vec->plla_mode = hssclk;
                p_hss_vec->intf_div_a = intf_div;
                p_hss_vec->core_div_a[0] = core_div;
                p_hss_vec->plla_ref_cnt++;
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
            }
        }

        p_hss_vec->serdes_info[lane_id].mode = mode;
        p_hss_vec->serdes_info[lane_id].group = group;
        p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
        p_hss_vec->serdes_info[lane_id].pll_sel = (p_hss_vec->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
                    ? SYS_DATAPATH_PLL_SEL_PLLB: SYS_DATAPATH_PLL_SEL_PLLA;
        p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
        p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
        p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
        need_add = 1;
    }
    else
    {
        if (SYS_DATAPATH_HSSCLK_DISABLE == hssclk)
        {
            /*Only used for dynamic switch, swith Used Mode to Disable
               In thie case, clockTree and hssCfg do not change*/
            if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
            {
                p_hss_vec->pllb_ref_cnt--;
            }
            else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
            {
                p_hss_vec->plla_ref_cnt--;
            }

            p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_NONE;
            p_hss_vec->serdes_info[lane_id].mode = CTC_CHIP_SERDES_NONE_MODE;
            p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
        }
        else
        {
            if (p_hss_vec->plla_mode == hssclk)
            {
                /*one pll can only provide one intf div*/
                if ((p_hss_vec->intf_div_a != intf_div) && (p_hss_vec->intf_div_a)&&(intf_div))
                {
                    if ((CTC_CHIP_SERDES_SGMII_MODE == mode) || (CTC_CHIP_SERDES_QSGMII_MODE == mode))
                    {
                        if (SYS_DATAPATH_HSSCLK_DISABLE == p_hss_vec->pllb_mode)
                        {
                            /*need use pllb*/
                            p_hss_vec->pllb_mode = hssclk;
                            p_hss_vec->intf_div_b = intf_div;
                            p_hss_vec->core_div_b[0] = core_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
                        }
                        p_hss_vec->serdes_info[lane_id].mode = mode;
                        p_hss_vec->serdes_info[lane_id].group = group;
                        p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                        p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                        p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                        p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
                        if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                        {
                            p_hss_vec->pllb_ref_cnt++;
                        }
                        else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
                        {
                            p_hss_vec->plla_ref_cnt--;
                            p_hss_vec->pllb_ref_cnt++;
                        }
                        p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLB;
                    }
                    else
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide two intf divider, please check!! \n");
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                        return CTC_E_NOT_SUPPORT;
                    }
                }
                else
                {
                    if (!p_hss_vec->core_div_a[0])
                    {
                        p_hss_vec->core_div_a[0] = core_div;
                        CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
                    }
                    else if (p_hss_vec->core_div_a[0] != core_div)
                    {
                        if ((p_hss_vec->core_div_a[1] != core_div) && (p_hss_vec->core_div_a[1]))
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide more core divider, please check!! \n");
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                            return CTC_E_NOT_SUPPORT;
                        }

                        if (p_hss_vec->core_div_a[1] != core_div)
                        {
                            p_hss_vec->core_div_a[1] = core_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A1);
                        }
                    }

                    if ((mode == CTC_CHIP_SERDES_SGMII_MODE) || (mode == CTC_CHIP_SERDES_2DOT5G_MODE) ||
                        (mode == CTC_CHIP_SERDES_QSGMII_MODE))
                    {
                        if (p_hss_vec->intf_div_a != intf_div)
                        {
                            p_hss_vec->intf_div_a = intf_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
                        }
                    }

                    /*exist pll can provide serdes frequency*/
                    p_hss_vec->serdes_info[lane_id].mode = mode;
                    p_hss_vec->serdes_info[lane_id].group = group;
                    p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                    p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                    p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                    p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;

                    if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                    {
                        p_hss_vec->plla_ref_cnt++;
                    }
                    else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
                    {
                        /*NO need change*/
                    }
                    else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
                    {
                        p_hss_vec->plla_ref_cnt++;
                        p_hss_vec->pllb_ref_cnt--;
                    }
                    p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLA;
                }
            }
            else if (p_hss_vec->pllb_mode == hssclk)
            {
                /*one pll can only provide one intf div*/
                if ((p_hss_vec->intf_div_b != intf_div) && (p_hss_vec->intf_div_b)&&(intf_div))
                {
                    if ((CTC_CHIP_SERDES_SGMII_MODE == mode) || (CTC_CHIP_SERDES_QSGMII_MODE == mode))
                    {
                        if (p_hss_vec->plla_mode == SYS_DATAPATH_HSSCLK_DISABLE)
                        {
                            /*need using plla*/
                            p_hss_vec->plla_mode = hssclk;
                            p_hss_vec->intf_div_a = intf_div;
                            p_hss_vec->core_div_a[0] = core_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
                        }
                        p_hss_vec->serdes_info[lane_id].mode = mode;
                        p_hss_vec->serdes_info[lane_id].group = group;
                        p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                        p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                        p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                        p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
                        if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                        {
                            p_hss_vec->plla_ref_cnt++;
                        }
                        else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
                        {
                            p_hss_vec->pllb_ref_cnt--;
                            p_hss_vec->plla_ref_cnt++;
                        }
                        p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLA;
                    }
                    else
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide two intf divider, please check!! \n");
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                        return CTC_E_NOT_SUPPORT;
                    }
                }
                else
                {
                    if (!p_hss_vec->core_div_b[0])
                    {
                        p_hss_vec->core_div_b[0] = core_div;
                        CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
                    }
                    else if (p_hss_vec->core_div_b[0] != core_div)
                    {
                        if ((p_hss_vec->core_div_b[1] != core_div) && (p_hss_vec->core_div_b[1]))
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide more core divider, please check!! \n");
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                            return CTC_E_NOT_SUPPORT;
                        }

                        if (p_hss_vec->core_div_b[1] != core_div)
                        {
                            p_hss_vec->core_div_b[1] = core_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B1);
                        }
                    }
                    if ((mode == CTC_CHIP_SERDES_SGMII_MODE) || (mode == CTC_CHIP_SERDES_2DOT5G_MODE) ||
                        (mode == CTC_CHIP_SERDES_QSGMII_MODE))
                    {
                        if (p_hss_vec->intf_div_b != intf_div)
                        {
                            p_hss_vec->intf_div_b = intf_div;
                            CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                        }
                    }

                    /*exist pll can provide serdes frequency*/
                    p_hss_vec->serdes_info[lane_id].mode = mode;
                    p_hss_vec->serdes_info[lane_id].group = group;
                    p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                    p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                    p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                    p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
                    if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                    {
                        p_hss_vec->pllb_ref_cnt++;
                    }
                    else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
                    {
                        /*NO need change*/
                    }
                    else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
                    {
                        p_hss_vec->plla_ref_cnt--;
                        p_hss_vec->pllb_ref_cnt++;
                    }
                    p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLB;
                }
            }
            else if ((p_hss_vec->plla_mode == SYS_DATAPATH_HSSCLK_DISABLE) &&
                        (((mode != CTC_CHIP_SERDES_XAUI_MODE)
                        && (mode != CTC_CHIP_SERDES_DXAUI_MODE)
                        && (mode != CTC_CHIP_SERDES_XXVG_MODE)
                        && (mode != CTC_CHIP_SERDES_LG_MODE)
                        && (mode != CTC_CHIP_SERDES_CG_MODE)
                        && (mode != CTC_CHIP_SERDES_2DOT5G_MODE)
                        && SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)) ||
                        (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)
                        && (overclocking_speed != CTC_CHIP_SERDES_OCS_MODE_12_58G))))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_hss_vec->plla_mode == SYS_DATAPATH_HSSCLK_DISABLE\n");
                /*need using plla*/
                p_hss_vec->plla_mode = hssclk;
                p_hss_vec->intf_div_a = intf_div;
                p_hss_vec->core_div_a[0] = core_div;
                p_hss_vec->serdes_info[lane_id].mode = mode;
                p_hss_vec->serdes_info[lane_id].group = group;
                p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;

                if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                {
                    p_hss_vec->plla_ref_cnt++;
                }
                else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
                {
                    p_hss_vec->plla_ref_cnt++;
                    p_hss_vec->pllb_ref_cnt--;
                }

                p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLA;
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_A);
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_A0);
            }
            else if (p_hss_vec->pllb_mode == SYS_DATAPATH_HSSCLK_DISABLE)
            {
                /*need using pllb*/
                p_hss_vec->pllb_mode = hssclk;
                p_hss_vec->intf_div_b = intf_div;
                p_hss_vec->core_div_b[0] = core_div;
                p_hss_vec->serdes_info[lane_id].mode = mode;
                p_hss_vec->serdes_info[lane_id].group = group;
                p_hss_vec->serdes_info[lane_id].overclocking_speed = overclocking_speed;
                p_hss_vec->serdes_info[lane_id].rate_div = rate_div;
                p_hss_vec->serdes_info[lane_id].bit_width = bit_width;
                p_hss_vec->serdes_info[lane_id].is_dyn = is_dyn;
                if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_NONE)
                {
                    p_hss_vec->pllb_ref_cnt++;
                }
                else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
                {
                    p_hss_vec->plla_ref_cnt--;
                    p_hss_vec->pllb_ref_cnt++;
                }
                p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_PLLB;
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_INTF_B);
                CTC_BIT_SET(p_hss_vec->clktree_bitmap, SYS_DATAPATH_CLKTREE_CORE_B0);
            }
            else
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;

            }
        }
    }

    if (p_hss_vec->plla_ref_cnt == 0)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Need Disable Plla!! \n");
        /*Disable plla hssclk*/
        p_hss_vec->plla_mode = SYS_DATAPATH_HSSCLK_DISABLE;
        p_hss_vec->core_div_a[0] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->core_div_a[1] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->intf_div_a = SYS_DATAPATH_CLKTREE_DIV_USELESS;
    }

    if (p_hss_vec->pllb_ref_cnt == 0)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Need Disable Pllb!! \n");
        /*Disable pllb hssclk*/
        p_hss_vec->pllb_mode = SYS_DATAPATH_HSSCLK_DISABLE;
        p_hss_vec->core_div_b[0] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->core_div_b[1] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->intf_div_b = SYS_DATAPATH_CLKTREE_DIV_USELESS;
    }

    if (p_hss_vec->serdes_info[0].group != p_hss_vec->serdes_info[1].group)
    {
        serdes_grp_flag = 1;
    }

    CTC_ERROR_RETURN(_sys_usw_datapath_get_serdes_mapping(lchip, serdes_id, mode, serdes_grp_flag, &lport,
        &mac_id, &chan_id, &speed_mode, &port_num));
    p_hss_vec->serdes_info[lane_id].lport = lport;
    p_hss_vec->serdes_info[lane_id].port_num = port_num;

    for (index = 0; index < port_num; index++)
    {
        CTC_ERROR_RETURN(_sys_usw_get_mac_mii_pcs_index(lchip, mac_id+index, serdes_id, mode, &sgmac_idx,
                                                          &mii_idx, &pcs_idx, &internal_mac_idx));
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport+index, slice_id);
        if (NULL == port_attr)
        {
            if (need_add)
            {
                mem_free(p_hss_vec);
            }
            return CTC_E_INVALID_PARAM;
        }
        port_attr->mac_id           = mac_id+index;
        port_attr->port_type        = (mode)? SYS_DMPS_NETWORK_PORT : SYS_DMPS_RSV_PORT;
        port_attr->chan_id          = (mode)? (chan_id+index) : 0xff;
        port_attr->speed_mode       = speed_mode;
        port_attr->slice_id         = slice_id;
        port_attr->pcs_mode         = mode;
        port_attr->serdes_id        = serdes_id;
        port_attr->serdes_num       = 1;
        port_attr->is_serdes_dyn    = is_dyn;
        port_attr->an_fec           = (1 << CTC_PORT_FEC_TYPE_RS)|(1 << CTC_PORT_FEC_TYPE_BASER);
        port_attr->flag             = serdes_grp_flag;
        port_attr->sgmac_idx        = sgmac_idx;
        port_attr->mii_idx          = mii_idx;
        port_attr->pcs_idx          = pcs_idx;
        port_attr->internal_mac_idx = internal_mac_idx;
        SYS_DATAPATH_GET_IFTYPE_WITH_MODE(mode, port_attr->interface_type);

        if (SYS_DATAPATH_NEED_EXTERN(mode) && is_dyn)
        {
            if (((mac_id+index)%4) == 0)
            {
                if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) && mode == CTC_CHIP_SERDES_LG_MODE)
                {
                    port_attr->serdes_num = 2;
                }
                else
                {
                    port_attr->serdes_num = 4;
                }
            }
            else if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) && (((mac_id+index)%4) == 2) &&
                mode == CTC_CHIP_SERDES_LG_MODE)
            {
                port_attr->serdes_num = 2;
            }
        }
    }

    if (need_add)
    {
        ctc_vector_add(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx, (void*)p_hss_vec);
    }

    for (index = 0; index < port_num; index++)
    {
        _sys_usw_datapath_store_mcu_data_mem(lchip, lport+index, 0);
    }

    return CTC_E_NONE;
}

/*
   set serdes link enable or disable,dir:0-rx, 1-tx, 2-both
*/
STATIC int32
_sys_usw_datapath_set_link_enable(uint8 lchip, uint8 serdes_id, uint8 enable, uint8 dir)
{
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
        if (enable)
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*enable tx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*enable rx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }
        }
        else
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*disable tx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*disable rx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }
        }
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];

        if (enable)
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*enable tx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*enable rx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }
        }
        else
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*disable tx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*disable rx link enable register*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_EN_BIT);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }
        }
    }

    return CTC_E_NONE;
}

/*
   set serdes link dfe reset or release
*/
int32
sys_usw_datapath_set_dfe_reset(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
        if (enable)
        {
            /*reset rx dfe reset*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            value |= (1 << 0);
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
        else
        {
            /*release rx dfe reset*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            value &= ~(1 << 0);
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];

        if (enable)
        {
            /*reset rx dfe reset*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value |= (1 << 0);
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
        else
        {
            /*release rx dfe reset*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value &= ~(1 << 0);
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
    }

    return CTC_E_NONE;
}

/*
   set serdes link dfe enable or disable
*/
/*jqiu modify for cl73 auto*/
int32
sys_usw_datapath_set_dfe_en(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
        if (enable)
        {
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            value |= (0x2030);
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
        else
        {
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            value &= ~(0x30);
            value |= (0x2000);
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];

        if (enable)
        {
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value |= (0x2030);
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
        else
        {
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value &= ~(0x30);
            value |= (0x2000);
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_serdes_link_stuck(uint8 lchip, uint8 serdes_id, uint32 *p_link_stuck)
{
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint16 value = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;
    uint16 C105 = 0;
    uint16 C108 = 0;
    uint16 C115 = 0;
    uint16 C116 = 0;
    uint16 C125 = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (p_link_stuck)
    {
        *p_link_stuck = 0;
    }

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
    }
    else
    {
        /* Only check HSS28G */
        return CTC_E_NONE;
    }

    /* C105: peak: rx reg 0x0b[13:9] */
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0xb);
    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
    C105 = (value >> 9) & 0x1f;

    /* C108: pkinit: rx reg 0x1b[4:0]*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1b);
    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
    C108 = value & 0x1f;

    /* C115: OES: rx reg 0x1d[10:6] */
    /* C116: OLS: rx reg 0x1d[15:11] */
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1d);
    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
    C115 = (value >> 6) & 0x1f;
    C116 = (value >> 11) & 0x1f;

    /* C125: OAE: rx reg 0x1c[3:0] */
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1c);
    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
    C125 = value & 0xf;

    /* if (($C105 == $C108) & ($C115 == 0) & ($C116 == 0) & ($C125 == 0)) , link stuck exist*/
    if ((C105 == C108) && (0 == C115) && (0 == C116) && (0 == C125) && p_link_stuck)
    {
        *p_link_stuck = 1;
    }

    return CTC_E_NONE;
}

/*
   set serdes link reset or release,dir:0-rx, 1-tx, 2-both
*/
int32
sys_usw_datapath_set_link_reset(uint8 lchip, uint8 serdes_id, uint8 enable, uint8 dir)
{
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
        if (enable)
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*reset tx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*reset rx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }
        }
        else
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*release tx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*release rx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
            }

        }
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];

        if (enable)
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*reset tx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*reset rx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value |= (1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }
        }
        else
        {
            if ((SYS_DATAPATH_SERDES_DIR_TX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*release tx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }

            if ((SYS_DATAPATH_SERDES_DIR_RX == dir) || (SYS_DATAPATH_SERDES_DIR_BOTH == dir))
            {
                /*release rx link reset*/
                address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
                CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
                value &= ~(1 << SYS_DATAPATH_HSS_LINK_RST);
                CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
            }
        }
    }

    return CTC_E_NONE;
}

/*
    Set serdes mode interface, used for init and dynamic switch
    Notice:Before using the interface must using sys_usw_datapath_build_serdes_info to build serdes info
*/
int32
sys_usw_datapath_set_hss_internal(uint8 lchip, uint8 serdes_id, uint8 mode)
{
    uint8 hss_idx = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint16 mask_value = 0;
    uint32 tb_id = 0;
    uint32 cmd = 0;
    uint32 data = 0;

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];
    }

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    hss_id = p_hss->hss_id;
    p_serdes = &p_hss->serdes_info[lane_id];

    if (p_serdes->mode != mode)
    {
        /*serdes info is not ready*/
        return CTC_E_INVALID_PARAM;
    }

    if (p_serdes->mode == CTC_CHIP_SERDES_NONE_MODE)
    {
        sys_usw_datapath_set_link_reset(lchip, serdes_id, TRUE, SYS_DATAPATH_SERDES_DIR_BOTH);
        _sys_usw_datapath_set_link_enable(lchip, serdes_id, FALSE, SYS_DATAPATH_SERDES_DIR_BOTH);
        return CTC_E_NONE;
    }

    /*Set link enable*/
    _sys_usw_datapath_set_link_enable(lchip, serdes_id, TRUE, SYS_DATAPATH_SERDES_DIR_BOTH);
    sys_usw_datapath_set_link_reset(lchip, serdes_id, TRUE, SYS_DATAPATH_SERDES_DIR_BOTH);
    sal_task_sleep(1);

    sys_usw_datapath_set_link_reset(lchip, serdes_id, FALSE, SYS_DATAPATH_SERDES_DIR_BOTH);
    sal_task_sleep(1);

    /*Cfg serdes internal register*/
    if (p_hss->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
    {
        /* update the PLL charge pump control register
           cfg PLL register address 0xA = 0x7; */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0xa);
        value = 0x7;
        CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));

        /*cfg tx register*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
        CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
        value &= ~(uint16)SYS_DATAPATH_HSS_REG_MASK;
        mask_value = 0;
        CTC_ERROR_RETURN(_sys_usw_datapath_encode_serdes_cfg(lchip, p_hss->hss_type, 0, &mask_value, p_serdes));
        value |= mask_value;
        CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));

        /*enable tx*/
        tb_id = HsCfg0_t + p_hss->hss_id;
        data = 1;
        cmd = DRV_IOW(tb_id, HsCfg0_cfgHss15gHssTxAOe_f+lane_id*2);
        DRV_IOCTL(lchip, 0, cmd, &data);

        /*cfg rx register*/
        mask_value = 0;
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
        CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
        value &= ~(uint16)SYS_DATAPATH_HSS_REG_MASK;
        CTC_ERROR_RETURN(_sys_usw_datapath_encode_serdes_cfg(lchip, p_hss->hss_type, 1, &mask_value, p_serdes));
        value |= mask_value;
        CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));

        if ((CTC_CHIP_SERDES_DXAUI_MODE == p_serdes->mode)
            || (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
        {
            value = 0xfffb;
        }
        else
        {
            value = 0xffff;
        }
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1f);
        CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
    }
    else
    {
        /*xaui, dxaui,cg must using pllb*/
        if ((p_serdes->pll_sel == SYS_DATAPATH_PLL_SEL_PLLA) && ((p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE) ||
                 (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_2DOT5G_MODE)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hss28G Must using pllb for the mode!! \n");
            return CTC_E_INVALID_PARAM;
        }

        /*cfg tx register*/
        mask_value = 0;
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        value &= ~(uint16)SYS_DATAPATH_HSS_REG_MASK;
        CTC_ERROR_RETURN(_sys_usw_datapath_encode_serdes_cfg(lchip, p_hss->hss_type, 0, &mask_value, p_serdes));
        value |= mask_value;
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

        /*enable tx*/
        tb_id = CsCfg0_t + p_hss->hss_id;
        data = 1;
        cmd = DRV_IOW(tb_id, CsCfg0_cfgHss28gHssTxAOe_f+lane_id*2);
        DRV_IOCTL(lchip, 0, cmd, &data);

        /* Turn off dynamic LTE */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x32);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        value &= ~(0xc0);
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

        /* Turn off Dynamic Power Management */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0xe);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        value &= ~(0xc0);
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

        /*cfg rx register*/
        mask_value = 0;
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        value &= ~(uint16)SYS_DATAPATH_HSS_REG_MASK;
        CTC_ERROR_RETURN(_sys_usw_datapath_encode_serdes_cfg(lchip, p_hss->hss_type, 1, &mask_value, p_serdes));
        value |= mask_value;
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

        /*cfg rx register */
        if (p_serdes->mode == CTC_CHIP_SERDES_SGMII_MODE)
        {
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x6);
            value = 0x14;
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x7);
            value = 0x14;
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }

        if ((CTC_CHIP_SERDES_DXAUI_MODE == p_serdes->mode)
            || (CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode))
        {
            value = 0xfffb;
        }
        else
        {
            value = 0xffff;
        }
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1f);
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

        /* MODE8B10B cfg */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1b);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        if ((CTC_CHIP_SERDES_SGMII_MODE == mode)
            || (CTC_CHIP_SERDES_2DOT5G_MODE == mode)
            || (CTC_CHIP_SERDES_XAUI_MODE == mode)
            || (CTC_CHIP_SERDES_DXAUI_MODE == mode))
        {
            value |= (1 << 7);
        }
        else
        {
            value &= ~(1 << 7);
        }
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
    }

    return CTC_E_NONE;
}

#define __DATAPATH__

int32
_sys_usw_datapath_get_gcd(uint16* array, uint8 count, uint16* gcd)
{
    uint8 index1 = 0;
    uint8 index2 = 0;
    uint8 zero_ct = 0;
    uint16 tmp = 0;
    uint16 divisor = 0;
    uint16 remain = 0;
    uint16 *pos = NULL;
    uint16 tmp_array[128] = {0};

    *gcd = 1;

    if (count == 1)
    {
        if (array[0])
        {
            *gcd = array[0];
        }

        return CTC_E_NONE;
    }

    if (count > 128)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memcpy(tmp_array, array, count * sizeof(uint16));

    for (index1 = 0; index1 < count - 1; index1++)
    {
        for (index2 = 0; index2 < count - 1 - index1; index2++)
        {
            if (tmp_array[index2] > tmp_array[index2+1])
            {
                tmp = tmp_array[index2+1];
                tmp_array[index2+1] = tmp_array[index2];
                tmp_array[index2] = tmp;
            }
        }
    }

    for (index1 = 0; index1 < count; index1++)
    {
        if (!tmp_array[index1])
        {
            zero_ct++;
        }
    }

    pos = tmp_array + zero_ct;
    count -= zero_ct;
    if (!count)
    {
        *gcd = 1;

        return CTC_E_NONE;
    }

    for (index1 = 1; index1 < count; index1++)
    {
        divisor = pos[index1-1];

        do
        {
            remain = pos[index1] % divisor;
            pos[index1] = divisor;
            divisor = remain;
        }while(remain);

        pos[index1-1] = pos[index1];
    }

    *gcd = pos[count-1];

    return CTC_E_NONE;
}

/*
  Using for datapath init and dynamic, for network port
*/
STATIC int32
_sys_usw_datapath_set_netrx_buf_mgr(uint8 lchip, uint8 slice_id)
{
    uint8 lport = 0;
    uint8 txqm = 0;
    uint8 lowthrd = 0;
    uint32 tmp_val32 = 0;
    uint8 speed = 0;
    uint32 cmd = 0;
    DsNetRxBufAdmissionCtl_m buf_ctl;
    NetRxBufManagement_m rx_buf;
    sys_datapath_lport_attr_t* port_attr = NULL;

    cmd = DRV_IOR((NetRxBufManagement_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &rx_buf);

    for (txqm = 0; txqm < 4; txqm++)
    {
        lowthrd = 0;
        for (lport = txqm*16; lport < (txqm + 1)*16; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
            {
                continue;
            }

            SYS_DATAPATH_MODE_TO_SPEED(port_attr->speed_mode, speed);
             /*calc by base 10G, value = 8*/
             if (port_attr->speed_mode <= CTC_PORT_SPEED_2G5)
            {
                tmp_val32 = speed;
            }
            else
            {
                tmp_val32 = 8 * speed / 10;
            }

            lowthrd += tmp_val32;
            if (tmp_val32 > 8)
            {
                lowthrd -= 2;
            }

            cmd = DRV_IOR((DsNetRxBufAdmissionCtl_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->mac_id, cmd, &buf_ctl);
            DRV_IOW_FIELD(lchip, DsNetRxBufAdmissionCtl_t, DsNetRxBufAdmissionCtl_cfgGuaranteeBufferNum_f, &tmp_val32, &buf_ctl);
            cmd = DRV_IOW((DsNetRxBufAdmissionCtl_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->mac_id, cmd, &buf_ctl);
        }
        lowthrd += 2;

        tmp_val32 = lowthrd;
        DRV_IOW_FIELD(lchip, NetRxBufManagement_t, (NetRxBufManagement_cfgLowThrd0_f+txqm), &tmp_val32, &rx_buf);
        cmd = DRV_IOW((NetRxBufManagement_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &rx_buf);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_set_nettx_portmode(uint8 lchip, uint8 chan_id)
{
    uint8 slice_id = 0;
    uint16 lport = 0;
    uint32 tmp_val32 = 0;
    uint32 cmd = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetTxPortStaticInfo_m static_info;

    sal_memset(&static_info, 0, sizeof(NetTxPortStaticInfo_m));

    lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        return CTC_E_NONE;
    }

    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_DMPS_NETWORK_PORT == port_attr->port_type)
    {
        switch(port_attr->speed_mode)
        {
            case CTC_PORT_SPEED_5G:
                tmp_val32 = 0x01;
                break;
            case CTC_PORT_SPEED_10G:
            case CTC_PORT_SPEED_20G:
                tmp_val32 = 0x02;
                break;
            case CTC_PORT_SPEED_25G:
                tmp_val32 = 0x03;
                break;
            case CTC_PORT_SPEED_40G:
                tmp_val32 = 0x04;
                break;
            case CTC_PORT_SPEED_50G:
                tmp_val32 = 0x05;
                break;
            case CTC_PORT_SPEED_100G:
                tmp_val32 = 0x06;
                break;
            case CTC_PORT_SPEED_10M:
            case CTC_PORT_SPEED_100M:
            case CTC_PORT_SPEED_1G:
            case CTC_PORT_SPEED_2G5:
            default:
                tmp_val32 = 0x00;
                break;
        }
        cmd = DRV_IOR((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &static_info);
        DRV_IOW_FIELD(lchip, (NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f, &tmp_val32, &static_info);
        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &static_info);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_init_nettx_buf(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint32 credit_thrd = 0;
    uint32 nettx_bufsz = 0;
    uint16 s_pointer = 0;
    uint16 e_poniter = 0;
    uint32 tmp_val32 = 0;
    uint8 mac_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetTxPortStaticInfo_m static_info;
    NetTxPortDynamicInfo_m dyn_info;
    EpeScheduleToNet0CreditConfigRa_m epe_credit;
    uint8 step = 0;

    sal_memset(&static_info, 0, sizeof(NetTxPortStaticInfo_m));
    sal_memset(&dyn_info, 0, sizeof(NetTxPortDynamicInfo_m));
    sal_memset(&epe_credit, 0, sizeof(EpeScheduleToNet0CreditConfigRa_m));

    if (s_pointer >= NETTX_MEM_MAX_DEPTH_PER_TXQM)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"nettx mac_id:%d, s-pointer:%d \n", mac_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
    {
        if (mac_id % 16 == 0)
        {
            s_pointer = 0;
        }
        credit_thrd = nettx_bufsz = g_nettx_cn[mac_id];

        cmd = DRV_IOR((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &static_info);

        e_poniter = nettx_bufsz ? (s_pointer+nettx_bufsz-1) : s_pointer;
        if (e_poniter >= NETTX_MEM_MAX_DEPTH_PER_TXQM)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"nettx mac_id:%d, e-pointer:%d \n", mac_id, e_poniter);
            return CTC_E_INVALID_PARAM;
        }
        tmp_val32 = s_pointer;
        DRV_IOW_FIELD(lchip, (NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_startPktPtr_f, &tmp_val32, &static_info);
        tmp_val32 = e_poniter;
        DRV_IOW_FIELD(lchip, (NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_endPktPtr_f, &tmp_val32, &static_info);

        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &static_info);
        tmp_val32 = s_pointer;
        DRV_IOW_FIELD(lchip, (NetTxPortDynamicInfo_t+slice_id), NetTxPortDynamicInfo_readPktPtr_f, &tmp_val32, &dyn_info);
        DRV_IOW_FIELD(lchip, (NetTxPortDynamicInfo_t+slice_id), NetTxPortDynamicInfo_writePktPtr_f, &tmp_val32, &dyn_info);
        cmd = DRV_IOW((NetTxPortDynamicInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &dyn_info);

        s_pointer = e_poniter + 1;

        step = EpeScheduleToNet1CreditConfigRa_t - EpeScheduleToNet0CreditConfigRa_t;
        cmd = DRV_IOR((EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*step+slice_id*8), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id%16, cmd, &epe_credit);

        tmp_val32 = credit_thrd;
        DRV_IOW_FIELD(lchip, (EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*step+slice_id*8),
            EpeScheduleToNet0CreditConfigRa_cfgCreditValue_f, &tmp_val32, &epe_credit);
        cmd = DRV_IOW((EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*step+slice_id*8), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id%16, cmd, &epe_credit);

        lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, mac_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        _sys_usw_datapath_set_nettx_portmode(lchip, port_attr->chan_id);
    }

    return CTC_E_NONE;
}

/*
   Using for datapath init and dynamic, Notice this interface only used to set network port channel
*/
STATIC int32
_sys_usw_datapath_set_qmgr_credit(uint8 lchip, uint8 chan_id, uint8 slice_id, uint8 is_dyn)
{
    uint32 cmd = 0;
    uint8 step = 0;
    uint32 tmp_val32 = 0;
    DsQMgrChanCredit_m ra_credit;

    sal_memset(&ra_credit, 0, sizeof(DsQMgrChanCredit_m));

    if (is_dyn)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR((DsQMgrChanCredit_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &ra_credit);

    step = g_bufretrv_alloc[chan_id];
    tmp_val32 = step;
    DRV_IOW_FIELD(lchip, DsQMgrChanCredit_t, DsQMgrChanCredit_credit_f, &tmp_val32, &ra_credit);
    cmd = DRV_IOW((DsQMgrChanCredit_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &ra_credit);

    return CTC_E_NONE;
}

/*
   Using for datapath init and dynamic, Config BufRetrvPktConfigMem and BufRetrvPktStatusMem
   Usefull for network channel and internal channel
*/
STATIC int32
_sys_usw_datapath_set_bufretrv_pkt_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16* p_end)
{
    uint32 cmd = 0;
    uint8 step = 0;
    uint16 e_pointer = 0;
    uint32 tmp_val32 = 0;
    BufRetrvPktConfigMem_m bufretrv_cfg;
    BufRetrvPktStatusMem_m bufretrv_status;

    sal_memset(&bufretrv_cfg, 0 ,sizeof(BufRetrvPktConfigMem_m));
    sal_memset(&bufretrv_status, 0 ,sizeof(BufRetrvPktStatusMem_m));

    if (s_pointer >= BUFRETRV_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"bufretrv pkt mem chan_id: %d, s-pointer: %d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    step = g_bufretrv_alloc[chan_id];

     /* cfg below is used for init */
    if(NULL != p_end)
    {
        e_pointer = step ? (s_pointer+step-1) : s_pointer;
        if (e_pointer >= BUFRETRV_MEM_MAX_DEPTH)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"bufretrv pkt mem chan_id: %d, e-pointer: %d \n", chan_id, e_pointer);
            return CTC_E_INVALID_PARAM;
        }

        tmp_val32 = s_pointer;
        DRV_IOW_FIELD(lchip, BufRetrvPktConfigMem_t, BufRetrvPktConfigMem_startPtr_f, &tmp_val32, &bufretrv_cfg);
        tmp_val32 = e_pointer;
        DRV_IOW_FIELD(lchip, BufRetrvPktConfigMem_t, BufRetrvPktConfigMem_endPtr_f, &tmp_val32, &bufretrv_cfg);
        cmd = DRV_IOW((BufRetrvPktConfigMem_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &bufretrv_cfg);

        tmp_val32 = 0;
        DRV_IOW_FIELD(lchip, BufRetrvPktStatusMem_t, BufRetrvPktStatusMem_aFull_f, &tmp_val32, &bufretrv_status);
        tmp_val32 = s_pointer;
        DRV_IOW_FIELD(lchip, BufRetrvPktStatusMem_t, BufRetrvPktStatusMem_headPtr_f, &tmp_val32, &bufretrv_status);
        DRV_IOW_FIELD(lchip, BufRetrvPktStatusMem_t, BufRetrvPktStatusMem_tailPtr_f, &tmp_val32, &bufretrv_status);
        cmd = DRV_IOW((BufRetrvPktStatusMem_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &bufretrv_status);

        *p_end = step ? (e_pointer + 1) : e_pointer;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_bufretrv_credit_config(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tbl_id  = 0;
    uint32 fld_id  = 0;
    uint32 i = 0;
    BufRetrvEpeScheduleCreditConfig_m credit_cfg;

    tbl_id = BufRetrvEpeScheduleCreditConfig_t;
    cmd = DRV_IOR((BufRetrvEpeScheduleCreditConfig_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &credit_cfg);

    for (i = 0; i < sizeof(g_bufsz_step)/sizeof(g_bufsz_step[0]); i++)
    {
        if ((g_bufsz_step[i].br_credit_cfg >= 4) && (g_bufsz_step[i].br_credit_cfg <= 7))
        {
            fld_id = BufRetrvEpeScheduleCreditConfig_bodyCreditConfig0_f + g_bufsz_step[i].br_credit_cfg;
            value = g_bufsz_step[i].body_buf_num;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &credit_cfg);
            fld_id = BufRetrvEpeScheduleCreditConfig_sopCreditConfig0_f + g_bufsz_step[i].br_credit_cfg;
            if (g_bufsz_step[i].sop_buf_num < 3)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] BufRetrvEpeSchedule sop Credit is too low \n");
                return CTC_E_HW_FAIL;
            }
            else if (g_bufsz_step[i].sop_buf_num == 3)
            {
                value = g_bufsz_step[i].sop_buf_num + 1;
            }
            else
            {
                value = g_bufsz_step[i].sop_buf_num;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &credit_cfg);
        }
    }

    cmd = DRV_IOW((BufRetrvEpeScheduleCreditConfig_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &credit_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_get_bufsz_step(uint8 lchip, uint8 chan_id, sys_datapath_bufsz_step_t *step)
{
    uint8 slice_id = 0;
    uint32 flag = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        return CTC_E_NONE;
    }

    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (SYS_DMPS_NETWORK_PORT != port_attr->port_type) /* Misc Channel */
    {
        if ((MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == chan_id))
        {
            flag = SPEED_MODE_MISC_100;
        }
        else if ((MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3) == chan_id) ||
            (MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE) == chan_id))
        {
            flag = SPEED_MODE_MISC_10;
        }
        else
        {
            flag = SPEED_MODE_MISC_10;
        }
    }
    else    /* network channel */
    {
        switch (port_attr->speed_mode)
        {
        case CTC_PORT_SPEED_100G:
            flag = SPEED_MODE_100;
            break;
        case CTC_PORT_SPEED_50G:
            flag = SPEED_MODE_50;
            break;
        case CTC_PORT_SPEED_40G:
            flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_100 : SPEED_MODE_40);
            break;
        case CTC_PORT_SPEED_25G:
            flag = SPEED_MODE_25;
            break;
        case CTC_PORT_SPEED_10G:
        case CTC_PORT_SPEED_20G:
            if ((1 == SDK_WORK_PLATFORM) && p_usw_datapath_master[lchip]->wlan_enable)
            {
                flag = SPEED_MODE_10;   /* UML env */
            }
            else
            {
                flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_25 : SPEED_MODE_10);
            }
            break;
        case CTC_PORT_SPEED_5G:
            flag = SPEED_MODE_5;
            break;
        case CTC_PORT_SPEED_10M:
        case CTC_PORT_SPEED_100M:
        case CTC_PORT_SPEED_1G:
        case CTC_PORT_SPEED_2G5:
            if (port_attr->is_serdes_dyn)
            {
                flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_25 : SPEED_MODE_10);
            }
            else
            {
                flag = SPEED_MODE_0_5;
            }
            break;
        default:
            flag = SPEED_MODE_INV;
            break;
        }
    }
    step->body_buf_num  = g_bufsz_step[flag].body_buf_num;
    step->sop_buf_num   = g_bufsz_step[flag].sop_buf_num;
    step->br_credit_cfg = g_bufsz_step[flag].br_credit_cfg;

    /* #1, specially for Q/U */
    if ((CTC_CHIP_SERDES_QSGMII_MODE == port_attr->pcs_mode)
        || (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr->pcs_mode))
    {
        step->body_buf_num = g_bufsz_step[SPEED_MODE_0_5].body_buf_num;
        step->sop_buf_num  = g_bufsz_step[SPEED_MODE_0_5].sop_buf_num;
        step->br_credit_cfg = g_bufsz_step[SPEED_MODE_0_5].br_credit_cfg;
    }

    /* #2, specially for Eloop sop */
    if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == chan_id)
    {
        step->sop_buf_num = 50;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_bufretrv_credit(uint8 lchip, uint8 slice_id, uint8 chan_id)
{
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    BufRetrvCreditConfigMem_m credit_cfg;
    sys_datapath_bufsz_step_t step;

    sal_memset(&credit_cfg, 0, sizeof(BufRetrvCreditConfigMem_m));
    sal_memset(&step, 0, sizeof(sys_datapath_bufsz_step_t));

    _sys_usw_datapath_get_bufsz_step(lchip, chan_id, &step);
    if (-1 == step.br_credit_cfg)
    {
        return CTC_E_NONE;
    }

    tmp_val32 = step.br_credit_cfg;
    DRV_IOW_FIELD(lchip, BufRetrvCreditConfigMem_t, BufRetrvCreditConfigMem_config_f, &tmp_val32, &credit_cfg);
    cmd = DRV_IOW((BufRetrvCreditConfigMem_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &credit_cfg);

    return CTC_E_NONE;
}

uint32
_sys_usw_datapath_cal_interval(uint32 speed, uint32 num, uint32 mod, uint8 extra)
{
    uint32 cnt = 0;
    /* 1G: 187; 10G: 18.7; 20G: 9.35; 25G: 7.486;40G: 4.67;50G: 3.7; 100G: 1.87 (average)*/
    /* 1G: 187; 10G: 23;   20G: 11;   25G: 9;    40G: 5;   50G: 4;   100G: 2    (max) */

    if (speed >= 100)
    {
        cnt = (num%7 == 1) ? 1 : 2;
    }
    else if (speed >= 50)
    {
        if (extra)
        {
            cnt = 4;
        }
        else
        {
            switch (num % 3)
            {
            case 1:
                cnt = 3;
                break;
            case 2:
                cnt = 4;
                break;
            case 0:
            default:
                cnt = 4;
                break;
            }
        }
    }
    else if (speed >= 40)
    {
        if (mod)
            cnt = 5;
        else if (extra)
            cnt = 5;
        else
        {
            cnt = (num%2 == 1) ? 4 : 5;
        }
    }
    else if (speed >= 25)
    {
        if (mod)
        {
            cnt = 8;
        }
        else if (extra)
        {
            cnt = 8;
        }
        else
        {
            switch (num % 7)
            {
            case 0:
                cnt = 8;
                break;
            case 1:
                cnt = 7;
                break;
            case 2:
                cnt = 7;
                break;
            case 3:
                cnt = 8;
                break;
            case 4:
                cnt = 7;
                break;
            case 5:
                cnt = 8;
                break;
            default:
                cnt = 7;
                break;
            }
        }
    }
	else if (speed >= 20)
	{
		if (mod)
		{
			cnt = 9;
		}
		else if (extra)
		{
			cnt = 9;
		}
		else
		{
			switch (num % 3)
            {
            case 0:
                cnt = 10;
                break;
            case 1:
                cnt = 9;
                break;
            default:
                cnt = 9;
                break;
            }
		}
	}
    else if (speed >= 10)
    {
        if (mod)
		{
            cnt = 22;
		}
        else
		{
            cnt = 18;
		}
    }
    else if (speed >= 5)
    {
        cnt = 37;
    }
    else if (speed >= 1)
    {
        if (mod)
		{
            cnt = 75;
		}
        else
		{
            cnt = 74;
		}
    }
    else
    {
        cnt = 0;
    }

    return cnt;
}

STATIC int32
_sys_usw_datapath_get_oversub_status(uint8 lchip,
                                     uint8 slice_id,
                                     uint32 *is_oversub,
                                     uint32 *total_speed,
                                     uint32 *is_spec_ff)
{
    uint16 lport = 0;
    uint8 serdes_id = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 speed = 0;
    uint32 total = 0;
    uint8 xg_num = 0;
    uint8 xxvg_num = 0;
    uint8 hss28g_serdes_dyn_num = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
    {
        port_attr = &p_usw_datapath_master[lchip]->port_attr[slice_id][lport];
        if ((!port_attr) || (SYS_DMPS_RSV_PORT == port_attr->port_type))
        {
            speed = 0;
        }
        else
        {
            SYS_DATAPATH_MODE_TO_SPEED(port_attr->speed_mode, speed);
        }
        total += speed;
        if (10 == speed)
        {
            xg_num++;
        }
        else if (25 == speed)
        {
            xxvg_num++;
        }
    }

    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }
        if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)
            && (p_hss_vec->serdes_info[lane_id].is_dyn))
        {
            hss28g_serdes_dyn_num++;
        }
    }

    if ((hss28g_serdes_dyn_num >= 12) && is_oversub)
    {
        *is_oversub = 1;
    }
    if (total_speed)
    {
        *total_speed = total;
    }
    if ((24 == xg_num) && (16 == xxvg_num) && is_spec_ff)
    {
        *is_spec_ff = 1;
    }

    return CTC_E_NONE;
}


int32
sys_usw_datapath_set_calendar_cfg(uint8 lchip, uint8 slice_id, uint32 txqm_id)
{
    uint32 cmd = 0;
    uint8 back = 0;
    uint32 tmp_val32 = 0;
    NetTxCal_m cal_en;
    NetTxCalCtl_m cal_ctl;
    NetTxMiscCtl_m misc_ctl;

    uint8 i = 0, j = 0;
    uint32 port_id = 0;     /* port_id, global, need add txqm id */
    uint32 walk_end       = 0;
    uint32 *p_cal_entry   = NULL;
    uint32 *PortSpeed     = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint8 mac_id = 0;
    uint16 lport = 0;
    uint8 extra = 0;

    uint8 workaround_flag0 = 0;
    uint8 workaround_flag1 = 0;
    uint32 speed_xg_cnt = 0;
    uint32 speed_xlg_cnt = 0;
    uint32 speed_xxvg_cnt = 0;

    uint32 interval[SYS_CALENDAR_PORT_NUM_PER_TXQM] = {0};      /* interval cycle to the next schedule */
    uint32 interval_tmp[SYS_CALENDAR_PORT_NUM_PER_TXQM+1] = {0};  /* interval cycle to the next schedule */
    uint32 hitnum[SYS_CALENDAR_PORT_NUM_PER_TXQM] = {0};        /* hit num, per port */
    uint32 active[SYS_CALENDAR_PORT_NUM_PER_TXQM+1] = {0};        /* active list */
    uint32 en[SYS_CALENDAR_PORT_NUM_PER_TXQM] = {0};            /* port enable */
    uint32 interval_order[SYS_CALENDAR_PORT_NUM_PER_TXQM+1] = {0};
    uint32 portid_cp[SYS_CALENDAR_PORT_NUM_PER_TXQM+1] = {0};
    uint32 selport = 0;

    uint32 speedSum     = 0;    /* speed sum per txqm */
    uint32 oversubBW    = 140;  /* 140GB per txqm */
    uint32 oversubFlag  = 0;
    uint32 memberNum    = 0;

    uint32 speedMaxLock = 0;
    uint32 low_prio = 0;

    uint32 cycle = 0;  /*calendar cycle */
    uint32 done  = 0;  /* convergence status flag */
    uint32 error = 0;  /* convergence failure flag */
    uint32 force_sel = 0;  /* specical case, must select */
    uint32 sel_in_active = 0;

    uint32 minId     = 0;
    uint32 minIntv   = 0;
    uint32 minIndex  = 0;
    uint32 activeflag = 0;

    uint32 xgid = 0;
    uint32 xxgid = 0;
    uint32 xlgid = 0;
    uint32 xxvgid = 0;
    uint32 hasXg = 0;
    uint32 hasXxg = 0;
    uint32 hasXlg = 0;
    uint32 hasXxvg = 0;
    uint32 xgIllegal  = 0;
    uint32 xxgIllegal  = 0;
    uint32 xlgIllegal = 0;
    uint32 xxvgIllegal = 0;

    int32 ret = CTC_E_NONE;

    sal_memset(&cal_en, 0, sizeof(NetTxCal_m));
    sal_memset(&cal_ctl, 0, sizeof(NetTxCalCtl_m));
    sal_memset(&misc_ctl, 0, sizeof(NetTxMiscCtl_m));

    cmd = DRV_IOR((NetTxCalCtl_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cal_ctl);
    back = GetNetTxCalCtl(V, calEntry0Sel_f+txqm_id, &cal_ctl);

    cmd = DRV_IOR(NetTxMiscCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &misc_ctl);
    tmp_val32 = GetNetTxMiscCtl(V, netTxReady_f, &misc_ctl);
    if (0 == tmp_val32) /* init state */
    {
        back = 0;
    }
    else    /* dynamic switch */
    {
        back = back ? 0 : 1;
    }

    PortSpeed = mem_malloc(MEM_SYSTEM_MODULE, SYS_CALENDAR_CYCLE*sizeof(uint32));
    if (NULL == PortSpeed)
    {
        return CTC_E_NO_MEMORY;
    }
    p_cal_entry = mem_malloc(MEM_SYSTEM_MODULE, SYS_CALENDAR_CYCLE*sizeof(uint32));
    if (NULL == p_cal_entry)
    {
        ret = CTC_E_NO_MEMORY;
        goto mem_err;
    }
    sal_memset(PortSpeed, 0, SYS_CALENDAR_CYCLE*sizeof(uint32));
    sal_memset(p_cal_entry, 0, SYS_CALENDAR_CYCLE*sizeof(uint32));

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
    {
        lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, mac_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            PortSpeed[mac_id] = 0;
        }

        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            PortSpeed[mac_id] = 0;
        }
        else
        {
            SYS_DATAPATH_MODE_TO_SPEED(port_attr->speed_mode, PortSpeed[mac_id]);
        }
    }

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
    {
        lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, mac_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if ((!port_attr) || (!port_attr->xpipe_en))
        {
            continue;
        }
        if (port_attr->xpipe_en)
        {
            PortSpeed[port_attr->pmac_id] = 0;
        }
    }

    for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
    {
        speedSum += PortSpeed[txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + i];
    }

    if (speedSum > oversubBW)
    {
        oversubFlag = 1;   /* each txqm bw > 140G, we mean it a dangerous signal */
    }

    speedMaxLock = 0;
    /* calulate interval */
    for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
    {
        port_id = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + i;
        if ((PortSpeed[port_id] >= 1) && (PortSpeed[port_id] > speedMaxLock))
        {
            speedMaxLock = PortSpeed[port_id];
        }
    }

    speed_xg_cnt = 0;
    speed_xlg_cnt = 0;
    speed_xxvg_cnt = 0;

    for (i = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM; i < (txqm_id+1)*SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
    {
        if (10 == PortSpeed[i])
        {
            speed_xg_cnt++;
        }
        else if (40 == PortSpeed[i])
        {
            speed_xlg_cnt++;
        }
        else if (25 == PortSpeed[i])
        {
            speed_xxvg_cnt++;
        }
    }

    if ((2 == speed_xlg_cnt) && (speed_xxvg_cnt >= 3))
    {
        workaround_flag0 = 1;
    }
    else if ((8 == speed_xg_cnt) && (speed_xxvg_cnt >= 3))
    {
        workaround_flag1 = 1;
    }

    if (workaround_flag0)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "2*40G + 4*25G mode, Now Enter Workaround hard code...\n");
        walk_end = 13;
        p_cal_entry[0]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[1]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 0 );
        p_cal_entry[2]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[3]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 8 );
        p_cal_entry[4]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);
        p_cal_entry[5]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[6]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 0 );
        p_cal_entry[7]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[8]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 8 );
        p_cal_entry[9]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[10] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 0 );
        p_cal_entry[11] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);
        p_cal_entry[12] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[13] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 8 );
    }
    else if (workaround_flag1)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "8*10G + 4*25G mode, Now Enter Workaround hard code...\n");
        walk_end = 35;
        p_cal_entry[0]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[1]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 0 );
        p_cal_entry[2]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[3]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 1 );
        p_cal_entry[4]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[5]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 2 );
        p_cal_entry[6]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);

        p_cal_entry[7]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[8]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 3 );
        p_cal_entry[9]  = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[10] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 8 );
        p_cal_entry[11] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[12] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 9 );
        p_cal_entry[13] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);

        p_cal_entry[14] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 10);
        p_cal_entry[15] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[16] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[17] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 11);
        p_cal_entry[18] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[19] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 0 );
        p_cal_entry[20] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);

        p_cal_entry[21] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 1 );
        p_cal_entry[22] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[23] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 2 );
        p_cal_entry[24] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[25] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[26] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 3 );
        p_cal_entry[27] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);

        p_cal_entry[28] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 8 );
        p_cal_entry[29] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 12);
        p_cal_entry[30] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 9 );
        p_cal_entry[31] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 13);
        p_cal_entry[32] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 10);
        p_cal_entry[33] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 14);
        p_cal_entry[34] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 15);
        p_cal_entry[35] = (txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + 11);
    }
    else
    {
        for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
        {
            port_id = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + i;
            if ((oversubFlag) && (speedMaxLock != PortSpeed[port_id]))
            {
                low_prio = 1;
            }

            /* Get the init interval of each port, so the hit num must be 1 */
            interval[i] = _sys_usw_datapath_cal_interval(PortSpeed[port_id], 1, low_prio, 0);

            /* valid network port, init active/en/hitnum  */
            if (PortSpeed[port_id] >= 1)
            {
                active[i] = 1;
                en[i] = 1;
                hitnum[i] = 1;
                memberNum++; /* network port number */
            }
            else /* invalid network port */
            {
                active[i] = 0;
                en[i] = 0;
                hitnum[i] = 0;
            }
        }

        cycle = 0;
        done = 0;
        error = 0;
        force_sel = 0;

        /* get calendar entry */
        while ((!done) && (!error))
        {
            for (i = 1; i <= SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
            {
                interval_tmp[i] = interval[i - 1];
            }

            /* bubble sort */
            for (i = 1; i <= memberNum; i++)
            {
                minId = 1;
                minIntv = 10000;
                for (j = 1; j <= SYS_CALENDAR_PORT_NUM_PER_TXQM; j++)
                {
                    /* skip zero interval */
                    if (interval_tmp[j] && (minIntv > interval_tmp[j]))
                    {
                        /* get the minimum interval, and record the ID */
                        minIntv = interval_tmp[j];
                        minId = j;
                    }
                    else if (interval_tmp[j] && (minIntv == interval_tmp[j]) && active[j])
                    {
                        minId = j;
                    }
                }
                interval_tmp[minId] = 10000;
                interval_order[i] = minIntv;
                portid_cp[i]   = minId - 1;
            }

            minIndex = portid_cp[1];
            minIntv = interval_order[1];

            force_sel = 0;
            sel_in_active = 0;

            /* detect force and error */
            for (i = 1; i <= memberNum; i++)
            {
                if (i > interval_order[i])
                {
                    error = 2;
                    break;
                }
                selport = portid_cp[i];
                if ((i <= interval_order[i])
                    && active[selport]
                    && (!sel_in_active)
                    && (!force_sel))
                {
                    minIndex = portid_cp[i];
                    sel_in_active = 1;
                    minIntv = interval_order[i];
                }
                if (i == interval_order[i])
                {
                    force_sel = 1;
                }
            }

            for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
            {
                if (en[i])
                {
                    interval[i]--;
                } /* end of en[i] condition */
            } /* end of for [0..15] */

            p_cal_entry[cycle] = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + minIndex;
            if ((minIntv > 1) && force_sel)
            {
                extra = 1;
            }
            else
            {
                extra = 0;
            }

            port_id = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM + minIndex;
            if ((oversubFlag) && (speedMaxLock != PortSpeed[port_id]))
            {
                low_prio = 1;
            }

            interval[minIndex] = _sys_usw_datapath_cal_interval(PortSpeed[port_id], hitnum[minIndex], low_prio, extra);

            hitnum[minIndex]++;
            active[minIndex] = 0;
            activeflag = 0;

            for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
            {
                if (active[i])
                {
                    activeflag = 1;
                }
            }

            if (!activeflag)
            {
                xgIllegal = 0;
                xxgIllegal = 0;
                xlgIllegal = 0;
                xxvgIllegal = 0;

                for (i = 0; i < SYS_CALENDAR_PORT_NUM_PER_TXQM; i++)
                {
                    hasXg = 0;
                    hasXxg = 0;
                    hasXlg = 0;
                    hasXxvg = 0;
                    port_id = txqm_id*SYS_CALENDAR_PORT_NUM_PER_TXQM+i;
                    if (PortSpeed[port_id] == 10)
                    {
                        hasXg = 1;
                        xgid = i;
                    }
                    if (PortSpeed[port_id] == 20)
                    {
                        hasXxg = 1;
                        xxgid = i;
                    }
                    if (PortSpeed[port_id] == 40)
                    {
                        hasXlg = 1;
                        xlgid = i;
                    }
                    if (PortSpeed[port_id] == 25)
                    {
                        hasXxvg = 1;
                        xxvgid = i;
                    }

                    if (hasXg && (10*(cycle+1) > 187*(hitnum[xgid]-1)))
                    {
                        xgIllegal = 1;
                        active[i] = 1;
                    }
                    if (hasXxg && (10*(cycle+1) > 93*(hitnum[xxgid]-1)))
                    {
                        xxgIllegal = 1;
                        active[i] = 1;
                    }
                    if (hasXlg && (10*(cycle+1) > 46*(hitnum[xlgid]-1)))
                    {
                        xlgIllegal = 1;
                        active[i] = 1;
                    }
                    if (hasXxvg && (1000*(cycle+1) > 7486*(hitnum[xxvgid]-1)))
                    {
                        xxvgIllegal = 1;
                        active[i] = 1;
                    }
                }

                if (!xgIllegal && !xxgIllegal && !xlgIllegal && !xxvgIllegal)
                {
                    done = 1;
                    walk_end = cycle;
                }

            }
            cycle++;
            if (cycle > SYS_CALENDAR_CYCLE)
            {
                error = 1;
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " calendar failed, cycle is greater than 64!\n");
                ret = CTC_E_INVALID_CONFIG;
                goto mem_err;
            }
        } /* end of while loop */
        if (error)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "TXQM %d NetTx calendar generate mistake!\n", txqm_id);
            ret = CTC_E_INVALID_CONFIG;
            goto mem_err;
        }
    }

    /*cfg NetTxCal for calendar*/
    for (i = 0; i <= walk_end; i++)
    {
        if (!back)
        {
            tmp_val32 = p_cal_entry[i];
            DRV_IOW_FIELD(lchip, NetTxCal_t, NetTxCal_calEntry_f, &tmp_val32, &cal_en);
            cmd = DRV_IOW((NetTxCal_t+slice_id), DRV_ENTRY_FLAG);
        }
        else
        {
            tmp_val32 = p_cal_entry[i];
            DRV_IOW_FIELD(lchip, NetTxCalBak_t, NetTxCalBak_calEntry_f, &tmp_val32, &cal_en);
            cmd = DRV_IOW((NetTxCalBak_t+slice_id), DRV_ENTRY_FLAG);
        }
        DRV_IOCTL(lchip, txqm_id*SYS_CALENDAR_CYCLE+i, cmd, &cal_en);
    }

    /*cfg NetTxCalCtl for calendar*/
    if (!back)
    {
        tmp_val32 = walk_end;
        DRV_IOW_FIELD(lchip, NetTxCalCtl_t, (NetTxCalCtl_walkerEnd0_f + txqm_id * 2), &tmp_val32, &cal_ctl);
        tmp_val32 = 0;
        DRV_IOW_FIELD(lchip, NetTxCalCtl_t, (NetTxCalCtl_calEntry0Sel_f + txqm_id), &tmp_val32, &cal_ctl);
    }
    else
    {
        tmp_val32 = walk_end;
        DRV_IOW_FIELD(lchip, NetTxCalCtl_t, (NetTxCalCtl_walkerEnd0Bak_f + txqm_id * 2), &tmp_val32, &cal_ctl);
        tmp_val32 = 1;
        DRV_IOW_FIELD(lchip, NetTxCalCtl_t, (NetTxCalCtl_calEntry0Sel_f + txqm_id), &tmp_val32, &cal_ctl);
    }
    cmd = DRV_IOW((NetTxCalCtl_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cal_ctl);

mem_err:
    if (PortSpeed)
    {
        mem_free(PortSpeed);
        PortSpeed = NULL;
    }
    if (p_cal_entry)
    {
        mem_free(p_cal_entry);
        p_cal_entry = NULL;
    }

    return ret;
}

int32
sys_usw_datapath_nettxtxthrd_init(uint8 lchip, uint8 slice_id, uint32 is_oversub)
{
    uint8 txThrd[7]         = {3, 3,  6,  6,  8, 10, 6};
    uint8 txThrd_oversub[7] = {3, 7, 11, 11, 23, 23, 6};
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    uint8 i = 0;
    NetTxTxThrdCfg_m thrd_cfg;

    sal_memset(&thrd_cfg, 0, sizeof(NetTxTxThrdCfg_m));

    cmd = DRV_IOR((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);
    for (i = 0; i < 7; i++)
    {
        tmp_val32 = (is_oversub ? txThrd_oversub[i] : txThrd[i]);
        DRV_IOW_FIELD(lchip, NetTxTxThrdCfg_t, NetTxTxThrdCfg_txThrd0_f+i, &tmp_val32, &thrd_cfg);
    }
    cmd = DRV_IOW((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);

    return CTC_E_NONE;
}

/*reserved for some special application*/
int32
sys_usw_datapath_set_nettx_thrd(uint8 lchip, ctc_port_speed_t speed, uint8 thrd_val)
{
    uint8 slice_id = 0;
    uint32 cmd = 0;
    uint32 fld_id = 0;
    uint32 tmp_val32 = 0;
    NetTxTxThrdCfg_m thrd_cfg;

    sal_memset(&thrd_cfg, 0, sizeof(NetTxTxThrdCfg_m));

    cmd = DRV_IOR((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);

    switch (speed)
    {
    case CTC_PORT_SPEED_1G:
    case CTC_PORT_SPEED_100M:
    case CTC_PORT_SPEED_10M:
    case CTC_PORT_SPEED_2G5:
        fld_id = NetTxTxThrdCfg_txThrd0_f;
        break;
    case CTC_PORT_SPEED_5G:
        fld_id = NetTxTxThrdCfg_txThrd1_f;
        break;
    case CTC_PORT_SPEED_10G:
    case CTC_PORT_SPEED_20G:
        fld_id = NetTxTxThrdCfg_txThrd2_f;
        break;
    case CTC_PORT_SPEED_25G:
        fld_id = NetTxTxThrdCfg_txThrd3_f;
        break;
    case CTC_PORT_SPEED_40G:
        fld_id = NetTxTxThrdCfg_txThrd4_f;
        break;
    case CTC_PORT_SPEED_50G:
        fld_id = NetTxTxThrdCfg_txThrd5_f;
        break;
    case CTC_PORT_SPEED_100G:
        fld_id = NetTxTxThrdCfg_txThrd6_f;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    tmp_val32 = thrd_val;
    DRV_IOW_FIELD(lchip, NetTxTxThrdCfg_t, fld_id, &tmp_val32, &thrd_cfg);
    cmd = DRV_IOW((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_nettx_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 txqm_id = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    uint32 oversub = 0;
    NetTxMiscCtl_m misc_ctl;

    sal_memset(&misc_ctl, 0, sizeof(NetTxMiscCtl_m));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        cmd = DRV_IOR((NetTxMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &misc_ctl);
        tmp_val32 = 0;
        DRV_IOW_FIELD(lchip, NetTxMiscCtl_t, NetTxMiscCtl_pauseEn_f, &tmp_val32, &misc_ctl);
        cmd = DRV_IOW((NetTxMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &misc_ctl);
    }

    /*alloc buffer by actual status, but if serdes_num = 4, extern rsv port don't alloc buffer, while if port 2 is rsv port, port 2 need alloc buffer*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_init_nettx_buf(lchip, slice_id));
        for (txqm_id = 0; txqm_id < 4; txqm_id++)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_set_calendar_cfg(lchip, slice_id, txqm_id));
        }
        CTC_ERROR_RETURN(_sys_usw_datapath_get_oversub_status(lchip, slice_id, &oversub, NULL, NULL));
        if (oversub)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_nettxtxthrd_init(lchip, slice_id, TRUE));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_nettxtxthrd_init(lchip, slice_id, FALSE));
        }
    }
    sal_task_sleep(1);

    /*set nettx ready*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        cmd = DRV_IOR((NetTxMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &misc_ctl);
        tmp_val32 = 1;
        DRV_IOW_FIELD(lchip, NetTxMiscCtl_t, NetTxMiscCtl_netTxReady_f, &tmp_val32, &misc_ctl);
        cmd = DRV_IOW((NetTxMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &misc_ctl);
    }

    return CTC_E_NONE;
}

/* The last step */
int32
_sys_usw_datapath_misc_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 index = 0;
    uint32 tmp_val32 = 0;
    BufRetrvParityCtl_m br_parity;
    EpeGlobalChannelMap_m epe_chanmap;
    DsLinkAggregateChannelMemberSet_m chan_member_set;
    DsLinkAggregateMemberSet_m member_set;
    DsDot1AePnCheck_m   dot1ae_check;
    AutoGenPktEn_m    pkt_en;
    AutoGenPktTxPktAck_m pkt_ack;
    AutoGenPktRxPktStats_m  rxpkt_stats;
    AutoGenPktTxPktStats_m  txpkt_stats;
    AutoGenPktPktCfg_m      pkt_cfg;
    ParserUdfCam_m          parser_udf;
    IpeLkupMgrReserved_m    ipe_lkp_rsv;
    EpeHdrEditReserved_m    epe_hdr_rsv;
    EpeOamCtl_m             epe_oam_ctl;
    IpeUserIdCtl_m          ipe_uid_ctl;
    IpeUserIdTcamCtl_m      ipe_tcm_ctl;

    sal_memset(&chan_member_set, 0, sizeof(DsLinkAggregateChannelMemberSet_m));
    sal_memset(&member_set, 0, sizeof(DsLinkAggregateMemberSet_m));
    sal_memset(&dot1ae_check, 0, sizeof(DsDot1AePnCheck_m));
    sal_memset(&pkt_en, 0, sizeof(AutoGenPktEn_m));
    sal_memset(&pkt_ack, 0, sizeof(AutoGenPktTxPktAck_m));
    sal_memset(&rxpkt_stats, 0, sizeof(AutoGenPktRxPktStats_m));
    sal_memset(&txpkt_stats, 0, sizeof(AutoGenPktTxPktStats_m));
    sal_memset(&pkt_cfg, 0, sizeof(AutoGenPktPktCfg_m));
    sal_memset(&parser_udf, 0, sizeof(ParserUdfCam_m));
    sal_memset(&ipe_lkp_rsv, 0, sizeof(IpeLkupMgrReserved_m));
    sal_memset(&epe_hdr_rsv, 0, sizeof(EpeHdrEditReserved_m));
    sal_memset(&epe_oam_ctl, 0, sizeof(EpeOamCtl_m));
    sal_memset(&ipe_uid_ctl, 0, sizeof(IpeUserIdCtl_m));
    sal_memset(&ipe_tcm_ctl, 0, sizeof(IpeUserIdTcamCtl_m));

    cmd = DRV_IOR(BufRetrvParityCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &br_parity);
    tmp_val32 = 1;
    DRV_IOW_FIELD(lchip, BufRetrvParityCtl_t, BufRetrvParityCtl_dsBufRetrvExcpEccDetectDis_f, &tmp_val32, &br_parity);
    cmd = DRV_IOW(BufRetrvParityCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &br_parity);

    cmd = DRV_IOR(EpeGlobalChannelMap_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanmap);
    tmp_val32 = 74;
    DRV_IOW_FIELD(lchip, EpeGlobalChannelMap_t, EpeGlobalChannelMap_macDecryptChannel_f, &tmp_val32, &epe_chanmap);
    cmd = DRV_IOW(EpeGlobalChannelMap_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanmap);

    cmd = DRV_IOW(DsLinkAggregateChannelMemberSet_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 64; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &chan_member_set);
    }

    cmd = DRV_IOW(DsLinkAggregateMemberSet_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 64; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &member_set);
    }

    cmd = DRV_IOW(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 128; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &dot1ae_check);
    }

    cmd = DRV_IOW(AutoGenPktEn_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 8; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &pkt_en);
    }

    cmd = DRV_IOW(AutoGenPktTxPktAck_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 8; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &pkt_ack);
    }

    cmd = DRV_IOW(AutoGenPktRxPktStats_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 8; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &rxpkt_stats);
    }

    cmd = DRV_IOW(AutoGenPktTxPktStats_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 8; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &txpkt_stats);
    }

    cmd = DRV_IOW(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 8; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &pkt_cfg);
    }

    /*clear udf cam*/
    cmd = DRV_IOW(ParserUdfCam_t, DRV_ENTRY_FLAG);
    for (index = 0; index < 16; index++)
    {
        DRV_IOCTL(lchip, index, cmd, &parser_udf);
    }

    if(DRV_IS_TSINGMA(lchip))
    {
        /*TM ECO patch cfg*/
        /*IpeLkupMgrReserved.reserve  0x1*/
        /*SYSTEM MODIFIED by yejl for tsingma tap nvgre stript header, 2019-08-22*/
#if defined(OFPRODUCT) || defined(TAPPRODUCT)
        tmp_val32 = 0;
#else
        tmp_val32 = 1;
#endif
        DRV_IOW_FIELD(lchip, IpeLkupMgrReserved_t, IpeLkupMgrReserved_reserve_f, &tmp_val32, &ipe_lkp_rsv);
        cmd = DRV_IOW(IpeLkupMgrReserved_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ipe_lkp_rsv);

        /*EpeHdrEditReserved.reserved  0x8000 reserved[15] = 1*/
        tmp_val32 = 0x8000;
        DRV_IOW_FIELD(lchip, EpeHdrEditReserved_t, EpeHdrEditReserved_reserved_f, &tmp_val32, &epe_hdr_rsv);
        cmd = DRV_IOW(EpeHdrEditReserved_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_hdr_rsv);

        /*EpeOamCtl.oamFilterDiscardOpCode  0x0*/
        cmd = DRV_IOR(EpeOamCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_oam_ctl);
        tmp_val32 = 0x0;
        DRV_IOW_FIELD(lchip, EpeOamCtl_t, EpeOamCtl_oamFilterDiscardOpCode_f, &tmp_val32, &epe_oam_ctl);
        cmd = DRV_IOW(EpeOamCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_oam_ctl);

        /*IpeUserIdCtl.sclTcamAdMergeMode  0x0*/
        /*IpeUserIdCtl.userIdResultPriorityMode  0x0*/
        cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ipe_uid_ctl);
        tmp_val32 = 0x0;
        DRV_IOW_FIELD(lchip, IpeUserIdCtl_t, IpeUserIdCtl_sclTcamAdMergeMode_f, &tmp_val32, &ipe_uid_ctl);
        DRV_IOW_FIELD(lchip, IpeUserIdCtl_t, IpeUserIdCtl_userIdResultPriorityMode_f, &tmp_val32, &ipe_uid_ctl);
        cmd = DRV_IOW(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ipe_uid_ctl);

        /*IpeUserIdTcamCtl.sclUnknownL3TypeForceToMacKey  0x0*/
        cmd = DRV_IOR(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ipe_tcm_ctl);
        tmp_val32 = 0x0;
        DRV_IOW_FIELD(lchip, IpeUserIdTcamCtl_t, IpeUserIdTcamCtl_sclUnknownL3TypeForceToMacKey_f, &tmp_val32, &ipe_tcm_ctl);
        cmd = DRV_IOW(IpeUserIdTcamCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ipe_tcm_ctl);

    }


    return CTC_E_NONE;
}

/* flag: 1-Sop; 0-Body */
STATIC int32
_sys_usw_datapath_get_epesched_eptr(uint8 lchip, uint8 chan_id, uint8 flag, uint16 sptr, uint16 *p_end, uint32 *prev4lane_is_dyn)
{
    uint8   slice_id      = 0;
    uint16  lport         = 0;
    uint16  chip_port     = 0;
    uint8   hss_idx       = 0;
    uint8   lane_id       = 0;
    uint16  serdes_id     = 0;
    uint8   num           = 0;
    uint16  e_pointer     = 0;
    sys_datapath_bufsz_step_t  step;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes    = NULL;
    sys_datapath_lport_attr_t* port_attr    = NULL;

    sal_memset(&step, 0, sizeof(sys_datapath_bufsz_step_t));
    _sys_usw_datapath_get_bufsz_step(lchip, chan_id, &step);
    if (flag)
    {
        e_pointer = sptr + step.sop_buf_num - 1;
    }
    else
    {
        e_pointer = sptr + step.body_buf_num - 1;
    }
    if (p_end)
        *p_end = e_pointer;

    if (chan_id >= SYS_PHY_PORT_NUM_PER_SLICE)
    {
        return CTC_E_NONE;
    }

    lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        return CTC_E_NONE;
    }

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];
    if (0 == p_serdes->is_dyn)
    {
        /* Not support dynamic switch, s_pointer is equal to previous e_pointer+1 */
        if (prev4lane_is_dyn)
            *prev4lane_is_dyn = 0;
    }
    else
    {
        /* support dynamic switch */
        if (prev4lane_is_dyn)
            *prev4lane_is_dyn = 1;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_datapath_set_epe_body_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16 e_pointer)
{
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    EpeScheduleBodyRamConfigRa_m body_ram_cfg;
    EpeScheduleBodyRamRdPtrRa_m body_rd_ptr;
    EpeScheduleBodyRamWrPtrRa_m body_wr_ptr;
    BufRetrvCreditConfigMem_m credit_cfg;

    sal_memset(&body_ram_cfg, 0 ,sizeof(EpeScheduleBodyRamConfigRa_m));
    sal_memset(&body_rd_ptr, 0 ,sizeof(EpeScheduleBodyRamRdPtrRa_m));
    sal_memset(&body_wr_ptr, 0 ,sizeof(EpeScheduleBodyRamWrPtrRa_m));
    sal_memset(&credit_cfg, 0, sizeof(BufRetrvCreditConfigMem_m));

    if (s_pointer >= EPE_BODY_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id: %d, s-pointer:%d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    if (e_pointer >= EPE_BODY_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id: %d, e-pointer:%d \n", chan_id, e_pointer);
        return CTC_E_INVALID_PARAM;
    }

    /* write to HW table */
    tmp_val32 = 1;
    DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_txThrdSel_f, &tmp_val32, &body_ram_cfg);
    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgStartPtr_f, &tmp_val32, &body_ram_cfg);
    tmp_val32 = e_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgEndPtr_f, &tmp_val32, &body_ram_cfg);
    cmd = DRV_IOW((EpeScheduleBodyRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &body_ram_cfg);

    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamRdPtrRa_t+slice_id), EpeScheduleBodyRamRdPtrRa_rdPtrUsed_f, &tmp_val32, &body_rd_ptr);
    cmd = DRV_IOW((EpeScheduleBodyRamRdPtrRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &body_rd_ptr);

    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamWrPtrRa_t+slice_id), EpeScheduleBodyRamWrPtrRa_wrPtrUsed_f, &tmp_val32, &body_wr_ptr);
    cmd = DRV_IOW((EpeScheduleBodyRamWrPtrRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &body_wr_ptr);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_epe_sop_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16 e_pointer)
{
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    EpeScheduleSopRamConfigRa_m sop_ram_cfg;
    EpeScheduleSopRamRdPtrRa_m sop_rd_ptr;
    EpeScheduleSopRamWrPtrRa_m sop_wr_ptr;

    sal_memset(&sop_ram_cfg, 0 ,sizeof(EpeScheduleSopRamConfigRa_m));
    sal_memset(&sop_rd_ptr, 0 ,sizeof(EpeScheduleSopRamRdPtrRa_m));
    sal_memset(&sop_wr_ptr, 0 ,sizeof(EpeScheduleSopRamWrPtrRa_m));

    if (s_pointer >= EPE_SOP_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_sop chan_id:%d, s-pointer:%d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    if (e_pointer >= EPE_SOP_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_sop chan_id:%d, e-pointer:%d \n", chan_id, e_pointer);
        return CTC_E_INVALID_PARAM;
    }

    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgStartPtr_f, &tmp_val32, &sop_ram_cfg);
    tmp_val32 = e_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgEndPtr_f, &tmp_val32, &sop_ram_cfg);
    cmd = DRV_IOW((EpeScheduleSopRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &sop_ram_cfg);

    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleSopRamRdPtrRa_t+slice_id), EpeScheduleSopRamRdPtrRa_rdPtrUsed_f, &tmp_val32, &sop_rd_ptr);
    cmd = DRV_IOW((EpeScheduleSopRamRdPtrRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &sop_rd_ptr);

    tmp_val32 = s_pointer;
    DRV_IOW_FIELD(lchip, (EpeScheduleSopRamWrPtrRa_t+slice_id), EpeScheduleSopRamWrPtrRa_wrPtrUsed_f, &tmp_val32, &sop_wr_ptr);
    cmd = DRV_IOW((EpeScheduleSopRamWrPtrRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &sop_wr_ptr);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_epe_sched_mem(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    uint8 sec_enable = 0;
    uint8 wlan_enable = 0;
    uint8 chan_id = 0;
    uint16 lport = 0;
    uint16 prev4lane_sptr_body    = 0;
    uint16 prev4lane_sptr_sop     = 0;
    uint16 net15g_sptr_body  = 0;
    uint16 net15g_sptr_sop   = 0;
    uint16 misc_sptr_body    = 0;
    uint16 misc_sptr_sop     = 0;
    uint16 net28g_sptr_body  = 0;
    uint16 net28g_sptr_sop   = 0;
    uint16 end1 = 0;
    uint16 end2 = 0;
    uint32 prev4lane_is_dyn  = 0;
    uint8 cnt = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    wlan_enable = p_usw_datapath_master[lchip]->wlan_enable;
    sec_enable = p_usw_datapath_master[lchip]->dot1ae_enable;

    for (chan_id = 0; chan_id < SYS_PHY_PORT_NUM_PER_SLICE; chan_id++)
    {
        lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        /* #1, alloc port on HSS15G first */
        if ((SYS_DMPS_NETWORK_PORT == port_attr->port_type)
            &&(!SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id)))
        {
            if (SYS_DATAPATH_CHAN_IS_STUB(chan_id))
            {
                if (prev4lane_is_dyn)
                {
                    net15g_sptr_body = prev4lane_sptr_body + 4*(g_bufsz_step[SPEED_MODE_10].body_buf_num);
                    net15g_sptr_sop  = prev4lane_sptr_sop + 4*(g_bufsz_step[SPEED_MODE_10].sop_buf_num);
                }
                /* record stub channel startPtr */
                prev4lane_sptr_body = net15g_sptr_body;   /* the last used channel in prev4lane */
                prev4lane_sptr_sop  = net15g_sptr_sop;   /* the last used channel in prev4lane */

                g_epe_mem_info[cnt].stub_chan_id = chan_id;
                g_epe_mem_info[cnt].body_s_pointer = net15g_sptr_body;
                g_epe_mem_info[cnt].sop_s_pointer = net15g_sptr_sop;
                cnt++;
            }
            /* use current startPtr, get current endPtr */
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 0, net15g_sptr_body, &end1, &prev4lane_is_dyn));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, port_attr->chan_id, net15g_sptr_body, end1));
            /* get next startPtr */
            net15g_sptr_body = end1 + 1;

            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 1, net15g_sptr_sop, &end2, &prev4lane_is_dyn));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, port_attr->chan_id, net15g_sptr_sop, end2));
            /* get next startPtr */
            net15g_sptr_sop = end2 + 1;
        }
    }

    /* get HSS15G port network channel End */
    misc_sptr_body = 0;
    misc_sptr_sop = 0;
    for (chan_id = 0; chan_id < SYS_PHY_PORT_NUM_PER_SLICE; chan_id++)
    {
        lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }
        if (!SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id))
        {
            cmd = DRV_IOR(EpeScheduleBodyRamConfigRa_t+slice_id, EpeScheduleBodyRamConfigRa_cfgEndPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &tmp_val32);
            if (tmp_val32 >= misc_sptr_body)
                misc_sptr_body = tmp_val32;

            cmd = DRV_IOR(EpeScheduleSopRamConfigRa_t+slice_id, EpeScheduleSopRamConfigRa_cfgEndPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &tmp_val32);
            if (tmp_val32 >= misc_sptr_sop)
                misc_sptr_sop = tmp_val32;
        }
    }
    misc_sptr_body += 1;
    misc_sptr_sop  += 1;

    chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
    if (prev4lane_is_dyn)
    {
        misc_sptr_body = prev4lane_sptr_body + 4*(g_bufsz_step[SPEED_MODE_10].body_buf_num);
        misc_sptr_sop  = prev4lane_sptr_sop + 4*(g_bufsz_step[SPEED_MODE_10].sop_buf_num);
    }

    /*!!! ALERT, alloc Eloop first, and then alloc MacSec, this sequence MUST NOT change,
                 and NO OTHER channel between them !!!*/
    /* #2, alloc Eloop channel: 75 */
    CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 0, misc_sptr_body, &end1, NULL));
    CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, chan_id, misc_sptr_body, end1));
    misc_sptr_body = end1 + 1;
    CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 1, misc_sptr_sop, &end2, NULL));
    CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, misc_sptr_sop, end2));
    misc_sptr_sop = end2 + 1;

    if (sec_enable)
    {
        /* misc channel 73/74: MacEncrypt/MacDecrypt */
        for (chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT); chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT); chan_id++)
        {
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 0, misc_sptr_body, &end1, NULL));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, chan_id, misc_sptr_body, end1));
            misc_sptr_body = end1 + 1;
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 1, misc_sptr_sop, &end2, NULL));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, misc_sptr_sop, end2));
            misc_sptr_sop = end2 + 1;
        }
    }
    if (wlan_enable)
    {
        /* misc channel :
           64 -- WLAN_ENCRYPT0
           65 -- WLAN_ENCRYPT1
           66 -- WLAN_ENCRYPT2
           67 -- WLAN_ENCRYPT3
           68 -- WLAN_DECRYPT0
           69 -- WLAN_DECRYPT1
           70 -- WLAN_DECRYPT2
           71 -- WLAN_DECRYPT3
           72 -- WLAN_REASSEMBLE
          */
        for (chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0); chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE); chan_id++)
        {
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 0, misc_sptr_body, &end1, NULL));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, chan_id, misc_sptr_body, end1));
            misc_sptr_body = end1 + 1;
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 1, misc_sptr_sop, &end2, NULL));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, misc_sptr_sop, end2));
            misc_sptr_sop = end2 + 1;
        }
    }

    /* get HSS15G + Misc channel End */
    net28g_sptr_body = 0;
    net28g_sptr_sop = 0;
    for (chan_id = 0; chan_id <= MCHIP_CAP(SYS_CAP_CHANID_ELOOP); chan_id++)
    {
        lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }
        if (!SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id))
        {
            cmd = DRV_IOR(EpeScheduleBodyRamConfigRa_t+slice_id, EpeScheduleBodyRamConfigRa_cfgEndPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &tmp_val32);
            if (tmp_val32 >= net28g_sptr_body)
                net28g_sptr_body = tmp_val32;

            cmd = DRV_IOR(EpeScheduleSopRamConfigRa_t+slice_id, EpeScheduleSopRamConfigRa_cfgEndPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &tmp_val32);
            if (tmp_val32 >= net28g_sptr_sop)
                net28g_sptr_sop = tmp_val32;
        }
    }
    net28g_sptr_body += 1;
    net28g_sptr_sop  += 1;
    prev4lane_is_dyn = 0;

    for (chan_id = 0; chan_id < SYS_PHY_PORT_NUM_PER_SLICE; chan_id++)
    {
        lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
        if (SYS_COMMON_USELESS_MAC == lport)
        {
            continue;
        }
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        /* #3, alloc port on HSS28G last */
        if ((SYS_DMPS_NETWORK_PORT == port_attr->port_type)
            &&(SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id)))
        {
            if (SYS_DATAPATH_CHAN_IS_STUB(chan_id))
            {
                if (prev4lane_is_dyn)
                {
                    net28g_sptr_body = prev4lane_sptr_body + 4*(g_bufsz_step[SPEED_MODE_25].body_buf_num);
                    net28g_sptr_sop = prev4lane_sptr_sop + 4*(g_bufsz_step[SPEED_MODE_25].sop_buf_num);
                }
                /* record stub channel startPtr */
                prev4lane_sptr_body = net28g_sptr_body;   /* the last used channel in prev4lane */
                prev4lane_sptr_sop = net28g_sptr_sop;   /* the last used channel in prev4lane */
                g_epe_mem_info[cnt].stub_chan_id = chan_id;
                g_epe_mem_info[cnt].body_s_pointer = net28g_sptr_body;
                g_epe_mem_info[cnt].sop_s_pointer = net28g_sptr_sop;
                cnt++;
            }
            /* use current startPtr, get current endPtr */
            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 0, net28g_sptr_body, &end1, &prev4lane_is_dyn));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, port_attr->chan_id, net28g_sptr_body, end1));
            /* get next startPtr */
            net28g_sptr_body = end1 + 1;

            CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, chan_id, 1, net28g_sptr_sop, &end2, &prev4lane_is_dyn));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, port_attr->chan_id, net28g_sptr_sop, end2));
            /* get next startPtr */
            net28g_sptr_sop = end2 + 1;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_epe_wrr_weight(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 weight = 0;
    uint8 txqm = 0;
    uint8 array_ct = 0;
    uint8 xg_ct = 0;
    uint8 xxvg_ct = 0;
    uint16 divisor[3] = {0};
    uint16 xg_speed[4] = {0};
    uint16 xlg_speed[4] = {0};
    uint16 cg_speed[4] = {0};
    uint16 net_speed[4] = {0};
    uint16 misc_speed = 0;
    uint16 net_speed_all = 0;
    uint32 tmp_val32 = 0;
    uint32 wrrbase = 0;
    uint32 fld_id = 0;
    uint16 array[3][84];/*xg, xlg, group*/
    sys_datapath_lport_attr_t* port_attr = NULL;
    EpeScheduleMiscCtl_m    misc_ctl;
    EpeScheduleWrrWeightConfig_m wrr_cfg;
    EpeScheduleNetPortWeightConfigRa_m port_wt;
    EpeScheduleMiscChanWeightConfig_m misc_wt;

    sal_memset(&wrr_cfg, 0,sizeof(EpeScheduleWrrWeightConfig_m));
    sal_memset(&port_wt, 0,sizeof(EpeScheduleNetPortWeightConfigRa_m));
    sal_memset(&misc_wt, 0,sizeof(EpeScheduleMiscChanWeightConfig_m));
    sal_memset(array, 0, 3 * SYS_PHY_PORT_NUM_PER_SLICE * sizeof(uint16));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
            {
                SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);

                if ((port_attr->speed_mode <= CTC_PORT_SPEED_10G)||(CTC_PORT_SPEED_5G == port_attr->speed_mode))
                {
                    xg_ct++;
                    xg_speed[lport/16] += wrrbase;
                    array[0][lport] = wrrbase;
                }
                else if ((port_attr->speed_mode <= CTC_PORT_SPEED_50G)&&(port_attr->speed_mode != CTC_PORT_SPEED_100G))
                {
                    if (port_attr->speed_mode == CTC_PORT_SPEED_25G)
                    {
                        xxvg_ct++;
                    }
                    xlg_speed[lport/16] += wrrbase;
                    array[0][lport] = wrrbase;
                }
                else if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                {
                    cg_speed[lport/16] += wrrbase;
                    array[0][lport] = wrrbase;
                }
            }
            else
            {
                if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)))
                {
                    array[0][lport] = 10 * 10;
                    misc_speed += 10 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
            }
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[0], 84, &divisor[0]));

        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                continue;
            }

            weight = array[0][lport]/divisor[0];
            tmp_val32 = weight;
            if (port_attr->chan_id < SYS_PHY_PORT_NUM_PER_SLICE)
            {
                cmd = DRV_IOR((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->mac_id, cmd, &port_wt));
                DRV_IOW_FIELD(lchip, (EpeScheduleNetPortWeightConfigRa_t+slice_id), EpeScheduleNetPortWeightConfigRa_cfgWeight_f,
                    &tmp_val32, &port_wt);
                cmd = DRV_IOW((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->mac_id, cmd, &port_wt));
            }
            else
            {
                cmd = DRV_IOR((EpeScheduleMiscChanWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_wt));

                if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capEnc0WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capEnc1WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capEnc2WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capEnc3WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capDec0WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capDec1WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capDec2WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capDec3WtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_capReassembleWtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_macEncWtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_macDecWtCfg_f;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                        fld_id = EpeScheduleMiscChanWeightConfig_eloopWtCfg_f;
                }
                else
                {
                        continue;
                }
                DRV_IOW_FIELD(lchip, (EpeScheduleMiscChanWeightConfig_t+slice_id), fld_id, &tmp_val32, &misc_wt);

                cmd = DRV_IOW((EpeScheduleMiscChanWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_wt));
            }
        }

        for (txqm = 0; txqm < 4; txqm++)
        {
            net_speed[txqm] = xg_speed[txqm] + xlg_speed[txqm];
        }

        for (txqm = 0; txqm < 4; txqm++)
        {
            net_speed_all += net_speed[txqm];
        }

        array_ct = 0;
        for (txqm = 0; txqm < 4; txqm++)
        {
            array[2][array_ct++] = xg_speed[txqm];
        }
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        cmd = DRV_IOR((EpeScheduleWrrWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));

        for (txqm = 0; txqm < 4; txqm++)
        {
            array_ct = 0;
            array[2][array_ct++] = xg_speed[txqm];
            array[2][array_ct++] = xlg_speed[txqm];
            CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

            tmp_val32 = xg_speed[txqm]/divisor[2];
            DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), (EpeScheduleWrrWeightConfig_toGroup0XgWtCfg_f+txqm*2),
                &tmp_val32, &wrr_cfg);
            tmp_val32 = xlg_speed[txqm]/divisor[2];
            DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), (EpeScheduleWrrWeightConfig_toGroup0XlgWtCfg_f+txqm*2),
                &tmp_val32, &wrr_cfg);
        }

        array_ct = 0;
        for (txqm = 0; txqm < 4; txqm++)
        {
            array[2][array_ct++] = net_speed[txqm];
        }
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        for (txqm = 0; txqm < 4; txqm++)
        {
            tmp_val32 = net_speed[txqm]/divisor[2];
            DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), (EpeScheduleWrrWeightConfig_toGroup6Net0WtCfg_f+txqm),
                &tmp_val32, &wrr_cfg);
        }

        array_ct = 0;
        for (txqm = 0; txqm < 4; txqm++)
        {
            array[2][array_ct++] = cg_speed[txqm];
        }
        array[2][array_ct++] = net_speed_all;
        array[2][array_ct++] = misc_speed;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        for (txqm = 0; txqm < 4; txqm++)
        {
            tmp_val32 = cg_speed[txqm]/divisor[2];
            DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), (EpeScheduleWrrWeightConfig_toGroup5Cg0WtCfg_f+txqm),
                &tmp_val32, &wrr_cfg);
        }
        tmp_val32 = misc_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), EpeScheduleWrrWeightConfig_toGroup5MiscWtCfg_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = net_speed_all/divisor[2];
        DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), EpeScheduleWrrWeightConfig_toGroup5NetWtCfg_f,
            &tmp_val32, &wrr_cfg);

        tmp_val32 = 0x3c;
        DRV_IOW_FIELD(lchip, (EpeScheduleWrrWeightConfig_t+slice_id), EpeScheduleWrrWeightConfig_wrrHiPriorityEnBmp_f,
            &tmp_val32, &wrr_cfg);

        cmd = DRV_IOW((EpeScheduleWrrWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));

        /*fix rtl bug, only for 24*10G + 16*25G*/
        if ((xg_ct == 24) && (xxvg_ct == 16))
        {
            cmd = DRV_IOR((EpeScheduleMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_ctl));

            tmp_val32 = 1;
            DRV_IOW_FIELD(lchip, (EpeScheduleMiscCtl_t+slice_id), EpeScheduleMiscCtl_cfgNetGroupWithRR_f,
                &tmp_val32, &misc_ctl);
            cmd = DRV_IOW((EpeScheduleMiscCtl_t+slice_id), DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &misc_ctl));
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_epe_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    EpeScheduleNetPortToChanRa_m port_to_chan;
    EpeScheduleNetChanToPortRa_m chan_to_port;

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        /*config epehdradj port mapping, and epeschedule to nettx port mapping*/
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            tmp_val32 = lport;
            cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t+slice_id, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->chan_id, cmd, &tmp_val32));

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
        }
        CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sched_mem(lchip, slice_id));

        CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_misc_chan_init(uint8 lchip)
{
    uint32 cmd = 0;
    QMgrDeqChanIdCfg_m qmgr_chanid_cfg;
    BufRetrvChanIdCfg_m buf_chanid_cfg;
    EpeScheduleChanIdCfg_m epe_chanid_cfg;

    sal_memset(&qmgr_chanid_cfg, 0, sizeof(QMgrDeqChanIdCfg_m));
    sal_memset(&buf_chanid_cfg, 0, sizeof(BufRetrvChanIdCfg_m));
    sal_memset(&epe_chanid_cfg, 0, sizeof(EpeScheduleChanIdCfg_m));

    cmd = DRV_IOR(QMgrDeqChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &qmgr_chanid_cfg);
    cmd = DRV_IOR(BufRetrvChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_chanid_cfg);
    cmd = DRV_IOR(EpeScheduleChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanid_cfg);

    /*set QMgrDeqChanIdCfg*/
    SetQMgrDeqChanIdCfg(V, cfgCapwapDecrypt0ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetQMgrDeqChanIdCfg(V, cfgCapwapDecrypt1ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1));
    SetQMgrDeqChanIdCfg(V, cfgCapwapDecrypt2ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2));
    SetQMgrDeqChanIdCfg(V, cfgCapwapDecrypt3ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3));
    SetQMgrDeqChanIdCfg(V, cfgCapwapEncrypt0ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetQMgrDeqChanIdCfg(V, cfgCapwapEncrypt1ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1));
    SetQMgrDeqChanIdCfg(V, cfgCapwapEncrypt2ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2));
    SetQMgrDeqChanIdCfg(V, cfgCapwapEncrypt3ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3));
    SetQMgrDeqChanIdCfg(V, cfgCapwapReassembleChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    SetQMgrDeqChanIdCfg(V, cfgMacDecryptChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT));
    SetQMgrDeqChanIdCfg(V, cfgMacEncryptChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT));
    SetQMgrDeqChanIdCfg(V, cfgILoopChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_ILOOP));
    SetQMgrDeqChanIdCfg(V, cfgELoopChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_ELOOP));
    SetQMgrDeqChanIdCfg(V, cfgOamChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_OAM));
    SetQMgrDeqChanIdCfg(V, cfgDma0ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0));
    SetQMgrDeqChanIdCfg(V, cfgDma1ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1));
    SetQMgrDeqChanIdCfg(V, cfgDma2ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2));
    SetQMgrDeqChanIdCfg(V, cfgDma3ChanId_f, &qmgr_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3));

    /*set BufRetrvChanIdCfg*/
    SetBufRetrvChanIdCfg(V, cfgCapwapDecrypt0ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetBufRetrvChanIdCfg(V, cfgCapwapDecrypt1ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1));
    SetBufRetrvChanIdCfg(V, cfgCapwapDecrypt2ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2));
    SetBufRetrvChanIdCfg(V, cfgCapwapDecrypt3ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3));
    SetBufRetrvChanIdCfg(V, cfgCapwapEncrypt0ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetBufRetrvChanIdCfg(V, cfgCapwapEncrypt1ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1));
    SetBufRetrvChanIdCfg(V, cfgCapwapEncrypt2ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2));
    SetBufRetrvChanIdCfg(V, cfgCapwapEncrypt3ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3));
    SetBufRetrvChanIdCfg(V, cfgCapwapReassembleChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    SetBufRetrvChanIdCfg(V, cfgMacDecryptChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT));
    SetBufRetrvChanIdCfg(V, cfgMacEncryptChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT));
    SetBufRetrvChanIdCfg(V, cfgILoopChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_ILOOP));
    SetBufRetrvChanIdCfg(V, cfgELoopChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_ELOOP));
    SetBufRetrvChanIdCfg(V, cfgOamChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_OAM));
    SetBufRetrvChanIdCfg(V, cfgDma0ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0));
    SetBufRetrvChanIdCfg(V, cfgDma1ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1));
    SetBufRetrvChanIdCfg(V, cfgDma2ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2));
    SetBufRetrvChanIdCfg(V, cfgDma3ChanId_f, &buf_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3));

    /*set EpeScheduleChanIdCfg*/
    SetEpeScheduleChanIdCfg(V, cfgCapwapDecrypt0ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetEpeScheduleChanIdCfg(V, cfgCapwapDecrypt1ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1));
    SetEpeScheduleChanIdCfg(V, cfgCapwapDecrypt2ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2));
    SetEpeScheduleChanIdCfg(V, cfgCapwapDecrypt3ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3));
    SetEpeScheduleChanIdCfg(V, cfgCapwapEncrypt0ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetEpeScheduleChanIdCfg(V, cfgCapwapEncrypt1ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1));
    SetEpeScheduleChanIdCfg(V, cfgCapwapEncrypt2ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2));
    SetEpeScheduleChanIdCfg(V, cfgCapwapEncrypt3ChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3));
    SetEpeScheduleChanIdCfg(V, cfgCapwapReassembleChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    SetEpeScheduleChanIdCfg(V, cfgMacDecryptChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT));
    SetEpeScheduleChanIdCfg(V, cfgMacEncryptChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT));
    SetEpeScheduleChanIdCfg(V, cfgELoopChanId_f, &epe_chanid_cfg, MCHIP_CAP(SYS_CAP_CHANID_ELOOP));

    cmd = DRV_IOW(QMgrDeqChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &qmgr_chanid_cfg);
    cmd = DRV_IOW(BufRetrvChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_chanid_cfg);
    cmd = DRV_IOW(EpeScheduleChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanid_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_net_chan_cfg(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 slice_id = 0;
    uint8 txqm = 0;
    uint8 cg_bitmap = 0;
    uint32 xg_bitmap[2] = {0, 0};
    uint32 xlg_bitmap[2] = {0, 0};
    uint16 tmp_bitmap = 0;
    uint32 tmp_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    QMgrDeqChanIdCfg_m qmgr_chanid_cfg;
    BufRetrvChanIdCfg_m buf_chanid_cfg;
    EpeScheduleChanIdCfg_m epe_chanid_cfg;
    EpeScheduleChanEnableCfg_m epe_chanen_cfg;

    sal_memset(&qmgr_chanid_cfg, 0, sizeof(QMgrDeqChanIdCfg_m));
    sal_memset(&buf_chanid_cfg, 0, sizeof(BufRetrvChanIdCfg_m));
    sal_memset(&epe_chanid_cfg, 0, sizeof(EpeScheduleChanIdCfg_m));
    sal_memset(&epe_chanen_cfg, 0, sizeof(EpeScheduleChanEnableCfg_m));

    /* read tables */
    cmd = DRV_IOR((QMgrDeqChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &qmgr_chanid_cfg);
    cmd = DRV_IOR((BufRetrvChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &buf_chanid_cfg);
    cmd = DRV_IOR((EpeScheduleChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanid_cfg);
    cmd = DRV_IOR((EpeScheduleChanEnableCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_chanen_cfg);

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
            {
                continue;
            }

            if ((SYS_DMPS_RSV_PORT == port_attr->port_type)
                && (port_attr->chan_id < SYS_PHY_PORT_NUM_PER_SLICE))
            {
                CTC_BMP_UNSET(xg_bitmap, port_attr->chan_id);
                CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                if ((port_attr->chan_id == 12)
                    || (port_attr->chan_id == 28)
                    || (port_attr->chan_id == 44)
                    || (port_attr->chan_id == 60))
                {
                    CLEAR_BIT(cg_bitmap, port_attr->chan_id / 16);
                }
            }

            if ((CTC_PORT_SPEED_1G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_100M == port_attr->speed_mode)
                || (CTC_PORT_SPEED_10M == port_attr->speed_mode)
                || (CTC_PORT_SPEED_2G5 == port_attr->speed_mode)
                || (CTC_PORT_SPEED_10G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_5G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_20G == port_attr->speed_mode))
            {
                CTC_BMP_SET(xg_bitmap, port_attr->chan_id);
                CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                if ((port_attr->chan_id == 12)
                    || (port_attr->chan_id == 28)
                    || (port_attr->chan_id == 44)
                    || (port_attr->chan_id == 60))
                {
                    CLEAR_BIT(cg_bitmap, port_attr->chan_id / 16);
                }
            }
            else if ((CTC_PORT_SPEED_40G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_25G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_50G == port_attr->speed_mode))
            {
                CTC_BMP_UNSET(xg_bitmap, port_attr->chan_id);
                CTC_BMP_SET(xlg_bitmap, port_attr->chan_id);
                if ((port_attr->chan_id == 12)
                    || (port_attr->chan_id == 28)
                    || (port_attr->chan_id == 44)
                    || (port_attr->chan_id == 60))
                {
                    CLEAR_BIT(cg_bitmap, port_attr->chan_id / 16);
                }
            }
            else if (CTC_PORT_SPEED_100G == port_attr->speed_mode)
            {
                CTC_BMP_UNSET(xg_bitmap, port_attr->chan_id);
                CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                SET_BIT(cg_bitmap, port_attr->chan_id / 16);

                tmp_val32 = port_attr->chan_id;
                DRV_IOW_FIELD(lchip, QMgrDeqChanIdCfg_t, (QMgrDeqChanIdCfg_cfgCg0ChanId_f + port_attr->chan_id / 16),
                    &tmp_val32, &qmgr_chanid_cfg);
                DRV_IOW_FIELD(lchip, BufRetrvChanIdCfg_t, (BufRetrvChanIdCfg_cfgCg0ChanId_f + port_attr->chan_id / 16),
                    &tmp_val32, &buf_chanid_cfg);
                DRV_IOW_FIELD(lchip, EpeScheduleChanIdCfg_t, (EpeScheduleChanIdCfg_cfgCg0PortId_f + port_attr->chan_id / 16),
                    &tmp_val32, &epe_chanid_cfg);
            }
        }

        SetQMgrDeqChanIdCfg(V, cfgNetCgChanEn_f, &qmgr_chanid_cfg, cg_bitmap);
        SetQMgrDeqChanIdCfg(A, cfgNetXGChanEn_f, &qmgr_chanid_cfg, xg_bitmap);
        SetQMgrDeqChanIdCfg(A, cfgNetXLGChanEn_f, &qmgr_chanid_cfg, xlg_bitmap);

        SetBufRetrvChanIdCfg(V, cfgCgChanEn_f, &buf_chanid_cfg, cg_bitmap);
        SetBufRetrvChanIdCfg(A, cfgNetXGChanEn_f, &buf_chanid_cfg, xg_bitmap);
        SetBufRetrvChanIdCfg(A, cfgNetXLGChanEn_f, &buf_chanid_cfg, xlg_bitmap);

        for (txqm = 0; txqm < 4; txqm++)
        {
            tmp_bitmap = xg_bitmap[txqm/2] >> (16 * (txqm % 2));
            tmp_val32 = tmp_bitmap;
            DRV_IOW_FIELD(lchip, EpeScheduleChanEnableCfg_t, EpeScheduleChanEnableCfg_net0XGPortBmp_f+txqm*2, &tmp_val32, &epe_chanen_cfg);

            tmp_bitmap = xlg_bitmap[txqm/2] >> (16 * (txqm % 2));
            tmp_val32 = tmp_bitmap;
            DRV_IOW_FIELD(lchip, EpeScheduleChanEnableCfg_t, EpeScheduleChanEnableCfg_net0XLGPortBmp_f+txqm*2, &tmp_val32, &epe_chanen_cfg);
        }
        tmp_val32 = cg_bitmap;
        DRV_IOW_FIELD(lchip, EpeScheduleChanEnableCfg_t, EpeScheduleChanEnableCfg_cfgCg3To0ChanIdEnBmp_f, &tmp_val32, &epe_chanen_cfg);

        /* write tables */
        cmd = DRV_IOW((QMgrDeqChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &qmgr_chanid_cfg);
        cmd = DRV_IOW((BufRetrvChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &buf_chanid_cfg);
        cmd = DRV_IOW((EpeScheduleChanIdCfg_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_chanid_cfg);
        cmd = DRV_IOW((EpeScheduleChanEnableCfg_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_chanen_cfg);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_bufretrv_wrr_weight(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 txqm = 0;
    uint8 weight = 0;
    uint8 array_ct = 0;
    uint8 xg_group_rr = 1;
    uint8 xlg_group_rr = 1;
    uint8 first_xg_mode = CTC_PORT_SPEED_MAX;
    uint8 first_xlg_mode = CTC_PORT_SPEED_MAX;
    uint16 xg_speed = 0;
    uint16 xlg_speed = 0;
    uint16 cg_speed[4] = {0};
    uint16 divisor[3] = {0};
    uint16 net_speed = 0;
    uint16 intf_speed = 0;
    uint16 misc_speed = 0;
    uint32 tmp_val32 = 0;
    uint32 net_grp_wt = 0;
    uint32 wrrbase = 0;
    uint32 fld_id  = 0;
    uint16 array[3][84];/*xg, xlg, group*/
    uint32 total_speed = 0;
    uint32 is_spec_ff = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    BufRetrvPktMiscWeightConfig_m wrr_cfg;
    BufRetrvNetPktWtCfgMem_m bufretrv_cfgwt;

    sal_memset(array, 0, 3 * SYS_PHY_PORT_NUM_PER_SLICE * sizeof(uint16));

    cmd = DRV_IOR((BufRetrvPktMiscWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                /*rsv port have no channel info*/
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
            {
                SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);
                if ((port_attr->speed_mode <= CTC_PORT_SPEED_10G)||(CTC_PORT_SPEED_5G == port_attr->speed_mode))
                {
                    xg_speed += wrrbase;
                    array[0][lport] = wrrbase;

                    if (first_xg_mode == CTC_PORT_SPEED_MAX)
                    {
                        first_xg_mode = port_attr->speed_mode;
                    }

                    if (port_attr->speed_mode != first_xg_mode)
                    {
                        xg_group_rr = 0;
                    }
                }
                else if ((port_attr->speed_mode <= CTC_PORT_SPEED_50G)&&(port_attr->speed_mode != CTC_PORT_SPEED_100G))
                {
                    xlg_speed += wrrbase;
                    array[0][lport] = wrrbase;

                    if (first_xlg_mode == CTC_PORT_SPEED_MAX)
                    {
                        first_xlg_mode = port_attr->speed_mode;
                    }

                    if (port_attr->speed_mode != first_xlg_mode)
                    {
                        xlg_group_rr = 0;
                    }
                }
                else if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                {
                    cg_speed[lport/16] += wrrbase;
                    array[0][lport] = wrrbase;
                }
            }
            else
            {
                if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)))
                {
                    array[0][lport] = 10 * 10;
                    intf_speed += 10 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
                {
                    array[0][lport] = 100 * 10;
                    intf_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ILOOP) == port_attr->chan_id)
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_OAM)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)))
                {
                    array[0][lport] = 10 * 10;
                    misc_speed += 10 * 10;
                }
            }
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[0], 84, &divisor[0]));

        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                continue;
            }

            weight = array[0][lport]/divisor[0];

            if (port_attr->chan_id < 64)
            {
                cmd = DRV_IOR((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->chan_id, cmd, &bufretrv_cfgwt));
                tmp_val32 = weight;
                DRV_IOW_FIELD(lchip, (BufRetrvNetPktWtCfgMem_t+slice_id), BufRetrvNetPktWtCfgMem_weight_f,
                    &tmp_val32, &bufretrv_cfgwt);
                cmd = DRV_IOW((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->chan_id, cmd, &bufretrv_cfgwt));
            }
            else
            {
                cmd = DRV_IOR((BufRetrvPktMiscWeightConfig_t + slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));
                tmp_val32 = weight;

                if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapEncrypt0ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapEncrypt1ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapEncrypt2ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapEncrypt3ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapDecrypt0ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapDecrypt1ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapDecrypt2ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapDecrypt3ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgCapwapReassembleChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgMacDecryptChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgMacEncryptChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_ELOOP))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgELoopChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_ILOOP))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgILoopChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_OAM))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgOamChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgDma0ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX1))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgDma1ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX2))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgDma2ChanWeight_f;
                }
                else if (port_attr->chan_id == MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3))
                {
                    fld_id = BufRetrvPktMiscWeightConfig_cfgDma3ChanWeight_f;
                }

                DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), fld_id,
                    &tmp_val32, &wrr_cfg);
                cmd = DRV_IOW((BufRetrvPktMiscWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));
            }
        }

        net_speed = xg_speed + xlg_speed;

        array[2][array_ct++] = xg_speed;
        array[2][array_ct++] = xlg_speed;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        tmp_val32 = xg_speed/divisor[2];
        DRV_IOW_FIELD(lchip, BufRetrvPktMiscWeightConfig_t, BufRetrvPktMiscWeightConfig_cfgXgGroupWeight_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = xlg_speed/divisor[2];
        DRV_IOW_FIELD(lchip, BufRetrvPktMiscWeightConfig_t, BufRetrvPktMiscWeightConfig_cfgXlgGroupWeight_f,
            &tmp_val32, &wrr_cfg);

        array_ct = 0;
        for (txqm = 0; txqm < 4; txqm++)
        {
            array[2][array_ct++] = cg_speed[txqm];
        }
        array[2][array_ct++] = net_speed;
        array[2][array_ct++] = intf_speed;
        array[2][array_ct++] = misc_speed;

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));
        net_grp_wt = net_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroupWeight_f,
            &net_grp_wt, &wrr_cfg);

        for (txqm = 0; txqm < 4; txqm++)
        {
            if (0 != cg_speed[txqm])
            {
                tmp_val32 = cg_speed[txqm]/divisor[2];
            }
            else
            {
                tmp_val32 = 0;
            }
            DRV_IOW_FIELD(lchip, BufRetrvPktMiscWeightConfig_t, (BufRetrvPktMiscWeightConfig_cfgCg0ChanWeight_f+txqm),
                &tmp_val32, &wrr_cfg);
        }

        tmp_val32 = intf_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgEpeIntfMiscGroupWeight_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = misc_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgMiscGroupWeight_f,
            &tmp_val32, &wrr_cfg);

        tmp_val32 = xg_group_rr;
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroup0WithRR_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = xlg_group_rr;
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroup1WithRR_f,
            &tmp_val32, &wrr_cfg);

        _sys_usw_datapath_get_oversub_status(lchip, slice_id, NULL, &total_speed, &is_spec_ff);
        if (total_speed > 400)
        {
            tmp_val32 = 0;
        }
        else
        {
            tmp_val32 = 0x78;
        }
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_group5HiPriPtr_f,
            &tmp_val32, &wrr_cfg);
        if (is_spec_ff)
        {
            tmp_val32 = 1;
        }
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroup0WithRR_f,
            &tmp_val32, &wrr_cfg);
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroup1WithRR_f,
            &tmp_val32, &wrr_cfg);

        cmd = DRV_IOW((BufRetrvPktMiscWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_datapath_bufretrv_init(uint8 lchip)
{
    uint16 lport = 0;
    uint16 s_pointer = 0;  /* startPtr */
    uint16 e_pointer = 0;  /* endPtr */
    uint8 index = 0;
    uint8 slice_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /*enough for all rsv port, include 2.5G; alloc buffer by position, example: port 0 alloc by 40G, port 12 alloc by 100G*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        s_pointer = 0;
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            /*BufRetrvPktStatusMem has 84 entries*/
            if (((SYS_DMPS_NETWORK_PORT != port_attr->port_type) && (port_attr->chan_id == 0)) || (port_attr->chan_id >= 84))
            {
                continue;
            }

            CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_pkt_mem(lchip, slice_id, port_attr->chan_id, s_pointer, &e_pointer));
            s_pointer = e_pointer;

            /*BufRetrvCreditConfigMem has 76 entries, Iloop,Oam,Dma don't flow to epe*/
            if (port_attr->chan_id < 76)
            {
                /*relate to BufRetrvEpeScheduleCreditConfig*/
                CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_credit(lchip, slice_id, port_attr->chan_id));
            }
        }

        /*init BufRetrvEpeScheduleCreditConfig and BufRetrvNetPktWtCfgMem*/
        CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_credit_config(lchip, slice_id));
        CTC_ERROR_RETURN(_sys_usw_datapath_bufretrv_wrr_weight(lchip, slice_id));

       /* all channel is allocated by new method. */
        /*Dma channel now do not have lport, need distribute resource seperately*/
        for (index = 0; index < 4; index++)
        {
            CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->dma_chan+index), s_pointer, &e_pointer));
             s_pointer = e_pointer;
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->elog_chan), s_pointer, &e_pointer));
         s_pointer = e_pointer;
        CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->qcn_chan), s_pointer, &e_pointer));
         s_pointer = e_pointer;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_qmgr_wrr_weight(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 index = 0;
    uint8 weight = 0;
    uint8 array_ct = 0;
    uint16 xg_speed = 0;
    uint16 xlg_speed = 0;
    uint16 net_speed = 0;
    uint16 misc_speed = 0;
    uint16 cpwp_speed = 0;
    uint16 divisor[3] = {0};
    uint32 tmp_val32 = 0;
    uint32 wrrbase = 0;
    uint16 array[3][84];/*xg, xlg, group*/
    sys_datapath_lport_attr_t* port_attr = NULL;
    DsQMgrChanWeight_m wt_cfg;
    QMgrDeqIntfWrrCtl_m wrr_cfg;

    sal_memset(array, 0, 3 * SYS_PHY_PORT_NUM_PER_SLICE * sizeof(uint16));
    sal_memset(&wt_cfg, 0, sizeof(DsQMgrChanWeight_m));
    sal_memset(&wrr_cfg, 0, sizeof(QMgrDeqIntfWrrCtl_m));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
            {
                SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);

                if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                {
                    array[0][lport] = wrrbase;
                }
                else if ((port_attr->speed_mode == CTC_PORT_SPEED_50G)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_40G)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_25G))
                {
                    array[0][lport] = wrrbase;
                    xlg_speed += wrrbase;
                }
                else if ((port_attr->speed_mode == CTC_PORT_SPEED_10G)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_5G)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_2G5)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_1G)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_100M)
                    || (port_attr->speed_mode == CTC_PORT_SPEED_10M))
                {
                    array[0][lport] = wrrbase;
                    xg_speed += wrrbase;
                }
            }
            else
            {
                if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)))
                {
                    array[0][lport] = 10 * 10;
                    cpwp_speed += 10 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
                {
                    array[0][lport] = 100 * 10;
                    cpwp_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ILOOP) == port_attr->chan_id)
                {
                    array[0][lport] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_OAM)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)))
                {
                    array[0][lport] = 10 * 10;
                    misc_speed += 10 * 10;
                }
            }
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[0], 84, &divisor[0]));

        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            weight = 0;
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
            {
                weight = array[0][lport]/divisor[0];
            }
            else
            {
                if (0 == port_attr->chan_id)
                {
                    continue;
                }
                else
                {
                    if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0) ) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3) ))
                    {
                        weight = array[0][lport]/divisor[0];
                    }
                    else
                    {
                        weight = 0;
                    }
                }
            }

            cmd = DRV_IOR((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &wt_cfg);
            tmp_val32 = weight;
            DRV_IOW_FIELD(lchip, (DsQMgrChanWeight_t+slice_id), DsQMgrChanWeight_weight_f, &tmp_val32, &wt_cfg);

            cmd = DRV_IOW((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &wt_cfg);
        }

        /*special for dma channel*/
        for (index = 0; index < 4; index++)
        {
            cmd = DRV_IOR((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, (p_usw_datapath_master[lchip]->dma_chan+index), cmd, &wt_cfg);
            tmp_val32 = 3;
            DRV_IOW_FIELD(lchip, (DsQMgrChanWeight_t+slice_id), DsQMgrChanWeight_weight_f, &tmp_val32, &wt_cfg);

            cmd = DRV_IOW((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, (p_usw_datapath_master[lchip]->dma_chan+index), cmd, &wt_cfg);
        }

        net_speed = xg_speed + xlg_speed;

        array[2][array_ct++] = xg_speed;
        array[2][array_ct++] = xlg_speed;
        array[2][array_ct++] = net_speed;
        array[2][array_ct++] = misc_speed;
        array[2][array_ct++] = cpwp_speed;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        cmd = DRV_IOR((QMgrDeqIntfWrrCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &wrr_cfg);

        tmp_val32 = xg_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (QMgrDeqIntfWrrCtl_t+slice_id), QMgrDeqIntfWrrCtl_xgIntfWtCfg_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = xlg_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (QMgrDeqIntfWrrCtl_t+slice_id), QMgrDeqIntfWrrCtl_xlgIntfWtCfg_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = net_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (QMgrDeqIntfWrrCtl_t+slice_id), QMgrDeqIntfWrrCtl_netIntfWtCfg_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = misc_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (QMgrDeqIntfWrrCtl_t+slice_id), QMgrDeqIntfWrrCtl_miscIntfWtCfg_f,
            &tmp_val32, &wrr_cfg);
        tmp_val32 = cpwp_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (QMgrDeqIntfWrrCtl_t+slice_id), QMgrDeqIntfWrrCtl_cpwpIntfWtCfg_f,
            &tmp_val32, &wrr_cfg);

        cmd = DRV_IOW((QMgrDeqIntfWrrCtl_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &wrr_cfg);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_qmgr_init(uint8 lchip)
{
    uint8 index = 0;
    uint8 slice_id = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        /*init QMgrRaChanCredit, relate to buffer size of bufretrv*/
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            /*DsQMgrChanCredit has 84 entries*/
            if (((lport >= SYS_PHY_PORT_NUM_PER_SLICE) && (!port_attr->chan_id)) || (port_attr->chan_id >= 84))
            {
                continue;
            }

            CTC_ERROR_RETURN(_sys_usw_datapath_set_qmgr_credit(lchip, port_attr->chan_id, port_attr->slice_id, 0));
        }
        /*Dma channel now do not have lport, need distribute resource seperately*/
        for (index = 0; index < 4; index++)
        {
            CTC_ERROR_RETURN(_sys_usw_datapath_set_qmgr_credit(lchip, (p_usw_datapath_master[lchip]->dma_chan+index), slice_id, 0));
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_set_qmgr_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_netrx_wrr_weight(uint8 lchip, uint8 slice_id)
{
    uint32 cmd = 0;
    uint8 txqm = 0;
    uint8 speed = 0;
    uint8 array_ct = 0;
    uint16 lport = 0;
    uint16 divisor = 0;
    uint16 array[8] = {0};
    uint16 xg_speed[4] = {0};
    uint16 xlg_speed[4] = {0};
    uint16 cg_speed[4] = {0};
    uint16 net_speed[4] = {0};
    uint32 tmp_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    NetRxWrrWeight_m wrr_cfg;

    cmd = DRV_IOR((NetRxWrrWeight_t+slice_id), DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (!port_attr)
            {
                continue;
            }

            if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
            {
                /*rsv port have no channel info*/
                continue;
            }

            SYS_DATAPATH_MODE_TO_SPEED(port_attr->speed_mode, speed);
            if ((port_attr->speed_mode <= CTC_PORT_SPEED_10G)||(CTC_PORT_SPEED_5G == port_attr->speed_mode))
            {
                xg_speed[lport/16] += speed;
            }
            else if ((port_attr->speed_mode <= CTC_PORT_SPEED_50G)&&(port_attr->speed_mode != CTC_PORT_SPEED_100G))
            {
                xlg_speed[lport/16] += speed;
            }
            else if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
            {
                cg_speed[lport/16] += speed;
            }
        }

        for (txqm = 0; txqm < 4; txqm++)
        {
            net_speed[txqm] = xg_speed[txqm] + xlg_speed[txqm] + cg_speed[txqm];
            array[array_ct++] = net_speed[txqm];
        }

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array, array_ct, &divisor));

        for (txqm = 0; txqm < 4; txqm++)
        {
            tmp_val32 = net_speed[txqm]/divisor;
            DRV_IOW_FIELD(lchip, (NetRxWrrWeight_t+slice_id), (NetRxWrrWeight_cfgNetWorkWeight0_f+txqm),
                &tmp_val32, &wrr_cfg);
        }
        cmd = DRV_IOW((NetRxWrrWeight_t+slice_id), DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_netrx_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    NetRxChannelMap_m chan_map;
    NetRxToIpeCombNum_m comb_num;
    IpeHeaderAdjustPhyPortMap_m port_map;
    sys_datapath_lport_attr_t* port_attr = NULL;

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        /*1. init channel-mac-lport mapping*/
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }

            /* mac to channel */
            tmp_val32 = port_attr->chan_id;
            DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgPortToChanMapping_f,
                &tmp_val32, &chan_map);
            cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->mac_id, cmd, &chan_map);

            tmp_val32 = port_attr->mac_id;
            DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgChanToPortMapping_f,
                &tmp_val32, &chan_map);
            cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &chan_map);

            /* channel to lport */
            tmp_val32 = lport;
            DRV_IOW_FIELD(lchip, (IpeHeaderAdjustPhyPortMap_t+slice_id), IpeHeaderAdjustPhyPortMap_localPhyPort_f,
                &tmp_val32, &port_map);
            cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &port_map);

            /* NetRxToIpeCombNum */
            tmp_val32 = 0x3;
            DRV_IOW_FIELD(lchip, (NetRxToIpeCombNum_t+slice_id), NetRxToIpeCombNum_data_f,
                &tmp_val32, &comb_num);
            cmd = DRV_IOW((NetRxToIpeCombNum_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->mac_id, cmd, &comb_num);
        }

        /* 2. init NetRxBufManagement */
        CTC_ERROR_RETURN(_sys_usw_datapath_set_netrx_buf_mgr(lchip, slice_id));

        /* 3. init NetRx Wrr weight*/
        CTC_ERROR_RETURN(_sys_usw_datapath_set_netrx_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_mac_init(uint8 lchip)
{
    uint8 index = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  step  = 0;
    SharedMii0Cfg0_m  mii_cfg;

    sal_memset(&mii_cfg, 0, sizeof(SharedMii0Cfg0_m));

    if (0 == SDK_WORK_PLATFORM)
    {
        /* link status filter */
        value = 1;
        for (index = 0; index < 16; index++)
        {
            step = SharedMii0Cfg1_t - SharedMii0Cfg0_t;
            tbl_id = (SharedMii0Cfg0_t+step*index);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
            fld_id = SharedMii0Cfg0_cfgMiiRxLinkFilterEn0_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &mii_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));

            step = SharedMii1Cfg1_t - SharedMii1Cfg0_t;
            tbl_id = (SharedMii1Cfg0_t+step*index);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
            fld_id = SharedMii1Cfg0_cfgMiiRxLinkFilterEn1_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &mii_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));

            step = SharedMii2Cfg1_t - SharedMii2Cfg0_t;
            tbl_id = (SharedMii2Cfg0_t+step*index);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
            fld_id = SharedMii2Cfg0_cfgMiiRxLinkFilterEn2_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &mii_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));

            step = SharedMii3Cfg1_t - SharedMii3Cfg0_t;
            tbl_id = (SharedMii3Cfg0_t+step*index);
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
            fld_id = SharedMii3Cfg0_cfgMiiRxLinkFilterEn3_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &mii_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_wlan_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 index = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    RaCpwpCipDecPlConfig_m pl_cfg;
    RaCpwpCipInBufCreditDecCfg_m credit_cfg;
    RaCpwpCipMidBufDecConfig_m buf_cfg;

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (index = 0; index < 4; index++)
        {
            tmp_val32 = index * 20;
            DRV_IOW_FIELD(lchip, (RaCpwpCipDecPlConfig_t+slice_id), RaCpwpCipDecPlConfig_startPtr_f,
                &tmp_val32, &pl_cfg);
            tmp_val32 = index * 20 + 19;
            DRV_IOW_FIELD(lchip, (RaCpwpCipDecPlConfig_t+slice_id), RaCpwpCipDecPlConfig_endPtr_f,
                &tmp_val32, &pl_cfg);
            cmd = DRV_IOW(RaCpwpCipDecPlConfig_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &pl_cfg);

            tmp_val32 = 36;
            DRV_IOW_FIELD(lchip, (RaCpwpCipInBufCreditDecCfg_t+slice_id), RaCpwpCipInBufCreditDecCfg_creditCfg_f,
                &tmp_val32, &credit_cfg);
            cmd = DRV_IOW(RaCpwpCipInBufCreditDecCfg_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &credit_cfg);

            tmp_val32 = 16 + index * 40;
            DRV_IOW_FIELD(lchip, (RaCpwpCipMidBufDecConfig_t+slice_id), RaCpwpCipMidBufDecConfig_startPtr_f,
                &tmp_val32, &buf_cfg);
            tmp_val32 = 55 + index * 40;
            DRV_IOW_FIELD(lchip, (RaCpwpCipMidBufDecConfig_t+slice_id), RaCpwpCipMidBufDecConfig_endPtr_f,
                &tmp_val32, &buf_cfg);
            cmd = DRV_IOW(RaCpwpCipMidBufDecConfig_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &buf_cfg);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_datapth_init(uint8 lchip)
{
    /*1. netrx init */
    CTC_ERROR_RETURN(_sys_usw_datapath_netrx_init(lchip));

    /*2. Qmgr init */
    CTC_ERROR_RETURN(_sys_usw_datapath_qmgr_init(lchip));

    /*3. bufretrv init */
    CTC_ERROR_RETURN(_sys_usw_datapath_bufretrv_init(lchip));

    /*4. chan init */
    CTC_ERROR_RETURN(_sys_usw_datapath_net_chan_cfg(lchip));
    CTC_ERROR_RETURN(_sys_usw_datapath_misc_chan_init(lchip));

    /*5. epe init */
    CTC_ERROR_RETURN(_sys_usw_datapath_epe_init(lchip));

    /*6. nettx init */
    CTC_ERROR_RETURN(_sys_usw_datapath_nettx_init(lchip));

    /*7. wlan init */
    CTC_ERROR_RETURN(_sys_usw_datapath_wlan_init(lchip));

    /*8. mac init*/
    CTC_ERROR_RETURN(_sys_usw_datapath_mac_init(lchip));

    /*9. misc init */
    CTC_ERROR_RETURN(_sys_usw_datapath_misc_init(lchip));

    return CTC_E_NONE;
}

int32
sys_usw_datapath_module_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 index = 0;
    uint8 slice_id = 0;
    uint32 fld_id = 0;
    uint32 value = 0;
    uint8 max_sgmac_num = (DRV_IS_TSINGMA(lchip) ? TM_SGMAC_NUM : 16);

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        value = 1;

        for (index = 0; index < max_sgmac_num; index++)
        {
            fld_id = 0;
            cmd = DRV_IOW((QuadSgmacInit0_t+index+slice_id*max_sgmac_num), fld_id);
            DRV_IOCTL(lchip, 0, cmd, &value);
                               //index+slice_id*max_sgmac_num, "init", 1);
        }

        sys_usw_ftm_misc_config(lchip);

        /*do BufRetrvInit init*/
        cmd = DRV_IOW((BufRetrvInit_t+slice_id), BufRetrvInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        /*do bufstore init*/
        cmd = DRV_IOW(BufStoreInit_t, BufStoreInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        cmd = DRV_IOW((CapwapCipherInit_t+slice_id), CapwapCipherInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((CapwapProcInit_t+slice_id), CapwapProcInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((CoppInit_t+slice_id), CoppInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((DlbInit_t+slice_id), DlbInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((DsAgingInit_t+slice_id), DsAgingInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((DynamicAdInit_t+slice_id), DynamicAdInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((DynamicEditInit_t+slice_id), DynamicEditInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((DynamicKeyInit_t+slice_id), DynamicKeyInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EfdInit_t+slice_id), EfdInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        cmd = DRV_IOW((EgrOamHashInit_t+slice_id), EgrOamHashInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeAclOamInit_t+slice_id), EpeAclOamInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeScheduleInit_t+slice_id), EpeScheduleInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeHdrAdjInit_t+slice_id), EpeHdrAdjInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeHdrEditInit_t+slice_id), EpeHdrEditInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeHdrProcInit_t+slice_id), EpeHdrProcInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((EpeNextHopInit_t+slice_id), EpeNextHopInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FibAccInit_t+slice_id), FibAccInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FibEngineInit_t+slice_id), FibEngineInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FlowAccInit_t+slice_id), FlowAccInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FlowAccInit0_t+slice_id), FlowAccInit0_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FlowAccInit1_t+slice_id), FlowAccInit1_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FlowHashInit_t+slice_id), FlowHashInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((FlowTcamInit_t+slice_id), FlowTcamInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((GlobalStatsInit_t+slice_id), GlobalStatsInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((IpfixHashInit0_t+slice_id), IpfixHashInit0_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((IpfixHashInit1_t+slice_id), IpfixHashInit1_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(( ParserInit_t+slice_id),  ParserInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(( ParserInit1_t+slice_id),  ParserInit1_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(( ParserInit2_t+slice_id),  ParserInit2_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        /*do ipe init*/
        cmd = DRV_IOW(IpeFwdInit_t+slice_id, IpeFwdInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeHdrAdjInit_t+slice_id, IpeHdrAdjInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeIntfMapInit_t+slice_id, IpeIntfMapInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeLkupMgrInit_t+slice_id, IpeLkupMgrInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpePktProcInit_t+slice_id, IpePktProcInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((LinkAggInit_t+slice_id), LinkAggInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((LpmTcamInit_t+slice_id), LpmTcamInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((MacSecEngineInit_t+slice_id), MacSecEngineInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((MetFifoInit_t+slice_id), MetFifoInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(NetRxInit_t+slice_id, NetRxInit_init_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW((NetTxInit_t+slice_id), NetTxInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((OamAutoGenPktInit_t+slice_id), OamAutoGenPktInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((OamFwdInit_t+slice_id), OamFwdInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((OamProcInit_t+slice_id), OamProcInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((OobFcInit_t+slice_id), OobFcInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((PolicingEpeInit_t+slice_id), PolicingEpeInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((PolicingIpeInit_t+slice_id), PolicingIpeInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        /*do QMgrEnq init*/
        cmd = DRV_IOW(QMgrEnqInit_t, QMgrEnqInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((QMgrDeqInit_t+slice_id), QMgrDeqInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW((QMgrDeqInit_t+slice_id), QMgrDeqInit_stallInit_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(QMgrMsgStoreInit_t, QMgrMsgStoreInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        cmd = DRV_IOW((StormCtlInit_t+slice_id), StormCtlInit_init_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        if(DRV_IS_TSINGMA(lchip))
        {
            cmd = DRV_IOW((FibIntHashInit_t+slice_id), FibIntHashInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW(IpeAclInit_t+slice_id, IpeAclInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((MdioInit_t+slice_id), MdioInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((DmaCtlInit_t+slice_id), DmaCtlInit_dmaInit_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((ShareBufferInit_t+slice_id), ShareBufferInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW(( MplsHashInit_t+slice_id),  MplsHashInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
        }
        else if(DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOW((FibHashInit_t+slice_id), FibHashInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((MdioInit0_t+slice_id), MdioInit0_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((MdioInit1_t+slice_id), MdioInit1_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((PbCtlInit_t+slice_id), PbCtlInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((UserIdHashInit_t+slice_id), UserIdHashInit_init_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW(DsStatsEdramRefreshCtl_t, DsStatsEdramRefreshCtl_refEn_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((GlobalStatsEdramInit_t+slice_id), GlobalStatsEdramInit_edramInit_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            cmd = DRV_IOW((EpeHdrEditInit_t+slice_id), EpeHdrEditInit_extDsInit_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
        }
    }

    sal_task_sleep(1000);

    if (0 == SDK_WORK_PLATFORM)
    {
        /*2. check init done */
        for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
        {
            fld_id = 0;
            for (index = 0; index < max_sgmac_num; index++)
            {
                cmd = DRV_IOR((QuadSgmacInitDone0_t+index+slice_id*max_sgmac_num), fld_id);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
            }

            cmd = DRV_IOR((BufStoreInitDone_t+slice_id), BufStoreInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR((BufRetrvInitDone_t+slice_id), BufRetrvInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR((CapwapCipherInitDone_t+slice_id), CapwapCipherInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((CapwapProcInitDone_t+slice_id), CapwapProcInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((CoppInitDone_t+slice_id), CoppInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((DlbInitDone_t+slice_id), DlbInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((DsAgingInitDone_t+slice_id), DsAgingInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((DynamicAdInitDone_t+slice_id), DynamicAdInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((DynamicEditInitDone_t+slice_id), DynamicEditInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((DynamicKeyInitDone_t+slice_id), DynamicKeyInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EfdInitDone_t+slice_id), EfdInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EgrOamHashInitDone_t+slice_id), EgrOamHashInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeAclOamInitDone_t+slice_id), EpeAclOamInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeScheduleInitDone_t+slice_id), EpeScheduleInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeHdrAdjInitDone_t+slice_id), EpeHdrAdjInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeHdrEditInitDone_t+slice_id), EpeHdrEditInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeHdrProcInitDone_t+slice_id), EpeHdrProcInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((EpeNextHopInitDone_t+slice_id), EpeNextHopInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR((FibAccInitDone_t+slice_id), FibAccInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((FibEngineInitDone_t+slice_id), FibEngineInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((FlowAccInitDone_t+slice_id), FlowAccInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((FlowHashInitDone_t+slice_id), FlowHashInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((FlowTcamInitDone_t+slice_id), FlowTcamInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((GlobalStatsInitDone_t+slice_id), GlobalStatsInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(IpeFwdInitDone_t+slice_id, IpeFwdInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR(IpeHdrAdjInitDone_t+slice_id, IpeHdrAdjInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(IpeIntfMapInitDone_t+slice_id, IpeIntfMapInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(IpeLkupMgrInitDone_t+slice_id, IpeLkupMgrInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(IpePktProcInitDone_t+slice_id, IpePktProcInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((LinkAggInitDone_t+slice_id), LinkAggInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((LpmTcamInitDone_t+slice_id), LpmTcamInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR((MacSecEngineInitDone_t+slice_id), MacSecEngineInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((MetFifoInitDone_t+slice_id), MetFifoInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(NetRxInitDone_t+slice_id, NetRxInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((NetTxInitDone_t+slice_id), NetTxInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((OamAutoGenPktInitDone_t+slice_id), OamAutoGenPktInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((OamFwdInitDone_t+slice_id), OamFwdInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((OamProcInitDone_t+slice_id), OamProcInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((OobFcInitDone_t+slice_id), OobFcInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((PolicingEpeInitDone_t+slice_id), PolicingEpeInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((PolicingIpeInitDone_t+slice_id), PolicingIpeInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((QMgrDeqInitDone_t+slice_id), QMgrDeqInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((QMgrDeqInitDone_t+slice_id), QMgrDeqInitDone_stallInitDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR(QMgrEnqInitDone_t, QMgrEnqInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR(QMgrMsgStoreInitDone_t, QMgrMsgStoreInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((StormCtlInitDone_t+slice_id), StormCtlInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            cmd = DRV_IOR((FlowAccInitDone0_t+slice_id), FlowAccInitDone0_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((FlowAccInitDone1_t+slice_id), FlowAccInitDone1_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((IpfixHashInitDone0_t+slice_id), IpfixHashInitDone0_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((IpfixHashInitDone1_t+slice_id), IpfixHashInitDone1_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((ParserInitDone_t+slice_id), ParserInitDone_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((ParserInitDone1_t+slice_id), ParserInitDone1_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            cmd = DRV_IOR((ParserInitDone2_t+slice_id), ParserInitDone2_initDone_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            if (!value)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }

            if(DRV_IS_TSINGMA(lchip))
            {
                cmd = DRV_IOR((FibIntHashInitDone_t+slice_id), FibIntHashInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR(IpeAclInitDone_t+slice_id, IpeAclInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((MdioInitDone_t+slice_id), MdioInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }

                if((CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
                {
                    /*when wb reloading, do not check DmaCtlInitDone*/
                }
                else
                {
                    cmd = DRV_IOR(DmaCtlInitDone_t, DmaCtlInitDone_dmaInitDone_f);
                    DRV_IOCTL(lchip, 0, cmd, &value);
                    if (!value)
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                        return CTC_E_NOT_INIT;
                    }
                }
                cmd = DRV_IOR(ShareBufferInitDone_t, ShareBufferInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR(MplsHashInitDone_t, MplsHashInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
            }
            else if(DRV_IS_DUET2(lchip))
            {
                cmd = DRV_IOR((FibHashInitDone_t+slice_id), FibHashInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((GlobalStatsEdramInitDone_t+slice_id), GlobalStatsEdramInitDone_edramInitDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((MdioInitDone0_t+slice_id), MdioInitDone0_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((MdioInitDone1_t+slice_id), MdioInitDone1_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((PbCtlInitDone_t+slice_id), PbCtlInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((UserIdHashInitDone_t+slice_id), UserIdHashInitDone_initDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
                cmd = DRV_IOR((EpeHdrEditInitDone_t+slice_id), EpeHdrEditInitDone_extDsInitDone_f);
                DRV_IOCTL(lchip, 0, cmd, &value);
                if (!value)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }
            }
        }

        value = 1;
        cmd = DRV_IOW(RefDivQMgrDeqShpPulse_t, RefDivQMgrDeqShpPulse_cfgResetDivQMgrDeqNetShpPulse_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(RefDivQMgrDeqShpPulse_t, RefDivQMgrDeqShpPulse_cfgResetDivQMgrDeqMiscShpPulse_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(RefDivQMgrDeqShpPulse_t, RefDivQMgrDeqShpPulse_cfgResetDivQMgrDeqExtShpPulse_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_serdes_prbs_rx(uint8 lchip, ctc_chip_serdes_prbs_t* p_prbs)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   serdes_id = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint32 tb_id = 0;
    uint32 cmd = 0;
    uint32 table_value = 0;

    serdes_id = p_prbs->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    /* relese hscfg.cfgRx#PrbsRst */
    if (p_hss->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
    {
        tb_id = HsCfg0_t + p_hss->hss_id;
        table_value = 0;
        cmd = DRV_IOW(tb_id, HsCfg0_cfgHss15gRxAPrbsRst_f+lane_id*8);
        DRV_IOCTL(lchip, 0, cmd, &table_value);
    }
    else
    {
        tb_id = CsCfg0_t + p_hss->hss_id;
        table_value = 0;
        cmd = DRV_IOW(tb_id, CsCfg0_cfgHss28gRxAPrbsRst_f+lane_id*10);
        DRV_IOCTL(lchip, 0, cmd, &table_value);
    }


    /* cfg Receiver Test Control Register, offset 0x01 */
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 1);

    /* set PATSEL(bit2:0) */
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value |= 0x8;
    value = (value&0xfff8) + (p_prbs->polynome_type);
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* reset, PRST(bit4)*/
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value |= 0x10;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    sal_task_sleep(1);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value &= 0xffef;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* read result */
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    p_prbs->value = (uint8)((value & 0x300)>>8);

    /*disable prbs rx check*/
    value &= 0xfff7;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* reset, PRST(bit4)*/
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value |= 0x10;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value &= 0xffef;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* reset hscfg.cfgRx#PrbsRst */
    if (p_hss->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
    {
        tb_id = HsCfg0_t + p_hss->hss_id;
        table_value = 1;
        cmd = DRV_IOW(tb_id, HsCfg0_cfgHss15gRxAPrbsRst_f+lane_id*8);
        DRV_IOCTL(lchip, 0, cmd, &table_value);
    }
    else
    {
        tb_id = CsCfg0_t + p_hss->hss_id;
        table_value = 1;
        cmd = DRV_IOW(tb_id, CsCfg0_cfgHss28gRxAPrbsRst_f+lane_id*10);
        DRV_IOCTL(lchip, 0, cmd, &table_value);
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_tx_en(uint8 lchip, uint16 serdes_id, bool enable)
{
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint8  hss_type = 0;
    uint8  internal_lane = 0;
    uint16 address = 0;
    uint16 value = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x03);

    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(enable)
    {
        value &= 0xffdf;
    }
    else
    {
        value |= 0x0020;
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_prbs_tx(uint8 lchip, void* p_data)
{
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint8  hss_type = 0;
    uint8  internal_lane = 0;
    uint16 address = 0;
    uint16 value = 0;
    ctc_chip_serdes_prbs_t* p_prbs = (ctc_chip_serdes_prbs_t*)p_data;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    serdes_id = p_prbs->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    /* cfg Transmit Test Control Register, offset 0x01 */
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 1);

    /* set SPSEL(bit13:11)=000 */
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value &= 0xc3ff;

    /* value: 1--enable, 0--disbale */
    if(p_prbs->value)
    {
        /* set TPGMD(bit3)=1 */
        value |= 0x8;
    }
    else
    {
        /* set TPGMD(bit3)=0 */
        value &= 0xfff7;
    }

    /* set TPSEL(bit2:0) */
    value = (value&0xfff8) + p_prbs->polynome_type;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* reset, PRST(bit4)*/
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value |= 0x10;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    sal_task_sleep(1);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value &= 0xffef;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_prbs(uint8 lchip, void* p_data)
{
    uint32 enable;
    uint8 hss_idx = 0;
    uint16 drv_port = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 lane_id = 0;
    uint8 slice_id = 0;
    ctc_chip_serdes_prbs_t* p_prbs = (ctc_chip_serdes_prbs_t*)p_data;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    CTC_PTR_VALID_CHECK(p_prbs);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, value:%d, type:%d, mode:%d\n", p_prbs->serdes_id, p_prbs->value, p_prbs->mode, p_prbs->polynome_type);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_prbs->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_prbs->serdes_id);
    slice_id = (p_prbs->serdes_id >= SERDES_NUM_PER_SLICE)?1:0;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];
    drv_port = p_serdes->lport + slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, drv_port);
    /* check, if mac disable, can not do prbs */
    CTC_ERROR_RETURN(sys_usw_mac_get_mac_en(lchip, gport, &enable));
    if (FALSE == enable)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " mac is not enable \n");
            return CTC_E_INVALID_CONFIG;

    }

    switch(p_prbs->mode)
    {
        case 0: /* 0--Rx */
            CTC_ERROR_RETURN(_sys_usw_datapath_set_serdes_prbs_rx(lchip, p_prbs));
            break;
        case 1: /* 1--Tx */
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_prbs_tx(lchip, p_prbs));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_serdes_loopback_inter(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8  hss_id    = 0;
    uint8  hss_idx   = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint8  slice_id  = 0;
    uint8  internal_lane = 0;
    uint8  hss_type = 0;
    uint16 address = 0;
    uint16 value = 0;
    uint32 value_pcs =0;
    uint32 cmd              = 0;
    uint8  index            = 0;
    uint32 tbl_id           = 0;
    uint32 fld_id           = 0;
    uint8  step             = 0;
    uint32 tmp_val32        = 0;
    uint8  mac_id           = 0;
    uint8 internal_mac_idx  = 0;
    uint8 sgmac_idx         = 0;
    uint8  mac_num          = 0;
    uint8  i                = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;
    ctc_chip_serdes_polarity_t polarity;
    uint8  flag = 0;
    /* SYSTEM MODIFIED by qicx, bug44302, 2017-06-09 */
    uint32 gport = 0;
    uint32 enable = 0;
    /* SYSTEM MODIFIED by qicx end */
    SharedPcsSerdes0Cfg0_m       serdes_cfg;
    QsgmiiPcsCfg0_m              qsgmii_serdes_cfg;
    UsxgmiiPcsSerdesCfg0_m       usxgmii_serdes_cfg;
    HsMacroCfg0_m                hs_macro;
    CsMacroCfg0_m                cs_macro;
    Sgmac0RxCfg0_m               mac_rx_cfg;

    sal_memset(&polarity, 0, sizeof(ctc_chip_serdes_polarity_t));
    sal_memset(&serdes_cfg, 0, sizeof(SharedPcsSerdes0Cfg0_m));
    sal_memset(&qsgmii_serdes_cfg, 0, sizeof(QsgmiiPcsCfg0_m));
    sal_memset(&usxgmii_serdes_cfg, 0, sizeof(UsxgmiiPcsSerdesCfg0_m));
    sal_memset(&hs_macro, 0, sizeof(HsMacroCfg0_m));
    sal_memset(&cs_macro, 0, sizeof(CsMacroCfg0_m));
    sal_memset(&mac_rx_cfg, 0, sizeof(Sgmac0RxCfg0_m));

    /* 1.get serdes and pcs info */
    serdes_id = p_loopback->serdes_id;
    slice_id = (serdes_id >= SERDES_NUM_PER_SLICE)?1:0;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id); /* notice: hss_id is different from hss_idx */
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }
    port_attr = (sys_datapath_lport_attr_t*)(&p_usw_datapath_master[lchip]->port_attr[slice_id][p_hss_vec->serdes_info[lane_id].lport]);

    if ((port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        if ((lane_id == 2) || (lane_id == 3) || (lane_id == 6) || (lane_id == 7))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"serdes %d not used in mode %d\n", serdes_id, port_attr->pcs_mode);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid configuration \n");
            return CTC_E_INVALID_PARAM;
        }
    }
    SYS_DATAPATH_GET_HW_MAC_BY_SERDES(serdes_id, p_hss_vec->serdes_info[lane_id].mode, mac_id, mac_num);
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tmp_val32 = 0;
    for (i = 0; i < mac_num; i++)
    {
        internal_mac_idx = (mac_id+i) % 4;
        sgmac_idx = (mac_id + i)/ 4;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxPktEn_f, &tmp_val32, &mac_rx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));
    }

    /* 2.cfg Receiver Test Control Register, offset 0x01 */
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 1);
    /* set WPLPEN(bit6) WRPMD(bit5), and do polarity swap */
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(p_loopback->enable)
    {
        value |= 0x60;
        /* if rx_polarity is reverse, should set it normal */
        if(1 == p_hss_vec->serdes_info[lane_id].rx_polarity)
        {
            polarity.dir = 0;
            polarity.polarity_mode = 0;
            flag |= 0x1; /* flag is use for record polarity_mode swap */
            polarity.serdes_id = serdes_id;
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &polarity));
        }
    }
    else
    {
        value &= 0xff9f;
        /* recovery rx_polarity status */
        if(1 == p_hss_vec->serdes_info[lane_id].rx_polarity)
        {
            polarity.dir = 0;
            polarity.polarity_mode = 1;
            polarity.serdes_id = serdes_id;
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &polarity));
        }
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* 3. cfg PMA TX reset of SGMII/QSGMII/SGMII2.5G */
    if ((port_attr->pcs_mode == CTC_CHIP_SERDES_SGMII_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_2DOT5G_MODE))
    {
        if (serdes_id < 24)
        {
            tbl_id = HsMacroCfg0_t + hss_id;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetHssTxD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetHssTxE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetHssTxF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetHssTxG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetHssTxH_f;
                break;
            default:
                fld_id = 0;
                break;
            }

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            tmp_val32 = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            sal_task_sleep(1);

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            tmp_val32 = 0;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
        }
        else
        {
            tbl_id = CsMacroCfg0_t + hss_id;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetHssTxD_f;
                break;
            default:
                fld_id = 0;
                break;
            }

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
            tmp_val32 = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &cs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            sal_task_sleep(1);

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
            tmp_val32 = 0;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &cs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        }
    }

    /* 4.cfg the force signal detect */
    if (port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
    {
        tbl_id = QsgmiiPcsCfg0_t + (serdes_id/4)*2 + serdes_id%4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &qsgmii_serdes_cfg));
        value_pcs = (p_loopback->enable)?1:0;
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsCfg0_forceSignalDetect_f, &value_pcs, &qsgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &qsgmii_serdes_cfg));
    }
    else if (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + (serdes_id/4)*2;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &usxgmii_serdes_cfg));
        value_pcs = (p_loopback->enable)?1:0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f+serdes_id%4, &value_pcs, &usxgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &usxgmii_serdes_cfg));
    }
    else if((port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + (serdes_id/4)*2 + serdes_id%4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &usxgmii_serdes_cfg));
        value_pcs = (p_loopback->enable)?1:0;
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f, &value_pcs, &usxgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &usxgmii_serdes_cfg));
    }
    else
    {
        step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
        tbl_id = SharedPcsSerdes0Cfg0_t + serdes_id/4 + (serdes_id%4)*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &serdes_cfg));
        value_pcs = (p_loopback->enable)?1:0;
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value_pcs, &serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &serdes_cfg));
    }

    SYS_DATAPATH_GET_HW_MAC_BY_SERDES(serdes_id, p_hss_vec->serdes_info[lane_id].mode, mac_id, mac_num);
    step = Sgmac1RxCfg0_t - Sgmac0RxCfg0_t;
    tmp_val32 = 1;
    for (i = 0; i < mac_num; i++)
    {
        internal_mac_idx = (mac_id+i) % 4;
        sgmac_idx = (mac_id + i)/ 4;
        tbl_id = Sgmac0RxCfg0_t + internal_mac_idx*step + sgmac_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, Sgmac0RxCfg0_cfgSgmac0RxPktEn_f, &tmp_val32, &mac_rx_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_rx_cfg));
    }

    /* 4.change sw by flag, used for enable */
    if(CTC_IS_BIT_SET(flag, 0))
    {
        p_hss_vec->serdes_info[lane_id].rx_polarity = 1;
        CTC_BIT_UNSET(flag, 0);
    }

    /* SYSTEM MODIFIED by qicx, bug44302, 2017-06-09 */
    /* 5. mac enable, disable send */
    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, serdes_id, &gport));
    CTC_ERROR_RETURN(sys_usw_mac_get_mac_en(lchip, gport, &enable));
    if (TRUE == enable)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, serdes_id, !(p_loopback->enable)));
    }
    /* SYSTEM MODIFIED by qicx end */
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_serdes_loopback_exter(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint8  hss_type = 0;
    uint8  internal_lane = 0;
    uint16 address = 0;
    uint16 value = 0;
    uint32 gport = 0;
    uint32 enable = 0;

    /* 1.get serdes and pcs info */
    serdes_id = p_loopback->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id); /* notice: hss_id is different from hss_idx */
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    /* 2.cfg Transmit Configuration Mode Register, offset 0x00 */
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
    /* set Hss15g RXLOOP(bit8), and Hss28g RXLOOP(bit5) */
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(hss_type)
    {
        if(p_loopback->enable)
        {
            value |= 0x20;
        }
        else
        {
            value &= 0xffdf;
        }
        drv_chip_write_hss28g(lchip, hss_id, address, value);
    }
    else
    {
        if(p_loopback->enable)
        {
            value |= 0x100;
        }
        else
        {
            value &= 0xfeff;
        }
        drv_chip_write_hss15g(lchip, hss_id, address, value);
    }

    /* 3. mac disable, enable send */
    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, serdes_id, &gport));
    CTC_ERROR_RETURN(sys_usw_mac_get_mac_en(lchip, gport, &enable));
    if (FALSE == enable)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_tx_en(lchip, serdes_id, p_loopback->enable));
    }

    return CTC_E_NONE;
}

/* serdes internal and external loopback */
int32
sys_usw_datapath_set_serdes_loopback(uint8 lchip, void* p_data)
{
    uint32 gport = 0;
    uint16 lport = 0;
    uint8  hss_idx   = 0;
    uint8  lane_id   = 0;
    uint32 an_en = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    ctc_chip_serdes_loopback_t* p_loopback = (ctc_chip_serdes_loopback_t*)p_data;
    ctc_chip_serdes_loopback_t lb_param;
    uint8 il_sta = 0;  /* internal loopback status */
    uint8 el_sta = 0;  /* external loopback status */

    CTC_PTR_VALID_CHECK(p_loopback);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d, type:%d\n", p_loopback->serdes_id, p_loopback->enable, p_loopback->mode);

    if(SERDES_NUM_PER_SLICE <= p_loopback->serdes_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_datapath_set_serdes_loopback: serdes id %d out of bound!\n", p_loopback->serdes_id);
        return CTC_E_INVALID_PARAM;
    }

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_loopback->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_loopback->serdes_id);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }
    /* filter serdes none mode */
    if (CTC_CHIP_SERDES_NONE_MODE == p_hss_vec->serdes_info[lane_id].mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cannot set serdes loopback while mode is NULL \n");
        return CTC_E_INVALID_CONFIG;
    }
    /* filter duplicated config and additional config */
    sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
    lb_param.serdes_id = p_loopback->serdes_id;
    lb_param.mode = 0; /* get internal */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
    if (lb_param.enable)
    {
        il_sta = 1;
    }
    lb_param.mode = 1; /* get external */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
    if (lb_param.enable)
    {
        el_sta = 1;
    }
    if (p_loopback->enable)
    {
        if (1 == il_sta) /* current internal loopback mode */
        {
            if (0 == p_loopback->mode)
            {
                /* same config, do nothing */
                return CTC_E_NONE;
            }
            else if (1 == p_loopback->mode)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cannot set serdes loopback both internal and external \n");
                return CTC_E_INVALID_CONFIG;
            }
        }
        if (1 == el_sta) /* current external loopback mode */
        {
            if (1 == p_loopback->mode)
            {
                /* same config, do nothing */
                return CTC_E_NONE;
            }
            else if (0 == p_loopback->mode)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cannot set serdes loopback both internal and external \n");
                return CTC_E_INVALID_CONFIG;
            }
        }
    }
    else  /* disable */
    {
        if (!il_sta && !el_sta)
        {
            /* same config, do nothing */
            return CTC_E_NONE;
        }
    }
    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, p_loopback->serdes_id, &gport));
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    CTC_ERROR_RETURN(_sys_usw_mac_get_cl73_auto_neg(lchip, lport, CTC_PORT_PROP_AUTO_NEG_EN, &an_en));
    if (an_en)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] CL73 Auto-Nego is enable cannot set serdes loopback \n");
        return CTC_E_INVALID_CONFIG;
    }

    switch(p_loopback->mode)
    {
        case 0: /* 0--interal */
            CTC_ERROR_RETURN(_sys_usw_datapath_set_serdes_loopback_inter(lchip, p_loopback));
            break;
        case 1: /* 1--external */
            CTC_ERROR_RETURN(_sys_usw_datapath_set_serdes_loopback_exter(lchip, p_loopback));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}


/* serdes internal and external loopback */
int32
sys_usw_datapath_get_serdes_loopback(uint8 lchip, void* p_data)
{
    uint8  hss_id    = 0;
    uint8  hss_idx   = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint8  internal_lane = 0;
    uint8  hss_type = 0;
    uint16 address = 0;
    uint16 value = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    ctc_chip_serdes_loopback_t* p_loopback = (ctc_chip_serdes_loopback_t*)p_data;
    CTC_PTR_VALID_CHECK(p_loopback);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d, type:%d\n", p_loopback->serdes_id, p_loopback->enable, p_loopback->mode);

    serdes_id = p_loopback->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id); /* notice: hss_id is different from hss_idx */
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }

    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    switch(p_loopback->mode)
    {
        case 0: /* 0--interal */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 1);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        if ((value & 0x60) == 0x60)
        {
            p_loopback->enable = 1;
        }
        else
        {
            p_loopback->enable = 0;
        }
        break;

        case 1: /* 1--external */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        if(hss_type)
        {
            if((value & 0x20) == 0x20)
            {
                p_loopback->enable = 1;
            }
            else
            {
                p_loopback->enable = 0;
            }
        }
        else
        {
            if ((value & 0x100) == 0x100)
            {
                p_loopback->enable = 1;
            }
            else
            {
                p_loopback->enable = 0;
            }
        }
        break;

        default:
            return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_cfg(uint8 lchip, uint32 chip_prop, void* p_data)
{
    ctc_chip_serdes_cfg_t* p_serdes_cfg = (ctc_chip_serdes_cfg_t*)p_data;
    uint16 address = 0;
    uint16 value  = 0;
    uint8 serdes_id = 0;
    uint8 hss_id = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 internal_lane = 0;
    uint8   hss_type = 0;

    CTC_PTR_VALID_CHECK(p_serdes_cfg);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, value:%d\n", p_serdes_cfg->serdes_id, p_serdes_cfg->value);

    if (p_serdes_cfg->serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    serdes_id = p_serdes_cfg->serdes_id;
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);
    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    hss_id = p_hss->hss_id;

    switch(chip_prop)
    {
        case CTC_CHIP_PROP_SERDES_P_FLAG:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1b);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
            {
                return CTC_E_NOT_SUPPORT;
            }
            if (p_serdes_cfg->value)
            {
                value |= 0x60;
            }
            else
            {
                value &= 0x9f;
            }
            break;
        case CTC_CHIP_PROP_SERDES_PEAK:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0b);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
            {
                if (p_serdes_cfg->value > 0x1F)
                {
                    return CTC_E_INVALID_PARAM;
                }
                /*[13:9]bit*/
                value &= 0xC1FF;
                value |= (p_serdes_cfg->value<<9);
            }
            else
            {
                if (p_serdes_cfg->value > 0x7)
                {
                    return CTC_E_INVALID_PARAM;
                }
                /*[10:8]bit*/
                value &= 0xF8FF;
                value |= (p_serdes_cfg->value<<8);
            }
            break;
        case CTC_CHIP_PROP_SERDES_DPC:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1f);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (p_serdes_cfg->value)
            {
                value |= (1<<15);
            }
            else
            {
                value &= ~(1<<15);
            }
            break;
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
            if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
            {
                if (p_serdes_cfg->value >= 2)
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                if (p_serdes_cfg->value >= 4)
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x3);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            value &= ~(3<<2);
            value |= (p_serdes_cfg->value<<2);
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;

    }

    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_serdes_cfg(uint8 lchip, uint32 chip_prop, void* p_data)
{
    ctc_chip_serdes_cfg_t* p_serdes_cfg = (ctc_chip_serdes_cfg_t*)p_data;
    uint16 address = 0;
    uint16 value  = 0;
    uint8 serdes_id = 0;
    uint8 hss_id = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint8 internal_lane = 0;
    uint8   hss_type = 0;

    CTC_PTR_VALID_CHECK(p_serdes_cfg);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    if (p_serdes_cfg->serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    serdes_id = p_serdes_cfg->serdes_id;
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    hss_id = p_hss->hss_id;
    switch(chip_prop)
    {
        case CTC_CHIP_PROP_SERDES_P_FLAG:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1b);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if ((value & 0x60) == 0x60)
            {
                p_serdes_cfg->value = 1;
            }
            else
            {
                p_serdes_cfg->value = 0;
            }
            break;
        case CTC_CHIP_PROP_SERDES_PEAK:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x0b);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
            {
                p_serdes_cfg->value = ((value>>9)&0x1F);
            }
            else
            {
                p_serdes_cfg->value = ((value>>8)&0x7);
            }
            break;
        case CTC_CHIP_PROP_SERDES_DPC:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x1f);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            if (CTC_IS_BIT_SET(value, 15))
            {
                p_serdes_cfg->value = 1;
            }
            else
            {
                p_serdes_cfg->value = 0;
            }
            break;
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x3);
            DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
            p_serdes_cfg->value = (value>>2)&0x3;
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;

    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_eqmode(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    /* debug */
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable:%d\n", serdes_id, enable);

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;

    }

    /* set EQMODE bit4*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(enable)
    {
        value |= 0x0010; /* used for 802.3ap mode ffe */
    }
    else
    {
        value &= 0xffef; /* used for traditional mode ffe */
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_serdes_ffe_status(uint8 lchip, uint8 serdes_id, uint8 mode, uint8* p_value)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    /* debug */
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    /* set EQMODE bit4*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if (mode == CTC_CHIP_SERDES_FFE_MODE_3AP && CTC_IS_BIT_SET(value, 4))
    {
        *p_value = 1; /* used for 802.3ap mode ffe */
    }
    else if (mode != CTC_CHIP_SERDES_FFE_MODE_3AP && !CTC_IS_BIT_SET(value, 4))
    {
        *p_value = 1; /* used for traditional mode ffe */

    }
    else
    {
        *p_value = 0;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"serdes_id:%d, status:%u\n", serdes_id, *p_value);

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_check_serdes_ffe(uint8 lchip, ctc_chip_serdes_ffe_t* p_ffe)
{
    uint16 c0 = 0;
    uint16 c1 = 0;
    uint16 c2 = 0;
    uint16 c3 = 0;

    c0 = p_ffe->coefficient[0];
    c1 = p_ffe->coefficient[1];
    c2 = p_ffe->coefficient[2];
    c3 = p_ffe->coefficient[3];

    if (CTC_CHIP_SERDES_FFE_MODE_TYPICAL == p_ffe->mode)
    {
        return CTC_E_NONE;
    }

    if(SYS_DATAPATH_SERDES_IS_HSS28G(p_ffe->serdes_id))
    {
        if ((c0+c1+c2+c3) > 48 )
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c0+c1+c2+c3 <= 48\n", p_ffe->serdes_id);
            return CTC_E_INVALID_PARAM;
        }

        if (CTC_CHIP_SERDES_FFE_MODE_3AP == p_ffe->mode)
        {
            if ((c2-c0-c1-c3) < 4)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c2-c0-c1-c3 >= 4\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }

            if (0 != c0)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c0=0\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if (c1 > 5)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c1<=5\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if ((c2 < 23) || (c2 > 40))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must 23<=c2<=40\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if (c3 > 14)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c3<=14\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if (c2 <= (c0+c1+c3))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c2>c0+c1+c3\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    else
    {
        if ((c0+c1+c2) > 60)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c0+c1+c2<=60\n", p_ffe->serdes_id);
            return CTC_E_INVALID_PARAM;
        }

        if(CTC_CHIP_SERDES_FFE_MODE_3AP == p_ffe->mode)
        {
            if ((c0+c2) >= c1)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c1>c0+c2\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }

            if (c0>15)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c0<=15\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if (c1>60)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c1<=60\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if (c2>30)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c2<=30\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            if (c0>25)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c0<=25\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if ((c1>60) || (c1 <13))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must 13<=c1<=60\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
            if (c2>48)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"serdes_id:%d, the ffe param must c2<=48\n", p_ffe->serdes_id);
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    return CTC_E_NONE;
}

/* set serdes ffe for 802.3ap mode */
int32
sys_usw_datapath_set_serdes_ffe_3ap(uint8 lchip, ctc_chip_serdes_ffe_t* p_ffe)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   serdes_id = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    /* debug param */
    CTC_PTR_VALID_CHECK(p_ffe);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, C0:%d, C1:%d, C2:%d, C3:%d\n", p_ffe->serdes_id, p_ffe->coefficient[0], p_ffe->coefficient[1], p_ffe->coefficient[2], p_ffe->coefficient[3]);

    /* get serdes info by serdes id */
    serdes_id = p_ffe->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;

    }

    /* CFG AESRC, use pin(3ap train) or register FFE param */
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value &= 0xffdf;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* CFG c0 c1 c2 c3.... */
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        /* step 1, enable EQMODE */
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_eqmode(lchip, serdes_id, 1));

        /* step 2, cfg Vm Maximum Value Register, max +60 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x3d;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0xf;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 3, cfg V2 Limit Extended Register, min 0 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x00;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x11;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 4, cfg C0 Limit Extended Register: 0-0xa */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x11;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x05;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 5, cfg C1 Limit Extended Register 0xD-0x3C */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x3d00;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x09;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 6, cfg C2 Limit Extended Register 0-0x1F */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x0021;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x0d;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 7, cfg C0/C1/C2 Init Extended Register */
         /*C0 bit[4:0]*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[0];
        value = (~value);
        value &= 0x000f;
        value |= 0x0010;
        value += 1;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x03;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

         /*C1 bit[6:0]*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[1];
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x07;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

         /*C2 bit[5:0]*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[2];
        value = (~value);
        value &= 0x001f;
        value |= 0x0020;
        value += 1;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x0b;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 8, cfg Transmit 802.3ap Adaptive Equalization Command Register */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0e);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value |= 0x1000;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* setp 9, Read Status */

        /* step 10, write 0 to bits[13:0] 0f the command register to put all of the coefficients in the hold state */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0e);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0xc000;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    }
    else if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
    {
        /* step 1, cfg tx 0x02 bit[6:4]*/
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_eqmode(lchip, serdes_id, 1));

        /* step 2, cfg Vm Maximum Value Register, max +64 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x30;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0xf;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 3, cfg V2 Limit Extended Register, min 0 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x00;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x11;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 4, cfg C0 Limit Extended Register: 0-0xa */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x50;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x05;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* C1 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x50;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x09;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* C2 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x3000;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x0d;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* C3 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = 0x50;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x15;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 5, cfg C0/C1/C2/C3 Init Extended Register */
         /*C0*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[0];
        value = (~value);
        value &= 0x007f;
        value |= 0x0080;
        value += 1;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x03;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

         /*C1*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[1];
        value = (~value);
        value &= 0x007f;
        value |= 0x0080;
        value += 1;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x07;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

         /*C2*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[2];
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x0b;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

         /*C3*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1e);
        value = p_ffe->coefficient[3];
        value = (~value);
        value &= 0x007f;
        value |= 0x0080;
        value += 1;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1f);
        value = 0x13;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* step 6, cfg Transmit 802.3ap Adaptive Equalization Command Register */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0e);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value |= 0x1000;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

        /* setp 7, Read Status  */

        /* step 8, write 0 to bits[13:0] 0f the command register to put all of the coefficients in the hold state */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0e);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x0000;
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    }

    return CTC_E_NONE;
}

/* set serdes ffe for traditional sum mode */
int32
sys_usw_datapath_set_serdes_ffe_traditional(uint8 lchip, ctc_chip_serdes_ffe_t* p_ffe)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   serdes_id = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    uint16  coefficient[4] = {0};
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    /* debug param */
    CTC_PTR_VALID_CHECK(p_ffe);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, C0:%d, C1:%d, C2:%d, C3:%d\n", p_ffe->serdes_id, p_ffe->coefficient[0], p_ffe->coefficient[1], p_ffe->coefficient[2], p_ffe->coefficient[3]);

    /* get serdes info by serdes id */
    serdes_id = p_ffe->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] Hss can not ready \n");
            return CTC_E_HW_FAIL;

    }

    p_serdes = &p_hss->serdes_info[lane_id];

    coefficient[0] = p_ffe->coefficient[0];
    coefficient[1] = p_ffe->coefficient[1];
    coefficient[2] = p_ffe->coefficient[2];
    coefficient[3] = p_ffe->coefficient[3];

    /* disable EQMODE */
    CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_eqmode(lchip, serdes_id, 0));

    /* pff->mode==0 is typical mode, pff->mode==1 is user define mode */
    /* if is typical mode, c0,c1,c2,c3 value is set by motherboard material and trace length */
    /* TODO, need asic give typical mode cfg value */
    if(0 == p_ffe->mode)
    {
        /* for FR4 board */
        if (0 == p_ffe->board_material)
        {
            /* for 15G serdes */
            if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
            {
                /* trace length is 0~4inch c0:, c1:, c2:, c3: */
                if (p_ffe->trace_len == 0)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x19;
                    coefficient[2] = 0x12;
                }
                /* trace length is 4~7inch c0:, c1:, c2:, c3: */
                else if (p_ffe->trace_len == 1)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x19;
                    coefficient[2] = 0x18;
                }
                /* trace length is 7~8inch c0:, c1:, c2:, c3: */
                else if (p_ffe->trace_len == 2)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x19;
                    coefficient[2] = 0x1b;
                }
                /* trace length is 8~9inch c0:, c1:, c2:, c3: */
                else if (p_ffe->trace_len == 3)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x19;
                    coefficient[2] = 0x20;
                }
                /* trace length >9inch c0:, c1:, c2:, c3: */
                else
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x1a;
                    coefficient[2] = 0x20;
                }
            }
            /* for 28G serdes */
            else
            {
                /* in lo mode*/
                if((p_serdes->mode != CTC_CHIP_SERDES_CG_MODE)
                    &&(p_serdes->mode != CTC_CHIP_SERDES_XXVG_MODE)
                    &&(p_serdes->mode != CTC_CHIP_SERDES_LG_MODE))
                {
                    if (p_ffe->trace_len == 0)
                    {
                        coefficient[0] = 0x00;
                        coefficient[1] = 0x00;
                        coefficient[2] = 0x14;
                        coefficient[3] = 0x03;
                    }
                    else
                    {
                        coefficient[0] = 0x00;
                        coefficient[1] = 0x00;
                        coefficient[2] = 0x14;
                        coefficient[3] = 0x03;
                    }
                }
                /* in hi mode*/
                else
                {
                    if (p_ffe->trace_len == 0)
                    {
                        coefficient[0] = 0x00;
                        coefficient[1] = 0x04;
                        coefficient[2] = 0x23;
                        coefficient[3] = 0x08;
                    }
                    else
                    {
                        coefficient[0] = 0x00;
                        coefficient[1] = 0x04;
                        coefficient[2] = 0x23;
                        coefficient[3] = 0x08;
                    }
                }
            }
        }
        /* for M4 board. Just support FR4 and M4 two board material now.*/
        else
        {
            /* for 15G serdes */
            if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
            {
                /* trace length is 0~4inch c0:, c1:, c2:, c3: */
                if (p_ffe->trace_len == 0)
                {
                    coefficient[0] = 0x00;
                    coefficient[1] = 0x1a;
                    coefficient[2] = 0x0b;
                }
                /* trace length is 4~7inch c0:, c1:, c2:, c3: */
                else if (p_ffe->trace_len == 1)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x1c;
                    coefficient[2] = 0x0b;
                }
                /* trace length is 7~11inch c0:, c1:, c2:, c3: */
                else if (p_ffe->trace_len == 2)
                {
                    coefficient[0] = 0x02;
                    coefficient[1] = 0x1a;
                    coefficient[2] = 0x14;
                }
            }
            /* for 28G serdes */
            else
            {
                /* in lo mode*/
                if((p_serdes->mode != CTC_CHIP_SERDES_CG_MODE)
                    &&(p_serdes->mode != CTC_CHIP_SERDES_XXVG_MODE)
                    &&(p_serdes->mode != CTC_CHIP_SERDES_LG_MODE))
                {
                    /* For HSS28G, traditional config only support (0~4)inch */
                    coefficient[0] = 0x00;
                    coefficient[1] = 0x00;
                    coefficient[2] = 0x18;
                    coefficient[3] = 0x04;
                }
                /* in hi mode*/
                else
                {
                    /* For HSS28G, traditional config only support (0~4)inch */
                    coefficient[0] = 0x00;
                    coefficient[1] = 0x04;
                    coefficient[2] = 0x24;
                    coefficient[3] = 0x08;
                }
            }
        }
    }

    /* 0.cfg Transmit Tap 0 Coefficient Register, offset 0x08, set NXTT0(bit4:0) or NXTT0(bit6:0)*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x08);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        value &= 0xffe0;
        value += coefficient[0];
    }
    else
    {
        value &= 0xff80;
        value += coefficient[0];
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* 1.HSS28G cfg Transmit Tap 1 Coefficient Register, offset 0x09, set NXTT1(bit6:0) */
    /*   HSS15G cfg Transmit Amplitude Register, offset 0x0c, set TXPWR(bit5:0) */
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x09);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0xffc0;
        value += coefficient[1];
    }
    else
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x09);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0xff80;
        value += coefficient[1];
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* 2.cfg Transmit Tap 2 Coefficient Register, offset 0x0a, set NXTT2(bit5:0) or NXTT2(bit6:0)*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0a);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        value &= 0xffc0;
        value += coefficient[2];
    }
    else
    {
        value &= 0xff80;
        value += coefficient[2];
    }
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    /* 3.cfg Transmit Tap 3 Coefficient Register, offset 0x0b, set NXTT3(bit6:0), be used for HHS28G */
    if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0b);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0xff80;
        value += coefficient[3];
        DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);
    }

    /* 4.cfg Transmit Coefficient Control Register, offset 0x02, set ALOAD(bit0)*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    value |= 0x01;
    DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, value);

    return CTC_E_NONE;
}

/* set serdes ffe */
int32
sys_usw_datapath_set_serdes_ffe(uint8 lchip, void* p_data)
{
    ctc_chip_serdes_ffe_t* p_ffe = (ctc_chip_serdes_ffe_t*)p_data;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, mode:%d\n", p_ffe->serdes_id, p_ffe->mode);

    CTC_PTR_VALID_CHECK(p_ffe);
    if (p_ffe->serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_datapath_check_serdes_ffe(lchip, p_ffe));
    if(CTC_CHIP_SERDES_FFE_MODE_3AP == p_ffe->mode)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe_3ap(lchip, p_ffe));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe_traditional(lchip, p_ffe));
    }

    return CTC_E_NONE;
}

/* get serdes ffe with 3ap mode */
int32
sys_usw_datapath_get_serdes_ffe_3ap(uint8 lchip, uint8 serdes_id, uint16* coefficient)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    /* get c0 c1 c2 c3 value */
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        /* c0 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x18);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x1f;
        coefficient[0] = (value/2);

        /* c2 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x1a);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x3f;
        coefficient[2] = (value/2);

        /* c1 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x19);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x3f;
        coefficient[1] = ((value-coefficient[0])-coefficient[2]);
    }
    else if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
    {
        /* c0 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x10);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value = (~value);
        value += 1;
        value &= 0x7f;
        coefficient[0] = value;

        /* c1 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x11);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value = (~value);
        value += 1;
        value &= 0x7f;
        coefficient[1] = value;

        /* c2 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x12);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x7f;
        coefficient[2] = value;

        /* c3 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x13);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value = (~value);
        value += 1;
        value &= 0x7f;
        coefficient[3] = value;
    }

    return CTC_E_NONE;
}

/* get serdes ffe with traditional mode */
int32
sys_usw_datapath_get_serdes_ffe_traditional(uint8 lchip, uint8 serdes_id, uint16* coefficient)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint8   internal_lane = 0;
    uint8   hss_type = 0;
    uint32  address = 0;
    uint16  value = 0;
    ctc_chip_serdes_ffe_t ffe;

    sal_memset(&ffe, 0, sizeof(ctc_chip_serdes_ffe_t));

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(hss_type, lane_id);

    /* 0.get Transmit Tap 0 Coefficient Register, offset 0x08, set NXTT0(bit4:0) or NXTT0(bit6:0)*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x08);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        value &= 0x1f;
        coefficient[0] = value;
    }
    else
    {
        value &= 0x7f;
        coefficient[0] = value;
    }

    /* 1.HSS28G get Transmit Tap 1 Coefficient Register, offset 0x09, set NXTT1(bit6:0) */
    /*   HSS15G get Transmit Amplitude Register, offset 0x0c, set TXPWR(bit5:0) */
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x09);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x3f;
        coefficient[1] = value;
    }
    else
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x09);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x7f;
        coefficient[1] = value;
    }

    /* 2.get Transmit Tap 2 Coefficient Register, offset 0x0a, set NXTT2(bit5:0) or NXTT2(bit6:0)*/
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0a);
    DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
    if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
    {
        value &= 0x3f;
        coefficient[2] = value;
    }
    else
    {
        value &= 0x7f;
        coefficient[2] = value;
    }

    /* 3.get Transmit Tap 3 Coefficient Register, offset 0x0b, set NXTT3(bit6:0), be used for HHS28G */
    if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
    {
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0b);
        DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, &value);
        value &= 0x7f;
        coefficient[3] = value;
    }

    return CTC_E_NONE;
}

/* get serdes ffe */
int32
sys_usw_datapath_get_serdes_ffe(uint8 lchip, uint8 serdes_id, uint16* coefficient, uint8 mode, uint8* status)
{
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, ffe_mode:%d\n", serdes_id, mode);

    if (serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(CTC_CHIP_SERDES_FFE_MODE_3AP == mode)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe_3ap(lchip, serdes_id, coefficient));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe_traditional(lchip, serdes_id, coefficient));
    }

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe_status(lchip, serdes_id, mode, status));

    return CTC_E_NONE;
}

/* get serdes signal detect */
uint32
sys_usw_datapath_get_serdes_sigdet(uint8 lchip, uint16 serdes_id)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   hss_type  = 0;
    uint32  value_pcs = 0;
    uint32  value     = 0;
    uint32  cmd        = 0;
    uint32  tbl_id     = 0;
    uint32  fld_id     = 0;
    uint8   step       = 0;
    SharedPcsSerdes0Cfg0_m  serdes_cfg;
    QsgmiiPcsCfg0_m  qsgmii_serdes_cfg;
    UsxgmiiPcsSerdesCfg0_m usxgmii_serdes_cfg;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes    = NULL;

    sal_memset(&serdes_cfg, 0, sizeof(SharedPcsSerdes0Cfg0_m));
    sal_memset(&qsgmii_serdes_cfg, 0, sizeof(QsgmiiPcsCfg0_m));
    sal_memset(&usxgmii_serdes_cfg, 0, sizeof(UsxgmiiPcsSerdesCfg0_m));

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }

    if (hss_type)
    {
        tbl_id = CsMon0_t + hss_id;
        switch (lane_id)
        {
        case 0:
            fld_id = CsMon0_monHss28gRxASigdet_f;
            break;
        case 1:
            fld_id = CsMon0_monHss28gRxBSigdet_f;
            break;
        case 2:
            fld_id = CsMon0_monHss28gRxCSigdet_f;
            break;
        case 3:
            fld_id = CsMon0_monHss28gRxDSigdet_f;
            break;
        }
    }
    else
    {
        tbl_id = HsMon0_t + hss_id;
        switch (lane_id)
        {
        case 0:
            fld_id = HsMon0_monHss15gRxASigdet_f;
            break;
        case 1:
            fld_id = HsMon0_monHss15gRxBSigdet_f;
            break;
        case 2:
            fld_id = HsMon0_monHss15gRxCSigdet_f;
            break;
        case 3:
            fld_id = HsMon0_monHss15gRxDSigdet_f;
            break;
        case 4:
            fld_id = HsMon0_monHss15gRxESigdet_f;
            break;
        case 5:
            fld_id = HsMon0_monHss15gRxFSigdet_f;
            break;
        case 6:
            fld_id = HsMon0_monHss15gRxGSigdet_f;
            break;
        case 7:
            fld_id = HsMon0_monHss15gRxHSigdet_f;
            break;
        }
    }

    cmd = DRV_IOR(tbl_id, fld_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        return CTC_E_NOT_INIT;
    }
    p_serdes = &p_hss_vec->serdes_info[lane_id];

    /* get force signal detect */
    if (p_serdes->mode == CTC_CHIP_SERDES_QSGMII_MODE)
    {
        tbl_id = QsgmiiPcsCfg0_t + (serdes_id/4)*2 + serdes_id%4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_serdes_cfg));
        value_pcs = GetQsgmiiPcsCfg0(V, forceSignalDetect_f, &qsgmii_serdes_cfg);
    }
    else if (p_serdes->mode == CTC_CHIP_SERDES_USXGMII0_MODE)
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + (serdes_id/4)*2;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        value_pcs = GetUsxgmiiPcsSerdesCfg0(V, forceSignalDetect0_f+serdes_id%4, &usxgmii_serdes_cfg);
    }
    else if((p_serdes->mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (p_serdes->mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + (serdes_id/4)*2 + serdes_id%4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        value_pcs = GetUsxgmiiPcsSerdesCfg0(V, forceSignalDetect0_f, &usxgmii_serdes_cfg);
    }
    else
    {
        step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
        tbl_id = SharedPcsSerdes0Cfg0_t + serdes_id/4 + (serdes_id%4)*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &serdes_cfg));
        value_pcs = GetSharedPcsSerdes0Cfg0(V, forceSignalDetect0_f, &serdes_cfg);
    }

    return (value_pcs?1:(value?1:0));
}


uint32
sys_usw_datapath_get_serdes_no_random_data_flag(uint8 lchip, uint8 serdes_id)
{
    uint16 value = 0;
    uint8 internal_lane = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;
    uint32 no_random_flag = 0;

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        internal_lane = lane_id;
        /*check loopback internal*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 1);
        drv_chip_read_hss28g(lchip, hss_id, address, &value);
        if ((value & 0x60) == 0x60)
        {
            no_random_flag = 0;
            return no_random_flag;
        }

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
        /*bit 2 reset*/
        no_random_flag = CTC_IS_BIT_SET(value, 2);
    }
    else
    {
        internal_lane = g_usw_hss15g_lane_map[lane_id];
        /*check loopback internal*/
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 1);
        drv_chip_read_hss15g(lchip, hss_id, address, &value);
        if ((value & 0x60) == 0x60)
        {
            no_random_flag = 0;
            return no_random_flag;
        }

        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x8);
        CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
        /*bit 2 reset*/
        no_random_flag = CTC_IS_BIT_SET(value, 2);
    }

    return no_random_flag;

}

int32
sys_usw_datapath_set_serdes_link_up_cfg(uint8 lchip,
                                        uint16 lport,
                                        uint8 mac_id,
                                        uint8 lg_flag,
                                        uint8 pcs_mode)
{
    uint8 slice_id = 0;
    uint16 chip_port = 0;
    uint8 hss_id = 0;
    uint8  lane_id   = 0;
    uint16 serdes_id  = 0;
    uint8 tmp_lane = 0;
    uint8 num = 0;
    uint32 address = 0;
    uint16 data = 0;
    uint32 i = 0;

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        for (i = 0; i < num; i++)
        {
            GET_SERDES_LANE(mac_id, lane_id, i, lg_flag, pcs_mode, tmp_lane);
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[tmp_lane], 0xa);
            drv_chip_read_hss28g(lchip, hss_id, address, &data);
            if (data & 0x8000)
            {
                data &= 0x7fff;
                drv_chip_write_hss28g(lchip, hss_id, address, data);
            }
        }
    }
    return CTC_E_NONE;
}

int16
sys_usw_datapath_hss15g_serdes_aec_aet(uint8 lchip, uint8 hss_id, uint8 lane_id, sys_datapath_hss_attribute_t* p_hss_vec, bool enable, uint16 serdes_id)
{
    uint32  address = 0;
    uint16  value = 0;
    uint32  tmp_val = 0;
    uint32  cmd = 0;
    uint32  tb_id = 0;
    uint32  fld_id = 0;
    uint8   internal_lane = 0;
    sys_datapath_serdes_info_t* p_serdes    = NULL;
    HsMacroCfg0_m hs_macro;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"hss_id:%d, lane_id:%d, enable_value:%d\n", hss_id, lane_id, enable);

    internal_lane = g_usw_hss15g_lane_map[lane_id];
    p_serdes = &p_hss_vec->serdes_info[lane_id];

    if((CTC_CHIP_SERDES_XFI_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_XLG_MODE == p_serdes->mode))
    {
        tb_id = HsMacroCfg0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);

        tmp_val = 0;
        switch (lane_id)
        {
        case 0:
            fld_id = HsMacroCfg0_cfgAnethTxASel_f;
            break;
        case 1:
            fld_id = HsMacroCfg0_cfgAnethTxBSel_f;
            break;
        case 2:
            fld_id = HsMacroCfg0_cfgAnethTxCSel_f;
            break;
        case 3:
            fld_id = HsMacroCfg0_cfgAnethTxDSel_f;
            break;
        case 4:
            fld_id = HsMacroCfg0_cfgAnethTxESel_f;
            break;
        case 5:
            fld_id = HsMacroCfg0_cfgAnethTxFSel_f;
            break;
        case 6:
            fld_id = HsMacroCfg0_cfgAnethTxGSel_f;
            break;
        case 7:
            fld_id = HsMacroCfg0_cfgAnethTxHSel_f;
            break;
        default:
            fld_id = DRV_ENTRY_FLAG;
            break;
        }
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);

        /* 1 cfg serdes lanes tx EQMODE, AESRC and P/N swap */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
        if(enable)
        {
            value = (p_serdes->tx_polarity)?(0x0270):(0x0230);
        }
        else
        {
            drv_chip_read_hss15g(lchip, hss_id, address, &value);
            value &= 0xffcf;
            value |= 0x01;
        }
        drv_chip_write_hss15g(lchip, hss_id, address, value);

        /* 2 cfg serdes lanes rx SPI enable, cfg DFE Control Register, offset 0x08, value = 0x2082 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x08);
        drv_chip_read_hss15g(lchip, hss_id, address, &value);
        if(enable)
        {
            value |= 0x2000;
        }
        else
        {
            value &= 0xdfff;
        }
        drv_chip_write_hss15g(lchip, hss_id, address, value);

        /* 3. CFG HsMacroCfg forHSS15G */
        tb_id = HsMacroCfg0_t + p_hss_vec->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);
        if(enable)
        {
            /*  release HssRx/HssTx/Aec/Aet reset */
            tmp_val = 0;
            /* disable Aec Tx bypass */
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAecTxAByp_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgAecTxBByp_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgAecTxCByp_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgAecTxDByp_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgAecTxEByp_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgAecTxFByp_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgAecTxGByp_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgAecTxHByp_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            tmp_val = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgPmaRxAWidth_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgPmaRxBWidth_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgPmaRxCWidth_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgPmaRxDWidth_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgPmaRxEWidth_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgPmaRxFWidth_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgPmaRxGWidth_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgPmaRxHWidth_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgPmaTxAWidth_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgPmaTxBWidth_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgPmaTxCWidth_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgPmaTxDWidth_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgPmaTxEWidth_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgPmaTxFWidth_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgPmaTxGWidth_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgPmaTxHWidth_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            tmp_val = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetHssRxA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetHssRxB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetHssRxC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetHssRxD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetHssRxE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetHssRxF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetHssRxG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetHssRxH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetHssTxD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetHssTxE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetHssTxF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetHssTxG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetHssTxH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetAecA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetAecB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetAecC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetAecD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetAecE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetAecF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetAecG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetAecH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetAetA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetAetB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetAetC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetAetD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetAetE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetAetF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetAetG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetAetH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);
        }
        else
        {
            tmp_val = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAecTxAByp_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgAecTxBByp_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgAecTxCByp_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgAecTxDByp_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgAecTxEByp_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgAecTxFByp_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgAecTxGByp_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgAecTxHByp_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetHssRxA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetHssRxB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetHssRxC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetHssRxD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetHssRxE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetHssRxF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetHssRxG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetHssRxH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetHssTxD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetHssTxE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetHssTxF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetHssTxG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetHssTxH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetAecA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetAecB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetAecC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetAecD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetAecE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetAecF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetAecG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetAecH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgResetAetA_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgResetAetB_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgResetAetC_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgResetAetD_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgResetAetE_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgResetAetF_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgResetAetG_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgResetAetH_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &hs_macro);

        }
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &hs_macro);

        if (enable)
        {
            /* 1. CFG AET AEC REG */
            address = 0x00;
            value = 0xb0;
            drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 1, address, value);

            address = 0x01;
            value = 0x3000;
            drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 1, address, value);

            address = 0x02;
            value = 0x08c4;
            drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 1, address, value);

            address = 0x03;
            value = 0x00;
            drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 1, address, value);

            /* do DFE reset & release */
            CTC_ERROR_RETURN(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, 1));
            CTC_ERROR_RETURN(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, 0));
        }

        address = 0x04;         /* cfg REG_MODE */
        value = (enable)?0x0044:0x00;
        drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 0, address, value);

        address = 0x06;        /* cfg FSM_CTL */
        value = (enable)?0x8003:0x00;
        drv_chip_write_hss15g_aec_aet(lchip, hss_id, lane_id, 0, address, value);
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes mode is not support  \n");
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int16
sys_usw_datapath_hss28g_serdes_aec_aet(uint8 lchip, uint8 hss_id, uint8 lane_id, sys_datapath_hss_attribute_t* p_hss_vec, bool enable, uint16 serdes_id)
{
    uint32  address = 0;
    uint16  value = 0;
    uint32  tmp_val = 0;
    uint32  cmd = 0;
    uint32  tb_id = 0;
    uint32  fld_id = 0;
    uint8   internal_lane = 0;
    sys_datapath_serdes_info_t* p_serdes    = NULL;
    CsMacroCfg0_m cs_macro;

    ctc_chip_device_info_t dev_info;
    sal_memset(&dev_info, 0, sizeof(ctc_chip_device_info_t));

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"hss_id:%d, lane_id:%d, enable_value:%d\n", hss_id, lane_id, enable);

    sys_usw_chip_get_device_info(lchip, &dev_info);
    internal_lane = lane_id;
    p_serdes = &p_hss_vec->serdes_info[lane_id];

    if((CTC_CHIP_SERDES_XFI_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_CG_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_XLG_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_XXVG_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_LG_MODE == p_serdes->mode))
    {
        tb_id = CsMacroCfg0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);

        switch (lane_id)
        {
        case 0:
            fld_id = CsMacroCfg0_cfgLaneAUseAetDpc_f;
            break;
        case 1:
            fld_id = CsMacroCfg0_cfgLaneBUseAetDpc_f;
            break;
        case 2:
            fld_id = CsMacroCfg0_cfgLaneCUseAetDpc_f;
            break;
        case 3:
            fld_id = CsMacroCfg0_cfgLaneDUseAetDpc_f;
            break;
        default:
            fld_id = DRV_ENTRY_FLAG;
            break;
        }
        if (enable)
        {
            tmp_val = 1;
        }
        else
        {
            tmp_val = 0;
        }
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

        tmp_val = 0;
        switch (lane_id)
        {
        case 0:
            fld_id = CsMacroCfg0_cfgAnethTxASel_f;
            break;
        case 1:
            fld_id = CsMacroCfg0_cfgAnethTxBSel_f;
            break;
        case 2:
            fld_id = CsMacroCfg0_cfgAnethTxCSel_f;
            break;
        case 3:
            fld_id = CsMacroCfg0_cfgAnethTxDSel_f;
            break;
        default:
            fld_id = DRV_ENTRY_FLAG;
            break;
        }

        DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);

        /* 1.cfg serdes lanes tx EQMODE, AESRC and P/N swap */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
        if(enable)
        {
            value = (p_serdes->tx_polarity)?(0x0070):(0x0030);
        }
        else
        {
            drv_chip_read_hss28g(lchip, hss_id, address, &value);
            value &= 0xffcf;
            value |= 0x01;
        }
        drv_chip_write_hss28g(lchip, hss_id, address, value);

        /* 2.cfg serdes lanes rx SPI enable, cfg DFE Control Register, offset 0x08, value = 0x2082 */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x08);
        drv_chip_read_hss28g(lchip, hss_id, address, &value);
        if(enable)
        {
            value |= 0x2000;
            value &= 0xfffe;
        }
        else
        {
            value &= 0xdffe;
        }
        drv_chip_write_hss28g(lchip, hss_id, address, value);

        /* 3.CFG HsMacroCfg forHSS15G or CsMacroCfg for HSS28G */
        tb_id = CsMacroCfg0_t + p_hss_vec->hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        if(enable)
        {
            /* disable Aec Tx bypass */
            tmp_val = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAecTxAByp_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgAecTxBByp_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgAecTxCByp_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgAecTxDByp_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            if((CTC_CHIP_SERDES_XFI_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_XLG_MODE == p_serdes->mode))
            {
                tmp_val = 1;  /* pma data width(20-bit) */
            }
            else
            {
                tmp_val = 3;  /* pma data width(40-bit) */
            }
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgPmaRxAWidth_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgPmaRxBWidth_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgPmaRxCWidth_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgPmaRxDWidth_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgPmaTxAWidth_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgPmaTxBWidth_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgPmaTxCWidth_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgPmaTxDWidth_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            tmp_val = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetHssRxA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetHssRxB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetHssRxC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetHssRxD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetHssTxD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            tmp_val = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetAecA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetAecB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetAecC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetAecD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetAetA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetAetB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetAetC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetAetD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);
        }
        else
        {
            tmp_val = 1;

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAecTxAByp_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgAecTxBByp_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgAecTxCByp_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgAecTxDByp_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetHssRxA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetHssRxB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetHssRxC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetHssRxD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetHssTxA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetHssTxB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetHssTxC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetHssTxD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetAecA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetAecB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetAecC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetAecD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

            tmp_val = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgResetAetA_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgResetAetB_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgResetAetC_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgResetAetD_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tb_id, fld_id, &tmp_val, &cs_macro);

        }
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &cs_macro);

        if (enable)
        {
            /* 4. CFG AET AEC REG */
            address = 0x00;
            value = 0xb0;
            drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 1, address, value);

            address = 0x01;
            value = 0x3000;
            drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 1, address, value);

            address = 0x02;
            value = 0x08c4;
            drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 1, address, value);

            address = 0x03;
            value = 0x00;
            drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 1, address, value);

            /* do DFE reset & release */
            CTC_ERROR_RETURN(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, 1));
            CTC_ERROR_RETURN(sys_usw_datapath_set_dfe_reset(lchip, serdes_id, 0));
        }

        /* see 'sys_usw_datapath_set_serdes_link_up_cfg' */
        address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0xa);
        drv_chip_write_hss28g(lchip, hss_id, address, 0x8000);

        address = 0x04;         /* cfg REG_MODE */
        value = (enable)?0x0044:0x00;
        drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 0, address, value);

        address = 0x06;        /* cfg FSM_CTL */
        value = (enable)?0x8003:0x00;
        drv_chip_write_hss28g_aec_aet(lchip, hss_id, internal_lane, 0, address, value);
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes mode %d is not support  \n", p_serdes->mode);
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_gport_with_serdes(uint8 lchip, uint16 serdes_id, uint32* p_gport)
{
    uint8 hss_idx = 0;
    uint16 drv_port = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 lane_id = 0;
    uint8 slice_id = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    CTC_PTR_VALID_CHECK(p_gport);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    slice_id = (serdes_id >= SERDES_NUM_PER_SLICE)?1:0;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];
    drv_port = p_serdes->lport + slice_id * SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, drv_port);

    *p_gport = gport;

    return CTC_E_NONE;
}

/* Support 802.3ap, auto_neg for Backplane Ethernet */
int16
sys_usw_datapath_set_serdes_auto_neg_en(uint8 lchip, uint16 serdes_id, uint32 enable)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint8   hss_type  = 0;
    uint16  tbl_id    = 0;
    uint16  step      = 0;
    uint32  cmd       = 0;
    uint32  value     = 0;
    uint16  fld_id    = 0;
    uint8   slice_id  = 0;
    HsMacroCfg0_m    hs_macro;
    CsMacroCfg0_m    cs_macro;
    AnethCtl0_m      aneth_ctl;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable:0x%X\n", serdes_id, enable);

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, serdes_id, &p_serdes));

    port_attr = sys_usw_datapath_get_port_capability(lchip, p_serdes->lport, slice_id);
    if (port_attr == NULL)
    {
        return CTC_E_NOT_INIT;
    }

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
    }
    else
    {
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
    }
    SYS_DATAPATH_HSS_STEP(serdes_id, step);

    /* enable cl73 atuo-neg */
    if(enable)
    {
        if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
        {
            /* #1, Cfg Aneth mode */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* #2, Cfg AdvAbility */

            /* #3, Cfg Aec Bypass */
            tbl_id = HsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            value = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAecRxAByp_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgAecRxBByp_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgAecRxCByp_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgAecRxDByp_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgAecRxEByp_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgAecRxFByp_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgAecRxGByp_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgAecRxHByp_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAPcsRxReadyEn_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgBPcsRxReadyEn_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgCPcsRxReadyEn_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgDPcsRxReadyEn_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgEPcsRxReadyEn_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgFPcsRxReadyEn_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgGPcsRxReadyEn_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgHPcsRxReadyEn_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* #4, Enable Aneth */
            tbl_id = HsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* #1   cfg  HsMacroCfg[0..2]_cfg[A..H]AnethLinkStatusSel =1'b1 ; */
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAAnethLinkStatusSel_f;
                value = 0;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgBAnethLinkStatusSel_f;
                value = 1;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgCAnethLinkStatusSel_f;
                value = 2;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgDAnethLinkStatusSel_f;
                value = 3;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgEAnethLinkStatusSel_f;
                value = 0;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgFAnethLinkStatusSel_f;
                value = 1;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgGAnethLinkStatusSel_f;
                value = 2;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgHAnethLinkStatusSel_f;
                value = 3;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }

            if (CTC_PORT_SPEED_40G == port_attr->speed_mode)
                value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #2   cfg  HsMacroCfg[0..2]_cfg[A..H]AnethAutoTxEn =1'b1; */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAnethTxASel_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgAnethTxBSel_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgAnethTxCSel_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgAnethTxDSel_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgAnethTxESel_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgAnethTxFSel_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgAnethTxGSel_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgAnethTxHSel_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #3    Cfg HsMacroCfg[0..2]_cfgLane[A..H]PrtReady = 1'b1;     */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgLaneAPrtReady_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgLaneBPrtReady_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgLaneCPrtReady_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgLaneDPrtReady_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgLaneEPrtReady_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgLaneFPrtReady_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgLaneGPrtReady_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgLaneHPrtReady_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* cfg autoneg en */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
        }
        else if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
        {
            /* #1, Cfg Aneth mode */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            value = 0;
            if ((CTC_PORT_SPEED_25G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_50G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_100G == port_attr->speed_mode))
            {
                value = 1;
            }

            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* #2, Cfg AdvAbility */

            /* #3, Cfg Aec Bypass */
            tbl_id = CsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
            value = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAecRxAByp_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgAecRxBByp_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgAecRxCByp_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgAecRxDByp_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAPcsRxReadyEn_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgBPcsRxReadyEn_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgCPcsRxReadyEn_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgDPcsRxReadyEn_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            /* #4, Enable Aneth */

            /* 2. cfg CsMacroCfg */
            /* #1   cfg  CsMacroCfg[0..3]_cfg[A..D]AnethLinkStatusSel =1'b1 ; */
            tbl_id = CsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAAnethLinkStatusSel_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgBAnethLinkStatusSel_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgCAnethLinkStatusSel_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgDAnethLinkStatusSel_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }

            if ((CTC_PORT_SPEED_40G == port_attr->speed_mode)
                ||(CTC_PORT_SPEED_100G == port_attr->speed_mode))
            {
                value = 0;
            }
            else if (CTC_PORT_SPEED_50G == port_attr->speed_mode)
            {
                if ((0 == lane_id) || (1 == lane_id))
                {
                    value = 0;
                }
                if ((2 == lane_id) || (3 == lane_id))
                {
                    value = 2;
                }
            }
            else
            {
                value = lane_id;
            }

            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            /* #2   cfg  CsMacroCfg[0..3]_cfg[A..D]AnethAutoTxEn =1'b1; */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAnethTxASel_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgAnethTxBSel_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgAnethTxCSel_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgAnethTxDSel_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            /* #3    Cfg CsMacroCfg[0..3]_cfgLane[A..D]PrtReady = 1'b1;     */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgLaneAPrtReady_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgLaneBPrtReady_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgLaneCPrtReady_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgLaneDPrtReady_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            /* 1. cfg AnethCtl_cfgAutoNegEnable */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
        }
    }
    else   /* AN disable */
    {
        if(SYS_DATAPATH_HSS_TYPE_15G == hss_type)
        {
            /* 1. cfg AnethCtl_cfgAutoNegEnable disable */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* 1. cfg HsMacroCfg */
            tbl_id = HsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* cfg cfgAecRx[A..h]Byp = 1'b1 */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgAecRxAByp_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgAecRxBByp_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgAecRxCByp_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgAecRxDByp_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgAecRxEByp_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgAecRxFByp_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgAecRxGByp_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgAecRxHByp_f;
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #1 cfg HsMacroCfg[0..2]_cfg[A..H]AnethAutoTxEn = 1'b0 */
            value = 0;
            switch (lane_id)
            {
            case 0:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgAPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 1:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgBPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 2:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgCPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 3:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgDPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 4:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgEPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 5:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgFPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 6:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgGPcsRxReadyEn_f, &value, &hs_macro);
                break;
            case 7:
                DRV_IOW_FIELD(lchip, tbl_id, HsMacroCfg0_cfgHPcsRxReadyEn_f, &value, &hs_macro);
                break;
            default:
                break;
            }

            /* #1 Cfg HsMacroCfg[0..2]_cfgLane[A..H]PrtReady = 1'b0; */
            value = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = HsMacroCfg0_cfgLaneAPrtReady_f;
                break;
            case 1:
                fld_id = HsMacroCfg0_cfgLaneBPrtReady_f;
                break;
            case 2:
                fld_id = HsMacroCfg0_cfgLaneCPrtReady_f;
                break;
            case 3:
                fld_id = HsMacroCfg0_cfgLaneDPrtReady_f;
                break;
            case 4:
                fld_id = HsMacroCfg0_cfgLaneEPrtReady_f;
                break;
            case 5:
                fld_id = HsMacroCfg0_cfgLaneFPrtReady_f;
                break;
            case 6:
                fld_id = HsMacroCfg0_cfgLaneGPrtReady_f;
                break;
            case 7:
                fld_id = HsMacroCfg0_cfgLaneHPrtReady_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
        }
        else if(SYS_DATAPATH_HSS_TYPE_28G == hss_type)
        {
            /* 1. cfg AnethCtl_cfgAutoNegEnable disable */
            tbl_id = AnethCtl0_t + step;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* 2. cfg CsMacroCfg */
            tbl_id = CsMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            /* cfgAecRx[A..D]Byp = 1'b1 */
            value = 1;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgAecRxAByp_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgAecRxBByp_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgAecRxCByp_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgAecRxDByp_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            /* #1   cfg  CsMacroCfg[0..3]_cfg[A..D]AnethAutoTxEn =1'b0; */
            value = 0;
            switch (lane_id)
            {
            case 0:
                DRV_IOW_FIELD(lchip, tbl_id, CsMacroCfg0_cfgAPcsRxReadyEn_f, &value, &cs_macro);
                break;
            case 1:
                DRV_IOW_FIELD(lchip, tbl_id, CsMacroCfg0_cfgBPcsRxReadyEn_f, &value, &cs_macro);
                break;
            case 2:
                DRV_IOW_FIELD(lchip, tbl_id, CsMacroCfg0_cfgCPcsRxReadyEn_f, &value, &cs_macro);
                break;
            case 3:
                DRV_IOW_FIELD(lchip, tbl_id, CsMacroCfg0_cfgDPcsRxReadyEn_f, &value, &cs_macro);
                break;
            default:
                fld_id = DRV_ENTRY_FLAG;
                break;
            }

            /* #1 Cfg CsMacroCfg[0..3]_cfgLane[A..D]PrtReady = 1'b0; */
            value = 0;
            switch (lane_id)
            {
            case 0:
                fld_id = CsMacroCfg0_cfgLaneAPrtReady_f;
                break;
            case 1:
                fld_id = CsMacroCfg0_cfgLaneBPrtReady_f;
                break;
            case 2:
                fld_id = CsMacroCfg0_cfgLaneCPrtReady_f;
                break;
            case 3:
                fld_id = CsMacroCfg0_cfgLaneDPrtReady_f;
                break;
            default:
                fld_id = 0;
                break;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        }
    }
    return CTC_E_NONE;
}

/* Support 802.3ap, auto_neg for Backplane Ethernet */
int16
sys_usw_datapath_set_serdes_auto_neg_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability)
{
    uint8 slice_id    = 0;
    uint16  tbl_id    = 0;
    uint32  cmd       = 0;
    uint32  value     = 0;
    uint32  value_arr[2] = {0};
    uint16  fld_id    = 0;
    AnethCtl0_m aneth_ctl;
    AnethAdvAbility0_m auto_neg_cfg;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16  step      = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, base page[0]:0x%x, [1]:0x%x, next page1[0]:0x%x, [1]:0x%x\n",
        serdes_id, p_ability->base_ability0, p_ability->base_ability1, p_ability->np1_ability0, p_ability->np1_ability1);

    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, serdes_id, &p_serdes));

    port_attr = sys_usw_datapath_get_port_capability(lchip, p_serdes->lport, slice_id);
    if (port_attr == NULL)
    {
        return CTC_E_NOT_INIT;
    }

    SYS_DATAPATH_HSS_STEP(serdes_id, step);

    /* #2, cfgAdvAbility */
    tbl_id = AnethAdvAbility0_t + step;
    fld_id = AnethAdvAbility0_cfgAdvAbility_f;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &auto_neg_cfg));

    DRV_IOR_FIELD(lchip, tbl_id, fld_id, value_arr, &auto_neg_cfg);
    value_arr[0] &= 0x001FFFFF;
    value_arr[0] |= p_ability->base_ability0;
    value_arr[1] &= 0xFFFF0FFF;
    value_arr[1] |= p_ability->base_ability1;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, value_arr, &auto_neg_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &auto_neg_cfg));

    if(p_ability->np1_ability0 || p_ability->np1_ability1)
    {
        /* #1, cfgAutoNextPageEn & cfgNextPageNum */
        tbl_id = AnethCtl0_t + step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_ctl));

        fld_id = AnethCtl0_cfgAutoNextPageEn_f;
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &aneth_ctl);
        fld_id = AnethCtl0_cfgNextPageNum_f;
        value = 2;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &aneth_ctl);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_ctl));

        /* #2, cfg AdvAbility(Base page) && Next page ability */
        /* ## 2.1 cfgNextPageTx1 */
        tbl_id = AnethAdvAbility0_t + step;
        fld_id = AnethAdvAbility0_cfgNextPageTx1_f;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &auto_neg_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, value_arr, &auto_neg_cfg);

        value_arr[0] &= 0xFCCFFFFF;
        value_arr[0] |= p_ability->np1_ability0;
        value_arr[1] &= 0xFFFFF0FF;
        value_arr[1] |= p_ability->np1_ability1;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, value_arr, &auto_neg_cfg);

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &auto_neg_cfg));
    }
    else
    {
        /* #1, cfgAutoNextPageEn & cfgNextPageNum */
        tbl_id = AnethCtl0_t + step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_ctl));

        fld_id = AnethCtl0_cfgAutoNextPageEn_f;
        value = 1;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &aneth_ctl);
        fld_id = AnethCtl0_cfgNextPageNum_f;
        value = 0;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &aneth_ctl);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_ctl));
    }

    return CTC_E_NONE;
}
/* Support 802.3ap, auto_neg for Backplane Ethernet */

int16
sys_usw_datapath_get_serdes_auto_neg_local_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability)
{
    uint16  tbl_id    = 0;
    uint32  cmd       = 0;
    AnethCtl0_m aneth_ctl;
    AnethAdvAbility0_m adv_ability;
    uint16  step      = 0;
    uint32  np_en, np_cnt;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    sal_memset(p_ability, 0, sizeof(sys_datapath_an_ability_t));
    SYS_DATAPATH_HSS_STEP(serdes_id, step);

    tbl_id = AnethCtl0_t + step;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_ctl));
    GetAnethCtl0(A, cfgAutoNextPageEn_f, &aneth_ctl, &np_en);
    GetAnethCtl0(A, cfgNextPageNum_f, &aneth_ctl, &np_cnt);

    tbl_id = AnethAdvAbility0_t + step;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &adv_ability));
    GetAnethAdvAbility0(A, cfgAdvAbility_f, &adv_ability, &(p_ability->base_ability0));
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get local base page ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
        p_ability->base_ability0, p_ability->base_ability1);
    if(np_en && (np_cnt!=0))
    {
        GetAnethAdvAbility0(A, cfgNextPageTx0_f, &adv_ability, &(p_ability->np0_ability0));
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get local next page0 ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
            p_ability->np0_ability0, p_ability->np0_ability1);
        GetAnethAdvAbility0(A, cfgNextPageTx1_f, &adv_ability, &(p_ability->np1_ability0));
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get local next page1 ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
            p_ability->np1_ability0, p_ability->np1_ability1);
    }
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
        "serdes %d local base page ability:0x%x, 0x%x, next page0 ability:0x%x, 0x%x, next page1 ability:0x%x, 0x%x\n",
        serdes_id, p_ability->base_ability0,p_ability->base_ability1,p_ability->np0_ability0, p_ability->np0_ability1,
        p_ability->np1_ability0, p_ability->np1_ability1);

    return CTC_E_NONE;
}



/* Support 802.3ap, auto_neg for Backplane Ethernet */
int16
sys_usw_datapath_get_serdes_auto_neg_remote_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability)
{
    uint16  tbl_id    = 0;
    uint32  cmd       = 0;
    uint16  step      = 0;
    AnethNegAbility0_m auto_neg_mon;
    uint32  lp_np_cnt = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    sal_memset(&auto_neg_mon, 0, sizeof(AnethNegAbility0_m));
    sal_memset(p_ability, 0, sizeof(sys_datapath_an_ability_t));
    SYS_DATAPATH_HSS_STEP(serdes_id, step);

    tbl_id = AnethNegAbility0_t + step;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &auto_neg_mon));
    GetAnethNegAbility0(A, monLpAdvAbility_f, &auto_neg_mon, &(p_ability->base_ability0));
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get LP base page ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
        p_ability->base_ability0, p_ability->base_ability1);
    GetAnethNegAbility0(A, monLpNextPageCnt_f, &auto_neg_mon, &lp_np_cnt);
    if(lp_np_cnt)
    {
        GetAnethNegAbility0(A, monLpNextPage0_f, &auto_neg_mon, &(p_ability->np0_ability0));
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get LP next page0 ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
            p_ability->np0_ability0, p_ability->np0_ability1);

        GetAnethNegAbility0(A, monLpNextPage1_f, &auto_neg_mon, &(p_ability->np1_ability0));
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get LP next page1 ability, raw format, ability[0] = 0x%x, [1] = 0x%x\n",
            p_ability->np1_ability0, p_ability->np1_ability1);
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
        "serdes %d remote base page ability:0x%x, 0x%x, next page0 ability:0x%x, 0x%x, next page1 ability:0x%x, 0x%x\n",
        serdes_id, p_ability->base_ability0,p_ability->base_ability1,p_ability->np0_ability0, p_ability->np0_ability1,
        p_ability->np1_ability0, p_ability->np1_ability1);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_serdes_cl73_status(uint8 lchip, uint16 serdes_id, uint16* p_value)
{
    uint16  tbl_id    = 0;
    uint32  cmd       = 0;
    uint32  value     = 0;
    AnethStatus0_m aneth_status;
    uint16  step      = 0;

    SYS_DATAPATH_HSS_STEP(serdes_id, step);

    tbl_id = AnethStatus0_t + step;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aneth_status));
    value = GetAnethStatus0(V, monAutoNegComplete_f, &aneth_status);
    value += GetAnethStatus0(V, monIncompatibleLink_f, &aneth_status);
    *p_value = value;
    return CTC_E_NONE;
}


/*return lport is per chip port, not driver port, mac id is per slice*/
uint16
sys_usw_datapath_get_lport_with_mac(uint8 lchip, uint8 slice_id, uint8 mac_id)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 temp = 0;
    uint8 find_flag = 0;

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        if (port_attr->port_type == SYS_DMPS_RSV_PORT)
        {
            continue;
        }

        if (mac_id == port_attr->mac_id)
        {
            find_flag = 1;
            temp = lport;
            break;
        }
    }

    if (find_flag)
    {
        return temp;
    }
    else
    {
        return SYS_COMMON_USELESS_MAC;
    }

}
/*return lport is per chip port, not driver port, mac id is per slice*/
uint16
sys_usw_datapath_get_lport_with_mac_tbl_id(uint8 lchip, uint8 slice_id, uint8 mac_tbl_id)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 temp = 0;
    uint8 find_flag = 0;

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        if (port_attr->port_type == SYS_DMPS_RSV_PORT)
        {
            continue;
        }

        if (mac_tbl_id == port_attr->tbl_id)
        {
            find_flag = 1;
            temp = lport;
            break;
        }
    }

    if (find_flag)
    {
        return temp;
    }
    else
    {
        return SYS_COMMON_USELESS_MAC;
    }

}
/*return lport is per chip port, not driver port, chan id is per slice*/
uint16
sys_usw_datapath_get_lport_with_chan(uint8 lchip, uint8 slice_id, uint8 chan_id)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    uint16 temp = 0;
    uint8 find_flag = 0;

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (!port_attr)
        {
            continue;
        }

        if (port_attr->port_type == SYS_DMPS_RSV_PORT)
        {
            continue;
        }

        if (chan_id == port_attr->chan_id)
        {
            find_flag = 1;
            temp = lport;
            break;
        }
    }

    if (find_flag)
    {
        return temp;
    }
    else
    {
        return SYS_COMMON_USELESS_MAC;
    }

}

/*
    get port attribute, lport is chip port, notice not driver port
*/
void*
sys_usw_datapath_get_port_capability(uint8 lchip, uint16 lport, uint8 slice_id)
{
    sys_datapath_lport_attr_t* p_init_port = NULL;

    if (lport >= MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM))
    {
        return NULL;
    }

    if (NULL == p_usw_datapath_master[lchip])
    {
        return NULL;
    }

    p_init_port = (sys_datapath_lport_attr_t*)(&p_usw_datapath_master[lchip]->port_attr[slice_id][lport]);

    return p_init_port;
}

int32
sys_usw_datapath_get_hss_info(uint8 lchip, uint8 hss_idx, sys_datapath_hss_attribute_t** p_hss)
{
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);

    *p_hss = p_hss_vec;

    return CTC_E_NONE;
}


int32
sys_usw_datapath_get_serdes_info(uint8 lchip, uint8 serdes_id, sys_datapath_serdes_info_t** p_serdes)
{
    uint16 hss_idx = 0;
    uint8 lane_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes_info = NULL;

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_idx = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
            return CTC_E_INVALID_CONFIG;

    }
    p_serdes_info = &p_hss->serdes_info[lane_idx];

    if (p_serdes_info == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
            return CTC_E_INVALID_CONFIG;

    }

    *p_serdes = p_serdes_info;

    return CTC_E_NONE;
}


int32
sys_usw_datapath_get_serdes_with_lport(uint8 lchip, uint16 chip_port, uint8 slice, uint16* p_serdes_id, uint8* num)
{
    uint16 hss_idx = 0;
    uint8 lane_idx = 0;
    uint16 serdes_idx = 0;
    uint8 slice_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    uint8 flag = 0;

    for (serdes_idx = 0; serdes_idx < SERDES_NUM_PER_SLICE; serdes_idx++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_idx);
        lane_idx = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_idx);

        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss)
        {
            continue;
        }

        p_serdes = &p_hss->serdes_info[lane_idx];

        if ((chip_port >= p_serdes->lport) && (chip_port <= p_serdes->lport + p_serdes->port_num - 1) && (slice_id == slice))
        {
            *p_serdes_id = serdes_idx;
            flag = 1;
        }

        if (flag && num)
        {
            if ((p_serdes->mode == CTC_CHIP_SERDES_XAUI_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_DXAUI_MODE) ||
               (p_serdes->mode == CTC_CHIP_SERDES_XLG_MODE) || (p_serdes->mode == CTC_CHIP_SERDES_CG_MODE))
            {
                *num = 4;
            }
            else if (p_serdes->mode == CTC_CHIP_SERDES_LG_MODE)
            {
                *num = 2;
            }
            else
            {
                *num = 1;
            }

            break;
        }

    }


    return (flag?CTC_E_NONE:CTC_E_INVALID_PARAM);
}

int32
sys_usw_datapath_set_serdes_polarity(uint8 lchip, void* p_data)
{
    uint32 address = 0;
    uint8 hss_id = 0;
    uint8 lane_id = 0;
    uint8 internal_lane = 0;
    uint16 value = 0;
    uint8 hss_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    ctc_chip_serdes_polarity_t* p_polarity = (ctc_chip_serdes_polarity_t*)p_data;

    DATAPATH_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_polarity);

    if ((p_polarity->serdes_id >= SERDES_NUM_PER_SLICE) || (p_polarity->dir > 1) ||(p_polarity->polarity_mode > 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_polarity->serdes_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_NONE;
    }

    hss_id = p_hss->hss_id;
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_polarity->serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(p_polarity->serdes_id))
    {
        /*Hss28g*/
        internal_lane = lane_id;

        if (p_polarity->dir)
        {
            /*tx*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 2);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));

            /* used for EQMODE enable */
            if (p_polarity->polarity_mode)
            {
                value |= (uint16)(1<<6);
            }
            else
            {
                value &= ~((uint16)(1<<6));
            }
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

            /* used for EQMODE disable */
            /*1. set Transmit Polarity Register */
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0d);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            if (p_polarity->polarity_mode)
            {
                /*set bit0~bit3:0100*/
                value &= 0xfff0;
                value |= 0x4;
            }
            else
            {
                /*set bit0~bit3:1011*/
                value &= 0xfff0;
                value |= 0xb;
            }
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));

            /*2. set Transmit Coefficient Control Register for apply */
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            value |= 0x01;
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
        else
        {
            /*rx*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x20);
            CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, &value));
            if (p_polarity->polarity_mode)
            {
                value |= (uint16)(1<<6);
            }
            else
            {
                value &= ~((uint16)(1<<6));
            }
            CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, hss_id, address, value));
        }
    }
    else
    {
        /*Hss15g*/
        internal_lane = g_usw_hss15g_lane_map[lane_id];

        if (p_polarity->dir)
        {
            /*tx*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 2);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));

            /* used for EQMODE enable */
            if (p_polarity->polarity_mode)
            {
                value |= (uint16)(1<<6);
            }
            else
            {
                value &= ~((uint16)(1<<6));
            }
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));

            /* used for EQMODE enable */
            /*1. set Transmit Polarity Register */
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x0d);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
           if (p_polarity->polarity_mode)
            {
                /*set bit0~bit2:101*/
                value &= 0xfff8;
                value |= 0x5;
            }
            else
            {
                /*set bit0~bit2:010*/
                value &= 0xfff8;
                value |= 0x2;
            }
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));

            /*2. set Transmit Coefficient Control Register for apply */
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));
            value |= 0x01;
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
        else
        {
            /*rx*/
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_rx_addr_map[internal_lane], 0x20);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, &value));

            if (p_polarity->polarity_mode)
            {
                value |= (uint16)(1<<6);
            }
            else
            {
                value &= ~((uint16)(1<<6));
            }

            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, hss_id, address, value));
        }
    }

    /*update sw*/
    p_serdes = &p_hss->serdes_info[lane_id];
    if (p_polarity->dir)
    {
        p_serdes->tx_polarity =  p_polarity->polarity_mode;
    }
    else
    {
        p_serdes->rx_polarity =  p_polarity->polarity_mode;
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_get_serdes_polarity(uint8 lchip, void* p_data)
{
    uint8 lane_id = 0;
    uint8 hss_idx = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    ctc_chip_serdes_polarity_t* p_polarity = (ctc_chip_serdes_polarity_t*)p_data;

    DATAPATH_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_polarity);

    if ((p_polarity->serdes_id >= SERDES_NUM_PER_SLICE) || (p_polarity->dir > 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_polarity->serdes_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        return CTC_E_INVALID_PARAM;
    }

    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_polarity->serdes_id);

    p_serdes = &p_hss->serdes_info[lane_id];

    if (p_polarity->dir)
    {
        p_polarity->polarity_mode = p_serdes->tx_polarity;
    }
    else
    {
        p_polarity->polarity_mode = p_serdes->rx_polarity;
    }

    return CTC_E_NONE;

}

/*get clock core, core_type:0-plla, 1-pllb*/
uint16
sys_usw_datapath_get_core_clock(uint8 lchip, uint8 core_type)
{
    uint16 freq = 0;

    if (NULL == p_usw_datapath_master[lchip])
    {
        return 0;
    }

    if (core_type == 0)
    {
        freq = p_usw_datapath_master[lchip]->core_plla;
    }
    else if (core_type == 1)
    {
        freq = p_usw_datapath_master[lchip]->core_pllb;
    }

    return freq;
}

int32
sys_usw_datapath_set_impedance_calibration(uint8 lchip)
{
    uint8 index = 0;
    uint8 slave_index = 0;
    uint8 hssidx[2] = {0, 3};
    uint8 hssnum[2] = {3, 4};
    sys_datapath_hss_attribute_t* p_hss = NULL;
    uint32 value = 0;
    uint16 serdes_value = 0;
    uint32 tb_id = 0;
    uint32 cmd = 0;
    HsCfg0_m hs_cfg;
    HsMacroMiscMon0_m hs_mon;
    CsCfg0_m cs_cfg;
    CsMacroMiscMon0_m cs_mon;
    uint32 timeout = 0x4000;
    uint8 plla_ready = 0;
    uint8 pllb_ready = 0;
    uint8 train_done = 0;
    uint16 address = 0;
    uint8 hss_id = 0;
    uint8 hss_master_init = 0;
    uint8 hss_master_need_init = 0;

    /*For Hss15g Master HssIdx is 0, For Hss28g Master HssIdx is 3*/
    for (index = 0; index < 2; index++)
    {
        timeout = 0x4000;
        plla_ready = 0;
        pllb_ready = 0;
        hss_master_init = 0;
        hss_master_need_init = 0;
        train_done = 0;

        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hssidx[index]);
        if (p_hss)
        {
            /*hss have do init, skip*/
            hss_master_init = 1;
        }

        /*adjust slaver hss */
        for (slave_index = 1; slave_index < hssnum[index]; slave_index++)
        {
            p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hssidx[index]+slave_index);
            if (p_hss)
            {
                break;
            }
        }
        if ((slave_index != hssnum[index]) && (0 == hss_master_init))
        {
            hss_master_need_init = 1;
        }

        /*get hssid by hssidx*/
        hss_id = 0;

        /*release HSSRESET */
        if ((1 == hss_master_need_init) && (0 == hssidx[index]))
        {
            tb_id = HsCfg0_t + hss_id;
            value = 0;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssPdwnPllA_f, &value, &hs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidB_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            value = 1;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssRefClkValidA_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            value = 0;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, HsCfg0_cfgHss15gHssReset_f, &value, &hs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_cfg);

            tb_id = HsMacroMiscMon0_t + hss_id;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_mon);

            if (0 == SDK_WORK_PLATFORM)
            {
                /*check HssMonitor*/
                while(--timeout)
                {
                    DRV_IOCTL(lchip, 0, cmd, &hs_mon);
                    if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyA_f, &hs_mon))
                    {
                        plla_ready = 1;
                    }

                    if (GetHsMacroMiscMon0(V,monHss15gHssPrtReadyB_f, &hs_mon))
                    {
                        pllb_ready = 1;
                    }

                    if (plla_ready || pllb_ready)
                    {
                        break;
                    }
                }

                if ((0 == plla_ready) && (0 == pllb_ready))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Impedance Calibration fail! hssidx %d \n", hssidx[index]);
                    return CTC_E_DATAPATH_HSS_NOT_READY;
                }
            }
        }
        else if ((1 == hss_master_need_init) && (3 == hssidx[index]))
        {
            tb_id = CsCfg0_t + hss_id;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 0;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssReset_f, &value, &cs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssPdwnPllA_f, &value, &cs_cfg);
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidB_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            tb_id = CsCfg0_t + hss_id;
            cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);
            value = 1;
            DRV_IOW_FIELD(lchip, tb_id, CsCfg0_cfgHss28gHssRefClkValidA_f, &value, &cs_cfg);
            cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_cfg);

            if (0 == SDK_WORK_PLATFORM)
            {
                tb_id = CsMacroMiscMon0_t + hss_id;
                cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &cs_mon);

                while(--timeout)
                {
                    DRV_IOCTL(lchip, 0, cmd, &cs_mon);
                    if (GetCsMacroMiscMon0(V,monHss28gHssPrtReadyA_f, &cs_mon))
                    {
                        plla_ready = 1;
                    }

                    if (GetCsMacroMiscMon0(V,monHss28gHssPrtReadyB_f, &cs_mon))
                    {
                        pllb_ready = 1;
                    }


                    if (plla_ready || pllb_ready)
                    {
                        break;
                    }
                }

                /*check ready*/
                if ((0 == plla_ready) && (0 == pllb_ready))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Impedance Calibration fail! hssidx %d \n", hssidx[index]);
                    return CTC_E_DATAPATH_HSS_NOT_READY;
                }

                /*check Impedance Calibration done*/
                timeout = 0x4000;
                address = DRV_HSS_ADDR_COMBINE(0x10, 0x21);
                while(--timeout)
                {
                    drv_chip_read_hss28g(lchip, hss_id, address, &serdes_value);
                    if (serdes_value & 0x01)
                    {
                        train_done = 1;
                    }

                    if (train_done)
                    {
                        break;
                    }
                }

                if (!train_done)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Impedance Calibration train fail! hssidx %d \n", hssidx[index]);
                    return CTC_E_DATAPATH_HSS_NOT_READY;
                }
            }

            /*for 28G slave core*/
            p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, (hssidx[index]+1));
            if (p_hss)
            {
                /*bc to all tx lane*/
                address = DRV_HSS_ADDR_COMBINE(0x11, 0x02);
                drv_chip_read_hss28g(lchip, hss_id+1, address, &serdes_value);
                serdes_value |= 0x01;
                drv_chip_write_hss28g(lchip, hss_id+1, address, serdes_value);
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_misc_cfg(uint8 lchip)
{
    uint8  slice_id  = 0;
    uint16 serdes_id = 0;
    uint16 tbl_id = 0;
    uint16 fld_id = 0;
    uint8 lane_id = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 step = 0;
    RlmHsCtlReset_m hs_rst;
    RlmCsCtlReset_m cs_rst;

    DATAPATH_INIT_CHECK();
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    for (serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

        /* reset/release, clear random value */
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            tbl_id = RlmHsCtlReset_t + slice_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_rst);
            step = RlmHsCtlReset_resetCoreHss15GAnethBWrapper0_f - RlmHsCtlReset_resetCoreHss15GAnethAWrapper0_f;
            fld_id = RlmHsCtlReset_resetCoreHss15GAnethAWrapper0_f + (serdes_id/8) + ((serdes_id%8)*3);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"tbl_id:%d, fld_id:%d\n", tbl_id, fld_id);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_rst);

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_rst);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_rst);
        }
        else
        {
            tbl_id = RlmCsCtlReset_t + slice_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_rst);
            step = RlmCsCtlReset_resetCoreHss28GAnethBWrapper0_f - RlmCsCtlReset_resetCoreHss28GAnethAWrapper0_f;
            fld_id = RlmCsCtlReset_resetCoreHss28GAnethAWrapper0_f + ((serdes_id-24)/4) + lane_id*step;
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_rst);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"tbl_id:%d, fld_id:%d\n", tbl_id, fld_id);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_rst);

            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_rst);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_rst);

        }
    }

    return CTC_E_NONE;
}

#define __DYNAMIC_SWITCH__

int32
sys_usw_datapath_dynamic_switch_get_info(uint8 lchip, uint16 lport, uint8 serdes_id,
                 uint8 src_mode, uint8 mode, uint8 portbased_flag, sys_datapath_dynamic_switch_attr_t *attr)
{
    uint8 i = 0, j = 0, k = 0;
    int32 src_mac_cnt = 0;
    int32 dst_mac_cnt = 0;
    int32 repeat_cnt = 0;
    int32 repeat_cnt1 = 0;
    uint32 serdes_switch_flag[4] = {0, 0, 0, 0};
    int32 src_mac_list[8] = {0};
    int32 dst_mac_list[8] = {0};
    uint32 mac_flag[8] = {0};
    uint8 len = 0;
    uint8 slice_id = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 internal_lane_id = 0;
    int8 lane_id2 = -1;
    uint8 base_serdes_id = 0;
    uint8 base_mac_id = 0;
    sys_datapath_dynamic_switch_mac_t switch_mac[16];
    sys_datapath_dynamic_switch_mac_t tmp;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_group_mac_info_t src_serdes_group_mac_attr;
    sys_datapath_serdes_group_mac_info_t dst_serdes_group_mac_attr;

    sal_memset(&src_serdes_group_mac_attr, 0, sizeof(sys_datapath_serdes_group_mac_info_t));
    sal_memset(&dst_serdes_group_mac_attr, 0, sizeof(sys_datapath_serdes_group_mac_info_t));
    sal_memset(switch_mac, 0, 16*sizeof(sys_datapath_dynamic_switch_mac_t));
    sal_memset(&tmp, 0, sizeof(sys_datapath_dynamic_switch_mac_t));

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }


    base_serdes_id = serdes_id/4*4;
    SYS_DATAPATH_GET_BASE_MAC_BY_SERDES(serdes_id, mode, base_mac_id);

    len = sizeof(g_serdes_group_mac_attr)/sizeof(g_serdes_group_mac_attr[0]);
    for (i = 0; i < len; i++)
    {
        if (mode == g_serdes_group_mac_attr[i].port_mode)
        {
            sal_memcpy(&dst_serdes_group_mac_attr, &g_serdes_group_mac_attr[i], sizeof(g_serdes_group_mac_attr[i]));
        }
    }

    for (i = 0; i < 4; i++)
    {
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(base_serdes_id+i);
        p_serdes = &p_hss_vec->serdes_info[lane_id];
        if ((CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode)
            || (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode)
            || (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
        {
            serdes_switch_flag[i] = 1;
        }
    }

    if (CTC_CHIP_SERDES_NONE_MODE == src_mode)
    {
        for (i = 0; i < 4; i++)
        {
            lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(base_serdes_id+i);
            p_serdes = &p_hss_vec->serdes_info[lane_id];
            if ((CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode)
                || (CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode)
                || (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
            {
                src_mode = p_serdes->mode;
                break;
            }
        }
    }

    if (p_hss_vec->serdes_info[0].group != p_hss_vec->serdes_info[1].group)
    {
        for (i = 0; i < len; i++)
        {
            if (CTC_CHIP_SERDES_LG_MODE == g_serdes_group_mac_attr[i].port_mode)
            {
                /* update serdes/mac mapping when 50G flag is 1 */
                g_serdes_group_mac_attr[i].mac_list[0][0] = 2;
                g_serdes_group_mac_attr[i].mac_list[2][0] = 0;
            }
        }
    }

    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(base_serdes_id);
    internal_lane_id = serdes_id % 4;

    serdes_switch_flag[internal_lane_id] = 1;

    /* get src and dst info */
    for (i = 0; i < len; i++)
    {
        if (src_mode == g_serdes_group_mac_attr[i].port_mode)
        {
            sal_memcpy(&src_serdes_group_mac_attr, &g_serdes_group_mac_attr[i], sizeof(g_serdes_group_mac_attr[i]));
        }
    }

    if (SYS_COMMON_USELESS_MAC != lport)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    }
    if (port_attr && (CTC_CHIP_SERDES_NONE_MODE == dst_serdes_group_mac_attr.port_mode))
    {
        if (src_serdes_group_mac_attr.mac_num >= src_serdes_group_mac_attr.serdes_num)
        {
            attr->serdes_num = 1;
            attr->s[0].serdes_id = port_attr->serdes_id;
            attr->s[0].src_mode = src_serdes_group_mac_attr.port_mode;
            attr->s[0].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
            attr->mac_num  = src_serdes_group_mac_attr.mac_num/src_serdes_group_mac_attr.serdes_num;
            if (1 == attr->mac_num)
            {
                attr->m[0].mac_id = port_attr->mac_id;
                attr->m[0].add_drop_flag = 2;
            }
            else
            {
                for (i = 0; i < attr->mac_num; i++)
                {
                    attr->m[i].mac_id = port_attr->mac_id/4*4+i;
                    attr->m[i].add_drop_flag = 2;
                }
            }
        }
        else
        {
            attr->serdes_num = 4;
            for (i = 0; i < 4; i++)
            {
                attr->s[i].serdes_id = base_serdes_id+i;
                attr->s[i].src_mode = src_serdes_group_mac_attr.port_mode;
                attr->s[i].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
            }
            attr->mac_num = 4 / src_serdes_group_mac_attr.serdes_num;
            for (i = 0; i < attr->mac_num; i++)
            {
                attr->m[i].mac_id = port_attr->mac_id/4*4+i*2;
                attr->m[i].add_drop_flag = 2;
            }
        }

        return CTC_E_NONE;
    }


    for (i = 0; i < 4; i++)
    {
        p_serdes = &p_hss_vec->serdes_info[i+lane_id];
        for (j = 0; j < len; j++)
        {
            if (p_serdes->mode == g_serdes_group_mac_attr[j].port_mode)
            {
                for (k = 0; k < 4; k++)
                {
                    src_serdes_group_mac_attr.mac_list[i][k] = g_serdes_group_mac_attr[j].mac_list[i][k];
                }
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (internal_lane_id == i)  /* current serdes/mac doing switch */
            continue;
        if ((dst_serdes_group_mac_attr.mac_list[internal_lane_id][0] == dst_serdes_group_mac_attr.mac_list[i][0])
            && (-1 != dst_serdes_group_mac_attr.mac_list[internal_lane_id][0]))
        {
            serdes_switch_flag[i] = 1;
            lane_id2 = i;
        }

        if ((src_serdes_group_mac_attr.mac_list[internal_lane_id][0] == src_serdes_group_mac_attr.mac_list[i][0])
            && (-1 != src_serdes_group_mac_attr.mac_list[internal_lane_id][0]))
        {
            serdes_switch_flag[i] = 1;
            lane_id2 = i;
        }
    }
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            /* MAC infection estimation */
            for (k = 0; k < 4; k++)
            {
                if ((dst_serdes_group_mac_attr.mac_list[internal_lane_id][k] == src_serdes_group_mac_attr.mac_list[i][j])
                    && (-1 != dst_serdes_group_mac_attr.mac_list[internal_lane_id][k]))
                {
                    serdes_switch_flag[i] = 1;
                }

                if (lane_id2 >= 0)
                {
                    if ((dst_serdes_group_mac_attr.mac_list[lane_id2][k] == src_serdes_group_mac_attr.mac_list[i][j])
                        && (-1 != dst_serdes_group_mac_attr.mac_list[lane_id2][k]))
                    {
                        serdes_switch_flag[i] = 1;
                    }

                    if ((src_serdes_group_mac_attr.mac_list[lane_id2][k] == dst_serdes_group_mac_attr.mac_list[i][j])
                        && (-1 != src_serdes_group_mac_attr.mac_list[lane_id2][k]))
                    {
                        serdes_switch_flag[i] = 1;
                    }
                }
            }
        }
    }

    if (dst_serdes_group_mac_attr.mac_list[internal_lane_id][0] != src_serdes_group_mac_attr.mac_list[internal_lane_id][0])
    {
        for (i = 0; i < 4; i++)
        {
            if (i == internal_lane_id)
            {
                continue;
            }

            if ((dst_serdes_group_mac_attr.mac_list[i][0] == src_serdes_group_mac_attr.mac_list[internal_lane_id][0])
                && (-1 != dst_serdes_group_mac_attr.mac_list[i][0]))
            {
                serdes_switch_flag[i] = 1;
            }
        }
    }

    /* Include serdes which cannot indexed by port */
    for (i = 0; i < 4; i++)
    {
        if ((CTC_CHIP_SERDES_NONE_MODE != src_mode)
            && (-1 == src_serdes_group_mac_attr.mac_list[i][0])
            && (-1 != dst_serdes_group_mac_attr.mac_list[i][0])
            && portbased_flag)
        {
            if ((CTC_CHIP_SERDES_QSGMII_MODE != src_mode)
                 && (CTC_CHIP_SERDES_USXGMII0_MODE != src_mode)
                 && (CTC_CHIP_SERDES_USXGMII1_MODE != src_mode)
                 && (CTC_CHIP_SERDES_USXGMII2_MODE != src_mode))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port-based dynamic switch DO NOT support serdes none mode \n");
                return CTC_E_INVALID_CONFIG;
            }
            serdes_switch_flag[i] = 1;
        }
    }

    /* specially */
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    p_serdes = &p_hss_vec->serdes_info[lane_id];
    if ((CTC_CHIP_SERDES_LG_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_LG_MODE == mode))
    {
        serdes_switch_flag[0] = 1;
        serdes_switch_flag[1] = 1;
        serdes_switch_flag[2] = 1;
        serdes_switch_flag[3] = 1;
    }
    if ((CTC_CHIP_SERDES_USXGMII0_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == mode))
    {
        serdes_switch_flag[0] = 1;
        serdes_switch_flag[1] = 1;
        serdes_switch_flag[2] = 1;
        serdes_switch_flag[3] = 1;
    }

    if (((src_serdes_group_mac_attr.mac_num > src_serdes_group_mac_attr.serdes_num)
        && (dst_serdes_group_mac_attr.mac_num <= dst_serdes_group_mac_attr.serdes_num))
        || ((dst_serdes_group_mac_attr.mac_num > dst_serdes_group_mac_attr.serdes_num)
        && (src_serdes_group_mac_attr.mac_num <= src_serdes_group_mac_attr.serdes_num)))
    {
        for (i = 0; i < 4; i++)
        {
            if ((src_serdes_group_mac_attr.mac_list[i][0] == dst_serdes_group_mac_attr.mac_list[i][0])
                && (-1 == src_serdes_group_mac_attr.mac_list[i][0]))
            {
                continue;
            }
            serdes_switch_flag[i] = 1;
        }
    }

    /*################ !!! PLS DO NOT MODIFY ###################*/
    for (i = 0,k = 0; i < 4; i++)
    {
        if (0 == serdes_switch_flag[i])
        {
            continue;
        }
        attr->s[k].serdes_id = base_serdes_id+i;
        attr->s[k].src_mode = p_serdes->mode;
        attr->s[k].dst_mode = mode;
        if ((-1 != src_serdes_group_mac_attr.mac_list[i][0]) && (-1 == dst_serdes_group_mac_attr.mac_list[i][0]))
        {
            attr->s[k].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
        }
        /* get src mac list and dest mac list */
        for (j = 0; j < 4; j++)
        {
            if (-1 != src_serdes_group_mac_attr.mac_list[i][j])
            {
                src_mac_list[src_mac_cnt++] = src_serdes_group_mac_attr.mac_list[i][j];
            }

            if (-1 != dst_serdes_group_mac_attr.mac_list[i][j])
            {
                dst_mac_list[dst_mac_cnt++] = dst_serdes_group_mac_attr.mac_list[i][j];
            }
        }
        k++;
    }
    attr->serdes_num = k;

    /* calculate the combined mac list and add_drop_flag */
    for (i = 0; i < src_mac_cnt; i++)
    {
        switch_mac[i].mac_id = src_mac_list[i];
        switch_mac[i].add_drop_flag = 1;
    }
    for (i = 0; i < dst_mac_cnt; i++)
    {
        switch_mac[i+src_mac_cnt].mac_id = dst_mac_list[i];
        switch_mac[i+src_mac_cnt].add_drop_flag = 1;
    }
    for (i = 0; i < src_mac_cnt; i++)
    {
        for (j = 0; j < dst_mac_cnt; j++)
        {
            if (src_mac_list[i] == dst_mac_list[j])
            {
                mac_flag[i] = 1;
                switch_mac[j+src_mac_cnt].mac_id = -1;
            }
        }
        if (0 == mac_flag[i])
        {
            switch_mac[i].add_drop_flag = 2;
        }
        else
        {
            switch_mac[i].add_drop_flag = 0;
        }
    }

    /* switch_mac is Union Set organized by src_mac_list and dst_mac_list */
    /* #1, delete -1 elements */
    for (i = 0; i < (src_mac_cnt+dst_mac_cnt); i++)
    {
        if (-1 == switch_mac[i].mac_id)
        {
            do
            {
                for (j = i; j < (src_mac_cnt+dst_mac_cnt-repeat_cnt); j++)
                {
                    switch_mac[j].mac_id = switch_mac[j+1].mac_id;
                    switch_mac[j].add_drop_flag = switch_mac[j+1].add_drop_flag;
                }
                repeat_cnt++;
            }while (-1 == switch_mac[i].mac_id);
        }
    }

    /* #2, bubble sort */
    for (i = 0; i < (src_mac_cnt+dst_mac_cnt-repeat_cnt); i++)
    {
        for (j = 0; j < (src_mac_cnt+dst_mac_cnt-repeat_cnt-i-1); j++)
        {
            if (switch_mac[j].mac_id > switch_mac[j+1].mac_id)
            {
                tmp.mac_id = switch_mac[j].mac_id;
                tmp.add_drop_flag = switch_mac[j].add_drop_flag;
                switch_mac[j].mac_id = switch_mac[j+1].mac_id;
                switch_mac[j].add_drop_flag = switch_mac[j+1].add_drop_flag;
                switch_mac[j+1].mac_id = tmp.mac_id;
                switch_mac[j+1].add_drop_flag = tmp.add_drop_flag;
            }
        }
    }

    /* #3, delete repeated elements */
    for (i = 0; i < (src_mac_cnt+dst_mac_cnt-repeat_cnt-1); i++)
    {
        while ((switch_mac[i].mac_id == switch_mac[i+1].mac_id) && (-1 != switch_mac[i].mac_id))
        {
            for (j = i; j < (src_mac_cnt+dst_mac_cnt-1); j++)
            {
                switch_mac[j].mac_id = switch_mac[j+1].mac_id;
                switch_mac[j].add_drop_flag = switch_mac[j+1].add_drop_flag;
            }
            switch_mac[src_mac_cnt+dst_mac_cnt-1-repeat_cnt].mac_id = -1;
            repeat_cnt1++;
        }
    }

    /* get mac info */
    if ((src_mac_cnt+dst_mac_cnt-repeat_cnt-repeat_cnt1-1) < 0)
    {
        attr->mac_num = src_mac_cnt + dst_mac_cnt - repeat_cnt - repeat_cnt1 + 1;
    }
    else
    {
        attr->mac_num = src_mac_cnt + dst_mac_cnt - repeat_cnt - repeat_cnt1;
    }

    for (i = 0; i < attr->mac_num; i++)
    {
        attr->m[i].mac_id = switch_mac[i].mac_id + base_mac_id;
        if (((8 == base_mac_id) || (40 == base_mac_id)) && (switch_mac[i].mac_id >= 4))
        {
            attr->m[i].mac_id += 4;
        }
        else if (((12 == base_mac_id) || (44 == base_mac_id)) && (switch_mac[i].mac_id <= 3))
        {
            if (!SYS_DATAPATH_SERDES_IS_HSS28G(attr->s[i].serdes_id))
            {
                attr->m[i].mac_id -= 4;
            }
        }
        attr->m[i].add_drop_flag = switch_mac[i].add_drop_flag;
    }
    /*################ !!! DO NOT MODIFY END ###################*/

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_check_serdes_pll(uint8 lchip, uint8 mode, uint16 overclocking_speed, sys_datapath_dynamic_switch_attr_t* p_ds_attr, int16 s_flag)
{
    uint8 hss_idx = 0;
    uint8 hss_type = 0;
    uint8 hssclk = 0;
    uint8 hss_lane_num = 0;
    uint8 rate_div = 0;
    uint8 bit_width = 0;
    uint8 core_div = 0;
    uint8 intf_div = 0;
    uint32 i = 0, j = 0, cnt = 0;
    uint32 plla_clk[HSS15G_LANE_NUM] = {0};
    uint32 pllb_clk[HSS15G_LANE_NUM] = {0};
    uint8  plla_all_zero = 0;
    uint8  pllb_all_zero = 0;
    uint16 base_serdes_id = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    /*hss_idx used to index vector, hss_id is actual id*/
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_ds_attr->s[0].serdes_id);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[0].serdes_id))
    {
        base_serdes_id = p_ds_attr->s[0].serdes_id - p_ds_attr->s[0].serdes_id%4;
        hss_type = SYS_DATAPATH_HSS_TYPE_28G;
        hss_lane_num = HSS28G_LANE_NUM;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_rate(lchip, mode,SYS_DATAPATH_HSS_TYPE_28G,
            &hssclk, &bit_width, &rate_div, &core_div, &intf_div, overclocking_speed));
    }
    else
    {
        base_serdes_id = p_ds_attr->s[0].serdes_id - p_ds_attr->s[0].serdes_id%8;
        hss_type = SYS_DATAPATH_HSS_TYPE_15G;
        hss_lane_num = HSS15G_LANE_NUM;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_rate(lchip, mode,SYS_DATAPATH_HSS_TYPE_15G,
            &hssclk, &bit_width, &rate_div, &core_div, &intf_div, overclocking_speed));
    }

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);

    for (i = 0; i < hss_lane_num; i++)
    {
        p_serdes = &p_hss_vec->serdes_info[i];
        if (SYS_DATAPATH_PLL_SEL_PLLA == p_serdes->pll_sel)
        {
            plla_clk[i] = p_hss_vec->plla_mode;
        }

        if (SYS_DATAPATH_PLL_SEL_PLLB == p_serdes->pll_sel)
        {
            pllb_clk[i] = p_hss_vec->pllb_mode;
        }
    }

    for (i = 0; i < p_ds_attr->serdes_num; i++)
    {
        j = i;
        if (1 == p_ds_attr->serdes_num)
        {
            j = p_ds_attr->s[0].serdes_id % hss_lane_num;
        }
        else if ((SYS_DATAPATH_HSS_TYPE_15G == hss_type)
            && ((p_ds_attr->s[0].serdes_id%8)>=4))
        {
            j = i+4;
        }

        if (SYS_DATAPATH_HSSCLK_DISABLE == hssclk)
        {
            plla_clk[j] = 0;
            pllb_clk[j] = 0;
        }
        else
        {
            if (p_hss_vec->plla_mode == hssclk)
            {
                /*one pll can only provide one intf div*/
                if ((p_hss_vec->intf_div_a != intf_div) && (p_hss_vec->intf_div_a)&&(intf_div))
                {
                    if (((mode == CTC_CHIP_SERDES_SGMII_MODE) || (mode == CTC_CHIP_SERDES_QSGMII_MODE))
                        && ((p_hss_vec->pllb_mode == hssclk) || (p_hss_vec->pllb_mode == SYS_DATAPATH_HSSCLK_DISABLE)))
                    {
                        plla_clk[j] = 0;
                        pllb_clk[j] = hssclk;
                    }
                    else
                    {
                        if (s_flag >= 0)
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Set serdes ");
                            for (i = 0; i < hss_lane_num; i++)
                            {
                                if ((pllb_clk[i] != hssclk)
                                    && (pllb_clk[i] != 0))
                                {
                                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                                    cnt = 1;
                                }
                            }
                            if (cnt)
                            {
                                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b or ");
                                cnt = 0;
                            }
                            for (i = 0; i < hss_lane_num; i++)
                            {
                                if (plla_clk[i])
                                {
                                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                                }
                            }
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b to NONE mode first!! \n");
                            return CTC_E_NOT_SUPPORT;
                        }
                        plla_clk[j] = hssclk+1;
                        pllb_clk[j] = 0;
                    }
                }
                else
                {
                    plla_clk[j] = hssclk;
                    pllb_clk[j] = 0;
                }
            }
            else if (p_hss_vec->pllb_mode == hssclk)
            {
                /*one pll can only provide one intf div*/
                if ((p_hss_vec->intf_div_b != intf_div) && (p_hss_vec->intf_div_b)&&(intf_div))
                {
                    if (((mode == CTC_CHIP_SERDES_SGMII_MODE) || (mode == CTC_CHIP_SERDES_QSGMII_MODE))
                        && ((p_hss_vec->plla_mode == hssclk) || (p_hss_vec->plla_mode == SYS_DATAPATH_HSSCLK_DISABLE)))
                    {
                        pllb_clk[j] = 0;
                        plla_clk[j] = hssclk;
                    }
                    else
                    {
                        if (s_flag >= 0)
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Set serdes ");
                            for (i = 0; i < hss_lane_num; i++)
                            {
                                if ((plla_clk[i] != hssclk)
                                    && (plla_clk[i] != 0))
                                {
                                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                                    cnt = 1;
                                }
                            }
                            if (cnt)
                            {
                                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b or ");
                                cnt = 0;
                            }
                            for (i = 0; i < hss_lane_num; i++)
                            {
                                if (pllb_clk[i])
                                {
                                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                                }
                            }
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b to NONE mode first!! \n");
                            return CTC_E_NOT_SUPPORT;
                        }
                        plla_clk[j] = 0;
                        pllb_clk[j] = hssclk+1;
                    }
                }
                else
                {
                    plla_clk[j] = 0;
                    pllb_clk[j] = hssclk;
                }

            }
            else if ((p_hss_vec->plla_mode == SYS_DATAPATH_HSSCLK_DISABLE) &&
                        (((mode != CTC_CHIP_SERDES_XAUI_MODE)
                        && (mode != CTC_CHIP_SERDES_DXAUI_MODE)
                        && (mode != CTC_CHIP_SERDES_XXVG_MODE)
                        && (mode != CTC_CHIP_SERDES_LG_MODE)
                        && (mode != CTC_CHIP_SERDES_CG_MODE)
                        && (mode != CTC_CHIP_SERDES_2DOT5G_MODE)
                        && SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[i].serdes_id)) ||
                        (!SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[i].serdes_id)
                        && (overclocking_speed != CTC_CHIP_SERDES_OCS_MODE_12_58G))))
            {
                plla_clk[j] = hssclk;
                pllb_clk[j] = 0;
            }
            else if (p_hss_vec->pllb_mode == SYS_DATAPATH_HSSCLK_DISABLE)
            {
                plla_clk[j] = 0;
                pllb_clk[j] = hssclk;
            }
            else
            {
                if (s_flag >= 0)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Set serdes ");
                    for (i = 0; i < hss_lane_num; i++)
                    {
                        if ((plla_clk[i] != hssclk)
                            && (plla_clk[i] != 0))
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                            cnt = 1;
                        }
                    }
                    if (cnt)
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b or ");
                        cnt = 0;
                    }
                    for (i = 0; i < hss_lane_num; i++)
                    {
                        if ((pllb_clk[i] != hssclk)
                            && (pllb_clk[i] != 0))
                        {
                            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%d/", (base_serdes_id+i));
                        }
                    }
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\b to NONE mode first!! \n");
                    return CTC_E_NOT_SUPPORT;
                }
                plla_clk[j] = 0;
                pllb_clk[j] = hssclk;
            }
        }
    }

    plla_all_zero = 1;
    pllb_all_zero = 1;
    for (i = 0; i < hss_lane_num; i++)
    {
        if (0 != plla_clk[i])
            plla_all_zero = 0;
        if (0 != pllb_clk[i])
            pllb_all_zero = 0;
    }

    for (i = 0; i < p_ds_attr->serdes_num; i++)
    {
        j = i;
        if (1 == p_ds_attr->serdes_num)
        {
            j = p_ds_attr->s[0].serdes_id % hss_lane_num;
        }
        else if ((SYS_DATAPATH_HSS_TYPE_15G == hss_type)
            && ((p_ds_attr->s[0].serdes_id%8)>=4))
        {
            j = i+4;
        }

        if ((p_hss_vec->plla_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            && (SYS_DATAPATH_HSSCLK_DISABLE != hssclk)
            && (((mode != CTC_CHIP_SERDES_XAUI_MODE)
                && (mode != CTC_CHIP_SERDES_DXAUI_MODE)
                && (mode != CTC_CHIP_SERDES_XXVG_MODE)
                && (mode != CTC_CHIP_SERDES_LG_MODE)
                && (mode != CTC_CHIP_SERDES_CG_MODE)
                && (mode != CTC_CHIP_SERDES_2DOT5G_MODE)
                && SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[i].serdes_id)) ||
                (!SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[i].serdes_id)
                && (overclocking_speed != CTC_CHIP_SERDES_OCS_MODE_12_58G)))
            && (1 == plla_all_zero) /* if plla not used, switch to plla */
            && (-1 == s_flag))
        {
            plla_clk[j] = hssclk;
            pllb_clk[j] = 0;
        }
        if ((p_hss_vec->pllb_mode != SYS_DATAPATH_HSSCLK_DISABLE)
            && (SYS_DATAPATH_HSSCLK_DISABLE != hssclk)
            && (1 == pllb_all_zero)  /* if pllb not used, switch to pllb */
            && (-1 == s_flag))
        {
            plla_clk[j] = 0;
            pllb_clk[j] = hssclk;
        }
    }

    for (i = 0; i < hss_lane_num; i++)
    {
        if (pllb_clk[i] == 0)
        {
            continue;
        }
        for (j = i; j < hss_lane_num; j++)
        {
            if ((pllb_clk[j] != 0) && (pllb_clk[j] != pllb_clk[i]))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide more core divider, please check!! \n");
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;
            }
        }
    }

    for (i = 0; i < hss_lane_num; i++)
    {
        if (plla_clk[i] == 0)
        {
            continue;
        }
        for (j = i; j < hss_lane_num; j++)
        {
            if ((plla_clk[j] != 0) && (plla_clk[j] != plla_clk[i]))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"One PLL can not provide more core divider, please check!! \n");
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_set_serdes_none_mode(uint8 lchip, uint8 serdes_id)
{
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 ori_plla = 0;
    uint8 ori_pllb = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];
    if (CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode)
    {
        return CTC_E_NONE;
    }

    ori_plla = p_hss_vec->plla_mode;
    ori_pllb = p_hss_vec->pllb_mode;

    /*Only used for dynamic switch, swith Used Mode to Disable
       In thie case, clockTree and hssCfg do not change*/
    if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLB)
    {
        p_hss_vec->pllb_ref_cnt--;
    }
    else if (p_hss_vec->serdes_info[lane_id].pll_sel == SYS_DATAPATH_PLL_SEL_PLLA)
    {
        p_hss_vec->plla_ref_cnt--;
    }

    p_hss_vec->serdes_info[lane_id].pll_sel = SYS_DATAPATH_PLL_SEL_NONE;
    p_hss_vec->serdes_info[lane_id].mode = CTC_CHIP_SERDES_NONE_MODE;


    if (p_hss_vec->plla_ref_cnt == 0)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Need Disable Plla!! \n");
        /*Disable plla hssclk*/
        p_hss_vec->plla_mode = SYS_DATAPATH_HSSCLK_DISABLE;
        p_hss_vec->core_div_a[0] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->core_div_a[1] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->intf_div_a = SYS_DATAPATH_CLKTREE_DIV_USELESS;
    }

    if (p_hss_vec->pllb_ref_cnt == 0)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Need Disable Pllb!! \n");
        /*Disable pllb hssclk*/
        p_hss_vec->pllb_mode = SYS_DATAPATH_HSSCLK_DISABLE;
        p_hss_vec->core_div_b[0] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->core_div_b[1] = SYS_DATAPATH_CLKTREE_DIV_USELESS;
        p_hss_vec->intf_div_b = SYS_DATAPATH_CLKTREE_DIV_USELESS;
    }

    /*5.2 reconfig Hss */
    if (ori_plla != p_hss_vec->plla_mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Need change plla cfg, ori:%d, new:%d \n", ori_plla, p_hss_vec->plla_mode);

        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA, p_hss_vec->plla_mode));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA, p_hss_vec->plla_mode));
        }
    }
    if (ori_pllb != p_hss_vec->pllb_mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Need change pllb cfg, ori:%d, new:%d \n", ori_pllb, p_hss_vec->pllb_mode);
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB, p_hss_vec->pllb_mode));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB, p_hss_vec->pllb_mode));
        }
    }
    sys_usw_datapath_set_link_reset(lchip, serdes_id, TRUE, SYS_DATAPATH_SERDES_DIR_BOTH);
    _sys_usw_datapath_set_link_enable(lchip, serdes_id, FALSE, SYS_DATAPATH_SERDES_DIR_BOTH);

    /*5.4 reconfig hss clktree */
    if (ori_plla != p_hss_vec->plla_mode)
    {
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA));
        }
    }
    if (ori_pllb != p_hss_vec->pllb_mode)
    {
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB));
        }
    }
    /*5.5 reconfig per lane clktree */
    if (p_hss_vec->hss_type)
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_hss28g_cfg_lane_clktree(lchip, lane_id, p_hss_vec));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_hss15g_cfg_lane_clktree(lchip, lane_id, p_hss_vec));
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_serdes_mode(uint8 lchip, uint8 serdes_id, uint8 mode, uint16 overclocking_speed)
{
    int32 ret = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 ori_plla = 0;
    uint8 ori_pllb = 0;
    uint8 ori_core_div_a[SYS_DATAPATH_CORE_NUM] = {0};
    uint8 ori_core_div_b[SYS_DATAPATH_CORE_NUM] = {0};
    uint8 ori_intf_div_a = 0;
    uint8 ori_intf_div_b = 0;
    uint16 address = 0;
    uint16 serdes_val = 0;
    uint8 internal_lane = 0;
    uint8 plla_recfg_flag = 0;
    uint8 pllb_recfg_flag = 0;
    uint8 ffe_mode = 0;
    uint8 ffe_status = 0;
    uint16 coefficient[4] = {0};
    ctc_chip_serdes_ffe_t ffe;
    ctc_chip_serdes_polarity_t serdes_polarity;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    sal_memset(&ffe, 0, sizeof(ctc_chip_serdes_ffe_t));
    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d,  Mode:%d \n", serdes_id, mode);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;

    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];

    ori_plla = p_hss_vec->plla_mode;
    ori_pllb = p_hss_vec->pllb_mode;
    ori_core_div_a[0] = p_hss_vec->core_div_a[0];
    ori_core_div_a[1] = p_hss_vec->core_div_a[1];
    ori_core_div_b[0] = p_hss_vec->core_div_b[0];
    ori_core_div_b[1] = p_hss_vec->core_div_b[1];
    ori_intf_div_a = p_hss_vec->intf_div_a;
    ori_intf_div_b = p_hss_vec->intf_div_b;

    /*5. serdes switch */
    /* get ffe value */
    internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(p_hss_vec->hss_type, lane_id);
    address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
    DRV_CHIP_READ_HSS(p_hss_vec->hss_type, lchip, p_hss_vec->hss_id, address, &serdes_val);
    serdes_val &= 0x10;
    ffe_mode = (serdes_val)?CTC_CHIP_SERDES_FFE_MODE_3AP:0;  /* 3ap mode, traditional mode */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe(lchip, serdes_id, coefficient, ffe_mode, &ffe_status));

    /*5.1 rebuild serdes info */
    CTC_ERROR_RETURN(sys_usw_datapath_build_serdes_info(lchip, serdes_id, mode,
        p_hss_vec->serdes_info[internal_lane].group, 1, overclocking_speed));

    /*5.2 reconfig Hss */
    if (ori_plla != p_hss_vec->plla_mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Need change plla cfg, ori:%d, new:%d \n", ori_plla, p_hss_vec->plla_mode);

        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA, p_hss_vec->plla_mode));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA, p_hss_vec->plla_mode));
        }
    }
    if (ori_pllb != p_hss_vec->pllb_mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Need change pllb cfg, ori:%d, new:%d \n", ori_pllb, p_hss_vec->pllb_mode);
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB, p_hss_vec->pllb_mode));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_pll_cfg(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB, p_hss_vec->pllb_mode));
        }
    }
    /*5.3 reconfig hss internal register for rx */
    CTC_ERROR_RETURN(sys_usw_datapath_set_hss_internal(lchip, serdes_id, mode));
    /*5.4 reconfig hss clktree */
    if (ori_plla != p_hss_vec->plla_mode)
    {
        plla_recfg_flag = 1;
    }
    else if ((ori_core_div_a[0] != p_hss_vec->core_div_a[0]) || (ori_core_div_a[1] != p_hss_vec->core_div_a[1]))
    {
        plla_recfg_flag = 1;
    }
    else if (ori_intf_div_a != p_hss_vec->intf_div_a)
    {
        plla_recfg_flag = 1;
    }
    else
    {
        plla_recfg_flag = 0;
    }

    if (ori_pllb != p_hss_vec->pllb_mode)
    {
        pllb_recfg_flag = 1;
    }
    else if ((ori_core_div_b[0] != p_hss_vec->core_div_b[0]) || (ori_core_div_b[1] != p_hss_vec->core_div_b[1]))
    {
        pllb_recfg_flag = 1;
    }
    else if (ori_intf_div_b != p_hss_vec->intf_div_b)
    {
        pllb_recfg_flag = 1;
    }
    else
    {
        pllb_recfg_flag = 0;
    }

    if (plla_recfg_flag)
    {
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLA));
        }
    }
    if (pllb_recfg_flag)
    {
        if (p_hss_vec->hss_type)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss28g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_datapath_hss15g_cfg_clktree(lchip, hss_idx, SYS_DATAPATH_PLL_SEL_PLLB));
        }
    }
    /*5.5 reconfig per lane clktree */
    if (p_hss_vec->hss_type)
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_hss28g_cfg_lane_clktree(lchip, lane_id, p_hss_vec));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_datapath_hss15g_cfg_lane_clktree(lchip, lane_id, p_hss_vec));
    }

    /*5. 6 reset serdes tx link, should re-cfg ffe, set poly */
    CTC_ERROR_RETURN(sys_usw_datapath_set_link_reset(lchip, (serdes_id), TRUE, 1));
    sal_task_sleep(1);
    CTC_ERROR_RETURN(sys_usw_datapath_set_link_reset(lchip, (serdes_id), FALSE, 1));
    sal_task_sleep(1);

    serdes_polarity.serdes_id = serdes_id;

    serdes_polarity.dir = 0;
    serdes_polarity.polarity_mode= (p_serdes->rx_polarity)?1:0;
    CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &serdes_polarity));

    serdes_polarity.dir = 1;
    serdes_polarity.polarity_mode = (p_serdes->tx_polarity)?1:0;
    CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, &serdes_polarity));

    /*5.7 cfg ffe, need select ffe_mode, 0--traditional mode, 1--3ap mode */
    if(0 == ffe_mode)
    {
        if (p_hss_vec->hss_type == 0) /*hss15g using sum mode*/
        {
            internal_lane = g_usw_hss15g_lane_map[lane_id];
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 2);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, p_hss_vec->hss_id, address, &serdes_val));
            serdes_val &= 0xff7f;
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, p_hss_vec->hss_id, address, serdes_val));
            address = DRV_HSS_ADDR_COMBINE(g_usw_hss_tx_addr_map[internal_lane], 0x02);
            CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, p_hss_vec->hss_id, address, &serdes_val));
            serdes_val |= 0x01;
            CTC_ERROR_RETURN(drv_chip_write_hss15g(lchip, p_hss_vec->hss_id, address, serdes_val));
        }
        ffe.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
    }
    else
    {
        ffe.mode = CTC_CHIP_SERDES_FFE_MODE_3AP;
    }
    ffe.serdes_id = serdes_id;
    ffe.coefficient[0] = coefficient[0];
    ffe.coefficient[1] = coefficient[1];
    ffe.coefficient[2] = coefficient[2];
    ffe.coefficient[3] = coefficient[3];
    CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe(lchip, &ffe));

    return ret;
}

STATIC int32
_sys_usw_datapath_set_pre_dynamic_switch(uint8 lchip, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint32 depth = 0;
    uint32 nettx_used = 0;
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 gchip = 0;
    uint8 chan_id = 0;
    uint8 slice_id = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    uint32 i = 0;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_qos_shape_profile_t shp_profile[8];
    ctc_chip_serdes_loopback_t lb_param;

    for (i = 0; i < 8; i++)
    {
        sal_memset(&shp_profile[i], 0, sizeof(sys_qos_shape_profile_t));
    }

    for (i = 0; i < ds_attr->mac_num; i++)
    {
        if (1 != ds_attr->m[i].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                continue;
            }
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);

            /* clear unidir cfg */
            CTC_ERROR_RETURN(_sys_usw_mac_set_unidir_en(lchip, tmp_lport, FALSE));
            /* clear FEC if enable */
            if ((CTC_CHIP_SERDES_CG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XXVG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_LG_MODE == port_attr_tmp->pcs_mode))
            {
                CTC_ERROR_RETURN(_sys_usw_mac_set_fec_en(lchip, tmp_lport, CTC_PORT_FEC_TYPE_NONE));
            }
            /* clear mac enable cfg */
            CTC_ERROR_RETURN(_sys_usw_mac_set_mac_pre_config(lchip, tmp_lport));
            CTC_ERROR_RETURN(_sys_usw_mac_set_mac_en(lchip, tmp_gport, FALSE));
            if ((CTC_CHIP_SERDES_XFI_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XLG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XXVG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_LG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XAUI_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr_tmp->pcs_mode))
            {
                sal_task_sleep(50);
            }
        }
    }

    /* disable serdes loopback */
    for (i = 0; i < ds_attr->serdes_num; i++)
    {
        sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
        lb_param.serdes_id = ds_attr->s[i].serdes_id;
        lb_param.mode = 0;
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            lb_param.enable = 0;
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_loopback(lchip, (void*)&lb_param));
        }
        lb_param.mode = 1;
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
        if (lb_param.enable)
        {
            lb_param.enable = 0;
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_loopback(lchip, (void*)&lb_param));
        }
    }

    for (i = 0; i < ds_attr->mac_num; i++)
    {
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);

        if (1 != ds_attr->m[i].add_drop_flag)
        {
            /* 2. set vanished mac to drop channel */
            chan_id = MCHIP_CAP(SYS_CAP_CHANID_DROP);
            CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, tmp_lport, chan_id));

            /*1. If port is dest, enqdrop */
            /*for restore shape profile*/
            CTC_ERROR_RETURN(sys_usw_queue_get_profile_from_hw(lchip, tmp_gport, &shp_profile[i]));
            /*enqdrop*/
            CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, tmp_gport, TRUE, &shp_profile[i]));

            /*1.1. check queue flush clear*/
            CTC_ERROR_RETURN(sys_usw_queue_get_port_depth(lchip, tmp_gport, &depth));
            while (depth)
            {
                sal_task_sleep(20);
                if((index++) >50)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cannot flush queue depth(%d) to Zero \n", depth);
                    CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &shp_profile[i]));
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] dynamic switch fail \n");
                    return CTC_E_HW_FAIL;
                }
                CTC_ERROR_RETURN(sys_usw_queue_get_port_depth(lchip, tmp_gport, &depth));
            }

            index = 0;
            /* 2. check NetTx used */
            cmd = DRV_IOR(NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f+ds_attr->m[i].mac_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_used));
            while (nettx_used)
            {
                sal_task_sleep(20);
                if((index++) >50)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NetTxCreditStatus_port%dCreditUsed cannot return to Zero \n", tmp_gport);
                    return CTC_E_HW_FAIL;
                }
                cmd = DRV_IOR(NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f+ds_attr->m[i].mac_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_used));
            }

            sal_task_sleep(100);

            /* 3. disable enqdrop, only network port use */
            if (2 == ds_attr->m[i].add_drop_flag)
            {
                CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &shp_profile[i]));
                CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, port_attr_tmp->chan_id, 0, 0));
                CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, port_attr_tmp->chan_id, 0, 0));
                CTC_ERROR_RETURN(_sys_usw_datapath_store_mcu_data_mem(lchip, tmp_lport, 1));
                p_usw_datapath_master[lchip]->port_attr[slice_id][tmp_lport].port_type = SYS_DMPS_RSV_PORT;
            }

            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Pre dynamic switch failed \n");
                return CTC_E_INVALID_PARAM;
            }

            if (2 == ds_attr->m[i].add_drop_flag)
            {
                sys_usw_qos_remove_mcast_queue_to_channel(lchip, tmp_lport, port_attr_tmp->chan_id);
            }
        }
    }

    for (i = 0; i < ds_attr->mac_num; i++)
    {
        sal_memcpy(&(ds_attr->shp_profile[i]), &shp_profile[i], sizeof(sys_qos_shape_profile_t));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_datapath_set_post_dynamic_switch(uint8 lchip, uint16 lport, uint8 mode, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 gchip = 0;
    uint8 chan_id = 0;
    uint8 base_chan = 0;
    uint8 num     = 0;
    uint8 slice_id = 0;
    uint32 txqm_id = 0;
    uint32 cmd = 0;
    uint32 i = 0, j = 0;
    uint32 tmp_val32 = 0;
    uint32 step = 0;
    uint32 s_pointer1 = 0;
    uint16 e_pointer1 = 0;
    uint32 s_pointer2 = 0;
    uint16 e_pointer2 = 0;
    uint32 base_s_pointer1 = 0;
    uint32 base_s_pointer2 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    ctc_port_if_type_t interface_type = 0;

    port_attr = (sys_datapath_lport_attr_t*)(&p_usw_datapath_master[lchip]->port_attr[slice_id][lport]);
    if (port_attr == NULL)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (!port_attr->is_serdes_dyn)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_DATAPATH_GET_IFTYPE_WITH_MODE(mode, interface_type);

    for (i = 0; i < ds_attr->serdes_num; i++)
    {
        SYS_DATAPATH_GET_BASE_MAC_BY_SERDES(ds_attr->s[i].serdes_id, ds_attr->s[i].src_mode, base_chan);
        for (j = 0; j < SYS_DATAPATH_STUB_CHAN_NUM; j++)
        {
            if (base_chan == g_epe_mem_info[j].stub_chan_id)
            {
                base_s_pointer1 = g_epe_mem_info[j].body_s_pointer;
                base_s_pointer2 = g_epe_mem_info[j].sop_s_pointer;
                break;
            }
        }

        SYS_DATAPATH_GET_HW_MAC_BY_SERDES(ds_attr->s[i].serdes_id, ds_attr->s[i].src_mode, chan_id, num);

        tmp_val32 = SYS_DATAPATH_IS_HSS28G(chan_id) ? SPEED_MODE_25 : SPEED_MODE_10;
        s_pointer1 = base_s_pointer1 + (ds_attr->s[i].serdes_id % 4)*(g_bufsz_step[tmp_val32].body_buf_num);
        s_pointer2 = base_s_pointer2 + (ds_attr->s[i].serdes_id % 4)*(g_bufsz_step[tmp_val32].sop_buf_num);
        if ((CTC_CHIP_SERDES_QSGMII_MODE == ds_attr->s[i].dst_mode)
            || (CTC_CHIP_SERDES_USXGMII1_MODE == ds_attr->s[i].dst_mode)
            || (CTC_CHIP_SERDES_USXGMII2_MODE == ds_attr->s[i].dst_mode))
        {
            tmp_val32 = SPEED_MODE_0_5;
            s_pointer2 = base_s_pointer2 + (ds_attr->s[i].serdes_id % 4)*(g_bufsz_step[tmp_val32].sop_buf_num)*4;
        }

        SYS_DATAPATH_GET_HW_MAC_BY_SERDES(ds_attr->s[i].serdes_id, ds_attr->s[i].dst_mode, chan_id, num);
        for (j = 0; j < num; j++)
        {
            if ((SYS_COMMON_USELESS_MAC != sys_usw_datapath_get_lport_with_chan(lchip, slice_id, (chan_id+j)))
                && (CTC_CHIP_SERDES_NONE_MODE != ds_attr->s[i].dst_mode))
            {
                CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, (chan_id+j), 0, s_pointer1, &e_pointer1, NULL));
                CTC_ERROR_RETURN(_sys_usw_datapath_get_epesched_eptr(lchip, (chan_id+j), 1, s_pointer2, &e_pointer2, NULL));
                CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, (chan_id+j), s_pointer1, e_pointer1));
                CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, (chan_id+j), s_pointer2, e_pointer2));
                s_pointer1 = e_pointer1+1;
                s_pointer2 = e_pointer2+1;
            }
        }
    }
    /* update datapath */
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        if (2 == ds_attr->m[i].add_drop_flag)
        {
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_body_mem(lchip, slice_id, ds_attr->m[i].mac_id, 0, 0));
            CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_sop_mem(lchip, slice_id, ds_attr->m[i].mac_id, 0, 0));
            continue;
        }
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
        port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
        if (port_attr_tmp == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed \n");
            return CTC_E_INVALID_PARAM;
        }

        /* #1, update NetTxPortStaticInfo_portMode_f */
        _sys_usw_datapath_set_nettx_portmode(lchip, port_attr_tmp->chan_id);

        /* #3. reconfig bufretrv credit, qmgr credit don't need config, beause bufretrv alloc buffer maximal*/
        CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_credit(lchip, slice_id, port_attr_tmp->chan_id));
        CTC_ERROR_RETURN(_sys_usw_datapath_set_bufretrv_credit_config(lchip, slice_id));
    }

    /*global cfg need config at last time*/
    /* #4. reconfig Netrx*/
    CTC_ERROR_RETURN(_sys_usw_datapath_set_netrx_buf_mgr(lchip, slice_id));
    CTC_ERROR_RETURN(_sys_usw_datapath_set_netrx_wrr_weight(lchip, slice_id));

    /* #5. reconfig qmgr*/
    CTC_ERROR_RETURN(_sys_usw_datapath_set_qmgr_wrr_weight(lchip, slice_id));

    /* #6. reconfig bufretrv, buf bufretrv buffer pointer need not config*/
    CTC_ERROR_RETURN(_sys_usw_datapath_bufretrv_wrr_weight(lchip, slice_id));

    /* #7. reconfig chan distribute*/
    CTC_ERROR_RETURN(_sys_usw_datapath_net_chan_cfg(lchip));

    /* #8. reconfig calendar*/
    for (txqm_id = 0; txqm_id < 4; txqm_id++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_calendar_cfg(lchip, slice_id, txqm_id));
    }

    /* #9. reconfig epe schedule*/
    CTC_ERROR_RETURN(_sys_usw_datapath_set_epe_wrr_weight(lchip, slice_id));

    /* #10. reset QuadSgmac*/
    if (4 == ds_attr->serdes_num)
    {
        step = RlmMacCtlReset_resetCoreQuadSgmac1_f - RlmMacCtlReset_resetCoreQuadSgmac0_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmac0_f+step*(port_attr->mac_id/4));
        tmp_val32 = 1;
        DRV_IOCTL(lchip, 0, cmd, &tmp_val32);
        tmp_val32 = 0;
        DRV_IOCTL(lchip, 0, cmd, &tmp_val32);
    }

    /* #11. reconfig MAC */
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        if (2 != ds_attr->m[i].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed \n");
                return CTC_E_INVALID_PARAM;
            }
            /* 11.1 reconfig mac */
            CTC_ERROR_RETURN(_sys_usw_mac_set_mac_config(lchip, tmp_lport, TRUE));

            /* 11.2 reconfig dlb */
            CTC_ERROR_RETURN(sys_usw_peri_set_dlb_chan_type(lchip, port_attr_tmp->chan_id));
            /* 11.3 reconfig flowctrl */
            CTC_ERROR_RETURN(_sys_usw_mac_flow_ctrl_cfg(lchip, tmp_lport));
            /* 11.4 update port_attr->interface_type */
            port_attr_tmp->interface_type = interface_type;
            /* 11.5 reset port_attr->an_fec to default */
            port_attr_tmp->an_fec = (1 << CTC_PORT_FEC_TYPE_RS)|(1 << CTC_PORT_FEC_TYPE_BASER);
        }
    }

    /* #12. disable enqdrop, only network port use */
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        if (0 == ds_attr->m[i].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
            sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &(ds_attr->shp_profile[i]));
        }
    }

    /* #13. add new-built mac to channel */
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        if (2 != ds_attr->m[i].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed \n");
                return CTC_E_INVALID_PARAM;
            }
            sys_usw_add_port_to_channel(lchip, tmp_lport, port_attr_tmp->chan_id);

            if (1 == ds_attr->m[i].add_drop_flag)
            {
                sys_usw_qos_add_mcast_queue_to_channel(lchip, tmp_lport, port_attr_tmp->chan_id);
            }
        }
    }

    /*  #14. reconfig MDIO scan */
    sys_usw_peri_set_phy_scan_cfg(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_set_dynamic_switch(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    int32 ret = 0;
    uint16 lport = 0;
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 gchip = 0;
    uint8 hss_idx = 0;
    uint8 slice_id = 0;
    uint8 lane_id = 0;
    uint32 i = 0;
    uint8 mode = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr;
    uint16 serdes_id = 0;
    uint16 chip_port = 0;
    uint8 num = 0;

    DATAPATH_INIT_CHECK();

    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (port_attr == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    if (CTC_PORT_IF_NONE == if_mode->interface_type)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid interface type \n");
        return CTC_E_INVALID_PARAM;
    }
    SYS_DATAPATH_GET_MODE_WITH_IFMODE(if_mode->speed, if_mode->interface_type, mode);

    if (port_attr->pcs_mode == mode)
    {
        /*switch mode not change, do not do switch*/
        return CTC_E_NONE;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    sal_memset(&ds_attr, 0, sizeof(sys_datapath_dynamic_switch_attr_t));

    /* collect all serdes/mac info associated with Dynamic switch process */
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    /* get the minimum serdes id when num > 1 */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &num));
    CTC_ERROR_RETURN(sys_usw_datapath_dynamic_switch_get_info(lchip, lport, serdes_id, port_attr->pcs_mode, mode, 1, &ds_attr));

    for (i = 0; i < ds_attr.mac_num; i++)
    {
        if (port_attr->mac_id > ds_attr.m[i].mac_id)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Datapath] Mac %d is not the lowest when dynamic switch \n", port_attr->mac_id);
            return CTC_E_INVALID_PARAM;
        }
    }

    /* parameter check */
    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(ds_attr.s[i].serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(ds_attr.s[i].serdes_id);

        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;
        }

        p_serdes = &p_hss_vec->serdes_info[lane_id];
        if (!p_serdes->is_dyn)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " SerDes %d DO NOT support dynamic switch \n", ds_attr.s[i].serdes_id);
            return CTC_E_NOT_SUPPORT;
        }

        if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
            && ((CTC_CHIP_SERDES_CG_MODE == mode)||(CTC_CHIP_SERDES_XXVG_MODE == mode)||(CTC_CHIP_SERDES_LG_MODE == mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " HSS15G DO NOT support 25G/50G/100G \n");
            return CTC_E_NOT_SUPPORT;
        }
        else if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_28G)
            && ((CTC_CHIP_SERDES_QSGMII_MODE == mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == mode)
            ||(CTC_CHIP_SERDES_USXGMII1_MODE == mode) || (CTC_CHIP_SERDES_USXGMII2_MODE == mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " HSS28G DO NOT support QSGMII/USXGMII \n");
            return CTC_E_NOT_SUPPORT;
        }
    }

    CTC_ERROR_RETURN(_sys_usw_datapath_check_serdes_pll(lchip, mode, CTC_CHIP_SERDES_OCS_MODE_NONE, &ds_attr, -1));

    /* Do switch step#1 */
    CTC_ERROR_GOTO(_sys_usw_datapath_set_pre_dynamic_switch(lchip, &ds_attr), ret, error);

    /* Do switch step#2 */
    /* switch serdes attr one by one */
    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        CTC_ERROR_GOTO(_sys_usw_datapath_set_serdes_none_mode(lchip, ds_attr.s[i].serdes_id), ret, error);
    }
    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        CTC_ERROR_GOTO(sys_usw_datapath_set_serdes_mode(lchip, ds_attr.s[i].serdes_id, ds_attr.s[i].dst_mode, CTC_CHIP_SERDES_OCS_MODE_NONE), ret, error);
    }

    /* Do switch step#3 */
    CTC_ERROR_GOTO(_sys_usw_datapath_set_post_dynamic_switch(lchip, lport, mode, &ds_attr), ret, error);

    return CTC_E_NONE;

error:
    for (i = 0; i < ds_attr.mac_num; i++)
    {
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr.m[i].mac_id);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
        sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &(ds_attr.shp_profile[i]));
    }
    return ret;
}

int32
sys_usw_datapath_set_serdes_dynamic_switch(uint8 lchip, uint8 serdes_id, uint8 mode, uint16 overclocking_speed)
{
    int32 ret = 0;
    uint16 tmp_lport = 0;
    uint32 tmp_gport = 0;
    uint8 gchip = 0;
    uint8 hss_idx = 0;
    uint8 slice_id = 0;
    uint8 lane_id = 0;
    uint32 i = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr;
    uint32 port_step = 0;
    uint32 tmp_val32 = 0;

    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d,  Mode:%d \n", serdes_id, mode);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    slice_id = (serdes_id >= SERDES_NUM_PER_SLICE)?1:0;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];
    if (!p_serdes->is_dyn)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if ((mode == CTC_CHIP_SERDES_QSGMII_MODE) || (mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        if ((lane_id == 2) || (lane_id == 3) || (lane_id == 6) || (lane_id == 7))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"serdes %d don't support mode %d \n", serdes_id, mode);
            return CTC_E_INVALID_PARAM;
        }
    }

    if (overclocking_speed == SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_11G_MODE)
    {
        overclocking_speed = CTC_CHIP_SERDES_OCS_MODE_11_06G;
    }
    else if (overclocking_speed == SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_12DOT5G_MODE)
    {
        overclocking_speed = CTC_CHIP_SERDES_OCS_MODE_12_58G;
    }

    if ((p_hss_vec->hss_type == 1) && (overclocking_speed != CTC_CHIP_SERDES_OCS_MODE_NONE))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if ((p_serdes->mode == mode) && (p_serdes->overclocking_speed == overclocking_speed))
    {
        /*switch mode not change, do not do switch*/
        return CTC_E_NONE;
    }


    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    sal_memset(&ds_attr, 0, sizeof(sys_datapath_dynamic_switch_attr_t));

    if (CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode)
    {
        tmp_lport = SYS_COMMON_USELESS_MAC;
    }
    else
    {
        tmp_lport = p_serdes->lport;
    }
    CTC_ERROR_RETURN(sys_usw_datapath_dynamic_switch_get_info(lchip, tmp_lport, serdes_id, p_serdes->mode, mode, 0, &ds_attr));

    CTC_ERROR_RETURN(_sys_usw_datapath_check_serdes_pll(lchip, mode, overclocking_speed, &ds_attr, serdes_id));

    if (CTC_CHIP_SERDES_100BASEFX_MODE == mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_15G)
        && ((CTC_CHIP_SERDES_CG_MODE == mode)||(CTC_CHIP_SERDES_XXVG_MODE == mode)||(CTC_CHIP_SERDES_LG_MODE == mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    else if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_28G)
        && ((CTC_CHIP_SERDES_QSGMII_MODE == mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == mode)
        ||(CTC_CHIP_SERDES_USXGMII1_MODE == mode) || (CTC_CHIP_SERDES_USXGMII2_MODE == mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

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
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
            /* check CL73 AN enable cfg */
            if ((CTC_CHIP_SERDES_XFI_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XLG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_CG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XXVG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_LG_MODE == port_attr_tmp->pcs_mode))
            {
                CTC_ERROR_RETURN(_sys_usw_mac_set_cl73_auto_neg_en(lchip, tmp_gport, FALSE));
                port_step = SYS_USW_MCU_PORT_DS_BASE_ADDR + tmp_lport * SYS_USW_MCU_PORT_ALLOC_BYTE;
                CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, (port_step+0x8), &tmp_val32));
                tmp_val32 &= ~(1 << 8);
                CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, (port_step+0x8), tmp_val32));
            }
        }
    }

    CTC_ERROR_GOTO(_sys_usw_datapath_set_pre_dynamic_switch(lchip, &ds_attr), ret, error);
    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        CTC_ERROR_GOTO(sys_usw_datapath_set_serdes_mode(lchip, ds_attr.s[i].serdes_id, ds_attr.s[i].dst_mode, overclocking_speed), ret, error);
    }
    if (CTC_CHIP_SERDES_NONE_MODE != mode)
    {
        CTC_ERROR_GOTO(_sys_usw_datapath_set_post_dynamic_switch(lchip, p_serdes->lport, mode, &ds_attr), ret, error);
    }

    return CTC_E_NONE;


error:
    for (i = 0; i < ds_attr.mac_num; i++)
    {
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr.m[i].mac_id);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
        sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &(ds_attr.shp_profile[i]));
    }
    return ret;
}


#define __INIT__

uint8
sys_usw_datapath_get_wlan_en(uint8 lchip)
{
    return p_usw_datapath_master[lchip]->wlan_enable;
}

uint8
sys_usw_datapath_get_dot1ae_en(uint8 lchip)
{
    return p_usw_datapath_master[lchip]->dot1ae_enable;
}

int32
sys_usw_datapath_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_hss_wb_mapping(sys_wb_datapath_hss_attribute_t *p_wb_hss, sys_datapath_hss_attribute_t *p_hss, uint8 sync)
{
    uint16 loop = 0;

    if (sync)
    {
        p_wb_hss->hss_type         = p_hss->hss_type;
        p_wb_hss->hss_id           = p_hss->hss_id;
        p_wb_hss->plla_mode        = p_hss->plla_mode;
        p_wb_hss->pllb_mode        = p_hss->pllb_mode;
        p_wb_hss->intf_div_a       = p_hss->intf_div_a;
        p_wb_hss->intf_div_b       = p_hss->intf_div_b;
        p_wb_hss->plla_ref_cnt     = p_hss->plla_ref_cnt;
        p_wb_hss->pllb_ref_cnt     = p_hss->pllb_ref_cnt;
        p_wb_hss->clktree_bitmap   = p_hss->clktree_bitmap;
        p_wb_hss->pllc_mode        = p_hss->pllc_mode;
        p_wb_hss->pllc_ref_cnt     = p_hss->pllc_ref_cnt;

        for(loop = 0; loop < SYS_DATAPATH_CORE_NUM; loop++)
        {
            p_wb_hss->core_div_a[loop] = p_hss->core_div_a[loop];
            p_wb_hss->core_div_b[loop] = p_hss->core_div_b[loop];
        }

        for(loop = 0; loop < HSS15G_LANE_NUM; loop++)
        {
            p_wb_hss->serdes_info[loop].mode = p_hss->serdes_info[loop].mode;
            p_wb_hss->serdes_info[loop].pll_sel = p_hss->serdes_info[loop].pll_sel;
            p_wb_hss->serdes_info[loop].rate_div = p_hss->serdes_info[loop].rate_div;
            p_wb_hss->serdes_info[loop].bit_width = p_hss->serdes_info[loop].bit_width;
            p_wb_hss->serdes_info[loop].lport = p_hss->serdes_info[loop].lport;
            p_wb_hss->serdes_info[loop].port_num = p_hss->serdes_info[loop].port_num;
            p_wb_hss->serdes_info[loop].rx_polarity = p_hss->serdes_info[loop].rx_polarity;
            p_wb_hss->serdes_info[loop].tx_polarity = p_hss->serdes_info[loop].tx_polarity;
            p_wb_hss->serdes_info[loop].lane_id = p_hss->serdes_info[loop].lane_id;
            p_wb_hss->serdes_info[loop].is_dyn = p_hss->serdes_info[loop].is_dyn;
            p_wb_hss->serdes_info[loop].group = p_hss->serdes_info[loop].group;
            p_wb_hss->serdes_info[loop].overclocking_speed = p_hss->serdes_info[loop].overclocking_speed;
        }
    }
    else
    {
        p_hss->hss_type         = p_wb_hss->hss_type;
        p_hss->hss_id           = p_wb_hss->hss_id;
        p_hss->plla_mode 	    = p_wb_hss->plla_mode;
        p_hss->pllb_mode 	    = p_wb_hss->pllb_mode;
        p_hss->intf_div_a       = p_wb_hss->intf_div_a;
        p_hss->intf_div_b       = p_wb_hss->intf_div_b;
        p_hss->plla_ref_cnt     = p_wb_hss->plla_ref_cnt;
        p_hss->pllb_ref_cnt     = p_wb_hss->pllb_ref_cnt;
        p_hss->clktree_bitmap   = p_wb_hss->clktree_bitmap;
        p_hss->pllc_mode        = p_wb_hss->pllc_mode;
        p_hss->pllc_ref_cnt     = p_wb_hss->pllc_ref_cnt;

        for(loop = 0; loop < SYS_DATAPATH_CORE_NUM; loop++)
        {
             p_hss->core_div_a[loop] = p_wb_hss->core_div_a[loop];
             p_hss->core_div_b[loop] = p_wb_hss->core_div_b[loop];
        }

        for(loop = 0; loop < HSS15G_LANE_NUM; loop++)
        {
            p_hss->serdes_info[loop].mode = p_wb_hss->serdes_info[loop].mode;
            p_hss->serdes_info[loop].pll_sel = p_wb_hss->serdes_info[loop].pll_sel;
            p_hss->serdes_info[loop].rate_div = p_wb_hss->serdes_info[loop].rate_div;
            p_hss->serdes_info[loop].bit_width = p_wb_hss->serdes_info[loop].bit_width;
            p_hss->serdes_info[loop].lport = p_wb_hss->serdes_info[loop].lport;
            p_hss->serdes_info[loop].port_num = p_wb_hss->serdes_info[loop].port_num;
            p_hss->serdes_info[loop].rx_polarity = p_wb_hss->serdes_info[loop].rx_polarity;
            p_hss->serdes_info[loop].tx_polarity = p_wb_hss->serdes_info[loop].tx_polarity;
            p_hss->serdes_info[loop].lane_id = p_wb_hss->serdes_info[loop].lane_id;
            p_hss->serdes_info[loop].is_dyn = p_wb_hss->serdes_info[loop].is_dyn;
            p_hss->serdes_info[loop].group = p_wb_hss->serdes_info[loop].group;
            p_hss->serdes_info[loop].overclocking_speed = p_wb_hss->serdes_info[loop].overclocking_speed;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_datapath_hss_wb_sync_func(void* array_data, uint32 index, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_datapath_hss_attribute_t *p_hss = (sys_datapath_hss_attribute_t *)array_data;
    sys_wb_datapath_hss_attribute_t  *p_wb_hss;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)user_data;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_hss = (sys_wb_datapath_hss_attribute_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_hss, 0, sizeof(sys_wb_datapath_hss_attribute_t));

    p_wb_hss->hss_idx = index;

    CTC_ERROR_RETURN(_sys_usw_datapath_hss_wb_mapping(p_wb_hss, p_hss, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_datapath_wb_sync(uint8 lchip)
{
    uint8 slice_id = 0;
    uint16 lport = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_datapath_master_t * p_wb_datapath_master = NULL;

    /*syncup datapath matser*/
    wb_data.buffer = mem_malloc(MEM_SYSTEM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_datapath_master_t, CTC_FEATURE_DATAPATH, SYS_WB_APPID_DATAPATH_SUBID_MASTER);

    p_wb_datapath_master = (sys_wb_datapath_master_t *)wb_data.buffer;

    p_wb_datapath_master->lchip = lchip;
    p_wb_datapath_master->version = SYS_WB_VERSION_DATAPATH;

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            p_wb_datapath_master->port_attr[slice_id][lport].port_type = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].port_type;
            p_wb_datapath_master->port_attr[slice_id][lport].mac_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].mac_id;
            p_wb_datapath_master->port_attr[slice_id][lport].chan_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].chan_id;
            p_wb_datapath_master->port_attr[slice_id][lport].speed_mode = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].speed_mode;
            p_wb_datapath_master->port_attr[slice_id][lport].slice_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].slice_id;
            p_wb_datapath_master->port_attr[slice_id][lport].serdes_num = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].serdes_num;
            p_wb_datapath_master->port_attr[slice_id][lport].pcs_mode = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pcs_mode;
            p_wb_datapath_master->port_attr[slice_id][lport].serdes_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].serdes_id;
            p_wb_datapath_master->port_attr[slice_id][lport].is_first = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].is_first;
            p_wb_datapath_master->port_attr[slice_id][lport].flag = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].flag;
            p_wb_datapath_master->port_attr[slice_id][lport].interface_type = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].interface_type;
            p_wb_datapath_master->port_attr[slice_id][lport].an_fec = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].an_fec;
            p_wb_datapath_master->port_attr[slice_id][lport].is_serdes_dyn = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].is_serdes_dyn;
            p_wb_datapath_master->port_attr[slice_id][lport].code_err_count = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].code_err_count;
            p_wb_datapath_master->port_attr[slice_id][lport].tbl_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].tbl_id;
            p_wb_datapath_master->port_attr[slice_id][lport].sgmac_idx = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].sgmac_idx;
            p_wb_datapath_master->port_attr[slice_id][lport].mii_idx = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].mii_idx;
            p_wb_datapath_master->port_attr[slice_id][lport].pcs_idx = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pcs_idx;
            p_wb_datapath_master->port_attr[slice_id][lport].internal_mac_idx = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].internal_mac_idx;
            p_wb_datapath_master->port_attr[slice_id][lport].first_led_mode = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].first_led_mode;
            p_wb_datapath_master->port_attr[slice_id][lport].sec_led_mode = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].sec_led_mode;
            p_wb_datapath_master->port_attr[slice_id][lport].xpipe_en = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].xpipe_en;
            p_wb_datapath_master->port_attr[slice_id][lport].pmac_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pmac_id;
            p_wb_datapath_master->port_attr[slice_id][lport].emac_id = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].emac_id;
            p_wb_datapath_master->port_attr[slice_id][lport].pmac_chanid = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pmac_chanid;
            p_wb_datapath_master->port_attr[slice_id][lport].emac_chanid = p_usw_datapath_master[lchip]->port_attr[slice_id][lport].emac_chanid;
        }
    }

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup datapath hss*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_datapath_hss_attribute_t, CTC_FEATURE_DATAPATH, SYS_WB_APPID_DATAPATH_SUBID_HSS_ATTRIBUTE);

    CTC_ERROR_GOTO(ctc_vector_traverse2(p_usw_datapath_master[lchip]->p_hss_vector, 0, _sys_usw_datapath_hss_wb_sync_func, (void *)&wb_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    sal_memcpy(&(p_wb_datapath_master->qm_choice), &(p_usw_datapath_master[lchip]->qm_choice), sizeof(sys_datapath_qm_choice_t));
    sal_memcpy(p_wb_datapath_master->serdes_mac_map, p_usw_datapath_master[lchip]->serdes_mac_map, 32*4*sizeof(uint8));
    sal_memcpy(p_wb_datapath_master->mii_lport_map, p_usw_datapath_master[lchip]->mii_lport_map, 32*4*sizeof(uint16));
    p_wb_datapath_master->glb_xpipe_en = p_usw_datapath_master[lchip]->glb_xpipe_en;

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_datapath_wb_restore(uint8 lchip)
{
    uint8 slice_id = 0;
    uint16 lport = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_datapath_master_t*  p_wb_datapath_master = NULL;
    sys_wb_datapath_hss_attribute_t wb_hss = {0};
    sys_datapath_hss_attribute_t *p_hss;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    p_wb_datapath_master = mem_malloc(MEM_SYSTEM_MODULE,  sizeof(sys_wb_datapath_master_t));
    if (NULL == p_wb_datapath_master)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(p_wb_datapath_master, 0, sizeof(sys_wb_datapath_master_t));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_datapath_master_t, CTC_FEATURE_DATAPATH, SYS_WB_APPID_DATAPATH_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  datapath_master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query datapath master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8*)p_wb_datapath_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_DATAPATH, p_wb_datapath_master->version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].port_type = p_wb_datapath_master->port_attr[slice_id][lport].port_type;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].mac_id = p_wb_datapath_master->port_attr[slice_id][lport].mac_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].chan_id = p_wb_datapath_master->port_attr[slice_id][lport].chan_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].speed_mode = p_wb_datapath_master->port_attr[slice_id][lport].speed_mode;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].slice_id = p_wb_datapath_master->port_attr[slice_id][lport].slice_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].serdes_num = p_wb_datapath_master->port_attr[slice_id][lport].serdes_num;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pcs_mode = p_wb_datapath_master->port_attr[slice_id][lport].pcs_mode;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].serdes_id = p_wb_datapath_master->port_attr[slice_id][lport].serdes_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].is_first = p_wb_datapath_master->port_attr[slice_id][lport].is_first;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].flag = p_wb_datapath_master->port_attr[slice_id][lport].flag;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].interface_type = p_wb_datapath_master->port_attr[slice_id][lport].interface_type;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].an_fec = p_wb_datapath_master->port_attr[slice_id][lport].an_fec;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].is_serdes_dyn = p_wb_datapath_master->port_attr[slice_id][lport].is_serdes_dyn;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].code_err_count = p_wb_datapath_master->port_attr[slice_id][lport].code_err_count;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].tbl_id = p_wb_datapath_master->port_attr[slice_id][lport].tbl_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].sgmac_idx = p_wb_datapath_master->port_attr[slice_id][lport].sgmac_idx;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].mii_idx = p_wb_datapath_master->port_attr[slice_id][lport].mii_idx;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pcs_idx = p_wb_datapath_master->port_attr[slice_id][lport].pcs_idx;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].internal_mac_idx = p_wb_datapath_master->port_attr[slice_id][lport].internal_mac_idx;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].first_led_mode = p_wb_datapath_master->port_attr[slice_id][lport].sec_led_mode;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].first_led_mode = p_wb_datapath_master->port_attr[slice_id][lport].sec_led_mode;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].xpipe_en = p_wb_datapath_master->port_attr[slice_id][lport].xpipe_en;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].emac_id = p_wb_datapath_master->port_attr[slice_id][lport].emac_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pmac_id = p_wb_datapath_master->port_attr[slice_id][lport].pmac_id;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].emac_chanid = p_wb_datapath_master->port_attr[slice_id][lport].emac_chanid;
            p_usw_datapath_master[lchip]->port_attr[slice_id][lport].pmac_chanid = p_wb_datapath_master->port_attr[slice_id][lport].pmac_chanid;
        }
    }

    sal_memcpy(&(p_usw_datapath_master[lchip]->qm_choice),   &(p_wb_datapath_master->qm_choice),   sizeof(sys_datapath_qm_choice_t));
    sal_memcpy(p_usw_datapath_master[lchip]->serdes_mac_map, p_wb_datapath_master->serdes_mac_map, 32*4*sizeof(uint8));
    sal_memcpy(p_usw_datapath_master[lchip]->mii_lport_map,  p_wb_datapath_master->mii_lport_map,  32*4*sizeof(uint16));
    p_usw_datapath_master[lchip]->glb_xpipe_en = p_wb_datapath_master->glb_xpipe_en;

    ctc_vector_traverse(p_usw_datapath_master[lchip]->p_hss_vector, (vector_traversal_fn)sys_usw_datapath_free_node_data, NULL);

    /*restore datapath hss*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_datapath_hss_attribute_t, CTC_FEATURE_DATAPATH, SYS_WB_APPID_DATAPATH_SUBID_HSS_ATTRIBUTE);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_hss, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_hss = mem_malloc(MEM_SYSTEM_MODULE,  sizeof(sys_datapath_hss_attribute_t));
        if (NULL == p_hss)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_hss, 0, sizeof(sys_datapath_hss_attribute_t));

        ret = _sys_usw_datapath_hss_wb_mapping(&wb_hss, p_hss, 0);
        if (ret)
        {
            continue;
        }

        /*add to soft table*/
        ctc_vector_add(p_usw_datapath_master[lchip]->p_hss_vector, wb_hss.hss_idx, p_hss);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (p_wb_datapath_master)
    {
        mem_free(p_wb_datapath_master);
    }
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

int32
sys_usw_datapath_init(uint8 lchip, void* p_global_cfg)
{
    ctc_datapath_global_cfg_t* p_datapath_cfg = NULL;

    CTC_PTR_VALID_CHECK(p_global_cfg);
    p_datapath_cfg = (ctc_datapath_global_cfg_t*)p_global_cfg;

    if (p_usw_datapath_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*create datapath master*/
    p_usw_datapath_master[lchip] = (sys_datapath_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_datapath_master_t));
    if (NULL == p_usw_datapath_master[lchip])
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_datapath_master[lchip], 0, sizeof(sys_datapath_master_t));

    p_usw_datapath_master[lchip]->p_hss_vector = ctc_vector_init(2, SYS_DATAPATH_HSS_NUM /2 + 1);
    if (NULL == p_usw_datapath_master[lchip]->p_hss_vector)
    {
        mem_free(p_usw_datapath_master[lchip]);
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    if(MCHIP_API(lchip)->datapath_init)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->datapath_init(lchip, p_datapath_cfg));
    }
    else return CTC_E_INVALID_PTR;

    return CTC_E_NONE;
}

int32
sys_usw_datapath_deinit(uint8 lchip)
{
    uint8 status = 0;
    if (!p_usw_datapath_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_get_working_status(lchip, &status);
    if(CTC_FTM_MEM_CHANGE_REBOOT == status || CTC_FTM_MEM_CHANGE_RECOVER == status)
    {
        return CTC_E_NONE;
    }
    ctc_vector_traverse(p_usw_datapath_master[lchip]->p_hss_vector, (vector_traversal_fn)sys_usw_datapath_free_node_data, NULL);
    ctc_vector_release(p_usw_datapath_master[lchip]->p_hss_vector);

    mem_free(p_usw_datapath_master[lchip]);

    return CTC_E_NONE;
}

int32
sys_usw_datapath_chip_reset_recover_proc(uint8 lchip, void* p_data)
{
    uint8  hss_idx   = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint32 value     = 0;
    ctc_datapath_global_cfg_t     global_cfg;
    sys_datapath_hss_attribute_t* p_hss_vec  = NULL;

    /*Get global config*/
    sal_memset(&global_cfg, 0 ,sizeof(ctc_datapath_global_cfg_t));
    global_cfg.core_frequency_a = p_usw_datapath_master[lchip]->core_plla;
    global_cfg.core_frequency_b = p_usw_datapath_master[lchip]->core_pllb;
    global_cfg.wlan_enable      = p_usw_datapath_master[lchip]->wlan_enable;
    global_cfg.dot1ae_enable    = p_usw_datapath_master[lchip]->dot1ae_enable;

    for(serdes_id = 0; serdes_id < SERDES_NUM_PER_SLICE; serdes_id++)
    {
        hss_idx   = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id   = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if(p_hss_vec == NULL)
        {
            continue;
        }
        global_cfg.serdes[serdes_id].mode        = p_hss_vec->serdes_info[lane_id].mode;
        global_cfg.serdes[serdes_id].group       = p_hss_vec->serdes_info[lane_id].group;
        global_cfg.serdes[serdes_id].is_dynamic  = p_hss_vec->serdes_info[lane_id].is_dyn;
        global_cfg.serdes[serdes_id].rx_polarity = p_hss_vec->serdes_info[lane_id].rx_polarity;
        global_cfg.serdes[serdes_id].tx_polarity = p_hss_vec->serdes_info[lane_id].tx_polarity;
    }

    sys_usw_mac_get_monitor_link_enable(lchip, &value);
    if(0 != value)
    {
        sys_usw_mac_set_monitor_link_enable(lchip, 0);
    }

    /*datapath reconfig*/
    sys_usw_datapath_deinit(lchip);
    sys_usw_datapath_init(lchip, &global_cfg);

    if(0 != value)
    {
        sys_usw_mac_set_monitor_link_enable(lchip, value);
    }

    if(MCHIP_API(lchip)->mac_init)
    {
        CTC_ERROR_RETURN(MCHIP_API(lchip)->mac_init(lchip));
    }
    else return CTC_E_INVALID_PTR;

    return CTC_E_NONE;
}

