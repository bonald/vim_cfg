/****************************************************************************
 * vsc3308_drv.c   vsc3308 access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Chenxi Qi
 * Date:         2012-11-05.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_i2c.h"
#include "drv_debug.h"
#include "vsc3308_drv.h"


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
vsc3308_i2c_write(const vsc3308_io_handle_t* p_hdl, vsc3308_access_t* p_access)
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
    DRV_LOG_DEBUG(DRV_VSC3308,"Write I2C VSC3308: len %d, offset %d, val %d, ret %d\n", 
        i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
vsc3308_i2c_read(const vsc3308_io_handle_t *p_hdl, vsc3308_access_t *p_access)
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
    DRV_LOG_DEBUG(DRV_VSC3308,"Read I2C VSC3308: len %d, offset %d ret %d\n", 
        i2c_para.len, i2c_para.offset, ret);
    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

static vsc3308_io_handle_t *
_vsc3308_i2c_create_handle(i2c_handle_t *i2c_phdl)
{
    vsc3308_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (vsc3308_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_VSC3308_INFO,sizeof(vsc3308_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;
    p_hdl->read = vsc3308_i2c_read;
    p_hdl->write = vsc3308_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_VSC3308_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}


static vsc3308_io_handle_t *
_vsc3308_i2c_register(i2c_gen_t* i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _vsc3308_i2c_create_handle( i2c_phdl);      
}

/****************************************************************************
 * Name : vsc3308_io_register
 * Purpose: register vsc3308 io handler, initialize the handler
 * Input    :   io type and the pointer to general infomation struct
 * Output:  
 * Return:  the io handler of the vsc3308
 * Note :
****************************************************************************/   
vsc3308_io_handle_t*
vsc3308_io_register(const void *p_data)
{
    i2c_gen_t *i2c_pgen;
    DRV_CTC_CHK_PTR_NULL(p_data);
    i2c_pgen = (i2c_gen_t *)p_data;
    
    return _vsc3308_i2c_register(i2c_pgen);
}


