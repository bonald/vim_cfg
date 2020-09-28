/**
 @file sys_usw_interrupt.h

 @date 2012-10-23

 @version v2.0

*/
#ifndef _SYS_USW_INTERRUPT_H
#define _SYS_USW_INTERRUPT_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_interrupt.h"
#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define SYS_INTR_MAX_INTR       32              /**< max number of sup-level interrupts */



/**
 @brief [GG] SYS_INTR_GB_CHIP_FATAL interrupt sub-type
*/
enum sys_interrupt_type_sub_fatal_e
{
    /*reserved*/
    SYS_INTR_SUB_FATAL_RLM_HS =0,
    SYS_INTR_SUB_FATAL_RLM_CS = 1,
    SYS_INTR_SUB_FATAL_RLM_MAC = 2,

    SYS_INTR_SUB_FATAL_RLM_NET = 3,
    SYS_INTR_SUB_FATAL_RLM_EPE_RX = 4,
    SYS_INTR_SUB_FATAL_RLM_EPE_TX = 5,
    SYS_INTR_SUB_FATAL_RLM_IPE_RX = 6,
    SYS_INTR_SUB_FATAL_RLM_IPE_TX = 7,
    SYS_INTR_SUB_FATAL_RLM_WLAN = 8,
    SYS_INTR_SUB_FATAL_RLM_SECURITY = 9,
    SYS_INTR_SUB_FATAL_RLM_KEY = 10,
    SYS_INTR_SUB_FATAL_RLM_TCAM = 11,
    SYS_INTR_SUB_FATAL_RLM_BSR = 12,
    SYS_INTR_SUB_FATAL_RLM_QMG = 13,
    SYS_INTR_SUB_FATAL_RLM_AD = 14,
    SYS_INTR_SUB_FATAL_PCIE_INTF = 15 ,
    SYS_INTR_SUB_FATAL_DMA = 16,

    SYS_INTR_SUB_FATAL_MAX = 17
};
typedef enum sys_interrupt_type_sub_fatal_e sys_interrupt_type_sub_fatal_t;
/**
 @brief [D2] SYS_INTR_NORMAL interrupt sub-type
*/

enum sys_interrupt_type_sub_normal_e
{
    SYS_INTR_SUB_NORMAL_RLM_HS = 0,
    SYS_INTR_SUB_NORMAL_RLM_CS = 1,
    SYS_INTR_SUB_NORMAL_RLM_MAC = 2,
    SYS_INTR_SUB_NORMAL_RLM_NET = 3,
    SYS_INTR_SUB_NORMAL_RLM_EPE_RX = 4,
    SYS_INTR_SUB_NORMAL_RLM_EPE_TX = 5,
    SYS_INTR_SUB_NORMAL_RLM_IPE_RX = 6,
    SYS_INTR_SUB_NORMAL_RLM_IPE_TX = 7,
    SYS_INTR_SUB_NORMAL_RLM_WLAN = 8,
    SYS_INTR_SUB_NORMAL_RLM_SECURITY = 9,
    SYS_INTR_SUB_NORMAL_RLM_KEY = 10,
    SYS_INTR_SUB_NORMAL_RLM_TCAM = 11,
    SYS_INTR_SUB_NORMAL_RLM_BSR = 12,
    SYS_INTR_SUB_NORMAL_RLM_QMG = 13,
    SYS_INTR_SUB_NORMAL_RLM_AD = 14,
    SYS_INTR_SUB_NORMAL_PCIE_INTF = 15,
    SYS_INTR_SUB_NORMAL_DMA = 16,

    SYS_INTR_SUB_NORMAL_MCU_SUP =17,
    SYS_INTR_SUB_NORMAL_MCU_NET = 18,

    SYS_INTR_SUB_NORMAL_I2C_MASTER0 = 19,
    SYS_INTR_SUB_NORMAL_I2C_MASTER1 = 20,

    SYS_INTR_SUB_NORMAL_MAX = 21
};
typedef enum sys_interrupt_type_sub_normal_e sys_interrupt_type_sub_normal_t;
/**
 @brief [GG] SYS_INTR_GB_DMA_FUNC interrupt sub-type
*/
enum sys_interrupt_type_sub_dma_func_e
{
    SYS_INTR_SUB_DMA_FUNC_CHAN_0 = 0,
    SYS_INTR_SUB_DMA_FUNC_CHAN_1,
    SYS_INTR_SUB_DMA_FUNC_CHAN_2,
    SYS_INTR_SUB_DMA_FUNC_CHAN_3,
    SYS_INTR_SUB_DMA_FUNC_CHAN_4,
    SYS_INTR_SUB_DMA_FUNC_CHAN_5,
    SYS_INTR_SUB_DMA_FUNC_CHAN_6,
    SYS_INTR_SUB_DMA_FUNC_CHAN_7,
    SYS_INTR_SUB_DMA_FUNC_CHAN_8,
    SYS_INTR_SUB_DMA_FUNC_CHAN_9,
    SYS_INTR_SUB_DMA_FUNC_CHAN_10,
    SYS_INTR_SUB_DMA_FUNC_CHAN_11,
    SYS_INTR_SUB_DMA_FUNC_CHAN_12,
    SYS_INTR_SUB_DMA_FUNC_CHAN_13,
    SYS_INTR_SUB_DMA_FUNC_CHAN_14,
    SYS_INTR_SUB_DMA_FUNC_CHAN_15,
    SYS_INTR_SUB_DMA_FUNC_MAX
};
typedef enum sys_interrupt_type_sub_dma_func_e  sys_interrupt_type_sub_dma_func_t;

enum sys_interrupt_type_sub_pcie_e
{
    SYS_INTR_SUB_PCIE_INTF_REG_BURST_DONE = 0,

    SYS_INTR_SUB_PCIE_INTF_MAX
};
typedef enum sys_interrupt_type_sub_pcie_e sys_interrupt_type_sub_pcie_t;


enum sys_interrupt_type_e
{
    SYS_INTR_CHIP_FATAL = 0,
    SYS_INTR_CHIP_NORMAL = 1,

    SYS_INTR_FUNC_PTP_TS_CAPTURE_FIFO = 2,
    SYS_INTR_FUNC_PTP_TOD_PULSE_IN = 3,
    SYS_INTR_FUNC_PTP_TOD_CODE_IN_RDY = 4,
    SYS_INTR_FUNC_PTP_SYNC_PULSE_IN = 5,
    SYS_INTR_FUNC_PTP_SYNC_CODE_IN_RDY = 6,
    SYS_INTR_FUNC_PTP_SYNC_CODE_IN_ACC = 7,

    SYS_INTR_FUNC_STATS_STATUS_ADDR = 8,
    SYS_INTR_FUNC_MET_LINK_SCAN_DONE = 9,

    SYS_INTR_FUNC_MDIO_CHANGE_0 = 10,
    SYS_INTR_FUNC_STMCTL_STATE = 10,  /*tsingma*/

    SYS_INTR_FUNC_MDIO_CHANGE_1 = 11,
    SYS_INTR_FUNC_EPE_IPFIX_USEAGE_OVERFLOW = 11, /*tsingma*/

	SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN = 12,
    SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW = 13,
    SYS_INTR_FUNC_FIB_ACC_LEARN_OUT_FIFO = 14,

    SYS_INTR_FUNC_OAM_DEFECT_CACHE = 15,
    SYS_INTR_FUNC_BSR_C2C_PKT_CONGESTION = 16,
    SYS_INTR_FUNC_BSR_TOTAL_CONGESTION = 17,

    SYS_INTR_FUNC_BSR_OAM_CONGESTION = 18,
    SYS_INTR_FUNC_BSR_DMA_CONGESTION = 19,

    SYS_INTR_FUNC_BSR_CRITICAL_PKT_CONGESTION = 20,
    SYS_INTR_FUNC_OAM_AUTO_GEN = 21,

    SYS_INTR_PCS_LINK_31_0 = 22,
    SYS_INTR_PCS_LINK_47_32 = 23,

    SYS_INTR_FUNC_MDIO_XG_CHANGE_0 = 24,
    SYS_INTR_FUNC_MDIO_XG_CHANGE_1 = 25,

    SYS_INTR_PCIE_BURST_DONE = 26,
    SYS_INTR_DMA = 27,

    SYS_INTR_MCU_SUP = 28,
    SYS_INTR_GPIO = 28, /*Tsingma*/

    SYS_INTR_MCU_NET = 29,
    SYS_INTR_SOC_SYS = 29,/*Tsingma*/

    SYS_INTR_MAX = 30
};
typedef enum sys_interrupt_type_e sys_interrupt_type_t;
/**
 @brief [GG] Define struct for sys inter type
*/
struct sys_intr_type_s
{
    uint32  intr;                   /**<[GG] sup-level interrupt type defined in ctc_interrupt_type_CHIP_t */
    uint32  sub_intr;            /**<[GG] sub-level interrupt type defined in ctc_interrupt_type_CHIP_sub_SUP_t */
    uint32  low_intr;            /**<[GG] low-level interrupt type, used only for fatal and normal interrupt  */
    uint32  slice_id;             /**<[GG]0:slice0, 1:slice1, 2: slice0+slice1 */
};
typedef struct sys_intr_type_s sys_intr_type_t;

typedef int32 (* SYS_INTR_EVENT_CONFIG_FUNC)(uint8 lchip, CTC_INTERRUPT_EVENT_FUNC cb);

#define INTR_TYPE_CHECK(intr) \
    do { \
        if (((intr) >= SYS_INTR_MAX) )\
        { \
            return CTC_E_INVALID_PARAM; \
        } \
    } while (0)

#define INTR_SUB_TYPE_CHECK(intr, max) \
    do { \
        if ((intr) >= (max)) \
        { \
            return CTC_E_INVALID_PARAM; \
        } \
    } while (0)

#define INTR_SUP_FUNC_TYPE_CHECK(intr) \
    do { \
        if ((intr) < SYS_INTR_PCS_LINK_31_0 || (intr) > SYS_INTR_DMA) \
        { \
            return CTC_E_INVALID_PARAM; \
        } \
    } while (0)

#define SYS_INTETRUPT_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(interrupt, interrupt, INTERRUPT_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define INTR_INDEX_VAL_SET      0
#define INTR_INDEX_VAL_RESET    1
#define INTR_INDEX_MASK_SET     2
#define INTR_INDEX_MASK_RESET   3
#define INTR_INDEX_MAX          4

/**
 @brief [GB] Define parameters of a sup-level interrupt
*/
struct sys_intr_mapping_s
{
    int32               group;                      /**< group ID of this interrupt */
    uint32              intr;                       /**< type/ID of this interrupt, refer to ctc_interrupt_type_CHIP_t */
    char                desc[CTC_INTR_DESC_LEN];    /**< description of this interrupt */
    CTC_INTERRUPT_FUNC  isr;                        /**< callback function of this interrupt */
};
typedef struct sys_intr_mapping_s sys_intr_mapping_t;

/**
 @brief [GB] Define global configure parameters to initialize interrupt module
*/
struct sys_intr_global_cfg_s
{
    uint32              group_count;                /**< count of groups */
    uint32              intr_count;                 /**< count of interrupts */
    ctc_intr_group_t    group[SYS_USW_MAX_INTR_GROUP];  /**< array of groups */
    sys_intr_mapping_t intr[SYS_INTR_MAX_INTR];    /**< array of interrupts */
    uint8                intr_mode;                  /**< interrupt mode, 0: interrupt pin, 1: msi, 2:legacy*/
    uint8                rsv[3];
};
typedef struct sys_intr_global_cfg_s sys_intr_global_cfg_t;


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
/**
 @brief Initialize interrupt module
*/
extern int32
sys_usw_interrupt_init(uint8 lchip, void* intr_global_cfg);
/**
 @brief De-Initialize interrupt module
*/
extern int32
sys_usw_interrupt_deinit(uint8 lchip);
/**
 @brief Clear interrupt status
*/

extern int32
sys_usw_interrupt_clear_status(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp);
/**
 @brief Get interrupt status
*/
extern int32
sys_usw_interrupt_get_status(uint8 lchip, sys_intr_type_t* p_type, uint32* p_status);
/**
 @brief Set interrupt enable
*/
extern int32
sys_usw_interrupt_set_en(uint8 lchip, sys_intr_type_t* p_type, uint32 enable);

/**
 @brief register interrupt isr
*/
extern int32
sys_usw_interrupt_register_isr(uint8 lchip, sys_interrupt_type_t type, CTC_INTERRUPT_FUNC cb);

/**
 @brief Enable interrupt
*/
extern int32
sys_usw_interrupt_set_group_en(uint8 lchip, uint8 enable);

/**
 @brief Register event callback function
*/
extern int32
sys_usw_interrupt_register_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC cb);

/**
 @brief Get event callback function
*/
extern int32
sys_usw_interrupt_get_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC* p_cb);

extern int32
sys_usw_interrupt_get_info(uint8 lchip, sys_intr_type_t* p_ctc_type, uint32 intr_bit, tbls_id_t* p_intr_tbl, uint8* p_action_type, uint8* p_ecc_or_parity);
/**
 @brief Enable drv_ecc callback function
*/
extern int32
sys_usw_interrupt_proc_ecc_or_parity(uint8 lchip, tbls_id_t intr_tbl, uint32 intr_bit);
/**
 @brief Get group interrupt enable or not function
*/
extern int32
sys_usw_interrupt_get_group_en(uint8 lchip, uint8* enable);
extern int32
sys_usw_interrupt_set_group_en(uint8 lchip, uint8 enable);
#ifdef __cplusplus
}
#endif

#endif

