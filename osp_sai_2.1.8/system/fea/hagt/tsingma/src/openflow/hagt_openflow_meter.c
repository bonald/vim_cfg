/* Added by weizj for meter */
#include "ctc_api.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_openflow.h"


#define ____METER_MODULE_APIs____
/*************************************************************************************************
 * Name         : hagt_openflow_meter_get_stats
 * Purpose      : Hagt layer openflow meter get meter stats
 * Input        : meter_id: uint32
 * Output       : p_meter_stats: glb_qos_policer_stats_t*
 * Return       : HAGT_E_XXX
**************************************************************************************************/
extern void ctc_sai_policer_alloc_policer_id(uint32* p_policer_id);

extern void ctc_sai_policer_release_policer_id(uint32 policer_id);

uint32
hagt_openflow_merter_get_ctc_policer_id(uint32 openflow_meter_id)
{
    hagt_openflow_policerid_t* p_openflow_policerid = NULL;
    hagt_openflow_policerid_t openflow_policerid;

    sal_memset(&openflow_policerid, 0x0, sizeof(hagt_openflow_policerid_t));
    openflow_policerid.meter_id = openflow_meter_id;
    p_openflow_policerid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_policerid_hash, &openflow_policerid);
    if(p_openflow_policerid)
    {
        return p_openflow_policerid->ctc_policer_id;
    }

    return 0;
}

int32
hagt_openflow_meter_get_stats(uint32 meter_id, glb_qos_policer_stats_t* p_meter_stats)
{
    ctc_qos_policer_stats_t policer_stats;
    ctc_qos_policer_stats_info_t* p_stats = NULL;
    uint32 ctc_policer_id = 0;

    sal_memset(&policer_stats, 0, sizeof(ctc_qos_policer_stats_t));
    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(meter_id);
    if(ctc_policer_id)
    {
        policer_stats.id.policer_id = ctc_policer_id;
    }

    //HAGT_IF_ERROR_RETURN(ctc_qos_query_policer_stats(&policer_stats));
    /* flow policer stats_ptr alloced when policer apply to acl */
    ctc_qos_query_policer_stats(&policer_stats);
    p_stats = &policer_stats.stats;

    p_meter_stats->confirm_stats.packet_count = p_stats->confirm_pkts;
    p_meter_stats->confirm_stats.byte_count = p_stats->confirm_bytes;
    p_meter_stats->exceed_stats.packet_count = p_stats->exceed_pkts;
    p_meter_stats->exceed_stats.byte_count = p_stats->exceed_bytes;
    p_meter_stats->violate_stats.packet_count = p_stats->violate_pkts;
    p_meter_stats->violate_stats.byte_count = p_stats->violate_bytes;

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_meter_clear_stats
 * Purpose      : Hagt layer openflow meter clear stats
 * Input        : meter_id: uint32
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
hagt_openflow_meter_clear_stats(uint32 meter_id)
{
    ctc_qos_policer_stats_t policer_stats;
    uint32 ctc_policer_id = 0;

    sal_memset(&policer_stats, 0, sizeof(ctc_qos_policer_stats_t));

    policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
    ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(meter_id);
    if(ctc_policer_id)
    {
        policer_stats.id.policer_id = ctc_policer_id;
    }
    //HAGT_IF_ERROR_RETURN(ctc_qos_clear_policer_stats(&policer_stats));
    /* flow policer stats_ptr alloced when policer apply to acl */
    ctc_qos_clear_policer_stats(&policer_stats);
    return HAGT_E_NONE;
}

#define ____METER_INTERNAL_API____
/*************************************************************************************************
 * Name         : _hagt_openflow_meter_map_meter_entry
 * Purpose      : Hagt layer openflow meter map meter entry
 * Input        : req: HalMsgOpenFlowMeterEntryKey_t *, is_del: bool
 * Output       : p_meter_entry: ctc_qos_policer_t
 * Return       : HAGT_E_XXX
**************************************************************************************************/
static int32
_hagt_openflow_meter_map_meter_entry(HalMsgOpenFlowMeterEntryKey_t *req,
    ctc_qos_policer_t *p_meter_entry, bool is_del)
{
    HalMsgOpenFlowMeterBandKey_t* p_band_info = NULL;
    ctc_qos_policer_param_t* p_policer = NULL;

    p_meter_entry->type = CTC_QOS_POLICER_TYPE_FLOW;
    p_meter_entry->id.policer_id = req->meterId;

    if (is_del)
    {
        p_meter_entry->enable = 0;

        return HAGT_E_NONE;
    }

    p_meter_entry->policer.stats_en = req->statsEn;
    p_meter_entry->enable = 1;

    /* mapping meter band */
    p_band_info = &req->meterBand;
    p_policer = &p_meter_entry->policer;

    /* use strcm */
    p_policer->policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
#ifndef USW
    p_policer->is_srtcm = 1;
#endif
    /* color blind */
    p_policer->is_color_aware = 0;

    /* drop color */
    p_policer->drop_color = CTC_QOS_COLOR_YELLOW;

    /* cir */
    p_policer->cir = p_band_info->rate;
    /* cbs */
    p_policer->cbs = p_band_info->burst;

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_meter_update_ipg_status
 * Purpose      : Hagt layer openflow meter update ipg status
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
static int32
_hagt_openflow_meter_update_ipg_status(Hsrv2AgtMsg_t* p_msg_data)
{
    ctc_qos_glb_cfg_t cfg;
    HalMsgOpenFlowMeterEntryKey_t *req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowUpdateMeterIpgStatus;
    sal_memset(&cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_IPG_EN;
    cfg.u.value = req->ipgEn;

    HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&cfg));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_meter_add_meter_entry
 * Purpose      : Hagt layer openflow meter add meter entry
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
static int32
_hagt_openflow_meter_add_meter_entry(Hsrv2AgtMsg_t* p_msg_data)
{   
    ctc_qos_policer_t meter_entry;
    HalMsgOpenFlowMeterEntryKey_t *req;
    hagt_openflow_policerid_t* p_new_policer_info =NULL;
    uint32 ctc_policer_id = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddMeterEntry;
    sal_memset(&meter_entry, 0, sizeof(ctc_qos_policer_t));

    ctc_policer_id = hagt_openflow_merter_get_ctc_policer_id(req->meterId);
    if(0 == ctc_policer_id)
    {
        ctc_sai_policer_alloc_policer_id(&ctc_policer_id);
        p_new_policer_info = (hagt_openflow_policerid_t*)HAGT_OPENFLOW_CALLOC( sizeof(hagt_openflow_policerid_t));
        p_new_policer_info->meter_id = req->meterId;
        p_new_policer_info->ctc_policer_id = ctc_policer_id;
        ctclib_hash_get(g_pst_hagt_openflow_master->pst_openflow_policerid_hash, p_new_policer_info,
                    ctclib_hash_alloc_intern);
    }
    HAGT_IF_ERROR_RETURN(_hagt_openflow_meter_map_meter_entry(req, &meter_entry, FALSE));
    meter_entry.id.policer_id = ctc_policer_id;
    HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&meter_entry));
    
    /* Added by weizj for meter bug 36265 */
    if (req->statsEn)
    {
        HAGT_IF_ERROR_RETURN(hagt_openflow_meter_clear_stats(req->meterId));
        /* flow policer stats_ptr alloced when policer apply to acl */
        hagt_openflow_meter_clear_stats(req->meterId);
    }

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_meter_delete_meter_entry
 * Purpose      : Hagt layer openflow meter delete meter entry
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
static int32
_hagt_openflow_meter_delete_meter_entry(Hsrv2AgtMsg_t* p_msg_data)
{   
    ctc_qos_policer_t meter_entry;
    HalMsgOpenFlowMeterEntryKey_t *req;
    hagt_openflow_policerid_t openflow_policerid;
    hagt_openflow_policerid_t* p_openflow_policerid = NULL;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelMeterEntry;
    sal_memset(&meter_entry, 0, sizeof(ctc_qos_policer_t));

    sal_memset(&openflow_policerid, 0, sizeof(hagt_openflow_policerid_t));
    openflow_policerid.meter_id = req->meterId;
    p_openflow_policerid = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_openflow_policerid_hash, &openflow_policerid);
    if(p_openflow_policerid)
    {
        HAGT_IF_ERROR_RETURN(_hagt_openflow_meter_map_meter_entry(req, &meter_entry, TRUE));
        meter_entry.id.policer_id = p_openflow_policerid->ctc_policer_id;
        HAGT_IF_ERROR_RETURN(ctc_qos_set_policer(&meter_entry));
        ctc_sai_policer_release_policer_id(p_openflow_policerid->ctc_policer_id);
        ctclib_hash_release(g_pst_hagt_openflow_master->pst_openflow_policerid_hash, p_openflow_policerid);
    }

    return HAGT_E_NONE;
}

#define ____METER_INIT_APIs____
/*************************************************************************************************
 * Name         : hagt_openflow_meter_init
 * Purpose      : Hagt layer openflow meter init to sdk
 * Input        : N/A
 * Output       : p_qos_cfg: ctc_qos_global_cfg_t*
 * Return       : void
**************************************************************************************************/
void
hagt_openflow_meter_init(ctc_qos_global_cfg_t* p_qos_cfg)
{
    ctc_qos_resrc_drop_profile_t *p_profile = NULL;

    p_qos_cfg->queue_num_per_network_port  = CTC_QOS_PORT_QUEUE_NUM_8;
    p_qos_cfg->queue_num_for_cpu_reason    = 128;
    p_qos_cfg->max_cos_level = 3;
    p_qos_cfg->policer_num = 4096;

    p_qos_cfg->resrc_pool.igs_pool_mode = CTC_QOS_RESRC_POOL_DISABLE;
    p_qos_cfg->resrc_pool.egs_pool_mode = CTC_QOS_RESRC_POOL_USER_DEFINE;
    p_qos_cfg->resrc_pool.egs_pool_size[CTC_QOS_EGS_RESRC_DEFAULT_POOL]  = 215*1024/10;
    p_qos_cfg->resrc_pool.egs_pool_size[CTC_QOS_EGS_RESRC_NON_DROP_POOL] = 0;
    p_qos_cfg->resrc_pool.egs_pool_size[CTC_QOS_EGS_RESRC_SPAN_POOL]     = 0;
    p_qos_cfg->resrc_pool.egs_pool_size[CTC_QOS_EGS_RESRC_CONTROL_POOL]  = 0;

    p_profile = &p_qos_cfg->resrc_pool.drop_profile[CTC_QOS_EGS_RESRC_DEFAULT_POOL];
    p_profile->congest_level_num = 4;
    p_profile->congest_threshold[0] = 16*1024;
    p_profile->congest_threshold[1] = 18*1024;
    p_profile->congest_threshold[2] = 20*1024;
    p_profile->queue_drop[0].max_th[0] = 15*1024;
    p_profile->queue_drop[0].max_th[1] = 15*1024;
    p_profile->queue_drop[0].max_th[2] = 15*1024;
    p_profile->queue_drop[0].max_th[3] = 15*1024;
    p_profile->queue_drop[1].max_th[0] = 7*1024;
    p_profile->queue_drop[1].max_th[1] = 7*1024;
    p_profile->queue_drop[1].max_th[2] = 7*1024;
    p_profile->queue_drop[1].max_th[3] = 7*1024;
    p_profile->queue_drop[2].max_th[0] = 2*1024;
    p_profile->queue_drop[2].max_th[1] = 2*1024;
    p_profile->queue_drop[2].max_th[2] = 2*1024;
    p_profile->queue_drop[2].max_th[3] = 2*1024;
    p_profile->queue_drop[3].max_th[0] = 128;
    p_profile->queue_drop[3].max_th[1] = 128;
    p_profile->queue_drop[3].max_th[2] = 128;
    p_profile->queue_drop[3].max_th[3] = 128;
   
    return;
}

/*************************************************************************************************
 * Name         : hagt_openflow_meter_register_callback
 * Purpose      : Hagt layer openflow meter msg callback register
 * Input        : N/A
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
hagt_openflow_meter_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddMeterEntry, _hagt_openflow_meter_add_meter_entry);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelMeterEntry, _hagt_openflow_meter_delete_meter_entry);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowUpdateMeterIpgStatus, _hagt_openflow_meter_update_ipg_status);
#undef Reg

    return HAGT_E_NONE;
}