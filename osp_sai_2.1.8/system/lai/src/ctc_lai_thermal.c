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

lai_status_t
ctc_lai_get_thermal_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_thermal_info_t* thermal)
{
    glb_card_t* p_card = NULL; 
    uint32 id = 0;

    if(NULL == thermal)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if (oid >= p_card->sensor_num + p_card->chip_sensor_num)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    sal_memset(thermal, 0, sizeof(lai_thermal_info_t));
    id = oid & 0x000000FF;

    thermal->oid = oid;
    thermal->slot_id = p_card->phy_slot_no;
    thermal->is_chip = FALSE;
    
    thermal->tempr = p_card->tmpr_status[id].tmpr_val;
    
    switch(p_card->tmpr_status[oid].pos)
    {
        case SENSOR_CPU:
            sal_strcpy(thermal->posision, "AROUND_CPU");
            break;
            
        case SENSOR_BEFORE_CHIP:
            sal_strcpy(thermal->posision, "AROUND_CHIP");
            break;
            
        case SENSOR_BEHIND_CHIP:
            sal_strcpy(thermal->posision, "AROUND_CHIP");
            break;
            
        case SENSOR_FAN:
            sal_strcpy(thermal->posision, "AROUND_FAN");
            break;
            
        case SENSOR_PSU:
            sal_strcpy(thermal->posision, "AROUND_PSU");
            break;
            
        case SENSOR_CHIP0:
            thermal->is_chip = TRUE;
            sal_strcpy(thermal->posision, "SWITCH_CHIP0");
            break;
            
        case SENSOR_CHIP1:
            thermal->is_chip = TRUE;
            sal_strcpy(thermal->posision, "SWITCH_CHIP1");
            break;
            
        default:
            sal_strcpy(thermal->posision, "UNKNOWN");
    }


    return LAI_STATUS_SUCCESS;
}



lai_status_t
ctc_lai_set_tempr_thershold(
    _In_ lai_object_id_t oid,
    _In_ lai_thermal_info_t* thermal)
{

    return LAI_STATUS_SUCCESS;
}

static lai_thermal_api_t      g_lai_thermal_api_func = {
    .get_thermal_info      = ctc_lai_get_thermal_info,
    .set_thermal_info  = NULL,
};

static ctc_lai_api_reg_info_t g_thermal_api_reg_info = {
    .id         = LAI_API_THERMAL,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_thermal_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_thermal_init()
{
    lai_api_reg_register_fn(&g_thermal_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}





