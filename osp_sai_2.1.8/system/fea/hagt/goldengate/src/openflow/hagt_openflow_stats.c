
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "hagt_inc.h"

#include "hagt_openflow.h"
#include "hagt_openflow_stats.h"
#include "hagt_openflow_stats_db.h"

/****************************************************************************************
 *
 * Globals
 *
 ****************************************************************************************/
extern hagt_openflow_master_t *g_pst_hagt_openflow_master;

/****************************************************************************************
 *
 * Function
 *
 ****************************************************************************************/
uint32
hagt_openflow_stats_get_ctc_stats_id(uint32 openflow_stats_id)
{
    hagt_openflow_statsid_t* p_openflow_statsid = NULL;
    hagt_openflow_statsid_t openflow_statsid;

    sal_memset(&openflow_statsid, 0x0, sizeof(hagt_openflow_statsid_t));
    openflow_statsid.stats_id = openflow_stats_id;
    p_openflow_statsid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_statsid_hash, &openflow_statsid);
    if(p_openflow_statsid)
    {
        return p_openflow_statsid->ctc_stats_id;
    }

    return 0;
}


int32
hagt_openflow_add_stats(Hsrv2AgtMsg_t* p_msg_data)
{
    int32 ret;
    int32 resource_id;
    hagt_openflow_stats_info_t openflow_stats_info;
    hagt_openflow_stats_resource_type_t resource_type;
    HalMsgOpenFlowAddStatsReq_t *req;
    ctc_stats_statsid_t statsid;
    hagt_openflow_statsid_t *p_new_stats_info = NULL;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddStats;
    sal_memset(&statsid, 0x0, sizeof(ctc_stats_statsid_t));

    switch (req->resourceType)
    {
        case HalMsgOpenFlowStatsResourceType_flowResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW;
            if (req->isMplsKey)
            {
                statsid.type = CTC_STATS_STATSID_TYPE_MPLS;
                if (req->isMplsVc)
                {
                    statsid.statsid.is_vc_label = TRUE;
                }
            }
            else
            {
                statsid.type = CTC_STATS_STATSID_TYPE_ACL;
            }
            break;
        case HalMsgOpenFlowStatsResourceType_meterResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER;
            break;
        case HalMsgOpenFlowStatsResourceType_groupResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP;
            switch (req->resourceAllocType.unionResourceAllocType.groupAllocType)
            {
                case HalMsgOpenFlowGroupAllocType_groupAllocTypeMcast:
                    statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
                    break;
                case HalMsgOpenFlowGroupAllocType_groupAllocTypeEcmp:
                    statsid.type = CTC_STATS_STATSID_TYPE_ECMP;
                    break;
                default:
                    return HAGT_E_INVALID_PARAM;
            }
            break;
        default:
            return HAGT_E_INVALID_PARAM;
        break;
    }

    /* Added by weizj for meter, statsid is allocated by sdk */
    if (HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER == resource_type)
    {
        resource_id = req->resourceId;

        sal_memset(&openflow_stats_info, 0x0, sizeof(hagt_openflow_stats_info_t));
        openflow_stats_info.stats.resource_id = resource_id;
        ret = hagt_openflow_statsdb_add_stats(resource_type, resource_id, &openflow_stats_info);

        return ret;
    }
    /* End by weizj for meter */
    
    statsid.dir = CTC_INGRESS;
    //statsid.stats_id = req->statsId;
    

    p_new_stats_info = (hagt_openflow_statsid_t*)HAGT_OPENFLOW_CALLOC( sizeof(hagt_openflow_statsid_t));
    p_new_stats_info->stats_id = req->statsId;
    ctclib_hash_get(g_pst_hagt_openflow_master->pst_openflow_statsid_hash, p_new_stats_info,
                    ctclib_hash_alloc_intern);
    HAGT_IF_ERROR_RETURN(ctc_stats_create_statsid(&statsid));
    HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(statsid.stats_id));
    p_new_stats_info->ctc_stats_id = statsid.stats_id;
    
    resource_id = req->resourceId;

    sal_memset(&openflow_stats_info, 0x0, sizeof(hagt_openflow_stats_info_t));
    openflow_stats_info.stats_id = req->statsId;
    openflow_stats_info.stats.resource_id = resource_id;
    ret = hagt_openflow_statsdb_add_stats(resource_type, resource_id, &openflow_stats_info);
    return ret;
}

int32
hagt_openflow_del_stats(Hsrv2AgtMsg_t* p_msg_data)
{
    int32 resource_id;
    hagt_openflow_stats_resource_type_t resource_type;
    HalMsgOpenFlowDelStatsReq_t *req;
    uint32 ctc_stats_id = 0;
    hagt_openflow_statsid_t* p_openflow_statsid = NULL;
    hagt_openflow_statsid_t openflow_statsid;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelStats;

    resource_id = req->resourceId;
    switch (req->resourceType)
    {
        case HalMsgOpenFlowStatsResourceType_flowResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW;
            break;
        case HalMsgOpenFlowStatsResourceType_meterResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER;
            break;
        case HalMsgOpenFlowStatsResourceType_groupResource:
            resource_type = HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP;
            break;
        default:
            return HAGT_E_INVALID_PARAM;
        break;
    }

    hagt_openflow_statsdb_del_stats(resource_type, resource_id);

    /* Added by weizj for meter, statsid is allocated by sdk */
    if (HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER == resource_type)
    {
        return HAGT_E_NONE;
    }
    /* End by weizj for meter */

    sal_memset(&openflow_statsid, 0x0, sizeof(hagt_openflow_statsid_t));
    openflow_statsid.stats_id = req->statsId;
    p_openflow_statsid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_statsid_hash, &openflow_statsid);
    if(p_openflow_statsid)
    {
        ctc_stats_id = p_openflow_statsid->ctc_stats_id;
        ctclib_hash_release(g_pst_hagt_openflow_master->pst_openflow_statsid_hash, p_openflow_statsid);
        HAGT_OPENFLOW_FREE(p_openflow_statsid);
        HAGT_IF_ERROR_RETURN(ctc_stats_destroy_statsid(ctc_stats_id));
    }

    return HAGT_E_NONE;
}

static int32
_hagt_openflow_get_stats(uint32 stats_id, glb_openflow_stats_t *p_stats)
{
    ctc_stats_basic_t stats;
    uint32 ctc_stats_id = 0;

    memset(&stats, 0, sizeof(ctc_stats_basic_t));

    if (stats_id != CTC_STATS_MAX_STATSID)
    {
        ctc_stats_id = hagt_openflow_stats_get_ctc_stats_id(stats_id);
        HAGT_IF_ERROR_RETURN(ctc_stats_get_stats(ctc_stats_id, &stats));
        p_stats->byte_count   = stats.byte_count;
        p_stats->packet_count = stats.packet_count;
    }
    return HAGT_E_NONE;
}

int32
hagt_openflow_clear_stats(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowClearStatsReq_t *req;
    hagt_openflow_stats_info_t* p_stats_info;
    uint32 ctc_stats_id = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowClearStats;

    p_stats_info = hagt_openflow_statsdb_get_stats(req->resourceType, req->resourceId);
    if (!p_stats_info)
    {
        return HAGT_E_ENTRY_NOT_EXIST;
    }

    /* Added by weizj for meter, statsid is allocated by sdk */
    if (HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER == req->resourceType)
    {
        p_stats_info->stats.confirm_bytes = 0;
        p_stats_info->stats.confirm_packets = 0;
        p_stats_info->stats.exceed_bytes = 0;
        p_stats_info->stats.exceed_packets = 0;
        p_stats_info->stats.violate_bytes = 0;
        p_stats_info->stats.violate_packets = 0;

        HAGT_IF_ERROR_RETURN(hagt_openflow_meter_clear_stats(req->resourceId));

        return HAGT_E_NONE;
    }

    p_stats_info->stats.byte_count = 0;
    p_stats_info->stats.packet_count = 0;
    ctc_stats_id = hagt_openflow_stats_get_ctc_stats_id(req->statsId);
    HAGT_IF_ERROR_RETURN(ctc_stats_clear_stats(ctc_stats_id));
    
    return HAGT_E_NONE;
}

static bool
_hagt_openflow_fetch_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id)
{
    glb_openflow_stats_t stats;
    glb_qos_policer_stats_t new_policer_stats;
    hagt_openflow_stats_info_t* p_stats_info;

    p_stats_info = hagt_openflow_statsdb_get_stats(resource_type, resource_id);

    if (!p_stats_info)
    {
        return FALSE;
    }

    sal_memset(&stats, 0, sizeof(stats));

    /* Added by weizj for meter */
    if (HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER == resource_type)
    {
        sal_memset(&new_policer_stats, 0, sizeof(glb_qos_policer_stats_t));

        HAGT_IF_ERROR_RETURN(hagt_openflow_meter_get_stats(resource_id, &new_policer_stats));

        if (p_stats_info->stats.confirm_packets == new_policer_stats.confirm_stats.packet_count && 
            p_stats_info->stats.exceed_packets == new_policer_stats.exceed_stats.packet_count &&
            p_stats_info->stats.violate_packets == new_policer_stats.violate_stats.packet_count)
        {
            return FALSE;
        }

        p_stats_info->stats.confirm_packets = new_policer_stats.confirm_stats.packet_count;
        p_stats_info->stats.confirm_bytes = new_policer_stats.confirm_stats.byte_count;
        p_stats_info->stats.exceed_packets = new_policer_stats.exceed_stats.packet_count;
        p_stats_info->stats.exceed_bytes = new_policer_stats.exceed_stats.byte_count;
        p_stats_info->stats.violate_packets = new_policer_stats.violate_stats.packet_count;
        p_stats_info->stats.violate_bytes = new_policer_stats.violate_stats.byte_count;

        return TRUE;
    }

    HAGT_IF_ERROR_RETURN(_hagt_openflow_get_stats(p_stats_info->stats_id, &stats));
    if ((p_stats_info->stats.byte_count == stats.byte_count) && 
        (p_stats_info->stats.packet_count == stats.packet_count))
    {
        return FALSE;
    }

    p_stats_info->stats.byte_count = stats.byte_count;
    p_stats_info->stats.packet_count = stats.packet_count;

    return TRUE;
}

static int32
_hagt_openflow_flow_stats_traverse_item(ctclib_hash_backet_t* phb, int cur_index, void *arg1, void *arg2)
{
    bool need_notify = TRUE;
    uint32 agent_id = 0;
    Hagt2SrvMsg_t hagt_msg;
    hagt_openflow_stats_info_t* pst_stats_info = NULL;
    int *loop_index = (int *)arg1;
    uint32 *loop_count = (uint32 *)arg2;

    if (!phb)
    {
        goto COUNT_CHECK;
    }
    
    pst_stats_info = (hagt_openflow_stats_info_t*) phb->data;

    need_notify = _hagt_openflow_fetch_stats(HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW, pst_stats_info->stats.resource_id);

    if (need_notify)
    {
        uint8 tmp_stats[HAGT_OPENFLOW_FLOW_STATS_MAX_ITEM][8];
        agent_id = 0; /*TODO: get agent_id */
        sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
        hagt_msg.agentId = agent_id;
        hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = GLB_STATS_TYPE_OPENFLOW_FLOW;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.notify = TRUE;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.present = HalMsgStatsResult_PR_openflowRslt;
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.packet_count, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.pkts, tmp_stats[0]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.byte_count, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.octets, tmp_stats[1]);
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.resourceId = pst_stats_info->stats.resource_id;
        HAGT2SRV_MSG_SEND(&hagt_msg);
    }

COUNT_CHECK:
    /* Break iterate if it have done enough. */
    *loop_count = *loop_count + 1;
    if (*loop_count >= HAGT_OPENFLOW_FLOW_STATS_POLLING_ONE_ROUND)
    {
        *loop_index = cur_index;
        return -1;
    }    
    
    return 0;
}

/* Added by weizj for meter */
static int32
_hagt_openflow_meter_stats_traverse_item(ctclib_hash_backet_t* phb, int cur_index, void *arg1, void *arg2)
{
    bool need_notify = TRUE;
    uint32 agent_id = 0;
    Hagt2SrvMsg_t hagt_msg;
    hagt_openflow_stats_info_t* pst_stats_info = NULL;
    int *loop_index = (int *)arg1;
    uint32 *loop_count = (uint32 *)arg2;

    if (!phb)
    {
        goto COUNT_CHECK;
    }

    pst_stats_info = (hagt_openflow_stats_info_t*) phb->data;

    need_notify = _hagt_openflow_fetch_stats(HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER, pst_stats_info->stats.resource_id);

    if (need_notify)
    {
        uint8 tmp_stats[HAGT_OPENFLOW_METER_STATS_MAX_ITEM][8];
        agent_id = 0; /*TODO: get agent_id */
        sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
        hagt_msg.agentId = agent_id;
        hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = GLB_STATS_TYPE_OPENFLOW_METER;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.notify = TRUE;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.present = HalMsgStatsResult_PR_policerRslt;
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.confirm_bytes, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.confirmBytes, tmp_stats[0]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.confirm_packets, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.confirmPackets, tmp_stats[1]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.exceed_bytes, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.exceedBytes, tmp_stats[2]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.exceed_packets, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.exceedPackets, tmp_stats[3]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.violate_bytes, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.violateBytes, tmp_stats[4]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.violate_packets, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.violatePackets, tmp_stats[5]);
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.policerRslt.resourceId = pst_stats_info->stats.resource_id;
        HAGT2SRV_MSG_SEND(&hagt_msg);
    }

COUNT_CHECK:
    /* Break iterate if it have done enough. */
    *loop_count = *loop_count + 1;
    if (*loop_count >= HAGT_OPENFLOW_METER_STATS_POLLING_ONE_ROUND)
    {
        *loop_index = cur_index;
        return -1;
    }    
    
    return 0;
}
/* End by weizj for meter */

static int32
_hagt_openflow_group_stats_traverse_item(ctclib_hash_backet_t* phb, int cur_index, void *arg1, void *arg2)
{
    bool need_notify = TRUE;
    uint32 agent_id = 0;
    Hagt2SrvMsg_t hagt_msg;
    hagt_openflow_stats_info_t* pst_stats_info = NULL;
    int *loop_index = (int *)arg1;
    uint32 *loop_count = (uint32 *)arg2;

    if (!phb)
    {
        goto COUNT_CHECK;
    }

    pst_stats_info = (hagt_openflow_stats_info_t*) phb->data;

    need_notify = _hagt_openflow_fetch_stats(HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP, pst_stats_info->stats.resource_id);

    if (need_notify)
    {
        uint8 tmp_stats[HAGT_OPENFLOW_GROUP_STATS_MAX_ITEM][8];
        agent_id = 0; /*TODO: get agent_id */
        sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
        hagt_msg.agentId = agent_id;
        hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = GLB_STATS_TYPE_OPENFLOW_GROUP;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.notify = TRUE;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.present = HalMsgStatsResult_PR_openflowRslt;
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.packet_count, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.pkts, tmp_stats[0]);
        HAGT_OPENFLOW_UINT64_TO_OCTET_STR(pst_stats_info->stats.byte_count, hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.octets, tmp_stats[1]);
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.openflowRslt.resourceId = pst_stats_info->stats.resource_id;
        HAGT2SRV_MSG_SEND(&hagt_msg);
    }

COUNT_CHECK:
    /* Break iterate if it have done enough. */
    *loop_count = *loop_count + 1;
    if (*loop_count >= HAGT_OPENFLOW_GROUP_STATS_POLLING_ONE_ROUND)
    {
        *loop_index = cur_index;
        return -1;
    }    
    
    return 0;
}

static void
_hagt_flow_stats_poll_handler(void* user_param)
{
    /* Do consecutive poll for each flow using static value. */
    static int loop_index = 0;
    uint32 count_per_on_iteration = 0;

    /* XXX: implement a adaptive poller to get rid of these small optizimation. */
    if (!g_pst_hagt_openflow_master->pst_flow_stats_info_hash->size)
    {
        goto REACTIVATE_TIMER;
    }

    ctclib_hash_iterate2_cont_round(g_pst_hagt_openflow_master->pst_flow_stats_info_hash, loop_index,
                                    _hagt_openflow_flow_stats_traverse_item, 
                                    &loop_index, &count_per_on_iteration);

REACTIVATE_TIMER:
    /* Reactivate the timer */
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_flow_stats_poll_handler, NULL, HAGT_OPENFLOW_FLOW_STATS_POLLING_INTERVAL);
}

/* Added by weizj for meter */
static void
_hagt_meter_stats_poll_handler(void* user_param)
{
    /* Do consecutive poll for each flow using static value. */
    static int loop_index = 0;
    uint32 count_per_on_iteration = 0;

    /* XXX: implement a adaptive poller to get rid of these small optizimation. */
    if (!g_pst_hagt_openflow_master->pst_meter_stats_info_hash)
    {
        goto REACTIVATE_TIMER;
    }

    ctclib_hash_iterate2_cont_round(g_pst_hagt_openflow_master->pst_meter_stats_info_hash, loop_index,
                                    _hagt_openflow_meter_stats_traverse_item, 
                                    &loop_index, &count_per_on_iteration);

REACTIVATE_TIMER:
    /* Reactivate the timer */
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_meter_stats_poll_handler, NULL, HAGT_OPENFLOW_METER_STATS_POLLING_INTERVAL);
}
/* End by weizj for meter */

static void
_hagt_group_stats_poll_handler(void* user_param)
{
    /* Do consecutive poll for each flow using static value. */
    static int loop_index = 0;
    uint32 count_per_on_iteration = 0;

    if (!g_pst_hagt_openflow_master->pst_group_stats_info_hash->size)
    {
        goto REACTIVATE_TIMER;
    }
    
    ctclib_hash_iterate2_cont_round(g_pst_hagt_openflow_master->pst_group_stats_info_hash, loop_index,
                                    _hagt_openflow_group_stats_traverse_item, 
                                    &loop_index, &count_per_on_iteration);

REACTIVATE_TIMER:    
    /* Reactivate the timer */
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_group_stats_poll_handler, NULL, HAGT_OPENFLOW_GROUP_STATS_POLLING_INTERVAL);
}

int32
_hagt_openflow_flow_stats_init(void)
{
    if (sal_mutex_create(&g_pst_hagt_openflow_master->p_flow_stats_mutex))
    {
        HAGT_OPENFLOW_LOG_ERR("Create openflow flow stats mutex fail.");
        goto err_handle;
    }

#if 0
    /* create flow stats polling thread */
    if (0 != sal_task_create(&g_pst_hagt_openflow_master->p_flow_stats_thread,
                             "openflow_flow_stats",
                             256*1024, SAL_TASK_PRIO_DEF+10, _hagt_flow_stats_poll_handler, NULL))
    {
        HAGT_LOG_CRIT("Create read openflow flow stats thread fail.");
        goto err_handle;
    }
#endif
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_flow_stats_poll_handler, NULL, HAGT_OPENFLOW_FLOW_STATS_POLLING_INTERVAL);

    return HAGT_E_NONE;

err_handle:
    if (g_pst_hagt_openflow_master->p_flow_stats_mutex)
    {
        sal_mutex_destroy(g_pst_hagt_openflow_master->p_flow_stats_mutex);
        g_pst_hagt_openflow_master->p_flow_stats_mutex = NULL;
    }

    return HAGT_E_NONE;
}

int32
_hagt_openflow_meter_stats_init(void)
{
    if (sal_mutex_create(&g_pst_hagt_openflow_master->p_meter_stats_mutex))
    {
        HAGT_OPENFLOW_LOG_ERR("Create openflow meter stats mutex fail.");
        goto err_handle;
    }
#if 0
    /* create meter stats polling thread */
    if (0 != sal_task_create(&g_pst_hagt_openflow_master->p_meter_stats_thread,
                             "openflow_meter_stats",
                             256*1024, SAL_TASK_PRIO_DEF+10, _hagt_meter_stats_poll_handler, NULL))
    {
        HAGT_LOG_CRIT("Create read openflow meter stats thread fail.");
        goto err_handle;
    }
#endif
    /* Added by weizj for meter */
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_meter_stats_poll_handler, NULL, HAGT_OPENFLOW_METER_STATS_POLLING_INTERVAL);

    return HAGT_E_NONE;

err_handle:
    if (g_pst_hagt_openflow_master->p_meter_stats_mutex)
    {
        sal_mutex_destroy(g_pst_hagt_openflow_master->p_meter_stats_mutex);
        g_pst_hagt_openflow_master->p_meter_stats_mutex = NULL;
    }

    return HAGT_E_NONE;
}

int32
_hagt_openflow_group_stats_init(void)
{
    if (sal_mutex_create(&g_pst_hagt_openflow_master->p_group_stats_mutex))
    {
        HAGT_OPENFLOW_LOG_ERR("Create openflow group stats mutex fail.");
        goto err_handle;
    }

#if 0
    /* create group stats polling thread */
    if (0 != sal_task_create(&g_pst_hagt_openflow_master->p_group_stats_thread,
                             "openflow_group_stats",
                             256*1024, SAL_TASK_PRIO_DEF+10, _hagt_group_stats_poll_handler, NULL))
    {
        HAGT_LOG_CRIT("Create read openflow group stats thread fail.");
        goto err_handle;
    }
    return HAGT_E_NONE;
#endif
    ctc_task_add_timer(TASK_PRI_LOW, _hagt_group_stats_poll_handler, NULL, HAGT_OPENFLOW_GROUP_STATS_POLLING_INTERVAL);

err_handle:
    if (g_pst_hagt_openflow_master->p_group_stats_mutex)
    {
        sal_mutex_destroy(g_pst_hagt_openflow_master->p_group_stats_mutex);
        g_pst_hagt_openflow_master->p_group_stats_mutex = NULL;
    }

    return HAGT_E_NONE;
}

#define _____INIT_____
int32
hagt_openflow_stats_init(void)
{
    hagt_openflow_statsdb_init();
    _hagt_openflow_flow_stats_init();
    _hagt_openflow_group_stats_init();
    /* Added by weizj for meter */
    _hagt_openflow_meter_stats_init();

    return HAGT_E_NONE;
}

int32
hagt_openflow_stats_register_callback(void)
{
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddStats, hagt_openflow_add_stats);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelStats, hagt_openflow_del_stats);
    //Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowGetStats, hagt_openflow_get_stats);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowClearStats, hagt_openflow_clear_stats);

#undef Reg
    return HAGT_E_NONE;
}
