#include "sal.h"
#include "ctc_api.h"
#include "sal_error.h"
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "ctc_sai_sai.h"
#include "ctc_sai_common.h"
#include "ctc_sai_common_attr.h"
#include "ctc_sai_port.h"
#include "ctc_sai_isolation.h"
#include <ctc_sai_debug.h>

static ctc_sai_isolation_info_t g_ctc_sai_isolation_info;

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_ISOLATION_ATTR_EGRESS_BLOCK_PORT_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ISOLATION_ATTR_INRESS_BLOCK_PORT_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ISOLATION_ATTR_DROP_PACKET_TYPE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ISOLATION_ATTR_MODE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};
#define ________SAI_ISOLATION_DEBUG_FUNC
sai_status_t
ctc_sai_create_isolation_group_debug_param(
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list)
{
    const sai_attribute_t *pattr_entry = NULL;
    uint32_t                    attr_idx = 0;

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = &attr_list[attr_idx];
        switch (pattr_entry->id)
        {
        case SAI_ISOLATION_ATTR_DROP_PACKET_TYPE:
            CTC_SAI_DEBUG("in:SAI_ACL_ENTRY_ATTR_TABLE_ID %d", pattr_entry->value.s32);
            break;
        case SAI_ISOLATION_ATTR_MODE:
            CTC_SAI_DEBUG("in:SAI_ISOLATION_ATTR_MODE %d", pattr_entry->value.s32);
            break;

         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
#define ________SAI_ISOLATION_INNER_FUNC

sai_status_t
ctc_sai_isolation_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_isolation_info_t *p_info = &g_ctc_sai_isolation_info;
    p_info->max_count = CTC_SAI_ISOLATION_MAX;
    p_info->first_alloc = TRUE;

    if (0 != ctclib_opb_create(&p_sai_glb->isolation_id_opb, 1, p_info->max_count, "isolation_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_create_isolation_group(
        _Out_ sai_object_id_t *group_id,
        _In_  uint32_t attr_count,
        _In_  const sai_attribute_t *attr_list)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_isolation_info_t *p_info = &g_ctc_sai_isolation_info;
    ctc_sai_isolation_group_t *p_db_group = NULL;
    sai_isolaton_group_drop_pkt_type_t type = SAI_ISOLATION_GROUP_DROP_PACKET_TYPE_ALL;
    sai_isolaton_group_mode_t mode = SAI_ISOLATION_GROUP_MODE_ISOLATION;
    uint32_t opf_index = 0;
    uint32_t attr_idx = 0; 
    sai_attribute_t*  attr = NULL;    
    int32_t sdk_ret = CTC_E_NONE;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_create_isolation_group_debug_param(attr_count, attr_list);

    if (p_info->first_alloc)
    {
        if (0 != ctclib_opb_alloc_offset_last_reverse(&p_sai_glb->isolation_id_opb, &opf_index))
        {
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
        p_info->first_alloc = FALSE;
    }
    else
    {
        if (0 != ctclib_opb_alloc_offset(&p_sai_glb->isolation_id_opb, &opf_index))
        {
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_ISOLATION_ATTR_DROP_PACKET_TYPE:
            type = attr->value.s32;
            break;
            
        case SAI_ISOLATION_ATTR_MODE:
            mode = attr->value.s32;
            break;

        default: /*not support other field now*/
            break;
        }
    }

    p_db_group = &p_info->group[opf_index];
    p_db_group->isolated_id = opf_index;
    p_db_group->isolated_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_ISOLATION_GROUP, p_db_group->isolated_id);
    p_db_group->type = type;
    p_db_group->mode = mode;

    *group_id = p_db_group->isolated_oid;
    CTC_SAI_DEBUG("out:group_id %d", (*group_id));

    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    return ret;
}

sai_status_t
ctc_sai_remove_isolation_group(
        _In_ sai_object_id_t group_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_isolation_info_t *p_info = &g_ctc_sai_isolation_info;
    ctc_sai_isolation_group_t *p_db_group = NULL;
    uint32_t opf_index = 0;
    int32_t sdk_ret = CTC_E_NONE;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:group_id 0x%llx", group_id);

    if (SAI_OBJECT_TYPE_ISOLATION_GROUP != CTC_SAI_OBJECT_TYPE_GET(group_id))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    opf_index = CTC_SAI_OBJECT_INDEX_GET(group_id);

    if (0 != ctclib_opb_free_offset(&p_sai_glb->isolation_id_opb, opf_index))
    {
       return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_db_group = &p_info->group[opf_index];
    p_db_group->isolated_id = 0;
    p_db_group->isolated_oid = 0;
    p_db_group->type = 0;

    ret = ctc_sai_get_error_from_sdk_error(sdk_ret);
    return ret;
}

ctc_sai_isolation_group_t*
ctc_sai_get_isolation_group(sai_object_id_t isolation_oid)
{
    ctc_sai_isolation_info_t *p_info = &g_ctc_sai_isolation_info;
    ctc_sai_isolation_group_t *p_db_group = NULL;
    uint32 isolation_id = 0;
    
    if (SAI_OBJECT_TYPE_ISOLATION_GROUP != CTC_SAI_OBJECT_TYPE_GET(isolation_oid))
    {
        return NULL;
    }

    isolation_id = CTC_SAI_OBJECT_INDEX_GET(isolation_oid);
    if (CTC_SAI_ISOLATION_MAX < isolation_id)
    {
        return NULL;
    }

    p_db_group = &p_info->group[isolation_id];
    if (0 == p_db_group->isolated_oid)
    {
        return NULL;
    }

    return p_db_group;
}

sai_status_t
ctc_sai_set_isolation_group_attribute(
        _In_ sai_object_id_t group_id,
        _In_ const  sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();

    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;
}

sai_status_t
ctc_sai_get_isolation_group_attribute(
        _In_ sai_object_id_t group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    uint32_t attr_idx    = 0;
    sai_attribute_t* attr        = NULL;
    ctc_sai_isolation_group_t* p_group = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:group_id 0x%llx, attr_count %u", group_id, attr_count);

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    p_group = ctc_sai_get_isolation_group(group_id);
    if (NULL == p_group)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries, attr);
        if (ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_ISOLATION_ATTR_DROP_PACKET_TYPE:
            attr->value.s32 = p_group->type;
            break;
        case SAI_ISOLATION_ATTR_MODE:
            attr->value.s32 = p_group->mode;
            break;
        default:
            ret = SAI_STATUS_NOT_IMPLEMENTED;
            break;
        }
    }
    
    return ret;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    ctc_sai_isolation_db_init();
    preg->init_status = INITIALIZED;

    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    
    preg->init_status = UNINITIALIZED;

    return ret;
}

/* define sai  */
static sai_isolation_api_t g_sai_api_func = {
    .create_isolation_group            = ctc_sai_create_isolation_group,
    .remove_isolation_group            = ctc_sai_remove_isolation_group,
    .set_isolation_group_attribute     = ctc_sai_set_isolation_group_attribute,
    .get_isolation_group_attribute     = ctc_sai_get_isolation_group_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_ISOLATION,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_ISOLATION_OUTER_FUNC

sai_status_t
ctc_sai_isolation_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

