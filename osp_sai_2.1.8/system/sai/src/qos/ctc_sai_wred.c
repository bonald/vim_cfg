#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_wred.h>
#include <ctc_sai_queue.h>
#include <ctc_sai_debug.h>

ctc_sai_wred_info_t g_sai_wred_info;
ctc_qos_queue_drop_t    wred_default;
extern ctc_sai_queue_info_t  g_sai_queue_info;
extern ctc_sai_queue_hash_t*
ctc_sai_queue_hash_get_by_gport(uint32 gport);

extern ctc_sai_queue_hash_t*
ctc_sai_queue_hash_add_hash(uint32 gport);

sai_status_t 
ctc_sai_wred_map_db_attribute(const sai_attribute_t *attr, ctc_sai_wred_t  *pwred)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t   *attr_value = NULL;
    sai_attribute_t *attr_list_db = NULL;
    attr_value = ctc_vector_get(pwred->wred_vector, attr->id - SAI_WRED_ATTR_GREEN_ENABLE);
    if(NULL == attr_value)
    {
        attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
        sal_memcpy(attr_list_db, attr, sizeof(sai_attribute_t));

        if(FALSE == ctc_vector_add(pwred->wred_vector, attr->id - SAI_WRED_ATTR_GREEN_ENABLE, attr_list_db))
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
ctc_sai_wred_db_map_wred(ctc_sai_wred_t* psai_wred, ctc_qos_drop_t* pctc_drop)
{
    sai_attribute_t   *attr_value = NULL;

    sal_memcpy(&pctc_drop->drop, &wred_default, sizeof(ctc_qos_queue_drop_t));
    attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_GREEN_ENABLE - SAI_WRED_ATTR_GREEN_ENABLE);
    if (attr_value)
    {
        if(TRUE == attr_value->value.booldata)
        {
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_GREEN_MIN_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.min_th[2] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_GREEN_MAX_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.max_th[2] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_GREEN_DROP_PROBABILITY - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.drop_prob[2] =  (attr_value->value.s32 *31)/100;
            }
        }
    }

    #ifdef USW
    pctc_drop->drop.max_th[3] = USW_DEFAULT_WRED_DROP_MAX_TH_3_CRITIC_VALUE;
    #endif
    
    attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_YELLOW_ENABLE - SAI_WRED_ATTR_GREEN_ENABLE);
    if (attr_value)
    {
        if(TRUE == attr_value->value.booldata)
        {
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.min_th[1] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.max_th[1] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.drop_prob[1] =  (attr_value->value.s32 *31)/100;
            }
        }
    }

    attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_RED_ENABLE - SAI_WRED_ATTR_GREEN_ENABLE);
    if (attr_value)
    {
        if(TRUE == attr_value->value.booldata)
        {
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_RED_MIN_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.min_th[0] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_RED_MAX_THRESHOLD - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.max_th[0] =  attr_value->value.s32;
            }
            attr_value = ctc_vector_get(psai_wred->wred_vector, SAI_WRED_ATTR_RED_DROP_PROBABILITY - SAI_WRED_ATTR_GREEN_ENABLE);
            if(attr_value)
            {
                pctc_drop->drop.drop_prob[0] =  (attr_value->value.s32 *31)/100;
            }
        }
    }

#if 1 /* SYSTEM MODIFIED: fix qos bug 38760 add by wangqj at 2016/9/18 */
#ifdef GOLDENGATE
    pctc_drop->drop.drop_prob[0] =  GOLDENGATE_DEFAULT_WRED_PROBABILITY;
    pctc_drop->drop.drop_prob[1] =  GOLDENGATE_DEFAULT_WRED_PROBABILITY;
    pctc_drop->drop.drop_prob[2] =  GOLDENGATE_DEFAULT_WRED_PROBABILITY;
#endif
#endif

    return SAI_STATUS_SUCCESS;
    
}



sai_status_t
ctc_sai_wred_db_alloc_wred(ctc_sai_wred_t** ppwred)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_wred_t *pwred = NULL;
    uint32_t            index     = 0;

    if (g_sai_wred_info.pvector->used_cnt >= p_sai_glb->qos_wred_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }

    pwred = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_wred_t));

    if(pwred)
    {
        sal_memset(pwred,0,sizeof(ctc_sai_wred_t));
        ctclib_opb_alloc_offset(&p_sai_glb->qos_wred_opb, &index);
        pwred->wred_id= index;
        pwred->used_cnt = 0;
    }

    *ppwred = pwred;

    return SAI_STATUS_SUCCESS;
}

void
ctc_sai_wred_db_release_wred(ctc_sai_wred_t *pwred)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_uint32_t index = 0;
    sai_attribute_t   *attr_value = NULL;
    if(NULL == pwred)
    {
        return ;
    }
    for (index = SAI_WRED_ATTR_GREEN_ENABLE; index <= SAI_WRED_ATTR_ECN_MARK_ENABLE; index++)
    {
        attr_value = ctc_vector_get(pwred->wred_vector, index - SAI_WRED_ATTR_GREEN_ENABLE);
        if (attr_value)
        {
            XFREE(MEM_FEA_TEMP_TODO, attr_value);
            attr_value = NULL;
            ctc_vector_del(pwred->wred_vector, index - SAI_WRED_ATTR_GREEN_ENABLE);
        }
    }
    ctclib_opb_free_offset(&p_sai_glb->qos_wred_opb, pwred->wred_id);
    ctc_vector_release(pwred->wred_vector); /* fix qos memory leak bug add by wangqj in 2016/10/25 */
    XFREE(MEM_FEA_TEMP_TODO, pwred);
}

sai_status_t
ctc_sai_wred_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();

    p_sai_glb->qos_wred_max_count = 1000;
    g_sai_wred_info.pvector = ctc_vector_init(10,100);

    if(NULL == g_sai_wred_info.pvector)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    if (0 != ctclib_opb_create(&p_sai_glb->qos_wred_opb, 1, p_sai_glb->qos_wred_max_count, "wred"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_wred_info.counter_max = 2048; /*need check the number*/
    g_sai_wred_info.pcounter_vector = ctc_vector_init(32,64);

    if(NULL == g_sai_wred_info.pcounter_vector)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    sal_memset(&wred_default, 0x0, sizeof(ctc_qos_queue_drop_t));
    wred_default.mode = CTC_QUEUE_DROP_WTD;
    
    /* SYSTEM MODIFIED: fix qos bug 39957 add by wangqj at 2016/9/13
     * reference: "modified by yejl to fix bug39742 and bug39747" */
    wred_default.max_th[2] = SAI_QOS_DROP_DEF_THRESH;
    wred_default.min_th[2] = SAI_QOS_DROP_DEF_THRESH/8;
    wred_default.drop_prob[2] = SAI_QOS_DROP_DEF_PRO;
    wred_default.max_th[1] = SAI_QOS_DROP_DEF_THRESH;
    wred_default.min_th[1] = SAI_QOS_DROP_DEF_THRESH/8;
    wred_default.drop_prob[1] = SAI_QOS_DROP_DEF_PRO;
    wred_default.max_th[0] = SAI_QOS_DROP_DEF_THRESH;
    wred_default.min_th[1] = SAI_QOS_DROP_DEF_THRESH/8;
    wred_default.drop_prob[0] = SAI_QOS_DROP_DEF_PRO;
    #ifdef USW
    wred_default.max_th[3] = USW_DEFAULT_WRED_DROP_MAX_TH_3_CRITIC_VALUE;
    #endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_wred_queue_get_wred(_In_ sai_object_id_t queue_id,
                                    _Inout_ sai_attribute_t *attr)
{
    uint32_t  sai_queue_id = 0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port = 0;
    uint32_t ctc_wred_id = 0;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;
    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_port = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);

    p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(ctc_port);
    if(!p_queue_hash_info)
    {
        return SAI_STATUS_FAILURE;
    }

    if (SAI_QUEUE_ATTR_DROP_TYPE == attr->id)
    {
        attr->value.s32 = p_queue_hash_info->queue_entries[ctc_queue_id].wred_type;
    }
    else
    {
        ctc_wred_id = p_queue_hash_info->queue_entries[ctc_queue_id].wred_id;
        attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, ctc_wred_id);
    }
 
    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_wred_queue_set_wred(_In_ sai_object_id_t queue_id,
                                _In_ const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  sai_queue_id = 0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port = 0;
    uint32_t  ctc_wred_id = 0;
    ctc_sai_wred_t     *pwred = NULL;
    ctc_qos_drop_t  ctc_drop;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;

    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    sal_memset(&ctc_drop, 0x0, sizeof(ctc_qos_drop_t));
    ctc_port = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);

    p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(ctc_port);
    if(!p_queue_hash_info)
    {
        p_queue_hash_info = ctc_sai_queue_hash_add_hash(ctc_port);
    }
    if (SAI_QUEUE_ATTR_DROP_TYPE == attr->id)
    {
        p_queue_hash_info->queue_entries[ctc_queue_id].wred_type = attr->value.s32;
        if(SAI_NULL_OBJECT_ID == p_queue_hash_info->queue_entries[ctc_queue_id].wred_id)
        {
            return SAI_STATUS_SUCCESS;
        }
        else
        {
            pwred = ctc_vector_get(g_sai_wred_info.pvector, p_queue_hash_info->queue_entries[ctc_queue_id].wred_id);
            if(NULL == pwred)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
            ret = ctc_sai_wred_db_map_wred(pwred, &ctc_drop);
            if(SAI_STATUS_SUCCESS != ret)
            {
                return ret;
            }
            ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            ctc_drop.queue.gport = ctc_port;
            ctc_drop.queue.queue_id = ctc_queue_id;
            if(SAI_DROP_TYPE_TAIL == p_queue_hash_info->queue_entries[ctc_queue_id].wred_type)
            {    
                ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
/* modified by liwh for bug 53876, 2019-10-15 */
#ifdef GREATBELT
                ctc_drop.drop.max_th[3] = 0x44;
#endif
/* liwh end */
            }
            else
                ctc_drop.drop.mode = CTC_QUEUE_DROP_WRED;
            ret = ctc_qos_set_drop_scheme(&ctc_drop);
            if(ret)
            {
                return ret;
            }
        }
    }
    if(SAI_QUEUE_ATTR_WRED_PROFILE_ID == attr->id)
    {
        ctc_wred_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
        if(SAI_NULL_OBJECT_ID != ctc_wred_id)
        {
            if(p_queue_hash_info->queue_entries[ctc_queue_id].wred_id)
            {
                return SAI_STATUS_OBJECT_IN_USE;
            }
            pwred = ctc_vector_get(g_sai_wred_info.pvector, ctc_wred_id);
            if(NULL == pwred)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }

            ret = ctc_sai_wred_db_map_wred(pwred, &ctc_drop);
            if(SAI_STATUS_SUCCESS != ret)
            {
                return ret;
            }
            ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            ctc_drop.queue.gport = ctc_port;
            ctc_drop.queue.queue_id = ctc_queue_id;
            if (SAI_DROP_TYPE_TAIL == p_queue_hash_info->queue_entries[ctc_queue_id].wred_type)
            {
                ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
/* modified by liwh for bug 53876, 2019-10-15 */
#ifdef GREATBELT
                ctc_drop.drop.max_th[3] = 0x44;
#endif
/* liwh end */
            }
            else
                ctc_drop.drop.mode = CTC_QUEUE_DROP_WRED;
            ret = ctc_qos_set_drop_scheme(&ctc_drop);
            if(ret)
            {
                return ret;
            }
            p_queue_hash_info->queue_entries[ctc_queue_id].wred_id = ctc_wred_id;
            pwred->used_cnt ++;
        }
        else
        {
            pwred = ctc_vector_get(g_sai_wred_info.pvector, p_queue_hash_info->queue_entries[ctc_queue_id].wred_id);
            if(NULL == pwred)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
            if(SAI_NULL_OBJECT_ID == p_queue_hash_info->queue_entries[ctc_queue_id].wred_id)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
            
            ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            ctc_drop.queue.gport = ctc_port;
            ctc_drop.queue.queue_id = ctc_queue_id;

            sal_memcpy(&ctc_drop.drop, &wred_default, sizeof(ctc_qos_queue_drop_t));
            ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
            ret = ctc_qos_set_drop_scheme(&ctc_drop);
            if(ret)
            {
                return ret;
            }
            p_queue_hash_info->queue_entries[ctc_queue_id].wred_id = 0;
            /* mod by hansf, should not change drop type */
            //p_queue_hash_info->queue_entries[ctc_queue_id].wred_type = SAI_DROP_TYPE_TAIL;
            pwred->used_cnt --;
        }
    }
 
     return ret;
}

#define ________SAI_WRED_API_FUNC
sai_status_t sai_create_wred_profile(
    _Out_ sai_object_id_t *wred_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_wred_t     *pwred = NULL;
    sai_attribute_t *attr_list_db = NULL;
    sai_uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:wred_id 0x%llx", (*wred_id));
    CTC_SAI_PTR_VALID_CHECK(wred_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);


    ret = ctc_sai_wred_db_alloc_wred(&pwred);
    if(NULL == pwred)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /*if the attr changed, this init block_size should be changed too*/
    pwred->wred_vector = ctc_vector_init(1,SAI_WRED_ATTR_ECN_MARK_ENABLE - SAI_WRED_ATTR_GREEN_ENABLE + 1);
    if(NULL == pwred->wred_vector)
    {
        ret = SAI_STATUS_NO_MEMORY;
        return ret;
    }

    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_WRED_ATTR_GREEN_ENABLE <= attr_list[index].id) && (SAI_WRED_ATTR_ECN_MARK_ENABLE >= attr_list[index].id))
        {
            /* save entry info in db */
            attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
            sal_memcpy(attr_list_db, &attr_list[index], sizeof(sai_attribute_t));

            if(FALSE == ctc_vector_add(pwred->wred_vector, attr_list[index].id - SAI_WRED_ATTR_GREEN_ENABLE, attr_list_db))
            {
                ret = SAI_STATUS_FAILURE;
                return ret;
            }
         }     
     }

    if(FALSE == ctc_vector_add(g_sai_wred_info.pvector,
                CTC_SAI_OBJECT_INDEX_GET(pwred->wred_id),
                pwred))
    {
        ret = SAI_STATUS_FAILURE;
        return ret;
    }

    *wred_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_WRED,pwred->wred_id);

    return ret;
}

sai_status_t  sai_remove_wred_profile(
    _In_ sai_object_id_t  wred_id)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_wred_t     *pwred = NULL;
    uint32_t  ctc_wred_id = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:wred_id 0x%llx", wred_id);

    ctc_wred_id = CTC_SAI_OBJECT_INDEX_GET(wred_id);
    pwred = ctc_vector_get(g_sai_wred_info.pvector, ctc_wred_id);
    if(NULL == pwred)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(pwred->used_cnt)
    {
        return SAI_STATUS_OBJECT_IN_USE;
    }

    ctc_vector_del(g_sai_wred_info.pvector, ctc_wred_id);

    ctc_sai_wred_db_release_wred(pwred);

    return ret;
}

sai_status_t sai_set_wred_attribute(
    _In_ sai_object_id_t wred_id,
    _In_ const sai_attribute_t *attr
    )
{
    uint32_t  ctc_wred_id = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_qos_drop_t  ctc_drop;
    ctc_sai_wred_t     *pwred = NULL;
    uint32 port_index = 0;
    uint32 queue_id_index = 0;
    uint32 used_cnt = 0;
    ctc_sai_queue_hash_t* p_queue_hash_info = NULL;
    uint32 gport = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:wred_id 0x%llx", wred_id);

    ctc_wred_id = CTC_SAI_OBJECT_INDEX_GET(wred_id);
    sal_memset(&ctc_drop, 0x0, sizeof(ctc_qos_drop_t));

    pwred = ctc_vector_get(g_sai_wred_info.pvector, ctc_wred_id);
    if(NULL == pwred)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    ctc_sai_wred_map_db_attribute(attr, pwred);
    ctc_sai_wred_db_map_wred(pwred, &ctc_drop);

    if(pwred->used_cnt)
    {
        for(port_index = 0; port_index < CTC_MAX_LPORT; port_index ++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(0, port_index);
            p_queue_hash_info = ctc_sai_queue_hash_get_by_gport(gport);
            if(!p_queue_hash_info)
            {
                continue;
            }
            for(queue_id_index = 0; queue_id_index < CTC_SAI_PORT_QUEUE_NUM; queue_id_index ++)
            {
                if(p_queue_hash_info->queue_entries[queue_id_index].wred_id == pwred->wred_id)
                {
                    used_cnt ++;
                    
                    ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                    ctc_drop.queue.gport = gport;
                    ctc_drop.queue.queue_id = queue_id_index;
                    if(SAI_DROP_TYPE_TAIL == p_queue_hash_info->queue_entries[queue_id_index].wred_type)
                        ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
                    else
                        ctc_drop.drop.mode = CTC_QUEUE_DROP_WRED;
                    ret = ctc_qos_set_drop_scheme(&ctc_drop);
                    if(ret || (used_cnt == pwred->used_cnt))
                    {
                        return ret;
                    }
                }
            }
        }
    }

    return ret;
}

sai_status_t sai_get_wred_attribute(
    _In_ sai_object_id_t wred_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
   )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t *attr        = NULL;
    uint32_t         attr_idx    = 0;
    uint32_t        ctc_wred_id = 0;
    ctc_sai_wred_t         *pwred = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:wred_id 0x%llx", wred_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    ctc_wred_id = CTC_SAI_OBJECT_INDEX_GET(wred_id);
    pwred = ctc_vector_get(g_sai_wred_info.pvector,ctc_wred_id);
    if(NULL == pwred)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        if ((SAI_WRED_ATTR_GREEN_ENABLE <= attr->id) && (SAI_WRED_ATTR_ECN_MARK_ENABLE >= attr->id))
        {
            attr = ctc_vector_get(pwred->wred_vector, attr->id - SAI_WRED_ATTR_GREEN_ENABLE);
            if(NULL == attr)
            {
                continue;
            }
            sal_memcpy(attr_list + attr_idx,attr,sizeof(sai_attribute_t));
        }
    }

    return ret;
}

#define ________SAI_QUEUE_INNER_FUNC
sai_status_t __wred_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_wred_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __wred_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static  sai_wred_api_t  g_sai_wred_api_func = {
    .create_wred_profile           = sai_create_wred_profile,
    .remove_wred_profile           = sai_remove_wred_profile,
    .set_wred_attribute    = sai_set_wred_attribute,
    .get_wred_attribute    = sai_get_wred_attribute,
};


static ctc_sai_api_reg_info_t g_wred_api_reg_info = {
        .id  = SAI_API_WRED,
        .init_func = __wred_init_mode_fn,
        .exit_func = __wred_exit_mode_fn,
        .api_method_table = &g_sai_wred_api_func,
        .private_data     = NULL,
};

#define ________SAI_WRED_OUTER_FUNC
sai_status_t ctc_sai_wred_init()
{
    api_reg_register_fn(&g_wred_api_reg_info);

    return SAI_STATUS_SUCCESS;
}
