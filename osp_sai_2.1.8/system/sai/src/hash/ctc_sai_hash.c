#include <sal.h>
#include <sai.h>
#include "glb_hash_field_value_define.h"
#include "ctc_api.h"
#include <saitypes.h>
#include <saistatus.h>
#include <saihash.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_hash.h>
#include <ctc_sai_debug.h>

static ctc_sai_check_u32_range_t g_range_type_limit =
{
    .min = SAI_NATIVE_HASH_FIELD_SRC_IP,
    .max = SAI_NATIVE_HASH_FIELD_MAX,
};

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_HASH_NATIVE_FIELD_LIST,
        .type   = SAI_ATTR_FALG_CREATE  | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_type_limit,
        }
    },
    {
        .id     = SAI_HASH_UDF_GROUP_LIST,
        .type   = SAI_ATTR_FALG_CREATE  | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

static struct ctc_sai_hash_info_s g_ctc_sai_hash_info;

uint32 g_hash_value_global_agg_hash;

ctc_sai_hash_entry_t*
ctc_sai_hash_entry_get(sai_object_id_t hash_id)
{
    uint32 i = 0;

    for (i = 0; i < HASH_ID_MAX_NUM; i++)
    {
        if (hash_id == g_ctc_sai_hash_info.hash[i].hash_id)
        {
            return &g_ctc_sai_hash_info.hash[i];
        }
    }   
    return NULL;
}

#define ________SAI_SAI_INNER_DEBUG_FUNC
sai_status_t
ctc_sai_create_hash_debug_param(
    _Out_ sai_object_id_t* hash_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    const sai_attribute_t *pattr_entry = NULL;
    uint32_t          attr_idx = 0; 
    uint32_t          list_idx = 0; 
    uint32            type = 0;

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = attr_list + attr_idx;
        switch (pattr_entry->id)
        {
        case SAI_HASH_NATIVE_FIELD_LIST:
            for(list_idx = 0; list_idx < pattr_entry->value.u32list.count; list_idx++)
            {
                type = pattr_entry->value.u32list.list[list_idx];
                switch (type)
                {
                case SAI_NATIVE_HASH_FIELD_SRC_IP:
                    CTC_SAI_DEBUG("out:SAI_NATIVE_HASH_FIELD_SRC_IP");
                    break;
                case SAI_NATIVE_HASH_FIELD_DST_IP:
                    CTC_SAI_DEBUG("out:SAI_NATIVE_HASH_FIELD_DST_IP");
                    break;
                case SAI_NATIVE_HASH_FIELD_SRC_MAC:
                    CTC_SAI_DEBUG("out:SAI_NATIVE_HASH_FIELD_SRC_MAC");
                    break;
                case SAI_NATIVE_HASH_FIELD_DST_MAC:
                    CTC_SAI_DEBUG("out:SAI_NATIVE_HASH_FIELD_DST_MAC");
                    break;
                default:
                    CTC_SAI_DEBUG("in:NOT SUPPORT");
                    break;
                }
            }
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t
ctc_sai_set_hash_attribute_debug_param(
    _In_ sai_object_id_t hash_id,
    _In_ const sai_attribute_t *attr)
{
    uint32       list_idx = 0;
    uint32       type = 0;

    switch (attr->id)
    {
        case SAI_HASH_NATIVE_FIELD_LIST:
            for(list_idx = 0; list_idx < attr->value.u32list.count; list_idx++)
            {
                type = attr->value.u32list.list[list_idx];
                switch (type)
                {
                case SAI_NATIVE_HASH_FIELD_SRC_IP:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_SRC_IP");
                    break;
                case SAI_NATIVE_HASH_FIELD_DST_IP:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_DST_IP");
                    break;
                case SAI_NATIVE_HASH_FIELD_VLAN_ID:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_VLAN_ID");
                    break;
                case SAI_NATIVE_HASH_FIELD_ETHERTYPE:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_ETHERTYPE");
                    break;
                case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_L4_SRC_PORT");
                    break;
                case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_L4_DST_PORT");
                    break;
                case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:          
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_IP_PROTOCOL");
                    break;
                case SAI_NATIVE_HASH_FIELD_SRC_MAC:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_SRC_MAC");
                    break;
                case SAI_NATIVE_HASH_FIELD_DST_MAC:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_DST_MAC");
                    break;
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC");
                    break;
                case SAI_NATIVE_HASH_FIELD_INNER_DST_MAC:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_INNER_DST_MAC");
                    break;
                case SAI_NATIVE_HASH_FIELD_VXLAN_VNI:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_VXLAN_VNI");
                    break;
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_IP:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_INNER_SRC_IP");
                    break;
                case SAI_NATIVE_HASH_FIELD_INNER_DST_IP:
                    CTC_SAI_DEBUG("in:SAI_NATIVE_HASH_FIELD_INNER_DST_IP");
                    break;
                default:
                    CTC_SAI_DEBUG("in:NOT SUPPORT");
                    break;
                }
            }
        default:
            break;
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_SAI_INNER_API_FUNC
sai_status_t
ctc_sai_hash_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    p_sai_glb->hash_max_count = HASH_ID_MAX_NUM;
    if (0 != ctclib_opb_create(&p_sai_glb->hash_opb, 0, p_sai_glb->hash_max_count, "hash_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_hash_create_hash(
    _Out_ sai_object_id_t* hash_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    stbl_sai_glb_t    *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t          opf_index;
    uint32_t          list_idx = 0; 
    uint32_t          attr_idx = 0; 
    uint32            type = 0;
    sai_attribute_t*  attr = NULL;
    ctc_sai_hash_entry_t *p_entry = NULL;

    if (0 != ctclib_opb_alloc_offset(&p_sai_glb->hash_opb, &opf_index))
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    p_entry = &g_ctc_sai_hash_info.hash[opf_index];
    *hash_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_HASH, opf_index);
    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_HASH_NATIVE_FIELD_LIST:
            for(list_idx = 0; list_idx < attr->value.u32list.count; list_idx++)
            {
                type = attr->value.u32list.list[list_idx];
                switch (type)
                {
                case SAI_NATIVE_HASH_FIELD_SRC_IP:
                case SAI_NATIVE_HASH_FIELD_DST_IP:
                case SAI_NATIVE_HASH_FIELD_VLAN_ID:
                case SAI_NATIVE_HASH_FIELD_ETHERTYPE:
                case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
                case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
                case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:          
                case SAI_NATIVE_HASH_FIELD_SRC_MAC:
                case SAI_NATIVE_HASH_FIELD_DST_MAC:
                case SAI_NATIVE_HASH_FIELD_VXLAN_VNI:
                case SAI_NATIVE_HASH_FIELD_INNER_DST_MAC:
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC:
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_IP:
                case SAI_NATIVE_HASH_FIELD_INNER_DST_IP:
                case SAI_NATIVE_HASH_FIELD_GRE_KEY:
                case SAI_NATIVE_HASH_FIELD_NVGRE_VNI:
                case SAI_NATIVE_HASH_FIELD_NVGRE_FLOWID:
                    CTC_BIT_SET(p_entry->flags, type);
                    break;
                default:
                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
        break;     
            
        default:
            break;
        }
    }  
    
    p_entry->hash_id = *hash_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_create_hash(
    _Out_ sai_object_id_t* hash_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t* pattr_entry_list = NULL;
    
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_create_hash_debug_param(hash_id, attr_count, attr_list);
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_PTR_VALID_CHECK(hash_id);
    
    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    ret = ctc_sai_hash_create_hash(hash_id, attr_count, attr_list);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    CTC_SAI_DEBUG("out:hash_id 0x%llx", (*hash_id));
    
out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

sai_status_t
ctc_sai_remove_hash(
    _In_ sai_object_id_t hash_id)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_set_hash_attribute(
    _In_ sai_object_id_t hash_id,
    _In_ const sai_attribute_t *attr)
{ 
    uint32       list_idx = 0;
    uint32       type = 0;
    ctc_sai_hash_entry_t *p_entry = NULL;
    
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_set_hash_attribute_debug_param(hash_id, attr);
    CTC_SAI_PTR_VALID_CHECK(attr);
 
    if (SAI_OBJECT_TYPE_HASH != CTC_SAI_OBJECT_TYPE_GET(hash_id))
    {
         return SAI_STATUS_INVALID_PARAMETER;
    } 
    p_entry = ctc_sai_hash_entry_get(hash_id);
    if (NULL == p_entry)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_entry->flags = 0;
    switch (attr->id)
    {
        case SAI_HASH_NATIVE_FIELD_LIST:
            for(list_idx = 0; list_idx < attr->value.u32list.count; list_idx++)
            {
                type = attr->value.u32list.list[list_idx];
                switch (type)
                {
                case SAI_NATIVE_HASH_FIELD_SRC_IP:
                case SAI_NATIVE_HASH_FIELD_DST_IP:
                case SAI_NATIVE_HASH_FIELD_VLAN_ID:
                case SAI_NATIVE_HASH_FIELD_ETHERTYPE:
                case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
                case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
                case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:          
                case SAI_NATIVE_HASH_FIELD_SRC_MAC:
                case SAI_NATIVE_HASH_FIELD_DST_MAC:
                case SAI_NATIVE_HASH_FIELD_VXLAN_VNI:
                case SAI_NATIVE_HASH_FIELD_INNER_DST_MAC:
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC:
                case SAI_NATIVE_HASH_FIELD_INNER_SRC_IP:
                case SAI_NATIVE_HASH_FIELD_INNER_DST_IP:
                case SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL:
                case SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT:
                case SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT:
                case SAI_NATIVE_HASH_FIELD_GRE_KEY:
                case SAI_NATIVE_HASH_FIELD_NVGRE_VNI:
                case SAI_NATIVE_HASH_FIELD_NVGRE_FLOWID:
                    CTC_BIT_SET(p_entry->flags, type);
                    break;
                default:
                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
        break; 
        
        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }
    p_entry->hash_id = hash_id;
    return SAI_STATUS_SUCCESS;
}

#ifdef TSINGMA
sai_status_t
ctc_sai_hash_field_profile_disable_nosupport_tunnel(sai_uint32_t profile_id)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(ctc_lb_hash_config_t));
    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.value = 1;

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN4;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN4;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN4;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN6;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN6;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN6;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_l2_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));
    
    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_L2;
    if (cfg_value & GLB_HASH_PROFILE_L2_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_L2_ETH_TYPE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_ETHER_TYPE);
    }
    if (cfg_value & GLB_HASH_PROFILE_L2_VLAN)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
    }
    if (cfg_value & GLB_HASH_PROFILE_L2_DST_MAC)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_MI);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_L2_SRC_MAC)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_MI);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_HI);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
    
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_ip_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4;
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_IP_PROTOCOL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_DST_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_DST_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_TCP_UDP;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
    
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_ipv6_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6;
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_IP_PROTOCOL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_DST_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_DST_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_IP_SRC_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_TCP_UDP;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_gre_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_GRE;
    
    if (cfg_value & GLB_HASH_PROFILE_GRE_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_GRE_PROTOCOL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_GRE_PROTOCOL);
    }
    if (cfg_value & GLB_HASH_PROFILE_GRE_SRC_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_GRE_DST_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_GRE_KEY)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_GRE_KEY_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_GRE_KEY_HI);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_GRE;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_vxlan_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_VXLAN;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_VLAN)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_DST_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_SRC_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_DST_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_OUTER_SRC_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_VNI)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        if (parser_cfg.value)
        {
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
            
            parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_VXLAN;
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        }
    }
    else if (cfg_value & GLB_HASH_PROFILE_VXLAN_L2IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_VXLAN_INNER_L2;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_ETH_TYPE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_ETHER_TYPE);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_DST_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_SRC_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_VNI)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_VXLAN_USE_INNER_L2;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    else if (cfg_value & GLB_HASH_PROFILE_VXLAN_L3IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_IP_PROTOCOL)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_DST_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_SRC_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_DST_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_INNER_SRC_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_VNI)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }

        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_select = CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_VXLAN_USE_INNER_L2;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    else
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_VXLAN;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_VXLAN_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VXLAN_VNI)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        if (parser_cfg.value)
        {
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
            
            parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_VXLAN;
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        }
    }
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_nvgre_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_NVGRE_AWARE;
    parser_cfg.value = 1;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    if (cfg_value & GLB_HASH_PROFILE_NVGRE_OUT_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_NVGRE;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_OUTER_GRE_PROTOCOL)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_GRE_PROTOCOL);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_OUTER_DST_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_OUTER_SRC_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_VSID)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        if (parser_cfg.value)
        {
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
            
            parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_NVGRE;
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        }
    }
    else if (cfg_value & GLB_HASH_PROFILE_VXLAN_L2IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_NVGRE_INNER_L2;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_ETH_TYPE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_ETHER_TYPE);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_DST_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_SRC_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_VSID)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_NVGRE_USE_INNER_L2;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    else if (cfg_value & GLB_HASH_PROFILE_VXLAN_L3IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_IP_PROTOCOL)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_DST_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_INNER_SRC_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_VSID)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }

        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_select = CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_NVGRE_USE_INNER_L2;
        parser_cfg.value = 0;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    else
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4;
        parser_cfg.value = 1;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_NVGRE;
        parser_cfg.value = 0;

        if (cfg_value & GLB_HASH_PROFILE_NVGRE_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_NVGRE_VSID)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_VNI_HI);
        }
        if (parser_cfg.value)
        {
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
            
            parser_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_NVGRE;
            CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        }
    }
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}


sai_status_t
ctc_sai_hash_field_profile_mpls_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2;
    parser_cfg.value = 0;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_MPLS_LABEL2_EN;
    parser_cfg.value = 1;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS;
    parser_cfg.value = 0;

    if (cfg_value & GLB_HASH_PROFILE_MPLS_TOP_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL0_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL0_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_MPLS_2ND_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL1_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL1_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_MPLS_3RD_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL2_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL2_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_MPLS_DST_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_MPLS_SRC_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_MPLS_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_INNER_IPV4;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_INNER_IPV6;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}


sai_status_t
ctc_sai_hash_field_profile_vpws_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2;
    parser_cfg.value = 0;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_VPWS_USE_INNER;
    parser_cfg.value = 1;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_VPWS_RAW;
    parser_cfg.value = 0;

    if (cfg_value & GLB_HASH_PROFILE_VPWS_TOP_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL0_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL0_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_VPWS_2ND_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL1_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL1_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_VPWS_3RD_LABEL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL2_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_LABEL2_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_VPWS_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_vpls_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2;
    parser_cfg.value = 0;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    if (cfg_value & GLB_HASH_PROFILE_VPLS_L2IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L2_ETH_TYPE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_ETHER_TYPE);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L2_VLAN)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L2_DST_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACDA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L2_SRC_MAC)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_MI);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_MACSA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    if (cfg_value & GLB_HASH_PROFILE_VPLS_L3IN_BIT)
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L3_IP_PROTOCOL)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L3_DST_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L3_SRC_PORT)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L3_DST_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_L3_SRC_IP)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
        }
        if (cfg_value & GLB_HASH_PROFILE_VPLS_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    if (((cfg_value & GLB_HASH_PROFILE_VPLS_L2IN_BIT) == 0) && 
        ((cfg_value & GLB_HASH_PROFILE_VPLS_L3IN_BIT) == 0))
    {
        parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_VPLS_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4;
        parser_cfg.value = 0;
        if (cfg_value & GLB_HASH_PROFILE_VPLS_SRC_INTERFACE)
        {
            GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
        
        parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6;
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    }
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}


sai_status_t
ctc_sai_hash_field_profile_l3vpn_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2;
    parser_cfg.value = 0;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4;
    parser_cfg.value = 0;

    if (cfg_value & GLB_HASH_PROFILE_L3VPN_IP_PROTOCOL)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
    }
    if (cfg_value & GLB_HASH_PROFILE_L3VPN_DST_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_L3VPN_SRC_PORT)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
    }
    if (cfg_value & GLB_HASH_PROFILE_L3VPN_DST_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_L3VPN_SRC_IP)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    }
    if (cfg_value & GLB_HASH_PROFILE_L3VPN_SRC_INTERFACE)
    {
        GLB_SET_FLAG(parser_cfg.value, CTC_LB_HASH_FIELD_SRC_PORT);
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}


sai_status_t
ctc_sai_hash_field_profile_mode_symmetry_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_IPV4_SYMMETRIC_EN;
    parser_cfg.value = cfg_value;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_IPV6_SYMMETRIC_EN;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_FCOE_SYMMETRIC_EN;
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}


sai_status_t
ctc_sai_hash_field_profile_hash_arithmetic_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.sel_id = profile_id;
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_HASH_TYPE_A;
    if (cfg_value & GLB_HASH_PROFILE_HASH_ARITHMETIC_A_CRC16_1)
    {
        parser_cfg.value = CTC_LB_HASH_TYPE_CRC16_POLY1;
    }
    else if (cfg_value & GLB_HASH_PROFILE_HASH_ARITHMETIC_A_XOR16)
    {
        parser_cfg.value = CTC_LB_HASH_TYPE_XOR16;
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_HASH_TYPE_B;
    parser_cfg.value = 0;
    if (cfg_value & GLB_HASH_PROFILE_HASH_ARITHMETIC_B_CRC16_1)
    {
        parser_cfg.value = CTC_LB_HASH_TYPE_CRC16_POLY1;
    }
    else if (cfg_value & GLB_HASH_PROFILE_HASH_ARITHMETIC_B_XOR16)
    {
        parser_cfg.value = CTC_LB_HASH_TYPE_XOR16;
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_field_profile_bitmap_disable_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_config_t parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));
    
    parser_cfg.sel_id = profile_id;
    /*ip disable*/
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_IPV4;
    parser_cfg.value = 0;
    if (cfg_value & GLB_HASH_PROFILE_IP_DISABLE)
    {
        parser_cfg.value = 1;
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    /*ipv6 disable*/
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_IPV6;
    parser_cfg.value = 0;
    if (cfg_value & GLB_HASH_PROFILE_IPV6_DISABLE)
    {
        parser_cfg.value = 1;
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);

    /*mpls disable*/
    parser_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    parser_cfg.hash_control = CTC_LB_HASH_CONTROL_DISABLE_MPLS;
    parser_cfg.value = 0;
    if (cfg_value & GLB_HASH_PROFILE_MPLS_DISABLE)
    {
        parser_cfg.value = 1;
    }
    CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY, &parser_cfg), ret, out);
    
    ctc_sai_hash_field_profile_disable_nosupport_tunnel(profile_id);
out:
    return ret;
}

sai_status_t
ctc_sai_hash_value_profile_agg_static_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_agg_dynamic_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_ecmp_static_l3_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_IP;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_ecmp_static_nvgre_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_NVGRE;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_ecmp_static_vxlan_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_VXLAN;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_ecmp_dynamic_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_DLB_MEMBER;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_profile_ecmp_dynamic_flow_id_set(sai_uint32_t profile_id, uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = profile_id;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
    parser_cfg.offset = cfg_value;
    parser_cfg.use_packet_head_hash = 0;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_agg_set(uint32 cfg_value)
{
    g_hash_value_global_agg_hash = cfg_value;
    return SAI_STATUS_SUCCESS;
        
}

sai_status_t
ctc_sai_hash_value_global_ecmp_static_l3_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = CTC_LB_HASH_ECMP_GLOBAL_PROFILE;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_IP;
    parser_cfg.offset = cfg_value;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_ecmp_static_nvgre_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = CTC_LB_HASH_ECMP_GLOBAL_PROFILE;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_NVGRE;
    parser_cfg.offset = cfg_value;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_ecmp_static_vxlan_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = CTC_LB_HASH_ECMP_GLOBAL_PROFILE;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_STATIC;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_VXLAN;
    parser_cfg.offset = cfg_value;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_ecmp_dynamic_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = CTC_LB_HASH_ECMP_GLOBAL_PROFILE;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_DLB_MEMBER;
    parser_cfg.offset = cfg_value;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_ecmp_dynamic_flow_id_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = CTC_LB_HASH_ECMP_GLOBAL_PROFILE;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_ECMP;
    parser_cfg.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
    parser_cfg.offset = cfg_value;
    
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_hash_value_global_lbid_set(uint32 cfg_value)
{
    int32_t  ret = 0;
    ctc_lb_hash_offset_t   parser_cfg;
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));

    parser_cfg.profile_id = 0;
    parser_cfg.hash_module = CTC_LB_HASH_MODULE_HEAD_LAG;
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_UC;
    parser_cfg.use_entropy_hash = 0;
    parser_cfg.use_packet_head_hash = 0;
    parser_cfg.offset = cfg_value;
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    if (ret)
    {
        return ret;
    }
    parser_cfg.fwd_type = CTC_LB_HASH_FWD_NON_UC;
    ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &parser_cfg);
    return ctc_sai_get_error_from_sdk_error(ret);
}
#endif

sai_status_t
ctc_sai_set_hash_profile_attribute(
    _In_ sai_uint32_t profile_id,
    _In_ const sai_attribute_t *attr)
{
    int32_t  ret = 0;
#ifdef TSINGMA
    uint32 cfg_value = 0;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);
    cfg_value = attr->value.s32;
    switch (attr->id)
    {
     /*start with hash field profile process*/
     case SAI_NATIVE_HASH_FIELD_PROFILE_L2_SET:
        ret = ctc_sai_hash_field_profile_l2_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_IP_SET:
        ret = ctc_sai_hash_field_profile_ip_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_IPV6_SET:
        ret = ctc_sai_hash_field_profile_ipv6_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_GRE_SET:
        ret = ctc_sai_hash_field_profile_gre_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_VXLAN_SET:
        ret = ctc_sai_hash_field_profile_vxlan_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_NVGRE_SET:
        ret = ctc_sai_hash_field_profile_nvgre_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_MPLS_SET:
        ret = ctc_sai_hash_field_profile_mpls_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_VPWS_SET:
        ret = ctc_sai_hash_field_profile_vpws_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_VPLS_SET:
        ret = ctc_sai_hash_field_profile_vpls_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_L3VPN_SET:
        ret = ctc_sai_hash_field_profile_l3vpn_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_MODE_SYMMETRY:
        ret = ctc_sai_hash_field_profile_mode_symmetry_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_HASH_ARITHMETIC:
        ret = ctc_sai_hash_field_profile_hash_arithmetic_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_FIELD_PROFILE_BITMAP_DISABLE:
        ret = ctc_sai_hash_field_profile_bitmap_disable_set(profile_id, cfg_value);
        break;
     /*end with hash field profile*/

     /*start with hash value profile process*/
     case SAI_NATIVE_HASH_VALUE_PROFILE_AGG_STATIC:
        ret = ctc_sai_hash_value_profile_agg_static_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_PROFILE_AGG_DYNAMIC:
        ret = ctc_sai_hash_value_profile_agg_dynamic_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_L3:
        ret = ctc_sai_hash_value_profile_ecmp_static_l3_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE:
        ret = ctc_sai_hash_value_profile_ecmp_static_nvgre_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN:
        ret = ctc_sai_hash_value_profile_ecmp_static_vxlan_set(profile_id, cfg_value);
        break; 
     case SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC:
        ret = ctc_sai_hash_value_profile_ecmp_dynamic_set(profile_id, cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID:
        ret = ctc_sai_hash_value_profile_ecmp_dynamic_flow_id_set(profile_id, cfg_value);
        break;
     /*end with hash value profile*/

     /*start with hash value global process*/
     case SAI_NATIVE_HASH_VALUE_GLOBAL_AGG:
        ret = ctc_sai_hash_value_global_agg_set(cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_L3:
        ret = ctc_sai_hash_value_global_ecmp_static_l3_set(cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE:
        ret = ctc_sai_hash_value_global_ecmp_static_nvgre_set(cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN:
        ret = ctc_sai_hash_value_global_ecmp_static_vxlan_set(cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_DYNAMIC:
        ret = ctc_sai_hash_value_global_ecmp_dynamic_set(cfg_value);
        break;
     case SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID:
        ret = ctc_sai_hash_value_global_ecmp_dynamic_flow_id_set(cfg_value);
        break; 
     case SAI_NATIVE_HASH_VALUE_GLOBAL_LBID:
        ret = ctc_sai_hash_value_global_lbid_set(cfg_value);
        break;
     /*end with hash value global*/
     default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
#endif
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_create_hash_profile(
    _In_ sai_uint32_t profile_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    int32_t  ret = 0;
#ifdef TSINGMA
    uint32_t          attr_idx = 0;
    sai_attribute_t*  attr = NULL;
    ctc_lb_hash_config_t parser_cfg;
    uint32 cfg_value = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));
    parser_cfg.sel_id = profile_id;
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch (attr->id)
        {
         case SAI_NATIVE_HASH_FIELD_PROFILE_L2_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_l2_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_IP_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_ip_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_IPV6_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_ipv6_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_GRE_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_gre_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_VXLAN_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_vxlan_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_NVGRE_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_nvgre_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_MPLS_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_mpls_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_VPWS_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_vpws_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_VPLS_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_vpls_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_L3VPN_SET:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_l3vpn_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_MODE_SYMMETRY:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_mode_symmetry_set(profile_id, cfg_value);
            break;
         case SAI_NATIVE_HASH_FIELD_PROFILE_HASH_ARITHMETIC:
            cfg_value = attr->value.s32;
            ret = ctc_sai_hash_field_profile_hash_arithmetic_set(profile_id, cfg_value);
            break;
         default:
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
#endif
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t
ctc_sai_get_hash_attribute(
    _In_ sai_object_id_t hash_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_INNER_FUNC
static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_hash_db_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    preg->init_status =  INITIALIZED;
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.3 */
static sai_hash_api_t      g_sai_api_func = {
    .create_hash            = ctc_sai_create_hash,
    .remove_hash            = ctc_sai_remove_hash,
    .create_hash_profile    = ctc_sai_create_hash_profile,
    .set_hash_attribute     = ctc_sai_set_hash_attribute,
    .set_hash_profile_attribute  = ctc_sai_set_hash_profile_attribute,
    .get_hash_attribute     = ctc_sai_get_hash_attribute,
};
static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id        = SAI_API_HASH,
        .init_func = __init_mode_fn,
        .exit_func = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_FDB_OUTER_FUNC
sai_status_t
ctc_sai_hash_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}


