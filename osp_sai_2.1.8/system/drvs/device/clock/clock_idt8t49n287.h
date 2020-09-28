/****************************************************************************
* $Id$
*  The header file of the idt8t49n287 operation.
*
* (C) Copyright Actus Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chani
* Date          : 2013-09-30 
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_IDT8T49N287__
#define __CLOCK_IDT8T49N287__

#include "ctc_i2c.h"

#define IDT8T49N287_DATA_LENTH 1    /* data length (byte)*/  

#define IDT8T49N287_PLL0_LOCK_STATUS     0x0206
#define IDT8T49N287_PLL0_INPUT_CTL     0x000a

#define IDT8T49N287_PLL1_LOCK_STATUS     0x0210
#define IDT8T49N287_PLL1_INPUT_CTL     0x003d

#define IDT8T49N287_INTR_CTL     0x00c7
#define IDT8T49N287_INTR_STATUS     0x0200

int32 clock_idt8t49n287_register_driver(clock_handle_t* hdl, clock_info_t* clock_info);

#endif 

