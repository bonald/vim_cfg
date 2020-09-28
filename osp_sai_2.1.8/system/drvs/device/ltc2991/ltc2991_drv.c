/****************************************************************************
 * ltc2991_drv.c   ltc2991 access interface
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
#include "ltc2991_drv.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
// Calibration Variables
//! Typical single-ended LSB weight in volts
const float32 LTC2991_SINGLE_ENDED_lsb = 3.05176E-04;
//! Typical differential LSB weight in volts
const float32 LTC2991_DIFFERENTIAL_lsb = 1.90735E-05;
//! Typical VCC LSB weight in volts
const float32 LTC2991_VCC_lsb = 3.05176E-04;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
const float32 LTC2991_TEMPERATURE_lsb = 0.0625;
//! Typical remote diode LSB weight in volts.
//! Used to readback diode voltage when in temperature measurement mode.
const float32 LTC2991_DIODE_VOLTAGE_lsb = 3.815E-05;

/*********************************************************************
 * Name    : ltc2991_i2c_read
 * Purpose :  read a i2c type ltc2991 register
 * Input   : const ltc2991_io_handle_t *p_hdl       - the handler of the ltc2991
          ltc2991_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc2991_i2c_read(const ltc2991_io_handle_t* p_hdl, ltc2991_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_LTC2991,"Read I2C thermal ltc2991: len %d, offset %d ret %d\n", 
    //    i2c_para.len, i2c_para.offset, ret);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : ltc2991_i2c_write
 * Purpose :  write a i2c type ltc2991 register
 * Input   : const ltc2991_io_handle_t *p_hdl       - the handler of the ltc2991
          ltc2991_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc2991_i2c_write(const ltc2991_io_handle_t* p_hdl, ltc2991_access_t* p_para)
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
    //DRV_LOG_DEBUG(DRV_LTC2991,"Write I2C thermal ltc2991: len %d, offset %d, val %d, ret %d\n", 
    //    i2c_para.len, i2c_para.offset, *i2c_para.p_val, ret);    
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

/*********************************************************************
 * Name    : ltc2991_i2c_close
 * Purpose :  free memory and pointer
 * Input   : ltc2991_io_handle_t *phdl       - the handler of the ltc2991
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
ltc2991_i2c_close(ltc2991_io_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_LTC2991_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}

static ltc2991_io_handle_t *
_ltc2991_create_i2c_handle(i2c_handle_t *i2c_phdl)
{
    ltc2991_io_handle_t *p_hdl = NULL; 
    
    p_hdl = (ltc2991_io_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_LTC2991_INFO,sizeof(ltc2991_io_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = ltc2991_i2c_close;
    p_hdl->read = ltc2991_i2c_read;
    p_hdl->write = ltc2991_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_LTC2991_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}

static ltc2991_io_handle_t *
_ltc2991_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _ltc2991_create_i2c_handle( i2c_phdl);      
}


/****************************************************************************
 * Name	: ltc2991_register
 * Purpose: register ltc2991 device handler, initialize the handler
 * Input	:ltc2991 type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the thermal ltc2991
 * Note	:
****************************************************************************/	
ltc2991_io_handle_t *
ltc2991_io_register(ltc2991_io_type_e type, const void *p_data)
{
    i2c_gen_t *i2c_pgen;
    
    DRV_CTC_CHK_PTR_NULL(p_data);
    
    switch(type)
    {
        case LTC2991_I2C:
            i2c_pgen = (i2c_gen_t *)p_data;
            return _ltc2991_i2c_register(i2c_pgen);

        default:
            break;        
    }
    
    return NULL;
}

// Calculates the LTC2991 single-ended input voltages
float32 ltc2991_code_to_single_ended_voltage(int16 adc_code, float32 LTC2991_single_ended_lsb)
{
  float32 voltage;
  int16 sign = 1;
  if (adc_code >> 14)
  {
    adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1) Converts two's complement to binary
    sign = -1;
  }
  adc_code = (adc_code & 0x3FFF);
  voltage = ((float32) adc_code) * LTC2991_single_ended_lsb * sign;   //! 2) Convert code to voltage from lsb
  return (voltage);
}

// Calculates the LTC2991 Vcc voltage
float32 ltc2991_code_to_vcc_voltage(int16 adc_code, float32 LTC2991_single_ended_lsb)
{
  float32 voltage;
  int16 sign = 1;
  if (adc_code >> 14)
  {
    adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1) Converts two's complement to binary
    sign = -1;
  }
  
  voltage = (((float32) adc_code) * LTC2991_single_ended_lsb * sign) + 2.5; //! 2) Convert code to Vcc Voltage from single-ended lsb
  return (voltage);
}

// Calculates the LTC2991 differential input voltage.
float32 ltc2991_code_to_differential_voltage(int16 adc_code, float32 LTC2991_differential_lsb)
{
  float32 voltage;
  int16 sign = 1;
  if (adc_code >> 14)
  {
    adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1)Converts two's complement to binary
    sign = -1;
  }
  voltage = ((float32) adc_code) * LTC2991_differential_lsb * sign;   //! 2) Convert code to voltage form differential lsb
  return (voltage);
}

// Calculates the LTC2991 temperature
float32 ltc2991_temperature(int16 adc_code, float32 LTC2991_temperature_lsb, boolean unit)
{
  float32 temperature;
  adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
  if(!unit){                                                     //! 2)Checks to see if it's Kelvin
    if(adc_code >>12)
    {
        adc_code = (adc_code | 0xE000);                         //! Sign extend if it's not Kelvin (Celsius)
    }
  }
  temperature = ((float32) adc_code) * LTC2991_temperature_lsb;   //! 3) Converts code to temperature from temperature lsb
  
  return (temperature);
}

//  Calculates the LTC2991 diode voltage
float32 ltc2991_code_to_diode_voltage(int16 adc_code, float32 LTC2991_diode_voltage_lsb)
{
  float32 voltage;
  adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
  voltage = ((float32) adc_code) * LTC2991_diode_voltage_lsb;     //! 2) Convert code to voltage from diode voltage lsb
  return (voltage);
}
