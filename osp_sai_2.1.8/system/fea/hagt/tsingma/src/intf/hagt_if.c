/****************************************************************************
* $Id$
*  Centec Interface private information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-07-23 
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hagt_inc.h"
#include "hagt_if.h"
#include "hagt_if_priv.h"
#include "hagt_stats.h"
#include "ctclib_show.h"
#include "lcapi.h"
#include "hagt_lcmtlk.h"
#include "glb_hw_define.h"
#include "glb_stats_define.h"
/******************************************************************************
* Defines and Macros
******************************************************************************/

/******************************************************************************
* Global and Declaration
******************************************************************************/
hagt_if_master_t* g_pst_hagt_if_master = NULL;
extern hagt_master_t *g_pst_hagt_master;
/* modified by cuixl for bug 17874, 2012-02-27 */
extern sal_time_t _g_ctclib_last_upsec;

/* modified by cuixl for bug _hagt_ifdb_vector_get, 2012-02-24 */
extern void* hagt_ifdb_port_vector_get(uint16 gportid);

extern int32 _hagt_tap_ts_ether_init(uint16 lport);

/* Modifed by weizj for bug 36460 */
/* #ifdef GREATBELT
#define HAGT_CHIP_MAX_PHY_PORT CTC_MAX_PHY_PORT
#endif

#ifdef HUMBER
#define HAGT_CHIP_MAX_PHY_PORT CTC_MAX_HUMBER_PHY_PORT
#endif

#ifdef GOLDENGATE
#define HAGT_CHIP_MAX_PHY_PORT CTC_MAX_PHY_PORT
#endif */

/*********************************************************************
 *  
 * Functions
 *
*********************************************************************/
/*********************************************************************
 * Name    : _hagt_if_stats_map_stats_result
 * Purpose : Map if_statistics result to asn1 ds
 * Input   : N/A
 * Output  : N/A
 * Return  : HSRV_E_XXX
 * Note    : N/A
*********************************************************************/
static int32
_hagt_if_stats_map_stats_result(glb_if_stats_t* p_stats, HalMsgStatsResult_t* resp, uint8 tmp_stats[][8])
{
    HAGT_IF_PTR_VALID_CHECK(p_stats);
    HAGT_IF_PTR_VALID_CHECK(resp);
    resp->present = HalMsgStatsResult_PR_portRslt;

    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->good_octets_rcv      , resp->portRslt.goodOctetsRcv, tmp_stats[0]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->bad_octets_rcv       , resp->portRslt.badOctetsRcv, tmp_stats[1]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->mac_transmit_err     , resp->portRslt.macTransmitErr, tmp_stats[2]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->good_pkts_rcv        , resp->portRslt.goodPktsRcv, tmp_stats[3]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->bad_pkts_rcv         , resp->portRslt.badPktsRcv, tmp_stats[4]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->uc_pkts_rcv          , resp->portRslt.ucPktsRcv, tmp_stats[5]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->brdc_pkts_rcv        , resp->portRslt.brdcPktsRcv, tmp_stats[6]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->mc_pkts_rcv          , resp->portRslt.mcPktsRcv, tmp_stats[7]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->oam_pkts_rcv         , resp->portRslt.oamPktsRcv, tmp_stats[8]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_64_octets       , resp->portRslt.pkts64octets, tmp_stats[9]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_65_127_octets   , resp->portRslt.pkts65To127octets,\
                             tmp_stats[10]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_128_255_octets  , resp->portRslt.pkts128To255octets,\
                             tmp_stats[11]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_256_511_octets  , resp->portRslt.pkts256To511octets,\
                             tmp_stats[12]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_512_1023_octets , resp->portRslt.pkts512To1023octets,\
                             tmp_stats[13]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_1024_max_octets , resp->portRslt.pkts1024TomaxOctets,\
                             tmp_stats[14]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->good_octets_send     , resp->portRslt.goodOctetsSend, tmp_stats[15]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->good_pkts_send       , resp->portRslt.goodPktsSend, tmp_stats[16]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->uc_pkts_send         , resp->portRslt.ucPktsSend, tmp_stats[17]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->mc_pkts_send         , resp->portRslt.mcPktsSend, tmp_stats[18]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->brdc_pkts_send       , resp->portRslt.brdcPktsSend, tmp_stats[19]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->oam_pkts_send        , resp->portRslt.oamPktsSend, tmp_stats[20]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->drop_events          , resp->portRslt.dropEvents, tmp_stats[21]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->undersize_pkts       , resp->portRslt.undersizePkts, tmp_stats[22]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->fragments_pkts       , resp->portRslt.fragmentsPkts, tmp_stats[23]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->oversize_pkts        , resp->portRslt.oversizePkts, tmp_stats[24]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->jabber_pkts          , resp->portRslt.jabberPkts, tmp_stats[25]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->mac_rcv_error        , resp->portRslt.macRcvError, tmp_stats[26]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->bad_crc              , resp->portRslt.badCrc, tmp_stats[27]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->octets_rcv           , resp->portRslt.octetsRcv, tmp_stats[28]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_rcv             , resp->portRslt.pktsRcv, tmp_stats[29]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->octets_send          , resp->portRslt.octetsSend, tmp_stats[30]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_send            , resp->portRslt.pktsSend, tmp_stats[31]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->frame_error          , resp->portRslt.frameError, tmp_stats[32]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pause_rcv            , resp->portRslt.pauseRcv, tmp_stats[33]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pause_send           , resp->portRslt.pauseSend, tmp_stats[34]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->bad_pkts_rcv_sec     , resp->portRslt.badPktsRcvSec, tmp_stats[35]);
    /* modified by cuixl for bug 17874, 2012-02-26 */            
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->octets_rcv_rate     , resp->portRslt.octetsRcvRate, tmp_stats[36]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_rcv_rate     , resp->portRslt.pktsRcvRate, tmp_stats[37]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->octets_send_rate     , resp->portRslt.octetsSendRate, tmp_stats[38]);
    HAGT_IF_UINT64_TO_OCTET_STR(p_stats->pkts_send_rate     , resp->portRslt.pktsSendRate, tmp_stats[39]);
    
    return HAGT_E_NONE;
}
/******************************************************************************
*
*Functions
*
******************************************************************************/
/******************************************************************************
* physical interface attributes
******************************************************************************/


/***************************************************************************************************
 * Name         : hagt_if_tx_link_status 
 * Purpose      : transmit link status, include speed, duplex, link up status and media type      
 * Input        : port_no: physical port no, started from 1          
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : This func is special for port_no by pv_arg is not panel port no.
***************************************************************************************************/
int32
hagt_if_tx_link_status (void* pv_arg)
{
    Hagt2SrvMsg_t hsrv_msg;
    lcapi_lcm_get_port_status_t port_status;
    lcapi_lcm_port_info_t *p_lcm_port_info = NULL;

    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IF_PTR_VALID_CHECK(pv_arg);
    p_lcm_port_info = (lcapi_lcm_port_info_t*)pv_arg;

    /*Fix bug 14686. jqiu 2011-06-15*/
    /*When send request, port_idx is same to port_idx in glb_port_t, when recv answer, port_idx is same to 
        panel_port_no in glb_port_t*/
    port_status.slot_no = p_lcm_port_info->slot_no;
    port_status.port_no = p_lcm_port_info->port_no;
    port_status.sub_port_no = p_lcm_port_info->sub_port_no;
    HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_GET_LINK_STATUS, &port_status)); 
    HAGT_L2IF_DEBUG("  port_no=%d, speed=%d, duplex=%d, flowctrl send=%d, recv=%d, enable=%d, phy_type=0x%x.", 
        port_status.port_no, port_status.speed, port_status.duplex, port_status.flowctrl.send, port_status.flowctrl.recv, port_status.enable, port_status.phy_type); 

    sal_memset(&hsrv_msg, 0, sizeof(Hagt2SrvMsg_t));
    hsrv_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgSetLinkStatus; 
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.slotno = port_status.slot_no;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.portno = port_status.port_no;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.subportno = port_status.sub_port_no;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.speed = port_status.speed;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.duplex = port_status.duplex;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.linkStatus = port_status.enable;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.media = port_status.phy_type;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.flowCtrl.flowCtrlSend = port_status.flowctrl.send;
    hsrv_msg.opHagt2Srv.hagt2SrvMsgSetLinkStatus.flowCtrl.flowCtrlRecv = port_status.flowctrl.recv;

    HAGT2SRV_MSG_SEND(&hsrv_msg);

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : hagt_if_tx_err_crc_log 
 * Purpose      : transmit err crc log request     
 * Input        : port_no: gport          
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : This func is special for port_no by pv_arg is not panel port no.
***************************************************************************************************/
int32
hagt_if_tx_err_crc_log (uint16 gportid, uint64 new_bad_crc_no, uint64 total_bad_crc_no)
{
    Hagt2SrvMsg_t hsrv_msg;
    uint8 tmp_stats[2][8];
    
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_L2IF_DEBUG("gport_no=%d, new_bad_crc_no=%llu, total_bad_crc_no=%llu .", 
                    gportid, new_bad_crc_no, total_bad_crc_no); 

    sal_memset(&hsrv_msg, 0, sizeof(Hagt2SrvMsg_t));
    hsrv_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgWriteCRCErrorLog; 
    hsrv_msg.opHagt2Srv.hagt2SrvMsgWriteCRCErrorLog.gportid = gportid;

    HAGT_IF_UINT64_TO_OCTET_STR(new_bad_crc_no, hsrv_msg.opHagt2Srv.hagt2SrvMsgWriteCRCErrorLog.newCRCno, tmp_stats[0]);
    HAGT_IF_UINT64_TO_OCTET_STR(total_bad_crc_no, hsrv_msg.opHagt2Srv.hagt2SrvMsgWriteCRCErrorLog.totalCRCno, tmp_stats[1]);

    HAGT2SRV_MSG_SEND(&hsrv_msg);

    return HAGT_E_NONE;
}


/***************************************************************************************************
 * Name         : _hagt_if_get_link_status 
 * Purpose      : get link status, include speed, duplex, link up status and media type      
 * Input        : pv_msg_data: physical interface information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_get_link_status (Hsrv2AgtMsg_t* pv_msg_data)
{
    lcapi_lcm_get_port_num_t ports_num;   
    uint8 port_idx; /*for listloop port num, start from 1*/   


    /*dump information*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);  

    /*2. get port numbers*/
    HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_GET_PORT_NUM, &ports_num)); 
    HAGT_L2IF_DEBUG("  port num=%d.", ports_num.port_num); 

    /*3. send to hal server*/  
    
    for (port_idx =1; port_idx <= ports_num.port_num; port_idx++)
    {
       HAGT_IF_ERROR_RETURN(hagt_if_tx_link_status((void*)&port_idx));
    }
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_phy_prop 
 * Purpose      : set port speed, duplex and phy enable      
 * Input        : pv_msg_data: physical interface information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_phy_prop (Hsrv2AgtMsg_t* pv_msg_data)
{    
    uint32 phy_prop_type;
    uint32 phy_prop_value;
    uint8 slot_no;
    uint8 port_no;
    uint8 sub_port_no;
    lcapi_lcm_set_port_status_t port_status; /*set speed, duplex or mac enable*/
    lcapi_lcm_set_port_fec_enable_t fec_en; /*set fec enable*/
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    slot_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetPhyProp.slotno;
    port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetPhyProp.portno;
    sub_port_no  = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetPhyProp.subportno;
    phy_prop_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetPhyProp.type;
    phy_prop_value = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetPhyProp.value;
    if (slot_no != hagt_get_phy_slotid())
    {
        return HAGT_E_NONE;
    }

    HAGT_L2IF_DEBUG("  slot_no=%d, port_no=%d, phy prop type=%d, phy prop value=%d.", 
        slot_no, port_no, phy_prop_type, phy_prop_value);

    /*2. set to SDK, LCM*/
    /*Fix bug 14686. jqiu 2011-06-15*/
    port_status.slot_idx = slot_no;
    port_status.port_idx = port_no;
    port_status.subport_idx = sub_port_no;
    port_status.status = phy_prop_value;
    if (HalIFPhyTypeDefine_speed == phy_prop_type)
    {
        HAGT_L2IF_DEBUG("  set port speed.");        
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_SPEED, &port_status));       
    }
    else if (HalIFPhyTypeDefine_duplex == phy_prop_type)
    {
        HAGT_L2IF_DEBUG("  set port duplex.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_DUPLEX, &port_status));         
    }
    else if (HalIFPhyTypeDefine_media == phy_prop_type)
    {
        HAGT_L2IF_DEBUG("  set port media.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_MEDIA, &port_status));         
    }
    else if (HalIFPhyTypeDefine_phyEnable == phy_prop_type)
    {
        HAGT_L2IF_DEBUG("  set port phy enable.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_PHY_ENABLE, &port_status)); 
        
#ifdef _GLB_UML_SYSTEM_
    lcapi_lcm_port_info_t port_info;
    port_info.slot_no = slot_no;
    port_info.port_no = port_no;
    port_info.sub_port_no = sub_port_no;
    sal_task_sleep(10);
    /*notify hsrv and kernel that interface is up/down, running flag is called by kernel to nsm*/
    HAGT_IF_ERROR_RETURN(hagt_if_tx_link_status((void*)&port_info));
#endif /*_GLB_UML_SYSTEM_*/
    }  
    else if (HalIFPhyTypeDefine_unidir == phy_prop_type)
    {
        HAGT_L2IF_DEBUG("  set port port unidir.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_UNIDIR, &port_status)); 
    } 
    else if (HalIFPhyTypeDefine_fec == phy_prop_type)
    {
        fec_en.slot_idx = slot_no;
        fec_en.port_idx = port_no;
        fec_en.subport_idx = sub_port_no;
        fec_en.enable = phy_prop_value;
        HAGT_L2IF_DEBUG("  set port port fec.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_FEC_ENABLE, &fec_en)); 
    } 
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_set_flow_ctrl 
 * Purpose      : set port flow ctrl, must be physical port       
 * Input        : pv_msg_data: flow ctrl information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_flow_ctrl (Hsrv2AgtMsg_t* pv_msg_data)
{   
    bool enable;
    uint8 slot_no;
    uint8 port_no;
    uint8 sub_port_no;
    uint16 gportid;
    uint16 direction;
#ifdef _GLB_UML_SYSTEM_
    lcapi_lcm_port_info_t lcm_port_info;
#endif/*_GLB_UML_SYSTEM_*/
    lcapi_lcm_set_flowctrl_t port_status; /* set flow control */
    ctc_port_fc_prop_t fc;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
   
    slot_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.slotno;
    port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.portno;
    sub_port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.subportno;
    gportid  = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.gport;
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.enable;
    direction = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctl.dir;
    if (slot_no != hagt_get_phy_slotid())
    {
        return HAGT_E_NONE;
    }

    HAGT_L2IF_DEBUG("  slot_no=%d, port_no=%d, enable=%d, direction=%d.", 
        slot_no, port_no, enable, direction);

    /*2. set SDK, LCM*/
    port_status.slot_idx = slot_no;
    port_status.port_idx = port_no;
    port_status.subport_idx = sub_port_no;
    port_status.direction = direction;
    port_status.enable = enable;
    HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_FLOWCTRL, &port_status));

#ifdef _GLB_UML_SYSTEM_
    /* notify hsrv and kernel that interface is up/down, running flag is called by kernel to nsm*/
    lcm_port_info.slot_no = slot_no;
    lcm_port_info.port_no = port_no;
    lcm_port_info.sub_port_no = sub_port_no;
    HAGT_IF_ERROR_RETURN(hagt_if_tx_link_status((void *)&lcm_port_info));
#endif/*_GLB_UML_SYSTEM_*/

    //10G and 40G unspport consult, just set to sdk
    fc.gport = gportid;
    //fc.cos   = 0;
    fc.is_pfc = FALSE;
    fc.enable = enable;
    if (direction)
    {
         fc.dir = CTC_EGRESS;
    }
    else
    {
         fc.dir = CTC_INGRESS;
    }
    HAGT_IF_ERROR_RETURN(ctc_port_set_flow_ctl_en(&fc));
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_jumbo_frame_enable 
 * Purpose      : set system jumbo frame enable      
 * Input        : pv_msg_data: frame size information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_jumbo_frame_enable (Hsrv2AgtMsg_t* pv_msg_data)
{  
    uint16 gportid;
    uint16 size;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetJumboFrameEn.gportid;
    size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetJumboFrameEn.enable;

    if (0) // if (hagt_stack_is_stack_port(gportid))
    {
        size = size + 200; /*stacking port, jumber enable, add stacking header length*/
    }
    
    HAGT_L2IF_DEBUG("  gport id=%d, size=%d.", gportid, size);

    /*2. set SDK*/
    if (!HAGT_GPORT_IS_LOCAL(gportid))
    {
        return HAGT_E_NONE;        
    }
    HAGT_IF_ERROR_RETURN(ctc_port_set_max_frame(gportid, size));
    
    return HAGT_E_NONE;
}


#if 0
/* modified by chenxw for bug 24578, 2013-08-22. */ 
/***************************************************************************************************
 * Name         : _hagt_if_set_flow_ctrl 
 * Purpose      : set port flow ctrl, must be physical port       
 * Input        : pv_msg_data: flow ctrl information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_flow_ctrl_enable (Hsrv2AgtMsg_t* pv_msg_data)
{
#if 0
    bool enable;
    ctc_qos_glb_cfg_t glb_cfg;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
   
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSetFlowctlEnable.enable;

    HAGT_L2IF_DEBUG("  enable=%d.", enable);    

    /* no need to memset */
    sal_memset (&glb_cfg, 0, sizeof (glb_cfg));
    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_EGR_MGT_EN;
    if (enable)
    {
        glb_cfg.u.value = 0;
    }
    else
    {
        glb_cfg.u.value = 1;  
    }
    HAGT_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
#endif    

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_phy_prop 
 * Purpose      : set port speed, duplex and phy enable      
 * Input        : pv_msg_data: physical interface information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_poe_prop (Hsrv2AgtMsg_t* pv_msg_data)
{    
    uint32 poe_prop_type;
    uint32 poe_prop_value;
    uint8 slot_no;
    uint8 port_no;
    lcapi_lcm_set_poe_status_t poe_status; /*set poe enable, budget*/
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    slot_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetPOEProp.slotno;
    port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetPOEProp.portno;
    sub_port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetPOEProp.subportno;
    poe_prop_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetPOEProp.type;
    poe_prop_value = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetPOEProp.value;
    if (slot_no != hagt_get_phy_slotid())
    {
        return HAGT_E_NONE;
    }

    HAGT_L2IF_DEBUG("  slot_no=%d, port_no=%d, poe prop type=%d, poe prop value=%d.", 
        slot_no, port_no, poe_prop_type, poe_prop_value);

    /*2. set to SDK, LCM*/
    poe_status.slot_idx = slot_no;
    poe_status.port_idx = port_no;
    poe_status.subport_idx = sub_port_no;
    poe_status.status = poe_prop_value;
    if (HalIFPOETypeDefine_enable == poe_prop_type)
    {
        HAGT_L2IF_DEBUG("  set poe enable.");        
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_ENABLE, &poe_status));       
    }
    else if (HalIFPOETypeDefine_budget == poe_prop_type)
    {
        HAGT_L2IF_DEBUG("  set poe budget.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_BUDGET, &poe_status));         
    }
    else if (HalIFPOETypeDefine_priority == poe_prop_type)
    {
        HAGT_L2IF_DEBUG("  set poe priority.");       
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_PRIORITY, &poe_status));         
    }
    
    return HAGT_E_NONE;
}

/* bug25647, qicx modified to support 10G/1G XFI/XSGMII dynamic switch,2013-11-30 */
static int32
_hagt_if_map_serdes_mode(uint32* serdes_mode)
{
    switch(*serdes_mode)
    {
        case GLB_IF_SERDES_XFI_MODE:
            *serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
            break;
        case GLB_IF_SERDES_SGMII_MODE:
            *serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
            break;
        case GLB_IF_SERDES_XSGMII_MODE:
            *serdes_mode = CTC_CHIP_SERDES_XSGMII_MODE;
            break;
        case GLB_IF_SERDES_QSGMII_MODE:
            *serdes_mode = CTC_CHIP_SERDES_QSGMII_MODE;
            break;
        case GLB_IF_SERDES_XAUI_MODE:
            *serdes_mode = CTC_CHIP_SERDES_XAUI_MODE;
            break;
        default:
            HAGT_IF_LOG_ERR("Cannot map serdes mode to greatbelt SDK.");
            return HAGT_E_INVALID_PARAM;
    }
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_serdes_mode 
 * Purpose      : set port speed, duplex and phy enable      
 * Input        : pv_msg_data: physical interface information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_serdes_mode(Hsrv2AgtMsg_t* pv_msg_data)
{    
    uint8 set_serdes_mode_flag;
    uint32 serdes_mode;
    uint8 slot_no;
    uint8 port_no;
    ctc_chip_serdes_info_t serdes_info;
    lcapi_lcm_get_chip_serdes_id_t chip_serdes_info;
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    slot_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetSerDesMode.slotno;
    port_no = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetSerDesMode.portno;
    set_serdes_mode_flag = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetSerDesMode.setserdesmodeflag;
    HAGT_L2IF_DEBUG("  slot_no=%d, port_no=%d, serdes mode=%d.", 
        slot_no, port_no, set_serdes_mode_flag);


    /* see zebos/lib/if.h definition and zebos/nsm/nsm_fea.c, function nsm_fea_if_speed_change() */
    /* XFI->XSGMII */
    if (0x1 == (set_serdes_mode_flag & 0xf))
    {
        serdes_mode = GLB_IF_SERDES_XSGMII_MODE;
    }
    /* XSGMII->XFI */
    else if (0x2 == (set_serdes_mode_flag & 0xf))
    {
        serdes_mode = GLB_IF_SERDES_XFI_MODE;
    }
    /* currently don't support other switch mode */
    else
    {
        return HAGT_E_INVALID_PARAM;
    }
    HAGT_IF_ERROR_RETURN(_hagt_if_map_serdes_mode(&serdes_mode));

    /*2. get serdes id from port no*/
    chip_serdes_info.slot_idx = slot_no;
    chip_serdes_info.port_idx = port_no;
    HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_GET_CHIP_SERDES_ID, &chip_serdes_info)); 
    HAGT_L2IF_DEBUG("  slot_no=%d, port_no=%d, chip_serdes_id=%d.", 
        chip_serdes_info.slot_idx, chip_serdes_info.port_idx, chip_serdes_info.chip_serdes_id); 

    if (chip_serdes_info.chip_serdes_id < 0)
    {
        HAGT_IF_LOG_ERR("Cannot set serdes mode.");
        return HAGT_E_INVALID_PARAM;
    }
    serdes_info.serdes_id = chip_serdes_info.chip_serdes_id;
    serdes_info.serdes_mode = serdes_mode;

    HAGT_L2IF_DEBUG("  set serdes mode.");    

    /*3. set to SDK*/
    HAGT_IF_ERROR_RETURN(ctc_chip_set_serdes_mode(0, &serdes_info));
    
    return HAGT_E_NONE;
}
/* End of qicx added for bug25647 */

#endif

/***************************************************************************************************
 * Name         : _hagt_if_set_stats_intval 
 * Purpose      : set phy port or agg port load intval      
 * Input        : pv_msg_data: physical or agg interface information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_stats_intval (Hsrv2AgtMsg_t* pv_msg_data)
{    
    uint16 gportid;
    uint32 intval;
    uint32 type;
    hagt_if_port_info_t* pst_port_info = NULL;
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetHagtStatsIntval.gportid;
    intval = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetHagtStatsIntval.intval;
    type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetHagtStatsIntval.type;
    HAGT_L2IF_DEBUG("  gportid=%d, intval=%d, type=%d.", gportid, intval, type);

    HAGT_IF_STATS_LOCK;

    pst_port_info = hagt_ifdb_port_vector_get(gportid);  
    if (NULL == pst_port_info)
    {
        HAGT_IF_STATS_UNLOCK;
        return HAGT_E_NONE;
    }

    pst_port_info->notify_intval[type] = intval;
    /* we support configure on agg port or member port, follow cisco */
#if 0    
    if (GLB_IS_LINKAGG_PORT(gportid))
    {
        /* set notify intval to agg member */
        _hagt_if_set_agg_stats_intval(pst_port_info);
    }
#endif    
    HAGT_IF_STATS_UNLOCK;

    return HAGT_E_NONE;
}

int32
hagt_if_create_destroy_port(uint16 gportid, uint16 frame_size, mac_addr_t mac_addr, bool enable,
                            uint16 slotno, uint16 portno, uint16 subportno)
{   
    //ctc_port_mac_prefix_t mac_prefix;
    ctc_port_mac_postfix_t mac_postfix;
    ctc_port_fc_prop_t fc;
    uint8 gchip;
    int32 lchip;
    uint16 lport;
    
    /*1. get values and dump*/
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    /* modified by liyf for greatbelt system port mac */
    //sal_memset(&mac_prefix, 0, sizeof(ctc_port_mac_prefix_t));
    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    /* Modifed by weizj for bug 36460 */
    gchip = GLB_GET_GLOBAL_CHIPID(gportid);
    lport = GLB_GET_LOCAL_PORTID(gportid);
    lchip = hagt_chip_glb2local(gchip);

    /*2. store to database*/
    if (enable)
    {
        if (HAGT_GPORT_IS_LOCAL(gportid))
        {
            lcapi_hagt_set_mac_en_t port_status;    /*modified by xgu for bug 13493, 2011-01-11*/
        
            HAGT_IF_ERROR_RETURN(hagt_ifdb_create_interface(gportid, slotno, portno, subportno));        
            /*Added by xgu, set port's tx/rx to false, 2013-2-27*/
            HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(gportid, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en(gportid, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_port_set_bridge_en(gportid, TRUE));

            HAGT_IF_ERROR_RETURN(ctc_port_set_property(gportid, CTC_PORT_PROP_PTP_EN, 1));

            /*modify by wangl for port use label, modified by  ychen for bug 25183 in 2013-10-11*/            
            HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, CTC_BOTH_DIRECTION, 1));

            HAGT_IF_ERROR_RETURN(ctc_port_set_max_frame(gportid, frame_size));

#if 0 // TODO: Commented by xgu for compile pass, 2012-12-17
            /*_hagt_if_set_max_frame_size: in GMAC/XGMAC/SGMAC,
              ptp_en will attach 8 bytes for incoming timestamp and remove in NETRX*/
            HAGT_IF_ERROR_RETURN(ctc_ptp_port_enable(gportid, TRUE));
#endif            
            if(GLB_OUTER_VLAN_IS_SVLAN)
            {
                HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_domain(gportid, CTC_PORT_VLAN_DOMAIN_SVLAN));
             }
            else
            {
                HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_domain(gportid, CTC_PORT_VLAN_DOMAIN_CVLAN));
            }
            /*temp code for remote mirror*/
            HAGT_IF_ERROR_RETURN(ctc_port_set_stag_tpid_index(gportid, CTC_BOTH_DIRECTION, 2));
            /*added by wangl for efd*/
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(gportid, CTC_PORT_PROP_NVGRE_ENABLE, 1));

            /*modified by xgu for bug 13493, 2011-01-11*/
            HAGT_L2IF_DEBUG("  set port created enable.");
            port_status.gport = gportid;
            port_status.flag = TRUE;
            HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_PORT_CREATE_DONE, &port_status)); 

            // TODO:  merge sdk 2.1.13
            fc.gport = gportid;
            //fc.cos   = 0;
            fc.enable = FALSE;
            fc.dir = GLB_INGRESS;
            fc.is_pfc = FALSE;
            ctc_port_set_flow_ctl_en(&fc);  

            fc.dir = GLB_EGRESS;  
            ctc_port_set_flow_ctl_en(&fc);
            /*modified by wangl for TAP*/
            //_hagt_tap_ts_ether_init(lport);

            /* Modifed by weizj for bug 36460 */
            g_pst_hagt_if_master->port_stats_info.p_port_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(glb_if_stats_t));
            g_pst_hagt_if_master->port_stats_info.p_mac_rx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(ctc_stats_mac_rec_t));
            g_pst_hagt_if_master->port_stats_info.p_mac_tx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(ctc_stats_mac_snd_t));

            /* Modifed by wangl for set queue */
            ctc_qos_queue_cfg_t que_cfg;
            uint16 queue_id = 0;
            for(queue_id =0; queue_id < 8; queue_id++)
            {
                sal_memset(&que_cfg, 0x0, sizeof(ctc_qos_queue_cfg_t));
                que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
                que_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                que_cfg.value.stats.queue.queue_id = queue_id;
                que_cfg.value.stats.queue.gport = gportid;
                que_cfg.value.stats.stats_en = 1;
                ctc_qos_set_queue(&que_cfg);
            }
        }

#if 0 /* Commented by xgu, 2012-12-17, gb sdk init this in function sys_greatbelt_port_init*/
        /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
        nh_type = CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU;
        HAGT_IF_ERROR_RETURN(ctc_l2_create_ucast_nh(gportid, nh_type)); /*this is global, all line card should create it*/
#endif        
    }
    else
    {
#if 0 /* Commented by xgu, 2012-12-17, gb sdk init this in function sys_greatbelt_port_init*/
        HAGT_IF_ERROR_RETURN(ctc_l2_delete_ucast_nh(gportid));
#endif        
        if (HAGT_GPORT_IS_LOCAL(gportid))
        {
            lcapi_hagt_set_mac_en_t port_status;    /*modified by xgu for bug 13493, 2011-01-11*/

        
            HAGT_IF_ERROR_RETURN(ctc_port_set_bridge_en(gportid, FALSE));
            HAGT_IF_ERROR_RETURN(hagt_ifdb_destroy_interface(gportid)); 
	        /*modify by wangl for port use label, modified by  ychen for bug 25183 in 2013-10-11*/            
            HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(gportid, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, CTC_BOTH_DIRECTION, 0));

            /* Modifed by weizj for bug 36460 */
            HAGT_FREE(CTCLIB_MEM_INTF_MODULE, g_pst_hagt_if_master->port_stats_info.p_port_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport]);
            g_pst_hagt_if_master->port_stats_info.p_port_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = NULL;
            HAGT_FREE(CTCLIB_MEM_INTF_MODULE, g_pst_hagt_if_master->port_stats_info.p_mac_rx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport]);
            g_pst_hagt_if_master->port_stats_info.p_mac_rx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = NULL;
            HAGT_FREE(CTCLIB_MEM_INTF_MODULE, g_pst_hagt_if_master->port_stats_info.p_mac_tx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport]);
            g_pst_hagt_if_master->port_stats_info.p_mac_tx_stats[lchip*GLB_PORT_NUM_PER_CHIP + lport] = NULL;

            /*modified by xgu for bug 13493, 2011-01-11*/
            port_status.gport = gportid;
            port_status.flag = FALSE;
            HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_PORT_CREATE_DONE, &port_status)); 
        }
    }    

    // TODO: should we consider set port to access port in default?
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_create_destroy_port 
 * Purpose      : create port or destroy port, only for database storage       
 * Input        : pv_msg_data: port create/destroy information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_create_destroy_port (Hsrv2AgtMsg_t* pv_msg_data)
{   
    uint16 gportid;
    uint16 frame_size;
    bool enable;
    /* modified by liyf for greatbelt system port mac */
    mac_addr_t mac_addr;
    
#if 0 /* Commented by xgu, 2012-12-17, gb sdk init this in function sys_greatbelt_port_init*/
    /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
    ctc_nh_param_brguc_sub_type_t nh_type;
#endif

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.gportid;
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.enable;
    frame_size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.framesize;

    /* modified by liyf for greatbelt system port mac */
    if (enable)
    {
        sal_memcpy(mac_addr, pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.portmac.buf,
            pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.portmac.size);
        HAGT_L2IF_DEBUG("  gportid=%d, enable=%d portmac=0x%02x%02x.%02x%02x.%02x%02x.",
            gportid, enable, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    }
    else
    {
        HAGT_L2IF_DEBUG("  gportid=%d, enable=%d.", gportid, enable);
    }
    
    return hagt_if_create_destroy_port(gportid, frame_size, mac_addr, enable,
                pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.slotno,
                pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.portno,
                pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfCreatePort.subportno);
}

#if 0
/******************************************************************************
* system attributes
******************************************************************************/
/***************************************************************************************************
 * Name         : _hagt_if_set_jumbo_frame_enable 
 * Purpose      : set system jumbo frame enable      
 * Input        : pv_msg_data: frame size information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_jumbo_frame_enable (Hsrv2AgtMsg_t* pv_msg_data)
{  
    uint16 gportid;
    uint16 size;
    ctc_qos_queue_cfg_t que_cfg = { 0 };

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
   
    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetJumboFrameEn.gportid;
    size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetJumboFrameEn.enable;

    if (0) // if (hagt_stack_is_stack_port(gportid))
    {
        size = size + 200; /*stacking port, jumber enable, add stacking header length*/
    }
    
    HAGT_L2IF_DEBUG("  gport id=%d, size=%d.", gportid, size);

    /*2. set SDK*/
    if (!HAGT_GPORT_IS_LOCAL(gportid))
    {
        return HAGT_E_NONE;        
    }    
    HAGT_IF_ERROR_RETURN(ctc_port_set_max_frame(gportid, size));

/* modified by cuixl , 2013-10-17 */
/* modification start */  
    que_cfg.type = CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU;    
    que_cfg.value.value_32 = gportid << 16 | size;
    HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&que_cfg));
/* modification end */
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_system_routermac 
 * Purpose      : set system routermac      
 * Input        : pv_msg_data: routemac information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_system_routermac (Hsrv2AgtMsg_t* pv_msg_data)
{
    mac_addr_t routemac; 

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);      

    sal_memcpy(routemac, pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermac.rmac.buf, 
        pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermac.rmac.size);
    HAGT_L2IF_DEBUG("  system routemac=0x%02x%02x.%02x%02x.%02x%02x.", 
        routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);

    /*2. set to SDK*/
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_router_mac(routemac));

    /* modified by liyf for greatbelt system */
    /* set port mac prefix */
    /*
    CTC_SET_FLAG(port_mac.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
    sal_memcpy(port_mac.port_mac[0], routemac, sizeof(mac_addr_t));
    HAGT_L2IF_DEBUG("  portmac prefix type=%d,mac prefix=0x%02x%02x.%02x%02x.%02x%02x.",
        port_mac.prefix_type, port_mac.port_mac[0][0], port_mac.port_mac[0][1], port_mac.port_mac[0][2],
        port_mac.port_mac[0][3], port_mac.port_mac[0][4], port_mac.port_mac[0][5]);
    HAGT_IF_ERROR_RETURN(ctc_port_set_port_mac_prefix(&port_mac));
    */

    /* modified by liyf for greatbelt system */
    /* set oam bridge mac */
    if ((TRUE == hagt_gb_get_feature_enabled(HAGT_FEA_OAM_ENABLED)))
    {
        /* modified by liyf for greatbelt system */
        /* ctc_port_mac_prefix_t port_mac; */
        ctc_oam_property_t prop;

        /* modified by liyf for greatbelt system */
        /* sal_memset(&port_mac, 0, sizeof(ctc_port_mac_prefix_t)); */
        sal_memset(&prop, 0, sizeof(ctc_oam_property_t));
          
        prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
        prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC;
        prop.u.y1731.p_value = &routemac;
        HAGT_L2IF_DEBUG("  oam bridgemac=0x%02x%02x.%02x%02x.%02x%02x.",
            routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);
        HAGT_IF_ERROR_RETURN(ctc_oam_set_property(&prop));
    }

    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_set_system_routermac_prefix 
 * Purpose      : set system routermac  prefix to DS_ROUTERMAC_CTRL   
 * Input        : pv_msg_data: routemac prefix information           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_set_system_routermac_prefix (Hsrv2AgtMsg_t* pv_msg_data)
{ 
    mac_addr_t routemac;
    uint8 routemac_type;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    routemac_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermacPrefix.prefixType;
    sal_memcpy(routemac, pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermacPrefix.rmacPrefix.buf, 
        pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermacPrefix.rmacPrefix.size);
    HAGT_L2IF_DEBUG("  prefix type=%d, system routemac prefix=0x%02x%02x.%02x%02x.%02x%02x.", 
        routemac_type, routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);
    
    /*2. set to SDK*/
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_vmac_prefix(routemac_type, routemac));
    
    return HAGT_E_NONE;
}
#endif
/***************************************************************************************************
 * Name         : _hagt_if_set_running_flag 
 * Purpose      : set/unset interface running flag  
 * Input        : pv_msg_data: portid, running enable          
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical port, agg port is not allowed
***************************************************************************************************/
static int32
_hagt_if_set_running_flag (Hsrv2AgtMsg_t* pv_msg_data)
{  
    bool enable;
    uint32 ports_num;
    uint32 index; /*for list loop the port, must be physical interface*/
    uint16 gportid;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg info*/
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfFlagsSet.enable;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfFlagsSet.gportid.int32.list.count;
    HAGT_L2IF_DEBUG("  enable=%d, ports_num=%d.", enable, ports_num);

    /*3. set port flag to SDK*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfFlagsSet.gportid.int32.list.array[index]);
        HAGT_L2IF_DEBUG("  [%d]gportid=%d, enable=%d.", index, gportid, enable);

        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }
        //HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_RUNNING, enable));
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en(gportid, enable));
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(gportid, enable)); 
    }  
  
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_set_port_routed 
 * Purpose      : set port type to routed or switched  
 * Input        : pv_msg_data: gportid(for agg), memberportid, l3ifid(route_en), untag default vlan         
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : 1)layer3 interface attributes setted by l3ifid, but port attribute setted by physical portid
                  2)ctc_l2_create_ucast_nh, ucast nh create or destroy should be cared only when port create or destroy
***************************************************************************************************/
static int32
_hagt_if_set_port_routed (Hsrv2AgtMsg_t* pv_msg_data)
{ 
    bool route_en;
    uint16 l3ifid;
    uint16 gportid; /*only when route enable, this variable is needed*/
    uint32 ports_num;/*only when route enable, this variable is needed*/
    uint16 mem_portid;
    uint32 index; /*for list loop the port, must be physical interface*/
    bool untag_dft_vid;
    ctc_l3if_t l3if_info; 

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);    

    /*2. get msg information*/
    route_en = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.routeEn;
    l3ifid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.l3ifid;
    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.gportid;
    untag_dft_vid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.unTagDefVid;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.memportid.int32.list.count;
    HAGT_L2IF_DEBUG("  route_en=%d, l3ifid=%d, gportid=%d, untag def vid=%d, ports_num=%d.", 
        route_en, l3ifid, gportid, untag_dft_vid, ports_num);

    /*3. process routing enable information*/
    l3if_info.gport = gportid;
    l3if_info.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if_info.vlan_id = GLB_INVALID_VLANID;
    if (route_en)
    {
        HAGT_IF_ERROR_RETURN(ctc_l3if_create(l3ifid, &l3if_info));
        //HAGT_IF_ERROR_RETURN(hagt_l3if_set_default_attributes(l3ifid, &l3if_info));         
    }
    else
    {
        //HAGT_IF_ERROR_RETURN(hagt_l3if_clear_all_attributes(l3ifid, &l3if_info));
        HAGT_IF_ERROR_RETURN(ctc_l3if_destory(l3ifid, &l3if_info));    
    }

    /*5. set physical interface route enable*/
    for (index = 0; index < ports_num; index++)
    {        
        mem_portid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortRouted.memportid.int32.list.array[index]);
        HAGT_L2IF_DEBUG("  [%d]member portid=%d, route_en=%d", index, mem_portid, route_en);

        /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(mem_portid))
        {
            continue;
        } 

        /*6. process other layer2 or layer3 common features*/
#if 0 /*Commented by xgu, sdk will do these config in sys_greatbelt_port_set_phy_if_en, 2013-1-11*/
        HAGT_IF_ERROR_RETURN(ctc_port_set_bridge_en(mem_portid, TRUE));      
        HAGT_IF_ERROR_RETURN(ctc_port_set_default_vlan(mem_portid, 1));
#endif        
        if (route_en)
        {     
            /*dest port stp check disable is not need in humber, epe will do routed port check*/
            HAGT_IF_ERROR_RETURN(ctc_stp_clear_all_inst_state(mem_portid));            
        }

        /*clear all l2if port flag when routing enabled*/
        HAGT_IF_ERROR_RETURN(hagt_ifdb_change_port_mode(mem_portid, route_en));
        HAGT_IF_ERROR_RETURN(ctc_port_set_phy_if_en(mem_portid, route_en)); 
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(mem_portid, CTC_PORT_PROP_BRIDGE_EN, 1)); 

        /*for bug 13143*/
        HAGT_IF_ERROR_RETURN(ctc_port_set_learning_en(mem_portid,(route_en ? FALSE:TRUE)));
    }
  
    return HAGT_E_NONE;
}

static int32
_hagt_if_set_system_routermac(Hsrv2AgtMsg_t* pv_msg_data)
{
    mac_addr_t routemac;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);  

    sal_memcpy(routemac, pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermac.rmac.buf, 
        pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysSetRoutermac.rmac.size);
    HAGT_L2IF_DEBUG("  system routemac=0x%02x%02x.%02x%02x.%02x%02x.", 
        routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);

    /*2. set to SDK*/
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_router_mac(routemac));

#ifdef HAVE_ETH_OAM    
    if (g_pst_hagt_master->is_ethoam_supported) /*added by ychen for ms image without oam key entry, for bug 16547*/
    {
        /* modified by liwh to support cfm for bug 14710 */
 // TODO: Commented by xgu for compile, 20121120          HAGT_IF_ERROR_RETURN(ctc_eth_oam_set_global_parameter(ETH_OAM_BRIDGE_MAC, routemac));
    }
#endif

    return HAGT_E_NONE;
}

static int32
_hagt_if_set_crc_chk_en(Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 crc_chk_en;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*2. get msg information*/
    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetCrcCheckEn.gportid;
    crc_chk_en = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetCrcCheckEn.enable;
    HAGT_L2IF_DEBUG("  enable=%d, gportid=%d.", crc_chk_en, gportid);
    
    /*3. set to SDK and store to database*/
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gportid, CTC_PORT_PROP_ERROR_CHECK, crc_chk_en));
    
    return HAGT_E_NONE;
}

static int32
_hagt_if_set_crc_recalculation_en(Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 crc_chk_en;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*2. get msg information*/
    gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetCrcRecalcEn.gportid;
    crc_chk_en = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetCrcRecalcEn.enable;
    HAGT_L2IF_DEBUG("  enable=%d, gportid=%d.", crc_chk_en, gportid);
    
    /*3. set to SDK and store to database*/
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gportid, CTC_PORT_PROP_CRC_RECALC_EN, crc_chk_en));

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_mapping_vlan_filter_type 
 * Purpose      : mapping vlan filter type to ctc filter type 
 * Input        : filter_type:   HalIFVlanFilterDefine  
                  gportid: global portid
                  enable: TRUE=enable, FALSE=disable
 * Output       : N/A           
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static inline int32
_hagt_if_mapping_vlan_filter_type (uint16 gportid, uint32 dir, uint32 enable)
{
    if ((GLB_INGRESS == dir) || (GLB_BOTH_DIRECTION == dir))
    {
        /* Modified by Yan Xing'an, 2015-3-13, for openflow, bug 31785 */
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_INGRESS_FILTER_EN, enable));
        /* HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(gportid, GLB_INGRESS, enable)); */
    }
    
    if ((GLB_EGRESS == dir) || (GLB_BOTH_DIRECTION == dir))
    {
        /* Modified by Yan Xing'an, 2015-3-13, for openflow, bug 31785 */
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_EGRESS_FILTER_EN, enable));
        /* HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(gportid, GLB_EGRESS, enable)); */
    }

    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_set_vlan_filter_en 
 * Purpose      : set vlan ingress or egress filter 
 * Input        : pv_msg_data: memberportid, filter type, filter enable         
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_vlan_filter_en (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 ports_num;/*only when route enable, this variable is needed*/   
    uint32 index; /*for list loop the port, must be physical interface*/
    uint32 dir;
    uint32 enable;
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    dir = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.filterDir;
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.filterEnable;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.gportid.int32.list.count;
    HAGT_L2IF_DEBUG("  dir=%d, enable=%d, ports_num=%d.", 
        dir, enable, ports_num);

    /*3. set to SDK and store to database*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.gportid.int32.list.array[index]);  

         /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }
        HAGT_IF_ERROR_RETURN(_hagt_if_mapping_vlan_filter_type(gportid, dir, enable));
    }
    
    return HAGT_E_NONE;
}

static int32
_hagt_if_mapping_port_vlan_type (uint16 gportid, uint32 port_type)
{    
    hagt_if_flag_t flag = 0;

    /* add vlan mapping codes by liwh for bug 13480, 2010-11-15
       unset switchport mode qinq when port type is changed */
    HAGT_IF_ERROR_RETURN(hagt_ifdb_get_port_flag(gportid, &flag));
    if (GLB_FLAG_ISSET(flag, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE)
        && (GLB_VLAN_PORT_TYPE_QINQ != port_type))
    {
#ifdef TODO
        HAGT_IF_ERROR_RETURN(hagt_vlan_mapping_unset_port_qinq_mode(gportid));
#endif
    }

    if (GLB_VLAN_PORT_TYPE_ACCESS == port_type)
    {
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, 
            (HAGT_IF_PORT_FLAG_ALLOW_TAGGED|HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED), FALSE));
        
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED, TRUE));
    }
    else if (GLB_VLAN_PORT_TYPE_TRUNK == port_type)
    {
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, 
            (HAGT_IF_PORT_FLAG_ALLOW_TAGGED|HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED), TRUE));   
#ifdef TODO
        /* modified by liwh for bug 13408, 2010-11-26 */
        if (GLB_FLAG_ISSET(flag, HAGT_IF_PORT_FLAG_VLAN_CLASS_ENABLE))
        {  
            HAGT_IF_ERROR_RETURN(hagt_vlan_class_set_port_flag(gportid, port_type, TRUE));
            return HAGT_E_NONE;
        }
#endif
    }
    /* add vlan mapping codes by liwh for bug 13480, 2010-11-15
       set switchport mode qinq */
    else if ((GLB_VLAN_PORT_TYPE_QINQ == port_type)
        && (!GLB_FLAG_ISSET(flag, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE)))
    {
#ifdef TODO
        HAGT_IF_ERROR_RETURN(hagt_vlan_mapping_set_port_qinq_mode(gportid));
#endif
    }


    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_port_vlan_type 
 * Purpose      : set switch port type, access or trunk 
 * Input        : pv_msg_data: memberportid, filter type, filter enable , untag pvid, port type        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_port_vlan_type (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 ports_num;/*only when route enable, this variable is needed*/   
    uint32 index; /*for list loop the port, must be physical interface*/
    uint32 port_type; /*glb_vlan_port_type_t*/

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    port_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanPortType.portType;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanPortType.gportid.int32.list.count;
    HAGT_L2IF_DEBUG("  port type=%d, ports_num=%d.", 
        port_type, ports_num);
    
    /*3. set to SDK and store to database*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanPortType.gportid.int32.list.array[index]);  

         /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }

        /*c. set port type, may affect vlan tag ctrl, dot1q type*/
        HAGT_IF_ERROR_RETURN(_hagt_if_mapping_port_vlan_type(gportid, port_type));
    }
    
    return HAGT_E_NONE;
}


/***************************************************************************************************
 * Name         : _hagt_if_set_port_pvid 
 * Purpose      : set port pvid
 * Input        : pv_msg_data: memberportid, default vid       
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_port_pvid (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 ports_num;/*only when route enable, this variable is needed*/   
    uint32 index; /*for list loop the port, must be physical interface*/
    uint16 def_vid;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*2. get msg information*/
    def_vid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortDefVid.vid;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortDefVid.port.int32.list.count;
    HAGT_L2IF_DEBUG("  default vlan=%d, ports_num=%d.", def_vid, ports_num);

    if (!def_vid)
    {
        return HAGT_E_NONE; /*when routed port, this function is called with default vlan 0*/
    }

    /*3. set to SDK*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortDefVid.port.int32.list.array[index]);
        HAGT_L2IF_DEBUG("  [%d]gportid=%d, vid=%d.", index, gportid, def_vid);

         /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }

        HAGT_IF_ERROR_RETURN(ctc_port_set_default_vlan(gportid, def_vid));
    }
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_port_untag_pvid 
 * Purpose      : set/unset port untag pvid enable 
 * Input        : pv_msg_data: memberportid, default vid       
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_port_untag_pvid (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 ports_num;/*only when route enable, this variable is needed*/   
    uint32 index; /*for list loop the port, must be physical interface*/
    bool untag_dft_vid;
    bool untag_svlan;
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    untag_dft_vid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetUntagPvid.untagPvid;
    untag_svlan = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetUntagPvid.untagSvlan;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetUntagPvid.port.int32.list.count;
    HAGT_L2IF_DEBUG("  untag_dft_vid=%d, untag_svlan=%d, ports_num=%d.", 
        untag_dft_vid, untag_svlan, ports_num);

    /*3. set to SDK*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetUntagPvid.port.int32.list.array[index]);
        HAGT_L2IF_DEBUG("  [%d]gportid=%d, untag def vid=%d, untag svlan=%d.", 
            index, gportid, untag_dft_vid, untag_svlan);

        /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }
        HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(gportid, untag_dft_vid, untag_svlan));
    }
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_plus_mac_rx_tx_stats 
 * Purpose      : plus rx_tx stats
 * Input        : N/A     
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_plus_mac_rx_tx_stats(bool is_10G_port,
                            ctc_stats_mac_rec_t* p_sum_rx,
                            ctc_stats_mac_snd_t* p_sum_tx,
                            ctc_stats_mac_rec_t* p_rx,
                            ctc_stats_mac_snd_t* p_tx,
                            bool is_stats_accumulted)
{

#define PLUS_MAC_STATS(sum, a)                                          \
do{                                                                     \
    if(!is_stats_accumulted)                                               \
    {                                                                   \
        sum += a;                                                         \
    }                                                                   \
    else                                                                \
    {                                                                   \
        sum = a;                                                       \
    }                                                                   \
}while(0)

    PLUS_MAC_STATS(p_sum_rx->good_ucast_pkts       , p_rx->good_ucast_pkts         );
    PLUS_MAC_STATS(p_sum_rx->good_ucast_bytes      , p_rx->good_ucast_bytes        );
    PLUS_MAC_STATS(p_sum_rx->good_mcast_pkts       , p_rx->good_mcast_pkts         );
    PLUS_MAC_STATS(p_sum_rx->good_mcast_bytes      , p_rx->good_mcast_bytes        );
    PLUS_MAC_STATS(p_sum_rx->good_bcast_pkts       , p_rx->good_bcast_pkts         );
    PLUS_MAC_STATS(p_sum_rx->good_bcast_bytes      , p_rx->good_bcast_bytes        );
    PLUS_MAC_STATS(p_sum_rx->good_pause_pkts       , p_rx->good_pause_pkts         );
    PLUS_MAC_STATS(p_sum_rx->good_pause_bytes      , p_rx->good_pause_bytes        );
    PLUS_MAC_STATS(p_sum_rx->good_control_pkts     , p_rx->good_control_pkts       );
    PLUS_MAC_STATS(p_sum_rx->good_control_bytes    , p_rx->good_control_bytes      );
    PLUS_MAC_STATS(p_sum_rx->jabber_pkts           , p_rx->jabber_pkts             );
    PLUS_MAC_STATS(p_sum_rx->jabber_bytes          , p_rx->jabber_bytes            );
    PLUS_MAC_STATS(p_sum_rx->collision_pkts        , p_rx->collision_pkts          );
    PLUS_MAC_STATS(p_sum_rx->collision_bytes       , p_rx->collision_bytes         );
    PLUS_MAC_STATS(p_sum_rx->fcs_error_pkts        , p_rx->fcs_error_pkts          );
    PLUS_MAC_STATS(p_sum_rx->fcs_error_bytes       , p_rx->fcs_error_bytes         );
    PLUS_MAC_STATS(p_sum_rx->alignment_error_pkts  , p_rx->alignment_error_pkts    );
    PLUS_MAC_STATS(p_sum_rx->alignment_error_bytes , p_rx->alignment_error_bytes   );
    PLUS_MAC_STATS(p_sum_rx->mac_overrun_pkts      , p_rx->mac_overrun_pkts        );
    PLUS_MAC_STATS(p_sum_rx->mac_overrun_bytes     , p_rx->mac_overrun_bytes       );
    PLUS_MAC_STATS(p_sum_rx->good_oversize_pkts    , p_rx->good_oversize_pkts      );
    PLUS_MAC_STATS(p_sum_rx->good_oversize_bytes   , p_rx->good_oversize_bytes     );
    PLUS_MAC_STATS(p_sum_rx->good_undersize_pkts   , p_rx->good_undersize_pkts     );
    PLUS_MAC_STATS(p_sum_rx->good_undersize_bytes  , p_rx->good_undersize_bytes    );
    PLUS_MAC_STATS(p_sum_rx->gmac_good_oam_pkts    , p_rx->gmac_good_oam_pkts      );
    PLUS_MAC_STATS(p_sum_rx->gmac_good_oam_bytes   , p_rx->gmac_good_oam_bytes     );
    PLUS_MAC_STATS(p_sum_rx->good_63_pkts          , p_rx->good_63_pkts            );
    PLUS_MAC_STATS(p_sum_rx->good_63_bytes         , p_rx->good_63_bytes           );
    PLUS_MAC_STATS(p_sum_rx->bad_63_pkts           , p_rx->bad_63_pkts             );
    PLUS_MAC_STATS(p_sum_rx->bad_63_bytes          , p_rx->bad_63_bytes            );
    PLUS_MAC_STATS(p_sum_rx->good_1519_pkts        , p_rx->good_1519_pkts          );
    PLUS_MAC_STATS(p_sum_rx->good_1519_bytes       , p_rx->good_1519_bytes         );
    PLUS_MAC_STATS(p_sum_rx->bad_1519_pkts         , p_rx->bad_1519_pkts           );
    PLUS_MAC_STATS(p_sum_rx->bad_1519_bytes        , p_rx->bad_1519_bytes          );
    PLUS_MAC_STATS(p_sum_rx->good_jumbo_pkts       , p_rx->good_jumbo_pkts         );
    PLUS_MAC_STATS(p_sum_rx->good_jumbo_bytes      , p_rx->good_jumbo_bytes        );
    PLUS_MAC_STATS(p_sum_rx->bad_jumbo_pkts        , p_rx->bad_jumbo_pkts          );
    PLUS_MAC_STATS(p_sum_rx->bad_jumbo_bytes       , p_rx->bad_jumbo_bytes         );
    PLUS_MAC_STATS(p_sum_rx->pkts_64               , p_rx->pkts_64                 );
    PLUS_MAC_STATS(p_sum_rx->bytes_64              , p_rx->bytes_64                );
    PLUS_MAC_STATS(p_sum_rx->pkts_65_to_127        , p_rx->pkts_65_to_127          );
    PLUS_MAC_STATS(p_sum_rx->bytes_65_to_127       , p_rx->bytes_65_to_127         );
    PLUS_MAC_STATS(p_sum_rx->pkts_128_to_255       , p_rx->pkts_128_to_255         );
    PLUS_MAC_STATS(p_sum_rx->bytes_128_to_255      , p_rx->bytes_128_to_255        );
    PLUS_MAC_STATS(p_sum_rx->pkts_256_to_511       , p_rx->pkts_256_to_511         );
    PLUS_MAC_STATS(p_sum_rx->bytes_256_to_511      , p_rx->bytes_256_to_511        );
    PLUS_MAC_STATS(p_sum_rx->pkts_512_to_1023      , p_rx->pkts_512_to_1023        );
    PLUS_MAC_STATS(p_sum_rx->bytes_512_to_1023     , p_rx->bytes_512_to_1023       );
    PLUS_MAC_STATS(p_sum_rx->pkts_1024_to_1518     , p_rx->pkts_1024_to_1518       );
    PLUS_MAC_STATS(p_sum_rx->bytes_1024_to_1518    , p_rx->bytes_1024_to_1518      );

    PLUS_MAC_STATS(p_sum_tx->good_ucast_pkts       , p_tx->good_ucast_pkts         );
    PLUS_MAC_STATS(p_sum_tx->good_ucast_bytes      , p_tx->good_ucast_bytes        );
    PLUS_MAC_STATS(p_sum_tx->good_mcast_pkts       , p_tx->good_mcast_pkts         );
    PLUS_MAC_STATS(p_sum_tx->good_mcast_bytes      , p_tx->good_mcast_bytes        );
    PLUS_MAC_STATS(p_sum_tx->good_bcast_pkts       , p_tx->good_bcast_pkts         );
    PLUS_MAC_STATS(p_sum_tx->good_bcast_bytes      , p_tx->good_bcast_bytes        );
    PLUS_MAC_STATS(p_sum_tx->good_pause_pkts       , p_tx->good_pause_pkts         );
    PLUS_MAC_STATS(p_sum_tx->good_pause_bytes      , p_tx->good_pause_bytes        );
    PLUS_MAC_STATS(p_sum_tx->good_control_pkts     , p_tx->good_control_pkts       );
    PLUS_MAC_STATS(p_sum_tx->good_control_bytes    , p_tx->good_control_bytes      );
    PLUS_MAC_STATS(p_sum_tx->good_oam_pkts         , p_tx->good_oam_pkts           );
    PLUS_MAC_STATS(p_sum_tx->good_oam_bytes        , p_tx->good_oam_bytes          );
    PLUS_MAC_STATS(p_sum_tx->pkts_63               , p_tx->pkts_63                 );
    PLUS_MAC_STATS(p_sum_tx->bytes_63              , p_tx->bytes_63                );
    PLUS_MAC_STATS(p_sum_tx->pkts_64               , p_tx->pkts_64                 );
    PLUS_MAC_STATS(p_sum_tx->bytes_64              , p_tx->bytes_64                );
    PLUS_MAC_STATS(p_sum_tx->pkts_65_to_127        , p_tx->pkts_65_to_127          );
    PLUS_MAC_STATS(p_sum_tx->bytes_65_to_127       , p_tx->bytes_65_to_127         );
    PLUS_MAC_STATS(p_sum_tx->pkts_128_to_255       , p_tx->pkts_128_to_255         );
    PLUS_MAC_STATS(p_sum_tx->bytes_128_to_255      , p_tx->bytes_128_to_255        );
    PLUS_MAC_STATS(p_sum_tx->pkts_256_to_511       , p_tx->pkts_256_to_511         );
    PLUS_MAC_STATS(p_sum_tx->bytes_256_to_511      , p_tx->bytes_256_to_511        );
    PLUS_MAC_STATS(p_sum_tx->pkts_512_to_1023      , p_tx->pkts_512_to_1023        );
    PLUS_MAC_STATS(p_sum_tx->bytes_512_to_1023     , p_tx->bytes_512_to_1023       );
    PLUS_MAC_STATS(p_sum_tx->pkts_1024_to_1518     , p_tx->pkts_1024_to_1518       );
    PLUS_MAC_STATS(p_sum_tx->bytes_1024_to_1518    , p_tx->bytes_1024_to_1518      );
    PLUS_MAC_STATS(p_sum_tx->pkts_1519             , p_tx->pkts_1519               );
    PLUS_MAC_STATS(p_sum_tx->bytes_1519            , p_tx->bytes_1519              );
    PLUS_MAC_STATS(p_sum_tx->jumbo_pkts            , p_tx->jumbo_pkts              );
    PLUS_MAC_STATS(p_sum_tx->jumbo_bytes           , p_tx->jumbo_bytes             );
    PLUS_MAC_STATS(p_sum_tx->mac_underrun_pkts     , p_tx->mac_underrun_pkts       );
    PLUS_MAC_STATS(p_sum_tx->mac_underrun_bytes    , p_tx->mac_underrun_bytes      );
    PLUS_MAC_STATS(p_sum_tx->fcs_error_pkts        , p_tx->fcs_error_pkts          );
    PLUS_MAC_STATS(p_sum_tx->fcs_error_bytes       , p_tx->fcs_error_bytes         );

#undef PLUS_MAC_STATS
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_minus_agg_port_stats
 * Purpose      : delete port stats to agg port
 * Input        : N/A     
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for agg interface
***************************************************************************************************/
static int32
_hagt_if_update_agg_port_stats(glb_if_stats_t* p_agg_stats, glb_if_stats_t* p_old_stats, bool is_add)
{
/*modify chenxw for bug23994, 2013-08-02.*/              
#define UPDATE_AGG_PORT_STATS(agg, port)                                          \
do{                                                                     \
    if(is_add)                                               \
    {                                                                   \
        agg += port;                                                       \
    }                                                                   \
    else                                                                \
    {                                                                   \
        agg -= port;                                                       \
    }                                                                   \
}while(0)

    UPDATE_AGG_PORT_STATS(p_agg_stats->good_octets_rcv        ,      p_old_stats->good_octets_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->bad_octets_rcv         ,      p_old_stats->bad_octets_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->mac_transmit_err       ,      p_old_stats->mac_transmit_err);
    UPDATE_AGG_PORT_STATS(p_agg_stats->good_pkts_rcv          ,      p_old_stats->good_pkts_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->bad_pkts_rcv           ,      p_old_stats->bad_pkts_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->uc_pkts_rcv            ,      p_old_stats->uc_pkts_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->mc_pkts_rcv            ,      p_old_stats->mc_pkts_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->brdc_pkts_rcv          ,      p_old_stats->brdc_pkts_rcv);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->oam_pkts_rcv           ,      p_old_stats->oam_pkts_rcv);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_64_octets         ,      p_old_stats->pkts_64_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_65_127_octets     ,      p_old_stats->pkts_65_127_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_128_255_octets    ,      p_old_stats->pkts_128_255_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_256_511_octets    ,      p_old_stats->pkts_256_511_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_512_1023_octets   ,      p_old_stats->pkts_512_1023_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_1024_max_octets   ,      p_old_stats->pkts_1024_max_octets);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->good_octets_send       ,      p_old_stats->good_octets_send);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->good_pkts_send         ,      p_old_stats->good_pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->excessive_collisions   ,      p_old_stats->excessive_collisions);    
    UPDATE_AGG_PORT_STATS(p_agg_stats->uc_pkts_send           ,      p_old_stats->uc_pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->mc_pkts_send           ,      p_old_stats->mc_pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->brdc_pkts_send         ,      p_old_stats->brdc_pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->oam_pkts_send          ,      p_old_stats->oam_pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->unrecog_mac_cntr_rcv   ,      p_old_stats->unrecog_mac_cntr_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->fc_send                ,      p_old_stats->fc_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->good_fc_rcv            ,      p_old_stats->good_fc_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->drop_events            ,      p_old_stats->drop_events);
    UPDATE_AGG_PORT_STATS(p_agg_stats->undersize_pkts         ,      p_old_stats->undersize_pkts);
    UPDATE_AGG_PORT_STATS(p_agg_stats->fragments_pkts         ,      p_old_stats->fragments_pkts);
    UPDATE_AGG_PORT_STATS(p_agg_stats->oversize_pkts          ,      p_old_stats->oversize_pkts);
    UPDATE_AGG_PORT_STATS(p_agg_stats->jabber_pkts            ,      p_old_stats->jabber_pkts);
    UPDATE_AGG_PORT_STATS(p_agg_stats->mac_rcv_error          ,      p_old_stats->mac_rcv_error);
    UPDATE_AGG_PORT_STATS(p_agg_stats->bad_crc                ,      p_old_stats->bad_crc);
    UPDATE_AGG_PORT_STATS(p_agg_stats->collisions             ,      p_old_stats->collisions);
    UPDATE_AGG_PORT_STATS(p_agg_stats->late_collisions        ,      p_old_stats->late_collisions);
    UPDATE_AGG_PORT_STATS(p_agg_stats->bad_fc_rcv             ,      p_old_stats->bad_fc_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->octets_rcv             ,      p_old_stats->octets_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_rcv               ,      p_old_stats->pkts_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->octets_send            ,      p_old_stats->octets_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->pkts_send              ,      p_old_stats->pkts_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->underruns              ,      p_old_stats->underruns);
    UPDATE_AGG_PORT_STATS(p_agg_stats->frame_error            ,      p_old_stats->frame_error);
    UPDATE_AGG_PORT_STATS(p_agg_stats->pause_rcv              ,      p_old_stats->pause_rcv);
    UPDATE_AGG_PORT_STATS(p_agg_stats->pause_send             ,      p_old_stats->pause_send);
    UPDATE_AGG_PORT_STATS(p_agg_stats->no_carrier             ,      p_old_stats->no_carrier);    

#undef UPDATE_AGG_PORT_STATS                                          
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_plus_mac_port_stats 
 * Purpose      : merge ctc rx&tx mac stats to glb stats
 * Input        : N/A     
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_plus_mac_port_stats(glb_if_stats_t* p_stats,
                            ctc_stats_mac_rec_t* p_mac_rx_stats,
                            ctc_stats_mac_snd_t* p_mac_tx_stats,
                            bool is_accumulate)/* is_accumulate means sdk store accumulate data */
{
#define PLUS_MAC_PORT_STATS(sum, a)                                          \
do{                                                                     \
    if(!is_accumulate)                                               \
    {                                                                   \
        sum += a;                                                       \
    }                                                                   \
    else                                                                \
    {                                                                   \
        sum = a;                                                       \
    }                                                                   \
}while(0)
    /*get mac receive ucast stats*/
    PLUS_MAC_PORT_STATS(p_stats->uc_pkts_rcv     , p_mac_rx_stats->good_ucast_pkts);
    
    PLUS_MAC_PORT_STATS(p_stats->good_pkts_rcv   , p_mac_rx_stats->good_ucast_pkts);
    p_stats->good_pkts_rcv   += p_mac_rx_stats->good_mcast_pkts;
    p_stats->good_pkts_rcv   += p_mac_rx_stats->good_bcast_pkts;
    p_stats->good_pkts_rcv   += p_mac_rx_stats->gmac_good_oam_pkts;
    
    PLUS_MAC_PORT_STATS(p_stats->good_octets_rcv , p_mac_rx_stats->good_ucast_bytes);
    p_stats->good_octets_rcv += p_mac_rx_stats->good_mcast_bytes;
    p_stats->good_octets_rcv += p_mac_rx_stats->good_bcast_bytes;
    p_stats->good_octets_rcv += p_mac_rx_stats->gmac_good_oam_bytes;
    
    /*get mac receive mcast stats*/
    PLUS_MAC_PORT_STATS(p_stats->mc_pkts_rcv     , p_mac_rx_stats->good_mcast_pkts);
    
    /*get mac receive bcast stats*/
    PLUS_MAC_PORT_STATS(p_stats->brdc_pkts_rcv   , p_mac_rx_stats->good_bcast_pkts);
    
    /*get mac receive oam stats*/
    /*Modified by yanxa, for bug 17571, 2011-12-26*/
    PLUS_MAC_PORT_STATS(p_stats->oam_pkts_rcv    , p_mac_rx_stats->gmac_good_oam_pkts);
    
    /*get mac receive pause stats*/
    PLUS_MAC_PORT_STATS(p_stats->pause_rcv , p_mac_rx_stats->good_pause_pkts);
    
    /*get port ingress stats*/
    {
        /*get mac receive 63 stats*/
        
        PLUS_MAC_PORT_STATS(p_stats->octets_rcv , p_mac_rx_stats->good_63_bytes);
        p_stats->octets_rcv += p_mac_rx_stats->bad_63_bytes;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_64;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_65_to_127;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_128_to_255;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_256_to_511;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_512_to_1023;
        p_stats->octets_rcv += p_mac_rx_stats->bytes_1024_to_1518;
        p_stats->octets_rcv += p_mac_rx_stats->good_1519_bytes;
        p_stats->octets_rcv += p_mac_rx_stats->bad_1519_bytes;
        p_stats->octets_rcv += p_mac_rx_stats->good_jumbo_bytes;
        p_stats->octets_rcv += p_mac_rx_stats->bad_jumbo_bytes;

        /*for bug 24466*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_rcv   , p_mac_rx_stats->good_63_pkts);
        p_stats->pkts_rcv   += p_mac_rx_stats->bad_63_pkts;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_64;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_65_to_127;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_128_to_255;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_256_to_511;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_512_to_1023;
        p_stats->pkts_rcv   += p_mac_rx_stats->pkts_1024_to_1518;
        p_stats->pkts_rcv   += p_mac_rx_stats->good_1519_pkts;
        p_stats->pkts_rcv   += p_mac_rx_stats->bad_1519_pkts;
        p_stats->pkts_rcv   += p_mac_rx_stats->good_jumbo_pkts;
        p_stats->pkts_rcv   += p_mac_rx_stats->bad_jumbo_pkts;

        /*get mac receive 64 stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_64_octets , p_mac_rx_stats->bytes_64);

        /*get mac receive 65-127 stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_65_127_octets , p_mac_rx_stats->bytes_65_to_127);

        /*get mac receive 128-255 stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_128_255_octets , p_mac_rx_stats->bytes_128_to_255);

        /*get mac receive 256-511 stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_256_511_octets , p_mac_rx_stats->bytes_256_to_511);

        /*get mac receive 512-1023 stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_512_1023_octets , p_mac_rx_stats->bytes_512_to_1023);

        /*get mac receive 1024-max stats*/
        PLUS_MAC_PORT_STATS(p_stats->pkts_1024_max_octets , p_mac_rx_stats->bytes_1024_to_1518);

    }
    
    /*get mac receive under size stats*/
    PLUS_MAC_PORT_STATS(p_stats->undersize_pkts , p_mac_rx_stats->good_undersize_pkts);
    
    PLUS_MAC_PORT_STATS(p_stats->bad_octets_rcv , p_mac_rx_stats->good_undersize_bytes);
    //p_stats->bad_octets_rcv += p_mac_rx_stats->good_oversize_bytes;
    p_stats->bad_octets_rcv += p_mac_rx_stats->jabber_bytes;
    p_stats->bad_octets_rcv += p_mac_rx_stats->collision_bytes;
    p_stats->bad_octets_rcv += p_mac_rx_stats->fcs_error_bytes;
    p_stats->bad_octets_rcv += p_mac_rx_stats->alignment_error_bytes;
    
    PLUS_MAC_PORT_STATS(p_stats->bad_pkts_rcv   , p_mac_rx_stats->good_undersize_pkts);
    //p_stats->bad_pkts_rcv   += p_mac_rx_stats->good_oversize_pkts;
    p_stats->bad_pkts_rcv   += p_mac_rx_stats->jabber_pkts;    
    p_stats->bad_pkts_rcv   += p_mac_rx_stats->collision_pkts;
    p_stats->bad_pkts_rcv   += p_mac_rx_stats->fcs_error_pkts;    
    p_stats->bad_pkts_rcv   += p_mac_rx_stats->alignment_error_pkts;    

    PLUS_MAC_PORT_STATS(p_stats->mac_rcv_error  , p_mac_rx_stats->good_undersize_pkts);
    p_stats->mac_rcv_error  += p_mac_rx_stats->jabber_pkts;
    p_stats->mac_rcv_error  += p_mac_rx_stats->collision_pkts;
    p_stats->mac_rcv_error  += p_mac_rx_stats->fcs_error_pkts;
    p_stats->mac_rcv_error  += p_mac_rx_stats->alignment_error_pkts;
    p_stats->mac_rcv_error += p_mac_rx_stats->mac_overrun_pkts;

    /*get mac receive over size stats*/
    PLUS_MAC_PORT_STATS(p_stats->oversize_pkts  , p_mac_rx_stats->good_oversize_pkts);
    
    /* modified by liwh for bug 19945, 2012-07-25 
       oversize packets should not be counted as error packet */
    /*p_stats->mac_rcv_error  += p_mac_rx_stats->good_oversize_pkts;*/
    
    /*get mac receive jabber stats*/
    PLUS_MAC_PORT_STATS(p_stats->jabber_pkts    , p_mac_rx_stats->jabber_pkts);
    
    /*get mac receive collision fragment stats*/
    PLUS_MAC_PORT_STATS(p_stats->fragments_pkts , p_mac_rx_stats->collision_pkts);
    
    /*get mac recieve Fcs error stats*/
    PLUS_MAC_PORT_STATS(p_stats->bad_crc        , p_mac_rx_stats->fcs_error_pkts);
    
    /*get mac recieve alignment error stats*/
    PLUS_MAC_PORT_STATS(p_stats->frame_error    , p_mac_rx_stats->alignment_error_pkts);
    
    /*get mac receive over run stats*/
    PLUS_MAC_PORT_STATS(p_stats->drop_events   , p_mac_rx_stats->mac_overrun_pkts);

    
    /*get mac send ucast stats*/
    PLUS_MAC_PORT_STATS(p_stats->uc_pkts_send     , p_mac_tx_stats->good_ucast_pkts);
    
    PLUS_MAC_PORT_STATS(p_stats->good_pkts_send   , p_mac_tx_stats->good_ucast_pkts);
    p_stats->good_pkts_send   += p_mac_tx_stats->good_mcast_pkts;
    p_stats->good_pkts_send   += p_mac_tx_stats->good_bcast_pkts;
    p_stats->good_pkts_send   += p_mac_tx_stats->good_oam_pkts;

    PLUS_MAC_PORT_STATS(p_stats->good_octets_send , p_mac_tx_stats->good_ucast_bytes);
    p_stats->good_octets_send += p_mac_tx_stats->good_mcast_bytes;
    p_stats->good_octets_send += p_mac_tx_stats->good_bcast_bytes;
    p_stats->good_octets_send += p_mac_tx_stats->good_oam_bytes;

    /*get mac send mcast stats*/
    PLUS_MAC_PORT_STATS(p_stats->mc_pkts_send     , p_mac_tx_stats->good_mcast_pkts);
    
    /*get mac send bcast stats*/
    PLUS_MAC_PORT_STATS(p_stats->brdc_pkts_send   , p_mac_tx_stats->good_bcast_pkts);

    /*get mac send oam stats*/
    /*Modified by yanxa, for bug 17571, 2011-12-26*/
    PLUS_MAC_PORT_STATS(p_stats->oam_pkts_send    , p_mac_tx_stats->good_oam_pkts);
    
    /*get mac send mac Fcs error stats*/
    PLUS_MAC_PORT_STATS(p_stats->mac_transmit_err , p_mac_tx_stats->fcs_error_pkts);
    
    /*get mac send under run stats*/
    PLUS_MAC_PORT_STATS(p_stats->underruns , p_mac_tx_stats->mac_underrun_pkts);
    
    /*get mac send pause stats*/
    PLUS_MAC_PORT_STATS(p_stats->pause_send , p_mac_tx_stats->good_pause_pkts);
    
    /*get mac send total stats*/
    {
        PLUS_MAC_PORT_STATS(p_stats->pkts_send   , p_mac_tx_stats->pkts_63);
        p_stats->pkts_send   += p_mac_tx_stats->pkts_64;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_65_to_127;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_128_to_255;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_256_to_511;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_512_to_1023;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_1024_to_1518;
        p_stats->pkts_send   += p_mac_tx_stats->pkts_1519;
        p_stats->pkts_send   += p_mac_tx_stats->jumbo_pkts;

        PLUS_MAC_PORT_STATS(p_stats->octets_send , p_mac_tx_stats->bytes_63);        
        p_stats->octets_send += p_mac_tx_stats->bytes_64;        
        p_stats->octets_send += p_mac_tx_stats->bytes_65_to_127;        
        p_stats->octets_send += p_mac_tx_stats->bytes_128_to_255;        
        p_stats->octets_send += p_mac_tx_stats->bytes_256_to_511;        
        p_stats->octets_send += p_mac_tx_stats->bytes_512_to_1023;        
        p_stats->octets_send += p_mac_tx_stats->bytes_1024_to_1518;        
        p_stats->octets_send += p_mac_tx_stats->bytes_1519;        
        p_stats->octets_send += p_mac_tx_stats->jumbo_bytes;
    }    
  

#undef PLUS_MAC_PORT_STATS
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_fetch_stats 
 * Purpose      : getch port statistics from database
 * Input        : gport: agg trunk id or phy port gport id   
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : 
***************************************************************************************************/
static glb_if_stats_t*
_hagt_if_fetch_stats(uint16 gportid)
{
    glb_if_stats_t* p_stats_db = NULL;
    hagt_if_port_stats_info_t* p_stats_info = NULL;
    uint16 tid = 0;
    uint8 gchip;
    int32 lchip;
    /* Modifed by weizj for bug 36460 */
    uint16 lport;

    /* agg port */
    if (GLB_IS_LINKAGG_PORT(gportid))
    {
        tid = GLB_TRANS_AGGPORT_TO_TID(gportid);
        p_stats_info = &g_pst_hagt_if_master->agg_port_stats_info;
        /* Modifed by weizj for bug 36460 */
        p_stats_db = p_stats_info->p_port_stats[tid - 1];
    }
    else
    {
        gchip = GLB_GET_GLOBAL_CHIPID(gportid);
        lport = GLB_GET_LOCAL_PORTID(gportid);
        lchip = hagt_chip_glb2local(gchip);
        if (GLB_INVALID_CHIPID == lchip)
        {
            HAGT_LOG_ERR("The chip %u is not local chip.", gchip);
            return NULL;
        }

        /* Modifed by weizj for bug 36460 */
        if (lport >= GLB_PORT_NUM_PER_CHIP)
        {
            HAGT_LOG_ERR("The local port %u is not phy port.", lport);
            return NULL;
        }
        p_stats_info = &g_pst_hagt_if_master->port_stats_info;
        /* Modifed by weizj for bug 36460 */
        //p_stats_db = &p_stats_info->p_port_stats[lchip * HAGT_CHIP_MAX_PHY_PORT + lport];
        p_stats_db = p_stats_info->p_port_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport];
        
    }
    
    return p_stats_db;
}

/***************************************************************************************************
 * Name         : _hagt_if_calc_speed 
 * Purpose      : calc port statistics
 * Input        : gport: agg trunk id or phy port gport id   
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : 
***************************************************************************************************/
static int32
_hagt_if_calc_speed(uint16 gportid)
{
    uint32 index;
    glb_if_stats_t* p_stats_db = NULL;
    hagt_if_port_info_t* pst_port_info = NULL;    
    int interval;
    int notify_intval;
    u_int64_t octets_rcv;
    u_int64_t pkts_rcv;
    u_int64_t octets_send;
    u_int64_t pkts_send;

    /* agg port */
    p_stats_db = _hagt_if_fetch_stats(gportid);
    pst_port_info = hagt_ifdb_port_vector_get(gportid);

    if ((NULL ==p_stats_db) || (NULL ==pst_port_info))
    {
        return HAGT_E_NONE;
    }

    if (0 == pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH])
    {
        pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] = GLB_IF_STATS_LOAD_DEF_INTVAL;
    }
    
    notify_intval = pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH];

    /* stats store more than one inteval */
    if ((_g_ctclib_last_upsec - pst_port_info->start_time) > pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH])
    {
        interval = _g_ctclib_last_upsec -  pst_port_info->store_time[pst_port_info->index];
        index = pst_port_info->index;
    }
    else
    {
        interval = notify_intval;
        index = 0;            
    }
    
    if (0 == interval)
    {
        return HAGT_E_NONE;
    }
    octets_rcv = p_stats_db->octets_rcv - pst_port_info->octets_rcv[index];
    pkts_rcv = p_stats_db->pkts_rcv - pst_port_info->pkts_rcv[index];
    octets_send = p_stats_db->octets_send - pst_port_info->octets_send[index];
    pkts_send = p_stats_db->pkts_send - pst_port_info->pkts_send[index];        

    p_stats_db->octets_rcv_rate  = (octets_rcv * 8)/interval;
    p_stats_db->pkts_rcv_rate    = pkts_rcv / interval;
    p_stats_db->octets_send_rate = (octets_send * 8)/interval;
    p_stats_db->pkts_send_rate   = pkts_send / interval;
    
    return HAGT_E_NONE;
}


/***************************************************************************************************
 * Name         : hagt_if_get_phy_port_mac_stats 
 * Purpose      : get port statistics
 * Input        : pv_msg_data: gportid    
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
int32
hagt_if_get_phy_port_mac_stats(uint16 gport)
{
    glb_if_stats_t* p_stats_db = NULL;
    glb_if_stats_t* p_agg_stats_db = NULL;
    ctc_stats_mac_rec_t* p_rx_db = NULL;
    ctc_stats_mac_snd_t* p_tx_db = NULL;
    hagt_if_port_stats_info_t* p_stats_info = NULL;
    hagt_if_port_stats_info_t* p_agg_stats_info = NULL;
    ctc_mac_stats_t mac_rx_stats, mac_tx_stats;
    uint8 gchip;
    int32 lchip;
    /* Modifed by weizj for bug 36460 */
    uint16 lport;
    hagt_if_flag_t flag = 0;
    bool is_10G_port = FALSE;
    hagt_if_port_info_t* pst_port_info = NULL;
    uint64 last_bad_pkts_rcv = 0;
    uint64 last_bad_pkts_rcv_agg = 0;
    uint64 last_bad_crc = 0;
    ctc_stats_property_param_t stats_param;
    bool is_stats_accumulted;

    gchip = GLB_GET_GLOBAL_CHIPID(gport);
    lport = GLB_GET_LOCAL_PORTID(gport);
    lchip = hagt_chip_glb2local(gchip);

    if (GLB_INVALID_CHIPID == lchip)
    {
        HAGT_LOG_ERR("The chip %u is not local chip.", gchip);
        return HAGT_E_INVALID_PARAM;
    }
    /* Modifed by weizj for bug 36460 */
    if (lport >= GLB_PORT_NUM_PER_CHIP)
    {
        HAGT_LOG_ERR("The local port %u is not phy port.", lport);
        return HAGT_E_INVALID_PARAM;
    }
    
    sal_memset(&mac_rx_stats, 0, sizeof(ctc_mac_stats_t));
    sal_memset(&mac_tx_stats, 0, sizeof(ctc_mac_stats_t));
    sal_memset(&stats_param, 0, sizeof(ctc_stats_property_param_t));

    pst_port_info = (hagt_if_port_info_t*) hagt_ifdb_port_vector_get(gport);

    if (NULL == pst_port_info)
    {
        HAGT_LOG_ERR("The phy port %u is not exist.", gport);
        return HAGT_E_INVALID_PARAM;
    }
    p_stats_info = &g_pst_hagt_if_master->port_stats_info;

    /* Modifed by weizj for bug 36460 */
    /*1. get port mac stats db */
    p_stats_db = p_stats_info->p_port_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport];

    /*2. rx and tx temp db */
    p_rx_db = p_stats_info->p_mac_rx_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport];
    p_tx_db = p_stats_info->p_mac_tx_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport];

/* sdk init mac stats as clear-after-read and sdk stores mac stats by default */
/* api is not supported */
    /* get is_stats_accumulted from sdk */
    is_stats_accumulted = TRUE;

    /*3. get rx and tx from asic */    
    mac_rx_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    ctc_stats_get_mac_stats(gport, CTC_STATS_MAC_STATS_RX, &mac_rx_stats);
    mac_tx_stats.stats_mode = CTC_STATS_MODE_DETAIL;
    ctc_stats_get_mac_stats(gport, CTC_STATS_MAC_STATS_TX, &mac_tx_stats);

    HAGT_IF_ERROR_RETURN(hagt_ifdb_get_port_flag(gport, &flag));
    last_bad_pkts_rcv = p_stats_db->bad_pkts_rcv;
    last_bad_crc = p_stats_db->bad_crc;
    is_10G_port = GLB_FLAG_ISSET(flag, HAGT_IF_PORT_FLAG_IS_10G_PORT);

    /*4. delete port stats from agg port */
    if (pst_port_info->trunkid)
    {
        p_agg_stats_info = &g_pst_hagt_if_master->agg_port_stats_info;
        /* Modifed by weizj for bug 36460 */
        p_agg_stats_db = p_agg_stats_info->p_port_stats[pst_port_info->trunkid - 1];
        last_bad_pkts_rcv_agg = p_agg_stats_db->bad_pkts_rcv;
        _hagt_if_update_agg_port_stats(p_agg_stats_db, p_stats_db, FALSE);        
    }
    
   /*5. map mac stats to hagt tmp db*/
    _hagt_if_plus_mac_rx_tx_stats(is_10G_port, p_rx_db, p_tx_db, 
        &mac_rx_stats.u.stats_detail.stats.rx_stats, &mac_tx_stats.u.stats_detail.stats.tx_stats, is_stats_accumulted);

    /*6. store accumulte mac stats to hagt db*/
    _hagt_if_plus_mac_port_stats(p_stats_db, p_rx_db, p_tx_db, is_stats_accumulted);

    /*7. increase bad pkt */
    if (p_stats_db->bad_pkts_rcv > last_bad_pkts_rcv)
    {
        p_stats_db->bad_pkts_rcv_sec++;
    }

    /*fix bug 17591 by zhangjx 2012-06-29*/
    if (p_stats_db->bad_crc > last_bad_crc)
    {
        hagt_if_tx_err_crc_log(gport, p_stats_db->bad_crc - last_bad_crc, p_stats_db->bad_crc);
    }
    else if (p_stats_db->bad_crc < last_bad_crc)
    {
        hagt_if_tx_err_crc_log(gport, p_stats_db->bad_crc - last_bad_crc, p_stats_db->bad_crc - last_bad_crc);
    }
    
    /*7. this port belongs to agg, add agg stats */
    if (pst_port_info->trunkid)
    {        
        _hagt_if_update_agg_port_stats(p_agg_stats_db, p_stats_db, TRUE);                
        if (p_agg_stats_db->bad_pkts_rcv > last_bad_pkts_rcv_agg)
        {
            p_agg_stats_db->bad_pkts_rcv_sec++;
        }
        
    }
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_port_stats_traverse_item 
 * Purpose      : Get port stats per item
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
_hagt_if_agg_member_mac_stats_get (void* pv_array_data, void* pv_user_data)
{
    hagt_if_port_info_t* pst_port_info = NULL;
    uint16 tid = 0;
    
    if ((NULL == (pst_port_info = (hagt_if_port_info_t*)pv_array_data)) 
                    || (NULL == pv_user_data))
    {
        return HAGT_E_NONE; /*the node may be null by vector traverse*/
    }

    pst_port_info = (hagt_if_port_info_t*)pv_array_data;
    tid = *((uint16 *)pv_user_data);
    if (pst_port_info->trunkid != tid)
    {
        return HAGT_E_NONE;
    }
    /* get port mac stats from sdk */
    hagt_if_get_phy_port_mac_stats(pst_port_info->gportid);

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_get_agg_stats 
 * Purpose      : get agg port statistics
 * Input        : tid: agg trunk id    
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for agg interface
***************************************************************************************************/
static int32
_hagt_if_get_agg_stats(uint16 tid)
{
    uint8 index;
    uint8  local_chip_num = 1;
    uint16 agg_gport;
    
    HAGT_IF_ERROR_RETURN(ctc_get_local_chip_num(&local_chip_num));

    for (index = 0; index < local_chip_num; index++)
    {
        ctclib_vector_traverse (g_pst_hagt_if_master->pst_port_info[index], _hagt_if_agg_member_mac_stats_get, &tid);   
    }
    agg_gport = GLB_TRANS_TID_TO_AGGPORT(tid);
    _hagt_if_calc_speed(agg_gport);
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_get_count 
 * Purpose      : get port statistics
 * Input        : pv_msg_data: gportid    
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_get_count(HalMsgStatsKey_t* key, glb_stats_result_t* p_rslt)
{
    glb_if_stats_t* p_stats_db = NULL;
    uint16 gport;
    uint16 tid = 0;
    
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IF_PTR_VALID_CHECK(key);
    HAGT_IF_PTR_VALID_CHECK(p_rslt);
    HAGT_IF_PTR_VALID_CHECK(g_pst_hagt_if_master);
    gport = key->portKey.gport;
    HAGT_L2IF_DEBUG("  gport = %d.", gport);

    sal_memset(&p_rslt->port, 0, sizeof(glb_if_stats_t));
    HAGT_IF_STATS_LOCK;
    
    /* get agg port stats */
    if (GLB_IS_LINKAGG_PORT(gport))
    {
        tid = GLB_TRANS_AGGPORT_TO_TID(gport);

        /* get agg port and member port stats */
        HAGT_IF_STATS_ERROR_RETURN_WITH_UNLOCK(_hagt_if_get_agg_stats(tid));

        /* fetch agg port stats */
        p_stats_db = _hagt_if_fetch_stats(gport);
    }
    /* get phy port stats */
    else 
    {
        if (!HAGT_GPORT_IS_LOCAL(gport))
        {
            HAGT_IF_STATS_UNLOCK;
            return HAGT_E_NONE;
        }
        
        /* read phy port stats */
        HAGT_IF_STATS_ERROR_RETURN_WITH_UNLOCK(hagt_if_get_phy_port_mac_stats(gport));
        HAGT_IF_STATS_ERROR_RETURN_WITH_UNLOCK(_hagt_if_calc_speed(gport));
        
        /* fetch phy port stats */
        p_stats_db = _hagt_if_fetch_stats(gport);
        
    }
    
    if (p_stats_db)
    {
        sal_memcpy(&p_rslt->port, p_stats_db, sizeof(glb_if_stats_t));
    }
    HAGT_IF_STATS_UNLOCK;
    
    return HAGT_E_NONE;
}

/*******************************************************************************
 * Name         : _hagt_if_clear_count(void *arg)
 * Purpose      : The function clear port stats data
 * Input        : pv_msg_data : gportid
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : N/A
 ******************************************************************************/
static int32
_hagt_if_clear_count(HalMsgStatsKey_t* key)
{
    uint8 gchip;
    int32 lchip;
    /* Modifed by weizj for bug 36460 */
    uint16 lport;
    uint16 gport;
    hagt_if_port_stats_info_t* p_stats_info = NULL;
    /* modified by cuixl for bug 17874, 2012-02-23 */
    uint16 tid = 0;
    glb_if_stats_t* p_stats_db = NULL;
    hagt_if_port_info_t* pst_port_info = NULL;
    
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IF_PTR_VALID_CHECK(key);
    HAGT_IF_PTR_VALID_CHECK(g_pst_hagt_if_master);
    gport = key->portKey.gport;
    HAGT_L2IF_DEBUG("  gportid = %d.", gport);

    HAGT_IF_STATS_LOCK;
    
    if (GLB_IS_LINKAGG_PORT(gport))
    {
        tid = GLB_TRANS_AGGPORT_TO_TID(gport);
        _hagt_if_get_agg_stats(tid);    /* read all agg memeber stats to agg, then clear agg stats */
        p_stats_info = &g_pst_hagt_if_master->agg_port_stats_info;
        /* Modifed by weizj for bug 36460 */
        p_stats_db = p_stats_info->p_port_stats[tid - 1];

        if (p_stats_db)
        {
            sal_memset(p_stats_db, 0, sizeof(glb_if_stats_t));
        }
        
        pst_port_info = hagt_ifdb_port_vector_get(gport);

        /* clear strored stats */
        if (pst_port_info)
        {
            sal_memset (pst_port_info->octets_rcv, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->pkts_rcv, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->octets_send, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->pkts_send, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            pst_port_info->index = 0;
            pst_port_info->start_time = _g_ctclib_last_upsec;
        }
    }

    else 
    {
        gchip = GLB_GET_GLOBAL_CHIPID(gport);
        lport = GLB_GET_LOCAL_PORTID(gport);
        lchip = hagt_chip_glb2local(gchip);

        if (GLB_INVALID_CHIPID == lchip)
        {
            HAGT_IF_STATS_UNLOCK;
            HAGT_LOG_ERR("The chip %u is not local chip.", gchip);
            return HAGT_E_INVALID_PARAM;
        }
        /* Modifed by weizj for bug 36460 */
        if (lport >= GLB_PORT_NUM_PER_CHIP)
        {
            HAGT_IF_STATS_UNLOCK;
            HAGT_LOG_ERR("The local port %u is not phy port.", lport);
            return HAGT_E_INVALID_PARAM;
        }
        hagt_if_get_phy_port_mac_stats(gport);
        p_stats_info = &g_pst_hagt_if_master->port_stats_info;
        /* Modifed by weizj for bug 36460 */
        p_stats_db = p_stats_info->p_port_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport];                 
        sal_memset(p_stats_info->p_port_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport], 0, sizeof(glb_if_stats_t));
        sal_memset(p_stats_info->p_mac_rx_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport], 0, sizeof(ctc_stats_mac_rec_t));
        sal_memset(p_stats_info->p_mac_tx_stats[lchip * GLB_PORT_NUM_PER_CHIP + lport], 0, sizeof(ctc_stats_mac_snd_t));

        ctc_stats_clear_mac_stats(gport, CTC_STATS_MAC_STATS_RX);
        ctc_stats_clear_mac_stats(gport, CTC_STATS_MAC_STATS_TX);

        pst_port_info = hagt_ifdb_port_vector_get(gport);
        /* clear strored stats */
        if (pst_port_info)
        {          
            sal_memset (pst_port_info->octets_rcv, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->pkts_rcv, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->octets_send, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            sal_memset (pst_port_info->pkts_send, 0, sizeof(u_int64_t) * HAGT_IF_STATS_STORE_CNT);
            pst_port_info->index = 0;
            pst_port_info->start_time = _g_ctclib_last_upsec;
        }        
    }
    HAGT_IF_STATS_UNLOCK;
    return HAGT_E_NONE;
}

static int32
_hagt_if_stats_map_queue_stats_result(ctc_qos_queue_stats_t* p_queue_stats, HalMsgStatsResult_t* resp, uint8 tmp_stats[][8])
{
    HAGT_IF_PTR_VALID_CHECK(p_queue_stats);
    HAGT_IF_PTR_VALID_CHECK(resp);
    resp->present = HalMsgStatsResult_PR_queueRslt;


    if(p_queue_stats->queue.queue_id == 0)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts0, tmp_stats[0]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes0, tmp_stats[1]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts0, tmp_stats[2]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes0, tmp_stats[3]);
    }

    if(p_queue_stats->queue.queue_id == 1)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts1, tmp_stats[4]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes1, tmp_stats[5]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts1, tmp_stats[6]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes1, tmp_stats[7]);
    }

    if(p_queue_stats->queue.queue_id == 2)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts2, tmp_stats[8]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes2, tmp_stats[9]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts2, tmp_stats[10]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes2, tmp_stats[11]);
    }
    if(p_queue_stats->queue.queue_id == 3)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts3, tmp_stats[12]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes3, tmp_stats[13]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts3, tmp_stats[14]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes3, tmp_stats[15]);
    }
    if(p_queue_stats->queue.queue_id == 4)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts4, tmp_stats[16]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes4, tmp_stats[17]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts4, tmp_stats[18]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes4, tmp_stats[19]);
    }
    if(p_queue_stats->queue.queue_id == 5)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts5, tmp_stats[20]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes5, tmp_stats[21]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts5, tmp_stats[22]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes5, tmp_stats[23]);
    }
    if(p_queue_stats->queue.queue_id == 6)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts6, tmp_stats[24]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes6, tmp_stats[25]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts6, tmp_stats[26]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes6, tmp_stats[27]);
    }
    if(p_queue_stats->queue.queue_id == 7)
    {
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_packets   , resp->queueRslt.deqPkts7, tmp_stats[28]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.deq_bytes   , resp->queueRslt.deqBytes7, tmp_stats[29]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_packets   , resp->queueRslt.dropPkts7, tmp_stats[30]);
        HAGT_IF_UINT64_TO_OCTET_STR(p_queue_stats->stats.drop_bytes   , resp->queueRslt.dropBytes7, tmp_stats[31]);
    }
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_port_stats_traverse_item 
 * Purpose      : Get port stats per item
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : polling phy port mac stats
***************************************************************************************************/
static int32
_hagt_phy_port_mac_stats_traverse_item (void* pv_array_data, void* pv_user_data)
{
    /*modified by jcao for bug 20022, for demoOS image dont' need this 2012-08-03*/
#ifdef BRINGUP_IMAGE    
    return 0;
#endif    
    hagt_if_port_info_t* pst_port_info = NULL;
    glb_if_stats_t* p_stats_db = NULL;
    uint16 gport;
    bool need_store = FALSE;
    bool need_notify = FALSE;
    Hagt2SrvMsg_t hagt_msg;
    uint32 agent_id = 0;

    if (NULL == pv_array_data)
    {
        return HAGT_E_NONE; /*the node may be null by vector traverse*/
    }
    
    pst_port_info = (hagt_if_port_info_t*)pv_array_data;
    gport = pst_port_info->gportid;

    /* check if need read mac stats */
    if (0 == pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH])
    {
        pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] = GLB_IF_STATS_LOAD_DEF_INTVAL;
    }
    
    need_store = (0 == g_pst_hagt_if_master->p_stats_poll_cnt % (pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] / (HAGT_IF_STATS_STORE_CNT - 1)));    
    
    if (pst_port_info->notify_intval[GLB_IF_STATS_REQ_RMON])
    {
        need_notify = (0 == g_pst_hagt_if_master->p_stats_poll_cnt % pst_port_info->notify_intval[GLB_IF_STATS_REQ_RMON]);
    }
    if (pst_port_info->notify_intval[GLB_IF_STATS_REQ_SFLOW])
    {
        need_notify = need_notify ? TRUE : 
                    (0 == g_pst_hagt_if_master->p_stats_poll_cnt % pst_port_info->notify_intval[GLB_IF_STATS_REQ_SFLOW]);
    }
    if (pst_port_info->notify_intval[GLB_IF_STATS_REQ_EFM])
    {
        need_notify = need_notify ? TRUE : 
                    (0 == g_pst_hagt_if_master->p_stats_poll_cnt % pst_port_info->notify_intval[GLB_IF_STATS_REQ_EFM]);
    }

    /* On OSP, always notify interrface stats to switchd when timer kick in,
       application should use the mechansim cdb provides to subscribe if stats 
       and get notified, and should no use GLB_IF_STATS_REQ_* anymore. */
    need_notify = need_notify ? TRUE : 
                (0 == g_pst_hagt_if_master->p_stats_poll_cnt % 5);
    
    if (!need_store && !need_notify)
    {
        return HAGT_E_NONE;
    }
    /* get port mac stats from sdk */
    HAGT_IF_STATS_LOCK;
    hagt_if_get_phy_port_mac_stats(gport);
    HAGT_IF_STATS_UNLOCK;
    /* fetch phy port stats */
    p_stats_db = _hagt_if_fetch_stats(gport);
    if (NULL == p_stats_db)
    {
        return HAGT_E_NONE;
    }
    /* store mac for speed. */
    if (need_store)
    {
        /* store phy port mac stats */        
        pst_port_info->octets_rcv[pst_port_info->index] = p_stats_db->octets_rcv;
        pst_port_info->pkts_rcv[pst_port_info->index] = p_stats_db->pkts_rcv;
        pst_port_info->octets_send[pst_port_info->index] = p_stats_db->octets_send;
        pst_port_info->pkts_send[pst_port_info->index] = p_stats_db->pkts_send;
        pst_port_info->store_time[pst_port_info->index] = _g_ctclib_last_upsec;
        pst_port_info->index = (1+pst_port_info->index) % HAGT_IF_STATS_STORE_CNT;
    }    

    /* we only need send phy port stats */
    if (need_notify)
    {
        uint8 tmp_stats[HAGT_IF_STATS_MAX_ITEM][8];
        agent_id = 0;//TODO: get agent_id
        sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
        hagt_msg.agentId = agent_id;
        hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = GLB_STATS_TYPE_INTF;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.portRslt.gportid = gport;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.notify = TRUE;
        _hagt_if_stats_map_stats_result(p_stats_db, &hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt, tmp_stats);
        HAGT2SRV_MSG_SEND(&hagt_msg);     


        uint16 queue_id = 0;
        ctc_qos_queue_stats_t queue_stats;
        uint8 tmp_stats2[HAGT_IF_STATS_MAX_ITEM][8];
        agent_id = 0;//TODO: get agent_id
        sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
        for(queue_id = 0; queue_id < 8; queue_id ++)
        {
            sal_memset(&queue_stats,0x0, sizeof(ctc_qos_queue_stats_t));
            queue_stats.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
            queue_stats.queue.gport = gport;
            queue_stats.queue.queue_id = queue_id;

            ctc_qos_query_queue_stats(&queue_stats);
           _hagt_if_stats_map_queue_stats_result(&queue_stats, &hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt, tmp_stats2);
              
        }
        hagt_msg.agentId = agent_id;
        hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgStatsResponse;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.type = GLB_STATS_TYPE_PORT_QUEUE;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.rslt.queueRslt.gportId = gport;
        hagt_msg.opHagt2Srv.hagt2SrvMsgStatsResponse.notify = TRUE;
        HAGT2SRV_MSG_SEND(&hagt_msg);  
    }
     
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_agg_port_mac_stats_traverse_item 
 * Purpose      : Get agg port stats per item
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : store agg port mac stats
***************************************************************************************************/
static int32
_hagt_agg_port_mac_stats_traverse_item (void* pv_array_data, void* pv_user_data)
{
    hagt_if_port_info_t* pst_port_info = NULL;
    glb_if_stats_t* p_stats_db = NULL;
    bool need_store = FALSE;
    uint16 gport;
    
    if (NULL == pv_array_data)
    {
        return HAGT_E_NONE; /*the node may be null by vector traverse*/
    }
    
    pst_port_info = (hagt_if_port_info_t*)pv_array_data;
    gport = GLB_TRANS_TID_TO_AGGPORT(pst_port_info->trunkid);
    /* check if need read mac stats */
    if (0 == pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH])
    {
        pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] = GLB_IF_STATS_LOAD_DEF_INTVAL;
    }
    
    need_store = (0 == g_pst_hagt_if_master->p_stats_poll_cnt % (pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] / (HAGT_IF_STATS_STORE_CNT - 1)));    
        
    if (!need_store)
    {
        return HAGT_E_NONE;
    }
    
    /* store agg for speed. */
    p_stats_db = _hagt_if_fetch_stats(gport);
    if(p_stats_db)
    {
        pst_port_info->octets_rcv[pst_port_info->index] = p_stats_db->octets_rcv;
        pst_port_info->pkts_rcv[pst_port_info->index] = p_stats_db->pkts_rcv;
        pst_port_info->octets_send[pst_port_info->index] = p_stats_db->octets_send;
        pst_port_info->pkts_send[pst_port_info->index] = p_stats_db->pkts_send;
        pst_port_info->store_time[pst_port_info->index] = _g_ctclib_last_upsec;
        pst_port_info->index = (1+pst_port_info->index) % HAGT_IF_STATS_STORE_CNT;
    }        
     
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_port_mac_stats_poll_handle 
 * Purpose      : get phy port stats thread
 * Input        : user_param: thread param
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static void
_hagt_port_mac_stats_poll_handle(void* user_param)
{
    uint32 index;
    uint8  local_chip_num = 1;
    
    if (ctc_get_local_chip_num(&local_chip_num) < 0 )
    {
        // TODO: should release thread resource?
        return;
    }

    while ( 1 )
    {
        //HAGT_IF_STATS_LOCK;
        for (index = 0; index < local_chip_num; index++)
        {
            ctclib_vector_traverse (g_pst_hagt_if_master->pst_port_info[index], _hagt_phy_port_mac_stats_traverse_item, NULL);   
        }
        
        /* store agg port stats */
        ctclib_vector_traverse (g_pst_hagt_if_master->pst_agg_port_info, _hagt_agg_port_mac_stats_traverse_item, NULL);   
        //HAGT_IF_STATS_UNLOCK;
        
        /* increase global count per second */
        g_pst_hagt_if_master->p_stats_poll_cnt++;
        
        /* polling all phy port stats per second */
        sleep(HAGT_IF_STATS_POLLING_INTVAL);
    }
    
    return;
}

/***************************************************************************************************
 * Name         : _hagt_if_stats_init 
 * Purpose      : intit port stats
 * Input        : gportid: global portid
                  flag_bmp: port flags
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_stats_init(void)
{
    uint16 i = 0;
    hagt_stast_pm_data_t pm_data;
    //hagt_if_port_stats_info_t* p_stats_info = NULL;
    hagt_if_port_stats_info_t* p_agg_stats_info = NULL;

    /* Modifed by weizj for bug 36460 */
    /* init agg port structure; no need to malloc for p_mac_rx_stats & p_mac_rx_stats, we don't use it on agg */
    /*p_stats_info = &g_pst_hagt_if_master->port_stats_info;
    p_stats_info->p_port_stats = 
        HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(glb_if_stats_t) * CTC_MAX_LOCAL_CHIP_NUM * HAGT_CHIP_MAX_PHY_PORT);
    if (NULL == p_stats_info->p_port_stats)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if module.");
        goto err_handle;
    }

    p_stats_info->p_mac_rx_stats = 
        HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(ctc_stats_mac_rec_t) * CTC_MAX_LOCAL_CHIP_NUM * HAGT_CHIP_MAX_PHY_PORT);
    if (NULL == p_stats_info->p_port_stats)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if module.");
        goto err_handle;
    }
    
    p_stats_info->p_mac_tx_stats = 
        HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(ctc_stats_mac_snd_t) * CTC_MAX_LOCAL_CHIP_NUM * HAGT_CHIP_MAX_PHY_PORT);
    if (NULL == p_stats_info->p_port_stats)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if module.");
        goto err_handle;
    }*/

    /* modified by cuixl for bug 17874 , 2012-02-20 */
    /* init agg port structure*/
    p_agg_stats_info = &g_pst_hagt_if_master->agg_port_stats_info;
    /*p_agg_stats_info->p_port_stats = 
        HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(glb_if_stats_t) * GLB_LINKAGG_MAX_GROUP);
    if (NULL == p_agg_stats_info->p_port_stats)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if module.");
        goto err_handle;
    }*/
    for (i = 0; i <= GLB_LINKAGG_MAX_GROUP; i++)
    {
        p_agg_stats_info->p_port_stats[i] = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(glb_if_stats_t));
    }
    /* no need to malloc for p_mac_rx_stats & p_mac_rx_stats, we don't use it on agg */

    if (sal_mutex_create(&g_pst_hagt_if_master->p_port_stats_mutex))
    {
        HAGT_IF_LOG_ERR("Create if stats mutex fail.");
        goto err_handle;
    }

    /* create phy port stats polling thread */
    if (0 != sal_task_create(&g_pst_hagt_if_master->p_poll_port_stats_thread,
                             "port_mac_stats",
                             256*1024, SAL_TASK_PRIO_DEF+10, _hagt_port_mac_stats_poll_handle, NULL))
    {
        HAGT_LOG_CRIT("Create read port mac stats thread fail.");
        goto err_handle;
    }
    
    pm_data.get_stats_cb   = _hagt_if_get_count;
    pm_data.clear_stats_cb = _hagt_if_clear_count;
     
    hagt_stats_regrister_pm_data(GLB_STATS_TYPE_INTF, &pm_data);
    // TODO: comment for by cuixl for complie
    //hagt_oam_efm_regrister_pm_data(&_hagt_if_get_count);

/* modified by cuixl for bug mac stats, 2013-03-30 */
/* init stats mac stats as clear-after-read for compatible legency system*/
// TODO: this may be optimized later, since GB supoort clear-after-read
/* sdk init gmac and sgmac stats as clear-after-read */
#if 0
/*modification start*/
    ctc_stats_property_param_t stats_param = { 0 };
    ctc_stats_property_t stats_prop;

    stats_param.prop_type = CTC_STATS_PROPERTY_CLEAR_AFTER_READ;
    stats_prop.data.enable = TRUE;
/* 1G port*/
    stats_param.stats_type = CTC_STATS_TYPE_GMAC;   
    ctc_stats_set_global_cfg(stats_param, stats_prop);
    // TODO: because board initializtion is not correct, add this code temprotory
    ctc_stats_set_clear_after_read(CTC_STATS_TYPE_GMAC, TRUE);
    ctc_stats_set_clear_after_read(CTC_STATS_TYPE_SGMAC, TRUE);
// TODO: 10G port uses SGMAC???
#if 1
/* 10G port*/
    stats_param.stats_type = CTC_STATS_TYPE_SGMAC;   
    ctc_stats_set_global_cfg(stats_param, stats_prop);    
#endif    
#endif
/*modification end*/

    return HAGT_E_NONE;

err_handle:
    /* Modifed by weizj for bug 36460 */
    /*if (p_stats_info->p_mac_rx_stats)
    {
        HAGT_FREE(CTCLIB_MEM_INTF_MODULE, p_stats_info->p_mac_rx_stats);
    }
    if (p_stats_info->p_mac_tx_stats)
    {
        HAGT_FREE(CTCLIB_MEM_INTF_MODULE, p_stats_info->p_mac_tx_stats);
    }
    if (p_agg_stats_info->p_port_stats)
    {
        HAGT_FREE(CTCLIB_MEM_INTF_MODULE, p_agg_stats_info->p_port_stats);
    }*/
    if (g_pst_hagt_if_master->p_port_stats_mutex)
    {
        sal_mutex_destroy(g_pst_hagt_if_master->p_port_stats_mutex);
        g_pst_hagt_if_master->p_port_stats_mutex = NULL;
    }

    return HAGT_E_NONE;    
}


/***************************************************************************************************
 * Name         : _hagt_if_set_tpid_value_act 
 * Purpose      : set tpid value 
 * Input        : pv_msg_data        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_tpid_value_act (uint16 type, uint16 tpid)
{
    if (GLB_TPID_TYPE_1 == type)
    {
        /* nsm tpid type 1 response to ctc svlan tpid 2 */
        HAGT_IF_ERROR_RETURN(ctc_parser_set_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_2, tpid));
    }
    else
    {
        /* nsm tpid type 2 response to ctc svlan tpid 1 */
        HAGT_IF_ERROR_RETURN(ctc_parser_set_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_1, tpid));
    }
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_if_set_tpid_value 
 * Purpose      : set tpid value 
 * Input        : pv_msg_data        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_tpid_value (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 tpid;
    uint16 type;
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    tpid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetTpidValue.value;
    type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetTpidValue.type;

    return _hagt_if_set_tpid_value_act(type, tpid);
}

/***************************************************************************************************
 * Name         : _hagt_if_set_port_tpid_type
 * Purpose      : set port tpid type
 * Input        : pv_msg_data: memberportid, tpid type      
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_port_tpid_type (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint16 type;
    uint32 ports_num;/*only when route enable, this variable is needed*/   
    uint32 index; /*for list loop the port, must be physical interface*/

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetTpidType.type;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetUntagPvid.port.int32.list.count;
    
    /*3. set to SDK*/
    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetTpidType.port.int32.list.array[index]);

         /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }
        
        if (GLB_TPID_TYPE_1 == type)
        {    
            /* nsm tpid type 1 response to ctc svlan tpid 2 */
            HAGT_IF_ERROR_RETURN(ctc_port_set_stag_tpid_index(gportid, CTC_BOTH_DIRECTION, GLB_TPID_INDEX_2));
        }
        else
        {
            /* nsm tpid type 2 response to ctc svlan tpid 1 */
            HAGT_IF_ERROR_RETURN(ctc_port_set_stag_tpid_index(gportid, CTC_BOTH_DIRECTION, GLB_TPID_INDEX_1));        
        }
    }
    
    return HAGT_E_NONE;
}


#if 0
/* Added by yanxa, for bug 16102, 2011-08-31 */
/***************************************************************************************************
 * Name         : _hagt_if_set_db_media_type
 * Purpose      : set port media type
 * Input        : pv_msg_data: gport, media type      
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_db_media_type (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint16 gportid; 
    uint32 media_type;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);

    /*2. get msg information*/
    gportid    = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetHagtDbMediaType.gport;
    media_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIFSetHagtDbMediaType.mediaType;

    /* Only care whether the port is working on 10G in hagt layer */
    if ((media_type>>30) & 0x1) /*This port is working on 10G */
    {
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_IS_10G_PORT, TRUE));
    }

    return HAGT_E_NONE;
}
    
#endif
/* support port-isolate*/
/***************************************************************************************************
* Name         : _hagt_if_set_isolate_group  
* Purpose      : This function is used to set port - isolate gourp to SDK  
* Input        : Hsrv2AgtMsg_t
* pv_msg_data 
* Output       : N / A               
* Return       : HAGT_E_NONE * Note         : N / A
***************************************************************************************************/
static int32
_hagt_if_set_isolate_group (Hsrv2AgtMsg_t* pv_msg_data)
{
    HalMsgCOMMIfSetPortIsolateGroupReq_t* pst_isolate_group = NULL;    
    uint32 i = 0;    
    uint16 us_gport = 0;    
    ctc_port_restriction_t port_restriction;
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);    
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*recieve the message*/ 
    pst_isolate_group = &pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortIsolateGroup;   

    /*list loop*/  
    for (i = 0; i < pst_isolate_group->isolatePortList.list.count; i++)    
    {
        us_gport = *(int32*)pst_isolate_group->isolatePortList.list.array[i];

        if(!HAGT_GPORT_IS_LOCAL(us_gport))
        {
            continue;
        }

        /*set to SDK*/     
        HAGT_L2IF_DEBUG("[param2sdk] us_gport = %d, group_id = %ld",       
                        us_gport, pst_isolate_group->groupID);
        sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));
        port_restriction.dir = CTC_BOTH_DIRECTION;
        port_restriction.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
        /*modify by chenxw for GG, 2014-03-27.*/
//        port_restriction.isolate_group = pst_isolate_group->groupID;
        port_restriction.isolated_id = pst_isolate_group->groupID;
        port_restriction.type = CTC_PORT_ISOLATION_ALL;
        HAGT_IF_ERROR_RETURN(ctc_port_set_restriction(us_gport, &port_restriction));
    }
    return HAGT_E_NONE;
}

/*************************************************************************************************** 
* Name         : _hagt_if_set_isolate_mode  
* Purpose      : This function is used to set port - isolate mode to SDK  
* Input        : Hsrv2AgtMsg_t* pv_msg_data 
* Output       : N / A               
* Return       : HAGT_E_NONE * Note         : N / A
***************************************************************************************************/
static int32
_hagt_if_set_isolate_mode (Hsrv2AgtMsg_t* pv_msg_data)
{
    HalMsgCOMMIfSetPortIsolateModeReq_t* pst_isolate_mode = NULL;    
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);    
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);          

    /*recieve the message*/   
    pst_isolate_mode = &pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPortIsolateMode;      

    /*set to SDK*/  
    if (GLB_PORT_ISOLATE_ALL == pst_isolate_mode->mode)    
    {
        HAGT_L2IF_DEBUG("[param2sdk] mode = %s", "ALL");   
        HAGT_IF_ERROR_RETURN(ctc_port_isolation_set_route_obey_isolated_en(TRUE));    
    }
    else if(GLB_PORT_ISOLATE_L2 == pst_isolate_mode->mode)    
    {
        HAGT_L2IF_DEBUG("[param2sdk] mode = %s", "L2");      
        HAGT_IF_ERROR_RETURN(ctc_port_isolation_set_route_obey_isolated_en(FALSE));    
    }
    
    return HAGT_E_NONE;
}

/* modified by huangxt for bug 26924, 2014-01-20. support pvlan */
/*************************************************************************************************** 
* Name         : _hagt_if_set_pvlan  
* Purpose      : This function is used to set private-vlan to SDK  
* Input        : Hsrv2AgtMsg_t* pv_msg_data 
* Output       : N / A               
* Return       : HAGT_E_NONE 
* Note         : N / A
***************************************************************************************************/
static int32
_hagt_if_set_pvlan (Hsrv2AgtMsg_t* pv_msg_data)
{
    HalMsgCOMMIfSetPVlanReq_t* pst_pvlan = NULL;  
    
    uint32 i = 0;    
    uint16 us_gport = 0;    
    ctc_port_restriction_t port_restriction;
    
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);    
    HAGT_L2IF_DEBUG( "Enter into %s.", __FUNCTION__);   

    /*recieve the message*/ 
    pst_pvlan = &pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetPVlan;   

    /*list loop*/  
    for (i = 0; i < pst_pvlan->pVlanPortList.list.count; i++)    
    {
        us_gport = *(int32*)pst_pvlan->pVlanPortList.list.array[i];

        if(!HAGT_GPORT_IS_LOCAL(us_gport))
        {
            continue;
        }

        /*set to SDK*/     
        HAGT_L2IF_DEBUG( "[param2sdk] us_gport = %d, mode = %ld, groupID = %ld",       
                        us_gport, pst_pvlan->mode, pst_pvlan->groupID);

        sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));
        if (GLB_IF_PVLAN_TYPE_NONE == pst_pvlan->mode)
        {
            port_restriction.mode = CTC_PORT_RESTRICTION_DISABLE;
        }
        else
        {
            port_restriction.mode = CTC_PORT_RESTRICTION_PVLAN;
            port_restriction.isolated_id = pst_pvlan->groupID;
            port_restriction.type = pst_pvlan->mode;
        }
        HAGT_IF_ERROR_RETURN(ctc_port_set_restriction(us_gport, &port_restriction));
    }
    return HAGT_E_NONE;
}

#if 0
/***************************************************************************************************
 * Name         : _hagt_if_set_vlan_range_en 
 * Purpose      : set tpid value 
 * Input        : pv_msg_data        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : only for physical interface
***************************************************************************************************/
static int32
_hagt_if_set_vlan_range_en (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint32 ports_num;
    uint32 index; /*for list loop the port, must be physical interface*/
    uint16 gportid;
    bool enable;
    ctc_vlan_range_info_t vrange_info;

    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);
    
    sal_memset(&vrange_info, 0, sizeof(vrange_info));
    enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetVlanRangeEn.enable;
    vrange_info.direction = CTC_INGRESS;
    vrange_info.vrange_grpid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgIfSetVlanRangeEn.group - 1;
    ports_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.gportid.int32.list.count;
    
    HAGT_L2IF_DEBUG("Set VLAN range vlan group=%d, enable=%d, ports_num=%d.", 
        vrange_info.vrange_grpid + 1, enable, ports_num);

    for (index = 0; index < ports_num; index++)
    {
        gportid = *(pv_msg_data->opHsrv2Agt.hsrv2AgtMsgCOMMIfSetVlanFilter.gportid.int32.list.array[index]);
        /*must be in local linecard*/
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }
        
        HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_range(gportid, &vrange_info, enable)); 
    }
    
    return HAGT_E_NONE;
}

int32
hagt_if_is_stacking_port (uint16 gportid)
{
    return hagt_ifdb_is_stacking_port(gportid);
}
/******************************************************************************
* initialize
******************************************************************************/
/***************************************************************************************************
 * Name         : _hagt_if_sync_mapping_port_flag 
 * Purpose      : mapping port flag, store to database and set to SDK
 * Input        : gportid: global portid
                  flag_bmp: port flags
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_sync_mapping_port_flag (uint16 gportid, uint32 flag_bmp)
{
    bool untag_def_pvid = FALSE;
    bool untag_svlan = FALSE;
    
    /*1. running flag*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_RUNNING))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_RUNNING, TRUE));
    }

    /*2. routing enable flag*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_ROUTE_EN))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_ROUTED_EN, TRUE));
    }

    /*3. agg flag*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_AGGREGATED))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_AGGREGATED, TRUE));
    }

    /*4. other flag*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_UNTAG_DEF_VID))
    {
        untag_def_pvid = TRUE;
    }
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_UNTAG_SVLAN))
    {
        untag_svlan = TRUE;
    }
    HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(gportid, untag_def_pvid, untag_svlan));    

    /*5. EFM flag*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_EFM_LINK_MONITOR_ON))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_EFM_LINK_MONITOR, TRUE));
    }

    /*Select qinq*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_SELECTIVE_QINQ))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE, TRUE));
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_SELECTIVE, TRUE));
    }

    /*VLAN Translation*/
    if (GLB_FLAG_ISSET(flag_bmp, GLB_IF_SYNC_VLAN_TRANSLATION))
    {
       HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_VLAN_TRANSLATION_ENABLE, TRUE));
    }

    return HAGT_E_NONE;

}

/***************************************************************************************************
 * Name         : _hagt_if_sync_port_info 
 * Purpose      : sync port information, include portid create, port type, default vlan and port flag      
 * Input        : pv_msg_data: message information from hsrv          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : This func is special for portno is gportid, not panel_port_no
***************************************************************************************************/
static int32
_hagt_if_sync_port_info (Hsrv2AgtMsg_t* pv_msg_data)
{
    uint32 count, index; /*for list loop the physical port*/
    //uint16 flowctrl_dir;
    uint16 gportid, slotno, portno;
    uint16 def_vlan;
    uint32 port_type; /*glb_vlan_port_type_t*/
    uint32 filter_type;
    uint32 flag;
    uint32 frame_size;
    /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
//    ctc_nh_param_brguc_sub_type_t nh_type;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__); 

    /*2. get msg information*/
    count = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.count;
    HAGT_L2IF_DEBUG("  port count=%d.", count);

    /*3. list loop the port nums*/
    for (index = 0; index < count; index++)
    {
        gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->portno;
        port_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->portType;
        filter_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->filterType;
        def_vlan = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->defVlan;
        flag = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->flagBmp;
        frame_size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->frameSize;
        /*Added by xgu for hardware learning, dump hw fdb database need ifname, 2013-9-16*/
        slotno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->slotno;
        portno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->slotportno;
        subportno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgL2ifAttriSync.l2ifList.list.array[index]->subportno;

        HAGT_L2IF_DEBUG("  gportid=%d, porttype=%d, def_vlan=%d, flow ctrl dir=%d, flag=0x%x.", 
            gportid, port_type, def_vlan, 0, flag);

        /*a. set portid*/
        if (hagt_ifdb_port_vector_get(gportid) == NULL)
        {
            HAGT_IF_ERROR_RETURN(hagt_ifdb_create_interface(gportid, slotno, portno, subportno));
        }
#if 0 /* Commented by xgu, 2012-12-17, gb sdk init this in function sys_greatbelt_port_init*/
        /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
        nh_type = CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS
                | CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU;
        HAGT_IF_ERROR_RETURN(ctc_l2_create_ucast_nh(gportid, nh_type));
#endif        
        if (!HAGT_GPORT_IS_LOCAL(gportid))
        {
            continue;
        }

        if (hagt_if_is_stacking_port(gportid))
        {
            continue;
        }

        HAGT_IF_ERROR_RETURN(ctc_port_set_bridge_en(gportid, TRUE));
        /*temp code for remote mirror*/
        HAGT_IF_ERROR_RETURN(ctc_port_set_stag_tpid_index(gportid, CTC_BOTH_DIRECTION, 2));

        /* Modified Percy Wang for bug 26645, 2014-01-02 */
        if (GLB_VLAN_PORT_TYPE_ACCESS == port_type)
        {
            HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, 
                (HAGT_IF_PORT_FLAG_ALLOW_TAGGED|HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED), FALSE));
            
            HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED, TRUE));
        }
        else if (GLB_VLAN_PORT_TYPE_TRUNK == port_type)
        {
            HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, 
                (HAGT_IF_PORT_FLAG_ALLOW_TAGGED|HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED), TRUE));   

            if (GLB_FLAG_ISSET(flag, GLB_IF_SYNC_VLAN_TRANSLATION))
            {  
                HAGT_IF_ERROR_RETURN(hagt_vlan_class_set_port_flag(gportid, port_type, TRUE));
                return HAGT_E_NONE;
            }
        }

        /*2. set default vlan*/
        if (!def_vlan)
        {
            def_vlan = 1; /*for routed port, default vlan is 0 in hsrv layer*/
        }
        HAGT_IF_ERROR_RETURN(ctc_port_set_default_vlan(gportid, def_vlan));

        /*4. set port vlan type*/
        /*Move from _hagt_if_sync_mapping_port_flag to here for _hagt_if_mapping_port_vlan_type 
         will use HAGT_IF_PORT_FLAG_AGGREGATED, modified by xgu, 2013-12-18*/
        if (GLB_FLAG_ISSET(flag, GLB_IF_SYNC_AGGREGATED))
        {
           HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(gportid, HAGT_IF_PORT_FLAG_AGGREGATED, TRUE));
        }
        HAGT_IF_ERROR_RETURN(_hagt_if_mapping_port_vlan_type(gportid, port_type));

        /*5. set port vlan filter type*/
        HAGT_IF_ERROR_RETURN(_hagt_if_mapping_vlan_filter_type(gportid, filter_type, TRUE));

        /*6. mapping port flag*/
        HAGT_IF_ERROR_RETURN(_hagt_if_sync_mapping_port_flag(gportid, flag));

        /*7. sync vlan mapping / selective qinq property*/
        if (!GLB_IS_LINKAGG_PORT(gportid))
        {
            if(GLB_FLAG_ISSET(flag, GLB_IF_SYNC_SELECTIVE_QINQ))
            {
                HAGT_IF_ERROR_RETURN(hagt_vlan_mapping_sync_selective_qinq(flag, gportid));
            }
            if(GLB_FLAG_ISSET(flag, GLB_IF_SYNC_VLAN_TRANSLATION))
            {
                HAGT_IF_ERROR_RETURN(hagt_vlan_mapping_sync_vlan_translation(gportid));
            }
        }
        HAGT_IF_ERROR_RETURN(ctc_port_set_max_frame(gportid, frame_size));
    }
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_sync_phy_info 
 * Purpose      : sync phy port information, include speed, duplex, and phy enable     
 * Input        : pv_msg_datal: msg data from hsrv           
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_sync_phy_info (Hsrv2AgtMsg_t* pv_msg_data)
{ 
    uint32 index;  
    uint8 port_num;
    uint16 slotno;
    uint8 portno;
    uint8 subportno;
    uint32 speed;
    uint32 duplex;
    uint32 send;
    uint32 recv;
    uint32 media;
    bool enable;
    uint32 poe_enable, poe_budget, poe_priority;
    lcapi_lcm_set_port_status_t port_status; /*set speed, duplex or mac enable*/
    lcapi_lcm_set_flowctrl_t port_status_flowctl; /* set flow control */
    lcapi_lcm_port_info_t lcm_port_info;
#if 0
    lcapi_lcm_set_poe_status_t port_status_poe; /* set port POE */
#endif
    uint16 gportid;
    
    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*2. get port num*/
    port_num = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.count;
      /*Fix bug 14686. jqiu 2011-06-15*/
    slotno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.slotno;
    /*3. set to SDK or LCM*/
    for (index = 0; index < port_num; index++)
    {     
        gportid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->gport;
        portno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->portno;
        subportno = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->subportno;
        speed = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->speed;
        duplex = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->duplex;
        enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->phyEnable; 
        media = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->media; 
        send = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->flowCtrl.flowCtrlSend;
        recv = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->flowCtrl.flowCtrlRecv;
        poe_enable = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->poeEnable;
        poe_budget = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->poeBudget;
        poe_priority = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgPHYIfSync.phySyncList.list.array[index]->poePriority;

        HAGT_L2IF_DEBUG("  port_no=%d, speed=%d, duplex=%d, phy_enable=%d.", 
            portno, speed, duplex, enable);

        if (hagt_if_is_stacking_port(gportid))
        {
            continue;
        }
        
        /*set to LCM*/
        port_status.slot_idx = slotno;
        port_status.port_idx = portno;
        port_status.subport_idx = subportno;
        port_status.status = speed;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_SPEED, &port_status)); 

        port_status.status = duplex;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_DUPLEX, &port_status));

        port_status.status = enable;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_PHY_ENABLE, &port_status));

        port_status.status = media;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_MEDIA, &port_status));         

        port_status_flowctl.slot_idx = slotno;
        port_status_flowctl.port_idx = portno;
        port_status_flowctl.subport_idx = subportno;
        port_status_flowctl.direction = HAGT_IF_PORT_FLOWCTRL_SEND;
        port_status_flowctl.enable = send;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_FLOWCTRL, &port_status_flowctl));

        port_status_flowctl.direction = HAGT_IF_PORT_FLOWCTRL_RECV;
        port_status_flowctl.enable = recv;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_FLOWCTRL, &port_status_flowctl));

#if 0
        port_status_poe.slot_idx = slotno;
        port_status_poe.port_idx = portno;
        port_status_poe.status = poe_enable;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_ENABLE, &port_status_poe));
        port_status_poe.status = poe_budget;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_BUDGET, &port_status_poe));
#endif
#ifdef _GLB_UML_SYSTEM_
        /*notify hsrv and kernel that interface is up/down, running flag is called by kernel to nsm*/
        lcm_port_info.slot_no = slotno;
        lcm_port_info.port_no = portno;
        lcm_port_info.sub_port_no = subportno;
        HAGT_IF_ERROR_RETURN(hagt_if_tx_link_status((void*)&lcm_port_info));
#endif /*_GLB_UML_SYSTEM_*/
#if 0
        port_status_poe.status = poe_priority;
        HAGT_IF_ERROR_RETURN(HAGT2LCM_CALLBACK_SEND(HAGT2LCM_CB_SET_POE_PRIORITY, &port_status_poe));
#endif
    }
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_sync_system_attribute 
 * Purpose      : sync system attribute, include max frame size, routemac and routemac prefix    
 * Input        : pv_msg_datal: msg data from hsrv           
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_sync_system_attribute (Hsrv2AgtMsg_t* pv_msg_data)
{ 
    uint32 index; /*index for loop routemac prefix*/
    uint32 count; /*routemac prefix count*/
    mac_addr_t routemac; /*used for routmac and routermac prefix*/
    uint16 max_frame_size;
    uint16 jumbo_frame_size;
    uint8 prefix_type; /*routemac prefix type*/
    uint32 tpid;
    ctc_oam_property_t prop;

    /*1. get values and dump*/
    HAGT_IF_PTR_VALID_CHECK(pv_msg_data);
    HAGT_L2IF_DEBUG("Enter into %s.", __FUNCTION__);   

    /*2. get max frame size and system routemac*/
    max_frame_size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.frameSize;
    jumbo_frame_size = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.jumboFrameSize;
    sal_memcpy(routemac, pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.routemac.buf, 
        pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.routemac.size);
    
    HAGT_L2IF_DEBUG("  frame size=%d, jumbo size=%d, routemac=0x%02x%02x.%02x%02x.%02x%02x.", 
        max_frame_size, jumbo_frame_size, 
        routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);  
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_router_mac(routemac));

    prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
    prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC;
    prop.u.y1731.p_value = &routemac;
    HAGT_L2IF_DEBUG("  oam bridgemac=0x%02x%02x.%02x%02x.%02x%02x.",
        routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);
    HAGT_IF_ERROR_RETURN(ctc_oam_set_property(&prop));

    /*3. set routermac prefix*/
    count = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.rmacPrefix.list.count;
    for (index = 0; index < count; index++)
    {
        prefix_type = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.rmacPrefix.list.array[index]->prefixType;
        sal_memcpy(routemac, 
            pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.rmacPrefix.list.array[index]->rmacPrefix.buf,
            pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.rmacPrefix.list.array[index]->rmacPrefix.size);
        
        HAGT_L2IF_DEBUG("  prefix type=%d, routemac=0x%02x%02x.%02x%02x.%02x%02x.", 
        prefix_type, routemac[0], routemac[1], routemac[2], routemac[3], routemac[4], routemac[5]);  
        HAGT_IF_ERROR_RETURN(ctc_l3if_set_vmac_prefix(prefix_type, routemac));
    }

    /*4. set svlan tpid */
    tpid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.svlanTpid1;
    _hagt_if_set_tpid_value_act(GLB_TPID_TYPE_1, tpid);

    tpid = pv_msg_data->opHsrv2Agt.hsrv2AgtMsgSysAttriSync.svlanTpid2;
    _hagt_if_set_tpid_value_act(GLB_TPID_TYPE_2, tpid);

    return HAGT_E_NONE;
}

/*added by ychen for bug 25183 in 2013-10-11
for supporting 88G snake line speed, enable global acl lookup only when ipv6 enabled*/
/***************************************************************************************************
 * Name         : _hagt_if_apply_acllkp_traverse_item 
 * Purpose      : enable/disable global acl lookup for every physical port   
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_apply_acllkp_traverse_item (void* pv_array_data, void* pv_user_data)
{
    hagt_if_port_info_t* pst_port_info = NULL;
    uint32* p_enable = NULL; 

    if (NULL == pv_array_data || NULL == pv_user_data)
    {
        return HAGT_E_NONE; /*the node may be null by vector traverse*/
    }

    pst_port_info = (hagt_if_port_info_t*)pv_array_data;
    p_enable = (uint32*)pv_user_data;
    if (*p_enable)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(pst_port_info->gportid, 
            CTC_PORT_DIR_PROP_ACL_EN, CTC_BOTH_DIRECTION, 0xf));
    }
    else
    {
        if (!GLB_FLAG_ISSET(pst_port_info->flag, HAGT_IF_PORT_FLAG_ACL_APPLY))
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(pst_port_info->gportid, 
                CTC_PORT_DIR_PROP_ACL_EN, CTC_BOTH_DIRECTION, 0x0));
        }
    }   

    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : hagt_if_apply_global_acl_lkp 
 * Purpose      : enable/disable global acl lookup for multicast acl ipv6 for mld packet
 * Input        : enalbe: 1, enable; 0, disable;           
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_if_apply_global_acl_lkp(uint32* enable)
{    
    uint8 index; 
   
    for (index = 0; index < CTC_MAX_LOCAL_CHIP_NUM; index++)
    {
        ctclib_vector_traverse (g_pst_hagt_if_master->pst_port_info[index], 
            _hagt_if_apply_acllkp_traverse_item, (void*)enable);  
    }

    return HAGT_E_NONE; 
}

#ifdef _GLB_ENABLE_DBGSHOW_
/***************************************************************************************************
 * Name         : _hagt_if_dump_detailed_flag 
 * Purpose      : dump interface database detailed flag info    
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_dump_detailed_flag (void)
{   
    CTCLIB_DUMP_OUT("PORT FLAG:\n");
    CTCLIB_DUMP_OUT("running=0x1,             route_en=0x2,              allow tagged=0x4\n");
    CTCLIB_DUMP_OUT("allow untagged=0x8,      mirror dest=0x10,          aggregated=0x20\n");      
    CTCLIB_DUMP_OUT("agg stand alone=0x40,    agg dis distribute=0x80,   agg dis collector=0x100\n");  
    CTCLIB_DUMP_OUT("dot1x unauthed=0x200,    dot1x dir in=0x400,        dot1x dir out=0x800\n");
    CTCLIB_DUMP_OUT("efm tx discard=0x1000,   efm rx discard=0x2000,     vpls ether mode=0x4000\n");  
    CTCLIB_DUMP_OUT("vlan mapping en=0x10000, vlan mapping qinq=0x20000, vlan mapping qinq sel=0x40000\n\n"); 

    /*dump vlan tag*/
    CTCLIB_DUMP_OUT("VLAN TAG CTRL:\n");
    CTCLIB_DUMP_OUT("0:allow all packet,    1:drop all untagged\n");
    CTCLIB_DUMP_OUT("2:drop all tagged,     3:drop all\n");
    CTCLIB_DUMP_OUT("4:drop without 2 tag,  5:drop with 2 tag\n");
    CTCLIB_DUMP_OUT("6:drop all svlan tag,  7:drop all none svlan tag\n");
    CTCLIB_DUMP_OUT("8:drop only svlan tag, 9:permit only svlan tag\n");
    CTCLIB_DUMP_OUT("10:drop all cvlan tag, 11:drop all none cvlan tag\n");
    CTCLIB_DUMP_OUT("12:drop only cvlan tag,13:permit only cvlan tag\n\n");

    /*dump dot1q type*/
    CTCLIB_DUMP_OUT("DOT1Q TYPE:\n");
    CTCLIB_DUMP_OUT("0: none dot1q type,    1: dot1q type cvlan\n");
    CTCLIB_DUMP_OUT("2: dot1q type svlan,   3: dot1q type both\n");
        
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_dump_traverse_item 
 * Purpose      : dump out interface info per item   
 * Input        : N/A          
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_if_dump_traverse_item (void* pv_array_data, void* pv_user_data)
{
    hagt_if_port_info_t* pst_port_info = NULL;

    if (NULL == pv_array_data)
    {
        return HAGT_E_NONE; /*the node may be null by vector traverse*/
    }

    pst_port_info = (hagt_if_port_info_t*)pv_array_data;
    CTCLIB_DUMP_OUT("0x%-4x      0x%-8x  %-2d          %-2d            %d\n",
        pst_port_info->gportid, pst_port_info->flag,
        pst_port_info->status.tag_ctrl, pst_port_info->status.dot1q_type, pst_port_info->status.transmit_en);

    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_if_dump_portinfo 
 * Purpose      : dump out interface info  
 * Input        : pv_arg: parameters for dump out         
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_if_dump_portinfo(void* pv_arg)
{
    hagt_if_dump_db_t* pst_dump_db = NULL;
    uint32 index;

    HAGT_IF_PTR_VALID_CHECK(pv_arg);
    pst_dump_db = (hagt_if_dump_db_t*)pv_arg;  

    /*show the flag information*/
    if (pst_dump_db->detaied)
    {         
       _hagt_if_dump_detailed_flag();       
    }
    
    CTCLIB_DUMP_OUT("-----------------------------------------------------------\n");
    CTCLIB_DUMP_OUT("gportid     flags       tag_ctrl    dot1q_type    tx_en\n");
    CTCLIB_DUMP_OUT("-----------------------------------------------------------\n");
    for (index = 0; index < CTC_MAX_LOCAL_CHIP_NUM; index++)
    {
        ctclib_vector_traverse (g_pst_hagt_if_master->pst_port_info[index], _hagt_if_dump_traverse_item, NULL);  
    }

    return HAGT_E_NONE;    
}
#endif /*_GLB_ENABLE_DBGSHOW_*/
/***************************************************************************************************
 * Name         : _hagt_if_register_hagt_message_callback 
 * Purpose      : register msg callback from hsrv to hagt      
 * Input        : N/A           
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/

#endif

static int32
_hagt_if_register_hagt_message_callback (void)
{
    /*1. register if message*/
    /*a. port create*/
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfCreatePort, _hagt_if_create_destroy_port);  
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetPortDefVid, _hagt_if_set_port_pvid);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfFlagsSet, _hagt_if_set_running_flag);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfSetPhyProp, _hagt_if_set_phy_prop);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfGetLinkStatus, _hagt_if_get_link_status);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfSetFlowctl, _hagt_if_set_flow_ctrl);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIFSetJumboFrameEn, _hagt_if_set_jumbo_frame_enable);
    
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetVlanPortType, _hagt_if_set_port_vlan_type);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetUntagPvid, _hagt_if_set_port_untag_pvid);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetVlanFilter, _hagt_if_set_vlan_filter_en);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetTpidValue, _hagt_if_set_tpid_value);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetTpidType, _hagt_if_set_port_tpid_type);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetPortRouted, _hagt_if_set_port_routed);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgSysSetRoutermac, _hagt_if_set_system_routermac);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgIfSetCrcCheckEn, _hagt_if_set_crc_chk_en);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgIfSetCrcRecalcEn, _hagt_if_set_crc_recalculation_en);
    
#if 0
    /*b. system attributes*/
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgSysSetRoutermacPrefix, _hagt_if_set_system_routermac_prefix);

    
    /*c. physical interface status*/
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfGetLinkStatus, _hagt_if_get_link_status);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfSetFlowctl, _hagt_if_set_flow_ctrl);
    /* modified by chenxw for bug 24578, 2013-08-22. */ 
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfSetFlowctlEnable, _hagt_if_set_flow_ctrl_enable);    
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgSysSetPOEProp, _hagt_if_set_poe_prop);
    /* Qicx added to support GB XFI/SGMII dynamic switch per port, 2013-09-24 */
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgSysSetSerDesMode, _hagt_if_set_serdes_mode);
    /* End of qicx added */


    /*d. interface attributes*/

    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetVlanPortType, _hagt_if_set_port_vlan_type);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetUntagPvid, _hagt_if_set_port_untag_pvid);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetVlanFilter, _hagt_if_set_vlan_filter_en);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIFSetHagtDbMediaType, _hagt_if_set_db_media_type);
    
    /*e. sync up functions*/
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgL2ifAttriSync, _hagt_if_sync_port_info);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgPHYIfSync, _hagt_if_sync_phy_info);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgSysAttriSync, _hagt_if_sync_system_attribute);

#endif
    /*f. support port-isolate*/
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetPortIsolateGroup, _hagt_if_set_isolate_group);
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetPortIsolateMode, _hagt_if_set_isolate_mode);

    /* modified by huangxt for bug 26924, 2014-01-20. support pvlan */
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIfSetPVlan, _hagt_if_set_pvlan);
#if 0
/* modified by cuixl for bug 17874, 2012-02-23 */
    /*g. set port stats notify intval */
    
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgIfSetVlanRangeEn, _hagt_if_set_vlan_range_en);
    
#endif
    hagt_message_set_msg_callback (opHsrv2Agt_PR_hsrv2AgtMsgCOMMIFSetHagtStatsIntval, _hagt_if_set_stats_intval);

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : hagt_if_init 
 * Purpose      : init hagt interface module, malloc module memory and init       
 * Input        : pst_master: hagt master           
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_if_init (hagt_master_t* pst_master)
{
    hagt_module_info_t* pst_if_module = NULL; 
    ctc_frame_size_t max_frame_index;

    /*1. get interface master, allocte memory*/
    pst_if_module = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(hagt_module_info_t));
    if (NULL == pst_if_module)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if module.");
        return HAGT_E_NO_MEMORY;
    }
    
    g_pst_hagt_if_master = HAGT_CALLOC(CTCLIB_MEM_INTF_MODULE, sizeof(hagt_if_master_t));
    if (NULL == g_pst_hagt_if_master)
    {
        HAGT_IF_LOG_ERR("malloc memory failed for hagt if master.");
        HAGT_FREE(CTCLIB_MEM_INTF_MODULE, pst_if_module);
        pst_if_module = NULL;
        return HAGT_E_NO_MEMORY;
    }

    /*2. init interface module */
    pst_if_module->module_id = HAGT_MODULE_INTERFACE;
    pst_if_module->p_data_info = g_pst_hagt_if_master;

    /*3. register message callback*/
    _hagt_if_register_hagt_message_callback ();

    /*4. register to hsrv master*/
    pst_master->past_module_info[HAGT_MODULE_INTERFACE] = pst_if_module; 

    /*5.init interface database*/
    hagt_ifdb_init(); 

    /*6.init interface statistics*/
    HAGT_IF_ERROR_RETURN(_hagt_if_stats_init());
    
    /*7.init port isolate mode*/
    HAGT_IF_ERROR_RETURN(ctc_port_isolation_set_route_obey_isolated_en(FALSE));

    /*8. init default max frame size, added by xgu for ggsystem, 2014-5-4*/
    for(max_frame_index = CTC_FRAME_SIZE_0; max_frame_index < CTC_FRAME_SIZE_MAX; max_frame_index++)
    {
        if(CTC_FRAME_SIZE_0 == max_frame_index)
        {
            HAGT_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_0));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_1));
        }
    }

    return HAGT_E_NONE;
}


