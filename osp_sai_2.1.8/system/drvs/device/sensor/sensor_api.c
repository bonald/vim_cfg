/****************************************************************************
 * sensor_api.c    sensor api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-09-21.
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
#include "sensor_api.h"
#include "sensor_drv.h"
#include "glb_hw_define.h"
/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define SENSOR_ACCESS_ERROR_COUNT_MAX 3
#define LM77_TEMP_VALUE_LEN 2
#define LM77_REG_VALUE_LEN  1
#define MAX1617A_VALUE_LEN 1
#define TMP42X_VALUE_LEN 1

/* added by liuht for bug 24525 to support LM75,2013-0918 */
#define LM75_TEMP_VALUE_LEN 2
#define LM75_REG_VALUE_LEN  1

#define SD5075_TEMP_VALUE_LEN 2
#define SD5075_REG_VALUE_LEN   1

#define ADT7476A_REG_VALUE_LEN  1

/* temperature sensor handler */
static sensor_handle_t* g_sensor_hdl;
static uint32 g_sensor_num;

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


/*****LM77 driver start*****************************************************/

/****************************************************************************
 * Name	: _LM77_decode_temp_val
 * Purpose: decode the temperature of the sensor from raw register value to real temperature value 
 * Input	:  the raw register value
 * Output:   
 * Return:  the real temperature value
 * Note	: in register, bit[12,3] is temperature value, temperature step is 0.5.
 *        bug 16809. jqiu 2011-11-04
****************************************************************************/	
static int16 
_LM77_decode_temp_val(uint16 value)
{
    int16 tmpr;
    /* If >0, remove low 3 bits, then /2, is real temperature*/
    if ((value&0x1000) == 0)
    {
        tmpr = (value>>4);
    }
    else
    {
        /*If <0, remove low 3 bits, then /2, the low 9 bits is useful, other insert 1.*/
        tmpr = (0xfe00 | ((value>>4)&0x1ff));
    }
    return tmpr;
}

/****************************************************************************
 * Name	: _sensor_encode_temp_val
 * Purpose: encode the temperature of the sensor from real temperature value to raw register value
 * Input	:  the real temperatuer
 * Output:   
 * Return:  the raw register value write to sensor
 * Note	: in register, bit[12,3] is temperature value, temperature step is 0.5. 
 *        bug 16809. jqiu 2011-11-04
****************************************************************************/	
static uint16 
_LM77_encode_temp_val(int16 value)
{
    uint16 tmpr;
    /* If >0, value set to bit[12,4], and high 3 bits is sign 0. */
    if ((value&0x8000) == 0)
    {
        tmpr = (value << 4);
    }
    else
    {
        /*If <0, value set to bit[12,4], and high 3 bits is sign 1. */
        tmpr = 0xe000 | (value<<4);
    }
    return tmpr;
}

/*raw register read, not do data translate*/
static int32
_sensor_LM77_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
#ifdef _CTC_X86_64_
    if (len == 2)
        access.width = 2;
#endif
    return handle->io_hdl->read(handle->io_hdl, &access);
}

/*raw register write, not do data translate*/
static int32
_sensor_LM77_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
#ifdef _CTC_X86_64_
    if (len == 2)
        access.width = 2;
#endif
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_sensor_LM77_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{    
    int32 ret;
    uint8 reg;
    uint8 buf[LM77_TEMP_VALUE_LEN];
    uint16 val;
    
    reg = handle->reg_info[type];
    ret = _sensor_LM77_reg_read(handle, reg, buf, LM77_TEMP_VALUE_LEN);
    
    val = (buf[0]<<8)+buf[1];
    *temp = _LM77_decode_temp_val(val);
    return ret;
}

static int32
_sensor_LM77_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
    int32 ret;
    uint8 reg;
    uint8 buf[LM77_TEMP_VALUE_LEN];
    uint16 val;
    
    reg = handle->reg_info[type];
    val = _LM77_encode_temp_val(temp);
    buf[0] = (val>>8)&0xff;
    buf[1] = val & 0xff;
    ret = _sensor_LM77_reg_write(handle, reg, buf, LM77_TEMP_VALUE_LEN);     
    return ret;    
}

static int32
_sensor_LM77_init(sensor_handle_t* handle)
{
    _sensor_LM77_set_temp(handle, CRIT_TEMP, 65);
    _sensor_LM77_set_temp(handle, THSYT_TEMP, 2);
    return RESULT_OK;
}

static int32 
_sensor_LM77_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_LM77_init;
    handle->get_temp = _sensor_LM77_get_temp;
    handle->set_temp = _sensor_LM77_set_temp;
    handle->reg_read = _sensor_LM77_reg_read;
    handle->reg_write = _sensor_LM77_reg_write;
    handle->reg_info[CURRENT_TEMP] = 0;
    handle->reg_info[LOW_TEMP] = 0x4;
    handle->reg_info[HIGH_TEMP] = 0x5;
    handle->reg_info[THSYT_TEMP] = 0x2;    
    handle->reg_info[CRIT_TEMP] = 0x3;
    return RESULT_OK;
}

/*****LM77 driver end *****************************************************/

/*****MAX1617A driver start*************************************************/
static int32
_sensor_MAX1617A_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_sensor_MAX1617A_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}


static int32
_sensor_MAX1617A_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{
    int32 ret;
    uint8 reg;
    uint8 buf[MAX1617A_VALUE_LEN];
    int8 val;
    
    reg = handle->reg_info[type];
    ret = _sensor_MAX1617A_reg_read(handle, reg, buf, MAX1617A_VALUE_LEN);
    val = (int8)buf[0];
    *temp = (int16)val;
    return ret;
}

static int32
_sensor_MAX1617A_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
    int32 ret;
    uint8 reg;
    uint8 buf[MAX1617A_VALUE_LEN];
    
    reg = handle->reg_info[type];
    if(reg == 0xff)
        return RESULT_OK;
    buf[0] = (uint8)temp;
    ret = _sensor_MAX1617A_reg_write(handle, reg, buf, MAX1617A_VALUE_LEN);

    return ret;    
}

static int32
_sensor_MAX1617A_init(sensor_handle_t* handle)
{
    _sensor_MAX1617A_set_temp(handle, CRIT_TEMP, 90);
    return RESULT_OK;
}

static int32 
_sensor_MAX1617A_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_MAX1617A_init;
    handle->get_temp = _sensor_MAX1617A_get_temp;
    handle->set_temp = _sensor_MAX1617A_set_temp;
    handle->reg_read = _sensor_MAX1617A_reg_read;
    handle->reg_write = _sensor_MAX1617A_reg_write;
    handle->reg_info[CURRENT_TEMP] = 1;
    handle->reg_info[LOW_TEMP] = 0x8;
    handle->reg_info[HIGH_TEMP] = 0x7;
    handle->reg_info[THSYT_TEMP] = 0xff; /*Dont support this*/
    handle->reg_info[CRIT_TEMP] = 0xff; /*Dont support this*/
    return RESULT_OK;
}

/*****MAX1617A driver end **************************************************/

/*****LM75 driver start*****************************************************/

/****************************************************************************
 * Name	: _LM75_decode_temp_val
 * Purpose: decode the temperature of the sensor from raw register value to real temperature value 
 * Input	:  the raw register value
 * Output:   
 * Return:  the real temperature value
 * Note	: 
****************************************************************************/	
static int16 
_LM75_decode_temp_val(uint16 value)
{
    int16 tmpr;

    if ((value&0x8000) == 0)  /* positive tmpr data */
    {
        tmpr = (value>>8);   /* tmpr = value/8, and the low 5-bit is useless*/
    }
    else   /* negtive tmpr data */
    {
        value >>= 5;
        tmpr = (0x3ff ^ value);
        tmpr += 0x1;
        tmpr |= 0x400;
        tmpr &= 0x3ff;
        tmpr >>= 3;
        tmpr = 0 - tmpr;
    }
    return tmpr;
}

/****************************************************************************
 * Name	: _sensor_encode_temp_val
 * Purpose: encode the temperature of the sensor from real temperature value to raw register value
 * Input	:  the real temperatuer
 * Output:   
 * Return:  the raw register value write to sensor
 * Note	: 
****************************************************************************/	
static uint16 
_LM75_encode_temp_val(int16 value)
{
    uint16 tmpr;
    /* If >0, value set to bit[12,4], and high 3 bits is sign 0. */
    if ((value&0x8000) == 0)
    {
        tmpr = (value << 3);
        tmpr <<= 5;
    }
    else
    {
        /*If <0, value set to bit[12,4], and high 3 bits is sign 1. */
        //tmpr = 0xe000 | (value<<4);
        tmpr = (value << 3) & 0x7ff;
        tmpr <<= 5;
    }
    return tmpr;
}

/*raw register read, not do data translate*/
static int32
_sensor_LM75_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

/*raw register write, not do data translate*/
static int32
_sensor_LM75_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_sensor_LM75_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{    
    int32 ret;
    uint8 reg;
    uint8 buf[LM75_TEMP_VALUE_LEN];
    uint16 val;
    
    reg = handle->reg_info[type];
    ret = _sensor_LM75_reg_read(handle, reg, buf, LM75_TEMP_VALUE_LEN);
    
    val = (buf[0]<<8)+buf[1];
    *temp = _LM75_decode_temp_val(val);
    return ret;
}

static int32
_sensor_LM75_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
    int32 ret = 0;
    uint8 reg;
    uint8 buf[LM75_TEMP_VALUE_LEN];
    uint16 val;
    uint16 thsyt=2;

    /* set critical temp */
    reg = handle->reg_info[type];
    if(reg == 0xff)
        return RESULT_OK;
    val = _LM75_encode_temp_val(temp);
    buf[0] = (val>>8)&0xff;
    buf[1] = val & 0xff;
    ret = _sensor_LM75_reg_write(handle, reg, buf, LM75_TEMP_VALUE_LEN);   

    if(CRIT_TEMP == type)
    {
        /* set thsyt temp, which is critical temp - 2 */
        reg = handle->reg_info[THSYT_TEMP];
        val = _LM75_encode_temp_val(temp-thsyt);
        buf[0] = (val>>8)&0xff;
        buf[1] = val & 0xff;
        ret = _sensor_LM75_reg_write(handle, reg, buf, LM75_TEMP_VALUE_LEN);
    }
    
    return ret;    
}

static int32
_sensor_LM75_init(sensor_handle_t* handle)
{
    _sensor_LM75_set_temp(handle, CRIT_TEMP, 90);
    return RESULT_OK;
}

static int32 
_sensor_LM75_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_LM75_init;
    handle->get_temp = _sensor_LM75_get_temp;
    handle->set_temp = _sensor_LM75_set_temp;
    handle->reg_read = _sensor_LM75_reg_read;
    handle->reg_write = _sensor_LM75_reg_write;
    handle->reg_info[CURRENT_TEMP] = 0;
    /*deleted by liuht to support LM75,2013-09-06*/	
    //handle->reg_info[LOW_TEMP] = 0x4;
    //handle->reg_info[HIGH_TEMP] = 0x5;
    handle->reg_info[THSYT_TEMP] = 0x2;    
    handle->reg_info[CRIT_TEMP] = 0x3;
    return RESULT_OK;
}

/*****LM75 driver end *****************************************************/

/*****ADT7476A driver start*****************************************************/

/****************************************************************************
 * Name	: _ADT7476A_decode_temp_val
 * Purpose: decode the temperature of the sensor from raw register value to real temperature value 
 * Input	:  the raw register value
 * Output:   
 * Return:  the real temperature value
 * Note	: 
****************************************************************************/	
static int16 
_ADT7476A_decode_temp_val(uint16 value)
{
    int16 tmpr;

    if ((value&0x80) == 0)  /* positive tmpr data */
    {
        tmpr = (uint16)value;
    }
    else                   /* negtive tmpr data */
    {
        tmpr = (~value)+1;
        tmpr = 0-tmpr;
    }
    return tmpr;
}

static int32
_sensor_ADT7476A_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_sensor_ADT7476A_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_sensor_ADT7476A_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{    
    int32 ret;
    uint8 reg;
    uint8 buf[ADT7476A_REG_VALUE_LEN];
    uint16 val;

    reg = handle->reg_info[type];
    reg += handle->chan;
    ret = _sensor_ADT7476A_reg_read(handle, reg, buf, ADT7476A_REG_VALUE_LEN);
    val = (uint16)buf[0];
    *temp = _ADT7476A_decode_temp_val(val);
    return ret;
}

static int32
_sensor_ADT7476A_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
  
    return RESULT_OK;    
}

static int32
_sensor_ADT7476A_init(sensor_handle_t* handle)
{
    uint8 value;
    
    value = 0x01;
    handle->reg_write(handle, 0x40, &value, 1);
    
    return RESULT_OK;
}

static int32 
_sensor_ADT7476A_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_ADT7476A_init;
    handle->get_temp = _sensor_ADT7476A_get_temp;
    handle->set_temp = _sensor_ADT7476A_set_temp;
    handle->reg_read = _sensor_ADT7476A_reg_read;
    handle->reg_write = _sensor_ADT7476A_reg_write;
    handle->reg_info[CURRENT_TEMP] = 0x25;
    return RESULT_OK;
}
/*****ADT7476A driver end*****************************************************/

/*****TMP42X driver start *****************************************************/
static int16 
_TMP42X_decode_temp_val(uint8 value)
{
    int16 tmpr;

    if ((value&0x80) == 0)  /* positive tmpr data */
    {
        tmpr = (uint16)value;
    }
    else                   /* negtive tmpr data */
    {
        tmpr = (~value)+1;
        tmpr = 0-tmpr;
    }
    return tmpr;
}

static int32
_sensor_TMP42X_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_sensor_TMP42X_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_sensor_TMP42X_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{    
    int32 ret;
    uint8 reg;
    uint8 buf[TMP42X_VALUE_LEN];
    
    reg = handle->reg_info[type];
    reg += handle->chan;
    ret = _sensor_TMP42X_reg_read(handle, reg, buf, TMP42X_VALUE_LEN);
    
    *temp = _TMP42X_decode_temp_val(buf[0]);
    return ret;
}

static int32
_sensor_TMP42X_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
    return RESULT_OK; 
}

static int32
_sensor_TMP42X_init(sensor_handle_t* handle)
{
    return RESULT_OK;
}
static int32 
_sensor_TMP42X_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_TMP42X_init;
    handle->get_temp = _sensor_TMP42X_get_temp;
    handle->set_temp = _sensor_TMP42X_set_temp;
    handle->reg_read = _sensor_TMP42X_reg_read;
    handle->reg_write = _sensor_TMP42X_reg_write;
    handle->reg_info[CURRENT_TEMP] = 0;
    return RESULT_OK;
}

/*****TMP42X driver end *****************************************************/

/*****SD5075 driver start*****************************************************/

/****************************************************************************
 * Name	: _SD5075_decode_temp_val
 * Purpose: decode the temperature of the sensor from raw register value to real temperature value 
 * Input	:  the raw register value
 * Output:   
 * Return:  the real temperature value
 * Note	: 
****************************************************************************/	
static int16 
_SD5075_decode_temp_val(uint16 value)
{
    int16 tmpr;

    if ((value&0x8000) == 0)  /* positive tmpr data */
    {
        tmpr = value>>8;
    }
    else                   /* negtive tmpr data */
    {
        tmpr = (value>>8)  - 256;
    }
    return tmpr;
}

static uint16 
_SD5075_encode_temp_val(int16 value)
{
    uint16 tmpr;

    if (value > 0)  /* positive tmpr data */
    {
        tmpr = value<<8;
    }
    else                   /* negtive tmpr data */
    {
        tmpr = (value + 256) << 8;
    }
    return tmpr;
}
static int32
_sensor_SD5075_reg_read(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->read(handle->io_hdl, &access);
}

static int32
_sensor_SD5075_reg_write(sensor_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    sensor_access_t access;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    return handle->io_hdl->write(handle->io_hdl, &access);    
}

static int32
_sensor_SD5075_get_temp(sensor_handle_t* handle, tmpr_type_t type, int16* temp )
{    
    int32 ret;
    uint8 reg;
    uint8 buf[SD5075_TEMP_VALUE_LEN];
    uint16 val;

    reg = handle->reg_info[type];
    ret = _sensor_SD5075_reg_read(handle, reg, buf, SD5075_TEMP_VALUE_LEN);
    val = buf[0];
    val = (val<<8)  | buf[1];
    *temp = _SD5075_decode_temp_val(val);
    return ret;
}

static int32
_sensor_SD5075_set_temp(sensor_handle_t* handle, tmpr_type_t type, int16 temp )
{
     int32 ret;
    uint8 reg;
    uint8 buf[SD5075_TEMP_VALUE_LEN];
    uint16 val;
    
    reg = handle->reg_info[type];
    val = _SD5075_encode_temp_val(temp);
    buf[0] = (val>>8)&0xff;
    buf[1] = val & 0xff;
    
    ret = _sensor_SD5075_reg_write(handle, reg, buf, SD5075_TEMP_VALUE_LEN);    
    return ret;    
}

static int32
_sensor_SD5075_init(sensor_handle_t* handle)
{
    _sensor_SD5075_set_temp(handle, CRIT_TEMP,  MAX_CRIT_TMPR);
    
    return RESULT_OK;
}

static int32 
_sensor_SD5075_register_driver(sensor_handle_t* handle)
{
    handle->sensor_init = _sensor_SD5075_init;
    handle->get_temp = _sensor_SD5075_get_temp;
    handle->set_temp = _sensor_SD5075_set_temp;
    handle->reg_read = _sensor_SD5075_reg_read;
    handle->reg_write = _sensor_SD5075_reg_write;
    handle->reg_info[CURRENT_TEMP] = 0x0;
    handle->reg_info[HIGH_TEMP] = 0x2;
    handle->reg_info[CRIT_TEMP] = 0x3;
    return RESULT_OK;
}
/*****SD5075 driver end*****************************************************/

/***** sensor API ********************************************************/
/* init sensor chip */
int32
sensor_dev_init(uint32 sensor_id)
{
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_sensor_hdl[sensor_id].sensor_init);
    return g_sensor_hdl[sensor_id].sensor_init(&g_sensor_hdl[sensor_id]);
}

/* get temp from sensor. return value: RESULT_OK, RESULT_ERROR  */
int32
sensor_dev_get_temp(uint32 sensor_id, tmpr_type_t type, int16* temp)
{
    int32 ret;
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_sensor_hdl[sensor_id].get_temp);
    ret = g_sensor_hdl[sensor_id].get_temp(&g_sensor_hdl[sensor_id], type, temp);
#if 0 /* for bug 47891, when read i2c fail, do not update sensor info */
    if(ret != RESULT_OK)
    {
        g_sensor_hdl[sensor_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_sensor_hdl[sensor_id].access_err_cnt > SENSOR_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *temp = 0;
            g_sensor_hdl[sensor_id].access_err_cnt=0;
        }
    }
    else
    {
        g_sensor_hdl[sensor_id].access_err_cnt=0;
    }
#endif    
    return ret;
}

/* set temp from sensor, return value: RESULT_OK, RESULT_ERROR  */
int32
sensor_dev_set_temp(uint32 sensor_id, tmpr_type_t type, int16 temp)
{
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_sensor_hdl[sensor_id].set_temp);
    return g_sensor_hdl[sensor_id].set_temp(&g_sensor_hdl[sensor_id], type, temp);
}

int32
sensor_dev_get_position(uint32 sensor_id, uint8* position)
{
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    *position = g_sensor_hdl[sensor_id].position;
    return RESULT_OK;
}

int32
sensor_dev_get_level(uint32 sensor_id, tmpr_level_t *level)
{
    if(g_sensor_num == 0)
    {
        return RESULT_OK;
    }
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    level->low = g_sensor_hdl[sensor_id].level.low;
    level->high = g_sensor_hdl[sensor_id].level.high;
    level->full = g_sensor_hdl[sensor_id].level.full;
    
    return RESULT_OK;
}

int32
sensor_reg_read(uint32 sensor_id, uint8 reg, uint8* value, uint8 len)
{
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_sensor_hdl[sensor_id].reg_read);
    return g_sensor_hdl[sensor_id].reg_read(&g_sensor_hdl[sensor_id], reg, value, len);
}

int32
sensor_reg_write(uint32 sensor_id, uint8 reg, uint8* value, uint8 len)
{
    if(sensor_id >= g_sensor_num)
    {
        DRV_LOG_ERR("Sensor id %d is error!, max is %d\n", sensor_id, g_sensor_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_sensor_hdl[sensor_id].reg_read);
    return g_sensor_hdl[sensor_id].reg_write(&g_sensor_hdl[sensor_id], reg, value, len);
}


/*********************************************************************
 * Name    : sensor_init
 * Purpose :  init some data structure and register sensor dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of sensor'i2c bus implement
          uint32 num         - the number of sensor
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
sensor_init(void** p_data, sensor_chip_t* type, uint32 num)
{
    int32 i = 0;

    DRV_CTC_CHK_PTR(p_data);  
    DRV_CTC_CHK_PTR(type);
    
    g_sensor_hdl = (sensor_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_SENSOR_INFO, sizeof(sensor_handle_t)*num);
    if(NULL == g_sensor_hdl)
    {
        DRV_LOG_ERR("Sensor alloc handler fail!\n");
        return RESULT_ERROR;
    }
    
    /* register temperature sensor dev */
    for (i = 0; i < num; i++)
    {
        g_sensor_hdl[i].io_hdl = sensor_io_register(type[i].io_type, p_data[i]);
        g_sensor_hdl[i].access_err_cnt = 0;
        g_sensor_hdl[i].position = type[i].pos;
        g_sensor_hdl[i].chip_type = type[i].chip_type;
        g_sensor_hdl[i].chan = type[i].chan;
        switch(type[i].chip_type)
        {
            case SENSOR_LM77:
                _sensor_LM77_register_driver(&g_sensor_hdl[i]);
                break;
            case SENSOR_MAX1617A:
                _sensor_MAX1617A_register_driver(&g_sensor_hdl[i]);
                break;
            case SENSOR_LM75:
                _sensor_LM75_register_driver(&g_sensor_hdl[i]);
                break;
            case SENSOR_TMP42X:
                _sensor_TMP42X_register_driver(&g_sensor_hdl[i]);
                break;
            case SENSOR_SD5075:
                _sensor_SD5075_register_driver(&g_sensor_hdl[i]);
                break;
            case SENSOR_ADT7476A:
                _sensor_ADT7476A_register_driver(&g_sensor_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport sensor chip type %d!\n", type[i].chip_type);
                return RESULT_ERROR;
        }
    }

    g_sensor_num = num;
    return RESULT_OK;
}

int32 
_sensor_diagnostic_test(uint8 sensor_num, sensor_diag_result_t* diag_result)
{
    int32 idx;
    int32 ret1 = RESULT_OK, ret2 = RESULT_OK;
    int16 temp;
    
    DRV_CTC_CHK_PTR(diag_result);
    
    for(idx = 0; idx < sensor_num; idx++)
    {
        ret1 = sensor_dev_get_temp(idx, CURRENT_TEMP, &temp);
        if(ret1)
        {
            diag_result[idx].access_fail = 1;
            ret2 = RESULT_ERROR;
            continue;
        }

        if(temp > DEFAULT_CRIT_TMPR - 5)
        {
            diag_result[idx].temp_alarm = 1;
            ret2 = RESULT_ERROR;
        }
    }

    return ret2;
}

int32 
sensor_diagnostic_test_and_report(uint8 sensor_num)
{
#ifndef _GLB_UML_SYSTEM_  
    uint8 i;
    int32 result=0;    
    sensor_diag_result_t diag_result[DIAG_MAX_SENSOR_NUM];
    
    sal_memset(diag_result, 0, sizeof(sensor_diag_result_t)*DIAG_MAX_SENSOR_NUM);
    result = _sensor_diagnostic_test(sensor_num, diag_result);

    if (result != 0)
    {
        for(i = 0; i < sensor_num; i++)
        {
            if(diag_result[i].access_fail)
            {
                DRV_DIAG_LOG("Sensor %d access fail!\n", i);
            }
            else if(diag_result[i].temp_alarm)
            {
                DRV_DIAG_LOG("Sensor %d temp alarm!\n", i);
            }
        }
        return RESULT_ERROR;
    }
    else
    {
        for (i = 0; i < sensor_num; i++)
        {
                DRV_DIAG_LOG("Sensor %d diag pass!\n", i);
        }
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}

