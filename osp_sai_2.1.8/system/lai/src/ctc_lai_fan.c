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
ctc_lai_get_fan_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_fan_info_t* fan)
{
    glb_card_t* p_card = NULL; 
    uint32_t fannum = 0;
    uint32_t trayid = 0;

    if(NULL == fan)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if((oid >> 16) != p_card->phy_slot_no)
    {
        return LAI_STATUS_ADDR_NOT_FOUND;
    }
    
    sal_memset(fan, 0, sizeof(lai_fan_info_t));
    fan->oid = oid;
    fan->slot_id = p_card->phy_slot_no;
    
    trayid = (oid >> 8) & 0X000000FF;
    fannum = oid & 0x000000FF;
    fan->tray = trayid;
    if(p_card->fan_status[trayid].present != FAN_PRESENT)
    {
        fan->status |= LAI_FAN_STATUS_ABSENT;
    }
    else
    {
        fan->status |= LAI_FAN_STATUS_PRESENT;
        fan->status |= LAI_FAN_STATUS_F2B;
        if(p_card->fan_status[trayid].speed_adjust)
        {
            fan->status |= LAI_FAN_STATUS_AUTO ;
        }
        else
        {
            fan->status |= LAI_FAN_STATUS_FORCE;
        }
        
        if((p_card->fan_status[trayid].status >> fannum)&0x01)
        {
            fan->status |= LAI_FAN_STATUS_OK;
        }
        else
        {
            fan->status |= LAI_FAN_STATUS_FAILED;
        }
        fan->caps = LAI_FAN_CAPS_GET_PERCENTAGE;
        fan->percentage = p_card->fan_status[trayid].speed_rate;
    }
    
    return LAI_STATUS_SUCCESS;
}

static lai_fan_api_t      g_lai_fan_api_func = {
    .get_fan_info      = ctc_lai_get_fan_info,
};

static ctc_lai_api_reg_info_t g_fan_api_reg_info = {
    .id         = LAI_API_FAN,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_fan_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_fan_init()
{
    lai_api_reg_register_fn(&g_fan_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}


