#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_stats_priv.h"
#include "hsrv_openflow_stats.h"

hsrv_openflow_stats_master_t* g_pst_openflow_stats_master;

#define _____DB______
static inline uint32
_hsrv_openflow_stats_info_hash_make(void* p_info_void)
{
    hsrv_of_stats_t* p_info = NULL;

    p_info = p_info_void;
    return (uint32) p_info->stats_id;
}

static inline bool
_hsrv_openflow_stats_info_hash_compare(void* p_info_in_bucket_void, void* p_info_to_lkp_void)
{
    hsrv_of_stats_t* p_info_in_bucket = NULL;
    hsrv_of_stats_t* p_info_to_lkp = NULL;

    p_info_in_bucket = p_info_in_bucket_void;
    p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->stats_id != p_info_to_lkp->stats_id)
    {
        return FALSE;
    }

    return TRUE;
}

static inline hsrv_of_stats_t*
_hsrv_openflow_stats_calloc_stats_info (void)
{
    return (hsrv_of_stats_t*) HSRV_OPENFLOW_STATS_CALLOC( sizeof(hsrv_of_stats_t));
}

static inline int32
_hsrv_openflow_stats_free_stats_info (hsrv_of_stats_t* p_stats_info)
{
    HSRV_OPENFLOW_PTR_CHECK(p_stats_info);

    HSRV_OPENFLOW_STATS_FREE(p_stats_info);

    return HSRV_E_NONE;
}

#define _____MESSAGE_API_____
int32
hsrv_openflow_msg_create_stats(hsrv_of_stats_t* p_stats)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowAddStatsReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddStats;
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddStats;
    p_req->statsId = p_stats->stats_id;
    p_req->resourceId = p_stats->stats.resource_id;

    switch (p_stats->resource_type)
    {
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_flowResource;
            p_req->isMplsKey = p_stats->is_mpls_key;
            p_req->isMplsVc = p_stats->is_mpls_vc;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_meterResource;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_groupResource;
            switch (p_stats->resource_alloc_type.sub_type.group_alloc_type)
            {
                case GLB_OF_GROUP_ALLOC_TYPE_MCAST:
                    p_req->resourceAllocType.unionResourceAllocType.groupAllocType = HalMsgOpenFlowGroupAllocType_groupAllocTypeMcast;
                break;
                case GLB_OF_GROUP_ALLOC_TYPE_ECMP:
                    p_req->resourceAllocType.unionResourceAllocType.groupAllocType = HalMsgOpenFlowGroupAllocType_groupAllocTypeEcmp;
                break;
                default:
                    return HSRV_E_INVALID_PARAM;
                break;
            }
            break;
        default:
            return HSRV_E_INVALID_PARAM;
            break;
    }
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    /* Added by weizj for meter, statsid is allocated by sdk */
    if (HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER == p_stats->resource_type)
    {
        return HSRV_E_NONE;
    }

    /* stats db operation */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_stats_add_stats(p_req->statsId, p_stats));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_del_stats(hsrv_of_stats_t* p_stats)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDelStatsReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelStats;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelStats;
    p_req->statsId = p_stats->stats_id;
    p_req->resourceId = p_stats->stats.resource_id;

    switch (p_stats->resource_type)
    {
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_flowResource;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_meterResource;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_groupResource;
            break;
        default:
            return HSRV_E_INVALID_PARAM;
            break;
    }
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    /* Added by weizj for meter, statsid is allocated by sdk */
    if (HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER == p_stats->resource_type)
    {
        return HSRV_E_NONE;
    }

    /* stats db operation */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_stats_del_stats(p_req->statsId));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_clear_stats(hsrv_of_stats_t* p_stats)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowClearStatsReq_t* p_req;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowClearStats;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowClearStats;
    p_req->statsId = p_stats->stats_id;
    p_req->resourceId = p_stats->stats.resource_id;

    switch (p_stats->resource_type)
    {
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_flowResource;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_meterResource;
            break;
        case HSRV_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            p_req->resourceType = HalMsgOpenFlowStatsResourceType_groupResource;
            break;
        default:
            return HSRV_E_INVALID_PARAM;
            break;
    }

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

#define _____MODULE_API_____

int32
hsrv_openflow_stats_add_stats(uint32 stats_id, hsrv_of_stats_t *p_stats_info)
{
    hsrv_of_stats_t *p_new_stats_info = NULL;
    p_new_stats_info = _hsrv_openflow_stats_calloc_stats_info();
    p_new_stats_info->stats_id = stats_id;
    p_new_stats_info->stats.resource_id = p_stats_info->stats.resource_id;
    p_new_stats_info->resource_type = p_stats_info->resource_type;

    p_stats_info = ctclib_hash_get(HSRV_OF_STATS_HASH, p_new_stats_info,
            ctclib_hash_alloc_intern);
    if (!p_stats_info)
    {
        HSRV_OPENFLOW_STATS_FREE(p_new_stats_info);
        return HSRV_E_NO_MEMORY;
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_stats_del_stats(uint32 stats_id)
{
    hsrv_of_stats_t stats_info;
    hsrv_of_stats_t *p_stats_info;

    memset(&stats_info, 0, sizeof(hsrv_of_stats_t));
    stats_info.stats_id = stats_id;
    p_stats_info = ctclib_hash_lookup(HSRV_OF_STATS_HASH, &stats_info);
    if (p_stats_info)
    {
        ctclib_hash_release(HSRV_OF_STATS_HASH, p_stats_info);
        _hsrv_openflow_stats_free_stats_info(p_stats_info);
    }
    return HSRV_E_NONE;
}

void*
hsrv_openflow_stats_get_stats(uint32 stats_id)
{
    hsrv_of_stats_t stats_info;

    memset(&stats_info, 0, sizeof(hsrv_of_stats_t));
    stats_info.stats_id = stats_id;
    return ctclib_hash_lookup(HSRV_OF_STATS_HASH, &stats_info);
}

#define _____DEBUG_____

static void
_hsrv_openflow_show_stats_info__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_of_stats_t* p_stats = (hsrv_of_stats_t*) phb->data;
    hsrv_openflow_stats_debug_data_t* p_data = user_data;

    sal_fprintf(p_data->fp, "%5u %8u %13s %11u\n",
            p_data->i,
            p_stats->stats_id,
            HSRV_OF_STATS_TYPE_STR(p_stats->resource_type),
            p_stats->stats.resource_id);
    p_data->i ++;
}

int32
hsrv_openflow_show_stats(FILE *fp)
{
    hsrv_openflow_stats_debug_data_t data;

    sal_fprintf(fp, "-------------HSRV OPENFLOW STATS------------------\n");
    sal_fprintf(fp, "%5s %8s %13s %11s\n",
            "index",
            "stats_id",
            "resource_type",
            "resource_id");
    sal_fprintf(fp, "--------------------------------------------------\n");

    data.fp = fp;
    data.i  = 0;

    ctclib_hash_iterate(HSRV_OF_STATS_HASH, _hsrv_openflow_show_stats_info__, &data);

    return HSRV_E_NONE;
}

#define _____INIT_____

int32
hsrv_openflow_stats_start(void)
{
    g_pst_openflow_stats_master = HSRV_OPENFLOW_STATS_CALLOC(sizeof(hsrv_openflow_stats_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_stats_master);

    HSRV_OF_STATS_HASH = ctclib_hash_create(
            _hsrv_openflow_stats_info_hash_make,
            _hsrv_openflow_stats_info_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OF_STATS_HASH);

    g_pst_openflow_stats_master->pst_openflow_stats_id_opf = ctclib_opf_init(1); 
    ctclib_opf_init_offset (g_pst_openflow_stats_master->pst_openflow_stats_id_opf, 0, 1, 16384);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_stats_init(hsrv_system_spec_info_t* p_system_info)
{
    return HSRV_E_NONE;
}

#define _____END_LINE_____
