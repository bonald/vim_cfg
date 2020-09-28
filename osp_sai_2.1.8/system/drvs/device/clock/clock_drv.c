/****************************************************************************
 * clock_drv.c   clock access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2018-03-02.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_spi.h"
#include "ctc_i2c.h"
#include "drv_debug.h"
#include "clock_drv.h"

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
 * Name    : clock_spi_close
 * Purpose :  free memory and pointer
 * Input   : clock_io_handle_t *phdl       - the handler of the clock
          
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
clock_spi_close(clock_io_handle_t* phdl)
{
    spi_handle_t* spi_phdl = NULL;
    int32 ret = 0;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(phdl->handle);
    
    spi_phdl = (spi_handle_t*)phdl->handle;
    ret = spi_phdl->close(spi_phdl);

    DRV_FREE( CTCLIB_MEM_DRIVER_CLOCK_INFO, phdl);
    phdl = NULL;

    return ret;
}

/*********************************************************************
 * Name    : clock_spi_write
 * Purpose :  write a spi type clock register
 * Input   : const clock_io_handle_t *phdl       - the handler of the clock
                clock_io_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
clock_spi_write(const clock_io_handle_t* phdl, clock_io_access_t* paccess)
{
    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para;

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(paccess);
    
    spi_phdl = (spi_handle_t *)phdl->handle;
    
    spi_para.addr= paccess->addr;
    spi_para.val = paccess->val;    

    DRV_LOG_DEBUG(DRV_CLOCK,"clock_spi_write addr %d, val pointer %p",spi_para.addr, spi_para.val);
    
    return spi_phdl->write(spi_phdl, &spi_para);
}

/*********************************************************************
 * Name    : clock_spi_read
 * Purpose :  read a spi type clock register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
clock_spi_read(const clock_io_handle_t* phdl, clock_io_access_t* paccess)
{
    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para;    

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(paccess);
    
    spi_phdl = (spi_handle_t *)phdl->handle;
      
    spi_para.addr = paccess->addr;
    spi_para.val  = paccess->val;

    DRV_LOG_DEBUG(DRV_CLOCK,"clock_spi_read addr %d, val pointer %p",spi_para.addr, spi_para.val);    
    
    return spi_phdl->read(spi_phdl, &spi_para);
}

/*********************************************************************
 * Name    : clock_spi_create_handle
 * Purpose :  create a clock device handler
 * Input   : spi_handle_t *spi_phdl       - the handler of the spi bus layer          
                                       
 * Output  : N/A
 * Return  : the handler of the clock
          
 * Note    : N/A
*********************************************************************/

static clock_io_handle_t*
_clock_create_spi_handle(spi_handle_t* spi_phdl)
{
    clock_io_handle_t* phdl = NULL; 
    
    DRV_CTC_CHK_PTR_NULL(spi_phdl);
    
    phdl = (clock_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_CLOCK_INFO, sizeof(clock_io_handle_t));

    if (NULL == phdl)
    {
        goto err_out;
    }

    phdl->handle = (void *)spi_phdl;

    phdl->close = clock_spi_close;
    phdl->read = clock_spi_read;
    phdl->write = clock_spi_write;
    
    return phdl;
    
err_out:
    if (NULL != phdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_CLOCK_INFO, phdl);
        phdl = NULL;
    }

    return NULL;
}

/*********************************************************************
 * Name    : clock_spi_register
 * Purpose :  register spi type clock
 * Input   : spi_gen_t *spi_pgen      - some info about the way of clock'spi bus implement, 
                                        such as gpio, fpga etc
                                       
 * Output  : N/A
 * Return  : the handler of the clock
          
 * Note    : N/A
*********************************************************************/

static clock_io_handle_t*
_clock_spi_register(spi_gen_t* spi_pgen)
{
    spi_handle_t* spi_phdl = NULL;
    
    spi_phdl = spi_create_handle(spi_pgen);

    return _clock_create_spi_handle(spi_phdl);      
}

int32
clock_i2c_read(const clock_io_handle_t* p_hdl, clock_io_access_t* p_para)
{
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl->handle);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_para->len;
    i2c_para.offset = p_para->addr;
    i2c_para.p_val = (uint8 *)p_para->val;
    
    return i2c_phdl->read(i2c_phdl, &i2c_para);
}

int32
clock_i2c_write(const clock_io_handle_t* p_hdl, clock_io_access_t* p_para)
{
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl->handle);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_para->len;
    i2c_para.offset = p_para->addr;
    i2c_para.p_val = (uint8 *)p_para->val;
   
    return i2c_phdl->write(i2c_phdl, &i2c_para);
}

int32
clock_i2c_close(clock_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_CLOCK_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static clock_io_handle_t *
_clock_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    clock_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (clock_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_CLOCK_INFO,sizeof(clock_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = clock_i2c_close;
    p_hdl->read = clock_i2c_read;
    p_hdl->write = clock_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_CLOCK_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static clock_io_handle_t *
_clock_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;
    
    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _clock_create_i2c_handle( i2c_phdl);      
}

/*********************************************************************
 * Name    : clock_register
 * Purpose :  register a clock device handler
 * Input   : const void *pgen        - some info about the way of clock'spi bus implement, 
                                       such as gpio, fpga etc
          clock_io_type_t type         - the type of accessing clock, now just have one type clock
          
 * Output  : N/A
 * Return  : the handler of the clock
           
 * Note    : N/A
*********************************************************************/

clock_io_handle_t*
clock_io_register(clock_io_type_t type, const void* pgen)
{       
    DRV_CTC_CHK_PTR_NULL(pgen);
    
    switch(type)
    {
        case E_CLOCK_SPI:           
            return _clock_spi_register((spi_gen_t*)pgen);
        case E_CLOCK_I2C:
            return _clock_i2c_register((i2c_gen_t*)pgen);
        default:
            break;        
    }
    
    return NULL;
}


