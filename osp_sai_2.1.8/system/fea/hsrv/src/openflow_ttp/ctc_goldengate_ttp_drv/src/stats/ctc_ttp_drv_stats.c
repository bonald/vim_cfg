
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_stats.h"

int32
ctc_ttp_drv_stats_create(hsrv_openflow_ttp_stats_type_t stats_type, uint32 *stats_id)
{
    int32 ret = 0;
    ctc_stats_statsid_t     ctc_stats;

    HSRV_OPENFLOW_PTR_CHECK(stats_id);
    sal_memset(&ctc_stats, 0, sizeof(ctc_stats_statsid_t));
    switch(stats_type)
    {
    case HSRV_OPENFLOW_TTP_STATS_TYPE_SCL:
        ctc_stats.dir = CTC_INGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_SCL;
        break;
    case HSRV_OPENFLOW_TTP_STATS_TYPE_ACL:
        ctc_stats.dir = CTC_INGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_ACL;
        break;
    case HSRV_OPENFLOW_TTP_STATS_TYPE_NEXTHOP:
        ctc_stats.dir = CTC_EGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_NEXTHOP;
        break;
    case HSRV_OPENFLOW_TTP_STATS_TYPE_ECMP:
        ctc_stats.dir = CTC_EGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_ECMP;
        break;
    case HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST:
        ctc_stats.dir = CTC_EGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
        break;
    case HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL:
        ctc_stats.dir = CTC_INGRESS;
        ctc_stats.type = CTC_STATS_STATSID_TYPE_TUNNEL;
        break;
    default:
        return 0;
        break;
    }
    
    ret = ctc_stats_create_statsid(&ctc_stats);
    if (ret)
    {
        return ret;
    }
    *stats_id = ctc_stats.stats_id;
    
    return ret;
}

int32
ctc_ttp_drv_stats_destory(uint32 stats_id)
{
    return ctc_stats_destroy_statsid(stats_id);
}

int32
ctc_ttp_drv_stats_clear(uint32 stats_id)
{
    return ctc_stats_clear_stats(stats_id);
}

int32
ctc_ttp_drv_stats_get(hsrv_openflow_ttp_stats_t *stats_info, uint32 stats_id)
{
    int32 ret = 0;
    ctc_stats_basic_t       basic_stats;

    HSRV_OPENFLOW_PTR_CHECK(stats_info);
    sal_memset(&basic_stats, 0, sizeof(ctc_stats_basic_t));

    ret = ctc_stats_get_stats(stats_id, &basic_stats);
    if (ret)
    {
        return ret;
    }
    stats_info->byte_count = basic_stats.byte_count;
    stats_info->packet_count = basic_stats.packet_count;
    
    return ret;
}

