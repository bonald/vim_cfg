/****************************************************************************
 * $Id$
 *  HAGT Interrupt
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Kun Cao
 * Date          : 2010-07-24 16:00
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __HAGT_INTR_H__
#define __HAGT_INTR_H__

/****************************************************************************
 *
 * Header Files 
 *
 ****************************************************************************/

#include "ctc_interrupt.h"
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
// TODO: Commented by xgu for compile, 20121206       #include "ctc_foam_interrupt.h"
#endif /*FOAM_SUPPORT*/

/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/
typedef int32 hagt_intr_callback_func(uint8 gchip, uint8 lchip);

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
int32 hagt_abnormal_int_init();

int32
hagt_intr_debug_dump();
int32
hagt_intr_debug_clear_stats();
#if 0// TODO: Commented by xgu for compile, 20121120                         
int32
hagt_intr_register_callback(ctc_interrupt_normal_intr_type_t type, hagt_intr_callback_func* p_cb);
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
int32
hagt_foam_intr_register_callback(ctc_foam_interrupt_normal_intr_type_t type, hagt_intr_callback_func* p_cb);
#endif /*FOAM_SUPPORT*/
#endif
int32
hagt_intr_init(hagt_master_t* p_master);
#endif /*!__HAGT_INTR_H__*/

