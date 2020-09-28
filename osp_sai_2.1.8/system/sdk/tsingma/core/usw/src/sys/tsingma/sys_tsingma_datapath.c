/**
 @file sys_tsingma_datapath.c

 @date 2017-11-31

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
#include "sys_usw_qos_api.h"
#include "sys_usw_mac.h"
#include "sys_usw_peri.h"
#include "sys_usw_dmps.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_mcu.h"
#include "sys_tsingma_datapath.h"
#include "sys_tsingma_mac.h"
#include "sys_tsingma_mcu.h"

#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"

#include "sys_tsingma_hss28g_firmware.inc"

/*0-no force cfg  1-mux choose qm8  2-mux choose qm17  3-serdes 20~23 forbidden  4-serdes 28~31 forbidden*/
uint8 g_qm_force_cfg = 1;

 /*/TODO:*/
/*
 !!!Warning!!!
 The address listed below is based on drv_ds_tm.h, 
 so every time drv_ds_tm.h has been updated, ensure
 that these address is same with those in .h file.
 If not, address difference may cause unpredictable
 problem.
*/
#define SUP_REG_CTL                     0x00000098   /*SupRegCtl*/
#define SUP_RESET_CTL_ADDR              0x00042070   /*SupResetCtl*/
#define RLM_HS_CTL_EN_CLK_ADDR          0x00400068   /*RlmHsCtlEnClk*/
#define RLM_CS_CTL_EN_CLK_ADDR          0x00200064   /*RlmCsCtlEnClk*/
#define RLM_MAC_CTL_EN_CLK_ADDR         0x00800020   /*RlmMacCtlEnClk*/
#define RLM_BSR_CTL_EN_CLK_ADDR         0x00a0002c   /*RlmBsrCtlEnClk*/
#define RLM_HS_CTL_RESET_ADDR           0x00400064   /*RlmHsCtlReset*/
#define RLM_CS_CTL_RESET_ADDR           0x00200060   /*RlmCsCtlReset*/
#define RLM_MAC_CTL_RESET_ADDR          0x00800000   /*RlmMacCtlReset*/
#define RLM_NET_CTL_RESET_ADDR          0x00c00020   /*RlmNetCtlReset*/
#define RLM_IPERX_CTL_RESET_ADDR        0x01800020   /*RlmIpeRxCtlReset*/
#define RLM_IPETX_CTL_RESET_ADDR        0x01400020   /*RlmIpeTxCtlReset*/
#define RLM_EPERX_CTL_RESET_ADDR        0x00e00020   /*RlmEpeRxCtlReset*/
#define RLM_EPETX_CTL_RESET_ADDR        0x01000020   /*RlmEpeTxCtlReset*/
#define RLM_BSR_CTL_RESET_ADDR          0x00a00028   /*RlmBsrCtlReset*/
#define RLM_QMGR_CTL_RESET_ADDR         0x00600020   /*RlmQMgrCtlReset*/
#define RLM_KEY_CTL_RESET_ADDR          0x08000020   /*RlmKeyCtlReset*/
#define RLM_TCAM_CTL_RESET_ADDR         0x02000020   /*RlmTcamCtlReset*/
#define RLM_AD_CTL_RESET_ADDR           0x04000020   /*RlmAdCtlReset*/
#define RLM_SECURITY_CTL_RESET_ADDR     0x00120020   /*RlmSecurityCtlReset*/
#define RLM_CAPWAP_CTL_RESET_ADDR       0x00140020   /*RlmCapwapCtlReset*/
#define SUP_INTR_FUNC_ADDR              0x00042080   /*SupIntrFunc*/
#define RLM_BUFFER_CTL_EN_CLK_ADDR      0x06000024   /*RlmBufferCtlEnClk*/
#define RLM_CAPWAP_CTL_EN_CLK_ADDR      0x00140024   /*RlmCapwapCtlEnClk*/
#define RLM_SECURITY_CTL_EN_CLK_ADDR    0x00120024   /*RlmSecurityCtlEnClk*/
#define HSS_12G_UNIT_EN_CLK0_ADDR       0x00483298   /*Hss12GUnitEnClk0*/
#define HSS_12G_UNIT_EN_CLK1_ADDR       0x00501298   /*Hss12GUnitEnClk1*/
#define HSS_12G_UNIT_EN_CLK2_ADDR       0x00584298   /*Hss12GUnitEnClk2*/
#define HSS_28G_UNIT_EN_CLK_ADDR        0x00283104   /*Hss28GUnitEnClk*/
#define HSS_12G_UNIT_RESET0_ADDR        0x00483260   /*Hss12GUnitReset0*/
#define HSS_12G_UNIT_RESET1_ADDR        0x00501260   /*Hss12GUnitReset1*/
#define HSS_12G_UNIT_RESET2_ADDR        0x00584260   /*Hss12GUnitReset2*/
#define HSS_28G_UNIT_RESET_ADDR         0x002830e0   /*Hss28GUnitReset*/
#define RLM_BUFFER_CTL_RESET_ADDR       0x06000020   /*RlmBufferCtlReset*/

#define NETTX_MEM_MAX_DEPTH_PER_TXQM 256
#define BUFRETRV_MEM_MAX_DEPTH 320
#define EPE_BODY_MEM_MAX_DEPTH 384
#define EPE_SOP_MEM_MAX_DEPTH 768

#define TSINGMA_SERDES_NUM_PER_SLICE   32
#define TSINGMA_HSS28G_NUM_PER_SLICE   2
#define TSINGMA_HSS15G_NUM_PER_SLICE   3
#define TSINGMA_SYS_DATAPATH_HSS_NUM   (TSINGMA_HSS15G_NUM_PER_SLICE+TSINGMA_HSS28G_NUM_PER_SLICE)

#define SYS_DATAPATH_MAP_HSSID_TO_HSSIDX(id, type)  \
    (SYS_DATAPATH_HSS_TYPE_15G == type) ? (id) : (id + TSINGMA_HSS15G_NUM_PER_SLICE)
    
#define SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% TSINGMA_SERDES_NUM_PER_SLICE) - SYS_DATAPATH_SERDES_28G_START_ID)/HSS28G_LANE_NUM) \
    :(((serdes_id)% TSINGMA_SERDES_NUM_PER_SLICE)/HSS12G_LANE_NUM)

#define SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% TSINGMA_SERDES_NUM_PER_SLICE) - SYS_DATAPATH_SERDES_28G_START_ID)/HSS28G_LANE_NUM + SYS_DATAPATH_SERDES_28G_START_ID/HSS12G_LANE_NUM) \
    :(((serdes_id)% TSINGMA_SERDES_NUM_PER_SLICE)/HSS12G_LANE_NUM)

#define SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id) \
    SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) \
    ?(((serdes_id% TSINGMA_SERDES_NUM_PER_SLICE) - SYS_DATAPATH_SERDES_28G_START_ID)%HSS28G_LANE_NUM) \
    :((serdes_id% TSINGMA_SERDES_NUM_PER_SLICE)%HSS12G_LANE_NUM)

#define SYS_TSINGMA_CMUID_TO_PLLSEL(cmu_id)       ((cmu_id)+1)
#define SYS_TSINGMA_PLLSEL_TO_CMUID(pll_sel)      ((pll_sel)-1)

#define SYS_TSINGMA_GET_LG_ANOTHER_SERDES(serdes_id, another_lane) switch(serdes_id % 4) \
    { \
        case 0: \
            another_lane = serdes_id + 3; \
            break; \
        case 1: \
            another_lane = serdes_id + 1; \
            break; \
        case 2: \
            another_lane = serdes_id - 1; \
            break; \
        case 3: \
        default: \
            another_lane = serdes_id - 3; \
            break; \
    }

#define SYS_TSINGMA_GET_LG_SWAP_SERDES(mac_id, lane_0, lane_1) switch(mac_id) \
    { \
        case 44: \
            lane_0 = 25; \
            lane_1 = 26; \
            break; \
        case 46: \
            lane_0 = 24; \
            lane_1 = 27; \
            break; \
        case 60: \
            lane_0 = 29; \
            lane_1 = 30; \
            break; \
        case 62: \
        default: \
            lane_0 = 28; \
            lane_1 = 31; \
            break; \
    }


#define HSS28G_WR_DELAY(lchip, acc_id, addr, val)\
{  \
    drv_chip_write_hss28g(lchip, acc_id, addr, val);\
    sal_task_sleep(5);\
}

#define HSS12G_MODE_IS_32BIT_FULLRATE(mode) ((CTC_CHIP_SERDES_XFI_MODE == mode) || (CTC_CHIP_SERDES_XLG_MODE == mode) || \
                                             (CTC_CHIP_SERDES_USXGMII0_MODE == mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == mode) || \
                                             (CTC_CHIP_SERDES_USXGMII2_MODE == mode))

#define HSS12G_JUDGE_USXGMII_TXSEL_CHANGE(src_mode, dst_mode, serdes_id) ((((CTC_CHIP_SERDES_USXGMII1_MODE == src_mode) || \
                                                                  (CTC_CHIP_SERDES_USXGMII2_MODE == src_mode)) && \
                                                                 ((CTC_CHIP_SERDES_USXGMII1_MODE != dst_mode) && \
                                                                  (CTC_CHIP_SERDES_USXGMII2_MODE != dst_mode))) || \
                                                                (((CTC_CHIP_SERDES_USXGMII1_MODE != src_mode) && \
                                                                  (CTC_CHIP_SERDES_USXGMII2_MODE != src_mode)) && \
                                                                 ((CTC_CHIP_SERDES_USXGMII1_MODE == dst_mode) || \
                                                                  (CTC_CHIP_SERDES_USXGMII2_MODE == dst_mode))) || \
                                                                ((0 == serdes_id % 4) && \
                                                                 (((CTC_CHIP_SERDES_USXGMII2_MODE == src_mode) && \
                                                                   (CTC_CHIP_SERDES_USXGMII1_MODE == dst_mode)) || \
                                                                  ((CTC_CHIP_SERDES_USXGMII1_MODE == src_mode) && \
                                                                   (CTC_CHIP_SERDES_USXGMII2_MODE == dst_mode)))))
                                                             
#define HSS12G_JUDGE_MULTI_LANE_MODE(mode)  ((CTC_CHIP_SERDES_XLG_MODE == mode) || (CTC_CHIP_SERDES_XAUI_MODE == mode) || \
                                             (CTC_CHIP_SERDES_DXAUI_MODE == mode))

#define HSS12G_JUDGE_MULTI_LANE_CHANGE(src_mode, dst_mode) (((HSS12G_JUDGE_MULTI_LANE_MODE(src_mode)) && \
                                                              (!HSS12G_JUDGE_MULTI_LANE_MODE(dst_mode))) || \
                                                             ((!HSS12G_JUDGE_MULTI_LANE_MODE(src_mode)) && \
                                                              (HSS12G_JUDGE_MULTI_LANE_MODE(dst_mode))))

#define HSS28G_MODE_IS_SUPPORT_SWAP(mode) ((CTC_CHIP_SERDES_XFI_MODE == mode) || (CTC_CHIP_SERDES_XLG_MODE == mode) || \
                                           (CTC_CHIP_SERDES_XXVG_MODE == mode) || (CTC_CHIP_SERDES_LG_MODE == mode) || \
                                           (CTC_CHIP_SERDES_CG_MODE == mode))

#define HSS28G_JUDGE_SWAP_CHANGE(src_mode, dst_mode) (HSS28G_MODE_IS_SUPPORT_SWAP(src_mode) &&  \
                                                      ((CTC_CHIP_SERDES_SGMII_MODE == dst_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == dst_mode) ||  \
                                                       (CTC_CHIP_SERDES_100BASEFX_MODE == dst_mode) || (CTC_CHIP_SERDES_XAUI_MODE == dst_mode) || \
                                                       (CTC_CHIP_SERDES_DXAUI_MODE == dst_mode)))
#define SERDES_JUDGE_LAST_LANE_IN_HSS(serdes_id) (((!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)) && (0 == (serdes_id+1)%HSS12G_LANE_NUM)) || \
                                                  ((SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)) && (0 == (serdes_id+1)%HSS28G_LANE_NUM)))


extern uint8 g_usw_mac_port_map[];

extern sys_datapath_master_t* p_usw_datapath_master[];

/*for hss28g , E,F,G,H is not used */
extern uint8 g_usw_hss_rx_addr_map[];

extern sys_mac_master_t* p_usw_mac_master[];

extern uint8 g_usw_hss15g_lane_map[];

sys_datapath_bufsz_step_t g_tsingma_bufsz_step[] =
{
    {80, 40, 0},   /* 100G */
    {40, 20, 1},   /* 50G */
    {32, 16, 2},   /* 40G */
    {20, 10, 3},   /* 25G */
    {8,  4,  4},   /* 10G */
    {4,  2,  5},   /* 5G */
    {4,  2,  6},   /* 10M/100M/1G/2.5G */
    {60, 30, 7},   /* Misc channel 100G: MacSec/Eloop*/
    {6,  3,  8},   /* Misc channel 10G: Wlan*/
    {0,  0,  15},   /* Invalid port */
};

uint32 g_tsingma_nettx_bufsize[SYS_PHY_PORT_NUM_PER_SLICE] =
{
    36,10,10,10,  10, 6, 6, 6,  36,10,10,10,  36,10,10,10,   /*TXQM0  total 226*/
    10, 6, 6, 6,  36,10,10,10,  36,10,10,10,  36,10,36,10,   /*TXQM1  total 252*/
    10, 6, 6, 6,  10, 6, 6, 6,  10, 6, 6, 6,  62,18,36,18,   /*TXQM2  total 218*/
    10, 6, 6, 6,  10, 6, 6, 6,  36,10,10,10,  62,18,36,18    /*TXQM3  total 256*/
};

uint32 g_tsingma_epesch_mem_origin[(SYS_TSINGMA_EPESCH_MAX_CHANNEL+1)/4] = 
{
    0,   16,  24,  40,   /*QM 0~3*/
    56,  64,  80,  96,   /*QM 4~7*/
    128, 136, 144, 152,  /*QM 8~11*/
    192, 200, 208, 224,  /*QM 12~15*/
    264, 276, 288        /*misc*/
};

extern sys_datapath_lport_xpipe_mapping_t g_tsingma_xpipe_mac_mapping[SYS_USW_XPIPE_PORT_NUM];

int32 sys_tsingma_datapath_dynamic_get_lane_swap_flag(uint8 lchip, uint8* lane_swap_0, uint8* lane_swap_1);
STATIC int32 _sys_tsingma_datapath_update_glb_xpipe(uint8 lchip);
extern int32 _sys_usw_mac_set_mac_config(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr);
extern uint16 sys_usw_datapath_get_lport_with_chan(uint8 lchip, uint8 slice_id, uint8 chan_id);
STATIC int32 _sys_tsingma_datapath_get_net_bufsz_step_by_port_attr(uint8 lchip, sys_datapath_lport_attr_t* port_attr, sys_datapath_bufsz_step_t *step);
STATIC int32 _sys_tsingma_serdes_set_hss28g_link_training_en(uint8 lchip, uint16 serdes_id, uint8 enable);
int32 sys_tsingma_datapath_set_hss28g_firmware_mode(uint8 lchip, uint8 hss_idx, uint8 lane_id);
extern int32 sys_tsingma_mac_write_table_elem(uint8 lchip, tbls_id_t tbl_id, uint32 field_num, fld_id_t field_id[], uint32 value[], void* val_stru);
extern int32 _sys_usw_datapath_get_gcd(uint16* array, uint8 count, uint16* gcd);
extern int32 sys_usw_datapath_get_serdes_polarity(uint8 lchip, void* p_data);
extern int32 sys_usw_datapath_set_calendar_cfg(uint8 lchip, uint8 slice_id, uint32 txqm_id);
extern int32 _sys_usw_datapath_misc_init(uint8 lchip);
extern int32 sys_tsingma_peri_set_phy_scan_cfg(uint8 lchip);
extern int32 _sys_tsingma_mac_set_mac_en(uint8 lchip, uint32 gport, bool enable);
extern int32 _sys_tsingma_mac_set_unidir_en(uint8 lchip, uint32 gport, bool enable);
extern int32 _sys_tsingma_mac_set_xpipe_en(uint8 lchip, uint16 lport, uint8 enable);
extern int32 sys_usw_datapath_wb_restore(uint8 lchip);
extern int32 sys_tsingma_mac_set_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32 value);
extern int32 _sys_tsingma_mac_set_cl37_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable);
extern int32 _sys_tsingma_mac_set_cl73_auto_neg_en(uint8 lchip, uint32 gport, uint32 enable, uint8 restart_flag);
extern int32 sys_tsingma_serdes_hss12g_set_ctle_value(uint8 serdes_id, uint8 ctle_type, uint8 val_u8);
extern int32 sys_tsingma_datapath_switch_serdes_get_none_info(uint8 lchip, uint8 serdes_id, uint8 dst_mode, sys_datapath_serdes_info_t* p_serdes,
                                                              sys_datapath_dynamic_switch_attr_t* p_ds_attr);
extern int32 _sys_tsingma_mac_set_quadsgmaccfg(uint8 lchip, uint32 value, uint8 sgmac_idx);
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define __DEBUG__



#define __CLKTREE__

int32
_sys_tsingma_datapath_get_rate(uint8 mode, uint8 type, uint8 ovclk, uint8* p_width, uint8* p_div, uint8* p_speed)
{
    uint8 hss12g_rate[CTC_CHIP_MAX_SERDES_MODE][3] = {
        //DataWidth                       RateSel                          LaneSpeed
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_NONE_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_XFI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_10BIT, SYS_TM_DATAPATH_SERDES_DIV_OCTL, SYS_TM_LANE_1D25G    }, //CTC_CHIP_SERDES_SGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_XSGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_10BIT, SYS_TM_DATAPATH_SERDES_DIV_HALF, SYS_TM_LANE_5G       }, //CTC_CHIP_SERDES_QSGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_20BIT, SYS_TM_DATAPATH_SERDES_DIV_QUAD, SYS_TM_LANE_3D125G   }, //CTC_CHIP_SERDES_XAUI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_20BIT, SYS_TM_DATAPATH_SERDES_DIV_HALF, SYS_TM_LANE_6D25G    }, //CTC_CHIP_SERDES_DXAUI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_XLG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_CG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_10BIT, SYS_TM_DATAPATH_SERDES_DIV_QUAD, SYS_TM_LANE_3D125G   }, //CTC_CHIP_SERDES_2DOT5G_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_USXGMII0_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_USXGMII1_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_USXGMII2_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_XXVG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_LG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_10BIT, SYS_TM_DATAPATH_SERDES_DIV_OCTL, SYS_TM_LANE_1D25G    }  //CTC_CHIP_SERDES_100BASEFX_MODE
    };
    uint8 hss28g_rate[CTC_CHIP_MAX_SERDES_MODE][3] = {
        //DataWidth                       RateSel                          LaneSpeed
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_NONE_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_HALF, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_XFI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_HEXL, SYS_TM_LANE_1D25G    }, //CTC_CHIP_SERDES_SGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_XSGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_QSGMII_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_OCTL, SYS_TM_LANE_3D125G   }, //CTC_CHIP_SERDES_XAUI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_QUAD, SYS_TM_LANE_6D25G    }, //CTC_CHIP_SERDES_DXAUI_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_HALF, SYS_TM_LANE_10D3125G }, //CTC_CHIP_SERDES_XLG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_25D78125G}, //CTC_CHIP_SERDES_CG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_OCTL, SYS_TM_LANE_3D125G   }, //CTC_CHIP_SERDES_2DOT5G_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_USXGMII0_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_USXGMII1_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }, //CTC_CHIP_SERDES_USXGMII2_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_25D78125G}, //CTC_CHIP_SERDES_XXVG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_32BIT, SYS_TM_DATAPATH_SERDES_DIV_FULL, SYS_TM_LANE_25D78125G}, //CTC_CHIP_SERDES_LG_MODE
        {SYS_TM_DATAPATH_BIT_WIDTH_NULL,  SYS_TM_DATAPATH_SERDES_DIV_NULL, SYS_TM_LANE_SPD_NULL }  //CTC_CHIP_SERDES_100BASEFX_MODE
    };

    switch(ovclk)
    {
        case 1:
            hss28g_rate[CTC_CHIP_SERDES_XFI_MODE][2]  = SYS_TM_LANE_12D5G;
            hss28g_rate[CTC_CHIP_SERDES_XLG_MODE][2]  = SYS_TM_LANE_12D5G;
            hss12g_rate[CTC_CHIP_SERDES_XFI_MODE][2]  = SYS_TM_LANE_12D5G;
            hss12g_rate[CTC_CHIP_SERDES_XLG_MODE][2]  = SYS_TM_LANE_12D5G;
            break;
        case 3:
            hss28g_rate[CTC_CHIP_SERDES_XFI_MODE][2]  = SYS_TM_LANE_11D40625G;
            hss28g_rate[CTC_CHIP_SERDES_XLG_MODE][2]  = SYS_TM_LANE_11D40625G;
            break;
        case 4:
            hss28g_rate[CTC_CHIP_SERDES_XXVG_MODE][2] = SYS_TM_LANE_28D125G;
            hss28g_rate[CTC_CHIP_SERDES_LG_MODE][2]   = SYS_TM_LANE_28D125G;
            hss28g_rate[CTC_CHIP_SERDES_CG_MODE][2]   = SYS_TM_LANE_28D125G;
            break;
    }

    if(CTC_CHIP_MAX_SERDES_MODE <= mode)
    {
        return CTC_E_INVALID_PARAM;
    }
    if(SYS_DATAPATH_HSS_TYPE_28G == type)
    {
        SYS_USW_VALID_PTR_WRITE(p_width, hss28g_rate[mode][0]);
        SYS_USW_VALID_PTR_WRITE(p_div,   hss28g_rate[mode][1]);
        SYS_USW_VALID_PTR_WRITE(p_speed, hss28g_rate[mode][2]);
    }
    else
    {
        SYS_USW_VALID_PTR_WRITE(p_width, hss12g_rate[mode][0]);
        SYS_USW_VALID_PTR_WRITE(p_div,   hss12g_rate[mode][1]);
        SYS_USW_VALID_PTR_WRITE(p_speed, hss12g_rate[mode][2]);
    }

    return CTC_E_NONE;
}

/*
For init and dyn switch only.
To get mac tbl id, use sys_datapath_lport_attr_t.tbl_id
*/
uint16
sys_tsingma_datapath_calc_mac_tblid_by_macid(uint8 lchip, uint8 macid)
{
    uint16 mac_tbl_id = 0;

    if(NULL == p_usw_datapath_master[lchip])
    {
        return SYS_DATAPATH_USELESS_MAC;
    }

    /*macid 0~19*/
    if(MAC_PORT_NUM_PER_TXQM+MAC_PORT_NUM_PER_QM > macid)
    {
        mac_tbl_id = macid;
    }

    /*macid 20~31*/
    else if(2*MAC_PORT_NUM_PER_TXQM > macid)
    {
        if(SYS_DATAPATH_QM_MODE_0 == p_usw_datapath_master[lchip]->qm_choice.qmmode)
        {
            mac_tbl_id = macid+4;
        }
        else
        {
            mac_tbl_id = macid;
        }
        switch(p_usw_datapath_master[lchip]->qm_choice.qmmode)
        {
            case SYS_DATAPATH_QM_MODE_0:
                mac_tbl_id = macid+4;
                break;
            case SYS_DATAPATH_QM_MODE_4:
                /*28~31(only 28, 30 actually)*/
                if(MAC_PORT_NUM_PER_TXQM+3*MAC_PORT_NUM_PER_QM <= macid)
                {
                    mac_tbl_id = 2*macid-28;
                }
                else
                {
                    mac_tbl_id = macid;
                }
                break;
            case SYS_DATAPATH_QM_MODE_1:
            case SYS_DATAPATH_QM_MODE_2:
            case SYS_DATAPATH_QM_MODE_3:
            default:
                mac_tbl_id = macid;
                break;
        }
    }

    /*macid 32~51*/
    else if(3*MAC_PORT_NUM_PER_TXQM+MAC_PORT_NUM_PER_QM > macid)
    {
        mac_tbl_id = macid+4;
    }

    /*macid 52~63*/
    else
    {
        switch(p_usw_datapath_master[lchip]->qm_choice.qmmode)
        {
            case SYS_DATAPATH_QM_MODE_0:
            case SYS_DATAPATH_QM_MODE_1:
            case SYS_DATAPATH_QM_MODE_4:
                mac_tbl_id = macid+4;
                break;
            case SYS_DATAPATH_QM_MODE_2:
                /*56~59*/
                if(3*MAC_PORT_NUM_PER_TXQM+2*MAC_PORT_NUM_PER_QM <= macid && 3*MAC_PORT_NUM_PER_TXQM+3*MAC_PORT_NUM_PER_QM > macid)
                {
                    mac_tbl_id = macid+12;
                }
                /*52~55, 60~63*/
                else
                {
                    mac_tbl_id = macid+4;
                }
                break;
            case SYS_DATAPATH_QM_MODE_3:
            default:
                /*52~59*/
                if(3*MAC_PORT_NUM_PER_TXQM+MAC_PORT_NUM_PER_QM <= macid && 3*MAC_PORT_NUM_PER_TXQM+3*MAC_PORT_NUM_PER_QM > macid)
                {
                    mac_tbl_id = macid+4;
                }
                /*60~63*/
                else
                {
                    mac_tbl_id = macid+8;
                }
                 /*mac_tbl_id = macid;*/
                break;
        }
    }

    return mac_tbl_id;
}

STATIC int32
_sys_tsingma_datapath_get_mode_level(uint8 mode, uint8* p_mode_level, uint8* p_port_num, uint8* p_speed)
{
    uint8 mode_level = 0;
    uint8 port_num = 0;
    uint8 speed = 0;

    switch(mode)
    {
        case CTC_CHIP_SERDES_QSGMII_MODE:
            mode_level = 0;
            port_num = 4;
            speed = CTC_PORT_SPEED_1G;
            break;
        case CTC_CHIP_SERDES_USXGMII1_MODE:
            mode_level = 0;
            port_num = 4;
            speed = CTC_PORT_SPEED_2G5;
            break;
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            mode_level = 0;
            port_num = 2;
            speed = CTC_PORT_SPEED_5G;
            break;
        case CTC_CHIP_SERDES_SGMII_MODE:
            mode_level = 1;
            port_num = 1;
            speed = CTC_PORT_SPEED_1G;
            break;
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            mode_level = 1;
            port_num = 1;
            speed = CTC_PORT_SPEED_2G5;
            break;
        case CTC_CHIP_SERDES_XFI_MODE:
        case CTC_CHIP_SERDES_USXGMII0_MODE:
            mode_level = 1;
            port_num = 1;
            speed = CTC_PORT_SPEED_10G;
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
            mode_level = 1;
            port_num = 1;
            speed = CTC_PORT_SPEED_25G;
            break;
        case CTC_CHIP_SERDES_100BASEFX_MODE:
            mode_level = 1;
            port_num = 1;
            speed = CTC_PORT_SPEED_100M;
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            mode_level = 2;
            port_num = 1;
            speed = CTC_PORT_SPEED_50G;
            break;
        case CTC_CHIP_SERDES_XAUI_MODE:
            mode_level = 3;
            port_num = 1;
            speed = CTC_PORT_SPEED_10G;
            break;
        case CTC_CHIP_SERDES_DXAUI_MODE:
            mode_level = 3;
            port_num = 1;
            speed = CTC_PORT_SPEED_20G;
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            mode_level = 3;
            port_num = 1;
            speed = CTC_PORT_SPEED_40G;
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            mode_level = 3;
            port_num = 1;
            speed = CTC_PORT_SPEED_100G;
            break;
        default:
            //SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mode = %d error! \n", mode);
            return CTC_E_INVALID_PARAM;
    }
    
    if(NULL != p_mode_level)
    {
        *p_mode_level = mode_level;
    }
    if(NULL != p_port_num)
    {
        *p_port_num = port_num;
    }
    if(NULL != p_speed)
    {
        *p_speed = speed;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_get_serdes_mapping(uint8 lchip, uint8 serdes_id, uint8 mode, uint16* p_lport,
                        uint8* p_mac, uint8* p_chan, uint8* p_speed, uint8* p_port_num, uint8* p_mac_tbl_id)
{
    uint8 mode_level = 0;
    uint8 mac_tmp = 0;
    uint8 port_num = 0;
    uint8 speed = 0;
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    
    if(CTC_E_NONE != _sys_tsingma_datapath_get_mode_level(mode, &mode_level, &port_num, &speed))
    {
        SYS_USW_VALID_PTR_WRITE(p_mac,        0);
        SYS_USW_VALID_PTR_WRITE(p_chan,       0);
        SYS_USW_VALID_PTR_WRITE(p_speed,      0);
        SYS_USW_VALID_PTR_WRITE(p_port_num,   0);
        SYS_USW_VALID_PTR_WRITE(p_lport,      0);
        SYS_USW_VALID_PTR_WRITE(p_mac_tbl_id, 0);
        return CTC_E_NONE;
    }
    
#if(SDK_WORK_PLATFORM == 1)
        mac_tmp     = serdes_id;
        SYS_USW_VALID_PTR_WRITE(p_mac,        mac_tmp);
        SYS_USW_VALID_PTR_WRITE(p_chan,       mac_tmp);
        SYS_USW_VALID_PTR_WRITE(p_speed,      speed);
        SYS_USW_VALID_PTR_WRITE(p_port_num,   1);
        SYS_USW_VALID_PTR_WRITE(p_lport,      mac_tmp);
        SYS_USW_VALID_PTR_WRITE(p_mac_tbl_id, mac_tmp);
        return CTC_E_NONE;
#endif

    if(TSINGMA_SERDES_NUM_PER_SLICE <= serdes_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " serdes_id = %d error! \n", serdes_id);
        return CTC_E_NONE;
    }
    
    mac_tmp = p_usw_datapath_master[lchip]->serdes_mac_map[mode_level][serdes_id];
    if(0xff == mac_tmp)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Para error! %d %d\n", mode, serdes_id);
        return CTC_E_NONE;
    }

    SYS_USW_VALID_PTR_WRITE(p_mac,        mac_tmp);
    SYS_USW_VALID_PTR_WRITE(p_chan,       mac_tmp);
    SYS_USW_VALID_PTR_WRITE(p_speed,      speed);
    SYS_USW_VALID_PTR_WRITE(p_port_num,   port_num);
    SYS_USW_VALID_PTR_WRITE(p_lport,      mac_tmp);
    SYS_USW_VALID_PTR_WRITE(p_mac_tbl_id, sys_tsingma_datapath_calc_mac_tblid_by_macid(lchip, mac_tmp));

    return CTC_E_NONE;
}

/*
Release Bclk Reset: cfg Hss12GGlbCfg.cfgHssBclkRstN = 1'b1 (active at low) 
Release Global Reset: cfg Hss12GGlbCfg.cfgHssGlbRstN  = 1'b1 (active at low) 
*/
int32
_sys_tsingma_datapath_hss12g_set_global_reset(uint8 lchip, uint8 hss_id, uint8 enable)
{
    uint32 cmd    = 0;
    uint32 tb_id  = Hss12GGlbCfg0_t + hss_id;
    uint32 value  = (TRUE == enable) ? 1 : 0;
    Hss12GGlbCfg0_m glb_cfg;


    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    
    DRV_IOW_FIELD(lchip, tb_id, Hss12GGlbCfg0_cfgHssBclkRstN_f, &value, &glb_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GGlbCfg0_cfgHssGlbRstN_f, &value, &glb_cfg);
    
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));


    return CTC_E_NONE;
}

/*
Release Per-lane Reset:
cfg Hss12GGlbCfg.cfgHssL#[0-7]RstN   = 1'b1(active at low) 
cfg Hss12GGlbCfg.cfgHssL#[0-7]RxRstN = 1'b1(active at low)
cfg Hss12GGlbCfg.cfgHssL#[0-7]TxRstN = 1'b1(active at low)
*/
int32
_sys_tsingma_datapath_hss12g_set_lane_reset(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint8  idx       = 0;
    uint8  step      = Hss12GGlbCfg0_cfgHssL1RstN_f - Hss12GGlbCfg0_cfgHssL0RstN_f;
    uint32 cmd       = 0;
    uint32 tb_id     = Hss12GGlbCfg0_t;
    uint32 value     = (TRUE == enable) ? 1 : 0;
    uint32 fld_id[3] = {Hss12GGlbCfg0_cfgHssL0RstN_f, Hss12GGlbCfg0_cfgHssL0RxRstN_f, Hss12GGlbCfg0_cfgHssL0TxRstN_f};
    Hss12GGlbCfg0_m glb_cfg;

    if(SYS_DATAPATH_SERDES_28G_START_ID <= serdes_id)
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_id  = serdes_id/HSS12G_LANE_NUM;
    lane_id = serdes_id%HSS12G_LANE_NUM;
    tb_id  += hss_id;

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    
    for(idx = 0; idx < 3; idx++)
    {
        fld_id[idx] += step*lane_id;
        DRV_IOW_FIELD(lchip, tb_id, fld_id[idx], &value, &glb_cfg);
    }
    
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));


    return CTC_E_NONE;
}

/*
Release Per-CMU Reset:
cfg Hss12GGlbCfg.cfgHssCmu[0-2]RstN = 1'b1(active at low) 
*/
int32
_sys_tsingma_datapath_hss12g_set_cmu_reset(uint8 lchip, uint8 hss_id, uint8 cmu_id, uint8 enable)
{
    uint8  step      = Hss12GGlbCfg0_cfgHssCmu1RstN_f - Hss12GGlbCfg0_cfgHssCmu0RstN_f;
    uint32 cmd       = 0;
    uint32 tb_id     = Hss12GGlbCfg0_t;
    uint32 value     = (TRUE == enable) ? 1 : 0;
    uint32 fld_id    = Hss12GGlbCfg0_cfgHssCmu0RstN_f;
    Hss12GGlbCfg0_m glb_cfg;


    if((HSS12G_NUM_PER_SLICE <= hss_id) || (3 <= cmu_id))
    {
        return CTC_E_INVALID_PARAM;
    }

    tb_id  += hss_id;
    fld_id += cmu_id*step;
    
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));

    
    return CTC_E_NONE;
}

/*
1. Power Off Serdes All lanes
   Hss12GGlbCfg.cfgHssPmaPowerCtrl0 = 25'h1ef_4e0c (default, power off)
2. delay 20us
3. Power On Serdes All lanes
   cfg Hss12GGlbCfg.cfgHssPmaPowerCtrl0 = 25'h010_b003 
*/
int32
_sys_tsingma_datapath_hss12g_set_power_reset(uint8 lchip, uint8 hss_id)
{
    uint32 tb_id  = 0;
    uint32 cmd    = 0;
    uint32 value  = 0;
    uint32 fld_id = Hss12GGlbCfg0_cfgHssPmaPowerCtrl0_f;
    Hss12GGlbCfg0_m glb_cfg;
    Hss12GCmuCfg0_m cmu_cfg;

    /*1. power off*/
    value = 0x01ef4e0c;
    tb_id = Hss12GGlbCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));

    /*2. delay*/
    sal_task_sleep(1);    

    /*3. power on*/
    value = 0x10b1f3;
    tb_id = Hss12GGlbCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));

    /*4. all CMU enable*/
    value = 1;
    tb_id = Hss12GCmuCfg0_t + hss_id;
    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cmu_cfg));
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu0HwtEnTxCkUp_f, &value, &cmu_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu0HwtEnTxCkDn_f, &value, &cmu_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu1HwtEnTxCkUp_f, &value, &cmu_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu1HwtEnTxCkDn_f, &value, &cmu_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu2HwtEnTxCkUp_f, &value, &cmu_cfg);
    DRV_IOW_FIELD(lchip, tb_id, Hss12GCmuCfg0_cfgHssCmu2HwtEnTxCkDn_f, &value, &cmu_cfg);
    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cmu_cfg));

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_hss12g_wait_lane_ok(uint8 lchip, uint8 hss_id, uint8 lane_bitmap)
{
    uint8  lane_id  = 0;
    uint8  stat_bmp = 0;
    uint8  step     = Hss12GLaneMon0_monHssL1PmaRstDone_f - Hss12GLaneMon0_monHssL0PmaRstDone_f;
    uint32 cmd      = 0;
    uint32 time     = 0;
    uint32 max_time = 0x5;
    uint32 tb_id    = Hss12GLaneMon0_t;
    uint32 value    = 0;
    uint32 fld_id[HSS12G_LANE_NUM] = {0};
    Hss12GLaneMon0_m hss_mon;

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }
    
    tb_id  += hss_id;
    for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
    {
        fld_id[lane_id] = Hss12GLaneMon0_monHssL0PmaRstDone_f + step*lane_id;
    }

    for(time = 0; time < max_time; time++)
    {
        sal_task_sleep(10);
        
        stat_bmp = 0;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_mon));

        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            DRV_IOR_FIELD(lchip, tb_id, fld_id[lane_id], &value, &hss_mon);
            stat_bmp |= (uint8)((value & 0x1) << lane_id);
        }
        
        if(CTC_FLAG_ISSET(stat_bmp, lane_bitmap))
        {
            return CTC_E_NONE;
        }
    }
    
    
    return CTC_E_HW_TIME_OUT;
}


int32
sys_tsingma_datapath_check_mode_valid(uint8 lchip, uint8 serdes_id, uint8 mode)
{
    /*HSS12G*/
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        if ((mode == CTC_CHIP_SERDES_XXVG_MODE) || (mode == CTC_CHIP_SERDES_LG_MODE) ||
            (mode == CTC_CHIP_SERDES_CG_MODE))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
    }
    /*HSS28G*/
    else
    {
        if ((mode == CTC_CHIP_SERDES_USXGMII0_MODE) || (mode == CTC_CHIP_SERDES_USXGMII1_MODE) ||
            (mode == CTC_CHIP_SERDES_USXGMII2_MODE) || (mode == CTC_CHIP_SERDES_QSGMII_MODE) || 
            (mode == CTC_CHIP_SERDES_100BASEFX_MODE))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
    }
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_init_serdes_mac_map(uint8 lchip, uint8 lane_swap_0, uint8 lane_swap_1)
{
    uint8 original_serdes_mac_map[4][32] = {
    /*                  Serdes 0~7                                  Serdes 8~15                                  Serdes 16~23                                 Serdes 24~31*/
    /*port n, lane 1*/{0,4,16,20,8,32,36,40,                      24,48,52,56,0xff,0xff,0xff,0xff,          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
    /*port 1, lane 1*/{0,1,2,3,8,9,10,11,                         20,21,22,23,12,13,14,15,                   24,25,26,27,28,29,30,31,                   44,45,46,47,60,61,62,63},
    /*port 1, lane 2*/{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 44,44,46,46,60,60,62,62},
    /*port 1, lane 4*/{0,0,0,0,8,8,8,8,                            20,20,20,20,12,12,12,12,                  24,24,24,24,28,28,28,28,                   44,44,44,44,60,60,60,60}
    };
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    if(TRUE == lane_swap_0)
    {
        original_serdes_mac_map[1][24] = 46;
        original_serdes_mac_map[1][26] = 44;
        original_serdes_mac_map[2][24] = 46;
        original_serdes_mac_map[2][25] = 44;
        original_serdes_mac_map[2][26] = 44;
        original_serdes_mac_map[2][27] = 46;
    }

    if(TRUE == lane_swap_1)
    {
        original_serdes_mac_map[1][28] = 62;
        original_serdes_mac_map[1][30] = 60;
        original_serdes_mac_map[2][28] = 62;
        original_serdes_mac_map[2][29] = 60;
        original_serdes_mac_map[2][30] = 60;
        original_serdes_mac_map[2][31] = 62;
    }

    sal_memcpy(p_usw_datapath_master[lchip]->serdes_mac_map, original_serdes_mac_map, 4*32*sizeof(uint8));
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_init_mii_lport_map(uint8 lchip)
{
    uint16 original_mii_lport_map[4][32] = {
    /*            Intr mii id    0-31 */
    /* HSS12G0 */{0,1,2,3,     4,5,6,7,     16,17,18,19,    20,21,22,23,     8,9,10,11,    32,33,34,35,    36,37,38,39,    40,41,42,43},
    /* HSS12G1 */{20,21,22,23, 48,49,50,51, 52,53,54,55,    56,57,58,59,     12,13,14,15, },
    /* HSS12G2 */{24,25,26,27, 28,29,30,31, },
    /* HSS28G  */{44,45,46,47, 60,61,62,63, }
    };
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    sal_memcpy(p_usw_datapath_master[lchip]->mii_lport_map, original_mii_lport_map, 4*32*sizeof(uint16));
    return CTC_E_NONE;
}

/*Modify global serdes-mac map, according to the config of TXQM1, TXQM3 and Mux.
HARD CODE. DO NOT ASK WHY!*/
int32
_sys_tsingma_datapath_modify_serdes_mac_map(uint8 lchip, uint32 txqm1mode, uint32 txqm3mode, uint32 muxmode, uint8 lane_swap_0, uint8 lane_swap_1)
{
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    p_usw_datapath_master[lchip]->qm_choice.txqm1   = txqm1mode;
    p_usw_datapath_master[lchip]->qm_choice.txqm3   = txqm3mode;
    p_usw_datapath_master[lchip]->qm_choice.muxmode = muxmode;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_serdes_mac_map(lchip, lane_swap_0, lane_swap_1));

    if(0 == muxmode)  /*MUX: 8    TXQM3: 13 14 15 16*/
    {
        if(1 == txqm1mode)  /*TXQM1: 4 5 6 7*/
        {
            p_usw_datapath_master[lchip]->serdes_mac_map[1][8] = 24;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][9] = 25;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][10] = 26;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][11] = 27;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][8]), 24, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->serdes_mac_map[1][16] = 28;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][17] = 29;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][18] = 30;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][19] = 31;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][16]), 28, 4*sizeof(uint8));
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[1][20]), 0xff, 4*sizeof(uint8));
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][20]), 0xff, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->qm_choice.qmmode = SYS_DATAPATH_QM_MODE_1;
        }
        else if(0 == txqm1mode)  /*TXQM1: 4 6 7 8*/
        {
            p_usw_datapath_master[lchip]->serdes_mac_map[0][3] = 0xff;
            p_usw_datapath_master[lchip]->serdes_mac_map[0][8] = 20;
            p_usw_datapath_master[lchip]->qm_choice.qmmode = SYS_DATAPATH_QM_MODE_0;
        }
        else  /*TXQM1: 4 5 6 7/8*/
        {
            p_usw_datapath_master[lchip]->serdes_mac_map[1][8] = 24;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][9] = 25;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][10] = 26;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][11] = 27;

            p_usw_datapath_master[lchip]->serdes_mac_map[1][16] = 28;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[1][17]), 0xff, 3*sizeof(uint8));
            p_usw_datapath_master[lchip]->serdes_mac_map[1][20] = 30;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[1][21]), 0xff, 3*sizeof(uint8));

            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][16]), 28, 4*sizeof(uint8));
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][20]), 30, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->serdes_mac_map[3][8] = 24;
            p_usw_datapath_master[lchip]->serdes_mac_map[3][9] = 24;
            p_usw_datapath_master[lchip]->serdes_mac_map[3][10] = 24;
            p_usw_datapath_master[lchip]->serdes_mac_map[3][11] = 24;
            p_usw_datapath_master[lchip]->qm_choice.qmmode = SYS_DATAPATH_QM_MODE_4;
        }
    }
    else  /*MUX: 17    TXQM1: 4 5 6 7*/
    {
        p_usw_datapath_master[lchip]->serdes_mac_map[1][8] = 24;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][9] = 25;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][10] = 26;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][11] = 27;
        sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][8]), 24, 4*sizeof(uint8));
        p_usw_datapath_master[lchip]->serdes_mac_map[1][16] = 28;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][17] = 29;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][18] = 30;
        p_usw_datapath_master[lchip]->serdes_mac_map[1][19] = 31;
        sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][16]), 28, 4*sizeof(uint8));
        if(2 == txqm3mode)  /*TXQM3: 13 14 15 17*/
        {
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[1][28]), 0xff, 4*sizeof(uint8));
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[2][28]), 0xff, 4*sizeof(uint8));
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][28]), 0xff, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->serdes_mac_map[1][20] = 60;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][21] = 61;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][22] = 62;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][23] = 63;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][20]), 60, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->qm_choice.qmmode = SYS_DATAPATH_QM_MODE_3;
        }
        else if(1 == txqm3mode)  /*TXQM3: 13 14 17 16*/
        {
            p_usw_datapath_master[lchip]->serdes_mac_map[0][11] = 0xff;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][20] = 56;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][21] = 57;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][22] = 58;
            p_usw_datapath_master[lchip]->serdes_mac_map[1][23] = 59;
            sal_memset(&(p_usw_datapath_master[lchip]->serdes_mac_map[3][20]), 56, 4*sizeof(uint8));
            p_usw_datapath_master[lchip]->qm_choice.qmmode = SYS_DATAPATH_QM_MODE_2;
        }
        else  /*TXQM3: 13 14 15 16 (impossible)*/
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"_sys_tsingma_datapath_set_serdes_mac_map: Illegal qm config! txqm1mode %d, txqm3mode %d, muxmode %d\n", txqm1mode, txqm3mode, muxmode);
            return CTC_E_NOT_SUPPORT;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_modify_mii_lport_map(uint8 lchip, uint32 txqm1mode, uint32 txqm3mode, uint32 muxmode)
{
    uint8 port_idx;
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    
    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_mii_lport_map(lchip));
    
    /*1. fill up blank by 0xffff*/
    for(port_idx = 20; port_idx < 32; port_idx++)
    {
        p_usw_datapath_master[lchip]->mii_lport_map[1][port_idx] = 0xffff;
    }
    for(port_idx = 8; port_idx < 32; port_idx++)
    {
        p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 0xffff;
    }
    for(port_idx = 8; port_idx < 32; port_idx++)
    {
        p_usw_datapath_master[lchip]->mii_lport_map[3][port_idx] = 0xffff;
    }

    /*2. fix mii_lport_map based on txqm1 choice*/
    if(0 == txqm1mode)
    {
        for(port_idx = 12; port_idx < 16; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[0][port_idx] = 0xffff;
        }
        for(port_idx = 0; port_idx < 4; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[1][port_idx] = 20 + port_idx;
        }
    }
    else if(1 == txqm1mode)
    {
        for(port_idx = 0; port_idx < 4; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[1][port_idx] = 24 + port_idx;
        }
        for(port_idx = 0; port_idx < 4; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 28 + port_idx;
        }
        for(port_idx = 4; port_idx < 8; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 0xffff;
        }
    }
    else
    {
        for(port_idx = 0; port_idx < 4; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[1][port_idx] = 24 + port_idx;
        }
        for(port_idx = 0; port_idx < 2; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 28 + port_idx;
        }
        for(port_idx = 2; port_idx < 4; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 0xffff;
        }
        for(port_idx = 4; port_idx < 6; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 26 + port_idx;
        }
        for(port_idx = 6; port_idx < 8; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 0xffff;
        }
    }

    /*3. fix mii_lport_map based on txqm3 choice*/
    if(1 == txqm3mode)
    {
        for(port_idx = 12; port_idx < 16; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[1][port_idx] = 0xffff;
        }
        if(1 == muxmode)
        {
            for(port_idx = 4; port_idx < 8; port_idx++)
            {
                p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 52 + port_idx;
            }
        }
    }
    else if(2 == txqm3mode)
    {
        if(1 == muxmode)
        {
            for(port_idx = 4; port_idx < 8; port_idx++)
            {
                p_usw_datapath_master[lchip]->mii_lport_map[2][port_idx] = 56 + port_idx;
            }
        }
        for(port_idx = 4; port_idx < 8; port_idx++)
        {
            p_usw_datapath_master[lchip]->mii_lport_map[3][port_idx] = 0xffff;
        }
    }
    return CTC_E_NONE;
}

/* Select TXQM1, TXQM3 and MUX. Just save result in global, no table operations.
Table operations take place in function sys_tsingma_mac_set_txqm_mode_config. */
int32
_sys_tsingma_datapath_select_qm(uint8 lchip, ctc_datapath_serdes_prop_t *serdes)
{
    uint8  condition1 = 1;  /*true: serdes 28~31 is NA*/
    uint8  condition2 = 0;  /*true: serdes 11 is QSGMII/USXGMII-multi*/
    uint8  condition3 = 1;  /*true: serdes 20~23 is NA*/
    uint8  condition4 = 1;  /*true: serdes 16~23 is 40G/XAUI/DXAUI, or 16 & 20 are 1-1 mode*/
    uint8  condition5 = 0;  /*true: serdes 3 is QSGMII/USXGMII-multi*/
    uint8  condition6 = 1;  /*true: serdes 16~23 do not support dynamic switch*/
    uint8  serdes_id  = 0;
    uint32 txqm1mode  = 0;
    uint32 txqm3mode  = 0;
    uint32 muxmode    = 0;
    uint8  lane_swap_0 = 0;
    uint8  lane_swap_1 = 0;
    uint8  cond_temp0 = 1;  /*true: 16~19 is xfi|none|none|none*/
    uint8  cond_temp1 = 1;  /*true: 20~23 is xfi|none|none|none*/

    /*judge 16~19 is xfi|none|none|none*/
    if((CTC_CHIP_SERDES_XFI_MODE == serdes[16].mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == serdes[16].mode) || 
       (CTC_CHIP_SERDES_SGMII_MODE == serdes[16].mode) || (CTC_CHIP_SERDES_100BASEFX_MODE == serdes[16].mode))
    {
        for(serdes_id = 17; serdes_id <= 19; serdes_id++)
        {
            if((CTC_CHIP_SERDES_NONE_MODE != serdes[serdes_id].mode) && (CTC_CHIP_SERDES_XSGMII_MODE != serdes[serdes_id].mode))
            {
                cond_temp0 = 0;
                break;
            }
        }
    }
    /*judge 20~23 is xfi|none|none|none*/
    if((CTC_CHIP_SERDES_XFI_MODE == serdes[20].mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == serdes[20].mode) || 
       (CTC_CHIP_SERDES_SGMII_MODE == serdes[20].mode) || (CTC_CHIP_SERDES_100BASEFX_MODE == serdes[20].mode))
    {
        for(serdes_id = 21; serdes_id <= 23; serdes_id++)
        {
            if((CTC_CHIP_SERDES_NONE_MODE != serdes[serdes_id].mode) && (CTC_CHIP_SERDES_XSGMII_MODE != serdes[serdes_id].mode))
            {
                cond_temp1 = 0;
                break;
            }
        }
    }

    /*global serdes config info collection*/
    for (serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        if((3 == serdes_id) &&
           (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(serdes[serdes_id].mode)))
        {
            condition5 = 1;
        }
        if((11 == serdes_id) &&
           (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(serdes[serdes_id].mode)))
        {
            condition2 = 1;
        }
        if(condition3 &&
           ((20 <= serdes_id) && (23 >= serdes_id)) &&
           (CTC_CHIP_SERDES_NONE_MODE != serdes[serdes_id].mode) &&
           (CTC_CHIP_SERDES_XSGMII_MODE != serdes[serdes_id].mode))
        {
            condition3 = 0;
        }
        if(condition4 &&
           ((16 <= serdes_id) && (23 >= serdes_id)) &&
           ((CTC_CHIP_SERDES_XLG_MODE != serdes[serdes_id].mode) && 
            (CTC_CHIP_SERDES_XAUI_MODE != serdes[serdes_id].mode) && 
            (CTC_CHIP_SERDES_DXAUI_MODE != serdes[serdes_id].mode)))
        {
            if(((16 <= serdes_id) && (19 >= serdes_id) && (!cond_temp0)) || 
               ((20 <= serdes_id) && (23 >= serdes_id) && (!cond_temp1)))
            {
                condition4 = 0;
            }
        }
        if(condition6 &&
           ((16 <= serdes_id) && (23 >= serdes_id)) &&
           (0 != serdes[serdes_id].is_dynamic))
        {
            condition6 = 0;
        }
        if(condition1 &&
           (28 <= serdes_id) &&
           (CTC_CHIP_SERDES_NONE_MODE != serdes[serdes_id].mode) &&
           (CTC_CHIP_SERDES_XSGMII_MODE != serdes[serdes_id].mode))
        {
            condition1 = 0;
        }
    }

     /*20171205 temp stub*/
     /*condition2 = 0;*/

    /* judgement for table values */
    /* 1. txqm1mode */
    if(condition5)
    {
        if(condition4 && ((condition2 && (!condition1)) || condition6))
        {
            txqm1mode = 2;  /*TXQM1: 4 5 6 7/8*/
        }
        else
        {
            txqm1mode = 1;  /*TXQM1: 4 5 6 7*/
        }
    }
    else
    {
        txqm1mode = 0;  /*TXQM1: 4 6 7 8*/
    }
    /* 2. txqm3mode */
    txqm3mode = 0;  /*TXQM3: 13 14 15 16*/
    if(!condition3 && 1 == txqm1mode)
    {
        if(!condition1 && !condition2)
        {
            txqm3mode = 1;  /*TXQM3: 13 14 17 16*/
        }
        else if(condition1)
        {
            txqm3mode = 2;  /*TXQM3: 13 14 15 17*/
        }
    }
    /* 3. muxmode */
    if(0 == txqm3mode)
    {
        muxmode = 0;  /*MUX: QM8*/
    }
    else
    {
        muxmode = 1;  /*MUX: QM17*/
    }

    /*if QM 8 and 17 are both disabled, and serdes 20~23 mode is valid, return error*/
    if((1 == txqm1mode) && (0 == txqm3mode))
    {
        for(serdes_id = 20; serdes_id < 24; serdes_id++)
        {
            if ((CTC_CHIP_SERDES_NONE_MODE != serdes[serdes_id].mode)
                && (CTC_CHIP_SERDES_XSGMII_MODE != serdes[serdes_id].mode))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "%s: QM choice conflict!\n", __FUNCTION__);
                return CTC_E_INVALID_CONFIG;
            }
        }
    }

    /* 4. qm force config operation */
    if(condition1)
    {
        g_qm_force_cfg = 4;
    }
    if(condition3)
    {
        g_qm_force_cfg = 3;
    }
    if(2 != txqm1mode)
    {
        if(2 == g_qm_force_cfg)
        {
            if(condition2 && (!condition1))
            {
                txqm3mode = 2;  /*TXQM3: 13 14 15 17*/
            }
            else if((!condition2) && condition1)
            {
                txqm3mode = 1;  /*TXQM3: 13 14 17 16*/
            }
            
            if(0 == txqm1mode)
            {
                txqm1mode = 1;  /*TXQM1: 4 5 6 7*/
            }
            
            muxmode = 1;    /*MUX: QM17*/
        }
        else if(3 == g_qm_force_cfg)
        {
            txqm1mode = 1;  /*TXQM1: 4 5 6 7*/
            txqm3mode = 0;  /*TXQM3: 13 14 15 16*/
            muxmode = 0;    /*MUX: QM8  set as default*/
        }
        else if(4 == g_qm_force_cfg)
        {
            txqm1mode = 1;  /*TXQM1: 4 5 6 7*/
            txqm3mode = 2;  /*TXQM3: 13 14 15 17*/
            muxmode = 1;    /*MUX: QM17*/
        }
    }

    /*modify serdes-mac map*/
    if ((1 == serdes[24].group) && (0 == serdes[25].group) && (0 == serdes[26].group) && (1 == serdes[27].group))
    {
        lane_swap_0 = 1;
    }
    if ((1 == serdes[28].group) && (0 == serdes[29].group) && (0 == serdes[30].group) && (1 == serdes[31].group))
    {
        lane_swap_1 = 1;
    }
    CTC_ERROR_RETURN(_sys_tsingma_datapath_modify_serdes_mac_map(lchip, txqm1mode, txqm3mode, muxmode, lane_swap_0, lane_swap_1));

    CTC_ERROR_RETURN(_sys_tsingma_datapath_modify_mii_lport_map(lchip, txqm1mode, txqm3mode, muxmode));

    return CTC_E_NONE;
}

int32 
sys_tsingma_datapath_read_hss12g_reg(uint8 lchip, uint8 hss_id, uint8 acc_id, uint8 addr, uint8 mask, uint8* p_data)
{
    int32  ret = 0;
    uint8  raw_data = 0;
    uint16 raw_data_u16 = 0;
    uint8  mask_op = ~mask;
    uint8  bit;
    uint32 fix_addr = addr;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"hss_id %u, acc_id %u, addr 0x%02x, mask 0x%02x\n", hss_id, acc_id, addr, mask);

    CTC_PTR_VALID_CHECK(p_data);
    if((HSS12G_NUM_PER_SLICE <= hss_id) || (SYS_DATAPATH_HSS12G_ACC_ID_MAX < acc_id) || (0xff == mask))
    {
        return CTC_E_INVALID_PARAM;
    }

    fix_addr |= (((uint32)acc_id) << 8);
    ret = drv_chip_read_hss15g(lchip, hss_id, fix_addr, &raw_data_u16);
    if(DRV_E_NONE != ret)
    {
        return CTC_E_INVALID_PARAM;
    }
    raw_data = (uint8)raw_data_u16;
    raw_data &= mask_op;
    for(bit = 0; 0 == ((mask_op >> bit) & 0x1); bit++);

    *p_data = raw_data >> bit;

    return CTC_E_NONE;
}

/*
acc_id:
4'd[0-2]:  access select CMU[0-2];
4'd[3-10]: access select Lane[0-7];
*/
int32 
sys_tsingma_datapath_write_hss12g_reg(uint8 lchip, uint8 hss_id, uint8 acc_id, uint8 addr, uint8 mask, uint8 data)
{
    int32  ret = 0;
    uint8  raw_data = 0;
    uint16 raw_data_u16 = 0;
    uint8  fix_data = 0;
    uint8  mask_op = ~mask;
    uint8  bit;
    uint32 fix_addr = addr;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"hss_id %u, acc_id %u, addr 0x%02x, mask 0x%02x, data 0x%02x\n", hss_id, acc_id, addr, mask, data);

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return CTC_E_NONE;
    }

    if((HSS12G_NUM_PER_SLICE <= hss_id) || (SYS_DATAPATH_HSS12G_ACC_ID_MAX < acc_id) || (0xff == mask))
    {
        return CTC_E_INVALID_PARAM;
    }
    for(bit = 0; 0 == ((mask_op >> bit) & 0x1); bit++);
    fix_data = data << bit;

    fix_addr |= (((uint32)acc_id) << 8);
    ret = drv_chip_read_hss15g(lchip, hss_id, fix_addr, &raw_data_u16);
    if(DRV_E_NONE != ret)
    {
        return CTC_E_INVALID_PARAM;
    }
    raw_data = (uint8)raw_data_u16;
    fix_data |= (raw_data & mask);

    ret = drv_chip_write_hss15g(lchip, hss_id, fix_addr, (uint16)fix_data);
    if(DRV_E_NONE != ret)
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32 
sys_tsingma_datapath_read_hss28g_reg(uint8 lchip, uint8 acc_id, uint16 addr, uint16 mask, uint16* p_data)
{
    uint16 raw_data = 0;
    uint16 mask_op = ~mask;
    uint8  bit;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"acc_id %u, addr 0x%04x, mask 0x%04x\n", acc_id, addr, mask);

    CTC_PTR_VALID_CHECK(p_data);
    
    if (SYS_DATAPATH_HSS28G_ACC_ID_MAX < acc_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"%s@%d: HSS28G acc_id CANNOT be %d\n", __FUNCTION__, __LINE__, acc_id);
        return CTC_E_INVALID_PARAM;
    }
    if (0xffff == mask)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"%s@%d: HSS28G read mask CANNOT be all 1\n", __FUNCTION__, __LINE__);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, acc_id, (uint32)addr, &raw_data));

    raw_data &= mask_op;
    for(bit = 0; 0 == ((mask_op >> bit) & 0x1); bit++);

    *p_data = raw_data >> bit;

    return CTC_E_NONE;
}

/*
acc_id:
1'b1: select HSS28G1
1'b0: select HSS28G0
*/
int32 
sys_tsingma_datapath_write_hss28g_reg(uint8 lchip, uint8 acc_id, uint16 addr, uint16 mask, uint16 data)
{
    uint16 raw_data = 0;
    uint16 fix_data = 0;
    uint16 mask_op = ~mask;
    uint8  bit;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"acc_id %u, addr 0x%04x, mask 0x%04x, data 0x%x\n", acc_id, addr, mask, data);

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return CTC_E_NONE;
    }

    if (SYS_DATAPATH_HSS28G_ACC_ID_MAX < acc_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"%s@%d: HSS28G acc_id CANNOT be %d\n", __FUNCTION__, __LINE__, acc_id);
        return CTC_E_INVALID_PARAM;
    }
    if (0xffff == mask)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"%s@%d: HSS28G write mask CANNOT be all 1\n", __FUNCTION__, __LINE__);
        return CTC_E_INVALID_PARAM;
    }
    for(bit = 0; 0 == ((mask_op >> bit) & 0x1); bit++);
    fix_data = data << bit;
    CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, acc_id, (uint32)addr, &raw_data));
    
    fix_data |= (raw_data & mask);
    CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, acc_id, (uint32)addr, fix_data));

    return CTC_E_NONE;
}


/*
0x980d[11:0] DOMAIN_RESET
There are 4 kind of reset. DOMAIN_RESET must be written back to 0 after any kind of reset happens.
*/
int32
_sys_tsingma_datapath_hss28g_set_domain_reset(uint8 lchip, uint8 hss_id, uint8 rst_type)
{
                       /*LOGIC_RESET  ALL_RESET  REGISTER_RESET  CPU_RESET*/
    uint16 rst_val[4] = {0x0777,      0x0888,    0x0999,         0x0aaa};

    if(SYS_TM_28G_CPU_RESET < rst_type)
    {
        return CTC_E_INVALID_PARAM;
    }
    
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, 0x980d, 0xf000, rst_val[rst_type]));
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, 0x980d, 0xf000, 0));
    
    return CTC_E_NONE;
}


int32 
_sys_tsingma_datapath_12g_load_cmu_val(uint8 mode, uint8 tbl_type, uint32* p_val, uint8 overclk_flag)
{
    uint8 cmu_cfg[CTC_CHIP_MAX_SERDES_MODE][3] = {
    //cfgHssCmu#[0-2]HwtCfgDivSel, cfgHssCmu#[0-2]HwtVcoDivSel, cfgHssCmu#[0-2]HwtMultiLaneMode
        {0xff,                       0xff,                       0xff},    //CTC_CHIP_SERDES_NONE_MODE
        {0x01,                       0x00,                       0x00},    //CTC_CHIP_SERDES_XFI_MODE
        {0x00,                       0x00,                       0x00},    //CTC_CHIP_SERDES_SGMII_MODE
        {0xff,                       0xff,                       0xff},    //CTC_CHIP_SERDES_XSGMII_MODE
        {0x00,                       0x00,                       0x00},    //CTC_CHIP_SERDES_QSGMII_MODE
        {0x02,                       0x00,                       0x00},    //CTC_CHIP_SERDES_XAUI_MODE
        {0x02,                       0x00,                       0x00},    //CTC_CHIP_SERDES_DXAUI_MODE
        {0x01,                       0x00,                       0x00},    //CTC_CHIP_SERDES_XLG_MODE
        {0xff,                       0xff,                       0xff},    //CTC_CHIP_SERDES_CG_MODE
        {0x02,                       0x00,                       0x00},    //CTC_CHIP_SERDES_2DOT5G_MODE
        {0x01,                       0x00,                       0x00},    //CTC_CHIP_SERDES_USXGMII0_MODE
        {0x01,                       0x00,                       0x00},    //CTC_CHIP_SERDES_USXGMII1_MODE
        {0x01,                       0x00,                       0x00},    //CTC_CHIP_SERDES_USXGMII2_MODE
        {0xff,                       0xff,                       0xff},    //CTC_CHIP_SERDES_XXVG_MODE
        {0xff,                       0xff,                       0xff},    //CTC_CHIP_SERDES_LG_MODE
        {0x00,                       0x00,                       0x00}     //CTC_CHIP_SERDES_100BASEFX_MODE
    };
    uint8 cmu_cfg_oc[3] = {
    //cfgHssCmu#[0-2]HwtCfgDivSel, cfgHssCmu#[0-2]HwtVcoDivSel, cfgHssCmu#[0-2]HwtMultiLaneMode
         0x02,                       0x00,                       0x00      //12.5G per lane
    };

    if((CTC_CHIP_MAX_SERDES_MODE <= mode) || (3 <= tbl_type) || (0xff == cmu_cfg[mode][tbl_type]))
    {
        return CTC_E_INVALID_PARAM;
    }
    
    if(1 == overclk_flag)
    {
        sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XFI_MODE], cmu_cfg_oc, 3*sizeof(uint8));
        sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XLG_MODE], cmu_cfg_oc, 3*sizeof(uint8));
    }
    
    *p_val = cmu_cfg[mode][tbl_type];
    return CTC_E_NONE;
}

int32 
_sys_tsingma_datapath_28g_load_cmu_val(uint8 mode, uint8 tbl_type, uint16* p_val, uint16* p_mask, uint16* p_addr, uint8 overclk_flag)
{
    uint16 cmu_cfg[CTC_CHIP_MAX_SERDES_MODE][6] = {
    //REFCLK_SRC_SEL    REFCLK_DIV4_EN    PLL_RX_N    RX_PLL_VCO_RANGE    PLL_TX_N    TX_PLL_VCO_RANGE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_NONE_MODE
        {0x0000,        0x0000,           0x0021,     0x0005,             0x0021,     0x0005 },    //CTC_CHIP_SERDES_XFI_MODE
        {0x0000,        0x0000,           0x0020,     0x0005,             0x0020,     0x0005 },    //CTC_CHIP_SERDES_SGMII_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_XSGMII_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_QSGMII_MODE
        {0x0000,        0x0000,           0x0028,     0x0002,             0x0028,     0x0002 },    //CTC_CHIP_SERDES_XAUI_MODE
        {0x0000,        0x0000,           0x0028,     0x0002,             0x0028,     0x0002 },    //CTC_CHIP_SERDES_DXAUI_MODE
        {0x0000,        0x0000,           0x0021,     0x0005,             0x0021,     0x0005 },    //CTC_CHIP_SERDES_XLG_MODE
        {0x0000,        0x0001,           0x00a5,     0x0002,             0x00a5,     0x0002 },    //CTC_CHIP_SERDES_CG_MODE
        {0x0000,        0x0000,           0x0028,     0x0002,             0x0028,     0x0002 },    //CTC_CHIP_SERDES_2DOT5G_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_USXGMII0_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_USXGMII1_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff },    //CTC_CHIP_SERDES_USXGMII2_MODE
        {0x0000,        0x0001,           0x00a5,     0x0002,             0x00a5,     0x0002 },    //CTC_CHIP_SERDES_XXVG_MODE
        {0x0000,        0x0001,           0x00a5,     0x0002,             0x00a5,     0x0002 },    //CTC_CHIP_SERDES_LG_MODE
        {0xffff,        0xffff,           0xffff,     0xffff,             0xffff,     0xffff }     //CTC_CHIP_SERDES_100BASEFX_MODE
    };
    uint16 cmu_cfg_oc[3][6] = {
    //REFCLK_SRC_SEL    REFCLK_DIV4_EN    PLL_RX_N    RX_PLL_VCO_RANGE    PLL_TX_N    TX_PLL_VCO_RANGE
        {0x0000,        0x0001,           0x00b4,     0x0000,             0x00b4,     0x0000 },    //28.125G per lane
        {0x0000,        0x0001,           0x00c0,     0x0000,             0x00c0,     0x0000 },    //11.40625G per lane (value is incorrect, just for occupy space!)
        {0x0000,        0x0001,           0x00c0,     0x0000,             0x00c0,     0x0000 }     //12.5G per lane (value is incorrect, just for occupy space!)
    };
    uint16 mask[6] = {0xfffd, 0xffbf, 0x00ff, 0x1fff, 0x00ff, 0x1fff};
    uint16 addr[6] = {0x84fb, 0x84ff, 0x84fe, 0x84fd, 0x84f5, 0x84f4};
    
    if((CTC_CHIP_MAX_SERDES_MODE <= mode) || (6 <= tbl_type) || (0xff == cmu_cfg[mode][tbl_type]))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch(overclk_flag)
    {
        case 1:
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XFI_MODE],  cmu_cfg_oc[2], 6*sizeof(uint16));
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XLG_MODE],  cmu_cfg_oc[2], 6*sizeof(uint16));
            break;
        case 3:
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XFI_MODE],  cmu_cfg_oc[1], 6*sizeof(uint16));
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XLG_MODE],  cmu_cfg_oc[1], 6*sizeof(uint16));
            break;
        case 4:
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_XXVG_MODE], cmu_cfg_oc[0], 6*sizeof(uint16));
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_LG_MODE],   cmu_cfg_oc[0], 6*sizeof(uint16));
            sal_memcpy(cmu_cfg[CTC_CHIP_SERDES_CG_MODE],   cmu_cfg_oc[0], 6*sizeof(uint16));
        default:
            break;
    }

    if(p_val)
    {
        *p_val  = cmu_cfg[mode][tbl_type];
    }
    if(p_mask)
    {
        *p_mask = mask[tbl_type];
    }
    if(p_addr)
    {
        *p_addr = addr[tbl_type];
    }
    return CTC_E_NONE;
}

int32 
sys_tsingma_datapath_12g_is_pll_equal(uint8 mode_1, uint8 mode_2, uint8 ovclk_flag_1, uint8 ovclk_flag_2, uint8* p_flag)
{
    uint32 value_1 = 0;
    uint32 value_2 = 0;
    uint8  tbl_type;

    CTC_PTR_VALID_CHECK(p_flag);

    /*NONE mode pll equals to any other mode*/
    if((CTC_CHIP_SERDES_NONE_MODE == mode_1) || (CTC_CHIP_SERDES_NONE_MODE == mode_2))
    {
        *p_flag = TRUE;
        return CTC_E_NONE;
    }

    for(tbl_type = 0; tbl_type < 3; tbl_type++)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_load_cmu_val(mode_1, tbl_type, &value_1, ovclk_flag_1));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_load_cmu_val(mode_2, tbl_type, &value_2, ovclk_flag_2));
        if(value_1 != value_2)
        {
            *p_flag = FALSE;
            return CTC_E_NONE;
        }
    }
    
    *p_flag = TRUE;
    return CTC_E_NONE;
}

int32 
sys_tsingma_datapath_28g_is_pll_equal(uint8 mode_1, uint8 mode_2, uint8 ovclk_flag_1, uint8 ovclk_flag_2, uint8* p_flag)
{
    uint16 value_1 = 0;
    uint16 value_2 = 0;
    uint8  tbl_type;

    CTC_PTR_VALID_CHECK(p_flag);

    /*NONE mode pll equals to any other mode*/
    if((CTC_CHIP_SERDES_NONE_MODE == mode_1) || (CTC_CHIP_SERDES_NONE_MODE == mode_2) || 
       (CTC_CHIP_SERDES_XSGMII_MODE == mode_1) || (CTC_CHIP_SERDES_XSGMII_MODE == mode_2))
    {
        *p_flag = TRUE;
        return CTC_E_NONE;
    }

    for(tbl_type = 0; tbl_type < 6; tbl_type++)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_28g_load_cmu_val(mode_1, tbl_type, &value_1, NULL, NULL, ovclk_flag_1));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_28g_load_cmu_val(mode_2, tbl_type, &value_2, NULL, NULL, ovclk_flag_2));
        if(value_1 != value_2)
        {
            *p_flag = FALSE;
            return CTC_E_NONE;
        }
    }
    
    *p_flag = TRUE;
    return CTC_E_NONE;
}

/*
hss12g:
+---+--------+---+--------+---+
|CMU| serdes |CMU| serdes |CMU|
| 0 |  0~3   | 1 |  4~7   | 2 |
+---+--------+---+--------+---+
Every hss(8 serdes) has 3 CMU. For each quad serdes, the available CMUs are as follows:
serdes 0~3 can be allocated in CMU 0 or 1;
serdes 4~7 can be allocated in CMU 2 or 1.
Each quad serdes can have maximum 2 kinds of pll, and octal serdes have maximum 3 in total.

hss28g:
+--------+---+--------+ +--------+---+--------+
| serdes |CMU| serdes | | serdes |CMU| serdes |
|  0~1   | 0 |  2~3   | |  0~1   | 1 |  2~3   |
+--------+---+--------+ +--------+---+--------+
Every hss(4 serdes) just have 1 CMU.
serdes 0~3 must keep same in pll, or it cannot be allocated.
*/
int32
_sys_tsingma_datapath_init_cmu_parser(uint8 lchip, ctc_datapath_serdes_prop_t *p_serdes, uint8 *p_cmu_id)
{
    uint8 cmp_id;
    uint8 hss_id_12g;
    uint8 hss_id_28g;
    uint8 tmp_cmu_id;
    uint8 ref_isd_id;
    uint8 share_flag;
    uint8 half_choice_1;    //inner serdes 0~3 pll choice count
    uint8 half_choice_2;    //inner serdes 4~7 pll choice count
    uint8 cur_pll_choice;
    uint8 inner_serdes_id;
    uint8 equal_flag        = FALSE;
    uint8 pll_chg_flag      = TRUE;
    uint8 serdes_id         = 0;
    uint8 serdes_id_1       = 0;
    uint8 quad_1_lane       = 0;
    uint8 quad_2_lane       = 0;
    uint8 share_pll_choice  = 0xff;
    uint8 pll_choice[HSS12G_LANE_NUM];  //0~255
    
    CTC_PTR_VALID_CHECK(p_serdes);
    CTC_PTR_VALID_CHECK(p_cmu_id);

    /* HSS12G */
    for(hss_id_12g = 0; hss_id_12g < HSS12G_NUM_PER_SLICE; hss_id_12g++)
    {
        /* 0. Set initial value */
        sal_memset(pll_choice, 0xff, HSS12G_LANE_NUM * sizeof(uint8));
        cur_pll_choice   = 0;
        half_choice_1    = 0;
        half_choice_2    = 0;
        ref_isd_id       = 0;
        tmp_cmu_id       = 0;
        share_flag       = FALSE;
        share_pll_choice = 0xff;
        
        /* 1. Calculate how many pll choices within octal serdes, and record choice of each serdes. */
        /*    check if inner_serdes_id pll is equal to previous inner serdes */
        /*    pll_choice - record choice of each serdes    cur_pll_choice - pll choices count of octal serdes */
        /* 1.1 calculate first quad and octal serdes choice, and choice of every serdes */
        for(inner_serdes_id = 0; inner_serdes_id < HSS12G_LANE_NUM; inner_serdes_id++)
        {
            serdes_id = hss_id_12g * HSS12G_LANE_NUM + inner_serdes_id;
            if ((CTC_CHIP_SERDES_NONE_MODE == p_serdes[serdes_id].mode)
                || (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes[serdes_id].mode))
            {
                continue;
            }
            if(0 == inner_serdes_id)
            {
                pll_choice[inner_serdes_id] = cur_pll_choice;
                cur_pll_choice++;
                half_choice_1++;
                continue;
            }
            for(cmp_id = 0; cmp_id < inner_serdes_id; cmp_id++)
            {
                if((CTC_CHIP_SERDES_NONE_MODE == p_serdes[hss_id_12g * HSS12G_LANE_NUM + cmp_id].mode)
                   || (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes[hss_id_12g * HSS12G_LANE_NUM + cmp_id].mode))
                {
                    continue;
                }
                CTC_ERROR_RETURN(sys_tsingma_datapath_12g_is_pll_equal((p_serdes[hss_id_12g*HSS12G_LANE_NUM+cmp_id].mode), 
                                                                        (p_serdes[serdes_id].mode), FALSE, FALSE, &equal_flag));
                if(equal_flag)    
                {
                    pll_choice[inner_serdes_id] = pll_choice[cmp_id];
                    pll_chg_flag = FALSE;
                    break;
                }
            }
            if(pll_chg_flag)
            {
                pll_choice[inner_serdes_id] = cur_pll_choice;
                cur_pll_choice++;
                if(HSS12G_LANE_NUM/2 > inner_serdes_id)
                {
                    half_choice_1++;
                }
            }
            pll_chg_flag = TRUE;
        }

        /* 1.2 calculate second quad serdes choice */
        for(inner_serdes_id = HSS12G_LANE_NUM/2; inner_serdes_id < HSS12G_LANE_NUM; inner_serdes_id++)
        {
            if(0xff == pll_choice[inner_serdes_id])
            {
                continue;
            }
            if(HSS12G_LANE_NUM/2 == inner_serdes_id)
            {
                half_choice_2++;
                continue;
            }
            for(cmp_id = HSS12G_LANE_NUM/2; cmp_id < inner_serdes_id; cmp_id++)
            {
                if((pll_choice[inner_serdes_id] == pll_choice[cmp_id]) && (0xff != pll_choice[cmp_id]))
                {
                    pll_chg_flag = FALSE;
                    break;
                }
            }
            if(pll_chg_flag)
            {
                half_choice_2++;
            }
            pll_chg_flag = TRUE;
        }

        /* 2. Check if this octal serdes is able to be allocated.
              Each quad serdes can have maximum 2 kinds of pll, and octal serdes have maximum 3 in total. */
        if((2 < half_choice_1) || (2 < half_choice_2) || (3 < cur_pll_choice))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s: hss_id_12g %d, half_choice_1 %d, half_choice_2 %d, cur_pll_choice %d\n", 
                                 __FUNCTION__, hss_id_12g, half_choice_1, half_choice_2, cur_pll_choice);
            return CTC_E_INVALID_CONFIG;
        }

        /* 3. Decide which CMU is chosen for each serdes. */
        /* 3.1 find public pll of 1st & 2nd quad lane, and share_pll_choice is public pll */
        for(quad_1_lane = 0; quad_1_lane < HSS12G_LANE_NUM/2; quad_1_lane++)
        {
            for(quad_2_lane = HSS12G_LANE_NUM/2; quad_2_lane < HSS12G_LANE_NUM; quad_2_lane++)
            {
                if((pll_choice[quad_1_lane] == pll_choice[quad_2_lane]) && (0xff != pll_choice[quad_1_lane]))
                {
                    share_flag = TRUE;
                    share_pll_choice = pll_choice[quad_1_lane];
                    break;
                }
            }
            if(share_flag)
            {
                break;
            }
        }
        /* 3.2 decide which CMU is chosen for each serdes */
        for(inner_serdes_id = 0; inner_serdes_id < HSS12G_LANE_NUM; inner_serdes_id++)
        {
            serdes_id = hss_id_12g * HSS12G_LANE_NUM + inner_serdes_id;
            if((CTC_CHIP_SERDES_NONE_MODE == p_serdes[serdes_id].mode)
               || (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes[serdes_id].mode))
            {
                continue;
            }
            /* 3.2.1 if no public pll, serdes 0-CMU0, serdes 4-CMU2, other serdes choose CMU based on 0 or 4 */
            if(!share_flag)
            {
                switch(inner_serdes_id)
                {
                    case 0:
                        p_cmu_id[serdes_id] = 0;
                        break;
                    case 1:
                    case 2:
                    case 3:
                        if(pll_choice[0] == pll_choice[inner_serdes_id])
                        {
                            p_cmu_id[serdes_id] = 0;
                        }
                        else
                        {
                            p_cmu_id[serdes_id] = 1;
                        }
                        break;
                    case 4:
                        p_cmu_id[serdes_id] = 2;
                        break;
                    case 5:
                    case 6:
                    case 7:
                    default:
                        if(pll_choice[4] == pll_choice[inner_serdes_id])
                        {
                            p_cmu_id[serdes_id] = 2;
                        }
                        else
                        {
                            p_cmu_id[serdes_id] = 1;
                        }
                        break;
                }
                continue;
            }
            /* 3.2.2 If there is public pll, and if this lane choose public pll, use CMU1, else use CMU0 or CMU2 */
            if((share_flag) && (share_pll_choice == pll_choice[inner_serdes_id]) && (0xff != pll_choice[inner_serdes_id]))
            {
                p_cmu_id[serdes_id] = 1;
                continue;
            }
            /* Use CMU0 (1st quad) or CMU2 (2nd quad) */
            if(inner_serdes_id < (HSS12G_LANE_NUM/2))
            {
                tmp_cmu_id = 0;
                ref_isd_id = quad_1_lane;
            }
            else
            {
                tmp_cmu_id = 2;
                ref_isd_id = quad_2_lane;
            }
            if(pll_choice[inner_serdes_id] == pll_choice[ref_isd_id])
            {
                p_cmu_id[serdes_id] = 1;
            }
            else
            {
                p_cmu_id[serdes_id] = tmp_cmu_id;
            }
        }
    }
    
    /* HSS28G */
    /* 1. Check serdes in one hss is belong to one PLL */
    for(hss_id_28g = 0; hss_id_28g < TSINGMA_HSS28G_NUM_PER_SLICE; hss_id_28g++)
    {
        for(inner_serdes_id = 0; inner_serdes_id < HSS28G_LANE_NUM; inner_serdes_id++)
        {
            serdes_id = SYS_DATAPATH_SERDES_28G_START_ID + hss_id_28g * HSS28G_LANE_NUM + inner_serdes_id;
            if ((CTC_CHIP_SERDES_NONE_MODE == p_serdes[serdes_id].mode)
                || (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes[serdes_id].mode))
            {
                continue;
            }
            for(cmp_id = 0; cmp_id < inner_serdes_id; cmp_id++)
            {
                serdes_id_1 = SYS_DATAPATH_SERDES_28G_START_ID + hss_id_28g * HSS28G_LANE_NUM + cmp_id;
                CTC_ERROR_RETURN(sys_tsingma_datapath_28g_is_pll_equal(p_serdes[serdes_id_1].mode, p_serdes[serdes_id].mode, FALSE, FALSE, &equal_flag));
                if(!equal_flag)    
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s: hss_id_28g %d, serdes %d and %d are in different PLL!\n", 
                                         __FUNCTION__, hss_id_28g, serdes_id, serdes_id_1);
                    return CTC_E_INVALID_CONFIG;
                }
            }
        }
    }
    /* 2. All 28G quad serdes choose same CMU */
    for(serdes_id = SYS_DATAPATH_SERDES_28G_START_ID; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        p_cmu_id[serdes_id] = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_set_hss12g_cmu_cfg(uint8 lchip, uint8 hss_id, uint8 *p_serdes_mode, uint8* p_cmu_id, uint8* p_ovclk_flag, uint8* p_cmu_flag)
{
    uint8    fld            = 0;
    uint8    inner_serdes_id = 0;
    uint8    cmu_id         =  0;
    uint32   tbl_id         =  0;
    uint8    serdes_id      =  0;
    uint32   value[3]       = {0};
    uint8    cmu_dp_flag[3] = {FALSE, FALSE, FALSE};
    uint32   step = Hss12GCmuCfg0_cfgHssCmu1HwtCfgDivSel_f - Hss12GCmuCfg0_cfgHssCmu0HwtCfgDivSel_f;
    fld_id_t fld_id[3]      = {0};
    fld_id_t fld_id_base[3] = {
        Hss12GCmuCfg0_cfgHssCmu0HwtCfgDivSel_f, 
        Hss12GCmuCfg0_cfgHssCmu0HwtVcoDivSel_f, 
        Hss12GCmuCfg0_cfgHssCmu0HwtMultiLaneMode_f
    };
    Hss12GCmuCfg0_m cmu_cfg;

    for(inner_serdes_id = 0; inner_serdes_id < HSS12G_LANE_NUM; inner_serdes_id++)
    {
        serdes_id = HSS12G_LANE_NUM*hss_id + inner_serdes_id;
        cmu_id = p_cmu_id[serdes_id];
        
        if(CTC_CHIP_SERDES_NONE_MODE == (p_serdes_mode[serdes_id]) || (3 <= cmu_id))
        {
            continue;
        }
        /*ensure each pll just be written once*/
        if((p_cmu_flag[cmu_id]) && (FALSE == cmu_dp_flag[cmu_id]))
        {
            for(fld = 0; fld < 3; fld++)
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_load_cmu_val(p_serdes_mode[serdes_id], fld, &(value[fld]), p_ovclk_flag[serdes_id]));
                fld_id[fld] = fld_id_base[fld] + cmu_id*step;
            }
            tbl_id = Hss12GCmuCfg0_t + hss_id*(Hss12GCmuCfg1_t-Hss12GCmuCfg0_t);
            CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 3, fld_id, value, &cmu_cfg));
            cmu_dp_flag[cmu_id] = TRUE;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_12g_additional_cmu_cfg(uint8 lchip, uint8 hss_id, uint8 *p_serdes_mode, uint8* p_cmu_id, uint8* p_ovclk_flag, uint8* p_cmu_flag)
{
    uint8    fld;
    uint8    inner_serdes_id;
    uint8    cmu_id         =  0;
    uint8    serdes_id      =  0;
    uint8    cmu_dp_flag[3] = {FALSE, FALSE, FALSE};
    /*Additional 12G cmu cfg*/
    /*cfg_sw_8g  cfg_sw_10g  r_auto_pwrchg_en  r_en_ratechg_ctrl*/
    uint8    speed          = 0;
    uint8    addr[4]        = {0x09, 0x09, 0x42, 0x45};
    uint8    mask[4]        = {0xef, 0xdf, 0xfb, 0xfe};
    uint8    data[4]        = {0x00, 0x01, 0x00, 0x00};


    /*Additional 12G cmu cfg*/
    sal_memset(cmu_dp_flag, FALSE, 3 * sizeof(uint8));
    for(inner_serdes_id = 0; inner_serdes_id < HSS12G_LANE_NUM; inner_serdes_id++)
    {
        serdes_id = HSS12G_LANE_NUM*hss_id + inner_serdes_id;
        cmu_id = p_cmu_id[serdes_id];
        
        if(CTC_CHIP_SERDES_NONE_MODE == (p_serdes_mode[serdes_id]) || (3 <= cmu_id))
        {
            continue;
        }
        /*ensure each pll just be written once*/
        if((p_cmu_flag[cmu_id]) && (FALSE == cmu_dp_flag[cmu_id]))
        {
            //cfg_sw_10g value fix
            CTC_ERROR_RETURN(_sys_tsingma_datapath_get_rate(p_serdes_mode[serdes_id], SYS_DATAPATH_HSS_TYPE_15G, p_ovclk_flag[serdes_id], NULL, NULL, &speed));
            data[1] = ((SYS_TM_LANE_3D125G == speed) || (SYS_TM_LANE_6D25G == speed) || (SYS_TM_LANE_12D5G == speed)) ? 0 : 0x1;
            
            for(fld = 0; fld < 4; fld++)
            {
                sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, cmu_id, addr[fld], mask[fld], data[fld]);
            }
            cmu_dp_flag[cmu_id] = TRUE;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
sys_tsingma_datapath_hss28g_tx_swing_cfg(uint8 lchip, uint8 hss_id)
{
    uint16 addr = 0;
    uint16 data = 0;
    uint16 mask = 0;
    uint8 i = 0;

    /* 8zf5[2:0] = 111; 8zf4[15:13] = 110; 8zf4[12:10] = 110 */
    for (i = 0; i < 4; i++)
    {
        addr = 0x80f5 + i*0x100;
        data = 7;
        mask = 0xfff8;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));

        addr = 0x80f4 + i*0x100;
        data = 0x36;
        mask = 0x03ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));
    }
    
    return CTC_E_NONE;
}

void
sys_tsingma_datapath_hss28g_10g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FD, 0x8248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FE, 0x8498);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x8244);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0x8498);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4384|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4384|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4384|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4384|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61fc);

    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}
void
sys_tsingma_datapath_hss28g_25g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d,  0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081,  0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082,  0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075,  0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb,  0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8,  0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005,  0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050,  0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006,  0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d,  0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181,  0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182,  0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175,  0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb,  0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8,  0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105,  0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150,  0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106,  0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d,  0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281,  0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282,  0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275,  0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb,  0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8,  0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205,  0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250,  0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206,  0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d,  0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381,  0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382,  0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375,  0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb,  0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8,  0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305,  0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350,  0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306,  0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063,  0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002,  0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d,  0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082,  0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075,  0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005,  0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163,  0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102,  0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d,  0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182,  0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175,  0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105,  0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263,  0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202,  0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d,  0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282,  0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275,  0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205,  0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363,  0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302,  0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d,  0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382,  0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375,  0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305,  0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff,  0x3df0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fe,  0xa598);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fd,  0x2248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc,  0x4a66);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb,  0x6dd4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa,  0x52c0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6,  0xfdc0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5,  0xa598);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4,  0x2246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3,  0x5336);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2,  0x6ea4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef,  0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3,  0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4,  0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5,  0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3,  0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4,  0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5,  0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3,  0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4,  0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5,  0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3,  0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4,  0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5,  0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff,  0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff,  0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff,  0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff,  0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0,  0x4080|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061,  0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e,  0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0,  0x4080|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161,  0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e,  0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0,  0x4080|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261,  0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e,  0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0,  0x4080|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361,  0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e,  0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b,  0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b,  0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b,  0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b,  0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001,  0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079,  0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101,  0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179,  0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201,  0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279,  0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301,  0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379,  0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000,  0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007,  0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008,  0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083,  0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084,  0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085,  0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086,  0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087,  0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088,  0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100,  0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107,  0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108,  0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183,  0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184,  0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185,  0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186,  0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187,  0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188,  0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200,  0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207,  0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208,  0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283,  0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284,  0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285,  0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286,  0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287,  0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288,  0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300,  0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307,  0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308,  0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383,  0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384,  0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385,  0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386,  0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387,  0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388,  0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003,  0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103,  0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203,  0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303,  0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345,  0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076,  0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176,  0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276,  0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376,  0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077,  0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177,  0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277,  0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377,  0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078,  0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178,  0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278,  0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378,  0x61ff);

    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}
void
sys_tsingma_datapath_hss28g_1g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FD, 0x8248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FE, 0x8098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);  
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x8246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0x8098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4387|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4387|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4387|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4387|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x88ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x88ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x88ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x88ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61ff);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}
void
sys_tsingma_datapath_hss28g_2dot5g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FD, 0x4248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FE, 0xa098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);  
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x4246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0xa098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4386|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4386|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4386|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4386|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x88df);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x88df);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x88df);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x88df);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61ff);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}
void
sys_tsingma_datapath_hss28g_dxaui_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fe, 0xa098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fd, 0x4248); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0xa098); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x4246); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4085|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4085|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4085|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4085|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x88bf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x88bf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x88bf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x88bf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61ff);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}

void
sys_tsingma_datapath_hss28g_11g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d , 0x0000); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081 , 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082 , 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075 , 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb , 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8 , 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005 , 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050 , 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006 , 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d , 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181 , 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182 , 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175 , 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb , 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8 , 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105 , 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150 , 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106 , 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d , 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281 , 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282 , 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275 , 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb , 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8 , 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205 , 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250 , 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206 , 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d , 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381 , 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382 , 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375 , 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb , 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8 , 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305 , 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350 , 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306 , 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063 , 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002 , 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d , 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082 , 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075 , 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005 , 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163 , 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102 , 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d , 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182 , 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175 , 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105 , 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263 , 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202 , 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d , 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282 , 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275 , 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205 , 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363 , 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302 , 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d , 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382 , 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375 , 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305 , 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff , 0x3df0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fe , 0x9298); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fd , 0x6248); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc , 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb , 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa , 0x52c0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6 , 0xfdc0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5 , 0x9298); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4 , 0x6244); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3 , 0x5336);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2 , 0x6ea4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef , 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3 , 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4 , 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5 , 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3 , 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4 , 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5 , 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3 , 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4 , 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5 , 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3 , 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4 , 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5 , 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff , 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff , 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff , 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff , 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0 , 0x4084|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7 , 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061 , 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e , 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0 , 0x4084|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7 , 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161 , 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e , 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0 , 0x4084|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7 , 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261 , 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e , 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0 , 0x4084|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7 , 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361 , 0x3120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e , 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b , 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b , 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b , 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b , 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001 , 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079 , 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101 , 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179 , 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201 , 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279 , 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301 , 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379 , 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000 , 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007 , 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008 , 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083 , 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084 , 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085 , 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086 , 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087 , 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088 , 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100 , 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107 , 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108 , 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183 , 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184 , 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185 , 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186 , 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187 , 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188 , 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200 , 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207 , 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208 , 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283 , 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284 , 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285 , 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286 , 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287 , 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288 , 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300 , 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307 , 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308 , 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383 , 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384 , 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385 , 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386 , 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387 , 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388 , 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003 , 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103 , 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203 , 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303 , 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345 , 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076 , 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176 , 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276 , 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376 , 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077 , 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177 , 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277 , 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377 , 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078 , 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178 , 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278 , 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378 , 0x61fc);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}

void
sys_tsingma_datapath_hss28g_12dot5g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fe, 0xa098);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fd, 0x4248); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0xa098); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x4244); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4004|0xeb00); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x0120); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4004|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4004|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4004|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61fc);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}

void
sys_tsingma_datapath_hss28g_28dot125g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fe, 0xb498);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fd, 0x0248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0xb498); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x0246); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0x4000|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0x4000|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0x4000|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0x4000|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x0120);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0201);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x881f);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61ff);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}

void
sys_tsingma_datapath_hss28g_12dot96875g_cfg(uint8 lchip, uint8 hss_id)
{
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8081, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8050, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8006, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8181, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8150, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8106, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8281, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8250, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8206, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x0000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8381, 0x1000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xb800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xfbbf);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fb, 0x6d87);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f8, 0x6850);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0x80a2);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8350, 0x5040);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8306, 0xa100);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8063, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8064, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8002, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8082, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8075, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8005, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8163, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8164, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8102, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8182, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8175, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8105, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8263, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8264, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8202, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8282, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8275, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8205, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8363, 0x0080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8364, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8302, 0x4006);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834d, 0x8000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8382, 0xd800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8375, 0xc6ff);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8305, 0xae82);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ff, 0x3df0); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FD, 0x2248);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84FE, 0xa698);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fc, 0x4a66); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fb, 0x6dd4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84fa, 0x52c0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f6, 0xfdc0);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f4, 0x2244);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f5, 0xa698);
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f3, 0x5336); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84f2, 0x6ea4); 
    HSS28G_WR_DELAY(lchip, hss_id, 0x84ef, 0xa020);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f3, 0x4000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f4, 0x4a40);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83f5, 0x0023);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83ff, 0x88b6);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a0, 0xeb04|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x80a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8061, 0x3520);
    HSS28G_WR_DELAY(lchip, hss_id, 0x804e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a0, 0xeb04|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8161, 0x3520);
    HSS28G_WR_DELAY(lchip, hss_id, 0x814e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a0, 0xeb04|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8261, 0x3520);
    HSS28G_WR_DELAY(lchip, hss_id, 0x824e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a0, 0xeb04|0xeb00);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83a7, 0x1800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8361, 0x3520);
    HSS28G_WR_DELAY(lchip, hss_id, 0x834e, 0x3800);
    HSS28G_WR_DELAY(lchip, hss_id, 0x803b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x813b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x823b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x833b, 0xd000);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8001, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8079, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8101, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8179, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8201, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8279, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8301, 0x0401);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8379, 0x889e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8000, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8007, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8008, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8083, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8084, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8085, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8086, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8087, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8088, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8100, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8107, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8108, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8183, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8184, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8185, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8186, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8187, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8188, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8200, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8207, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8208, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8283, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8284, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8285, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8286, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8287, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8288, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8300, 0x43fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8307, 0x4382);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8308, 0x6a9b);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8383, 0x427e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8384, 0x8202);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8385, 0x407e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8386, 0xc07e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8387, 0x7f7c);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8388, 0x7f7e);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8003, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8103, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8203, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8303, 0x5933);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8045, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8145, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8245, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8345, 0x8080);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8076, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8176, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8276, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8376, 0x0420);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8077, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8177, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8277, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8377, 0xc414);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8078, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8178, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8278, 0x61fc);
    HSS28G_WR_DELAY(lchip, hss_id, 0x8378, 0x61fc);
    
    HSS28G_WR_DELAY(lchip, hss_id, 0x80fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x81fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x82fd,  0x0246);
    HSS28G_WR_DELAY(lchip, hss_id, 0x83fd,  0x0246);
}

int32
_sys_tsingma_datapath_28g_additional_cfg(uint8 lchip, uint8 hss_id, uint8 mode, uint8  ovclk_flag)
{
    /*ovclk_flag: 0-no change  1-(Any)XFI 12.5G  2-(Any)disable  3-(28G)XFI 11.40625G  4-(28G)XXVG 28.125G  5-(28G)XFI 12.96875G*/
    if((3 == ovclk_flag) && SYS_DATAPATH_IS_SERDES_10G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_11g_cfg(lchip, hss_id);
        return CTC_E_NONE;
    }
    if((1 == ovclk_flag) && SYS_DATAPATH_IS_SERDES_10G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_12dot5g_cfg(lchip, hss_id);
        return CTC_E_NONE;
    }
    if((4 == ovclk_flag) && SYS_DATAPATH_IS_SERDES_25G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_28dot125g_cfg(lchip, hss_id);
        return CTC_E_NONE;
    }
    if((5 == ovclk_flag) && SYS_DATAPATH_IS_SERDES_10G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_12dot96875g_cfg(lchip, hss_id);
        return CTC_E_NONE;
    }

    if(SYS_DATAPATH_IS_SERDES_25G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_25g_cfg(lchip, hss_id);
    }
    else if(SYS_DATAPATH_IS_SERDES_10G_PER_LANE(mode))
    {
        sys_tsingma_datapath_hss28g_10g_cfg(lchip, hss_id);
    }
    else if(CTC_CHIP_SERDES_SGMII_MODE == mode)
    {
        sys_tsingma_datapath_hss28g_1g_cfg(lchip, hss_id);
    }
    /* 2.5G/XAUI use 2.5g config */
    else if((CTC_CHIP_SERDES_2DOT5G_MODE == mode) || (CTC_CHIP_SERDES_XAUI_MODE == mode))
    {
        sys_tsingma_datapath_hss28g_2dot5g_cfg(lchip, hss_id);
    }
    else if(CTC_CHIP_SERDES_DXAUI_MODE == mode)
    {
        sys_tsingma_datapath_hss28g_dxaui_cfg(lchip, hss_id);
    }
    else
    {
        return CTC_E_NONE;
    }

    sys_tsingma_datapath_hss28g_tx_swing_cfg(lchip, hss_id);
    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_12g_lane_misc_cfg(uint8 lchip, uint8 hss_id, uint8 lane_id, uint8 mode, uint8 cmu_id)
{
    uint8    fld;
    uint32   tbl_id         =  0;
    uint32   cmu_val        =  0;
    uint32   field_num      =  9;
    uint32   value[9]       = {0};
    fld_id_t fld_id[9]      = {0};
    fld_id_t fld_id_base[9] = {
        Hss12GLaneMiscCfg0_cfgHssL0DataWidthSel_f, 
        Hss12GLaneMiscCfg0_cfgHssL0PmaTxCkSel_f, 
        Hss12GLaneMiscCfg0_cfgHssL0PmaRxDivSel_f,
        Hss12GLaneMiscCfg0_cfgHssL0RxRateSel_f, 
        Hss12GLaneMiscCfg0_cfgHssL0TxRateSel_f, 
        Hss12GLaneMiscCfg0_cfgHssL0HwtMultiLaneMode_f,
        Hss12GLaneMiscCfg0_cfgHssL0Pcs2PmaPiExtDacBit20To14_f,
        Hss12GLaneMiscCfg0_cfgHssL0AuxTxCkSel_f, 
        Hss12GLaneMiscCfg0_cfgHssL0AuxRxCkSel_f
    };
    uint32   step = Hss12GLaneMiscCfg0_cfgHssL1DataWidthSel_f - Hss12GLaneMiscCfg0_cfgHssL0DataWidthSel_f;
    uint8    width_rate_cfg[CTC_CHIP_MAX_SERDES_MODE][7] = {
    /*DataWidthSel PmaTxCkSel PmaRxDivSel RxRateSel TxRateSel HwtMultiLaneMode Pcs2PmaPiExtDacBit20To14*/
        {0xff,     0xff,      0xff,       0xff,     0xff,     0xff,            0x06},    /*CTC_CHIP_SERDES_NONE_MODE*/
        {0x03,     0x03,      0x03,       0x00,     0x00,     0x00,            0x06},    /*CTC_CHIP_SERDES_XFI_MODE*/
        {0x00,     0x00,      0x00,       0x03,     0x03,     0x00,            0x06},    /*CTC_CHIP_SERDES_SGMII_MODE*/
        {0xff,     0xff,      0xff,       0xff,     0xff,     0xff,            0x06},    /*CTC_CHIP_SERDES_XSGMII_MODE*/
        {0x00,     0x00,      0x00,       0x01,     0x01,     0x00,            0x06},    /*CTC_CHIP_SERDES_QSGMII_MODE*/
        {0x02,     0x02,      0x02,       0x02,     0x02,     0x00,            0x06},    /*CTC_CHIP_SERDES_XAUI_MODE*/
        {0x02,     0x02,      0x02,       0x01,     0x01,     0x00,            0x06},    /*CTC_CHIP_SERDES_DXAUI_MODE*/
        {0x03,     0x03,      0x03,       0x00,     0x00,     0x00,            0x06},    /*CTC_CHIP_SERDES_XLG_MODE*/
        {0xff,     0xff,      0xff,       0xff,     0xff,     0xff,            0x06},    /*CTC_CHIP_SERDES_CG_MODE*/
        {0x00,     0x00,      0x00,       0x02,     0x02,     0x00,            0x06},    /*CTC_CHIP_SERDES_2DOT5G_MODE*/
        {0x03,     0x03,      0x03,       0x00,     0x00,     0x00,            0x06},    /*CTC_CHIP_SERDES_USXGMII0_MODE*/
        {0x03,     0x03,      0x03,       0x00,     0x00,     0x00,            0x06},    /*CTC_CHIP_SERDES_USXGMII1_MODE*/
        {0x03,     0x03,      0x03,       0x00,     0x00,     0x00,            0x06},    /*CTC_CHIP_SERDES_USXGMII2_MODE*/
        {0xff,     0xff,      0xff,       0xff,     0xff,     0xff,            0x06},    /*CTC_CHIP_SERDES_XXVG_MODE*/
        {0xff,     0xff,      0xff,       0xff,     0xff,     0xff,            0x06},    /*CTC_CHIP_SERDES_LG_MODE*/
        {0x00,     0x00,      0x00,       0x03,     0x03,     0x00,            0x06}     /*CTC_CHIP_SERDES_100BASEFX_MODE*/
    };
    Hss12GLaneMiscCfg0_m lane_cfg;

    
    for(fld = 0; fld < 7; fld++)
    {
        if(0xff == width_rate_cfg[mode][fld])
        {
            return CTC_E_NONE;
        }
        value[fld]  = width_rate_cfg[mode][fld];
        fld_id[fld] = fld_id_base[fld] + lane_id*step;
    }

    if(0xff != cmu_id)
    {
        if(((lane_id < HSS12G_LANE_NUM/2) && (1 < cmu_id)) || ((lane_id >= HSS12G_LANE_NUM/2) && ((1 > cmu_id)||(2 < cmu_id))))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%s: hss_id %d, lane_id %d, cmu_id %d invalid!\n", __FUNCTION__, hss_id, lane_id, cmu_id);
            return CTC_E_INVALID_PARAM;
        }
        /* Lane0~Lane3: 
           1'b0:  select CMU0
           1'b1:  select CMU1
           Lane4~Lane7: 
           1'b0:  select CMU1
           1'b1:  select CMU2 */
        cmu_val   = (lane_id < HSS12G_LANE_NUM/2) ? (cmu_id) : (cmu_id-1);
        value[7]  = cmu_val;
        fld_id[7] = fld_id_base[7] + lane_id*step;
        value[8]  = cmu_val;
        fld_id[8] = fld_id_base[8] + lane_id*step;
    }
    else
    {
        field_num = 7;
    }
    
    tbl_id = Hss12GLaneMiscCfg0_t + hss_id*(Hss12GLaneMiscCfg1_t-Hss12GLaneMiscCfg0_t);
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, field_num, fld_id, value, &lane_cfg));

    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_12g_clk_tree_cfg(uint8 lchip, uint8 hss_id, uint8 lane_id, uint8 mode)
{
    uint32   step     = 1;
    uint8    i        = 0;
    uint8    loop_num = 0;
    uint8    tmp_mode = 0;
    uint32   tbl_id   = Hss12GClkTreeCfg0_t + hss_id*(Hss12GClkTreeCfg1_t-Hss12GClkTreeCfg0_t);
    uint32   value    = 0;
    fld_id_t fld_id   = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    Hss12GClkTreeCfg0_m lane_cfg;

    fld_id = Hss12GClkTreeCfg0_cfgHssL0MultiLane_f + lane_id*step;
    value = ((CTC_CHIP_SERDES_XLG_MODE == mode) || (CTC_CHIP_SERDES_XAUI_MODE == mode) || (CTC_CHIP_SERDES_DXAUI_MODE == mode)) ? (1) : (0);
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, &fld_id, &value, &lane_cfg));

    if((CTC_CHIP_SERDES_QSGMII_MODE == mode) || (CTC_CHIP_SERDES_USXGMII1_MODE == mode) || (CTC_CHIP_SERDES_USXGMII2_MODE == mode))
    {
        if(0 == lane_id % 4)
        {
            value = 1;
            loop_num = ((CTC_CHIP_SERDES_USXGMII2_MODE == mode) ? 2 : 4);
            for (i = 0; i < loop_num; i++)
            {
                fld_id = Hss12GClkTreeCfg0_cfgClockUsxgmii0TxSel0_f + i*step;
                if (4 == lane_id)
                {
                    fld_id += 4;
                }
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, &fld_id, &value, &lane_cfg));
            }
        }
    }
    else
    {
        if(0 == lane_id % 4)
        {
            value = 0;
            loop_num = ((CTC_CHIP_SERDES_USXGMII2_MODE == mode) ? 2 : 4);
            for (i = 0; i < loop_num; i++)
            {
                fld_id = Hss12GClkTreeCfg0_cfgClockUsxgmii0TxSel0_f + i*step;
                if (4 == lane_id)
                {
                    fld_id += 4;
                }
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, &fld_id, &value, &lane_cfg));
            }
        }
        else
        {
            p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_id);
            if(p_hss_vec == NULL)
            {
                return CTC_E_INVALID_PTR;
            }
            tmp_mode = p_hss_vec->serdes_info[lane_id/4*4].mode;
            if(((CTC_CHIP_SERDES_QSGMII_MODE != tmp_mode) && 
                (CTC_CHIP_SERDES_USXGMII1_MODE != tmp_mode) && 
                (CTC_CHIP_SERDES_USXGMII2_MODE != tmp_mode)) || 
               ((CTC_CHIP_SERDES_USXGMII2_MODE == tmp_mode) && (1 < lane_id % 4)))
            {
                value = 0;
                fld_id = Hss12GClkTreeCfg0_cfgClockUsxgmii0TxSel0_f + lane_id*step;
                CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, &fld_id, &value, &lane_cfg));
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_28g_clk_tree_cfg(uint8 lchip, uint16 lport)
{
    uint8    slice_id  = 0;
    uint8    i         = 0;
    uint32   tbl_id    = 0;
    uint32   fld_id    = 0;
    uint32   cmd       = 0;
    uint32   step      = 0;
    uint32   step_idx  = 0;
    uint32   tmp_val32[5] = {0};
    uint8 lane_swap_0 = 0;
    uint8 lane_swap_1 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    Hss28GClkTreeCfg_m  hss28g_clktree;

    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_NONE;
    }
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        return CTC_E_NONE;
    }

    if (!SYS_DATAPATH_SERDES_IS_HSS28G(port_attr->serdes_id))
    {
        return CTC_E_NONE;
    }
    
    step_idx = port_attr->internal_mac_idx;
    if ((port_attr->serdes_id >= 28) && (port_attr->serdes_id <= 31))
    {
        step_idx += 4;
    }

    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, &lane_swap_0, &lane_swap_1));

    tbl_id = Hss28GClkTreeCfg_t;

    /* toggle cfgResetHssL[0-7]TxDiv2*/
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));
    tmp_val32[0] = 1;
    for (i = 0; i < port_attr->serdes_num; i++)
    {
        step = Hss28GClkTreeCfg_cfgResetHssL1TxDiv2_f - Hss28GClkTreeCfg_cfgResetHssL0TxDiv2_f;
        fld_id = Hss28GClkTreeCfg_cfgResetHssL0TxDiv2_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[0], &hss28g_clktree);        
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));
    tmp_val32[0] = 0;
    for (i = 0; i < port_attr->serdes_num; i++)
    {
        step = Hss28GClkTreeCfg_cfgResetHssL1TxDiv2_f - Hss28GClkTreeCfg_cfgResetHssL0TxDiv2_f;
        fld_id = Hss28GClkTreeCfg_cfgResetHssL0TxDiv2_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[0], &hss28g_clktree);        
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));
    /* toggle cfgResetHssL[0-7]TxDiv2 end */

    /* tmp_val32[1~4] assignment */
    if (((port_attr->serdes_id >= 24) && (port_attr->serdes_id <= 27) && (1 == lane_swap_0))   /* HSS28G0 lane swap */
        || ((port_attr->serdes_id >= 28) && (port_attr->serdes_id <= 31) && (1 == lane_swap_1)))  /* HSS28G1 lane swap */
    {
        switch (port_attr->pcs_mode)
        {
        case CTC_CHIP_SERDES_XFI_MODE:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            if (2 == port_attr->internal_mac_idx)
            {
                tmp_val32[2] = 1;
            }
            tmp_val32[3] = 0;
            if (0 == (port_attr->internal_mac_idx % 2))
            {
                tmp_val32[3] = 1;
            }
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            if (2 == port_attr->internal_mac_idx)
            {
                tmp_val32[2] = 1;
            }
            tmp_val32[3] = 0;
            if (0 == (port_attr->internal_mac_idx % 2))
            {
                tmp_val32[3] = 1;
            }
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            if (2 == port_attr->internal_mac_idx)
            {
                tmp_val32[2] = 1;
            }
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
        default:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            tmp_val32[3] = 0;
            tmp_val32[4] = 1;
            break;
        }
    }
    else
    {
        switch (port_attr->pcs_mode)
        {
        case CTC_CHIP_SERDES_XFI_MODE:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            tmp_val32[3] = 0;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            tmp_val32[3] = 0;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            if (2 == port_attr->internal_mac_idx)
            {
                tmp_val32[2] = 1;
            }
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_CG_MODE:
            tmp_val32[1] = 1;
            tmp_val32[2] = 0;
            tmp_val32[3] = 1;
            tmp_val32[4] = 0;
            break;
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
        default:
            tmp_val32[1] = 0;
            tmp_val32[2] = 0;
            tmp_val32[3] = 0;
            tmp_val32[4] = 1;
            break;
        }
    }

    /* write table/field */
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));
    for (i = 0; i < port_attr->serdes_num; i++)
    {
        step = Hss28GClkTreeCfg_cfgClockHssL1TxDiv2Sel_f - Hss28GClkTreeCfg_cfgClockHssL0TxDiv2Sel_f;
        fld_id = Hss28GClkTreeCfg_cfgClockHssL0TxDiv2Sel_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[1], &hss28g_clktree);

        step = Hss28GClkTreeCfg_cfgClockHssL1TxL2Div2Sel_f - Hss28GClkTreeCfg_cfgClockHssL0TxL2Div2Sel_f;
        fld_id = Hss28GClkTreeCfg_cfgClockHssL0TxL2Div2Sel_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[2], &hss28g_clktree);

        step = Hss28GClkTreeCfg_cfgClockHssL1TxMultiLane_f - Hss28GClkTreeCfg_cfgClockHssL0TxMultiLane_f;
        fld_id = Hss28GClkTreeCfg_cfgClockHssL0TxMultiLane_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[3], &hss28g_clktree);

        step = Hss28GClkTreeCfg_cfgClockPcsL1TxFastSel_f - Hss28GClkTreeCfg_cfgClockPcsL0TxFastSel_f;
        fld_id = Hss28GClkTreeCfg_cfgClockPcsL0TxFastSel_f + step*(step_idx+i);
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32[4], &hss28g_clktree);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_clktree));
    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_12g_data_out_cfg(uint8 lchip, uint8 hss_id, uint8 lane_id, uint8 mode)
{
    uint32   step      = Hss12GMacroCfg0_cfgHssL1TxDataOutSel_f - Hss12GMacroCfg0_cfgHssL0TxDataOutSel_f;
    uint32   tbl_id    = Hss12GMacroCfg0_t + hss_id*(Hss12GMacroCfg1_t-Hss12GMacroCfg0_t);
    fld_id_t fld_id[2] = {
        Hss12GMacroCfg0_cfgHssL0TxDataOutSel_f + lane_id*step, 
        Hss12GMacroCfg0_cfgHssL0UsxgmiiMode_f  + lane_id*step
    };
    uint32   data_out_cfg[CTC_CHIP_MAX_SERDES_MODE][2] = {
    //TxDataOutSel    UsxgmiiMode
        {0xffffffff,  0xffffffff},    //CTC_CHIP_SERDES_NONE_MODE
        {0x00000002,  0x00000000},    //CTC_CHIP_SERDES_XFI_MODE
        {0x00000000,  0x00000000},    //CTC_CHIP_SERDES_SGMII_MODE
        {0xffffffff,  0xffffffff},    //CTC_CHIP_SERDES_XSGMII_MODE
        {0x00000003,  0x00000000},    //CTC_CHIP_SERDES_QSGMII_MODE
        {0x00000001,  0x00000000},    //CTC_CHIP_SERDES_XAUI_MODE
        {0x00000001,  0x00000000},    //CTC_CHIP_SERDES_DXAUI_MODE
        {0x00000002,  0x00000000},    //CTC_CHIP_SERDES_XLG_MODE
        {0xffffffff,  0xffffffff},    //CTC_CHIP_SERDES_CG_MODE
        {0x00000000,  0x00000000},    //CTC_CHIP_SERDES_2DOT5G_MODE
        {0x00000004,  0x00000001},    //CTC_CHIP_SERDES_USXGMII0_MODE
        {0x00000004,  0x00000000},    //CTC_CHIP_SERDES_USXGMII1_MODE
        {0x00000004,  0x00000000},    //CTC_CHIP_SERDES_USXGMII2_MODE
        {0xffffffff,  0xffffffff},    //CTC_CHIP_SERDES_XXVG_MODE
        {0xffffffff,  0xffffffff},    //CTC_CHIP_SERDES_LG_MODE
        {0x00000000,  0x00000000}     //CTC_CHIP_SERDES_100BASEFX_MODE
    };
    Hss12GMacroCfg0_m hss_cfg;


    if(CTC_CHIP_MAX_SERDES_MODE <= mode)
    {
        return CTC_E_INVALID_PARAM;
    }
    if((0xffffffff == data_out_cfg[mode][0]) || (0xffffffff == data_out_cfg[mode][1]))
    {
        return CTC_E_NONE;
    }
    
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 2, fld_id, data_out_cfg[mode], &hss_cfg));

    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_12g_additional_lane_cfg(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 ovclk_flag)
{
    uint8 hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8 lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    uint8 data[20][5] = {
       /*10.3125G  12.5G     5G        1.25G     3.125G*/
        {0x0a,     0x0a,     0x0a,     0x0a,     0x0a}, //cfg_tap_dly2[3:0]
        {0x02,     0x02,     0x02,     0x02,     0x02}, //cfg_ctle_negC
        {0x05,     0x02,     0x02,     0x02,     0x05}, //cfg_pi_ext_dac[7:0]
        {0x80,     0x80,     0x80,     0x80,     0x80}, //cfg_pi_ext_dac[15:8]
        {0x01,     0x01,     0x01,     0x01,     0x01}, //cfg_pi_ext_dac[23:16]
        {0x18,     0x18,     0x18,     0x18,     0x18}, //cfg_pi_ext_dac[30:24]
        {0x03,     0x03,     0x03,     0x03,     0x03}, //cfg_pi_ext_qrt[1:0]
        {0x00,     0x00,     0x04,     0x0e,     0x0c}, //cfg_pi_bw_gen1[3:0]
        {0x84,     0x82,     0x82,     0x82,     0x84}, //cfg_iscan_ext_dac[7:0]
        {0x1f,     0x1f,     0x1f,     0x1f,     0x1f}, //cfg_iscan_ext_dac[15:8]
        {0x20,     0x20,     0x20,     0x20,     0x20}, //cfg_iscan_ext_dac[23:16]
        {0x28,     0x28,     0x28,     0x28,     0x28}, //cfg_iscan_ext_dac[30:24]
        {0x03,     0x03,     0x03,     0x03,     0x03}, //cfg_iscan_ext_qrt[1:0]
        {0x00,     0x00,     0x01,     0x02,     0x02}, //cfg_rxfilt_z_gen1[1:0]
        {0x0c,     0x0c,     0x0e,     0x02,     0x06}, //cfg_mp_max_gen1[3:0]
        {0x05,     0x05,     0x05,     0x05,     0x05}, //cfg_mf_max[3:0]
        {0x20,     0x20,     0x20,     0x20,     0x20}, //cfg_lane_reserve[7:0]
        {0xf1,     0xf1,     0xf1,     0xf1,     0xf1}, //cfg_lane_reserve[15:8]
        {0x03,     0x03,     0x03,     0x01,     0x01}, //cfg_rx_sp_ctle[1:0]
        {0x00,     0x00,     0x00,     0x00,     0x00}  //r_en_ratechg_ctrl
    };
    uint8 addr[20] = {
        0x05, //cfg_tap_dly2[3:0]
        0x08, //cfg_ctle_negC
        0x14, //cfg_pi_ext_dac[7:0]
        0x15, //cfg_pi_ext_dac[15:8]
        0x16, //cfg_pi_ext_dac[23:16]
        0x17, //cfg_pi_ext_dac[30:24]
        0x18, //cfg_pi_ext_qrt[1:0]
        0x24, //cfg_pi_bw_gen1[3:0]
        0x26, //cfg_iscan_ext_dac[7:0]
        0x27, //cfg_iscan_ext_dac[15:8]
        0x28, //cfg_iscan_ext_dac[23:16]
        0x29, //cfg_iscan_ext_dac[30:24]
        0x2a, //cfg_iscan_ext_qrt[1:0]
        0x38, //cfg_rxfilt_z_gen1[1:0]
        0x3a, //cfg_mp_max_gen1[3:0]
        0x3b, //cfg_mf_max[3:0]
        0x40, //cfg_lane_reserve[7:0]
        0x41, //cfg_lane_reserve[15:8]
        0x4a, //cfg_rx_sp_ctle[1:0]
        0x93  //r_en_ratechg_ctrl
    };
    uint8 mask[20] = {
        0xf0, //cfg_tap_dly2[3:0]        3:0
        0xcf, //cfg_ctle_negC            5:4
        0x00, //cfg_pi_ext_dac[7:0]      7:0
        0x00, //cfg_pi_ext_dac[15:8]     7:0
        0x00, //cfg_pi_ext_dac[23:16]    7:0
        0x80, //cfg_pi_ext_dac[30:24]    6:0
        0xcf, //cfg_pi_ext_qrt[1:0]      5:4
        0xf0, //cfg_pi_bw_gen1[3:0]      3:0
        0x00, //cfg_iscan_ext_dac[7:0]   7:0
        0x00, //cfg_iscan_ext_dac[15:8]  7:0
        0x00, //cfg_iscan_ext_dac[23:16] 7:0
        0x80, //cfg_iscan_ext_dac[30:24] 6:0
        0xcf, //cfg_iscan_ext_qrt[1:0]   5:4
        0xcf, //cfg_rxfilt_z_gen1[1:0]   5:4
        0x0f, //cfg_mp_max_gen1[3:0]     7:4
        0x0f, //cfg_mf_max[3:0]          7:4
        0x00, //cfg_lane_reserve[7:0]    7:0
        0x00, //cfg_lane_reserve[15:8]   7:0
        0xfc, //cfg_rx_sp_ctle[1:0]      1:0
        0xf7  //r_en_ratechg_ctrl        3:3
    };
    uint8 reg_idx;
    uint8 speed = 0;
    uint8 speed_fix = 0;


    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_rate(mode, SYS_DATAPATH_HSS_TYPE_15G, ovclk_flag, NULL, NULL, &speed));
    
    switch(speed)
    {
        case SYS_TM_LANE_10D3125G:
            speed_fix = 0;
            break;
        case SYS_TM_LANE_12D5G:
            speed_fix = 1;
            break;
        case SYS_TM_LANE_5G:
            speed_fix = 2;
            break;
        case SYS_TM_LANE_1D25G:
            speed_fix = 3;
            break;
        case SYS_TM_LANE_3D125G:
        case SYS_TM_LANE_6D25G:
            speed_fix = 4;
            break;
        default:
            return CTC_E_NONE;
    }

    for(reg_idx = 0; reg_idx < 20; reg_idx++)
    {
        sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id+3), addr[reg_idx], mask[reg_idx], data[reg_idx][speed_fix]);
    }
    

    return CTC_E_NONE;
}


int32
_sys_tsingma_datapath_28g_data_out_cfg(uint8 lchip, uint8 hss_id, uint8 lane_id, uint8 mode)
{
    uint32   step      = Hss28GMacroCfg_cfgHssL1TxDataOutSel_f - Hss28GMacroCfg_cfgHssL0TxDataOutSel_f;
    uint32   tbl_id    = Hss28GMacroCfg_t;
    fld_id_t fld_id    = Hss28GMacroCfg_cfgHssL0TxDataOutSel_f + (lane_id+hss_id*HSS28G_LANE_NUM)*step;
    uint32   data_out_cfg[CTC_CHIP_MAX_SERDES_MODE] = {
    //TxDataOutSel
        0xffffffff,    //CTC_CHIP_SERDES_NONE_MODE
        0x00000002,    //CTC_CHIP_SERDES_XFI_MODE
        0x00000000,    //CTC_CHIP_SERDES_SGMII_MODE
        0xffffffff,    //CTC_CHIP_SERDES_XSGMII_MODE
        0xffffffff,    //CTC_CHIP_SERDES_QSGMII_MODE
        0x00000001,    //CTC_CHIP_SERDES_XAUI_MODE
        0x00000001,    //CTC_CHIP_SERDES_DXAUI_MODE
        0x00000002,    //CTC_CHIP_SERDES_XLG_MODE
        0x00000003,    //CTC_CHIP_SERDES_CG_MODE
        0x00000000,    //CTC_CHIP_SERDES_2DOT5G_MODE
        0xffffffff,    //CTC_CHIP_SERDES_USXGMII0_MODE
        0xffffffff,    //CTC_CHIP_SERDES_USXGMII1_MODE
        0xffffffff,    //CTC_CHIP_SERDES_USXGMII2_MODE
        0x00000003,    //CTC_CHIP_SERDES_XXVG_MODE
        0x00000003,    //CTC_CHIP_SERDES_LG_MODE
        0xffffffff     //CTC_CHIP_SERDES_100BASEFX_MODE
    };
    Hss28GMacroCfg_m hss_cfg;


    if(CTC_CHIP_MAX_SERDES_MODE <= mode)
    {
        return CTC_E_INVALID_PARAM;
    }
    if(0xffffffff == data_out_cfg[mode])
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, &fld_id, &(data_out_cfg[mode]), (void*)(&hss_cfg)));

    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_fill_12g_pll_info(sys_datapath_hss_attribute_t* p_hss_vec)
{
    uint8 lane_id = 0;
    sys_datapath_serdes_info_t* p_serdes_info = NULL;
    
    CTC_PTR_VALID_CHECK(p_hss_vec);
    
    for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
    {
        p_serdes_info = &(p_hss_vec->serdes_info[lane_id]);
        if((CTC_CHIP_SERDES_NONE_MODE == p_serdes_info->mode) || 
           (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes_info->mode))
        {
            continue;
        }
        switch(p_serdes_info->pll_sel)
        {
            case 1:
                (p_hss_vec->plla_ref_cnt)++;
                if(SYS_DATAPATH_CMU_IDLE == p_hss_vec->plla_mode)
                {
                    SYS_TSINGMA_12G_GET_CMU_TYPE(p_serdes_info->mode, FALSE, p_hss_vec->plla_mode);
                }
                break;
            case 2:
                (p_hss_vec->pllb_ref_cnt)++;
                if(SYS_DATAPATH_CMU_IDLE == p_hss_vec->pllb_mode)
                {
                    SYS_TSINGMA_12G_GET_CMU_TYPE(p_serdes_info->mode, FALSE, p_hss_vec->pllb_mode);
                }
                break;
            case 3:
                (p_hss_vec->pllc_ref_cnt)++;
                if(SYS_DATAPATH_CMU_IDLE == p_hss_vec->pllc_mode)
                {
                    SYS_TSINGMA_12G_GET_CMU_TYPE(p_serdes_info->mode, FALSE, p_hss_vec->pllc_mode);
                }
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
    }
    
    return CTC_E_NONE;
}


/**
 @brief get register index of Mac Mii and SharedPcs
*/
int32
_sys_tsingma_get_mac_mii_pcs_index(uint8 lchip, uint8 mac_tbl_id, uint8 serdes_id, uint8 pcs_mode, uint8* sgmac_idx,
                                              uint8* mii_idx, uint8* pcs_idx, uint8* internal_mac_idx)
{
    uint8 sgmac_id;
    uint8 mii_id;
    uint8 pcs_id;
    uint8 max_pcs_idx;

    CTC_MAX_VALUE_CHECK(mac_tbl_id, TM_MAC_TBL_NUM_PER_SLICE - 1);
    
    if((CTC_CHIP_SERDES_QSGMII_MODE   == pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == pcs_mode) ||
       (CTC_CHIP_SERDES_USXGMII1_MODE == pcs_mode) || (CTC_CHIP_SERDES_USXGMII2_MODE == pcs_mode))
    {
        max_pcs_idx = 12;
    }
    else
    {
        max_pcs_idx = 8;
    }

    sgmac_id = mac_tbl_id / 4;

    if (SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(pcs_mode))
    {
        if (11 < serdes_id)
        {
            return CTC_E_INVALID_PARAM;
        }

        if (pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
        {
            mii_id = serdes_id / 4 * 4;
            pcs_id = serdes_id / 4 * 4;
        }
        else
        {
            mii_id = serdes_id;
            pcs_id = serdes_id;
        }
    }
    else
    {
        mii_id = (SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(serdes_id)) ? ((serdes_id/4)*4) : ((serdes_id-12)/4+12);
        pcs_id = serdes_id / 4;
    }

    if((sgmac_id >= TM_SGMAC_NUM) || (mii_id >= TM_SHARED_MII_NUM) || (pcs_id >= max_pcs_idx))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Get index error! mac_id %u, sgmac_idx %u, mii_idx %u, pcs_idx %u\n", 
                        mac_tbl_id, sgmac_id, mii_id, pcs_id);
        return CTC_E_INVALID_PARAM;
    }

    SYS_USW_VALID_PTR_WRITE(sgmac_idx,        sgmac_id);
    SYS_USW_VALID_PTR_WRITE(mii_idx,          mii_id);
    SYS_USW_VALID_PTR_WRITE(pcs_idx,          pcs_id);
    SYS_USW_VALID_PTR_WRITE(internal_mac_idx, (mac_tbl_id % 4));

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_init_hss_pll_soft_table(uint8 lchip, uint8 serdes_id)
{
    uint8  index;
    uint8  tmp_lane  = 0;
    uint8  hss_idx   = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if(p_hss_vec != NULL)
    {
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_fill_12g_pll_info(p_hss_vec));
        }
        else
        {
            tmp_lane = 3;
            for(index = 0; index < 4; index++)
            {
                if((CTC_CHIP_SERDES_NONE_MODE != p_hss_vec->serdes_info[index].mode) && 
                   (CTC_CHIP_SERDES_XSGMII_MODE != p_hss_vec->serdes_info[index].mode))
                {
                    p_hss_vec->plla_ref_cnt++;
                    tmp_lane = index;
                }
            }
            if(SYS_DATAPATH_28G_CMU_IDLE == p_hss_vec->plla_mode)
            {
                SYS_TSINGMA_28G_GET_CMU_TYPE(p_hss_vec->serdes_info[tmp_lane].mode, FALSE, p_hss_vec->plla_mode);
            }
        }
    }

    return CTC_E_NONE;
}

/*
   Build serdes info for serdes mode config
   There are 3 things done in this function:
     1. Building serdes clocktree (pll operation).
     2. Mapping one serdes to a local (mac) port.
     3. Filling port_attr structure, which is the basis of all port operations.
*/
int32
sys_tsingma_datapath_build_serdes_info(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 group, uint8 is_dyn, 
                                                    uint8 cmu_id, uint8 init_flag, uint8 xpipe_en, uint16 ovclk_speed)
{
    uint8  hss_idx          = 0;
    uint8  hss_id           = 0;
    uint8  lane_id          = 0;
    uint8  index            = 0;
    uint8  port_num         = 0;
    uint8  chan_id          = 0;
    uint8  mac_id           = 0;
    uint8  need_add         = 0;
    uint8  speed_mode       = 0;
    uint8  slice_id         = 0;
    uint8  mac_tbl_id       = 0;
    uint8  width            = 0;
    uint8  div              = 0;
    uint16 lport            = 0;
    uint8  sgmac_idx        = 0;
    uint8  mii_idx          = 0;
    uint8  pcs_idx          = 0;
    uint8  internal_mac_idx = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t*    port_attr = NULL;

    CTC_ERROR_RETURN(sys_tsingma_datapath_check_mode_valid(lchip, serdes_id, mode));

    /*1. Build serdes clocktree (pll operation)*/

    /*hss_idx used to index vector, hss_id is actual id*/
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

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

        /*soft tables init hss info*/
        sal_memset(p_hss_vec, 0, sizeof(sys_datapath_hss_attribute_t));

        p_hss_vec->hss_id = hss_id;
        p_hss_vec->hss_type = SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id) ? SYS_DATAPATH_HSS_TYPE_28G : SYS_DATAPATH_HSS_TYPE_15G;
        need_add = 1;
    }
    /*soft tables init serdes info*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_rate(mode, p_hss_vec->hss_type, FALSE, &width, &div, NULL));
    if(HSS15G_LANE_NUM <= lane_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " lane id exceeds max value!\n");
        return CTC_E_INVALID_PARAM;
    }
    p_hss_vec->serdes_info[lane_id].lane_id   = lane_id;
    p_hss_vec->serdes_info[lane_id].mode      = mode;
    p_hss_vec->serdes_info[lane_id].group     = group;
    p_hss_vec->serdes_info[lane_id].is_dyn    = is_dyn;
    p_hss_vec->serdes_info[lane_id].pll_sel   = SYS_TSINGMA_CMUID_TO_PLLSEL(cmu_id);
    p_hss_vec->serdes_info[lane_id].bit_width = width;
    p_hss_vec->serdes_info[lane_id].rate_div  = div;
    p_hss_vec->serdes_info[lane_id].overclocking_speed = ovclk_speed;

    /*write hss 12G/28G pll tables    if this is the last lane of HSS, init soft table of hss pll info*/
    if(init_flag && SERDES_JUDGE_LAST_LANE_IN_HSS(serdes_id))
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_init_hss_pll_soft_table(lchip, serdes_id));
    }

    /*2. Map one serdes to a local (mac) port*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_serdes_mapping(lchip, serdes_id, mode, &lport, &mac_id, &chan_id, &speed_mode, &port_num, &mac_tbl_id));

    p_hss_vec->serdes_info[lane_id].lport    = lport;
    p_hss_vec->serdes_info[lane_id].port_num = port_num;

    /*3. Fill port_attr structure, which is the basis of all port operations*/
    for (index = 0; index < port_num; index++)
    {
        CTC_ERROR_RETURN(_sys_tsingma_get_mac_mii_pcs_index(lchip, (mac_tbl_id+index), serdes_id, mode, &sgmac_idx, 
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
        port_attr->tbl_id           = mac_tbl_id+index;
        port_attr->speed_mode       = speed_mode;
        port_attr->slice_id         = slice_id;
        port_attr->pcs_mode         = mode;
        port_attr->serdes_id        = serdes_id;
        port_attr->serdes_num       = 1;
        port_attr->is_serdes_dyn    = is_dyn;
        port_attr->an_fec           = (1 << CTC_PORT_FEC_TYPE_RS)|(1 << CTC_PORT_FEC_TYPE_BASER);
        port_attr->sgmac_idx        = sgmac_idx;
        port_attr->mii_idx          = mii_idx;
        port_attr->pcs_idx          = pcs_idx;
        port_attr->internal_mac_idx = internal_mac_idx;

        /* X-PIPE init/update */
        port_attr->xpipe_en    = xpipe_en;
        SYS_DATAPATH_GET_XPIPE_MAC(port_attr->mac_id, port_attr->emac_id, port_attr->pmac_id);
        SYS_DATAPATH_GET_XPIPE_MAC(port_attr->chan_id, port_attr->emac_chanid, port_attr->pmac_chanid);

        SYS_DATAPATH_GET_IFTYPE_WITH_MODE(mode, port_attr->interface_type);

        if (SYS_DATAPATH_NEED_EXTERN(mode))
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

    return CTC_E_NONE;
}

/*
   set serdes link dfe reset or release
*/
int32
sys_tsingma_datapath_set_dfe_reset(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    return CTC_E_NONE;
}

/*0x23[0]    cfg_dfe_pd           0-enable 1-disable  set 0 if loopback enabled*/
/*0x23[2]    cfg_dfeck_en         1-enable 0-disable                   */
/*0x23[3]    cfg_erramp_pd        0-enable 1-disable                   */
/*0x22[4:0]  cfg_dfetap_en        5'h1f-enable 5'h0-disable            */
/*0x1a[2]    cfg_pi_dfe_en        1-enable 0-disable                   */
/*0x30[0]    cfg_summer_en        1                                    */
/*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaHoldDfe  0*/
/*Hss12GLaneRxEqCfg[0~2].cfgHssL[0~7]Pcs2PmaEnDfeDig 1-enable 0-disable*/
/*0x31[2]    cfg_rstn_dfedig_gen1 1-enable 0-disable                   */
int32
sys_tsingma_datapath_set_dfe_en(uint8 lchip, uint8 serdes_id, uint8 enable)
{
    uint8  idx;
    uint8  hss_id     = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8  lane_id    = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    uint8  acc_id     = lane_id+3;
    uint8  addr[7]    = {0x23, 0x23, 0x23, 0x22, 0x1a, 0x30, 0x31};
    uint8  mask[7]    = {0xfe, 0xfb, 0xf7, 0xe0, 0xfb, 0xfe, 0xfb};
    uint8  val_en[7]  = {0x00, 0x01, 0x00, 0x1f, 0x01, 0x01, 0x01};
    uint8  val_dis[7] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00};
    uint8* p_val      = enable ? val_en : val_dis;
    uint32 tb_id      = Hss12GLaneRxEqCfg0_t + (Hss12GLaneRxEqCfg1_t-Hss12GLaneRxEqCfg0_t)*hss_id;
    uint32 step       = (Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaEnDfeDig_f-Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEnDfeDig_f)*lane_id;
    uint32 fld_id     = 0;
    uint32 cmd        = 0;
    uint32 value      = 0;
    ctc_chip_serdes_loopback_t loopback = {0};
    Hss12GLaneRxEqCfg0_m rx_cfg;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%u, enable:%u\n", serdes_id, enable);

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /*0x23[0] cfg_dfe_pd  always set 0 if loopback enabled*/
        loopback.serdes_id = serdes_id;
        loopback.mode = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)(&loopback)));
        if(1 == loopback.enable)
        {
            p_val[0] = 0x00;
        }
        /*write rgeisters 0~5*/
        for(idx = 0; idx < 6; idx++)
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr[idx], mask[idx], p_val[idx]));
        }

        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_cfg));
        value = enable ? 1 : 0;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEnDfeDig_f + step;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &rx_cfg);
        value = 0;
        fld_id = Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaHoldDfe_f + step;
        DRV_IOW_FIELD(lchip, tb_id, fld_id, &value, &rx_cfg);
        cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_cfg));

        /*write rgeisters 6*/
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr[6], mask[6], p_val[6]));
    }

    return CTC_E_NONE;
}

/*
    Set serdes mode interface, used for init and dynamic switch
    Notice:Before using the interface must using sys_tsingma_datapath_build_serdes_info to build serdes info
*/
int32
sys_tsingma_datapath_set_serdes_lane_cfg(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 cmu_id, uint8 ovclk_flag)
{
    uint8 hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8 lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_lane_misc_cfg(lchip, hss_id, lane_id, mode, cmu_id));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_clk_tree_cfg(lchip, hss_id, lane_id, mode));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_data_out_cfg(lchip, hss_id, lane_id, mode));
    }
    else
    {
        /*lane misc cfg    merge into additional cfg*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_28g_data_out_cfg(lchip, hss_id, lane_id, mode));
    }

    return CTC_E_NONE;
}

#define __DATAPATH__

/*
  Using for datapath init and dynamic, for network port
*/
STATIC int32
_sys_tsingma_datapath_set_netrx_buf_mgr(uint8 lchip, uint8 slice_id)
{
    uint16 lport = 0;
    uint8 txqm = 0;
    uint8 lowthrd = 0;
    uint32 tmp_val32 = 0;
    uint8 speed = 0;
    uint32 cmd = 0;
    uint8 i = 0;
    uint8 forloop = 0;
    uint8 tmp_port = 0;
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
            
            forloop = (port_attr->xpipe_en ? 2 : 1);
            for (i = 0; i < forloop; i++)
            {
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

                tmp_port = (port_attr->xpipe_en ? \
                    (i?(port_attr->pmac_id):(port_attr->emac_id)) : lport);
                cmd = DRV_IOR((DsNetRxBufAdmissionCtl_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, tmp_port, cmd, &buf_ctl);
                DRV_IOW_FIELD(lchip, DsNetRxBufAdmissionCtl_t, DsNetRxBufAdmissionCtl_cfgGuaranteeBufferNum_f, &tmp_val32, &buf_ctl);
                cmd = DRV_IOW((DsNetRxBufAdmissionCtl_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, tmp_port, cmd, &buf_ctl);
            }
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
_sys_tsingma_get_nettx_port_mode_by_speed(uint8 speed_mode, uint32* port_mode)
{
    if(CTC_PORT_SPEED_MAX <= speed_mode)
    {
        return CTC_E_INVALID_PARAM;
    }
    if(NULL == port_mode)
    {
        return CTC_E_INVALID_PTR;
    }
    
    switch(speed_mode)
    {
        case CTC_PORT_SPEED_10M:
        case CTC_PORT_SPEED_100M:
        case CTC_PORT_SPEED_1G:
        case CTC_PORT_SPEED_2G5:
            *port_mode = 0x01;
            break;
        case CTC_PORT_SPEED_10G:
        case CTC_PORT_SPEED_20G:
            *port_mode = 0x02;
            break;
        case CTC_PORT_SPEED_25G:
            *port_mode = 0x03;
            break;
        case CTC_PORT_SPEED_40G:
            *port_mode = 0x04;
            break;
        case CTC_PORT_SPEED_50G:
            *port_mode = 0x05;
            break;
        case CTC_PORT_SPEED_100G:
            *port_mode = 0x06;
            break;
        default:
            *port_mode = 0x00;
            break;
    }
    
    return CTC_E_NONE;
}

/*
  Using for datapath init and dynamic, for network port cfg Nettx dynamic and STATIC info
  if p_end == NULL,means dynamic
*/
STATIC int32
_sys_tsingma_datapath_init_nettx_buf(uint8 lchip, uint8 slice_id)
{
    uint8  mac_id        = 0;
    uint8  txqm_id       = 0;
    uint16 lport         = 0;
    uint16 s_pointer     = 0;
    uint16 e_poniter     = 0;
    uint32 cmd           = 0;
    uint32 tmp_val32     = 0;
    uint32 nettx_bufsize = 0;
    sys_datapath_lport_attr_t*        port_attr   = NULL;
    NetTxPortStaticInfo_m             static_info = {{0}};
    NetTxPortDynamicInfo_m            dyn_info    = {{0}};
    EpeScheduleToNet0CreditConfigRa_m epe_credit  = {{0}};

    for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
    {
        if (txqm_id < mac_id / 16)
        {
            s_pointer = 0;
            txqm_id = mac_id / 16;
        }
        nettx_bufsize = g_tsingma_nettx_bufsize[mac_id];

        /*table operations*/
        cmd = DRV_IOR((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &static_info);

        e_poniter = nettx_bufsize ? (s_pointer + nettx_bufsize - 1) : s_pointer;
        if (e_poniter >= NETTX_MEM_MAX_DEPTH_PER_TXQM)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"nettx mac_id:%d, e-pointer:%d \n", mac_id, e_poniter);
            continue;
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

        cmd = DRV_IOR((EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*2+slice_id*8), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id%16, cmd, &epe_credit);

        DRV_IOW_FIELD(lchip, (EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*2+slice_id*8),
            EpeScheduleToNet0CreditConfigRa_cfgCreditValue_f, &nettx_bufsize, &epe_credit);
        cmd = DRV_IOW((EpeScheduleToNet0CreditConfigRa_t+(mac_id/16)*2+slice_id*8), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id%16, cmd, &epe_credit);

         /*config NetTxPortStaticInfo_portMode*/
        lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, mac_id);
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if ((NULL != port_attr) && (SYS_DMPS_NETWORK_PORT == port_attr->port_type))
        {
            CTC_ERROR_RETURN(_sys_tsingma_get_nettx_port_mode_by_speed(port_attr->speed_mode, &tmp_val32));
            DRV_IOW_FIELD(lchip, (NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f, &tmp_val32, &static_info);
        }
        else
        {
            tmp_val32 = 0;
            DRV_IOW_FIELD(lchip, (NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f, &tmp_val32, &static_info);
        }
        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mac_id, cmd, &static_info);
    }

    return CTC_E_NONE;
}

uint8
_sys_tsingma_datapath_get_msg_entry_num(uint8 chan_id)
{
    uint8 msg_entry_num[TM_MAX_CHAN_ID_NUM] = {
        6,  3,  3,  3,  3,  3,  3,  3,  6,  3,  3,  3,  6,  3,  3,  3,
        3,  3,  3,  3,  6,  3,  3,  3,  6,  3,  3,  3,  6,  3,  6,  3,
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 10,  4,  6,  4,
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 10,  4,  6,  4,
        3,  3,  3,  3,  3,  3,  3,  3,  3, 10, 10, 10,  0, 10,  3,  3,
        3,  3,  3
    };
    return msg_entry_num[chan_id];
}

/*
   Using for datapath init and dynamic, Notice this interface only used to set network port channel
*/
STATIC int32
_sys_tsingma_datapath_set_qmgr_credit(uint8 lchip, uint8 chan_id, uint8 slice_id, uint8 is_dyn)
{
    uint32 cmd = 0;
    uint8 step = 0;
    uint32 tmp_val32 = 0;
    DsQMgrChanCredit_m ra_credit = {{0}};

    if (is_dyn)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR((DsQMgrChanCredit_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &ra_credit);

    step = _sys_tsingma_datapath_get_msg_entry_num(chan_id);
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
_sys_tsingma_datapath_set_bufretrv_pkt_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16* p_end)
{
    uint32 cmd = 0;
    uint8 step = 0;
    uint16 e_pointer = 0;
    uint32 tmp_val32 = 0;
    BufRetrvPktConfigMem_m bufretrv_cfg = {{0}};
    BufRetrvPktStatusMem_m bufretrv_status = {{0}};

    if (s_pointer >= BUFRETRV_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"bufretrv pkt mem chan_id: %d, s-pointer: %d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    step = _sys_tsingma_datapath_get_msg_entry_num(chan_id);

     /* cfg below is used for init */
    if(NULL != p_end)
    {
        e_pointer = step ? (s_pointer + step -1) : s_pointer;
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
_sys_tsingma_datapath_set_bufretrv_credit_config(uint8 lchip, uint8 slice_id)
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

    for (i = 0; i < sizeof(g_tsingma_bufsz_step)/sizeof(g_tsingma_bufsz_step[0]); i++)
    {
        if ((g_tsingma_bufsz_step[i].br_credit_cfg >= 4) && (g_tsingma_bufsz_step[i].br_credit_cfg <= 8))
        {
            fld_id = BufRetrvEpeScheduleCreditConfig_bodyCreditConfig0_f + g_tsingma_bufsz_step[i].br_credit_cfg;
            value = g_tsingma_bufsz_step[i].body_buf_num;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &credit_cfg);
            fld_id = BufRetrvEpeScheduleCreditConfig_sopCreditConfig0_f + g_tsingma_bufsz_step[i].br_credit_cfg;
            if (g_tsingma_bufsz_step[i].sop_buf_num < 3)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] BufRetrvEpeSchedule sop Credit is too low \n");
                return CTC_E_HW_FAIL;
            }
            else if (g_tsingma_bufsz_step[i].sop_buf_num == 3)
            {
                value = g_tsingma_bufsz_step[i].sop_buf_num + 1;
            }
            else
            {
                value = g_tsingma_bufsz_step[i].sop_buf_num;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &credit_cfg);
        }
    }

    cmd = DRV_IOW((BufRetrvEpeScheduleCreditConfig_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &credit_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_fill_bufsz_step(uint32 step_flag, sys_datapath_bufsz_step_t *step)
{
    if(NULL == step || SPEED_MODE_INV < step_flag)
    {
        return CTC_E_INVALID_PARAM;
    }
    step->body_buf_num   = g_tsingma_bufsz_step[step_flag].body_buf_num;
    step->sop_buf_num    = g_tsingma_bufsz_step[step_flag].sop_buf_num;
    step->br_credit_cfg  = g_tsingma_bufsz_step[step_flag].br_credit_cfg;
    return CTC_E_NONE;
}

/*For net port only*/
STATIC int32
_sys_tsingma_datapath_get_net_bufsz_step_by_port_attr(uint8 lchip, sys_datapath_lport_attr_t* port_attr, sys_datapath_bufsz_step_t *step)
{
    uint32 flag = 0;

    switch (port_attr->speed_mode)
    {
        case CTC_PORT_SPEED_100G:
            flag = SPEED_MODE_100;
            break;
        case CTC_PORT_SPEED_50G:
            flag = SPEED_MODE_50;
            break;
        case CTC_PORT_SPEED_40G:
        case CTC_PORT_SPEED_20G:
            flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_100 : SPEED_MODE_40);
            break;
        case CTC_PORT_SPEED_25G:
            flag = SPEED_MODE_25;
            break;
        case CTC_PORT_SPEED_10G:
            if(CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode)
            {
                flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_100 : SPEED_MODE_40);
            }
            else
            {
                if ((1 == SDK_WORK_PLATFORM) && p_usw_datapath_master[lchip]->wlan_enable)
                {
                    flag = SPEED_MODE_10;
                }
                else
                {
                    flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_25 : SPEED_MODE_10);
                }
            }
            break;
        case CTC_PORT_SPEED_5G:
            if(SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(port_attr->pcs_mode) && 
               (0 != port_attr->tbl_id / 4) && (2 != port_attr->tbl_id / 4) && (6 != port_attr->tbl_id / 4))
            {
                flag = SPEED_MODE_5;
            }
            else
            {
                flag = SPEED_MODE_10;
            }
            break;
        case CTC_PORT_SPEED_10M:
        case CTC_PORT_SPEED_100M:
        case CTC_PORT_SPEED_1G:
        case CTC_PORT_SPEED_2G5:
            if(SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(port_attr->pcs_mode) && 
               (0 != port_attr->tbl_id / 4) && (2 != port_attr->tbl_id / 4) && (6 != port_attr->tbl_id / 4))
            {
                flag = SPEED_MODE_0_5;
            }
            else
            {
                flag = (SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id) ? SPEED_MODE_25 : SPEED_MODE_10);
            }
            break;
        default:
            flag = SPEED_MODE_INV;
            break;
    }

    CTC_ERROR_RETURN(_sys_tsingma_datapath_fill_bufsz_step(flag, step));
    return CTC_E_NONE;
}

/*For misc channel only*/
STATIC int32
_sys_tsingma_datapath_get_misc_bufsz_step_by_channid(uint8 lchip, uint8 chan_id, sys_datapath_bufsz_step_t *step)
{
    uint32 flag = 0;

    if((MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT) == chan_id) ||
       (MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT) == chan_id) ||
       (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == chan_id))
    {
        flag = SPEED_MODE_MISC_100;
    }
    else
    {
        flag = SPEED_MODE_MISC_10;
    }

    CTC_ERROR_RETURN(_sys_tsingma_datapath_fill_bufsz_step(flag, step));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_get_bufsz_step_by_chanid(uint8 lchip, uint8 chan_id, sys_datapath_bufsz_step_t *step)
{
    uint8  slice_id = 0;
    uint16 lport    = 0;
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

    if (SYS_DMPS_NETWORK_PORT != port_attr->port_type)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_misc_bufsz_step_by_channid(lchip, chan_id, step));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_net_bufsz_step_by_port_attr(lchip, port_attr, step));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_bufretrv_credit(uint8 lchip, uint8 slice_id, uint8 chan_id)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint32 tmp_val32 = 0;
    BufRetrvCreditConfigMem_m credit_cfg = {{0}};
    sys_datapath_bufsz_step_t step = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_bufsz_step_by_chanid(lchip, chan_id, &step));
    if (-1 == step.br_credit_cfg)
    {
        return CTC_E_NONE;
    }

    tmp_val32 = step.br_credit_cfg;
    DRV_IOW_FIELD(lchip, BufRetrvCreditConfigMem_t, BufRetrvCreditConfigMem_epeIntfCreditConfig_f, &tmp_val32, &credit_cfg);
    cmd = DRV_IOW((BufRetrvCreditConfigMem_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &credit_cfg);

    if (chan_id >= SYS_PHY_PORT_NUM_PER_SLICE)
    {
        return CTC_E_NONE;
    }
    lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan_id);
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        return CTC_E_NONE;
    }
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr || (SYS_DMPS_RSV_PORT == port_attr->port_type))
    {
        return CTC_E_NONE;
    }
    
    if (port_attr->xpipe_en)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_bufsz_step_by_chanid(lchip, port_attr->chan_id, &step));
        tmp_val32 = step.br_credit_cfg;
        DRV_IOW_FIELD(lchip, BufRetrvCreditConfigMem_t, BufRetrvCreditConfigMem_epeIntfCreditConfig_f, &tmp_val32, &credit_cfg);
        cmd = DRV_IOW((BufRetrvCreditConfigMem_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, (port_attr->emac_chanid), cmd, &credit_cfg);

        DRV_IOW_FIELD(lchip, BufRetrvCreditConfigMem_t, BufRetrvCreditConfigMem_epeIntfCreditConfig_f, &tmp_val32, &credit_cfg);
        cmd = DRV_IOW((BufRetrvCreditConfigMem_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &credit_cfg);
    }
    else
    {
        if (0xff != port_attr->pmac_chanid)
        {
            tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, port_attr->pmac_id, slice_id);
            if (tmp_port_attr)
            {
                if ((CTC_CHIP_SERDES_NONE_MODE != tmp_port_attr->pcs_mode) &&
                    (CTC_CHIP_SERDES_XSGMII_MODE != tmp_port_attr->pcs_mode))/* restore to normal port */
                {
                    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_bufsz_step_by_chanid(lchip, tmp_port_attr->chan_id, &step));
                    tmp_val32 = step.br_credit_cfg;
                }
                else
                {
                    tmp_val32 = 0;
                }
                DRV_IOW_FIELD(lchip, BufRetrvCreditConfigMem_t, BufRetrvCreditConfigMem_epeIntfCreditConfig_f, &tmp_val32, &credit_cfg);
                cmd = DRV_IOW((BufRetrvCreditConfigMem_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &credit_cfg);
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_nettx_set_TxThrd(uint8 lchip, uint8 slice_id)
{
    uint8 i = 0;
    uint32 cmd = 0;
    uint8 txThrd[7] = {3, 3, 6, 6, 8, 10, 6};
    uint32 tmp_val32 = 0;
    NetTxTxThrdCfg_m thrd_cfg;

    sal_memset(&thrd_cfg, 0, sizeof(NetTxTxThrdCfg_m));

    cmd = DRV_IOR((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);
    for (i = 0; i <= 6; i++)
    {
        tmp_val32 = txThrd[i];
        DRV_IOW_FIELD(lchip, NetTxTxThrdCfg_t, NetTxTxThrdCfg_txThrd0_f+i, &tmp_val32, &thrd_cfg);
    }
    cmd = DRV_IOW((NetTxTxThrdCfg_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &thrd_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_nettx_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 txqm_id = 0;
    uint8 slice_id = 0;
    NetTxMiscCtl_m misc_ctl;
    uint32 tmp_val32 = 0;

    sal_memset(&misc_ctl, 0, sizeof(NetTxMiscCtl_m));

    /*alloc buffer by actual status, but if serdes_num = 4, extern rsv port don't alloc buffer, while if port 2 is rsv port, port 2 need alloc buffer*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_init_nettx_buf(lchip, slice_id));
        
        for (txqm_id = 0; txqm_id < 4; txqm_id++)
        {
            CTC_ERROR_RETURN(sys_usw_datapath_set_calendar_cfg(lchip, slice_id, txqm_id));
        }
        CTC_ERROR_RETURN(_sys_tsingma_datapath_nettx_set_TxThrd(lchip, slice_id));
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

STATIC int32
_sys_tsingma_datapath_set_epe_body_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16* p_end)
{
    uint16 e_pointer        = 0;
    uint32 cmd              = 0;
    uint32 tmp_val32        = 0;
    uint32 remained_bodyram = 0;
    uint32 tmp_body_buf     = 0;
    uint16 s_pointer_fix    = 0;
    sys_datapath_bufsz_step_t step = {0};
    EpeScheduleBodyRamConfigRa_m body_ram_cfg = {{0}};
    EpeScheduleBodyRamRdPtrRa_m body_rd_ptr = {{0}};
    EpeScheduleBodyRamWrPtrRa_m body_wr_ptr = {{0}};

    if(SYS_TSINGMA_EPESCH_MAX_CHANNEL < chan_id)
    {
        return CTC_E_NONE;
    }
    s_pointer_fix = ((0 == chan_id % 4) ? g_tsingma_epesch_mem_origin[chan_id/4] : s_pointer);

    if (s_pointer_fix >= EPE_BODY_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id:%d, s-pointer:%d \n", chan_id, s_pointer_fix);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR((EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgEndPtr_f);
    DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_ELOOP), cmd, &tmp_val32);
    remained_bodyram = EPE_BODY_MEM_MAX_DEPTH - tmp_val32 - 1;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_bufsz_step_by_chanid(lchip, chan_id, &step));

    if ((chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
    {
        tmp_body_buf = (remained_bodyram >= 2*step.body_buf_num) ? (step.body_buf_num) : (remained_bodyram/2);
        step.body_buf_num = tmp_body_buf;
    }

    if (NULL != p_end)
    {
        e_pointer = (step.body_buf_num) ? (s_pointer_fix+(step.body_buf_num)-1) : s_pointer_fix;
        if (e_pointer >= EPE_BODY_MEM_MAX_DEPTH)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id: %d, e-pointer:%d \n", chan_id, e_pointer);
            return CTC_E_INVALID_PARAM;
        }

        tmp_val32 = 1;
        DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_txThrdSel_f, &tmp_val32, &body_ram_cfg);
        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgStartPtr_f, &tmp_val32, &body_ram_cfg);

        tmp_val32 = e_pointer;
        DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgEndPtr_f, &tmp_val32, &body_ram_cfg);
        cmd = DRV_IOW((EpeScheduleBodyRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &body_ram_cfg);

        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamRdPtrRa_t+slice_id), EpeScheduleBodyRamRdPtrRa_rdPtrUsed_f, &tmp_val32, &body_rd_ptr);
        cmd = DRV_IOW((EpeScheduleBodyRamRdPtrRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &body_rd_ptr);

        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleBodyRamWrPtrRa_t+slice_id), EpeScheduleBodyRamWrPtrRa_wrPtrUsed_f, &tmp_val32, &body_wr_ptr);
        cmd = DRV_IOW((EpeScheduleBodyRamWrPtrRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &body_wr_ptr);

        *p_end = (step.body_buf_num)? (e_pointer + 1) : e_pointer;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_epe_sop_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16 s_pointer, uint16* p_end)
{
    uint32 cmd = 0;
    uint16 e_pointer = 0;
    uint32 tmp_val32 = 0;
    uint32 remained_sopram = 0;
    uint32 tmp_sop_buf = 0;
    uint16 s_pointer_fix = 0;
    EpeScheduleSopRamConfigRa_m sop_ram_cfg;
    EpeScheduleSopRamRdPtrRa_m sop_rd_ptr;
    EpeScheduleSopRamWrPtrRa_m sop_wr_ptr;
    sys_datapath_bufsz_step_t step;

    if(SYS_TSINGMA_EPESCH_MAX_CHANNEL < chan_id)
    {
        return CTC_E_NONE;
    }
    s_pointer_fix = ((0 == chan_id % 4) ? (2*g_tsingma_epesch_mem_origin[chan_id/4]) : s_pointer);

    sal_memset(&sop_ram_cfg, 0 ,sizeof(EpeScheduleSopRamConfigRa_m));
    sal_memset(&step, 0, sizeof(sys_datapath_bufsz_step_t));

    if (s_pointer_fix >= EPE_SOP_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_sop chan_id:%d, s-pointer:%d \n", chan_id, s_pointer_fix);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR((EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgEndPtr_f);
    DRV_IOCTL(lchip, MCHIP_CAP(SYS_CAP_CHANID_ELOOP), cmd, &tmp_val32);
    remained_sopram = EPE_SOP_MEM_MAX_DEPTH - tmp_val32 - 1;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_bufsz_step_by_chanid(lchip, chan_id, &step));

    if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == chan_id)
    {
        step.sop_buf_num = 50;
    }
    else if ((chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
    {
        tmp_sop_buf = (remained_sopram >= 2*step.sop_buf_num) ? (step.sop_buf_num) : (remained_sopram/2);
        step.sop_buf_num = tmp_sop_buf;
    }

    if (NULL != p_end)
    {
        e_pointer = (step.sop_buf_num)? (s_pointer_fix+(step.sop_buf_num)-1) : s_pointer_fix;
        if (e_pointer >= EPE_SOP_MEM_MAX_DEPTH)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_sop chan_id:%d, e-pointer:%d \n", chan_id, e_pointer);
            return CTC_E_INVALID_PARAM;
        }

        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgStartPtr_f, &tmp_val32, &sop_ram_cfg);
        tmp_val32 = e_pointer;
        DRV_IOW_FIELD(lchip, (EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgEndPtr_f, &tmp_val32, &sop_ram_cfg);
        cmd = DRV_IOW((EpeScheduleSopRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &sop_ram_cfg);

        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleSopRamRdPtrRa_t+slice_id), EpeScheduleSopRamRdPtrRa_rdPtrUsed_f, &tmp_val32, &sop_rd_ptr);
        cmd = DRV_IOW((EpeScheduleSopRamRdPtrRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &sop_rd_ptr);

        tmp_val32 = s_pointer_fix;
        DRV_IOW_FIELD(lchip, (EpeScheduleSopRamWrPtrRa_t+slice_id), EpeScheduleSopRamWrPtrRa_wrPtrUsed_f, &tmp_val32, &sop_wr_ptr);
        cmd = DRV_IOW((EpeScheduleSopRamWrPtrRa_t+slice_id), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, chan_id, cmd, &sop_wr_ptr);

        *p_end = (step.sop_buf_num) ? (e_pointer + 1) : e_pointer;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_rsrv_epe_body_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16* p_end)
{
    uint16 e_pointer = 0;
    uint32 cmd       = 0;
    uint32 tmp_val32 = 0;
    uint16 s_pointer = g_tsingma_epesch_mem_origin[chan_id/4];
    EpeScheduleBodyRamConfigRa_m body_ram_cfg = {{0}};
    EpeScheduleBodyRamRdPtrRa_m body_rd_ptr   = {{0}};
    EpeScheduleBodyRamWrPtrRa_m body_wr_ptr   = {{0}};

    if (s_pointer >= EPE_BODY_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id:%d, s-pointer:%d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }

    if (NULL != p_end)
    {
        e_pointer = s_pointer;  /*This operation is incorrect. Our purpose is just reserve this memory.*/
        if (e_pointer >= EPE_BODY_MEM_MAX_DEPTH)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_body chan_id: %d, e-pointer:%d \n", chan_id, e_pointer);
            return CTC_E_INVALID_PARAM;
        }

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

        *p_end = e_pointer + 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_rsrv_epe_sop_mem(uint8 lchip, uint8 slice_id, uint8 chan_id, uint16* p_end)
{
    uint32 cmd       = 0;
    uint16 e_pointer = 0;
    uint32 tmp_val32 = 0;
    uint16 s_pointer = 2*g_tsingma_epesch_mem_origin[chan_id/4];
    EpeScheduleSopRamConfigRa_m sop_ram_cfg = {{0}};
    EpeScheduleSopRamRdPtrRa_m sop_rd_ptr   = {{0}};
    EpeScheduleSopRamWrPtrRa_m sop_wr_ptr   = {{0}};

    if (s_pointer >= EPE_SOP_MEM_MAX_DEPTH)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"epe_sop chan_id:%d, s-pointer:%d \n", chan_id, s_pointer);
        return CTC_E_INVALID_PARAM;
    }
    
    if (NULL != p_end)
    {
        e_pointer = s_pointer;  /*This operation is incorrect. Our purpose is just reserve this memory.*/
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

        *p_end = e_pointer + 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_epe_wrr_weight(uint8 lchip, uint8 slice_id)
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
    uint16 cg_speed[2] = {0};
    uint16 net_speed[4] = {0};
    uint16 misc_speed = 0;
    uint16 net_speed_all = 0;
    uint32 tmp_val32 = 0;
    uint32 wrrbase = 0;
    uint32 fld_id = 0;
    uint16 array[3][84];/*xg, xlg, group*/
    uint8 i = 0;
    uint8 forloop = 0;
    uint16 tmp_port = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;
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
                forloop = (port_attr->xpipe_en ? 2 : 1);
                for (i = 0; i < forloop; i++)
                {
                    SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);
                    tmp_port = (port_attr->xpipe_en ? \
                        (i?(port_attr->pmac_id):(port_attr->emac_id)) : lport);
                    if ((port_attr->speed_mode <= CTC_PORT_SPEED_10G)||(CTC_PORT_SPEED_5G == port_attr->speed_mode))
                    {
                        xg_ct++;
                        xg_speed[lport/16] += wrrbase;
                        array[0][tmp_port] = wrrbase;
                    }
                    else if ((port_attr->speed_mode <= CTC_PORT_SPEED_50G)&&(port_attr->speed_mode != CTC_PORT_SPEED_100G))
                    {
                        if (port_attr->speed_mode == CTC_PORT_SPEED_25G)
                        {
                            xxvg_ct++;
                        }
                        xlg_speed[lport/16] += wrrbase;
                        array[0][tmp_port] = wrrbase;
                    }
                    else if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                    {
                        cg_speed[lport/16 - 2] += wrrbase;
                        array[0][tmp_port] = wrrbase;
                    }
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
            if (port_attr->chan_id < 64)
            {
                cmd = DRV_IOR((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_wt));
                DRV_IOW_FIELD(lchip, (EpeScheduleNetPortWeightConfigRa_t+slice_id), EpeScheduleNetPortWeightConfigRa_cfgWeight_f,
                    &tmp_val32, &port_wt);
                cmd = DRV_IOW((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_wt));
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

        for (tmp_port = 0; tmp_port < SYS_PHY_PORT_NUM_PER_SLICE; tmp_port++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_port, slice_id);
            if (!port_attr || (SYS_DMPS_NETWORK_PORT != port_attr->port_type))
            {
                continue;
            }
            
            if (port_attr->xpipe_en)
            {
                tmp_val32 = array[0][tmp_port]/divisor[0];
                
                cmd = DRV_IOR((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, port_attr->emac_id, cmd, &port_wt);
                DRV_IOW_FIELD(lchip, (EpeScheduleNetPortWeightConfigRa_t+slice_id), EpeScheduleNetPortWeightConfigRa_cfgWeight_f, 
                    &tmp_val32, &port_wt);
                cmd = DRV_IOW((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, port_attr->emac_id, cmd, &port_wt);

                cmd = DRV_IOR((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &port_wt);
                DRV_IOW_FIELD(lchip, (EpeScheduleNetPortWeightConfigRa_t+slice_id), EpeScheduleNetPortWeightConfigRa_cfgWeight_f, 
                    &tmp_val32, &port_wt);
                cmd = DRV_IOW((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &port_wt);
            }
            else
            {
                if (0xff != port_attr->pmac_id)
                {
                    tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, port_attr->pmac_id, slice_id);
                    if (tmp_port_attr)
                    {
                        if ((CTC_CHIP_SERDES_NONE_MODE != tmp_port_attr->pcs_mode) &&
                            (CTC_CHIP_SERDES_XSGMII_MODE != tmp_port_attr->pcs_mode))  /* restore to normal port */
                        {
                            tmp_val32 = array[0][tmp_port]/divisor[0];
                        }
                        else
                        {
                            tmp_val32 = 0;
                        }
                        
                        cmd = DRV_IOR((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &port_wt));
                        DRV_IOW_FIELD(lchip, (EpeScheduleNetPortWeightConfigRa_t+slice_id), EpeScheduleNetPortWeightConfigRa_cfgWeight_f,
                            &tmp_val32, &port_wt);
                        cmd = DRV_IOW((EpeScheduleNetPortWeightConfigRa_t+slice_id), DRV_ENTRY_FLAG);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &port_wt));
                    }
                }
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
        for (txqm = 0; txqm < 2; txqm++)
        {
            array[2][array_ct++] = cg_speed[txqm];
        }
        array[2][array_ct++] = net_speed_all;
        array[2][array_ct++] = misc_speed;
        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        for (txqm = 0; txqm < 2; txqm++)
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
_sys_tsingma_datapath_epesched_xpipe_per_chan(uint8 lchip, uint8 chan)
{
    uint8 slice_id = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint8 base_chan = 0;
    uint8 dst_chan = 0;
    uint16 s_pointer1 = 0; /* body */
    uint16 e_pointer1 = 0;
    uint16 s_pointer2 = 0; /* sop */
    uint16 e_pointer2 = 0;
    uint32 body_step = 0;
    uint32 sop_step = 0;
    uint8 speed_mode_level = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    EpeScheduleBodyRamConfigRa_m body_ram_cfg = {{0}};
    EpeScheduleSopRamConfigRa_m sop_ram_cfg = {{0}};

    lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, chan);
    if (SYS_COMMON_USELESS_MAC == lport)
    {
        return CTC_E_NONE;
    }
    
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if ((!port_attr) || (SYS_DMPS_RSV_PORT == port_attr->port_type))
    {
        return CTC_E_NONE;
    }
    if (!port_attr->xpipe_en)
    {
        return CTC_E_NONE;
    }

    switch (chan)
    {
    case 0:
    case 1:
        base_chan = 4;
        dst_chan = chan+4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 2:
    case 3:
        base_chan = 32;
        dst_chan = chan+4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 20:
    case 21:
        base_chan = 16;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 22:
    case 23:
        base_chan = 48;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        base_chan = 8;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 28:
    case 29:
    case 30:
    case 31:
        base_chan = 24;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_10;
        break;
    case 44:
    case 45:
    case 46:
    case 47:
        base_chan = 40;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_25;
        break;
    case 60:
    case 61:
    case 62:
    case 63:
        base_chan = 56;
        dst_chan = chan-4;
        speed_mode_level = SPEED_MODE_25;
        break;
    default:
        base_chan = 0xff;
        dst_chan = 0xff;
        speed_mode_level = SPEED_MODE_INV;
        break;
    }

    /* Body */
    body_step = g_tsingma_bufsz_step[speed_mode_level].body_buf_num;
    cmd = DRV_IOR((EpeScheduleBodyRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, base_chan, cmd, &body_ram_cfg);
    s_pointer1 = GetEpeScheduleBodyRamConfigRa(V, cfgStartPtr_f, &body_ram_cfg);
    if ((2 == chan) || (3 == chan) || (22 == chan) || (23 == chan))
    {
        s_pointer1 += body_step * (chan%2);
    }
    else
    {
        s_pointer1 += body_step * (chan%4);
    }
    SetEpeScheduleBodyRamConfigRa(V, cfgStartPtr_f, &body_ram_cfg, s_pointer1);
    e_pointer1 = s_pointer1 + body_step - 1;
    SetEpeScheduleBodyRamConfigRa(V, cfgEndPtr_f, &body_ram_cfg, e_pointer1);
    cmd = DRV_IOW((EpeScheduleBodyRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, dst_chan, cmd, &body_ram_cfg);

    /* Sop */
    sop_step = g_tsingma_bufsz_step[speed_mode_level].sop_buf_num;
    cmd = DRV_IOR((EpeScheduleSopRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, base_chan, cmd, &sop_ram_cfg);
    s_pointer2 = GetEpeScheduleSopRamConfigRa(V, cfgStartPtr_f, &sop_ram_cfg);
    if ((2 == chan) || (3 == chan) || (22 == chan) || (23 == chan))
    {
        s_pointer2 += sop_step * (chan%2);
    }
    else
    {
        s_pointer2 += sop_step * (chan%4);
    }
    SetEpeScheduleSopRamConfigRa(V, cfgStartPtr_f, &sop_ram_cfg, s_pointer2);
    e_pointer2 = s_pointer2 + sop_step - 1;
    SetEpeScheduleSopRamConfigRa(V, cfgEndPtr_f, &sop_ram_cfg, e_pointer2);
    cmd = DRV_IOW((EpeScheduleSopRamConfigRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, dst_chan, cmd, &sop_ram_cfg);

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_epe_xpipe_init(uint8 lchip)
{
    uint8 chan = 0;
    
    for (chan = 0; chan < SYS_PHY_PORT_NUM_PER_SLICE; chan++)
    {
        _sys_tsingma_datapath_epesched_xpipe_per_chan(lchip, chan);
    }
    
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_epe_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint16 s_pointer1 = 0;
    uint16 s_pointer2 = 0;
    uint16 e_pointer1 = 0;
    uint16 e_pointer2 = 0;
    uint8 chan_id = 0;
    uint8 sec_enable = 0;
    uint8 wlan_enable = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    EpeScheduleNetPortToChanRa_m port_to_chan;
    EpeScheduleNetChanToPortRa_m chan_to_port;
    EpeHeaderAdjustPhyPortMap_m  adj_port_map;

    /*if wlan and mac sec enable, support max mode 48*2.5G+4*25G+2*40G*/
    wlan_enable = p_usw_datapath_master[lchip]->wlan_enable;
    sec_enable = p_usw_datapath_master[lchip]->dot1ae_enable;

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
            DRV_IOW_FIELD(lchip, (EpeHeaderAdjustPhyPortMap_t+slice_id), EpeHeaderAdjustPhyPortMap_localPhyPort_f,
                &tmp_val32, &adj_port_map);
            cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t+slice_id, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->chan_id, cmd, &adj_port_map));

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
            
            if (SYS_DMPS_NETWORK_PORT == port_attr->port_type)
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_body_mem(lchip, slice_id, port_attr->chan_id, s_pointer1, &e_pointer1));
                s_pointer1 = e_pointer1;
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_sop_mem(lchip, slice_id, port_attr->chan_id, s_pointer2, &e_pointer2));
                s_pointer2 = e_pointer2;
            }
            /*Reserve EpeSch memory space for idle QMs (all ports in this QM are null).
              Just for 1st port in this QM.*/
            else if((SYS_DMPS_RSV_PORT == port_attr->port_type) && (0 == (port_attr->chan_id) % 4))
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_rsrv_epe_body_mem(lchip, slice_id, port_attr->chan_id, &e_pointer1));
                s_pointer1 = e_pointer1;
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_rsrv_epe_sop_mem(lchip, slice_id, port_attr->chan_id, &e_pointer2));
                s_pointer2 = e_pointer2;
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
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_body_mem(lchip, slice_id, chan_id, s_pointer1, &e_pointer1));
                s_pointer1 = e_pointer1;
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, s_pointer2, &e_pointer2));
                s_pointer2 = e_pointer2;
            }
        }

        if (sec_enable)
        {
            /* misc channel 73/74: MacEncrypt/MacDecrypt */
            for (chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT); chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT); chan_id++)
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_body_mem(lchip, slice_id, chan_id, s_pointer1, &e_pointer1));
                s_pointer1 = e_pointer1;
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, s_pointer2, &e_pointer2));
                s_pointer2 = e_pointer2;
            }
        }

        /* misc channel 75: Eloop */
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_body_mem(lchip, slice_id, MCHIP_CAP(SYS_CAP_CHANID_ELOOP), s_pointer1, &e_pointer1));
        s_pointer1 = e_pointer1;
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_sop_mem(lchip, slice_id, MCHIP_CAP(SYS_CAP_CHANID_ELOOP), s_pointer2, &e_pointer2));
        s_pointer2 = e_pointer2;

        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_misc_chan_init(uint8 lchip)
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
_sys_tsingma_datapath_net_chan_cfg(uint8 lchip)
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
    uint32 step = 0;
    uint8 i = 0;
    uint8 forloop = 0;
    uint8 tmp_chan = 0;
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

            forloop = (port_attr->xpipe_en ? 2 : 1);
            for (i = 0; i < forloop; i++)
            {
                tmp_chan = (port_attr->xpipe_en ? \
                        (i?(port_attr->pmac_chanid):(port_attr->emac_chanid)) : (port_attr->chan_id));
                if ((SYS_DMPS_RSV_PORT == port_attr->port_type)
                    && (port_attr->chan_id < SYS_PHY_PORT_NUM_PER_SLICE))
                {
                    CTC_BMP_UNSET(xg_bitmap, tmp_chan);
                    CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                    if((port_attr->chan_id == 44) || (port_attr->chan_id == 60))  /* TM is different from D2*/
                    {
                        CLEAR_BIT(cg_bitmap, port_attr->chan_id / 60);
                    }
                }

                if ((CTC_PORT_SPEED_1G == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_100M == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_10M == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_2G5 == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_10G == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_5G == port_attr->speed_mode))
                {
                    CTC_BMP_SET(xg_bitmap, tmp_chan);
                    CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                    if((port_attr->chan_id == 44) || (port_attr->chan_id == 60))
                    {
                        CLEAR_BIT(cg_bitmap, port_attr->chan_id / 60);
                    }
                }
                else if ((CTC_PORT_SPEED_20G == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_40G == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_25G == port_attr->speed_mode)
                    || (CTC_PORT_SPEED_50G == port_attr->speed_mode))
                {
                    CTC_BMP_UNSET(xg_bitmap, port_attr->chan_id);
                    CTC_BMP_SET(xlg_bitmap, port_attr->chan_id);
                    if((port_attr->chan_id == 44) || (port_attr->chan_id == 60))
                    {
                        CLEAR_BIT(cg_bitmap, port_attr->chan_id / 60);
                    }
                }
                else if (CTC_PORT_SPEED_100G == port_attr->speed_mode)
                {
                    CTC_BMP_UNSET(xg_bitmap, port_attr->chan_id);
                    CTC_BMP_UNSET(xlg_bitmap, port_attr->chan_id);
                    SET_BIT(cg_bitmap, port_attr->chan_id / 60);

                    tmp_val32 = port_attr->chan_id;
                    step = ((port_attr->chan_id >= 44) && (port_attr->chan_id <= 47)) ? 0 : 1;
                    DRV_IOW_FIELD(lchip, QMgrDeqChanIdCfg_t, (QMgrDeqChanIdCfg_cfgCg0ChanId_f+step), &tmp_val32, &qmgr_chanid_cfg);
                    DRV_IOW_FIELD(lchip, BufRetrvChanIdCfg_t, (BufRetrvChanIdCfg_cfgCg0ChanId_f+step), &tmp_val32, &buf_chanid_cfg);
                    DRV_IOW_FIELD(lchip, EpeScheduleChanIdCfg_t, (EpeScheduleChanIdCfg_cfgCg0PortId_f+step), &tmp_val32, &epe_chanid_cfg);
                }
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
        DRV_IOW_FIELD(lchip, EpeScheduleChanEnableCfg_t, EpeScheduleChanEnableCfg_cfgCgChanIdEnBmp_f, &tmp_val32, &epe_chanen_cfg);

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
_sys_tsingma_datapath_bufretrv_wrr_weight(uint8 lchip, uint8 slice_id)
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
    uint16 cg_speed[2] = {0};
    uint16 divisor[3] = {0};
    uint16 net_speed = 0;
    uint16 intf_speed = 0;
    uint16 misc_speed = 0;
    uint32 tmp_val32 = 0;
    uint32 wrrbase = 0;
    uint32 fld_id  = 0;
    uint16 array[3][84];/*xg, xlg, group*/
    uint8 i = 0;
    uint8 forloop = 0;
    uint8 tmp_chan = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;
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
                forloop = (port_attr->xpipe_en ? 2 : 1);
                for (i = 0; i < forloop; i++)
                {
                    SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);
                    tmp_chan = (port_attr->xpipe_en ? \
                        (i?(port_attr->pmac_chanid):(port_attr->emac_chanid)) : (port_attr->chan_id));
                    if ((port_attr->speed_mode <= CTC_PORT_SPEED_10G)||(CTC_PORT_SPEED_5G == port_attr->speed_mode))
                    {
                        xg_speed += wrrbase;
                        array[0][tmp_chan] = wrrbase;

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
                        array[0][tmp_chan] = wrrbase;

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
                        cg_speed[lport/16 - 2] += wrrbase;
                        array[0][tmp_chan] = wrrbase;
                    }
                }
            }
            else
            {
                if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)))
                {
                    array[0][port_attr->chan_id] = 10 * 10;
                    intf_speed += 10 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    intf_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ILOOP) == port_attr->chan_id)
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_OAM)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)))
                {
                    array[0][port_attr->chan_id] = 10 * 10;
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

            weight = array[0][port_attr->chan_id]/divisor[0];

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
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->chan_id, cmd, &wrr_cfg));
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
        
        for (tmp_chan = 0; tmp_chan < SYS_PHY_PORT_NUM_PER_SLICE; tmp_chan++)
        {
            lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, tmp_chan);
            if (SYS_DATAPATH_USELESS_MAC != lport)
            {
                port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
                if (!port_attr)
                {
                    continue;
                }
                if (port_attr->xpipe_en)
                {
                    tmp_val32 = array[0][tmp_chan]/divisor[0];
                    
                    cmd = DRV_IOR((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &bufretrv_cfgwt);
                    DRV_IOW_FIELD(lchip, (BufRetrvNetPktWtCfgMem_t+slice_id), BufRetrvNetPktWtCfgMem_weight_f, &tmp_val32, &bufretrv_cfgwt);
                    cmd = DRV_IOW((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &bufretrv_cfgwt);

                    cmd = DRV_IOR((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &bufretrv_cfgwt);
                    DRV_IOW_FIELD(lchip, (BufRetrvNetPktWtCfgMem_t+slice_id), BufRetrvNetPktWtCfgMem_weight_f, &tmp_val32, &bufretrv_cfgwt);
                    cmd = DRV_IOW((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &bufretrv_cfgwt);
                }
                else
                {
                    if (0xff != port_attr->pmac_chanid)
                    {
                        tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, port_attr->pmac_id, slice_id);
                        if (tmp_port_attr)
                        {
                            if (CTC_CHIP_SERDES_NONE_MODE != tmp_port_attr->pcs_mode)  /* restore to normal port */
                            {
                                tmp_val32 = array[0][tmp_chan]/divisor[0];
                            }
                            else
                            {
                                tmp_val32 = 0;
                            }
                            
                            cmd = DRV_IOR((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &bufretrv_cfgwt));
                            DRV_IOW_FIELD(lchip, (BufRetrvNetPktWtCfgMem_t+slice_id), BufRetrvNetPktWtCfgMem_weight_f, &tmp_val32, &bufretrv_cfgwt);
                            cmd = DRV_IOW((BufRetrvNetPktWtCfgMem_t+slice_id), DRV_ENTRY_FLAG);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &bufretrv_cfgwt));
                        }
                    }
                }
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
        for (txqm = 0; txqm < 2; txqm++)
        {
            array[2][array_ct++] = cg_speed[txqm];
        }
        array[2][array_ct++] = net_speed;
        array[2][array_ct++] = intf_speed;
        array[2][array_ct++] = misc_speed;

        CTC_ERROR_RETURN(_sys_usw_datapath_get_gcd(array[2], array_ct, &divisor[2]));

        for (txqm = 0; txqm < 2; txqm++)
        {
            tmp_val32 = cg_speed[txqm]/divisor[2];
            DRV_IOW_FIELD(lchip, BufRetrvPktMiscWeightConfig_t, (BufRetrvPktMiscWeightConfig_cfgCg0ChanWeight_f+txqm),
                &tmp_val32, &wrr_cfg);
        }

        tmp_val32 = net_speed/divisor[2];
        DRV_IOW_FIELD(lchip, (BufRetrvPktMiscWeightConfig_t+slice_id), BufRetrvPktMiscWeightConfig_cfgNetGroupWeight_f,
            &tmp_val32, &wrr_cfg);
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

        cmd = DRV_IOW((BufRetrvPktMiscWeightConfig_t+slice_id), DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wrr_cfg));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_datapath_bufretrv_init(uint8 lchip)
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
            if (((lport >= SYS_PHY_PORT_NUM_PER_SLICE) && (port_attr->chan_id == 0)) || (port_attr->chan_id >= TM_MAX_CHAN_ID_NUM))
            {
                continue;
            }

            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_pkt_mem(lchip, slice_id, port_attr->chan_id, s_pointer, &e_pointer));
            s_pointer = e_pointer;

            /*BufRetrvCreditConfigMem has 76 entries, Iloop,Oam,Dma don't flow to epe*/
            if (port_attr->chan_id < 76)
            {
                /*relate to BufRetrvEpeScheduleCreditConfig*/
                CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_credit(lchip, slice_id, port_attr->chan_id));
            }
        }

        /*init BufRetrvEpeScheduleCreditConfig and BufRetrvNetPktWtCfgMem*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_credit_config(lchip, slice_id));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_bufretrv_wrr_weight(lchip, slice_id));

       /* all channel is allocated by new method. */
        /*Dma channel now do not have lport, need distribute resource seperately*/
        for (index = 0; index < 4; index++)
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->dma_chan+index), s_pointer, &e_pointer));
             s_pointer = e_pointer;
        }

        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->elog_chan), s_pointer, &e_pointer));
         s_pointer = e_pointer;
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_pkt_mem(lchip, slice_id,
                (p_usw_datapath_master[lchip]->qcn_chan), s_pointer, &e_pointer));
         s_pointer = e_pointer;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_qmgr_wrr_weight(uint8 lchip, uint8 slice_id)
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
    uint8 i = 0;
    uint8 forloop = 0;
    uint8 tmp_chan = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;
    DsQMgrChanWeight_m wt_cfg;
    QMgrDeqIntfWrrCtl_m wrr_cfg;

    sal_memset(array, 0, 3 * 84 * sizeof(uint16));
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
                forloop = (port_attr->xpipe_en ? 2 : 1);
                for (i = 0; i < forloop; i++)
                {
                    SYS_DATAPATH_SPEED_TO_WRRCFG(port_attr->speed_mode, wrrbase);
                    tmp_chan = (port_attr->xpipe_en ? \
                        (i?(port_attr->pmac_chanid):(port_attr->emac_chanid)) : (port_attr->chan_id));
                    if (port_attr->speed_mode == CTC_PORT_SPEED_100G)
                    {
                        array[0][tmp_chan] = wrrbase;
                    }
                    else if ((port_attr->speed_mode == CTC_PORT_SPEED_50G)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_40G)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_25G))
                    {
                        array[0][tmp_chan] = wrrbase;
                        xlg_speed += wrrbase;
                    }
                    else if ((port_attr->speed_mode == CTC_PORT_SPEED_10G)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_5G)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_2G5)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_1G)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_100M)
                        || (port_attr->speed_mode == CTC_PORT_SPEED_10M))
                    {
                        array[0][tmp_chan] = wrrbase;
                        xg_speed += wrrbase;
                    }
                }
            }
            else
            {
                if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE)))
                {
                    array[0][port_attr->chan_id] = 10 * 10;
                    cpwp_speed += 10 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT)))
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    cpwp_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ELOOP) == port_attr->chan_id)
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if (MCHIP_CAP(SYS_CAP_CHANID_ILOOP) == port_attr->chan_id)
                {
                    array[0][port_attr->chan_id] = 100 * 10;
                    misc_speed += 100 * 10;
                }
                else if ((port_attr->chan_id >= MCHIP_CAP(SYS_CAP_CHANID_OAM)) && (port_attr->chan_id <= MCHIP_CAP(SYS_CAP_CHANID_DMA_RX3)))
                {
                    array[0][port_attr->chan_id] = 10 * 10;
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
                weight = array[0][port_attr->chan_id]/divisor[0];
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
                        weight = array[0][port_attr->chan_id]/divisor[0];
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

        for (tmp_chan = 0; tmp_chan < SYS_PHY_PORT_NUM_PER_SLICE; tmp_chan++)
        {
            lport = sys_usw_datapath_get_lport_with_chan(lchip, slice_id, tmp_chan);
            if (SYS_DATAPATH_USELESS_MAC != lport)
            {
                port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
                if (!port_attr)
                {
                    continue;
                }
                
                if (port_attr->xpipe_en)
                {
                    tmp_val32 = array[0][tmp_chan]/divisor[0];
                    
                    cmd = DRV_IOR((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &wt_cfg);
                    DRV_IOW_FIELD(lchip, (DsQMgrChanWeight_t+slice_id), DsQMgrChanWeight_weight_f, &tmp_val32, &wt_cfg);
                    cmd = DRV_IOW((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &wt_cfg);

                    cmd = DRV_IOR((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &wt_cfg);
                    DRV_IOW_FIELD(lchip, (DsQMgrChanWeight_t+slice_id), DsQMgrChanWeight_weight_f, &tmp_val32, &wt_cfg);
                    cmd = DRV_IOW((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &wt_cfg);
                }
                else
                {
                    if (0xff != port_attr->pmac_chanid)
                    {
                        tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, port_attr->pmac_id, slice_id);
                        if (tmp_port_attr)
                        {
                            if ((CTC_CHIP_SERDES_NONE_MODE != tmp_port_attr->pcs_mode) &&
                                (CTC_CHIP_SERDES_XSGMII_MODE != tmp_port_attr->pcs_mode))  /* restore to normal port */
                            {
                                tmp_val32 = array[0][tmp_chan]/divisor[0];
                            }
                            else
                            {
                                tmp_val32 = 0;
                            }
                            
                            cmd = DRV_IOR((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &wt_cfg));
                            DRV_IOW_FIELD(lchip, (DsQMgrChanWeight_t+slice_id), DsQMgrChanWeight_weight_f, &tmp_val32, &wt_cfg);
                            cmd = DRV_IOW((DsQMgrChanWeight_t+slice_id), DRV_ENTRY_FLAG);
                            CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &wt_cfg));
                        }
                    }
                }
            }
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
_sys_tsingma_datapath_qmgr_init(uint8 lchip)
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
            if (((lport >= SYS_PHY_PORT_NUM_PER_SLICE) && (!port_attr->chan_id)) || (port_attr->chan_id >= TM_MAX_CHAN_ID_NUM))
            {
                continue;
            }

            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_qmgr_credit(lchip, port_attr->chan_id, port_attr->slice_id, 0));
        }
        /*Dma channel now do not have lport, need distribute resource seperately*/
        for (index = 0; index < 4; index++)
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_qmgr_credit(lchip, (p_usw_datapath_master[lchip]->dma_chan+index), slice_id, 0));
        }

        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_qmgr_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_netrx_wrr_weight(uint8 lchip, uint8 slice_id)
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
    uint8 i = 0;
    uint8 forloop = 0;
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

            forloop = (port_attr->xpipe_en ? 2 : 1);
            for (i = 0; i < forloop; i++)
            {
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
_sys_tsingma_datapath_netrx_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint8 slice_id = 0;
    uint32 tmp_val32 = 0;
    NetRxChannelMap_m chan_map;
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
            DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgPortToChanMapping_f, &tmp_val32, &chan_map);
            cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->mac_id, cmd, &chan_map);

            tmp_val32 = port_attr->mac_id;
            DRV_IOW_FIELD(lchip, (NetRxChannelMap_t+slice_id), NetRxChannelMap_cfgChanToPortMapping_f, &tmp_val32, &chan_map);
            cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &chan_map);

            /* channel to lport */
            tmp_val32 = lport;
            DRV_IOW_FIELD(lchip, (IpeHeaderAdjustPhyPortMap_t+slice_id), IpeHeaderAdjustPhyPortMap_localPhyPort_f, &tmp_val32, &port_map);
            cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &port_map);

            /* NetRxToIpeCombNum */
             /*tmp_val32 = 0x3;*/
             /*DRV_IOW_FIELD(lchip, (NetRxToIpeCombNum_t+slice_id), NetRxToIpeCombNum_data_f,*/
             /*    &tmp_val32, &comb_num, port_attr->mac_id);*/
             /*cmd = DRV_IOW((NetRxToIpeCombNum_t+slice_id), DRV_ENTRY_FLAG);*/
             /*DRV_IOCTL(lchip, port_attr->mac_id, cmd, &comb_num);*/
        }

        /* 2. init NetRxBufManagement */
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_buf_mgr(lchip, slice_id));

        /* 3. init NetRx Wrr weight*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_wrr_weight(lchip, slice_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_wlan_init(uint8 lchip)
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
            DRV_IOW_FIELD(lchip, (RaCpwpCipDecPlConfig_t+slice_id), RaCpwpCipDecPlConfig_startPtr_f, &tmp_val32, &pl_cfg);
            tmp_val32 = index * 20 + 19;
            DRV_IOW_FIELD(lchip, (RaCpwpCipDecPlConfig_t+slice_id), RaCpwpCipDecPlConfig_endPtr_f, &tmp_val32, &pl_cfg);
            cmd = DRV_IOW(RaCpwpCipDecPlConfig_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &pl_cfg);

            tmp_val32 = 36;
            DRV_IOW_FIELD(lchip, (RaCpwpCipInBufCreditDecCfg_t+slice_id), RaCpwpCipInBufCreditDecCfg_creditCfg_f, &tmp_val32, &credit_cfg);
            cmd = DRV_IOW(RaCpwpCipInBufCreditDecCfg_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &credit_cfg);

            tmp_val32 = 16 + index * 40;
            DRV_IOW_FIELD(lchip, (RaCpwpCipMidBufDecConfig_t+slice_id), RaCpwpCipMidBufDecConfig_startPtr_f, &tmp_val32, &buf_cfg);
            tmp_val32 = 55 + index * 40;
            DRV_IOW_FIELD(lchip, (RaCpwpCipMidBufDecConfig_t+slice_id), RaCpwpCipMidBufDecConfig_endPtr_f, &tmp_val32, &buf_cfg);
            cmd = DRV_IOW(RaCpwpCipMidBufDecConfig_t+slice_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, index, cmd, &buf_cfg);
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_check_xpipe_param(uint8 lchip, uint8 slice_id, uint16 lport, uint8 enable)
{
    uint16 tmp_port = 0;
    uint8 xpipe_forbid[SYS_PHY_PORT_NUM_PER_SLICE] = {0};
    sys_datapath_lport_attr_t* port_attr = NULL;

    for (tmp_port = 0; tmp_port < SYS_PHY_PORT_NUM_PER_SLICE; tmp_port++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_port, slice_id);
        if (!port_attr || (SYS_DMPS_NETWORK_PORT != port_attr->port_type))
        {
            continue;
        }
        if (port_attr->serdes_id <= 23)
        {
            if ((CTC_CHIP_SERDES_100BASEFX_MODE != port_attr->pcs_mode)
                && (CTC_CHIP_SERDES_SGMII_MODE != port_attr->pcs_mode))
            {
                xpipe_forbid[tmp_port] = 1;
            }
        }
        else
        {
            if (CTC_CHIP_SERDES_XFI_MODE != port_attr->pcs_mode)
            {
                xpipe_forbid[tmp_port] = 1;
            }
        }
    }

    for (tmp_port = 0; tmp_port < SYS_PHY_PORT_NUM_PER_SLICE; tmp_port++)
    {
        if (xpipe_forbid[tmp_port] && enable)
        {
            port_attr = (sys_datapath_lport_attr_t*)(sys_usw_datapath_get_port_capability(lchip, tmp_port, slice_id));
            if(NULL == port_attr)
            {
                return CTC_E_INVALID_PTR;
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " At leat 1 port mode CANNOT support X-PIPE, the first port is %d with pcs mode %d\n", tmp_port, port_attr->pcs_mode);
            return CTC_E_INVALID_PARAM;
        }
    }

    for (tmp_port = 0; tmp_port < SYS_USW_XPIPE_PORT_NUM; tmp_port++)
    {
        if ((lport == g_tsingma_xpipe_mac_mapping[tmp_port].pmac_id) && enable)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port %d DO NOT support X-PIPE \n", tmp_port);
            return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_xpipe_en(uint8 lchip, uint16 lport, uint8 enable)
{
    uint8 slice_id = 0;
    uint32 cmd = 0;
    uint32 tmp_val32 = 0;
    uint32 tmp2_val32 = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;
    IpeHeaderAdjustPhyPortMap_m  ipe_port_map;
    EpeHeaderAdjustPhyPortMap_m  epe_port_map;

    sal_memset(&ipe_port_map, 0, sizeof(IpeHeaderAdjustPhyPortMap_m));
    sal_memset(&epe_port_map, 0, sizeof(EpeHeaderAdjustPhyPortMap_m));

    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if(NULL == port_attr)
    {
        return CTC_E_INVALID_PTR;
    }
    
    /* #0. Set chan-lport mapping */
    if (enable)
    {
        tmp_val32 = port_attr->emac_id;
        tmp2_val32 = port_attr->emac_id;
    }
    else
    {
        tmp_val32 = port_attr->emac_id;
        tmp2_val32 = port_attr->pmac_id;
    }
    DRV_IOW_FIELD(lchip, (IpeHeaderAdjustPhyPortMap_t+slice_id), IpeHeaderAdjustPhyPortMap_localPhyPort_f, &tmp_val32, &ipe_port_map);
    cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t+slice_id), DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &ipe_port_map));
    DRV_IOW_FIELD(lchip, (IpeHeaderAdjustPhyPortMap_t+slice_id), IpeHeaderAdjustPhyPortMap_localPhyPort_f, &tmp2_val32, &ipe_port_map);
    cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t+slice_id), DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &ipe_port_map));

    DRV_IOW_FIELD(lchip, (EpeHeaderAdjustPhyPortMap_t+slice_id), EpeHeaderAdjustPhyPortMap_localPhyPort_f, &tmp_val32, &epe_port_map);
    cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t+slice_id, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->emac_chanid, cmd, &epe_port_map));
    DRV_IOW_FIELD(lchip, (EpeHeaderAdjustPhyPortMap_t+slice_id), EpeHeaderAdjustPhyPortMap_localPhyPort_f, &tmp2_val32, &epe_port_map);
    cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t+slice_id, EpeHeaderAdjustPhyPortMap_localPhyPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_attr->pmac_chanid, cmd, &epe_port_map));

    /* #1. update NetTxPortStaticInfo_portMode_f, set pmac to reserved port */
    if (enable)
    {
        CTC_ERROR_RETURN(_sys_tsingma_get_nettx_port_mode_by_speed(port_attr->speed_mode, &tmp_val32));
        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f);
        DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &tmp_val32);
    }
    else
    {
        tmp_val32 = 0;
        
        /* restore portMode, and pmac_id to network port if necessary */
        tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, port_attr->pmac_id, slice_id);
        if (tmp_port_attr)
        {
            if ((CTC_CHIP_SERDES_NONE_MODE != tmp_port_attr->pcs_mode) &&
                (CTC_CHIP_SERDES_XSGMII_MODE != tmp_port_attr->pcs_mode))
            {
                p_usw_datapath_master[lchip]->port_attr[slice_id][port_attr->pmac_id].port_type = SYS_DMPS_NETWORK_PORT;
                CTC_ERROR_RETURN(_sys_tsingma_get_nettx_port_mode_by_speed(tmp_port_attr->speed_mode, &tmp_val32));
            }
        }
        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f);
        DRV_IOCTL(lchip, port_attr->pmac_id, cmd, &tmp_val32);
    }

    /* #2. reconfig EpeSchedule */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_epesched_xpipe_per_chan(lchip, port_attr->chan_id));

    /* #3. reconfig bufretrv credit, qmgr credit don't need config, beause bufretrv alloc buffer maximal*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_credit(lchip, slice_id, port_attr->chan_id));

    /* #4. reconfig Netrx*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_buf_mgr(lchip, slice_id));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_wrr_weight(lchip, slice_id));

    /* #5. reconfig qmgr*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_qmgr_wrr_weight(lchip, slice_id));

    /* #6. reconfig bufretrv, buf bufretrv buffer pointer need not config*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_bufretrv_wrr_weight(lchip, slice_id));

    /* #7. reconfig chan distribute*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_net_chan_cfg(lchip));

    /* #8. reconfig calendar*/
    CTC_ERROR_RETURN(sys_usw_datapath_set_calendar_cfg(lchip, slice_id, (lport/16)));

    /* #9. reconfig epe schedule*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_wrr_weight(lchip, slice_id));

    if (enable)
    {
        p_usw_datapath_master[lchip]->port_attr[slice_id][port_attr->pmac_id].port_type = SYS_DMPS_RSV_PORT;
    }

    /* #11, update glb xpipe config */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_update_glb_xpipe(lchip));


    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_datapth_init(uint8 lchip)
{
    /*1. netrx init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_netrx_init(lchip));

    /*2. Qmgr init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_qmgr_init(lchip));

    /*3. bufretrv init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_bufretrv_init(lchip));

    /*4. chan init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_net_chan_cfg(lchip));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_misc_chan_init(lchip));

    /*5. epe init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_epe_init(lchip));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_epe_xpipe_init(lchip));

    /*6. nettx init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_nettx_init(lchip));

    /*7. wlan init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_wlan_init(lchip));

    /*8. mac init*/
 /*    CTC_ERROR_RETURN(_sys_tsingma_datapath_mac_init(lchip));*/

    /*9. misc init */
    CTC_ERROR_RETURN(_sys_usw_datapath_misc_init(lchip));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_sup_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value = 0;
    /* SYSTEM Modified by tongzb,2019-03-13, for bug 51274 */
    uint32 clk_dir_value = 0;
    uint8 index = 0;
    uint8  step  = 0;
    uint32 Hss12GUnitReset[3] = {0};
    SupDskCfg_m sup_dsk;
    SupMiscCfg_m sup_misc;
    Hss28GUnitReset_m hss_rst;
    Hss28GCfg_m hss28g_cfg;

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }

/*#ifdef EMULATION_ENV
    return 0;
#endif*/

    drv_usw_chip_write(lchip, SUP_REG_CTL, 0x010040a3);

    /* SupDskCfg and SupMiscCfg configuration */
    value = 0;
    /* SYSTEM Modified by tongzb,2019-03-13, for bug 51274 */
    clk_dir_value = 2;/* 256M */
    cmd = DRV_IOR(SupDskCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_dsk);
    /* SYSTEM Modified by tongzb,2019-03-13, for bug 51274 */
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgClkTsRefDiv_f, &clk_dir_value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkTodDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkLedDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkMdioADiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkMdioBDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkTsRefDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkOobFcTxDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkMiscDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkHss12GRegAccDiv_f, &value, &sup_dsk);
    DRV_IOW_FIELD(lchip, SupDskCfg_t, SupDskCfg_cfgResetClkHss25GRegAccDiv_f, &value, &sup_dsk);
    cmd = DRV_IOW(SupDskCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_dsk);

     /*release sup reset core*/
    cmd = DRV_IOR(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);
    DRV_IOW_FIELD(lchip, SupMiscCfg_t, SupMiscCfg_softResetCore_f, &value, &sup_misc);
    cmd = DRV_IOW(SupMiscCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &sup_misc);

    /*do SupResetCtl reset*/
    if (0 == sys_usw_chip_get_reset_hw_en(lchip))
    {
        drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0xffffffff);
    }
    else
    {
        drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0xfffcffff);
    }
    sal_task_sleep(1);
    drv_usw_chip_write(lchip, SUP_RESET_CTL_ADDR, 0x01000000);

    value = 1;
    cmd = DRV_IOW(SupRegCtl_t, SupRegCtl_regReqCrossEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    /*init cfgTsUseIntRefClk */
    value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgTsUseIntRefClk_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    
    value = 0;
    cmd = DRV_IOW(RefDivCoppUpdPulse_t, RefDivCoppUpdPulse_cfgResetDivCoppUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivDlbPulse_t, RefDivDlbPulse_cfgResetDivDlbPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivEfdPulse_t, RefDivEfdPulse_cfgResetDivEfdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivLinkAggDrePulse_t, RefDivLinkAggDrePulse_cfgResetDivLinkAggDrePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    /*cmd = DRV_IOW(RefDivMfpUpdPulse_t, RefDivMfpUpdPulse_cfgResetDivMfpUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    TSINGMA_DUMP_PRINT(g_tm_dump_fp, "write chip 3 tbl-reg %s 0 field %s 0x%x\n", "RefDivMfpUpdPulse", "cfgResetDivMfpUpdPulse", 0);*/
    cmd = DRV_IOW(RefDivNetTxEeePulse_t, RefDivNetTxEeePulse_cfgResetDivNetTxEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivOamTokenUpd_t, RefDivOamTokenUpd_cfgResetDivOamTokenUpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivHsPcsEeePulse_t, RefDivHsPcsEeePulse_cfgResetDivHsEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivCsPcsEeePulse_t, RefDivCsPcsEeePulse_cfgResetDivCsEeePulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivHsPcsLinkPulse_t, RefDivHsPcsLinkPulse_cfgResetDivHsLinkFilterPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivHsPcsLinkPulse_t, RefDivHsPcsLinkPulse_cfgResetDivHsLinkPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivCsPcsLinkPulse_t, RefDivCsPcsLinkPulse_cfgResetDivCsLinkFilterPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivCsPcsLinkPulse_t, RefDivCsPcsLinkPulse_cfgResetDivCsLinkPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseLockPulse_t, RefDivSgmacPauseLockPulse_cfgRefDivSgmacPauseLockPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseLockPulse_t, RefDivSgmacPauseLockPulse_cfgResetDivSgmacPauseLockPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivEpePolicing1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivEpePolicing0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivIpePolicing1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivPolicingUpdPulse_t, RefDivPolicingUpdPulse_cfgResetDivIpePolicing0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    cmd = DRV_IOW(RefDivSgmacPauseLockPulse_t, RefDivSgmacPauseLockPulse_cfgResetDivSgmacPauseLockPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer3Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer2Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer1Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivSgmacPauseTimerPulse_t, RefDivSgmacPauseTimerPulse_cfgResetDivSgmacPauseTimer0Pulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivStormCtlUpdPulse_t, RefDivStormCtlUpdPulse_cfgResetDivStormCtl1UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(RefDivStormCtlUpdPulse_t, RefDivStormCtlUpdPulse_cfgResetDivStormCtl0UpdPulse_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    /* move from sup_init */

    /* Enable Mac Clock */
    /*enable RlmMacCtlEnClk */
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR+4, 0xffffffff);
    drv_usw_chip_write(lchip, RLM_MAC_CTL_EN_CLK_ADDR+8, 0xffffffff);


    /* Enable SharedMii/PCS Clock */
    /*enable RlmCsCtlEnClk */
    drv_usw_chip_write(lchip, RLM_CS_CTL_EN_CLK_ADDR, 0x1);

    /*enable RlmHsCtlEnClk */
    drv_usw_chip_write(lchip, RLM_HS_CTL_EN_CLK_ADDR, 0x7);


    /*
     *  #1, cfg RlmMacCtlReset.resetCoreTxqm0..3 = 0
     *  #2, cfg RlmMacCtlReset.resetCoreQuadSgmac0..15Reg = 0
     *  #3, cfg RlmMacCtlReset.resetCoreQuadSgmac0..15 = 0
     * the 3 step must be done in sys_tsingma_datapath_mac_init();
     * other field in RlmMacCtlReset won't be released unless mac module used, mac-en/dis, an/fec, in sys_tsingma_mac.c
     */ 
    value = 0;   
    for (index = 0; index < TM_SGMAC_NUM-2; index++)
    {
        step = RlmMacCtlReset_resetCoreQuadSgmacReg1_f - RlmMacCtlReset_resetCoreQuadSgmacReg0_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmacReg0_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);

        step = RlmMacCtlReset_resetCoreQuadSgmac1_f - RlmMacCtlReset_resetCoreQuadSgmac0_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmac0_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }
    for(index = 0; index < 2; index++)
    {
        step = RlmMacCtlReset_resetCoreQuadSgmacReg17_f - RlmMacCtlReset_resetCoreQuadSgmacReg16_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmacReg16_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);
        
        step = RlmMacCtlReset_resetCoreQuadSgmac17_f - RlmMacCtlReset_resetCoreQuadSgmac16_f;
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreQuadSgmac16_f+step*index);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }
    
    value = 0;
    for (index = 0; index < MAC_PORT_NUM_PER_QM; index++)
    {
        cmd = DRV_IOW(RlmMacCtlReset_t, RlmMacCtlReset_resetCoreTxqm0_f+index);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    /* Release SharedMii/PCS reset */
    /*release RlmCsCtlReset*/
    drv_usw_chip_write(lchip, RLM_CS_CTL_RESET_ADDR, 0);

    /*release RlmHsCtlReset*/
    drv_usw_chip_write(lchip, RLM_HS_CTL_RESET_ADDR, 0);

    /*enable Hss12GUnitEnClk */
    drv_usw_chip_write(lchip, HSS_12G_UNIT_EN_CLK0_ADDR, 0xffffffff);
    drv_usw_chip_write(lchip, HSS_12G_UNIT_EN_CLK1_ADDR, 0xffffffff);
    drv_usw_chip_write(lchip, HSS_12G_UNIT_EN_CLK2_ADDR, 0xffffffff);

    /*enable Hss28GUnitEnClk */
    drv_usw_chip_write(lchip, HSS_28G_UNIT_EN_CLK_ADDR, 0xffffffff);

    /*release Hss28GUnitReset.resetHss28GRegAcc */
    cmd = DRV_IOR(Hss28GUnitReset_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hss_rst);
    DRV_IOW_FIELD(lchip, Hss28GUnitReset_t, Hss28GUnitReset_resetHss28GRegAcc_f, &value, &hss_rst);
    cmd = DRV_IOW(Hss28GUnitReset_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hss_rst);

    /*release Hss28GCfg.cfgHss[0-1]ApbResetBar */
    value = 1;
    cmd = DRV_IOR(Hss28GCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hss28g_cfg);
    DRV_IOW_FIELD(lchip, Hss28GCfg_t, Hss28GCfg_cfgHss0ApbResetBar_f, &value, &hss28g_cfg);
    DRV_IOW_FIELD(lchip, Hss28GCfg_t, Hss28GCfg_cfgHss1ApbResetBar_f, &value, &hss28g_cfg);
    cmd = DRV_IOW(Hss28GCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hss28g_cfg);


    /*release RlmNetCtlReset*/
    drv_usw_chip_write(lchip, RLM_BUFFER_CTL_RESET_ADDR, 0);

    /*release RlmNetCtlReset*/
    drv_usw_chip_write(lchip, RLM_NET_CTL_RESET_ADDR, 0);

    /*release RlmIpeCtlReset */
    drv_usw_chip_write(lchip, RLM_IPERX_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_IPETX_CTL_RESET_ADDR, 0);

    /*do RlmBsrCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_BSR_CTL_RESET_ADDR, 0);

    /*release RlmEpeCtlReset */
    drv_usw_chip_write(lchip, RLM_EPERX_CTL_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, RLM_EPETX_CTL_RESET_ADDR, 0);

    /*do RlmQMgrCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_QMGR_CTL_RESET_ADDR, 0);

    /*do RlmKeyCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_KEY_CTL_RESET_ADDR, 0);

    /*do RlmTcamCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_TCAM_CTL_RESET_ADDR, 0);

    /*release RlmAdCtlReset */
    drv_usw_chip_write(lchip, RLM_AD_CTL_RESET_ADDR, 0);

    /*do RlmSecurityCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_SECURITY_CTL_RESET_ADDR, 0);

    /*do RlmCapwapCtlReset reset*/
    drv_usw_chip_write(lchip, RLM_CAPWAP_CTL_RESET_ADDR, 0);

    /*do SupIntrFunc reset*/
    drv_usw_chip_write(lchip, SUP_INTR_FUNC_ADDR+4, 0xffffffff);

    /*TsEngineTodCfg_todReset_f[4,28,1] set 0*/
    value = 0;
    drv_usw_chip_read(lchip, 0x00a00200+4*4, &value);
    value &= (0xfeffffff);
    drv_usw_chip_write(lchip, 0x00a00200+4*4, value);

    /*Hss12GUnitReset[0-2]_resetCoreKrLinkCtl[0-7]Reg_f[3,4-11,1] set 0*/
    Hss12GUnitReset[0] = 0x00483260;
    Hss12GUnitReset[1] = 0x00501260;
    Hss12GUnitReset[2] = 0x00584260;
    for (step = 0; step <3; step++)
    {
        value = 0;
        drv_usw_chip_read(lchip, Hss12GUnitReset[step]+3*4, &value);
        value &= (0xfffff00f);
        drv_usw_chip_write(lchip, Hss12GUnitReset[step]+3*4, value);
    }

    return CTC_E_NONE;
}

/* Write tables about qm choice, according to the global saved g_qm_choice. */
int32
sys_tsingma_datapath_set_txqm_mode_config(uint8 lchip)
{
    uint32   mux_value       = 0;
    uint32   mac_value[2]    = {0};
    fld_id_t mux_field_id    = RlmHsMuxModeCfg_cfgHssUnit2MuxMode_f;
    fld_id_t mac_field_id[2] = {RlmMacMuxModeCfg_cfgTxqm1Mode_f, RlmMacMuxModeCfg_cfgTxqm3Mode_f};
    RlmMacMuxModeCfg_m mac_cfg;
    RlmHsMuxModeCfg_m  mux_cfg;
    
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    mux_value    = p_usw_datapath_master[lchip]->qm_choice.muxmode;
    mac_value[0] = p_usw_datapath_master[lchip]->qm_choice.txqm1;
    mac_value[1] = p_usw_datapath_master[lchip]->qm_choice.txqm3;

    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, RlmMacMuxModeCfg_t, 2, mac_field_id, mac_value, &mac_cfg));
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, RlmHsMuxModeCfg_t,  1, &mux_field_id, &mux_value, &mux_cfg));
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_param_check(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8 serdes_id;
    uint8 illegal_flag1 = FALSE; //FALSE - no lane is_dynamic=1 and mode is none    TRUE - at least 1 lane is_dynamic=1 and mode is none
    uint8 illegal_flag2 = FALSE; //FALSE - no mac id conflict    TRUE - mac id conflict exists
    uint8 illegal_flag3 = FALSE; //TRUE - serdes 24~27 or 28~31 exist none mode    FALSE - else
    uint8 illegal_flag4 = FALSE; //TRUE - serdes 24~27 or 28~31 is at least 1 sgmii    FALSE - else
    DevId_m read_device_id;
    uint32 cmd = 0;
    uint32 tmp_device_id = 0xffffffff;
    int32  ret = 0;

    /*init hardware I/O check*/
    cmd = DRV_IOR(DevId_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &read_device_id);
    tmp_device_id = GetDevId(V, deviceId_f, &read_device_id);
    if((0 != ret) || (0xffffffff == tmp_device_id))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " [CHIP REG IO] Hardware I/O not ready! \n");
        return CTC_E_HW_FAIL;
    }

    for (serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        if((1 == p_datapath_cfg->serdes[serdes_id].is_dynamic) && (FALSE == illegal_flag1) && 
           (CTC_CHIP_SERDES_NONE_MODE == p_datapath_cfg->serdes[serdes_id].mode))
        {
            illegal_flag1 = TRUE;
        }

        if((12 > serdes_id) && (0 != serdes_id%4) && (!illegal_flag2))
        {
            if(((CTC_CHIP_SERDES_QSGMII_MODE == p_datapath_cfg->serdes[serdes_id/4*4].mode) || 
                 (CTC_CHIP_SERDES_USXGMII1_MODE == p_datapath_cfg->serdes[serdes_id/4*4].mode)) && 
                (SYS_DATAPATH_IS_SERDES_1_PORT_1(p_datapath_cfg->serdes[serdes_id].mode)))
            {
                illegal_flag2 = TRUE;
            }
            else if((2 > serdes_id%4) && 
                    (CTC_CHIP_SERDES_USXGMII2_MODE == p_datapath_cfg->serdes[serdes_id/4*4].mode) && 
                    (SYS_DATAPATH_IS_SERDES_1_PORT_1(p_datapath_cfg->serdes[serdes_id].mode)))
            {
                illegal_flag2 = TRUE;
            }
        }

        if((!illegal_flag3) || (!illegal_flag4))
        {
            if(28 == serdes_id)
            {
                illegal_flag3 = FALSE;
                illegal_flag4 = FALSE;
            }
            if(23 < serdes_id)
            {
                if((CTC_CHIP_SERDES_SGMII_MODE == p_datapath_cfg->serdes[serdes_id].mode) || 
                   (CTC_CHIP_SERDES_2DOT5G_MODE == p_datapath_cfg->serdes[serdes_id].mode))
                {
                    illegal_flag4 = TRUE;
                }
                if(CTC_CHIP_SERDES_NONE_MODE == p_datapath_cfg->serdes[serdes_id].mode) /*XSGMII is permitted*/
                {
                    illegal_flag3 = TRUE;
                }
            }
        }
    }

    if((illegal_flag1) || (illegal_flag2) || (illegal_flag3 && illegal_flag4))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " [DATAPATH] datapath_cfg.txt check fail [%u %u %u %u]! \n", 
                             illegal_flag1, illegal_flag2, illegal_flag3, illegal_flag4);
        return CTC_E_INVALID_CONFIG;
    }
    return CTC_E_NONE;
}

/*
For tsingma, just core clock A is used. Core clock B is invalid.
*/
STATIC int32
_sys_tsingma_datapath_core_clock_init(uint8 lchip, uint16 core_a)
{
    PllCoreCfg_m core_cfg;
    PllCoreMon_m core_mon;
    uint32 cmd = 0;
    uint32 value = 0;


    sal_memset(&core_cfg, 0, sizeof(PllCoreCfg_m));
    sal_memset(&core_mon, 0, sizeof(PllCoreMon_m));

    /*reset core pll*/
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    value = 1;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreReset_f, &value, &core_cfg);
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePllPwd_f, &value, &core_cfg);
    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    
    if(core_a == 200)
    {
        value = 0x37;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 0x13;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSic_f, &value, &core_cfg);
        value = 0xd;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSip_f, &value, &core_cfg);
        value = 0xd;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePostDiv_f, &value, &core_cfg);
    }
    else if(core_a == 400)
    {
        value = 47;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 0xf;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSic_f, &value, &core_cfg);
        value = 0x9;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSip_f, &value, &core_cfg);
        value = 5;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePostDiv_f, &value, &core_cfg);
    }
    else if(core_a == 600)
    {
        value = 47;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreMultInt_f, &value, &core_cfg);
        value = 0xf;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSic_f, &value, &core_cfg);
        value = 0x9;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSip_f, &value, &core_cfg);
        value = 3;
        DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePostDiv_f, &value, &core_cfg);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    value = 0;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreDcoBypass_f, &value, &core_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreIntFbk_f, &value, &core_cfg);
    value = 0;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePreDiv_f, &value, &core_cfg);
    value = 4;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSgain_f, &value, &core_cfg);
    value = 1;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreSlock_f, &value, &core_cfg);

    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));

    sal_task_sleep(1);

    /*reset core pll*/
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCorePllPwd_f, &value, &core_cfg);
    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    
    sal_task_sleep(1);
    
    cmd = DRV_IOR(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));
    value = 0;
    DRV_IOW_FIELD(lchip, PllCoreCfg_t, PllCoreCfg_cfgPllCoreReset_f, &value, &core_cfg);
    cmd = DRV_IOW(PllCoreCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_cfg));

    sal_task_sleep(1);

    /*check pll lock*/
    cmd = DRV_IOR(PllCoreMon_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &core_mon));

    if (0 == SDK_WORK_PLATFORM)
    {
        if (!GetPllCoreMon(V, monPllCoreLock_f, &core_mon))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " [DATAPATH] pll cannot lock \n");
            return CTC_E_HW_NOT_LOCK;

        }
    }


    return CTC_E_NONE;
}

/*
PRBS check
*/
STATIC int32
_sys_tsingma_datapath_set_serdes_prbs_rx(uint8 lchip, ctc_chip_serdes_prbs_t* p_prbs)
{
    uint8  hss_id     = 0;
    uint8  lane_id    = 0;
    uint8  serdes_id  = 0;
    uint8  acc_id     = 0;
    uint8  bist_stat  = 0;
    uint8  byte_cnt   = 0;
    uint16 addr       = 0;
    uint16 mask       = 0;
    uint16 data       = 0;
    uint16 error_high = 0;
    uint16 error_low  = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    serdes_id = p_prbs->serdes_id;
    hss_id    = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id   = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /*1. PRBS Mode Select  set RX_PRBS_MODE (reg0x8z61[13:12])*/
        acc_id = hss_id;
        addr   = 0x8061 + lane_id * 0x100;
        mask   = 0xcfff;
        switch (p_prbs->polynome_type)
        {
            case CTC_CHIP_SERDES_PRBS7_PLUS:
            case CTC_CHIP_SERDES_PRBS7_SUB:
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;
            case CTC_CHIP_SERDES_PRBS15_PLUS:
            case CTC_CHIP_SERDES_PRBS15_SUB:
                data = 0x01;
                break;
            case CTC_CHIP_SERDES_PRBS23_PLUS:
            case CTC_CHIP_SERDES_PRBS23_SUB:
                data = 0x02;
                break;
            case CTC_CHIP_SERDES_PRBS31_PLUS:
            case CTC_CHIP_SERDES_PRBS31_SUB:
                data = 0x03;
                break;
            default:
                return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
        
        /*2. PRBS Check Enable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b1*/
        data = 1;
        mask = 0xfbff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

        /*reset count*/
        data   = 1;
        mask   = 0x7fff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
        data   = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
        
        /*3. PRBS Check Status Read*/
        sal_task_sleep(1);
        mask = 0;
        addr = 0x8066 + lane_id * 0x100;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &error_high));
        addr = 0x8067 + lane_id * 0x100;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &error_low));
        p_prbs->value = 0;
        if(0 != error_high || 0 != error_low)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "PRBS chek status error! error_high = %u, error_low = %u\n", error_high, error_low);
            return CTC_E_NONE;
        }
        
        /*4. PRBS Check disable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b0*/
        data = 0;
        addr = 0x8061 + lane_id * 0x100;
        mask = 0xfbff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
    }
    else
    {
        /*0. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        acc_id = lane_id + 3;
        addr = 0x76;
        mask = 0x8f;
        switch (p_prbs->polynome_type)
        {
            case CTC_CHIP_SERDES_PRBS7_PLUS:
            case CTC_CHIP_SERDES_PRBS7_SUB:
                data = 0x00;
                break;
            case CTC_CHIP_SERDES_PRBS15_PLUS:
            case CTC_CHIP_SERDES_PRBS15_SUB:
                data = 0x03;
                break;
            case CTC_CHIP_SERDES_PRBS23_PLUS:
            case CTC_CHIP_SERDES_PRBS23_SUB:
                data = 0x04;
                break;
            case CTC_CHIP_SERDES_PRBS31_PLUS:
            case CTC_CHIP_SERDES_PRBS31_SUB:
                data = 0x05;
                break;
            default:
                return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
        
        /*1. Enable r_bist_chk  set r_bist_chk(reg0x77[0]) = 1 and r_bist_chk_zero(reg0x77[1]) = 1*/
        acc_id = lane_id + 3;
        addr = 0x77;
        data = 1;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
        
        /*2. Read bist status*/
        sal_task_sleep(1);
        mask = 0;
        addr = 0xe0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, &bist_stat));
        p_prbs->value = bist_stat;
        if(3 != (3 & bist_stat))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "PRBS chek status error! ");
            for(byte_cnt = 0; byte_cnt < 4; byte_cnt++)
            {
                addr = 0xe1+byte_cnt;
                CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, &bist_stat));
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "err_cnt[%u]=0x%x ", byte_cnt, bist_stat);
            }
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
            return CTC_E_INVALID_PARAM;
        }
        
        /*3. Disable r_bist_chk  set r_bist_chk(reg0x77[0]) = 0 and r_bist_chk_zero(reg0x77[1]) = 0*/
        addr = 0x77;
        data = 0;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_prbs_tx(uint8 lchip, void* p_data)
{
    uint8 hss_id    = 0;
    uint8 lane_id   = 0;
    uint8 serdes_id = 0;
    uint8 acc_id = 0;
    uint16 addr = 0;
    uint16 mask = 0;
    uint16 data = 0;
    uint16 raw_data = 0;
    ctc_chip_serdes_prbs_t* p_prbs = (ctc_chip_serdes_prbs_t*)p_data;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    
    serdes_id = p_prbs->serdes_id;
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /*1. PRBS mode select  set TX_PRBS_MODE(reg0x8zA0[9:8]) */
        acc_id = hss_id;
        addr = 0x80a0 + lane_id * 0x100;
        mask = 0xfcff;
        switch (p_prbs->polynome_type)
        {
            case CTC_CHIP_SERDES_PRBS7_PLUS:
            case CTC_CHIP_SERDES_PRBS7_SUB:
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;
            case CTC_CHIP_SERDES_PRBS15_PLUS:
            case CTC_CHIP_SERDES_PRBS15_SUB:
                data = 0x01;
                break;
            case CTC_CHIP_SERDES_PRBS23_PLUS:
            case CTC_CHIP_SERDES_PRBS23_SUB:
                data = 0x02;
                break;
            case CTC_CHIP_SERDES_PRBS31_PLUS:
            case CTC_CHIP_SERDES_PRBS31_SUB:
                data = 0x03;
                break;
            default:
                return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
        
        /*2. PRBS Test Enable*/
        /* set TX_TEST_DATA_SRC (reg0x8zA0[15]) = 1'b1 */
        /* set TX_PRBS_CLOCK_EN (reg0x8zA0[14]) = 1'b1 */
        /* set TX_TEST_EN (reg0x8zA0[13]) = 1'b1 */
        /* set TX_PRBS_GEN_EN (0x8zA0[11]) = 1'b1 */
        CTC_ERROR_RETURN(drv_chip_read_hss28g(lchip, acc_id, (uint32)addr, &raw_data));
        data = raw_data & 0x17ff;
        if (p_prbs->value)  /* enable */
        {
            data |= 0xe800;
        }
        CTC_ERROR_RETURN(drv_chip_write_hss28g(lchip, acc_id, (uint32)addr, data));
    }
    else
    {
        /*1. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        acc_id = lane_id + 3;
        addr = 0x76;
        mask = 0x8f;
        switch (p_prbs->polynome_type)
        {
            case CTC_CHIP_SERDES_PRBS7_PLUS:
            case CTC_CHIP_SERDES_PRBS7_SUB:
                data = 0x00;
                break;
            case CTC_CHIP_SERDES_PRBS15_PLUS:
            case CTC_CHIP_SERDES_PRBS15_SUB:
                data = 0x03;
                break;
            case CTC_CHIP_SERDES_PRBS23_PLUS:
            case CTC_CHIP_SERDES_PRBS23_SUB:
                data = 0x04;
                break;
            case CTC_CHIP_SERDES_PRBS31_PLUS:
            case CTC_CHIP_SERDES_PRBS31_SUB:
                data = 0x05;
                break;
            default:
                return CTC_E_INVALID_CONFIG;
        }
        
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
        
        /*2. Enable r_bist_en  set r_bist_en(reg0x76[0]) = 1*/
        mask = 0xfe;
        data = (p_prbs->value ? 1 : 0);
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_prbs(uint8 lchip, void* p_data)
{
    uint32 enable;
    uint32 gport    = 0;
    uint16 drv_port = 0;
    uint8  hss_idx  = 0;
    uint8  gchip    = 0;
    uint8  lane_id  = 0;
    uint8  slice_id = 0;
    ctc_chip_serdes_prbs_t* p_prbs = (ctc_chip_serdes_prbs_t*)p_data;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t*   p_serdes  = NULL;
    CTC_PTR_VALID_CHECK(p_prbs);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, value:%d, type:%d, mode:%d\n", p_prbs->serdes_id, p_prbs->value, p_prbs->polynome_type, p_prbs->mode);

    hss_idx  = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_prbs->serdes_id);
    lane_id  = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_prbs->serdes_id);
    slice_id = (p_prbs->serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;

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
            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_serdes_prbs_rx(lchip, p_prbs));
            break;
        case 1: /* 1--Tx */
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs_tx(lchip, p_prbs));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_prbs_status(uint8 lchip,uint8 serdes_id,uint8* enable,uint8* hss_id,uint8* hss_type,uint8* lane_id)
{
    uint8 acc_id = 0;
    uint16 addr = 0;
    uint16 mask = 0;
    uint16 data = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    *hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    *lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        *hss_type = SYS_DATAPATH_HSS_TYPE_28G;
        /*1. PRBS mode select  set TX_PRBS_MODE(reg0x8zA0[9:8]) */
        acc_id = *hss_id;
        addr = 0x80a0 + *lane_id * 0x100;    
        mask = 0xdfff;  /*bit[13]*/
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &data));
        *enable = (data == 1) ? 1 :0;
    }
    else
    {
        *hss_type = SYS_DATAPATH_HSS_TYPE_15G;
        /*1. PRBS mode select  set r_bist_mode(reg0x76[6:4]) */
        acc_id = *lane_id + 3;
        addr = 0x76;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, *hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8*)&data));
        *enable = (data == 1) ? 1 :0;
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_prbs31_en(uint8 lchip, uint16 serdes_id, uint8 enable)
{
    ctc_chip_serdes_prbs_t prbs_param;

    sal_memset(&prbs_param, 0, sizeof(ctc_chip_serdes_prbs_t));

    prbs_param.serdes_id = serdes_id;
    prbs_param.polynome_type = CTC_CHIP_SERDES_PRBS31_PLUS;
    prbs_param.mode = 1;  /*tx*/
    prbs_param.value = enable;
    CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs_tx(lchip, &prbs_param));

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_prbs31_en(uint8 lchip, uint16 serdes_id, uint8* enable)
{
    uint8 hss_id = 0;
    uint8 hss_type = 0;
    uint8 lane_id = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_prbs_status(lchip, serdes_id, enable, &hss_id, &hss_type, &lane_id));

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_prbs31_errcnt(uint8 lchip, uint16 serdes_id, uint32* errcnt)
{
    uint8  hss_id     = 0;
    uint8  lane_id    = 0;
    uint8  acc_id     = 0;
    uint16 addr       = 0;
    uint16 mask       = 0;
    uint16 data       = 0;
    uint16 error_high = 0;
    uint16 error_low  = 0;
    
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SerDes] %s not support HSS12G!\n", __FUNCTION__);
        return CTC_E_NOT_SUPPORT;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    hss_id    = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id   = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    /*1. PRBS Mode Select  set RX_PRBS_MODE (reg0x8z61[13:12])*/
    acc_id = hss_id;
    addr   = 0x8061 + lane_id * 0x100;
    mask   = 0xcfff;
    data = 0x03;  /*prbs31*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
    
    /*2. PRBS Check Enable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b1*/
    data = 1;
    mask = 0xfbff;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    /*reset count*/
    data   = 1;
    mask   = 0x7fff;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
    data   = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));
    
    /*3. PRBS Check Status Read*/
    sal_task_sleep(1);
    mask = 0;
    addr = 0x8066 + lane_id * 0x100;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &error_high));
    addr = 0x8067 + lane_id * 0x100;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &error_low));

    SYS_USW_VALID_PTR_WRITE(errcnt, ((error_high << 16)|error_low));
    
    /*4. PRBS Check disable  set RX_PRBS_CHECK_EN(reg0x8z61[10]) = 1'b0*/
    data = 0;
    addr = 0x8061 + lane_id * 0x100;
    mask = 0xfbff;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_no_random_flag(uint8 lchip, uint16 serdes_id, uint8 *flag)
{
    uint8  hss_id     = 0;
    uint8  lane_id    = 0;
    uint8  acc_id     = 0;
    uint16 addr       = 0;
    uint16 mask       = 0;
    uint16 data       = 0;
    
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SerDes] %s not support HSS12G!\n", __FUNCTION__);
        return CTC_E_NOT_SUPPORT;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);

    hss_id    = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id   = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    acc_id = hss_id;
    mask = 0xff0f;
    addr = 0x9811;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &data));

    SYS_USW_VALID_PTR_WRITE(flag, ((data >> lane_id) & 0x1));
    
    return CTC_E_NONE;
}


/*
Set all loopback mode to default, e.g. disable all loopback.
*/
int32
sys_tsingma_datapath_set_loopback_hss12g_clear(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8 hss_id = 0;
    uint8 acc_id = 0;
    uint8 addr   = 0;
    uint8 mask   = 0;
    uint8 data   = 0;

    if(SYS_DATAPATH_SERDES_28G_START_ID <= p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }
    hss_id = (p_loopback->serdes_id) / HSS12G_LANE_NUM;
    acc_id = (p_loopback->serdes_id) % HSS12G_LANE_NUM + 3;

    /*reg0x06[4] cfg_rx2tx_lp_en  reg0x06[3] cfg_tx2rx_lp_en*/
    addr   = 0x06;
    mask   = 0xe7;
    data   = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    /*reg0x0e[5] cfg_txlb_en  reg0x0e[4] cfg_rxlb_en*/
    addr   = 0x0e;
    mask   = 0xcf;
    data   = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    /*reg0x13[2] cfg_cdrck_en*/
    addr   = 0x13;
    mask   = 0xfb;
    data   = 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));
    
    /*reg0x91[0] r_lbslv_in_pmad*/
    addr   = 0x91;
    mask   = 0xfe;
    data   = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));
    
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_check_hss12g_poly(uint8 lchip, uint8 serdes_id)
{
    uint16 rx_pol = 0;
    uint16 tx_pol = 0;
    ctc_chip_serdes_polarity_t pola_param;

    sal_memset(&pola_param, 0, sizeof(ctc_chip_serdes_polarity_t));
    pola_param.serdes_id = serdes_id;
    pola_param.dir = 0; /* get rx pol */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_polarity(lchip, &pola_param));
    rx_pol = pola_param.polarity_mode;
    pola_param.dir = 1; /* get rx pol */
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_polarity(lchip, &pola_param));
    tx_pol = pola_param.polarity_mode;
    if (rx_pol ^ tx_pol)  /* rx or tx has at least one dir swap */
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SerDes] HSS12G SerDes %d DO NOT support external loopback for only %s PN swap!\n", 
            serdes_id, (rx_pol?"RX":"TX"));
        return CTC_E_NOT_SUPPORT;
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_loopback_hss12g_ls2(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8 hss_id = 0;
    uint8 acc_id = 0;
    uint8 addr   = 0;
    uint8 mask   = 0;
    uint8 data   = 0;
    
    if(SYS_DATAPATH_SERDES_28G_START_ID <= p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_tsingma_datapath_check_hss12g_poly(lchip, p_loopback->serdes_id));

    CTC_ERROR_RETURN(sys_tsingma_datapath_set_loopback_hss12g_clear(lchip, p_loopback));
    
    hss_id = (p_loopback->serdes_id) / HSS12G_LANE_NUM;
    acc_id = (p_loopback->serdes_id) % HSS12G_LANE_NUM + 3;

    /*reg0x06[4] cfg_rx2tx_lp_en*/
    addr   = 0x06;
    mask   = 0xef;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    /*reg0x0e[4] cfg_rxlb_en*/
    addr   = 0x0e;
    mask   = 0xef;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    /*reg0x13[2] cfg_cdrck_en*/
    addr   = 0x13;
    mask   = 0xfb;
    data   = 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));
    
    return CTC_E_NONE;
}


int32
sys_tsingma_datapath_set_loopback_hss12g_lm1(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8 hss_id = 0;
    uint8 acc_id = 0;
    uint8 addr   = 0;
    uint8 mask   = 0;
    uint8 data   = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 rx_flag = 0;
    uint8 tx_flag = 0;
    uint16 lport  = 0;
    uint8  slice_id = 0;
    uint32 tbl_id           = 0;
    uint32 cmd              = 0;
    uint32 value_pcs =0;
    uint8  step             = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    ctc_chip_serdes_polarity_t polarity;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SharedPcsSerdes0Cfg0_m       serdes_cfg;
    QsgmiiPcsCfg0_m              qsgmii_serdes_cfg;
    UsxgmiiPcsSerdesCfg0_m       usxgmii_serdes_cfg;

    sal_memset(&polarity, 0, sizeof(ctc_chip_serdes_polarity_t));

    if(SYS_DATAPATH_SERDES_28G_START_ID <= p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_tsingma_datapath_set_loopback_hss12g_clear(lchip, p_loopback));
    
    hss_id = (p_loopback->serdes_id) / HSS12G_LANE_NUM;
    acc_id = (p_loopback->serdes_id) % HSS12G_LANE_NUM + 3;

    /*reg0x06[3] cfg_tx2rx_lp_en*/
    addr   = 0x06;
    mask   = 0xf7;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    /*reg0x0e[5] cfg_txlb_en*/
    /*reg0x0e[4] cfg_txlb_en*/
    addr   = 0x0e;
    mask   = 0xcf;
    data   = (0 == p_loopback->enable) ? 0 : 3;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_loopback->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_loopback->serdes_id);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if(p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    /*reg0x23[0] cfg_dfe_pd    if loopback enable, set 0; if loopback disable, set 0 in 10G lane speed, else set 1. */
    addr   = 0x23;
    mask   = 0xfe;
    data   = 0;
    if((0 == p_loopback->enable) && (!SYS_DATAPATH_IS_SERDES_10G_PER_LANE(p_hss_vec->serdes_info[lane_id].mode)))
    {
        data = 1;
    }
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, data));

    polarity.polarity_mode = (p_loopback->enable) ? 0 : 1;
    /* if rx_polarity/tx_polarity is reverse, should set it normal */
    if(1 == p_hss_vec->serdes_info[lane_id].rx_polarity)
    {
        if (p_loopback->enable)
        {
            rx_flag |= 0x1; /* flag is use for record polarity_mode swap */
        }
        polarity.dir = 0;
        polarity.serdes_id = p_loopback->serdes_id;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &polarity));
    }
    if(1 == p_hss_vec->serdes_info[lane_id].tx_polarity)
    {
        if (p_loopback->enable)
        {
            tx_flag |= 0x1; /* flag is use for record polarity_mode swap */
        }
        polarity.dir = 1;
        polarity.serdes_id = p_loopback->serdes_id;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &polarity));
    }

    /* cfg forceSignalDetect */
    lport = p_hss_vec->serdes_info[lane_id].lport;
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PTR;
    }
    value_pcs = (p_loopback->enable)?1:0;
    if (port_attr->pcs_mode == CTC_CHIP_SERDES_QSGMII_MODE)
    {
        tbl_id = QsgmiiPcsCfg0_t + p_loopback->serdes_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, QsgmiiPcsCfg0_forceSignalDetect_f, &value_pcs, &qsgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_serdes_cfg));
    }
    else if (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII0_MODE)
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + ((p_loopback->serdes_id)/4)*4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, (UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f+(p_loopback->serdes_id)%4), &value_pcs, &usxgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
    }
    else if((port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII1_MODE)
        || (port_attr->pcs_mode == CTC_CHIP_SERDES_USXGMII2_MODE))
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + p_loopback->serdes_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f, &value_pcs, &usxgmii_serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
    }
    else
    {
        step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
        tbl_id = SharedPcsSerdes0Cfg0_t + (p_loopback->serdes_id)/4 + ((p_loopback->serdes_id)%4)*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &serdes_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value_pcs, &serdes_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &serdes_cfg));
    }

    /* change sw by flag, used for enable */
    if(CTC_IS_BIT_SET(rx_flag, 0))
    {
        p_hss_vec->serdes_info[lane_id].rx_polarity = 1;
        CTC_BIT_UNSET(rx_flag, 0);
    }
    if(CTC_IS_BIT_SET(tx_flag, 0))
    {
        p_hss_vec->serdes_info[lane_id].tx_polarity = 1;
        CTC_BIT_UNSET(tx_flag, 0);
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_loopback_hss28g_ext(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8  acc_id = 0;
    uint8  lane_id = 0;
    uint16 mask   = 0;
    uint16 data   = 0;
    uint16 addr   = 0;
    uint32 cmd    = 0;
    uint32 field_id  = 0;
    uint32 tmp_val32 = 0;
    uint16 tmp_val16 = 0;
    Hss28GReserved_m ext_lpbk;

    if(SYS_DATAPATH_SERDES_28G_START_ID > p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }
    
    
    acc_id = (p_loopback->serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) / HSS28G_LANE_NUM;
    lane_id = (p_loopback->serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) % HSS28G_LANE_NUM;

    addr   = 0x84f5;
    mask   = 0x00ff;
    if (p_loopback->enable)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &data));
        tmp_val16 = data << (acc_id*8);
        tmp_val32 = (uint32)tmp_val16;
        field_id = Hss28GReserved_reserved_f;
        CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, Hss28GReserved_t, 1, (fld_id_t *)(&field_id), &tmp_val32, &ext_lpbk));
        data = 0x28;
    }
    else
    {
        cmd = DRV_IOR(Hss28GReserved_t, Hss28GReserved_reserved_f);
        DRV_IOCTL(lchip, 0, cmd, &tmp_val32);
        tmp_val16 = (tmp_val32 >> (acc_id*8)) & 0xffff;
        data = tmp_val16;
    }
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    addr   = 0x84f6;
    mask   = 0xffbf;
    data   = (0 == p_loopback->enable) ? 1 : 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    addr   = 0x84f0;
    mask   = 0xfffe;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    addr   = 0x84f0;
    mask   = 0x3fff;
    data   = lane_id;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    addr   = 0x84ef;
    mask   = 0xffe7;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    /*Register 9803h: LOOPBACK FIFO    Bit15 - LOOPBACK_EN*/
    addr   = 0x9803;
    mask   = 0x7fff;
    data   = (0 == p_loopback->enable) ? 0 : 1;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr, mask, data));

    return CTC_E_NONE;
}


int32
sys_tsingma_datapath_get_loopback_hss12g_ls2(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8 hss_id = 0;
    uint8 acc_id = 0;
    uint8 addr   = 0;
    uint8 mask   = 0;
    uint8 val_reg0x06 = 0;
    uint8 val_reg0x0e = 0;
    uint8 val_reg0x13 = 0;

    if(SYS_DATAPATH_SERDES_28G_START_ID <= p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }

    hss_id = (p_loopback->serdes_id) / HSS12G_LANE_NUM;
    acc_id = (p_loopback->serdes_id) % HSS12G_LANE_NUM + 3;

    /*reg0x06[4] cfg_rx2tx_lp_en*/
    addr   = 0x06;
    mask   = 0xef;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, addr, mask, &val_reg0x06));

    /*reg0x0e[4] cfg_rxlb_en*/
    addr   = 0x0e;
    mask   = 0xef;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, addr, mask, &val_reg0x0e));

    /*reg0x13[2] cfg_cdrck_en*/
    addr   = 0x13;
    mask   = 0xfb;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, addr, mask, &val_reg0x13));

    if((1 == val_reg0x06) && (1 == val_reg0x0e) && (1 == val_reg0x13))
    {
        p_loopback->enable = 1;
    }
    else
    {
        p_loopback->enable = 0;
    }
    
    return CTC_E_NONE;
}


int32
sys_tsingma_datapath_get_loopback_hss12g_lm1(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8 hss_id = 0;
    uint8 acc_id = 0;
    uint8 addr   = 0;
    uint8 mask   = 0;
    uint8 val_reg0x06 = 0;
    uint8 val_reg0x0e = 0;

    if(SYS_DATAPATH_SERDES_28G_START_ID <= p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }
    
    hss_id = (p_loopback->serdes_id) / HSS12G_LANE_NUM;
    acc_id = (p_loopback->serdes_id) % HSS12G_LANE_NUM + 3;

    /*reg0x06[3] cfg_tx2rx_lp_en*/
    addr   = 0x06;
    mask   = 0xe7;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, addr, mask, &val_reg0x06));

    /*reg0x0e[5] cfg_txlb_en*/
    addr   = 0x0e;
    mask   = 0xcf;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss12g_reg(lchip, hss_id, acc_id, addr, mask, &val_reg0x0e));

    if((1 == val_reg0x06) && (3 == val_reg0x0e))
    {
        p_loopback->enable = 1;
    }
    else
    {
        p_loopback->enable = 0;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_loopback_hss28g_ext(uint8 lchip, ctc_chip_serdes_loopback_t* p_loopback)
{
    uint8  acc_id = 0;
    uint16 mask   = 0;
    uint16 data   = 0;
    uint16 addr   = 0;

    if(SYS_DATAPATH_SERDES_28G_START_ID > p_loopback->serdes_id)
    {
        return CTC_E_NONE;
    }
    acc_id = (p_loopback->serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) / HSS28G_LANE_NUM;

    /*Register 9803h: LOOPBACK FIFO    Bit15 - LOOPBACK_EN*/
    addr   = 0x9803;
    mask   = 0x7fff;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr, mask, &data));
    
    if(1 == data)
    {
        p_loopback->enable = 1;
    }
    else
    {
        p_loopback->enable = 0;
    }

    return CTC_E_NONE;
}

/* serdes loopback config */
int32
sys_tsingma_datapath_set_serdes_loopback(uint8 lchip, void* p_data)
{
    uint32 gport = 0;
    uint16 lport = 0;
    ctc_chip_serdes_loopback_t* p_loopback = (ctc_chip_serdes_loopback_t*)p_data;

    CTC_PTR_VALID_CHECK(p_loopback);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d, type:%d\n", p_loopback->serdes_id, p_loopback->enable, p_loopback->mode);

    if(TSINGMA_SERDES_NUM_PER_SLICE <= p_loopback->serdes_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_datapath_set_serdes_loopback: serdes id %d out of bound!\n", p_loopback->serdes_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_datapath_get_gport_with_serdes(lchip, p_loopback->serdes_id, &gport));
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if (p_usw_mac_master[lchip]->mac_prop[lport].cl73_enable)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] CL73 Auto-Nego is enable cannot set serdes loopback \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*Internal loopback (0) for 12G is LM1 mode, for 28G no such mode;
    External loopback (1) for 12G is LS1 mode, for 28G is external mode.
    Due to the limitation of ctc command, other mode do not support now.*/
    switch(p_loopback->mode)
    {
        case 0: /* Internal Loopback */
            if(!SYS_DATAPATH_SERDES_IS_HSS28G(p_loopback->serdes_id))
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_loopback_hss12g_lm1(lchip, p_loopback));
            }
            else
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [SerDes] HSS28G SerDes %d DO NOT support internal loopback!\n", p_loopback->serdes_id);
                return CTC_E_NOT_SUPPORT;
            }
            break;
        case 1: /* External Loopback */
            if(!SYS_DATAPATH_SERDES_IS_HSS28G(p_loopback->serdes_id))
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_loopback_hss12g_ls2(lchip, p_loopback));
            }
            else
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_loopback_hss28g_ext(lchip, p_loopback));
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }



    return CTC_E_NONE;
}

/* serdes internal and external loopback */
int32
sys_tsingma_datapath_get_serdes_loopback(uint8 lchip, void* p_data)
{
    ctc_chip_serdes_loopback_t* p_loopback = (ctc_chip_serdes_loopback_t*)p_data;
    CTC_PTR_VALID_CHECK(p_loopback);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d, type:%d\n", p_loopback->serdes_id, p_loopback->enable, p_loopback->mode);

    p_loopback->enable = 0;

    /*Internal loopback (0) for 12G is LM1 mode, for 28G no such mode;
    External loopback (1) for 12G is LS1 mode, for 28G is external mode.
    Other loopback mode does not support now.*/
    switch(p_loopback->mode)
    {
        case 0: /* Internal Loopback */
            if(!SYS_DATAPATH_SERDES_IS_HSS28G(p_loopback->serdes_id))
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_get_loopback_hss12g_lm1(lchip, p_loopback));
            }
            else
            {
                /* HSS28G Not Support internal Loopback */
                p_loopback->enable = 0;
            }
            break;
        case 1: /* External Loopback */
            if(!SYS_DATAPATH_SERDES_IS_HSS28G(p_loopback->serdes_id))
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_get_loopback_hss12g_ls2(lchip, p_loopback));
            }
            else
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_get_loopback_hss28g_ext(lchip, p_loopback));
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/*set CTLE auto enable, or set manual value*/
int32
sys_tsingma_datapath_set_serdes_ctle(uint8 lchip, void* p_data)
{
    uint8  ctle_type = 0; /*0-VGA  1-EQC  2-EQR*/
    uint8  hss_id    = 0;
    uint8  lane_id   = 0;
    uint16 mask      = 0x0010;
    uint16 addr      = 0x804e;
    uint32 time_cnt  = 1000;
    uint32 addr_u32  = 0;
    uint32 value     = 0;
    uint32 addr_u32_base[3] = {0x004e061c, 0x0056061c, 0x005e061c};
    uint32 addr_u32_ctl[3]  = {0x004e0020, 0x00560020, 0x005e0020};
    ctc_chip_serdes_ctle_t* p_ctle = (ctc_chip_serdes_ctle_t*)p_data;
    CTC_PTR_VALID_CHECK(p_ctle);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d\n", p_ctle->serdes_id, p_ctle->auto_en);

    hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(p_ctle->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_ctle->serdes_id);

    /*CTLE auto adjust enable*/
    if(1 == p_ctle->auto_en)
    {
        if(SYS_DATAPATH_SERDES_IS_HSS28G(p_ctle->serdes_id))
        {
            /*28G auto CTLE default enable, and it keeps working and updating config untill manual value set*/
            mask = mask << lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, 0x980f, mask, 0));
        }
        else
        {
            /*12G auto CTLE default disable. After enable, the MCU FW will find best value and update config only once*/
            addr_u32 = addr_u32_base[hss_id] + lane_id*0x20;
            CTC_ERROR_RETURN(drv_usw_chip_write(lchip, addr_u32, 1));
            CTC_ERROR_RETURN(drv_usw_chip_write(lchip, addr_u32_ctl[hss_id], 3));
            while(--time_cnt)
            {
                CTC_ERROR_RETURN(drv_usw_chip_read(lchip, addr_u32, &value));
                if(0 == value)
                {
                    break;
                }
                sal_task_sleep(10);
            }
            if(!time_cnt)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "CTLE auto adjust timeout!\n");
                return CTC_E_HW_TIME_OUT;
            }
        }
    }
    /*CTLE manual value config*/
    else
    {
        if(SYS_DATAPATH_SERDES_IS_HSS28G(p_ctle->serdes_id))
        {
            /*28G only have 1 CLTE reg*/
            if(7 < p_ctle->value[0])
            {
                 SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "CTLE value out of bound (max 7)! serdes_id=%u, value=%u\n", 
                                      p_ctle->serdes_id, p_ctle->value[0]);
                 return CTC_E_INVALID_CONFIG;
            }
            mask = mask << lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, 0x980f, mask, 1));
            addr+= 0x0100*lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, 0xc7ff, p_ctle->value[0]));
        }
        else
        {
            /*12G has 3 CLTE reg: VGA/EQC/EQR*/
            if((15 < p_ctle->value[0]) || (15 < p_ctle->value[1]) || (15 < p_ctle->value[2]))
            {
                 SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "CTLE value out of bound (max 15/15/15)! serdes_id=%u, value=%u/%u/%u\n", 
                                      p_ctle->serdes_id, p_ctle->value[0], p_ctle->value[1], p_ctle->value[2]);
                 return CTC_E_INVALID_CONFIG;
            }
            for(ctle_type = 0; ctle_type < 3; ctle_type++)
            {
                CTC_ERROR_RETURN(sys_tsingma_serdes_hss12g_set_ctle_value(p_ctle->serdes_id, ctle_type, ((uint8)(p_ctle->value[ctle_type]))));
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_ctle(uint8 lchip, void* p_data)
{
    uint8  hss_id = 0;
    uint8  lane_id = 0;
    uint8  step    = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaVgaCtrl_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f;
    uint16 addr = 0x804e;
    uint32 tbl_id  = 0;
    uint32 cmd     = 0;
    uint32 value = 0;
    ctc_chip_serdes_ctle_t* p_ctle = (ctc_chip_serdes_ctle_t*)p_data;
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;
    CTC_PTR_VALID_CHECK(p_ctle);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", p_ctle->serdes_id);

    hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(p_ctle->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_ctle->serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(p_ctle->serdes_id))
    {
        addr += 0x0100*lane_id;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, hss_id, addr, 0xc7ff, &(p_ctle->value[0])));
    }
    else
    {
        tbl_id = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f + step*lane_id), &value, &rx_eq_cfg);
        p_ctle->value[0] = (uint16)value;
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqcForce_f + step*lane_id), &value, &rx_eq_cfg);
        p_ctle->value[1] = (uint16)value;
        DRV_IOR_FIELD(lchip, tbl_id, (Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqRes_f + step*lane_id), &value, &rx_eq_cfg);
        p_ctle->value[2] = (uint16)value;
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    CTC_PTR_VALID_CHECK(p_value);
    switch (chip_prop)
    {
    case CTC_CHIP_PROP_SERDES_PRBS:
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_FFE:
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_ffe(lchip, p_value));
        break;
    case CTC_CHIP_PEOP_SERDES_POLARITY:
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_LOOPBACK:
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_loopback(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_P_FLAG:
    case CTC_CHIP_PROP_SERDES_PEAK:
    case CTC_CHIP_PROP_SERDES_DPC:
    case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s, %d\n", __FUNCTION__, __LINE__);
        return CTC_E_NOT_SUPPORT;
    case CTC_CHIP_PROP_SERDES_CTLE:
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_ctle(lchip, p_value));
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    uint16 coefficient[SYS_USW_CHIP_FFE_PARAM_NUM] = {0};
    uint8 index = 0;
    uint8 value = 0;
    ctc_chip_serdes_ffe_t* p_ffe = NULL;
    CTC_PTR_VALID_CHECK(p_value);
    switch (chip_prop)
    {
    case CTC_CHIP_PROP_SERDES_FFE:
        p_ffe = (ctc_chip_serdes_ffe_t*)p_value;
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe(lchip, p_ffe->serdes_id, coefficient, p_ffe->mode, &value));
        for (index = 0; index < SYS_USW_CHIP_FFE_PARAM_NUM; index++)
        {
            p_ffe->coefficient[index] = coefficient[index];
        }
        p_ffe->status = value;
        break;
    case CTC_CHIP_PEOP_SERDES_POLARITY:
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_polarity(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_LOOPBACK:
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, p_value));
        break;
    case CTC_CHIP_PROP_SERDES_P_FLAG:
    case CTC_CHIP_PROP_SERDES_PEAK:
    case CTC_CHIP_PROP_SERDES_DPC:
    case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s, %d\n", __FUNCTION__, __LINE__);
        break;
    case CTC_CHIP_PROP_SERDES_CTLE:
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ctle(lchip, p_value));
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_ffe_status(uint8 lchip, uint8 serdes_id, uint8 mode, uint8* p_value)
{
    uint32 cmd     = 0;
    uint32 tbl_id  = 0;
    uint32 fld_id  = 0;    
    uint32 en_main = 0;
    uint32 en_adv  = 0;
    uint32 en_dly  = 0;
    uint8  step    = 0;
    uint8  hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8  lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    Hss12GLaneTxEqCfg0_m tx_eq_cfg;
    Hss28GCfg_m hss_an;

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        if (CTC_CHIP_SERDES_FFE_MODE_3AP == mode)
        {
            cmd = DRV_IOR(Hss28GCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_an));
            step = Hss28GCfg_cfgHssL1AutoNegStatus_f - Hss28GCfg_cfgHssL0AutoNegStatus_f;
            fld_id = Hss28GCfg_cfgHssL0AutoNegStatus_f + step*(serdes_id%8);
            DRV_IOR_FIELD(lchip, Hss28GCfg_t, fld_id, &en_main, &hss_an);
            if (0 == en_main)
            {
                *p_value = 0;
            }
            else
            {
                *p_value = 1;
            }
        }
        else
        {
            *p_value = 1;
        }
    }
    else
    {
        tbl_id = Hss12GLaneTxEqCfg0_t + hss_id * (Hss12GLaneTxEqCfg1_t - Hss12GLaneTxEqCfg0_t);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_eq_cfg));

        /*pcs_en_main 02h[1]  = 1'b1*/
        /*Hss12GLaneTxEqCfg[0~2].cfgHssL[0~7]PcsEnMain    {0,2,1}*/
        step = Hss12GLaneTxEqCfg0_cfgHssL1PcsEnMain_f - Hss12GLaneTxEqCfg0_cfgHssL0PcsEnMain_f;
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsEnMain_f + step * lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &en_main, &tx_eq_cfg);

        /*pcs_en_adv 02h[0]   = 1'b1*/
        /*Hss12GLaneTxEqCfg[0~2].cfgHssL[0~7]PcsEnAdv    {0,0,1}*/
        step = Hss12GLaneTxEqCfg0_cfgHssL1PcsEnAdv_f - Hss12GLaneTxEqCfg0_cfgHssL0PcsEnAdv_f;
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsEnAdv_f + step * lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &en_adv, &tx_eq_cfg);

        /*pcs_en_dly 02h[2]   = 1'b1*/
        /*Hss12GLaneTxEqCfg[0~2].cfgHssL[0~7]PcsEnDly    {0,1,1}*/
        step = Hss12GLaneTxEqCfg0_cfgHssL1PcsEnDly_f - Hss12GLaneTxEqCfg0_cfgHssL0PcsEnDly_f;
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsEnDly_f + step * lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &en_dly, &tx_eq_cfg);

        if(en_main && en_adv && en_dly)
        {
            *p_value = 1;
        }
        else
        {
            *p_value = 0;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_ffe_typical_value(uint8 lchip, ctc_chip_serdes_ffe_t* p_ffe)
{
    uint8  hss_idx  = 0;
    uint8  lane_id  = 0;
    uint8  group_id = 0;
    uint8  coeff_id = 0;
    uint16 coefficient[7][CTC_CHIP_FFE_PARAM_NUM] = {
        {2,  70,  7,  0,  0 },  /*12G  FR4 0~4 inch, optical module*/
        {2,  90,  11, 0,  0 },  /*12G  FR4 4~7 inch, optical module*/
        {1,  40,  4,  0,  0 },  /*12G  M4 0~4 inch, optical module*/
        {2,  70,  5,  0,  0 },  /*12G  M4 4~7 inch, optical module*/
        {63, 20,  60, 0,  0 },  /*28G  25G per-lane, optical module*/
        {0,  15,  63, 0,  0 },  /*28G  10G per-lane, optical module*/
        {0,  15,  63, 0,  0 },  /*28G  1G/2.5G/5G per-lane, optical module*/
    };
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    if(SYS_DATAPATH_SERDES_IS_HSS28G(p_ffe->serdes_id))
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_ffe->serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_ffe->serdes_id);

        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        
        if(SYS_DATAPATH_IS_SERDES_25G_PER_LANE(p_hss_vec->serdes_info[lane_id].mode))
        {
            group_id = 4;
        }
        else if(SYS_DATAPATH_IS_SERDES_10G_PER_LANE(p_hss_vec->serdes_info[lane_id].mode))
        {
            group_id = 5;
        }
        else if((CTC_CHIP_SERDES_SGMII_MODE == p_hss_vec->serdes_info[lane_id].mode) || 
                (CTC_CHIP_SERDES_2DOT5G_MODE == p_hss_vec->serdes_info[lane_id].mode) || 
                (CTC_CHIP_SERDES_XAUI_MODE == p_hss_vec->serdes_info[lane_id].mode) || 
                (CTC_CHIP_SERDES_DXAUI_MODE == p_hss_vec->serdes_info[lane_id].mode))
        {
            group_id = 6;
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized (illegal mode)\n");
            return CTC_E_HW_FAIL;
        }
    }
    else
    {
        if(0 == p_ffe->trace_len) //0~4 inch
        {
            if(0 == p_ffe->board_material) //FR4
            {
                group_id = 0;
            }
            else if(1 == p_ffe->board_material) //M4
            {
                group_id = 2;
            }
            else
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized (0~4 inch, M6 material)\n");
                return CTC_E_HW_FAIL;
            }
        }
        else if(1 == p_ffe->trace_len)  //4~7 inch
        {
            if(0 == p_ffe->board_material) //FR4
            {
                group_id = 1;
            }
            else if(1 == p_ffe->board_material) //M4
            {
                group_id = 3;
            }
            else
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized (4~7 inch, M6 material)\n");
                return CTC_E_HW_FAIL;
            }
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized (more than 7 inch)\n");
            return CTC_E_HW_FAIL;
        }
    }

    for(coeff_id = 0; coeff_id < CTC_CHIP_FFE_PARAM_NUM; coeff_id++)
    {
        p_ffe->coefficient[coeff_id] = coefficient[group_id][coeff_id];
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_ffe_traditional(uint8 lchip, ctc_chip_serdes_ffe_t* p_ffe)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   serdes_id = 0;
    uint8   hss_type = 0;
    uint32  tmp_val32 = 0;
    uint16  coefficient[5] = {0};
    uint32  lane_step  = 0;
    uint32  tbl_id     = 0;
    uint32  fld_id     = 0;
    uint32  cmd        = 0;
    uint8  acc_id     = 0;
    /*uint8  addr   = 0;
    uint8  mask   = 0;
    uint8  data   = 0;*/

    uint16 addr16     = 0;
    uint16 mask16     = 0;
    uint16 data16     = 0;
    int32  ret = 0;
    uint32 lock_id   = DRV_MCU_LOCK_TXEQ_CFG;
    
    sys_datapath_hss_attribute_t* p_hss = NULL;
    Hss12GLaneTxEqCfg0_m tx_tap;

    /* debug param */
    CTC_PTR_VALID_CHECK(p_ffe);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, C0:%d, C1:%d, C2:%d, C3:%d\n", p_ffe->serdes_id, p_ffe->coefficient[0], p_ffe->coefficient[1], p_ffe->coefficient[2], p_ffe->coefficient[3]);

    /*typical mode not support now*/
    if(CTC_CHIP_SERDES_FFE_MODE_TYPICAL == p_ffe->mode)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_ffe_typical_value(lchip, p_ffe));
    }

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

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_HW_FAIL;
    }

    coefficient[0] = p_ffe->coefficient[0];
    coefficient[1] = p_ffe->coefficient[1];
    coefficient[2] = p_ffe->coefficient[2];
    coefficient[3] = p_ffe->coefficient[3];
    coefficient[4] = p_ffe->coefficient[4];

    /* write to HW tables */
    if (SYS_DATAPATH_HSS_TYPE_15G == hss_type)  /* HSS12G */
    {
        /*coefficient[0]  pcs_tap_adv       0~15*/
        /*coefficient[1]  pcs2pma_txmargin  0~255*/
        /*coefficient[2]  pcs_tap_dly       0~31*/
        if((coefficient[0] > 15) || (coefficient[1] > 255) || (coefficient[2] > 31))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Coefficient out of bound! c0 0~15, c1 0~255, c2 0~31\n");
            return CTC_E_INVALID_CONFIG;
        }

        ret = drv_usw_mcu_lock(lchip, lock_id, hss_id);
        if(0 != ret)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Get lock error (TXEQ)! hss_id=%u, lane_id=%u\n", hss_idx, lane_id);
            return CTC_E_HW_TIME_OUT;
        }

        lane_step = Hss12GLaneTxEqCfg0_cfgHssL1Pcs2PmaTxMargin_f - Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f;
        tbl_id = Hss12GLaneTxEqCfg0_t + hss_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_tap));
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsEnAdv_f + lane_step*lane_id;
        tmp_val32 = 1;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsTapAdv_f + lane_step*lane_id;
        tmp_val32 = coefficient[0];
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f + lane_step*lane_id;
        tmp_val32 = coefficient[1];
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsEnDly_f + lane_step*lane_id;
        tmp_val32 = 1;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsTapDly_f + lane_step*lane_id;
        tmp_val32 = coefficient[2];
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_tap));

        ret = drv_usw_mcu_unlock(lchip, lock_id, hss_id);
        if (0 != ret)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Get unlock error (RXEQ)! hss_id=%u, lane_id=%u\n", hss_idx, lane_id);
            return CTC_E_HW_TIME_OUT;
        }
    }
    else   /* HSS28G */
    {
        /*coefficient[0]  PRE-CURSOR        0~63*/
        /*coefficient[1]  MAIN TAP          0~63*/
        /*coefficient[2]  POST-CURSOR TAP1  0~63*/
        /*coefficient[3]  POST-CURSOR TAP2  0~63*/
        /*coefficient[4]  POST-CURSOR TAP3  0~63*/
        if((coefficient[0] > 63) || (coefficient[1] > 63) || (coefficient[2] > 63) || 
           (coefficient[3] > 63) || (coefficient[4] > 63))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Coefficient out of bound! c0/1/2/3/4 0~63\n");
            return CTC_E_INVALID_CONFIG;
        }
        /*1. register 8zA8h  TX FIR PRE-CURSOR COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a8 + lane_id * 0x100;
        mask16  = 0xc0ff;
        data16  = coefficient[0];
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr16, mask16, data16));
        
        /*2. register 8zA7h  TX FIR MAIN TAP COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a7 + lane_id * 0x100;
        mask16  = 0xc0ff;
        data16  = coefficient[1];
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr16, mask16, data16));

        /*3. register 8zA6h  TX FIR POST-CURSOR TAP1 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a6 + lane_id * 0x100;
        mask16  = 0xc0ff;
        data16  = coefficient[2];
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr16, mask16, data16));

        /*4. register 8zA5h  TX FIR POST-CURSOR TAP2 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a5 + lane_id * 0x100;
        mask16  = 0xc0ff;
        data16  = coefficient[3];
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr16, mask16, data16));

        /*5. register 8zA4h  TX FIR POST-CURSOR TAP3 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a4 + lane_id * 0x100;
        mask16  = 0xc0ff;
        data16  = coefficient[4];
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, addr16, mask16, data16));
    }
    return CTC_E_NONE;
}

/* set serdes ffe */
int32
sys_tsingma_datapath_set_serdes_ffe(uint8 lchip, void* p_data)
{
    ctc_chip_serdes_ffe_t* p_ffe = (ctc_chip_serdes_ffe_t*)p_data;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, mode:%d\n", p_ffe->serdes_id, p_ffe->mode);

    CTC_PTR_VALID_CHECK(p_ffe);
    if (p_ffe->serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    else
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_ffe_traditional(lchip, p_ffe));
    }

    return CTC_E_NONE;
}


/* get serdes ffe with traditional mode */
int32
sys_tsingma_datapath_get_serdes_ffe_traditional(uint8 lchip, uint8 serdes_id, uint16* coefficient)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint8   hss_type = 0;
    uint32  lane_step  = 0;
    uint32  tbl_id     = 0;
    uint32  fld_id     = 0;
    uint32  cmd        = 0;
    uint32  tmp_val32  = 0;
    uint16  tmp_val16  = 0;
    uint8  acc_id     = 0;
    uint16 addr16     = 0;
    uint16 mask16     = 0;
    
    sys_datapath_hss_attribute_t* p_hss = NULL;
    Hss12GLaneTxEqCfg0_m tx_tap;

    /* debug param */
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    //SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, C0:%d, C1:%d, C2:%d, C3:%d\n", p_ffe->serdes_id, p_ffe->coefficient[0], p_ffe->coefficient[1], p_ffe->coefficient[2], p_ffe->coefficient[3]);

    CTC_PTR_VALID_CHECK(coefficient);

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

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (!p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_HW_FAIL;
    }

    /* read to HW tables */
    if (SYS_DATAPATH_HSS_TYPE_15G == hss_type)  /* HSS12G */
    {
        lane_step = Hss12GLaneTxEqCfg0_cfgHssL1Pcs2PmaTxMargin_f - Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f;
        tbl_id = Hss12GLaneTxEqCfg0_t + hss_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_tap));
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsTapAdv_f + lane_step*lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        coefficient[0] = (uint16)tmp_val32;
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0Pcs2PmaTxMargin_f + lane_step*lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        coefficient[1] = (uint16)tmp_val32;
        fld_id = Hss12GLaneTxEqCfg0_cfgHssL0PcsTapDly_f + lane_step*lane_id;
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &tx_tap);
        coefficient[2] = (uint16)tmp_val32;
    }
    else   /* HSS28G */
    {
        /*1. register 8zA8h  TX FIR PRE-CURSOR COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a8 + lane_id * 0x100;
        mask16  = 0xc0ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr16, mask16, &tmp_val16));
        coefficient[0] = tmp_val16;
        
        /*2. register 8zA7h  TX FIR MAIN TAP COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a7 + lane_id * 0x100;
        mask16  = 0xc0ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr16, mask16, &tmp_val16));
        coefficient[1] = tmp_val16;

        /*3. register 8zA6h  TX FIR POST-CURSOR TAP1 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a6 + lane_id * 0x100;
        mask16  = 0xc0ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr16, mask16, &tmp_val16));
        coefficient[2] = tmp_val16;

        /*4. register 8zA5h  TX FIR POST-CURSOR TAP2 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a5 + lane_id * 0x100;
        mask16  = 0xc0ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr16, mask16, &tmp_val16));
        coefficient[3] = tmp_val16;

        /*5. register 8zA4h  TX FIR POST-CURSOR TAP3 COEFFICIENT */
        acc_id  = hss_id;
        addr16  = 0x80a4 + lane_id * 0x100;
        mask16  = 0xc0ff;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, addr16, mask16, &tmp_val16));
        coefficient[4] = tmp_val16;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_serdes_get_hss28g_fw_info(uint8 lchip, uint8 acc_id)
{
    uint16 reg_address[] = {0x9816, 0x9815};
    uint32 checktime = 0;
    uint16 status = 0;
    uint8 pass = 0;
    uint32 hashcode = 0;
    
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_address[1], 0, 0xf000));
    for (; checktime < 1000; checktime++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_address[1], 0, &status));
        if (0 == (status & 0xf000))
        {
            pass = 1;
            break;
        }
    }
    
    if (0 == pass)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[HSS28G Firmware] Failed to get fw version\n");
        return CTC_E_HW_FAIL;
    }

    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_address[1], 0, &status));
    hashcode = ((status&0xff) << 16);
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_address[0], 0, &status));
    hashcode |= status;

    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, SYS_TSINGMA_MCU_3_VERSION_BASE_ADDR, hashcode));
    
    return CTC_E_NONE;
}

/***********************************************************************************************************************
1)traning state: training_state = BE_debug(lane_num, 2, 1) # 0: disable, 1: running, 2: done
2)training FFE value: tap0 = BE_debug(lane_num, 2, 2); tap1 = BE_debug(lane_num, 2, 3); tapm1 = BE_debug(lane_num, 2, 4)
************************************************************************************************************************/
STATIC int32
_sys_tsingma_serdes_get_hss28g_link_training_status(uint8 lchip, uint16 serdes_id, uint16 fw_index, uint16* p_value)
{
    uint16 reg_address[] = {0x9816, 0x9815};
    uint16 data = 0;
    uint16 status = 0;
    uint8 acc_id = 0;
    uint8 lane_id = 0;
    uint16 check_value = 0;
    uint8 right_status = 0;
    uint32 checktime = 0;
    uint8 fw_mode = 2;  /* BE_debug(lane_num, 2, 1) */

    if (SYS_DATAPATH_SERDES_28G_START_ID > serdes_id)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid SerDes ID \n");
        return CTC_E_INVALID_PARAM;
    }

    acc_id = (serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) / 4;
    lane_id = (serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) % 4;

    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_address[0], 0, fw_index));
    data = 0xb000 + ((fw_mode & 0xf) << 4) + lane_id;
    check_value = 0x0b00 + fw_mode;

    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_address[1], 0, data));

    for (; checktime < 1000; checktime++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_address[1], 0, &status));
        if (status != data)
        {
            right_status = 1;
            break;
        }
    }
    
    if (right_status)
    {
        if (status != check_value)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[HSS28G Firmware] Debug command failed with code %04x\n", status);
            return CTC_E_HW_FAIL;
        }
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[HSS28G Firmware] Command 0x%04x failed. is firmware loaded?\n", data);
        return CTC_E_HW_FAIL;
    }
    
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_address[0], 0, &status));

    *p_value = status;
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_hss28g_ctle_done(uint8 lchip, uint8 serdes_id, uint8* done)
{
    uint8 acc_id = 0;
    uint8 lane_id = 0;
    uint16 mask = 0;
    uint16 status = 0;
    
    acc_id = (serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) / 4;
    lane_id = (serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) % 4;

    mask = 0xfff0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, 0x9811, mask, &status));
    status >>= lane_id;

    SYS_USW_VALID_PTR_WRITE(done, (uint8)(status & 0x1));
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_serdes_ffe_3ap(uint8 lchip, uint8 serdes_id, uint16* coefficient)
{
    CTC_PTR_VALID_CHECK(coefficient);

    if (SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /* c0(tapm1): BE_debug(lane_num, 2, 4) */
        CTC_ERROR_RETURN(_sys_tsingma_serdes_get_hss28g_link_training_status(lchip, serdes_id, 4, &coefficient[0]));
        /* c1(tap0): BE_debug(lane_num, 2, 2) */
        CTC_ERROR_RETURN(_sys_tsingma_serdes_get_hss28g_link_training_status(lchip, serdes_id, 2, &coefficient[1]));
        /* c2(tap1): BE_debug(lane_num, 2, 3) */
        CTC_ERROR_RETURN(_sys_tsingma_serdes_get_hss28g_link_training_status(lchip, serdes_id, 3, &coefficient[2]));
        coefficient[0] &= 0x3f;
        coefficient[1] &= 0x3f;
        coefficient[2] &= 0x3f;
        coefficient[3] = 0;
        coefficient[4] = 0;
    }
    else
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe_traditional(lchip, serdes_id, coefficient));
    }

    return CTC_E_NONE;
}

/* get serdes ffe */
int32
sys_tsingma_datapath_get_serdes_ffe(uint8 lchip, uint8 serdes_id, uint16* coefficient, uint8 mode, uint8* status)
{
    uint16 cl72_status = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, ffe_mode:%d\n", serdes_id, mode);

    if (serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_tsingma_serdes_get_link_training_status(lchip, serdes_id, &cl72_status));
    CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe_status(lchip, serdes_id, mode, status));

    if(CTC_CHIP_SERDES_FFE_MODE_3AP == mode)
    {
        if(SYS_PORT_CL72_DISABLE == cl72_status)
        {
            *status = 0;
        }
    }
    else
    {
        if(SYS_PORT_CL72_DISABLE != cl72_status)
        {
            *status = 0;
        }
    }

    if(CTC_CHIP_SERDES_FFE_MODE_3AP == mode)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe_3ap(lchip, serdes_id, coefficient));
    }
    else
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_ffe_traditional(lchip, serdes_id, coefficient));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_sm_reset(uint8 lchip, uint16 serdes_id)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint16 addr      = 0;
    uint16 mask      = 0;
    uint16 data      = 0;
    
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d\n", serdes_id);

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        return CTC_E_NONE;
    }

    addr = 0x8081 + lane_id * 0x100;
    mask = 0xf7ff;  /*bit[11]*/
    data = 1; /* reset */
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));
    sal_task_sleep(10);
    data = 0; /* release */
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));
    sal_task_sleep(10);
    
    return CTC_E_NONE;
}

/* get serdes signal detect */
uint32
sys_tsingma_datapath_get_serdes_sigdet(uint8 lchip, uint16 serdes_id)
{
    uint8   hss_id    = 0;
    uint8   hss_idx   = 0;
    uint8   lane_id   = 0;
    uint32  value_pcs = 0;
    uint32  value_mon = 0;
    uint32  cmd       = 0;
    uint32  tbl_id    = 0;
    uint32  fld_id    = 0;
    uint8   step      = 0;
    SharedPcsSerdes0Cfg0_m  serdes_cfg;
    QsgmiiPcsCfg0_m  qsgmii_serdes_cfg;
    UsxgmiiPcsSerdesCfg0_m usxgmii_serdes_cfg;
    Hss12GLaneMon0_m hss12g_mon;
    Hss28GMon_m      hss28g_mon;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_serdes_info_t* p_serdes    = NULL;

    sal_memset(&serdes_cfg, 0, sizeof(SharedPcsSerdes0Cfg0_m));
    sal_memset(&qsgmii_serdes_cfg, 0, sizeof(QsgmiiPcsCfg0_m));
    sal_memset(&usxgmii_serdes_cfg, 0, sizeof(UsxgmiiPcsSerdesCfg0_m));
    sal_memset(&hss12g_mon, 0, sizeof(Hss12GLaneMon0_m));
    sal_memset(&hss28g_mon, 0, sizeof(Hss28GMon_m));

    /* 1. get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    /* 2. read Lane Mon SigDet info */
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        tbl_id = Hss12GLaneMon0_t + hss_id;
        step = Hss12GLaneMon0_monHssL1RxEiFiltered_f - Hss12GLaneMon0_monHssL0RxEiFiltered_f;
        fld_id = Hss12GLaneMon0_monHssL0RxEiFiltered_f + step*lane_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12g_mon));
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &value_mon, &hss12g_mon);
    }
    else
    {
        tbl_id = Hss28GMon_t;
        step = Hss28GMon_monHssL1RxSigDet_f - Hss28GMon_monHssL0RxSigDet_f;
        fld_id = Hss28GMon_monHssL0RxSigDet_f + step*(lane_id+hss_id*4);
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_mon));
        DRV_IOR_FIELD(lchip, tbl_id, fld_id, &value_mon, &hss28g_mon);
    }

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if(p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    p_serdes = &p_hss_vec->serdes_info[lane_id];

    /* 3. read force signal detect */
    if(CTC_CHIP_SERDES_QSGMII_MODE == p_serdes->mode)
    {
        tbl_id = QsgmiiPcsCfg0_t + serdes_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qsgmii_serdes_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, QsgmiiPcsCfg0_forceSignalDetect_f, &value_pcs, &qsgmii_serdes_cfg);
    }
    else if(CTC_CHIP_SERDES_USXGMII0_MODE == p_serdes->mode)
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + (serdes_id/4)*4;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, (UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f+(serdes_id%4)), &value_pcs, &usxgmii_serdes_cfg);
    }
    else if((CTC_CHIP_SERDES_USXGMII1_MODE == p_serdes->mode)
        || (CTC_CHIP_SERDES_USXGMII2_MODE == p_serdes->mode))
    {
        tbl_id = UsxgmiiPcsSerdesCfg0_t + serdes_id;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &usxgmii_serdes_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, UsxgmiiPcsSerdesCfg0_forceSignalDetect0_f, &value_pcs, &usxgmii_serdes_cfg);
    }
    else
    {
        step = SharedPcsSerdes1Cfg0_t - SharedPcsSerdes0Cfg0_t;
        tbl_id = SharedPcsSerdes0Cfg0_t + (serdes_id/4) + (serdes_id%4)*step;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &serdes_cfg));
        DRV_IOR_FIELD(lchip, tbl_id, SharedPcsSerdes0Cfg0_forceSignalDetect0_f, &value_pcs, &serdes_cfg);
    }

    return (value_pcs || value_mon);
}

int16
sys_tsingma_datapath_12g_set_auto_neg_default_eq(uint8 lchip, uint8 serdes_id, uint8 an_en)
{
    uint8   hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    ctc_chip_serdes_ffe_t ffe;

    ffe.serdes_id = serdes_id;
    ffe.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
    if(an_en)
    {
        ffe.coefficient[0] = 2;
        ffe.coefficient[1] = (0 == hss_id) ? 95 : 75;
        ffe.coefficient[2] = (0 == hss_id) ? 11 : 7;
    }
    else
    {
        ffe.coefficient[0] = 0;
        ffe.coefficient[1] = 192;
        ffe.coefficient[2] = 0;
    }
    CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_ffe_traditional(lchip, &ffe));
    sys_tsingma_serdes_hss12g_set_ctle_value(serdes_id, 0, 5);
    sys_tsingma_serdes_hss12g_set_ctle_value(serdes_id, 1, 8);
    sys_tsingma_serdes_hss12g_set_ctle_value(serdes_id, 2, 10);

    return CTC_E_NONE;
}

/* Support 802.3ap, auto_neg for Backplane Ethernet */
int16
sys_tsingma_datapath_set_serdes_auto_neg_en(uint8 lchip, uint16 serdes_id, uint32 enable)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint16  tbl_id    = 0;
    uint32  cmd       = 0;
    uint32  value     = 0;
    uint16  fld_id    = 0;
    uint8   slice_id  = 0;
    Hss12GMacroCfg0_m    hs_macro;
    Hss28GMacroCfg_m    cs_macro;
    AnethCtl0_m      aneth_ctl;
    AnethAdvAbility0_m anethadv_ctl;
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

    /* enable cl73 atuo-neg */
    if(enable)
    {
        /* HSS12G */
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            /* #0, set local FFE & CTLE to default value */
            CTC_ERROR_RETURN(sys_tsingma_datapath_12g_set_auto_neg_default_eq(lchip, (uint8)serdes_id, TRUE));

            /* #1, Cfg Aneth mode */
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* #2, Cfg AdvAbility */
            tbl_id = AnethAdvAbility0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
            DRV_IOR_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
            if(1 != (value & 0x0000001f))
            {
                value &= 0xffffffe0;
                value |= 1;
                DRV_IOW_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
            }

            /* #3, Cfg Aec Bypass */
            tbl_id = Hss12GMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            value  = 0;
            fld_id = Hss12GMacroCfg0_cfgHssL0AecRxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecRxByp_f-Hss12GMacroCfg0_cfgHssL0AecRxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            value  = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PcsRxReadyEn_f-Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #4.1   cfg Hss12GMacroCfg[0..2]_cfg[0~7]AnethLinkStatusSel */
            if (CTC_PORT_SPEED_40G != port_attr->speed_mode)
            {
                value = lane_id % 4;
            }
            else
            {
                value = 0;
            }
            
            fld_id = Hss12GMacroCfg0_cfgHssL0AnethLinkStatusSel_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AnethLinkStatusSel_f-Hss12GMacroCfg0_cfgHssL0AnethLinkStatusSel_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #4.2   cfg Hss12GMacroCfg[0..2]_cfg[0~7]AnethAutoTxEn */
            value = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AnethAutoTxEn_f-Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            /* #4.3    Cfg Hss12GMacroCfg[0..2]_cfgLane[0~7]PmaReady = 1'b1;     */
            value = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0PmaReady_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PmaReady_f-Hss12GMacroCfg0_cfgHssL0PmaReady_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* cfg autoneg en */
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
        }
        /* HSS28G */
        else
        {
            /* #1, Cfg Aneth mode */
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            if ((CTC_PORT_SPEED_25G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_50G == port_attr->speed_mode)
                || (CTC_PORT_SPEED_100G == port_attr->speed_mode))
            {
                value = 3;
            }
            else
            {
                value = 1;
            }
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            /* #2, Cfg AdvAbility */
            if((CTC_PORT_SPEED_25G == port_attr->speed_mode) || (CTC_PORT_SPEED_50G == port_attr->speed_mode))
            {               
                tbl_id = AnethAdvAbility0_t + serdes_id;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
                DRV_IOR_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
                if(1 != (value & 0x0000001f))
                {
                    value &= 0xffffffe0;
                    value |= 1;
                    DRV_IOW_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
                }
            }
            else
            {
                tbl_id = AnethAdvAbility0_t + serdes_id;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
                DRV_IOR_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
                if(1 != (value & 0x0000001f))
                {
                    value &= 0xffffffe0;
                    value |= 1;
                    DRV_IOW_FIELD(lchip, tbl_id, AnethAdvAbility0_cfgAdvAbility_f, &value, &anethadv_ctl);
                    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, 0, cmd, &anethadv_ctl);
                }
            }

            /* #3, Enable Aneth */
            /* #3.1 Hss28GMacroCfg */
            tbl_id = Hss28GMacroCfg_t;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            fld_id = Hss28GMacroCfg_cfgHssL0AnethLinkStatusSel_f + (serdes_id%8)*
                     (Hss28GMacroCfg_cfgHssL1AnethLinkStatusSel_f-Hss28GMacroCfg_cfgHssL0AnethLinkStatusSel_f);
            if((CTC_PORT_SPEED_40G == port_attr->speed_mode)
                ||(CTC_PORT_SPEED_100G == port_attr->speed_mode))
            {
                value = 0;
            }
            else if(CTC_PORT_SPEED_50G == port_attr->speed_mode)
            {
                if(0 == ((serdes_id%8)/2)%2)
                {
                    value = 0;
                }
                else
                {
                    value = 2;
                }
            }
            else
            {
                value = lane_id;
            }
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);

            value = 1;
            fld_id = Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f + (serdes_id%8)*
                     (Hss28GMacroCfg_cfgHssL1AnethAutoTxEn_f-Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);
            fld_id = Hss28GMacroCfg_cfgHssL0PmaReady_f + (serdes_id%8)*
                     (Hss28GMacroCfg_cfgHssL1PmaReady_f-Hss28GMacroCfg_cfgHssL0PmaReady_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);

            /* #3.2 AnethCtl#[24-31].cfgAutoNegEnable */
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            fld_id = AnethCtl0_cfgAutoNegEnable_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
        }
    }
    /* AN disable */
    else
    {
        /* HSS12G */
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            
            /*set to default value*/
            value = 0;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            value = 1;
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            tbl_id = Hss12GMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            value  = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0AecRxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecRxByp_f-Hss12GMacroCfg0_cfgHssL0AecRxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            value  = 0;
            fld_id = Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PcsRxReadyEn_f-Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            fld_id = Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AnethAutoTxEn_f-Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            fld_id = Hss12GMacroCfg0_cfgHssL0PmaReady_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PmaReady_f-Hss12GMacroCfg0_cfgHssL0PmaReady_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);
            
            /*set to default value*/
            value  = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PcsRxReadyEn_f-Hss12GMacroCfg0_cfgHssL0PcsRxReadyEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &hs_macro);

            /* #0, set local FFE & CTLE to default value */
            CTC_ERROR_RETURN(sys_tsingma_datapath_12g_set_auto_neg_default_eq(lchip, (uint8)serdes_id, FALSE));
        }
        /* HSS28G */
        else
        {
            value = 0;
            tbl_id = AnethCtl0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAutoNegEnable_f, &value, &aneth_ctl);
            /*set to default value*/
            DRV_IOW_FIELD(lchip, tbl_id, AnethCtl0_cfgAnethBitDivSel_f, &value, &aneth_ctl);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &aneth_ctl);

            tbl_id = Hss28GMacroCfg_t;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
            fld_id = Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f + (serdes_id%8)*
                     (Hss28GMacroCfg_cfgHssL1AnethAutoTxEn_f-Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);
            fld_id = Hss28GMacroCfg_cfgHssL0PmaReady_f + (serdes_id%8)*
                     (Hss28GMacroCfg_cfgHssL1PmaReady_f-Hss28GMacroCfg_cfgHssL0PmaReady_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &cs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &cs_macro);
        }
    }
    return CTC_E_NONE;
}


/* Support 802.3ap, auto link training for Backplane Ethernet */
int32
sys_tsingma_serdes_set_link_training_en(uint8 lchip, uint16 serdes_id, bool enable)
{
    uint8   hss_id    = 0;
    uint8   lane_id   = 0;
    uint32  tbl_id    = 0;
    uint32  fld_id    = 0;
    uint32  value     = 0;
    uint32  cmd       = 0;
    Hss12GMacroCfg0_m hs_macro;
    KrLinkCtlCfg0_m   kr_cfg;
    Hss12GUnitReset0_m hs_rst;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d, enable_value:%d\n", serdes_id, enable);

    /* get serdes info by serdes id */
    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    /* operation */
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /* HsMacroCfg */
        if(!enable)  /*disable flow*/
        {
            /*stop MCU link training before reset KrLink*/
            CTC_ERROR_RETURN(sys_tsingma_mcu_set_cl72_enable(lchip, hss_id, lane_id, FALSE));

            value = 1;
            tbl_id = Hss12GMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_macro));
            fld_id = Hss12GMacroCfg0_cfgHssL0AecRxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecRxByp_f-Hss12GMacroCfg0_cfgHssL0AecRxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            fld_id = Hss12GMacroCfg0_cfgHssL0AecTxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecTxByp_f-Hss12GMacroCfg0_cfgHssL0AecTxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_macro));

            tbl_id = Hss12GUnitReset0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));
            fld_id = Hss12GUnitReset0_resetCoreKrLinkCtl0_f +
                     lane_id*(Hss12GUnitReset0_resetCoreKrLinkCtl1_f-Hss12GUnitReset0_resetCoreKrLinkCtl0_f);
            DRV_IOR_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
            if(0 == value)
            {
                /* KrLinkCtlCfg */
                value = 0;
                tbl_id = KrLinkCtlCfg0_t + serdes_id;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &kr_cfg));
                fld_id = KrLinkCtlCfg0_cfgFsmTrEn_f;
                DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &kr_cfg));

                /* HsMacroCfg */
                value = 1;
                tbl_id = Hss12GUnitReset0_t + hss_id;
                cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));
                fld_id = Hss12GUnitReset0_resetCoreKrLinkCtl0_f +
                         lane_id*(Hss12GUnitReset0_resetCoreKrLinkCtl1_f-Hss12GUnitReset0_resetCoreKrLinkCtl0_f);
                DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));

                value = 0;
                DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
                cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));
            }
        }
        else  /*enable flow*/
        {
            value = 0;
            tbl_id = Hss12GMacroCfg0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_macro));
            fld_id = Hss12GMacroCfg0_cfgHssL0AecRxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecRxByp_f-Hss12GMacroCfg0_cfgHssL0AecRxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            fld_id = Hss12GMacroCfg0_cfgHssL0AecTxByp_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1AecTxByp_f-Hss12GMacroCfg0_cfgHssL0AecTxByp_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);
            
            /*Hss12GMacroCfg[0..2]_cfgLane[0~7]PmaReady = 1'b1;     */
            value = 1;
            fld_id = Hss12GMacroCfg0_cfgHssL0PmaReady_f +
                     lane_id*(Hss12GMacroCfg0_cfgHssL1PmaReady_f-Hss12GMacroCfg0_cfgHssL0PmaReady_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_macro);

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_macro));

            value = 0;
            /* HsMacroCfg */
            tbl_id = Hss12GUnitReset0_t + hss_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));
            fld_id = Hss12GUnitReset0_resetCoreKrLinkCtl0_f +
                     lane_id*(Hss12GUnitReset0_resetCoreKrLinkCtl1_f-Hss12GUnitReset0_resetCoreKrLinkCtl0_f);
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &hs_rst);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hs_rst));

            /* KrLinkCtlCfg */
            tbl_id = KrLinkCtlCfg0_t + serdes_id;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &kr_cfg));
            fld_id = KrLinkCtlCfg0_cfgCinEnable_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
            value = 1;
            fld_id = KrLinkCtlCfg0_cfgFsmTrEn_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
            fld_id = KrLinkCtlCfg0_cfgFsmTrRst_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
            fld_id = KrLinkCtlCfg0_cfgManualRdy_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
            fld_id = KrLinkCtlCfg0_cfgMaxWaitTimerDisable_f;
            DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &kr_cfg);
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &kr_cfg));

            /*start MCU link training after reset KrLink*/
            CTC_ERROR_RETURN(sys_tsingma_mcu_set_cl72_enable(lchip, hss_id, lane_id, TRUE));
        }
    }
    else
    {
        CTC_ERROR_RETURN(_sys_tsingma_serdes_set_hss28g_link_training_en(lchip, serdes_id, enable));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_serdes_get_link_training_status(uint8 lchip, uint16 serdes_id, uint16* p_value)
{
    uint32 serdes_info_addr = 0;
    uint32 serdes_info_data = 0;
    uint16 status_tmp  = 0;
    uint16 status  = 0;
    uint16 fw_index = 1;  /* BE_debug(lane_num, 2, 1) */
    uint8  hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8  lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        CTC_ERROR_RETURN(_sys_tsingma_serdes_get_hss28g_link_training_status(lchip, serdes_id, fw_index, &status_tmp));
        switch (status_tmp)
        {
        case 0:
            status = SYS_PORT_CL72_DISABLE;
            break;
        case 1:
            status = SYS_PORT_CL72_PROGRESS;
            break;
        case 2:
            status = SYS_PORT_CL72_OK;
            break;
        case 3:
            status = SYS_PORT_CL72_FAIL;
            break;
        }
    }
    else
    {
        switch(hss_id)
        {
            case 0:
                serdes_info_addr = SYS_TSINGMA_MCU_0_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE;
                break;
            case 1:
                serdes_info_addr = SYS_TSINGMA_MCU_1_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE;
                break;
            case 2:
                serdes_info_addr = SYS_TSINGMA_MCU_2_SERDES_INFO_BASE_ADDR + lane_id*SYS_TSINGMA_MCU_SERDES_ALLOC_BYTE;
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
        /*read train_remote_en & train_local_en to get en/disable*/
        CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, (serdes_info_addr+4), &serdes_info_data));
        if(0 == (serdes_info_data & 0xffff))
        {
            status = SYS_PORT_CL72_DISABLE;
        }
        /*read cur_loc_state to get LT status, 10-ok, 11-fail, others-progress*/
        else
        {
            CTC_ERROR_RETURN(sys_usw_mcu_chip_read(lchip, serdes_info_addr, &serdes_info_data));
            SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_info_data 0x%x\n", serdes_info_data);
            switch((serdes_info_data >> 24) & 0xff)
            {
                case 10:
                    status = SYS_PORT_CL72_OK;
                    break;
                case 11:
                    status = SYS_PORT_CL72_FAIL;
                    break;
                default:
                    status = SYS_PORT_CL72_PROGRESS;
                    break;
            }
        }
        SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id %u, LT status %u\n", serdes_id, status);
    }
    *p_value = status << (4 * (lane_id % 4));
    SYS_CL73_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"Fixed LT status %u\n", *p_value);

    return CTC_E_NONE;
}

/*
set serdes polarity (hard & soft tables, init & set)
*/
int32
sys_tsingma_datapath_set_serdes_polarity(uint8 lchip, void* p_data)
{
    uint8  hss_id    = 0;
    uint8  hss_idx   = 0;
    uint8  lane_id   = 0;
    uint8  serdes_id = 0;
    uint8  acc_id    = 0;
    uint16 addr      = 0;
    uint16 mask      = 0;
    uint16 data      = 0;
    sys_datapath_hss_attribute_t* p_hss      = NULL;
    sys_datapath_serdes_info_t*   p_serdes   = NULL;
    ctc_chip_serdes_polarity_t*   p_polarity = (ctc_chip_serdes_polarity_t*)p_data;

    DATAPATH_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_polarity);
    serdes_id = p_polarity->serdes_id;
    if ((serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE) || (p_polarity->dir > 1) ||(p_polarity->polarity_mode > 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);

    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (NULL == p_hss)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NONE;
    }

    hss_id  = p_hss->hss_id;
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        /*HSS28G*/
        if (p_polarity->dir)
        {
            /*TX: TX_POL_FLIP(reg 0x8zA0[7]): 1'b0, Inverted(default)  1'b1, Normal*/
            addr = 0x80A0 + lane_id * 0x100;
            mask = 0xff7f;
            data = (0 == p_polarity->polarity_mode) ? 1 : 0;
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));
        }
        else
        {
            /*RX: RX_POL_FLIP(reg 0x8z61[14]): 1'b0, Normal(default)  1'b1, Inverted*/
            addr = 0x8061 + lane_id * 0x100;
            mask = 0xbfff;
            data = p_polarity->polarity_mode;
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, hss_id, addr, mask, data));
        }
    }
    else
    {
        /*HSS12G*/
        acc_id = lane_id + 3;
        data   = p_polarity->polarity_mode;
        addr   = 0x83;

        if (p_polarity->dir)
        {
            /*TX: r_tx_pol_inv(reg0x83[1]): 1'b0, Normal(default)  1'b1, Inverted*/
            mask = 0xfd;
        }
        else
        {
            /*RX: r_tx_pol_inv(reg0x83[3]): 1'b0, Normal(default)  1'b1, Inverted*/
            mask = 0xf7;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, (uint8)addr, (uint8)mask, (uint8)data));
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

STATIC int32
_sys_tsingma_datapath_set_hss12gmacro_powerdown(uint8 lchip, uint8 hss_id, uint8 is_down)
{
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint32 cmd       = 0;
    uint32 tmp_val32 = 0;
    Hss12GGlbCfg0_m glb_cfg;

    tbl_id = Hss12GGlbCfg0_t + hss_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
    tmp_val32 = (is_down ? 0x01ef4e0c : 0x0010b1f3);
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &glb_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_hss28gmacro_powerdown(uint8 lchip, uint8 hss_id, uint8 is_down)
{
    uint8  i         = 0;
    uint32 tbl_id    = 0;
    uint32 fld_id    = 0;
    uint32 cmd       = 0;
    uint32 tmp_val32 = 0;
    Hss28GUnitEnClk_m   hss28G_clk;
    Hss28GCfg_m         hss28g_cfg;
    Hss28GUnitReset_m   hss28g_reset;

    /*Hss28GUnitEnClk.enClkXfiPcs0/1/2/3/4/5/6/7*/
    /*Hss28GUnitEnClk.enClkSgmiiPcs0/1/2/3/4/5/6/7*/
    tbl_id = Hss28GUnitEnClk_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28G_clk));
    tmp_val32 = (is_down ? 0 : 1);
    for (i = 0; i < HSS28G_LANE_NUM; i++)
    {
        fld_id = Hss28GUnitEnClk_enClkSgmiiPcs0_f + i + hss_id*4;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28G_clk);    
        fld_id = Hss28GUnitEnClk_enClkXfiPcs0_f + i + hss_id*4;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28G_clk);    
    }    
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28G_clk));

    /*Hss28GCfg. cfgHss0ApbResetBar*/
    tbl_id = Hss28GCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_cfg));
    if (0 == hss_id)
    {
        fld_id = Hss28GCfg_cfgHss0ApbResetBar_f;
    }
    else
    {
        fld_id = Hss28GCfg_cfgHss1ApbResetBar_f;
    }
    tmp_val32 = (is_down ? 0 : 1);
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_cfg));

    /*Hss28GUnitReset*/
    tbl_id = Hss28GUnitReset_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_reset));
    tmp_val32 = (is_down ? 1 : 0);
    for (i = 0; i < HSS28G_LANE_NUM; i++)
    {
        fld_id = Hss28GUnitReset_resetCoreMii0_f + i + hss_id*4;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
        fld_id = Hss28GUnitReset_resetCorePcs0_f + i + hss_id*4;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
        fld_id = Hss28GUnitReset_resetCoreAneth0_f + i + hss_id*4;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    }
    fld_id = Hss28GUnitReset_resetSharedFec0_f + hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreCgPcs0_f + hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreXlgPcs0_f + hss_id*2;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreXlgPcs2_f + hss_id*2;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreSharedFecReg0_f + hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreSharedPcsReg0_f + hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    fld_id = Hss28GUnitReset_resetCoreSharedMiiReg0_f + hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_reset));
    
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_serdes_powerdown(uint8 lchip)
{
    uint8 hss_id  = 0;
    uint8 hss_idx = 0;
    uint8 hss28gmacro_pwrdn_flag[TSINGMA_HSS28G_NUM_PER_SLICE];
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint32 cmd    = 0;
    uint32 tmp_val32 = 0;
    Hss28GUnitReset_m   hss28g_reset;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    sal_memset(hss28gmacro_pwrdn_flag, 0, TSINGMA_HSS28G_NUM_PER_SLICE);

    for (hss_idx = 0; hss_idx < TSINGMA_SYS_DATAPATH_HSS_NUM; hss_idx++)
    {
        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);

        if (hss_idx < TSINGMA_HSS15G_NUM_PER_SLICE)  /* HSS12G */
        {
            hss_id = hss_idx;
            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_hss12gmacro_powerdown(lchip, hss_id, ((NULL==p_hss)?TRUE:FALSE)));
        }
        else     /* HSS28G */
        {
            hss_id = hss_idx - TSINGMA_HSS15G_NUM_PER_SLICE;
            if (NULL==p_hss)
            {
                hss28gmacro_pwrdn_flag[hss_id] = 1;
            }
            CTC_ERROR_RETURN(_sys_tsingma_datapath_set_hss28gmacro_powerdown(lchip, hss_id, ((NULL==p_hss)?TRUE:FALSE)));
        }
    }
    /* if HSS28G Macro both powerdown, set Hss28GUnitReset.resetHss28GRegAcc = 1*/
    tbl_id = Hss28GUnitReset_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_reset));
    fld_id = Hss28GUnitReset_resetHss28GRegAcc_f;
    if ((1 == hss28gmacro_pwrdn_flag[0]) && (1 == hss28gmacro_pwrdn_flag[1]))
    {
        tmp_val32 = 1;
    }
    else
    {
        tmp_val32 = 0;
    }
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28g_reset);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28g_reset));

    return CTC_E_NONE;
}

#define __DYNAMIC_SWITCH__
/*only use for epe mem*/
int32
_sys_tsingma_datapath_get_prev_used_chanid(uint8 lchip, uint8 slice_id, uint16 lport, uint8* prev_chanid)
{
    uint8 hss_flag = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    int8 tmp_lane_id = 0;
    uint16 tmp_lport = 0xff;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PARAM;
    }

    hss_flag = SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(port_attr->serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(port_attr->serdes_id);

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NONE;
    }

    if ((p_hss_vec->serdes_info[lane_id].port_num > 1) && (lport > p_hss_vec->serdes_info[lane_id].lport))
    {
        tmp_lport = lport - 1;
    }
    else
    {
        for (tmp_lane_id = lane_id; tmp_lane_id >= ((lane_id/4)*4); tmp_lane_id--)
        {
            if (p_hss_vec->serdes_info[tmp_lane_id].mode == CTC_CHIP_SERDES_NONE_MODE)
            {
                continue;
            }

            tmp_lport = p_hss_vec->serdes_info[tmp_lane_id].lport + p_hss_vec->serdes_info[tmp_lane_id].port_num - 1;
        }
    }

    if (tmp_lport == 0xff)
    {
        *prev_chanid = 0xff;
        return CTC_E_NONE;
    }

    port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
    if (!port_attr)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (hss_flag != SYS_DATAPATH_MAC_IN_HSS28G(lchip, port_attr->mac_id))
    {
        return CTC_E_INVALID_PARAM;
    }

    *prev_chanid = port_attr->chan_id;

    return CTC_E_NONE;
}

 /*for scene !Q/U->Q/U, 3 serdes are needed, and the last serdes should not be switched*/
STATIC int32
_sys_tsingma_datapath_dynamic_switch_qsgmii_serdes_control(uint8 lchip, uint8 src_mode, uint8 dst_mode, uint8 serdes_id, uint8* p_add_flag)
{
    *p_add_flag = 1;
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    if(!SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(src_mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(dst_mode) && (3 == serdes_id || 11 == serdes_id))
    {
        if((3 == serdes_id) && (0 == p_usw_datapath_master[lchip]->qm_choice.txqm1))
        {
            if((CTC_CHIP_SERDES_NONE_MODE != src_mode) && (!SYS_DATAPATH_IS_SERDES_4_PORT_1(src_mode)))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            *p_add_flag = 2;
        }
        if((11 == serdes_id) && (1 == p_usw_datapath_master[lchip]->qm_choice.txqm3))
        {
            if((CTC_CHIP_SERDES_NONE_MODE != src_mode) && (!SYS_DATAPATH_IS_SERDES_4_PORT_1(src_mode)))
            {
                return CTC_E_PARAM_CONFLICT;
            }
            *p_add_flag = 2;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_dynamic_switch_judge_serdes_add(int32 serdes_id, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 serdes_idx = 0;
    for(serdes_idx = 0; serdes_idx < target->serdes_num; serdes_idx++)
    {
        if(serdes_id == target->s[serdes_idx].serdes_id)
        {
            return 0;
        }
    }
    return 1;
}

STATIC int32
_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(uint8 lchip, uint8 serdes_id, uint8 dst_mode, sys_datapath_dynamic_switch_attr_t* target, uint8* target_serdes_cnt)
{
    sys_datapath_serdes_info_t* p_serdes = NULL;

    SYS_CONDITION_RETURN(!_sys_tsingma_datapath_dynamic_switch_judge_serdes_add(serdes_id, target), CTC_E_NONE);
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, serdes_id, &p_serdes));
    SYS_CONDITION_RETURN(!(p_serdes->is_dyn), CTC_E_PARAM_CONFLICT);
    
    target->s[target->serdes_num].serdes_id = serdes_id;
    target->s[target->serdes_num].src_mode = p_serdes->mode;
    target->s[target->serdes_num].dst_mode = dst_mode;
    (target->serdes_num)++;
    (*target_serdes_cnt)++;

    return CTC_E_NONE;
}

STATIC void
_sys_tsingma_datapath_dynamic_switch_get_port_id_base(uint8 lchip, uint8 serdes_id, uint8 mode, uint16* p_port_id_base)
{
    uint8 mode_level = 0;

    if(NULL == p_usw_datapath_master[lchip])
    {
        *p_port_id_base = SYS_DATAPATH_USELESS_MAC;
        return;
    }

    switch(mode) 
    {
        case CTC_CHIP_SERDES_QSGMII_MODE:
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            mode_level = 0;
            break;
        case CTC_CHIP_SERDES_SGMII_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
        case CTC_CHIP_SERDES_XFI_MODE:
        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_100BASEFX_MODE:
        case CTC_CHIP_SERDES_USXGMII0_MODE:
            mode_level = 1;
            break;
        case CTC_CHIP_SERDES_LG_MODE:
            mode_level = 2;
            break;
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_DXAUI_MODE:
        case CTC_CHIP_SERDES_XLG_MODE:
        case CTC_CHIP_SERDES_CG_MODE:
        default:
            mode_level = 3;
            break;
    }

    *p_port_id_base = p_usw_datapath_master[lchip]->serdes_mac_map[mode_level][serdes_id];
}

STATIC int32
_sys_tsingma_datapath_dynamic_switch_judge_port_add(int32 port_id, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 port_idx = 0;
    for(port_idx = 0; port_idx < target->mac_num; port_idx++)
    {
        if(port_id == target->m[port_idx].mac_id)
        {
            return FALSE;
        }
    }
    return TRUE;
}

STATIC uint32
_sys_tsingma_datapath_dynamic_switch_calc_add_drop_flag(uint8 lchip, int32 mac_id, sys_datapath_dynamic_switch_serdes_t* s, uint8 pre_switch_flag)
{
    uint8 pre_port_num = 0;
    uint8 post_port_num = 0;
    uint8 inner_port_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    
    SYS_DATAPATH_GET_PORT_NUM_PER_QM(s->src_mode, pre_port_num);
    SYS_DATAPATH_GET_PORT_NUM_PER_QM(s->dst_mode, post_port_num);
    
    inner_port_id = mac_id%4;

    if(pre_switch_flag)
    {
        if(SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(s->src_mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(s->dst_mode))
        {
            if((4 == pre_port_num) && (2 == post_port_num) && (2 == inner_port_id || 3 == inner_port_id))
            {
                return 2;
            }
            if((4 == pre_port_num) && (1 == post_port_num) && (1 == inner_port_id || 2 == inner_port_id || 3 == inner_port_id))
            {
                return 2;
            }
            if((2 == pre_port_num) && (1 == post_port_num) && (1 == inner_port_id))
            {
                return 2;
            }
            return 0;
        }
        if((!SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(s->src_mode)) && (!SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(s->dst_mode)))
        {
            if((4 == pre_port_num) && (2 == post_port_num) && (1 == inner_port_id || 3 == inner_port_id))
            {
                return 2;
            }
            if((4 == pre_port_num) && (1 == post_port_num) && (1 == inner_port_id || 2 == inner_port_id || 3 == inner_port_id))
            {
                return 2;
            }
            if((2 == pre_port_num) && (1 == post_port_num) && (2 == inner_port_id))
            {
                return 2;
            }
            return 0;
        }
        else
        {
            if(((0 == s->serdes_id) || (4 == s->serdes_id) || (8 == s->serdes_id)) && (0 == inner_port_id))
            {
                return 0;
            }
            else return 2;
        }
    }
    else
    {
        if(SYS_DATAPATH_JUDGE_NULL_PORT(lchip, mac_id, &port_attr))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

STATIC void
_sys_tsingma_datapath_dynamic_switch_add_port_to_target(uint8 lchip, sys_datapath_dynamic_switch_serdes_t* s, uint16 tmp_port_id_base, sys_datapath_dynamic_switch_attr_t* target, uint8* target_port_cnt, uint8 pre_switch_flag)
{
    uint8  port_num       = 0;
    uint8  inner_port_idx = 0;
    uint16 tmp_port_id    = 0;
    uint32 add_drop_flag  = 0;
    uint8  tmp_mode       = 0;

    if(63 < tmp_port_id_base)
    {
        return;
    }

    tmp_mode = (pre_switch_flag ? s->src_mode : s->dst_mode);
    switch(tmp_mode)
    {
        case CTC_CHIP_SERDES_USXGMII1_MODE:
        case CTC_CHIP_SERDES_QSGMII_MODE:
            port_num = 4;
            break;
        case CTC_CHIP_SERDES_USXGMII2_MODE:
            port_num = 2;
            break;
        case CTC_CHIP_SERDES_NONE_MODE:
            port_num = 0;
            break;
        default:
            port_num = 1;
            break;
    }

    /*add ports to target row*/
    for(inner_port_idx = 0; inner_port_idx < port_num; inner_port_idx++)
    {
        tmp_port_id = tmp_port_id_base + inner_port_idx;
        if(!_sys_tsingma_datapath_dynamic_switch_judge_port_add(tmp_port_id, target))
        {
            continue;
        }
        add_drop_flag = _sys_tsingma_datapath_dynamic_switch_calc_add_drop_flag(lchip, tmp_port_id, s, pre_switch_flag);
        target->m[target->mac_num].mac_id = tmp_port_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        (target->mac_num)++;
        (*target_port_cnt)++;
    }
}

/*
Influenced serdes & ports parser. Run steps:
0a. find influenced ports -> 0b. find serdes influenced by step a ports -> 
1a. find ports influenced by step b serdes -> 1b. find serdes influenced by step a ports -> ...
*/
STATIC int32
_sys_tsingma_datapath_dynamic_switch_serdes_port_parser(uint8 lchip, sys_datapath_lport_attr_t* src_port, uint8 dst_mode, sys_datapath_dynamic_switch_attr_t *target, uint8 serdes_id_raw)
{
    uint8 serdes_idx = 0;
    uint8 serdes_id = 0;
    uint8 serdes_id_base = 0;
    uint8 required_serdes_cnt = 0;
    uint8 inner_port_idx = 0;
    uint8 port_idx = 0;
    uint8 target_serdes_cnt = 0;
    uint8 target_port_cnt = 0;
    uint16 tmp_port_id_base = 0;
    uint16 tmp_port_id = 0;
    uint8 tmp_serdes_id = 0;
    uint8 lg_serdes_id = 0;
    uint8 tmp_mode = 0;
    /*uint8 tmp_dst_mode = 0;*/
    uint8 qu_add_flag = 1; //0-no add  1-add as dest mode  2-add as none mode
    sys_datapath_dynamic_switch_serdes_t* s = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /* #0 Recursion ending condition: 
          this port id does not related to any serdes, during pre-switch. */
    SYS_CONDITION_RETURN(((NULL == src_port) || ((CTC_CHIP_SERDES_NONE_MODE == src_port->pcs_mode) && (0xff == serdes_id_raw))), CTC_E_NONE);

    /* #1 Serdes parser, 2 things are done in loop: 
          1 add influenced serdes to target row (no duplication); 
          2 modify global serdes cfg for next step preparation. */
                    
    /*influenced serdes based on pre-switch config*/
    if((SYS_DMPS_RSV_PORT == src_port->port_type) && (0xff != serdes_id_raw))
    {
        serdes_id_base = serdes_id_raw;
    }
    else
    {
        if((CTC_CHIP_SERDES_LG_MODE == src_port->pcs_mode) && (1 == src_port->flag))
        {
            serdes_id_base = (src_port->serdes_id/4)*4;
            SYS_TSINGMA_GET_LG_ANOTHER_SERDES((src_port->serdes_id), lg_serdes_id);
            CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, lg_serdes_id, dst_mode, target, &target_serdes_cnt));
            CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, src_port->serdes_id, dst_mode, target, &target_serdes_cnt));
        }
        else
        {
            serdes_id_base = src_port->serdes_id + 1 - src_port->serdes_num;
            for(; serdes_idx < src_port->serdes_num; serdes_idx++)
            {
                serdes_id = serdes_id_base + serdes_idx;
                CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_qsgmii_serdes_control(lchip, src_port->pcs_mode, dst_mode, serdes_id, &qu_add_flag));
                if(1 == qu_add_flag)
                {
                    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, dst_mode, target, &target_serdes_cnt));
                }
                else if(2 == qu_add_flag)
                {
                    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, CTC_CHIP_SERDES_NONE_MODE, target, &target_serdes_cnt));
                }
            }
        }
    }
    /*influenced serdes based on post-switch config*/
    if((CTC_CHIP_SERDES_LG_MODE == dst_mode) && (1 == src_port->flag))
    {
        SYS_TSINGMA_GET_LG_SWAP_SERDES(src_port->mac_id, serdes_id, lg_serdes_id);
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, dst_mode, target, &target_serdes_cnt));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, lg_serdes_id, dst_mode, target, &target_serdes_cnt));
    }
    //dst_mode is serdes-1-port-1 and need do swap, only XFI and XXVG support this
    else if(((CTC_CHIP_SERDES_XFI_MODE == dst_mode) || (CTC_CHIP_SERDES_XXVG_MODE == dst_mode)) && (1 == src_port->flag))
    {
        //SYS_TSINGMA_GET_LG_SWAP_SERDES(src_port->mac_id, serdes_id, lg_serdes_id);
        switch (src_port->mac_id)
        {
        case 44:
            serdes_id = 26;
            break;
        case 45:
            serdes_id = 25;
            break;
        case 46:
            serdes_id = 24;
            break;
        case 47:
            serdes_id = 27;
            break;
        case 60:
            serdes_id = 30;
            break;
        case 61:
            serdes_id = 29;
            break;
        case 62:
            serdes_id = 28;
            break;
        case 63:
            serdes_id = 31;
            break;
        }
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, dst_mode, target, &target_serdes_cnt));
    }
    else
    {
        if(SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(src_port->pcs_mode) && ((CTC_CHIP_SERDES_XFI_MODE == dst_mode) || 
                                                                                (CTC_CHIP_SERDES_2DOT5G_MODE == dst_mode) || 
                                                                                (CTC_CHIP_SERDES_XXVG_MODE == dst_mode) || 
                                                                                (CTC_CHIP_SERDES_SGMII_MODE == dst_mode) || 
                                                                                (CTC_CHIP_SERDES_100BASEFX_MODE == dst_mode) ||
                                                                                (CTC_CHIP_SERDES_USXGMII0_MODE == dst_mode)))
        {
            required_serdes_cnt = 1;
        }
        else
        {
            SYS_DATAPATH_GET_SERDES_NUM_PER_PORT(dst_mode, required_serdes_cnt);
        }
        for(serdes_idx = 0; serdes_idx < required_serdes_cnt; serdes_idx++)
        {
             /*serdes_id_base adjustment*/
            if((CTC_CHIP_SERDES_LG_MODE == dst_mode) && 
               ((CTC_CHIP_SERDES_XXVG_MODE == src_port->pcs_mode) || (CTC_CHIP_SERDES_NONE_MODE == src_port->pcs_mode) || 
                (CTC_CHIP_SERDES_XFI_MODE == src_port->pcs_mode)))
            {
                serdes_id_base = serdes_id_base/2*2;
            }
            else
            {
                serdes_id_base = (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(dst_mode) || 
                                  (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(src_port->pcs_mode) &&
                                   (!SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode))) || 
                                  ((CTC_CHIP_SERDES_LG_MODE == src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode)) || 
                                  (SYS_DATAPATH_IS_SERDES_1_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode)) || 
                                  ((SYS_DMPS_NETWORK_PORT != src_port->port_type) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode))) ? 
                                  serdes_id_base : serdes_id_base/4*4;
            }
            serdes_id = serdes_id_base + serdes_idx;
            CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_qsgmii_serdes_control(lchip, src_port->pcs_mode, dst_mode, serdes_id, &qu_add_flag));
            if(1 == qu_add_flag)
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, dst_mode, target, &target_serdes_cnt));
            }
            else if(2 == qu_add_flag)
            {
                CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_serdes_to_target(lchip, serdes_id, CTC_CHIP_SERDES_NONE_MODE, target, &target_serdes_cnt));
            }
        }
    }
    
    /* #2 Find other influenced ports by target serdes id. 
          Each influenced port will: 1 be added to target row (no duplication); 2 enter next level to parse influenced serdes. */
    for(serdes_idx = 0; serdes_idx < target_serdes_cnt; serdes_idx++)
    {
        s = &(target->s[target->serdes_num - target_serdes_cnt + serdes_idx]);
        tmp_serdes_id = s->serdes_id;
        /*influenced ports based on pre-switch config*/
        tmp_mode = s->src_mode;

        if(CTC_CHIP_SERDES_NONE_MODE != tmp_mode)
        {
            _sys_tsingma_datapath_dynamic_switch_get_port_id_base(lchip, tmp_serdes_id, tmp_mode, &tmp_port_id_base);
            _sys_tsingma_datapath_dynamic_switch_add_port_to_target(lchip, s, tmp_port_id_base, target, &target_port_cnt, 1);
        }
        
        /*influenced ports based on post-switch config*/
        tmp_mode = s->dst_mode;
        if(CTC_CHIP_SERDES_NONE_MODE != tmp_mode)
        {
            _sys_tsingma_datapath_dynamic_switch_get_port_id_base(lchip, tmp_serdes_id, tmp_mode, &tmp_port_id_base);
            _sys_tsingma_datapath_dynamic_switch_add_port_to_target(lchip, s, tmp_port_id_base, target, &target_port_cnt, 0);
        }
    }
        
    /*enter next level to parse influenced serdes*/
    for(inner_port_idx = 0; inner_port_idx < target_port_cnt; inner_port_idx++)
    {
        port_idx = target->mac_num - target_port_cnt + inner_port_idx;
        tmp_port_id = target->m[port_idx].mac_id;
        /*if this port need moving (i.e. 3 == add_drop_flag), that means no change happened to this port mode*/
        if(3 == target->m[port_idx].add_drop_flag)
        {
            if(SYS_DATAPATH_JUDGE_NULL_PORT(lchip, tmp_port_id, &port_attr))
            {
                tmp_mode = CTC_CHIP_SERDES_NONE_MODE;
            }
            else
            {
                tmp_mode = port_attr->pcs_mode;
            }
        }
        if(SYS_DMPS_RSV_PORT == src_port->port_type)
        {
            continue;
        }
        _sys_tsingma_datapath_dynamic_switch_serdes_port_parser(lchip, port_attr, tmp_mode, target, 0xff);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_dynamic_hss28g_clock_change_parser(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, sys_datapath_dynamic_switch_attr_t *target)
{
    int32 ret = 0;
    uint8 port_idx = 0;
    uint8 port_base = ((48 > src_port->mac_id) ? 44 : 60);
    uint8 serdes_id_base = ((48 > src_port->mac_id) ? 24 : 28);
    uint8 tmp_mac_idx = 0;
    uint8 dst_port_num_per_qm = 0;
    uint8 src_port_num_per_qm = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    
    SYS_DATAPATH_GET_PORT_NUM_PER_QM(dst_mode, dst_port_num_per_qm);
    SYS_DATAPATH_GET_PORT_NUM_PER_QM(src_port->pcs_mode, src_port_num_per_qm);
    for(port_idx = 0; port_idx < 4; port_idx++)
    {
        tmp_mac_idx = port_idx + port_base;

        ret = sys_usw_mac_get_port_capability(lchip, tmp_mac_idx, &port_attr);
        if(CTC_E_NONE != ret)
        {
            continue;
        }
        if((CTC_CHIP_SERDES_NONE_MODE == port_attr->pcs_mode) && (tmp_mac_idx != src_port->mac_id))
        {
            continue;
        }
        if(((2 == dst_port_num_per_qm) || (2 == src_port_num_per_qm)) && (0 != tmp_mac_idx % 2))
        {
            continue;
        }
        if(((1 == dst_port_num_per_qm) && (1 == src_port_num_per_qm)) && (0 != tmp_mac_idx % 4))
        {
            continue;
        }
        target->m[target->mac_num].mac_id = tmp_mac_idx;
        if(SYS_DATAPATH_JUDGE_NULL_PORT(lchip, tmp_mac_idx, &port_attr))
        {
            if(SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode) && (0 != tmp_mac_idx % 4))
            {
                continue;
            }
            else if((CTC_CHIP_SERDES_LG_MODE == dst_mode) && (0 != tmp_mac_idx % 2))
            {
                continue;
            }
            target->m[target->mac_num].add_drop_flag = 1;
        }
        else if(((46 == tmp_mac_idx) || (62 == tmp_mac_idx)) && (1 == dst_port_num_per_qm))
        {
            target->m[target->mac_num].add_drop_flag = 2;
        }
        else if(((45 == tmp_mac_idx) || (47 == tmp_mac_idx) || (61 == tmp_mac_idx) || (63 == tmp_mac_idx)) && (4 != dst_port_num_per_qm))
        {
            target->m[target->mac_num].add_drop_flag = 2;
        }
        else
        {
            target->m[target->mac_num].add_drop_flag = 0;
        }
        (target->mac_num)++;
    }

    /*sgmii check 4 lane no none mode*/
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id_base);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    for(lane_id = 0; lane_id < 4; lane_id++)
    {
        if(((p_hss_vec == NULL) || 
            (CTC_CHIP_SERDES_NONE_MODE == p_hss_vec->serdes_info[lane_id].mode) || 
            (CTC_CHIP_SERDES_XSGMII_MODE == p_hss_vec->serdes_info[lane_id].mode)) && 
           ((CTC_CHIP_SERDES_SGMII_MODE == dst_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == dst_mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " None mode cannot switch to SGMII/2.5G! \n");
            return CTC_E_PARAM_CONFLICT;
        }
    }

    for(port_idx = 0; port_idx < 4; port_idx++)
    {
        tmp_mac_idx = port_idx + port_base;
        ret = sys_usw_mac_get_port_capability(lchip, tmp_mac_idx, &port_attr);
        if(CTC_E_NONE != ret)
        {
            continue;
        }
        if((CTC_CHIP_SERDES_NONE_MODE == port_attr->pcs_mode) && (tmp_mac_idx != src_port->mac_id))
        {
            continue;
        }
        ret = _sys_tsingma_datapath_dynamic_switch_serdes_port_parser(lchip, port_attr, dst_mode, target, (serdes_id_base+port_idx));
        if(CTC_E_PARAM_CONFLICT == ret)
        {
            return CTC_E_PARAM_CONFLICT;
        }
    }

    return CTC_E_NONE;
}

/* Check serdes available. 
   For example, if there are serdes not available for dynamic switch in a quad serdes (0 == p_serdes->is_dyn), it is not allowed to switch to XLG.
*/
STATIC int32
_sys_tsingma_datapath_dynamic_switch_check_serdes_available(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode)
{
    uint8 cnt = 0;
    uint8 src_mode_level = 0;
    uint8 dst_mode_level = 0;
    int32 ret = 0;
    uint8 max_cnt = 4;
    uint8 serdes_base = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(src_port->pcs_mode, &src_mode_level, 0, 0));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(dst_mode, &dst_mode_level, 0, 0));
    
    if(dst_mode_level > src_mode_level)
    {
        if(2 == dst_mode_level && 1 == src_mode_level)
        {
            max_cnt = 2;
            serdes_base = src_port->serdes_id;
        }
        else if(1 == dst_mode_level && 0 == src_mode_level)
        {
            max_cnt = 1;
            serdes_base = src_port->serdes_id;
        }
        else
        {
            max_cnt = 4;
            serdes_base = (src_port->serdes_id/4)*4;
        }
        for(; cnt < max_cnt; cnt++)
        {
            ret = sys_usw_datapath_get_serdes_info(lchip, (serdes_base + cnt), &p_serdes);
            if(CTC_E_NONE != ret || 0 == p_serdes->is_dyn)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                return CTC_E_NOT_SUPPORT;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_dynamic_switch_add_influenced_port(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, uint8* influenced_port_num, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 port_num = 0;
    uint8 port_idx;
    uint8 tmp_mac_id = 0;
    int32 ret = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if(CTC_CHIP_SERDES_NONE_MODE != src_port->pcs_mode)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_check_serdes_available(lchip, src_port, dst_mode));
    }

    /* 1 port 1 lane mode <-> Q/U, 4 ports in one QM are all infuenced */
    /*if((SYS_DATAPATH_IS_SERDES_1_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(dst_mode)) || 
       (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode)))
    {
        port_num = 4;
    }
    else
    {*/
        switch(dst_mode)
        {
            case CTC_CHIP_SERDES_USXGMII1_MODE:
            case CTC_CHIP_SERDES_QSGMII_MODE:
                port_num = 4;
                break;
            case CTC_CHIP_SERDES_USXGMII2_MODE:
                port_num = 2;
                break;
            default:
                port_num = 1;
                break;
        }
    /*}*/

    for(port_idx = 0; port_idx < port_num; port_idx++)
    {
        tmp_mac_id = src_port->mac_id + port_idx;
        ret = sys_usw_mac_get_port_capability(lchip, tmp_mac_id, &port_attr);

        if((CTC_E_NONE != ret) || (NULL == port_attr) || (SYS_DMPS_RSV_PORT == port_attr->port_type) || (CTC_CHIP_SERDES_NONE_MODE == port_attr->pcs_mode))
        {
            target->m[target->mac_num].add_drop_flag = 1;
        }
        /*1 port 1 lane mode -> USXGMII single/2 port, in these conditions, port should be dropped: */
        else if(SYS_DATAPATH_IS_SERDES_1_PORT_1(src_port->pcs_mode))
        {
            if(CTC_CHIP_SERDES_USXGMII2_MODE == dst_mode && 1 < port_idx)  /*1. if dst mode is USXGMII 2 port mode, and port_idx is 2/3, should be dropped*/
            {
                target->m[target->mac_num].add_drop_flag = 2;
            }
        }
        else if(SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(src_port->pcs_mode) && 
                (SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode) ||  SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode)) && 
                ((port_attr->tbl_id > 3 && port_attr->tbl_id < 8) || (port_attr->tbl_id > 12 && port_attr->tbl_id < 23) || (port_attr->tbl_id > 27)))
        {
            target->m[target->mac_num].add_drop_flag = 2;
        }
        else
        {
            target->m[target->mac_num].add_drop_flag = 0;
        }
        target->m[target->mac_num].mac_id = tmp_mac_id;
        (target->mac_num)++;
    }

    *influenced_port_num = port_num;

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_dynamic_usxgmii0_check(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, uint8 serdes_id_raw)
{
    uint8 serdes_id = (0xff == serdes_id_raw) ? src_port->serdes_id : serdes_id_raw;
    uint8 serdes_id_l0 = serdes_id/4*4;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    /*USXGMII-single mode, for every quad serdes, if lane 0 is not USXGMII-single, other lanes cannot be USXGMII-single*/
    if(((CTC_CHIP_SERDES_USXGMII0_MODE == dst_mode) && (CTC_CHIP_SERDES_USXGMII0_MODE != src_port->pcs_mode)) && 
       (serdes_id != serdes_id_l0))
    {
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, serdes_id_l0, &p_serdes));
        if(NULL == p_serdes)
        {
            return CTC_E_INVALID_PTR;
        }
        if((CTC_CHIP_SERDES_USXGMII0_MODE != p_serdes->mode))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes %u mode is not usxgmii-single, so serdes %u cannot support.\n", 
                                 serdes_id_l0, serdes_id);
            return CTC_E_NOT_SUPPORT;
        }
    }
    
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_datapath_dynamic_switch_para_check(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, uint8 serdes_id_raw)
{
    uint8 mac_tbl_id = 0;
    uint8 src_mode = 0;
    uint8 src_mode_level = 0;
    uint8 dst_mode_level = 0;
    uint8 tmp_flag = 0;
    uint8 idx;
    uint8 hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(src_port->serdes_id);
    uint8 lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(src_port->serdes_id);
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    src_mode = src_port->pcs_mode;
    mac_tbl_id = src_port->tbl_id;

    if(CTC_CHIP_SERDES_NONE_MODE != dst_mode)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(src_mode, &src_mode_level, 0, 0));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(dst_mode, &dst_mode_level, 0, 0));
        tmp_flag = src_mode_level*dst_mode_level;

        /*if (0 != mac_tbl_id % 4), 3 conditions can be accepted:  */
        /* #1 (1 port 1 lane) <-> (1 port 1 lane)  */
        /* #2 (1 port 1 lane) <-> (1 port 2 lane) and (0 == mac_tbl_id % 2)*/
        if(0 != mac_tbl_id % 4)
        {   /* two illegal conditions */
            if((1 != tmp_flag) && (2 != tmp_flag))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mac table id %u does not support mode %u. (1)\n", mac_tbl_id, dst_mode);
                return CTC_E_INVALID_CONFIG;
            }
            if((2 == tmp_flag) && (0 != mac_tbl_id % 2))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mac table id %u does not support mode %u. (2)\n", mac_tbl_id, dst_mode);
                return CTC_E_INVALID_CONFIG;
            }
        }
    }
    
     /*Only HSS28G port can be switched to XXVG/LG/CG*/
    if(((0xff == serdes_id_raw) && (!SYS_DATAPATH_SERDES_IS_HSS28G(src_port->serdes_id))) && 
       (CTC_CHIP_SERDES_XXVG_MODE == dst_mode || CTC_CHIP_SERDES_LG_MODE == dst_mode || CTC_CHIP_SERDES_CG_MODE == dst_mode))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u does not support mode %u. (1)\n", src_port->serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }
    
     /*Only port 0/8/20/24 (20/24 mac tbl id is 24) can be switched between Q/U and other mode level*/
    if(((0 == src_mode_level && 0 != dst_mode_level) || (0 != src_mode_level && 0 == dst_mode_level)) &&
       ((mac_tbl_id > 3 && mac_tbl_id < 8) || (mac_tbl_id > 11 && mac_tbl_id < 24) || (mac_tbl_id > 27)) && 
       (0xff == serdes_id_raw))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mac table id %u does not support mode %u. (3)\n", mac_tbl_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }

     /*for USXGMII-single check*/
    if((((CTC_CHIP_SERDES_USXGMII0_MODE == dst_mode) && (CTC_CHIP_SERDES_USXGMII0_MODE != src_mode)) || 
        ((CTC_CHIP_SERDES_USXGMII0_MODE != dst_mode) && (CTC_CHIP_SERDES_USXGMII0_MODE == src_mode))) &&
        (!SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(src_port->serdes_id)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u does not support mode %u. (2)\n", src_port->serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }
    /*USXGMII-single mode, for every quad serdes, if lane 0 is not USXGMII-single, other lanes cannot be USXGMII-single*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_usxgmii0_check(lchip, src_port, dst_mode, serdes_id_raw));
    /*lane 0 USX-single -> other mode, check lane 1~3 USX-single*/
    if((CTC_CHIP_SERDES_USXGMII0_MODE == src_mode) && (CTC_CHIP_SERDES_USXGMII0_MODE != dst_mode) && 
       (0 == lane_id % 4))
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if(p_hss_vec == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " HSS idx %u null pointer!\n", hss_idx);
            return CTC_E_INVALID_PTR;
        }
        for(idx = 1; idx < 4; idx++)
        {
            if(CTC_CHIP_SERDES_USXGMII0_MODE == p_hss_vec->serdes_info[lane_id+idx].mode)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Lane %u is USXGMII0, so lane 0 must be USXGMII0.\n", lane_id+idx);
                return CTC_E_NOT_SUPPORT;
            }
        }
    }

    /*HSS28G cannot support 100BaseFx*/
    if((SYS_DATAPATH_SERDES_IS_HSS28G(src_port->serdes_id)) && (CTC_CHIP_SERDES_100BASEFX_MODE == dst_mode))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u does not support mode %u. (3)\n", src_port->serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }

    /*LG lane swap check*/
    if((2 == src_port->flag) && (CTC_CHIP_SERDES_LG_MODE == dst_mode))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "50G lane swap config error!\n");
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

/*In Tm, every 4 serdes must be in one same clock shift.*/
STATIC int32
_sys_tsingma_datapath_dynamic_judge_hss28g_clock_change(uint8 lchip, uint8 serdes_id, ctc_chip_serdes_mode_t dst_mode)
{
    uint32 dst_pll_mode = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        return FALSE;
    }

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id));
    if(p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return FALSE;
    }
    SYS_TSINGMA_28G_GET_CMU_TYPE(dst_mode, FALSE, dst_pll_mode);
    if(p_hss_vec->plla_mode != dst_pll_mode)
    {
        return TRUE;
    }
    return FALSE;
}

int32
sys_tsingma_datapath_dynamic_switch_get_swap_info(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 cnt;
    uint8 mac_id_base = (src_port->mac_id/4)*4;
    uint8 serdes_id_base = (src_port->serdes_id/4)*4;

    /*xxvg->lg*/
    if((CTC_CHIP_SERDES_LG_MODE == dst_mode) && (CTC_CHIP_SERDES_XXVG_MODE == src_port->pcs_mode))
    {
        for(cnt = 0; cnt < 4; cnt++)
        {
            target->m[target->mac_num].add_drop_flag = (cnt % 2 == 0) ? 0 : 2;
            target->m[target->mac_num].mac_id = mac_id_base + cnt;
            target->mac_num++;
        }
    }
    /*lg->xxvg*/
    else
    {
        for(cnt = 0; cnt < 4; cnt++)
        {
            target->m[target->mac_num].add_drop_flag = (cnt % 2 == 0) ? 0 : 1;
            target->m[target->mac_num].mac_id = mac_id_base + cnt;
            target->mac_num++;
        }
    }
    
    for(cnt = 0; cnt < 4; cnt++)
    {
        target->s[target->serdes_num].serdes_id = serdes_id_base + cnt;
        target->s[target->serdes_num].src_mode = src_port->pcs_mode;
        target->s[target->serdes_num].dst_mode = dst_mode;
        target->serdes_num++;
    }
    return CTC_E_NONE;
}

static int32
_sys_tsingma_datapath_dynamic_switch_qm7_8_parser(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 idx = 0;
    uint8 add_drop_flag = 0;
    uint8 quad_1st_flag = (28 == src_port->mac_id) ? TRUE : FALSE;

    /*XLG/XAUI/DXAUI/XFI/SGMII/2DOT5/100BASEFX*/
    if(((CTC_CHIP_SERDES_XLG_MODE       != src_port->pcs_mode) && (CTC_CHIP_SERDES_XAUI_MODE   != src_port->pcs_mode) && 
        (CTC_CHIP_SERDES_DXAUI_MODE     != src_port->pcs_mode) && (CTC_CHIP_SERDES_XFI_MODE    != src_port->pcs_mode) && 
        (CTC_CHIP_SERDES_SGMII_MODE     != src_port->pcs_mode) && (CTC_CHIP_SERDES_2DOT5G_MODE != src_port->pcs_mode) && 
        (CTC_CHIP_SERDES_100BASEFX_MODE != src_port->pcs_mode) && (CTC_CHIP_SERDES_NONE_MODE   != src_port->pcs_mode) &&
        (CTC_CHIP_SERDES_XSGMII_MODE    != src_port->pcs_mode)) || 
       ((CTC_CHIP_SERDES_XLG_MODE       != dst_mode)           && (CTC_CHIP_SERDES_XAUI_MODE   != dst_mode)           && 
        (CTC_CHIP_SERDES_DXAUI_MODE     != dst_mode)           && (CTC_CHIP_SERDES_XFI_MODE    != dst_mode)           && 
        (CTC_CHIP_SERDES_SGMII_MODE     != dst_mode)           && (CTC_CHIP_SERDES_2DOT5G_MODE != dst_mode)           && 
        (CTC_CHIP_SERDES_100BASEFX_MODE != dst_mode)           && (CTC_CHIP_SERDES_NONE_MODE   != dst_mode)           &&
        (CTC_CHIP_SERDES_XSGMII_MODE    != dst_mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port 28 & 30 does not support such mode! \n");
        return CTC_E_NOT_SUPPORT;
    }

    /*get add_drop_flag*/
    if((CTC_CHIP_SERDES_NONE_MODE == dst_mode) || (CTC_CHIP_SERDES_XSGMII_MODE == dst_mode))
    {
        add_drop_flag = 2;
    }
    else if((CTC_CHIP_SERDES_NONE_MODE == src_port->pcs_mode) || (CTC_CHIP_SERDES_XSGMII_MODE == src_port->pcs_mode))
    {
        add_drop_flag = 1;
    }

    /*1-1 -> 1-1*/
    if(SYS_DATAPATH_IS_SERDES_1_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        target->s[target->serdes_num].serdes_id = quad_1st_flag ? 16 : 20;
        target->s[target->serdes_num].src_mode = src_port->pcs_mode;
        target->s[target->serdes_num].dst_mode = dst_mode;
        target->serdes_num++;
    }
    /*4-1 -> 4-1*/
    else if(SYS_DATAPATH_IS_SERDES_4_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        for(idx = 0; idx < 4; idx++)
        {
            target->s[target->serdes_num].serdes_id = (quad_1st_flag ? 16 : 20) + idx;
            target->s[target->serdes_num].src_mode = src_port->pcs_mode;
            target->s[target->serdes_num].dst_mode = dst_mode;
            target->serdes_num++;
        }
    }
    /*1-1 -> 4-1*/
    else if(SYS_DATAPATH_IS_SERDES_1_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        for(idx = 0; idx < 4; idx++)
        {
            target->s[target->serdes_num].serdes_id = (quad_1st_flag ? 16 : 20) + idx;
            target->s[target->serdes_num].src_mode = (0 == idx) ? src_port->pcs_mode : CTC_CHIP_SERDES_NONE_MODE;
            target->s[target->serdes_num].dst_mode = dst_mode;
            target->serdes_num++;
        }
    }
    /*4-1 -> 1-1*/
    else if(SYS_DATAPATH_IS_SERDES_4_PORT_1(src_port->pcs_mode) && SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        for(idx = 0; idx < 4; idx++)
        {
            target->s[target->serdes_num].serdes_id = (quad_1st_flag ? 16 : 20) + idx;
            target->s[target->serdes_num].src_mode = src_port->pcs_mode;
            target->s[target->serdes_num].dst_mode = (0 == idx) ? dst_mode : CTC_CHIP_SERDES_NONE_MODE;
            target->serdes_num++;
        }
    }
    /*none -> 4-1*/
    else if(((CTC_CHIP_SERDES_NONE_MODE == src_port->pcs_mode) || (CTC_CHIP_SERDES_XSGMII_MODE == src_port->pcs_mode)) && 
            SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        for(idx = 0; idx < 4; idx++)
        {
            if(!_sys_tsingma_datapath_dynamic_switch_judge_serdes_add(((quad_1st_flag ? 16 : 20) + idx), target))
            {
                continue;
            }
            target->s[target->serdes_num].serdes_id = (quad_1st_flag ? 16 : 20) + idx;
            target->s[target->serdes_num].src_mode = src_port->pcs_mode;
            target->s[target->serdes_num].dst_mode = dst_mode;
            target->serdes_num++;
        }
    }
    /*none -> 1-1*/
    else if(((CTC_CHIP_SERDES_NONE_MODE == src_port->pcs_mode) || (CTC_CHIP_SERDES_XSGMII_MODE == src_port->pcs_mode)) && 
            SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode))
    {
        target->m[target->mac_num].mac_id = src_port->mac_id;
        target->m[target->mac_num].add_drop_flag = add_drop_flag;
        target->mac_num++;
        if(_sys_tsingma_datapath_dynamic_switch_judge_serdes_add((quad_1st_flag ? 16 : 20), target))
        {
            target->s[target->serdes_num].serdes_id = quad_1st_flag ? 16 : 20;
            target->s[target->serdes_num].src_mode = src_port->pcs_mode;
            target->s[target->serdes_num].dst_mode = dst_mode;
            target->serdes_num++;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_switch_get_info(uint8 lchip, sys_datapath_lport_attr_t* src_port, ctc_chip_serdes_mode_t dst_mode, sys_datapath_dynamic_switch_attr_t *target, uint8 qm_chg_flag, uint8 serdes_id_raw)
{
    uint8 influenced_port_num = 0;
    uint8 port_idx = 0;
    uint8 none_flag = FALSE;
    uint8 src_mode_level = 0;
    uint8 dst_mode_level = 0;
    uint32 sereds_idx = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if(SYS_DMPS_NETWORK_PORT == src_port->port_type)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_para_check(lchip, src_port, dst_mode, serdes_id_raw));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_usxgmii0_check(lchip, src_port, dst_mode, serdes_id_raw));
        none_flag = TRUE;
    }

    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    if(_sys_tsingma_datapath_dynamic_judge_hss28g_clock_change(lchip, (0xff == serdes_id_raw ? src_port->serdes_id : serdes_id_raw), dst_mode) || 
       (SYS_DATAPATH_SERDES_IS_HSS28G(src_port->serdes_id) && 
        (CTC_CHIP_SERDES_2DOT5G_MODE == src_port->pcs_mode) && (CTC_CHIP_SERDES_XAUI_MODE == dst_mode)))
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_hss28g_clock_change_parser(lchip, src_port, dst_mode, target));
        return CTC_E_NONE;
    }

    /*12xQSGMII+4x10G+4x40G*/
    if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && (28 == src_port->mac_id || 30 == src_port->mac_id))
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_qm7_8_parser(lchip, src_port, dst_mode, target));
        return CTC_E_NONE;
    }

    /*Add initial influenced ports to target*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_add_influenced_port(lchip, src_port, dst_mode, &influenced_port_num, target));

    /*Parse initial influenced ports: 1. add influenced serdes; 2. add ports influenced by serdes... */
    for(port_idx = 0; port_idx < influenced_port_num; port_idx++)
    {
        sys_usw_mac_get_port_capability(lchip, (src_port->mac_id + port_idx), &port_attr);
        if((SYS_DMPS_RSV_PORT == src_port->port_type) && (FALSE == none_flag))
        {
            continue;
        }
        if(CTC_E_PARAM_CONFLICT == 
           _sys_tsingma_datapath_dynamic_switch_serdes_port_parser(lchip, port_attr, dst_mode, target, serdes_id_raw))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    /*Influenced serdes lane number cannot exceed 1, except 1 port 1 lane <-> 1 port 4 lane*/
    _sys_tsingma_datapath_get_mode_level(src_port->pcs_mode, &src_mode_level, NULL, NULL);
    _sys_tsingma_datapath_get_mode_level(dst_mode, &dst_mode_level, NULL, NULL);
    if((!SYS_DATAPATH_SERDES_IS_HSS28G(src_port->serdes_id)) &&
       (!((((1 == src_mode_level) || (CTC_CHIP_SERDES_NONE_MODE == src_mode_level)) && (3 == dst_mode_level)) || 
            ((3 == src_mode_level) && ((1 == dst_mode_level) || (CTC_CHIP_SERDES_NONE_MODE == dst_mode_level))) || 
            ((3 == src_mode_level) && (3 == dst_mode_level)))) &&
       (1 < target->serdes_num))
    {
        if(0xff == serdes_id_raw)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Influenced serdes lane number exceeds 1! serdes_num=%u, src_mode=%u, dst_mode=%u. \n", 
                                 target->serdes_num, src_port->pcs_mode, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
        else
        {
            for(sereds_idx = 0; sereds_idx < target->serdes_num; sereds_idx++)
            {
                if((CTC_CHIP_SERDES_NONE_MODE != target->s[sereds_idx].src_mode) && 
                   (0xff != serdes_id_raw) && (target->s[sereds_idx].serdes_id != serdes_id_raw))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Influenced serdes lane number exceeds 1! serdes_num=%u, src_mode=%u, dst_mode=%u. \n", 
                                         target->serdes_num, src_port->pcs_mode, dst_mode);
                    return CTC_E_INVALID_PARAM;
                }
            }
        }
    }


    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_set_serdes_none_mode(uint8 lchip, uint8 serdes_id)
{
    uint8 hss_idx = 0;
    uint8 slice_id = 0;
    uint8 lane_id = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    DATAPATH_INIT_CHECK();

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    slice_id = (serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss_vec == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    p_serdes = &p_hss_vec->serdes_info[lane_id];

    port_attr = sys_usw_datapath_get_port_capability(lchip, p_serdes->lport, slice_id);
    if (port_attr == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

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
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_soft_tables(uint8 lchip, sys_datapath_dynamic_switch_attr_t* ds_attr, sys_datapath_hss_pll_info_t* p_dst_pll)
{
    uint8  hss_idx = 0;
    uint8  lane_id = 0;
    uint8  i;
    uint8  internal_lane = 0;
    uint8  mode = 0;
    uint32 serdes_id = 0;
    int32  mac_id = 0;
    uint8  cmu_id = 0;
    uint8  slice_id   = 0;
    uint16 ovclk_speed = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t*    port_attr = NULL;
    
    DATAPATH_INIT_CHECK();

    /* rebuild serdes info */
    for (i = 0; i < ds_attr->serdes_num; i++)
    {
        serdes_id = ds_attr->s[i].serdes_id;
        mode = ds_attr->s[i].dst_mode;
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

        internal_lane = LANE_ID_MAP_INTERAL_LANE_ID(p_hss_vec->hss_type, lane_id);

        cmu_id = SYS_TSINGMA_PLLSEL_TO_CMUID(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].pll_sel);

        switch(ds_attr->s[i].ovclk_flag)
        {
            case 1:
                ovclk_speed = CTC_CHIP_SERDES_OCS_MODE_12_12G;
                break;
            case 3:
                ovclk_speed = CTC_CHIP_SERDES_OCS_MODE_11_06G;
                break;
            case 4:
                ovclk_speed = CTC_CHIP_SERDES_OCS_MODE_27_27G;
                break;
            case 5:
                ovclk_speed = CTC_CHIP_SERDES_OCS_MODE_12_58G;
                break;
            case 2:
            default:
                ovclk_speed = CTC_CHIP_SERDES_OCS_MODE_NONE;
                break;
        }

        CTC_ERROR_RETURN(sys_tsingma_datapath_build_serdes_info(lchip, serdes_id, mode, p_hss_vec->serdes_info[internal_lane].group, 
                                                                p_hss_vec->serdes_info[internal_lane].is_dyn, cmu_id, FALSE, FALSE, ovclk_speed));
    }
    CTC_ERROR_RETURN(_sys_tsingma_datapath_update_glb_xpipe(lchip));

    /* refresh hss-level pll info */
    for(hss_idx = 0; hss_idx < TSINGMA_HSS15G_NUM_PER_SLICE + TSINGMA_HSS28G_NUM_PER_SLICE; hss_idx++)
    {
        if(p_dst_pll[hss_idx].pll_chg_flag)
        {
            p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
            if(p_hss_vec == NULL)
            {
                continue;
            }
            p_hss_vec->plla_mode    = p_dst_pll[hss_idx].plla_mode;
            p_hss_vec->plla_ref_cnt = p_dst_pll[hss_idx].plla_ref_cnt;
            if(TSINGMA_HSS15G_NUM_PER_SLICE > hss_idx)
            {
                p_hss_vec->pllb_mode    = p_dst_pll[hss_idx].pllb_mode;
                p_hss_vec->pllb_ref_cnt = p_dst_pll[hss_idx].pllb_ref_cnt;
                p_hss_vec->pllc_mode    = p_dst_pll[hss_idx].pllc_mode;
                p_hss_vec->pllc_ref_cnt = p_dst_pll[hss_idx].pllc_ref_cnt;
            }
        }
    }

    /* refresh mcu port info */
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        mac_id = ds_attr->m[i].mac_id;
        if(2 == ds_attr->m[i].add_drop_flag)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, mac_id, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }
            port_attr->port_type = SYS_DMPS_RSV_PORT;
            port_attr->pcs_mode  = CTC_CHIP_SERDES_NONE_MODE;
        }
    }
    
    return CTC_E_NONE;
}

STATIC void
_sys_tsingma_datapath_get_switch_serdes_num(uint8 lchip, uint8 curr_mode, uint8 mode, uint32 *serdes_num)
{
    if (SYS_DATAPATH_IS_SERDES_1_PORT_1(curr_mode) || SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(curr_mode))
    {
        if (SYS_DATAPATH_IS_SERDES_1_PORT_1(mode) || (CTC_CHIP_SERDES_NONE_MODE == mode) || SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(mode))
        {
            *serdes_num = 1;
        }
        else if (SYS_DATAPATH_IS_SERDES_4_PORT_1(mode) || (CTC_CHIP_SERDES_LG_MODE == mode))
        {
            *serdes_num = 4;
        }
    }
    else if (SYS_DATAPATH_IS_SERDES_4_PORT_1(curr_mode) || (CTC_CHIP_SERDES_LG_MODE == curr_mode))
    {
        if((CTC_CHIP_SERDES_LG_MODE == curr_mode) && 
           ((CTC_CHIP_SERDES_NONE_MODE == mode) || (CTC_CHIP_SERDES_XXVG_MODE == mode)))
        {
            *serdes_num = 2;
        }
        else
        {
            *serdes_num = 4;
        }
    }
    else if (CTC_CHIP_SERDES_NONE_MODE == curr_mode)
    {
        if (SYS_DATAPATH_IS_SERDES_1_PORT_1(mode) || SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(mode))
        {
            *serdes_num = 1;
        }
        else if (SYS_DATAPATH_IS_SERDES_4_PORT_1(mode))
        {
            *serdes_num = 4;
        }
        else if(CTC_CHIP_SERDES_LG_MODE == mode)
        {
            *serdes_num = 2;
        }
    }
    else
    {
        *serdes_num = 0;
    }

    if(((CTC_CHIP_SERDES_LG_MODE == curr_mode) && (CTC_CHIP_SERDES_XXVG_MODE == mode)) || 
       ((CTC_CHIP_SERDES_XXVG_MODE == curr_mode) && (CTC_CHIP_SERDES_LG_MODE == mode)))
    {
        *serdes_num = 2;
    }
}

int32
sys_tsingma_datapath_check_queue_flush_clear(uint8 lchip, uint32 tmp_gport, uint32* p_depth, sys_qos_shape_profile_t* shp_profile)
{
    uint8 index = 0;
    
    CTC_ERROR_RETURN(sys_usw_queue_get_port_depth(lchip, tmp_gport, p_depth));
    while(*p_depth)
    {
        sal_task_sleep(20);
        if((index++) >50)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cannot flush queue depth(%d) to Zero, port %d \n", *p_depth, tmp_gport);
            CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, shp_profile));
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DATAPATH] dynamic switch fail \n");
            return CTC_E_HW_FAIL;
        }
        CTC_ERROR_RETURN(sys_usw_queue_get_port_depth(lchip, tmp_gport, p_depth));
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_check_nettx_used(uint8 lchip, uint32 mac_id, uint32 tmp_gport)
{
    uint32 cmd        = 0;
    uint8  index      = 0;
    uint32 nettx_used = 0;
    uint32 value_arr[1] = {0};
    
    cmd = DRV_IOR(NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f + mac_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_used));
    while (nettx_used)
    {
        sal_task_sleep(20);
        if((index++) >50)
        {
            DRV_IOR_FIELD(lchip, NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f + mac_id, value_arr, &nettx_used);
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NetTxCreditStatus_port%dCreditUsed cannot return to Zero, value %d \n", tmp_gport, value_arr[0]);
            return CTC_E_HW_FAIL;
        }
        cmd = DRV_IOR(NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f + mac_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_used));
    }
    
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_pre_dynamic_switch(uint8 lchip, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8  gchip      = 0;
    uint8  chan_id    = 0;
    uint8  slice_id   = 0;
    uint8  index      = 0;
    uint8  mode       = 0;
    uint16 tmp_lport  = 0;
    uint32 cmd        = 0;
    uint32 i          = 0;
    uint32 depth      = 0;
    uint32 tmp_gport  = 0;
    uint32 nettx_used = 0;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_qos_shape_profile_t shp_profile[SYS_DATAPATH_DS_MAX_PORT_NUM] = {{0}};
    ctc_chip_serdes_loopback_t lb_param = {0};

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
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_unidir_en(lchip, tmp_gport, FALSE));
            /* recover ipg to default value */
            CTC_ERROR_RETURN(_sys_usw_mac_set_ipg(lchip, tmp_lport, 12));
            /* recover preamble cfg to default value */
            CTC_ERROR_RETURN(_sys_usw_mac_set_internal_property(lchip, tmp_lport, CTC_PORT_PROP_PREAMBLE, 8));
            /* recover padding cfg to default value */
            CTC_ERROR_RETURN(_sys_usw_mac_set_internal_property(lchip, tmp_lport, CTC_PORT_PROP_PADING_EN, 1));

            /* CL73 AN status set to disable (move before pre proc) */

            /* clear mac enable cfg */
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_mac_en(lchip, tmp_gport, FALSE));

            /* clear FEC if enable */
            if ((CTC_CHIP_SERDES_CG_MODE      == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XXVG_MODE == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_LG_MODE   == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XFI_MODE  == port_attr_tmp->pcs_mode)
                || (CTC_CHIP_SERDES_XLG_MODE  == port_attr_tmp->pcs_mode))
            {
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_fec_en(lchip, tmp_lport, CTC_PORT_FEC_TYPE_NONE));
            }

            /* CL37 AN status set to enable (default) */
            if((CTC_CHIP_SERDES_SGMII_MODE == port_attr_tmp->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr_tmp->pcs_mode) ||
               (CTC_CHIP_SERDES_QSGMII_MODE == port_attr_tmp->pcs_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == port_attr_tmp->pcs_mode) ||
               (CTC_CHIP_SERDES_USXGMII1_MODE == port_attr_tmp->pcs_mode) ||(CTC_CHIP_SERDES_USXGMII2_MODE == port_attr_tmp->pcs_mode))
            {
                CTC_ERROR_RETURN(_sys_tsingma_mac_set_cl37_auto_neg_en(lchip, tmp_gport, TRUE));
            }
        }
    }

    /* disable serdes loopback */
    for (i = 0; i < ds_attr->serdes_num; i++)
    {
        lb_param.serdes_id = ds_attr->s[i].serdes_id;
        for(mode = 0; mode < 2; mode++)
        {
            lb_param.mode = mode;
            CTC_ERROR_RETURN(sys_tsingma_datapath_get_serdes_loopback(lchip, (void*)&lb_param));
            if(lb_param.enable)
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_loopback(lchip, (void*)&lb_param));
            }
        }
    }

    for (i = 0; i < ds_attr->mac_num; i++)
    {
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[i].mac_id);
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);

        if (1 != ds_attr->m[i].add_drop_flag)
        {
            /* 0. set vanished mac to drop channel */
            chan_id = MCHIP_CAP(SYS_CAP_CHANID_DROP);
            CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, tmp_lport, chan_id));

            /* 1. If port is dest, enqdrop */
            /*for restore shape profile*/
            CTC_ERROR_RETURN(sys_usw_queue_get_profile_from_hw(lchip, tmp_gport, &shp_profile[i]));
            /*enqdrop*/
            CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, tmp_gport, TRUE, &shp_profile[i]));

            /*1.1. check queue flush clear*/
            CTC_ERROR_RETURN(sys_usw_queue_get_port_depth(lchip, tmp_gport, &depth));
            while(depth)
            {
#ifndef EMULATION_ENV
                sal_task_sleep(20);
#endif
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
            while(nettx_used)
            {
#ifndef EMULATION_ENV
                sal_task_sleep(20);
#endif
                if((index++) >50)
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "NetTxCreditStatus_port%dCreditUsed cannot return to Zero \n", tmp_gport);
                    return CTC_E_HW_FAIL;
                }
                cmd = DRV_IOR(NetTxCreditStatus_t, NetTxCreditStatus_port0CreditUsed_f+ds_attr->m[i].mac_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &nettx_used));
            }
            
#ifndef EMULATION_ENV
            sal_task_sleep(100);
#endif


            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Pre dynamic switch failed \n");
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    
    for (i = 0; i < ds_attr->mac_num; i++)
    {
        sal_memcpy(&(ds_attr->shp_profile[i]), &shp_profile[i], sizeof(sys_qos_shape_profile_t));
    }

    return CTC_E_NONE;
}


int32
sys_tsingma_datapath_dynamic_reset_corequadsgmac(uint8 lchip, uint8 qm_id)
{
    uint32 value = 1;
    uint32 step = (qm_id < 16) ? (RlmMacCtlReset_resetCoreQuadSgmac1_f - RlmMacCtlReset_resetCoreQuadSgmac0_f) : 
                                 (RlmMacCtlReset_resetCoreQuadSgmac17_f - RlmMacCtlReset_resetCoreQuadSgmac16_f);
    fld_id_t field_id = (qm_id < 16) ? (RlmMacCtlReset_resetCoreQuadSgmac0_f + step*qm_id) : 
                                       (RlmMacCtlReset_resetCoreQuadSgmac16_f + step*(qm_id-16));
    RlmMacCtlReset_m mac_cfg;

    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, RlmMacCtlReset_t, 1, &field_id, &value, &mac_cfg));
    value = 0;
    CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, RlmMacCtlReset_t, 1, &field_id, &value, &mac_cfg));
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_reset_tx_buf(uint8 lchip, sys_datapath_dynamic_switch_attr_t* ds_attr, uint32* value)
{
    uint32 step = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint8  count = 0;
    int32  tmp_mac_id = 0;
    QuadSgmacCfg0_m mac_cfg;
    
    SYS_CONDITION_RETURN(NULL == ds_attr, CTC_E_INVALID_PTR);
    
    for(; count < ds_attr->mac_num; count++)  /* There are only 2 mac id definitely.*/
    {
        tmp_mac_id = ds_attr->m[count].mac_id;
        step = QuadSgmacCfg1_t - QuadSgmacCfg0_t;
        tbl_id = QuadSgmacCfg0_t + step*(11 == tmp_mac_id/4 ? 12 : 16);
        step = QuadSgmacCfg0_cfgSgmac1TxBufRst_f - QuadSgmacCfg0_cfgSgmac0TxBufRst_f;
        fld_id = QuadSgmacCfg0_cfgSgmac0TxBufRst_f + step*(tmp_mac_id%4);
        CTC_ERROR_RETURN(sys_tsingma_mac_write_table_elem(lchip, (tbls_id_t)tbl_id, 1, (fld_id_t*)(&fld_id), value, &mac_cfg));
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_reset_qm(uint8 lchip, uint8 dst_mode, sys_datapath_dynamic_switch_attr_t* ds_attr, sys_datapath_lport_attr_t* port_attr)
{
    uint32 tmp_val32 = 0;
    uint8  rst_qm_id[4] = {0xff, 0xff, 0xff, 0xff};
    uint8  idx;
    uint8  slice_id  = 0;
    uint8  rst_flag = TRUE;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    sys_datapath_lport_attr_t* tmp_port_attr = NULL;
     
    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    /* 4 serdes switched, i.e. 4 ports in one QM are all switched, reset resetCoreQuadSgmac */
    /* 1 serdes switched, and Q/U<->Q/U, reset resetCoreQuadSgmac */
    if(1 == ds_attr->serdes_num)
    {
        /*Q/U-m<->Q/U-m, reset resetCoreQuadSgmac*/
        if((SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(ds_attr->s[0].src_mode)) && 
           (SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(ds_attr->s[0].dst_mode)))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
        }
        /*Q/U-m -> U-s, and serdes id is 0/4/8, reset resetCoreQuadSgmac*/
        else if(((((CTC_CHIP_SERDES_USXGMII1_MODE == ds_attr->s[0].src_mode) || 
                    (CTC_CHIP_SERDES_QSGMII_MODE == ds_attr->s[0].src_mode)) && 
                   (CTC_CHIP_SERDES_USXGMII0_MODE == ds_attr->s[0].dst_mode)) || 
                  (((CTC_CHIP_SERDES_USXGMII1_MODE == ds_attr->s[0].dst_mode) || 
                    (CTC_CHIP_SERDES_QSGMII_MODE == ds_attr->s[0].dst_mode)) && 
                  (CTC_CHIP_SERDES_USXGMII0_MODE == ds_attr->s[0].src_mode))) && 
                 ((0 == port_attr->serdes_id) || (4 == port_attr->serdes_id) || (8 == port_attr->serdes_id)))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
        }
        /*U-s -> U-2, if serdes id is 0/4/8, check mac 2 & 3 is none then do reset; else do reset without check*/
        else if((CTC_CHIP_SERDES_USXGMII0_MODE == ds_attr->s[0].src_mode) && 
                (CTC_CHIP_SERDES_USXGMII2_MODE == ds_attr->s[0].dst_mode))
        {
            if((0 == port_attr->serdes_id) || (4 == port_attr->serdes_id) || (8 == port_attr->serdes_id))
            {
                for(idx = 2; idx < 4; idx++)
                {
                    tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, (port_attr->mac_id/4*4+idx), slice_id);
                    if(tmp_port_attr == NULL)
                    {
                        return CTC_E_NOT_INIT;
                    }
                    if((CTC_CHIP_SERDES_NONE_MODE != port_attr->pcs_mode) || (CTC_CHIP_SERDES_XSGMII_MODE != port_attr->pcs_mode))
                    {
                        rst_flag = FALSE;
                    }
                }
                if(rst_flag)
                {
                    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
            }
        }
        /*U-2 -> U-s, and serdes id is 0/4/8, do reset*/
        else if((CTC_CHIP_SERDES_USXGMII0_MODE == ds_attr->s[0].dst_mode) && 
                (CTC_CHIP_SERDES_USXGMII2_MODE == ds_attr->s[0].src_mode) &&
                ((0 == port_attr->serdes_id) || (4 == port_attr->serdes_id) || (8 == port_attr->serdes_id)))
        {
            rst_flag = TRUE;
            for(idx = 2; idx < 4; idx++)
            {
                tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, (port_attr->mac_id/4*4+idx), slice_id);
                if(tmp_port_attr == NULL)
                {
                    return CTC_E_NOT_INIT;
                }
                if((CTC_CHIP_SERDES_NONE_MODE != port_attr->pcs_mode) || (CTC_CHIP_SERDES_XSGMII_MODE != port_attr->pcs_mode))
                {
                    rst_flag = FALSE;
                }
            }
            if(rst_flag)
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
            }
        }
    }
    else if(4 == ds_attr->serdes_num)
    {
        if((SYS_DATAPATH_IS_SERDES_4_PORT_1(ds_attr->s[0].src_mode)) && 
           (SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(ds_attr->s[0].dst_mode)))
        {
            for(idx = 0; idx < ds_attr->serdes_num; idx++)
            {
                CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, ds_attr->s[idx].serdes_id, &p_serdes));
                if((CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode) || 
                   (CTC_CHIP_SERDES_XSGMII_MODE == p_serdes->mode))
                {
                    continue;
                }
                tmp_port_attr = sys_usw_datapath_get_port_capability(lchip, p_serdes->lport, slice_id);
                if(tmp_port_attr == NULL)
                {
                    return CTC_E_NOT_INIT;
                }
                if((CTC_CHIP_SERDES_NONE_MODE == tmp_port_attr->pcs_mode) || 
                   (CTC_CHIP_SERDES_XSGMII_MODE == tmp_port_attr->pcs_mode))
                {
                    continue;
                }
                if(0xff == rst_qm_id[idx])
                {
                    rst_qm_id[idx] = tmp_port_attr->tbl_id/4;
                }
            }
            for(idx = 0; idx < ds_attr->serdes_num; idx++)
            {
                if(0xff == rst_qm_id[idx])
                {
                    continue;
                }
                CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, rst_qm_id[idx]));
            }
        }
        else
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, (port_attr->tbl_id/4)));
        }
    }

    /* 8 serdes switched, there are 2 posiblities:
       #1 28G serdes clock change, which means QM12 & 17 need resetCoreQuadSgmac
       #2 the choice of txqm1 is 4567/8, and port 28/30 XLG->any, which means QM7 & 8 need resetCoreQuadSgmac*/
    if(8 == ds_attr->serdes_num)
    {
        if(_sys_tsingma_datapath_dynamic_judge_hss28g_clock_change(lchip, ds_attr->s[0].serdes_id, ds_attr->s[0].dst_mode))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, 12));
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, 16));
        }
        else if((1 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && (28 == ds_attr->m[0].mac_id) && (CTC_CHIP_SERDES_XLG_MODE == ds_attr->s[0].src_mode))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, 7));
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_corequadsgmac(lchip, 8));
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    /* 2 serdes switched, if XXVG<->LG, reset port QuadSgmacCfg.cfgSgmac[0..3]TxBufRst */
    if((2 == ds_attr->serdes_num) && 
       (((CTC_CHIP_SERDES_LG_MODE == dst_mode) && (CTC_CHIP_SERDES_XXVG_MODE == ds_attr->s[0].src_mode) && (CTC_CHIP_SERDES_XXVG_MODE == ds_attr->s[1].src_mode)) || 
       ((CTC_CHIP_SERDES_XXVG_MODE == dst_mode) && (CTC_CHIP_SERDES_LG_MODE == ds_attr->s[0].src_mode) && (CTC_CHIP_SERDES_LG_MODE == ds_attr->s[1].src_mode))))
    {
        tmp_val32 = 1;
        CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_tx_buf(lchip, ds_attr, &tmp_val32));
        tmp_val32 = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_tx_buf(lchip, ds_attr, &tmp_val32));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_set_hss_per_port(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr)
{
    uint16 serdes_id         = 0;
    uint16 chip_port         = 0;
    uint8  serdes_num        = 0;
    uint32 cmd               = 0;
    uint32 step              = 0;
    uint32 tbl_id            = 0;
    uint32 slice_id          = 0;
    uint32 field_id          = 0;
    uint32 tx_value          = 0;
    uint32 hss_macro_idx     = 0;
    uint32 inner_serdes_lane = 0;
    Hss12GMacroCfg0_m hss_cfg   ;
    Hss28GMacroCfg_m  hss28_cfg ;
    
    
    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    if(CTC_E_NONE != sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &serdes_id, &serdes_num))
    {
        return CTC_E_INVALID_CONFIG;
    }
    
    /*hss12gmacrocfg#[0...2].cfgHssL#[0...7]TxDataOutSel:
      0~sgmii  1~xaui  2~xfi/xlg  3~qsgmii 4~usxgmii*/
    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        step = Hss12GMacroCfg0_cfgHssL1TxDataOutSel_f - Hss12GMacroCfg0_cfgHssL0TxDataOutSel_f;
        switch (port_attr->pcs_mode) {
            case CTC_CHIP_SERDES_SGMII_MODE:
            case CTC_CHIP_SERDES_2DOT5G_MODE:
            case CTC_CHIP_SERDES_100BASEFX_MODE:
                tx_value = 0;
                break;
            case CTC_CHIP_SERDES_XAUI_MODE:
            case CTC_CHIP_SERDES_DXAUI_MODE:
                tx_value = 1;
                break;
            case CTC_CHIP_SERDES_XFI_MODE:
            case CTC_CHIP_SERDES_XLG_MODE:
                tx_value = 2;
                break;
            case CTC_CHIP_SERDES_QSGMII_MODE:
                tx_value = 3;
                break;
            case CTC_CHIP_SERDES_USXGMII0_MODE:
            case CTC_CHIP_SERDES_USXGMII1_MODE:
            case CTC_CHIP_SERDES_USXGMII2_MODE:
                tx_value = 4;
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
        
        hss_macro_idx = serdes_id / 8;
        tbl_id = Hss12GMacroCfg0_t + hss_macro_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_cfg));
        
        for(inner_serdes_lane = 0; inner_serdes_lane < serdes_num; inner_serdes_lane++)
        {
            field_id = Hss12GMacroCfg0_cfgHssL0TxDataOutSel_f + step*(serdes_id%8 + inner_serdes_lane);
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &tx_value, &hss_cfg);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_cfg));
    }
    
    /*hss28gmacrocfg.cfgHssL#[0...7]TxDataOutSel:
      0~sgmii  1~xaui  2~xfi/xlg  3~xxvg/lg/cg*/
    else
    {
        step = Hss28GMacroCfg_cfgHssL1TxDataOutSel_f - Hss28GMacroCfg_cfgHssL0TxDataOutSel_f;
        switch (port_attr->pcs_mode) {
            case CTC_CHIP_SERDES_SGMII_MODE:
            case CTC_CHIP_SERDES_2DOT5G_MODE:
                tx_value = 0;
                break;
            case CTC_CHIP_SERDES_XAUI_MODE:
            case CTC_CHIP_SERDES_DXAUI_MODE:
                tx_value = 1;
                break;
            case CTC_CHIP_SERDES_XFI_MODE:
            case CTC_CHIP_SERDES_XLG_MODE:
                tx_value = 2;
                break;
            case CTC_CHIP_SERDES_XXVG_MODE:
            case CTC_CHIP_SERDES_LG_MODE:
            case CTC_CHIP_SERDES_CG_MODE:
                tx_value = 3;
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
        tbl_id = Hss28GMacroCfg_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28_cfg));

        for(inner_serdes_lane = 0; inner_serdes_lane < serdes_num; inner_serdes_lane++)
        {
            field_id = Hss28GMacroCfg_cfgHssL0TxDataOutSel_f + step*(serdes_id - SYS_DATAPATH_SERDES_28G_START_ID + inner_serdes_lane);
            DRV_IOW_FIELD(lchip, tbl_id, field_id, &tx_value, &hss28_cfg);
        }

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28_cfg));
    }
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_get_influenced_txqm(uint8 lchip, uint8 slice_id, sys_datapath_dynamic_switch_attr_t* ds_attr, sys_datapath_influenced_txqm_t* p_dest_txqm)
{
    uint32 mac_cnt ;
    uint32 txqm_cnt;
    uint8  txqm_id = 0;
    bool   add_flag = TRUE;

    SYS_CONDITION_RETURN(((NULL == p_dest_txqm) || (NULL == ds_attr)), CTC_E_INVALID_PTR);
    
    for (mac_cnt = 0; mac_cnt < ds_attr->mac_num; mac_cnt++)
    {
        txqm_id = ds_attr->m[mac_cnt].mac_id / SYS_CALENDAR_PORT_NUM_PER_TXQM;
        
        for(txqm_cnt = 0; txqm_cnt < p_dest_txqm->txqm_num; txqm_cnt++)
        {
            if(txqm_id == p_dest_txqm->txqm_id[txqm_cnt])
            {
                add_flag = FALSE;
                break;
            }
        }
        
        if(add_flag && (p_dest_txqm->txqm_num < SYS_CALENDAR_TXQM_NUM))
        {
            p_dest_txqm->txqm_id[p_dest_txqm->txqm_num] = txqm_id;
            (p_dest_txqm->txqm_num)++;
        }
        
        add_flag = TRUE;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_calendar_reconfig(uint8 lchip, uint8 slice_id, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8 txqm_cnt;
    sys_datapath_influenced_txqm_t dest_txqm = {0};

    CTC_ERROR_RETURN(sys_tsingma_datapath_get_influenced_txqm(lchip, slice_id, ds_attr, &dest_txqm));

    for(txqm_cnt= 0; txqm_cnt < dest_txqm.txqm_num; txqm_cnt++)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_set_calendar_cfg(lchip, slice_id, dest_txqm.txqm_id[txqm_cnt]));
    }
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_get_s_ptr(uint8 lchip, uint16 lport, uint8 mode, uint8 is_body_flag, uint32* s_pointer, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8  slice_id   = 0;
    uint32 cmd        = 0;
    uint32 tbl_id     = is_body_flag ? EpeScheduleBodyRamConfigRa_t : EpeScheduleSopRamConfigRa_t;
    uint32 fld_id     = is_body_flag ? EpeScheduleBodyRamConfigRa_cfgEndPtr_f : EpeScheduleSopRamConfigRa_cfgEndPtr_f;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    sys_datapath_lport_attr_t* port_attr_0 = NULL;
    
    cmd = DRV_IOR((tbl_id+slice_id), fld_id);
    if(((CTC_CHIP_SERDES_LG_MODE == mode) && (0 != lport%4)) || 
       ((2 == ds_attr->serdes_num) && (2 == ds_attr->mac_num) && (CTC_CHIP_SERDES_XXVG_MODE == mode) && ((46 == lport) || (62 == lport))))
    {
        /*any|any|any|any -> any|any|lg|lg or any|any|lg|lg -> any|any|xxvg|xxvg
        if lane 0 & 1 is LG, take (lport-2) as prev port, else take (lport-1)*/
        port_attr_0 = sys_usw_datapath_get_port_capability(lchip, lport-2, slice_id);
        if(!port_attr_0)
        {
            return CTC_E_INVALID_PTR;
        }
        if(CTC_CHIP_SERDES_LG_MODE == port_attr_0->pcs_mode)
        {
            port_attr_tmp = port_attr_0;
        }
        else
        {
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, lport-1, slice_id);
            if(!port_attr_tmp)
            {
                return CTC_E_INVALID_PTR;
            }
        }

        DRV_IOCTL(lchip, port_attr_tmp->chan_id, cmd, s_pointer);
        *s_pointer += 1;
    }
    else
    {
        port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, lport-1, slice_id);
        if(!port_attr_tmp)
        {
            return CTC_E_INVALID_PTR;
        }

        if((0 == (lport-1)%4) &&
           ((CTC_CHIP_SERDES_NONE_MODE == port_attr_tmp->pcs_mode) || 
           (CTC_CHIP_SERDES_XSGMII_MODE == port_attr_tmp->pcs_mode)))
        {
            *s_pointer = g_tsingma_epesch_mem_origin[(port_attr_tmp->chan_id)/4];
            if(!is_body_flag)
            {
                *s_pointer *= 2;
            }
            if(((48 <= port_attr_tmp->tbl_id) && (51 >= port_attr_tmp->tbl_id)) || 
               ((64 <= port_attr_tmp->tbl_id) && (67 >= port_attr_tmp->tbl_id)))
            {
                *s_pointer += (is_body_flag ? g_tsingma_bufsz_step[3].body_buf_num : g_tsingma_bufsz_step[3].sop_buf_num);
            }
            else
            {
                *s_pointer += (is_body_flag ? g_tsingma_bufsz_step[4].body_buf_num : g_tsingma_bufsz_step[4].sop_buf_num);
            }
        }
        else
        {
            DRV_IOCTL(lchip, port_attr_tmp->chan_id, cmd, s_pointer);
            *s_pointer += 1;
        }
    }

    return CTC_E_NONE;
}

/*4lane1port or LG -> none mode, distribute Epe mem.
For example, CG mode body mem length is 40 which equals to 4 * 25G length, 
If CG -> XXVG directly, the new 4 * 25G will get mem correctly; 
But if CG -> none -> XXVG, the new XXVG will alloc mem based on old mem start point, 
but CG exhausted all 4 length in this QM, so port 1~3 start & end point are all 0, 
and the new XXVG cannot get mem correctly.
This function is designed for such condition, to make 4lane1port or LG mode release mem, 
and distribute mem into 4 or 2 part in current QM, then next time new port 1~3 can get mem correctly.*/
int32
_sys_tsingma_datapath_dynamic_epe_mem_distribute(uint8 lchip, uint8 mode, uint8 is_body_flag, 
                                                                 sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8  idx;
    uint8  step      = 0;
    uint8  chanid    = 0;
    uint32 cmd       = 0;
    uint32 start_ptr = 0;
    uint32 end_ptr   = 0;
    uint32 tbl_id    = is_body_flag ? EpeScheduleBodyRamConfigRa_t : EpeScheduleSopRamConfigRa_t;
    uint32 fld_end   = is_body_flag ? EpeScheduleBodyRamConfigRa_cfgEndPtr_f : EpeScheduleSopRamConfigRa_cfgEndPtr_f;
    uint32 fld_start = is_body_flag ? EpeScheduleBodyRamConfigRa_cfgStartPtr_f : EpeScheduleSopRamConfigRa_cfgStartPtr_f;


    /*4lane1port -> none    distribute channel 0 mem into 4 parts*/
    if((4 == ds_attr->serdes_num) && (1 == ds_attr->mac_num) && (2 == ds_attr->m[0].add_drop_flag) &&
       ((CTC_CHIP_SERDES_NONE_MODE == mode) || (CTC_CHIP_SERDES_XSGMII_MODE == mode)) &&
       SYS_DATAPATH_IS_SERDES_4_PORT_1(ds_attr->s[0].src_mode))
    {
        chanid = (ds_attr->m[0].mac_id)/4*4;

        if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && ((28 == chanid) || (30 == chanid)))
        {
            return CTC_E_NONE;
        }

        if(SYS_DATAPATH_SERDES_IS_HSS28G(ds_attr->s[0].serdes_id))
        {
            step = is_body_flag ? g_tsingma_bufsz_step[3].body_buf_num : g_tsingma_bufsz_step[3].sop_buf_num;
        }
        else
        {
            step = is_body_flag ? g_tsingma_bufsz_step[4].body_buf_num : g_tsingma_bufsz_step[4].sop_buf_num;
        }

        cmd = DRV_IOR(tbl_id, fld_start);
        DRV_IOCTL(lchip, chanid, cmd, &start_ptr);
        end_ptr = start_ptr - 1;

        for(idx= 0; idx < 4; idx++)
        {
            start_ptr = end_ptr + 1;
            end_ptr   = start_ptr + step - 1;

            cmd = DRV_IOW(tbl_id, fld_start);
            DRV_IOCTL(lchip, chanid+idx, cmd, &start_ptr);
            cmd = DRV_IOW(tbl_id, fld_end);
            DRV_IOCTL(lchip, chanid+idx, cmd, &end_ptr);
        }
    }
    /*lg -> none    distribute channel 0/2 mem into 2 parts*/
    if((2 == ds_attr->serdes_num) && (1 == ds_attr->mac_num) && (2 == ds_attr->m[0].add_drop_flag) &&
       ((CTC_CHIP_SERDES_NONE_MODE == mode) || (CTC_CHIP_SERDES_XSGMII_MODE == mode)) &&
       (CTC_CHIP_SERDES_LG_MODE == ds_attr->s[0].src_mode))
    {
        chanid = (ds_attr->m[0].mac_id)/2*2;

        if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && ((28 == chanid) || (30 == chanid)))
        {
            return CTC_E_NONE;
        }

        step   = is_body_flag ? g_tsingma_bufsz_step[3].body_buf_num : g_tsingma_bufsz_step[3].sop_buf_num;

        cmd = DRV_IOR(tbl_id, fld_start);
        DRV_IOCTL(lchip, chanid, cmd, &start_ptr);
        end_ptr = start_ptr - 1;
 
        for(idx= 0; idx < 2; idx++)
        {
            start_ptr = end_ptr + 1;
            end_ptr   = start_ptr + step - 1;

            cmd = DRV_IOW(tbl_id, fld_start);
            DRV_IOCTL(lchip, chanid+idx, cmd, &start_ptr);
            cmd = DRV_IOW(tbl_id, fld_end);
            DRV_IOCTL(lchip, chanid+idx, cmd, &end_ptr);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_set_post_dynamic_switch(uint8 lchip, uint16 lport, uint8 mode, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8  gchip      = 0;
    uint8  chan_id    = 0;
    uint8  prev_chan  = 0xff;
    uint8  slice_id   = 0;
    uint16 tmp_lport  = 0;
    uint16 e_pointer1 = 0;
    uint16 e_pointer2 = 0;
    uint32 s_pointer1 = 0;
    uint32 s_pointer2 = 0;
    uint32 tmp_gport  = 0;
    uint32 cmd        = 0;
    uint32 mac_idx    = 0;
    uint32 tmp_val32  = 0;
    ctc_port_if_type_t         if_type = 0;
    sys_datapath_lport_attr_t* port_attr     = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;

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
    
    SYS_DATAPATH_GET_IFTYPE_WITH_MODE(mode, if_type);

    /* update datapath */
    for (mac_idx = 0; mac_idx < ds_attr->mac_num; mac_idx++)
    {
        if (2 == ds_attr->m[mac_idx].add_drop_flag)
        {
            continue;
        }
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[mac_idx].mac_id);
        port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
        if (port_attr_tmp == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed (update datapath)\n");
            return CTC_E_INVALID_PARAM;
        }

        /* #1. update NetTxPortStaticInfo_portMode_f */
        CTC_ERROR_RETURN(_sys_tsingma_get_nettx_port_mode_by_speed(port_attr->speed_mode, &tmp_val32));
        cmd = DRV_IOW((NetTxPortStaticInfo_t+slice_id), NetTxPortStaticInfo_portMode_f);
        DRV_IOCTL(lchip, ds_attr->m[mac_idx].mac_id, cmd, &tmp_val32);

        /* #2. reconfig EpeSchedule */
        chan_id = port_attr_tmp->chan_id;
        if(0 != tmp_lport % 4)
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_get_prev_used_chanid(lchip, slice_id, tmp_lport, &prev_chan));
            if (prev_chan != 0xff)
            {
                _sys_tsingma_datapath_dynamic_get_s_ptr(lchip, tmp_lport, mode, 1, &s_pointer1, ds_attr);
                _sys_tsingma_datapath_dynamic_get_s_ptr(lchip, tmp_lport, mode, 0, &s_pointer2, ds_attr);
            }
        }
        else
        {
            cmd = DRV_IOR((EpeScheduleBodyRamConfigRa_t+slice_id), EpeScheduleBodyRamConfigRa_cfgStartPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &s_pointer1);
            cmd = DRV_IOR((EpeScheduleSopRamConfigRa_t+slice_id), EpeScheduleSopRamConfigRa_cfgStartPtr_f);
            DRV_IOCTL(lchip, chan_id, cmd, &s_pointer2);
        }
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_body_mem(lchip, slice_id, chan_id, s_pointer1, &e_pointer1));
        s_pointer1 = e_pointer1;
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_sop_mem(lchip, slice_id, chan_id, s_pointer2, &e_pointer2));
        s_pointer2 = e_pointer2;

        /* #3. reconfig bufretrv credit, qmgr credit don't need config, beause bufretrv alloc buffer maximal*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_credit(lchip, slice_id, chan_id));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_bufretrv_credit_config(lchip, slice_id));
    }

    /*global cfg need config at last time*/
    /* #4. reconfig Netrx*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_buf_mgr(lchip, slice_id));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_netrx_wrr_weight(lchip, slice_id));

    /* #5. reconfig qmgr*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_qmgr_wrr_weight(lchip, slice_id));

    /* #6. reconfig bufretrv, buf bufretrv buffer pointer need not config*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_bufretrv_wrr_weight(lchip, slice_id));

    /* #7. reconfig chan distribute*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_net_chan_cfg(lchip));

    /* #8. reconfig calendar*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_calendar_reconfig(lchip, slice_id, ds_attr));

    /* #9. reconfig epe schedule*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_epe_wrr_weight(lchip, slice_id));

    /* #10. reset QuadSgmac*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_reset_qm(lchip, mode, ds_attr, port_attr));
    /* #11. reconfig MAC */
    for (mac_idx = 0; mac_idx < ds_attr->mac_num; mac_idx++)
    {
        if (2 != ds_attr->m[mac_idx].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[mac_idx].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed (reconfig mac)\n");
                return CTC_E_INVALID_PARAM;
            }
            /* 11.1 reconfig mac */
            CTC_ERROR_RETURN(_sys_tsingma_mac_set_mac_config(lchip, tmp_lport, port_attr_tmp, TRUE));
            /* temp code: config hss12gmacrocfg#[0...2].cfgHssL#[0...7]TxDataOutSel */            
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_set_hss_per_port(lchip, tmp_lport, port_attr_tmp));
            /* 11.2 reconfig dlb */
            CTC_ERROR_RETURN(sys_usw_peri_set_dlb_chan_type(lchip, port_attr_tmp->chan_id));
            /* 11.3 update port_attr->interface_type */
            port_attr_tmp->interface_type = if_type;
            /* 11.4 reset port_attr->an_fec to default */
            port_attr_tmp->an_fec = (1 << CTC_PORT_FEC_TYPE_RS)|(1 << CTC_PORT_FEC_TYPE_BASER);
        }
    }

    /* #12. disable enqdrop, only network port use */
    for (mac_idx = 0; mac_idx < ds_attr->mac_num; mac_idx++)
    {
        if (2 != ds_attr->m[mac_idx].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[mac_idx].mac_id);
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            tmp_gport = CTC_MAP_LPORT_TO_GPORT(gchip, tmp_lport);
            sys_usw_queue_set_port_drop_en(lchip, tmp_gport, FALSE, &(ds_attr->shp_profile[mac_idx]));
        }
    }

    /* #13. add new-built mac to channel */
    for (mac_idx = 0; mac_idx < ds_attr->mac_num; mac_idx++)
    {
        if (2 != ds_attr->m[mac_idx].add_drop_flag)
        {
            tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, ds_attr->m[mac_idx].mac_id);
            port_attr_tmp = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr_tmp == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port dynamic switch failed (add new-built mac)\n");
                return CTC_E_INVALID_PARAM;
            }
            sys_usw_add_port_to_channel(lchip, tmp_lport, port_attr_tmp->chan_id);
        }
    }

    /*  #14. reconfig MDIO scan */
    sys_tsingma_peri_set_phy_scan_cfg(lchip);

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_switch_clear_pll(sys_datapath_hss_pll_info_t* p_hss_pll, uint8 lane_id)
{
    switch(p_hss_pll->serdes_pll_sel[lane_id].pll_sel)
    {
        case 1:
            if(0 != p_hss_pll->plla_ref_cnt)
            {
                (p_hss_pll->plla_ref_cnt)--;
            }
            else 
            {
                return CTC_E_INVALID_PARAM;
            }
            if(0 == p_hss_pll->plla_ref_cnt)
            {
                p_hss_pll->plla_mode = SYS_DATAPATH_CMU_IDLE;
            }
            if(HSS12G_LANE_NUM < p_hss_pll->plla_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            break;
        case 2:
            if(0 != p_hss_pll->pllb_ref_cnt)
            {
                (p_hss_pll->pllb_ref_cnt)--;
            }
            else 
            {
                return CTC_E_INVALID_PARAM;
            }
            if(0 == p_hss_pll->pllb_ref_cnt)
            {
                p_hss_pll->pllb_mode = SYS_DATAPATH_CMU_IDLE;
            }
            if(HSS12G_LANE_NUM < p_hss_pll->pllb_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            break;
        case 3:
            if(0 != p_hss_pll->pllc_ref_cnt)
            {
                (p_hss_pll->pllc_ref_cnt)--;
            }
            else 
            {
                return CTC_E_INVALID_PARAM;
            }
            if(0 == p_hss_pll->pllc_ref_cnt)
            {
                p_hss_pll->pllc_mode = SYS_DATAPATH_CMU_IDLE;
            }
            if(HSS12G_LANE_NUM < p_hss_pll->pllc_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 0;
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_switch_add_pll_hss12g(sys_datapath_hss_pll_info_t* p_hss_pll, uint8 lane_id)
{
    uint8 pll_mode = 0;
    uint8 dst_ovclk = (SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_12DOT5G_MODE == p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk) ? 
                      TRUE : FALSE;
    
    SYS_TSINGMA_12G_GET_CMU_TYPE(p_hss_pll->serdes_pll_sel[lane_id].dst_mode, dst_ovclk, pll_mode);

    /*1st quad lane*/
    if(HSS12G_LANE_NUM/2 > lane_id)
    {
        /*use CMU0, e.g. PLLa*/
        if(pll_mode == p_hss_pll->plla_mode)
        {
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 1;
            p_hss_pll->plla_ref_cnt++;
            if(HSS12G_LANE_NUM/2 < p_hss_pll->plla_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }

        /*use CMU1, e.g. PLLb*/
        if(pll_mode == p_hss_pll->pllb_mode)
        {
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 2;
            p_hss_pll->pllb_ref_cnt++;
            if(HSS12G_LANE_NUM < p_hss_pll->pllb_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }
        
        /*use CMU0, e.g. PLLa*/
        if(SYS_DATAPATH_CMU_IDLE == p_hss_pll->plla_mode)
        {
            p_hss_pll->plla_mode = pll_mode;
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 1;
            p_hss_pll->plla_ref_cnt++;
            if(HSS12G_LANE_NUM/2 < p_hss_pll->plla_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }

        /*use CMU1, e.g. PLLb*/
        if(SYS_DATAPATH_CMU_IDLE == p_hss_pll->pllb_mode)
        {
            p_hss_pll->pllb_mode = pll_mode;
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 2;
            p_hss_pll->pllb_ref_cnt++;
            if(HSS12G_LANE_NUM < p_hss_pll->pllb_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }
        /*cannot allocate pll*/
        return CTC_E_INVALID_PARAM;
    }
    
    /*2nd quad lane*/
    else
    {
        /*use CMU2, e.g. PLLc*/
        if(pll_mode == p_hss_pll->pllc_mode)
        {
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 3;
            p_hss_pll->pllc_ref_cnt++;
            if(HSS12G_LANE_NUM/2 < p_hss_pll->pllc_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }

        /*use CMU1, e.g. PLLb*/
        if(pll_mode == p_hss_pll->pllb_mode)
        {
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 2;
            p_hss_pll->pllb_ref_cnt++;
            if(HSS12G_LANE_NUM < p_hss_pll->pllb_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }
        
        /*use CMU2, e.g. PLLc*/
        if(SYS_DATAPATH_CMU_IDLE == p_hss_pll->pllc_mode)
        {
            p_hss_pll->pllc_mode = pll_mode;
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 3;
            p_hss_pll->pllc_ref_cnt++;
            if(HSS12G_LANE_NUM/2 < p_hss_pll->pllc_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }

        /*use CMU1, e.g. PLLb*/
        if(SYS_DATAPATH_CMU_IDLE == p_hss_pll->pllb_mode)
        {
            p_hss_pll->pllb_mode = pll_mode;
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = 2;
            p_hss_pll->pllb_ref_cnt++;
            if(HSS12G_LANE_NUM < p_hss_pll->pllb_ref_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
            return CTC_E_NONE;
        }
        /*cannot allocate pll*/
        return CTC_E_INVALID_PARAM;
    }
    
    return CTC_E_NONE;
}

/*
Judge if pll config is available to hss12g. No database info (soft table) or reg table changed here.
All the modification of pll will be saved in p_dst_pll, which is the basis of soft/reg table change config.
*/
int32
_sys_tsingma_datapath_dynamic_switch_pll_parser(uint8 lchip, sys_datapath_dynamic_switch_attr_t* p_ds_attr, sys_datapath_hss_pll_info_t* p_dst_pll)
{
    int32 ret = 0;
    uint8 index;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 src_cmu_type = 0;
    uint8 dst_cmu_type = 0;
    uint8 find_flag = FALSE;
    uint32 serdes_id = 0;
    uint8 src_ovclk = 0; /*0-disable  1-(Any)XFI 12.5G  2-(28G)XFI 11.40625G  3-(28G)XXVG 28.125G*/
    uint8 dst_ovclk = 0;
    /*uint8 src_width = 0;
    uint8 src_div = 0;
    uint8 src_speed = 0;
    uint8 dst_width = 0;
    uint8 dst_div = 0;
    uint8 dst_speed = 0;*/
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_hss_pll_info_t* p_hss_pll = NULL;

    /* 1. HSS12G clear old serdes pll choice */
    for(hss_idx = 0; hss_idx < HSS12G_NUM_PER_SLICE; hss_idx++)
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }
        
        /*init p_dst_pll*/
        p_hss_pll = &(p_dst_pll[hss_idx]);
        p_hss_pll->plla_mode = p_hss_vec->plla_mode;
        p_hss_pll->pllb_mode = p_hss_vec->pllb_mode;
        p_hss_pll->pllc_mode = p_hss_vec->pllc_mode;
        p_hss_pll->plla_ref_cnt = p_hss_vec->plla_ref_cnt;
        p_hss_pll->pllb_ref_cnt = p_hss_vec->pllb_ref_cnt;
        p_hss_pll->pllc_ref_cnt = p_hss_vec->pllc_ref_cnt;
        for(index = 0; index < HSS12G_LANE_NUM; index++)
        {
            p_hss_pll->serdes_pll_sel[index].pll_sel = p_hss_vec->serdes_info[index].pll_sel;
        }
        
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            serdes_id = hss_idx*HSS12G_LANE_NUM + lane_id;

            /*find serdes in p_ds_attr*/
            for(index = 0; index < p_ds_attr->serdes_num; index++)
            {
                if(serdes_id == p_ds_attr->s[index].serdes_id)
                {
                    find_flag = TRUE;
                    break;
                }
            }
            if(!find_flag)
            {
                continue;
            }
            find_flag = FALSE;

            /* alanyze pll change condition */
            if(0 == p_ds_attr->s[index].ovclk_flag)
            {
                src_ovclk = 0;
                dst_ovclk = 0;
            }
            else if(1 == p_ds_attr->s[index].ovclk_flag)
            {
                src_ovclk = 0;
                dst_ovclk = 1;
            }
            else
            {
                src_ovclk = 1;
                dst_ovclk = 0;
            }
            SYS_TSINGMA_12G_GET_CMU_TYPE(p_ds_attr->s[index].src_mode, src_ovclk, src_cmu_type);
            SYS_TSINGMA_12G_GET_CMU_TYPE(p_ds_attr->s[index].dst_mode, dst_ovclk, dst_cmu_type);
            /*no pll change, do nothing*/
            if(src_cmu_type == dst_cmu_type)
            {
                /*mode change but CMU same, still need reconfig & reset lane (e.g. 10G->40G)*/
                if(p_ds_attr->s[index].src_mode == p_ds_attr->s[index].dst_mode)
                {
                    continue;
                }
            }
            /*record change flag*/
            p_hss_pll->serdes_pll_sel[lane_id].src_mode = p_ds_attr->s[index].src_mode;
            p_hss_pll->serdes_pll_sel[lane_id].dst_mode = p_ds_attr->s[index].dst_mode;
            p_hss_pll->serdes_pll_sel[lane_id].ovclk_flag = p_ds_attr->s[index].ovclk_flag;
            p_hss_pll->serdes_pll_sel[lane_id].chg_flag = TRUE;
            if(1 == p_ds_attr->s[index].ovclk_flag)
            {
                p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_12DOT5G_MODE;
            }
            else if(2 == p_ds_attr->s[index].ovclk_flag)
            {
                p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = 0xffff;//enable to disable
            }
            else
            {
                p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_NONE_MODE;
            }
            if(!p_hss_pll->pll_chg_flag)
            {
                if(src_cmu_type == dst_cmu_type)
                {
                    p_hss_pll->pll_chg_flag = 2;
                }
                else
                {
                    p_hss_pll->pll_chg_flag = 1;
                }
            }
            /*none->any mode, no need to clear source mode*/
            if(CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[index].src_mode)
            {
                continue;
            }

            /*source mode pllx clear*/
            ret = _sys_tsingma_datapath_dynamic_switch_clear_pll(p_hss_pll, lane_id);
            if(CTC_E_NONE != ret)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Cannot clear pll info! hss_idx %u, lane_id %u\n", hss_idx, lane_id);
                return ret;
            }
        }
    }

    /* 2. HSS12G Add new serdes pll choice */
    for(hss_idx = 0; hss_idx < HSS12G_NUM_PER_SLICE; hss_idx++)
    {
        p_hss_pll = &(p_dst_pll[hss_idx]);
        if(0 == p_hss_pll->pll_chg_flag)
        {
            continue;
        }
        
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            if((FALSE == p_hss_pll->serdes_pll_sel[lane_id].chg_flag) || 
               (CTC_CHIP_SERDES_NONE_MODE == p_hss_pll->serdes_pll_sel[lane_id].dst_mode))
            {
                continue;
            }
            /*add serdes pll*/
            ret = _sys_tsingma_datapath_dynamic_switch_add_pll_hss12g(p_hss_pll, lane_id);
            if(CTC_E_NONE != ret)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "PLL resource is not available! hss_idx %u, lane_id %u\n", hss_idx, lane_id);
                return ret;
            }
        }
    }

    /* 2a. if CMU type no change but CMU ID changes, fix pll_chg_flag based on new pll usage */
    for(hss_idx = 0; hss_idx < HSS12G_NUM_PER_SLICE; hss_idx++)
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }
        p_hss_pll = &(p_dst_pll[hss_idx]);

        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            if((FALSE == p_hss_pll->serdes_pll_sel[lane_id].chg_flag) || 
               (CTC_CHIP_SERDES_NONE_MODE == p_hss_pll->serdes_pll_sel[lane_id].src_mode))
            {
                continue;
            }

            if(1 == p_hss_pll->pll_chg_flag)
            {
                /*pll id changed, and new pll id is already in use, do not reset pll*/
                if((p_hss_vec->serdes_info[lane_id].pll_sel != p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && 
                   (((1 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->plla_ref_cnt)) || 
                    ((2 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->pllb_ref_cnt)) || 
                    ((3 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->pllc_ref_cnt))))
                {
                    p_hss_pll->pll_chg_flag = 2;
                }
            }
            else
            {
                /*pll id changed, and new pll id is already in use, do not reset pll*/
                if((p_hss_vec->serdes_info[lane_id].pll_sel != p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && 
                   (((1 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->plla_ref_cnt)) || 
                    ((2 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->pllb_ref_cnt)) || 
                    ((3 == p_hss_pll->serdes_pll_sel[lane_id].pll_sel) && (0 != p_hss_vec->pllc_ref_cnt))))
                {
                    p_hss_pll->pll_chg_flag = 2;
                }
            }
        }
    }

    /* 3. HSS28G pll change */
    find_flag = FALSE;
    for(hss_idx = HSS12G_NUM_PER_SLICE; hss_idx < HSS12G_NUM_PER_SLICE + TSINGMA_HSS28G_NUM_PER_SLICE; hss_idx++)
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }
        p_hss_pll = &(p_dst_pll[hss_idx]);
        p_hss_pll->plla_mode = p_hss_vec->plla_mode;
        p_hss_pll->plla_ref_cnt = p_hss_vec->plla_ref_cnt;
        for(lane_id = 0; lane_id < HSS28G_LANE_NUM; lane_id++)
        {
            p_hss_pll->serdes_pll_sel[lane_id].pll_sel = p_hss_vec->serdes_info[lane_id].pll_sel;
            //find serdes_id in p_ds_attr
            serdes_id = SYS_DATAPATH_SERDES_28G_START_ID + (hss_idx-HSS12G_NUM_PER_SLICE)*HSS28G_LANE_NUM + lane_id;
            for(index = 0; index < p_ds_attr->serdes_num; index++)
            {
                if(serdes_id == p_ds_attr->s[index].serdes_id)
                {
                    find_flag = TRUE;
                    break;
                }
            }
            if(find_flag)
            {
                find_flag = FALSE;
            }
            else
            {
                continue;
            }

            if(0 == p_ds_attr->s[index].ovclk_flag) //no ovclk
            {
                src_ovclk = 0;
                dst_ovclk = 0;
            }
            else if(1 == p_ds_attr->s[index].ovclk_flag) //disable to XFI 12.5G
            {
                src_ovclk = 0;
                dst_ovclk = 1;
            }
            else if(3 == p_ds_attr->s[index].ovclk_flag) //disable to XFI 11.40625G
            {
                src_ovclk = 0;
                dst_ovclk = 2;
            }
            else if(4 == p_ds_attr->s[index].ovclk_flag) //disable to XXVG 28.125G
            {
                src_ovclk = 0;
                dst_ovclk = 3;
            }
            else if(5 == p_ds_attr->s[index].ovclk_flag) //disable to XFI 12.96875G
            {
                src_ovclk = 0;
                dst_ovclk = 4;
            }
            else //any overclock speed to disable
            {
                if(SYS_DATAPATH_IS_SERDES_10G_PER_LANE(p_ds_attr->s[index].dst_mode) || 
                   SYS_DATAPATH_IS_SERDES_25G_PER_LANE(p_ds_attr->s[index].dst_mode))
                {
                    src_ovclk = 1; //not always 12.5G but not important
                }
                dst_ovclk = 0;
            }

            //just operate the first found serdes in ds_attr, because each quad serdes pll is same
            SYS_TSINGMA_28G_GET_CMU_TYPE(p_ds_attr->s[index].src_mode, src_ovclk, src_cmu_type);
            SYS_TSINGMA_28G_GET_CMU_TYPE(p_ds_attr->s[index].dst_mode, dst_ovclk, dst_cmu_type);
            if(src_cmu_type == dst_cmu_type)
            {
                continue;
            }
            /*if((CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[index].src_mode) && (SYS_DATAPATH_CMU_IDLE != p_hss_vec->plla_mode) && 
               (dst_cmu_type != p_hss_vec->plla_mode) && (dst_cmu_type != SYS_DATAPATH_CMU_IDLE))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "PLL resource is not available! hss_idx %u, lane_id %u\n", hss_idx, lane_id);
                return CTC_E_INVALID_PARAM;
            }*/
            p_hss_pll->serdes_pll_sel[lane_id].src_mode = p_ds_attr->s[index].src_mode;
            p_hss_pll->serdes_pll_sel[lane_id].dst_mode = p_ds_attr->s[index].dst_mode;
            p_hss_pll->serdes_pll_sel[lane_id].ovclk_flag = p_ds_attr->s[index].ovclk_flag;
            p_hss_pll->serdes_pll_sel[lane_id].chg_flag = TRUE;
            if((1 == p_ds_attr->s[index].ovclk_flag) || (3 == p_ds_attr->s[index].ovclk_flag) || 
               (4 == p_ds_attr->s[index].ovclk_flag) || (5 == p_ds_attr->s[index].ovclk_flag))
            {
                if((CTC_CHIP_SERDES_XXVG_MODE == p_ds_attr->s[index].dst_mode) || 
                   (CTC_CHIP_SERDES_LG_MODE == p_ds_attr->s[index].dst_mode) || 
                   (CTC_CHIP_SERDES_CG_MODE == p_ds_attr->s[index].dst_mode))
                {
                    p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = CTC_CHIP_SERDES_OCS_MODE_27_27G;
                }
                else
                {
                    if(1 == p_ds_attr->s[index].ovclk_flag)
                    {
                        p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = CTC_CHIP_SERDES_OCS_MODE_12_12G;
                    }
                    else if(3 == p_ds_attr->s[index].ovclk_flag)
                    {
                        p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = CTC_CHIP_SERDES_OCS_MODE_11_06G;
                    }
                    else
                    {
                        p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = CTC_CHIP_SERDES_OCS_MODE_12_58G;
                    }
                }
            }
            else if(2 == p_ds_attr->s[index].ovclk_flag)
            {
                p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = 0xffff;//enable to disable
            }
            else
            {
                p_hss_pll->serdes_pll_sel[lane_id].dst_ovclk = SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_NONE_MODE;
            }

            if((CTC_CHIP_SERDES_NONE_MODE != p_ds_attr->s[index].dst_mode))
            {
                if(CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[index].src_mode)
                {
                    p_hss_pll->plla_ref_cnt++;
                }
                else
                {
                    if(!p_hss_pll->pll_chg_flag)
                    {
                        p_hss_pll->plla_ref_cnt = 1;
                    }
                    else
                    {
                        p_hss_pll->plla_ref_cnt++;
                    }
                }
                p_hss_pll->plla_mode = dst_cmu_type;
                
                if(!p_hss_pll->pll_chg_flag)
                {
                    p_hss_pll->pll_chg_flag = 1;
                }
            }
            if((CTC_CHIP_SERDES_NONE_MODE != p_ds_attr->s[index].src_mode) && 
               (CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[index].dst_mode))
            {
                if(!p_hss_pll->pll_chg_flag)
                {
                    p_hss_pll->pll_chg_flag = 1;
                }
                p_hss_pll->plla_ref_cnt--;
            }
        }
    }
    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_hss12g_set_reset(uint8 lchip, uint8 enable, uint8 lane_rst_flag[],
                                                              uint8 cmu_flag[HSS12G_NUM_PER_SLICE+TSINGMA_HSS28G_NUM_PER_SLICE][3])
{
    uint8  hss_id;
    uint8  cmu_id    =  0;
    uint8  serdes_id =  0;


    /*set serdes lane reset*/
    for(serdes_id = 0; serdes_id < SYS_DATAPATH_SERDES_28G_START_ID; serdes_id++)
    {
        if(lane_rst_flag[serdes_id])
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_lane_reset(lchip, serdes_id, enable));
        }
    }

    /*set cmu reset*/
    for(hss_id = 0; hss_id < HSS12G_NUM_PER_SLICE; hss_id++)
    {
        for(cmu_id = 0; cmu_id < 3; cmu_id++)
        {
            if(1 != cmu_flag[hss_id][cmu_id])
            {
                continue;
            }
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_cmu_reset(lchip, hss_id, cmu_id, enable));
        }
    }


    return CTC_E_NONE;
}

/*judge condition: none/none/none/none/ -> any*/
int32
_sys_tsingma_datapath_dynamic_hss28g_judge_all_none(uint8 lchip, uint8* p_all_none, 
                                                                     sys_datapath_dynamic_switch_attr_t* p_ds_attr)
{
    uint8  idx;
    uint8  lane;
    uint8  hss_id;
    uint8  tmp_serdes_id;
    uint8  switch_flag = FALSE;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    *p_all_none = FALSE;

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[0].serdes_id))
    {
        return CTC_E_NONE;
    }
    if((CTC_CHIP_SERDES_NONE_MODE != p_ds_attr->s[0].src_mode) || 
       (CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[0].dst_mode))
    {
        return CTC_E_NONE;
    }

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(p_ds_attr->s[0].serdes_id);

    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, (hss_id+HSS12G_NUM_PER_SLICE));
    for(lane = 0; lane < 4; lane++)
    {
        tmp_serdes_id = (0 == hss_id) ? (24 + lane) : (28 + lane);
        switch_flag = FALSE;
        for(idx = 0; idx < p_ds_attr->serdes_num; idx++)
        {
            if(tmp_serdes_id == p_ds_attr->s[idx].serdes_id)
            {
                switch_flag = TRUE;
                break;
            }
        }
        if(switch_flag)
        {
            continue;
        }

        if((CTC_CHIP_SERDES_NONE_MODE != p_hss_vec->serdes_info[lane].mode) && 
           (CTC_CHIP_SERDES_XSGMII_MODE != p_hss_vec->serdes_info[lane].mode))
        {
            *p_all_none = FALSE;
            return CTC_E_NONE;
        }
    }

    *p_all_none = TRUE;
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_hss28g_reset(uint8 lchip, sys_datapath_dynamic_switch_attr_t* p_ds_attr, 
                                                         sys_datapath_hss_pll_info_t* p_dst_pll, uint8 rst_flag_28g[])
{
    uint8  idx;
    uint8  lane;
    uint8  hss_id = 0;
    uint8  rst_flag = TRUE;
    uint8  all_none;
    
    _sys_tsingma_datapath_dynamic_hss28g_judge_all_none(lchip, &all_none, p_ds_attr);

    /*1. record hss to be reset  XXVG-LG not need reset*/
    for(idx = 0; idx < p_ds_attr->serdes_num; idx++)
    {
        if(SYS_DATAPATH_SERDES_28G_START_ID > p_ds_attr->s[idx].serdes_id)
        {
            continue;
        }
        if((SYS_DATAPATH_IS_SERDES_25G_PER_LANE(p_ds_attr->s[idx].src_mode) && 
            SYS_DATAPATH_IS_SERDES_25G_PER_LANE(p_ds_attr->s[idx].dst_mode) && 
            (0 == p_ds_attr->s[idx].ovclk_flag)) || 
           (SYS_DATAPATH_IS_SERDES_10G_PER_LANE(p_ds_attr->s[idx].src_mode) && 
            SYS_DATAPATH_IS_SERDES_10G_PER_LANE(p_ds_attr->s[idx].dst_mode) && 
            (0 == p_ds_attr->s[idx].ovclk_flag)) ||
           ((CTC_CHIP_SERDES_2DOT5G_MODE == p_ds_attr->s[idx].src_mode) && 
            (CTC_CHIP_SERDES_XAUI_MODE == p_ds_attr->s[idx].dst_mode)) || 
           ((CTC_CHIP_SERDES_XAUI_MODE == p_ds_attr->s[idx].src_mode) && 
            (CTC_CHIP_SERDES_2DOT5G_MODE == p_ds_attr->s[idx].dst_mode)) ||
           (CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[idx].dst_mode))
        {
            continue;
        }

        hss_id = (p_ds_attr->s[idx].serdes_id - SYS_DATAPATH_SERDES_28G_START_ID) / HSS28G_LANE_NUM;
        if((CTC_CHIP_SERDES_NONE_MODE == p_ds_attr->s[idx].src_mode) && 
           (CTC_CHIP_SERDES_NONE_MODE != p_ds_attr->s[idx].dst_mode))
        {
            if(all_none)
            {
                rst_flag = TRUE;
            }
            else
            {
                rst_flag = FALSE;
                for(lane = 0; lane < 4; lane++)
                {
                    if(p_dst_pll[hss_id+HSS12G_NUM_PER_SLICE].serdes_pll_sel[lane].chg_flag)
                    {
                        if(lane == (p_ds_attr->s[idx].serdes_id % 4))
                        {
                            continue;
                        }
                        rst_flag = TRUE;
                        break;
                    }
                }
            }
        }
        if((FALSE == rst_flag_28g[hss_id]) && (TRUE == rst_flag))
        {
            rst_flag_28g[hss_id] = TRUE;
        }
    }

    /*2. do ALL_RESET*/
    for(idx = 0; idx < 2; idx++)
    {
        if(rst_flag_28g[idx])
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_set_domain_reset(lchip, idx, SYS_TM_28G_ALL_RESET));
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_set_serdes_polarity_cfg(uint8 lchip, uint8 serdes_id, uint8 serdes_num)
{
    uint32 i = 0;
    uint8  lane_id   = 0;
    uint8  hss_idx   = 0;
    ctc_chip_serdes_polarity_t serdes_polarity;
    sys_datapath_hss_attribute_t*      p_hss_vec = NULL;

    for (i = 0; i < serdes_num; i++)
    {
        serdes_polarity.serdes_id = serdes_id + i;
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_polarity.serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_polarity.serdes_id);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;
        }
    
        /*set RX polarity*/
        serdes_polarity.dir = 0;
        serdes_polarity.polarity_mode= (p_hss_vec->serdes_info[lane_id].rx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &serdes_polarity));
        sal_task_sleep(1);

        /*set TX polarity*/
        serdes_polarity.dir = 1;
        serdes_polarity.polarity_mode = (p_hss_vec->serdes_info[lane_id].tx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &serdes_polarity));
        sal_task_sleep(1);
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_12g_dfe_lane_config(uint8 lchip, uint8 serdes_id, uint8 mode, uint8 init_flag)
{
    uint8  addr    = 0xff;
    uint8  mask    = 0xff;
    uint8  val_reg = 0;
    uint8  hss_id  = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    uint8  lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    uint8  acc_id  = lane_id+3;
    uint8  enable  = ((CTC_CHIP_SERDES_XFI_MODE == mode) || (CTC_CHIP_SERDES_XLG_MODE == mode)) ? TRUE : FALSE;
    uint32 tb_id   = 0;
    uint32 cmd     = 0;
    uint32 value   = 0;
    uint32 fld_id  = Hss12GGlbCfg0_cfgHssPmaPowerCtrl0_f;
    Hss12GGlbCfg0_m glb_cfg;

    if(init_flag)
    {
        /*1. read value of Hss12GGlbCfg0.cfgHssPmaPowerCtrl0*/
        tb_id = Hss12GGlbCfg0_t + hss_id;
        cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &glb_cfg));
        DRV_IOR_FIELD(lchip, tb_id, fld_id, &value, &glb_cfg);

        /*2. write related regs*/
        /*cfgHssPmaPowerCtrl[0]   0x13[2]   cfg_cdrck_en    */
        val_reg = (uint8)(value & 0x1);
        addr = 0x13;
        mask = 0xfb;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[1]   0x23[2]   cfg_dfeck_en    */
        val_reg = (uint8)((value>>1) & 0x1);
        addr = 0x23;
        mask = 0xfb;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[2]   0x23[0]   cfg_dfe_pd      */
        val_reg = (uint8)((value>>2) & 0x1);
        addr = 0x23;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[3]   NULL      cfg_dfedmx_pd   */
        
        /*cfgHssPmaPowerCtrl[8:4] 0x22[4:0] cfg_dfetap_en   */
        val_reg = (uint8)((value>>4) & 0x1f);
        addr = 0x22;
        mask = 0xe0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[9]   NULL      cfg_dmux_pd     */
        
        /*cfgHssPmaPowerCtrl[10]  NULL      cfg_dmux_clk_pd */
        
        /*cfgHssPmaPowerCtrl[11]  0x23[3]   cfg_erramp_pd   */
        val_reg = (uint8)((value>>11) & 0x1);
        addr = 0x23;
        mask = 0xf7;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[12]  0x1a[2]   cfg_pi_dfe_en   */
        val_reg = (uint8)((value>>12) & 0x1);
        addr = 0x1a;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[13]  0x1a[1]   cfg_pi_en       */
        val_reg = (uint8)((value>>13) & 0x1);
        addr = 0x1a;
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[14]  0x0b[4]    cfg_pd_ctle     */
        val_reg = (uint8)((value>>14) & 0x1);
        addr = 0x0b;
        mask = 0xef;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[15]  0x30[0]   cfg_summer_en   */
        val_reg = (uint8)((value>>15) & 0x1);
        addr = 0x30;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[16]  0x37[1]   cfg_pd_rx_cktree*/
        val_reg = (uint8)((value>>16) & 0x1);
        addr = 0x37;
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[17]  0x06[1]    cfg_pd_clk      */
        val_reg = (uint8)((value>>17) & 0x1);
        addr = 0x06;
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[18]  0x06[2]    cfg_pd_cml      */
        val_reg = (uint8)((value>>18) & 0x1);
        addr = 0x06;
        mask = 0xfb;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[19]  0x06[0]    cfg_pd_driver   */
        val_reg = (uint8)((value>>19) & 0x1);
        addr = 0x06;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[20]  0x07[1]    cfg_rx_reg_pu   */
        val_reg = (uint8)((value>>20) & 0x1);
        addr = 0x07;
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[21]  NULL      cfg_pd_rms_det  */
        
        /*cfgHssPmaPowerCtrl[22]  0x13[0]   cfg_dcdr_pd     */
        val_reg = (uint8)((value>>22) & 0x1);
        addr = 0x13;
        mask = 0xfe;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*cfgHssPmaPowerCtrl[23]  NULL      cfg_ecdr_pd     */
        
        /*cfgHssPmaPowerCtrl[24]  0x34[1]   cfg_pd_sq       */
        val_reg = (uint8)((value>>24) & 0x1);
        addr = 0x34;
        mask = 0xfd;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*pcs2pma_r50_en          0x31[5]   cfg_r50_en      */
        val_reg = 1;
        addr = 0x31;
        mask = 0xdf;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
        /*pcs2pma_eid_lp          0x36[4]   cfg_eid_lp      */
        val_reg = 1;
        addr = 0x36;
        mask = 0xef;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));

        /*3. switch to reg mode*/
        /*0x93[4] r_reg_manual write 1*/
        val_reg = 1;
        addr = 0x93;
        mask = 0xef;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, acc_id, addr, mask, val_reg));
    }

    /*3. config dfe enable*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_set_dfe_en(lchip, serdes_id, enable));

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_dynamic_switch_serdes_cfg(uint8 lchip, sys_datapath_hss_pll_info_t* p_dst_pll, sys_datapath_dynamic_switch_attr_t* p_ds_attr)
{
    uint8  hss_idx    = 0;
    uint8  hss_id     = 0;
    uint8  lane_id    = 0;
    uint8  idx;
    uint8  pll_ref_cnt = 0;
    uint8  dst_mode_28g = 0;
    uint8  dst_oc_28g   = 0;
    uint16 tmp_lport  = 0;
    uint32 serdes_id;
    uint8  all_none;
    uint8  lane_rst_flag[TSINGMA_SERDES_NUM_PER_SLICE]; /*(per lane) lane reset flag, 12G only*/
    uint8  serdes_mode[TSINGMA_SERDES_NUM_PER_SLICE]; /*(per lane) destination mode*/
    uint8  cmu_id[TSINGMA_SERDES_NUM_PER_SLICE];      /*(per lane) tell lane which cmu to choose*/
    uint8  ovclk_flag[TSINGMA_SERDES_NUM_PER_SLICE];  /*(per lane) 0-no change  1-(Any)XFI 12.5G  2-(Any)disable  3-(28G)XFI 11.40625G  4-(28G)XXVG 28.125G*/
    uint8  cmu_flag[HSS12G_NUM_PER_SLICE+TSINGMA_HSS28G_NUM_PER_SLICE][3];  /*(per hss/cmu) mark if this cmu need reset & reconfig*/
    uint8  rst_flag_28g[2] = {0};  /*(per macro) HSS28G reset flag*/
    ctc_chip_serdes_ffe_t serdes_ffe[4] = {{0}};      /*save 28G FFE before serdes reconfig, then recover*/
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    sal_memset(lane_rst_flag, 0, TSINGMA_SERDES_NUM_PER_SLICE * sizeof(uint8));
    sal_memset(serdes_mode, 0, TSINGMA_SERDES_NUM_PER_SLICE * sizeof(uint8));
    sal_memset(cmu_id, 0xff, TSINGMA_SERDES_NUM_PER_SLICE * sizeof(uint8));
    sal_memset(cmu_flag, 0, (HSS12G_NUM_PER_SLICE+TSINGMA_HSS28G_NUM_PER_SLICE) * 3 * sizeof(uint8));
    sal_memset(ovclk_flag, 0, TSINGMA_SERDES_NUM_PER_SLICE * sizeof(uint8));

    /*0. Assemble array serdes_mode and cmu_id*/
    for(serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);

        /*for any->none, do not change any config, just need soft table change*/
        if(CTC_CHIP_SERDES_NONE_MODE == p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode)
        {
            continue;
        }

        /*for 12G serdes lane speed change without cmu changes*/
        /*10G->QSGMII, these 2 modes cmu cfg are the same, but lane cfg need change*/
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            if((!HSS12G_MODE_IS_32BIT_FULLRATE(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].src_mode)) || 
               (!HSS12G_MODE_IS_32BIT_FULLRATE(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode)))
            {
                serdes_mode[serdes_id] = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode;
            }
            if(0 != p_dst_pll[hss_idx].serdes_pll_sel[lane_id].ovclk_flag) //GUC serdes 10G overclock to 12.5G
            {
                serdes_mode[serdes_id] = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode;
                ovclk_flag[serdes_id]  = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].ovclk_flag;
            }
        }

        if(0 == p_dst_pll[hss_idx].pll_chg_flag)
        {
            continue;
        }
        /*per lane info*/
        if(FALSE == p_dst_pll[hss_idx].serdes_pll_sel[lane_id].chg_flag)
        {
            continue;
        }
        if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)) /*for 28G serdes, lane speed change will certainly cause cmu/pll change*/
        {
            serdes_mode[serdes_id] = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode;
            dst_mode_28g = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode;
            dst_oc_28g   = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].ovclk_flag;
        }
        cmu_id[serdes_id]     = SYS_TSINGMA_PLLSEL_TO_CMUID(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].pll_sel);
        ovclk_flag[serdes_id]  = p_dst_pll[hss_idx].serdes_pll_sel[lane_id].ovclk_flag;

        /*per HSS info*/
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            if(CTC_CHIP_SERDES_NONE_MODE != serdes_mode[serdes_id])
            {
                lane_rst_flag[serdes_id] = TRUE;
            }
            else
            {
                if(HSS12G_MODE_IS_32BIT_FULLRATE(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].src_mode) && 
                   HSS12G_MODE_IS_32BIT_FULLRATE(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].dst_mode))
                {
                    lane_rst_flag[serdes_id] = TRUE;
                }
            }

            switch(cmu_id[serdes_id])
            {
                case 0:
                    pll_ref_cnt = p_dst_pll[hss_idx].plla_ref_cnt;
                    break;
                case 1:
                    pll_ref_cnt = p_dst_pll[hss_idx].pllb_ref_cnt;
                    break;
                case 2:
                    pll_ref_cnt = p_dst_pll[hss_idx].pllc_ref_cnt;
                    break;
            }
            if(SYS_DATAPATH_IS_SERDES_4_PORT_1(p_dst_pll[hss_idx].serdes_pll_sel[lane_id].src_mode) || 
                SYS_DATAPATH_IS_SERDES_4_PORT_1(serdes_mode[serdes_id]))
            {
                if(4 < pll_ref_cnt)
                {
                    continue;
                }
            }
            else if((CTC_CHIP_SERDES_NONE_MODE != serdes_mode[serdes_id]) && (1 < pll_ref_cnt))
            {
                continue;
            }
            else if((CTC_CHIP_SERDES_NONE_MODE != p_dst_pll[hss_idx].serdes_pll_sel[lane_id].src_mode) && 
                    (CTC_CHIP_SERDES_NONE_MODE == serdes_mode[serdes_id]))
            {
                continue;
            }
        }
        else
        {
            if((CTC_CHIP_SERDES_NONE_MODE != p_dst_pll[hss_idx].serdes_pll_sel[lane_id].src_mode) && 
               (CTC_CHIP_SERDES_NONE_MODE == serdes_mode[serdes_id]))
            {
                continue;
            }
        }
        if(0 == cmu_flag[hss_idx][cmu_id[serdes_id]])
        {
            cmu_flag[hss_idx][cmu_id[serdes_id]] = p_dst_pll[hss_idx].pll_chg_flag;
        }
    }

    /*save 28G FFE manual value before reconfig*/
    for(idx = 0; idx < p_ds_attr->serdes_num; idx++)
    {
        serdes_id = p_ds_attr->s[idx].serdes_id;
        if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
        {
            continue;
        }
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        serdes_ffe[lane_id].serdes_id = serdes_id;
        serdes_ffe[lane_id].mode      = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
        sys_tsingma_datapath_get_serdes_ffe_traditional(lchip, serdes_id, serdes_ffe[lane_id].coefficient);
    }

    /*1. Deactive serdes lane & CMU reset*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_hss12g_set_reset(lchip, FALSE, lane_rst_flag, cmu_flag));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_hss28g_reset(lchip, p_ds_attr, p_dst_pll, rst_flag_28g));
    
    /*2. Reconfig CMU    12G only, 28G merge into additional cfg*/
    for (hss_idx = 0; hss_idx < TSINGMA_HSS15G_NUM_PER_SLICE; hss_idx++)
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        hss_id = hss_idx;
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_hss12g_cmu_cfg(lchip, hss_id, serdes_mode, cmu_id, ovclk_flag, cmu_flag[hss_idx]));
    }
    /*3. Reconfig serdes lane*/
    for(idx = 0; idx < p_ds_attr->serdes_num; idx++)
    {
        serdes_id = p_ds_attr->s[idx].serdes_id;
        if((0 != serdes_mode[serdes_id]) || (0 != ovclk_flag[serdes_id]) || 
           ((!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id)) && 
            (HSS12G_JUDGE_MULTI_LANE_CHANGE(p_ds_attr->s[idx].src_mode, p_ds_attr->s[idx].dst_mode) ||
             HSS12G_JUDGE_USXGMII_TXSEL_CHANGE(p_ds_attr->s[idx].src_mode, p_ds_attr->s[idx].dst_mode, serdes_id))))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_lane_cfg(lchip, serdes_id, p_ds_attr->s[idx].dst_mode, cmu_id[serdes_id], ovclk_flag[serdes_id]));
        }
    }

    /*4. Reconfig 28g clktree */
    for (idx = 0; idx < p_ds_attr->mac_num; idx++)
    {
        tmp_lport = sys_usw_datapath_get_lport_with_mac(lchip, 0, p_ds_attr->m[idx].mac_id);
        CTC_ERROR_RETURN(_sys_tsingma_datapath_28g_clk_tree_cfg(lchip, tmp_lport));
    }

    /*5. Additional cfg*/
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[0].serdes_id))
    {
        /* dynamic switch range will not exceed one hss macro */
        hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(p_ds_attr->s[0].serdes_id);
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_additional_cmu_cfg(lchip, hss_id, serdes_mode, cmu_id, ovclk_flag, cmu_flag[hss_id]));
    }
    for(idx = 0; idx < p_ds_attr->serdes_num; idx++)
    {
        serdes_id = p_ds_attr->s[idx].serdes_id;
        if(((0 != serdes_mode[serdes_id]) || (0 != ovclk_flag[serdes_id])) && (SYS_DATAPATH_SERDES_28G_START_ID > serdes_id))
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_additional_lane_cfg(lchip, serdes_id, serdes_mode[serdes_id], ovclk_flag[serdes_id]));
            CTC_ERROR_RETURN(sys_tsingma_datapath_12g_dfe_lane_config(lchip, serdes_id, serdes_mode[serdes_id], FALSE));
        }
    }

    _sys_tsingma_datapath_dynamic_hss28g_judge_all_none(lchip, &all_none, p_ds_attr);
    if((1 != p_ds_attr->serdes_num) || 
       ((CTC_CHIP_SERDES_NONE_MODE != p_ds_attr->s[0].src_mode) && (CTC_CHIP_SERDES_XSGMII_MODE != p_ds_attr->s[0].src_mode)) ||
       (TRUE == all_none))
    {
        for(idx = 0; idx < TSINGMA_HSS28G_NUM_PER_SLICE; idx++)
        {
            if(rst_flag_28g[idx])
            {
                _sys_tsingma_datapath_28g_additional_cfg(lchip, idx, dst_mode_28g, dst_oc_28g);
            }
            _sys_tsingma_datapath_set_serdes_polarity_cfg(lchip, ((0 == idx) ? 24 : 28), 4);
        }
    }

    /*6. Release serdes lane & CMU reset*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_hss12g_set_reset(lchip, TRUE, lane_rst_flag, cmu_flag));

    /*7. Check lane bring-up ok*/
    if (!SYS_DATAPATH_SERDES_IS_HSS28G(p_ds_attr->s[0].serdes_id))
    {
        /* dynamic switch range will not exceed one hss macro */
        hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(p_ds_attr->s[0].serdes_id);
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_wait_lane_ok(lchip, hss_id, 0xff));
    }

    /*recover 28G FFE manual value after reconfig*/
    for(lane_id = 0; lane_id < 4; lane_id++)
    {
        if(0 == serdes_ffe[lane_id].serdes_id)
        {
            continue;
        }
        sys_tsingma_datapath_set_serdes_ffe_traditional(lchip, &serdes_ffe[lane_id]);
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_get_lane_swap_flag(uint8 lchip, uint8* lane_swap_0, uint8* lane_swap_1)
{
    uint8  hss_idx   = 0;
    uint8  group_l0  = 0;
    uint8  group_l1  = 0;
    uint8  group_l2  = 0;
    uint8  group_l3  = 0;
    uint8  lane_swap = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    for(hss_idx = TSINGMA_HSS15G_NUM_PER_SLICE; hss_idx < TSINGMA_SYS_DATAPATH_HSS_NUM; hss_idx++)
    {
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            continue;
        }

        group_l0 = p_hss_vec->serdes_info[0].group;
        group_l1 = p_hss_vec->serdes_info[1].group;
        group_l2 = p_hss_vec->serdes_info[2].group;
        group_l3 = p_hss_vec->serdes_info[3].group;

        if((group_l1 == group_l2) && (group_l0 == group_l3) && (group_l0 != group_l1))
        {
            lane_swap = TRUE;
        }
        else
        {
            lane_swap = FALSE;
        }

        switch(hss_idx)
        {
            case 3:
                SYS_USW_VALID_PTR_WRITE(lane_swap_0, lane_swap);
                break;
            case 4:
                SYS_USW_VALID_PTR_WRITE(lane_swap_1, lane_swap);
                break;
            default:
                break;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_switch_sort_target(uint8 lchip, sys_datapath_dynamic_switch_attr_t *target)
{
    uint8 i, j;
    uint8 lane_swap_0 = FALSE;
    uint8 lane_swap_1 = FALSE;
    sys_datapath_dynamic_switch_mac_t m_buf = {0};
    sys_datapath_dynamic_switch_serdes_t s_buf = {0};

    /*mac id sort*/
    for(i = 0; i < target->mac_num; i++)
    {
        for(j = i+1; j < target->mac_num; j++)
        {
            if(target->m[j].mac_id < target->m[i].mac_id)
            {
                /*exchange*/
                sal_memcpy(&m_buf, &(target->m[j]), sizeof(sys_datapath_dynamic_switch_mac_t));
                sal_memcpy(&(target->m[j]), &(target->m[i]), sizeof(sys_datapath_dynamic_switch_mac_t));
                sal_memcpy(&(target->m[i]), &m_buf, sizeof(sys_datapath_dynamic_switch_mac_t));
            }
        }
    }
    
    /*serdes id sort*/
    for(i = 0; i < target->serdes_num; i++)
    {
        for(j = i+1; j < target->serdes_num; j++)
        {
            if(target->s[j].serdes_id < target->s[i].serdes_id)
            {
                /*exchange*/
                sal_memcpy(&s_buf, &(target->s[j]), sizeof(sys_datapath_dynamic_switch_mac_t));
                sal_memcpy(&(target->s[j]), &(target->s[i]), sizeof(sys_datapath_dynamic_switch_mac_t));
                sal_memcpy(&(target->s[i]), &s_buf, sizeof(sys_datapath_dynamic_switch_mac_t));
            }
        }
    }

    /*for 28G swap change, mac 0 and 2 should exchange order (60 61 62 63 -> 62 61 60 63)*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, &lane_swap_0, &lane_swap_1));
    if((4 == target->serdes_num) && (4 == target->mac_num) && 
       (SYS_DATAPATH_SERDES_IS_HSS28G(target->s[0].serdes_id)) && 
       (((27 >= target->s[0].serdes_id) && lane_swap_0) || ((27 < target->s[0].serdes_id) && lane_swap_1)) &&
       (HSS28G_JUDGE_SWAP_CHANGE(target->s[0].src_mode, target->s[0].dst_mode) || 
        HSS28G_JUDGE_SWAP_CHANGE(target->s[0].dst_mode, target->s[0].src_mode)))
    {
        sal_memcpy(&m_buf, &(target->m[0]), sizeof(sys_datapath_dynamic_switch_mac_t));
        sal_memcpy(&(target->m[0]), &(target->m[2]), sizeof(sys_datapath_dynamic_switch_mac_t));
        sal_memcpy(&(target->m[2]), &m_buf, sizeof(sys_datapath_dynamic_switch_mac_t));
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_lane_swap_config(uint8 lchip, uint8 hss_id_28g, uint8 swap_flag)
{
    uint32 tbl_id      = 0;
    uint32 fld_id      = 0;
    uint32 cmd         = 0;
    uint32 tmp_val32   = swap_flag ? 1 : 0;
    SharedPcsLgCfg0_m       pcs_lg_cfg;
    Hss28GClkTreeCfg_m      lane_swap_cfg;
    GlobalCtlSharedFec0_m   sharedfec_cfg;

    tbl_id = SharedPcsLgCfg0_t + hss_id_28g + 6;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_lg_cfg));
    fld_id = SharedPcsLgCfg0_lgPcsLane0And3SwapEn_f;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &pcs_lg_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcs_lg_cfg));

    tbl_id = GlobalCtlSharedFec0_t + hss_id_28g;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    fld_id = GlobalCtlSharedFec0_cfgSharedFecLane0And3SwapEn_f;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &sharedfec_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharedfec_cfg));
    
    tbl_id = Hss28GClkTreeCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lane_swap_cfg));
    fld_id = hss_id_28g ? Hss28GClkTreeCfg_cfgHssLane4And6SwapEn_f : Hss28GClkTreeCfg_cfgHssLane0And2SwapEn_f;
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &lane_swap_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lane_swap_cfg));

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_lane_swap_fix(uint8 lchip, uint8 hss_id_28g, uint8 swap_flag, uint8* p_fix_swap)
{
    uint16 lport        = 0;
    uint8  lane_id      = 0;
    uint8  slice_id     = 0;
    uint16 start_lport  = (0 == hss_id_28g) ? 44 : 60;
    uint16 end_lport    = (0 == hss_id_28g) ? 47 : 63;
    sys_datapath_lport_attr_t*    port_attr = NULL;

    SYS_USW_VALID_PTR_WRITE(p_fix_swap, swap_flag);
    if(!swap_flag)
    {
        return CTC_E_NONE;
    }

    /*if txqm3 choose 13 14 15 17, port 60~63 have no swap config*/
    if((1 == hss_id_28g) && (2 == p_usw_datapath_master[lchip]->qm_choice.txqm3))
    {
        return CTC_E_NONE;
    }

    /*port info fix*/
    for(lport = start_lport; lport <= end_lport; lport++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if(NULL == port_attr)
        {
            continue;
        }

        /*the following modes need fix swap to normal mapping*/
        if((CTC_CHIP_SERDES_SGMII_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == port_attr->pcs_mode) ||
           (CTC_CHIP_SERDES_100BASEFX_MODE == port_attr->pcs_mode))
        {
            lane_id = port_attr->serdes_id % 4;
            port_attr->mac_id = lane_id + ((0 == hss_id_28g) ? 44 : 60);
            port_attr->internal_mac_idx = port_attr->mac_id % 4;
            SYS_USW_VALID_PTR_WRITE(p_fix_swap, FALSE);
        }
        else if((CTC_CHIP_SERDES_XAUI_MODE == port_attr->pcs_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == port_attr->pcs_mode))
        {
            SYS_USW_VALID_PTR_WRITE(p_fix_swap, FALSE);
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_dynamic_switch_swap_reconfig(uint8 lchip, sys_datapath_dynamic_switch_attr_t* ds_attr)
{
    uint8 lane_swap_0 = FALSE;
    uint8 lane_swap_1 = FALSE;
    uint8 fix_swap_0  = FALSE;
    uint8 fix_swap_1  = FALSE;
    uint8 i = 0;
    uint8 src_mode    = CTC_CHIP_SERDES_NONE_MODE;
    uint8 dst_mode    = CTC_CHIP_SERDES_NONE_MODE;
    uint8 sgmac_idx   = 0;
    uint8 mii_idx     = 0;
    uint32 tbl_id     = 0;
    uint32 cmd        = 0;
    uint32 value      = 0;
    uint8  hss_idx    = 0;
    uint8  lg_flag    = FALSE;
    SharedMiiCfg0_m   mii_cfg;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(ds_attr->s[0].serdes_id))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, &lane_swap_0, &lane_swap_1));

    /*1. swap reconfig*/
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, 3);
    if(NULL != p_hss_vec)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_fix(lchip, 0, lane_swap_0, &fix_swap_0));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_config(lchip, 0, fix_swap_0));
    }
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, 4);
    if(NULL != p_hss_vec)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_fix(lchip, 1, lane_swap_1, &fix_swap_1));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_config(lchip, 1, fix_swap_1));
    }

    /*2. for mode support swap -> mode not support, and mode is 4-lane after switch, 
         exchange m[0] and m[2] mac id to do post proc*/

    /*2.1  get pre and post switch mode*/
    for(i = 0; i < ds_attr->serdes_num; i++)
    {
        if((CTC_CHIP_SERDES_NONE_MODE != ds_attr->s[i].src_mode) && 
           (CTC_CHIP_SERDES_XSGMII_MODE != ds_attr->s[i].src_mode) && 
           (CTC_CHIP_SERDES_NONE_MODE == src_mode))
        {
            src_mode = ds_attr->s[i].src_mode;
        }
        if((CTC_CHIP_SERDES_NONE_MODE != ds_attr->s[i].dst_mode) && 
           (CTC_CHIP_SERDES_XSGMII_MODE != ds_attr->s[i].dst_mode) && 
           (CTC_CHIP_SERDES_NONE_MODE == dst_mode))
        {
            dst_mode = ds_attr->s[i].dst_mode;
        }
    }

    /*2.2  do exchange in specified situation*/
    if((HSS28G_MODE_IS_SUPPORT_SWAP(src_mode) || 
        (CTC_CHIP_SERDES_NONE_MODE == src_mode) || (CTC_CHIP_SERDES_XSGMII_MODE == src_mode) || 
        (CTC_CHIP_SERDES_XAUI_MODE == src_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == src_mode)) && 
       ((CTC_CHIP_SERDES_XAUI_MODE == dst_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == dst_mode)))
    {/*MODE_IS_SUPPORT_SWAP or NONE or XAUI/DXAUI -> XAUI/DXAUI, no exchange*/}
    else
    {
        if(lane_swap_0 && (!fix_swap_0) && (4 != ds_attr->mac_num))
        {
            for(i = 0; i < ds_attr->mac_num; i++)
            {
                ds_attr->m[i].mac_id = (44 == ds_attr->m[i].mac_id) ? (46) : ((46 == ds_attr->m[i].mac_id) ? 44 : ds_attr->m[i].mac_id);
            }
        }

        if(lane_swap_1 && (!fix_swap_1) && (4 != ds_attr->mac_num))
        {
            for(i = 0; i < ds_attr->mac_num; i++)
            {
                ds_attr->m[i].mac_id = (60 == ds_attr->m[i].mac_id) ? (62) : ((62 == ds_attr->m[i].mac_id) ? 60 : ds_attr->m[i].mac_id);
            }
        }
    }

    /*3. clear Tx/Rx BufMode value*/
    /*XLG/CG/XAUI/DXAUI->!(4-1 mode)*/
    /*2*25G+1*50G->4*25G*/
    sgmac_idx = (ds_attr->s[0].serdes_id < 28) ? 12 : 16;
    mii_idx   = (ds_attr->s[0].serdes_id < 28) ? 15 : 16;
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(ds_attr->s[0].serdes_id);
    p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    for(i = 0; i < 4; i++)
    {
        if(CTC_CHIP_SERDES_LG_MODE == p_hss_vec->serdes_info[i].mode)
        {
            lg_flag = TRUE;
            break;
        }
    }

    if(((4 == ds_attr->serdes_num) && SYS_DATAPATH_IS_SERDES_4_PORT_1(src_mode) && (!SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode))) || 
       ((2 == ds_attr->serdes_num) && (CTC_CHIP_SERDES_LG_MODE == src_mode) && (CTC_CHIP_SERDES_XXVG_MODE == dst_mode) && (FALSE == lg_flag)))
    {
        CTC_ERROR_RETURN(_sys_tsingma_mac_set_quadsgmaccfg(lchip, value, sgmac_idx));

        tbl_id = SharedMiiCfg0_t + mii_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
        DRV_IOW_FIELD(lchip, tbl_id, SharedMiiCfg0_cfgSharedMiiMuxMode_f, &value, &mii_cfg);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mii_cfg));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_dynamic_switch(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    uint32 i = 0;
    uint32 tmp_gport = 0;
    int32  ret       = 0;
    uint16 lport     = 0;
    uint16 tmp_lport = 0;
    uint8  gchip     = 0;
    uint8  hss_idx   = 0;
    uint8  slice_id  = 0;
    uint8  lane_id   = 0;
    uint8  mode      = 0;
    uint32 serdes_id = 0;
    uint8  hss_num   = HSS12G_NUM_PER_SLICE + TSINGMA_HSS28G_NUM_PER_SLICE;
    sys_datapath_serdes_info_t*        p_serdes  = NULL;
    sys_datapath_lport_attr_t*         port_attr = NULL;
    sys_datapath_hss_attribute_t*      p_hss_vec = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr   = {0};
    sys_datapath_hss_pll_info_t dst_pll[hss_num];

    DATAPATH_INIT_CHECK();

    sal_memset(dst_pll, 0, hss_num * sizeof(sys_datapath_hss_pll_info_t));
    sys_usw_get_gchip_id(lchip, &gchip);

    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    if (p_usw_datapath_master[lchip]->glb_xpipe_en)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Dynamic switch is not suppported when X-PIPE enable \n");
        return CTC_E_NOT_SUPPORT;
    }

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

    SYS_CONDITION_RETURN((port_attr->pcs_mode == mode), CTC_E_NONE);

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);

    if(((SYS_DATAPATH_IS_SERDES_1_PORT_1(port_attr->pcs_mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(mode)) || 
         (SYS_DATAPATH_IS_SERDES_1_PORT_1(mode) && SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(port_attr->pcs_mode))) && 
       (0 != (port_attr->serdes_id % 4)))
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_dynamic_switch(lchip, port_attr->serdes_id, mode, 
                         SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_NONE_MODE));
        return CTC_E_NONE;
    }

    /* collect all serdes/mac info associated with dynamic switch process */
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_switch_get_info(lchip, port_attr, mode, &ds_attr, TRUE, 0xff));
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_switch_sort_target(lchip, &ds_attr));

    /* parameter check */
    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        serdes_id = ds_attr.s[i].serdes_id;
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);

        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (p_hss_vec == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;
        }

        p_serdes = &p_hss_vec->serdes_info[lane_id];
        if (0 == (p_serdes->is_dyn))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }
    }
    /* collect serdes pll change info */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_pll_parser(lchip, &ds_attr, dst_pll));

    CTC_ERROR_GOTO(_sys_tsingma_datapath_set_pre_dynamic_switch(lchip, &ds_attr), ret, error);

    CTC_ERROR_GOTO(sys_tsingma_datapath_set_soft_tables(lchip, &ds_attr, dst_pll), ret, error);

    /*serdes config*/
    CTC_ERROR_GOTO(_sys_tsingma_datapath_dynamic_switch_serdes_cfg(lchip, dst_pll, &ds_attr), ret, error);

    /*swap reconfig*/
    CTC_ERROR_GOTO(sys_tsingma_datapath_dynamic_switch_swap_reconfig(lchip, &ds_attr), ret, error);

    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        serdes_id = ds_attr.s[i].serdes_id;
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        CTC_ERROR_GOTO(sys_tsingma_datapath_set_hss28g_firmware_mode(lchip, hss_idx, lane_id), ret, error);
    }

    _sys_tsingma_datapath_set_post_dynamic_switch(lchip, lport, mode, &ds_attr);

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
sys_tsingma_datapath_switch_serdes_get_info(uint8 lchip, sys_datapath_serdes_info_t* p_serdes, sys_datapath_hss_attribute_t* p_hss_vec, 
                                                          uint8 serdes_id, uint8 dst_mode, sys_datapath_dynamic_switch_attr_t* p_ds_attr)
{
    uint8 lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    uint16 tmp_lport = 0;
    uint16 tmp_src_port = 0;
    uint8  src_mode_level = 0;
    uint8  dst_mode_level = 0;
    uint8  lane_swap = 0;
    uint8  tmp_mode = 0;
    uint8  slice_id = (serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /*Only serdes 0~11 can be switched to Q/U*/
    if((11 < serdes_id) && (SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(dst_mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u does not support mode %u. (4)\n", serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(p_serdes->mode, &src_mode_level, NULL, NULL));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_mode_level(dst_mode, &dst_mode_level, NULL, NULL));

    if(SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        if(27 < serdes_id)
        {
            sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, NULL, &lane_swap);
        }
        else
        {
            sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, &lane_swap, NULL);
        }
    }

    if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && 
       (((17 <= serdes_id) && (19 >= serdes_id)) || ((21 <= serdes_id) && (23 >= serdes_id))) && 
       (!SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u cannot switch to mode %u.\n", serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }

    if((((3 == serdes_id) && (0 == p_usw_datapath_master[lchip]->qm_choice.txqm1)) || 
         ((11 == serdes_id) && (1 == p_usw_datapath_master[lchip]->qm_choice.txqm3))) && 
        (3 == src_mode_level) && (0 == dst_mode_level))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u cannot switch to mode %u.\n", serdes_id, dst_mode);
        return CTC_E_INVALID_CONFIG;
    }

    if((3 == src_mode_level) || (src_mode_level == dst_mode_level))
    {
        tmp_lport = p_serdes->lport;
    }
    else if((2 == src_mode_level) || (2 == dst_mode_level)) //LG<->other
    {
        if(1 == src_mode_level)
        {
            if(lane_swap)
            {
                switch(lane_id)
                {
                    case 1:
                        tmp_mode  = p_hss_vec->serdes_info[2].mode;
                        if((CTC_CHIP_SERDES_NONE_MODE != tmp_mode) && (CTC_CHIP_SERDES_XSGMII_MODE != tmp_mode))
                        {
                            tmp_lport = p_hss_vec->serdes_info[2].lport;
                        }
                        else
                        {
                            if(serdes_id > 27)
                            {
                                tmp_lport = 60;
                            }
                            else
                            {
                                tmp_lport = 44;
                            }
                        }
                        break;
                    case 3:
                        tmp_mode  = p_hss_vec->serdes_info[0].mode;
                        if((CTC_CHIP_SERDES_NONE_MODE != tmp_mode) && (CTC_CHIP_SERDES_XSGMII_MODE != tmp_mode))
                        {
                            tmp_lport = p_hss_vec->serdes_info[0].lport;
                        }
                        else
                        {
                            if(serdes_id > 27)
                            {
                                tmp_lport = 62;
                            }
                            else
                            {
                                tmp_lport = 46;
                            }
                        }
                        break;
                    default:
                        tmp_lport = p_hss_vec->serdes_info[lane_id].lport;
                        break;
                }
            }
            else
            {
                tmp_mode = p_hss_vec->serdes_info[lane_id > 1 ? 2 : 0].mode;
                if((CTC_CHIP_SERDES_NONE_MODE != tmp_mode) && (CTC_CHIP_SERDES_XSGMII_MODE != tmp_mode))
                {
                    tmp_lport = p_hss_vec->serdes_info[lane_id > 1 ? 2 : 0].lport;
                }
                else
                {
                    switch(serdes_id)
                    {
                        case 24:
                        case 25:
                            tmp_lport = 44;
                            break;
                        case 26:
                        case 27:
                            tmp_lport = 46;
                            break;
                        case 28:
                        case 29:
                            tmp_lport = 60;
                            break;
                        case 30:
                        case 31:
                        default:
                            tmp_lport = 62;
                            break;
                    }
                }
            }
        }
        else
        {
            tmp_lport = p_hss_vec->serdes_info[lane_id].lport;
            if(3 == dst_mode_level)
            {
                tmp_lport = tmp_lport/4*4;
            }
        }
    }
    else if(((0 == src_mode_level) && (1 == dst_mode_level)) ||
            ((1 == src_mode_level) && (0 == dst_mode_level)))
    {
        if((0 == serdes_id) || (4 == serdes_id) || (8 == serdes_id))
        {
            tmp_lport = p_hss_vec->serdes_info[lane_id].lport;
        }
        else
        {
            if((CTC_CHIP_SERDES_NONE_MODE == p_hss_vec->serdes_info[lane_id/4*4].mode) || 
               (CTC_CHIP_SERDES_XSGMII_MODE == p_hss_vec->serdes_info[lane_id/4*4].mode))
            {
                tmp_src_port = p_usw_datapath_master[lchip]->serdes_mac_map[dst_mode_level][serdes_id];
            }
            else
            {
                tmp_src_port = p_hss_vec->serdes_info[lane_id/4*4].lport + (lane_id%4);
            }
            port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_src_port, slice_id);
            if(port_attr == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            if(SYS_DMPS_RSV_PORT != port_attr->port_type)
            {
                if((0 == src_mode_level) && (1 == dst_mode_level))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Influenced port is in use! \n");
                    return CTC_E_INVALID_CONFIG;
                }
                if((CTC_CHIP_SERDES_USXGMII2_MODE == p_hss_vec->serdes_info[lane_id/4*4].mode) &&
                   (1 == lane_id%4) && (0 == dst_mode_level))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Influenced port not related to current lane! \n");
                    return CTC_E_INVALID_CONFIG;
                }
                else if(((CTC_CHIP_SERDES_USXGMII1_MODE == p_hss_vec->serdes_info[lane_id/4*4].mode) ||
                         (CTC_CHIP_SERDES_QSGMII_MODE == p_hss_vec->serdes_info[lane_id/4*4].mode)) && 
                        (0 == dst_mode_level))
                {
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Influenced port not related to current lane! \n");
                    return CTC_E_INVALID_CONFIG;
                }
                p_ds_attr->m[p_ds_attr->mac_num].mac_id = port_attr->mac_id;
                p_ds_attr->m[p_ds_attr->mac_num].add_drop_flag = 2;
                (p_ds_attr->mac_num)++;
            }
            switch(serdes_id)
            {
                case 1:
                    tmp_lport = 4;
                    break;
                case 2:
                    tmp_lport = 16;
                    break;
                case 3:
                    if(0 == p_usw_datapath_master[lchip]->qm_choice.txqm1)
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " TXQM1 mode is 0, cannot switch lane 3! \n");
                        return CTC_E_INVALID_CONFIG;
                    }
                    tmp_lport = 20;
                    break;
                case 5:
                case 6:
                case 7:
                    tmp_lport = 4 * (serdes_id - 5) + 32;
                    break;
                case 9:
                case 10:
                    tmp_lport = 4 * (serdes_id - 9) + 48;
                    break;
                case 11:
                    if(1 == p_usw_datapath_master[lchip]->qm_choice.txqm3)
                    {
                        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " TXQM3 mode is 1, cannot switch lane 3! \n");
                        return CTC_E_INVALID_CONFIG;
                    }
                    tmp_lport = 56;
                    break;
                default:
                    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mode error! \n");
                    return CTC_E_INVALID_CONFIG;
            }
        }
    }
    else
    {
        if(0 == lane_id % 4)
        {
            tmp_lport = p_hss_vec->serdes_info[lane_id].lport;
            port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            if(lane_swap)
            {
                if(HSS28G_MODE_IS_SUPPORT_SWAP(port_attr->pcs_mode) && 
                   ((CTC_CHIP_SERDES_NONE_MODE == p_hss_vec->serdes_info[2].mode) || 
                    (CTC_CHIP_SERDES_XSGMII_MODE == p_hss_vec->serdes_info[2].mode)))
                {
                    /*xfi|xfi|none|xfi with swap -> xlg, and lane 0 is specified, change to 2 as init serdes id*/
                    sys_tsingma_datapath_switch_serdes_get_none_info(lchip, (serdes_id+2), dst_mode, 
                                                                     &(p_hss_vec->serdes_info[2]), p_ds_attr);
                    return CTC_E_NONE;
                }
                else
                {
                    tmp_lport = p_hss_vec->serdes_info[2].lport;
                }
            }
        }
        else
        {
            if(3 == dst_mode_level)
            {
                tmp_lport = (p_hss_vec->serdes_info[lane_id].lport)/4*4;
            }
            else
            {
                tmp_lport = p_hss_vec->serdes_info[lane_id/4*4].lport + (lane_id%4);
            }
        }
    }
    port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
    if (port_attr == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_switch_get_info(lchip, port_attr, dst_mode, p_ds_attr, TRUE, serdes_id));
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_switch_serdes_get_none_info(uint8 lchip, uint8 serdes_id, uint8 dst_mode, sys_datapath_serdes_info_t* p_serdes,
                                                              sys_datapath_dynamic_switch_attr_t* p_ds_attr)
{
    uint16 lport = 0;
    uint16 tmp_lport = 0;
    /*uint8  port_num = 0;*/
    uint8  slice_id = (serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;
    uint8  lane_idx = 0;
    uint8  lane_swap_0 = FALSE;
    uint8  lane_swap_1 = FALSE;
    sys_datapath_lport_attr_t* port_attr = NULL;

    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }

    /*Only serdes 0~11 can be switched to Q/U*/
    if((11 < serdes_id) && (SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(dst_mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u does not support mode %u. (5)\n", serdes_id, dst_mode);
        return CTC_E_NOT_SUPPORT;
    }

    /*any mode -> none mode, drop all ports related to this serdes*/
    if(CTC_CHIP_SERDES_NONE_MODE == dst_mode)
    {
        /*if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && (16 <= serdes_id || 23 >= serdes_id))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " serdes_id %u does not support dynamic switch! \n", serdes_id);
            return CTC_E_NOT_SUPPORT;
        }*/
        if(SYS_DATAPATH_IS_SERDES_4_PORT_1(p_serdes->mode))
        {
            for(lane_idx = 0; lane_idx < 4; lane_idx++)
            {
                p_ds_attr->s[lane_idx].serdes_id = serdes_id/4*4 + lane_idx;
                p_ds_attr->s[lane_idx].src_mode = p_serdes->mode;
                p_ds_attr->s[lane_idx].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
            }
            p_ds_attr->serdes_num = 4;
        }
        else if(CTC_CHIP_SERDES_LG_MODE == p_serdes->mode)
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_get_lane_swap_flag(lchip, &lane_swap_0, &lane_swap_1));
            if(serdes_id > 27)
            {
                if(TRUE == lane_swap_1)
                {
                    if((28 == serdes_id) || (31 == serdes_id))
                    {
                        p_ds_attr->s[0].serdes_id = 28;
                        p_ds_attr->s[1].serdes_id = 31;
                    }
                    else
                    {
                        p_ds_attr->s[0].serdes_id = 29;
                        p_ds_attr->s[1].serdes_id = 30;
                    }
                    p_ds_attr->s[0].src_mode = CTC_CHIP_SERDES_LG_MODE;
                    p_ds_attr->s[0].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                    p_ds_attr->s[1].src_mode = CTC_CHIP_SERDES_LG_MODE;
                    p_ds_attr->s[1].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                }
                else
                {
                    for(lane_idx = 0; lane_idx < 2; lane_idx++)
                    {
                        p_ds_attr->s[lane_idx].serdes_id = serdes_id/2*2 + lane_idx;
                        p_ds_attr->s[lane_idx].src_mode = p_serdes->mode;
                        p_ds_attr->s[lane_idx].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                    }
                }
            }
            else
            {
                if(TRUE == lane_swap_0)
                {
                    if((24 == serdes_id) || (27 == serdes_id))
                    {
                        p_ds_attr->s[0].serdes_id = 24;
                        p_ds_attr->s[1].serdes_id = 27;
                    }
                    else
                    {
                        p_ds_attr->s[0].serdes_id = 25;
                        p_ds_attr->s[1].serdes_id = 26;
                    }
                    p_ds_attr->s[0].src_mode = CTC_CHIP_SERDES_LG_MODE;
                    p_ds_attr->s[0].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                    p_ds_attr->s[1].src_mode = CTC_CHIP_SERDES_LG_MODE;
                    p_ds_attr->s[1].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                }
                else
                {
                    for(lane_idx = 0; lane_idx < 2; lane_idx++)
                    {
                        p_ds_attr->s[lane_idx].serdes_id = serdes_id/2*2 + lane_idx;
                        p_ds_attr->s[lane_idx].src_mode = p_serdes->mode;
                        p_ds_attr->s[lane_idx].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
                    }
                }
            }
            p_ds_attr->serdes_num = 2;
        }
        else
        {
            p_ds_attr->s[0].serdes_id = serdes_id;
            p_ds_attr->s[0].src_mode = p_serdes->mode;
            p_ds_attr->s[0].dst_mode = CTC_CHIP_SERDES_NONE_MODE;
            p_ds_attr->serdes_num = 1;
        }

        lport = p_serdes->lport;
        for(tmp_lport = lport; tmp_lport < lport + p_serdes->port_num; tmp_lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, tmp_lport, slice_id);
            if (port_attr == NULL)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                return CTC_E_NOT_INIT;
            }
            if(CTC_CHIP_SERDES_NONE_MODE == port_attr->pcs_mode)
            {
                continue;
            }
            p_ds_attr->m[p_ds_attr->mac_num].add_drop_flag = 2;
            p_ds_attr->m[p_ds_attr->mac_num].mac_id = port_attr->mac_id;
            (p_ds_attr->mac_num)++;
        }
    }
    /*none mode -> any mode, add target port & serdes, then parser all influenced port & serdes*/
    else if(CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode)
    {
        if((2 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && 
           (((17 <= serdes_id) && (19 >= serdes_id)) || ((21 <= serdes_id) && (23 >= serdes_id))) && 
           (!SYS_DATAPATH_IS_SERDES_4_PORT_1(dst_mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes id %u cannot switch to mode %u.\n", serdes_id, dst_mode);
            return CTC_E_NOT_SUPPORT;
        }

        if(((0 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && (3 == serdes_id) && 
                                  SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(dst_mode)) ||
           ((1 == p_usw_datapath_master[lchip]->qm_choice.txqm3) && (11 == serdes_id) && 
                                  SYS_DATAPATH_IS_SERDES_QSGMII_USXGMII_MULTI(dst_mode)) ||
           ((1 == p_usw_datapath_master[lchip]->qm_choice.txqm1) && (0 == p_usw_datapath_master[lchip]->qm_choice.txqm3) && 
                                                 (20 <= serdes_id) && (23 >= serdes_id)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Cannot alloc new mac id!\n");
            return CTC_E_INVALID_PARAM;
        }

        if(!SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(serdes_id) && SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(dst_mode))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Serdes id %u cannot support mode %u!\n", serdes_id, dst_mode);
            return CTC_E_INVALID_PARAM;
        }

        if(!SYS_DATAPATH_SERDES_IS_WITH_QSGMIIPCS(serdes_id) && SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(dst_mode))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Serdes id %u cannot support mode %u!\n", serdes_id, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
        
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_serdes_mapping(lchip, serdes_id, dst_mode, &lport, 
                                                                  NULL, NULL, NULL, NULL, NULL));
        port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
        if (port_attr == NULL)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
            return CTC_E_NOT_INIT;
        }
        if((2 == port_attr->flag) && (CTC_CHIP_SERDES_LG_MODE == dst_mode))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "50G lane swap config error!\n");
            return CTC_E_INVALID_CONFIG;
        }
        
        p_ds_attr->s[0].serdes_id = serdes_id;
        p_ds_attr->s[0].src_mode = CTC_CHIP_SERDES_NONE_MODE;
        p_ds_attr->s[0].dst_mode = dst_mode;
        p_ds_attr->serdes_num = 1;

        CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_switch_get_info(lchip, port_attr, dst_mode, p_ds_attr, TRUE, serdes_id));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_overclock_valid_check(uint8 lchip, uint8 serdes_id, uint8 dst_mode, uint16 overclocking_speed)
{
    if(CTC_CHIP_SERDES_OCS_MODE_12_12G == overclocking_speed)
    {
        if((CTC_CHIP_SERDES_XFI_MODE != dst_mode) && (CTC_CHIP_SERDES_XLG_MODE != dst_mode))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Overclock parameter error! serdes=%u, ovclk=%u, mode=%u\n", serdes_id, overclocking_speed, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
    }
    else if(CTC_CHIP_SERDES_OCS_MODE_27_27G == overclocking_speed)
    {
        if((SYS_DATAPATH_SERDES_28G_START_ID > serdes_id) || ((CTC_CHIP_SERDES_XXVG_MODE != dst_mode) && 
                                                                (CTC_CHIP_SERDES_LG_MODE != dst_mode) && 
                                                                (CTC_CHIP_SERDES_CG_MODE != dst_mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Overclock parameter error! serdes=%u, ovclk=%u, mode=%u\n", serdes_id, overclocking_speed, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
    }
    else if(CTC_CHIP_SERDES_OCS_MODE_12_58G == overclocking_speed)
    {
        if((SYS_DATAPATH_SERDES_28G_START_ID > serdes_id) || ((CTC_CHIP_SERDES_XFI_MODE != dst_mode) && 
                                                                (CTC_CHIP_SERDES_XLG_MODE != dst_mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Overclock parameter error! serdes=%u, ovclk=%u, mode=%u\n", serdes_id, overclocking_speed, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
    }
    else if(CTC_CHIP_SERDES_OCS_MODE_11_06G == overclocking_speed)
    {
        if((SYS_DATAPATH_SERDES_28G_START_ID > serdes_id) || ((CTC_CHIP_SERDES_XFI_MODE != dst_mode) && 
                                                                (CTC_CHIP_SERDES_XLG_MODE != dst_mode)))
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Overclock parameter error! serdes=%u, ovclk=%u, mode=%u\n", serdes_id, overclocking_speed, dst_mode);
            return CTC_E_INVALID_PARAM;
        }
    }
    else if(CTC_CHIP_SERDES_OCS_MODE_NONE != overclocking_speed)
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_switch_serdes_get_overclock(uint8 lchip, uint8 serdes_id, sys_datapath_serdes_info_t* p_serdes, 
                                                                 uint8 dst_mode, sys_datapath_dynamic_switch_attr_t* p_ds_attr,
                                                                 uint16 overclocking_speed)
{
    uint8  slice_id = (serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;
    uint8  serdes_num = 0;
    uint8  idx = 0;
    uint8  ds_idx = 0;
    uint8  real_lport = 0;
    uint16 chip_port = 0;
    uint16 start_serdes = 0;
    uint16 real_serdes = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* real_port_attr = NULL;

    /*parameter check*/
    if(p_serdes->overclocking_speed == overclocking_speed)
    {
        return CTC_E_NONE;
    }
    /*different mode overclock does not support currently*/
    if(dst_mode != p_serdes->mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cannot support overclock with mode change! src mode %u\n", p_serdes->mode);
        return CTC_E_INVALID_PARAM;
    }

    chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(p_serdes->lport) - slice_id*SYS_USW_MAX_PORT_NUM_PER_CHIP;
    CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, slice_id, &start_serdes, &serdes_num));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, p_serdes->lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
        return CTC_E_INVALID_CONFIG;
    }

    /*1. For src_mode == dst_mode conditions, add all influenced mac & serdes to p_ds_attr*/
    if(p_serdes->mode == dst_mode)
    {
        if(SYS_DATAPATH_SERDES_IS_HSS28G(start_serdes))
        {
            for(idx = 0; idx < 4; idx++)
            {
                real_serdes = start_serdes/4*4 + idx;
                p_ds_attr->s[p_ds_attr->serdes_num].serdes_id = real_serdes;
                p_ds_attr->s[p_ds_attr->serdes_num].src_mode = p_serdes->mode;
                p_ds_attr->s[p_ds_attr->serdes_num].dst_mode = dst_mode;
                SYS_DATAPATH_FILL_OVCLK_FLAG(overclocking_speed, p_ds_attr->s[p_ds_attr->serdes_num].ovclk_flag);
                (p_ds_attr->serdes_num)++;

                real_lport = (p_serdes->lport)/4*4 + idx;
                sys_usw_mac_get_port_capability(lchip, real_lport, &real_port_attr);
                if(CTC_CHIP_SERDES_NONE_MODE == real_port_attr->pcs_mode)
                {
                    continue;
                }
                p_ds_attr->m[p_ds_attr->mac_num].mac_id = real_lport;
                p_ds_attr->m[p_ds_attr->mac_num].add_drop_flag = 0;
                (p_ds_attr->mac_num)++;
            }
        }
        else
        {
            for(idx = 0; idx < serdes_num; idx++)
            {
                GET_SERDES(port_attr->mac_id, start_serdes, idx, port_attr->flag, port_attr->pcs_mode, real_serdes);
                p_ds_attr->s[p_ds_attr->serdes_num].serdes_id = real_serdes;
                p_ds_attr->s[p_ds_attr->serdes_num].src_mode = p_serdes->mode;
                p_ds_attr->s[p_ds_attr->serdes_num].dst_mode = dst_mode;
                SYS_DATAPATH_FILL_OVCLK_FLAG(overclocking_speed, p_ds_attr->s[p_ds_attr->serdes_num].ovclk_flag);
                (p_ds_attr->serdes_num)++;
            }
            p_ds_attr->m[p_ds_attr->mac_num].mac_id = p_serdes->lport;
            p_ds_attr->m[p_ds_attr->mac_num].add_drop_flag = 0;
            (p_ds_attr->mac_num)++;
        }
    }
    
    /*2. For src_mode != dst_mode conditions, mark influenced serdes in p_ds_attr*/
    else
    {
        for(idx = 0; idx < serdes_num; idx++)
        {
            GET_SERDES(port_attr->mac_id, start_serdes, idx, port_attr->flag, port_attr->pcs_mode, real_serdes);
            /*find current serdes in p_ds_attr*/
            for(ds_idx = 0; ds_idx < p_ds_attr->serdes_num; ds_idx++)
            {
                if(p_ds_attr->s[ds_idx].serdes_id == real_serdes)
                {
                    break;
                }
            }
            /*if curren serdes is not exist in p_ds_attr, add this serdes into p_ds_attr*/
            if(ds_idx == p_ds_attr->serdes_num)
            {
                p_ds_attr->s[p_ds_attr->serdes_num].serdes_id = real_serdes;
                p_ds_attr->s[p_ds_attr->serdes_num].src_mode = p_serdes->mode;
                p_ds_attr->s[p_ds_attr->serdes_num].dst_mode = dst_mode;
                SYS_DATAPATH_FILL_OVCLK_FLAG(overclocking_speed, p_ds_attr->s[p_ds_attr->serdes_num].ovclk_flag);
                (p_ds_attr->serdes_num)++;
            }
            /*if curren serdes is exist, mark ovclk_flag*/
            else
            {
                SYS_DATAPATH_FILL_OVCLK_FLAG(overclocking_speed, p_ds_attr->s[ds_idx].ovclk_flag);
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_serdes_dynamic_switch(uint8 lchip, uint8 serdes_id, uint8 mode, uint16 overclocking_speed)
{
    uint8  gchip              = 0;
    uint8  hss_idx            = 0;
    uint8  slice_id           = 0;
    uint8  lane_id            = 0;
    int32  ret                = 0;
    uint16 tmp_lport          = 0;
    uint32 i                  = 0;
    uint32 related_serdes_num = 0;
    uint32 tmp_gport          = 0;
    uint16 lport              = 0;
    uint8  hss_num            = HSS12G_NUM_PER_SLICE + TSINGMA_HSS28G_NUM_PER_SLICE;
    sys_datapath_serdes_info_t*   p_serdes  = NULL;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_dynamic_switch_attr_t ds_attr;
    sys_datapath_hss_pll_info_t dst_pll[hss_num];
    sys_datapath_lport_attr_t* port_attr = NULL;
    sys_datapath_lport_attr_t* port_attr_tmp = NULL;
    
    DATAPATH_INIT_CHECK();

    sal_memset(dst_pll, 0, hss_num * sizeof(sys_datapath_hss_pll_info_t));

    if(NULL == p_usw_datapath_master[lchip])
    {
        return CTC_E_INVALID_PTR;
    }
    if (p_usw_datapath_master[lchip]->glb_xpipe_en)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Dynamic switch is not suppported when X-PIPE enabled \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%d,  Mode:%d \n", serdes_id, mode);

    sys_usw_get_gchip_id(lchip, &gchip);

    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    slice_id = (serdes_id >= TSINGMA_SERDES_NUM_PER_SLICE)?1:0;

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

    sal_memset(&ds_attr, 0, sizeof(sys_datapath_dynamic_switch_attr_t));

    /*parameter check*/
    if(CTC_CHIP_SERDES_XSGMII_MODE == mode)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_15G) &&
        ((CTC_CHIP_SERDES_CG_MODE == mode)||(CTC_CHIP_SERDES_XXVG_MODE == mode)||(CTC_CHIP_SERDES_LG_MODE == mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    else if ((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_28G) && 
             (SYS_DATAPATH_IS_SERDES_USXGMII_QSGMII(mode) || (CTC_CHIP_SERDES_100BASEFX_MODE == mode)))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    /*28G SGMII->NONE/NONE->SGMII limitation*/
    if((p_hss_vec->hss_type == SYS_DATAPATH_HSS_TYPE_28G) && 
       ((((CTC_CHIP_SERDES_XSGMII_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode)) &&
         ((CTC_CHIP_SERDES_SGMII_MODE == mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == mode))) ||
        (((CTC_CHIP_SERDES_XSGMII_MODE == mode) || (CTC_CHIP_SERDES_NONE_MODE == mode)) &&
         ((CTC_CHIP_SERDES_SGMII_MODE == p_serdes->mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == p_serdes->mode)))))
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cannot switch SGMII/2.5G <-> None directly. Please switch to other mode first. \n");
        return CTC_E_NOT_SUPPORT;
    }

    /*switch mode not change, do not do switch*/
    _sys_tsingma_datapath_get_switch_serdes_num(lchip, p_serdes->mode, mode, &related_serdes_num);
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_serdes_mapping(lchip, serdes_id, ((CTC_CHIP_SERDES_NONE_MODE == p_serdes->mode) ? mode : p_serdes->mode), 
                     &lport, NULL, NULL, NULL, NULL, NULL));
    port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if(NULL == port_attr)
    {
        return CTC_E_INVALID_PTR;
    }
    if(0 == related_serdes_num)
    {
        return CTC_E_NONE;
    }
    else if(4 == related_serdes_num)
    {
        for (i = 0; i < 4; i++)
        {
            if((p_hss_vec->serdes_info[lane_id/4*4+i].mode == mode) && (p_hss_vec->serdes_info[lane_id/4*4+i].overclocking_speed == overclocking_speed))
            {
                return CTC_E_NONE;
            }
        }
    }
    else if(2 == related_serdes_num)
    {
        if(0 == port_attr->flag)
        {
            for (i = 0; i < 2; i++)
            {
                if((p_hss_vec->serdes_info[lane_id/2*2+i].mode == mode) && (p_hss_vec->serdes_info[lane_id/2*2+i].overclocking_speed == overclocking_speed))
                {
                    return CTC_E_NONE;
                }
            }
        }
        else if(1 == port_attr->flag)
        {
            if((((lane_id == 0) || (lane_id == 3)) && 
                (p_hss_vec->serdes_info[0].mode == mode) && (p_hss_vec->serdes_info[0].overclocking_speed == overclocking_speed) && 
                (p_hss_vec->serdes_info[3].mode == mode) && (p_hss_vec->serdes_info[3].overclocking_speed == overclocking_speed)) || 
               (((lane_id == 1) || (lane_id == 2)) && 
                (p_hss_vec->serdes_info[1].mode == mode) && (p_hss_vec->serdes_info[1].overclocking_speed == overclocking_speed) && 
                (p_hss_vec->serdes_info[2].mode == mode) && (p_hss_vec->serdes_info[2].overclocking_speed == overclocking_speed)))
            {
                return CTC_E_NONE;
            }
            
        }
        else
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " 50G lane swap config error! lane_id %u \n", lane_id);
            return CTC_E_NOT_SUPPORT;
        }
    }
    else if(1 == related_serdes_num)
    {
        if((p_serdes->mode == mode) && (p_serdes->overclocking_speed == overclocking_speed))
        {
            return CTC_E_NONE;
        }
    }

    /*Fill ds_attr, which saves all influenced ports & serdes*/
    if(p_serdes->mode != mode)
    {
        if((CTC_CHIP_SERDES_NONE_MODE != mode) && (CTC_CHIP_SERDES_NONE_MODE != p_serdes->mode))
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_switch_serdes_get_info(lchip, p_serdes, p_hss_vec, serdes_id, mode, &ds_attr));
        }
        else
        {
            CTC_ERROR_RETURN(sys_tsingma_datapath_switch_serdes_get_none_info(lchip, serdes_id, mode, p_serdes, &ds_attr));
        }
    }

    /*Overclock serdes add into ds_attr*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_switch_serdes_get_overclock(lchip, serdes_id, p_serdes, mode, &ds_attr, overclocking_speed));
    CTC_ERROR_RETURN(sys_tsingma_datapath_dynamic_switch_sort_target(lchip, &ds_attr));

    /*Fill dst_pll, which saves serdes pll modification. If dest pll cannot be allocated, return.*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_dynamic_switch_pll_parser(lchip, &ds_attr, dst_pll));

    /* set CL73 AN cfg to default disable */
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

    CTC_ERROR_GOTO(_sys_tsingma_datapath_set_pre_dynamic_switch(lchip, &ds_attr), ret, error);

    /*soft tables config*/
    CTC_ERROR_GOTO(sys_tsingma_datapath_set_soft_tables(lchip, &ds_attr, dst_pll), ret, error);

    /*swap reconfig*/
    CTC_ERROR_GOTO(sys_tsingma_datapath_dynamic_switch_swap_reconfig(lchip, &ds_attr), ret, error);

    /*serdes config*/
    CTC_ERROR_GOTO(_sys_tsingma_datapath_dynamic_switch_serdes_cfg(lchip, dst_pll, &ds_attr), ret, error);

    for (i = 0; i < ds_attr.serdes_num; i++)
    {
        serdes_id = ds_attr.s[i].serdes_id;
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        CTC_ERROR_GOTO(sys_tsingma_datapath_set_hss28g_firmware_mode(lchip, hss_idx, lane_id), ret, error);
    }

    if (CTC_CHIP_SERDES_NONE_MODE != mode)
    {
        CTC_ERROR_GOTO(_sys_tsingma_datapath_set_post_dynamic_switch(lchip, p_serdes->lport, mode, &ds_attr), ret, error);
    }
    else
    {
        _sys_tsingma_datapath_dynamic_epe_mem_distribute(lchip, mode, 1, &ds_attr);
        _sys_tsingma_datapath_dynamic_epe_mem_distribute(lchip, mode, 0, &ds_attr);
        CTC_ERROR_GOTO(sys_tsingma_datapath_dynamic_reset_qm(lchip, mode, &ds_attr, port_attr), ret, error);
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

int32 
sys_tsingma_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info)
{
    uint8  hss_idx          = 0;
    uint8  lane_id          = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    CTC_PTR_VALID_CHECK(p_serdes_info);

    if (p_serdes_info->overclocking_speed == SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_11G_MODE)
    {
        p_serdes_info->overclocking_speed = CTC_CHIP_SERDES_OCS_MODE_11_06G;
    }
    else if (p_serdes_info->overclocking_speed == SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_12DOT5G_MODE)
    {
        p_serdes_info->overclocking_speed = CTC_CHIP_SERDES_OCS_MODE_12_58G;
    }

    MAC_LOCK;
    if (CTC_CHIP_SERDES_OCS_MODE_NONE != p_serdes_info->overclocking_speed)
    {
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_overclock_valid_check(lchip, p_serdes_info->serdes_id, 
                         p_serdes_info->serdes_mode, p_serdes_info->overclocking_speed));
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_dynamic_switch(lchip, \
                                        p_serdes_info->serdes_id, \
                                        p_serdes_info->serdes_mode, \
                                        CTC_CHIP_SERDES_OCS_MODE_NONE));
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_dynamic_switch(lchip, \
                                        p_serdes_info->serdes_id, \
                                        p_serdes_info->serdes_mode, \
                                        p_serdes_info->overclocking_speed));
    }
    else
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(p_serdes_info->serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(p_serdes_info->serdes_id);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if(NULL == p_hss_vec)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Serdes not initialized \n");
            MAC_UNLOCK;
            return CTC_E_NOT_SUPPORT;
        }
        if(CTC_CHIP_SERDES_OCS_MODE_NONE != p_hss_vec->serdes_info[lane_id].overclocking_speed)
        {
            CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_dynamic_switch(lchip, \
                                            p_serdes_info->serdes_id, \
                                            p_hss_vec->serdes_info[lane_id].mode, \
                                            CTC_CHIP_SERDES_OCS_MODE_NONE));
        }
        CTC_ERROR_RETURN_WITH_MAC_UNLOCK(sys_tsingma_datapath_set_serdes_dynamic_switch(lchip, \
                                        p_serdes_info->serdes_id, \
                                        p_serdes_info->serdes_mode, \
                                        CTC_CHIP_SERDES_OCS_MODE_NONE));
    }
    MAC_UNLOCK;

    return CTC_E_NONE;
}

#define __HSS28G_FIRMWARE__

/*******************************************************************************************************
1)ctle mode: 0x980f[7:4] correspond to lane3~lane0 # 1 is manual ctle mode, default is 0(default value 0)
********************************************************************************************************/
STATIC int32
_sys_tsingma_hss28g_ctle_mode_init(uint8 lchip, uint8 acc_id, uint8 ctle_mode)
{
    uint16 reg_address = 0x980f;
    uint16 mask = 0;
    uint16 data = 0;

    /* ctle mode config */
    mask = 0xff0f;
    data = ctle_mode? 1: 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_address, mask, data));
    
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_load_hss28g_firmware(uint8 lchip, uint8 acc_id)
{
    uint16 default_reg_addr[] = {0x9F00, 0x980D, 0x9814};
    uint32 start      = 0;
    uint32 len        = MCU_28G_GET_UINT32(g_tsingma_28g_firmware + start + 12);
    uint32 enter_pt   = MCU_28G_GET_UINT32(g_tsingma_28g_firmware + start + 8);
    uint32 start_addr = MCU_28G_GET_UINT32(g_tsingma_28g_firmware + start + 16);
    uint32 data_addr  = start + 20;
    uint32 ram_addr   = start_addr;
    /*uint32 checksum   = 0;    //In 28G FW loader, this is an signed int value*/
    int32 checksum   = 0;

    uint32 cnt, step;
    uint32 section    = len/24 + 1;
    uint32 checktime  = 0;
    uint16 status;
    uint16 mdio_data;

    /*reloading FW*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[2], 0, 0xfff0));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_set_domain_reset(lchip, acc_id, SYS_TM_28G_CPU_RESET));
    sal_task_sleep(500);

    for(cnt = 0; cnt < section; cnt++)
    {
        checksum = 0x800c;
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+12, 0, ram_addr>>16));
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+13, 0, ram_addr&0xffff));
        checksum += (ram_addr>>16) + (ram_addr&0xffff);
        for(step = 0; step < 12; step++)
        {
            mdio_data = MCU_28G_GET_UINT16(g_tsingma_28g_firmware + data_addr);
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+step, 0, mdio_data));
            checksum += mdio_data;
            data_addr += 2;
            ram_addr += 2;
        }
        /*checksum = (checksum ^ 0xffff) + 1;*/
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+14, 0, (-checksum)&0xffff));
        CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, 0x800c));

        checktime = 0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, &status));
        while(0x800c == status)
        {
            checktime++;
            if(1000 < checktime)
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[HSS28G Firmware] Cannot get status 0x800c!\n");
                return CTC_E_HW_TIME_OUT;
            }
            CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, &status));
            sal_task_sleep(10);
        }
        if(0 != status)
        {
            return CTC_E_HW_FAIL;
        }
    }

    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+12, 0, enter_pt>>16));
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+13, 0, enter_pt&0xffff));
    checksum = (enter_pt>>16) + (enter_pt&0xffff) + 0x4000;
    /*checksum = (checksum ^ 0xffff) + 1;*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+14, 0, (-checksum)&0xffff));
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, 0x4000));

    checktime = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, &status));
    while(0x4000 == status)
    {
        checktime++;
        if(1000 < checktime)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[HSS28G Firmware] Cannot get status 0x4000!\n");
            return CTC_E_HW_TIME_OUT;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[0]+15, 0, &status));
        sal_task_sleep(10);
    }

    /*Verify FW execution has started*/
    checktime = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[2], 0, &status));
    while(0x6a6a != status)
    {
        checktime++;
        if(1000 < checktime)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[HSS28G Firmware] Cannot get status 0x6a6a!\n");
            return CTC_E_HW_TIME_OUT;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, default_reg_addr[2], 0, &status));
        sal_task_sleep(10);
    }

    /*checktime = 0;
    CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, 0x9810, 0, &status));
    sal_printf("checking exec    status 0x%x\n", status);
    for(checktime = 0; checktime < 20; checktime++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, 0x9810, 0, &status));
        sal_printf("0x9810 checktime %u, status 0x%x\n", checktime, status);
        sal_task_sleep(1000);
    }*/
    
    return CTC_E_NONE;
}

/* EG6_config_mode */
int32
_sys_tsingma_datapath_hss28g_firmware_en(uint8 lchip, uint8 acc_id, uint8 lane_id, uint8 fw_mode, uint8 serdes_mode)
{
    uint16 data = 0;
    uint32 index = 0;
    uint8 an_lane = 0;
    uint16 status = 0;
    uint16 reg_cmd = 0x9815;
    uint16 reg_cmd_detail = 0x9816;
    uint8 right_status = 0;

    if (CTC_CHIP_SERDES_LG_MODE == serdes_mode)
    {
        an_lane = lane_id - lane_id % 2;
    }
    else if ((CTC_CHIP_SERDES_XLG_MODE == serdes_mode) || (CTC_CHIP_SERDES_CG_MODE == serdes_mode))
    {
        an_lane = lane_id - lane_id % 3;
    }
    else if ((CTC_CHIP_SERDES_XFI_MODE == serdes_mode) || (CTC_CHIP_SERDES_XXVG_MODE == serdes_mode))
    {
        an_lane = lane_id;
    }

    data = an_lane;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_cmd_detail, 0, data));
    
    data = 0x8000 + (fw_mode << 4) + lane_id;
    CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss28g_reg(lchip, acc_id, reg_cmd, 0, data));

    for (; index < 1000; index++)
    {
        CTC_ERROR_RETURN(sys_tsingma_datapath_read_hss28g_reg(lchip, acc_id, reg_cmd, 0, &status));
        if (status != data)
        {
            right_status = 1;
            break;
        }
    }
    if (right_status)
    {
        if (status != 0x0800)
        {
            SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[HSS28G Firmware] Debug command failed with code %04x\n", status);
            return CTC_E_HW_FAIL;
        }
    }
    else
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[HSS28G Firmware] Command 0x%04x failed. is firmware loaded?\n", data);
        return CTC_E_HW_FAIL;
    }
    
    return CTC_E_NONE;
}


int32
_sys_tsingma_datapath_get_hss28g_firmware_mode(uint8 mode, uint8 ovclk_speed, uint8* fw_mode)
{
    uint8 tmp_fw_mode = 0;

    switch (mode)
    {
        case CTC_CHIP_SERDES_XFI_MODE:
        case CTC_CHIP_SERDES_XLG_MODE:
            tmp_fw_mode = 1;
            if (CTC_CHIP_SERDES_OCS_MODE_11_06G == ovclk_speed)
            {
                tmp_fw_mode = 3;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_12G == ovclk_speed)
            {
                tmp_fw_mode = 4;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_58G == ovclk_speed)
            {
                tmp_fw_mode = 9;
            }
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_LG_MODE:    
        case CTC_CHIP_SERDES_CG_MODE:
            tmp_fw_mode = 2;
            if (CTC_CHIP_SERDES_OCS_MODE_27_27G == ovclk_speed)
            {
                tmp_fw_mode = 5;
            }
            break;
        case CTC_CHIP_SERDES_DXAUI_MODE:
            tmp_fw_mode = 6;
            break;
        case CTC_CHIP_SERDES_XAUI_MODE:
        case CTC_CHIP_SERDES_2DOT5G_MODE:
            tmp_fw_mode = 7;
            break;
        case CTC_CHIP_SERDES_SGMII_MODE:
            tmp_fw_mode = 8;
            break;
        case CTC_CHIP_SERDES_NONE_MODE:
        default:
            tmp_fw_mode = 0;
            break;
    }
    
    SYS_USW_VALID_PTR_WRITE(fw_mode, tmp_fw_mode);
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_datapath_get_autoneg_sel(uint8 lchip, uint8 mode, uint16 ocs, uint32* auto_neg_status)
{
    uint8 speed_mode = 0;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Enter %s\n", __FUNCTION__);
    
    switch (mode)
    {
        case CTC_CHIP_SERDES_XFI_MODE:
            speed_mode = 2;
            if (CTC_CHIP_SERDES_OCS_MODE_11_06G == ocs)
            {
                speed_mode = 19;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_12G == ocs)
            {
                speed_mode = 20;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_58G == ocs)
            {
                speed_mode = 21;
            }
            break;
        case CTC_CHIP_SERDES_XLG_MODE:
            speed_mode = 3;
            if (CTC_CHIP_SERDES_OCS_MODE_11_06G == ocs)
            {
                speed_mode = 19;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_12G == ocs)
            {
                speed_mode = 20;
            }
            else if (CTC_CHIP_SERDES_OCS_MODE_12_58G == ocs)
            {
                speed_mode = 21;
            }
            break;    
        case CTC_CHIP_SERDES_CG_MODE:
            speed_mode = 7;
            if (CTC_CHIP_SERDES_OCS_MODE_27_27G == ocs)
            {
                speed_mode = 22;
            }
            break;
        case CTC_CHIP_SERDES_XXVG_MODE:
        case CTC_CHIP_SERDES_LG_MODE:
            speed_mode = 10;
            if (CTC_CHIP_SERDES_OCS_MODE_27_27G == ocs)
            {
                speed_mode = 22;
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"speed_mode:%d.\n", speed_mode);
    
    SYS_USW_VALID_PTR_WRITE(auto_neg_status, speed_mode);
    
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_serdes_set_hss28g_link_training_en(uint8 lchip, uint16 serdes_id, uint8 enable)
{
    uint8  fw_mode = 0;
    uint8  hss_id   = 0;
    uint8  hss_idx  = 0;
    uint8  lane_id  = 0;
    uint8  step     = Hss28GCfg_cfgHssL1AutoNegDone_f - Hss28GCfg_cfgHssL0AutoNegDone_f;
    uint32 cmd      = 0;
    uint32 value    = ((FALSE == enable) ? 0 : 1);
    uint32 speed_value = 0;
    Hss28GCfg_m hss_an;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Enter %s\n", __FUNCTION__);
    SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"serdes_id:%u, enable:%u\n", serdes_id, enable);

    if(!SYS_DATAPATH_SERDES_IS_HSS28G(serdes_id))
    {
        return CTC_E_NONE;
    }

    hss_id = SYS_DATAPATH_MAP_SERDES_TO_HSSID(serdes_id);
    hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
    lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if(p_hss == NULL)
    {
        return CTC_E_NOT_EXIST;
    }
    
    if((0 == SDK_WORK_PLATFORM) && (0 == SDK_WORK_ENV) && (!enable))
    {
        /* stop fw */
#ifndef EMULATION_ENV
        CTC_ERROR_RETURN(_sys_tsingma_datapath_get_hss28g_firmware_mode(p_hss->serdes_info[lane_id].mode, 
                         p_hss->serdes_info[lane_id].overclocking_speed, &fw_mode));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_firmware_en(lchip, hss_id, lane_id, fw_mode, p_hss->serdes_info[lane_id].mode));
#endif
    }

    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_autoneg_sel(lchip, p_hss->serdes_info[lane_id].mode, p_hss->serdes_info[lane_id].overclocking_speed, &speed_value));

    cmd = DRV_IOR(Hss28GCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_an));
    DRV_IOW_FIELD(lchip, Hss28GCfg_t, (Hss28GCfg_cfgHssL0AutoNegSelect_f + (serdes_id-24)*step), &speed_value, &hss_an);
    DRV_IOW_FIELD(lchip, Hss28GCfg_t, (Hss28GCfg_cfgHssL0AutoNegStatus_f + (serdes_id-24)*step), &value, &hss_an);
    cmd = DRV_IOW(Hss28GCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss_an));
    
    if((0 == SDK_WORK_PLATFORM) && (0 == SDK_WORK_ENV) && enable)
    {    
        /* start fw */
#ifndef EMULATION_ENV
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_firmware_en(lchip, hss_id, lane_id, 0xf, p_hss->serdes_info[lane_id].mode));
#endif
    }
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_set_hss28g_firmware_mode(uint8 lchip, uint8 hss_idx, uint8 lane_id)
{
    uint8 fw_mode = 0;
    uint8 hss_id = 0;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    if (hss_idx < HSS12G_NUM_PER_SLICE)
    {
        /* only valid for HSS28G */
        return CTC_E_NONE;
    }
    p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
    if (p_hss == NULL)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }
    
    CTC_ERROR_RETURN(_sys_tsingma_datapath_get_hss28g_firmware_mode(p_hss->serdes_info[lane_id].mode, 
        p_hss->serdes_info[lane_id].overclocking_speed, &fw_mode));
    
    if((0 == SDK_WORK_PLATFORM) && (0 == SDK_WORK_ENV))
    {    
        /* start fw */
#ifndef EMULATION_ENV
        hss_id = hss_idx - HSS12G_NUM_PER_SLICE;
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_firmware_en(lchip, hss_id, lane_id, fw_mode, p_hss->serdes_info[lane_id].mode));
#endif
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_init_hss28g_firmware(uint8 lchip)
{
    uint8 hss_id  = 0;
    uint8 hss_idx = 0;
    uint8 lane_id = 0;
    uint8 write_info_flag = TRUE;
    sys_datapath_hss_attribute_t* p_hss = NULL;

    if (1 == SDK_WORK_PLATFORM)
    {
        return CTC_E_NONE;
    }

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return CTC_E_NONE;
    }

    /*clear old FW version before load FW*/
    CTC_ERROR_RETURN(sys_usw_mcu_chip_write(lchip, SYS_TSINGMA_MCU_3_VERSION_BASE_ADDR, 0));

    for (hss_id = 0; hss_id < TSINGMA_HSS28G_NUM_PER_SLICE; hss_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_HSSID_TO_HSSIDX(hss_id, SYS_DATAPATH_HSS_TYPE_28G);
        p_hss = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL != p_hss)
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_load_hss28g_firmware(lchip, hss_id));
            CTC_ERROR_RETURN(_sys_tsingma_hss28g_ctle_mode_init(lchip, hss_id, 0));
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_set_domain_reset(lchip, hss_id, SYS_TM_28G_LOGIC_RESET));
            sal_task_sleep(1);
            for (lane_id = 0; lane_id < HSS28G_LANE_NUM; lane_id++)
            {
                CTC_ERROR_RETURN(sys_tsingma_datapath_set_hss28g_firmware_mode(lchip, hss_idx, lane_id));
            }

            /*get new FW version after FW init*/
            if(write_info_flag)
            {
                CTC_ERROR_RETURN(_sys_tsingma_serdes_get_hss28g_fw_info(lchip, hss_id));
                write_info_flag = FALSE;
            }
        }
    }

    return CTC_E_NONE;
}

#define __INIT__

STATIC int32
_sys_tsingma_datapath_update_glb_xpipe(uint8 lchip)
{
    uint8 slice_id = 0;
    uint16 lport = 0;
    uint8 glb_xpipe_en = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }
            if (port_attr->xpipe_en)
            {
                glb_xpipe_en = 1;
            }
        }
    }

    p_usw_datapath_master[lchip]->glb_xpipe_en = glb_xpipe_en;

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_init_lane_swap(uint8 lchip)
{
    uint8  hss_idx      = 0;
    uint8  index        = 0;
    uint8  slice_id     = 0;
    uint8  swap_flag[2] = {0};
    uint8  fix_swap     = FALSE;
    uint16 lport;
    uint16 start_lport  = 0;
    uint16 end_lport    = 0;
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    sys_datapath_lport_attr_t*    port_attr = NULL;

    for(index = 0; index < TSINGMA_HSS28G_NUM_PER_SLICE; index++)
    {
        hss_idx = TSINGMA_HSS15G_NUM_PER_SLICE + index;
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        if((1 == p_hss_vec->serdes_info[0].group) && (0 == p_hss_vec->serdes_info[1].group) && 
           (0 == p_hss_vec->serdes_info[2].group) && (1 == p_hss_vec->serdes_info[3].group))
        {
            swap_flag[index] = 1;
        }
        else if((0 == p_hss_vec->serdes_info[0].group) && (0 == p_hss_vec->serdes_info[1].group) && 
                (1 == p_hss_vec->serdes_info[2].group) && (1 == p_hss_vec->serdes_info[3].group))
        {
            swap_flag[index] = 0;
        }
        else
        {
            swap_flag[index] = 2;
        }
    }

    for(index = 0; index < TSINGMA_HSS28G_NUM_PER_SLICE; index++)
    {
        hss_idx = TSINGMA_HSS15G_NUM_PER_SLICE + index;
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        /*if txqm3 choose 13 14 15 17, port 60~63 have no swap config*/
        if((1 == index) && (2 == p_usw_datapath_master[lchip]->qm_choice.txqm3))
        {
            continue;
        }
        start_lport = (0 == index) ? 44 : 60;
        end_lport   = (0 == index) ? 47 : 63;
        for(lport = start_lport; lport <= end_lport; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if(NULL == port_attr)
            {
                continue;
            }
            if((CTC_CHIP_SERDES_LG_MODE == port_attr->pcs_mode) && (2 == swap_flag[index]))
            {
                SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "50G lane swap config error! lport %u, swap_flag %u\n", lport, swap_flag[index]);
                return CTC_E_INVALID_CONFIG;
            }

            port_attr->flag = swap_flag[index];
            if(1 != swap_flag[index])
            {
                continue;
            }
        }
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_fix(lchip, index, ((1 == swap_flag[index]) ? TRUE : FALSE), &fix_swap));
        CTC_ERROR_RETURN(_sys_tsingma_datapath_lane_swap_config(lchip, index, fix_swap));
    }
    
    return CTC_E_NONE;
}

static int32
_sys_tsingma_datapath_init_db(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg, uint8* p_cmu_id)
{
    uint8  serdes_id = 0;
    uint8  slice_id  = 0;
    uint16 lport     = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;

    p_usw_datapath_master[lchip]->core_plla = p_datapath_cfg->core_frequency_a;
    p_usw_datapath_master[lchip]->core_pllb = p_datapath_cfg->core_frequency_b;

    p_usw_datapath_master[lchip]->wlan_enable = p_datapath_cfg->wlan_enable;
    p_usw_datapath_master[lchip]->dot1ae_enable = p_datapath_cfg->dot1ae_enable;

    CTC_ERROR_RETURN(_sys_tsingma_datapath_select_qm(lchip, p_datapath_cfg->serdes));

    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_cmu_parser(lchip, p_datapath_cfg->serdes, p_cmu_id));

/*#ifdef EMULATION_ENV
#else*/
    for (serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        if (CTC_CHIP_SERDES_XSGMII_MODE == p_datapath_cfg->serdes[serdes_id].mode)  /* for powerdown use */
        {
            /*if this is the last lane of HSS, init soft table of hss pll info*/
            if(SERDES_JUDGE_LAST_LANE_IN_HSS(serdes_id))
            {
                sys_tsingma_datapath_init_hss_pll_soft_table(lchip, serdes_id);
            }
            continue;
        }
        CTC_ERROR_RETURN(sys_tsingma_datapath_build_serdes_info(lchip, serdes_id, p_datapath_cfg->serdes[serdes_id].mode, 
                                                                p_datapath_cfg->serdes[serdes_id].group,
                                                                p_datapath_cfg->serdes[serdes_id].is_dynamic, 
                                                                p_cmu_id[serdes_id], TRUE, 
                                                                p_datapath_cfg->serdes[serdes_id].is_xpipe, 
                                                                SYS_DATAPATH_SERDES_OVERCLOCKING_SPEED_NONE_MODE));
    }
/*#endif*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_update_glb_xpipe(lchip));

    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }

/*#ifdef EMULATION_ENV
            port_attr->mac_id = lport;
            port_attr->port_type = SYS_DMPS_NETWORK_PORT ;
            port_attr->chan_id = lport;
            port_attr->speed_mode = 0;
            port_attr->slice_id = slice_id;
            port_attr->pcs_mode = 0;
            port_attr->serdes_id = serdes_id;
            port_attr->serdes_num = 1;
            port_attr->is_serdes_dyn = 0;
            port_attr->flag = 0;
#endif*/

            if (port_attr->port_type == SYS_DMPS_RSV_PORT)
            {
                port_attr->mac_id = g_usw_mac_port_map[lport];
                port_attr->chan_id = g_usw_mac_port_map[lport];
                port_attr->serdes_num = 0;
                port_attr->serdes_id = 0xff;
                port_attr->speed_mode = CTC_PORT_SPEED_MAX;
            }            
        }
    }

    /*internal channel alloc */
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_OAM_CPU_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_OAM);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_OAM_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_ILOOP_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_ILOOP_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_ELOOP_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
        port_attr->mac_id = 0xff;
        port_attr->port_type = SYS_DMPS_ELOOP_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT0_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT1_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT2_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_DECRYPT3_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANDEC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT0_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT1_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT2_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_ENCRYPT3_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANENC_PORT : SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_CAWAP_REASSEMBLE_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->wlan_enable ? SYS_DMPS_WLANREASSEMBLE_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_MAC_DECRYPT_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->dot1ae_enable ? SYS_DMPS_MAC_DECRYPT_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;

        port_attr = sys_usw_datapath_get_port_capability(lchip, SYS_RSV_PORT_MAC_ENCRYPT_ID, slice_id);
        if (NULL == port_attr)
        {
            return CTC_E_INVALID_PARAM;
        }

        port_attr->chan_id = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
        port_attr->mac_id = 0xff;
        port_attr->port_type = p_usw_datapath_master[lchip]->dot1ae_enable ? SYS_DMPS_MAC_ENCRYPT_PORT: SYS_DMPS_RSV_PORT;
        port_attr->slice_id = slice_id;
        port_attr->serdes_num = 0;
    }

    p_usw_datapath_master[lchip]->oam_chan = MCHIP_CAP(SYS_CAP_CHANID_OAM);
    p_usw_datapath_master[lchip]->dma_chan = MCHIP_CAP(SYS_CAP_CHANID_DMA_RX0);
    p_usw_datapath_master[lchip]->iloop_chan = MCHIP_CAP(SYS_CAP_CHANID_ILOOP);
    p_usw_datapath_master[lchip]->eloop_chan = MCHIP_CAP(SYS_CAP_CHANID_ELOOP);
    p_usw_datapath_master[lchip]->cawapdec0_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0);
    p_usw_datapath_master[lchip]->cawapdec1_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1);
    p_usw_datapath_master[lchip]->cawapdec2_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2);
    p_usw_datapath_master[lchip]->cawapdec3_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3);
    p_usw_datapath_master[lchip]->cawapenc0_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0);
    p_usw_datapath_master[lchip]->cawapenc1_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1);
    p_usw_datapath_master[lchip]->cawapenc2_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2);
    p_usw_datapath_master[lchip]->cawapenc3_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3);
    p_usw_datapath_master[lchip]->cawapreassemble_chan = MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE);
    p_usw_datapath_master[lchip]->macdec_chan = MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT);
    p_usw_datapath_master[lchip]->macenc_chan = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
    p_usw_datapath_master[lchip]->elog_chan = MCHIP_CAP(SYS_CAP_CHANID_ELOG);
    p_usw_datapath_master[lchip]->qcn_chan = MCHIP_CAP(SYS_CAP_CHANID_QCN);

 /*for emulation*/
#if (0 == SDK_WORK_PLATFORM)
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        uint32 field_val = 0;
        uint32 cmd = 0;

        /*1. init channel-mac-lport mapping*/
        for (lport = 0; lport < SYS_PHY_PORT_NUM_PER_SLICE; lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                continue;
            }

            field_val = lport;
            cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t + slice_id), IpeHeaderAdjustPhyPortMap_localPhyPort_f);
            DRV_IOCTL(lchip, port_attr->chan_id, cmd, &field_val);
        }
    }
#endif
    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_init_aneth_tx(uint8 lchip)
{
    uint32 tbl_id = 0;
    uint32 fld_id = 0;
    uint32 cmd    = 0;
    uint32 step   = 0;
    uint32 i      = 0;
    uint32 tmp_val32  = 0;
    Hss12GMacroCfg0_m hss12G0_macro;
    Hss12GMacroCfg1_m hss12G1_macro;
    Hss12GMacroCfg2_m hss12G2_macro;
    Hss28GMacroCfg_m  hss28G_macro;

    tbl_id = Hss12GMacroCfg0_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G0_macro));
    step   = Hss12GMacroCfg0_cfgHssL1AnethAutoTxEn_f - Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f;
    for (i = 0; i < 8; i++)
    {   
        fld_id = Hss12GMacroCfg0_cfgHssL0AnethAutoTxEn_f + step*i;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss12G0_macro);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G0_macro));

    tbl_id = Hss12GMacroCfg1_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G1_macro));
    step   = Hss12GMacroCfg1_cfgHssL1AnethAutoTxEn_f - Hss12GMacroCfg1_cfgHssL0AnethAutoTxEn_f;
    for (i = 0; i < 8; i++)
    {
        fld_id = Hss12GMacroCfg1_cfgHssL0AnethAutoTxEn_f + step*i;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss12G1_macro);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G1_macro));

    tbl_id = Hss12GMacroCfg2_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G2_macro));
    step   = Hss12GMacroCfg2_cfgHssL1AnethAutoTxEn_f - Hss12GMacroCfg2_cfgHssL0AnethAutoTxEn_f;
    for (i = 0; i < 8; i++)
    {
        fld_id = Hss12GMacroCfg2_cfgHssL0AnethAutoTxEn_f + step*i;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss12G2_macro);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss12G2_macro));

    tbl_id = Hss28GMacroCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28G_macro));
    step   = Hss28GMacroCfg_cfgHssL1AnethAutoTxEn_f - Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f;
    for (i = 0; i < 8; i++)
    {
        fld_id = Hss28GMacroCfg_cfgHssL0AnethAutoTxEn_f + step*i;
        DRV_IOW_FIELD(lchip, tbl_id, fld_id, &tmp_val32, &hss28G_macro);
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hss28G_macro));
    
    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_init_set_serdes_cfg(uint8 lchip, uint8 cmu_id[])
{
    uint8 serdes_id = 0;
    uint8 lane_id = 0;
    uint8 hss_id = 0;
    uint8 hss_idx = 0;
    uint8 cmu     = 0;
    uint16 port_id = 0;
    uint8 serdes_mode[TSINGMA_SERDES_NUM_PER_SLICE] = {0};
    uint8 ovclk_flag[TSINGMA_SERDES_NUM_PER_SLICE] = {0};
    uint8 cmu_flag[3] = {1, 1, 1};
    sys_datapath_hss_attribute_t* p_hss_vec = NULL;
    ctc_chip_serdes_ffe_t ffe = {0};

    for(serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_SERDES_TO_HSS_INDEX(serdes_id);
        lane_id = SYS_DATAPATH_MAP_SERDES_TO_LANE_ID(serdes_id);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        serdes_mode[serdes_id] = p_hss_vec->serdes_info[lane_id].mode;
    }

    /* HSS12G config */
    for (hss_id = 0; hss_id < TSINGMA_HSS15G_NUM_PER_SLICE; hss_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_HSSID_TO_HSSIDX(hss_id, SYS_DATAPATH_HSS_TYPE_15G);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        /*1. set all serdes power on*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_power_reset(lchip, hss_id));
        
        /*2. set cmu config*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_set_hss12g_cmu_cfg(lchip, hss_id, serdes_mode, cmu_id, ovclk_flag, cmu_flag));

        /*3. set serdes lane config*/
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            serdes_id = hss_id*HSS12G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_lane_cfg(lchip, serdes_id, serdes_mode[serdes_id], cmu_id[serdes_id], ovclk_flag[serdes_id]));
        }

        /*4. release global reset of all hss*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_global_reset(lchip, hss_id, TRUE));
#if 0
        /* enable PRBS */
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            serdes_id = hss_id*HSS12G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, serdes_id, TRUE));
        }
#endif //zwh add differ from sdk5.5.5RC
        /*5. Additional cfg*/
        CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_additional_cmu_cfg(lchip, hss_id, serdes_mode, cmu_id, ovclk_flag, cmu_flag));

        /*6. 12g signal detect*/
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            /*91[5:4] r_rxei_filter_factor = 3*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x91, 0xcf, 3));
            /*48[3:0] cfg_alos_thr_gen1 = 0*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x48, 0xf0, 0));

            /* 7. 12g additional config */
            serdes_id = hss_id*HSS12G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(_sys_tsingma_datapath_12g_additional_lane_cfg(lchip, serdes_id, serdes_mode[serdes_id], ovclk_flag[serdes_id]));
            /* 7.1 12g DFE per lane reconfig */
            CTC_ERROR_RETURN(sys_tsingma_datapath_12g_dfe_lane_config(lchip, serdes_id, serdes_mode[serdes_id], TRUE));
        }

        /*8a. 12g cmu use internal 10K resistor (just in init procedure)*/
        for(cmu = 0; cmu < 3; cmu++)
        {
            /*cfg_ic2ip_n     0x1f[2]   1'b0*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, cmu, 0x1f, 0xfb, 0));
            /*cfg_vtune_sel   0x1f[3]   1'b1*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, cmu, 0x1f, 0xf7, 1));
            /*cfg_pll_tp_sel  0x00[5:4] 2'b11*/
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, cmu, 0x00, 0xcf, 3));
        }

        /* 8. 12g cmu reset */
        for(cmu = 0; cmu < 3; cmu++)
        {
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_cmu_reset(lchip, hss_id, cmu, TRUE));
        }

        /* 9. 12g lane reset */
        ffe.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
        ffe.coefficient[0] = 0;
        ffe.coefficient[1] = 192;
        ffe.coefficient[2] = 0;
        for(lane_id = 0; lane_id < HSS12G_LANE_NUM; lane_id++)
        {
            serdes_id = hss_id*HSS12G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_set_lane_reset(lchip, serdes_id, TRUE));
            /*set 12g ffe as reg default value to active user-define mode*/
            ffe.serdes_id = serdes_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_ffe_traditional(lchip, &ffe));
#if 1
            /* overwrite lane config */
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x0c, 0x00, 0x21));
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x0f, 0x00, 0x64));
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x1a, 0x00, 0x06));
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x90, 0x00, 0x30));
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x9e, 0x00, 0x36));
            CTC_ERROR_RETURN(sys_tsingma_datapath_write_hss12g_reg(lchip, hss_id, (lane_id + 3), 0x93, 0x00, 0x76));
#endif //zwh	 add differ from sdk5.5.5RC		
        }
        printf("_sys_tsingma_datapath_hss12g_wait_lane_ok hss_id = %d\n",hss_id);
        /* 10. check lane bring-up ok */
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss12g_wait_lane_ok(lchip, hss_id, 0xff));
    }

    /* HSS28G config */
    for (hss_id = 0; hss_id < TSINGMA_HSS28G_NUM_PER_SLICE; hss_id++)
    {
        hss_idx = SYS_DATAPATH_MAP_HSSID_TO_HSSIDX(hss_id, SYS_DATAPATH_HSS_TYPE_28G);
        p_hss_vec = ctc_vector_get(p_usw_datapath_master[lchip]->p_hss_vector, hss_idx);
        if (NULL == p_hss_vec)
        {
            continue;
        }
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_set_domain_reset(lchip, hss_id, SYS_TM_28G_ALL_RESET));
#if 0
        /* enable PRBS */
        for(lane_id = 0; lane_id < HSS28G_LANE_NUM; lane_id++)
        {
            serdes_id = SYS_DATAPATH_SERDES_28G_START_ID + hss_id*HSS28G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_prbs31_en(lchip, serdes_id, TRUE));
        }
#endif //zwh	add differ from sdk5.5.5RC	
        /* logic reset */
        CTC_ERROR_RETURN(_sys_tsingma_datapath_hss28g_set_domain_reset(lchip, hss_id, SYS_TM_28G_LOGIC_RESET));
        
        /*2. set cmu config    merge into additional cfg*/

        /*3. set serdes lane config*/
        for(lane_id = 0; lane_id < HSS28G_LANE_NUM; lane_id++)
        {
            serdes_id = SYS_DATAPATH_SERDES_28G_START_ID + hss_id*HSS28G_LANE_NUM + lane_id;
            CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_lane_cfg(lchip, serdes_id, serdes_mode[serdes_id], cmu_id[serdes_id], ovclk_flag[serdes_id]));
        }
    }

    for (port_id = 0; port_id < SYS_PHY_PORT_NUM_PER_SLICE; port_id++)
    {
        _sys_tsingma_datapath_28g_clk_tree_cfg(lchip, port_id);
    }

    /*4. 28g additional config */
    for (hss_id = 0; hss_id < TSINGMA_HSS28G_NUM_PER_SLICE; hss_id++)
    {
        CTC_ERROR_RETURN(_sys_tsingma_datapath_28g_additional_cfg(lchip, hss_id, serdes_mode[(0 == hss_id) ? 24 : 28], ovclk_flag[(0 == hss_id) ? 24 : 28]));
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_datapath_omitted_table_init(uint8 lchip)
{
    uint8  entry = 0;
    uint32 cmd   = 0;
    AutoGenPktCtl_m        agp_ctl;
    AutoGenPktPktCfg_m     agp_cfg;
    DsQMgrOobFcGrpState_m  dsg_sta;
    DsQMgrPfcState_m       dsp_sta;
    BufRetrvPktConfigMem_m br_cfg;
    BufRetrvPktStatusMem_m br_sta;
    BufRetrvBkupPtrInfo_m  br_inf;

    sal_memset(&agp_ctl, 0, sizeof(AutoGenPktCtl_m));
    sal_memset(&agp_cfg, 0, sizeof(AutoGenPktPktCfg_m));
    sal_memset(&dsg_sta, 0, sizeof(DsQMgrOobFcGrpState_m));
    sal_memset(&dsp_sta, 0, sizeof(DsQMgrPfcState_m));
    sal_memset(&br_cfg, 0, sizeof(BufRetrvPktConfigMem_m));
    sal_memset(&br_sta, 0, sizeof(BufRetrvPktStatusMem_m));
    sal_memset(&br_inf, 0, sizeof(BufRetrvBkupPtrInfo_m));

    for(entry = 0; entry < 8; entry++)
    {
        cmd = DRV_IOW(AutoGenPktCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &agp_ctl);
        cmd = DRV_IOW(AutoGenPktPktCfg_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &agp_cfg);
    }
    
    for(entry = 0; entry < 32; entry++)
    {
        cmd = DRV_IOW(DsQMgrOobFcGrpState_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &dsg_sta);
    }

    for(entry = 0; entry < 64; entry++)
    {
        cmd = DRV_IOW(DsQMgrPfcState_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &dsp_sta);
    }

    for(entry = 0; entry < 84; entry++)
    {
        cmd = DRV_IOW(BufRetrvPktConfigMem_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &br_cfg);
        cmd = DRV_IOW(BufRetrvPktStatusMem_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &br_sta);
        cmd = DRV_IOW(BufRetrvBkupPtrInfo_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, entry, cmd, &br_inf);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_datapath_xpipe_init(uint8 lchip)
{
    uint8  slice_id = 0;
    uint16 lport = 0;
    uint16 tmp_port = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                return CTC_E_INVALID_PARAM;
            }
            for (tmp_port = 0; tmp_port < SYS_USW_XPIPE_PORT_NUM; tmp_port++)
            {
                if ((lport == g_tsingma_xpipe_mac_mapping[tmp_port].emac_id) && port_attr->xpipe_en)
                {
                    CTC_ERROR_RETURN(_sys_tsingma_mac_set_xpipe_en(lchip, lport, TRUE));
                }
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_init_polarity_cfg(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8 serdes_id = 0;
    ctc_chip_serdes_polarity_t serdes_polarity = {0};

    for (serdes_id = 0; serdes_id < TSINGMA_SERDES_NUM_PER_SLICE; serdes_id++)
    {
        serdes_polarity.serdes_id = serdes_id;

        /*set RX polarity*/
        serdes_polarity.dir = 0;
        serdes_polarity.polarity_mode= (p_datapath_cfg->serdes[serdes_id].rx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &serdes_polarity));
        sal_task_sleep(1);

        /*set TX polarity*/
        serdes_polarity.dir = 1;
        serdes_polarity.polarity_mode = (p_datapath_cfg->serdes[serdes_id].tx_polarity)?1:0;
        CTC_ERROR_RETURN(sys_tsingma_datapath_set_serdes_polarity(lchip, &serdes_polarity));
        sal_task_sleep(1);
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_init_hss_unit_reset(uint8 lchip)
{
    uint8 index;

    /*release Hss12GUnitReset */
    for(index = 0; index < 4; index++)
    {
        drv_usw_chip_write(lchip, HSS_12G_UNIT_RESET0_ADDR + 4*index, 0);
        drv_usw_chip_write(lchip, HSS_12G_UNIT_RESET1_ADDR + 4*index, 0);
        drv_usw_chip_write(lchip, HSS_12G_UNIT_RESET2_ADDR + 4*index, 0);
    }

    /*release Hss28GUnitReset */
    drv_usw_chip_write(lchip, HSS_28G_UNIT_RESET_ADDR, 0);
    drv_usw_chip_write(lchip, HSS_28G_UNIT_RESET_ADDR + 4, 0xff000000);

    return CTC_E_NONE;
}

int32
sys_tsingma_datapath_init(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg)
{
    uint8  mac_id   = 0;
    uint8  slice_id = 0;
    uint16 lport    = 0;
    uint32 cmd      = 0;
    uint8  cmu_id[TSINGMA_SERDES_NUM_PER_SLICE];
    NetTxPortDynamicInfo_m     dyn_info;
    sys_datapath_lport_attr_t* port_attr = NULL;

    /*init port attribute to rsv port default*/
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM); lport++)
        {
            port_attr = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
            if (NULL == port_attr)
            {
                return CTC_E_INVALID_PARAM;
            }
            port_attr->port_type =  SYS_DMPS_RSV_PORT;
        }
    }

    /*0. datapath_cfg.txt param check*/
    CTC_ERROR_RETURN(_sys_tsingma_datapath_param_check(lchip, p_datapath_cfg));

    /*1. do module release */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_core_clock_init(lchip, p_datapath_cfg->core_frequency_a));

    CTC_ERROR_RETURN(_sys_tsingma_datapath_sup_init(lchip));
    
    CTC_ERROR_RETURN(_sys_tsingma_datapath_omitted_table_init(lchip));

    /*0. prepare data base*/
    sal_memset(cmu_id, 0xff, TSINGMA_SERDES_NUM_PER_SLICE * sizeof(uint8));
    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_db(lchip, p_datapath_cfg, cmu_id));

    /* Write RlmMacMuxModeCfg and RlmHsMuxModeCfg before mac operation */
    CTC_ERROR_RETURN(sys_tsingma_datapath_set_txqm_mode_config(lchip));

    CTC_ERROR_RETURN(sys_usw_datapath_module_init(lchip));

    /*init NetTxPortDynamicInfo0 default*/
    sal_memset(&dyn_info, 0, sizeof(NetTxPortDynamicInfo_m));
    for (slice_id = 0; slice_id < SYS_MAX_LOCAL_SLICE_NUM; slice_id++)
    {
        for (mac_id = 0; mac_id < SYS_PHY_PORT_NUM_PER_SLICE; mac_id++)
        {
            cmd = DRV_IOW((NetTxPortDynamicInfo_t+slice_id), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, mac_id, cmd, &dyn_info);
        }
    }

    /*2. do serdes init */
    CTC_ERROR_RETURN(sys_tsingma_datapath_init_set_serdes_cfg(lchip, cmu_id));

    /*3. release hss unit reset */
    CTC_ERROR_RETURN(sys_tsingma_datapath_init_hss_unit_reset(lchip));

    /* disable cfgHssL0AnethAutoTxEn */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_aneth_tx(lchip));

    /*4. lane swap init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_lane_swap(lchip));

    /*5. set serdes polarity*/
    CTC_ERROR_RETURN(sys_tsingma_datapath_init_polarity_cfg(lchip, p_datapath_cfg));

    /*6. do datapath init */
    CTC_ERROR_RETURN(sys_tsingma_datapath_datapth_init(lchip));

    /*7 do datapath xpipe init */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_xpipe_init(lchip));

    /*8. init set SerDes Power Down */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_set_serdes_powerdown(lchip));

    /*9. load HSS28G SerDes firmware */
    CTC_ERROR_RETURN(_sys_tsingma_datapath_init_hss28g_firmware(lchip));

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_datapath_wb_restore(lchip));
    }

    return CTC_E_NONE;
}

/*
set VGA/EQR/EQC
*/
int32
sys_tsingma_serdes_hss12g_set_ctle_value(uint8 serdes_id, uint8 ctle_type, uint8 val_u8)
{
    uint8  lchip     = 0;
    uint8  step      = Hss12GLaneRxEqCfg0_cfgHssL1Pcs2PmaVgaCtrl_f - Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f;
    uint8  lane_id   = serdes_id % HSS12G_LANE_NUM;
    uint8  hss_id    = serdes_id / HSS12G_LANE_NUM;
    uint32 cmd       = 0;
    uint32 tbl_id    = Hss12GLaneRxEqCfg0_t + hss_id * (Hss12GLaneRxEqCfg1_t - Hss12GLaneRxEqCfg0_t);
    uint32 fld_id    = step * lane_id;
    uint32 value     = val_u8;
    int32  ret = 0;
    uint32 lock_id   = DRV_MCU_LOCK_RXEQ_CFG;
    Hss12GLaneRxEqCfg0_m rx_eq_cfg;

    ret = drv_usw_mcu_lock(lchip, lock_id, hss_id);
    if(0 != ret)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Get lock error (RXEQ)! hss_id=%u, lane_id=%u\n", hss_id, lane_id);
        return CTC_E_HW_TIME_OUT;
    }

    switch(ctle_type)
    {
        case 0:
            fld_id += Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaVgaCtrl_f;
            break;
        case 1:
            fld_id += Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqcForce_f;
            break;
        case 2:
        default:
            fld_id += Hss12GLaneRxEqCfg0_cfgHssL0Pcs2PmaEqRes_f;
            break;
    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg));
    DRV_IOW_FIELD(lchip, tbl_id, fld_id, &value, &rx_eq_cfg);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_eq_cfg));

    ret = drv_usw_mcu_unlock(lchip, lock_id, hss_id);
    if (0 != ret)
    {
        SYS_DATAPATH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Get unlock error (RXEQ)! hss_id=%u, lane_id=%u\n", hss_id, lane_id);
        return CTC_E_HW_TIME_OUT;
    }
    
    return CTC_E_NONE;
}


