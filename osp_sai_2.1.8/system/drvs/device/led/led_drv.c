/****************************************************************************
 * led_drv.c    led drv 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       zhuj
 * Date:         2010-11-19.
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
#include "led_drv.h"
#include "epld_api.h"
#include "gpio_api.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/


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
int32 led_op(const led_handle_t* phdl, glb_led_stat_t led_stat)
{
    led_info_t* p_info;

    if(NULL == phdl || NULL == phdl->p_data)
    {
        DRV_LOG_ERR("Control led: invalid handler pointer.");
        return RESULT_ERROR;    
    }

    if(led_stat < 0 || led_stat >=  E_LED_STAT_MAX)
    {
        DRV_LOG_ERR("Invalid parameter when control led, led stat %d.", led_stat);
        return RESULT_ERROR;    
    }

    p_info = (led_info_t*)phdl->p_data;
    switch (p_info->led_ctl)
    {
        case E_LED_EPLD0:
        case E_LED_EPLD1:
            epld_item_write(p_info->led_ctl, p_info->led_para[led_stat].op_reg, p_info->led_para[led_stat].val);
            break;
        case E_LED_I2C_GPIO0:
        case E_LED_I2C_GPIO1:
            gpio_set_special_bit(p_info->led_ctl-E_LED_I2C_GPIO0, p_info->led_para[led_stat].op_reg, p_info->led_para[led_stat].val);
            break;
        default:
            break;
    }

    return 0;
}


led_handle_t* led_register(led_info_t* p_info)
{
    led_handle_t* phdl;

    if(NULL == p_info)
    {
        DRV_LOG_ERR("Led register: invalid pointer.");
        return NULL;    
    }

    phdl = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(led_handle_t));
    if(!phdl)
    {
        DRV_LOG_ERR("Led register: no memory.");
        return NULL;
    }

    phdl->p_data = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(led_info_t));
    if(!phdl->p_data)
    {
        DRV_LOG_ERR("Led register: no memory.");
        DRV_FREE(CTCLIB_MEM_DRIVER_LED_INFO, phdl);
        return NULL;
    }

    sal_memcpy(phdl->p_data, p_info, sizeof(led_info_t));
    phdl->op = led_op;

    return phdl;    
}


