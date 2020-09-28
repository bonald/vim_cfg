/*********************************************************************
 * hagt_sflow.c:  Hal agent sflow process
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Chengw
 * Date         : 2015-01-19
 * Reason       : OSP system.
*********************************************************************/

/*********************************************************************
 *
 * Header Files 
 *
*********************************************************************/
#include "hagt_inc.h"
#include "sal.h"
#include "sal_types.h"
#include "ctclib_list.h"
#include "ctclib_show.h"
#include "hagt_stats.h"
#include "hagt_sflow_priv.h"
#include "hagt_sflow.h"
#include "hagt_if.h"

/*********************************************************************
 *  
 * Globals
 *
*********************************************************************/
static hagt_sflow_master_t* g_pst_hagt_sflow_master = NULL;

/*********************************************************************
 *  
 * Functions
 *
*********************************************************************/
/*********************************************************************
 * Name    : _hagt_sflow_set_sample
 * Purpose : Enable/Disable sFlow sample on interface.
 * Input   : p_hsrv_msg - request message data from Hal Server
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32 
_hagt_sflow_set_sample(Hsrv2AgtMsg_t* p_hsrv_msg)
{
    uint32** gport_array = NULL;
    ctc_direction_t ctc_dir;
    uint8  dir;
    uint8  enable;
    uint8  weight;
    uint8  entry_cnt;
    uint8  i;

    HAGT_DEBUG_SFLOW_FUNC();
    HAGT_SFLOW_PTR_CHECK(p_hsrv_msg);
    HAGT_SFLOW_PTR_CHECK(g_pst_hagt_sflow_master);
    dir         = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgSflowSetSample.dir;
    enable      = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgSflowSetSample.enable;
    weight      = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgSflowSetSample.weight;
    gport_array = (uint32**)p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgSflowSetSample.gportList.list.array;
    entry_cnt   = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgSflowSetSample.gportList.list.count;
    HAGT_SFLOW_PTR_CHECK(gport_array);

    ctc_dir = (dir == GLB_INGRESS) ? CTC_INGRESS : CTC_EGRESS;

    for (i = 0; i < entry_cnt; i++)
    {
		/* Modify by weidk, As we need obvious effect when the rate of flow is slowly which like V350,though SDK don't suggest set with this logical */
		HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(*gport_array[i], CTC_PORT_DIR_PROP_RANDOM_LOG_RATE, dir, HAGT_SFLOW_MAX_LOG_THRESHOLD/weight));
	
        HAGT_IF_ERROR_RETURN(ctc_port_set_random_log_en(*gport_array[i], ctc_dir, enable));

        if (!enable)
        {
            if (CTC_INGRESS == ctc_dir)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_clear_port_log_stats(*gport_array[i], CTC_INGRESS));
            }
            if (CTC_EGRESS == ctc_dir)
            {
                HAGT_IF_ERROR_RETURN(ctc_stats_clear_port_log_stats(*gport_array[i], CTC_EGRESS));
            }
        }
    }

    return HAGT_E_NONE;
}

#if 0
/*********************************************************************
 * Name    : _hagt_sflow_get_sample_pool
 * Purpose : Process msg to get sflow sample pool
 * Input   : p_hsrv_msg  - message pointer
 * Output  : N/A
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_sflow_get_sample_pool(HalMsgStatsKey_t* p_key, glb_stats_result_t* p_rslt)
{
    ctc_stats_basic_t ctc_stats;
    uint32** gport_array = NULL;
    glb_stats_t  stats;
    glb_stats_t* p_stats = NULL;
    bool  clear_after_read_en;
    uint8 igress_en;
    uint8 egress_en;
    uint8 entry_cnt;
    uint8 i;
    uint8 gchip;
    int32 lchip;
    uint8 lport;
    
    HAGT_DEBUG_SFLOW_FUNC();
    HAGT_SFLOW_PTR_CHECK(p_key);
    HAGT_SFLOW_PTR_CHECK(p_rslt);
    HAGT_SFLOW_PTR_CHECK(g_pst_hagt_sflow_master);
    igress_en   = p_key->sflowKey.ingressEn;
    egress_en   = p_key->sflowKey.egressEn;
    gport_array = (uint32**)p_key->sflowKey.gportListKey.list.array;
    entry_cnt   = p_key->sflowKey.gportListKey.list.count;
    HAGT_SFLOW_PTR_CHECK(gport_array);
    
    sal_memset(p_rslt, 0, sizeof(glb_stats_result_t));
    sal_memset(&stats, 0, sizeof(glb_stats_t));
    sal_memset(&ctc_stats, 0, sizeof(ctc_stats_basic_t));
    /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
    /*we configre all except mac stats type as forward*/
#if 0    /*Modified by xgu, sdk will not do clear_after_read, so clear_after_read_en is always false, 2013-7-8*/
    ctc_stats_get_clear_after_read_en(CTC_STATS_TYPE_FWD, &clear_after_read_en);  // --need to be checked
#else
    clear_after_read_en = FALSE;
#endif        
    for (i = 0; i < entry_cnt; i++)
    {
        HAGT_DEBUG_SFLOW(" i=%d, gport=%d, en[in]=%d, en[out]=%d",
            i, *gport_array[i], igress_en, egress_en);

        if (!HAGT_GPORT_IS_LOCAL(*gport_array[i]))
        {
            continue;
        }
        gchip = GLB_GET_GLOBAL_CHIPID(*gport_array[i]);
        lport = GLB_GET_LOCAL_PORTID(*gport_array[i]);
        lchip = hagt_chip_glb2local(gchip);
        if (GLB_INVALID_CHIPID == lchip)
        {
            continue;
        }
        if (lport >= CTC_MAX_PHY_PORT)
        {
            continue;
        }
        
        if (igress_en)
        {
            ctc_stats_get_port_log_stats(*gport_array[i], CTC_INGRESS, &ctc_stats);
            stats.packet_count = ctc_stats.packet_count;
            stats.byte_count   = ctc_stats.byte_count;
        }
        if (egress_en)
        {
            ctc_stats_get_port_log_stats(*gport_array[i], CTC_EGRESS, &ctc_stats);
            stats.packet_count += ctc_stats.packet_count;
            stats.byte_count   += ctc_stats.byte_count;
        }
        
        p_stats = &g_pst_hagt_sflow_master->p_port_log_stats[lchip * CTC_MAX_PHY_PORT + lport];
        if(clear_after_read_en)
        {
           p_stats->byte_count   += stats.byte_count;
           p_stats->packet_count += stats.packet_count;
        }
        else
        {
           p_stats->byte_count   = stats.byte_count;
           p_stats->packet_count = stats.packet_count;            
        }
    }
    p_rslt->basic.byte_count   = p_stats->byte_count;
    p_rslt->basic.packet_count = p_stats->packet_count;
    
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_sflow_port_log_stats_timer_cb
 * Purpose : Get port stats, save to hagt sflow db.
 * Input   : N/A
 * Output  : N/A
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
static int32 
_hagt_sflow_port_log_stats_timer_cb(struct thread* pst_thread)
{
    thread_master_t* p_thread_master = NULL;
    ctc_stats_basic_t  igs_stats;
    ctc_stats_basic_t  egs_stats;
    glb_stats_t* p_stats;
    uint8 lport;
    uint8 lchip;
    uint8 gchip;
    uint8 lchip_num;
    uint16 gport;

    HAGT_SFLOW_PTR_CHECK(pst_thread);
    HAGT_SFLOW_PTR_CHECK(g_pst_hagt_sflow_master);
    p_thread_master = ctclib_thread_get_master();
    ctclib_thread_add_timer_msec(
        p_thread_master, _hagt_sflow_port_log_stats_timer_cb,
        NULL, HAGT_SFLOW_PORT_LOG_STATS_INTERVAL);
    
    ctc_get_local_chip_num(&lchip_num);
    for (lchip = 0; lchip < lchip_num; lchip++)
    {
        gchip = hagt_chip_local2glb(lchip);
        for (lport = 0; lport < CTC_MAX_PHY_PORT; lport++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
            sal_memset(&igs_stats, 0, sizeof(ctc_stats_basic_t));
            sal_memset(&egs_stats, 0, sizeof(ctc_stats_basic_t));
            
            ctc_stats_get_port_log_stats(gport, CTC_INGRESS, &igs_stats);
            ctc_stats_get_port_log_stats(gport, CTC_EGRESS, &egs_stats);
            
            p_stats = &g_pst_hagt_sflow_master->p_port_log_stats[lchip * CTC_MAX_PHY_PORT + lport];
            p_stats->byte_count   += igs_stats.byte_count;
            p_stats->packet_count += igs_stats.packet_count;
            p_stats->byte_count   += egs_stats.byte_count;
            p_stats->packet_count += egs_stats.packet_count;               
        }
    }

    return HAGT_E_NONE;
}
#endif

/*********************************************************************
 * Name    : _hagt_sflow_msg_callback_register
 * Purpose : Register callback function to process message from hal layer
 * Input   : N/A
 * Output  : N/A
 * Return  : HAGT_E_XXX
 * Note    : N/A
*********************************************************************/
static int32 
_hagt_sflow_msg_callback_register(void)
{
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgSflowSetSample, _hagt_sflow_set_sample);
    return HAGT_E_NONE;
}

#if 0
/*********************************************************************
 * Name    : hagt_sflow_init
 * Purpose : Hal Agent Sflow initialization.
 * Input   : p_hagt_master - Hal Agent master pointer
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_sflow_init(hagt_master_t* p_hagt_master)
{
    hagt_stast_pm_data_t pm_data;
    thread_master_t* p_thread_master = NULL;
    hagt_module_info_t* p_sflow_module = NULL; 
    bool clear_after_read_en;

    HAGT_SFLOW_PTR_CHECK(p_hagt_master);
    p_sflow_module = HAGT_SFLOW_CALLOC(sizeof(hagt_module_info_t));
    HAGT_SFLOW_MEM_PTR_CHECK(p_sflow_module);
    
    g_pst_hagt_sflow_master = HAGT_SFLOW_CALLOC(sizeof(hagt_sflow_master_t));
    if (!g_pst_hagt_sflow_master)
    {
        HAGT_LOG_ERR("No memory when malloc hagt sflow master.\n");
        HAGT_FREE(CTCLIB_MEM_SFLOW_MODULE, p_sflow_module);
        return HAGT_E_NO_MEMORY;
    }
    p_sflow_module->module_id = HAGT_MODULE_SFLOW;
    p_sflow_module->p_data_info = g_pst_hagt_sflow_master;
    p_hagt_master->past_module_info[HAGT_MODULE_SFLOW] = p_sflow_module; 
    
    /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
    /*we configre all except mac stats type as forward*/
#if 0    /*Modified by xgu, sdk will not do clear_after_read, so clear_after_read_en is always false, 2013-7-8*/
    ctc_stats_get_clear_after_read_en(CTC_STATS_TYPE_FWD, &clear_after_read_en);  // --need to be checked
#else
    clear_after_read_en = FALSE;
#endif

    if (clear_after_read_en)
    {
        p_thread_master = ctclib_thread_get_master();
        ctclib_thread_add_timer_msec(
            p_thread_master, _hagt_sflow_port_log_stats_timer_cb, 
            (void*)(g_pst_hagt_sflow_master), HAGT_SFLOW_PORT_LOG_STATS_INTERVAL);
    }

    g_pst_hagt_sflow_master->p_port_log_stats =
        HAGT_SFLOW_CALLOC(sizeof(glb_stats_t) * CTC_MAX_LOCAL_CHIP_NUM * CTC_MAX_PHY_PORT);
    HAGT_SFLOW_MEM_PTR_CHECK(g_pst_hagt_sflow_master->p_port_log_stats);
    
    _hagt_sflow_msg_callback_register();
    
    /* Register callback function for stats*/
    pm_data.get_stats_cb   = _hagt_sflow_get_sample_pool;
    pm_data.clear_stats_cb = NULL;
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_SFLOW, &pm_data);
    
    if(ctc_stats_set_drop_packet_stats_en(CTC_STATS_RANDOM_LOG_DISCARD_STATS, FALSE))
    {
        HAGT_LOG_ERR("Set random log discard stats fail\n");
        return HAGT_E_NOT_INIT;
    };

    return HAGT_E_NONE;
}
#endif

/*********************************************************************
 * Name    : hagt_sflow_init
 * Purpose : Hal Agent Sflow initialization.
 * Input   : p_hagt_master - Hal Agent master pointer
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_sflow_init(hagt_master_t* p_hagt_master)
{
    hagt_module_info_t* p_sflow_module = NULL; 

    HAGT_SFLOW_PTR_CHECK(p_hagt_master);
    p_sflow_module = HAGT_SFLOW_CALLOC(sizeof(hagt_module_info_t));
    HAGT_SFLOW_MEM_PTR_CHECK(p_sflow_module);
    
    g_pst_hagt_sflow_master = HAGT_SFLOW_CALLOC(sizeof(hagt_sflow_master_t));
    if (!g_pst_hagt_sflow_master)
    {
        HAGT_LOG_ERR("No memory when malloc hagt sflow master.\n");
        HAGT_SFLOW_FREE(p_sflow_module);
        return HAGT_E_NO_MEMORY;
    }

    p_sflow_module->module_id = HAGT_MODULE_SFLOW;
    p_sflow_module->p_data_info = g_pst_hagt_sflow_master;
    p_hagt_master->past_module_info[HAGT_MODULE_SFLOW] = p_sflow_module; 

    _hagt_sflow_msg_callback_register();

    if(ctc_stats_set_drop_packet_stats_en(CTC_STATS_RANDOM_LOG_DISCARD_STATS, FALSE))
    {
        HAGT_LOG_ERR("Set random log discard stats fail\n");
        return HAGT_E_NOT_INIT;
    }

    return HAGT_E_NONE;
}

