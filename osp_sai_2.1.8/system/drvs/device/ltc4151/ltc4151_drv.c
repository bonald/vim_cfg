/****************************************************************************
 * ltc4151_drv.c   ltc4151 access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liangf 
 * Date:         2014-01-26.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "drv_debug.h"
#include "ltc4151_drv.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/

/*********************************************************************
 * Name    : ltc4151_i2c_read
 * Purpose :  read a i2c type ltc4151 register
 * Input   : const ltc4151_io_handle_t *p_hdl       - the handler of the ltc4151
          ltc4151_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc4151_i2c_read(const ltc4151_io_handle_t* p_hdl, ltc4151_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_LTC4151,"Read I2C thermal ltc4151: len %d, offset %d ret %d\n", 
    //    i2c_para.len, i2c_para.offset, ret);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : ltc4151_i2c_write
 * Purpose :  write a i2c type ltc4151 register
 * Input   : const ltc4151_io_handle_t *p_hdl       - the handler of the ltc4151
          ltc4151_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc4151_i2c_write(const ltc4151_io_handle_t* p_hdl, ltc4151_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_LTC4151,"Write I2C thermal ltc4151: len %d, offset %d, val %d, ret %d\n", 
    //    i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : ltc4151_i2c_close
 * Purpose :  free memory and pointer
 * Input   : ltc4151_io_handle_t *phdl       - the handler of the ltc4151
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
ltc4151_i2c_close(ltc4151_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_LTC4151_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static ltc4151_io_handle_t *
_ltc4151_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    ltc4151_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (ltc4151_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_LTC4151_INFO,sizeof(ltc4151_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = ltc4151_i2c_close;
    p_hdl->read = ltc4151_i2c_read;
    p_hdl->write = ltc4151_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_LTC4151_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static ltc4151_io_handle_t *
_ltc4151_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _ltc4151_create_i2c_handle( i2c_phdl);      
}


/****************************************************************************
 * Name	: ltc4151_register
 * Purpose: register ltc4151 device handler, initialize the handler
 * Input	:ltc4151 type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the thermal ltc4151
 * Note	:
****************************************************************************/	
ltc4151_io_handle_t *
ltc4151_io_register(ltc4151_io_type_e type, const void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    DRV_CTC_CHK_PTR_NULL(p_data);
    
    switch(type)
    {
        case LTC4151_I2C:
            i2c_pgen = (i2c_gen_t *)p_data;
            return _ltc4151_i2c_register(i2c_pgen);

        default:
            break;        
    }
    
    return NULL;
}

// Note that the following functions are somewhat contrived, and the units are arbitrary.  LTC4151_ADIN_voltage could be given an LSB
// weight (LTC4151_adin_lsb) in volts instead of mV, and the returned voltage will be in volts instead of mV.
// Similarly, the LTC4151_sense_current function could be written to accept an LSB weight in amps or milliamps.
// That would eliminate the extra float32ing point divide operation based on the sense resistor value.

// Calculates the LTC4151 sense current in Amps given "resistor" value in ohms and "LTC4151_sense_lsb" LSB weight in volts.
float32 ltc4151_code_to_sense_current(uint16 adc_code, float32 resistor, float32 LTC4151_sense_lsb)
{
  float32 voltage, current;
  voltage = (float32)adc_code * LTC4151_sense_lsb;    //! 1) Convert code to voltage from sense lsb
  current = voltage / resistor;                     //! 2) Calculate Current, I = V/R
  return (current);
}

// Calculates the LTC4151 V_IN voltage given "LTC_vin_lsb" LSB weight in volts
float32 ltc4151_code_to_vin_voltage(uint16 adc_code, float32 LTC4151_vin_lsb)
{
  float32 voltage;
  voltage = (float32)adc_code * LTC4151_vin_lsb;  //! 1) Convert code to voltage from VIN lsb
  return (voltage);
}

// Calculates the LTC4151 ADIN voltage in mV given "LTC4151_adin_lsb" LSB weight in mV
float32 ltc4151_code_to_ADIN_voltage(uint16 adc_code, float32 LTC4151_adin_lsb)
{
  float32 voltage;
  voltage = (float32)adc_code * LTC4151_adin_lsb; //! 1) Convert Code to voltage from ADIN lsb
  return (voltage);
}
