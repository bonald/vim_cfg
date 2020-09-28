/****************************************************************************
 * clock_cdcm6208.c   cdcm6208 access interface
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
#include "clock_cdcm6208.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
/*********************************************************************
 * Name    : _clock_cdcm6208_close
 * Purpose :  free memory and pointer
 * Input   : clock_handle_t *phdl       - the handler of the clock
          
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_cdcm6208_close(clock_handle_t* phdl)
{
    return phdl->iohdl->close(phdl->iohdl);
}

/*********************************************************************
 * Name    : _clock_cdcm6208_write
 * Purpose :  write cdcm6208 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_cdcm6208_write(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
    uint8 i = 0;
    uint32 value[CDCM6208_DATA_LENTH];
    
    for(i = 0; i < CDCM6208_DATA_LENTH; i++)
    {
        value[i] = ((*val) >>( i * 8))& 0xff;
    }
    access.addr= addr;
    access.val = value;    

    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_cdcm6208_write addr %d, val pointer %p",
                            access.addr, access.val);
    
    return phdl->iohdl->write(phdl->iohdl, &access);
}

/*********************************************************************
 * Name    : _clock_cdcm6208_read
 * Purpose :  read cdcm6208 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_cdcm6208_read(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
      
    access.addr = addr;   
    access.val = val;

    DRV_LOG_DEBUG(DRV_CLOCK,"_clock_cdcm6208_read addr %d, val pointer %p",access.addr, access.val);    
    
    return phdl->iohdl->read(phdl->iohdl, &access);
}

/*********************************************************************
 * Name    : clock_cdcm6208_register_driver
 * Purpose :  create a clock cdcm6208 device handler
 * Input   : clock_handle_t *spi_phdl       - the handler of the clock          
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
          
 * Note    : N/A
*********************************************************************/
int32
clock_cdcm6208_register_driver(clock_handle_t* hdl, clock_info_t* clock_info)
{
    clock_handle_t* clock_hdl;
    DRV_CTC_CHK_PTR(clock_info);
    
    spi_gen_t* spi_pgen = (spi_gen_t*)clock_info->clock_gen;
    spi_pgen->alen = CDCM6208_ADDRESS_LENTH;
    spi_pgen->len = CDCM6208_DATA_LENTH;
    
    clock_hdl = hdl;
    memcpy(&clock_hdl->clock_info, clock_info, sizeof(clock_info_t));
    clock_hdl->iohdl = clock_io_register(clock_info->clock_io_type, clock_info->clock_gen);
    
    clock_hdl->clock_close = _clock_cdcm6208_close;
    clock_hdl->reg_read = _clock_cdcm6208_read;
    clock_hdl->reg_write = _clock_cdcm6208_write;

    return 0;
}

