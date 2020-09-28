#include "hsrv_inc.h"
#include "hsrv_cpu_traffic.h"

extern hsrv_hostintf_master_t *hostif_master;

sai_hostif_trap_id_t
hsrv_cpu_traffic_system_to_sai_trap_id(glb_packet_tocpu_reason_t cpu_reason)
{
    switch(cpu_reason)
    {

        /*
        * switch trap 
        */
        case GLB_CPU_TRAFFIC_STP:
            return SAI_HOSTIF_TRAP_ID_STP;

        case GLB_CPU_TRAFFIC_LACP:
            return SAI_HOSTIF_TRAP_ID_LACP;

        case GLB_CPU_TRAFFIC_EAPOL:
            return SAI_HOSTIF_TRAP_ID_EAPOL;

        case GLB_CPU_TRAFFIC_LLDP:
            return SAI_HOSTIF_TRAP_ID_LLDP;

        case GLB_CPU_TRAFFIC_PVRST:
            return SAI_HOSTIF_TRAP_ID_PVRST;

        case GLB_CPU_TRAFFIC_IGMP_TYPE_QUERY:
            return SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY;

        case GLB_CPU_TRAFFIC_IGMP_TYPE_LEAVE:
            return SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE;

        case GLB_CPU_TRAFFIC_IGMP_TYPE_V1_REPORT:
            return SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT;

        case GLB_CPU_TRAFFIC_IGMP_TYPE_V2_REPORT:
            return SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT;

        case GLB_CPU_TRAFFIC_IGMP_TYPE_V3_REPORT:
            return SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT;

        case GLB_CPU_TRAFFIC_ERPS:
            return SAI_HOSTIF_TRAP_ID_ERPS;

        case GLB_CPU_TRAFFIC_IGMP:
            return SAI_HOSTIF_TRAP_ID_IGMP;

        /*
        * router trap 
        */
        case GLB_CPU_TRAFFIC_ARP_REQUEST:
            return SAI_HOSTIF_TRAP_ID_ARP_REQUEST;

        case GLB_CPU_TRAFFIC_ARP_RESPONSE:
            return SAI_HOSTIF_TRAP_ID_ARP_RESPONSE;

        case GLB_CPU_TRAFFIC_DHCP:
            return SAI_HOSTIF_TRAP_ID_DHCP;

        case GLB_CPU_TRAFFIC_OSPF:
            return SAI_HOSTIF_TRAP_ID_OSPF;

        case GLB_CPU_TRAFFIC_PIM:
            return SAI_HOSTIF_TRAP_ID_PIM;

        case GLB_CPU_TRAFFIC_VRRP:
            return SAI_HOSTIF_TRAP_ID_VRRP;

        case GLB_CPU_TRAFFIC_BGP:
            return SAI_HOSTIF_TRAP_ID_BGP;

        case GLB_CPU_TRAFFIC_DHCPV6:
            return SAI_HOSTIF_TRAP_ID_DHCPV6;

        case GLB_CPU_TRAFFIC_OSPFV6:
            return SAI_HOSTIF_TRAP_ID_OSPFV6;

        case GLB_CPU_TRAFFIC_VRRPV6:
            return SAI_HOSTIF_TRAP_ID_VRRPV6;

        case GLB_CPU_TRAFFIC_BGPV6:
            return SAI_HOSTIF_TRAP_ID_BGPV6;

        case GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY:
            return SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY;

        case GLB_CPU_TRAFFIC_IPV6_MLD_V1_V2:
            return SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_V2;

        case GLB_CPU_TRAFFIC_IPV6_MLD_V1_REPORT:
            return SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_REPORT;

        case GLB_CPU_TRAFFIC_IPV6_MLD_V1_DONE:
            return SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_DONE;

        case GLB_CPU_TRAFFIC_MLD_V2_REPORT:
            return SAI_HOSTIF_TRAP_ID_MLD_V2_REPORT;

        case GLB_CPU_TRAFFIC_ARP:
            return SAI_HOSTIF_TRAP_ID_ARP;
            
        /*for ptp add by chenc*/
        case GLB_CPU_TRAFFIC_PTP:
            return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP;
            
        /* 
        * pipeline exceptions   
        */
        case GLB_CPU_TRAFFIC_L3_MTU_ERROR:
            return SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR;

        case GLB_CPU_TRAFFIC_TTL_ERROR:
            return SAI_HOSTIF_TRAP_ID_TTL_ERROR;

        case GLB_CPU_TRAFFIC_COPY_TO_CPU:
        case GLB_CPU_TRAFFIC_TELNET:
        case GLB_CPU_TRAFFIC_SSH:
        case GLB_CPU_TRAFFIC_MLAG:
            return SAI_HOSTIF_TRAP_ID_FWD_TO_CPU;

        case GLB_CPU_TRAFFIC_PACKET_IN:
            return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER;

        case GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC:
            return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_PROTOCOL_MAC;

        case GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC:
            return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_GROUP_MAC;
            
        case GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR:
            return SAI_HOSTIF_TRAP_ID_CPU_MIRROR;

        default :
            return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_RANGE_BASE;
    }

     return SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_RANGE_BASE;
}

int32
hsrv_cpu_traffic_set_field(tbl_cpu_traffic_t* p_cpu_traffic)
{
    sai_int32_t reason_id = 0;

    sai_policer_api_t*  policer_api;
    sai_attribute_t attr;

    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    if (NULL == p_cpu_traffic)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    reason_id = p_cpu_traffic->key.reason_id;
    if (p_cpu_traffic->key.reason_id >= GLB_CPU_TRAFFIC_MAX)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /* Added by kcao 2018-11-13 for bug 49616, bypass uninited oid, GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY(26) only have valid oid in IPv6 profile */
    if (0 == hostif_master->reason_info[reason_id].trap_group_policer_oid)
    {
        return HSRV_E_NONE;
    }
    
    attr.id = SAI_POLICER_ATTR_CIR;
    attr.value.u64 = p_cpu_traffic->rate*1000/8;
    
    HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(hostif_master->reason_info[reason_id].trap_group_policer_oid, &attr));
#ifndef GREATBELT
    attr.id = SAI_POLICER_ATTR_CBS;
    attr.value.u64 = p_cpu_traffic->rate*2*1000/8;
    
    HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(hostif_master->reason_info[reason_id].trap_group_policer_oid, &attr));
#endif

    return HSRV_E_NONE;
}

int32
hsrv_cpu_traffic_group_set_field(tbl_cpu_traffic_group_t* p_cpu_traffic_group)
{ 
    sai_scheduler_api_t*  scheduler_api;
    sai_attribute_t attr;

    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));

    if (NULL == p_cpu_traffic_group)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr.value.u64 = p_cpu_traffic_group->rate*1000/8;
    HSRV_IF_ERROR_RETURN(scheduler_api->set_scheduler_attribute(hostif_master->scheduler_id[p_cpu_traffic_group->key.class_id], &attr));
    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    attr.id = SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = hostif_master->scheduler_id[p_cpu_traffic_group->key.class_id];
    HSRV_IF_ERROR_RETURN(hostif_master->copp_api->set_copp_queue_attribute(hostif_master->group_queue[p_cpu_traffic_group->key.class_id], &attr));

    return HSRV_E_NONE;
}

int32
hsrv_cpu_traffic_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_cpu_traffic_t* p_cpu_traffic = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_cpu_traffic = (tbl_cpu_traffic_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_cpu_traffic_set_field(p_cpu_traffic));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_cpu_traffic_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_cpu_traffic_group_t* p_cpu_traffic_group = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_cpu_traffic_group = (tbl_cpu_traffic_group_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_cpu_traffic_group_set_field(p_cpu_traffic_group));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

