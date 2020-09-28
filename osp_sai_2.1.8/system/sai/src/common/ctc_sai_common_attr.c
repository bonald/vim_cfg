#include "ctc_sai_common_attr.h"
#include "ctc_sai_common.h"
#include "stdio.h"


ctc_sai_check_u32_range_t g_packet_action_range =
{
    .min = SAI_PACKET_ACTION_DROP,
    .max = SAI_PACKET_ACTION_LOG,
};


const sai_attribute_t const*
ctc_sai_attr_get_attr_by_id(    const sai_attr_id_t id,
                                    const sai_attribute_t *attr_list,
                                    uint32_t attr_count,
                                    uint32_t *pattr_index)
{
    uint32_t    i = 0;

    for(i = 0; i < attr_count; i++)
    {
        if(id == attr_list[i].id)
        {
            if(pattr_index)
            {
                *pattr_index = i;
            }
            return attr_list + i;
        }
    }

    return NULL;
}

sai_status_t
ctc_sai_attr_alloc_attr_entry_list(const ctc_sai_attr_entry_info_t const *attr_info_entries,
                                 const sai_attribute_t *attr_list,
                                 uint32_t attr_count,
                                 _Out_ ctc_sai_attr_entry_list_t **pattr_entry_list)
{
    uint32_t attr_info_count = 0;
    uint32_t attr_idx        = 0;
    ctc_sai_attr_entry_list_t *psai_attr_entry_list = NULL;


    CTC_SAI_PTR_VALID_CHECK(attr_info_entries);
    CTC_SAI_PTR_VALID_CHECK(pattr_entry_list);

    while(attr_info_entries[attr_info_count].id != SAI_ATTR_ID_END)
        attr_info_count++;

    if(0 == attr_info_count)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    psai_attr_entry_list = XCALLOC(MEM_FEA_TEMP_TODO,sizeof(ctc_sai_attr_entry_list_t) * attr_info_count);
    *pattr_entry_list = psai_attr_entry_list;

    if(!psai_attr_entry_list)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    sal_memset(psai_attr_entry_list,0,sizeof(ctc_sai_attr_entry_list_t) * attr_info_count);

    for(attr_idx = 0; attr_list && attr_idx < attr_count; attr_idx++)
    {
        if(attr_list[attr_idx].id >= attr_info_count)
        {
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_idx;
        }

        psai_attr_entry_list[attr_list[attr_idx].id].attr_index = attr_idx;
        psai_attr_entry_list[attr_list[attr_idx].id].value = attr_list[attr_idx].value;
        psai_attr_entry_list[attr_list[attr_idx].id].valid = true;
    }

    attr_idx = 0;
    while(attr_info_entries[attr_idx].id != SAI_ATTR_ID_END)
    {
        if(SAI_ATTR_FLAG_DEFAULT == (SAI_ATTR_FLAG_DEFAULT &attr_info_entries[attr_idx].type))
        {
            if(false == psai_attr_entry_list[attr_idx].valid)
            {
                psai_attr_entry_list[attr_idx].value =
                    attr_info_entries[attr_idx].default_value;
                psai_attr_entry_list[attr_idx].valid = true;
            }
        }
        attr_idx++;
    }

    *pattr_entry_list = psai_attr_entry_list;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_attr_check_attr_entry_list(const ctc_sai_attr_entry_info_t const *attr_info_entries,
                                            ctc_sai_attr_entry_list_t *pattr_entry_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    CTC_SAI_PTR_VALID_CHECK(attr_info_entries);
    CTC_SAI_PTR_VALID_CHECK(pattr_entry_list);

    int32_t    i = 0;
    for(i = 0; attr_info_entries[i].id != SAI_ATTR_ID_END; i++)
    {
        if(SAI_ATTR_CHECK_CREATE == (attr_info_entries[i].type & SAI_ATTR_CHECK_CREATE))
        {
            if(!pattr_entry_list[i].valid)
            {
                return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            }

            if(attr_info_entries[i].check_fn.func_fn)
            {
                ret = attr_info_entries[i].check_fn.func_fn(&pattr_entry_list[i],
                                         attr_info_entries[i].check_fn.func_parameter);
            }

            if(SAI_STATUS_SUCCESS != ret)
            {
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr_entry_list[i].attr_index;
            }
        }

        if(SAI_ATTR_CHECK_WRITE == (attr_info_entries[i].type & SAI_ATTR_CHECK_WRITE)
            && pattr_entry_list[i].valid)
        {
            if(attr_info_entries[i].check_fn.func_fn)
            {
                ret = attr_info_entries[i].check_fn.func_fn(&pattr_entry_list[i],
                                     attr_info_entries[i].check_fn.func_parameter);
            }

            if(SAI_STATUS_SUCCESS != ret)
            {
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr_entry_list[i].attr_index;
            }
        }

        if(SAI_ATTR_CHECK_READ == attr_info_entries[i].type
            && pattr_entry_list[i].valid)
        {
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + pattr_entry_list[i].attr_index;
        }
    }
    return ret;
}

sai_status_t
ctc_sai_attr_check_read_attr(const ctc_sai_attr_entry_info_t const *attr_info_entries,
                                 const sai_attribute_t *attr)
{
    uint32_t    attr_info_count = 0;

    CTC_SAI_PTR_VALID_CHECK(attr_info_entries);
    CTC_SAI_PTR_VALID_CHECK(attr);

    while(attr_info_entries[attr_info_count].id != SAI_ATTR_ID_END)
        attr_info_count++;

    if(attr->id >= attr_info_count)
        return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    if(attr_info_entries[attr->id].type && SAI_ATTR_CHECK_READ == SAI_ATTR_CHECK_READ)
    {
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

sai_status_t
ctc_sai_attr_check_write_attr(const ctc_sai_attr_entry_info_t const *attr_info_entries,
                                 const sai_attribute_t *attr)
{
    uint32_t    attr_info_count = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   attr_entry;
    return SAI_STATUS_SUCCESS;

    CTC_SAI_PTR_VALID_CHECK(attr_info_entries);
    CTC_SAI_PTR_VALID_CHECK(attr);

    sal_memset(&attr_entry,0,sizeof(attr_entry));

    while(attr_info_entries[attr_info_count].id != SAI_ATTR_ID_END)
        attr_info_count++;

    if(attr->id >= attr_info_count)
        return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    if(attr_info_entries[attr->id].type && SAI_ATTR_CHECK_WRITE != SAI_ATTR_CHECK_WRITE)
    {
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    attr_entry.value = attr->value;
    attr_entry.attr_index = 0;

    if(attr_info_entries[attr->id].check_fn.func_fn)
    {
        ret = attr_info_entries[attr->id].check_fn.func_fn(&attr_entry,
                                     attr_info_entries[attr->id].check_fn.func_parameter);
    }

    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_check_i32_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data)
{
    ctc_sai_check_u32_range_t   *prange = data;

    if (pattr->value.s32 < prange->min || pattr->value.s32 > prange->max)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr->attr_index;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_check_u32_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data)
{
    ctc_sai_check_u32_range_t   *prange = data;

    if (pattr->value.u32 < prange->min || pattr->value.u32 > prange->max)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr->attr_index;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_check_object_type_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data)
{
    ctc_sai_check_object_type_range_t   *prange = data;
    if(CTC_SAI_OBJECT_TYPE_GET(pattr->value.oid) < prange->min
       || CTC_SAI_OBJECT_TYPE_GET(pattr->value.oid) > prange->max)
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + pattr->attr_index;

    return SAI_STATUS_SUCCESS;
}

