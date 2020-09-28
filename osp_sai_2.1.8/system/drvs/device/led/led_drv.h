/****************************************************************************
* $Id$
*  The header file of the led operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : zhuj
* Date          : 2010-08-04 
* Reason        : First Create.
****************************************************************************/
#ifndef __LED_DRV__
#define __LED_DRV__

#include "glb_hw_define.h"

typedef enum
{
    E_LED_EPLD0 = 0,        /* operate led by EPLD 0 */
    E_LED_EPLD1    ,        /* operate led by EPLD 1 */
    E_LED_I2C_GPIO0,
    E_LED_I2C_GPIO1,
}led_ctl_t;

struct led_para_s
{
    uint32 op_reg;   /* the control filed */
    uint8 val;       /* the value of the control filed */

};
typedef struct led_para_s led_para_t;

struct led_info_s{
    uint8 exist;
    led_ctl_t led_ctl;
    led_para_t led_para[E_LED_STAT_MAX];
};
typedef struct led_info_s led_info_t;

typedef struct led_handle_s led_handle_t;
struct led_handle_s
{
    int32 (*op)(const led_handle_t*, glb_led_stat_t);
    void* p_data;
};

led_handle_t* led_register(led_info_t* p_info);

#endif 

