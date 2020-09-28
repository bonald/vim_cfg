/****************************************************************************
 * idt8t49n287 access interface
 *
 * Copyright:    (c)2005 Actus Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Chani
 * Date:         2013-09-30.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_i2c.h"
#include "drv_debug.h"
#include "clock_drv.h"
#include "clock_idt8t49n287.h"
#include "glb_synce_define.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/


/****************************************************************************
 *
* Functions  
*
****************************************************************************/

int32
_clock_idt8t49n287_close(clock_handle_t* phdl)
{
    return phdl->iohdl->close(phdl->iohdl);
}

int32
_clock_idt8t49n287_write(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    int32 ret = 0;
    clock_io_access_t access;
    
    access.addr= addr;
    access.val = val;
    access.len = IDT8T49N287_DATA_LENTH;
    ret = phdl->iohdl->write(phdl->iohdl, &access);
    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_idt8t49n287_write addr 0x%x, val pointer 0x%x", access.addr, *access.val);   

    return ret;
}

int32
_clock_idt8t49n287_read(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    int32 ret = 0;
    clock_io_access_t access;
      
    access.addr = addr;
    access.val  = val;
    access.len = IDT8T49N287_DATA_LENTH;
    ret = phdl->iohdl->read(phdl->iohdl, &access);
    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_idt8t49n287_read addr 0x%x, val pointer 0x%x",access.addr, *access.val);    
    
    return ret;
}

int32
_clock_idt8t49n287_init(const clock_handle_t *phdl)
{    
    uint32 val=0x00;/* disable interruput */
    _clock_idt8t49n287_write(phdl, IDT8T49N287_INTR_CTL, &val);
    val=0xff;/* clear interruput */
    _clock_idt8t49n287_write(phdl, IDT8T49N287_INTR_STATUS, &val);
    //val = 0xc0; /* enable syncE */
    //_clock_idt8t49n287_write(phdl, IDT8T49N287_PLL0_INPUT_CTL, &val);
    return 0;
}

int32
_clock_idt8t49n287_get_status(const clock_handle_t *phdl, uint32 *state)
{
    uint32 val=0;
    //_clock_idt8t49n287_read(phdl, 0x0200, &val);
    //_clock_idt8t49n287_read(phdl, 0x000a, &val);
    //_clock_idt8t49n287_read(phdl, 0x0205, &val);
    _clock_idt8t49n287_read(phdl, IDT8T49N287_PLL0_LOCK_STATUS, &val);
    switch(val&0x03)
    {
        case 1:
            *state = GLB_SYNCE_STATE_FREERUN;
            break;
        case 2:
            *state = GLB_SYNCE_STATE_LOCKED;
            break;
        case 3:
            *state = GLB_SYNCE_STATE_HOLDOVER;
            break;
        default:
            *state = GLB_SYNCE_STATE_UNKNOWN1;
            break;
    }
    return 0;
}

int32
_clock_idt8t49n287_intr_fixup(const clock_handle_t *phdl)
{
    uint32 val=0xff;
    _clock_idt8t49n287_write(phdl, IDT8T49N287_INTR_STATUS, &val);
    return 0;
}

int32
clock_idt8t49n287_register_driver(clock_handle_t* hdl, clock_info_t* clock_info)
{
    clock_handle_t* clock_hdl;
    DRV_CTC_CHK_PTR(clock_info);
    
    clock_hdl = hdl;
    memcpy(&clock_hdl->clock_info, clock_info, sizeof(clock_info_t));
    clock_hdl->iohdl = clock_io_register(clock_info->clock_io_type, clock_info->clock_gen);
    
    clock_hdl->clock_close = _clock_idt8t49n287_close;
    clock_hdl->reg_read = _clock_idt8t49n287_read;
    clock_hdl->reg_write = _clock_idt8t49n287_write;
    clock_hdl->clock_init = _clock_idt8t49n287_init;
    clock_hdl->clock_get_status = _clock_idt8t49n287_get_status;
    clock_hdl->intr_fix_up = _clock_idt8t49n287_intr_fixup;

    return 0;
}
