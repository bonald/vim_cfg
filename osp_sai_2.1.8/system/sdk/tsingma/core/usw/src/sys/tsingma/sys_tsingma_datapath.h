/**
 @file sys_tsingma_datapath.h

 @date 2017-12-28

 @version v2.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_TSINGMA_DATAPATH_H
#define _SYS_TSINGMA_DATAPATH_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "sys_usw_dmps.h"
#include "sys_usw_common.h"
#include "sys_usw_datapath.h"

/***************************************************************
 *
 * Macro definition
 *
 ***************************************************************/
#define SYS_DATAPATH_IS_SERDES_1_PORT_1(dst_mode) ((CTC_CHIP_SERDES_XFI_MODE == dst_mode) || (CTC_CHIP_SERDES_2DOT5G_MODE == dst_mode) || (CTC_CHIP_SERDES_XXVG_MODE == dst_mode) ||\
                                                   (CTC_CHIP_SERDES_SGMII_MODE == dst_mode) || (CTC_CHIP_SERDES_100BASEFX_MODE == dst_mode) || (CTC_CHIP_SERDES_USXGMII0_MODE == dst_mode))
#define SYS_DATAPATH_IS_SERDES_4_PORT_1(mode) ((CTC_CHIP_SERDES_XAUI_MODE == mode) || (CTC_CHIP_SERDES_DXAUI_MODE == mode) ||\
                                               (CTC_CHIP_SERDES_XLG_MODE == mode) ||(CTC_CHIP_SERDES_CG_MODE == mode))
#define SYS_DATAPATH_GET_HSS28G_LEVEL(mode, level) do {\
        switch(mode) {\
            case CTC_CHIP_SERDES_SGMII_MODE:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_0;\
                break;\
            case CTC_CHIP_SERDES_2DOT5G_MODE:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_1;\
                break;\
            case CTC_CHIP_SERDES_XAUI_MODE:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_2;\
                break;\
            case CTC_CHIP_SERDES_XFI_MODE:\
            case CTC_CHIP_SERDES_XLG_MODE:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_3;\
                break;\
            case CTC_CHIP_SERDES_XXVG_MODE:\
            case CTC_CHIP_SERDES_LG_MODE:\
            case CTC_CHIP_SERDES_CG_MODE:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_4;\
                break;\
            default:\
                level = SYS_DATAPATH_28G_CLK_LEVEL_BUTT;\
                break;}\
    }while(0)
#define SYS_DATAPATH_GET_PORT_NUM_PER_QM(mode, port_num) do {\
        switch(mode)\
        {\
            case CTC_CHIP_SERDES_USXGMII1_MODE:\
            case CTC_CHIP_SERDES_QSGMII_MODE:\
            case CTC_CHIP_SERDES_XFI_MODE:\
            case CTC_CHIP_SERDES_SGMII_MODE:\
            case CTC_CHIP_SERDES_2DOT5G_MODE:\
            case CTC_CHIP_SERDES_XXVG_MODE:\
            case CTC_CHIP_SERDES_100BASEFX_MODE:\
            case CTC_CHIP_SERDES_USXGMII0_MODE:\
                port_num = 4;\
                break;\
            case CTC_CHIP_SERDES_USXGMII2_MODE:\
            case CTC_CHIP_SERDES_LG_MODE:\
                port_num = 2;\
                break;\
            case CTC_CHIP_SERDES_NONE_MODE:\
                port_num = 0;\
                break;\
            default:\
                port_num = 1;\
                break;\
        }\
    }while(0)
/*TRUE - null port  FALSE - valid port*/
#define SYS_DATAPATH_JUDGE_NULL_PORT(lchip, mac_id, port_attr) \
    ((CTC_E_INVALID_CONFIG == sys_usw_mac_get_port_capability(lchip, mac_id, (port_attr)) ||\
       NULL == (*(port_attr)) ||\
       SYS_DMPS_RSV_PORT == (*(port_attr))->port_type ||\
       CTC_CHIP_SERDES_NONE_MODE == (*(port_attr))->pcs_mode) ? TRUE : FALSE)
#define SYS_DATAPATH_GET_SERDES_NUM_PER_PORT(mode, serdes_num) do {\
        switch(mode)\
        {\
            case CTC_CHIP_SERDES_XLG_MODE:\
            case CTC_CHIP_SERDES_XAUI_MODE:\
            case CTC_CHIP_SERDES_DXAUI_MODE:\
            case CTC_CHIP_SERDES_CG_MODE:\
                serdes_num = 4;\
                break;\
            case CTC_CHIP_SERDES_LG_MODE:\
                serdes_num = 2;\
                break;\
            case CTC_CHIP_SERDES_NONE_MODE:\
                serdes_num = 0;\
                break;\
            default:\
                serdes_num = 1;\
                break;\
        }\
    }while(0)
#define SYS_TSINGMA_EPESCH_MAX_CHANNEL 75
#define SYS_TSINGMA_MAX_CMU_NUM_HSS    3
#define SYS_TSINGMA_12G_GET_CMU_TYPE(mode, ovclk, type) do {\
    if(ovclk) {type = SYS_DATAPATH_CMU_TYPE_4;}\
    else{\
        switch(mode)\
        {\
            case CTC_CHIP_SERDES_XLG_MODE:\
            case CTC_CHIP_SERDES_XFI_MODE:\
            case CTC_CHIP_SERDES_USXGMII0_MODE:\
            case CTC_CHIP_SERDES_USXGMII1_MODE:\
            case CTC_CHIP_SERDES_USXGMII2_MODE:\
                type = SYS_DATAPATH_CMU_TYPE_1;\
                break;\
            case CTC_CHIP_SERDES_SGMII_MODE:\
            case CTC_CHIP_SERDES_100BASEFX_MODE:\
            case CTC_CHIP_SERDES_QSGMII_MODE:\
                type = SYS_DATAPATH_CMU_TYPE_2;\
                break;\
            case CTC_CHIP_SERDES_2DOT5G_MODE:\
            case CTC_CHIP_SERDES_XAUI_MODE:\
            case CTC_CHIP_SERDES_DXAUI_MODE:\
                type = SYS_DATAPATH_CMU_TYPE_3;\
                break;\
            default:\
                type = SYS_DATAPATH_CMU_IDLE;\
                break;\
        }\
    }\
}while(0)
#define SYS_TSINGMA_28G_GET_CMU_TYPE(mode, ovclk, type) do {\
    if(ovclk) {\
        switch(mode)\
        {\
            case CTC_CHIP_SERDES_XLG_MODE:\
            case CTC_CHIP_SERDES_XFI_MODE:\
                if(1 == ovclk)\
                {\
                    type = SYS_DATAPATH_28G_CMU_TYPE_6;\
                }\
                else if(2 == ovclk)\
                {\
                    type = SYS_DATAPATH_28G_CMU_TYPE_8;\
                }\
                else if(4 == ovclk)\
                {\
                    type = SYS_DATAPATH_28G_CMU_TYPE_9;\
                }\
                break;\
            case CTC_CHIP_SERDES_XXVG_MODE:\
            case CTC_CHIP_SERDES_LG_MODE:\
            case CTC_CHIP_SERDES_CG_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_5;\
                break;\
            default:\
                type = SYS_DATAPATH_CMU_IDLE;\
                break;\
        }\
    }\
    else{\
        switch(mode)\
        {\
            case CTC_CHIP_SERDES_XXVG_MODE:\
            case CTC_CHIP_SERDES_LG_MODE:\
            case CTC_CHIP_SERDES_CG_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_1;\
                break;\
            case CTC_CHIP_SERDES_XLG_MODE:\
            case CTC_CHIP_SERDES_XFI_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_2;\
                break;\
            case CTC_CHIP_SERDES_SGMII_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_3;\
                break;\
            case CTC_CHIP_SERDES_2DOT5G_MODE:\
            case CTC_CHIP_SERDES_XAUI_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_4;\
                break;\
            case CTC_CHIP_SERDES_DXAUI_MODE:\
                type = SYS_DATAPATH_28G_CMU_TYPE_7;\
                break;\
            default:\
                type = SYS_DATAPATH_CMU_IDLE;\
                break;\
        }\
    }\
}while(0)
#define SYS_TSINGMA_EPESCH_MAX_CHANNEL 75




enum sys_tm_datapath_bit_width_e
{
    SYS_TM_DATAPATH_BIT_WIDTH_NULL,
    SYS_TM_DATAPATH_BIT_WIDTH_10BIT,
    SYS_TM_DATAPATH_BIT_WIDTH_16BIT,
    SYS_TM_DATAPATH_BIT_WIDTH_20BIT,
    SYS_TM_DATAPATH_BIT_WIDTH_32BIT,
    SYS_TM_DATAPATH_BIT_WIDTH_64BIT
};
typedef enum sys_tm_datapath_bit_width_e sys_tm_datapath_bit_width_t;

enum sys_tm_datapath_serdes_div_e
{
    SYS_TM_DATAPATH_SERDES_DIV_NULL,
    SYS_TM_DATAPATH_SERDES_DIV_FULL,
    SYS_TM_DATAPATH_SERDES_DIV_HALF,
    SYS_TM_DATAPATH_SERDES_DIV_QUAD,
    SYS_TM_DATAPATH_SERDES_DIV_OCTL,
    SYS_TM_DATAPATH_SERDES_DIV_HEXL
};
typedef enum sys_datapath_serdes_div_e sys_tm_datapath_serdes_div_t;

enum sys_tm_datapath_lane_speed_e
{
    SYS_TM_LANE_SPD_NULL,
    SYS_TM_LANE_10D3125G,
    SYS_TM_LANE_12D5G,
    SYS_TM_LANE_5G,
    SYS_TM_LANE_1D25G,
    SYS_TM_LANE_3D125G,
    SYS_TM_LANE_6D25G,
    SYS_TM_LANE_25D78125G,
    SYS_TM_LANE_28D125G,
    SYS_TM_LANE_11D40625G,
};
typedef enum sys_tm_datapath_lane_speed_e sys_tm_datapath_lane_speed_t;

enum sys_tm_datapath_serdes_ctle_e
{
    SYS_TM_SERDES_CTLE_VGA,
    SYS_TM_SERDES_CTLE_EQC,
    SYS_TM_SERDES_CTLE_EQR,
    SYS_TM_SERDES_CTLE_BUTT
};
typedef enum sys_tm_datapath_serdes_ctle_e sys_tm_datapath_serdes_ctle_t;

enum sys_tm_datapath_serdes_dfe_e
{
    SYS_TM_SERDES_DFE_H1,
    SYS_TM_SERDES_DFE_H2,
    SYS_TM_SERDES_DFE_H3,
    SYS_TM_SERDES_DFE_H4,
    SYS_TM_SERDES_DFE_H5,
    SYS_TM_SERDES_DFE_BUTT
};
typedef enum sys_tm_datapath_serdes_dfe_e sys_tm_datapath_serdes_dfe_t;

enum sys_tm_datapath_serdes_tx_adj_e
{
    SYS_TM_SERDES_TX_MARGIN,
    SYS_TM_SERDES_TX_ADV,
    SYS_TM_SERDES_TX_DLY,
    SYS_TM_SERDES_TX_BUTT
};
typedef enum sys_tm_datapath_serdes_tx_adj_e sys_tm_datapath_serdes_tx_adj_t;

enum sys_tm_datapath_serdes_reset_type_e
{
    SYS_TM_28G_LOGIC_RESET,
    SYS_TM_28G_ALL_RESET,
    SYS_TM_28G_REGISTER_RESET,
    SYS_TM_28G_CPU_RESET,
    SYS_TM_28G_RESET_BUTT
};
typedef enum sys_tm_datapath_serdes_reset_type_e sys_tm_datapath_serdes_reset_type_t;

/***************************************************************
 *
 * Function declaration
 *
 ***************************************************************/
int32 _sys_tsingma_mac_get_fec_en(uint8 lchip, uint32 lport, uint32* p_value);

extern int32 sys_tsingma_datapath_set_serdes_polarity(uint8 lchip, void* p_data);
extern int32 sys_tsingma_datapath_set_dynamic_switch(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);
extern int32 sys_tsingma_datapath_set_serdes_dynamic_switch(uint8 lchip, uint8 serdes_id, uint8 mode, uint16 overclocking_speed);
extern int32 sys_tsingma_datapath_set_serdes_prbs_tx(uint8 lchip, void* p_data);
extern int32 sys_tsingma_datapath_get_serdes_prbs_status(uint8 lchip,uint8 serdes_id,uint8* enable,uint8* hss_id,uint8* hss_type,uint8* lane_id);
extern int32 sys_tsingma_datapath_set_serdes_ffe(uint8 lchip, void* p_data);
extern int32 sys_tsingma_datapath_set_dfe_reset(uint8 lchip, uint8 serdes_id, uint8 enable);
extern uint32 sys_tsingma_datapath_get_serdes_sigdet(uint8 lchip, uint16 serdes_id);
extern int32 sys_tsingma_serdes_set_link_training_en(uint8 lchip, uint16 serdes_id, bool enable);
extern int32 sys_tsingma_datapath_get_serdes_ffe(uint8 lchip, uint8 serdes_id, uint16* coefficient, uint8 ffe_mode, uint8* status);
extern int32 sys_tsingma_datapath_get_serdes_loopback(uint8 lchip, void* p_data);
extern int16 sys_tsingma_datapath_set_serdes_auto_neg_en(uint8 lchip, uint16 serdes_id, uint32 enable);
extern int32 sys_tsingma_serdes_get_link_training_status(uint8 lchip, uint16 serdes_id, uint16* p_value);
extern int32 sys_tsingma_datapath_set_dfe_en(uint8 lchip, uint8 serdes_id, uint8 enable);
extern int32 sys_tsingma_datapath_link_training_cfg(uint8 lchip, uint32 gport);
extern int32 sys_tsingma_mcu_set_cl72_enable(uint8 lchip, uint8 hss_id, uint8 lane_id, bool enable);
extern int32 sys_tsingma_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32 sys_tsingma_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32 sys_tsingma_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);
extern int32 sys_tsingma_datapath_init(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg);
extern int32 sys_tsingma_datapath_read_hss12g_reg(uint8 lchip, uint8 hss_id, uint8 acc_id, uint8 addr, uint8 mask, uint8* p_data);
extern int32 sys_tsingma_datapath_write_hss12g_reg(uint8 lchip, uint8 hss_id, uint8 acc_id, uint8 addr, uint8 mask, uint8 data);
extern int32 sys_tsingma_datapath_read_hss28g_reg(uint8 lchip, uint8 acc_id, uint16 addr, uint16 mask, uint16* p_data);
extern int32 sys_tsingma_datapath_write_hss28g_reg(uint8 lchip, uint8 acc_id, uint16 addr, uint16 mask, uint16 data);
extern int32 _sys_tsingma_datapath_hss28g_set_domain_reset(uint8 lchip, uint8 hss_id, uint8 rst_type);
extern int32 sys_tsingma_datapath_set_xpipe_en(uint8 lchip, uint16 lport, uint8 enable);
#ifdef __cplusplus
}
#endif

#endif

