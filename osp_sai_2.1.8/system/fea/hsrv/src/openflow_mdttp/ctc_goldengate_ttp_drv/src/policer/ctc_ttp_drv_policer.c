
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_policer.h"

extern void 
ctc_sai_policer_alloc_policer_id(uint32* p_policer_id);

extern void 
ctc_sai_policer_release_policer_id(uint32 policer_id);

void
ctc_ttp_drv_policer_id_alloc(uint32 *policer_id)
{
    ctc_sai_policer_alloc_policer_id(policer_id);
}

void
ctc_ttp_drv_policer_id_free(uint32 policer_id)
{
    ctc_sai_policer_release_policer_id(policer_id);
}

int32
ctc_ttp_drv_policer_set(glb_openflow_meter_t  *p_config, uint32 policer_id)
{
    ctc_qos_policer_t   ctc_policer;
    ctc_qos_policer_param_t* p_policer = &ctc_policer.policer;

    sal_memset(&ctc_policer, 0 , sizeof(ctc_qos_policer_t));

    ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;
    ctc_policer.id.policer_id = policer_id;

    if (p_config->stats_enable)
    {
        p_policer->stats_en = p_config->stats_enable;
    }
    ctc_policer.enable = 1;
    
    /* use strcm */
    p_policer->policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
    p_policer->is_srtcm = 1;

    /* color blind */
    p_policer->is_color_aware = 0;

    /* drop color */
    p_policer->drop_color = CTC_QOS_COLOR_YELLOW;

    /* cir */
    p_policer->cir = p_config->rate;
    /* cbs */
    p_policer->cbs = p_config->burst_size;

    return ctc_qos_set_policer(&ctc_policer);
}

int32
ctc_ttp_drv_policer_remove(uint32 policer_id)
{
    ctc_qos_policer_t   ctc_policer;

    sal_memset(&ctc_policer, 0 , sizeof(ctc_qos_policer_t));

    ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;
    ctc_policer.id.policer_id = policer_id;
    ctc_policer.enable = 0;

    return ctc_qos_set_policer(&ctc_policer);
}

int32
ctc_ttp_drv_policer_get_stats(uint32 policer_id, hsrv_openflow_ttp_meter_stats_t *p_meter_stats)
{
    int32   rc = 0;
    ctc_qos_policer_stats_t policer_stats;
    ctc_qos_policer_stats_info_t    *p_stats = NULL;
    
    sal_memset(&policer_stats, 0, sizeof(ctc_qos_policer_stats_t));
    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    policer_stats.id.policer_id = policer_id;

    rc = ctc_qos_query_policer_stats(&policer_stats);
    if (rc)
    {
        return rc;
    }
    p_stats = &policer_stats.stats;

    p_meter_stats->confirm_stats.packet_count = p_stats->confirm_pkts;
    p_meter_stats->confirm_stats.byte_count = p_stats->confirm_bytes;
    p_meter_stats->exceed_stats.packet_count = p_stats->exceed_pkts;
    p_meter_stats->exceed_stats.byte_count = p_stats->exceed_bytes;
    p_meter_stats->violate_stats.packet_count = p_stats->violate_pkts;
    p_meter_stats->violate_stats.byte_count = p_stats->violate_bytes;

    return rc;
}

int32
ctc_ttp_drv_policer_clear_stats(uint32 policer_id)
{
    ctc_qos_policer_stats_t policer_stats;

    sal_memset(&policer_stats, 0, sizeof(ctc_qos_policer_stats_t));

    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    policer_stats.id.policer_id = policer_id;
    
    return ctc_qos_clear_policer_stats(&policer_stats);
}

