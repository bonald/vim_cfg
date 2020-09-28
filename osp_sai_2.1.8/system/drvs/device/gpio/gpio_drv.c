/****************************************************************************
 * gpio_drv.c   gpio device access interface
 *
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liuht
 * Date:         2014-12-06.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "drv_debug.h"
#include "gpio_drv.h"


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




/*********************************************************************
 * Name    : gpio_i2c_read
 * Purpose : read a i2c type gpio device register
 * Input   : const gpio_io_handle_t *p_hdl  - the handler of the gpio device
             gpio_access_t *p_para          - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
gpio_i2c_read(const gpio_io_handle_t* p_hdl, gpio_access_t* p_para)
{
    int32 ret = 0;
    i2c_op_para_t i2c_para;
    i2c_handle_t *i2c_phdl;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;   
    i2c_para.offset = p_para->reg;
    i2c_para.len = p_para->len;
    i2c_para.p_val = p_para->val;
    ret = i2c_phdl->read(i2c_phdl, &i2c_para);    
    //DRV_LOG_DEBUG(DRV_GPIO,"Read I2C gpio device: len %d, offset %d ret %d\n", 
    //    i2c_para.len, i2c_para.offset, ret);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : gpio_i2c_write
 * Purpose : write a i2c type gpio device register
 * Input   : const gpio_io_handle_t *p_hdl  - the handler of the gpio device
             gpio_access_t *p_para          - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
gpio_i2c_write(const gpio_io_handle_t* p_hdl, gpio_access_t* p_para)
{
    int32 ret=0;
    i2c_op_para_t i2c_para;       
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.offset = p_para->reg;
    i2c_para.len = p_para->len;
    i2c_para.p_val = p_para->val;

    ret = i2c_phdl->write(i2c_phdl, &i2c_para);
    //DRV_LOG_DEBUG(DRV_GPIO,"Write I2C gpio device: len %d, offset %d, val %d, ret %d\n", 
    //    i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : gpio_i2c_close
 * Purpose : free memory and pointer
 * Input   : gpio_io_handle_t *phdl    - the handler of the gpio device
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/

int32
gpio_i2c_close(gpio_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_GPIO_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static gpio_io_handle_t *
_gpio_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    gpio_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (gpio_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_GPIO_INFO,sizeof(gpio_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = gpio_i2c_close;
    p_hdl->read = gpio_i2c_read;
    p_hdl->write = gpio_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_GPIO_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static gpio_io_handle_t *
_gpio_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _gpio_create_i2c_handle( i2c_phdl);      
}


/****************************************************************************
 * Name	:   gpio_io_register
 * Purpose: register a gpio device handler, initialize the handler
 * Input	:  gpio device type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the gpio device
 * Note	:
****************************************************************************/	
gpio_io_handle_t *
gpio_io_register(gpio_io_type_e type, const void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    DRV_CTC_CHK_PTR_NULL(p_data);
    
    switch(type)
    {
        case GPIO_I2C:
            i2c_pgen = (i2c_gen_t *)p_data;
            return _gpio_i2c_register(i2c_pgen);

        default:
            break;        
    }
    
    return NULL;
}

