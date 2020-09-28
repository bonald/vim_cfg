/*********************************************************************
 * hagt_stats.c:  Hal Agent stats process
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Revision     : R0.01
 * Author       : Yan Xing'an
 * Date         : Nov.23, 2010
 * Reason       : Humber system

*********************************************************************/

/*********************************************************************
 *
 * Header Files 
 *
*********************************************************************/

#include "hagt_inc.h"
#include "hagt_stats.h"
#include "hagt_stats_priv.h"
#include "hagt_intr.h"


/*********************************************************************
 *  
 * Defines and Macros
 *
*********************************************************************/

/*********************************************************************
 *  
 * Global and Declaration
 *
*********************************************************************/
#if 1
hagt_stats_master_t* p_g_hagt_stats_master = NULL;

/*********************************************************************
 *  
 * Functions
 *
*********************************************************************/

/*********************************************************************
 * Name    : _hagt_stats_map_stats_result
 * Purpose : Map statistics result to asn1 ds
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_stats_map_stats_result(uint8 type, glb_stats_result_t* p_stats, HalMsgStatsResult_t* resp, 
                                    uint8 tmp_stats[][8])
{
    HAGT_STATS_CHECK_TYPE(type);
    HAGT_STATS_PTR_CHECK(p_stats);
    HAGT_STATS_PTR_CHECK(resp);
    switch(type)
    {
        case GLB_STATS_TYPE_ACL:
        case GLB_STATS_TYPE_SFLOW:
        case GLB_STATS_TYPE_MPLS_ALL:
        case GLB_STATS_TYPE_MPLS_ILM:
        case GLB_STATS_TYPE_MPLS_FTN:
        case GLB_STATS_TYPE_MPLS_VC:
        case GLB_STATS_TYPE_FLOW_RULE:
        case GLB_STATS_TYPE_OPENFLOW_FLOW:
        case GLB_STATS_TYPE_OPENFLOW_METER:
        case GLB_STATS_TYPE_OPENFLOW_GROUP:
        {
            resp->present = HalMsgStatsResult_PR_basicRslt;
            /*modified by weij for bug 17826, 2012-01-12*/
            HAGT_UINT64_TO_OCTET_STR(p_stats->basic.packet_count, resp->basicRslt.pkts, tmp_stats[0]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->basic.byte_count,   resp->basicRslt.octets, tmp_stats[1]);
            break;            
        }
        case GLB_STATS_TYPE_PORT_QUEUE:
        {
            resp->present = HalMsgStatsResult_PR_queueRslt;
            /*modified by weij for bug 17826, 2012-01-12*/
            /*modified by wangl for oepnflow 2016-7-11*/
            #if 0
            HAGT_UINT64_TO_OCTET_STR(p_stats->queue.deq_stats.packet_count,  resp->queueRslt.deqPkts, tmp_stats[0]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->queue.deq_stats.byte_count,    resp->queueRslt.deqBytes, tmp_stats[1]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->queue.drop_stats.packet_count, resp->queueRslt.dropPkts, tmp_stats[2]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->queue.drop_stats.byte_count,   resp->queueRslt.dropBytes, tmp_stats[3]);
            resp->queueRslt.supportFlag = p_stats->queue.support_flag;
            #endif
            break;
        }
        case GLB_STATS_TYPE_PORT_POLICER:
        case GLB_STATS_TYPE_FLOW_POLICER:
        case GLB_STATS_TYPE_AGGREGATE_POLICER:
        {
            resp->present = HalMsgStatsResult_PR_policerRslt;
            /*modified by weij for bug 17826, 2012-01-12*/
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.confirm_stats.packet_count, resp->policerRslt.confirmPackets,\
                                     tmp_stats[0]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.confirm_stats.byte_count,   resp->policerRslt.confirmBytes,\
                                     tmp_stats[1]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.exceed_stats.packet_count,  resp->policerRslt.exceedPackets,\
                                     tmp_stats[2]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.exceed_stats.byte_count,    resp->policerRslt.exceedBytes,\
                                     tmp_stats[3]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.violate_stats.packet_count, resp->policerRslt.violatePackets,\
                                     tmp_stats[4]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->policer.violate_stats.byte_count,   resp->policerRslt.violateBytes,\
                                     tmp_stats[5]);
            resp->policerRslt.supportFlag = p_stats->policer.support_flag;
            break;
        }
        case GLB_STATS_TYPE_INTF:
        {
            resp->present = HalMsgStatsResult_PR_portRslt;
            /*modified by weij for bug 17826, 2012-01-12*/
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.good_octets_rcv      , resp->portRslt.goodOctetsRcv, tmp_stats[0]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.bad_octets_rcv       , resp->portRslt.badOctetsRcv, tmp_stats[1]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.mac_transmit_err     , resp->portRslt.macTransmitErr, tmp_stats[2]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.good_pkts_rcv        , resp->portRslt.goodPktsRcv, tmp_stats[3]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.bad_pkts_rcv         , resp->portRslt.badPktsRcv, tmp_stats[4]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.uc_pkts_rcv          , resp->portRslt.ucPktsRcv, tmp_stats[5]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.brdc_pkts_rcv        , resp->portRslt.brdcPktsRcv, tmp_stats[6]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.mc_pkts_rcv          , resp->portRslt.mcPktsRcv, tmp_stats[7]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.oam_pkts_rcv         , resp->portRslt.oamPktsRcv, tmp_stats[8]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_64_octets       , resp->portRslt.pkts64octets, tmp_stats[9]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_65_127_octets   , resp->portRslt.pkts65To127octets,\
                                     tmp_stats[10]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_128_255_octets  , resp->portRslt.pkts128To255octets,\
                                     tmp_stats[11]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_256_511_octets  , resp->portRslt.pkts256To511octets,\
                                     tmp_stats[12]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_512_1023_octets , resp->portRslt.pkts512To1023octets,\
                                     tmp_stats[13]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_1024_max_octets , resp->portRslt.pkts1024TomaxOctets,\
                                     tmp_stats[14]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.good_octets_send     , resp->portRslt.goodOctetsSend, tmp_stats[15]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.good_pkts_send       , resp->portRslt.goodPktsSend, tmp_stats[16]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.uc_pkts_send         , resp->portRslt.ucPktsSend, tmp_stats[17]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.mc_pkts_send         , resp->portRslt.mcPktsSend, tmp_stats[18]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.brdc_pkts_send       , resp->portRslt.brdcPktsSend, tmp_stats[19]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.oam_pkts_send        , resp->portRslt.oamPktsSend, tmp_stats[20]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.drop_events          , resp->portRslt.dropEvents, tmp_stats[21]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.undersize_pkts       , resp->portRslt.undersizePkts, tmp_stats[22]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.fragments_pkts       , resp->portRslt.fragmentsPkts, tmp_stats[23]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.oversize_pkts        , resp->portRslt.oversizePkts, tmp_stats[24]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.jabber_pkts          , resp->portRslt.jabberPkts, tmp_stats[25]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.mac_rcv_error        , resp->portRslt.macRcvError, tmp_stats[26]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.bad_crc              , resp->portRslt.badCrc, tmp_stats[27]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.octets_rcv           , resp->portRslt.octetsRcv, tmp_stats[28]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_rcv             , resp->portRslt.pktsRcv, tmp_stats[29]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.octets_send          , resp->portRslt.octetsSend, tmp_stats[30]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_send            , resp->portRslt.pktsSend, tmp_stats[31]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.frame_error          , resp->portRslt.frameError, tmp_stats[32]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pause_rcv            , resp->portRslt.pauseRcv, tmp_stats[33]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pause_send           , resp->portRslt.pauseSend, tmp_stats[34]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.bad_pkts_rcv_sec     , resp->portRslt.badPktsRcvSec, tmp_stats[35]);
            /* modified by cuixl for bug 17874, 2012-02-26 */            
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.octets_rcv_rate     , resp->portRslt.octetsRcvRate, tmp_stats[36]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_rcv_rate     , resp->portRslt.pktsRcvRate, tmp_stats[37]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.octets_send_rate     , resp->portRslt.octetsSendRate, tmp_stats[38]);
            HAGT_UINT64_TO_OCTET_STR(p_stats->port.pkts_send_rate     , resp->portRslt.pktsSendRate, tmp_stats[39]);
            break;
        }
        default:
            return HAGT_E_INVALID_PARAM;
    }
    return HAGT_E_NONE;
}

/*modified by weij for bug 17826, 2012-01-12*/
#if 0
/*********************************************************************
 * Name    : _hagt_stats_free_octet_str
 * Purpose : Free octet string in asn1 ds
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_stats_free_octet_str(uint8 type, HalMsgStatsResult_t* resp)
{
    HAGT_STATS_CHECK_TYPE(type);
    HAGT_STATS_PTR_CHECK(resp);
    switch(type)
    {
        case GLB_STATS_TYPE_ACL:
        case GLB_STATS_TYPE_SFLOW:
        case GLB_STATS_TYPE_MPLS_ALL:
        case GLB_STATS_TYPE_MPLS_ILM:
        case GLB_STATS_TYPE_MPLS_FTN:
        case GLB_STATS_TYPE_MPLS_VC:
        case GLB_STATS_TYPE_FLOW_RULE:
        {
            resp->present = HalMsgStatsResult_PR_basicRslt;
            FREE_OCTET_STR(resp->basicRslt.pkts);
            FREE_OCTET_STR(resp->basicRslt.octets);
            break;            
        }
        case GLB_STATS_TYPE_PORT_QUEUE:
        {
            resp->present = HalMsgStatsResult_PR_queueRslt;
            FREE_OCTET_STR(resp->queueRslt.deqPkts);
            FREE_OCTET_STR(resp->queueRslt.deqBytes);
            FREE_OCTET_STR(resp->queueRslt.dropPkts);
            FREE_OCTET_STR(resp->queueRslt.dropBytes);
            break;
        }
        case GLB_STATS_TYPE_PORT_POLICER:
        case GLB_STATS_TYPE_FLOW_POLICER:
        {
            resp->present = HalMsgStatsResult_PR_policerRslt;
            FREE_OCTET_STR(resp->policerRslt.confirmPackets);
            FREE_OCTET_STR(resp->policerRslt.confirmBytes);
            FREE_OCTET_STR(resp->policerRslt.exceedPackets);
            FREE_OCTET_STR(resp->policerRslt.exceedBytes);
            FREE_OCTET_STR(resp->policerRslt.violatePackets);
            FREE_OCTET_STR(resp->policerRslt.violateBytes);
            break;
        }
        case GLB_STATS_TYPE_INTF:
        {
            resp->present = HalMsgStatsResult_PR_portRslt;
            FREE_OCTET_STR(resp->portRslt.goodOctetsRcv       );
            FREE_OCTET_STR(resp->portRslt.badOctetsRcv        );
            FREE_OCTET_STR(resp->portRslt.macTransmitErr      );
            FREE_OCTET_STR(resp->portRslt.goodPktsRcv         );
            FREE_OCTET_STR(resp->portRslt.badPktsRcv          );
            FREE_OCTET_STR(resp->portRslt.ucPktsRcv           );
            FREE_OCTET_STR(resp->portRslt.brdcPktsRcv         );
            FREE_OCTET_STR(resp->portRslt.mcPktsRcv           );
            FREE_OCTET_STR(resp->portRslt.oamPktsRcv          );
            FREE_OCTET_STR(resp->portRslt.pkts64octets        );
            FREE_OCTET_STR(resp->portRslt.pkts65To127octets   );
            FREE_OCTET_STR(resp->portRslt.pkts128To255octets  );
            FREE_OCTET_STR(resp->portRslt.pkts256To511octets  );
            FREE_OCTET_STR(resp->portRslt.pkts512To1023octets );
            FREE_OCTET_STR(resp->portRslt.pkts1024TomaxOctets );
            FREE_OCTET_STR(resp->portRslt.goodOctetsSend      );
            FREE_OCTET_STR(resp->portRslt.goodPktsSend        );
            FREE_OCTET_STR(resp->portRslt.ucPktsSend          );
            FREE_OCTET_STR(resp->portRslt.mcPktsSend          );
            FREE_OCTET_STR(resp->portRslt.brdcPktsSend        );
            FREE_OCTET_STR(resp->portRslt.oamPktsSend         );
            FREE_OCTET_STR(resp->portRslt.dropEvents          );
            FREE_OCTET_STR(resp->portRslt.undersizePkts       );
            FREE_OCTET_STR(resp->portRslt.fragmentsPkts       );
            FREE_OCTET_STR(resp->portRslt.oversizePkts        );
            FREE_OCTET_STR(resp->portRslt.jabberPkts          );
            FREE_OCTET_STR(resp->portRslt.macRcvError         );
            FREE_OCTET_STR(resp->portRslt.badCrc              );
            FREE_OCTET_STR(resp->portRslt.octetsRcv           );
            FREE_OCTET_STR(resp->portRslt.pktsRcv             );
            FREE_OCTET_STR(resp->portRslt.octetsSend          );
            FREE_OCTET_STR(resp->portRslt.pktsSend            );
            FREE_OCTET_STR(resp->portRslt.frameError          );
            FREE_OCTET_STR(resp->portRslt.pauseRcv            );
            FREE_OCTET_STR(resp->portRslt.pauseSend           );
            FREE_OCTET_STR(resp->portRslt.badPktsRcvSec       );
            break;
        }
        default:
            return HAGT_E_INVALID_PARAM;
    }
    return HAGT_E_NONE;
}
#endif

/*********************************************************************
 * Name    : _hagt_stats_get_stats
 * Purpose : get stats
 * Input   : p_hsrv_msg - message from hal server layer
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_stats_get_stats(Hsrv2AgtMsg_t* p_hsrv_msg)
{
    uint8 type;
    glb_stats_result_t stats_result;
    Hagt2SrvMsg_t hagt_msg;
    int ret = HAGT_E_NONE;
    uint8 tmp_stats[HAGT_STATS_MAX_ITEM][8];

    /*1.dump & check message information*/
    HAGT_DEBUG_STATS_FUNC();
    HAGT_STATS_PTR_CHECK(p_hsrv_msg);
    HAGT_STATS_PTR_CHECK(p_g_hagt_stats_master);

    type = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsGetStats.type;
    HAGT_DEBUG_STATS(" type = %d", type);
    HAGT_STATS_CHECK_TYPE(type);

    if (p_g_hagt_stats_master->pm_data[type].get_stats_cb)
    {
        /*2.call callback function to get stats*/
        sal_memset(&stats_result, 0, sizeof(glb_stats_result_t));
        ret = p_g_hagt_stats_master->pm_data[type].get_stats_cb(
            &p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsGetStats.statsKey, &stats_result);
        /* modified by chengw for bugid 16857 2011-11-03 if get error, dont return but send to hsrvd */
        if(HAGT_E_NONE != ret)
        {
            HAGT_STATS_LOG_NOTICE("stats get process error, type = %d\n", type); 
        }
    }
    
    /*3.send result to hal server*/
    sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
    hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
    hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = type;
/* modified by cuixl for bug 17874, 2012-02-09 */
/* for if stats, gport id is needed */
    if (GLB_STATS_TYPE_INTF == type)
    {
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.portRslt.gportid = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsGetStats.statsKey.portKey.gport;
    }
    
    _hagt_stats_map_stats_result(type, &stats_result, &hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt, tmp_stats);
    HAGT2SRV_MSG_SEND(&hagt_msg);

#if 0
    /*4.free octet string in asn1 ds*/
    _hagt_stats_free_octet_str(type, &hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt);
#endif

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_stats_clear_stats
 * Purpose : Clear stats
 * Input   : p_hsrv_msg - message from hal server layer
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_stats_clear_stats(Hsrv2AgtMsg_t* p_hsrv_msg)
{
    uint8 type;
    int i;
    int key_num;
    
    HAGT_DEBUG_STATS_FUNC();
    HAGT_STATS_PTR_CHECK(p_hsrv_msg);
    HAGT_STATS_PTR_CHECK(p_g_hagt_stats_master);
    
    type = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsClearStats.type;
    HAGT_DEBUG_STATS(" type = %d", type);
    HAGT_STATS_CHECK_TYPE(type);
    key_num = p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsClearStats.statsKey.list.count;
    
    if (p_g_hagt_stats_master->pm_data[type].clear_stats_cb)
    {
        for (i = 0; i< key_num; i++)
        {
            HAGT_IF_ERROR_RETURN(p_g_hagt_stats_master->pm_data[type].clear_stats_cb(
                p_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgStatsClearStats.statsKey.list.array[i]));
        }
    }
    return HAGT_E_NONE;
}

/****************************************************************************
 * Name    : _hagt_stats_intr_cb
 * Purpose : call back of the stats interrupt
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
 ****************************************************************************/
int32
_hagt_stats_intr_cb(uint8 gchip, uint8 lchip)
{
    HAGT_DEBUG_STATS_FUNC();
    HAGT_DEBUG_STATS("  gchip=%u, lchip=%u", gchip, lchip);
    int ret;
    
    ret = ctc_stats_intr_callback_func(&gchip);
    if (ret)
    {
        HAGT_STATS_LOG_NOTICE("stats interrupt process error\n");
    }
    
    return  HAGT_E_NONE;     
}

/*********************************************************************
 * Name    : hagt_stats_regrister_pm_data
 * Purpose : Register PM data
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_stats_regrister_pm_data(glb_stats_type_t type, hagt_stast_pm_data_t* p_pm_data)
{
    HAGT_STATS_CHECK_TYPE(type);
    HAGT_STATS_PTR_CHECK(p_pm_data);
    HAGT_STATS_PTR_CHECK(p_g_hagt_stats_master);
    p_g_hagt_stats_master->pm_data[type].get_stats_cb   = p_pm_data->get_stats_cb;
    p_g_hagt_stats_master->pm_data[type].clear_stats_cb = p_pm_data->clear_stats_cb;
    
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_stats_msg_callback_register
 * Purpose : Hal Agent Stats callback register.
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32 
_hagt_stats_msg_callback_register(void)
{
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgStatsGetStats,   _hagt_stats_get_stats);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgStatsClearStats, _hagt_stats_clear_stats);
    
    return HAGT_E_NONE;
}

/****************************************************************************
 * Name    : _hagt_stats_register_intr_callback
 * Purpose : register call back from interrupt
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
 ****************************************************************************/
static inline int32
_hagt_stats_register_intr_callback(void)
{
#if 0 // TODO: commented by xgu, 2012-12-8, fix here
    hagt_intr_register_callback(CTC_INTERRUPT_NORMAL_STATISTICS_INTR, _hagt_stats_intr_cb);
#endif    
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_stats_init
 * Purpose : Hal Agent Stats initialization.
 * Input   : p_hagt_master - Hal Agent master pointer
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
int32
hagt_stats_init(hagt_master_t* pst_hagt_master)
{
    hagt_module_info_t* p_stats_module = NULL; 

    p_g_hagt_stats_master = HAGT_STATS_CALLOC(sizeof(hagt_stats_master_t));
    HAGT_STATS_MEM_PTR_CHECK(p_g_hagt_stats_master);
    
    p_stats_module = HAGT_STATS_CALLOC(sizeof(hagt_module_info_t));
    if (NULL == p_stats_module)
    {
        HAGT_STATS_LOG_ERR("malloc hagt stats module failed!");
        HAGT_STATS_FREE(p_g_hagt_stats_master);
        p_g_hagt_stats_master = NULL;
        return HAGT_E_NO_MEMORY;
    }
    
    p_stats_module->module_id = HAGT_MODULE_STATS;
    p_stats_module->p_data_info = p_g_hagt_stats_master;
    
    pst_hagt_master->past_module_info[HAGT_MODULE_STATS] = p_stats_module;     
    
    _hagt_stats_msg_callback_register();
    _hagt_stats_register_intr_callback();
    return HAGT_E_NONE;
}
#endif

