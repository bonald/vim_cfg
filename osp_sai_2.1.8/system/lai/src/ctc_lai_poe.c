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
ctc_lai_get_poe_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_poe_info_t* poe)
{
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    uint8 poeid = 0;
    uint8 i = 0,cnt = 0;

    if(NULL == poe)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if((oid >> 16)  > p_card->poe_module_num)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memset(poe, 0, sizeof(lai_poe_info_t));

    poe->oid = oid;
    poe->slot_id = p_card->phy_slot_no;
    poeid = (uint8)(oid & 0x000000ff);

    sal_memset(poe, 0, sizeof(lai_poe_info_t));
    poe->glb_enable = p_card->poe_sys_info[poeid].sys_cfg.enable;
    poe->glb_power_limit = p_card->poe_sys_info[poeid].sys_cfg.power_limit;
    poe->glb_status = p_card->poe_sys_info[poeid].status;
    poe->glb_cur_consump = p_card->poe_sys_info[poeid].cur_consump;
    poe->glb_aver_consump = p_card->poe_sys_info[poeid].aver_consump;
    poe->glb_peak_consump = p_card->poe_sys_info[poeid].peak_consump;
    poe->pse_channel_num = p_card->poe_sys_info[poeid].pse_channel_num;
    poe->const_power_limit = p_card->poe_sys_info[poeid].const_power_limit;

    for (i = 0; i < p_card->port_num; i++)
    {
        p_port = p_card->pp_port[i];
        if(p_port == NULL)
        {
            LCM_LOG_BRINGUP("ctc_lai_get_poe_info, pp_port is NULL");
            return LAI_STATUS_PORT_NOT_INIT;
        }
        if((p_port->poe_info.pse_id == poeid)&&(p_port->poe_support))
        {
            for (cnt = 0; cnt < LAI_POE_PORT_NUM; cnt++)
            {
                if(cnt == p_port->poe_info.pse_channel_id)
                {
                    poe->pse_channel_id[cnt] = p_port->poe_info.pse_channel_id;
                    poe->panel_slot_no[cnt] = p_port->panel_slot_no;
                    poe->panel_port_no[cnt] = p_port->panel_port_no;
                    poe->panel_sub_port_no[cnt] = p_port->panel_sub_port_no;
                    poe->enable[cnt] = p_port->poe_info.channel_cfg.enable;
                    poe->force_power[cnt] = p_port->poe_info.channel_cfg.force_power;
                    poe->mode[cnt] = p_port->poe_info.channel_cfg.mode;
                    poe->priority[cnt] = p_port->poe_info.channel_cfg.priority;
                    poe->power_limit[cnt] = p_port->poe_info.channel_cfg.power_limit;
                    poe->class_level[cnt] = p_port->poe_info.class;
                    poe->status[cnt] = p_port->poe_info.status;
                    poe->cur_consump[cnt] = p_port->poe_info.cur_consump;
                    poe->aver_consump[cnt] = p_port->poe_info.aver_consump;
                    poe->peak_consump[cnt] = p_port->poe_info.peak_consump;
                    poe->current[cnt] = p_port->poe_info.current;
                    poe->voltage[cnt] = p_port->poe_info.voltage;
                    poe->temper[cnt] = p_port->poe_info.temper;
                }
            }
        }
    }
    
    return LAI_STATUS_SUCCESS;
}

static lai_poe_api_t      g_lai_poe_api_func = {
    .get_poe_info      = ctc_lai_get_poe_info,
};

static ctc_lai_api_reg_info_t g_poe_api_reg_info = {
    .id         = LAI_API_POE,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_poe_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_poe_init()
{
    lai_api_reg_register_fn(&g_poe_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}




