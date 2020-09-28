#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_group.h"

/*************************************************************************************************
 * Name         : _fei_group_send_group_info_head
 * Purpose      : Fei layer group send group info head
 * Input        : p_group: glb_openflow_group_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
uint32
_fei_group_send_group_info_head(glb_openflow_group_t* p_group)
{
    fei_openflow_add_group_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_add_group_req_t));

    req.partial_type = FEI_OPENFLOW_GROUP_PARTIAL_TYPE_HEAD;
    /* !!! convert p_group to partial group head by statically casting. */
    sal_memcpy(&req.u.head.group, p_group, sizeof(glb_openflow_group_t));

    return fei_talk(FEI_OPENFLOW_GROUP_ADD, &req, sizeof(req));
}

/*************************************************************************************************
 * Name         : _fei_group_send_group_info_bucket
 * Purpose      : Fei layer group send group info bucket
 * Input        : p_group: glb_openflow_group_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
uint32
_fei_group_send_group_info_bucket(glb_openflow_group_t* p_group)
{
    uint32 remained_bucket_count = p_group->n_buckets;
    uint32 chunked_bucket_count = 0;
    uint32 copied_bucket_count = 0;
    uint32 bucket_idx = 0;
    uint32 bucket_size;
    uint32 actions_size;
    uint32 req_len;
    void* fei_group = NULL;
    void* p_fei_group = NULL;
    fei_openflow_add_group_req_t* p_req = NULL;
    glb_openflow_bucket_t* p_bucket = NULL;

    /* BUCKETS CHUNK. */
    while (remained_bucket_count) 
    {
        if (remained_bucket_count >= GLB_OPENFLOW_PARTIAL_GROUP_BUCKET_COUNT)
        {
            chunked_bucket_count = GLB_OPENFLOW_PARTIAL_GROUP_BUCKET_COUNT;
        }
        else
        {
            chunked_bucket_count = remained_bucket_count;
        }

        actions_size = 0;
        for (bucket_idx = 0; bucket_idx < chunked_bucket_count; bucket_idx ++)
        {
            p_bucket = &p_group->bucket_list[copied_bucket_count + bucket_idx];
            actions_size += p_bucket->n_actions * sizeof(glb_openflow_flow_action_t);
        }

        /* malloc p_req and bucket info */
        req_len = sizeof(fei_openflow_add_group_req_t);
        bucket_size = chunked_bucket_count * sizeof(glb_openflow_bucket_t);
        p_fei_group = malloc(req_len + bucket_size + actions_size);
        sal_memset(p_fei_group, 0, req_len + bucket_size + actions_size);

        fei_group = p_fei_group;
        p_req = (fei_openflow_add_group_req_t*)fei_group;
        p_req->partial_type = FEI_OPENFLOW_GROUP_PARTIAL_TYPE_BUCKET;
        p_req->u.bucket.buckets_len = chunked_bucket_count;
        fei_group += sizeof(fei_openflow_add_group_req_t);

        /* malloc bucket actions */
        for (bucket_idx = 0; bucket_idx < chunked_bucket_count; bucket_idx ++)
        {
            p_bucket = &p_group->bucket_list[copied_bucket_count + bucket_idx];
            sal_memcpy(fei_group, p_bucket, sizeof(glb_openflow_bucket_t));
            fei_group += sizeof(glb_openflow_bucket_t);

            sal_memcpy(fei_group, p_bucket->actlist, p_bucket->n_actions * sizeof(glb_openflow_flow_action_t));
            fei_group += p_bucket->n_actions * sizeof(glb_openflow_flow_action_t);
        }

        copied_bucket_count = copied_bucket_count + chunked_bucket_count;
        remained_bucket_count = remained_bucket_count - chunked_bucket_count;

        fei_talk(FEI_OPENFLOW_GROUP_ADD, p_fei_group, req_len + bucket_size + actions_size);

        if (p_fei_group)
        {
            free(p_fei_group);
        }
    }

    return 0;
}

/*************************************************************************************************
 * Name         : _fei_group_send_group_info_tail
 * Purpose      : Fei layer group send group info tail
 * Input        : p_group: glb_openflow_group_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
uint32
_fei_group_send_group_info_tail(glb_openflow_group_t* p_group)
{
    fei_openflow_add_group_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_add_group_req_t));

    req.partial_type = FEI_OPENFLOW_GROUP_PARTIAL_TYPE_TAIL;
    /* message content is useless. */
    return fei_talk(FEI_OPENFLOW_GROUP_ADD, &req, sizeof(req));
}

/*************************************************************************************************
 * Name         : fei_group_add_group
 * Purpose      : Fei layer group add group
 * Input        : p_group: glb_openflow_group_t*
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_group_add_group(glb_openflow_group_t* p_group)
{
    _fei_group_send_group_info_head(p_group);
    _fei_group_send_group_info_bucket(p_group);
    _fei_group_send_group_info_tail(p_group);
    
    return 0;
}

/*************************************************************************************************
 * Name         : fei_group_del_group
 * Purpose      : Fei layer group delete group
 * Input        : group_id: uint32
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_group_del_group(uint32 group_id)
{
    fei_openflow_del_group_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_del_group_req_t));
    req.group_id = group_id;

    return fei_talk(FEI_OPENFLOW_GROUP_DEL, &req, sizeof(req));
}

/*************************************************************************************************
 * Name         : fei_group_clear_stats
 * Purpose      : Fei layer group clear group stats
 * Input        : group_id: uint32
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
int32
fei_group_clear_stats(uint32 group_id)
{
    fei_openflow_clear_stats_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_clear_stats_req_t));

    req.res_type = FEI_OPENFLOW_STATS_RES_TYPE_GROUP;
    req.id = group_id;

    return fei_talk(FEI_OPENFLOW_STATS_CLEAR, &req, sizeof(req));
}

/*************************************************************************************************
 * Name         : fei_group_set_drop_ingress
 * Purpose      : Fei layer group set drop ingress
 * Input        : is_group: uint8, is_drop: uint8
 * Output       : N/A 
 * Return       : OFP_ERR_XXX
**************************************************************************************************/
/* Added by weizj for bug 35135 */
int32
fei_group_set_drop_ingress(uint8 is_group, uint8 is_drop)
{
    fei_openflow_set_drop_ingress req;

    sal_memset(&req, 0, sizeof(fei_openflow_set_drop_ingress));

    req.is_group = is_group;
    req.is_drop = is_drop;

    return fei_talk(FEI_OPENFLOW_FLOW_DROP_INGRESS, &req, sizeof(req));
}

