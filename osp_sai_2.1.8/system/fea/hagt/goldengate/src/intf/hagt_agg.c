/**************************************************************************
 * hagt_lacp.c   :   khal link aggregation process 
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Xianghong Gu
 * Date         :       2006-07-31
 * Reason       :       First Create.
 **************************************************************************/

/**************************************************************************
 *
 * Header Files 
 *
 **************************************************************************/
#include "hagt_inc.h"
#include "glb_if_define.h"
#include "hagt_if.h"
#include "hagt_agg.h"
#include "hagt_agg_priv.h"

#include "glb_l2_define.h"

/**************************************************************************
 *  
 * Defines and Macros
 *
 **************************************************************************/
/* modified by cuixl for bug 17874, 2012-02-24 */
extern int32 hagt_ifdb_add_agg_interface (uint16 trunkid);
extern int32 hagt_ifdb_del_agg_interface (uint16 trunkid);
extern int32 hagt_ifdb_set_port_agg_attribute (uint16 trunkid, uint16 gportid, bool enable);
static bool hagt_agg_enable_dlb_status[GLB_LINKAGG_MAX_GROUP] = {0};
static glb_agg_mode_t hagt_agg_enable_mode[GLB_LINKAGG_MAX_GROUP] = {0};
/**************************************************************************
 *  
 * Global and Declaration
 *
 **************************************************************************/


/**************************************************************************
 *  
 * Function
 *
 **************************************************************************/
 
/*********************************************************************
 * Name    : _hagt_agg_add_aggregator
 * Purpose : create agg
 * Input   : agg tid
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_add_aggregator(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 trunkid = 0;  
    ctc_linkagg_group_t linkagg_group;
    mac_addr_t mac_addr;
    ctc_port_mac_postfix_t mac_postfix;

    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    /*get info form message*/
    trunkid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggAdd.trunkid;
    
    sal_memcpy(mac_addr, pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggAdd.portmac.buf,
        pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggAdd.portmac.size);

    if(hagt_agg_enable_dlb_status[trunkid])
        linkagg_group.linkagg_mode = 1;
    else
        linkagg_group.linkagg_mode = 0;
    linkagg_group.tid = trunkid;
    HAGT_AGG_DEBUG("_hagt_agg_add_aggregator: trunkid = %d", trunkid);

    if(hagt_agg_enable_mode[trunkid] == GLB_ROUND_ROBIN_LOAD_BALANCE)
    {
        linkagg_group.linkagg_mode = CTC_LINKAGG_MODE_RR;
    }
    
    /*create agg interface in SDK*/
    HAGT_IF_ERROR_RETURN(ctc_linkagg_create(&linkagg_group));
/* modified by cuixl for bug 17874, 2012-02-21 */
    HAGT_IF_ERROR_RETURN(hagt_ifdb_add_agg_interface(trunkid));

#if 0
    /* modified by liwh for bug 22271 for support cfm on agg, 2013-05-14 */
    gport = CTC_LINKAGG_CHIPID << 8 | trunkid;
    CTC_SET_FLAG(mac_postfix.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
    mac_postfix.low_8bits_mac = mac_addr[5];
    HAGT_IF_ERROR_RETURN(ctc_port_set_port_mac_postfix(gport, &mac_postfix));
#endif
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_del_aggregator
 * Purpose : delete agg
 * Input   : agg id
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_del_aggregator(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 trunkid = 0;  

    /*get info form message*/
    trunkid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggDel.trunkid;  
    HAGT_AGG_DEBUG("_hagt_agg_del_aggregator: trunkid = %d", trunkid);
    
    /*remove agg interface in SDK*/
    HAGT_IF_ERROR_RETURN(ctc_linkagg_destroy(trunkid));
/* modified by cuixl for bug 17874, 2012-02-21 */
    HAGT_IF_ERROR_RETURN(hagt_ifdb_del_agg_interface(trunkid));

    return HAGT_E_NONE;    
}

/*********************************************************************
 * Name    : _hagt_agg_attach_mux_to_aggregator
 * Purpose : set port property when port attach to agg
 * Input   : agg name, port index
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_attach_mux_to_aggregator(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint8 trunkid = 0;
    uint16 portid = 0; 
    mac_addr_t mac_addr;
    ctc_port_mac_postfix_t mac_postfix;

    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    /*get info form message*/
    trunkid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.trunkid;
    portid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.portid;
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.routeEn;   
    sal_memcpy(mac_addr, pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.portmac.buf,
                  pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.portmac.size);

    HAGT_AGG_DEBUG("_hagt_agg_attach_mux_to_aggregator: trunkid = %d, portid = %d, route_en=%d.", 
                        trunkid, portid, route_en);
    
    HAGT_IF_ERROR_RETURN(ctc_linkagg_add_port(trunkid, portid)); 

    if (HAGT_GPORT_IS_LOCAL(portid))
    {
        if (route_en)
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_phy_if_en(portid, TRUE));
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(portid, CTC_PORT_PROP_BRIDGE_EN, 1));
        }

        /*set rx and tx*/
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(portid, HAGT_IF_PORT_FLAG_AGG_OPRED, TRUE));
    }
/* modified by cuixl for bug 17874, 2012-02-21 */
    HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_agg_attribute(trunkid, portid, TRUE));        
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_detach_mux_from_aggregator
 * Purpose : set port property when port detach from agg
 * Input   : agg name, port index
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_detach_mux_from_aggregator(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint8 trunkid = 0;
    uint16 portid = 0;   
    mac_addr_t mac_addr;
    ctc_port_mac_postfix_t mac_postfix;

    sal_memset(&mac_postfix, 0, sizeof(ctc_port_mac_postfix_t));

    /*get info form message*/
    trunkid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxDetach.trunkid;
    portid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxDetach.portid;
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxDetach.routeEn;
    sal_memcpy(mac_addr, pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.portmac.buf,
                  pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggMuxAttach.portmac.size);

    HAGT_AGG_DEBUG("_hagt_agg_detach_mux_from_aggregator: \
                        trunkid = %d, portid = %d, route_en=%d.", 
                        trunkid, portid, route_en);

    /*first free agg port l3ifid, then del from agg*/   
    HAGT_IF_ERROR_RETURN(ctc_linkagg_remove_port(trunkid, portid));   

    /*set rx and tx*/
    HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(portid, HAGT_IF_PORT_FLAG_AGG_OPRED, FALSE));
    
    /* modified by cuixl for bug 17874, 2012-02-21 */
    HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_agg_attribute(trunkid, portid, FALSE));
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_sync_up_member_prop_for_attach
 * Purpose : sync port property to agg when add port to agg
 * Input   : agg property
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_sync_up_member_prop_for_attach(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint8 ip_bind_en = 0;   
    uint16 port_id = 0;

    /*get info form message*/
    port_id = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropSync.portid; 
    ip_bind_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropSync.secbindSync;      
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropSync.routeEn;
    HAGT_AGG_DEBUG("_hagt_agg_sync_up_member_prop_for_attach: port_id = %d, ip_bind_en = %d, route_en = %d", 
                        port_id, ip_bind_en, route_en);

    /*set rx and tx*/
    HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(port_id, HAGT_IF_PORT_FLAG_AGGREGATED, TRUE));
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_sync_up_member_prop_for_detach
 * Purpose : unsync port property to agg when del port from agg
 * Input   : agg property
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_sync_up_member_prop_for_detach(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint8 ip_bind_en = 0;   
    uint16 port_id = 0;

    /*get info form message*/
    port_id =  pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropClear.portid; 
    ip_bind_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropClear.secbindSync;
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpMemPropClear.routeEn;   
        
    HAGT_AGG_DEBUG("_hagt_agg_sync_up_member_prop_for_detach: port_id = %d, ip_bind_en = %d, route_en = %d", 
                        port_id, ip_bind_en, route_en);

    /*set rx and tx*/
    HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(port_id, HAGT_IF_PORT_FLAG_AGGREGATED, FALSE));
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_sync_up_agg_property
 * Purpose : sync agg property to port when add first port to agg
 * Input   : port property
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_sync_up_agg_property(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint16 agg_portid = 0;    
    uint32 l3ifid = 0;
    ctc_l3if_t l3if_info; 

    /*get info form message*/
    agg_portid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropSync.aggPortid;
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropSync.routeEn;
    l3ifid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropSync.l3ifid;

    HAGT_AGG_DEBUG("_hagt_agg_sync_up_agg_property: agg_portid = %d, l3ifid = %d, route_en = %d", 
                        agg_portid, l3ifid, route_en);

    if (route_en)
    {
        l3if_info.gport = agg_portid;
        l3if_info.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        l3if_info.vlan_id = GLB_INVALID_VLANID;
        HAGT_IF_ERROR_RETURN(ctc_l3if_create(l3ifid, &l3if_info));
        //HAGT_IF_ERROR_RETURN(hagt_l3if_set_default_attributes(l3ifid, &l3if_info));          
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_clear_agg_property
 * Purpose : unsync agg property from port when last port leave
 * Input   : port property
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_clear_agg_property(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 route_en = 0;
    uint16 agg_portid = 0; 
    uint32 l3ifid = 0;
    ctc_l3if_t l3if_info;

    /*get info form message*/
    agg_portid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropClear.aggPortid;
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropClear.routeEn;
    l3ifid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggPropClear.l3ifid;

    HAGT_AGG_DEBUG("_hagt_agg_clear_agg_property: agg_portid = %d, route_en = %d, l3ifid = %d", 
                        agg_portid, route_en, l3ifid);
    
    if (route_en)
    {
        l3if_info.gport = agg_portid;
        l3if_info.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        l3if_info.vlan_id = GLB_INVALID_VLANID;
        //HAGT_IF_ERROR_RETURN(hagt_l3if_clear_all_attributes(l3ifid, &l3if_info));
        HAGT_IF_ERROR_RETURN(ctc_l3if_destory(l3ifid, &l3if_info));
    }
    return HAGT_E_NONE;
}

#if 0
static int32
_hagt_agg_set_port_flag(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 enable = 0;
    uint8 flag = 0;   
    uint16 port_id = 0;

    /*get info form message*/
    port_id = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSetPortFlag.portid; 
    flag = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSetPortFlag.flag;      
    enable = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSetPortFlag.enable;
    HAGT_AGG_DEBUG("_hagt_agg_set_port_flag: port_id = %d, flag = %d, enable = %d", 
                        port_id, flag, enable);
    if (GLB_AGG_FLAG_DISABLE_DISTRIBUTOR == flag)
    {
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(port_id, HAGT_IF_PORT_FLAG_DIS_DISTRIBUTOR, enable));
        return HAGT_E_NONE;
    }
    if (GLB_AGG_FLAG_DISABLE_COLLECTOR == flag)
    {
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag(port_id, HAGT_IF_PORT_FLAG_DIS_COLLECTOR, enable));
        return HAGT_E_NONE;
    }
    if (GLB_AGG_FLAG_ENABLE_DLB == flag)
    {
        if(port_id <GLB_LINKAGG_MAX_GROUP)
        {
            hagt_agg_enable_dlb_status[port_id] = enable;
        }
    }
    
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_show_agg_member
 * Purpose : show member info in sdk
 * Input   : agg tid
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_show_agg_member(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 cnt = 0;
    uint8 i = 0;
    uint8 trunkid = 0; 
    uint16 gports[GLB_LINKAGG_MAX_MEMBER] = {0};
    char *p_str = NULL;
    char log_str[HAGT_AGG_LOG_STR_MAX_LEN] = {0};
    
    trunkid =  pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpShowMemPorts.tidorPortorVid; 
        
    HAGT_AGG_DEBUG("_hagt_agg_show_agg_member: trunkid = %d", trunkid); 
    /*if the agg is not exit return error*/
    ctc_linkagg_get_member_ports(trunkid, gports, &cnt);
    p_str = log_str;
    if (cnt)
    {
        for (i = 0; i < cnt; i++)
        {
            sal_sprintf(p_str, "%d, ", gports[i]);
            p_str = log_str + sal_strlen(log_str);
        }
        HAGT_AGG_DEBUG("Agg %d has %d members: %s", trunkid, cnt, log_str); 
    }
    else
    {
        HAGT_AGG_DEBUG("Agg %d has no members or not created", trunkid); 
    }
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_sync_aggregator
 * Purpose : create agg
 * Input   : agg tid
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_sync_aggregator(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint8 trunkid = 0; 
    uint8 route_en = 0;
    uint16 gport = 0;
    uint32 l3ifid = 0;
    ctc_l3if_t l3if_info; 
    ctc_linkagg_group_t linkagg_group;

    /*get info form message*/
    route_en = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSync.routeEn; 
    trunkid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSync.trunkid;
    l3ifid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpSync.l3ifid;
    gport = GLB_TRANS_TID_TO_AGGPORT(trunkid);
    linkagg_group.linkagg_mode = 0;
    linkagg_group.tid = trunkid;
    HAGT_AGG_DEBUG("_hagt_agg_sync_aggregator: trunkid = %d", trunkid);
    
    /*create agg interface in SDK*/
    HAGT_IF_ERROR_RETURN(ctc_linkagg_create(&linkagg_group));
    if (route_en)
    {
        l3if_info.gport = gport;
        l3if_info.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        l3if_info.vlan_id = GLB_INVALID_VLANID;
        HAGT_IF_ERROR_RETURN(ctc_l3if_create(l3ifid, &l3if_info));
        HAGT_IF_ERROR_RETURN(hagt_l3if_set_default_attributes(l3ifid, &l3if_info));         
     }

    return HAGT_E_NONE;
}
#endif

/*********************************************************************
 * Name    : _hagt_agg_set_load_balanec
 * Purpose : set load balance
 * Input   : load balance type
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_set_load_balance(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    int32 load_balance = 0;
    ctc_linkagg_psc_t linkagg_hash_ctl;

    /*get info form message*/
    load_balance = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggSetLoadBalance.loadBalance;
    HAGT_AGG_DEBUG("_hagt_agg_set_load_balanec: type is  = %d", load_balance);

    sal_memset(&linkagg_hash_ctl, 0, sizeof(ctc_linkagg_psc_t));  
    linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
    linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
    linkagg_hash_ctl.psc_type_bitmap |=  CTC_LINKAGG_PSC_TYPE_L4;

    if(load_balance & GLB_LAG_LOAD_BALANCE_SRC_MAC)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_MACSA); 
        HAGT_LOG_DEBUG_AGG("linkagg hash for macsa\n");
    }
    
    if(load_balance & GLB_LAG_LOAD_BALANCE_DST_MAC)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.l2_flag, CTC_LINKAGG_PSC_L2_MACDA);
        HAGT_LOG_DEBUG_AGG("linkagg hash for macda\n");
    }
        
    if(load_balance &  GLB_LAG_LOAD_BALANCE_SRC_IP)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_IPSA);
        HAGT_LOG_DEBUG_AGG("linkagg hash for ipsa\n");
    }

    if(load_balance &  GLB_LAG_LOAD_BALANCE_DST_IP)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.ip_flag, CTC_LINKAGG_PSC_USE_IP); 
        GLB_SET_FLAG(linkagg_hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_IPDA);
        HAGT_LOG_DEBUG_AGG("linkagg hash for ipda\n");
    }

    if(load_balance & GLB_LAG_LOAD_BALANCE_IP_PROTOCOL)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.ip_flag, CTC_LINKAGG_PSC_IP_PROTOCOL);
        HAGT_LOG_DEBUG_AGG("linkagg hash for ip protocol\n");
    }
        
    if(load_balance &  GLB_LAG_LOAD_BALANCE_SRC_PORT)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.l4_flag, CTC_LINKAGG_PSC_L4_SRC_PORT);
        HAGT_LOG_DEBUG_AGG("linkagg hash for layer4 source port\n");
    }
    
    if(load_balance &  GLB_LAG_LOAD_BALANCE_DST_PORT)
    {
        GLB_SET_FLAG(linkagg_hash_ctl.l4_flag, CTC_LINKAGG_PSC_USE_L4);
        GLB_SET_FLAG(linkagg_hash_ctl.l4_flag, CTC_LINKAGG_PSC_L4_DST_PORT);
        HAGT_LOG_DEBUG_AGG("linkagg hash for layer4 dest port\n");
    }

    ctc_linkagg_set_psc(&linkagg_hash_ctl); 

    return HAGT_E_NONE;
}

int32
_hagt_agg_set_load_balance_mode(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint32 tid = 0;
    uint32 mode = 0;

    tid = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggSetLoadBalanceMode.tid;
    mode = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgLacpAggSetLoadBalanceMode.mode;

    hagt_agg_enable_mode[tid] = mode;

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_agg_msg_callback_register
 * Purpose : This function is used to register hsrv function
 * Input   : N/A
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_msg_callback_register()
{ 
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggAdd, _hagt_agg_add_aggregator);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggDel, _hagt_agg_del_aggregator);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggPropSync, _hagt_agg_sync_up_agg_property);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggPropClear, _hagt_agg_clear_agg_property);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpMemPropSync, _hagt_agg_sync_up_member_prop_for_attach);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpMemPropClear, _hagt_agg_sync_up_member_prop_for_detach); 
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggMuxAttach, _hagt_agg_attach_mux_to_aggregator);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggMuxDetach, _hagt_agg_detach_mux_from_aggregator);
#if 0
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpSetPortFlag, _hagt_agg_set_port_flag);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpSync, _hagt_agg_sync_aggregator); 
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpShowMemPorts, _hagt_agg_show_agg_member); 
#endif
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggSetLoadBalance, _hagt_agg_set_load_balance);

    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgLacpAggSetLoadBalanceMode, _hagt_agg_set_load_balance_mode);

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_agg_nexthop_init
 * Purpose : This function is used to init agg nexthop
 * Input   : N/A
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
static int32
_hagt_agg_nexthop_init(void)
{
#if 0
    uint32 i = 0;
    uint16 gport = 0;
    /*modified by jiangz for bug16559, 2011-10-08, for merge sdk*/
    ctc_nh_param_brguc_sub_type_t subtype;

    subtype = CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC
            | CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED
            | CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS
            | CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU;

    for (i = 1; i < GLB_LINKAGG_MAX_GROUP; i++)
    {
        gport = GLB_TRANS_TID_TO_AGGPORT(i);
// TODO: Commented by xgu for compile, 20121120           HAGT_IF_ERROR_RETURN(ctc_l2_create_ucast_nh(gport, subtype));
    }
#endif
    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : hagt_agg_init
 * Purpose : This function is used to init vlan module
 * Input   : N/A
 * Output  : N/A
 * Return  : hagt_e_***  - hal agent xxx pointer
 * Note    : N/A
*********************************************************************/
int32
hagt_agg_init(hagt_master_t* pst_hagt_master)
{   
    /*register cb for MSG from hal server*/
    _hagt_agg_msg_callback_register ();

    HAGT_IF_ERROR_RETURN(_hagt_agg_nexthop_init());
    return HAGT_E_NONE;
}

