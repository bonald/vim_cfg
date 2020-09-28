/****************************************************************************
 * fan_api.c    fan api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-10-11.
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
#include "fan_drv.h"
#include "fan_api.h"
#include "epld_api.h"
#include "drv_log.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define FAN_ACCESS_ERROR_COUNT_MAX 2
#define FAN_STATUS_FAIL_COUNT_MAX 4
#define ADT7470_PWM_MAX     4
#define ADT7470_DELTA_RPM   2000
#define ADT7470_PRESENT_RPM 200

#define ADT7476A_PWM_MAX    4
#define ADT7476A_DELTA_RPM  3000/* for bug 45718*/

#define CTC5236_PWM_MAX     4
#define CTC5236_DELTA_RPM   2000
#define CTC5236_PRESENT_RPM 200

typedef enum
{
    ADT7470_TACH1_LOW=0x2a,
    ADT7470_TACH1_HIGH,
    ADT7470_TACH2_LOW,
    ADT7470_TACH2_HIGH,
    ADT7470_TACH3_LOW,
    ADT7470_TACH3_HIGH,
    ADT7470_TACH4_LOW,
    ADT7470_TACH4_HIGH,
}ADT7470_tach_register;

typedef enum
{
    ADT7470_PWM1=0x32,
    ADT7470_PWM2,
    ADT7470_PWM3,
    ADT7470_PWM4,        
}ADT7470_pwm_register;

typedef enum
{
    ADT7476A_TACH1_LOW=0x28,
    ADT7476A_TACH1_HIGH,
    ADT7476A_TACH2_LOW,
    ADT7476A_TACH2_HIGH,
    ADT7476A_TACH3_LOW,
    ADT7476A_TACH3_HIGH,
    ADT7476A_TACH4_LOW,
    ADT7476A_TACH4_HIGH,
}ADT7476A_tach_register;

typedef enum
{
    ADT7476A_PWM1=0x30,
    ADT7476A_PWM2,
    ADT7476A_PWM3,
}ADT7476A_pwm_register;

static fan_handle_t* g_fan_hdl;
static uint8 g_fan_chip_num;

/*the array seq is glb_fan_speed_level. */
 PWM_RPM_MAP g_fan_15000_PWM_map_table[]=  
     {{0x00, 0xff, 100000, FAN_SPEED_00, 1500},   
      {0x15, 0xff, 100000, FAN_SPEED_10, 1500},   
      {0x30, 0xff, 200000, FAN_SPEED_20, 3000},
      {0x45, 0xff, 300000, FAN_SPEED_30, 4500}, 
      {0x61, 0xf7, 400000, FAN_SPEED_40, 6000}, /* merge from old */
      {0x70, 0xff, 500000, FAN_SPEED_50, 7500}, 
      {0x95, 0xfa, 600000, FAN_SPEED_60, 9000}, /* merge from old */
      {0xa0, 0xff, 700000, FAN_SPEED_70, 10500}, 
      {0xb5, 0xfc, 800000, FAN_SPEED_80, 12000}, /* merge from old */
      {0xd0, 0xff, 900000, FAN_SPEED_90, 13500}, 
      {0xff, 0xff, 1000000, FAN_SPEED_100,15000}, /* merge from old */
      };

/* Modified by liuht for bug43997, 2017-0601 */
/* Modified by liuht for bug45718, 2017-1123 */
 PWM_RPM_MAP g_fan_13000_PWM_map_table[]=  
     {{0x24, 0xff, 100000, FAN_SPEED_00, 1300},   
      {0x24, 0xff, 100000, FAN_SPEED_10, 1300},   
      {0x24, 0xff, 200000, FAN_SPEED_20, 2600},
      {0x24, 0xff, 300000, FAN_SPEED_30, 3900}, 
      {0x24, 0xf7, 400000, FAN_SPEED_40, 5200}, /* merge from old */
      {0x32, 0xff, 500000, FAN_SPEED_50, 6500}, 
      {0x32, 0xfa, 600000, FAN_SPEED_60, 7800}, /* merge from old */
      {0x52, 0xff, 700000, FAN_SPEED_70, 9100}, 
      {0x52, 0xfc, 800000, FAN_SPEED_80, 10400}, /* merge from old */
      {0xff, 0xff, 900000, FAN_SPEED_90, 11700}, 
      {0xff, 0xff, 1000000, FAN_SPEED_100,13000}, 
      };

 PWM_RPM_MAP g_fan_9000_PWM_map_table[]=  
     {{0x00, 0xff, 0, FAN_SPEED_00, 900},   
      {0x30, 0xff, 22000, FAN_SPEED_10, 900},   
      {0x50, 0xff, 26000, FAN_SPEED_20, 1800},
      {0x60, 0xff, 29000, FAN_SPEED_30, 2700}, 
      {0x70, 0xf7, 33000, FAN_SPEED_40, 3600}, /* merge from old */
      {0x80, 0xff, 39000, FAN_SPEED_50, 4500}, 
      {0xa0, 0xfa, 46000, FAN_SPEED_60, 5400}, /* merge from old */
      {0xb0, 0xff, 55000, FAN_SPEED_70, 6300}, 
      {0xc0, 0xfc, 70000, FAN_SPEED_80, 7200}, /* merge from old */
      {0xe0, 0xff, 95000, FAN_SPEED_90, 8100}, 
      {0xff, 0xff, 1000000, FAN_SPEED_100,9000}, 
      };

PWM_RPM_MAP g_fan_PWM_map_table[FAN_SPEED_NUM];/*rate range from 0% to 100%,times of 10*/

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/

PWM_RPM_MAP*
_fan_get_PWM_table(fan_handle_t * handle, uint8 cfg_speed_level)
{
        uint8 temp_rate = 0;
        uint8 i;
        switch(cfg_speed_level)
        {
            case FAN_SPEED_BOTTOM:
                temp_rate = handle->p_data->pwm.bootom_rate;
                break;
            case FAN_SPEED_LOW:
                temp_rate = handle->p_data->pwm.low_rate;
                break;
            case FAN_SPEED_HIGH:
                temp_rate = handle->p_data->pwm.high_rate;
                break;
            case FAN_SPEED_FULL:
                temp_rate = handle->p_data->pwm.full_rate;
                break;
        }
        for(i = 0; i < sizeof(g_fan_PWM_map_table)/sizeof(PWM_RPM_MAP); i++)
        {
            if(g_fan_PWM_map_table[i].rate == temp_rate)
            {
                break;
            }
        }
        return &g_fan_PWM_map_table[i];
}

/* common api for fan control */
uint32 _fan_get_fan_pwm_adt7470(PWM_RPM_MAP *pwm_map_table,uint8 speed_level)
{
    int i;
    uint8 pwm=0xff;
    for(i = 0; i < FAN_SPEED_NUM; i++)
    {
            if(pwm_map_table[i].rate == speed_level)
            {
                pwm = pwm_map_table[i].ADT7470_pwm;
                break;
            }
    }
    return pwm;
}

uint32 _fan_get_fan_pwm_epld(PWM_RPM_MAP *pwm_map_table,uint8 speed_level)
{
    int i;
    uint8 pwm=0xff;
    for(i = 0; i < FAN_SPEED_NUM; i++)
    {
            if(pwm_map_table[i].rate == speed_level)
            {
                pwm = pwm_map_table[i].EPLD_pwm;
                break;
            }
    }
    return pwm;
}

uint32 _fan_get_fan_pwm_ctc5236(PWM_RPM_MAP *pwm_map_table,uint8 speed_level)
{
    int i;
    uint32 pwm = 1000000;
    for(i = 0; i < FAN_SPEED_NUM; i++)
    {
            if(pwm_map_table[i].rate == speed_level)
            {
                pwm = pwm_map_table[i].ctc5236_pwm;
                break;
            }
    }
    return pwm;
}

/*************get fan info by EPLD start*****************************************/

/* Added by liuht for bug26671, 2014-02-14 */
static int32
_fan_EPLD_reg_read(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    int32 ret = RESULT_OK;
    uint8 epld_idx;
    uint8 module_idx;
    uint8 reg_val;

    epld_idx = *((uint8 *)handle->io_hdl->p_data);
    module_idx = (handle-g_fan_hdl)/sizeof(fan_handle_t);
    epld_get_fan_speed(module_idx, &reg_val, epld_idx, reg);
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_FAN,"fan reg read fail.");
        return RESULT_ERROR;
    }	
    *value = reg_val;
    return RESULT_OK;
}

/* Added by liuht for bug26671, 2014-02-14 */
static int32
_fan_EPLD_reg_write(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    int32 ret = RESULT_OK;
    uint8 epld_idx;
    uint8 module_idx;
    uint8 fan_speed;

    fan_speed = *value;
    epld_idx = *((uint8 *)handle->io_hdl->p_data);
    module_idx = (handle-g_fan_hdl)/sizeof(fan_handle_t);
    epld_set_fan_speed(module_idx, fan_speed, epld_idx, reg);
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_FAN,"fan reg write fail");
        return RESULT_ERROR;
    }	
    return RESULT_OK;
}

/* Added by liuht for bug26671, 2014-02-14 */
static int32
_fan_EPLD_get_real_speed(fan_handle_t * handle, uint16* fan_speed)
{
    int32 ret = RESULT_OK;
    uint8 i,j;
    uint8 fan_status = 0;
    uint16 tmp_speed[5];
    uint8 tmp_speed_high, tmp_speed_low;
    uint8 fan_speed_reg[] = {0x27, 0x28, 0x29, 0x30, 0x31, 0x32};
    uint8 speed_equ_count = 0;
    uint32 count_speed = 0;
    static uint8 fan_fail_count[3] = {0, 0, 0};	                     

    ret = handle->reg_read(handle, 0x33, &fan_status, 1);	
    if(ret < 0)
        return RESULT_ERROR;

    /* There are 3 fan, every fan access 5 times */
    for(j = 0; j < 3; j++)
    {
        for(i = 0; i < 5; i++)
        {
            ret = handle->reg_read(handle, fan_speed_reg[2*j], &tmp_speed_low, 1);
            ret |= handle->reg_read(handle, fan_speed_reg[2*j+1], &tmp_speed_high, 1);
            if(ret < 0)
                return RESULT_ERROR;

            tmp_speed[i] = (tmp_speed_high << 8) + tmp_speed_low;
            count_speed += tmp_speed[i];	 
            if((i > 0) && (tmp_speed[i] == tmp_speed[i-1]))
            {
                speed_equ_count++;
            }
            usleep(2000);	 
        }
		
        /* When read fan speed for 5 times, and the returned value is equal from each other, then the fan is stall */
        if((4 == speed_equ_count) && ((fan_status & (1<<j))))
        {
             fan_fail_count[j]++;
             /* When fan fail for over 3 times, believe the fan is fail */
             if(fan_fail_count[j] >= FAN_STATUS_FAIL_COUNT_MAX)
             {
                 fan_speed[j] = 0xffff; 
             }
             else
             {
                 fan_speed[j] = (uint16)(count_speed/5);
             }
        }
        else
        {
             fan_speed[j] = (uint16)(count_speed/5); 
             fan_fail_count[j] = 0;
        }
        speed_equ_count = 0;
        count_speed = 0;
    }

    return RESULT_OK;
}

/* Added by liuht for bug26671, 2014-02-14 */
static uint8 
_fan_EPLD_speed_level_to_chip_value(fan_handle_t * handle, uint8 speed_level)
{
    uint8 seq;
    uint8 cycle_new[4]={0xf0, 0x60, 0x50, 0x00};
    
    if(speed_level == FAN_SPEED_BOTTOM)
        seq = 0;
    else if(speed_level == FAN_SPEED_LOW)
        seq = 1;
    else if(speed_level == FAN_SPEED_HIGH)
        seq = 2;
    else
        seq = 3;
    return cycle_new[seq];
    
}

/* Added by liuht for bug26671, 2014-02-14 */
static int32
_fan_EPLD_config_speed_level(fan_handle_t * handle, uint8 speed_level)
{
    int32 ret = RESULT_OK;
    uint8 chip_value;
	
    chip_value = _fan_EPLD_speed_level_to_chip_value(handle, speed_level);

    handle->reg_write(handle, 0x26, &chip_value, 1);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

static int32
_fan_EPLD_get_present(uint8 fan_module_idx, uint8* present)
{
    int32 ret;
    /*ADT7470 and EPLD also use this to get present. ADT7470 not store epld idx, so just use 0*/
    ret = epld_get_fan_present(fan_module_idx, present, 0);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

static int32
_fan_EPLD_init(fan_handle_t * handle)
{
    /*int32 ret;
    uint8 epld_idx;
    uint8 module_idx;

    epld_idx = *((uint8 *)handle->io_hdl->p_data);
    module_idx = (handle-g_fan_hdl)/sizeof(fan_handle_t);
    ret = epld_set_fan_power(module_idx, 1, epld_idx);
    return ret;    */
    return RESULT_OK;
}
/* Added by liuht for bug26671, 2014-02-14 */
/*status: 1 mean ok, 0 mean fail. each bit show 1 fan status*/
static int32
_fan_EPLD_get_status(fan_handle_t * handle, uint8* fan_status)
{
    int32 ret;
    uint8 i;
    uint16 local_speed[3];
    uint8 fan_access_fail=0;

    /*temply for fan not support speed adjust, just show ok*/
    if(handle->p_data->speed_adjust == 0)
    {
        *fan_status = 0xff;
        return RESULT_OK;
    }
    *fan_status = 0;

    ret = _fan_EPLD_get_real_speed(handle, local_speed);
    if(ret != RESULT_OK)
    {
     fan_access_fail = 1;
    }
    /*bug 18147. Add error process for access fail.*/
    if(fan_access_fail != 0)
    {
        handle->p_data->access_err_cnt++;
        if(handle->p_data->access_err_cnt > FAN_ACCESS_ERROR_COUNT_MAX)
        {
            DRV_LOG_ERR("Fan access fail.");
            handle->p_data->access_err_cnt=0;
            /*Set speed to 0xffff to notify users the fan access problem.*/
            for(i=0; i<3; i++)
            {
                local_speed[i] = 0xffff;
            }
        }
    }
    else
    {
        handle->p_data->access_err_cnt = 0;
    }

    /*For new version, can judge real speed value*/
    for(i=0; i<3; i++)
    {
        if(local_speed[i] < 0xffff)
        {
            *fan_status |= 1<<i;
        }
    }

    return RESULT_OK;
}

static int32
_fan_EPLD_register_driver(fan_handle_t* handle)
{
    handle->fan_init = _fan_EPLD_init;
    handle->get_real_speed = _fan_EPLD_get_real_speed;
    handle->config_speed_level = _fan_EPLD_config_speed_level;
    handle->get_status = _fan_EPLD_get_status;
    handle->reg_read = _fan_EPLD_reg_read;
    handle->reg_write = _fan_EPLD_reg_write;
    handle->get_fan_pwm = _fan_get_fan_pwm_epld;
    handle->user_config_speed_rate = NULL;
    handle->get_present = _fan_EPLD_get_present;
    return RESULT_OK;
}
/*************get fan info by EPLD end*****************************************/


/*************get fan info by ADT7470 start*****************************************/
static int32 _fan_ADT7470_config_speed_level(fan_handle_t * handle, uint8 speed_level);

static int32
_fan_ADT7470_reg_read(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    fan_access_t access;
    int ret;

    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
    if(ret < 0)
    {
        /*Fix bug 15142. jqiu 2011-08-02. sometime the fan module is absent when access.*/
        DRV_LOG_DEBUG(DRV_FAN,"fan reg read fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_fan_ADT7470_reg_write(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    fan_access_t access;
    int ret;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->write(handle->io_hdl, &access);      
    if(ret < 0)
    {
        /*Fix bug 15142. jqiu 2011-08-02. sometime the fan module is absent when access.*/
        DRV_LOG_DEBUG(DRV_FAN,"fan reg write fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_fan_ADT7470_init(fan_handle_t * handle)
{
    int32 ret;
    handle->p_data->cfg_speed_level = FAN_SPEED_FULL;
    /*Fix bug 18983. Because watchdog can't reset fan chip, so when init, maybe fan not run on full speed.
      Need to cfg to default full speed first.*/
    ret = _fan_ADT7470_config_speed_level(handle, FAN_SPEED_FULL);
    return ret;
}
/*speed calucate: real speed(RPM) = 90000*60/(tach_high*256+tach_low)*/
static int32
_fan_ADT7470_get_real_speed(fan_handle_t * handle, uint16* speed)
{
    int32 ret;
    uint8 i, tach_reg[ADT7470_PWM_MAX*2]; //ADT7470 chip can manage 4 fan speed
    uint16 tach;
    
    for(i=0; i<ADT7470_PWM_MAX*2; i++)
    {
        ret = _fan_ADT7470_reg_read(handle, ADT7470_TACH1_LOW+i, &tach_reg[i], 1);
        if(ret != RESULT_OK)
        {
            return RESULT_ERROR;
        }
    }
    for(i=0; i<ADT7470_PWM_MAX; i++)
    {
        tach = (tach_reg[i*2+1]<<8)+tach_reg[i*2];
        /*The tach value shouldn't be 0, there must be something error.*/
        if(tach == 0)
        {
            return RESULT_ERROR;
        }
        else
        {
            speed[i] = (uint16)(90000*60/tach);
        }
    }
    return RESULT_OK;
}

static uint8 _fan_ADT7470_speed_level_to_PWM(fan_handle_t * handle, uint8 speed_level)
{
    if(speed_level == FAN_SPEED_BOTTOM)
        return handle->p_data->pwm.bottom;
    else if(speed_level == FAN_SPEED_LOW)
        return handle->p_data->pwm.low;
    else if(speed_level == FAN_SPEED_HIGH)
        return handle->p_data->pwm.high;
    else
        return handle->p_data->pwm.full;
}

static int32
_fan_ADT7470_config_speed_level(fan_handle_t * handle, uint8 speed_level)
{
    uint8 i, pwm;
    int32 ret=RESULT_OK;
        
    pwm = _fan_ADT7470_speed_level_to_PWM(handle, speed_level);
    for(i=0; i<ADT7470_PWM_MAX; i++)
    {
        ret = _fan_ADT7470_reg_write(handle, ADT7470_PWM1+i, &pwm, 1);
    }
    return ret;
}

/*status: 1 mean ok, 0 mean fail. each bit show 1 fan status*/
/*Here get fan status depend on judge fan speed;*/
static int32
_fan_ADT7470_get_status(fan_handle_t * handle, uint8* fan_status)
{
    int32 ret;
    uint8 i;
    uint16 speed[ADT7470_PWM_MAX];
    uint8 cfg_speed_level;
    PWM_RPM_MAP* ptr;

    /*1. if fan doesn't support speed adjust, need give other way to check fan status. currently just report ok*/
    if(handle->p_data->speed_adjust == 0)
    {
        *fan_status = 0xf;
        return RESULT_OK;
    }
    /*set init value*/
    *fan_status = 0;
    /*2. read fan speed, and check fan work status*/     
    ret = _fan_ADT7470_get_real_speed(handle, speed);    
    if(ret == RESULT_ERROR)
    {
        /*access error count not hit max, still believe fan is ok*/
        if(handle->p_data->access_err_cnt < FAN_ACCESS_ERROR_COUNT_MAX)
        {
            handle->p_data->access_err_cnt++;
            *fan_status = 0xf;
            return RESULT_OK;
        }
        /*access error count hit max, fan is error.*/
        else if(handle->p_data->access_err_cnt == FAN_ACCESS_ERROR_COUNT_MAX)
        {
            DRV_LOG_ERR("Fan access fail.");
            handle->p_data->access_err_cnt++;            
        }        
        *fan_status = 0;
        return RESULT_OK;
    }
    else
    {
        /*When access ok, clear access error count*/
        handle->p_data->access_err_cnt = 0;
        cfg_speed_level = handle->p_data->cfg_speed_level;
        if(cfg_speed_level >= FAN_SPEED_LEVEL_MAX)
        {
            DRV_LOG_ERR("Fan config speed level %d is out of range.", cfg_speed_level);
            cfg_speed_level = FAN_SPEED_FULL;            
        }
        ptr = _fan_get_PWM_table(handle, cfg_speed_level);

        /*then check fan speed.*/
        for(i=0; i<ADT7470_PWM_MAX; i++)
        {
            /* if the fan speed is less than 200rpm, the fan is not in position */
            if((speed[i] > ADT7470_PRESENT_RPM) && (speed[i] + ADT7470_DELTA_RPM >= ptr->rpm))
            {
                *fan_status |= 1<<i;
            }
        }
    }
    return RESULT_OK;
}

int32
_fan_ADT7470_user_config_speed_rate(fan_handle_t * handle, uint16 *cfg_speed_rate, uint8 cfg_speed_num)
{
    uint8 i,j;
    uint8 temp_pwm[4];
    uint8 temp_rate[4];    
    uint8 temp_level;
    int ret = RESULT_OK;

    if((cfg_speed_rate == NULL) || (cfg_speed_num != FAN_SPEED_LEVEL_MAX))
    {
        DRV_LOG_ERR("Fan speed config input is error!\n");
        return RESULT_ERROR;
    }
    for(j = FAN_SPEED_BOTTOM; j < cfg_speed_num; j++)
    {
        temp_pwm[j] = handle->get_fan_pwm(g_fan_PWM_map_table,cfg_speed_rate[j]);
        temp_rate[j] = cfg_speed_rate[j];
    }

    /*If the configuration of the fan speed is the same as the orginal,it will be returned directly*/
    if((temp_rate[0] == handle->p_data->pwm.bootom_rate) 
        && (temp_rate[1] == handle->p_data->pwm.low_rate)
        && (temp_rate[2] == handle->p_data->pwm.high_rate)
        && (temp_rate[3] == handle->p_data->pwm.full_rate))
    {
        return RESULT_OK;
    }
    else
    {
        handle->p_data->pwm.bottom = temp_pwm[0];
        handle->p_data->pwm.bootom_rate = temp_rate[0];
        handle->p_data->pwm.low = temp_pwm[1];
        handle->p_data->pwm.low_rate= temp_rate[1];
        handle->p_data->pwm.high = temp_pwm[2];
        handle->p_data->pwm.high_rate= temp_rate[2];
        handle->p_data->pwm.full = temp_pwm[3];
        handle->p_data->pwm.full_rate= temp_rate[3];
             
        temp_level = handle->p_data->cfg_speed_level;

        for(i=0; i < ADT7470_PWM_MAX; i++)
        {
            ret = handle->reg_write(handle, ADT7470_PWM1 + i, &temp_pwm[temp_level], 1);            
            if(ret != RESULT_OK)
            {
                DRV_LOG_ERR("Fan speed changed error!\n");
                return RESULT_ERROR;
            }
        }
        return RESULT_OK;
    }

    return RESULT_OK;
}


static int32
_fan_ADT7470_register_driver(fan_handle_t* handle)
{
    handle->fan_init = _fan_ADT7470_init;
    handle->get_real_speed = _fan_ADT7470_get_real_speed;
    handle->config_speed_level = _fan_ADT7470_config_speed_level;
    handle->get_status = _fan_ADT7470_get_status;
    handle->reg_read = _fan_ADT7470_reg_read;
    handle->reg_write = _fan_ADT7470_reg_write;
    handle->get_fan_pwm = _fan_get_fan_pwm_adt7470;
    handle->user_config_speed_rate = _fan_ADT7470_user_config_speed_rate;
    return RESULT_OK;
}
/*************get fan info by ADT7470 end*****************************************/

/*************get fan info by ctc5236 start*****************************************/
static int32 _fan_ctc5236_config_speed_level(fan_handle_t * handle, uint8 speed_level);

static int32
_fan_ctc5236_init(fan_handle_t * handle)
{
    int32 ret = 0;
    int32 i = 0;
    char node[256] = {0};

    sal_snprintf(node, 256, "/sys/class/pwm/pwmchip%d/export", handle->p_data->pwm_chip_idx);
    for (i = 0 ; i < CTC5236_PWM_MAX; i++)
    {
        FILE *fp = NULL;
        fp = sal_fopen(node, "w");
        if (fp == NULL)
        {
            return RESULT_ERROR;
        }
        sal_fprintf(fp, "%d", handle->p_data->pwm_channel_idx[i]);
        sal_fclose(fp);
    }

    for (i = 0 ; i < CTC5236_PWM_MAX; i++)
    {
        sal_snprintf(node, 256, "/sys/class/pwm/pwmchip%d/pwm%d/enable", handle->p_data->pwm_chip_idx, handle->p_data->pwm_channel_idx[i]);
        FILE *fp = NULL;
        fp = sal_fopen(node, "w");
        if (fp == NULL)
        {
            return RESULT_ERROR;
        }
        sal_fprintf(fp, "%d", 1);
        sal_fclose(fp);
    }

    handle->p_data->cfg_speed_level = FAN_SPEED_FULL;
    ret = _fan_ctc5236_config_speed_level(handle, FAN_SPEED_FULL);
    
    return ret;
}

static int32
_fan_ctc5236_get_real_speed(fan_handle_t * handle, uint16* speed)
{
    int32 i = 0;
    char node[256] = {0};
    int64 period = 0, duty = 0;

    for (i = 0; i < CTC5236_PWM_MAX; i++)
    {
        sal_snprintf(node, 256, "/sys/class/pwm/pwmchip%d/pwm%d/capture", handle->p_data->pwm_chip_idx, handle->p_data->pwm_channel_idx[i]);
        FILE *fp = NULL;
        fp = sal_fopen(node, "r");
        if (fp == NULL)
        {
            return RESULT_ERROR;
        }
        fscanf(fp, "%"PRId64" %"PRId64, &period, &duty);
        sal_fclose(fp);
        if (period == 0)
        {
            speed[i] = 0;
        }
        else
        {
            speed[i] = (uint16)(30000000000ll/period);
        }
    }

    return RESULT_OK;
}

static uint32 _fan_ctc5236_speed_level_to_PWM(fan_handle_t * handle, uint8 speed_level)
{
    if(speed_level == FAN_SPEED_BOTTOM)
        return handle->p_data->pwm.bottom;
    else if(speed_level == FAN_SPEED_LOW)
        return handle->p_data->pwm.low;
    else if(speed_level == FAN_SPEED_HIGH)
        return handle->p_data->pwm.high;
    else
        return handle->p_data->pwm.full;
}

static int32 _fan_ctc5236_config_speed_level(fan_handle_t * handle, uint8 speed_level)
{
    int32 i = 0;
    int32 pwm = 0;
    char node[256] = {0};

    pwm = _fan_ctc5236_speed_level_to_PWM(handle, speed_level);

    for (i = 0; i < CTC5236_PWM_MAX; i++)
    {
        sal_snprintf(node, 256, "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle", handle->p_data->pwm_chip_idx, handle->p_data->pwm_channel_idx[i]);
        FILE *fp = NULL;
        fp = sal_fopen(node, "w");
        if (fp == NULL)
        {
            return RESULT_ERROR;
        }
        sal_fprintf(fp, "%d", pwm);
        sal_fclose(fp);
    }

    return RESULT_OK;
}

static int32
_fan_ctc5236_get_status(fan_handle_t * handle, uint8* fan_status)
{
    int32 ret;
    uint8 i;
    uint16 speed[CTC5236_PWM_MAX];
    uint8 cfg_speed_level;
    PWM_RPM_MAP* ptr;

    /*1. if fan doesn't support speed adjust, need give other way to check fan status. currently just report ok*/
    if(handle->p_data->speed_adjust == 0)
    {
        *fan_status = 0xf;
        return RESULT_OK;
    }
    /*set init value*/
    *fan_status = 0;
    /*2. read fan speed, and check fan work status*/     
    ret = _fan_ctc5236_get_real_speed(handle, speed);    
    if(ret == RESULT_ERROR)
    {
        /*access error count not hit max, still believe fan is ok*/
        if(handle->p_data->access_err_cnt < FAN_ACCESS_ERROR_COUNT_MAX)
        {
            handle->p_data->access_err_cnt++;
            *fan_status = 0xf;
            return RESULT_OK;
        }
        /*access error count hit max, fan is error.*/
        else if(handle->p_data->access_err_cnt == FAN_ACCESS_ERROR_COUNT_MAX)
        {
            DRV_LOG_ERR("Fan access fail.");
            handle->p_data->access_err_cnt++;            
        }        
        *fan_status = 0;
        return RESULT_OK;
    }
    else
    {
        /*When access ok, clear access error count*/
        handle->p_data->access_err_cnt = 0;
        cfg_speed_level = handle->p_data->cfg_speed_level;
        if(cfg_speed_level >= FAN_SPEED_LEVEL_MAX)
        {
            DRV_LOG_ERR("Fan config speed level %d is out of range.", cfg_speed_level);
            cfg_speed_level = FAN_SPEED_FULL;            
        }
        ptr = _fan_get_PWM_table(handle, cfg_speed_level);

        /*then check fan speed.*/
        for(i=0; i<CTC5236_PWM_MAX; i++)
        {
            /* if the fan speed is less than 200rpm, the fan is not in position */
            if((speed[i] > CTC5236_PRESENT_RPM) && (speed[i] + CTC5236_DELTA_RPM >= ptr->rpm))
            {
                *fan_status |= 1<<i;
            }
        }
    }
    return RESULT_OK;
}


int32
_fan_ctc5236_user_config_speed_rate(fan_handle_t * handle, uint16 *cfg_speed_rate, uint8 cfg_speed_num)
{
    uint8 i,j;
    uint32 temp_pwm[4];
    uint8 temp_rate[4];    
    uint8 temp_level;
    char node[256] = {0};
    int ret = RESULT_OK;

    if((cfg_speed_rate == NULL) || (cfg_speed_num != FAN_SPEED_LEVEL_MAX))
    {
        DRV_LOG_ERR("Fan speed config input is error!\n");
        return RESULT_ERROR;
    }
    for(j = FAN_SPEED_BOTTOM; j < cfg_speed_num; j++)
    {
        temp_pwm[j] = handle->get_fan_pwm(g_fan_PWM_map_table,cfg_speed_rate[j]);
        temp_rate[j] = cfg_speed_rate[j];
    }

    /*If the configuration of the fan speed is the same as the orginal,it will be returned directly*/
    if((temp_rate[0] == handle->p_data->pwm.bootom_rate) 
        && (temp_rate[1] == handle->p_data->pwm.low_rate)
        && (temp_rate[2] == handle->p_data->pwm.high_rate)
        && (temp_rate[3] == handle->p_data->pwm.full_rate))
    {
        return RESULT_OK;
    }
    else
    {
        handle->p_data->pwm.bottom = temp_pwm[0];
        handle->p_data->pwm.bootom_rate = temp_rate[0];
        handle->p_data->pwm.low = temp_pwm[1];
        handle->p_data->pwm.low_rate= temp_rate[1];
        handle->p_data->pwm.high = temp_pwm[2];
        handle->p_data->pwm.high_rate= temp_rate[2];
        handle->p_data->pwm.full = temp_pwm[3];
        handle->p_data->pwm.full_rate= temp_rate[3];
             
        temp_level = handle->p_data->cfg_speed_level;

        for(i = 0; i < CTC5236_PWM_MAX; i++)
        {
            sal_snprintf(node, 256, "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle", handle->p_data->pwm_chip_idx, handle->p_data->pwm_channel_idx[i]);
            FILE *fp = NULL;
            fp = sal_fopen(node, "w");
            if (fp == NULL)
            {
                return RESULT_ERROR;
            }
            sal_fprintf(fp, "%d", temp_pwm[temp_level]);
            sal_fclose(fp);
        }
        return RESULT_OK;
    }

    return ret;
}


static int32
_fan_ctc5236_register_driver(fan_handle_t* handle)
{
    handle->fan_init = _fan_ctc5236_init;
    handle->get_real_speed = _fan_ctc5236_get_real_speed;
    handle->config_speed_level = _fan_ctc5236_config_speed_level;
    handle->get_status = _fan_ctc5236_get_status;
    handle->get_fan_pwm = _fan_get_fan_pwm_ctc5236;
    handle->user_config_speed_rate = _fan_ctc5236_user_config_speed_rate;
    return RESULT_OK;
}

/*************get fan info by ctc5236 end*******************************************/

/*************get fan info by ADT7476A start*****************************************/
static int32
_fan_ADT7476A_reg_read(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    fan_access_t access;
    int ret;

    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_FAN,"fan reg read fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_fan_ADT7476A_reg_write(fan_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    fan_access_t access;
    int ret;
    
    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->write(handle->io_hdl, &access);      
    if(ret < 0)
    {
        DRV_LOG_DEBUG(DRV_FAN,"fan reg write fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_fan_ADT7476A_init(fan_handle_t * handle)
{
    uint8 value;

    /* 1. fastest speed calculated by all three temperature channel controls */
    value = 0xc2;
    handle->reg_write(handle, 0x5c, &value, 1);
    handle->reg_write(handle, 0x5d, &value, 1);
    handle->reg_write(handle, 0x5e, &value, 1);

    /* 2. temperature range value 32 */
    value = 0xc4;
    handle->reg_write(handle, 0x5f, &value, 1);
    handle->reg_write(handle, 0x60, &value, 1);
    handle->reg_write(handle, 0x61, &value, 1);

    /* 3. PWM Maximum Duty Cycle 100% */
    value = 0xff;
    handle->reg_write(handle, 0x38, &value, 1);
    handle->reg_write(handle, 0x39, &value, 1);
    handle->reg_write(handle, 0x3a, &value, 1);

    /* 4. PWM Minimum Duty Cycle 50% */
    value = 0x80;
    handle->reg_write(handle, 0x64, &value, 1);
    handle->reg_write(handle, 0x65, &value, 1);
    handle->reg_write(handle, 0x66, &value, 1);
    value = 0xe0;
    handle->reg_write(handle, 0x62, &value, 1);
    
    /* 5. Temperature Minimum 50 */
    value = 0x32;
    handle->reg_write(handle, 0x67, &value, 1);
    handle->reg_write(handle, 0x68, &value, 1);
    handle->reg_write(handle, 0x69, &value, 1);

    /* 6. enable fan speed auto adjust */
    value = 0x01;
    handle->reg_write(handle, 0x40, &value, 1);

    return RESULT_OK;
}

static int32
_fan_ADT7476A_get_speed(fan_handle_t * handle, uint16* speed)
{
    int32 ret;
    uint8 local_speed;

    ret = _fan_ADT7476A_reg_read(handle, ADT7476A_PWM1, &local_speed, 1);
    if(ret != RESULT_OK)
    {
        return RESULT_ERROR;
    }
    *speed = (local_speed*100)/255;
    
    return RESULT_OK;
}

static int32
_fan_ADT7476_get_feedback_speed(fan_handle_t * handle, uint16* speed)
{
    int32 ret;
    uint8 i, tach_reg[ADT7476A_PWM_MAX*2]; //ADT7476A chip can manage 4 fan speed
    uint16 tach;
    
    for(i=0; i<ADT7476A_PWM_MAX*2; i++)
    {
        ret = _fan_ADT7476A_reg_read(handle, ADT7476A_TACH1_LOW+i, &tach_reg[i], 1);
        if(ret != RESULT_OK)
        {
            return RESULT_ERROR;
        }
    }
    for(i=0; i<ADT7476A_PWM_MAX; i++)
    {
        tach = (tach_reg[i*2+1]<<8)+tach_reg[i*2];
        /*The tach value shouldn't be 0, there must be something error.*/
        if(tach == 0)
        {
            return RESULT_ERROR;
        }
        else
        {
            speed[i] = (uint16)(90000*60/tach);
        }
    }
    return RESULT_OK;
}

/*status: 1 mean ok, 0 mean fail. each bit show 1 fan status*/
/*Here get fan status depend on judge fan speed;*/
static int32
_fan_ADT7476A_get_status(fan_handle_t * handle, uint8* fan_status)
{
    int32 ret;
    uint8 i;
    uint16 speed[ADT7476A_PWM_MAX];
    uint16 speed_low_limit=5000;

    /*1. if fan doesn't support speed adjust, need give other way to check fan status. currently just report ok*/
    if(handle->p_data->speed_adjust == 0)
    {
        *fan_status = 0xff;
        return RESULT_OK;
    }
    /*set init value*/
    *fan_status = 0;
    /*2. read fan speed, and check fan work status*/     
    ret = _fan_ADT7476_get_feedback_speed(handle, speed);    
    if(ret == RESULT_ERROR)
    {
        /*access error count not hit max, still believe fan is ok*/
        if(handle->p_data->access_err_cnt < FAN_ACCESS_ERROR_COUNT_MAX)
        {
            handle->p_data->access_err_cnt++;
            *fan_status = 0xf;
            return RESULT_OK;
        }
        /*access error count hit max, fan is error.*/
        else if(handle->p_data->access_err_cnt == FAN_ACCESS_ERROR_COUNT_MAX)
        {
            DRV_LOG_ERR("Fan access fail.");
            handle->p_data->access_err_cnt++;            
        }        
        *fan_status = 0;
        return RESULT_OK;
    }
    else
    {
        /*When access ok, clear access error count*/
        handle->p_data->access_err_cnt = 0;

        /*then check fan speed.*/
        for(i=0; i<ADT7476A_PWM_MAX; i++)
        {
            if(speed[i] + ADT7476A_DELTA_RPM >= speed_low_limit)
            {
                *fan_status |= 1<<i;
            }
        }
    }
    return RESULT_OK;
}

static int32
_fan_ADT7476A_register_driver(fan_handle_t* handle)
{
    handle->fan_init = _fan_ADT7476A_init;
    handle->get_real_speed = _fan_ADT7476A_get_speed;
    handle->config_speed_level = NULL;
    handle->get_status = _fan_ADT7476A_get_status;
    handle->reg_read = _fan_ADT7476A_reg_read;
    handle->reg_write = _fan_ADT7476A_reg_write;
    handle->get_fan_pwm = NULL;
    handle->user_config_speed_rate = NULL;
    return RESULT_OK;
}
/*************get fan info by ADT7476A end*****************************************/

int32
fan_user_config_speed_rate(uint8 fan_module_idx, uint16 *cfg_speed_rate, uint8 cfg_speed_num)
{
    int ret = RESULT_OK;

    if(fan_module_idx >= g_fan_chip_num)
    {        
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }

    if (g_fan_hdl[fan_module_idx].user_config_speed_rate != NULL)
    {
        return g_fan_hdl[fan_module_idx].user_config_speed_rate(&g_fan_hdl[fan_module_idx], cfg_speed_rate, cfg_speed_num);
    }

    return ret;
}

int32
fan_dev_init(uint8 fan_module_idx)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(NULL != g_fan_hdl[fan_module_idx].fan_init)
    {
        return g_fan_hdl[fan_module_idx].fan_init(&g_fan_hdl[fan_module_idx]);
    }
    return RESULT_OK;
}

int32
fan_dev_reinit(uint8 fan_module_idx)
{
    int32 ret=RESULT_OK;
    fan_handle_t * handle;
    
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    handle = &g_fan_hdl[fan_module_idx];
    if (handle->config_speed_level != NULL)
    {
        ret = handle->config_speed_level(handle, handle->p_data->cfg_speed_level);
    }
    return ret;
}

int32
fan_config_speed_level(uint8 fan_module_idx, uint8 cfg_speed_level)
{
    int32 ret=RESULT_OK;
    uint8 present;
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(cfg_speed_level == g_fan_hdl[fan_module_idx].p_data->cfg_speed_level)
        return RESULT_OK;
    if((NULL != g_fan_hdl[fan_module_idx].config_speed_level)&&(FAN_SPEED_SW == g_fan_hdl[fan_module_idx].p_data->speed_adjust))
    {
        g_fan_hdl[fan_module_idx].p_data->cfg_speed_level = cfg_speed_level;
        ret = fan_get_present(fan_module_idx, &present);
        if(present)
        {
            ret=(ret!=RESULT_OK)?ret:g_fan_hdl[fan_module_idx].config_speed_level(&g_fan_hdl[fan_module_idx], cfg_speed_level);
        }
    }
    return ret;
}

int32
fan_get_speed_level(uint8 fan_module_idx, uint8* cfg_speed_level)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    *cfg_speed_level = g_fan_hdl[fan_module_idx].p_data->cfg_speed_level;
    return RESULT_OK;
}

int32
fan_get_speed_adjust(uint8 fan_module_idx, uint8* speed_adjust)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    *speed_adjust = g_fan_hdl[fan_module_idx].p_data->speed_adjust;
    return RESULT_OK;
}

int32
fan_get_num(uint8 fan_module_idx, uint8* num)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    *num = g_fan_hdl[fan_module_idx].p_data->fan_num;
    return RESULT_OK;
}


int32
fan_get_speed_rate_by_level(uint8 fan_module_idx, uint8 cfg_speed_level, uint8* rate)
{
    uint8 bottom_rate, low_rate, high_rate, full_rate;
    
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    /* if customized bottom/low/high/full speed rate is valid, then use it */
    bottom_rate = g_fan_hdl[fan_module_idx].p_data->pwm.bootom_rate;
    low_rate    = g_fan_hdl[fan_module_idx].p_data->pwm.low_rate;
    high_rate   = g_fan_hdl[fan_module_idx].p_data->pwm.high_rate;
    full_rate   = g_fan_hdl[fan_module_idx].p_data->pwm.full_rate;
    
    if (bottom_rate || low_rate || high_rate || full_rate)
    {
        if(cfg_speed_level == FAN_SPEED_BOTTOM)
            *rate = bottom_rate;
        else if(cfg_speed_level == FAN_SPEED_LOW)
            *rate = low_rate;
        else if(cfg_speed_level == FAN_SPEED_HIGH)
            *rate = high_rate;
        else
            *rate = full_rate;
        return RESULT_OK;
    }

    else
    {
        *rate = 0;
        DRV_LOG_ERR("Unsupport RPM fan %d.", g_fan_hdl[fan_module_idx].p_data->rpm);
        return RESULT_ERROR;
    }  
    return RESULT_OK;
}

int32
fan_get_present(uint8 fan_module_idx, uint8* present)
{
    int32 ret = 0;
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(g_fan_hdl[fan_module_idx].p_data->support_hot_swap == 0)
    {
        *present = 1;
        return RESULT_OK;
    }
    else
    {
        if (g_fan_hdl[fan_module_idx].get_present != NULL)
        {
            ret = g_fan_hdl[fan_module_idx].get_present(fan_module_idx, present);
        }
        return ret;
    }
}

int32
fan_get_real_speed(uint8 fan_module_idx, uint16* speed)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(NULL != g_fan_hdl[fan_module_idx].get_real_speed)
    {
        return g_fan_hdl[fan_module_idx].get_real_speed(&g_fan_hdl[fan_module_idx], speed);
    }
    return RESULT_OK;
}

int32
fan_get_status(uint8 fan_module_idx, uint8* status, uint8* fan_num)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    *fan_num = g_fan_hdl[fan_module_idx].p_data->fan_num;
    if(NULL != g_fan_hdl[fan_module_idx].get_status)
    {
        return g_fan_hdl[fan_module_idx].get_status(&g_fan_hdl[fan_module_idx], status);
    }
    return RESULT_OK;
}

int32
fan_reg_read(uint8 fan_module_idx, uint8 reg, uint8* value)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(NULL != g_fan_hdl[fan_module_idx].reg_read)
    {
        return g_fan_hdl[fan_module_idx].reg_read(&g_fan_hdl[fan_module_idx], reg, value, 1);
    }
    return RESULT_OK;
}

int32
fan_reg_write(uint8 fan_module_idx, uint8 reg, uint8 value)
{
    if(fan_module_idx >= g_fan_chip_num)
    {
        DRV_LOG_ERR("Fan module id %d is error! max is %d\n", fan_module_idx, g_fan_chip_num);
        return RESULT_ERROR;
    }
    if(NULL != g_fan_hdl[fan_module_idx].reg_write)
    {
        return g_fan_hdl[fan_module_idx].reg_write(&g_fan_hdl[fan_module_idx], reg, &value, 1);
    }
    return RESULT_OK;
}

int32
fan_init(void** p_data, fan_chip_t* type, uint8 chip_num)
{
    int32 i=0,j=0;
    fan_private_data_t* private_data; 

    DRV_CTC_CHK_PTR(p_data);
    DRV_CTC_CHK_PTR(type);

    g_fan_hdl = (fan_handle_t *)DRV_CALLOC(CTCLIB_MEM_DRIVER_FAN_INFO, sizeof(fan_handle_t)*chip_num);
    if(NULL == g_fan_hdl)
    {
        DRV_LOG_ERR("Fan alloc handler fail!\n");
        return RESULT_ERROR;
    }

    for(i=0; i<chip_num; i++)
    {
        private_data = (fan_private_data_t *)DRV_CALLOC(CTCLIB_MEM_DRIVER_FAN_INFO, sizeof(fan_private_data_t));
        g_fan_hdl[i].p_data = private_data;
        private_data->fan_num = type[i].fan_num;
        private_data->cfg_speed_level = FAN_SPEED_FULL;
        private_data->speed_adjust = type[i].speed_adjust;
        private_data->rpm = type[i].rpm;
        private_data->support_hot_swap = type[i].support_hot_swap;
        private_data->pwm = type[i].pwm;
        private_data->pwm_chip_idx = i;

        /*add by chenjr for bug 53774*/
        if(CUSTOM_FAN_PWM_CHANNAL== type[i].pwm_channel_flag)
        {
            for(j = 0; j < CTC5236_PWM_MAX; j++)
            {
                /* for bug 54068,54069, modified by tongzb */
                private_data->pwm_channel_idx[j] = type[i].pwm_channel_idx[j];
            }
        }
        else
        {
            for(j = 0; j < CTC5236_PWM_MAX; j++)
            {
                /* for bug 54068,54069, modified by tongzb */
                private_data->pwm_channel_idx[j] = j;
            }
        }
  
        if((type[i].pwm_map_table != NULL) && (type[i].pwm_map_table_num != 0))
        {
            memcpy(g_fan_PWM_map_table, type[i].pwm_map_table, sizeof(PWM_RPM_MAP) * type[i].pwm_map_table_num);
        }
        else
        {
            if(private_data->rpm == 9000)
            {
                memcpy(g_fan_PWM_map_table, g_fan_9000_PWM_map_table, sizeof(g_fan_9000_PWM_map_table));    
            }
            else if(private_data->rpm == 13000)
            {
                memcpy(g_fan_PWM_map_table, g_fan_13000_PWM_map_table, sizeof(g_fan_13000_PWM_map_table));    
            }
            else if(private_data->rpm == 15000)
            {
                memcpy(g_fan_PWM_map_table, g_fan_15000_PWM_map_table, sizeof(g_fan_15000_PWM_map_table));    
            }
            else
            {
                memcpy(g_fan_PWM_map_table, g_fan_15000_PWM_map_table, sizeof(g_fan_15000_PWM_map_table));    
            }
        }
        switch(type[i].chip_type)
        {
            case FAN_EPLD:                
                _fan_EPLD_register_driver(&g_fan_hdl[i]);
                break;
            case FAN_ADT7470:
                _fan_ADT7470_register_driver(&g_fan_hdl[i]);
                break;
            case FAN_ADT7476A:
                _fan_ADT7476A_register_driver(&g_fan_hdl[i]);
                break;
            case FAN_CTC5236:
                _fan_ctc5236_register_driver(&g_fan_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport fan chip type %d!\n", type[i].chip_type);
                DRV_LOG_USER(E_ERROR, DRV_3_FAN_TYPE_UNKNOWN, type[i].chip_type);
                return RESULT_ERROR;
        }
        private_data->pwm.bottom=g_fan_hdl[i].get_fan_pwm(g_fan_PWM_map_table,private_data->pwm.bootom_rate); 
        private_data->pwm.low=g_fan_hdl[i].get_fan_pwm(g_fan_PWM_map_table,private_data->pwm.low_rate); 
        private_data->pwm.high=g_fan_hdl[i].get_fan_pwm(g_fan_PWM_map_table,private_data->pwm.high_rate); 
        private_data->pwm.full=g_fan_hdl[i].get_fan_pwm(g_fan_PWM_map_table,private_data->pwm.full_rate); 
        g_fan_hdl[i].io_hdl = fan_io_register(type[i].io_type, p_data[i]);
    }
    g_fan_chip_num = chip_num;
    return RESULT_OK;
}

int32
_fan_diagnostic_test(uint8 fan_module_num, fan_diag_result_t* diag_result)
{
    int32 ret1 = RESULT_OK, ret2 = RESULT_OK;
    uint8 i = 0,j = 0;
    uint8 status;
    uint8 fan_num;
    uint8 present = 0;

    DRV_CTC_CHK_PTR(diag_result);
    /*bug19615. Add delay for fan speed up.*/
    sleep(1);
    for (i = 0; i < fan_module_num; i++)
    {
        ret1 = fan_get_present(i, &present);
        if (ret1 < 0)
        {
            diag_result[i].access_fail = 1;
            ret2 = RESULT_ERROR;
            continue;
        }
        
        if (present != 1)
        {
            diag_result[i].absent = 1;
            ret2 = RESULT_ERROR;
            continue;
        }
        
        ret1 = fan_get_status(i, &status, &fan_num);                
        if (ret1 < 0)
        {
            diag_result[i].access_fail = 1;
            ret2 = RESULT_ERROR;
            continue;
        }                       
        for(j=0; j<fan_num; j++)
        {
            if(((status>>j)&0x1) != 1 )
            {
                diag_result[i].status_fail |= (1<<j);
                ret2 = RESULT_ERROR;
            }
        }
    }
    return ret2;
}

int32
fan_diagnostic_test_and_report(uint8 fan_module_num)
{
#ifndef _GLB_UML_SYSTEM_  
    uint8 i;
    int32 result = 0;
    fan_diag_result_t diag_result[DIAG_MAX_FAN_NUM];

    sal_memset(diag_result, 0, sizeof(fan_diag_result_t)*DIAG_MAX_FAN_NUM);
    result = _fan_diagnostic_test(fan_module_num, diag_result);
    if (result != RESULT_OK)
    {
        for(i = 0; i < fan_module_num; i++)
        {
            if(diag_result[i].access_fail)
            {
                DRV_DIAG_LOG("Fan module %d access fail!\n", i);
            }
            else if(diag_result[i].absent)
            {
                DRV_DIAG_LOG("Fan module %d absent!\n", i);
            }
            else if(diag_result[i].status_fail)
            {
                DRV_DIAG_LOG("Fan module %d status %d fail!\n", i, diag_result[i].status_fail);
            }
            DRV_DIAG_LOG("Fan module %d diag fail!\n", i);
        }
        return RESULT_ERROR;
    }
    else
    {
        for (i = 0; i < fan_module_num; i++)
        {
                DRV_DIAG_LOG("Fan module %d diag pass!\n", i);
        }
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}
