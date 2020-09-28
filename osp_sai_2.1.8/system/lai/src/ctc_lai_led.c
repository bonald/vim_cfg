/****************************************************************************
* $Id$
*  Centec function information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-011
* Reason        : First Create.
****************************************************************************/

#include "laiinc.h"

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_tempfile_define.h"
#include "lcapi.h"
#include "lcm_specific.h"
#include "lcm_mgt.h"
#include "lcm_client.h"
#include "lcm_error.h"
#include "lcm_card.h"
#include "lcm_port.h"
#include "lcm_dpll.h"
#include "lcm_debug.h"
#include "lcm_srv.h"
#include "lcm_lai.h"

#ifndef _GLB_UML_SYSTEM_
static uint32_t ctc_lai_led_mode[E_MAX_LED] = {0};
static uint32_t ctc_lai_led_status[E_MAX_LED] = {0};

static char* ctc_lai_led_name[] = {"SYSTEM LED", "RUNNING LED", "ALARM LED", "FAN LED",
    "PSU1 LED", "PSU2 LED", "POE LED", "IND LED"};
//static char* ctc_lai_led_status_name[] = {"SYSTEM INIT", "SYSTEM RUNNING", "SYSTEM ABNORMAL", 
   // "SYSTEM ALARM", "SYSTEM NO ALARM", "SYSTEM ABSENT", "SYSTEM OFF", "SYSTEM ON"};
#endif
/*Note, LED mode is from normal led control, led status is from Centec LED driver implementation, 
it should be update to mode if driver is optimized*/

lai_status_t
ctc_lai_get_led_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_led_info_t* led)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 

    if(NULL == led)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if(oid >= E_MAX_LED)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memset(led, 0, sizeof(lai_led_info_t));

    led->oid = oid;
    led->slot_id = p_card->phy_slot_no;
    
    sal_strcpy(led->name, ctc_lai_led_name[oid]);
    led->caps = LAI_LED_CAPS_ON_OFF | LAI_LED_CAPS_RED | LAI_LED_CAPS_RED_BLINKING |
        LAI_LED_CAPS_ORANGE | LAI_LED_CAPS_ORANGE_BLINKING | LAI_LED_CAPS_YELLOW |
        LAI_LED_CAPS_YELLOW_BLINKING | LAI_LED_CAPS_GREEN |LAI_LED_CAPS_GREEN_BLINKING |
        LAI_LED_CAPS_BLUE | LAI_LED_CAPS_BLUE_BLINKING | LAI_LED_CAPS_PURPLE | LAI_LED_CAPS_PURPLE_BLINKING;
    led->mode = ctc_lai_led_mode[oid];
    led->status = ctc_lai_led_status[oid];
#endif
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_set_led_info(
    _In_ lai_object_id_t oid,
    _In_ lai_led_info_t* led)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;
    int location = 0;
    int mode = 0;

    if(NULL == led)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    if(oid >= E_MAX_LED)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    ctc_lai_led_mode[oid] = led->mode;
    ctc_lai_led_status[oid] = led->status;

    if(oid == LAI_LED_LOCATION_CHASSIS)
    {
        location = E_IND_LED;
    }

    if (led->mode)
    {
        mode = E_LED_SYS_ON;
    }
    else
    {
        mode = E_LED_SYS_OFF;
    }

    ret = lcm_lai_rx_laiLedSet(location, mode);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}


static lai_led_api_t      g_lai_led_api_func = {
    .get_led_info      = ctc_lai_get_led_info,
    .set_led_mode  = ctc_lai_set_led_info,
};

static ctc_lai_api_reg_info_t g_led_api_reg_info = {
    .id         = LAI_API_LED,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_led_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_led_init()
{
    lai_api_reg_register_fn(&g_led_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}



