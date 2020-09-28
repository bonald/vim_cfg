/****************************************************************************
* $Id$
*  The header file of the gpio device operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht
* Date          : 2014-12-06 
* Reason        : First Create.
****************************************************************************/
#ifndef __GPIO_DRV__
#define __GPIO_DRV__

#include "ctc_i2c.h"

typedef enum
{
    GPIO_I2C = 0,        /* access the gpio device by I2C bus */
} gpio_io_type_e;

/* define the structure including i2c operation paramaters */
struct gpio_access_s{
    uint8 reg;         /* the type of the gpio device register */     
    uint8* val;        /* pointer of the value buffur */
    uint8 len;
};
typedef struct gpio_access_s gpio_access_t;

typedef struct gpio_io_handle_s gpio_io_handle_t;
struct gpio_io_handle_s 
{
    int32 (*close)(gpio_io_handle_t *);
    int32 (*read)(const gpio_io_handle_t *, gpio_access_t *);
    int32 (*write)(const gpio_io_handle_t *, gpio_access_t *);
    void* handle;      /*handle of the low level operations */
};

gpio_io_handle_t *gpio_io_register(gpio_io_type_e type, const void *p_data);

#endif 

