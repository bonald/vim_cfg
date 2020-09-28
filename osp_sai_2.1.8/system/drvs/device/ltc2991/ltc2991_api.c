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
#include "ltc2991_api.h"
#include "ltc2991_drv.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define LTC2991_ACCESS_ERROR_COUNT_MAX 3
#define LTC2991_TIMEOUT   5

/* voltage ltc2991 handler */
static ltc2991_handle_t* g_ltc2991_hdl;
static uint32 g_ltc2991_num;

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
#define VOLT_VALUE_LEN 2
#define REG_VALUE_LEN  1
/****************************************************************************
 *
* Functions  
*
****************************************************************************/
// Reads a 14-bit adc_code from LTC2991.
int32 ltc2991_adc_read(ltc2991_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid)
{
    int32 ret = 0;
    uint8 code[2];
    uint16 temp = 0;
    ltc2991_access_t access;
    
    access.reg = msb_register_address;
    access.val = code;
    access.len = VOLT_VALUE_LEN;
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
int32 ltc2991_adc_read_timeout(ltc2991_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid, uint16 timeout)
{
    uint16 timer_count;  // Timer count for data_valid
    int32 ret = 0;
    
    for (timer_count = 0; timer_count < timeout; timer_count++)
    {
        ret |= ltc2991_adc_read(handle, msb_register_address, &(*adc_code), &(*data_valid));   //! 1)Read ADC until data valid bit is set
        if ((ret) || (*data_valid == 1)) break;
        sal_delay(1);
    }
    
    return ret;
}

// Reads new data (even after a mode change) by flushing old data and waiting for the data_valid bit to be set.
// This function simplifies adc reads when modes are changing.  For example, if V1-V2 changes from temperature mode
// to differential voltage mode, the data in the register may still correspond to the temperature reading immediately
// after the mode change.  Flushing one reading and waiting for a new reading guarantees fresh data is received.
// If the timeout is reached without valid data (*data_valid=1) the function exits.
int32 ltc2991_adc_read_new_data(ltc2991_handle_t* handle, uint8 msb_register_address, int16 *adc_code, int8 *data_valid, uint16 timeout)
{
  int8 ack = 0;

  ack |= ltc2991_adc_read(handle, msb_register_address, &(*adc_code), &(*data_valid)); //! 1)  Throw away old data
  ack |= ltc2991_adc_read_timeout(handle, msb_register_address, &(*adc_code), &(*data_valid), timeout); //! 2) Read new data

  return(ack);
}

// Reads an 8-bit register from the LTC2991 using the standard repeated start format.
int32 
_ltc2991_register_read(ltc2991_handle_t* handle, uint8 reg, uint8* value)
{
    ltc2991_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = REG_VALUE_LEN;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

// Write one byte to an LTC2991 register.
// Writes to an 8-bit register inside the LTC2991 using the standard I2C repeated start format.
int32
_ltc2991_register_write(ltc2991_handle_t* handle, uint8 reg, uint8* value)
{
    ltc2991_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = REG_VALUE_LEN;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

// Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
// bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
int32 
_ltc2991_register_set_clear_bits(ltc2991_handle_t* handle, uint8 reg, uint8 bits_to_set, uint8 bits_to_clear)
{
  uint8 register_data;
  int8 ack = 0;
    
  ack |= _ltc2991_register_read(handle, reg, &register_data);  //! 1) Read register
  register_data = register_data & (~bits_to_clear); //! 2) Clear bits that were set to be cleared
  register_data = register_data | bits_to_set;
  ack |= _ltc2991_register_write(handle, reg, &register_data);  //! 3) Write to register with the cleared bits

  return(ack);
}

int32
_ltc2991_get_volt(ltc2991_handle_t* handle, ltc2991_volt_type_t type, float32* voltage)
{    
    int32 ret = 0;
    int16 adc_code = 0;
    int8  data_valid = 0;
    uint8 reg = 0;
    uint8 value = 0;
    
    switch(type)
    {
        case LTC2991_SINGLE_V1_VOLTAGE:
            reg = LTC2991_V1_MSB_REG;
            break;
        case LTC2991_SINGLE_V2_VOLTAGE:
            reg = LTC2991_V2_MSB_REG;
            break;
        case LTC2991_SINGLE_V3_VOLTAGE:
            reg = LTC2991_V3_MSB_REG;
            break;
        case LTC2991_SINGLE_V4_VOLTAGE:
            reg = LTC2991_V4_MSB_REG;
            break;
        case LTC2991_SINGLE_V5_VOLTAGE:
            reg = LTC2991_V5_MSB_REG;
            break;
        case LTC2991_SINGLE_V6_VOLTAGE:
            reg = LTC2991_V6_MSB_REG;
            break;
        case LTC2991_SINGLE_V7_VOLTAGE:
            reg = LTC2991_V7_MSB_REG;
            break;
        case LTC2991_SINGLE_V8_VOLTAGE:
            reg = LTC2991_V8_MSB_REG;
            break;
        case LTC2991_VCC_VOLTAGE:
            reg = LTC2991_Vcc_MSB_REG;
            break;
        default:
            DRV_LOG_ERR("ltc2991 voltage type is unspport\n"); 
            return RESULT_ERROR;    
    }

    /*trigger*/
    value = 0xf8;
    _ltc2991_register_write(handle, LTC2991_CHANNEL_ENABLE_REG, &value);
    
    /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading */
#ifdef CNOS_OS
    ret = ltc2991_adc_read_new_data(handle, reg, &adc_code, &data_valid, LTC2991_TIMEOUT);
#else
    ret = ltc2991_adc_read_new_data(handle, reg, &adc_code, &data_valid, 0);
#endif
    //ret = ltc2991_adc_read_timeout(handle, reg, &adc_code, &data_valid, LTC2991_TIMEOUT);
    if(ret /*|| !data_valid*/)
    {
        *voltage = 0;
        DRV_LOG_ERR("ltc2991 voltage value is invalid, reg type:%d\n", type); 
        return RESULT_ERROR;
    }
    
    if(LTC2991_VCC_VOLTAGE == type)
    {
        *voltage = ltc2991_code_to_vcc_voltage(adc_code, LTC2991_VCC_lsb);
    }
    else
    {
        /* Converts code to voltage from single-ended lsb */
        *voltage = ltc2991_code_to_single_ended_voltage(adc_code, LTC2991_SINGLE_ENDED_lsb);
    }

    return ret;
}

int32
_ltc2991_get_cur(ltc2991_handle_t* handle, ltc2991_cur_type_t type, float32 rsense, float32* current)
{    
    int32 ret = 0;
    int16 adc_code = 0;
    int8  data_valid = 0;
    uint8 reg = 0;
    uint8 value = 0;
    float32 voltage = 0;
    switch(type)
    {
        case LTC2991_V1_V2_CURRENT:
            reg = LTC2991_V2_MSB_REG;
            break;
        case LTC2991_V3_V4_CURRENT:
            reg = LTC2991_V4_MSB_REG;
            break;
        case LTC2991_V5_V6_CURRENT:
            reg = LTC2991_V6_MSB_REG;
            break;
        case LTC2991_V7_V8_CURRENT:
            reg = LTC2991_V8_MSB_REG;
            break;
        default:
            DRV_LOG_ERR("ltc2991 current type is unspport\n"); 
            return RESULT_ERROR;    
    }
    
    /*trigger*/
    value = 0xf8;
    _ltc2991_register_write(handle, LTC2991_CHANNEL_ENABLE_REG, &value);

    /* Flush one ADC reading in case it is stale.  Then, take a new fresh reading */
    ret = ltc2991_adc_read_new_data(handle, reg, &adc_code, &data_valid, LTC2991_TIMEOUT);
    //ret = ltc2991_adc_read_timeout(handle, reg, &adc_code, &data_valid, LTC2991_TIMEOUT);
    if(ret /*|| !data_valid*/)
    {
        *current = 0;
        DRV_LOG_ERR("ltc2991 current value is invalid, current type:%d\n", type); 
        return RESULT_ERROR;
    }

    /* Converts code to differential voltage, then calculate current */
    voltage = ltc2991_code_to_differential_voltage(adc_code, LTC2991_DIFFERENTIAL_lsb);
    *current = voltage/rsense;
    //DRV_LOG_INFO("voltage:%f, rsense:%f, current:%f\n", voltage, rsense, *current);    
    return ret;
}

int32
_ltc2991_init(ltc2991_handle_t* handle, glb_board_type_t board_type)
{
    int32 ret = 0;

    //if(GLB_SERIES_CPCI_6U == board_type.series)
    //{
    //    ret += _ltc2991_register_set_clear_bits(handle, LTC2991_CONTROL_V1234_REG, 0x1, 0xfe);
    //}

    ret += _ltc2991_register_set_clear_bits(handle, LTC2991_CHANNEL_ENABLE_REG, 0xf8, 0x7);
    
    if(ret)
    {
        DRV_LOG_ERR("ltc2991_init fail\n"); 
        return RESULT_ERROR;
    }
    
    return RESULT_OK;
}

int32 
ltc2991_register_driver(ltc2991_handle_t* handle)
{
    handle->ltc2991_init = _ltc2991_init;
    handle->get_volt= _ltc2991_get_volt;
    handle->get_cur = _ltc2991_get_cur;
    handle->reg_read = _ltc2991_register_read;
    handle->reg_write = _ltc2991_register_write;
    
    return RESULT_OK;
}

/* init ltc2991 chip */
int32
ltc2991_dev_init(glb_board_type_t board_type, uint32 ltc2991_id)
{
    if(ltc2991_id >= g_ltc2991_num)
    {
        DRV_LOG_ERR("ltc2991 id %d is error!, max is %d\n", ltc2991_id, g_ltc2991_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc2991_hdl[ltc2991_id].ltc2991_init);
    return g_ltc2991_hdl[ltc2991_id].ltc2991_init(&g_ltc2991_hdl[ltc2991_id], board_type);
}

/* get voltage from ltc2991, return value: RESULT_OK, RESULT_ERROR  */
int32
ltc2991_dev_get_volt(uint32 ltc2991_id, ltc2991_volt_type_t type, float32* voltage)
{
    int32 ret = 0;
    
    if(ltc2991_id >= g_ltc2991_num)
    {
        DRV_LOG_ERR("ltc2991 id %d is error!, max is %d\n", ltc2991_id, g_ltc2991_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc2991_hdl[ltc2991_id].get_volt);
    ret = g_ltc2991_hdl[ltc2991_id].get_volt(&g_ltc2991_hdl[ltc2991_id], type, voltage);

    if(ret != RESULT_OK)
    {
        g_ltc2991_hdl[ltc2991_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_ltc2991_hdl[ltc2991_id].access_err_cnt > LTC2991_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *voltage = 0;
            g_ltc2991_hdl[ltc2991_id].access_err_cnt=0;
        }
    }
    else
    {
        g_ltc2991_hdl[ltc2991_id].access_err_cnt=0;
    }
    
    return ret;
}

/* get current from ltc2991, return value: RESULT_OK, RESULT_ERROR  */
int32
ltc2991_dev_get_cur(uint32 ltc2991_id, ltc2991_cur_type_t type, float32 rsense, float32* voltage)
{
    int32 ret = 0;
    
    if(ltc2991_id >= g_ltc2991_num)
    {
        DRV_LOG_ERR("ltc2991 id %d is error!, max is %d\n", ltc2991_id, g_ltc2991_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc2991_hdl[ltc2991_id].get_cur);
    ret = g_ltc2991_hdl[ltc2991_id].get_cur(&g_ltc2991_hdl[ltc2991_id], type, rsense, voltage);

    if(ret != RESULT_OK)
    {
        g_ltc2991_hdl[ltc2991_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_ltc2991_hdl[ltc2991_id].access_err_cnt > LTC2991_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *voltage = 0;
            g_ltc2991_hdl[ltc2991_id].access_err_cnt=0;
        }
    }
    else
    {
        g_ltc2991_hdl[ltc2991_id].access_err_cnt=0;
    }
    
    return RESULT_OK;
}

int32
ltc2991_reg_read(uint32 ltc2991_id, uint8 reg, uint8* value, uint8 len)
{
    if(ltc2991_id >= g_ltc2991_num)
    {
        DRV_LOG_ERR("ltc2991 id %d is error!, max is %d\n", ltc2991_id, g_ltc2991_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc2991_hdl[ltc2991_id].reg_read);
    
    return g_ltc2991_hdl[ltc2991_id].reg_read(&g_ltc2991_hdl[ltc2991_id], reg, value);
}

int32
ltc2991_reg_write(uint32 ltc2991_id, uint8 reg, uint8* value, uint8 len)
{
    if(ltc2991_id >= g_ltc2991_num)
    {
        DRV_LOG_ERR("ltc2991 id %d is error!, max is %d\n", ltc2991_id, g_ltc2991_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_ltc2991_hdl[ltc2991_id].reg_read);
    return g_ltc2991_hdl[ltc2991_id].reg_write(&g_ltc2991_hdl[ltc2991_id], reg, value);
}

#ifndef CNOS_OS
int32 
ltc_scan_real_time_value(uint32 id, volt_curr_status_t status[],volt_curr_cfg_t cfg[],uint8 channel_no_max,float *rsense)
{
    uint32 i;
    float32 value;
    uint8 ret=RESULT_OK;
    for(i=0; i<channel_no_max; i++)
    {
        /*measure volt*/
        if(cfg[i].flag & GLB_LTC_PIN_VOLT)                   
        {
            if(RESULT_OK == ltc2991_dev_get_volt(id, i,&value))
            {
                 /*get the real volt*/
                status[i].value = value*cfg[i].volt_split_factor;   
            }
            else
            {
                /*invalid data*/  
                ret=RESULT_ERROR;
            }
        }
        else if(cfg[i].flag & GLB_LTC_PIN_CURR)          /*measure current*/
        {
            if(((i % 2) == 1) && (RESULT_OK == ltc2991_dev_get_volt(id, i,&value)))
            {
                status[i].value = (status[i-1].value - value * cfg[i].volt_split_factor) / rsense[i] ;
            }
            else
            {
                ret=RESULT_ERROR;
            }
        }
        else                                   /*not use*/
        {
        }  
    }
    return ret;
}
#endif

/*********************************************************************
 * Name    : ltc2991_init
 * Purpose :  init some data structure and register ltc2991 dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of ltc2991'i2c bus implement
          uint32 num         - the number of ltc2991
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
ltc2991_init(void** p_data, ltc2991_chip_t* type, uint32 num)
{
    int32 i = 0;

    DRV_CTC_CHK_PTR(p_data);  
    DRV_CTC_CHK_PTR(type);
    
    g_ltc2991_hdl = (ltc2991_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_LTC2991_INFO, sizeof(ltc2991_handle_t)*num);
    if(NULL == g_ltc2991_hdl)
    {
        DRV_LOG_ERR("ltc2991 alloc handler fail!\n");
        return RESULT_ERROR;
    }
    
    /* register ltc2991 dev */
    for (i = 0; i < num; i++)
    {
        g_ltc2991_hdl[i].io_hdl = ltc2991_io_register(type[i].io_type, p_data[i]);
	 g_ltc2991_hdl[i].access_err_cnt = 0;
        switch(type[i].chip_type)
        {
            case LTC2991_CMS:
            case LTC2991_LMS:
                ltc2991_register_driver(&g_ltc2991_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport ltc2991 chip type %d!\n", type[i].chip_type);
                return RESULT_ERROR;
        }
    }

    g_ltc2991_num = num;
    return RESULT_OK;
}

