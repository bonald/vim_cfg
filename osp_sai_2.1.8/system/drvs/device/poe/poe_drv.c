/****************************************************************************
 * poe_drv.c   poe(Power of Ethernet) access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       tongzb 
 * Date:         2019-05-27.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "drv_debug.h"
#include "poe_drv.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
int32
poe_i2c_read(const poe_io_handle_t* p_hdl, poe_io_access_t* p_para)
{
    int32 ret = 0;
    i2c_op_para_t i2c_para;
    i2c_handle_t *i2c_phdl;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;   

    i2c_para.len = p_para->len;
    i2c_para.p_val = p_para->val;
    ret = i2c_phdl->read(i2c_phdl, &i2c_para);
    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
poe_i2c_write(const poe_io_handle_t* p_hdl, poe_io_access_t* p_para)
{
    int32 ret=0;
    i2c_op_para_t i2c_para;       
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    i2c_para.len = p_para->len;
    i2c_para.p_val = p_para->val;
    ret = i2c_phdl->write(i2c_phdl, &i2c_para);
   
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
poe_i2c_close(poe_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_POE_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static poe_io_handle_t *
_poe_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    poe_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (poe_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POE_INFO,sizeof(poe_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = poe_i2c_close;
    p_hdl->read = poe_i2c_read;
    p_hdl->write = poe_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_POE_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static poe_io_handle_t *
_poe_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _poe_create_i2c_handle( i2c_phdl);
}

poe_io_handle_t *
poe_io_register(poe_io_type_e type, void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    switch(type)
    {
        case POE_IO_I2C:
            DRV_CTC_CHK_PTR_NULL(p_data);
            i2c_pgen = (i2c_gen_t *)p_data;
            return _poe_i2c_register(i2c_pgen);
        default:
            break;        
    }
    
    return NULL;
}

