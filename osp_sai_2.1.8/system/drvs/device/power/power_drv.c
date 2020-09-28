/****************************************************************************
 * power_drv.c   power access interface
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
#include "drv_debug.h"
#include "power_drv.h"
#include "epld_api.h"
#include "ctc_i2c.h"
#include "power_api.h"

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

static psu_io_handle_t *
_psu_io_epld_register(psu_private_t* p_data)
{
    psu_io_handle_t *p_hdl = NULL; 

    p_hdl = (psu_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->p_data_epld= (void *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(uint8));
    sal_memcpy(p_hdl->p_data_epld, p_data->p_data_epld, sizeof(uint8));
    return p_hdl;

    err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_POWER_INFO, p_hdl);
        p_hdl = NULL;
    }
    return NULL;
}

/* added by liuht for bug 24525,2013-10-24 */
int32
psu_i2c_write(const psu_io_handle_t* p_hdl, psu_access_t* p_access)
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
    DRV_LOG_DEBUG(DRV_POWER,"Write I2C psu: len %d, offset 0x%x, val %d, ret %d\n", 
        i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);        
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
psu_i2c_read(const psu_io_handle_t *p_hdl, psu_access_t *p_access)
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
    DRV_LOG_DEBUG(DRV_POWER,"Read I2C psu: len %d, offset 0x%x ret %d\n", 
        i2c_para.len, i2c_para.offset, ret);
    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
psu_i2c_mattr(const psu_io_handle_t *p_hdl, uint8 new_addr)
{
    i2c_handle_t *i2c_phdl = NULL;
    i2c_gen_t* i2c_pgen = NULL;
    
    DRV_CTC_CHK_PTR(p_hdl);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_pgen = (i2c_gen_t *)i2c_phdl->data;
    i2c_pgen->addr = new_addr;

    return 0;
}

static psu_io_handle_t *
_psu_io_i2c_epld_register(psu_private_t *p_data)
{   
    psu_io_handle_t *p_io_hdl;
    i2c_gen_t *i2c_pgen;    
    i2c_handle_t *i2c_phdl = NULL;

    p_io_hdl = (psu_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_io_handle_t));
    if (NULL == p_io_hdl)
    {
        return NULL;
    }
    /*create i2c handle and store i2c info in i2c handle.*/
    i2c_pgen = (i2c_gen_t *)(p_data->p_data_i2c);
    i2c_phdl = i2c_create_handle(i2c_pgen);    
    p_io_hdl->handle = (void *)i2c_phdl;
    p_io_hdl->read = psu_i2c_read;
    p_io_hdl->write = psu_i2c_write;
    p_io_hdl->mattr = psu_i2c_mattr; 
    /*store epld info */
    p_io_hdl->p_data_epld = (void *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(uint8));
    sal_memcpy(p_io_hdl->p_data_epld, p_data->p_data_epld, sizeof(uint8));
    return p_io_hdl;
}

static psu_io_handle_t *
_psu_io_i2c_gpio_register(psu_private_t *p_data)
{
    psu_io_handle_t *p_io_hdl;
    i2c_gen_t *i2c_pgen;    
    i2c_handle_t *i2c_phdl = NULL;

    p_io_hdl = (psu_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_io_handle_t));
    if (NULL == p_io_hdl)
    {
        return NULL;
    }
    /*create i2c handle and store i2c info in i2c handle.*/
    i2c_pgen = (i2c_gen_t *)(p_data->p_data_i2c);
    i2c_phdl = i2c_create_handle(i2c_pgen);    
    p_io_hdl->handle = (void *)i2c_phdl;
    p_io_hdl->read = psu_i2c_read;
    p_io_hdl->write = psu_i2c_write;
    p_io_hdl->mattr = psu_i2c_mattr; 
    /*store epld info */
    p_io_hdl->p_data_gpio = (void *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_data_gpio));
    sal_memcpy(p_io_hdl->p_data_gpio, p_data->p_data_gpio, sizeof(psu_data_gpio));
    return p_io_hdl;    
}

static psu_io_handle_t *
_psu_io_gpio_register(psu_private_t* p_data)
{
    psu_io_handle_t *p_hdl = NULL; 

    p_hdl = (psu_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->p_data_gpio= (void *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_data_gpio));
    sal_memcpy(p_hdl->p_data_gpio, p_data->p_data_gpio, sizeof(psu_data_gpio));
    return p_hdl;

    err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_POWER_INFO, p_hdl);
        p_hdl = NULL;
    }
    return NULL;
}

/****************************************************************************
 * Name : psu_io_register
 * Purpose: initialize the io handler, register access func.
 * Input    : type show the io type,  p_data give the io information
 * Output:  
 * Return:  the handler of the psu io
 * Note :
****************************************************************************/   
psu_io_handle_t *
psu_io_register(psu_io_type_e type, const void *data)
{
    /* added by liuht for bug 24525,2013-10-24 */
    psu_private_t *p_data = (psu_private_t *)data;
       
    DRV_CTC_CHK_PTR_NULL(p_data);
    
    switch(type)
    {
        case PSU_IO_EPLD:
            return _psu_io_epld_register( p_data);
        case PSU_IO_I2C_EPLD:
            return _psu_io_i2c_epld_register(p_data);
        case PSU_IO_I2C_GPIO:
            return _psu_io_i2c_gpio_register(p_data);
        case PSU_IO_GPIO:
            return _psu_io_gpio_register(p_data);            
        default:
            break;        
    }
    
    return NULL;
}

