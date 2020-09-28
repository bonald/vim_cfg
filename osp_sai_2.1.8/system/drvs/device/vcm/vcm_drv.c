/****************************************************************************
 * vcm_drv.c   vcm(voltage and current monitor) access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       tongzb 
 * Date:         2017-07-24.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "drv_debug.h"
#include "vcm_drv.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/

/*********************************************************************
 * Name    : vcm_i2c_read
 * Purpose :  read a i2c type vcm register
 * Input   : const vcm_io_handle_t *p_hdl       - the handler of the vcm
          vcm_io_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
vcm_i2c_read(const vcm_io_handle_t* p_hdl, vcm_io_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_VCM,"Read I2C thermal vcm: len %d, offset %d ret %d\n", 
    //    i2c_para.len, i2c_para.offset, ret);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
vcm_epld_read(const vcm_io_handle_t* p_hdl, vcm_io_access_t* p_para)
{
    int32 ret=0;
#if 0
    int32 temp =0 ,temp1 =0, temp2=0;
    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    
    epld_item_write(0, EPLD_I2C_ADDR, p_para->reg);
    epld_item_read(0,  EPLD_I2C_ADDR, &temp);
    epld_item_write(0, EPLD_I2C_CTRL, 0x8);
    epld_item_write(0, EPLD_I2C_CTRL, 0xa);
    usleep(2000);
    epld_item_read(0,  EPLD_I2C_ADDR, &temp);
    epld_item_read(0,  EPLD_I2C_RD_DATA_L, &temp1);
    epld_item_read(0,  EPLD_I2C_RD_DATA_H, &temp2);
    p_para->val[0] = temp1;
    p_para->val[1] = temp2;
#endif
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : vcm_i2c_write
 * Purpose :  write a i2c type vcm register
 * Input   : const vcm_io_handle_t *p_hdl       - the handler of the vcm
          vcm_io_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
vcm_i2c_write(const vcm_io_handle_t* p_hdl, vcm_io_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_VCM,"Write I2C thermal vcm: len %d, offset %d, val %d, ret %d\n", 
    //    i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
vcm_epld_write(const vcm_io_handle_t* p_hdl, vcm_io_access_t* p_para)
{
    int32 ret = 0;
    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : vcm_i2c_close
 * Purpose :  free memory and pointer
 * Input   : vcm_io_handle_t *phdl       - the handler of the vcm
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
vcm_i2c_close(vcm_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_VCM_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

int32
vcm_epld_close(vcm_io_handle_t *p_hdl)
{
    int32 ret = 0;
    DRV_CTC_CHK_PTR(p_hdl);
    
    DRV_FREE( CTCLIB_MEM_DRIVER_VCM_INFO, p_hdl);
    return ret;
}

static vcm_io_handle_t *
_vcm_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    vcm_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (vcm_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_VCM_INFO,sizeof(vcm_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = vcm_i2c_close;
    p_hdl->read = vcm_i2c_read;
    p_hdl->write = vcm_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_VCM_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static vcm_io_handle_t *
_vcm_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _vcm_create_i2c_handle( i2c_phdl);      
}

static vcm_io_handle_t *
_vcm_create_epld_handle()
{
    vcm_io_handle_t *p_hdl = NULL; 
   
    p_hdl = (vcm_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_VCM_INFO,sizeof(vcm_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }
    
    p_hdl->close = vcm_epld_close;
    p_hdl->read = vcm_epld_read;
    p_hdl->write = vcm_epld_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_BMR_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static vcm_io_handle_t *
_vcm_epld_register()
{
    return _vcm_create_epld_handle();      
}

/****************************************************************************
 * Name	: vcm_register
 * Purpose: register vcm device handler, initialize the handler
 * Input	:vcm type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the thermal vcm
 * Note	:
****************************************************************************/	
vcm_io_handle_t *
vcm_io_register(vcm_io_type_e type, void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    switch(type)
    {
        case VCM_I2C:
            DRV_CTC_CHK_PTR_NULL(p_data);
            i2c_pgen = (i2c_gen_t *)p_data;
            return _vcm_i2c_register(i2c_pgen);
        case VCM_EPLD:
            return _vcm_epld_register();
        default:
            break;        
    }
    
    return NULL;
}

