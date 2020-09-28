/**
 @file ctc_usw_interrupt.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-10-23

 @version v2.0

 This file define ctc functions

*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_interrupt.h"
#include "sys_usw_common.h"
#include "sys_usw_interrupt.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
*
* Function
*
*****************************************************************************/
int32
ctc_usw_interrupt_init(uint8 lchip, void* p_global_cfg)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_init(lchip, p_global_cfg));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_interrupt_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_deinit(lchip));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_interrupt_register_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC cb)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_INTERRUPT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_register_event_cb(lchip, event, cb));
    }

    return CTC_E_NONE;
}

