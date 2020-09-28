#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <saiapsgroup.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_vlan.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_notifier_call.h>

sai_status_t
ctc_sai_create_raps_mc_group(uint16_t group_id)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_aps_create_raps_mcast_group(group_id);
    return ret;
}

sai_status_t
ctc_sai_destroy_raps_mc_group(uint16_t group_id)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_aps_destroy_raps_mcast_group(group_id);
    return ret;
}

 sai_status_t
  ctc_sai_aps_set_protection_en(
	_In_ uint16_t aps_group_id,
	_In_ uint8_t enable)
{
        sai_status_t    ret = SAI_STATUS_SUCCESS;
       ctc_aps_set_aps_bridge(aps_group_id, enable);
       return ret;
}
sai_status_t 
ctc_sai_add_aps_group(
	_In_ uint32_t gport,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list)
	{

        sai_status_t    ret = SAI_STATUS_SUCCESS;
        uint16_t  mgroup_id = 0;
        uint32_t  aps_group_id = 0;
        uint32_t  nh_id = 0;
        uint32_t  nh_offset = 0;
        uint32_t         attr_idx    = 0;
        sai_attribute_t* attr        = NULL;
        ctc_aps_bridge_group_t aps_group;
        ctc_vlan_edit_nh_param_t nh_param;
        
        sal_memset(&aps_group, 0, sizeof(ctc_aps_bridge_group_t));
        for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
        {
            attr = attr_list + attr_idx;
            switch(attr->id)
            {
               case SAI_APS_GROUP_MGROUP_TYPE_ID:
                    mgroup_id = attr->value.s16;
                    break;
               case SAI_APS_GROUP_TYPE_ID:
                    aps_group_id = attr->value.s32;
                    break;
               case SAI_APS_GROUP_TYPE_NH_ID:
                    nh_id = attr->value.s32;
                    break;
               case SAI_APS_GROUP_TYPE_NH_OFFSET:
                    nh_offset = attr->value.s32;
                    break;
            }
        }
        aps_group.working_gport = gport;
        aps_group.raps_group_id = mgroup_id;
        aps_group.protect_en = FALSE;
        aps_group.raps_en = TRUE;
        ctc_aps_create_aps_bridge_group(aps_group_id, &aps_group);
        sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
        nh_param.aps_en = TRUE;
        nh_param.dsnh_offset = nh_offset;
        nh_param.gport_or_aps_bridge_id = aps_group_id;

        ctc_nh_add_xlate(nh_id, &nh_param);
        return ret;
}

sai_status_t 
ctc_sai_aps_delete_aps_group(
	_In_ uint16_t aps_group_id, 
	_In_ uint32_t nh_id)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_aps_destroy_aps_bridge_group(aps_group_id);
    ctc_nh_remove_xlate(nh_id);
    return ret;
}
	
sai_status_t 
ctc_sai_aps_mgroup_update_member_port(
	_In_ uint32_t port_id,
	_In_ uint32_t attr_count,
	_Inout_ sai_attribute_t *attr_list)
{
        uint32_t         attr_idx    = 0;
        sai_attribute_t* attr        = NULL;
        uint8_t  is_add = 0;
        uint16_t  mgroup_id = 0;
        sai_status_t    ret = SAI_STATUS_SUCCESS;
        ctc_raps_member_t raps_member;
        
        for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
        {
            attr = attr_list + attr_idx;
            switch(attr->id)
            {
                case SAI_APS_GROUP_MGROUP_TYPE_ADD:
                    is_add = attr->value.s8;
                    break;
                case SAI_APS_GROUP_MGROUP_TYPE_ID:
                    mgroup_id = attr->value.s16;
                    break;
            }
        }
        sal_memset(&raps_member, 0, sizeof(raps_member));
        raps_member.group_id = mgroup_id;
        raps_member.mem_port = port_id;
        if (is_add)
        {
            raps_member.remove_flag = FALSE;
            ret = ctc_aps_update_raps_mcast_member(&raps_member);
            
        }
        else
        {
            raps_member.remove_flag = TRUE;
            ret = ctc_aps_update_raps_mcast_member(&raps_member);
            
        }
            
	return ret;
}

#define ________SAI_RAPS_MC_GROUP_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    preg->init_status              = INITIALIZED;

out:
    return ret;

    goto out;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_aps_group_api_t      g_sai_api_func = {
    .create_raps_mc_group            = ctc_sai_create_raps_mc_group,
    .destroy_raps_mc_group        = ctc_sai_destroy_raps_mc_group,
    .aps_mgroup_update_member_port   = ctc_sai_aps_mgroup_update_member_port,
    .add_aps_group                   = ctc_sai_add_aps_group,
    .set_protection_en              = ctc_sai_aps_set_protection_en,
    .delete_aps_group              = ctc_sai_aps_delete_aps_group,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_APS_GROUP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_FDB_OUTER_FUNC
sai_status_t
ctc_sai_aps_group_init()
{
    api_reg_register_fn(&g_api_reg_info);
    ctc_aps_init(NULL);
    return SAI_STATUS_SUCCESS;
}

