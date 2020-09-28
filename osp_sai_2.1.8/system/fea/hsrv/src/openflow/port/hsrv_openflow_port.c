#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_port_priv.h"
#include "hsrv_if.h"

hsrv_openflow_port_master_t *g_pst_openflow_port_master = NULL;

extern int32 hsrv_openflow_tunnel_group_update(fei_openflow_port_notify_link_t *pst_req);

/* Added by weizj for udf */
#define ____HASH_API____

static inline uint32
_hsrv_openflow_port_udf_info_hash_make(void* p_info_void)
{
    hsrv_openflow_port_udf_t* p_info = NULL;
    uint32 ifindex;

    p_info = (hsrv_openflow_port_udf_t*)p_info_void;
    ifindex = p_info->ifindex;

    p_info = NULL;

    return ifindex;
}

static inline bool
_hsrv_openflow_port_udf_info_hash_compare(void *p_info1, void *p_info2)
{
    hsrv_openflow_port_udf_t* p_info_in_bucket = NULL;
    hsrv_openflow_port_udf_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_port_udf_t*)p_info1;
    p_info_to_lkp = (hsrv_openflow_port_udf_t*)p_info2;

    if (p_info_in_bucket->ifindex != p_info_to_lkp->ifindex)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

#define _____MESSAGE_API_____

int32
hsrv_openflow_msg_port_enable(uint32 ifindex, bool enable, uint32 label, uint16 l3ifd)
{

    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport = 0;
    mac_addr_t mac = {0};
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            if(p_interface->lag)
            {
                CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
                    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
                    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortEnable;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.gport = gport;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.label = label;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.l3ifid = l3ifd;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.enable = enable;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.portMac.buf = mac;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.portMac.size = sizeof(mac_addr_t);
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.gchip = HSRV_GPORT_TO_GCHIP(gport);
                    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
                }
            }
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortEnable;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.label = label;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.l3ifid = l3ifd;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.enable = enable;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.portMac.buf = mac;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.portMac.size = sizeof(mac_addr_t);
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortEnable.gchip = HSRV_GPORT_TO_GCHIP(gport);
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_port_set_protected_vlan(uint32 ifindex, bool enable, uint32 vlan_id)
{
    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport = 0;
    uint32 label = 0;
    int32 group_id = 0;
    
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &label));
    group_id = _hsrv_openflow_get_group_id_by_ifindex_class_id(ifindex, label, 0);

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetProtectedVlan;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetProtectedVlan.gport = gport;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetProtectedVlan.enable = enable;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetProtectedVlan.vlanId = vlan_id;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetProtectedVlan.groupId = group_id;

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_port_ignore_vlan_filter(uint32 ifindex, bool ignore_vlan_filter)
{

    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            if(p_interface->lag)
            {
                CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
                    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
                    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl.gport = gport;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl.ignoreVlanFilter = ignore_vlan_filter;
                    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

                }
            }
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowIgnoreVlanFilterCtl.ignoreVlanFilter = ignore_vlan_filter;
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_port_tunnel_mpls_disable(uint32 ifindex, bool tunnel_mpls_disable)
{

    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            if(p_interface->lag)
            {
                CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
                    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
                    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl.gport = gport;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl.tunnelMplsDisable = tunnel_mpls_disable;
                    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

                }
            }
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowTunnelMplsDisableCtl.tunnelMplsDisable = tunnel_mpls_disable;
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_port_native_vlan_config(uint32 ifindex, uint32 ingress_add_native_vlan, uint32 egress_remove_native_vlan)
{

    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            if(p_interface->lag)
            {
                CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
                    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
                    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowNativeVlanConfig;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.gport = gport;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.ingressAddNativeVlan = ingress_add_native_vlan;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.egressRemoveNativeVlan = egress_remove_native_vlan;
                    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

                }
            }
        }
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowNativeVlanConfig;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.ingressAddNativeVlan = ingress_add_native_vlan;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowNativeVlanConfig.egressRemoveNativeVlan = egress_remove_native_vlan;
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }
    return HSRV_E_NONE;
}

int32
hsrv_openflow_msg_port_set_port_act(uint32 ifindex, uint32 *packet_action)
{
    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
        if(p_interface)
        {
            if(p_interface->lag)
            {
                CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
                    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetPacketAction;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.gport = gport;
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.stpAction = packet_action[GLB_OPF_PKT_TYPE_STP];
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.lacpAction = packet_action[GLB_OPF_PKT_TYPE_LACP];
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.lldpAction = packet_action[GLB_OPF_PKT_TYPE_LLDP];
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.igmpAction = packet_action[GLB_OPF_PKT_TYPE_IGMP];
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.erpsAction = packet_action[GLB_OPF_PKT_TYPE_ERPS];
                    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.dot1xAction = packet_action[GLB_OPF_PKT_TYPE_DOT1X];
                    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
                }
            }
        }
    }
    else if(GLB_IF_TYPE_PORT_IF == tbl_interface_get_type_by_ifindex(ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetPacketAction;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.stpAction = packet_action[GLB_OPF_PKT_TYPE_STP];
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.lacpAction = packet_action[GLB_OPF_PKT_TYPE_LACP];
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.lldpAction = packet_action[GLB_OPF_PKT_TYPE_LLDP];
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.igmpAction = packet_action[GLB_OPF_PKT_TYPE_IGMP];
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.erpsAction = packet_action[GLB_OPF_PKT_TYPE_ERPS];
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetPacketAction.dot1xAction = packet_action[GLB_OPF_PKT_TYPE_DOT1X];

        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }
    return HSRV_E_NONE;
}

#ifdef V580_OPENFLOW
static int32
hsrv_openflow_msg_port_set_config(uint32 ifindex, uint32 port_config)
{

    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;

    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetConfig;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.gport = gport;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.portConfig = port_config;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}
#endif
#ifdef V580_OPENFLOW
static void 
hsrv_openflow_msg_bond_set_config(ctclib_hash_backet_t* phb, void *user_data)
{

    int32 ret = 0;
    uint32 set_config = *((uint32 *)user_data);
    hsrv_openflow_slave_data_t* p_slave_info = phb->data;
    hsrv_openflow_port_data_t* p_port_data = NULL;

    if (p_slave_info)
    {
    
#ifdef V580_OPENFLOW
        ret = hsrv_ifdb_get_port_pm_data(p_slave_info->ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data);
#endif
        ret += hsrv_openflow_msg_port_set_config(p_slave_info->ifindex, set_config);
        if (HSRV_E_NONE != ret)
        {
            HSRV_LOG_ERR("HAL Server set slave config failed: ifindex = %d, set_config = %d", p_slave_info->ifindex, set_config);
        }

        p_port_data->port_config = set_config;
    }

    return ;
}
#endif
int32
hsrv_openflow_msg_port_set_label(uint32 ifindex, uint32 label)
{
    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport;
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetLabel;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetLabel.gport = gport;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetLabel.label = label;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    return HSRV_E_NONE;
}

/* Added by weizj for udf */
static int32
hsrv_openflow_msg_port_set_udf_config(uint32 ifindex, uint8 enable)
{
    Hsrv2AgtMsg_t hsrv_msg;
    uint16 gport = 0;
    tbl_interface_t* p_interface = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_interface)
    {
        return HSRV_E_NONE;
    }
    if(hsrv_openflow_cdb_port_is_bond(ifindex))
    {
        if(p_interface->lag)
        {
            CTCLIB_SLIST_LOOP(p_interface->lag->member_ports.obj_list, p_if_mem, listnode)
            {
                gport = CTC_SAI_OBJECT_INDEX_GET(p_if_mem->portid);
                sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
                HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

                hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetUdf;
                hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.gport = gport;
                hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.portConfig = enable;
                HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
            }
        }
    }
    else if (GLB_IF_TYPE_PORT_IF == tbl_interface_get_type_by_ifindex(ifindex))
    {
        gport = CTC_SAI_OBJECT_INDEX_GET(p_interface->portid);
        sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
        HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

        hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortSetUdf;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.gport = gport;
        hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortSetConfig.portConfig = enable;
        HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    }

    return HSRV_E_NONE;
}

#define ____INTERNAL_API_____
/* Added by weizj for udf */
static int32
hsrv_openflow_port_udf_enable(uint32 ifindex, uint16 gport)
{

    hsrv_openflow_port_udf_t lkp_info;
    hsrv_openflow_port_udf_t* p_udf = NULL;

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_port_udf_t));
    lkp_info.ifindex = ifindex;

    /* lookup hash */
    p_udf = ctclib_hash_lookup(HSRV_OPENFLOW_PORT_UDF_HASH, &lkp_info);
    if (p_udf)
    {
        return HSRV_E_ENTRY_NOT_EXIST; 
    }

    /* add to hash */
    p_udf = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_port_udf_t));
    HSRV_OPENFLOW_MEM_CHECK(p_udf);
    sal_memset(p_udf, 0, sizeof(hsrv_openflow_port_udf_t));

    p_udf->ifindex = ifindex;
    p_udf->gport = gport;
    ctclib_hash_get(HSRV_OPENFLOW_PORT_UDF_HASH, p_udf, ctclib_hash_alloc_intern);

    return HSRV_E_NONE;
}


static int32
hsrv_openflow_port_udf_disable(uint32 ifindex)
{
    hsrv_openflow_port_udf_t lkp_info;
    hsrv_openflow_port_udf_t* p_udf = NULL;

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_port_udf_t));
    lkp_info.ifindex = ifindex;

    /* lookup hash */
    p_udf = ctclib_hash_lookup(HSRV_OPENFLOW_PORT_UDF_HASH, &lkp_info);
    if (p_udf)
    {
        /* release hash data */
        ctclib_hash_release(HSRV_OPENFLOW_PORT_UDF_HASH, p_udf);
        HSRV_OPENFLOW_PORT_FREE(p_udf);
    }

    return HSRV_E_NONE;
}
/* End by weizj for udf */

#ifdef V580_OPENFLOW
static int32
hsrv_openflow_port_alloc_internal_data(uint32 ifindex)
{
    hsrv_openflow_port_data_t *p_port_data = NULL;

    p_port_data = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_port_data_t));
    HSRV_OPENFLOW_MEM_CHECK(p_port_data);
    sal_memset(p_port_data, 0, sizeof(hsrv_openflow_port_data_t));
    ctclib_list_init(&p_port_data->list);
    #ifdef V580_OPENFLOW
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_set_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, p_port_data));
    #endif
    /* Added by weizj for bug 36414 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_alloc_port_label(p_port_data));

    return HSRV_E_NONE;
}
#endif

static int32
hsrv_openflow_port_enable(fei_openflow_port_enable_t *pst_req)
{
    hsrv_openflow_label_t label;
    uint16 gport;
#ifdef V580_OPENFLOW
    hsrv_openflow_label_t label;
    hsrv_openflow_port_data_t *p_port_data = NULL;
    uint32 ifindex;
    uint16 gport;

    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;
    
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    if(NULL == p_port_data)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_alloc_internal_data(ifindex));
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    }
    
    if (p_port_data->enable)
    {
        return HSRV_E_NONE;
    }
    p_port_data->enable = pst_req->enable;
    p_port_data->link = pst_req->link_up;

    /* Allocated l3ifid through ifdb to reuse existing logic. */
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_new_l3ifid(GLB_IF_TYPE_PORT_IF, &(p_port_data->l3ifid)));

    label.type = GLB_OPENFLOW_LABEL_PORT;
    label.enable = pst_req->enable;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &label.label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_op_label(&label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(ifindex, pst_req->enable, label.label, p_port_data->l3ifid));
    
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_port_flex_nh(gport, &p_port_data->nh_info));
    /* Added by weizj for udf */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_udf_enable(ifindex, gport));
#endif
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(pst_req->ifindex, &label.label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_set_instance_enable(pst_req ->ifindex ,pst_req->enable));
#if 0
#ifdef DUET2    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(pst_req->ifindex, pst_req->enable, label.label, 0));
#else
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(pst_req->ifindex, pst_req->enable, 0, 0));
#endif
#endif
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(pst_req->ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_udf_enable(pst_req->ifindex, gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(pst_req->ifindex, pst_req->enable, 0, 0));
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_disable(uint32 ifindex)
{
    hsrv_openflow_label_t label;
#ifdef V580_OPENFLOW
    hsrv_openflow_port_data_t* p_port_data = NULL;
    hsrv_openflow_label_t label;
    //uint16 gport;
    
    label.type = GLB_OPENFLOW_LABEL_PORT;
    label.enable = FALSE;
    
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    HSRV_OPENFLOW_PTR_CHECK(p_port_data);
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &label.label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_op_label(&label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(ifindex, FALSE, label.label, p_port_data->l3ifid));
    /* Added by weizj for udf */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_udf_disable(ifindex));
    /* Added by weizj for bug 36414 */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_free_port_label(p_port_data->port_label));

    HSRV_IF_ERROR_RETURN(hsrv_ifdb_free_l3ifid(p_port_data->l3ifid));

#if 0 /* Deleted by weizj for tunnel bug 33724 */
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    {
        hsrv_of_nh_offset_t all_nh;
        hsrv_ofnh_mcast_member_param_t param;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_get_output_all_nhid(&all_nh));
        param.group_nhid = all_nh.nhid;
        param.op_type = HSRV_OFNH_MCAST_OP_DEL_MEMBER;
        param.mem_gport = gport;
        param.mem_nhid = p_port_data->nh_info.nh_offset.nhid;
        param.port_check_discard = TRUE;
        param.redirect_to_ingress_port = FALSE;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_remove_mcast_member(&param));
    }
#endif

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_release_nh_info(&p_port_data->nh_info));

    HSRV_IF_ERROR_RETURN(hsrv_ifdb_set_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, NULL));
    HSRV_OPENFLOW_PORT_FREE(p_port_data);
#endif

    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &label.label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_set_instance_enable(ifindex ,0));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_udf_disable(ifindex));
#ifdef USW
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(ifindex, 0, label.label, 0));
#else
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(ifindex, 0, 0, 0));
#endif
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_set_protected_vlan(fei_openflow_port_set_protected_vlan_t *pst_req)
{
    hsrv_openflow_msg_port_set_protected_vlan(pst_req->ifindex, pst_req->enable, pst_req->vlan_id);
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_ignore_vlan_filter(fei_openflow_port_ignore_vlan_filter_t *pst_req)
{
    uint32 ifindex;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_ignore_vlan_filter(ifindex, pst_req->ignore_vlan_filter));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_tunnel_mpls_disable(fei_openflow_port_tunnel_mpls_disable_t *pst_req)
{
    uint32 ifindex;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_tunnel_mpls_disable(ifindex, pst_req->tunnel_mpls_disable));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_native_vlan_enable(fei_openflow_port_native_vlan_config_t *pst_req)
{
    uint32 ifindex;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_native_vlan_config(ifindex, pst_req->ingress_add_native_vlan, pst_req->egress_remove_native_vlan));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_set_port_action(fei_openflow_port_pkt_action_t *pst_req)
{
    uint32 ifindex;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_set_port_act(ifindex, pst_req->action));

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_set_config(fei_openflow_port_set_config_t *pst_req)
{
#ifdef V580_OPENFLOW
    hsrv_openflow_port_data_t* p_port_data = NULL;
    hsrv_openflow_bond_data_t* p_bond_info = NULL;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);
        
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(pst_req->ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    if(NULL == p_port_data)
    {
        p_port_data = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_port_data_t));
        HSRV_OPENFLOW_MEM_CHECK(p_port_data);
        sal_memset(p_port_data, 0, sizeof(hsrv_openflow_port_data_t));
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_set_port_pm_data(pst_req->ifindex, HSRV_PMID_OPENFLOW, p_port_data));
    }
    
    p_port_data->port_config = pst_req->port_config;
    if (hsrv_openflow_port_is_bond(pst_req->ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(pst_req->ifindex, &p_bond_info));
        ctclib_hash_iterate(p_bond_info->bond_slave_hash, hsrv_openflow_msg_bond_set_config, &(pst_req->port_config));        
    }
    else if (hsrv_openflow_port_is_add_to_bridge(pst_req->ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_set_config(pst_req->ifindex, pst_req->port_config));
    }

    p_port_data->port_config = pst_req->port_config;
#endif
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_add_to_bridge(fei_openflow_port_add_to_bridge_req_t *pst_req)
{
    hsrv_openflow_port_add_to_bridge_by_cdb(pst_req->ifindex, pst_req->enable);
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_del_from_bridge(fei_openflow_port_add_to_bridge_req_t *pst_req)
{
    hsrv_openflow_port_add_to_bridge_by_cdb(pst_req->ifindex, pst_req->enable);
    return HSRV_E_NONE;
}

/* Added by weizj for udf */
static void
_hsrv_openflow_port_loop_udf(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_port_udf_t* p_udf = (hsrv_openflow_port_udf_t*)phb->data;
    uint8 *p_enable = user_data;

    if (*p_enable)
    {
        hsrv_openflow_msg_port_set_udf_config(p_udf->ifindex, *p_enable);
    }
    else
    {
        hsrv_openflow_msg_port_set_udf_config(p_udf->ifindex, *p_enable);
    }
}
/* Deleted by weizj for udf */

#define _____HAL_APIs_____

int32 
fei_api_openflow_port_enable(fei_openflow_port_enable_t *pst_req)
{
    if (pst_req->enable)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_enable(pst_req));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_disable(pst_req->ifindex));
    }
    
    return HSRV_E_NONE;
}

extern int32
fei_api_openflow_tunnel_port_decap_en(fei_openflow_port_decap_req_t* pst_req);

int32
fei_api_openflow_port_operation_agg(fei_openflow_port_operation_agg_t *pst_req)
{
    fei_openflow_port_decap_req_t tunnel_req;
    sal_memset(&tunnel_req, 0x0, sizeof(fei_openflow_port_decap_req_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_enable(pst_req->ifindex, pst_req->enable, 0, 0));
    if(pst_req->enable)
    {
        if(!pst_req->vlan_filter)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_ignore_vlan_filter(pst_req->ifindex, 1));
        }
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_native_vlan_config(pst_req->ifindex, pst_req->ingress_add_native_vlan, 1));
        if(pst_req->tunnel_mpls_disable)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_tunnel_mpls_disable(pst_req->ifindex, 1));
        }
        tunnel_req.ifindex = pst_req->ifindex;
        tunnel_req.enable = 1;
        tunnel_req.type = pst_req->type;
        HSRV_IF_ERROR_RETURN(fei_api_openflow_tunnel_port_decap_en(&tunnel_req));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_ignore_vlan_filter(pst_req->ifindex, 0));
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_native_vlan_config(pst_req->ifindex, 1, 1));
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_tunnel_mpls_disable(pst_req->ifindex, 0));
        tunnel_req.ifindex = pst_req->ifindex;
        tunnel_req.enable = 0;
        tunnel_req.type = GLB_OPENFLOW_TUNNEL_TYPE_VXLAN;
        HSRV_IF_ERROR_RETURN(fei_api_openflow_tunnel_port_decap_en(&tunnel_req));
    }
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_set_protected_vlan(fei_openflow_port_set_protected_vlan_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_set_protected_vlan(pst_req));
    return HSRV_E_NONE;
}


int32 
fei_api_openflow_port_ignore_vlan_filter(fei_openflow_port_ignore_vlan_filter_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_ignore_vlan_filter(pst_req));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_tunnel_mpls_disable(fei_openflow_port_tunnel_mpls_disable_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_tunnel_mpls_disable(pst_req));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_native_vlan_enable(fei_openflow_port_native_vlan_config_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_native_vlan_enable(pst_req));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_set_port_action(fei_openflow_port_pkt_action_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_set_port_action(pst_req));
    
    return HSRV_E_NONE;
}
    
int32 
fei_api_openflow_port_notify_link(fei_openflow_port_notify_link_t *pst_req)
{
    if(hsrv_openflow_phyport_is_openflow_port(pst_req->ifindex))
    {
        hsrv_openflow_group_update_group_liveness(pst_req->ifindex, pst_req->enable);
        hsrv_openflow_tunnel_group_update(pst_req);
    }
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_set_config(fei_openflow_port_set_config_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_set_config(pst_req));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_port_add_to_bridge(fei_openflow_port_add_to_bridge_req_t *pst_req)
{
    if (pst_req->enable)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_add_to_bridge(pst_req));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_del_from_bridge(pst_req));
    }
    
    return HSRV_E_NONE;
}

#define _____MODULE_APIs_____
/* Added by weizj for bug 36414 */
int32
hsrv_openflow_port_alloc_port_label(hsrv_openflow_port_data_t* p_port_data)
{

    uint32 port_label = CTCLIB_OPF_INVALID_OFFSET;

    HSRV_PTR_CHECK(p_port_data);
    
    port_label = ctclib_opf_alloc_offset (HSRV_OPENFLOW_PORT_LABLE_OPF, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == port_label)
    {
        HSRV_LOG_WARN("Openflow port not enough port label offset resource.");
        #ifdef V580_OPENFLOW
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
        #endif
        return -1;
    }
    
    p_port_data->port_label = port_label;
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_port_free_port_label(uint32 port_label)
{
    bool ret = FALSE;
  
    ret = ctclib_opf_free_offset(HSRV_OPENFLOW_PORT_LABLE_OPF, 0, 1, port_label);
    if (FALSE == ret)
    {       
        return HSRV_E_INVALID_PARAM;
    }
  
    return HSRV_E_NONE;
}
/* End by weizj for bug 36414 */

bool
hsrv_openflow_port_is_enabled(uint32 ifindex)
{
    return hsrv_openflow_phyport_is_openflow_port(ifindex);
}

bool
hsrv_openflow_port_is_add_to_bridge(uint32 ifindex)
{
#ifdef V580_OPENFLOW
    hsrv_openflow_port_data_t* p_port_data = NULL;
    
    hsrv_ifdb_get_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data);
    if(p_port_data)
    {
        if (GLB_FLAG_ISSET(p_port_data->flag, IF_PORT_OPENFLOW_FLAG_ADD_TO_BR0))
        {
            return TRUE;
        }
    }

    return FALSE;
#endif
    return TRUE;
}

bool
hsrv_openflow_port_get_link_status(uint32 ifindex)
{
    bool link_status = FALSE;

    if (hsrv_openflow_port_is_bond(ifindex))
    {
        link_status = hsrv_openflow_bond_get_link_status(ifindex);
        return link_status;
    }
#ifdef V580_OPENFLOW
    if (hsrv_openflow_port_is_stub(ifindex))
    {
        link_status = hsrv_openflow_stub_get_link_status(ifindex);
        return link_status;
    }
#endif
    /* Added by weizj for group bug 37038 */
    else if (hsrv_openflow_port_is_tunnel(ifindex))
    {
        /*added by wangl for tunnel port down up*/
        link_status = hsrv_openflow_tunnel_get_link_status(ifindex);
        return link_status;
    }
    else
    {
        link_status = hsrv_openflow_phyport_get_link_status(ifindex);
        return link_status;
    }

    return FALSE;
}

/* Added by weizj for udf */
int32
hsrv_openflow_port_set_udf_disable(void)
{
    int8 enable = 0;

    ctclib_hash_iterate(HSRV_OPENFLOW_PORT_UDF_HASH, _hsrv_openflow_port_loop_udf, &enable);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_port_set_udf_enable(void)
{
    int8 enable = 1;

    ctclib_hash_iterate(HSRV_OPENFLOW_PORT_UDF_HASH, _hsrv_openflow_port_loop_udf, &enable);

    return HSRV_E_NONE;
}
/* End by weizj for udf */

#define _____DEBUG_____

static int32 
hsrv_openflow_show_port__(uint32 ifindex, void* pv_arg)
{
    #ifdef V580_OPENFLOW
    hsrv_openflow_port_data_t * p_intf_data = NULL;
    FILE *fp = (FILE *)pv_arg;
    uint32 label = 0;

    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(ifindex, HSRV_PMID_OPENFLOW, (void**)&p_intf_data));
    if(NULL == p_intf_data)
    {
        return HSRV_E_NONE;
    }

    sal_fprintf(fp, "%7d %6s %12s %5s %10d %10d %11d %6d",
        ifindex,
        p_intf_data->enable ? "True" : "False",
        p_intf_data->ignore_vlan_filter ? "True" : "False",
        p_intf_data->link ? "Up": "Down",
        p_intf_data->nh_info.nh_offset.nhid,
        p_intf_data->nh_info.nh_offset.offset,
        p_intf_data->port_config,
        p_intf_data->l3ifid);
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &label));
    sal_fprintf(fp, " %5d", label);
    sal_fprintf(fp, "\n");
    #endif
    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_port(FILE *fp)
{
    sal_fprintf(fp, "-------------HSRV OPENFLOW PORT---------------------------\n");
    sal_fprintf(fp, "phy_port_label_base : %d\n", g_pst_openflow_port_master->phy_port_label_base);
    sal_fprintf(fp, "----------------------------------------------------------\n");

    sal_fprintf(fp, "%7s %6s %12s %5s %10s %10s %11s %6s %5s \n",
        "ifindex", "Enable", "Ingore-vlan", "link", "nhid", "nh-offset", "port-config", "l3ifid", "label");
    sal_fprintf(fp, "----------------------------------------------------------\n");

    hsrv_ifdb_do_listloop(GLB_IF_TYPE_PORT_IF, hsrv_openflow_show_port__, fp);

    sal_fprintf(fp, "----------------------------------------------------------\n");
    return HSRV_E_NONE;
}


#define _____INIT_____

int32
hsrv_openflow_port_start(void)
{
    g_pst_openflow_port_master = HSRV_OPENFLOW_PORT_CALLOC(sizeof(hsrv_openflow_port_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_port_master);
    
    g_pst_openflow_port_master->phy_port_label_base = HSRV_OPENFLOW_PHY_PORT_LABEL_BASE;

    /* Added by weizj for udf */
    HSRV_OPENFLOW_PORT_UDF_HASH = ctclib_hash_create(_hsrv_openflow_port_udf_info_hash_make,
        _hsrv_openflow_port_udf_info_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_PORT_UDF_HASH);

    /* Added by weizj for bug 36414 */
    HSRV_OPENFLOW_PORT_LABLE_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OPENFLOW_PORT_LABLE_OPF,
                            0,
                            1,
                            HSRV_OPENFLOW_PORT_LABEL_MAX);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_port_init(hsrv_system_spec_info_t* p_system_info)
{
    
    return HSRV_E_NONE;
}

#define _____END_LINE_____
