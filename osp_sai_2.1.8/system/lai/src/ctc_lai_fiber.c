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
ctc_lai_get_fiber_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_fiber_info_t* fiber)
{
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    int ret = 0;
    uint32 portid = 0;

    if(NULL == fiber)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    portid = oid&0x000000FF;

    if(portid >= p_card->port_num)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
        
    p_port = p_card->pp_port[portid];
    if(p_port == NULL)
    {
        return LAI_STATUS_PORT_NOT_INIT;
    }

    ret = lcm_lai_get_FiberInfo(&fiber, p_port);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }

    return LAI_STATUS_SUCCESS;
}

static lai_fiber_api_t      g_lai_fiber_api_func = {
    .get_fiber_info      = ctc_lai_get_fiber_info,
};

static ctc_lai_api_reg_info_t g_fiber_api_reg_info = {
    .id         = LAI_API_FIBER,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_fiber_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_fiber_init()
{
    lai_api_reg_register_fn(&g_fiber_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}


