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
ctc_lai_get_psu_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_psu_info_t* psu)
{
    glb_card_t* p_card = NULL; 
    uint8 psuid = 0;

    if(NULL == psu)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    if((oid >> 16)  > p_card->psu_module_num)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memset(psu, 0, sizeof(lai_psu_info_t));

    psu->oid = oid;
    psu->slot_id = p_card->phy_slot_no;
    psuid = (uint8)(oid & 0x000000ff);

    if(p_card->psu_status[psuid].psu_work_status != PSU_OK)
    {
        psu->status |= LAI_PSU_STATUS_FAILED;
    }
    if(p_card->psu_status[psuid].psu_absent == PSU_PRESENT)
    {
        psu->status |= LAI_PSU_STATUS_PRESENT;
    }
    else
    {
        psu->status |= LAI_PSU_STATUS_ABESENT;
    }
    if(p_card->psu_status[psuid].psu_alert == PSU_ALERT)
    {
        psu->status |= LAI_PSU_STATUS_ALERT;
    }
    /* modified by liuht for bug41517, 2016-11-04 */
    /* if psu mode is unknown, should not set any flag to psu->mode */
    if(p_card->psu_status[psuid].psu_mode == PSU_UNKNOWN)
    {
        psu->mode = 0;
    }
    else if(p_card->psu_status[psuid].psu_mode == PSU_DC)
    {
        psu->mode |= LAI_PSU_MODE_DC48;
    }
    else
    {
        psu->mode |= LAI_PSU_MODE_AC;
    }

    psu->fixed_status = p_card->psu_status[psuid].fixed_status;
    psu->psu_health_status_change = p_card->psu_status[psuid].psu_health_status_change;
    if (psu->psu_health_status_change)
    {
        psu->support_pmbus = p_card->psu_status[psuid].psu_pmbus_status.support_pmbus;
    
        psu->status_byte = p_card->psu_status[psuid].psu_pmbus_status.status_byte;
        psu->status_word = p_card->psu_status[psuid].psu_pmbus_status.status_word;
        psu->status_vout = p_card->psu_status[psuid].psu_pmbus_status.status_vout;
        psu->status_iout = p_card->psu_status[psuid].psu_pmbus_status.status_iout;
        psu->status_input = p_card->psu_status[psuid].psu_pmbus_status.status_input;
        psu->status_temperature = p_card->psu_status[psuid].psu_pmbus_status.status_temperature;
        psu->status_cml = p_card->psu_status[psuid].psu_pmbus_status.status_cml;
        psu->status_other = p_card->psu_status[psuid].psu_pmbus_status.status_other;
        psu->status_mfr_specific = p_card->psu_status[psuid].psu_pmbus_status.status_mfr_specific;
        psu->status_fans_1_2 = p_card->psu_status[psuid].psu_pmbus_status.status_fans_1_2;
        psu->status_fans_3_4 = p_card->psu_status[psuid].psu_pmbus_status.status_fans_3_4;
    }
    
    return LAI_STATUS_SUCCESS;
}

static lai_psu_api_t      g_lai_psu_api_func = {
    .get_psu_info      = ctc_lai_get_psu_info,
};

static ctc_lai_api_reg_info_t g_psu_api_reg_info = {
    .id         = LAI_API_PSU,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_psu_api_func,
    .init_status      = INITIALIZED,
};

lai_status_t
ctc_lai_psu_init()
{
    lai_api_reg_register_fn(&g_psu_api_reg_info);
    
    return LAI_STATUS_SUCCESS;
}




