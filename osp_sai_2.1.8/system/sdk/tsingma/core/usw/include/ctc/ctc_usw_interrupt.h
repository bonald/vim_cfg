/**
 @file ctc_usw_interrupt.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-10-23

 @version v2.0

 The interrupt is an asynchronous mechanism which allow hardware/chip to communicate with the software.
 For example, when learning cache in chip exceed threshold, chip notify software though interrupt;
 software received this interrupt, read out the entries in learning cache and notify application to add these MAC entries.
 There are two files for interrupt module:
 \p
 \d sys_usw_interrupt.c implements the APIs (init, set_en, get_en, clear_status, get_status) and the dispatch of groups and interrupts.
 \d sys_usw_interrupt_isr.c implements the default configurations of interrupt and the declaration of interrupt callback functions.
 \b
 User need to determine four things below.
 \p
 \d your application needs how many interrupt groups/PINs [0,5], and the IRQs and task priority of each group.
 \d your application needs how many sup-level interrupts [0,28], and the mapping to groups [0,5]
 \d your application needs register how many events (ctc_interrupt_event_t), and the callback function of registered events
 \d initialize interrupt module by parameter or by change group & interrupt default configuration in sys_usw_interrupt_isr.c
 \b
 GB interrupt is organized with multi-level architecture.
 \p
    (1) The group level has max 6 groups (corresponding to max interrupt PINs), each group need a IRQ number and handled by a thread/task.
     The 29 interrupts in sup-level can be arbitrary mapping to the 6 groups.
 \p  Command "show interrupt sdk" displays this mapping.
 \p (2) The sup-level has 29 interrupts. User need to be concerned mostly about the interrupts of sup-function(2-24) and DMA-function(25).
 \d   0 is interrupt to indicate chip's fatal status, it is subdivided to 48 sub-level interrupts in GbSupInterruptFatal
 \d   1 is interrupt to indicate chip's normal status, it is subdivided to 86 sub-level interrupts in GbSupInterruptNormal
 \d   2-24 are functional interrupts needed be processed by feature module, and each has no sub-interrupts
 \d   25 is interrupt to indicate DMA channels TX/RX, it is subdivided to 17 sub-level interrupts in PcieFuncIntr
 \d   26 is interrupt to indicate chip's DMA normal status, it is subdivided to 48 sub-level interrupts in PciExpCoreInterruptNormal
 \d   27-28 are interrupts to indicate PCIe's error status
 \p   Command "show interrupt 0 register all" displays the relationship between sup-level and sub-level.
 \b
                                Sup-level and Sub-level Interrupt Mapping Table
 \p
 \t |index| sup-level type                         | sup-level reg             | sub-level type                         | sub-level reg array       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  0  | CTC_INTR_GB_CHIP_FATAL                 | GbSupInterruptFatal       | ctc_interrupt_type_gb_sub_fatal_t      | g_intr_sub_reg_sup_fatal  |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  1  | CTC_INTR_GB_CHIP_NORMAL                | GbSupInterruptNormal      | ctc_interrupt_type_gb_sub_normal_t     | g_intr_sub_reg_sup_normal |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  2  | CTC_INTR_GB_FUNC_PTP_TS_CAPTURE        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  3  | CTC_INTR_GB_FUNC_PTP_MAC_TX_TS_CAPTURE | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  4  | CTC_INTR_GB_FUNC_PTP_TOD_PULSE_IN      | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  5  | CTC_INTR_GB_FUNC_PTP_TOD_CODE_IN_RDY   | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  6  | CTC_INTR_GB_FUNC_PTP_SYNC_PULSE_IN     | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  7  | CTC_INTR_GB_FUNC_PTP_SYNC_CODE_IN_RDY  | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  8  | CTC_INTR_GB_FUNC_OAM_TX_PKT_STATS      | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  9  | CTC_INTR_GB_FUNC_OAM_TX_OCTET_STATS    | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  10 | CTC_INTR_GB_FUNC_OAM_RX_PKT_STATS      | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  11 | CTC_INTR_GB_FUNC_OAM_RX_OCTET_STATS    | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  12 | CTC_INTR_GB_FUNC_OAM_DEFECT_CACHE      | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  13 | CTC_INTR_GB_FUNC_OAM_CLEAR_EN_3        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  14 | CTC_INTR_GB_FUNC_OAM_CLEAR_EN_2        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  15 | CTC_INTR_GB_FUNC_OAM_CLEAR_EN_1        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  16 | CTC_INTR_GB_FUNC_OAM_CLEAR_EN_0        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  17 | CTC_INTR_GB_FUNC_MDIO_XG_CHANGE        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  18 | CTC_INTR_GB_FUNC_MDIO_1G_CHANGE        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  19 | CTC_INTR_GB_FUNC_CHAN_LINKDOWN_SCAN_OK | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  20 | CTC_INTR_GB_FUNC_IPE_LEARN_CACHE       | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  21 | CTC_INTR_GB_FUNC_IPE_FIB_LEARN_FIFO    | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  22 | CTC_INTR_GB_FUNC_IPE_AGING_FIFO        | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  23 | CTC_INTR_GB_FUNC_STATS_FIFO            | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  24 | CTC_INTR_GB_FUNC_MET_LINK_SCAN_DONE    | GbSupInterruptFunction    | N/A                                    | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  25 | CTC_INTR_GB_DMA_FUNC                   | PcieFuncIntr              | ctc_interrupt_type_gb_sub_dma_func_t   | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  26 | CTC_INTR_GB_DMA_NORMAL                 | PciExpCoreInterruptNormal | ctc_interrupt_type_gb_sub_dma_normal_t | N/A                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  27 | CTC_INTR_GB_PCIE_SECOND                | TBD                       | TBD                                    | TBD                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|
 \t |  28 | CTC_INTR_GB_PCIE_PRIMARY               | TBD                       | TBD                                    | TBD                       |
 \t |-----|----------------------------------------|---------------------------|----------------------------------------|---------------------------|

*/

#ifndef _CTC_USW_INTERRUPT_H
#define _CTC_USW_INTERRUPT_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Header Files
*
***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @addtogroup interrupt INTERRUPT
 @{
*/


/****************************************************************
*
* Function
*
***************************************************************/

/**
 @brief Initialize interrupt module

 @param[in] lchip    local chip id

 @param[in] p_global_cfg    pointer to interrupt initialize parameter, refer to ctc_intr_global_cfg_t

 @remark[D2.TM]  User can initialize the interrupt configuration though p_global_cfg parameter,  If the p_global_cfg is NULL,
         SDK will using default configuration.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_interrupt_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize interrupt module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the interrupt configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_interrupt_deinit(uint8 lchip);

/**
 @brief Register event callback function

 @param[in] lchip    local chip id

 @param[in]  event      Event type

 @param[in]  cb         Callback function to handle this event type

 @remark[D2.TM]  User register callback functions of application concerned events though this API

 @return CTC_E_XXX
*/
extern int32
ctc_usw_interrupt_register_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC cb);

/**@} end of @addtogroup interrupt INTERRUPT */

#ifdef __cplusplus
}
#endif

#endif

