/****************************************************************************
* $Id$
*  line card port information
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : weij
* Date          : 2010-08-16
* Reason        : First Create.
****************************************************************************/
#ifndef __LCM_H__
#define __LCM_H__

#include "sal_common.h"
#include "lcm_specific.h"
#include "lcapi.h"
#include "lcm_port.h"

/* modified by kcao for bug 13571, 2010-11-22 : use memory management */
#define LCM_MALLOC(type,size) XMALLOC(type,size)
#define LCM_CALLOC(type,size) XCALLOC(type,size)
#define LCM_FREE(type,ptr)    XFREE(type,ptr)

enum timer_mask_s
{
    ALL_TIMER=0,
    PORT_TIMER=1,
    FIBER_TIMER=2,
    ENV_TIMER=3,
    PORT_INTR=4
};


#define LCM_CHECK_PTR(ptr)                                                               \
if(!ptr)                                                                                  \
{                                                                                         \
    return LCM_E_INVALID_PTR;                                                            \
}

#endif /*__LCM_H__*/
