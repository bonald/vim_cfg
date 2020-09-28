/****************************************************************************
* $Id$
*  The header file of the sensor operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-21 
* Reason        : First Create.
****************************************************************************/
#ifndef __SENSOR_DRV__
#define __SENSOR_DRV__

#include "ctc_i2c.h"

typedef enum
{
    SENSOR_I2C = 0,        /* access the temperature sensor by I2C bus */
} sensor_io_type_e;

/* define the structure including i2c operation paramaters */
struct sensor_access_s{
    uint8 reg;         /* the type of the temperature sensor register */     
    uint8* val;              /* pointer of the value buffur */
    uint8 len;
#ifdef _CTC_X86_64_
    uint8 width;
#endif
};
typedef struct sensor_access_s sensor_access_t;

typedef struct sensor_io_handle_s sensor_io_handle_t;
struct sensor_io_handle_s 
{
    int32 (*close)(sensor_io_handle_t *);
    int32 (*read)(const sensor_io_handle_t *, sensor_access_t *);
    int32 (*write)(const sensor_io_handle_t *, sensor_access_t *);
    void* handle;      /*handle of the low level operations */
};

sensor_io_handle_t *sensor_io_register(sensor_io_type_e type, const void *p_data);

#endif 

