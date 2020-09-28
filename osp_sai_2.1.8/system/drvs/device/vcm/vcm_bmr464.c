/****************************************************************************
 * bmr_api.c    bmr api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       guhg
 * Date:         2015-12-11.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "vcm_drv.h"
#include "vcm_bmr464.h"

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


/*****464 driver start*****************************************************/

/****************************************************************************
 * Name	: _bmr464_decode_val
 * Purpose: decode the temperature of the bmr from raw register value to real temperature value 
 * Input	:  the raw register value
 * Output:   
 * Return:  the real temperature value
 * Note	:  X = Y.2^N    X= real value, Y isthe last elevn bit of the value,two's complemen integer
 *              N is the first five bit of the value,two's complemen integer
****************************************************************************/	
static float32 
_bmr464_decode_val(uint16 value)
{
    int16 value_y,value_tmp,value_n;
    float32 data;
    value_y = value & 0x7ff;  /*11 bit*/
    value_n = value >> 11;     /*5 bit*/
    if(value_n & 0x10)    
    {
        value_tmp = value_n & 0xf;
        value_n = ~value_tmp + 1;
        value_n = -(value_n & 0xf);
    }
    if(value_y & 0x400)
    {
        value_tmp = value_y & 0x3ff;
        value_y = ~value_tmp + 1;
        value_y = -(value_y & 0x3ff);
    }

    if(value_n > 0)
    {
        data = (float32)value_y * (1 << value_n);
    }
    else
    {
        data = (float32)value_y / (1 << -value_n);
    }
    return data;
}

static float32 
_bmr464_out_volt_decode_val(uint16 value)
{
    float32 data;
    data = (float32)value / (1 << 13);
    return data;
}

/*raw register read, not do data translate*/
static int32
_vcm_bmr464_register_read(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = VCM_BMR_464_VALUE_LEN;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

/*raw register write, not do data translate*/
static int32
_vcm_bmr464_register_write(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = VCM_BMR_464_VALUE_LEN;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_vcm_bmr464_get_temp(vcm_handle_t* handle,vcm_access_t para,float32* data )
{  
    int32 ret = -1;
    uint8 buf[VCM_BMR_464_VALUE_LEN];
    uint16 val;
    
    switch(para.type)
    {
        case TMPR:
            ret = handle->reg_read(handle, VCM_BMR_464_TMPR1, buf);
            val = (buf[1]<<8)+buf[0];
            *data = _bmr464_decode_val(val);
            break;
        default:
            DRV_LOG_ERR("bmr type type is unspport\n"); 
            return RESULT_ERROR; 
    }

    return ret;
}

static int32
_vcm_bmr464_get_cur(vcm_handle_t* handle, vcm_access_t para, float32* current)
{
    int32 ret = -1;
    uint8 buf[VCM_BMR_464_VALUE_LEN];
    uint16 val;

    switch(para.type)
    {
        case COUT:
            ret = handle->reg_read(handle, VCM_BMR_464_IOUT, buf);
            val = (buf[1]<<8)+buf[0];
            *current = _bmr464_decode_val(val);
            break;
        default:
            DRV_LOG_ERR("bmr current type is unspport\n"); 
            return RESULT_ERROR; 
    }
    return ret;  
}

static int32
_vcm_bmr464_get_volt(vcm_handle_t* handle, vcm_access_t para, float32* voltage )
{
    int32 ret = -1;
    uint8 buf[VCM_BMR_464_VALUE_LEN];
    uint16 val;
    switch(para.type)
    {
        case VOUT:
            ret = handle->reg_read(handle, VCM_BMR_464_VOUT, buf);
            val = (buf[1]<<8)+buf[0];
            *voltage = _bmr464_out_volt_decode_val(val);
            break;
        case VIN:
            ret = handle->reg_read(handle, VCM_BMR_464_VIN, buf);
            val = (buf[1]<<8)+buf[0];
            *voltage = _bmr464_decode_val(val);
            break;
        default:
            DRV_LOG_ERR("bmr voltage type is unspport\n"); 
            return RESULT_ERROR; 
    }
    return ret;    
}

static int32
_vcm_bmr464_init(vcm_handle_t* handle)
{
    return RESULT_OK;
}

int32
vcm_bmr464_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info)
{
    vcm_handle_t* vcm_hal;
    DRV_CTC_CHK_PTR(vcm_info);  

    vcm_hal = hdl;
    /* register ltc2991 dev */
    memcpy(&vcm_hal->vcm_info, vcm_info, sizeof(vcm_info_t));
    vcm_hal->io_hdl = vcm_io_register(vcm_info->vcm_io_type, vcm_info->vcm_gen);
    vcm_hal->access_err_cnt = 0;
    
    vcm_hal->vcm_init = _vcm_bmr464_init;
    vcm_hal->get_temp = _vcm_bmr464_get_temp;
    vcm_hal->get_volt = _vcm_bmr464_get_volt;
    vcm_hal->get_cur = _vcm_bmr464_get_cur;
    vcm_hal->reg_read = _vcm_bmr464_register_read;
    vcm_hal->reg_write = _vcm_bmr464_register_write;

    return RESULT_OK;
}


