/****************************************************************************
* $Id$
*  The header file of the fan operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-10-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __FAN_DRV__
#define __FAN_DRV__

#include "ctc_i2c.h"


typedef enum
{
    E_FAN_EPLD = 0,        /* access fan by EPLD */
    E_FAN_I2C,           /* access fan by I2C */    
    E_FAN_CTC5236,
}fan_io_type_e;


/* define the structure including i2c operation paramaters */
struct fan_access_s
{
    uint8 reg;     /* fan control chip register*/   
    uint8 *val;    /* pointer of the value buffur */
    uint8 len;
} ;
typedef struct fan_access_s fan_access_t;

typedef struct fan_io_handle_s fan_io_handle_t;

struct fan_io_handle_s
{
    int32 (*read)(const fan_io_handle_t *, fan_access_t *);
    int32 (*write)(const fan_io_handle_t *, fan_access_t *);
    void* handle; /* handle of the low level operations*/
    void* p_data; /*private data for this handle*/
};
fan_io_handle_t* fan_io_register(fan_io_type_e type, const void *p_data);

#endif 

