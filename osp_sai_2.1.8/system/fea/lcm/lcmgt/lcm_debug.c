/****************************************************************************
 * lcm_debug.c      Lcm debug source file.
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianting Huang
 * Date          :2010-08-16
 * Reason        :First Create.
 ****************************************************************************/
#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_macro.h"
#include "lcm_client.h"
#include "lcm_mgt.h"
#include "lcm_error.h"
#include "lcm_debug.h"
#include "drv_debug.h"

#ifdef _LCM_DEBUG_
int8 g_lcm_bringup_print_on = 1;
#else
int8 g_lcm_bringup_print_on = 0;
#endif

int32 g_lcm_debug_type = 0;
int64 g_drv_debug_type = 0;

void lcm_bringup_log_set(uint8 value)
{
    g_lcm_bringup_print_on = value;
}

void lcm_debug_register()
{
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_CARD);
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_LCMGT);
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_CHSMTLK);
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_DRVS);
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_LCSH);
    GLB_UNSET_FLAG(g_lcm_debug_type,LCM_CLIENT);
}

void drv_debug_register()
{
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_BUS_I2C);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_BUS_SPI);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_BUS_MDIO);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_PHY);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_EPLD);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_FIBER);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_SENSOR);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_GPIO);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_MUX);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_FAN);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_EPLD);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_EEPROM);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_CLOCK);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_VCM);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_LED);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_LTC2991);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_LTC4151);
    GLB_UNSET_FLAG(g_drv_debug_type,DRV_VSC3308);    
}

/*********************************************************************
 * Name    : lcm_debug_init
 * Purpose : This is a init function for lcm debug module
 * Input   : N/A
 * Output  : N/A
 * Return  : LCM_E_SUCCESS
 * Note    : 2010.8.16 init version
*********************************************************************/
int32
lcm_debug_init(void)
{
    ctclib_debug_init(FALSE);

    /* new lcm debug system */
    lcm_debug_register();

    /* new drv debug system */
    drv_debug_register();

    return LCM_E_SUCCESS;
}

