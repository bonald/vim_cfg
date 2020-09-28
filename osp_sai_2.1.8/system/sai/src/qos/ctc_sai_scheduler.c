#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_scheduler.h>
#include <ctc_sai_queue.h>
#include <ctc_sai_debug.h>

ctc_sai_scheduler_info_t  g_sai_scheduler_info;
ctc_sai_queue_hash_t g_sai_scheduler_copp_info;

extern ctc_sai_queue_info_t g_sai_queue_info;

extern ctc_sai_queue_hash_t*
ctc_sai_queue_hash_get_by_gport(uint32 gport);

extern ctc_sai_queue_hash_t*
ctc_sai_queue_hash_add_hash(uint32 gport);

ctc_sai_scheduler_port_index_t*
ctc_sai_scheduler_port_index_get_by_gport(uint32 gport)
{
    ctc_sai_scheduler_port_index_t scheduler_port_index;

    sal_memset(&scheduler_port_index, 0, sizeof(scheduler_port_index));
    scheduler_port_index.gport = gport;
    return ctc_hash_lookup(g_sai_scheduler_info.ctc_sai_scheduler_port_index_hash, &scheduler_port_index);
}
ctc_sai_scheduler_port_index_t*
ctc_sai_scheduler_port_index_add_hash(uint32 gport)
{
    ctc_sai_scheduler_port_index_t* p_scheduler_port_index = NULL;

    p_scheduler_port_index = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_scheduler_port_index_t));
    if (NULL == p_scheduler_port_index)
    {
        return NULL;
    }
    p_scheduler_port_index->gport = gport;
    p_scheduler_port_index->ctc_sai_scheduler_port_index = 0;
    sal_memset(p_scheduler_port_index->ctc_sai_scheduler_queue_dwrr_mode, 0x0, sizeof(p_scheduler_port_index->ctc_sai_scheduler_queue_dwrr_mode));
    return ctc_hash_insert(g_sai_scheduler_info.ctc_sai_scheduler_port_index_hash, p_scheduler_port_index);
}

sai_status_t
ctc_sai_scheduler_get_port_scheduler_id(sai_object_id_t port_id, sai_attribute_t *attr)
{
    uint32_t ctc_port_id;
    ctc_sai_scheduler_port_index_t * p_scheduler_index;
    ctc_port_id = CTC_SAI_OBJECT_INDEX_GET(port_id);
    p_scheduler_index = ctc_sai_scheduler_port_index_get_by_gport(ctc_port_id);
    if(!p_scheduler_index)
    {
        return SAI_STATUS_FAILURE;
    }
    attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_scheduler_index->ctc_sai_scheduler_port_index);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_scheduler_queue_get_scheduler(_In_ sai_object_id_t queue_id,
                                    _Inout_ sai_attribute_t *attr)
{
    uint32_t  sai_queue_id = 0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port = 0;
    uint32_t  ctc_scheduler_id = 0;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;

    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_port = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);

    p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(ctc_port);
    if(!p_queue_hash_info)
    {
        return SAI_STATUS_FAILURE;
    }
    ctc_scheduler_id = p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id;
    attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, ctc_scheduler_id);

    return SAI_STATUS_SUCCESS;
}

static uint32_t
_scheduler_port_index_hash_make(
    _In_  void* data)
{
    ctc_sai_scheduler_port_index_t* p_scheduler_port_index = (ctc_sai_scheduler_port_index_t*)data;

    return ctc_hash_caculate(sizeof(uint32), &p_scheduler_port_index->gport);
}

static bool
_scheduler_port_index_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_scheduler_port_index_t* p_scheduler_port_index_key = data;
    ctc_sai_scheduler_port_index_t* p_scheduler_port_index_key1 = data1;

    if ((p_scheduler_port_index_key->gport) == (p_scheduler_port_index_key1->gport))
    {
        return TRUE;
    }

    return FALSE;
}

sai_status_t
ctc_sai_scheduler_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    p_sai_glb->qos_scheduler_max_count = 1000;
    g_sai_scheduler_info.pvector = ctc_vector_init(10,100);

    if(NULL == g_sai_scheduler_info.pvector)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    
    if (0 != ctclib_opb_create(&p_sai_glb->qos_scheduler_opb, 1, p_sai_glb->qos_scheduler_max_count, "scheduler"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_scheduler_info.counter_max = 2048; /*need check the number*/
    g_sai_scheduler_info.pcounter_vector = ctc_vector_init(32,64);

    if(NULL == g_sai_scheduler_info.pcounter_vector)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    g_sai_scheduler_info.ctc_sai_scheduler_port_index_hash = ctc_hash_create(64, 32, _scheduler_port_index_hash_make, _scheduler_port_index_hash_cmp);

    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_scheduler_map_db_attribute(const sai_attribute_t *attr, ctc_sai_scheduler_t  *pscheduler)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t   *attr_value = NULL;
    sai_attribute_t *attr_list_db = NULL;
    attr_value = ctc_vector_get(pscheduler->scheduler_vector, attr->id - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if(NULL == attr_value)
    {
        attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
        sal_memcpy(attr_list_db, attr, sizeof(sai_attribute_t));

        if(FALSE == ctc_vector_add(pscheduler->scheduler_vector, attr->id - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM, attr_list_db))
        {
            ret = SAI_STATUS_FAILURE;
            return ret;
        }
    }
    else
    {
        sal_memcpy(attr_value, attr, sizeof(sai_attribute_t));
    }

    return ret;
}

sai_status_t
ctc_sai_scheduler_db_alloc_scheduler(ctc_sai_scheduler_t** ppscheduler)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_scheduler_t *pscheduler = NULL;
    uint32_t            index     = 0;

    if(g_sai_scheduler_info.pvector->used_cnt >= p_sai_glb->qos_scheduler_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }

    pscheduler = XCALLOC(MEM_FEA_TEMP_TODO,sizeof(ctc_sai_scheduler_t));

    if(pscheduler)
    {
        sal_memset(pscheduler,0,sizeof(ctc_sai_scheduler_t));
        ctclib_opb_alloc_offset(&p_sai_glb->qos_scheduler_opb, &index);
        pscheduler->scheduler_id= index;
        pscheduler->used_cnt = 0;
    }

    *ppscheduler = pscheduler;

    return SAI_STATUS_SUCCESS;
}

void
ctc_sai_scheduler_db_release_scheduler(ctc_sai_scheduler_t *pscheduler)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_uint32_t index = 0;
    sai_attribute_t   *attr_value = NULL;
    if(NULL == pscheduler)
    {
        return;
    }
    ctclib_opb_free_offset(&p_sai_glb->qos_scheduler_opb, pscheduler->scheduler_id);

    for (index = SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM; index <= SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE; index++)
    {
        attr_value = ctc_vector_get(pscheduler->scheduler_vector, index - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
        if (attr_value)
        {
            XFREE(MEM_FEA_TEMP_TODO, attr_value);
            attr_value = NULL;
            ctc_vector_del(pscheduler->scheduler_vector, index - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
        }
    }
    ctc_vector_release(pscheduler->scheduler_vector); /* fix qos memory leak bug add by wangqj in 2016/10/25 */
    XFREE(MEM_FEA_TEMP_TODO, pscheduler);
}

sai_status_t
ctc_sai_scheduler_db_map_scheduler(ctc_sai_scheduler_t* psai_scheduler, ctc_scheduler_t* p_ctc_scheduler)
{
    sai_attribute_t   *attr_value = NULL;

    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->scheduler_mode = attr_value->value.s32;
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->scheduler_weight= attr_value->value.s32;
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_SHAPER_TYPE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        if(SAI_METER_TYPE_PACKETS == attr_value->value.s32)
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->shape_cir = (attr_value->value.u64)* 8/1000;
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->shape_cbs = (attr_value->value.u64)* 8/1000 ;
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->shape_pir = (attr_value->value.u64)* 8/1000;
    }
    attr_value = ctc_vector_get(psai_scheduler->scheduler_vector, SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
    if (attr_value)
    {
        p_ctc_scheduler->shape_pbs = (attr_value->value.u64)* 8/1000;
    }

    return SAI_STATUS_SUCCESS;
    
}

sai_status_t ctc_sai_scheduler_queue_set_scheduler(sai_object_id_t queue_id,
                                const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  sai_queue_id = 0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port = 0;
    uint32_t  ctc_scheduler_id = 0;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    ctc_qos_sched_t sched;
    ctc_qos_shape_t shape;
    ctc_scheduler_t ctc_scheduler;
    uint32_t queue_index = 0;
    uint32_t dwrr_class = 0;
    uint32_t need_change = FALSE;
    ctc_sai_scheduler_port_index_t* p_scheduler_index = NULL;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;
#ifdef GOLDENGATE
    ctc_chip_device_info_t chip_device_info;
#endif

    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    sal_memset(&sched, 0x0, sizeof(ctc_qos_sched_t));
    sal_memset(&shape, 0x0, sizeof(ctc_qos_shape_t));
    sal_memset(&ctc_scheduler, 0x0, sizeof(ctc_scheduler_t));
    ctc_port = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(ctc_port);
    if(!p_queue_hash_info)
    {
        p_queue_hash_info = ctc_sai_queue_hash_add_hash(ctc_port);
    }
    if(SAI_NULL_OBJECT_ID != ctc_scheduler_id)
    {
        if(p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id)
        {
            return SAI_STATUS_OBJECT_IN_USE;
        }
        pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, ctc_scheduler_id);
        if(NULL == pscheduler)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        ret = ctc_sai_scheduler_db_map_scheduler(pscheduler, &ctc_scheduler);
        if(SAI_STATUS_SUCCESS != ret)
        {
            return ret;
        }
        sched.type = CTC_QOS_SCHED_QUEUE;
        sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        sched.sched.queue_sched.queue.gport = ctc_port;
        sched.sched.queue_sched.queue.queue_id = ctc_queue_id;
        p_scheduler_index = ctc_sai_scheduler_port_index_get_by_gport(ctc_port);
        if(!p_scheduler_index)
        {
            p_scheduler_index = ctc_sai_scheduler_port_index_add_hash(ctc_port);
        }
        if(SAI_SCHEDULING_STRICT == ctc_scheduler.scheduler_mode)
        {    
        #ifndef USW    
            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
            sched.sched.queue_sched.confirm_class = ctc_queue_id;
            ret += ctc_qos_set_sched(&sched);
        #endif
            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
            sched.sched.queue_sched.exceed_class = ctc_queue_id;
            ret += ctc_qos_set_sched(&sched);
            p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[ctc_queue_id] = 0;
            
            for(queue_index = 0; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
            {
                if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                {
                    dwrr_class = queue_index;
                    need_change = TRUE;
                    break;
                }
            }
            if(need_change)
            {
                for(queue_index = dwrr_class; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                {
                    if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                    {
                       sched.type = CTC_QOS_SCHED_QUEUE;
                       sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                       sched.sched.queue_sched.queue.gport = ctc_port;
                       sched.sched.queue_sched.queue.queue_id = queue_index;
                 #ifndef USW       
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                        sched.sched.queue_sched.confirm_class = dwrr_class;
                        ret += ctc_qos_set_sched(&sched);
                 #endif
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                        sched.sched.queue_sched.exceed_class = dwrr_class;
                        ret += ctc_qos_set_sched(&sched);
                    }
                }
            }
        }
        else
        {
        #ifndef USW    
            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
            sched.sched.queue_sched.confirm_class = 0;
            ret += ctc_qos_set_sched(&sched);
        #endif
            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
            sched.sched.queue_sched.exceed_class = 0;
            ret += ctc_qos_set_sched(&sched);

            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
            sched.sched.queue_sched.exceed_weight = ctc_scheduler.scheduler_weight;
            ret += ctc_qos_set_sched(&sched);

#ifdef GREATBELT
            sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT;
            sched.sched.queue_sched.confirm_weight = ctc_scheduler.scheduler_weight;
            ret += ctc_qos_set_sched(&sched);
#endif

            p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[ctc_queue_id] = 1;

            for(queue_index = 0; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
            {
                if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                {
                    dwrr_class = queue_index;
                    need_change = TRUE;
                    break;
                }
            }
            if(need_change)
            {
                for(queue_index = dwrr_class; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                {
                    if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                    {
                        sched.type = CTC_QOS_SCHED_QUEUE;
                        sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                        sched.sched.queue_sched.queue.gport = ctc_port;
                        sched.sched.queue_sched.queue.queue_id = queue_index;
               #ifndef USW    
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                        sched.sched.queue_sched.confirm_class = dwrr_class;
                        ret += ctc_qos_set_sched(&sched);
               #endif
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                        sched.sched.queue_sched.exceed_class = dwrr_class;
                        ret += ctc_qos_set_sched(&sched);
                    }
                }
            }
        }
        shape.type = CTC_QOS_SHAPE_QUEUE;
        shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        shape.shape.queue_shape.queue.queue_id = ctc_queue_id;
        shape.shape.queue_shape.queue.gport = ctc_port;
        
        if(SAI_QOS_DEF_SHAPING_PIR != ctc_scheduler.shape_pir)
        {
            shape.shape.queue_shape.enable = 1;
            shape.shape.queue_shape.cir = ctc_scheduler.shape_cir;
            shape.shape.queue_shape.pir = ctc_scheduler.shape_pir;
            /*GG is not supported cbs or pbs*/
            shape.shape.queue_shape.pbs = ctc_scheduler.shape_pbs;
            shape.shape.queue_shape.cbs = ctc_scheduler.shape_cbs;
            /*added by hansf for bug 46089*/
            #ifdef GREATBELT
                shape.shape.queue_shape.pbs = 1024;
            #endif
        }
        else
        {
            shape.shape.queue_shape.enable = 0;
        }
#ifdef GOLDENGATE
        sal_memset(&chip_device_info, 0x0, sizeof(ctc_chip_device_info_t));
        ctc_chip_get_property(0, CTC_CHIP_PROP_DEVICE_INFO, (void*)&chip_device_info);
        if((chip_device_info.version_id <= 1) && (shape.type == CTC_QOS_SHAPE_QUEUE))
        {
            if(((shape.shape.queue_shape.pir > 60000000) || (shape.shape.queue_shape.pir < 400000))&& (shape.shape.queue_shape.pir != SAI_QOS_DEF_SHAPING_PIR))
            {
                shape.shape.queue_shape.enable = 0;
            }
        }
#endif
        ret += ctc_qos_set_shape(&shape);
        if(ret)
        {
            return ret;
        }
        p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id = ctc_scheduler_id;
        pscheduler->used_cnt ++;
    }
    else
    {
        if(SAI_NULL_OBJECT_ID == p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id);
        if(NULL == pscheduler)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        sched.type = CTC_QOS_SCHED_QUEUE;
        sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        sched.sched.queue_sched.queue.gport = ctc_port;
        sched.sched.queue_sched.queue.queue_id = ctc_queue_id;
       #ifndef USW    
        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
        sched.sched.queue_sched.confirm_class = ctc_queue_id;
        ret += ctc_qos_set_sched(&sched);
      #endif
        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
        sched.sched.queue_sched.exceed_class = ctc_queue_id;
        ret += ctc_qos_set_sched(&sched);
        
        shape.type = CTC_QOS_SHAPE_QUEUE;
        shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
        shape.shape.queue_shape.queue.queue_id = ctc_queue_id;
        shape.shape.queue_shape.queue.gport = ctc_port;

        shape.shape.queue_shape.enable = 0;
        ret += ctc_qos_set_shape(&shape);
        if(ret)
        {
            return ret;
        }
        p_queue_hash_info->queue_entries[ctc_queue_id].scheduler_id = 0;
        pscheduler->used_cnt --;
    } 
     return ret;
}

sai_status_t ctc_sai_scheduler_port_set_scheduler(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  ctc_port = 0;
    uint32_t  ctc_scheduler_id = 0;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    ctc_qos_shape_t shape;
    ctc_scheduler_t ctc_scheduler;
    ctc_sai_scheduler_port_index_t* p_scheduler_index = NULL;

    ctc_port = CTC_SAI_OBJECT_INDEX_GET(port_id);
    sal_memset(&shape, 0x0, sizeof(ctc_qos_shape_t));
    sal_memset(&ctc_scheduler, 0x0, sizeof(ctc_scheduler_t));

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    if(SAI_NULL_OBJECT_ID != ctc_scheduler_id)
    {
        pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, ctc_scheduler_id);
        if(NULL == pscheduler)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        p_scheduler_index = ctc_sai_scheduler_port_index_get_by_gport(ctc_port);
        if(!p_scheduler_index)
        {
            p_scheduler_index = ctc_sai_scheduler_port_index_add_hash(ctc_port);
        }
        if(p_scheduler_index->ctc_sai_scheduler_port_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        ret = ctc_sai_scheduler_db_map_scheduler(pscheduler, &ctc_scheduler);
        shape.type = CTC_QOS_SHAPE_PORT;
        shape.shape.port_shape.gport = ctc_port;
        
        if(SAI_QOS_DEF_SHAPING_PIR != ctc_scheduler.shape_pir)
        {
            shape.shape.port_shape.enable = 1;
            shape.shape.port_shape.pir = ctc_scheduler.shape_pir;
            /*GG is not supported cbs or pbs*/
            shape.shape.port_shape.pbs = ctc_scheduler.shape_pbs;
        }
        else
        {
            shape.shape.port_shape.enable = 0;
        }
        ret += ctc_qos_set_shape(&shape);
        if(ret)
        {
            return ret;
        }
        p_scheduler_index->ctc_sai_scheduler_port_index = ctc_scheduler_id;
        pscheduler->used_cnt ++;
    }
    else
    {
        p_scheduler_index = ctc_sai_scheduler_port_index_get_by_gport(ctc_port);
        if(!p_scheduler_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, p_scheduler_index->ctc_sai_scheduler_port_index);
        if(NULL == pscheduler)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        shape.type = CTC_QOS_SHAPE_PORT;
        shape.shape.port_shape.gport = ctc_port;
        shape.shape.port_shape.enable = 0;
        ret += ctc_qos_set_shape(&shape);
        if(ret)
        {
            return ret;
        }
        p_scheduler_index->ctc_sai_scheduler_port_index = 0;
        pscheduler->used_cnt --;
    }

     return ret;
}

sai_status_t 
ctc_sai_scheduler_copp_set_scheduler(sai_object_id_t queue_id, 
                                const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_scheduler_id = 0;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    ctc_qos_shape_t qos_shape;
//    ctc_qos_sched_t sched;
    ctc_scheduler_t ctc_scheduler;
#ifdef GOLDENGATE
    ctc_chip_device_info_t chip_device_info;
#endif
    /*CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2 is openflow to controller reason*/
#ifdef GREATBELT
    uint32 reason_id[8] = {CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,0,0,
        };
#endif
#ifdef GOLDENGATE
    uint32 reason_id[8] = {CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,0,0,
        };
#endif
#ifdef USW
    uint32 reason_id[8] = {CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,0,0,
        };
#endif


    sal_memset(&qos_shape, 0x0, sizeof(ctc_qos_shape_t));
    sal_memset(&ctc_scheduler, 0x0, sizeof(ctc_scheduler_t));

    ctc_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);

    if(SAI_NULL_OBJECT_ID != ctc_scheduler_id)
    {

        pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, ctc_scheduler_id);
        if(NULL == pscheduler)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        ret = ctc_sai_scheduler_db_map_scheduler(pscheduler, &ctc_scheduler);
        if(SAI_STATUS_SUCCESS != ret)
        {
            return ret;
        }
        sal_memset(&qos_shape, 0, sizeof(ctc_qos_shape_t));
        qos_shape.type = CTC_QOS_SHAPE_QUEUE;
        qos_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        qos_shape.shape.queue_shape.queue.cpu_reason = reason_id[ctc_queue_id];
        qos_shape.shape.queue_shape.enable = TRUE;
        qos_shape.shape.queue_shape.pir = ctc_scheduler.shape_pir;
        qos_shape.shape.queue_shape.pbs = ctc_scheduler.shape_pbs;
        qos_shape.shape.queue_shape.cir = ctc_scheduler.shape_cir;
        qos_shape.shape.queue_shape.cbs = ctc_scheduler.shape_cbs;
#ifdef GOLDENGATE
        sal_memset(&chip_device_info, 0x0, sizeof(ctc_chip_device_info_t));
        ctc_chip_get_property(0, CTC_CHIP_PROP_DEVICE_INFO, (void*)&chip_device_info);
        if(chip_device_info.version_id <= 1)
        {
            if((qos_shape.shape.queue_shape.pir > 1000) && (qos_shape.shape.queue_shape.pir != SAI_QOS_DEF_SHAPING_PIR))
            {
                qos_shape.shape.queue_shape.pir = 1000;
            }
        }
#endif
        ret += ctc_qos_set_shape(&qos_shape);

/*added by yejl for bug 44450, 2019-03-18, set the GB class 0 rate for the forward to cpu packet*/
#ifdef GREATBELT
        if (0 == ctc_queue_id)
        {
            sal_memset(&qos_shape, 0, sizeof(ctc_qos_shape_t));
            qos_shape.type = CTC_QOS_SHAPE_QUEUE;
            qos_shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
            qos_shape.shape.queue_shape.queue.cpu_reason = CTC_PKT_CPU_REASON_FWD_CPU;
            qos_shape.shape.queue_shape.enable = TRUE;
            qos_shape.shape.queue_shape.pir = ctc_scheduler.shape_pir;
            qos_shape.shape.queue_shape.pbs = ctc_scheduler.shape_pbs;
            qos_shape.shape.queue_shape.cir = ctc_scheduler.shape_cir;
            qos_shape.shape.queue_shape.cbs = ctc_scheduler.shape_cbs;
            ret += ctc_qos_set_shape(&qos_shape);
        }
#endif

/*
        sal_memset (&sched, 0, sizeof(ctc_qos_sched_t));
        sched.type = CTC_QOS_SCHED_QUEUE;
        sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
        sched.sched.queue_sched.queue.cpu_reason = reason_id[ctc_queue_id];
        sched.sched.queue_sched.confirm_class = 0;
        sched.sched.queue_sched.exceed_class = 0;
    #ifndef USW    
        //set confirm class
        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
        sched.sched.queue_sched.confirm_class = ctc_queue_id;
        //modify by huwx for bug 44348, when sdk fix bug reopen this api callback, 2017-06-22
        //ret += ctc_qos_set_sched(&sched);
    #endif
        //set exceed class
        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
        sched.sched.queue_sched.confirm_class = 0;
        sched.sched.queue_sched.exceed_class = ctc_queue_id;
        //modify by huwx for bug 44348, when sdk fix bug reopen this api callback, 2017-06-22
        //ret += ctc_qos_set_sched(&sched);
*/
        g_sai_scheduler_copp_info.queue_entries[ctc_queue_id].scheduler_id = ctc_scheduler_id;
    }
     
     return ret;
}

sai_status_t 
ctc_sai_scheduler_copp_get_scheduler(sai_object_id_t queue_id,
                               sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  ctc_queue_id = 0;

    ctc_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);

    attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_API_SCHEDULER, 
        g_sai_scheduler_copp_info.queue_entries[ctc_queue_id].scheduler_id);
    
     return ret;
}


#define ________SAI_SCHEDULER_DEBUG_FUNC
sai_status_t sai_create_scheduler_profile_debug_param(
    _Out_ sai_object_id_t  *scheduler_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    CTC_SAI_DEBUG("in:attr_count %u", attr_count);
    CTC_SAI_DEBUG("out:scheduler_id 0x%llx", (*scheduler_id));
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_scheduler_profile_debug_param(
    _In_ sai_object_id_t scheduler_id
    )
{
    CTC_SAI_DEBUG("in:scheduler_id 0x%llx", scheduler_id);
    return SAI_STATUS_SUCCESS;
}
sai_status_t sai_set_scheduler_attribute_debug_param(
    _In_ sai_object_id_t scheduler_id,
    _In_ const sai_attribute_t *attr
    )
{
    CTC_SAI_DEBUG("in:scheduler_id 0x%llx", scheduler_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_scheduler_attribute_debug_param(
    _In_ sai_object_id_t scheduler_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    CTC_SAI_DEBUG("in:scheduler_id 0x%llx", scheduler_id);
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SCHEDULER_INNER_FUNC
sai_status_t sai_create_scheduler_profile(
    _Out_ sai_object_id_t  *scheduler_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    sai_attribute_t *attr_list_db = NULL;
    sai_uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(scheduler_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);


    ret = ctc_sai_scheduler_db_alloc_scheduler(&pscheduler);
    if(NULL == pscheduler)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /*if the attr changed, this init block_size should be changed too*/
    pscheduler->scheduler_vector = ctc_vector_init(1,SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM + 1);
    if(NULL == pscheduler->scheduler_vector)
    {
        ret = SAI_STATUS_NO_MEMORY;
        return ret;
    }

    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM <= attr_list[index].id) && (SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE >= attr_list[index].id))
        {
            /* save entry info in db */
            attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
            sal_memcpy(attr_list_db, &attr_list[index], sizeof(sai_attribute_t));

            if(FALSE == ctc_vector_add(pscheduler->scheduler_vector, attr_list[index].id - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM, attr_list_db))
            {
                ret = SAI_STATUS_FAILURE;
                return ret;
            }
         }     
     }
    /* pass at least one field */
    if (0 == pscheduler->scheduler_vector->used_cnt)
    {
        ret = SAI_STATUS_INVALID_PARAMETER;
        return ret;
    }

    if(FALSE == ctc_vector_add(g_sai_scheduler_info.pvector,
                CTC_SAI_OBJECT_INDEX_GET(pscheduler->scheduler_id),
                pscheduler))
    {
        ret = SAI_STATUS_FAILURE;
        return ret;
    }

    *scheduler_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_SCHEDULER,pscheduler->scheduler_id);
    sai_create_scheduler_profile_debug_param(scheduler_id, attr_count, attr_list);

    return ret;
}

sai_status_t sai_remove_scheduler_profile(
    _In_ sai_object_id_t scheduler_id
    )
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    uint32_t  ctc_scheduler_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_remove_scheduler_profile_debug_param(scheduler_id);

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(scheduler_id);
    pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, ctc_scheduler_id);
    if(NULL == pscheduler)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(pscheduler->used_cnt)
    {
        return SAI_STATUS_OBJECT_IN_USE;
    }

    ctc_vector_del(g_sai_scheduler_info.pvector, ctc_scheduler_id);

    ctc_sai_scheduler_db_release_scheduler(pscheduler);

    return ret;
}

sai_status_t sai_set_scheduler_attribute(
    _In_ sai_object_id_t scheduler_id,
    _In_ const sai_attribute_t *attr
    )
{
    uint32_t  ctc_scheduler_id = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    //ctc_qos_drop_t  ctc_drop;
    ctc_sai_scheduler_t     *pscheduler = NULL;
    uint32 port_index = 0;
    uint32 queue_id_index = 0;
    ctc_scheduler_t ctc_scheduler;
    ctc_qos_sched_t sched;
    ctc_qos_shape_t shape;
    uint32_t queue_index = 0;
    uint32_t dwrr_class = 0;
    uint32_t need_change = FALSE;
    ctc_sai_scheduler_port_index_t* p_scheduler_index = NULL;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;
    uint32 gport = 0;
#ifdef GOLDENGATE
    ctc_chip_device_info_t chip_device_info;
#endif
    CTC_SAI_DEBUG_FUNC();
    sai_set_scheduler_attribute_debug_param(scheduler_id, attr);

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(scheduler_id);
    sal_memset(&ctc_scheduler, 0x0, sizeof(ctc_scheduler_t));
    sal_memset(&sched, 0x0, sizeof(ctc_qos_sched_t));
    sal_memset(&shape, 0x0, sizeof(ctc_qos_shape_t));

    pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector, ctc_scheduler_id);
    if(NULL == pscheduler)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    ctc_sai_scheduler_map_db_attribute(attr, pscheduler);
    ctc_sai_scheduler_db_map_scheduler(pscheduler, &ctc_scheduler);

    if(pscheduler->used_cnt)
    {
        for(port_index = 0; port_index < CTC_MAX_LPORT; port_index ++)
        {
           gport = CTC_MAP_LPORT_TO_GPORT(0, port_index);
            p_scheduler_index = ctc_sai_scheduler_port_index_get_by_gport(gport);
            if(!p_scheduler_index)
            {
                continue;
            }
            p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(gport);
            if(!p_queue_hash_info)
            {
                continue;
            }
            for(queue_id_index = 0; queue_id_index < CTC_SAI_PORT_QUEUE_NUM; queue_id_index ++)
            {
                if(p_queue_hash_info->queue_entries[queue_id_index].scheduler_id == pscheduler->scheduler_id)
                {
                    sched.type = CTC_QOS_SCHED_QUEUE;
                    sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                    sched.sched.queue_sched.queue.gport = gport;
                    sched.sched.queue_sched.queue.queue_id = queue_id_index;
                    if(SAI_SCHEDULING_STRICT == ctc_scheduler.scheduler_mode)
                    {
                    #ifndef USW    
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                        sched.sched.queue_sched.confirm_class = queue_id_index;
                        ret += ctc_qos_set_sched(&sched);
                   #endif
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                        sched.sched.queue_sched.exceed_class = queue_id_index;
                        ret += ctc_qos_set_sched(&sched);
                        p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_id_index] = 0;
            
                        for(queue_index = 0; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                        {
                            if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                            {
                                dwrr_class = queue_index;
                                need_change = TRUE;
                                break;
                            }
                        }
                        if(need_change)
                        {
                            for(queue_index = dwrr_class; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                            {
                                if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                                {
                                   sched.type = CTC_QOS_SCHED_QUEUE;
                                   sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                                   sched.sched.queue_sched.queue.gport = gport;
                                   sched.sched.queue_sched.queue.queue_id = queue_index;
                                 #ifndef USW    
                                    sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                                    sched.sched.queue_sched.confirm_class = dwrr_class;
                                    ret += ctc_qos_set_sched(&sched);
                                 #endif
                                    sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                                    sched.sched.queue_sched.exceed_class = dwrr_class;
                                    ret += ctc_qos_set_sched(&sched);
                                }
                            }
                        }

                    }
                    else
                    {
                        #ifndef USW    
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                        sched.sched.queue_sched.confirm_class = 0;
                        ret += ctc_qos_set_sched(&sched);
                        #endif
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                        sched.sched.queue_sched.exceed_class = 0;
                        ret += ctc_qos_set_sched(&sched);

                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_WEIGHT;
                        sched.sched.queue_sched.exceed_weight = ctc_scheduler.scheduler_weight;
                        ret += ctc_qos_set_sched(&sched);

#ifdef GREATBELT
                        sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT;
                        sched.sched.queue_sched.confirm_weight = ctc_scheduler.scheduler_weight;
                        ret += ctc_qos_set_sched(&sched);
#endif

                        p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_id_index] = 1;
                        for(queue_index = 0; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                        {
                            if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                            {
                                dwrr_class = queue_index;
                                need_change = TRUE;
                                break;
                            }
                        }
                        if(need_change)
                        {
                            for(queue_index = dwrr_class; queue_index < CTC_SAI_PORT_QUEUE_NUM; queue_index ++)
                            {
                                if(p_scheduler_index->ctc_sai_scheduler_queue_dwrr_mode[queue_index])
                                {
                                    sched.type = CTC_QOS_SCHED_QUEUE;
                                    sched.sched.queue_sched.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                                    sched.sched.queue_sched.queue.gport = gport;
                                    sched.sched.queue_sched.queue.queue_id = queue_index;
                                    #ifndef USW   
                                    sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_CONFIRM_CLASS;
                                    sched.sched.queue_sched.confirm_class = dwrr_class;
                                    ret += ctc_qos_set_sched(&sched);
                                    #endif
                                    sched.sched.queue_sched.cfg_type = CTC_QOS_SCHED_CFG_EXCEED_CLASS;
                                    sched.sched.queue_sched.exceed_class = dwrr_class;
                                    ret += ctc_qos_set_sched(&sched);
                                }
                            }
                        }

                    }
                    shape.type = CTC_QOS_SHAPE_QUEUE;
                    shape.shape.queue_shape.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                    shape.shape.queue_shape.queue.queue_id = queue_id_index;
                    shape.shape.queue_shape.queue.gport = gport;
                    
                    if(SAI_QOS_DEF_SHAPING_PIR != ctc_scheduler.shape_pir)
                    {
                        shape.shape.queue_shape.enable = 1;
                        shape.shape.queue_shape.cir = ctc_scheduler.shape_cir;
                        shape.shape.queue_shape.pir = ctc_scheduler.shape_pir;
                        /*GG is not supported cbs or pbs*/
                        shape.shape.queue_shape.pbs = ctc_scheduler.shape_pbs;
                        shape.shape.queue_shape.cbs = ctc_scheduler.shape_cbs;
                        /*added by hansf for bug 46089*/
                        #ifdef GREATBELT
                            shape.shape.queue_shape.pbs = 1024;
                        #endif
                    }
                    else
                    {
                        shape.shape.queue_shape.enable = 0;
                    }
#ifdef GOLDENGATE
                    sal_memset(&chip_device_info, 0x0, sizeof(ctc_chip_device_info_t));
                    ctc_chip_get_property(0, CTC_CHIP_PROP_DEVICE_INFO, (void*)&chip_device_info);
                    if((chip_device_info.version_id <= 1) && (shape.type == CTC_QOS_SHAPE_QUEUE))
                    {
                        if(((shape.shape.queue_shape.pir > 60000000) || (shape.shape.queue_shape.pir < 400000))&& (shape.shape.queue_shape.pir != SAI_QOS_DEF_SHAPING_PIR))
                        {
                            shape.shape.queue_shape.enable = 0;
                        }
                    }
#endif
                    ret += ctc_qos_set_shape(&shape);
                    if(ret)
                    {
                        return ret;
                    }

                }
            }

            if(p_scheduler_index->ctc_sai_scheduler_port_index == pscheduler->scheduler_id)
            {
                shape.type = CTC_QOS_SHAPE_PORT;
                shape.shape.port_shape.gport = gport;
                
                if(SAI_QOS_DEF_SHAPING_PIR != ctc_scheduler.shape_pir)
                {
                    shape.shape.port_shape.enable = 1;
                    shape.shape.port_shape.pir = ctc_scheduler.shape_pir;
                    /*GG is not supported cbs or pbs*/
                    shape.shape.port_shape.pbs = ctc_scheduler.shape_pbs;
                }
                else
                {
                    shape.shape.port_shape.enable = 0;
                }
                ret += ctc_qos_set_shape(&shape);
            }
        }
    }

    return ret;
}

sai_status_t sai_get_scheduler_attribute(
    _In_ sai_object_id_t scheduler_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t *attr        = NULL;
    uint32_t         attr_idx    = 0;
    uint32_t        ctc_scheduler_id = 0;
    ctc_sai_scheduler_t         *pscheduler = NULL;

    CTC_SAI_DEBUG_FUNC();
    sai_get_scheduler_attribute_debug_param(scheduler_id, attr_count, attr_list);

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    ctc_scheduler_id = CTC_SAI_OBJECT_INDEX_GET(scheduler_id);
    pscheduler = ctc_vector_get(g_sai_scheduler_info.pvector,ctc_scheduler_id);
    if(NULL == pscheduler)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        if ((SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM <= attr->id) && (SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE >= attr->id))
        {
            attr = ctc_vector_get(pscheduler->scheduler_vector, attr->id - SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM);
            if(NULL == attr)
            {
                continue;
            }
            sal_memcpy(attr_list + attr_idx,attr,sizeof(sai_attribute_t));
        }
    }

    return ret;
}

#define ________SAI_SCHEDULER_INNER_FUNC
sai_status_t __scheduler_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_scheduler_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __scheduler_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_scheduler_api_t      g_sai_scheduler_api_func = {
    .create_scheduler_profile           = sai_create_scheduler_profile,
    .remove_scheduler_profile           = sai_remove_scheduler_profile,
    .set_scheduler_attribute    = sai_set_scheduler_attribute,
    .get_scheduler_attribute    = sai_get_scheduler_attribute,
};

static ctc_sai_api_reg_info_t g_scheduler_api_reg_info = {
        .id  = SAI_API_SCHEDULER,
        .init_func = __scheduler_init_mode_fn,
        .exit_func = __scheduler_exit_mode_fn,
        .api_method_table = &g_sai_scheduler_api_func,
        .private_data     = NULL,
};

#define ________SAI_SCHEDULER_OUTER_FUNC
sai_status_t ctc_sai_scheduler_init()
{
    api_reg_register_fn(&g_scheduler_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
