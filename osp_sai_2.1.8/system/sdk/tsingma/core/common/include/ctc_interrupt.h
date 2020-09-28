/**
 @file ctc_interrupt.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-1-20

 @version v2.0

   This file contains all isr related data structure, enum, macro and proto.
*/

#ifndef _CTC_INTERRUPT_H
#define _CTC_INTERRUPT_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_mix.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup isr ISR
 @{
*/

#define CTC_INTERRUPT_PARITY_ERROR_MAX_ENTRY_WORD     8


/**
 @brief [GB] Define fatal interrupt action
*/
enum ctc_interrupt_fatal_intr_action_e
{
    CTC_INTERRUPT_FATAL_INTR_NULL = 0,    /**< [GB.GG.D2.TM] no any operation */
    CTC_INTERRUPT_FATAL_INTR_LOG,         /**< [GB.GG.D2.TM] only for log */
    CTC_INTERRUPT_FATAL_INTR_RESET,       /**< [GB.GG.D2.TM] need to reset chip */
};
typedef enum ctc_interrupt_fatal_intr_action_e ctc_interrupt_fatal_intr_action_t;

enum ctc_interrupt_ecc_type_e
{
   CTC_INTERRUPT_ECC_TYPE_SBE,                    /**< [GB.GG.D2.TM] single bit error */
   CTC_INTERRUPT_ECC_TYPE_MBE,                    /**< [GB.GG.D2.TM] muliple bit error*/
   CTC_INTERRUPT_ECC_TYPE_PARITY_ERROR,    /**< [GB.GG.D2.TM] parity error*/
   CTC_INTERRUPT_ECC_TYPE_TCAM_ERROR,      /**< [GB.GG.D2.TM] tcam error */
   CTC_INTERRUPT_ECC_TYPE_NUM
};
typedef enum ctc_interrupt_ecc_type_e ctc_interrupt_ecc_type_t;

/**
 @brief  Define ecc event info
*/
struct ctc_interrupt_ecc_s
{
   uint8  type;                 /**< [GB.GG.D2.TM] refer to ctc_interrupt_ecc_type_t */
   uint32 tbl_id;               /**< [GB.GG.D2.TM] table id */
   uint32 tbl_index;            /**< [GB.GG.D2.TM] table index */
   uint8  action;               /**< [GB.GG.D2.TM] refer to ctc_interrupt_fatal_intr_action_t */
   uint8  recover;              /**< [GB.GG.D2.TM] error recovered by ecc recover module */
};
typedef struct ctc_interrupt_ecc_s ctc_interrupt_ecc_t;

/**
 @brief Define interrupt related MACROs
*/
#define CTC_INTR_DESC_LEN       32              /**<[GB.GG.D2.TM] length of interrupt description string */
#define CTC_INTR_MAX_INTR       32              /**<[GB.GG.D2.TM] max number of sup-level interrupts */
#define CTC_INTR_STAT_SIZE      4               /**<[GB.GG.D2.TM] max number of DWORD/uint32 of interrupt status */
#define INVG                    ((uint32) - 1)    /**<[GB.GG.D2.TM] invalid group ID */
#define CTC_INTR_ALL            ((uint32) - 1)    /**<[GB.GG.D2.TM] all interrupt bits of sup-level/sub-level */
#define CTC_INTR_T2B(_type_)    (1 << (_type_))   /**<[GB.GG.D2.TM] convert interrupt type to bit */

/**
 @brief [GB] interrupt related MACRO
*/
#define CTC_INTR_GB_MAX_GROUP       6       /**< [GB.GG.D2.TM] max interrupt group(PIN) number */
#define CTC_INTR_GB_MAX_INTR        29      /**< [GB.GG.D2.TM] max sup-level interrupt number */

/**
 @brief sup-level interrupt type
*/
enum ctc_interrupt_type_e
{
    /* ASIC Internal Interrupt */
    CTC_INTR_CHIP_FATAL = 0,             /**< [GG.D2.TM] sup fatal exception appear, sub-intr [0,47] */
    CTC_INTR_CHIP_NORMAL,                /**< [GG.D2.TM] sup normal exception appear, sub-intr [0,85] */

    /* Functional Interrupts */
    CTC_INTR_FUNC_PTP_TS_CAPTURE,        /**< [GB.GG.D2.TM] PTP RX timestamp capture, when 1PPS or SyncPulse received */
    CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE, /**< [GB.GG] PTP TX timestamp capture, for 2-step */
    CTC_INTR_FUNC_PTP_TOD_PULSE_IN,      /**< [GB.GG.D2.TM] PTP 1PPS received */
    CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY,   /**< [GB.GG.D2.TM] PTP TOD input code is ready */
    CTC_INTR_FUNC_PTP_SYNC_PULSE_IN,     /**< [GB.GG.D2.TM] PTP SyncPulse received */
    CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY,  /**< [GB.GG.D2.TM] PTP Sync Interface input code is ready */
    CTC_INTR_FUNC_OAM_TRPT_STATS,        /**< [GB]OAM throughput stats */
    CTC_INTR_FUNC_OAM_DEFECT_CACHE,      /**< [GB.GG.D2.TM] OAM defect cache to notify OAM status change */
    CTC_INTR_FUNC_MDIO_CHANGE,           /**< [GB.GG.D2.TM] MDIO detects any status changes including 1G and XG,
                                                                       [GB] Only for XG Phy*/
    CTC_INTR_FUNC_MDIO_1G_CHANGE,        /**< [GB]MDIO detects any status changes of 1G PHYs, only used for GB */
    CTC_INTR_FUNC_LINKAGG_FAILOVER,      /**< [GB.GG.D2.TM] Linkagg protect switch done */
    CTC_INTR_FUNC_IPE_LEARN_CACHE,       /**< [GB.GG.D2.TM] Learning cache to notify SW learning */
    CTC_INTR_FUNC_IPE_AGING_FIFO,        /**< [GB.GG.D2.TM] Aging FIFO to notify SW aging */
    CTC_INTR_FUNC_STATS_FIFO,            /**< [GB.GG.D2.TM] Statistics exceed the threshold */
    CTC_INTR_FUNC_APS_FAILOVER,          /**< [GB.GG.D2.TM] APS protect switch done */
    CTC_INTR_FUNC_LINK_CHAGNE,           /**< [GG.D2.TM] Pcs link status change */
    CTC_INTR_FUNC_IPFIX_OVERFLOW,        /**< [D2.TM] Ipfix memory overflow */
    CTC_INTR_FUNC_EXTRAL_GPIO_CHANGE,    /**< [TM] Extral gpio interrupt */
    /* DMA Interrupt */
    CTC_INTR_DMA_FUNC,                   /**< [GB.GG.D2.TM] DMA channel need to handle, sub-intr [0,16] */
    CTC_INTR_DMA_NORMAL,                 /**< [GB]DMA channel exception appear, sub-intr [0,47] */

    CTC_INTR_FUNC_STMCTL_STATE,        /**< [TM] Storm control violate state */
    CTC_INTR_MAX
};
typedef enum ctc_interrupt_type_e ctc_interrupt_type_t;


/**
 @brief [GB] the follwing macros are to compatible with GB
*/
#define   CTC_INTR_GB_CHIP_FATAL                 CTC_INTR_CHIP_FATAL
#define   CTC_INTR_GB_CHIP_NORMAL                CTC_INTR_CHIP_NORMAL
#define   CTC_INTR_GB_FUNC_PTP_TS_CAPTURE        CTC_INTR_FUNC_PTP_TS_CAPTURE
#define   CTC_INTR_GB_FUNC_PTP_MAC_TX_TS_CAPTURE CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE
#define   CTC_INTR_GB_FUNC_PTP_TOD_PULSE_IN      CTC_INTR_FUNC_PTP_TOD_PULSE_IN
#define   CTC_INTR_GB_FUNC_PTP_TOD_CODE_IN_RDY   CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY
#define   CTC_INTR_GB_FUNC_PTP_SYNC_PULSE_IN     CTC_INTR_FUNC_PTP_SYNC_PULSE_IN
#define   CTC_INTR_GB_FUNC_PTP_SYNC_CODE_IN_RDY  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY
#define   CTC_INTR_GB_FUNC_OAM_DEFECT_CACHE      CTC_INTR_FUNC_OAM_DEFECT_CACHE
#define   CTC_INTR_GB_FUNC_MDIO_CHANGE           CTC_INTR_FUNC_MDIO_CHANGE
#define   CTC_INTR_GB_FUNC_MDIO_1G_CHANGE        CTC_INTR_FUNC_MDIO_1G_CHANGE
#define   CTC_INTR_GB_FUNC_LINKAGG_FAILOVER      CTC_INTR_FUNC_LINKAGG_FAILOVER
#define   CTC_INTR_GB_FUNC_IPE_LEARN_CACHE       CTC_INTR_FUNC_IPE_LEARN_CACHE
#define   CTC_INTR_GB_FUNC_IPE_AGING_FIFO        CTC_INTR_FUNC_IPE_AGING_FIFO
#define   CTC_INTR_GB_FUNC_STATS_FIFO            CTC_INTR_FUNC_STATS_FIFO
#define   CTC_INTR_GB_FUNC_APS_FAILOVER          CTC_INTR_FUNC_APS_FAILOVER

#define   CTC_INTR_GB_DMA_FUNC                   CTC_INTR_DMA_FUNC
#define   CTC_INTR_GB_DMA_NORMAL                 CTC_INTR_DMA_NORMAL
#define   CTC_INTR_GB_MAX                        CTC_INTR_MAX

/**
 @brief [GB] Define a prototype for interrupt callback function
*/
typedef int32 (* CTC_INTERRUPT_FUNC)(uint8 lchip, uint32 intr, void* p_data);

/**
 @brief [GB] Define a prototype for event callback function
*/
typedef int32 (* CTC_INTERRUPT_EVENT_FUNC)(uint8 gchip, void* p_data);

/**
 @brief [GB] Define event type SDK notify application
*/
enum ctc_interrupt_event_e
{
    CTC_EVENT_L2_SW_LEARNING = 0,          /**<[GB.GG.D2.TM] (MUST) Notify application newest entries needed to learn, refer to ctc_learning_cache_t */
    CTC_EVENT_L2_SW_AGING,                 /**<[GB.GG.D2.TM] (MUST) Notify application newest entries needed to aging, refer to ctc_aging_fifo_info_t */
    CTC_EVENT_OAM_STATUS_UPDATE,           /**<[GB.GG.D2.TM] (MUST) Notify application newest status changes of OAM session, refer to TBD */
    CTC_EVENT_PTP_TX_TS_CAPUTRE,           /**<[GB.GG] (MUST) Notify application captured TX TS entries, refer to ctc_ptp_ts_cache_t */
    CTC_EVENT_PTP_INPUT_CODE_READY,        /**<[GB.GG.D2.TM] (OPTIONAL) Notify application Sync or ToD Interface input code ready, refer to ctc_ptp_rx_ts_message_t*/
    CTC_EVENT_PORT_LINK_CHANGE,            /**<[GB.GG.D2.TM] (OPTIONAL) Notify application 1G or XG PHY's link status changs, refer to ctc_port_link_status_t */
    CTC_EVENT_ABNORMAL_INTR,               /**<[GB.GG.D2.TM] (OPTIONAL) Notify application chip's abnormal status, refer to ctc_interrupt_abnormal_status_t */
    CTC_EVENT_ECC_ERROR,                   /**<[GB.GG.D2.TM] (OPTIONAL) Notify application chip's memory ecc error, refer to ctc_interrupt_ecc_t */
    CTC_EVENT_IPFIX_OVERFLOW,              /**<[D2.TM]    (OPTIONAL) Notify application chip's ipfix memory overflow, refer to ctc_ipfix_event_t */
    CTC_EVENT_STORMCTL_STATE,              /**<[TM]    (OPTIONAL) Notify application chip's stormctl state, refer to ctc_security_stormctl_event_t */
    CTC_EVENT_PHY_STATUS_CHANGE,           /**<[D2.TM]    (OPTIONAL) Notify application chip's phy status change, refer to ctc_phy_status_t */
    CTC_EVENT_EXTRAL_GPIO_CHANGE,          /**<[TM]    (OPTIONAL) When soc disabled, Notify the gpio connect extral device occur interrupt, refer to ctc_gpio_event_t */
    CTC_EVENT_FCDL_DETECT,                 /**<[D2.TM]    (OPTIONAL) Notify application chip's flowctl deadlock-detect , refer to ctc_qos_fcdl_event_t */

    CTC_EVENT_MAX

};
typedef enum ctc_interrupt_event_e ctc_interrupt_event_t;

/**
 @brief [GB] Define cascading interrupt type, CHIP replaced by gb
*/
struct ctc_intr_type_s
{
    uint32  intr;                   /**<[GB.GG.D2.TM] sup-level interrupt type defined in ctc_interrupt_type_CHIP_t */
    uint32  sub_intr;               /**<[GB.GG.D2.TM] sub-level interrupt type defined in ctc_interrupt_type_CHIP_sub_SUP_t */
    uint32  low_intr;               /**<[GG.D2.TM] low-level interrupt type, used only for fatal and normal interrupt  */
};
typedef struct ctc_intr_type_s ctc_intr_type_t;

/**
 @brief [GB] Define interrupt status in bitmap
*/
struct ctc_intr_status_s
{
    uint32  bmp[CTC_INTR_STAT_SIZE];                 /**< [GB.GG.D2.TM] bitmap of interrupt status */
    uint32  bit_count;                               /**< [GB.GG.D2.TM] valid bit count */
    uint32  low_intr;                                /**< [GG.D2.TM] low-level interrupt type, used only for fatal and normal interrupt  */
};
typedef struct ctc_intr_status_s ctc_intr_status_t;

/**
 @brief [GB] Define chip abnormal status
*/
struct ctc_interrupt_abnormal_status_s
{
    ctc_intr_type_t     type;                        /**< [GB.GG.D2.TM] interrupt type */
    ctc_intr_status_t   status;                      /**< [GB.GG.D2.TM] low-level interrupt status */
    ctc_interrupt_fatal_intr_action_t action;        /**< [GB.GG.D2.TM] advised action of this status */
};
typedef struct ctc_interrupt_abnormal_status_s ctc_interrupt_abnormal_status_t;

/**
 @brief [GB] Define parameters of a sup-level interrupt
*/
struct ctc_intr_mapping_s
{
    int32               group;                        /**< [GB.GG.D2.TM]group ID of this interrupt */
    uint32              intr;                         /**< [GB.GG.D2.TM]type/ID of this interrupt, refer to ctc_interrupt_type_CHIP_t */
};
typedef struct ctc_intr_mapping_s ctc_intr_mapping_t;

/**
 @brief [GB] Define parameters of a group
*/
struct ctc_intr_group_s
{
    int32               group;                                 /**< [GB.GG.D2.TM]group ID */
    uint32              irq;                                   /**< [GB.GG.D2.TM]IRQ for this group, for msi this means msi irq index */
    int32               prio;                                  /**< [GB.GG.D2.TM]priority of thread/task for handle this group */
    char                desc[CTC_INTR_DESC_LEN];               /**< [GB.GG.D2.TM]description of this group */
};
typedef struct ctc_intr_group_s ctc_intr_group_t;

/**
 @brief [GB] Define global configure parameters to initialize interrupt module
*/
struct ctc_intr_global_cfg_s
{
    uint32               group_count;                 /**<[GB.GG.D2.TM] count of groups */
    uint32               intr_count;                  /**<[GB.GG.D2.TM] count of interrupts */
    ctc_intr_group_t*    p_group;                     /**<[GB.GG.D2.TM] array of groups */
    ctc_intr_mapping_t*  p_intr;                      /**<[GB.GG.D2.TM] array of interrupts */
    uint8                intr_mode;                   /**<[GB.GG.D2.TM] interrupt mode, 0: interrupt pin, 1: msi, 2:legacy*/
    uint8                rsv[3];
};
typedef struct ctc_intr_global_cfg_s ctc_intr_global_cfg_t;

/**@} end of @defgroup isr ISR  */

#ifdef __cplusplus
}
#endif

#endif

