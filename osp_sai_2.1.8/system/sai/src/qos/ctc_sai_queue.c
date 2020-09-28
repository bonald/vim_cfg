#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_queue.h>
#include <ctc_sai_debug.h>


ctc_sai_queue_info_t g_sai_queue_info;

extern sai_status_t ctc_sai_wred_queue_set_wred(sai_object_id_t queue_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_scheduler_queue_set_scheduler(sai_object_id_t queue_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_wred_queue_get_wred(sai_object_id_t queue_id,
                                const sai_attribute_t *attr);

extern sai_status_t ctc_sai_scheduler_queue_get_scheduler(_In_ sai_object_id_t queue_id,
                                    _Inout_  sai_attribute_t *attr);

ctc_sai_queue_hash_t*
ctc_sai_queue_hash_get_by_gport(uint32 gport)
{
    ctc_sai_queue_hash_t queue_hash_info;

    sal_memset(&queue_hash_info, 0, sizeof(queue_hash_info));
    queue_hash_info.gport = gport;
    return ctc_hash_lookup(g_sai_queue_info.ctc_sai_queue_info_hash, &queue_hash_info);
}
ctc_sai_queue_hash_t*
ctc_sai_queue_hash_add_hash(uint32 gport)
{
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;

    p_queue_hash_info = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_queue_hash_t));
    if (NULL == p_queue_hash_info)
    {
        return NULL;
    }
    p_queue_hash_info->gport = gport;
    sal_memset(p_queue_hash_info->queue_entries, 0x0, sizeof(p_queue_hash_info->queue_entries));
    return ctc_hash_insert(g_sai_queue_info.ctc_sai_queue_info_hash, p_queue_hash_info);
}

static uint32_t
_queue_hash_make(
    _In_  void* data)
{
    ctc_sai_queue_hash_t* p_queue_hash_info = (ctc_sai_queue_hash_t*)data;

    return ctc_hash_caculate(sizeof(uint32), &p_queue_hash_info->gport);
}

static bool
_queue_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_queue_hash_t* p_queue_hash_info_key = data;
    ctc_sai_queue_hash_t* p_queue_hash_info_key1 = data1;

    if ((p_queue_hash_info_key->gport) == (p_queue_hash_info_key1->gport))
    {
        return TRUE;
    }

    return FALSE;
}

sai_status_t ctc_sai_queue_db_init()
{
    g_sai_queue_info.ctc_sai_queue_info_hash = ctc_hash_create(64, 32, _queue_hash_make, _queue_hash_cmp);

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_QUEUE_DEBUG_FUNC
sai_status_t sai_set_queue_attribute_debug_param(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
    )
{
    CTC_SAI_DEBUG("in:queue_id 0x%llx", queue_id);
    switch(attr->id)
    {
    case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
        CTC_SAI_DEBUG("in:SAI_QUEUE_ATTR_WRED_PROFILE_ID %u", attr->value.u32);
        break;

    case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
        CTC_SAI_DEBUG("in:SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID %u", attr->value.u32);
        break;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_queue_attribute_debug_param(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t        attr_count,
    _Inout_ sai_attribute_t *attr
    )
{
    CTC_SAI_DEBUG("out:queue_id 0x%llx attr_count %u", queue_id, attr_count);
    switch(attr->id)
    {
    case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
        CTC_SAI_DEBUG("out:SAI_QUEUE_ATTR_WRED_PROFILE_ID %u", attr->value.u32);
        break;

    case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
        CTC_SAI_DEBUG("out:SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID %u", attr->value.u32);
        break;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_queue_statistics_debug_param(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_queue_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    )
{
    uint32_t         attr_idx    = 0;
    CTC_SAI_DEBUG("in:queue_id 0x%llx number_of_counters %u", queue_id, number_of_counters);

    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
            case SAI_QUEUE_STAT_PACKETS:
                CTC_SAI_DEBUG("out:SAI_QUEUE_STAT_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_QUEUE_STAT_BYTES:
                CTC_SAI_DEBUG("out:SAI_QUEUE_STAT_BYTES %"PRIu64, counters[attr_idx]);
                break;
            case SAI_QUEUE_STAT_DROPPED_PACKETS:
                CTC_SAI_DEBUG("out:SAI_QUEUE_STAT_DROPPED_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_QUEUE_STAT_DROPPED_BYTES:
                CTC_SAI_DEBUG("out:SAI_QUEUE_STAT_DROPPED_BYTES %"PRIu64, counters[attr_idx]);
                break;
            default:
                CTC_SAI_DEBUG("out:NOT SUPPORT");
                break;
        }
    }
    return SAI_STATUS_SUCCESS;
}
#define ________SAI_QUEUE_API_FUNC
sai_status_t sai_set_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    sai_set_queue_attribute_debug_param(queue_id, attr);

    CTC_SAI_PTR_VALID_CHECK(attr);

    switch(attr->id)
    {
        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
        case SAI_QUEUE_ATTR_DROP_TYPE:
                ret = ctc_sai_wred_queue_set_wred(queue_id, attr);
            break;
        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                ret = ctc_sai_scheduler_queue_set_scheduler(queue_id, attr);
            break;   
        default :
            break;
    }

    return ret;
}

sai_status_t sai_get_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t        attr_count,
    _Inout_ sai_attribute_t *attr
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    switch(attr->id)
    {
        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                ret = ctc_sai_wred_queue_get_wred(queue_id, attr);
            break;
        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                ret = ctc_sai_scheduler_queue_get_scheduler(queue_id, attr);
            break;   
        default :
            break;
    }

    sai_get_queue_attribute_debug_param(queue_id, attr_count, attr);
    return ret;
}

sai_status_t sai_get_queue_statistics(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_queue_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    uint32_t         attr_idx    = 0;
    ctc_qos_queue_stats_t queue_stats;
    uint32_t  sai_queue_id =0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port_id = 0;
    
    CTC_SAI_DEBUG_FUNC();

    sal_memset(&queue_stats, 0x0, sizeof(ctc_qos_queue_stats_t));
    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_port_id = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);


    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_stats.queue.queue_id = ctc_queue_id;
    queue_stats.queue.gport = ctc_port_id;
    
    ret = ctc_qos_query_queue_stats(&queue_stats);
    if(ret)
    {
        return ret;
    }
    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
            case SAI_QUEUE_STAT_PACKETS:
                counters[attr_idx] = queue_stats.stats.deq_packets + queue_stats.stats.drop_packets;
                //counters[attr_idx] = 10;
                break;
            case SAI_QUEUE_STAT_BYTES:
                counters[attr_idx] = queue_stats.stats.deq_bytes + queue_stats.stats.drop_bytes;
                //counters[attr_idx] = 100;
                break;
            case SAI_QUEUE_STAT_DROPPED_PACKETS:
                counters[attr_idx] = queue_stats.stats.drop_packets;
                //counters[attr_idx] = 5;
                break;
            case SAI_QUEUE_STAT_DROPPED_BYTES:
                counters[attr_idx] = queue_stats.stats.drop_bytes;
                //counters[attr_idx] = 50;
                break;
            default:
                ret = SAI_STATUS_NOT_SUPPORTED;
                break;
        }
    }
    sai_get_queue_statistics_debug_param(queue_id, counter_ids, number_of_counters, counters);
    return ret;
}

sai_status_t sai_clear_queue_statistics(
    _In_ sai_object_id_t queue_id
    )
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32_t  sai_queue_id =0;
    uint32_t  ctc_port_id = 0;
    uint32_t  ctc_queue_id = 0;
    ctc_qos_queue_stats_t queue_stats;

    CTC_SAI_DEBUG_FUNC();

    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_port_id = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);

    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    queue_stats.queue.queue_id = ctc_queue_id;
    queue_stats.queue.gport = ctc_port_id;

    ret = ctc_qos_clear_queue_stats(&queue_stats);

    return ret;
}

#define ________SAI_QUEUE_INNER_FUNC
sai_status_t __queue_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_queue_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __queue_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_queue_api_t      g_sai_queue_api_func = {
    .set_queue_attribute    = sai_set_queue_attribute,
    .get_queue_attribute    = sai_get_queue_attribute,
    .get_queue_stats        = sai_get_queue_statistics,
/* SYSTEM MODIFIED BEGIN: add clear qos stats interface by wangqj at 2016/8/8 */
    .clear_queue_stats      = sai_clear_queue_statistics,
/* END: add clear qos stats interface by wangqj at 2016/8/8 */
};

static ctc_sai_api_reg_info_t g_queue_api_reg_info = {
        .id  = SAI_API_QUEUE,
        .init_func = __queue_init_mode_fn,
        .exit_func = __queue_exit_mode_fn,
        .api_method_table = &g_sai_queue_api_func,
        .private_data     = NULL,
};

#define ________SAI_QUEUE_OUTER_FUNC
sai_status_t ctc_sai_queue_init()
{
    api_reg_register_fn(&g_queue_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
