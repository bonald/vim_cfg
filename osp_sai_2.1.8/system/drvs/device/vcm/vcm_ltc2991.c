/****************************************************************************
 * ltc2991_api.c    ltc2991 api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       liangf 
 * Date:         2014-01-26
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
#include "vcm_ltc2991.h"

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
// Calibration Variables
//! Typical single-ended LSB weight in volts
const float32 VCM_LTC2991_SINGLE_ENDED_lsb = 3.05176E-04;
//! Typical differential LSB weight in volts
const float32 VCM_LTC2991_DIFFERENTIAL_lsb = 1.90735E-05;
//! Typical VCC LSB weight in volts
const float32 VCM_LTC2991_VCC_lsb = 3.05176E-04;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
const float32 VCM_LTC2991_TEMPERATURE_lsb = 0.0625;
//! Typical remote diode LSB weight in volts.
//! Used to readback diode voltage when in temperature measurement mode.
const float32 VCM_LTC2991_DIODE_VOLTAGE_lsb = 3.815E-05;

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
// Reads a 14-bit adc_code from LTC2991.
int32 vcm_ltc2991_adc_read(vcm_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid)
{
    int32 ret = 0;
    uint8 code[2];
    uint16 temp = 0;
    vcm_io_access_t access;
    
    access.reg = msb_register_address;
    access.val = code;
    access.len = 2;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
   
    if(!ret)
    {
        //*data_valid = (code >> 15) & 0x01;   // Place Data Valid Bit in *data_valid  
        //*adc_code = code & 0x7FFF;  // Removes data valid bit to return proper adc_code value
        *data_valid = (code[0]>>7) & 0x01;
        temp = code[0];
        temp = (temp<<8)|code[1];
        *adc_code = temp&0x7fff;
    }
    
    return ret;
}

// Reads a 14-bit adc_code from the LTC2991 but enforces a maximum timeout.
// Similar to LTC2991_adc_read except it repeats until the data_valid bit is set, it fails to receive an I2C acknowledge, or the timeout (in milliseconds)
// expires. It keeps trying to read from the LTC2991 every millisecond until the data_valid bit is set (indicating new data since the previous
// time this register was read) or until it fails to receive an I2C acknowledge (indicating an error on the I2C bus).
int32 vcm_ltc2991_adc_read_timeout(vcm_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid, uint16 timeout)
{
    uint16 timer_count;  // Timer count for data_valid
    int32 ret = 0;
    
    for (timer_count = 0; timer_count < timeout; timer_count++)
    {
        ret |= vcm_ltc2991_adc_read(handle, msb_register_address, &(*adc_code), &(*data_valid));   //! 1)Read ADC until data valid bit is set
        if ((ret) || (*data_valid == 1)) break;
        sal_udelay(100);
    }
    
    return ret;
}

// Reads an 8-bit register from the LTC2991 using the standard repeated start format.
int32 
_vcm_ltc2991_register_read(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = 1;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

// Write one byte to an LTC2991 register.
// Writes to an 8-bit register inside the LTC2991 using the standard I2C repeated start format.
int32
_vcm_ltc2991_register_write(vcm_handle_t* handle, uint8 reg, uint8* value)
{
    vcm_io_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = 1;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

// Reads new data (even after a mode change) by flushing old data and waiting for the data_valid bit to be set.
// This function simplifies adc reads when modes are changing.  For example, if V1-V2 changes from temperature mode
// to differential voltage mode, the data in the register may still correspond to the temperature reading immediately
// after the mode change.  Flushing one reading and waiting for a new reading guarantees fresh data is received.
// If the timeout is reached without valid data (*data_valid=1) the function exits.
int32 vcm_ltc2991_adc_read_new_data(vcm_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid, uint16 timeout)
{
#if 0    
  int8 ack = 0;

  ack |= vcm_ltc2991_adc_read(handle, msb_register_address, &(*adc_code), &(*data_valid)); //! 1)  Throw away old data
  ack |= vcm_ltc2991_adc_read_timeout(handle, msb_register_address, &(*adc_code), &(*data_valid), timeout); //! 2) Read new data

  return(ack);
#endif
    /*fix bug44386, liangf, 2017-06-21*/
    int32 ret = 0;
    int32 timer_count = 0;
    uint8 val = 0;
    
    for (timer_count = 0; timer_count < timeout; timer_count++)
    {
        _vcm_ltc2991_register_read(handle, VCM_LTC2991_CHANNEL_ENABLE_REG, &val);
        if (val & VCM_LTC2991_BUSY)
        {
            //DRV_LOG_ERR("ltc2991 still in busy, msb_register_address:%d, timer_count:%d, val:0x%x\n", msb_register_address, timer_count, val); 
            sal_udelay(1000);
        }
        else 
        {
             //DRV_LOG_ERR("ltc2991 finish, msb_register_address:%d, timer_count:%d, val:0x%x\n", msb_register_address, timer_count, val);       
            break;
        }
    }
    if(timer_count == timeout)
    {
       //DRV_LOG_ERR("ltc2991 still in busy, msb_register_address:%d\n", msb_register_address); 
        return -1;
    }
    ret = vcm_ltc2991_adc_read(handle, msb_register_address, &(*adc_code), &(*data_valid));
    
    return ret;

}

// Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
// bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
int32 
vcm_ltc2991_register_set_clear_bits(vcm_handle_t* handle, uint8 reg, uint8 bits_to_set, uint8 bits_to_clear)
{
  uint8 register_data;
  int8 ack = 0;
    
  ack |= handle->reg_read(handle, reg, &register_data);  //! 1) Read register
  register_data = register_data & (~bits_to_clear); //! 2) Clear bits that were set to be cleared
  register_data = register_data | bits_to_set;
  ack |= handle->reg_write(handle, reg, &register_data);  //! 3) Write to register with the cleared bits

  return(ack);
}

// Calculates the LTC2991 single-ended input voltages
float32 vcm_ltc2991_code_to_single_ended_voltage(int16 adc_code, float32 LTC2991_single_ended_lsb)
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
float32 vcm_ltc2991_code_to_vcc_voltage(int16 adc_code, float32 LTC2991_single_ended_lsb)
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
float32 vcm_ltc2991_code_to_differential_voltage(int16 adc_code, float32 LTC2991_differential_lsb)
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
float32 vcm_ltc2991_temperature(int16 adc_code, float32 LTC2991_temperature_lsb, int8 unit)
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
float32 vcm_ltc2991_code_to_diode_voltage(int16 adc_code, float32 LTC2991_diode_voltage_lsb)
{
  float32 voltage;
  adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
  voltage = ((float32) adc_code) * LTC2991_diode_voltage_lsb;     //! 2) Convert code to voltage from diode voltage lsb
  return (voltage);
}

int32
_vcm_ltc2991_get_volt(vcm_handle_t* handle, vcm_access_t para, float32* voltage)
{    
    int32 ret = 0;
    int16 adc_code = 0;
    int8  data_valid = 0;
    uint8 reg = 0;
    /* channel_no   type            
        0       LTC2991_SINGLE_V1_VOLT,
        1       LTC2991_SINGLE_V2_VOLT,
        2       LTC2991_SINGLE_V3_VOLT,
        3       LTC2991_SINGLE_V4_VOLT,
        4       LTC2991_SINGLE_V5_VOLT,
        5       LTC2991_SINGLE_V6_VOLT,
        6       LTC2991_SINGLE_V7_VOLT,
        7       LTC2991_SINGLE_V8_VOLT,        
        8       LTC2991_VCC_VOLT,
    */ 
    switch(para.channel_no)
    {
        case 0:
            reg = VCM_LTC2991_V1_MSB_REG;
            break;
        case 1:
            reg = VCM_LTC2991_V2_MSB_REG;
            break;
        case 2:
            reg = VCM_LTC2991_V3_MSB_REG;
            break;
        case 3:
            reg = VCM_LTC2991_V4_MSB_REG;
            break;
        case 4:
            reg = VCM_LTC2991_V5_MSB_REG;
            break;
        case 5:
            reg = VCM_LTC2991_V6_MSB_REG;
            break;
        case 6:
            reg = VCM_LTC2991_V7_MSB_REG;
            break;
        case 7:
            reg = VCM_LTC2991_V8_MSB_REG;
            break;
        case 8:
            reg = VCM_LTC2991_Vcc_MSB_REG;
            break;
        default:
            DRV_LOG_ERR("ltc2991 voltage type is unspport\n"); 
            return RESULT_ERROR;    
    }
    
    /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading */
    ret = vcm_ltc2991_adc_read_new_data(handle, reg, &adc_code, &data_valid, 5);
    if(ret /*|| !data_valid*/)
    {
        *voltage = 0;
        //DRV_LOG_ERR("ltc2991 voltage value is invalid, reg 0x%x, ret %d\n", reg, ret); 
        return RESULT_ERROR;
    }
    
    if(8 == para.channel_no) /* for LTC2991_VCC_VOLT */
    {
        *voltage = vcm_ltc2991_code_to_vcc_voltage(adc_code, VCM_LTC2991_VCC_lsb);
    }
    else
    {
        /* Converts code to voltage from single-ended lsb */
        *voltage = vcm_ltc2991_code_to_single_ended_voltage(adc_code, VCM_LTC2991_SINGLE_ENDED_lsb);
    }

    return ret;
}

int32
_vcm_ltc2991_get_cur(vcm_handle_t* handle, vcm_access_t para, float32* current)
{    
    int32 ret = 0;
    int16 adc_code = 0;
    int8  data_valid = 0;
    uint8 reg = 0;
    float32 voltage = 0;
    float32 rsense;
    rsense = handle->vcm_info.vcm_rsense;
    switch(para.channel_no)
    {
        case 0:
            reg = VCM_LTC2991_V2_MSB_REG;
            break;
        case 1:
            reg = VCM_LTC2991_V4_MSB_REG;
            break;
        case 2:
            reg = VCM_LTC2991_V6_MSB_REG;
            break;
        case 3:
            reg = VCM_LTC2991_V8_MSB_REG;
            break;
        default:
            DRV_LOG_ERR("ltc2991 current type is unspport\n"); 
            return RESULT_ERROR;    
    }

    /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading, for timeout set to 5 */
    ret = vcm_ltc2991_adc_read_new_data(handle, reg, &adc_code, &data_valid, 5);
    if(ret /*|| !data_valid*/)
    {
        *current = 0;
        //DRV_LOG_ERR("ltc2991 current value is invalid, current reg:%d\n", reg); 
        return RESULT_ERROR;
    }

    /* Converts code to differential voltage, then calculate current */
    voltage = vcm_ltc2991_code_to_differential_voltage(adc_code, VCM_LTC2991_DIFFERENTIAL_lsb);
    *current = voltage/rsense;

    return ret;
}

/*fix bug44386, liangf, 2017-06-21*/
int32
_vcm_ltc2991_trigger(vcm_handle_t* handle, vcm_access_t para)
{
    uint8 value = 0;
    
    switch(para.channel_no)
    {
      case 0://LTC2991_SINGLE_V1_VOLTAGE:
      case 1://LTC2991_SINGLE_V2_VOLTAGE:
        value = 0x10;
        break;
      case 2://LTC2991_SINGLE_V3_VOLTAGE:
      case 3://LTC2991_SINGLE_V4_VOLTAGE:
        value = 0x20;
        break;
      case 4://LTC2991_SINGLE_V5_VOLTAGE:
      case 5://LTC2991_SINGLE_V6_VOLTAGE:
        value = 0x40;
        break;
      case 6://LTC2991_SINGLE_V7_VOLTAGE:
      case 7://LTC2991_SINGLE_V8_VOLTAGE:       
        value = 0x80;
        break;
      case 8://LTC2991_VCC_VOLTAGE:
        value = 0x08;
        break;
      case 9://LTC2991_MAX_VOLTAGE:
            value = 0xf8;
            break;
       default:
            value = 0xf8;
            break;
    }
    
     return _vcm_ltc2991_register_write(handle, VCM_LTC2991_CHANNEL_ENABLE_REG, &value);   
}

int32
_vcm_ltc2991_init(vcm_handle_t* handle)
{
    int32 ret = 0;

    ret += vcm_ltc2991_register_set_clear_bits(handle, VCM_LTC2991_CHANNEL_ENABLE_REG, 0xf8, 0x7);
    
    if(ret)
    {
        DRV_LOG_ERR("ltc2991_init fail\n"); 
        return RESULT_ERROR;
    }
    
    return RESULT_OK;
}

int32
vcm_ltc2991_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info)
{
    vcm_handle_t* vcm_hal;
    DRV_CTC_CHK_PTR(vcm_info);  
    
    vcm_hal = hdl;
    /* register ltc2991 dev */
    memcpy(&vcm_hal->vcm_info, vcm_info, sizeof(vcm_info_t));
    vcm_hal->io_hdl = vcm_io_register(vcm_info->vcm_io_type, vcm_info->vcm_gen);
    vcm_hal->access_err_cnt = 0;
    
    vcm_hal->vcm_init = _vcm_ltc2991_init;
    vcm_hal->get_volt = _vcm_ltc2991_get_volt;
    vcm_hal->get_cur = _vcm_ltc2991_get_cur;
    vcm_hal->reg_read = _vcm_ltc2991_register_read;
    vcm_hal->reg_write = _vcm_ltc2991_register_write;
    vcm_hal->trigger = _vcm_ltc2991_trigger;

    return RESULT_OK;
}

