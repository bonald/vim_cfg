/**
 @file sys_usw_mac.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-9

 @version v2.0

*/

#ifndef _SYS_USW_MAC_H
#define _SYS_USW_MAC_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_vlan.h"
#include "ctc_port.h"
#include "ctc_security.h"
#include "ctc_interrupt.h"
#include "sys_usw_datapath.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define SYS_USW_MIN_FRAMESIZE_MIN_VALUE        18
#define SYS_USW_MIN_FRAMESIZE_MAX_VALUE        127 /*GB is 64, 127 for cover emu test */
#define SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE    64
#define SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE    20

#define SYS_USW_MAX_FRAMESIZE_MIN_VALUE        128
#define SYS_USW_MAX_FRAMESIZE_MAX_VALUE        16127
#define SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE    9600u
#define SYS_USW_MAX_FRAMESIZE_TX_INDEX    16

#define SYS_PORT_MAX_LOG_NUM 32

#define MAC_PORT_NUM_PER_TXQM 16
#define MAC_PORT_NUM_PER_QM   4
#define PORT_RECOVER_CHECK_ROUND 3

#define SYS_USW_XPIPE_PORT_NUM  24

/*base page 0-31 */
#define SYS_PORT_CL73_NEXT_PAGE         (1<<15)/**< IEEE: NP */
#define SYS_PORT_CL73_10GBASE_KR        (1<<23)/**< IEEE: A2 */
#define SYS_PORT_CL73_40GBASE_KR4       (1<<24)/**< IEEE: A3 */
#define SYS_PORT_CL73_40GBASE_CR4       (1<<25)/**< IEEE: A4 */
#define SYS_PORT_CL73_100GBASE_KR4      (1<<28)/**< IEEE: A7 */
#define SYS_PORT_CL73_100GBASE_CR4      (1<<29)/**< IEEE: A8 */
#define SYS_PORT_CL73_25GBASE_KR_S      (1<<30)/**< IEEE: A9 */
#define SYS_PORT_CL73_25GBASE_CR_S      (1<<30)/**< IEEE: A9 */
#define SYS_PORT_CL73_25GBASE_KR        (1<<31)/**< IEEE: A10 */
#define SYS_PORT_CL73_25GBASE_CR        (1<<31)/**< IEEE: A10 */
/*base page 32-47*/
#define SYS_PORT_CL73_25G_RS_FEC_REQ       (1<<12)/**< IEEE: A23, 32+12 CL91 Req */
#define SYS_PORT_CL73_25G_BASER_FEC_REQ     (1<<13)/**< IEEE: A24, 32+13 CL74-25KRS Req */
#define SYS_PORT_CL73_FEC_SUP            (1<<14)/**< IEEE: F0, 32+14 CL74 Sup */
#define SYS_PORT_CL73_FEC_REQ            (1<<15)/**< IEEE: F1, 32+15 CL74 Req */
/*Next page1 0-31*/
#define SYS_PORT_CSTM_25GBASE_KR1        (1<<20) /**< [D2] Consortium mode 25GBase-KR1 ability */
#define SYS_PORT_CSTM_25GBASE_CR1        (1<<21) /**< [D2] Consortium mode 25GBase-CR1 ability */
#define SYS_PORT_CSTM_50GBASE_KR2        (1<<24) /**< [D2] Consortium mode 50GBase-KR2 ability */
#define SYS_PORT_CSTM_50GBASE_CR2        (1<<25) /**< [D2] Consortium mode 50GBase-CR2 ability */
/*Next page1 32-47*/
#define SYS_PORT_CSTM_CL91_FEC_SUP       (1<<8)  /**< [D2] F1: Consortium mode RS-FEC ability */
#define SYS_PORT_CSTM_CL74_FEC_SUP       (1<<9)  /**< [D2] F2: Consortium mode BASE-R ability */
#define SYS_PORT_CSTM_CL91_FEC_REQ       (1<<10) /**< [D2] F3: Consortium mode RS-FEC requested */
#define SYS_PORT_CSTM_CL74_FEC_REQ      (1<<11) /**< [D2] F4: Consortium mode BASE-R requested */



#define SYS_MAC_INIT_CHECK()            \
    do                                   \
    {                                    \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_mac_master[lchip]){      \
            return CTC_E_NOT_INIT; }     \
    }                                    \
    while (0)
#define CTC_ERROR_RETURN_WITH_MAC_UNLOCK(op) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            sal_mutex_unlock(p_usw_mac_master[lchip]->p_mac_mutex); \
            return (rv); \
        } \
    } while (0)
#define SYS_MAC_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(port, mac, MAC_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_CL73_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(port, cl73, CL73_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_MAC_JUDGE_HSS28G_BY_PCS_IDX(pcs_idx) ((pcs_idx) > 5 ? TRUE : FALSE)
#define SYS_MAC_GET_SHARED_FEC_IDX_BY_MII_ID(mii_idx) ((12 > mii_idx) ? (mii_idx / 4) : (mii_idx - 9))
#define SYS_MAC_JUDGE_PORT_HSS28G_BY_PORT_ID(port_id) (((44 <= port_id && 47 >= port_id) || \
                                                         ((2 != p_usw_datapath_master[lchip]->qm_choice.txqm3) && \
                                                          (60 <= port_id && 63 >= port_id))) ? TRUE : FALSE)


enum sys_port_pause_frame_type_e
{
    SYS_PORT_PAUSE_FRAME_TYPE_NORMAL = 0x1,
    SYS_PORT_PAUSE_FRAME_TYPE_PFC = 0x2,
    SYS_PORT_PAUSE_FRAME_TYPE_NUM = 2
};
typedef enum sys_port_pause_frame_type_e sys_port_pause_frame_type_t;

enum sys_port_mac_property_e
{
    SYS_PORT_MAC_PROPERTY_PADDING,
    SYS_PORT_MAC_PROPERTY_PREAMBLE,
    SYS_PORT_MAC_PROPERTY_CHKCRC,
    SYS_PORT_MAC_PROPERTY_STRIPCRC,
    SYS_PORT_MAC_PROPERTY_APPENDCRC,
    SYS_PORT_MAC_PROPERTY_ENTXERR,
    SYS_PORT_MAC_PROPERTY_CODEERR,
    SYS_PORT_MAC_PROPERTY_NUM
};
typedef enum sys_port_mac_property_e sys_port_mac_property_t;

enum sys_port_cl72_status_e
{
    SYS_PORT_CL72_DISABLE,                    /* cl72 training disable */
    SYS_PORT_CL72_PROGRESS,                   /* cl72 training in progress */
    SYS_PORT_CL72_FAIL,                       /* cl72 training fail */
    SYS_PORT_CL72_OK,                         /* cl72 training ok */
};
typedef enum sys_port_cl72_status_e sys_port_cl72_status_t;

enum sys_port_recover_e
{
    SYS_PORT_CODE_ERROR = (1 << 0),
    SYS_PORT_NOT_UP     = (1 << 1),
    SYS_PORT_NO_RANDOM_FLAG = (1 << 2),
    SYS_PORT_RECOVER_MAX,
};
typedef enum sys_port_recover_e sys_port_recover_t;

enum sys_port_mac_status_type_e
{
    SYS_PORT_MAC_STATUS_TYPE_LINK,
    SYS_PORT_MAC_STATUS_TYPE_CODE_ERR
};
typedef enum sys_port_mac_status_type_e sys_port_mac_status_type_t;

enum sys_port_framesize_type_e
{
    SYS_PORT_FRAMESIZE_TYPE_MIN,
    SYS_PORT_FRAMESIZE_TYPE_MAX,
    SYS_PORT_FRAMESIZE_TYPE_NUM
};
typedef enum sys_port_framesize_type_e sys_port_framesize_type_t;


struct sys_mac_prop_s
{
    uint32 port_mac_en        :1;
    uint32 speed_mode         :3;
    uint32 cl73_enable        :1;   /* 0-dis, 1-en*/
    uint32 old_cl73_status    :2;
    uint32 link_intr_en       :1;
    uint32 link_status        :1;
    uint32 unidir_en          :1;
    uint32 rx_rst_hold        :1;
    uint32 old_phy_ready      :1;
    uint32 rsv                :20;
};
typedef struct sys_mac_prop_s sys_mac_prop_t;


struct sys_usw_scan_log_s
{
    uint8 valid;
    uint32 gport;
    uint8 type;  /*bit1:link down, bit0:code error*/
    char time_str[50];
};
typedef struct sys_usw_scan_log_s sys_usw_scan_log_t;

struct sys_mac_master_s
{
    sys_mac_prop_t*        mac_prop;
    sal_mutex_t*           p_mac_mutex;

    sal_task_t*            p_monitor_scan;
    sys_usw_scan_log_t   scan_log[SYS_PORT_MAX_LOG_NUM];
    uint8                  cur_log_index;
    uint8                 polling_status;
};
typedef struct sys_mac_master_s sys_mac_master_t;

extern sys_mac_master_t* p_usw_mac_master[CTC_MAX_LOCAL_CHIP_NUM];
#define MAC_LOCK \
    if (p_usw_mac_master[lchip]->p_mac_mutex) sal_mutex_lock(p_usw_mac_master[lchip]->p_mac_mutex)
#define MAC_UNLOCK \
    if (p_usw_mac_master[lchip]->p_mac_mutex) sal_mutex_unlock(p_usw_mac_master[lchip]->p_mac_mutex)


#define GET_SERDES(mac_id, serdes_id, idx, lg_flag, mode, real_serdes) \
    if ((1 == lg_flag) && (CTC_CHIP_SERDES_LG_MODE == mode))           \
    {                                                                  \
        if (0 == mac_id % 4)                                          \
        {                                                             \
            real_serdes = serdes_id + idx;                            \
        }                                                             \
        else                                                          \
        {                                                             \
            if (0 == idx)                                             \
            {                                                         \
                real_serdes = serdes_id;                              \
            }                                                         \
            else                                                      \
            {                                                         \
                real_serdes = serdes_id + 3;                          \
            }                                                         \
        }                                                             \
    }                                                                 \
    else                                                              \
    {                                                                 \
        real_serdes = serdes_id + idx;                                \
    }

#define GET_QSGMII_TBL(mac_id, pcs_id, tbl_id)\
    do{                                \
        uint32 tbl_arr[4*12] =         \
            { QsgmiiPcsStatus00_t, QsgmiiPcsStatus10_t, QsgmiiPcsStatus20_t, QsgmiiPcsStatus30_t, \
              QsgmiiPcsStatus01_t, QsgmiiPcsStatus11_t, QsgmiiPcsStatus21_t, QsgmiiPcsStatus31_t, \
              QsgmiiPcsStatus02_t, QsgmiiPcsStatus12_t, QsgmiiPcsStatus22_t, QsgmiiPcsStatus32_t, \
              QsgmiiPcsStatus03_t, QsgmiiPcsStatus13_t, QsgmiiPcsStatus23_t, QsgmiiPcsStatus33_t, \
              QsgmiiPcsStatus04_t, QsgmiiPcsStatus14_t, QsgmiiPcsStatus24_t, QsgmiiPcsStatus34_t, \
              QsgmiiPcsStatus05_t, QsgmiiPcsStatus15_t, QsgmiiPcsStatus25_t, QsgmiiPcsStatus35_t, \
              QsgmiiPcsStatus06_t, QsgmiiPcsStatus16_t, QsgmiiPcsStatus26_t, QsgmiiPcsStatus36_t, \
              QsgmiiPcsStatus07_t, QsgmiiPcsStatus17_t, QsgmiiPcsStatus27_t, QsgmiiPcsStatus37_t, \
              QsgmiiPcsStatus08_t, QsgmiiPcsStatus18_t, QsgmiiPcsStatus28_t, QsgmiiPcsStatus38_t, \
              QsgmiiPcsStatus09_t, QsgmiiPcsStatus19_t, QsgmiiPcsStatus29_t, QsgmiiPcsStatus39_t, \
              QsgmiiPcsStatus010_t, QsgmiiPcsStatus110_t, QsgmiiPcsStatus210_t, QsgmiiPcsStatus310_t, \
              QsgmiiPcsStatus011_t, QsgmiiPcsStatus111_t, QsgmiiPcsStatus211_t, QsgmiiPcsStatus311_t, \
            };                                                                                       \
        uint8 index = 0;                                                                             \
        index = pcs_id*4 + mac_id%4;                                                                 \
        tbl_id = tbl_arr[index];                                                                     \
    }while(0)

/****************************************************************************
 *
* Function
*
*****************************************************************************/
int32
sys_usw_mac_init(uint8 lchip, ctc_port_global_cfg_t* p_port_global_cfg);

int32
sys_usw_mac_deinit(uint8 lchip);

int32
sys_usw_mac_get_port_capability(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t** p_port);

/*external property*/

extern int32
sys_usw_mac_get_mac_en(uint8 lchip, uint32 gport, uint32* p_enable);

extern int32
sys_duet2_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);

extern int32
sys_usw_mac_set_fec_en(uint8 lchip, uint32 gport, uint32 value);

extern int32
sys_usw_mac_set_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *fc_prop);

extern int32
sys_usw_mac_get_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *fc_prop);

extern int32
sys_usw_mac_get_quad_use_num(uint8 lchip, uint8 gmac_id, uint8* p_use_num);

extern int32
sys_usw_set_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value);

extern int32
sys_usw_get_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value);

extern int32
sys_usw_set_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value);

extern int32
sys_usw_get_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value);

extern int32
sys_usw_mac_set_internal_property(uint8 lchip, uint32 gport, ctc_port_property_t property, uint32 value);

extern int32
_sys_usw_mac_set_internal_property(uint8 lchip, uint16 lport, ctc_port_property_t property, uint32 value);

extern int32
sys_usw_mac_get_internal_property(uint8 lchip, uint32 gport, ctc_port_property_t property, uint32* p_value);

extern int32
sys_usw_mac_show_port_maclink(uint8 lchip);

extern int32
sys_usw_mac_get_3ap_training_en(uint8 lchip, uint32 gport, uint32* training_state);

/* ipg is mac tx property. value: 3 is 12Bytes, 2 is 8Bytes, 1 is 4Bytes, default is 12Bytes */
extern int32
sys_usw_mac_set_ipg(uint8 lchip, uint32 gport, uint32 value);

extern int32
_sys_usw_mac_set_ipg(uint8 lchip, uint16 gport, uint32 value);

extern int32
sys_duet2_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);

extern int32
sys_duet2_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);

extern int32
sys_duet2_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);

extern int32
sys_duet2_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);

extern int32
sys_usw_mac_get_serdes_info_capability(uint8 mac_id, uint16 serdes_id, uint8 serdes_num, uint8 pcs_mode, 
                                                    uint8 flag, ctc_port_serdes_info_t* p_serdes_port);
extern int32
sys_usw_mac_get_code_error_count(uint8 lchip, uint32 gport, uint32* p_value);

extern int32 
sys_usw_mac_set_monitor_link_enable(uint8 lchip, uint32 enable);

extern int32
sys_usw_mac_get_monitor_link_enable(uint8 lchip, uint32* p_value);

extern int32
sys_usw_mac_link_down_event(uint8 lchip, uint32 gport);

extern int32
sys_usw_mac_link_up_event(uint8 lchip, uint32 gport);

extern int32
sys_usw_mac_get_auto_neg(uint8 lchip, uint32 gport, uint32 type, uint32* p_value);

extern int32
sys_usw_mac_get_pcs_status(uint8 lchip, uint16 lport, uint32 *pcs_sta);

#ifdef __cplusplus
}
#endif

#endif

