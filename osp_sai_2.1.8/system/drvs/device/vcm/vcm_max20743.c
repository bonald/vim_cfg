/****************************************************************************
 * bmr_api.c    max 20743 api 
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
#include "vcm_max20743.h"

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
/*raw register read, not do data translate*/
static int32
_vcm_max20743_register_read(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = VCM_MAX20743_VALUE_LEN;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

/*raw register write, not do data translate*/
static int32
_vcm_max20743_register_write(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = VCM_MAX20743_VALUE_LEN;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_vcm_max20743_get_temp(vcm_handle_t* handle,vcm_access_t para,float32* data )
{  
    int32 ret = -1;
    uint8 buf[VCM_MAX20743_VALUE_LEN];
    uint16 val;
    
    switch(para.type)
    {
        case TMPR:
            ret = handle->reg_read(handle, VCM_MAX20743_TMPR1, buf);
            val = (buf[1]<<8)+buf[0];
            *data = (val * 10.0 - 5886)/19.0;
            break;
        default:
            DRV_LOG_ERR("bmr type type is unspport\n"); 
            return RESULT_ERROR; 
    }

    return ret;
}

static int32
_vcm_max20743_get_volt(vcm_handle_t* handle, vcm_access_t para, float32* voltage )
{
    int32 ret = -1;
    uint8 buf[VCM_MAX20743_VALUE_LEN];
    uint16 val;
    switch(para.type)
    {
        case VOUT:
            ret = handle->reg_read(handle, VCM_MAX20743_VOUT, buf);
            val = (buf[1]<<8)+buf[0];
            *voltage = (val /512.0);
            break;
        case VIN:
            ret = handle->reg_read(handle, VCM_MAX20743_VIN, buf);
            val = (buf[1]<<8)+buf[0];
            *voltage = (val * 100.0)/3585.0;
            break;
        default:
            DRV_LOG_ERR("bmr voltage type is unspport\n"); 
            return RESULT_ERROR; 
    }
    return ret;    
}

static int32
_vcm_max20743_get_cur(vcm_handle_t* handle, vcm_access_t para, float32* current)
{
    int32 ret = -1;
    uint8 buf[VCM_MAX20743_VALUE_LEN];
    uint16 val;
    vcm_access_t access;
    float32 vout,vin,tmp,D;
    switch(para.type)
    {
        case COUT:
            ret = handle->reg_read(handle, VCM_MAX20743_IOUT, buf);
            val = (buf[1]<<8)+buf[0];
            /* according to datasheet, the current is based on vout,vin and tmp */
            access.type = VOUT;
            _vcm_max20743_get_volt(handle, access, &vout);
            access.type = VIN;
            _vcm_max20743_get_volt(handle, access, &vin);
            D = vout/vin;
            access.type = TMPR;
            _vcm_max20743_get_temp(handle, access, &tmp);
            *current = (val * 10.0 - (5023.3-327.0*D))/(90.1-(3.53*D)) + 0.019*(tmp - 50);
            break;
        default:
            DRV_LOG_ERR("bmr current type is unspport\n"); 
            return RESULT_ERROR; 
    }
    return ret;  
}

static int32
_vcm_max20743_init(vcm_handle_t* handle)
{
    return RESULT_OK;
}

int32
vcm_max20743_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info)
{
    vcm_handle_t* vcm_hal;
    DRV_CTC_CHK_PTR(vcm_info);  

    vcm_hal = hdl;
    /* register ltc2991 dev */
    memcpy(&vcm_hal->vcm_info, vcm_info, sizeof(vcm_info_t));
    vcm_hal->io_hdl = vcm_io_register(vcm_info->vcm_io_type, vcm_info->vcm_gen);
    vcm_hal->access_err_cnt = 0;
    
    vcm_hal->vcm_init = _vcm_max20743_init;
    vcm_hal->get_temp = _vcm_max20743_get_temp;
    vcm_hal->get_volt = _vcm_max20743_get_volt;
    vcm_hal->get_cur = _vcm_max20743_get_cur;
    vcm_hal->reg_read = _vcm_max20743_register_read;
    vcm_hal->reg_write = _vcm_max20743_register_write;

    return RESULT_OK;
}


