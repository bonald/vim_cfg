#include "ctc_api.h"

#include "ctclib_vector.h"

#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_openflow.h"
#include "hagt_openflow_tpoam.h"

hagt_openflow_master_t *g_pst_hagt_openflow_master = NULL;

int32
hagt_openflow_init (hagt_master_t* p_hagt_master)
{
    uint8 reason_id;
    ctc_qos_queue_cfg_t queue_cfg;
#ifdef OFPRODUCT
    uint32 discard = 1;
#endif
    hagt_module_info_t* p_openflow_module = NULL; 
    
    g_pst_hagt_openflow_master = HAGT_OPENFLOW_MALLOC(sizeof(hagt_openflow_master_t));
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master);    

    p_openflow_module = HAGT_OPENFLOW_CALLOC(sizeof(hagt_module_info_t));
    if (NULL == p_openflow_module)
    {
        HAGT_OPENFLOW_LOG_ERR("malloc hagt openflow module failed!");
        HAGT_OPENFLOW_FREE(g_pst_hagt_openflow_master);
        return HAGT_E_NO_MEMORY;
    }
    
    p_openflow_module->module_id = HAGT_MODULE_OPENFLOW;
    p_openflow_module->p_data_info = g_pst_hagt_openflow_master;
    p_hagt_master->past_module_info[HAGT_MODULE_OPENFLOW] = p_openflow_module; 

    hagt_tpoam_init();
    
    hagt_openflow_port_register_callback();
    hagt_openflow_flow_register_callback();
    hagt_openflow_parser_register_callback();
    hagt_openflow_stats_register_callback();
    hagt_openflow_nexthop_register_callback();
    hagt_openflow_linkagg_register_callback();
    hagt_openflow_bond_register_callback();

    /* Added by weizj for tunnel */
    hagt_openflow_tunnel_register_callback();
    /* Added by weizj for meter */

    hagt_openflow_meter_register_callback();
#ifdef V580_OPENFLOW
    hagt_openflow_queue_register_callback();

    hagt_openflow_efd_register_callback();
#endif

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
    hagt_openflow_tpoam_register_callback();
    /* liwh end */

    for (reason_id = GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN; reason_id < GLB_PKT_OPENFLOW_TOCPU_MAX; reason_id++)
    {
        sal_memset(&queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
        queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
        queue_cfg.value.reason_dest.cpu_reason = CTC_PKT_CPU_REASON_CUSTOM_BASE + reason_id;
        queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
        //HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&queue_cfg));
    }    
    HAGT_IF_ERROR_RETURN(hagt_openflow_stats_init());
    HAGT_IF_ERROR_RETURN(hagt_openflow_linkagg_init());
    HAGT_IF_ERROR_RETURN(hagt_openflow_bond_init());
    HAGT_IF_ERROR_RETURN(hagt_openflow_nexthop_init());
#ifdef V580_OPENFLOW
    HAGT_IF_ERROR_RETURN(hagt_openflow_efd_init());
#endif


#ifdef OFPRODUCT
    /* Deleted by kcao 2017-05-19 for bug 44046, discard invalid packets */
#if 0
    /* Keep behavior same as V350. */    
    ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT, &no_discard);
    ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT, &no_discard);
    ctc_global_ctl_set(CTC_GLOBAL_DISCARD_MCAST_SA_PKT, &no_discard);
#endif
    ctc_global_ctl_set(CTC_GLOBAL_DISCARD_TTL_0_PKT, &discard);

    /* Added by weizj for bug 36543 */
    HAGT_IF_ERROR_RETURN(ctc_parser_enable_l3_type(CTC_PARSER_L3_TYPE_IPV6, TRUE));
#endif
    // SDK will handle this after new GG revision.
#if 0
    /* Legacy IO is fast in OpenFlow case. */
    sys_goldengate_acl_set_tcam_mode(0, 0);
#endif
    return HAGT_E_NONE;
}
