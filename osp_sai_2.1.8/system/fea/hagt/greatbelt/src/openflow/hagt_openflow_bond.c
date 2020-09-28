/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "hagt_inc.h"

#include "hagt_openflow.h"
#include "hagt_debug.h"
#include "hagt_openflow_bond.h"

#include "glb_cpu_traffic_define.h"
/****************************************************************************************
 *
 * Globals
 *
 ****************************************************************************************/

/****************************************************************************************
 *
 * Function
 *
 ****************************************************************************************/

#define _____INTERNAL_API_____

extern hagt_openflow_port_mac_t*
openflow_port_mac_get_by_gport(uint32 gport);

hagt_openflow_port_mac_t*
openflow_port_mac_add_hash(hagt_openflow_port_mac_t* p_openflow_port_mac_param);

static inline uint32
_hagt_openflow_bond_slave_hash_make(void *p_info_void)
{
    hagt_openflow_slave_info_t *p_info = p_info_void;

    return (uint32) p_info->gport;
}

static inline bool
_hagt_openflow_bond_slave_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hagt_openflow_slave_info_t *p_info_in_bucket = p_info_in_bucket_void;
    hagt_openflow_slave_info_t *p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->gport != p_info_to_lkp->gport)
    {
        return FALSE;
    }

    return TRUE;
}

static hagt_openflow_slave_info_t *
_hagt_openflow_bond_calloc_slave (void)
{
    return (hagt_openflow_slave_info_t *)HAGT_OPENFLOW_CALLOC(sizeof(hagt_openflow_slave_info_t));
}

static int32
_hagt_openflow_bond_free_slave (hagt_openflow_slave_info_t *pst_slave_info)
{
    HAGT_OPENFLOW_PTR_CHECK(pst_slave_info);
  
    HAGT_OPENFLOW_FREE(pst_slave_info);

    return HAGT_E_NONE;
}

static int32
_hagt_openflow_bond_add_slave(uint8 tid, uint16 gport)
{
    hagt_openflow_slave_info_t *p_new_slave_info = NULL;

    if (! HAGT_OPENFLOW_IS_VALID_BOND_ID(tid))
    {
        return HAGT_E_INVALID_PARAM;
    }

    p_new_slave_info = _hagt_openflow_bond_calloc_slave();
    HAGT_OPENFLOW_MEM_CHECK(p_new_slave_info);
    p_new_slave_info->tid = tid;
    p_new_slave_info->gport = gport;

    if (!ctclib_hash_get(g_pst_hagt_openflow_master->pst_slave_hash, p_new_slave_info, ctclib_hash_alloc_intern))
    {
        HAGT_IF_ERROR_RETURN(_hagt_openflow_bond_free_slave(p_new_slave_info));
        return HAGT_E_NO_MEMORY;
    }

    return HAGT_E_NONE;
}

static inline int32
_hagt_openflow_bond_del_slave(uint8 tid, uint16 gport)
{
    hagt_openflow_slave_info_t slave_info;
    hagt_openflow_slave_info_t *p_slave_info;

    if (! HAGT_OPENFLOW_IS_VALID_BOND_ID(tid))
    {
        return HAGT_E_INVALID_PARAM;
    }

    /* sync db */
    memset(&slave_info, 0, sizeof(hagt_openflow_slave_info_t));
    slave_info.gport = gport;

    p_slave_info = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_slave_hash, &slave_info);
    if (p_slave_info)
    {
        ctclib_hash_release(g_pst_hagt_openflow_master->pst_slave_hash, p_slave_info);
        HAGT_IF_ERROR_RETURN(_hagt_openflow_bond_free_slave(p_slave_info));
    }

    return HAGT_E_NONE;
}
#ifdef V580_OPENFLOW
/* reference: hagt_cpu_traffic_map_tocpu_reason */
static int32
_hagt_openflow_bond_map_tocpu_reason (glb_packet_tocpu_reason_t hsrv_reason,
                                   ctc_packet_tocpu_reason_t* p_ctc_reason,
                                   uint8* p_sub_idx)
{
    GLB_PTR_VALID_CHECK(p_ctc_reason, HAGT_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_sub_idx, HAGT_E_INVALID_PTR);

    *p_sub_idx = 0;

    switch(hsrv_reason)
    {
#if 0 // TODO: Commented by xgu, 2012-12-31
    /*IPE glb reason to ctc exception*/
        case GLB_PKT_REASON_USERID:
            *p_ctc_reason = CTC_EXCEP_INGRESS_USID;
            break;
        case GLB_PKT_REASON_PROTO_VLAN:
            *p_ctc_reason = CTC_EXCEP_INGRESS_PROTO_VLAN;
            break;
#endif
        /* BRIDGE exception blcok */
        case GLB_PKT_REASON_L2_BPDU:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = CTC_L2PDU_ACTION_INDEX_BPDU;
            break;
        case GLB_PKT_REASON_L2_CFM:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_CFM;
            break;
        case GLB_PKT_REASON_L2_SLOW_PROTO:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = CTC_L2PDU_ACTION_INDEX_SLOW_PROTO;
            break;
        case GLB_PKT_REASON_L2_EAPOL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = CTC_L2PDU_ACTION_INDEX_EAPOL;
            break;
        case GLB_PKT_REASON_L2_ERPS:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_ERPS;
            break;
        case GLB_PKT_REASON_L2_SMART_LINK:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_SMART_LINK;
            break;
        case GLB_PKT_REASON_L2_UDLD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_UDLD;
            break;
        /* added by liuyang 2012-11-21*/
        case GLB_PKT_REASON_L2_LLDP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_LLDP;
            break;
        case GLB_PKT_REASON_L2_DEFAULT:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_DEFAULT;
            break;
        /* modified by liyf for g8032 control vlan, 2011-11-24 */
        case GLB_PKT_REASON_L2_MAC_COPY_CPU:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_MAC_COPY_CPU;
            break;

        case GLB_PKT_TOCPU_L2PROTO_MASK_MAC0:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MASK_MAC0;
            break;
            
        /*added by ychen in 2013-09-24 for bug 25029*/
        case GLB_PKT_TOCPU_L2PROTO_MAC0:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC0;
            break;
        case GLB_PKT_TOCPU_L2PROTO_MAC1:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC1;
            break;
        case GLB_PKT_TOCPU_L2PROTO_MAC2:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC2;
            break;
        case GLB_PKT_TOCPU_L2PROTO_MAC3:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC3;
            break;
        case GLB_PKT_TOCPU_L2PROTO_MAC4:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC4;
            break;

        case GLB_PKT_TOCPU_L2PROTO_FULL_MAC0:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_FULL_MAC0;
            break;
        case GLB_PKT_TOCPU_L2PROTO_MAC_DFT:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC_DFT;
            break;
        case GLB_PKT_TOCPU_L2PROTO_TUNNEL_DMAC:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PDU;
            *p_sub_idx = GLB_L2PDU_ACTION_INDEX_L2PROTO_TUNNEL_DMAC;
            break;

        /* ROUTE-IPDA exception block */
        case GLB_PKT_REASON_L3_IPDA_ARP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_ARP;
            break;
        case GLB_PKT_REASON_L3_IPDA_DHCP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_DHCP;
            break;
        case GLB_PKT_REASON_L3_IPDA_RIP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_RIP;
            break;
        case GLB_PKT_REASON_L3_IPDA_LDP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_LDP;
            break;
        case GLB_PKT_REASON_L3_IPDA_OSPF:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_OSPF;
            break;
        case GLB_PKT_REASON_L3_IPDA_PIM:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_PIM;
            break;
        case GLB_PKT_REASON_L3_IPDA_VRRP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_VRRP;
            break;
        case GLB_PKT_REASON_L3_IPDA_RSVP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_RSVP;
            break;
        case GLB_PKT_REASON_L3_IPDA_ICMPV6:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_ICMPV6;
            break;
        case GLB_PKT_REASON_L3_IPDA_MPLS_OAM:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_BFD;
            break;
        case GLB_PKT_REASON_L3_IPDA_BFD: /* modified by huangxt for BFD develop, 2011-03-04 */
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_BFD;
            break;
        /*added by ychen for bug 24885 in 2013-09-13*/
        case GLB_PKT_TOCPU_PROTOCOL_IGMP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = CTC_L3PDU_ACTION_INDEX_IGMP;
            break;
/* modified by cuixl for bug 26218, 2013-12-11 */
        case GLB_PKT_TOCPU_PROTOCOL_MLD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_ACL_MATCH;
            break;
        case GLB_PKT_REASON_L3_IPDA_DEFAULT:
#if 0 /*modified by huangxt for bug 24160. 2013-0802*/
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_PDU;
            *p_sub_idx = GLB_L3PDU_ACTION_INDEX_IPDA;
#else
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_COPY_CPU;
#endif
            break;

        case GLB_PKT_REASON_ICMP_REDIRECT:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT;
            break;
        case GLB_PKT_REASON_LEARNING_FULL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_CPU_LEARN;
            break;
        case GLB_PKT_REASON_MCAST_RPF_FAIL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_MC_RPF_FAIL;
            break;

        /* ingress security */
        case GLB_PKT_REASON_MACSA_DISCARD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_COPY_CPU;            
            break;
        case GLB_PKT_REASON_MACSA_MISMATCH:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_MOVE;           
            break;
        case GLB_PKT_REASON_PORT_SECURITY_DISCARD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT;          
            break;
        case GLB_PKT_REASON_VLAN_SECURITY_DISCARD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L2_VLAN_LEARN_LIMIT;           
            break;

    /*EPE glb reason to ctc exception*/
       case GLB_PKT_REASON_MTU_DONTFRAG:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_MTU_FAIL;
            break;
        case GLB_PKT_REASON_MTU_FRAG:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_MTU_FAIL;
            break;
        case GLB_PKT_REASON_IP_TTL_FAIL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL;
            break;
        case GLB_PKT_REASON_MCAST_TTL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL;
            break;
        case GLB_PKT_REASON_TUNNEL_MTU_DONTFRAG:
            *p_ctc_reason = CTC_EXCEP_EGRESS_TUNNEL_MTU_DONTFRAG;
            break;
        case GLB_PKT_REASON_TUNNEL_MTU_FRAG:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_MTU_FAIL;
            break;
        case GLB_PKT_REASON_BFD_UDP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_OAM;
            *p_sub_idx = CTC_OAM_EXCP_LEARNING_BFD_TO_CPU;
            break;
        case GLB_PKT_REASON_PTP:
            *p_ctc_reason = CTC_PKT_CPU_REASON_PTP;
            break;

    /*Fatal global reason to ctc fatal exception*/
        case GLB_PKT_REASON_IP_OPTION:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_IP_OPTION;
            break;
        case GLB_PKT_REASON_GRE_UNKOWN:
            *p_ctc_reason = CTC_PKT_CPU_REASON_GRE_UNKNOWN;
            break;
        case GLB_PKT_REASON_UCAST_IP_TTL_FAIL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL;
            break;
        case GLB_PKT_REASON_MORE_RPF:
            *p_ctc_reason = CTC_PKT_CPU_REASON_L3_MC_MORE_RPF;
            break;
        case GLB_PKT_REASON_LINK_ID_FAIL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_LINK_ID_FAIL;
            break;
            /* add by chenyuqiang, for bug 13816, 2010-12-27*/
        case GLB_PKT_REASON_MPLS_TTL_FAIL:
            *p_ctc_reason = CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL;
            break;
        case GLB_PKT_REASON_IGMP_SNOOPING:
            *p_ctc_reason = CTC_PKT_CPU_REASON_IGMP_SNOOPING;
            break;
        /* add by yanxa, for sflow, 2010-08-25*/
#if 0
        case GLB_PKT_REASON_SFLOW_INGRESS:
            *p_ctc_reason = CTC_EXCEPTION_INGRESS_ACL_LOG3;
            break;
        case GLB_PKT_REASON_SFLOW_EGRESS:
            *p_ctc_reason = CTC_EXCEPTION_EGRESS_ACL_LOG3;
            break;
#else
        case GLB_PKT_REASON_SFLOW_INGRESS:
            *p_ctc_reason = CTC_PKT_CPU_REASON_SFLOW_SOURCE;
            break;
        case GLB_PKT_REASON_SFLOW_EGRESS:
            *p_ctc_reason = CTC_PKT_CPU_REASON_SFLOW_DEST;
            break;
#endif
        case GLB_PKT_REASON_NORMAL_FWD:
            *p_ctc_reason = CTC_PKT_CPU_REASON_FWD_CPU;
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}
#endif
#define _____MESSAGE_CB_______

int32
hagt_openflow_bond_add(Hsrv2AgtMsg_t *p_msg_data)
{
    uint8 tid = 0;
    ctc_linkagg_group_t linkagg_grp;
    HalMsgOpenFlowCreateBondReq_t *req = NULL;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowCreateBond;
    tid = req->tid;

    if (! HAGT_OPENFLOW_IS_VALID_BOND_ID(tid))
    {
        return HAGT_E_INVALID_PARAM;
    }

    sal_memset(&linkagg_grp, 0, sizeof(linkagg_grp));
    linkagg_grp.tid = tid;
    linkagg_grp.linkagg_mode = CTC_LINKAGG_MODE_STATIC;
#if 0
    HAGT_IF_ERROR_RETURN(ctc_linkagg_create(&linkagg_grp));

    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = req->gport;
    HAGT_IF_ERROR_RETURN(ctc_l3if_create(req->l3ifid, &l3if));

    //HAGT_IF_ERROR_RETURN(ctc_port_set_phy_if_en(req->gport, TRUE));
    /* Added by weizj for tunnel bug: 33368, default is CTC_DOT1Q_TYPE_BOTH */
    //HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));

    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(req->l3ifid, CTC_L3IF_PROP_MPLS_EN, TRUE ));
    HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(req->l3ifid, CTC_L3IF_PROP_CONTEXT_LABEL_EXIST, TRUE ));
#endif
    /* XXX_V350: SDK will disable DsSrcPort_BridgeEn_f and DsDestPort_BridgeEn_f
     * after we call ctc_port_set_phy_if_en, this will make layer 2 edit on epe
     * fail to work, so we enable this too property explicitly.
     * Note, we have to modify SDK code to allow set the property because SDK
     * check phy_if_en collision. */
    //HAGT_IF_ERROR_RETURN(ctc_port_set_bridge_en(req->gport, TRUE));

    /* Enable reflective bridge, we will control using met entry. */
    //HAGT_IF_ERROR_RETURN(ctc_port_set_reflective_bridge_en(req->gport, TRUE));

    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_del(Hsrv2AgtMsg_t *p_msg_data)
{
    uint8 tid = 0;
    HalMsgOpenFlowDelBondReq_t *req = NULL;;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelBond;
    tid = req->tid;

    if (! HAGT_OPENFLOW_IS_VALID_BOND_ID(tid))
    {
        return HAGT_E_INVALID_PARAM;
    }

    //HAGT_IF_ERROR_RETURN(ctc_linkagg_destroy(tid));
    
    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_slave_add(Hsrv2AgtMsg_t *p_msg_data)
{
    uint8 tid = 0;
    uint16 gport = 0;
    HalMsgOpenFlowAddBondSlaveReq_t *req = NULL;;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddBondSlave;
    tid = req->tid;
    gport = req->gport;

    HAGT_IF_ERROR_RETURN(ctc_linkagg_add_port(tid, gport));
    /* Ensure arp/dhcp packet is forwarded by openflow rules. */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_EXCP_NORMAL_FWD));
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, CTC_EXCP_NORMAL_FWD));
    _hagt_openflow_bond_add_slave(tid, gport);

    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_slave_del(Hsrv2AgtMsg_t *p_msg_data)
{
    uint8 tid = 0;
    uint16 gport = 0;
    HalMsgOpenFlowDelBondSlaveReq_t *req = NULL;;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelBondSlave;
    tid = req->tid;
    gport = req->gport;

    ctc_linkagg_remove_port(tid, gport);
    _hagt_openflow_bond_del_slave(tid, gport);

    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_set_mac(Hsrv2AgtMsg_t *p_msg_data)
{
    uint i = 0;
    hagt_openflow_port_mac_t openflow_port_mac_param;
    hagt_openflow_port_mac_t* p_openflow_port_mac_param = NULL;
    HalMsgOpenFlowBondSetMacReq_t *req = NULL;;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowBondSetMac;
    for (i = 0; i < req ->mac.size; i++)
    {
        openflow_port_mac_param.mac[i] = req ->mac.buf[i];
    }
    openflow_port_mac_param.gport = req->gport;
    p_openflow_port_mac_param = openflow_port_mac_get_by_gport(req->gport);
    if(NULL == p_openflow_port_mac_param)
    {
        openflow_port_mac_add_hash(&openflow_port_mac_param);
    }
    else
    {
        sal_memcpy(p_openflow_port_mac_param->mac, openflow_port_mac_param.mac, sizeof(mac_addr_t));
    }

    return HAGT_E_NONE;
}

#define ________INIT___________

int32
hagt_openflow_bond_register_callback(void)
{
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowCreateBond, hagt_openflow_bond_add);
    // NOT USED NOW SINCE WE PRECREATED ALL BOND PORT. Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelBond, hagt_openflow_bond_del);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddBondSlave, hagt_openflow_bond_slave_add);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelBondSlave, hagt_openflow_bond_slave_del);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowBondSetMac, hagt_openflow_bond_set_mac);

#undef Reg
    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_lacp_init(void)
{
/* reference : HAGT_L2PDU_KEY_INDEX_BPDU */
#define HAGT_OF_L2PDU_KEY_INDEX_BPDU     0

    ctc_qos_queue_cfg_t queue_cfg;
    ctc_packet_tocpu_reason_t reason= 0;
    uint8 sub_idx = 0;

    /*init slow protocol pdu (lacp) action, copy to cpu*/
    ctc_pdu_global_l2pdu_action_t l2pdu_action;
    sal_memset(&l2pdu_action, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    l2pdu_action.entry_valid = TRUE;
    l2pdu_action.copy_to_cpu = TRUE;
    l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_SLOW_PROTO;
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_global_action(
        CTC_PDU_L2PDU_TYPE_SLOW_PROTO,
        HAGT_OF_L2PDU_KEY_INDEX_BPDU,
        &l2pdu_action));
#ifdef V580_OPENFLOW
    /* init slow proto for lacp*/
    _hagt_openflow_bond_map_tocpu_reason(GLB_PKT_REASON_L2_SLOW_PROTO, &reason, &sub_idx);
#endif
    sal_memset(&queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    queue_cfg.value.reason_dest.cpu_reason = (reason + sub_idx);
    queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    HAGT_IF_ERROR_RETURN(ctc_qos_set_queue(&queue_cfg));

    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_db_init(void)
{
    g_pst_hagt_openflow_master->pst_slave_hash = ctclib_hash_create(
            _hagt_openflow_bond_slave_hash_make,
            _hagt_openflow_bond_slave_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_slave_hash);
    
    return HAGT_E_NONE;
}

int32
hagt_openflow_bond_init(void)
{
    hagt_openflow_bond_lacp_init();
    hagt_openflow_bond_db_init();
    
    return HAGT_E_NONE;
}

