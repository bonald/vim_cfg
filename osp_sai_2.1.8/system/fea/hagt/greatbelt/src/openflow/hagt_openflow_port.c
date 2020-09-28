
#include "ctc_api.h"

#include "glb_qos_define.h"

#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_openflow.h"
#include "hagt_if.h"
#include "glb_stm_define.h"
#include "ctc_hash.h"

/****************************************************************************************
 *  
 * Globals
 *
 ****************************************************************************************/

extern hagt_master_t *g_pst_hagt_master;

/* Added by weizj for inband */
static uint32 HAGT_OPENFLOW_INBAND_ILOOP_GPORT = 0;

extern uint32 hagt_ipuc_get_ipv6_enabled();

extern int32 ctc_sai_port_recover_vlan_ctl(uint32 gport);

extern int32 ctc_sai_port_openflow_set_vlan_filter(uint32 gport, bool enable);

extern int32 ctc_sai_port_openflow_set_openflow_enable(uint32 gport, bool enable);

/****************************************************************************************
 *  
 * Function
 *
 ****************************************************************************************/

hagt_openflow_port_mac_t*
openflow_port_mac_get_by_gport(uint32 gport)
{
    hagt_openflow_port_mac_t openflow_port_mac;

    sal_memset(&openflow_port_mac, 0, sizeof(hagt_openflow_port_mac_t));
    openflow_port_mac.gport = gport;
    return ctc_hash_lookup(g_pst_hagt_master->openflow_port_mac_hash, &openflow_port_mac);
}

hagt_openflow_port_mac_t*
openflow_port_mac_add_hash(hagt_openflow_port_mac_t* p_openflow_port_mac_param)
{
    hagt_openflow_port_mac_t* p_openflow_port_mac = NULL;

    p_openflow_port_mac = HAGT_MALLOC(MEM_FEA_HAGT_MODULE,sizeof(hagt_openflow_port_mac_t));
    if (NULL == p_openflow_port_mac)
    {
        return NULL;
    }
    sal_memcpy(p_openflow_port_mac, p_openflow_port_mac_param, sizeof(hagt_openflow_port_mac_t));
    return ctc_hash_insert(g_pst_hagt_master->openflow_port_mac_hash, p_openflow_port_mac);
}

static int32
_hagt_openflow_port_set_pdu_action(uint16 gport)
{
    /* Set well-known l2pdu action to normal forward to comply to OpenFlow standard.
       NOTE: 
       1. the action index for these l2pdu is set by hagt_cpu_traffic_init. 
       2. Until we support hybrid mode, we do not to set action for l3pdu. */
       
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_BPDU, 
                                CTC_PDU_L2PDU_ACTION_TYPE_FWD));               
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_LLDP, 
                                CTC_PDU_L2PDU_ACTION_TYPE_FWD)); 
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_SLOW_PROTO, 
                                CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU)); 

    /* Ensure arp/dhcp packet is forwarded by openflow rules. */
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_EXCP_NORMAL_FWD));
    HAGT_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, CTC_EXCP_NORMAL_FWD));

    return HAGT_E_NONE;
}

static int32
_hagt_openflow_port_unset_pdu_action(uint16 gport)
{
    /* Restore well-known l2pdu action. */
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_BPDU, 
                                CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU));                
    HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_LLDP, 
                                CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU));  


    // TODO:  hybrid will not be support ,l3pdu state is not needed to be restored.
    
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_set_protected_vlan(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortSetProtectedVlanReq_t *req;
    ctc_acl_entry_t acl_entry;
    uint32 entry_id = 0;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetProtectedVlan;
    sal_memset(&acl_entry, 0x0, sizeof(ctc_acl_entry_t));
    if(req->enable)
    {
        acl_entry.key.type = CTC_ACL_KEY_IPV4;
        acl_entry.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
        acl_entry.key.u.ipv4_key.flag = CTC_ACL_IPV4_KEY_FLAG_SVLAN;
        acl_entry.key.u.ipv4_key.svlan = req->vlanId;
        acl_entry.key.u.ipv4_key.svlan_mask = 0xffff;

        acl_entry.entry_id = 0xffffffff - (req->vlanId << 10) - req->groupId;
        acl_entry.priority_valid = 1;
        acl_entry.priority = 0xffffffff;
        HAGT_IF_ERROR_RETURN(ctc_acl_add_entry(req->groupId, &acl_entry));
        HAGT_IF_ERROR_RETURN(ctc_acl_install_entry(acl_entry.entry_id));    
    }
    else
    {
        entry_id = 0xffffffff - (req->vlanId << 10) - req->groupId;
        HAGT_IF_ERROR_RETURN(ctc_acl_uninstall_entry(entry_id));
        HAGT_IF_ERROR_RETURN(ctc_acl_remove_entry(entry_id));
    }

    return HAGT_E_NONE;
}

/* Added by weizj for inband */
int32 
hagt_openflow_iloop_port_enable(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortEnableReq_t *req;
    Hagt2SrvMsg_t hagt_msg;
    ctc_acl_group_info_t group_info;
    ctc_internal_port_assign_para_t port_assign;
    ctc_direction_t dir;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetIloopEnable;
    
    sal_memset(&group_info, 0, sizeof(ctc_acl_group_info_t));
    group_info.dir = dir = CTC_INGRESS;

    sal_memset(&port_assign, 0, sizeof(port_assign));
    port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    port_assign.gchip = 0;
    
    if (req->enable)
    {
        HAGT_IF_ERROR_RETURN(ctc_alloc_internal_port(&port_assign));
        req->gport = HAGT_OPENFLOW_INBAND_ILOOP_GPORT = port_assign.inter_port;
        
        //HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_EN, dir, CTC_ACL_EN_0));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, req->label));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, CTC_ACL_TCAM_LKUP_TYPE_L2_L3));
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, dir, 1));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_VLAN_FILTER_EN, dir, 1));
        HAGT_IF_ERROR_RETURN(_hagt_openflow_port_set_pdu_action(req->gport));
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en (req->gport, TRUE));
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(req->gport, TRUE));
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, TRUE)); 
        /* Enable reflective bridge, we will control using met entry. */
        HAGT_IF_ERROR_RETURN(ctc_port_set_reflective_bridge_en(req->gport, TRUE));
    }
    else
    {
        req->gport = port_assign.inter_port = HAGT_OPENFLOW_INBAND_ILOOP_GPORT;
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_EN, dir, 0));
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, dir, 0));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_VLAN_FILTER_EN, dir, 0));
        HAGT_IF_ERROR_RETURN(_hagt_openflow_port_unset_pdu_action(req->gport));
        /* Enable reflective bridge, we will control using met entry. */
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en (req->gport, FALSE));
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(req->gport, FALSE));
        HAGT_IF_ERROR_RETURN(ctc_port_set_reflective_bridge_en(req->gport, FALSE));

        HAGT_IF_ERROR_RETURN(ctc_free_internal_port(&port_assign));
        HAGT_OPENFLOW_INBAND_ILOOP_GPORT = 0;
    }

    sal_memset(&hagt_msg, 0, sizeof(Hagt2SrvMsg_t));
    hagt_msg.agentId = 0;
    hagt_msg.opHagt2Srv.present = opHagt2Srv_PR_hagt2SrvMsgOpenFlowPortSetConfig;
    hagt_msg.opHagt2Srv.hagt2SrvMsgOpenFlowPortSetConfig.gport = HAGT_OPENFLOW_INBAND_ILOOP_GPORT;
    HAGT2SRV_MSG_SEND(&hagt_msg);
    
    return HAGT_E_NONE;
}
/* End by weizj for inband */

int32 
hagt_openflow_port_ignore_vlan_filter_ctl(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortIgnoreVlanFilterCtlReq_t* req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl;

    if (req->ignoreVlanFilter)
    {
        HAGT_IF_ERROR_RETURN(ctc_sai_port_openflow_set_vlan_filter(req->gport, 1));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_sai_port_openflow_set_vlan_filter(req->gport, 0));
    }
    return HAGT_E_NONE;
}

int32 
hagt_openflow_port_tunnel_mpls_disable_ctl(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortTunnelMplsDisableReq_t* req;
    ctc_port_scl_property_t prop;


    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl;

    sal_memset(&prop, 0x0, sizeof(ctc_port_scl_property_t));
    prop.scl_id = 1;
    prop.direction = CTC_INGRESS;
    prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    if (req->tunnelMplsDisable)
    {
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
    }
    else
    {
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
    }
    return HAGT_E_NONE;
}

int32 hagt_openflow_port_native_vlan_config(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortNativeVlanConfigReq_t* req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig;
    if (req->ingressAddNativeVlan)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
    }

#if 0
    if(req->egressRemoveNativeVlan)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(req->gport, 1, 1));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(req->gport, 0, 0));
    }
#endif
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_action(int32 gport_id, hagt_openflow_port_l2pdu_action_t l2pdu_action, 
                                        hagt_openflow_port_l2pdu_action_indx_t action_index)
{
    switch (l2pdu_action)
    {
        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_FWD));
            break;
        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_DISCARD:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_DISCARD));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_MAX:
        default:
            break;
    }

    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_stp_action(int32 gport_id, int32 stp_action)
{
    hagt_openflow_port_l2pdu_action_t       l2pdu_action;
    hagt_openflow_port_l2pdu_action_indx_t  action_index;

    if (GLB_OPF_PKT_ACTION_FORWARD == stp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP;
    }
    if (GLB_OPF_PKT_ACTION_OVS == stp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP;
    }
    if (GLB_OPF_PKT_ACTION_SYSTEM == stp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP;
    }
    
    hagt_openflow_port_l2pdu_set_packet_action(gport_id, l2pdu_action, action_index);
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_lacp_action(int32 gport_id, int32 lacp_action)
{
    hagt_openflow_port_l2pdu_action_t       l2pdu_action;
    hagt_openflow_port_l2pdu_action_indx_t  action_index;

    if (GLB_OPF_PKT_ACTION_FORWARD == lacp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO;
    }
    if (GLB_OPF_PKT_ACTION_OVS == lacp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO;
    }
    if (GLB_OPF_PKT_ACTION_SYSTEM == lacp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO;
    }
    
    hagt_openflow_port_l2pdu_set_packet_action(gport_id, l2pdu_action, action_index);
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_dot1x_action(int32 gport_id, int32 dot1x_action)
{
    hagt_openflow_port_l2pdu_action_t       l2pdu_action;
    hagt_openflow_port_l2pdu_action_indx_t  action_index;

    if (GLB_OPF_PKT_ACTION_FORWARD == dot1x_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_EAPOL;
    }
    if (GLB_OPF_PKT_ACTION_OVS == dot1x_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_EAPOL;
    }
    if (GLB_OPF_PKT_ACTION_SYSTEM == dot1x_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_EAPOL;
    }
    
    hagt_openflow_port_l2pdu_set_packet_action(gport_id, l2pdu_action, action_index);
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_lldp_action(int32 gport_id, int32 lldp_action)
{
    hagt_openflow_port_l2pdu_action_t       l2pdu_action;
    hagt_openflow_port_l2pdu_action_indx_t  action_index;

    if (GLB_OPF_PKT_ACTION_FORWARD == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP;
    }
    if (GLB_OPF_PKT_ACTION_OVS == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP;
    }
    if (GLB_OPF_PKT_ACTION_SYSTEM == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP;
    }

    hagt_openflow_port_l2pdu_set_packet_action(gport_id, l2pdu_action, action_index);
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_l2pdu_set_packet_erps_action(int32 gport_id, int32 lldp_action)
{
    hagt_openflow_port_l2pdu_action_t       l2pdu_action;
    hagt_openflow_port_l2pdu_action_indx_t  action_index;

    if (GLB_OPF_PKT_ACTION_FORWARD == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_ERPS;
    }
    if (GLB_OPF_PKT_ACTION_OVS == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_ERPS;
    }
    if (GLB_OPF_PKT_ACTION_SYSTEM == lldp_action)
    {
        l2pdu_action = HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
        action_index = HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_ERPS;
    }

    hagt_openflow_port_l2pdu_set_packet_action(gport_id, l2pdu_action, action_index);
    return HAGT_E_NONE;
}

int32
hagt_openflow_port_set_packet_action(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortSetPacketActionReq_t* req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction;

    hagt_openflow_port_l2pdu_set_packet_stp_action(req->gport, req->stpAction);
    hagt_openflow_port_l2pdu_set_packet_lacp_action(req->gport, req->lacpAction);
    hagt_openflow_port_l2pdu_set_packet_lldp_action(req->gport, req->lldpAction);
    hagt_openflow_port_l2pdu_set_packet_erps_action(req->gport, req->erpsAction);
    hagt_openflow_port_l2pdu_set_packet_dot1x_action(req->gport, req->dot1xAction);

    /* TODO syn with system action or default, 2017-02-21 */
    return HAGT_E_NONE;
}

int32 
hagt_openflow_port_enable(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortEnableReq_t *req;
    ctc_port_scl_property_t prop;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable;

    sal_memset(&prop, 0x0, sizeof(ctc_port_scl_property_t));

    HAGT_IF_ERROR_RETURN(ctc_port_get_scl_property(req->gport, &prop));
    prop.direction = CTC_INGRESS;
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    if (req->enable)
    {
        prop.scl_id = 1;
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
        //HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(req->gport, 0, 0));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 1)); 
        HAGT_IF_ERROR_RETURN(ctc_sai_port_openflow_set_openflow_enable(req->gport, 1));
        hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU, HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO);
        hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD, HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP);
                /* Enable reflective bridge, we will control using met entry. */
        //HAGT_IF_ERROR_RETURN(ctc_port_set_reflective_bridge_en(req->gport, TRUE));
    }
    else
    {
        prop.scl_id = 1;
        prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
        HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(req->gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0)); 
        //HAGT_IF_ERROR_RETURN(ctc_port_set_untag_dft_vid(req->gport, 1, 1));
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
        HAGT_IF_ERROR_RETURN(ctc_sai_port_openflow_set_openflow_enable(req->gport, 0));
        hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU, HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO);
        hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU, HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP);
                /* disable reflective bridge, we will control using met entry. */
        //HAGT_IF_ERROR_RETURN(ctc_port_set_reflective_bridge_en(req->gport, FALSE));

    }
    
    return HAGT_E_NONE;
}

#if 0
int32
hagt_openflow_port_l2pdu_set_packet_action(int32 gport_id, hagt_openflow_port_l2pdu_action_t l2pdu_action, 
                                        hagt_openflow_port_l2pdu_action_indx_t action_index)
{
    switch (l2pdu_action)
    {
        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_FWD));
            break;
        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_DISCARD:
            HAGT_IF_ERROR_RETURN(ctc_l2pdu_set_port_action(gport_id, action_index, CTC_PDU_L2PDU_ACTION_TYPE_DISCARD));
            break;

        case HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_MAX:
        default:
            break;
    }

    return HAGT_E_NONE;
}
#endif

int32 
hagt_openflow_port_set_config(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortSetConfigReq_t* req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig;

    if (req->portConfig & HAGT_OPENFLOW_NO_FWD )
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en(req->gport, FALSE));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en(req->gport, TRUE));
    }

    if (req->portConfig & HAGT_OPENFLOW_NO_RECV)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(req->gport, FALSE));
#if 0
        if (req->portConfig & HAGT_OPENFLOW_PORT_NO_RECV_STP)
        {
            HAGT_IF_ERROR_RETURN(hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_DISCARD, 
                                                        HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU, 
                                                        HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP));
        }
#endif
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(req->gport, TRUE));
#if 0
        if (req->portConfig & HAGT_OPENFLOW_PORT_NO_RECV_STP)
        {
            HAGT_IF_ERROR_RETURN(hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_DISCARD, 
                                                        HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(hagt_openflow_port_l2pdu_set_packet_action(req->gport, HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD, 
                                                        HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP));
        }
#endif
    }

    return HAGT_E_NONE;
}

int32 
hagt_openflow_port_set_label(Hsrv2AgtMsg_t* p_msg_data)
{
    uint16 gport = 0;
    uint32 label = 0;
    HalMsgOpenFlowPortSetLabelReq_t *req;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetLabel;
    gport = req->gport;
    label = req->label;

    HAGT_IF_ERROR_RETURN(ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, CTC_INGRESS, label));

    return HAGT_E_NONE;
}

/* Added by weizj for udf */
int32 
hagt_openflow_port_set_udf_config(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortSetConfigReq_t* req;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetUdf;
    if (req->portConfig)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, FALSE));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, TRUE));
    }

    return HAGT_E_NONE;
}
/* End by weizj for udf */

static uint32_t
openflow_port_mac_hash_make(void* data)
{
    hagt_openflow_port_mac_t* p_openflow_port_mac = (hagt_openflow_port_mac_t*)data;

    return ctc_hash_caculate(sizeof(uint32), &p_openflow_port_mac->gport);
}

static bool
openflow_port_mac_hash_cmp(void *data, void *data1)
{
    hagt_openflow_port_mac_t* p_openflow_port_mac = data;
    hagt_openflow_port_mac_t* p_openflow_port_mac1 = data1;

    if ((p_openflow_port_mac->gport) == (p_openflow_port_mac1->gport))
    {
        return TRUE;
    }

    return FALSE;
}

int32
hagt_openflow_port_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortEnable, hagt_openflow_port_enable);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetProtectedVlan, hagt_openflow_port_set_protected_vlan);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl, hagt_openflow_port_ignore_vlan_filter_ctl);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl, hagt_openflow_port_tunnel_mpls_disable_ctl);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowNativeVlanConfig, hagt_openflow_port_native_vlan_config);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetPacketAction, hagt_openflow_port_set_packet_action);

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetConfig, hagt_openflow_port_set_config);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetLabel, hagt_openflow_port_set_label);
    /* Added by weizj for inband */
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetIloopEnable, hagt_openflow_iloop_port_enable);
    /* Added by weizj for udf */
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetUdf, hagt_openflow_port_set_udf_config);

#undef Reg
    g_pst_hagt_master->openflow_port_mac_hash = ctc_hash_create(64, 32, openflow_port_mac_hash_make, openflow_port_mac_hash_cmp);

    return HAGT_E_NONE;
}
