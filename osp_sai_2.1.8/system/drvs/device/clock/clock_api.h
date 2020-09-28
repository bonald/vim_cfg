/****************************************************************************
* $Id$
*  The header file of the clock api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-03-02
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_API_H__
#define __CLOCK_API_H__

#include "ctc_spi.h"
#include "ctc_i2c.h"
#include "clock_drv.h"
#include "glb_hw_define.h"

#define IDT8T49N287_ADDRESS_LENTH 2  /* address length (byte)*/ 

enum clock_error_e
{
    CLOCK_SUCCESS = 0,
    CLOCK_E_ERROR = -999,
    CLOCK_E_NO_MEMORY,
    CLOCK_E_NOT_INIT,
    CLOCK_E_INIT_FAILED,
    CLOCK_E_TIMEOUT,    
    CLOCK_E_READ,    
    CLOCK_E_WRITE,
    /* parameter check error */
    CLOCK_E_INVALID_PARAM,
    CLOCK_E_INVALID_PTR,
    CLOCK_E_INVALID_INDEX,    
    CLOCK_E_INVALID_LENGTH,
    CLOCK_E_INVALID_CHIP,
};

int32 clock_read(uint32 idx, uint16 addr, uint32  *val);
int32 clock_write(uint32 idx, uint16 addr, uint32  val);
int32 clock_read_indirect(uint32 idx, uint16 addr, uint32 *val);
int32 clock_write_indirect(uint32 idx, uint16 addr, uint32 val);
int32 clock_close(uint32 idx);
int32 clock_init(clock_info_t *clock_info, uint32 num);
int32 clock_get_status(uint32 idx, uint32 *state);
int32 clock_intr_fixup(uint32 idx);

#endif 

