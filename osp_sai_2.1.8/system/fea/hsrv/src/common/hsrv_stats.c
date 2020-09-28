/*********************************************************************
 * hsrv_stats.c:  Hal server stats process
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 * Revision     : R0.01
 * Author       : Yan Xing'an
 * Date         : Nov.23, 2010
 * Reason       : Humber system.
*********************************************************************/
 
#include "sal.h"
#include "sal_types.h"
#include "ctclib_list.h"
#include "ctclib_vector.h"
#include "hsrv_inc.h"
#include "glb_macro.h"
#include "hsrv_stats.h"
#include "hsrv_if.h"

/*********************************************************************
 *  
 * Globals
 *
*********************************************************************/

hsrv_stats_master_t* p_g_hsrv_stats_master = NULL;
/* modified by cuixl for bug 17874, 2012-02-27 */
extern int32
hsrv_ifdb_get_ifindex_by_gportid(uint16 gportid, uint32* p_ifindex);

/*********************************************************************
 *  
 * Functions
 *
*********************************************************************/

/*********************************************************************
 * Name    : _hsrv_stats_map_stats_result
 * Purpose : This function is used map stats result in message to hsrv data structure.
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
static int32
_hsrv_stats_map_stats_result(HalMsgStatsResult_t* resp, glb_stats_result_t* p_stats_rslt)
{
    switch(resp->present)
    {
        case HalMsgStatsResult_PR_basicRslt:
        {
            OCTET_STR_TO_UINT64(p_stats_rslt->basic.packet_count, resp->basicRslt.pkts);
            OCTET_STR_TO_UINT64(p_stats_rslt->basic.byte_count,   resp->basicRslt.octets);
            break;
        }
        case HalMsgStatsResult_PR_queueRslt:
        {
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats0.deq_stats.packet_count,  resp->queueRslt.deqPkts0);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats0.deq_stats.byte_count,    resp->queueRslt.deqBytes0);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats0.drop_stats.packet_count, resp->queueRslt.dropPkts0);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats0.drop_stats.byte_count,   resp->queueRslt.dropBytes0);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats1.deq_stats.packet_count,  resp->queueRslt.deqPkts1);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats1.deq_stats.byte_count,    resp->queueRslt.deqBytes1);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats1.drop_stats.packet_count, resp->queueRslt.dropPkts1);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats1.drop_stats.byte_count,   resp->queueRslt.dropBytes1);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats2.deq_stats.packet_count,  resp->queueRslt.deqPkts2);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats2.deq_stats.byte_count,    resp->queueRslt.deqBytes2);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats2.drop_stats.packet_count, resp->queueRslt.dropPkts2);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats2.drop_stats.byte_count,   resp->queueRslt.dropBytes2);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats3.deq_stats.packet_count,  resp->queueRslt.deqPkts3);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats3.deq_stats.byte_count,    resp->queueRslt.deqBytes3);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats3.drop_stats.packet_count, resp->queueRslt.dropPkts3);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats3.drop_stats.byte_count,   resp->queueRslt.dropBytes3);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats4.deq_stats.packet_count,  resp->queueRslt.deqPkts4);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats4.deq_stats.byte_count,    resp->queueRslt.deqBytes4);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats4.drop_stats.packet_count, resp->queueRslt.dropPkts4);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats4.drop_stats.byte_count,   resp->queueRslt.dropBytes4);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats5.deq_stats.packet_count,  resp->queueRslt.deqPkts5);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats5.deq_stats.byte_count,    resp->queueRslt.deqBytes5);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats5.drop_stats.packet_count, resp->queueRslt.dropPkts5);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats5.drop_stats.byte_count,   resp->queueRslt.dropBytes5);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats6.deq_stats.packet_count,  resp->queueRslt.deqPkts6);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats6.deq_stats.byte_count,    resp->queueRslt.deqBytes6);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats6.drop_stats.packet_count, resp->queueRslt.dropPkts6);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats6.drop_stats.byte_count,   resp->queueRslt.dropBytes6);

            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats7.deq_stats.packet_count,  resp->queueRslt.deqPkts7);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats7.deq_stats.byte_count,    resp->queueRslt.deqBytes7);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats7.drop_stats.packet_count, resp->queueRslt.dropPkts7);
            OCTET_STR_TO_UINT64(p_stats_rslt->queue.queue_stats7.drop_stats.byte_count,   resp->queueRslt.dropBytes7);
            //p_stats_rslt->queue.support_flag = resp->queueRslt.supportFlag;
            break;
        }
        case HalMsgStatsResult_PR_policerRslt:
        {
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.confirm_stats.packet_count, resp->policerRslt.confirmPackets);
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.confirm_stats.byte_count,   resp->policerRslt.confirmBytes);
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.exceed_stats.packet_count,  resp->policerRslt.exceedPackets);
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.exceed_stats.byte_count,    resp->policerRslt.exceedBytes);
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.violate_stats.packet_count, resp->policerRslt.violatePackets);
            OCTET_STR_TO_UINT64(p_stats_rslt->policer.violate_stats.byte_count,   resp->policerRslt.violateBytes);
            p_stats_rslt->policer.support_flag = resp->policerRslt.supportFlag;
            /* Added by weizj for meter */
            p_stats_rslt->policer.resource_id = resp->policerRslt.resourceId;
            break;
        }
        case HalMsgStatsResult_PR_portRslt:
        {
            OCTET_STR_TO_UINT64(p_stats_rslt->port.good_octets_rcv      , resp->portRslt.goodOctetsRcv       );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.bad_octets_rcv       , resp->portRslt.badOctetsRcv        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.mac_transmit_err     , resp->portRslt.macTransmitErr      );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.good_pkts_rcv        , resp->portRslt.goodPktsRcv         );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.bad_pkts_rcv         , resp->portRslt.badPktsRcv          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.uc_pkts_rcv          , resp->portRslt.ucPktsRcv           );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.brdc_pkts_rcv        , resp->portRslt.brdcPktsRcv         );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.mc_pkts_rcv          , resp->portRslt.mcPktsRcv           );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.oam_pkts_rcv         , resp->portRslt.oamPktsRcv          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_64_octets       , resp->portRslt.pkts64octets        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_65_127_octets   , resp->portRslt.pkts65To127octets   );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_128_255_octets  , resp->portRslt.pkts128To255octets  );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_256_511_octets  , resp->portRslt.pkts256To511octets  );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_512_1023_octets , resp->portRslt.pkts512To1023octets );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_1024_max_octets , resp->portRslt.pkts1024TomaxOctets );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.good_octets_send     , resp->portRslt.goodOctetsSend      );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.good_pkts_send       , resp->portRslt.goodPktsSend        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.uc_pkts_send         , resp->portRslt.ucPktsSend          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.mc_pkts_send         , resp->portRslt.mcPktsSend          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.brdc_pkts_send       , resp->portRslt.brdcPktsSend        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.oam_pkts_send        , resp->portRslt.oamPktsSend         );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.drop_events          , resp->portRslt.dropEvents          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.undersize_pkts       , resp->portRslt.undersizePkts       );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.fragments_pkts       , resp->portRslt.fragmentsPkts       );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.oversize_pkts        , resp->portRslt.oversizePkts        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.jabber_pkts          , resp->portRslt.jabberPkts          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.mac_rcv_error        , resp->portRslt.macRcvError         );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.bad_crc              , resp->portRslt.badCrc              );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.octets_rcv           , resp->portRslt.octetsRcv           );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_rcv             , resp->portRslt.pktsRcv             );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.octets_send          , resp->portRslt.octetsSend          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_send            , resp->portRslt.pktsSend            );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.frame_error          , resp->portRslt.frameError          );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pause_rcv            , resp->portRslt.pauseRcv            );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pause_send           , resp->portRslt.pauseSend           );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.bad_pkts_rcv_sec     , resp->portRslt.badPktsRcvSec       );
            /* modified by cuixl for bug 17874, 2012-02-26 */
            OCTET_STR_TO_UINT64(p_stats_rslt->port.octets_rcv_rate      , resp->portRslt.octetsRcvRate       );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_rcv_rate        , resp->portRslt.pktsRcvRate         );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.octets_send_rate     , resp->portRslt.octetsSendRate      );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.pkts_send_rate       , resp->portRslt.pktsSendRate        );
            
            OCTET_STR_TO_UINT64(p_stats_rslt->port.fcs_pkts_rcv       , resp->portRslt.fcsPktsRcv        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.fcs_octets_rcv       , resp->portRslt.fcsOctetsRcv        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.fcs_pkts_send       , resp->portRslt.fcsPktsSend        );
            OCTET_STR_TO_UINT64(p_stats_rslt->port.fcs_octets_send       , resp->portRslt.fcsOctetsSend        );
            
            break;
        }
        case HalMsgStatsResult_PR_openflowRslt:
        {
            OCTET_STR_TO_UINT64(p_stats_rslt->openflow_stats.packet_count, resp->openflowRslt.pkts);
            OCTET_STR_TO_UINT64(p_stats_rslt->openflow_stats.byte_count,   resp->openflowRslt.octets);
            p_stats_rslt->openflow_stats.resource_id = resp->openflowRslt.resourceId;
            break;
        }
        default:
            return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

/*********************************************************************
 * Name    : _hsrv_stats_response
 * Purpose : This function is used to process stats response from agents.
 * Input   : N/A 
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
static int32
_hsrv_stats_response(void* p_msg_data)
{
    Hagt2SrvMsg_t* p_hagt_msg = NULL;
    hsrv_stats_info_t* p_stats_info = NULL;
    uint8 type;

    /*1.Check & dump information*/
    p_hagt_msg = (Hagt2SrvMsg_t*)p_msg_data;
    
    type = p_hagt_msg->opHagt2Srv.hagt2SrvMsgStatsResponse.type;

    p_stats_info = &p_g_hsrv_stats_master->stats_info[type];


    /*2.Check whether time is late*/
    if (p_stats_info->is_time_out)
    {
        return HSRV_E_NONE;
    }

        /* For pizziabox system*/
        _hsrv_stats_map_stats_result(&p_hagt_msg->opHagt2Srv.hagt2SrvMsgStatsResponse.rslt, &p_stats_info->stats_rslt);
        if (GLB_STATS_TYPE_OPENFLOW_FLOW == type)
        {
            HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_FLOW_STATS, &p_stats_info->stats_rslt, GLB_STATS_RSLT_LEN(type));
        }
        else if (GLB_STATS_TYPE_OPENFLOW_METER == type)
        {
            HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_METER_STATS, &p_stats_info->stats_rslt, GLB_STATS_RSLT_LEN(type));
        }
        else if (GLB_STATS_TYPE_OPENFLOW_GROUP == type)
        {
            HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_GROUP_STATS, &p_stats_info->stats_rslt, GLB_STATS_RSLT_LEN(type));
        }
        else
        {
            HSRV2SWITCH_MSG_SEND_RESP(type, &p_stats_info->stats_rslt, GLB_STATS_RSLT_LEN(type));
        }
    
        return HSRV_E_NONE;
}

/*********************************************************************
 * Name    : hsrv_stats_get_stats
 * Purpose : This function is used to get statistics from agents
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
int32
hsrv_stats_get_stats(glb_stats_type_t type, uint32 agent_id, HalMsgStatsKey_t* key)
{
    Hsrv2AgtMsg_t hsrv_msg;
    hsrv_stats_info_t* p_stats_info = NULL;

    /*2.Save to db*/
    p_stats_info = &p_g_hsrv_stats_master->stats_info[type];
    sal_memset(p_stats_info, 0, sizeof(hsrv_stats_info_t));
    p_stats_info->agent_id_requested = agent_id;
    p_stats_info->type = type;

    /*4.Send to hal agent*/
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    hsrv_msg.agentId = agent_id;
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgStatsGetStats;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgStatsGetStats.type = type;
    sal_memcpy(&hsrv_msg.opHsrv2Agt.hsrv2AgtMsgStatsGetStats.statsKey, key, sizeof(HalMsgStatsKey_t));
    HSRV2AGT_MSG_SEND(&hsrv_msg);

    /* in pizzabox, hsrv and hagt in same process, so we need to wait for hagt.
    in distributed system, after send msg to linecard, we should set timer to wait response */
    return HSRV_E_NONE;
}

/*********************************************************************
 * Name    : hsrv_stats_clear_stats
 * Purpose : This function is used to clear statistics
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
int32
hsrv_stats_clear_stats(glb_stats_type_t type, uint32 agent_id, HalMsgStatsKey_t* p_key, int32 key_num)
{
    Hsrv2AgtMsg_t hsrv_msg;
    int i;
    /*1.Check & dump information*/
    if (key_num > GLB_STATS_CLEAR_KEY_MAX_NUM || key_num <= 0)
    {
        return HSRV_E_INVALID_PARAM;
    }
    /*2.Send to hal agent*/
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    hsrv_msg.agentId = agent_id;
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgStatsClearStats;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgStatsClearStats.type = type;

    for (i = 0; i < key_num; i++)
    {
        ASN_SET_ADD(&hsrv_msg.opHsrv2Agt.hsrv2AgtMsgStatsClearStats.statsKey.list, &p_key[i]);
    }
    HSRV2AGT_MSG_SEND(&hsrv_msg);
    asn_set_empty(&hsrv_msg.opHsrv2Agt.hsrv2AgtMsgStatsClearStats.statsKey.list);
    
    return HSRV_E_NONE;
}

/*********************************************************************
 * Name    : _hsrv_stats_register_agent_msg_callback
 * Purpose : This function is used registre callback function
 * Input   : N/A 
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
static int32
_hsrv_stats_register_agent_msg_callback(void)
{
    hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgStatsResponse, _hsrv_stats_response);
    
    return HSRV_E_NONE;
}

/*********************************************************************
 * Name    : hsrv_stats_module_start
 * Purpose : This function is used init stats module
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_xxxx
 * Note    : N/A
*********************************************************************/
int32
hsrv_stats_module_start(void)
{
    p_g_hsrv_stats_master = HSRV_CALLOC(MEM_FEA_HSRV_STATS, sizeof(hsrv_stats_master_t));
    if (!p_g_hsrv_stats_master)
    {
        return HSRV_E_NO_MEMORY;
    }
    _hsrv_stats_register_agent_msg_callback();

    return HSRV_E_NONE;
}
