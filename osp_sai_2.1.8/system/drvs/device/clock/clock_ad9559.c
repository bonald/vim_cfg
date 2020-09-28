/****************************************************************************
 * clock_ad9559.c   ad9559 access interface
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
#include "ctc_spi.h"
#include "drv_debug.h"
#include "clock_drv.h"
#include "clock_ad9559.h"
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

/*********************************************************************
 * Name    : _clock_ad9559_close
 * Purpose :  free memory and pointer
 * Input   : clock_handle_t *phdl       - the handler of the clock
          
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ad9559_close(clock_handle_t* phdl)
{
    return phdl->iohdl->close(phdl->iohdl);
}

/*********************************************************************
 * Name    : _clock_ad9559_write
 * Purpose :  write ad9559 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ad9559_write(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
    
    access.addr= addr | (AD_SPI_WIDTH_1<<13) | (AD_SPI_WRITE<<15);//from AD9559 DATASHEET
    access.val = val;    

    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_ad9559_write addr 0x%x, val pointer 0x%x", access.addr, *access.val);   

    return phdl->iohdl->write(phdl->iohdl, &access);
}

/*********************************************************************
 * Name    : _clock_ad9559_read
 * Purpose :  read ad9559 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ad9559_read(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
      
    access.addr = addr | (AD_SPI_WIDTH_1<<13) | (AD_SPI_READ<<15);//from AD9559 DATASHEET    
    access.val  = val;

    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_ad9559_read addr 0x%x, val pointer 0x%x",access.addr, *access.val);    
    
    return phdl->iohdl->read(phdl->iohdl, &access);
}


int32
_clock_ad9559_init(const clock_handle_t *phdl)
{
    uint32 tmp = 0x80;
    _clock_ad9559_write(phdl, 0x0, &tmp);
    
    return 0;
}



/****************************************************************************
 *
* Functions  
*
****************************************************************************/

/*********************************************************************
 * Name    : _clock_ad9559_get_status
 * Purpose : configuration ad9559
 * Input   : uint32 idx         - the index of ad9559, usually index = 0
 * Output  : N/A
 * Return  : AD9559_SUCCESS   = SUCCESS
           	  other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
_clock_ad9559_get_status(const clock_handle_t *phdl, uint32 *state)
{
    uint32 tmp = 0;

    tmp = 0x01;
    _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);

    _clock_ad9559_read(phdl, AD9559_PLL0_LOCK_STATUS, state);
    _clock_ad9559_read(phdl, AD9559_PLL0_LOCK_STATUS, state);
    _clock_ad9559_read(phdl, AD9559_PLL0_LOCK_STATUS, state);
    _clock_ad9559_read(phdl, AD9559_PLL0_LOCK_STATUS, state);
    if(*state == 0xf)
    {
        *state = GLB_SYNCE_STATE_LOCKED;
    }
    else
    {
        _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);
        _clock_ad9559_read(phdl, AD9559_PLL1_LOCK_STATUS, state);
        _clock_ad9559_read(phdl, AD9559_PLL1_LOCK_STATUS, state);
        _clock_ad9559_read(phdl, AD9559_PLL1_LOCK_STATUS, state);
        _clock_ad9559_read(phdl, AD9559_PLL1_LOCK_STATUS, state);
        if(*state == 0xf)
        {
            *state = GLB_SYNCE_STATE_LOCKED;
        }
        else
        {
            _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);
            _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
            _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
            _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
            _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
            if(*state&0x2)
            {
                *state = GLB_SYNCE_STATE_HOLDOVER;
            }
            else
            {
                _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);
                _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                if(*state&0x2)
                {
                    *state = GLB_SYNCE_STATE_HOLDOVER;
                }
                else 
                {
                    _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);
                    _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
                    _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
                    _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
                    _clock_ad9559_read(phdl, AD9559_PLL0_LOOP_STATE, state);
                    if(*state &0x1)
                    {
                        *state = GLB_SYNCE_STATE_FREERUN;
                    }
                    else
                    {
                        _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);
                        _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                        _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                        _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                        _clock_ad9559_read(phdl, AD9559_PLL1_LOOP_STATE, state);
                        if(*state & 0x1)
                        {
                            *state = GLB_SYNCE_STATE_FREERUN;
                        }
                        else
                        {
                            *state = GLB_SYNCE_STATE_PRELOCKED;
                        }
                    }
                }                
            }
        }
    }

    _clock_ad9559_write(phdl, AD9559_IRQ_CLEARING, &tmp);
    _clock_ad9559_write(phdl, AD9559_SOFT_IO_UPDATE, &tmp);

    return 0;
}

/*********************************************************************
 * Name    : clock_ad9559_register_driver
 * Purpose :  create a clock ad9559 device handler
 * Input   : clock_handle_t *spi_phdl       - the handler of the clock          
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
          
 * Note    : N/A
*********************************************************************/
int32
clock_ad9559_register_driver(clock_handle_t* hdl, clock_info_t* clock_info)
{
    clock_handle_t* clock_hdl;
    DRV_CTC_CHK_PTR(clock_info);
    
    spi_gen_t* spi_pgen = (spi_gen_t*)clock_info->clock_gen;
    spi_pgen->alen = AD9559_ADDRESS_LENTH;
    spi_pgen->len = AD9559_DATA_LENTH;
    
    clock_hdl = hdl;
    memcpy(&clock_hdl->clock_info, clock_info, sizeof(clock_info_t));
    clock_hdl->iohdl = clock_io_register(clock_info->clock_io_type, clock_info->clock_gen);
    
    clock_hdl->clock_close = _clock_ad9559_close;
    clock_hdl->reg_read = _clock_ad9559_read;
    clock_hdl->reg_write = _clock_ad9559_write;
    clock_hdl->clock_init = _clock_ad9559_init;
    clock_hdl->clock_get_status = _clock_ad9559_get_status;

    return 0;
}
