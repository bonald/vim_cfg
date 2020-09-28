/****************************************************************************
 * fan_drv.c   fan access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-10-11.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_i2c.h"
#include "drv_debug.h"
#include "fan_drv.h"


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

int32
fan_i2c_write(const fan_io_handle_t* p_hdl, fan_access_t* p_access)
{
    int32 ret=0;
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_access);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_access->len;
    i2c_para.offset = p_access->reg;
    i2c_para.p_val = p_access->val;  
   
    ret = i2c_phdl->write(i2c_phdl, &i2c_para);
    DRV_LOG_DEBUG(DRV_FAN,"Write I2C fan: len %d, offset %d, val %d, ret %d\n", 
        i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);        
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
fan_i2c_read(const fan_io_handle_t *p_hdl, fan_access_t *p_access)
{
    int32 ret=0;
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_access);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_access->len;
    i2c_para.offset = p_access->reg;
    i2c_para.p_val = p_access->val;    
    ret = i2c_phdl->read(i2c_phdl, &i2c_para);
    DRV_LOG_DEBUG(DRV_FAN,"Read I2C fan: len %d, offset %d ret %d\n", 
        i2c_para.len, i2c_para.offset, ret);
    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}


static fan_io_handle_t *
_fan_i2c_create_handle(i2c_handle_t *i2c_phdl)
{
    fan_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (fan_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_FAN_INFO,sizeof(fan_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;
    p_hdl->read = fan_i2c_read;
    p_hdl->write = fan_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_FAN_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}


static fan_io_handle_t *
_fan_i2c_register(i2c_gen_t* i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _fan_i2c_create_handle( i2c_phdl);      
}

static fan_io_handle_t *
_fan_epld_register(const void* p_data)
{
    fan_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (fan_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_FAN_INFO,sizeof(fan_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }
    
    p_hdl->p_data = (void *)DRV_MALLOC(CTCLIB_MEM_DRIVER_FAN_INFO,sizeof(uint8));
    sal_memcpy(p_hdl->p_data, p_data, sizeof(uint8));
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_FAN_INFO, p_hdl);
        p_hdl = NULL;
    }
    return NULL;
}

static fan_io_handle_t *
_fan_ctc5236_register(const void* p_data)
{
    return NULL;
}

/****************************************************************************
 * Name : fan_io_register
 * Purpose: register fan io handler, initialize the handler
 * Input    :   io type and the pointer to general infomation struct
 * Output:  
 * Return:  the io handler of the fan
 * Note :
****************************************************************************/   
fan_io_handle_t*
fan_io_register(fan_io_type_e type, const void *p_data)
{
    i2c_gen_t *i2c_pgen;

    if (type != E_FAN_CTC5236)
    {
        DRV_CTC_CHK_PTR_NULL(p_data);
    }
    switch(type)
    {
        case E_FAN_I2C:
            i2c_pgen = (i2c_gen_t *)p_data;
            return _fan_i2c_register(i2c_pgen);
        case E_FAN_EPLD:
            return _fan_epld_register(p_data);
        case E_FAN_CTC5236:
            return _fan_ctc5236_register(p_data);
        default:
            break;
    }
    return NULL;
}

