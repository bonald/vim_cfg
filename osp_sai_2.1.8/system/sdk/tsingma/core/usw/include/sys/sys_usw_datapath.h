/**
 @file sys_usw_datapath.h

 @date 2013-05-28

 @version v2.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_USW_DATAPATH_H
#define _SYS_USW_DATAPATH_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_chip.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "sys_usw_dmps.h"
#include "sys_usw_common.h"
#include "sys_usw_qos_api.h"

/***************************************************************
 *
 * Macro definition
 *
 ***************************************************************/
#define SYS_DATAPATH_SERDES_28G_START_ID 24
#define SYS_DATAPATH_MAX_LOCAL_SLICE_NUM 1

#define SERDES_NUM_PER_SLICE MCHIP_CAP(SYS_CAP_DMPS_SERDES_NUM_PER_SLICE)
#define HSS28G_NUM_PER_SLICE MCHIP_CAP(SYS_CAP_DMPS_HSS28G_NUM_PER_SLICE)
#define HSS15G_NUM_PER_SLICE MCHIP_CAP(SYS_CAP_DMPS_HSS15G_NUM_PER_SLICE)
#define SYS_DATAPATH_HSS_NUM MCHIP_CAP(SYS_CAP_DMPS_HSS_NUM)

#define SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) (((serdes_id) >= SYS_DATAPATH_SERDES_28G_START_ID) && ((serdes_id) < SERDES_NUM_PER_SLICE))
#define SYS_DATAPATH_MAC_IN_HSS28G(lchip, id) \
    (DRV_IS_TSINGMA(lchip) ? \
    (((43 < id) && (48 > id)) || ((59 < id) && (64 > id))) : \
    (((id >= 12) && (id <= 15)) || ((id >= 28) && (id <= 31)) || ((id >= 44) && (id <= 47)) || ((id >=60) && (id <= 63))))

#define SYS_DATAPATH_GET_XPIPE_MAC(mac, emac, pmac)    \
{                                                      \
    uint8 i = 0;                                       \
    emac = 0xff;                                       \
    pmac = 0xff;                                       \
    for (i = 0; i < SYS_USW_XPIPE_PORT_NUM; i++) \
    {                                                           \
        if (mac == g_tsingma_xpipe_mac_mapping[i].mac_id)       \
        {                                                       \
            emac = g_tsingma_xpipe_mac_mapping[i].emac_id;      \
            pmac = g_tsingma_xpipe_mac_mapping[i].pmac_id;      \
        }                                                       \
    }                                                           \
}

#define HSS15G_LANE_NUM                  8
#define HSS28G_LANE_NUM                  4
#define HSS12G_LANE_NUM                  HSS15G_LANE_NUM
#define HSS12G_NUM_PER_SLICE             3

#define MAX_SERDES_NUM_PER_SLICE         40

#define SYS_DATAPATH_REFCLOCK            156
#define SYS_DATAPATH_CORE_NUM            2
#define SYS_DATAPATH_DEBUG_SHOW_ALL      0x01
#define SYS_DATAPATH_USELESS_MAC         0xffff
#define TM_MAX_CHAN_ID_NUM               84
#define TM_SHARED_MII_NUM                17
#define TM_SGMAC_NUM                     18
#define TM_PCS_TOTAL_NUM                 12
#define TM_MAC_TBL_NUM_PER_SLICE         72

#define SYS_CALENDAR_CYCLE               MCHIP_CAP(SYS_CAP_DMPS_CALENDAR_CYCLE)   /*for D2 64*4=256D, for TM 128*4=512D*/
#define SYS_CALENDAR_TXQM_NUM            4
#define SYS_CALENDAR_PORT_NUM_PER_TXQM   16
#define SYS_CALENDAR_TOTAL_DEEP          (SYS_CALENDAR_CYCLE*SYS_CALENDAR_TXQM_NUM)

#define SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(serdes_id) (11 >= (serdes_id))
#define SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(mode)      ((CTC_CHIP_SERDES_USXGMII0_MODE == mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == mode) || \
                                                          (CTC_CHIP_SERDES_USXGMII2_MODE == mode) || (CTC_CHIP_SERDES_QSGMII_MODE == mode))
#define SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(mode)      ((CTC_CHIP_SERDES_USXGMII1_MODE == mode) || \
                                                                (CTC_CHIP_SERDES_USXGMII2_MODE == mode) || (CTC_CHIP_SERDES_QSGMII_MODE == mode))
#define SYS_DATAPATH_IS_SERDES_25G_PER_LANE(mode)        ((CTC_CHIP_SERDES_XXVG_MODE == mode) || (CTC_CHIP_SERDES_LG_MODE == mode) || \
                                                          (CTC_CHIP_SERDES_CG_MODE == mode))
#define SYS_DATAPATH_IS_SERDES_10G_PER_LANE(mode)        ((CTC_CHIP_SERDES_XFI_MODE == mode) || (CTC_CHIP_SERDES_XLG_MODE == mode))

#define SYS_DATAPATH_HSS_IS_HSS28G(idx)                  (((idx) >= 3) && ((idx) <= 6))
#define SYS_DATAPATH_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(datapath, datapath, DATAPATH_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)
#define SYS_DATAPATH_DS_MAX_PORT_NUM     16
#define SYS_DATAPATH_DS_MAX_SERDES_NUM   8

#define SYS_DATAPATH_HSS12G_ACC_ID_MAX  10
#define SYS_DATAPATH_HSS28G_ACC_ID_MAX  1

#define SYS_DATAPATH_DYNAMIC_SWITCH_BASE_MAC(mac_id, base_mac_id) \
{                                                                 \
    if (mac_id <= 7)                                              \
    {                                                             \
        base_mac_id = 0;                                          \
    }                                                             \
    else if ((mac_id >= 8) && (mac_id <= 11))                     \
    {                                                             \
        base_mac_id = 8;                                          \
    }                                                             \
    else if ((mac_id >= 12) && (mac_id <= 19))                    \
    {                                                             \
        base_mac_id = 12;                                         \
    }                                                             \
    else if ((mac_id >= 20) && (mac_id <= 27))                    \
    {                                                             \
        base_mac_id = 20;                                         \
    }                                                             \
    else if ((mac_id >= 28) && (mac_id <= 31))                    \
    {                                                             \
        base_mac_id = 28;                                         \
    }                                                             \
    else if ((mac_id >= 32) && (mac_id <= 39))                    \
    {                                                             \
        base_mac_id = 32;                                         \
    }                                                             \
    else if ((mac_id >= 40) && (mac_id <= 43))                    \
    {                                                             \
        base_mac_id = 40;                                         \
    }                                                             \
    else if ((mac_id >= 44) && (mac_id <= 51))                    \
    {                                                             \
        base_mac_id = 44;                                         \
    }                                                             \
    else if ((mac_id >= 52) && (mac_id <= 59))                    \
    {                                                             \
        base_mac_id = 52;                                         \
    }                                                             \
    else if ((mac_id >= 60) && (mac_id <= 63))                    \
    {                                                             \
        base_mac_id = 60;                                         \
    }                                                             \
}

#define GET_SERDES_LANE(mac_id, lane, idx, lg_flag, mode, real_lane)    \
    if ((1 == lg_flag) && (CTC_CHIP_SERDES_LG_MODE == mode))           \
    {                                                                  \
        if (0 == mac_id % 4)                                          \
        {                                                             \
            real_lane = lane + idx;                                   \
        }                                                             \
        else                                                          \
        {                                                             \
            if (0 == idx)                                             \
            {                                                         \
                real_lane = 0;                                        \
            }                                                         \
            else                                                      \
            {                                                         \
                real_lane = 3;                                        \
            }                                                         \
        }                                                             \
    }                                                                 \
    else                                                              \
    {                                                                 \
        real_lane = lane + idx;                                       \
    }

#define SYS_DATAPATH_GET_MODE_WITH_IFMODE(speed_mode, iftype, mode)  \
    switch (iftype)                                      \
    {                                                    \
    case CTC_PORT_IF_SGMII:                              \
        mode = CTC_CHIP_SERDES_SGMII_MODE;               \
        break;                                           \
    case CTC_PORT_IF_2500X:                              \
        mode = CTC_CHIP_SERDES_2DOT5G_MODE;              \
        break;                                           \
    case CTC_PORT_IF_QSGMII:                             \
        mode = CTC_CHIP_SERDES_QSGMII_MODE;              \
        break;                                           \
    case CTC_PORT_IF_USXGMII_S:                          \
        mode = CTC_CHIP_SERDES_USXGMII0_MODE;            \
        break;                                           \
    case CTC_PORT_IF_USXGMII_M2G5:                       \
        mode = CTC_CHIP_SERDES_USXGMII1_MODE;            \
        break;                                           \
    case CTC_PORT_IF_USXGMII_M5G:                        \
        mode = CTC_CHIP_SERDES_USXGMII2_MODE;            \
        break;                                           \
    case CTC_PORT_IF_XAUI:                               \
        mode = CTC_CHIP_SERDES_XAUI_MODE;                \
        break;                                           \
    case CTC_PORT_IF_DXAUI:                              \
        mode = CTC_CHIP_SERDES_DXAUI_MODE;               \
        break;                                           \
    case CTC_PORT_IF_XFI:                                \
        mode = CTC_CHIP_SERDES_XFI_MODE;                 \
        break;                                           \
    case CTC_PORT_IF_KR:                                 \
    case CTC_PORT_IF_CR:                                 \
        if (CTC_PORT_SPEED_25G == speed_mode)            \
        {                                                \
            mode = CTC_CHIP_SERDES_XXVG_MODE;            \
        }                                                \
        else if (CTC_PORT_SPEED_10G == speed_mode)       \
        {                                                \
            mode = CTC_CHIP_SERDES_XFI_MODE;             \
        }                                                \
        break;                                           \
    case CTC_PORT_IF_KR2:                                \
    case CTC_PORT_IF_CR2:                                \
        if (CTC_PORT_SPEED_50G == speed_mode)            \
        {                                                \
            mode = CTC_CHIP_SERDES_LG_MODE;              \
        }                                                \
        break;                                           \
    case CTC_PORT_IF_KR4:                                \
    case CTC_PORT_IF_CR4:                                \
        if (CTC_PORT_SPEED_40G == speed_mode)            \
        {                                                \
            mode = CTC_CHIP_SERDES_XLG_MODE;             \
        }                                                \
        else if (CTC_PORT_SPEED_100G == speed_mode)      \
        {                                                \
            mode = CTC_CHIP_SERDES_CG_MODE;              \
        }                                                \
        break;                                           \
    case CTC_PORT_IF_FX:                                 \
        if (CTC_PORT_SPEED_100M == speed_mode)           \
        {                                                \
            mode = CTC_CHIP_SERDES_100BASEFX_MODE;       \
        }                                                \
        break;                                           \
    default:                                             \
        break;                                           \
    }

#define SYS_DATAPATH_GET_TXQM_WITH_MAC(mac_id, txqm_id)     \
    if (mac_id <= 15)                                       \
    {                                                       \
        txqm_id = 0;                                        \
    }                                                       \
    else if ((mac_id >= 16) && (mac_id <= 31))              \
    {                                                       \
        txqm_id = 1;                                        \
    }                                                       \
    else if ((mac_id >= 32) && (mac_id <= 47))              \
    {                                                       \
        txqm_id = 2;                                        \
    }                                                       \
    else if ((mac_id >= 48) && (mac_id <= 63))              \
    {                                                       \
        txqm_id = 3;                                        \
    }

#define SYS_DATAPATH_GET_IFTYPE_WITH_MODE(mode, iftype)  \
    if (CTC_CHIP_SERDES_SGMII_MODE == mode)              \
        iftype = CTC_PORT_IF_SGMII;                      \
    else if (CTC_CHIP_SERDES_XFI_MODE == mode)           \
        iftype = CTC_PORT_IF_XFI;                        \
    else if (CTC_CHIP_SERDES_QSGMII_MODE == mode)        \
        iftype = CTC_PORT_IF_QSGMII;                     \
    else if (CTC_CHIP_SERDES_USXGMII0_MODE == mode)      \
        iftype = CTC_PORT_IF_USXGMII_S;                  \
    else if (CTC_CHIP_SERDES_USXGMII1_MODE == mode)      \
        iftype = CTC_PORT_IF_USXGMII_M2G5;               \
    else if (CTC_CHIP_SERDES_USXGMII2_MODE == mode)      \
        iftype = CTC_PORT_IF_USXGMII_M5G;                \
    else if (CTC_CHIP_SERDES_2DOT5G_MODE == mode)        \
        iftype = CTC_PORT_IF_2500X;                      \
    else if (CTC_CHIP_SERDES_XAUI_MODE == mode)          \
        iftype = CTC_PORT_IF_XAUI;                       \
    else if (CTC_CHIP_SERDES_DXAUI_MODE == mode)         \
        iftype = CTC_PORT_IF_DXAUI;                      \
    else if (CTC_CHIP_SERDES_XLG_MODE == mode)           \
        iftype = CTC_PORT_IF_CR4;                        \
    else if (CTC_CHIP_SERDES_CG_MODE == mode)            \
        iftype = CTC_PORT_IF_CR4;                        \
    else if (CTC_CHIP_SERDES_XXVG_MODE == mode)          \
        iftype = CTC_PORT_IF_CR;                         \
    else if (CTC_CHIP_SERDES_LG_MODE == mode)            \
        iftype = CTC_PORT_IF_CR2;                        \
    else if (CTC_CHIP_SERDES_100BASEFX_MODE == mode)     \
        iftype = CTC_PORT_IF_FX;

#define SYS_DATAPATH_MODE_TO_SPEED(mode, speed) \
{   \
    if ((mode == CTC_PORT_SPEED_1G) || (mode == CTC_PORT_SPEED_100M) || \
        (mode == CTC_PORT_SPEED_10M))    \
    {   \
        speed = 1; \
    }   \
    else if (mode == CTC_PORT_SPEED_2G5) \
    {   \
        speed = 2; \
    }   \
    else if (mode == CTC_PORT_SPEED_5G)  \
    {   \
        speed = 5; \
    }   \
    else if (mode == CTC_PORT_SPEED_10G)  \
    {   \
        speed = 10; \
    }   \
    else if (mode == CTC_PORT_SPEED_20G)  \
    {   \
        speed = 20; \
    }   \
    else if (mode == CTC_PORT_SPEED_25G) \
    {   \
        speed = 25; \
    }   \
    else if (mode == CTC_PORT_SPEED_40G) \
    {   \
        speed = 40; \
    }   \
    else if (mode == CTC_PORT_SPEED_50G) \
    {   \
        speed = 50; \
    }   \
    else if (mode == CTC_PORT_SPEED_100G) \
    {   \
        speed = 100; \
    }   \
}

/* wrrbase = speed * 10 */
#define SYS_DATAPATH_SPEED_TO_WRRCFG(speed, wrrbase) \
{   \
    if ((speed == CTC_PORT_SPEED_1G) || (speed == CTC_PORT_SPEED_100M) || \
        (speed == CTC_PORT_SPEED_10M))    \
    {   \
        wrrbase = 10; \
    }   \
    else if (speed == CTC_PORT_SPEED_2G5) \
    {   \
        wrrbase = 25; \
    }   \
    else if (speed == CTC_PORT_SPEED_5G)  \
    {   \
        wrrbase = 50; \
    }   \
    else if (speed == CTC_PORT_SPEED_10G)  \
    {   \
        wrrbase = 100; \
    }   \
    else if (speed == CTC_PORT_SPEED_20G)  \
    {   \
        wrrbase = 200; \
    }   \
    else if (speed == CTC_PORT_SPEED_25G) \
    {   \
        wrrbase = 250; \
    }   \
    else if (speed == CTC_PORT_SPEED_40G) \
    {   \
        wrrbase = 400; \
    }   \
    else if (speed == CTC_PORT_SPEED_50G) \
    {   \
        wrrbase = 500; \
    }   \
    else if (speed == CTC_PORT_SPEED_100G) \
    {   \
        wrrbase = 1000; \
    }   \
}

#define SYS_DATAPATH_GET_HSS15G_MULT(clk, mult) \
{   \
    if ((clk) == SYS_DATAPATH_HSSCLK_515DOT625)  \
    {  \
        (mult) = 0x58;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_500)  \
    {  \
        (mult) = 0x2c;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_625)  \
    {  \
        (mult) = 0x6f;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_567DOT1875)  \
    {  \
        (mult) = 0x10D;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_644DOT53125)  \
    {  \
        (mult) = 0x13D;  \
    }  \
}

#define SYS_DATAPATH_GET_HSS28G_MULT(clk, mult) \
{   \
    if ((clk) == SYS_DATAPATH_HSSCLK_515DOT625)  \
    {  \
        (mult) = 0x38;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_500)  \
    {  \
        (mult) = 0x0e;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_625)  \
    {  \
        (mult) = 0x4a;  \
    }  \
    else if ((clk) == SYS_DATAPATH_HSSCLK_644DOT53125)  \
    {  \
        (mult) = 0x157;  \
    }  \
}

#define DATAPATH_INIT_CHECK()                 \
{    LCHIP_CHECK(lchip);                     \
    if (NULL == p_usw_datapath_master[lchip])   \
    {                                       \
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
            return CTC_E_NOT_INIT;\
              \
    }                                       \
}

#define SYS_DATAPATH_GET_CLKTREE_DIV(div)  \
(((div) == SYS_DATAPATH_CLKTREE_DIV_FULL)?0:(((div) == SYS_DATAPATH_CLKTREE_DIV_HALF) \
    ?1:(((div) == SYS_DATAPATH_CLKTREE_DIV_QUAD)?3:7)))

#define SYS_DATAPATH_HSS_REG_MASK ((1<<SYS_DATAPATH_HSS_LINK_EN_BIT) \
    |(0x1<<SYS_DATAPATH_HSS_WR_MODE_BIT)|(0x3<<SYS_DATAPATH_HSS_PLL_SEL_BIT) \
    |(0x3<<SYS_DATAPATH_HSS_BW_SEL_BIT) \
    |(0x3<<SYS_DATAPATH_HSS_RATE_SEL_BIT))

#define SYS_DATAPATH_NEED_EXTERN(mode) \
    (((mode==CTC_CHIP_SERDES_XAUI_MODE) || (mode==CTC_CHIP_SERDES_DXAUI_MODE) || \
    (mode==CTC_CHIP_SERDES_XLG_MODE) || (mode==CTC_CHIP_SERDES_CG_MODE) || \
    (mode==CTC_CHIP_SERDES_LG_MODE))?1:0)

#define DRV_CHIP_READ_HSS(hss_type, lchip, hss_id, address, p_value) \
{    if(hss_type) \
    {   \
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, hss_id, address, p_value)); \
    }   \
    else \
    {   \
        CTC_ERROR_RETURN(drv_chip_read_hss15g(lchip, hss_id, address, p_value)); \
    }\
}

#define DRV_CHIP_WRITE_HSS(hss_type, lchip, hss_id, address, p_value) \
{    if(hss_type) \
    {   \
        drv_chip_write_hss28g(lchip, hss_id, address, p_value); \
    }   \
    else \
    {   \
        drv_chip_write_hss15g(lchip, hss_id, address, p_value); \
    }\
}

#define LANE_ID_MAP_INTERAL_LANE_ID(type, lane_id) ((DRV_IS_DUET2(lchip)) ? \
    ((type == SYS_DATAPATH_HSS_TYPE_28G)?(lane_id):(g_usw_hss15g_lane_map[lane_id])) : \
     (lane_id))
#define SYS_CONDITION_RETURN(condition, ret_value) do {if(condition) {return ret_value;}} while(0)
#define SYS_USW_VALID_PTR_WRITE(ptr, val)             do { if(ptr) {*(ptr) = (val);}} while(0)
#define SYS_DATAPATH_FILL_OVCLK_FLAG(overclocking_speed, ovclk_flag) \
    switch(overclocking_speed)\
    {\
        case CTC_CHIP_SERDES_OCS_MODE_NONE:\
            ovclk_flag = 2;\
            break;\
        case CTC_CHIP_SERDES_OCS_MODE_11_06G:\
            ovclk_flag = 3;\
            break;\
        case CTC_CHIP_SERDES_OCS_MODE_12_12G:\
            ovclk_flag = 1;\
            break;\
        case CTC_CHIP_SERDES_OCS_MODE_27_27G:\
            ovclk_flag = 4;\
            break;\
        case CTC_CHIP_SERDES_OCS_MODE_12_58G:\
            ovclk_flag = 5;\
            break;\
        default:\
            ovclk_flag = 0;\
            break;\
    }

/***************************************************************
 *
 * Structure definition
 *
 ***************************************************************/
/*0: 24x12.5G+8x28G 24x10G+2x100G 24x10G+8x25G 32x10G 8x40G
  1: 12xQSGMII+8x10G+8x25G 12xUSXGMII+4x25G+2x50G 12xQSGMII+4x25G+2x50G
  2: 9xQSGMII+12x10G+4x25G+2x50G
  3: 12xQSGMII+4x10G+2x40G 12xUSXGMII+4x10G+2x40G
  4: 12xQSGMII+4x10G+4x40G*/
enum sys_datapath_qm_mode_e
{
    SYS_DATAPATH_QM_MODE_0,       /*TXQM1: 4 6 7 8      TXQM3: 13 14 15 16 */
    SYS_DATAPATH_QM_MODE_1,       /*TXQM1: 4 5 6 7      TXQM3: 13 14 15 16 */
    SYS_DATAPATH_QM_MODE_2,       /*TXQM1: 4 5 6 7      TXQM3: 13 14 17 16 */
    SYS_DATAPATH_QM_MODE_3,       /*TXQM1: 4 5 6 7      TXQM3: 13 14 15 17 */
    SYS_DATAPATH_QM_MODE_4,       /*TXQM1: 4 5 6 7/8    TXQM3: 13 14 15 16 */
    SYS_DATAPATH_MAC_EN_BUTT
};

enum sys_datapath_pcs_type
{
    SYS_DATAPATH_Q_U_PCS = 0,
    SYS_DATAPATH_SHARED_PCS = 1,
    SYS_DATAPATH_PCS_BUTT
};

enum sys_datapath_dynamic_mac_flag
{
    SYS_DATAPATH_NO_CHANGE  = 0,
    SYS_DATAPATH_ADD        = 1,
    SYS_DATAPATH_DROP       = 2,
};

typedef struct {
    int32 table_id;
    uint32 add_drop_flag;  /* 0-no add, no drop; 1-add; 2-drop */
}sys_datapath_dynamic_switch_table_t;

struct sys_datapath_qm_choice_s
{
    uint8 txqm1;     /*RlmMacMuxModeCfg.cfgTxqm1Mode*/
    uint8 txqm3;     /*RlmMacMuxModeCfg.cfgTxqm3Mode*/
    uint8 qmmode;    /*Combination of txqm1 and txqm3 value*/
    uint8 muxmode;   /*RlmHsMuxModeCfg.cfgHssUnit2MuxMode*/
};
typedef struct sys_datapath_qm_choice_s sys_datapath_qm_choice_t;

struct sys_datapath_serdes_info_s
{
    uint8 mode;        /*pcs mode, refer to ctc_chip_serdes_mode_t*/
    uint8 pll_sel;       /* 0: disable,1:plla. 2:pllb    for TM it means cmu_id+1, 0: disable, 1: plla(CMU0), 2: pllb(CMU1), 3: pllc(CMU2) */
    uint8 rate_div;     /*0: full, 1:1/2, 2:1/4, 3:1/8*/
    uint8 bit_width;   /* bit width*/
    uint16 lport;        /*chip local phy port*/
    uint8 port_num;  /*serdes port count*/
    uint8 rx_polarity;  /*0:normal, 1:reverse*/
    uint8 tx_polarity;  /*0:normal, 1:reverse*/
    uint8 lane_id;  /*serdes internal lane id */
    uint8 is_dyn;
    uint8 group;
    uint16 overclocking_speed; /*serdes over clock*/
    uint8 rsv[2];
};
typedef struct sys_datapath_serdes_info_s   sys_datapath_serdes_info_t;


struct sys_datapath_hss_attribute_s
{
    uint8 hss_type;   /*0:Hss15G, 1:Hss28G, refer sys_datapath_hss_type_t*/
    uint8 hss_id;        /*for hss15g is 0~2, for hss28g is 0~3*/
    uint8 plla_mode;  /*refer to sys_datapath_hssclk_t  For TM it refers to sys_datapath_12g_cmu_type_t*/
    uint8 pllb_mode;  /*refer to sys_datapath_hssclk_t  For TM it refers to sys_datapath_12g_cmu_type_t */
    uint8 pllc_mode;  /*(TM ONLY) refers to sys_datapath_12g_cmu_type_t*/
    uint8 core_div_a[SYS_DATAPATH_CORE_NUM];  /*refer to sys_datapath_clktree_div_t, usefull plla is used*/
    uint8 core_div_b[SYS_DATAPATH_CORE_NUM];  /*refer to sys_datapath_clktree_div_t, usefull plla is used*/
    uint8 intf_div_a;   /*refer to sys_datapath_clktree_div_t, usefull pllb is used*/
    uint8 intf_div_b;   /*refer to sys_datapath_clktree_div_t, usefull pllb is used*/
    sys_datapath_serdes_info_t  serdes_info[HSS15G_LANE_NUM];   /*hss28G only 4 serdes info is valid, refer to sys_datapath_serdes_info_t  */
    uint8 plla_ref_cnt;      /*indicate used lane number for the plla, if the num decrease to 0, set plla to disable*/
    uint8 pllb_ref_cnt;      /*indicate used lane number for the pllb, if the num decrease to 0, set pllb to disable*/
    uint8 pllc_ref_cnt;      /*indicate used lane number for the pllc, if the num decrease to 0, set pllc to disable  For TM only*/
    uint16 clktree_bitmap; /*indicate clktree need cfg, refer to  sys_datapath_clktree_op_t */
};
typedef struct sys_datapath_hss_attribute_s  sys_datapath_hss_attribute_t;

struct sys_datapath_serdes_pll_info_s
{
    uint8 chg_flag;      /*pll change flag    if dst_mode changed but pll_sel not, set FALSE*/
    uint8 pll_sel;       /*0: disable, 1: plla(CMU0), 2: pllb(CMU1), 3: pllc(CMU2) */
    uint8 src_mode;
    uint8 dst_mode;
    uint16 dst_ovclk;     /*sys_datapath_serdes_overclocking_speed_t*/
    uint8 ovclk_flag;  /*0-no change  1-(Any)XFI 12.5G  2-(Any)disable  3-(28G)XFI 11.40625G  4-(28G)XXVG 28.125G*/
};
typedef struct sys_datapath_serdes_pll_info_s   sys_datapath_serdes_pll_info_t;

struct sys_datapath_hss_pll_info_s
{
    uint8 pll_chg_flag;                     /*hss pll change flag, 2-change without reset  1-change & reset  0-no change, no reset*/
    uint8 plla_mode;                        /*sys_datapath_12g_cmu_type_t*/
    uint8 pllb_mode;                        /*sys_datapath_12g_cmu_type_t */
    uint8 pllc_mode;                        /*sys_datapath_12g_cmu_type_t*/
    uint8 plla_ref_cnt;                     /*indicate used lane number for the plla, if the num decrease to 0, set plla to disable*/
    uint8 pllb_ref_cnt;                     /*indicate used lane number for the pllb, if the num decrease to 0, set pllb to disable*/
    uint8 pllc_ref_cnt;                     /*indicate used lane number for the pllc, if the num decrease to 0, set pllc to disable*/
    sys_datapath_serdes_pll_info_t serdes_pll_sel[HSS12G_LANE_NUM];  /* 0: disable, 1: plla(CMU0), 2: pllb(CMU1), 3: pllc(CMU2) */
};
typedef struct sys_datapath_hss_pll_info_s sys_datapath_hss_pll_info_t;


enum sys_datapath_hssclk_e
{
    SYS_DATAPATH_HSSCLK_DISABLE,          /* no use*/
    SYS_DATAPATH_HSSCLK_515DOT625,      /* 515.625M: for xfi,xlg */
    SYS_DATAPATH_HSSCLK_500,                  /* 500M: for sgmii/qsgmii */
    SYS_DATAPATH_HSSCLK_644DOT53125,   /* 644.53125M: for cg */
    SYS_DATAPATH_HSSCLK_625,                  /* 625M: for xaui,d-xaui,2.5G */
    SYS_DATAPATH_HSSCLK_567DOT1875,     /* 567.1875M: for 11g, 44g*/
    SYS_DATAPATH_HSSCLK_MAX
};
typedef enum sys_datapath_hssclk_e sys_datapath_hssclk_t;

enum sys_datapath_12g_cmu_type_e
{
    SYS_DATAPATH_CMU_IDLE = 0,      /*idle*/
    SYS_DATAPATH_CMU_TYPE_1,        /*XFI XLG USXGMII-S USXGMII-M*/
    SYS_DATAPATH_CMU_TYPE_2,        /*SGMII-1G QSGMII*/
    SYS_DATAPATH_CMU_TYPE_3,        /*SGMII-2G5 XAUI DXAUI*/
    SYS_DATAPATH_CMU_TYPE_4,        /*XFI-12G5*/
    SYS_DATAPATH_CMU_MAX
};
typedef enum sys_datapath_12g_cmu_type_e sys_datapath_12g_cmu_type_t;

enum sys_datapath_28g_cmu_type_e
{
    SYS_DATAPATH_28G_CMU_IDLE = 0,      /*idle*/
    SYS_DATAPATH_28G_CMU_TYPE_1,        /*XXVG LG CG*/
    SYS_DATAPATH_28G_CMU_TYPE_2,        /*XFI XLG*/
    SYS_DATAPATH_28G_CMU_TYPE_3,        /*SGMII-1G*/
    SYS_DATAPATH_28G_CMU_TYPE_4,        /*SGMII-2G5 XAUI*/
    SYS_DATAPATH_28G_CMU_TYPE_5,        /*XXVG-28.125G*/
    SYS_DATAPATH_28G_CMU_TYPE_6,        /*XFI-12.5G*/
    SYS_DATAPATH_28G_CMU_TYPE_7,        /*DXAUI*/
    SYS_DATAPATH_28G_CMU_TYPE_8,        /*XFI-11.40625G*/
    SYS_DATAPATH_28G_CMU_TYPE_9,        /*XFI-12.96875G*/
    SYS_DATAPATH_28G_CMU_MAX
};
typedef enum sys_datapath_12g_cmu_type_e sys_datapath_28g_cmu_type_t;

enum sys_datapath_pll_sel_e
{
    SYS_DATAPATH_PLL_SEL_NONE,
    SYS_DATAPATH_PLL_SEL_PLLA,
    SYS_DATAPATH_PLL_SEL_PLLB,
    SYS_DATAPATH_PLL_SEL_BOTH
};
typedef enum sys_datapath_pll_sel_e sys_datapath_pll_sel_t;

enum sys_datapath_serdes_dir_e
{
    SYS_DATAPATH_SERDES_DIR_RX,
    SYS_DATAPATH_SERDES_DIR_TX,
    SYS_DATAPATH_SERDES_DIR_BOTH
};
typedef enum sys_datapath_serdes_dir_e sys_datapath_serdes_dir_t;

enum sys_datapath_hss_type_e
{
    SYS_DATAPATH_HSS_TYPE_15G,
    SYS_DATAPATH_HSS_TYPE_28G
};
typedef enum sys_datapath_hss_type_e sys_datapath_hss_type_t;

enum sys_datapath_debug_type_e
{
    SYS_DATAPATH_DEBUG_HSS,
    SYS_DATAPATH_DEBUG_SERDES,
    SYS_DATAPATH_DEBUG_CLKTREE,
    SYS_DATAPATH_DEBUG_CALENDAR
};
typedef enum sys_datapath_debug_type_e sys_datapath_debug_type_t;


enum sys_datapath_clktree_op_e
{
    SYS_DATAPATH_CLKTREE_INTF_A,
    SYS_DATAPATH_CLKTREE_CORE_A0,
    SYS_DATAPATH_CLKTREE_CORE_A1,
    SYS_DATAPATH_CLKTREE_INTF_B,
    SYS_DATAPATH_CLKTREE_CORE_B0,
    SYS_DATAPATH_CLKTREE_CORE_B1
};
typedef enum sys_datapath_clktree_op_e sys_datapath_clktree_op_t;

enum sys_datapath_serdes_overclocking_speed_e
{
    SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_NONE_MODE = 0,
    SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_11G_MODE = 11,
    SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_12DOT5G_MODE = 12,

    SYS_DATAPATH_MAX_SERDES_OVERCLOCKING_SPEED_MODE
};
typedef enum sys_datapath_serdes_overclocking_speed_e sys_datapath_serdes_overclocking_speed_t;

typedef struct {
    uint32 serdes_id;
    uint32 src_mode;
    uint32 dst_mode;
    uint32 ovclk_flag;  /*0-no change  1-(Any)XFI 12.5G  2-(Any)disable  3-(28G)XFI 11.40625G  4-(28G)XXVG 28.125G*/
}sys_datapath_dynamic_switch_serdes_t;

typedef struct {
    int32 mac_id;
    uint32 add_drop_flag;  /* 0-no add, no drop; 1-add; 2-drop */
}sys_datapath_dynamic_switch_mac_t;

struct sys_datapath_dynamic_switch_attr_s
{
    uint32 mac_num;
    sys_datapath_dynamic_switch_mac_t m[SYS_DATAPATH_DS_MAX_PORT_NUM];
    sys_qos_shape_profile_t shp_profile[SYS_DATAPATH_DS_MAX_PORT_NUM];
    uint32 serdes_num;
    sys_datapath_dynamic_switch_serdes_t s[SYS_DATAPATH_DS_MAX_SERDES_NUM];
};
typedef struct sys_datapath_dynamic_switch_attr_s sys_datapath_dynamic_switch_attr_t;

struct sys_datapath_an_ability_s
{
    uint32 base_ability0;
    uint32 base_ability1;
    uint32 np0_ability0;
    uint32 np0_ability1;
    uint32 np1_ability0;
    uint32 np1_ability1;
};
typedef struct sys_datapath_an_ability_s sys_datapath_an_ability_t;

struct sys_datapath_lport_xpipe_mapping_s
{
    uint8 mac_id;
    uint8 emac_id;
    uint8 pmac_id;
};
typedef struct sys_datapath_lport_xpipe_mapping_s sys_datapath_lport_xpipe_mapping_t;

struct sys_datapath_lport_attr_s
{
    uint8 port_type;        /*refer to sys_dmps_lport_type_t */
    uint8 mac_id;
    uint8 chan_id;
    uint8 speed_mode;       /*refer to ctc_port_speed_t, only usefull for network port*/

    uint8 slice_id;
    uint8 serdes_num;       /*serdes num of port*/
    uint8 pcs_mode;         /*refer to ctc_chip_serdes_mode_t*/
    uint8 serdes_id;

    uint8 is_serdes_dyn;
    uint8 is_first;         /* 40G/100G, 4 interrupts will process the first*/
    uint8 flag;             /* for D2 50G, if serdes lane 0/1 and 2/3 form 50G port, flag is eq to 0;
                                     if serdes lane 2/1 and 0/3 form 50G port, flag is eq to 1; 
                                     (TM ONLY) if LG lane swap is illegal, flag eq to 2*/
    uint8 interface_type;   /* refer to ctc_port_if_type_t */

    uint8 an_fec;      /* refer to ctc_port_fec_type_t */
    uint8 code_err_count;   /* code error count*/
    uint8 pcs_reset_en;
    uint8 tbl_id;           /*TM ONLY  table id for mac table 0~71*/

    uint8 sgmac_idx;
    uint8 mii_idx;
    uint8 pcs_idx;
    uint8 internal_mac_idx;

    uint8 first_led_mode;
    uint8 sec_led_mode;

    uint8 xpipe_en;
    uint8 pmac_id;
    uint8 emac_id;
    uint8 pmac_chanid;
    uint8 emac_chanid;

    uint8 mutex_flag;
};
typedef struct sys_datapath_lport_attr_s sys_datapath_lport_attr_t;

struct sys_datapath_master_s
{
    ctc_vector_t* p_hss_vector; /* store hss and serdes info */
    sys_datapath_lport_attr_t port_attr[SYS_DATAPATH_MAX_LOCAL_SLICE_NUM][256];
    uint8 wlan_enable;
    uint8 dot1ae_enable;
    uint8 oam_chan;
    uint8 dma_chan;
    uint8 iloop_chan;
    uint8 eloop_chan;
    uint8 cawapdec0_chan;
    uint8 cawapdec1_chan;
    uint8 cawapdec2_chan;
    uint8 cawapdec3_chan;
    uint8 cawapenc0_chan;
    uint8 cawapenc1_chan;
    uint8 cawapenc2_chan;
    uint8 cawapenc3_chan;
    uint8 cawapreassemble_chan;
    uint8 macdec_chan;
    uint8 macenc_chan;
    uint8 elog_chan;
    uint8 qcn_chan;
    uint16 core_plla;   /*used for clock core*/
    uint16 core_pllb;   /*used for mdio/macled etc*/
    sys_datapath_qm_choice_t qm_choice;     /*for TM only*/
    uint8  serdes_mac_map[4][32];           /*for TM only*/
    uint16 mii_lport_map[4][32];            /*for TM only*/
    uint8  glb_xpipe_en;            /*for TM and later*/
};
typedef struct sys_datapath_master_s sys_datapath_master_t;

struct sys_datapath_influenced_txqm_s
{
    uint8 txqm_num;
    uint8 txqm_id[SYS_CALENDAR_TXQM_NUM];
};
typedef struct sys_datapath_influenced_txqm_s sys_datapath_influenced_txqm_t;

typedef int32 (* SYS_CALLBACK_DATAPATH_FUN_P)  (uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr);


enum sys_datapath_hss_cfg_bit_e
{
    /*
     * Rate Select [1:0]
     *   00 - Full-rate(default)
     *   01 - Half-rate
     *   10 - Quarter-rate
     *   11 - Eighth-rate
     */
    SYS_DATAPATH_HSS_RATE_SEL_BIT = 0,
    /*
     * Data Bus Width Select [3:2]
     *   00 - 8-bit (not valid in full rate mode, 16-bit mode is forced internally)
     *   01 - 10-bit (not valid in full rate mode, 20-bit mode is forced internally)
     *   10 - 16-bit
     *   11 - 20-bit (default)
     */
    SYS_DATAPATH_HSS_BW_SEL_BIT = 2,
    /* SYS_DATAPATH_HSS_DFE_BIT = 4, */
    /*
     * PLL Select
     *   00 - PLLA selected: Low range PLL (7.5-12.8G)
     *   01 - PLLB selected: High range PLL (10-15.0G)
     *   1x - INVALID (No PLL selected. Therefore, the link is nonfunctional. Do not use)
              On HSSRESET, this register is reset to '00' = PLLA unless HSSREFCLKVALIDA = '0',
              then this register resets to '01' = PLLB.
     */
    SYS_DATAPATH_HSS_PLL_SEL_BIT = 6,
    /*
     * Configuration Mode Write
     *   0  - Write disabled. The preconfiguration bits listed in Table 3-8 on page 125 are read-only (default).
     *   1  - Write enabled. Preconfiguration bits in this register listed in Table 3-8 are read/write.
     */
    SYS_DATAPATH_HSS_WR_MODE_BIT = 13,
    /*
     * LINK Reset
     *   0  - Normal operation(default)
     *   1  - Link reset applied (this setting creates reset condition L)
     */
    SYS_DATAPATH_HSS_LINK_RST = 14,
    /*
     * LINK Enable
     *   0  - Disabled
     *   1  - Enabled(deafult)
     */
    SYS_DATAPATH_HSS_LINK_EN_BIT = 15
};
typedef enum sys_datapath_hss_cfg_bit_e sys_datapath_hss_cfg_bit_t;

enum sys_datapath_bit_width_e
{
    SYS_DATAPATH_BIT_WIDTH_8BIT,
    SYS_DATAPATH_BIT_WIDTH_10BIT,
    SYS_DATAPATH_BIT_WIDTH_20BIT,
    SYS_DATAPATH_BIT_WIDTH_40BIT
};
typedef enum sys_datapath_bit_width_e sys_datapath_bit_width_t;

enum sys_datapath_serdes_div_e
{
    SYS_DATAPATH_SERDES_DIV_FULL,
    SYS_DATAPATH_SERDES_DIV_HALF,
    SYS_DATAPATH_SERDES_DIV_QUAD,
    SYS_DATAPATH_SERDES_DIV_EIGHTH
};
typedef enum sys_datapath_serdes_div_e sys_datapath_serdes_div_t;

enum sys_datapath_clktree_div_e
{
    SYS_DATAPATH_CLKTREE_DIV_USELESS,
    SYS_DATAPATH_CLKTREE_DIV_FULL,
    SYS_DATAPATH_CLKTREE_DIV_HALF,
    SYS_DATAPATH_CLKTREE_DIV_QUAD,
    SYS_DATAPATH_CLKTREE_DIV_EIGHTH
};
typedef enum sys_datapath_clktree_div_e sys_datapath_clktree_div_t;

struct sys_datapath_bufsz_step_s
{
    uint32 sop_buf_num;
    uint32 body_buf_num;
    int32  br_credit_cfg;
};
typedef struct sys_datapath_bufsz_step_s sys_datapath_bufsz_step_t;

enum sys_datapath_speed_range_e
{
    SPEED_MODE_100 = 0,       /* 100G */
    SPEED_MODE_50  = 1,       /* 50G */
    SPEED_MODE_40  = 2,       /* 40G */
    SPEED_MODE_25  = 3,       /* 25G */
    SPEED_MODE_10  = 4,       /* 10G */
    SPEED_MODE_5   = 5,       /* 5G */
    SPEED_MODE_0_5 = 6,       /* 10M/100M/1G/2.5G */
    SPEED_MODE_MISC_100 = 7,  /* Misc channel 100G: MacSec/Eloop*/
    SPEED_MODE_MISC_10  = 8,  /* Misc channel 10G: Wlan*/
    SPEED_MODE_INV,           /* Invalid port */

    SPEED_MODE_MAX,
};
typedef enum sys_datapath_speed_range_e sys_datapath_speed_range_t;

typedef struct
{
    ctc_chip_serdes_mode_t  port_mode;
    uint32 serdes_num;
    uint32 mac_num;
    int32 mac_list[4][4];
} sys_datapath_serdes_group_mac_info_t;

enum sys_datapath_hss28g_clk_level
{
    SYS_DATAPATH_28G_CLK_LEVEL_0,        /*SGMII*/
    SYS_DATAPATH_28G_CLK_LEVEL_1,        /*2.5G XAUI*/
    SYS_DATAPATH_28G_CLK_LEVEL_2,        /*DXAUI*/
    SYS_DATAPATH_28G_CLK_LEVEL_3,        /*XFI XLG*/
    SYS_DATAPATH_28G_CLK_LEVEL_4,        /*XXVG LG CG*/
    SYS_DATAPATH_28G_CLK_LEVEL_BUTT
};

/***************************************************************
 *
 * Function declaration
 *
 ***************************************************************/
extern int32 sys_usw_datapath_show_status(uint8 lchip);
extern int32 sys_usw_datapath_show_status_public(uint8 lchip);
extern int32 sys_usw_datapath_show_info(uint8 lchip, uint8 type, uint32 start, uint32 end, uint8 is_all);
extern void* sys_usw_datapath_get_port_capability(uint8 lchip, uint16 lport, uint8 slice_id);
extern uint16 sys_usw_datapath_get_lport_with_mac(uint8 lchip, uint8 slice_id, uint8 mac_id);
extern uint16 sys_usw_datapath_get_lport_with_mac_tbl_id(uint8 lchip, uint8 slice_id, uint8 mac_tbl_id);
extern int32 sys_usw_datapath_init(uint8 lchip, void* p_global_cfg);
extern int32 sys_usw_datapath_set_serdes_polarity(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_dynamic_switch(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);
extern int32 sys_usw_datapath_set_serdes_dynamic_switch(uint8 lchip, uint8 serdes_id, uint8 mode, uint16 overclocking_speed);
extern int32 sys_usw_datapath_set_serdes_loopback(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_serdes_prbs(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_serdes_prbs_tx(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_serdes_ffe(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_get_serdes_with_lport(uint8 lchip, uint16 chip_port, uint8 slice, uint16* p_serdes_id, uint8* num);
extern int32 sys_usw_datapath_set_link_reset(uint8 lchip, uint8 serdes_id, uint8 enable, uint8 dir);
extern int32 sys_usw_datapath_set_dfe_reset(uint8 lchip, uint8 serdes_id, uint8 enable);
extern uint32 sys_usw_datapath_get_serdes_sigdet(uint8 lchip, uint16 serdes_id);
extern int32 sys_duet2_serdes_set_link_training_en(uint8 lchip, uint16 serdes_id, bool enable);
extern int32 sys_usw_datapath_get_serdes_ffe(uint8 lchip, uint8 serdes_id, uint16* coefficient, uint8 ffe_mode, uint8* status);
extern int32 sys_usw_datapath_get_serdes_polarity(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_get_serdes_loopback(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_serdes_cfg(uint8 lchip, uint32 prop_type, void* p_data);
extern int32 sys_usw_datapath_get_serdes_cfg(uint8 lchip, uint32 prop_type, void* p_data);
extern uint16 sys_usw_datapath_get_core_clock(uint8 lchip, uint8 core_type);
extern int32 sys_usw_datapath_get_hss_info(uint8 lchip, uint8 hss_idx, sys_datapath_hss_attribute_t** p_hss);
extern int32 sys_usw_datapath_get_serdes_info(uint8 lchip, uint8 serdes_id, sys_datapath_serdes_info_t** p_serdes);
extern int16 sys_usw_datapath_set_serdes_auto_neg_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability);
extern int16 sys_usw_datapath_get_serdes_auto_neg_local_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability);
extern int16 sys_usw_datapath_get_serdes_auto_neg_remote_ability(uint8 lchip, uint16 serdes_id, sys_datapath_an_ability_t* p_ability);
extern int32 sys_usw_datapath_get_gport_with_serdes(uint8 lchip, uint16 serdes_id, uint32* p_gport);
extern int16 sys_usw_datapath_set_serdes_auto_neg_en(uint8 lchip, uint16 serdes_id, uint32 enable);
extern int32 sys_usw_datapath_set_serdes_tx_en(uint8 lchip, uint16 serdes_id, bool enable);
extern int16 sys_usw_datapath_set_serdes_aec_rx_en(uint8 lchip, uint16 serdes_id, uint32 enable);
extern int32 sys_usw_datapath_get_serdes_cl73_status(uint8 lchip, uint16 serdes_id, uint16* p_value);
extern int32 sys_duet2_serdes_get_link_training_status(uint8 lchip, uint16 serdes_id, uint16* p_value);
extern int32 sys_usw_datapath_get_useless_port(uint8 lchip, uint32* p_lport);
extern int32 sys_usw_datapath_set_dfe_en(uint8 lchip, uint8 serdes_id, uint8 enable);
extern uint8 sys_usw_datapath_get_wlan_en(uint8 lchip);
extern uint8 sys_usw_datapath_get_dot1ae_en(uint8 lchip);
extern int32 sys_usw_datapath_deinit(uint8 lchip);
extern int32 sys_duet2_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32 sys_duet2_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32 sys_duet2_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);
extern int32 sys_usw_datapath_chip_reset_recover_proc(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_module_init(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

