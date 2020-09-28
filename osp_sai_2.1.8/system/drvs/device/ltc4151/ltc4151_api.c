/****************************************************************************
 * ltc4151_api.c    ltc4151 api 
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
#include "ltc4151_api.h"
#include "ltc4151_drv.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define VOLT_VALUE_LEN 2
#define REG_VALUE_LEN  1

#define LTC4151_ACCESS_ERROR_COUNT_MAX 3
/* voltage ltc4151 handler */
static ltc4151_handle_t* g_ltc4151_hdl;
static uint32 g_ltc4151_num;

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
// Calibration Variables 
const float32 LTC4151_SENSE_RESISTOR = 0.5; /*milli-ohms*/
//! Typical single LSB weight in volts
const float32 LTC4151_SINGLE_lsb =  0.02; //0.02;  //0.015; /*milli-volt*/
//! Typical VIN LSB weight in volts
const float32 LTC4151_VIN_lsb = 0.025;  /*???*/

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
// Reads a 12-bit value from LTC4151
int8 
ltc4151_read_12_bits(ltc4151_handle_t* handle, uint8 adc_command, uint16 *adc_code)
{
    int32 ret = 0;
    uint8 code[2];
    uint16 temp = 0;
    ltc4151_access_t access;

    access.reg = adc_command;
    access.val = code;
    access.len = VOLT_VALUE_LEN;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
    if(!ret)
    {
        temp = code[0];
        temp = (temp<<8) | code[1];
        *adc_code = temp>>4;  
    }
    
  return ret;
}

// Reads an 8-bit register from the LTC4151 using the standard repeated start format.
int32
_ltc4151_register_read(ltc4151_handle_t* handle, uint8 reg, uint8* value)
{
    ltc4151_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = REG_VALUE_LEN;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

// Write one byte to an LTC4151 register.
// Writes to an 8-bit register inside the LTC4151 using the standard I2C repeated start format.
int32
_ltc4151_register_write(ltc4151_handle_t* handle, uint8 reg, uint8* value)
{
    ltc4151_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = REG_VALUE_LEN;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}
#if 0
// Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
// bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
int32 
_ltc4151_register_set_clear_bits(ltc4151_handle_t* handle, uint8 reg, uint8 bits_to_set, uint8 bits_to_clear)
{
  uint8 register_data;
  int8 ack = 0;
    
  ack |= ltc4151_register_read(handle, reg, &register_data);  //! 1) Read register
  register_data = register_data & (~bits_to_clear); //! 2) Clear bits that were set to be cleared
  register_data = register_data | bits_to_set;
  ack |= ltc4151_register_write(handle, reg, register_data);  //! 3) Write to register with the cleared bits

  return(ack);
}
#endif
int32
_ltc4151_get_volt_cur(ltc4151_handle_t* handle, float32* voltage, float32* current, float rsense)
{    
    int32 ret = 0;
    uint16 adc_code = 0;
    
    ret = ltc4151_read_12_bits(handle, LTC4151_SENSE_MSB_REG, &adc_code);
    *current = ltc4151_code_to_sense_current(adc_code, rsense, LTC4151_SINGLE_lsb);

    ret = ltc4151_read_12_bits(handle, LTC4151_VIN_MSB_REG, &adc_code);
    *voltage = ltc4151_code_to_vin_voltage(adc_code, LTC4151_VIN_lsb);

    return ret;
}

int32
_ltc4151_init(ltc4151_handle_t* handle)
{
    /*set ctrl reg*/
    return RESULT_OK;
}

int32 
ltc4151_register_driver(ltc4151_handle_t* handle)
{
    handle->ltc4151_init = _ltc4151_init;
    handle->get_volt_cur= _ltc4151_get_volt_cur;
    handle->reg_read = _ltc4151_register_read;
    handle->reg_write = _ltc4151_register_write;
    
    return RESULT_OK;
}

/* init ltc4151 chip */
int32
ltc4151_dev_init(uint32 ltc4151_id)
{
    if(ltc4151_id >= g_ltc4151_num)
    {
        DRV_LOG_ERR("ltc4151 id %d is error!, max is %d\n", ltc4151_id, g_ltc4151_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc4151_hdl[ltc4151_id].ltc4151_init);
    return g_ltc4151_hdl[ltc4151_id].ltc4151_init(&g_ltc4151_hdl[ltc4151_id]);
}

/* set temp from ltc4151, return value: RESULT_OK, RESULT_ERROR  */
int32
ltc4151_dev_get_volt_cur(uint32 ltc4151_id, float32* voltage, float32* current, float *rsense)
{
    int32 ret = 0;
    
    if(ltc4151_id >= g_ltc4151_num)
    {
        DRV_LOG_ERR("ltc4151 id %d is error!, max is %d\n", ltc4151_id, g_ltc4151_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc4151_hdl[ltc4151_id].get_volt_cur);
    /*for ltc4151, the pin 0 used to meseasure voltage, pin 1 is used to meseasure current*/
    ret = g_ltc4151_hdl[ltc4151_id].get_volt_cur(&g_ltc4151_hdl[ltc4151_id], voltage, current, rsense[1]);

    if(ret != RESULT_OK)
    {
        g_ltc4151_hdl[ltc4151_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_ltc4151_hdl[ltc4151_id].access_err_cnt > LTC4151_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *voltage = 0;
            g_ltc4151_hdl[ltc4151_id].access_err_cnt=0;
        }
    }
    else
    {
        g_ltc4151_hdl[ltc4151_id].access_err_cnt=0;
    }
    
    return RESULT_OK;
}

int32
ltc4151_reg_read(uint32 ltc4151_id, uint8 reg, uint8* value, uint8 len)
{
    if(ltc4151_id >= g_ltc4151_num)
    {
        DRV_LOG_ERR("ltc4151 id %d is error!, max is %d\n", ltc4151_id, g_ltc4151_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc4151_hdl[ltc4151_id].reg_read);
    return g_ltc4151_hdl[ltc4151_id].reg_read(&g_ltc4151_hdl[ltc4151_id], reg, value);
}

int32
ltc4151_reg_write(uint32 ltc4151_id, uint8 reg, uint8* value, uint8 len)
{
    if(ltc4151_id >= g_ltc4151_num)
    {
        DRV_LOG_ERR("ltc4151 id %d is error!, max is %d\n", ltc4151_id, g_ltc4151_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc4151_hdl[ltc4151_id].reg_read);
    return g_ltc4151_hdl[ltc4151_id].reg_write(&g_ltc4151_hdl[ltc4151_id], reg, value);
}


/*********************************************************************
 * Name    : ltc4151_init
 * Purpose :  init some data structure and register ltc4151 dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of ltc4151'i2c bus implement
          uint32 num         - the number of ltc4151
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc4151_init(void** p_data, ltc4151_chip_t* type, uint32 num)
{
    int32 i = 0;

    DRV_CTC_CHK_PTR(p_data);  
    DRV_CTC_CHK_PTR(type);
    
    g_ltc4151_hdl = (ltc4151_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_LTC4151_INFO, sizeof(ltc4151_handle_t)*num);
    if(NULL == g_ltc4151_hdl)
    {
        DRV_LOG_ERR("ltc4151 alloc handler fail!\n");
        return RESULT_ERROR;
    }
    
    /* register ltc4151 dev */
    for (i = 0; i < num; i++)
    {
        g_ltc4151_hdl[i].io_hdl = ltc4151_io_register(type[i].io_type, p_data[i]);
        g_ltc4151_hdl[i].access_err_cnt = 0;
        switch(type[i].chip_type)
        {
            case LTC4151_CDD:
            case LTC4151_IDD:
                ltc4151_register_driver(&g_ltc4151_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport ltc4151 chip type %d!\n", type[i].chip_type);
                return RESULT_ERROR;
        }
    }

    g_ltc4151_num = num;
    return RESULT_OK;
}

