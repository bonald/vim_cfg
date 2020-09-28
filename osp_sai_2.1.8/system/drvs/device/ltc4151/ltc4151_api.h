/****************************************************************************
* $Id$
*  The header file of the ltc4151 api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2014-01-26 
* Reason        : First Create.
****************************************************************************/
#ifndef __LTC4151_API_H__
#define __LTC4151_API_H__
#include "ltc4151_drv.h"

#define LTC4151_OFFSET_WIDTH 1

typedef enum 
{
    LTC4151_CDD,
    LTC4151_IDD,    
}ltc4151_chip_e;


typedef struct 
{
    ltc4151_chip_e chip_type; /* ltc4151 chip type*/
    ltc4151_io_type_e io_type; /* ltc4151 access type*/
}ltc4151_chip_t;

typedef enum ltc4151_volt_type_e
{
    LTC4151_SINGLE_ENDED_VOLTAGE,
    LTC4151_VCC_VOLTAGE,
    LTC4151_DIFFERENTIAL_VOLTAGE,
    LTC4151_MAX_VOLTAGE,
} ltc4151_volt_type_t;


typedef struct ltc4151_handle_s ltc4151_handle_t;
struct ltc4151_handle_s
{
    int32 (* ltc4151_init)(ltc4151_handle_t* );
    int32 (* get_volt_cur)(ltc4151_handle_t* , float32* , float32*, float rsense);
    int32 (* reg_read)(ltc4151_handle_t*, uint8 , uint8* );
    int32 (* reg_write)(ltc4151_handle_t*, uint8 , uint8* );
    uint8 access_err_cnt;
    ltc4151_io_handle_t *io_hdl;
};

int32 ltc4151_dev_get_volt_cur(uint32 ltc4151_id, float32* voltage, float32* current, float *rsense);
int32 ltc4151_reg_read(uint32 ltc4151_id, uint8 reg, uint8* value, uint8 len);
int32 ltc4151_reg_write(uint32 ltc4151_id, uint8 reg, uint8* value, uint8 len);
int32 ltc4151_init(void** p_data, ltc4151_chip_t* type, uint32 num);
int32 ltc4151_dev_init(uint32 ltc4151_id);
#endif 

