/****************************************************************************
* $Id$
*  HAGT Interrupt private information, only used by HAGT interrupt self
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Kun Cao
* Date          : 2010-08-09
* Reason        : First Create.
****************************************************************************/
#ifndef __HAGT_INTR_PRIVATE_H__
#define __HAGT_INTR_PRIVATE_H__
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/

#include "sal.h"
#include "ctc_const.h"

/****************************************************************************
 *
 * Defines and Macros
 *
 ****************************************************************************/
 
#define HAGT_INTR_LOCAL_CHIP_NUM_MAX        CTC_MAX_LOCAL_CHIP_NUM
#define HAGT_INTR_POLL_TIMEOUT              2000        /* ms */
#define HAGT_INTR_QUEUE_TIMEOUT             2000        /* ms */
#define HAGT_INTR_UML_SLEEP_TIMEOUT         100         /* ms */
#define HAGT_INGR_ENTRY_NAME_LEN            8
#define HAGT_INTR_QUEUE_DEPTH_MAX           256

#define HAGT_PRI_ENTRY      p_intr->p_primary_entry[index]
#define HAGT_SND_ENTRY      p_intr->p_secondary_entry[index]

struct hagt_intr_entry_s
{
    uint32                      intr_id;            /* defined in ctc_interrupt_normal_intr_type_t */
    uint32                      count;              /* Specific interrupt count */
    int8                        desc[HAGT_INGR_ENTRY_NAME_LEN];
    hagt_intr_callback_func*    p_cb;               /* callback function */
};
typedef struct hagt_intr_entry_s hagt_intr_entry_t;

struct hagt_intr_s
{
    int32               intr_fd[HAGT_INTR_LOCAL_CHIP_NUM_MAX]; /* FD to notify normal interrupt */
#ifndef _GLB_UML_SYSTEM_
    int32               fatal_intr_fd[HAGT_INTR_LOCAL_CHIP_NUM_MAX]; /* FD to notify fatal interrupt */
#endif
    uint32              local_chip_num;             /* local chip number per card */
    uint32              count;                      /* Total normal interrupt count */
    uint32              err_count;                  /* Error interrupt count */
    uint32              fatal_count;                /* Total fatal interrupt count */
    hagt_intr_entry_t*  p_primary_entry;
    hagt_intr_entry_t*  p_secondary_entry;
    sal_task_t*         p_intr_primary_thread;
    sal_task_t*         p_intr_secondary_thread;
#ifndef _GLB_UML_SYSTEM_
    sal_task_t*         p_intr_fatal_thread;
#endif
    sal_async_queue_t*  p_secondary_queue;          /* Queue for primary secondary communication */
    sal_mutex_t*        p_intr_mutex;               /* Mutex for R/W humber interrupt register */
};
typedef struct hagt_intr_s hagt_intr_t;

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
#if 0// TODO: Commented by xgu for compile, 20121120                         

#ifdef _GLB_UML_SYSTEM_
/* only need in UML */
extern int32
humber_ipe_intr_emu_set(uint8 lchip, uint32 state, ctc_interrupt_normal_intr_type_t type);
extern int32
humber_ipe_intr_emu_get(uint8 lchip, uint32* p_state);

#ifdef FOAM_SUPPORT
extern int32
humber_foam_intr_emu_set(uint8 lchip, uint32 state, ctc_foam_interrupt_normal_intr_type_t type);
extern int32
humber_foam_intr_emu_get(uint8 lchip, uint32* p_state);
#endif /* !FOAM_SUPPORT */
#endif /* !_GLB_UML_SYSTEM_ */
#endif
#endif /*__HAGT_INTR_PRIVATE_H__*/

