/****************************************************************************
* $Id$
*  The header file of the ltc2991 api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2014-01-26 
* Reason        : First Create.
****************************************************************************/
#ifndef __LTC2991_API_H__
#define __LTC2991_API_H__

#include "ltc2991_drv.h"
#include "glb_hw_define.h"

#define LTC2991_OFFSET_WIDTH 1

typedef enum 
{
    LTC2991_CMS,
    LTC2991_LMS,    
}ltc2991_chip_e;


typedef struct 
{
    ltc2991_chip_e chip_type;   /* ltc2991 chip type*/
    ltc2991_io_type_e io_type; /* ltc2991 access type*/
}ltc2991_chip_t;

typedef enum ltc2991_volt_type_e
{
    LTC2991_SINGLE_V1_VOLTAGE,
    LTC2991_SINGLE_V2_VOLTAGE,
    LTC2991_SINGLE_V3_VOLTAGE,
    LTC2991_SINGLE_V4_VOLTAGE,
    LTC2991_SINGLE_V5_VOLTAGE,
    LTC2991_SINGLE_V6_VOLTAGE,
    LTC2991_SINGLE_V7_VOLTAGE,
    LTC2991_SINGLE_V8_VOLTAGE,        
    LTC2991_VCC_VOLTAGE,
    LTC2991_MAX_VOLTAGE,
} ltc2991_volt_type_t;

typedef enum ltc2991_cur_type_e
{
    LTC2991_V1_V2_CURRENT,
    LTC2991_V3_V4_CURRENT,
    LTC2991_V5_V6_CURRENT,
    LTC2991_V7_V8_CURRENT,      
    LTC2991_MAX_CURRENT,
} ltc2991_cur_type_t;

typedef struct ltc2991_handle_s ltc2991_handle_t;
struct ltc2991_handle_s
{
    int32 (* ltc2991_init)(ltc2991_handle_t*, glb_board_type_t);
    int32 (* get_volt)(ltc2991_handle_t*, ltc2991_volt_type_t, float32* );
    int32 (* get_cur)(ltc2991_handle_t*, ltc2991_cur_type_t, float32, float32* );    
    int32 (* reg_read)(ltc2991_handle_t*, uint8, uint8* );
    int32 (* reg_write)(ltc2991_handle_t*, uint8, uint8* );
    uint8 access_err_cnt;
    ltc2991_io_handle_t *io_hdl;
};

int32 ltc2991_dev_get_volt(uint32 ltc2991_id, ltc2991_volt_type_t type, float32* voltage);
int32 ltc2991_dev_get_cur(uint32 ltc2991_id, ltc2991_cur_type_t type, float32 rsense, float32* voltage);

int32 ltc2991_reg_read(uint32 ltc2991_id, uint8 reg, uint8* value, uint8 len);
int32 ltc2991_reg_write(uint32 ltc2991_id, uint8 reg, uint8* value, uint8 len);
int32 ltc2991_init(void** p_data, ltc2991_chip_t* type, uint32 num);
int32 ltc2991_dev_init(glb_board_type_t board_type, uint32 ltc2991_id);
#ifndef CNOS_OS
int32 ltc_scan_real_time_value(uint32 id, volt_curr_status_t status[],volt_curr_cfg_t cfg[],uint8 channel_no_max,float *rsense);
#endif

#endif 

