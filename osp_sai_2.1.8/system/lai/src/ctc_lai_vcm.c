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
ctc_lai_get_vcm_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_vcm_info_t* vcm)
{
    glb_card_t* p_card = NULL; 
    uint8 vcmid = 0;
    uint8 i = 0;

    if(NULL == vcm)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if((oid >> 16)  > p_card->vcm_module_num)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memset(vcm, 0, sizeof(lai_vcm_info_t));

    vcm->oid = oid;
    vcm->slot_id = p_card->phy_slot_no;
    vcmid = (uint8)(oid & 0x000000ff);

    vcm->vcm_type = p_card->vcm_status[vcmid].vcm_type;
    vcm->vcm_channel_nums = p_card->vcm_status[vcmid].vcm_channel_nums;
    for (i = 0; i < LAI_VCM_CHANNEL_NUM; i++)
    {
        sal_memcpy(vcm->channel_name[i], p_card->vcm_cfg[vcmid].channel_cfg[i].name, MAX_VCM_CHANNEL_LEN+1);
        vcm->flag[i] = (uint32_t)p_card->vcm_cfg[vcmid].channel_cfg[i].flag;
        vcm->curr_depend_pin[i] = (uint8_t)p_card->vcm_cfg[vcmid].channel_cfg[i].curr_depend_pin;
        vcm->volt_split_factor[i] = (double)p_card->vcm_cfg[vcmid].channel_cfg[i].volt_split_factor;
        vcm->low[i] = (double)p_card->vcm_cfg[vcmid].channel_cfg[i].low;
        vcm->high[i] = (double)p_card->vcm_cfg[vcmid].channel_cfg[i].high;
        vcm->volt[i] = (double)p_card->vcm_status[vcmid].volt[i];
        vcm->curr[i] = (double)p_card->vcm_status[vcmid].curr[i];
        vcm->tmpr[i] = (double)p_card->vcm_status[vcmid].tmpr[i];
    }
    
    return LAI_STATUS_SUCCESS;
}

static lai_vcm_api_t      g_lai_vcm_api_func = {
    .get_vcm_info      = ctc_lai_get_vcm_info,
};

static ctc_lai_api_reg_info_t g_vcm_api_reg_info = {
    .id         = LAI_API_VCM,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_vcm_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_vcm_init()
{
    lai_api_reg_register_fn(&g_vcm_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}




