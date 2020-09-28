/****************************************************************************
 * mux_drv.c   mux device access interface
 *
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liuht
 * Date:         2014-12-15.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "drv_debug.h"
#include "mux_drv.h"


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
 * Name    : mux_i2c_read
 * Purpose : read a i2c type mux device register
 * Input   : const mux_io_handle_t *p_hdl  - the handler of the mux device
             mux_access_t *p_para          - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
mux_i2c_read(const mux_io_handle_t* p_hdl, mux_access_t* p_para)
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
    DRV_LOG_DEBUG(DRV_MUX,"Read I2C mux device: len %d, offset %d ret %d\n", 
        i2c_para.len, i2c_para.offset, ret);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : mux_i2c_write
 * Purpose : write a i2c type mux device register
 * Input   : const mux_io_handle_t *p_hdl  - the handler of the mux device
             mux_access_t *p_para          - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
mux_i2c_write(const mux_io_handle_t* p_hdl, mux_access_t* p_para)
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
    DRV_LOG_DEBUG(DRV_MUX,"Write I2C mux device: len %d, offset %d, val %d, ret %d\n", 
        i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : mux_i2c_close
 * Purpose : free memory and pointer
 * Input   : mux_io_handle_t *phdl    - the handler of the mux device
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/

int32
mux_i2c_close(mux_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_MUX_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static mux_io_handle_t *
_mux_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    mux_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (mux_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_MUX_INFO,sizeof(mux_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = mux_i2c_close;
    p_hdl->read = mux_i2c_read;
    p_hdl->write = mux_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_MUX_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static mux_io_handle_t *
_mux_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _mux_create_i2c_handle( i2c_phdl);      
}


/****************************************************************************
 * Name	:   mux_io_register
 * Purpose: register a mux device handler, initialize the handler
 * Input	:  mux device type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the mux device
 * Note	:
****************************************************************************/	
mux_io_handle_t *
mux_io_register(mux_io_type_e type, const void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    DRV_CTC_CHK_PTR_NULL(p_data);
    
    switch(type)
    {
        case MUX_I2C:
            i2c_pgen = (i2c_gen_t *)p_data;
            return _mux_i2c_register(i2c_pgen);

        default:
            break;        
    }
    
    return NULL;
}

