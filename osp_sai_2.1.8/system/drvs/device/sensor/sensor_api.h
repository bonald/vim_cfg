/****************************************************************************
* $Id$
*  The header file of the sensor api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-21 
* Reason        : First Create.
****************************************************************************/
#ifndef __SENSOR_API_H__
#define __SENSOR_API_H__
#include "sensor_drv.h"

#define LM77_OFFSET_WIDTH 1
#define MAX1617A_OFFSET_WIDTH 1
#define LM75_OFFSET_WIDTH 1
#define TMP42X_OFFSET_WIDTH 1
#define SD5075_OFFSET_WIDTH 1
#define ADT7476A_OFFSET_WIDTH 1
#define DIAG_MAX_SENSOR_NUM 5

typedef enum 
{
    SENSOR_LM77,
    SENSOR_MAX1617A,    
    SENSOR_LM75,
    SENSOR_TMP42X,
    SENSOR_SD5075,
    SENSOR_ADT7476A,
}sensor_chip_e;

struct tmpr_level_s
{
    int32   low;
    int32   high;
    int32   full;
};
typedef struct tmpr_level_s tmpr_level_t;

typedef struct 
{
    sensor_chip_e chip_type; /* sensor chip type*/
    sensor_io_type_e io_type; /* sensor access type*/
    uint8 pos;  /* bug 32404. sensor position*/
    uint8 chan;
}sensor_chip_t;

typedef struct 
{
    uint8 access_fail;
    uint8 temp_alarm;
}sensor_diag_result_t;

typedef enum tmpr_type_e
{
    CURRENT_TEMP,
    LOW_TEMP,
    HIGH_TEMP,
    THSYT_TEMP,
    CRIT_TEMP,
    MAX_TEMP,
} tmpr_type_t;

typedef struct sensor_handle_s sensor_handle_t;
struct sensor_handle_s
{
    int32 (* sensor_init)(sensor_handle_t* );
    int32 (* get_temp)(sensor_handle_t* , tmpr_type_t , int16* );
    int32 (* set_temp)(sensor_handle_t* , tmpr_type_t , int16 );
    int32 (* reg_read)(sensor_handle_t*, uint8 , uint8* , uint8 );
    int32 (* reg_write)(sensor_handle_t*, uint8 , uint8* , uint8 );
    uint8 reg_info[MAX_TEMP];
    uint8 access_err_cnt;
    uint8 position; /*descript sensor position see sensor_position_e*/
    uint8 chip_type;
    uint8 chan;
    tmpr_level_t level;
    sensor_io_handle_t *io_hdl;
};

int32 sensor_dev_get_temp(uint32 sensor_id, tmpr_type_t type, int16* temp);
int32 sensor_dev_set_temp(uint32 sensor_id, tmpr_type_t type, int16 temp);
int32 sensor_dev_get_position(uint32 sensor_id, uint8* position);
int32 sensor_dev_get_level(uint32 sensor_id, tmpr_level_t *level);
int32 sensor_reg_read(uint32 sensor_id, uint8 reg, uint8* value, uint8 len);
int32 sensor_reg_write(uint32 sensor_id, uint8 reg, uint8* value, uint8 len);
int32 sensor_init(void** p_data, sensor_chip_t* type, uint32 num);
int32 sensor_dev_init(uint32 sensor_id);

int32 sensor_diagnostic_test_and_report(uint8 sensor_num);
#endif 

