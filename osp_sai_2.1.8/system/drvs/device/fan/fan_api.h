/****************************************************************************
* $Id$
*  The header file of the fan api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-10-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __FAN_API_H__
#define __FAN_API_H__
#include "fan_drv.h"

#define ADT7470_OFFSET_WIDTH 1
#define DIAG_MAX_FAN_NUM    4
#define FAN_SPEED_NUM    11

/*add by chenjr for bug 53774*/
#define DEFAULT_FAN_PWM_CHANNAL      0
#define CUSTOM_FAN_PWM_CHANNAL      1

typedef enum
{
    FAN_EPLD,
    FAN_ADT7470,
    FAN_ADT7476A,
    FAN_CTC5236,
}fan_chip_e;

typedef enum
{
    FAN_SPEED_STABLE,
    FAN_SPEED_UP,
    FAN_SPEED_DOWN,    
}fan_speed_adjust_opt;

typedef struct
{
    uint32 bottom;      /* customized bottom pwm */
    uint8 bootom_rate; /* customized bottom speed rate */
    uint32 low;         /* customized low pwm */
    uint8 low_rate;    /* customized low speed rate */
    uint32 high;        /* customized high pwm */
    uint8 high_rate;   /* customized high speed rate */
    uint32 full;        /* customized full pwm */
    uint8 full_rate;   /* customized full speed rate */
}fan_pwm_t;

typedef struct
{
    uint8 ADT7470_pwm;
    uint8 EPLD_pwm;
    uint32 ctc5236_pwm;
    uint8 rate;
    uint16 rpm;
}PWM_RPM_MAP;

typedef struct
{
    uint8 fan_type;
    uint8 support_hot_swap; /*also support B330 fan. in bug18045 */
    fan_chip_e chip_type; /*fan chip type*/
    fan_io_type_e io_type; /*fan access type*/
    uint8 fan_num;  /*chip manage fan num*/
    uint8 speed_adjust; /*support speed adjust*/
    uint8 fan_ver;  /*fan version*/
    uint32 rpm;    /*Revolutions per minites, bug29436, qicx, 2014-08-08*/
    fan_pwm_t pwm;
    PWM_RPM_MAP *pwm_map_table;  /*add for adjust fan speed by rate,qianj,2017-09-22*/
    uint8 pwm_map_table_num;    /*add for adjust fan speed by rate,qianj,2017-09-22*/ 
    uint8 pwm_channel_flag;  /* add for bug 53774,to adjust fan channel order.0:default 1:custom,chenjr,2019-09-27 */ 
    uint8 pwm_channel_idx[4];
}fan_chip_t;

typedef struct
{
    uint8 fan_type;
    uint8 fan_num;
    uint8 speed_adjust;
    uint8 support_hot_swap; /*also support B330 fan. in bug18045 */
    uint8 fan_ver;
    uint8 access_err_cnt;
    uint8 cfg_speed_level;
    uint16 set_speed;
    uint8 local_set_speed[4]; /*bug 16600. for walkaround hw fan problem*/
    uint32 rpm;   /*Revolutions per minites, bug29436, qicx, 2014-08-08*/
    fan_pwm_t pwm;
    uint8 pwm_chip_idx;
    uint8 pwm_channel_idx[4];
}fan_private_data_t;

typedef struct
{
    uint8 access_fail;
    uint8 absent;
    uint8 status_fail;
    
}fan_diag_result_t;

typedef struct fan_handle_s fan_handle_t;
struct fan_handle_s
{
    int32 (* fan_init)(fan_handle_t* );
    int32 (* get_real_speed)(fan_handle_t* , uint16*);
    int32 (* config_speed_level)(fan_handle_t* , uint8 );
    int32 (* get_status)(fan_handle_t* , uint8*);
    int32 (* reg_read)(fan_handle_t*, uint8 , uint8* , uint8 );
    int32 (* reg_write)(fan_handle_t*, uint8 , uint8* , uint8 );
    uint32 (* get_fan_pwm)(PWM_RPM_MAP* , uint8);
    int32 (* user_config_speed_rate)(fan_handle_t*, uint16*, uint8);
    int32 (* get_present)(uint8, uint8*);
    fan_private_data_t* p_data;
    fan_io_handle_t *io_hdl;
};

int32 fan_dev_init(uint8 fan_module_idx);
int32 fan_dev_reinit(uint8 fan_module_idx);
int32 fan_config_speed_level(uint8 fan_module_idx, uint8 cfg_speed_level);
int32 fan_get_speed_level(uint8 fan_module_idx, uint8* cfg_speed_level);
int32 fan_get_speed_adjust(uint8 fan_module_idx, uint8* speed_adjust);
int32 fan_get_num(uint8 fan_module_idx, uint8* num);
int32 fan_get_speed_rate_by_level(uint8 fan_module_idx, uint8 cfg_speed_level, uint8* rate);
int32 fan_get_present(uint8 fan_module_idx, uint8* present);
int32 fan_get_status(uint8 fan_module_idx, uint8* status, uint8* fan_num);
int32 fan_init(void** p_data, fan_chip_t* type, uint8 chip_num);
int32 fan_reg_read(uint8 fan_module_idx, uint8 reg, uint8* value);
int32 fan_reg_write(uint8 fan_module_idx, uint8 reg, uint8 value);
int32 fan_update_dev_info(uint8 fan_module_idx);
int32 fan_diagnostic_test(uint8 fan_module_num, fan_diag_result_t* diag_result);
int32 fan_get_real_speed(uint8 fan_module_idx, uint16* speed);
int32 fan_user_config_speed_rate(uint8 fan_module_idx, uint16 *cfg_speed_rate, uint8 cfg_speed_num);
int32 fan_diagnostic_test_and_report(uint8 fan_module_num);
#endif 

