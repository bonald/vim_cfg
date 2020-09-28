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


ctc_lai_synce_select_port_t g_synce_port_select;

lai_status_t
ctc_lai_synce_set_global_enable(lai_synce_info_t* synce_info)
{
    lai_status_t     ret = LAI_STATUS_SUCCESS;

    lcapi_lcm_set_port_status_t port_status;

    uint32 synce_enable = FALSE;
    synce_enable = synce_info->synce_enable;

    if (g_synce_port_select.port_idx)
    {
        sal_memset(&port_status, 0, sizeof(port_status));
        port_status.status = (synce_enable && g_synce_port_select.status);
        port_status.slot_idx = g_synce_port_select.slot_idx;
        port_status.port_idx = g_synce_port_select.port_idx;
        port_status.subport_idx = g_synce_port_select.subport_idx;
        ret = lcm_port_set_syncE_enable(&port_status);
        if (ret < 0)
        {
            return LAI_STATUS_FAILURE;
        }
    }
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_synce_set_select_ifindex(lai_synce_info_t* synce_info)
{
    lai_status_t     ret = LAI_STATUS_SUCCESS;
    lcapi_lcm_set_port_status_t port_status;
    
    sal_memset(&port_status, 0, sizeof(port_status));
    port_status.slot_idx = synce_info->slot_idx;
    port_status.port_idx = synce_info->port_idx;
    port_status.status = synce_info->status;
    port_status.subport_idx = synce_info->subport_idx;

    ret = lcm_port_set_syncE_enable(&port_status);
    if (ret < 0)
    {
        return LAI_STATUS_FAILURE;
    }
    g_synce_port_select.port_idx = synce_info->port_idx;
    g_synce_port_select.slot_idx = synce_info->slot_idx;
    g_synce_port_select.status = synce_info->status;
    g_synce_port_select.subport_idx = synce_info->subport_idx;
    g_synce_port_select.gport = synce_info->gport;
    
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_synce_set_clock_recovery_select_enable(lai_synce_info_t* synce_info)
{
    lai_status_t     ret = LAI_STATUS_SUCCESS;
    lcapi_lcm_port_select_slot_clock_t slot_clock;

    if ((0 == synce_info->clock_recovery_select_en) && g_synce_port_select.status)
    {
        sal_memset(&slot_clock, 0x00, sizeof(lcapi_lcm_port_select_slot_clock_t));
        slot_clock.mode = g_synce_port_select.status;
        slot_clock.slot = g_synce_port_select.slot_idx;
        slot_clock.port_idx = g_synce_port_select.port_idx;
        slot_clock.gport = g_synce_port_select.gport;
    }
    ret = lcm_port_set_syncE_enable(&slot_clock);
    if (ret < 0)
    {
        return LAI_STATUS_FAILURE;
    }
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_synce_set_attribute(    
    _In_ lai_synce_info_t* synce_info)
{
    lai_status_t     ret = LAI_STATUS_SUCCESS;

    if (NULL == synce_info)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    switch (synce_info->id)
    {
    case LAI_SYNCE_GLOBAL_ENBALE:
        ret = ctc_lai_synce_set_global_enable(synce_info);
        break;
    case LAI_SYNCE_SELECTED_IFINDEX:
        ret = ctc_lai_synce_set_select_ifindex(synce_info);
        break;
    case LAI_SYNCE_CLOCK_RECOVERY_SELECT_ENABLE:
        ret = ctc_lai_synce_set_clock_recovery_select_enable(synce_info);
        break;

    default:
        break; 
    }
    
    return ret;
}



static lai_synce_api_t      g_lai_synce_api_func = {
    .set_synce_attribute    = ctc_lai_synce_set_attribute,
};

static ctc_lai_api_reg_info_t g_synce_api_reg_info = {
    .id         = LAI_API_SYNCE,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_synce_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_synce_init()
{
    lai_api_reg_register_fn(&g_synce_api_reg_info);    
    return LAI_STATUS_SUCCESS;
}

