/**
 @file ctc_app_isr.h

 @date 2010-3-2

 @version v2.0

 This file define the isr APIs
*/

#ifndef _CTC_ISR_H
#define _CTC_ISR_H
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
typedef int32 callback_func (uint8*);

/****************************************************************************
 *
* Function
*
*****************************************************************************/

extern int32
ctc_app_isr_get_intr_cfg(ctc_intr_global_cfg_t* p_intr_cfg, uint8 interrupt_mode);

extern int32
ctc_app_isr_init(void);

extern int32
ctcs_app_isr_init(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif

