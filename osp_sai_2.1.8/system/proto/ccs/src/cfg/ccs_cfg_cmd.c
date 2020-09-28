
#include "proto.h"
#include "ccs_cfg.h"
#include "gen/tbl_cpu_mirror_global_define.h"
#include "gen/tbl_cpu_mirror_global.h"

#if defined TAPPRODUCT

/* TAP product */
uint32 g_ccs_cfg_sort_array[] = 
{
    TBL_CLOCK,
    TBL_SYS_GLOBAL,
    TBL_FEA_TIME,
    TBL_ENABLE,
    TBL_AUTHEN,
    TBL_AUTHOR,
    TBL_ACCOUNT,
    TBL_ACCOUNTCMD,
    TBL_RSA,
    TBL_USER,
    TBL_LOGIN_SECURITY_GLB,
    TBL_CARD,
    TBL_PORT,
    TBL_NS_PORT_FORWARDING,    
    TBL_SSH_CFG,
    TBL_SYSLOG_CFG,
    TBL_AUTH_CFG,
    TBL_AUTH_SERVER,
    TBL_NTP_CFG,
    TBL_NTP_KEY,
    TBL_NTP_SERVER,
    TBL_NTP_ACE,
    TBL_SNMP_CFG,
    TBL_SNMP_VIEW,
    TBL_SNMP_COMMUNITY,
    TBL_SNMP_CONTEXT,
    TBL_SNMP_USM_USER,
    TBL_SNMP_GROUP_USM,
    TBL_SNMP_ACCESS_USM,
    TBL_SNMP_NOTIFY,
    TBL_SNMP_TARGET_PARAMS,
    TBL_SNMP_TARGET_ADDR,
    TBL_SNMP_TRAP,
    TBL_SNMP_INFORM,
    TBL_STATIC_DNS,
    TBL_DYNAMIC_DNS_DOMAIN,
    TBL_DYNAMIC_DNS_SERVER,
    TBL_MANAGE_IF,
    TBL_LOG_GLOBAL,
    TBL_LAG_GLOBAL, 
    TBL_ACL_UDF_ENTRY,
    TBL_ACL_CONFIG,
    TBL_CEM,
    TBL_DOT1X_GLOBAL,
    TBL_DOT1X_RADIUS,
#ifdef CONFIG_SFLOW
    TBL_SFLOW_GLOBAL,
    TBL_SFLOW_COLLECTOR,
#endif
    TBL_HASH_FIELD_PROFILE,
    TBL_HASH_VALUE_PROFILE,
    TBL_HASH_VALUE_GLOBAL,
    TBL_VRRP_GLOBAL,
    TBL_CFM_GLOBAL,
    TBL_CFM_MD,
    TBL_CFM_MA,
    TBL_CFM_LMEP,
    TBL_INTERFACE,
    TBL_IPSLA_ENTRY,
    TBL_TRACK_OBJECT,
    TBL_VRRP_SESSION,
    TBL_TAP_GROUP,
    TBL_TAP_DPI_GLOBAL,
    TBL_CONSOLE,
    TBL_VTY
};

#else

/* L2+ product */
uint32 g_ccs_cfg_sort_array[] = 
{
    TBL_CLOCK,
    TBL_TAG_SUMMER_CLOCK,
    TBL_SYS_GLOBAL,
    TBL_MEM_SUMMARY,
    TBL_ENABLE,
    TBL_AUTHEN,
    TBL_AUTHOR,
    TBL_ACCOUNT,
    TBL_ACCOUNTCMD,
    TBL_RSA,
    TBL_USER,
    TBL_LOGIN_SECURITY_GLB,
    TBL_CARD,
    TBL_PORT,
    TBL_NS_PORT_FORWARDING,    
    TBL_SSH_CFG,
    TBL_SYSLOG_CFG,
    TBL_AUTH_CFG,
    TBL_AUTH_SERVER,    
    TBL_NTP_CFG,
    TBL_NTP_KEY,
    TBL_NTP_SERVER,
    TBL_NTP_ACE,
    TBL_SNMP_CFG,
    TBL_SNMP_VIEW,
    TBL_SNMP_COMMUNITY,
    TBL_SNMP_CONTEXT,
    TBL_SNMP_USM_USER,
    TBL_SNMP_GROUP_USM,
    TBL_SNMP_ACCESS_USM,
    TBL_SNMP_NOTIFY,
    TBL_SNMP_TARGET_PARAMS,
    TBL_SNMP_TARGET_ADDR,
    TBL_SNMP_TRAP,
    TBL_SNMP_INFORM,
    TBL_STATIC_DNS,
    TBL_DYNAMIC_DNS_DOMAIN,
    TBL_DYNAMIC_DNS_SERVER,
    TBL_CPU_LIMIT,
    TBL_BRG_GLOBAL,
    TBL_ROUTE_GLOBAL,
    TBL_IPTABLES_PREVENT,
    TBL_ACL_WORM_FILTER,
    TBL_MANAGE_IF,
    TBL_LAG_GLOBAL,
    TBL_VLAN,
    TBL_TIME_RANGE,
    TBL_ACL_UDF_ENTRY,
    TBL_ACL_CONFIG,
    TBL_LOG_GLOBAL,
    TBL_BHM_GLOBAL,
    TBL_MSTP_GLOBAL,
    TBL_LLDP_GLOBAL,
    TBL_QOS_GLOBAL,
    TBL_QOS_MPLS_DOMAIN,
    TBL_QOS_DOMAIN,
    TBL_QOS_DROP_PROFILE,
    TBL_QOS_POLICER_ACTION_PROFILE,
    TBL_QOS_POLICER_PROFILE,
    TBL_QOS_QUEUE_SHAPE_PROFILE,    
    TBL_QOS_PORT_SHAPE_PROFILE,
    TBL_QOS_QUEUE_SMART_BUFFER,
    TBL_CLASS_MAP_CONFIG,
    TBL_POLICY_MAP_CONFIG,
    TBL_COPP_CFG,
    TBL_CPU_TRAFFIC,
    TBL_CPU_TRAFFIC_GROUP,
    TBL_ERRDISABLE,
    TBL_VLANCLASS_RULE,
    TBL_VLANCLASS_GROUP,
    TBL_L2_PROTOCOL,
    TBL_EVC_ENTRY,
    TBL_VLAN_GROUP,
    TBL_VLAN_MAPPING_TABLE,
    TBL_OPENFLOW, 
#ifdef OFDPAPRODUCT
    TBL_CONTROLLER, 
#endif
    TBL_CEM,
    TBL_DOT1X_GLOBAL,
    TBL_DOT1X_RADIUS,
#ifdef CONFIG_SFLOW
    TBL_SFLOW_GLOBAL,
    TBL_SFLOW_COLLECTOR,
#endif
    TBL_SSM_GLOBAL,
    TBL_PTP_GLOBAL,
    TBL_HASH_FIELD_PROFILE,
    TBL_HASH_VALUE_PROFILE,
    TBL_HASH_VALUE_GLOBAL,
    TBL_TPOAM_GLOBAL,
    TBL_VRRP_GLOBAL,
    TBL_CFM_GLOBAL,
    TBL_CFM_MD,
    TBL_CFM_MA,
    TBL_CFM_LMEP,
    TBL_INTERFACE,
    TBL_NS_ROUTE,
    TBL_IGSP_GLOBAL,
    TBL_MLAG,
    TBL_MACFILTER,
    TBL_FDB,
    TBL_IPSG_FIB,
    TBL_IPSG_S_MAC,
    TBL_MCFDB,
    TBL_ECMP_GLOBAL,
    TBL_RR_PREFIX,
    TBL_ARP_FIB,    
    TBL_ARP,
    TBL_ARPACL_CONFIG,
    TBL_ARPACE_CONFIG,
    TBL_ARPINSP,
    TBL_ND_FIB,
	TBL_BGP,
    TBL_BGP_NETWORK,
    TBL_BGP_NEIGHBOR,
	TBL_RIP,
    TBL_RIP_NETWORK,
    TBL_RIP_NEIGHBOR,
    TBL_RIP_PASSIVE_IF,
    TBL_RIPNG,
    TBL_RIPNG_NETWORK,
    TBL_RIPNG_PASSIVE_IF,
    TBL_OSPF,
    TBL_OSPF_NETWORK,
    TBL_OSPF_AREA_AUTH,
    TBL_OSPF_AREA_RANGE,
	TBL_OSPF_AREA_STUB,
    TBL_OSPF_AREA_NSSA,
    TBL_OSPF_AREA_VLINK,
    /*add by zhw for ospf6*/
	TBL_OSPF6,
	TBL_OSPF6_INTERFACE,
	TBL_OSPF6_AREA_RANGE,
	TBL_OSPF6_AREA_STUB,
	/*end add*/
    TBL_IPROUTE_NODE,
    TBL_IPSLA_ENTRY,
    TBL_TRACK_OBJECT,
    TBL_STATIC_ROUTE_CFG,
    TBL_IPMC_GLOBAL,
    TBL_PIM_STATIC_RP,
    /*add by zhw for ipmc*/
    TBL_PIM_SM,
	TBL_PIM_SM_BSR,
	TBL_PIM_SM_RP,
	TBL_PIM_DM,	
	TBL_IGMP_INTF,
	TBL_MRT,
	/*end add*/
    TBL_CPU_MIRROR_GLOBAL,
    TBL_MIRROR,
    TBL_MIRROR_MAC_ESCAPE,
    TBL_LSP_APS_GROUP,
    TBL_PW_APS_GROUP,
    TBL_APS_GROUP_INFO,
    TBL_TAP_GROUP,
    TBL_TAP_DPI_GLOBAL,
    TBL_DHCRELAY,
    TBL_DHCSRVGRP,
    TBL_DHCSNOOPING,
    TBL_DHCLIENT,
    TBL_RMON_EVENT,
    TBL_RMON_ALARM,
    TBL_OPM_GLOBAL,
    TBL_ERPS_DOMAIN,
    TBL_OAM_SESSION,
    TBL_VRRP_SESSION,
    TBL_NAT_GLOBAL,
    TBL_NAT_SESSION_LIMIT,
    TBL_G8032_RING,
    TBL_NAT_POOL,
    TBL_NAT_RULE,
    TBL_CONSOLE,
    TBL_VTY,
};

#endif

static int32
_ccs_cfg_build_cfg_cdb_action(FILE *fp, cdb_node_t *p_tbl_node, char *key, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (NULL == key)
    {
        return rc;
    }
    
    if (0 == sal_strcmp(key, "dot1x"))
    {
        rc = ccs_cfg_build_dot1x_global(fp, para);
    }
    else if (0 == sal_strcmp(key, "stp"))
    {
        rc = ccs_cfg_build_mstp_global(fp, para);
    }
    else if (0 == sal_strcmp(key, "vlan"))
    {
        rc = ccs_cfg_build_vlan(fp, para);
    }
    else if (0 == sal_strcmp(key, "route"))
    {
        rc += ccs_cfg_build_route_global(fp, para, FALSE, FALSE);
        rc += ccs_cfg_build_rt_static_cfg(fp, para, 1);
    }
    else if (0 == sal_strcmp(key, "routev6"))
    {
        //rc += ccs_cfg_build_route_global(fp, para, FALSE);
        rc += ccs_cfg_build_rt_static_cfg(fp, para, 2);
    }
    else if (0 == sal_strcmp(key, "igmp-snooping"))
    {
        rc = ccs_cfg_build_igmp_snooping(fp, para);
    }
    else if (0 == sal_strcmp(key, "radius-server"))
    {
        rc = ccs_cfg_build_auth_server(fp, para);
    }
    else if (0 == sal_strcmp(key, "tacacs-server"))
    {
        rc = ccs_cfg_build_tacacs_server(fp, para);
    }
    else if (0 == sal_strcmp(key, "ospf"))
    {
        rc += ccs_cfg_build_router_ospf(fp, para);
        rc += ccs_cfg_build_router_ospf_network(fp, para);
        rc += ccs_cfg_build_router_ospf_area_auth(fp, para);
        rc += ccs_cfg_build_router_ospf_area_range(fp, para);
		rc += ccs_cfg_build_router_ospf_area_stub(fp, para);
		rc += ccs_cfg_build_router_ospf_area_nssa(fp, para);
		rc += ccs_cfg_build_router_ospf_area_vlink(fp, para);
    }
	else if (0 == sal_strcmp(key, "ospf6"))
	{
		rc += ccs_cfg_build_router_ospf6(fp, para);
	}
	else if (0 == sal_strcmp(key, "bgp"))
    {
        rc = ccs_cfg_build_router_bgp(fp, para);
        rc = ccs_cfg_build_router_bgp_network(fp, para);
        rc = ccs_cfg_build_router_bgp_neighbor(fp, para);
    }
	else if (0 == sal_strcmp(key, "rip"))
    {
        rc = ccs_cfg_build_router_rip(fp, para);
        rc = ccs_cfg_build_router_rip_network(fp, para);
        rc = ccs_cfg_build_router_rip_neighbor(fp, para);
		rc = ccs_cfg_build_router_rip_passive_if(fp, para);
    }
	else if (0 == sal_strcmp(key, "ripng"))
    {
        rc = ccs_cfg_build_router_ripng(fp, para);
        rc = ccs_cfg_build_router_ripng_network(fp, para);
		rc = ccs_cfg_build_router_rip_passive_if(fp, para);
    }
    else if (0 == sal_strcmp(key, "vrrp"))
    {
        rc = ccs_cfg_build_vrrp_session(fp, para);
    }
/*Add by zhush for bug 52970, 2019-07-26*/
    else if (0 == sal_strcmp(key, "nat"))
    {
        rc += ccs_cfg_build_nat_pool(fp, para);
        rc += ccs_cfg_build_nat_rule(fp, para);
        rc += ccs_cfg_build_nat_session_limit(fp, para);
        rc += ccs_cfg_build_nat_global(fp, para);
    }
/*end zhush*/
	
	/*add by zhw for ipmc*/
	else if (0 == sal_strcmp(key, "pim_sm"))
    {
        rc = ccs_cfg_build_router_pim_sm(fp, para);
		rc = ccs_cfg_build_router_pim_sm_bsr(fp, para);
		rc = ccs_cfg_build_router_pim_sm_rp(fp, para);
    }
	else if (0 == sal_strcmp(key, "pim_dm"))
    {
        rc = ccs_cfg_build_router_pim_dm(fp, para);
    }
	/*end add zhw*/
    return rc;    
}

static int32
_ccs_cfg_build_cfg_cdb_dispatch(FILE *fp, cdb_node_t *p_tbl_node, char *key, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (p_tbl_node->id)
    {
    case TBL_SYS_GLOBAL:
        ccs_cfg_build_sys_global(fp, para);
        break;

    case TBL_MEM_SUMMARY:
        ccs_cfg_build_mem_summary(fp, para);
        break;

    case TBL_ROUTE_GLOBAL:
        rc = ccs_cfg_build_route_global(fp, para, TRUE, TRUE);
        break;
    
    case TBL_IPTABLES_PREVENT:
        ccs_cfg_build_app_iptables_prevent(fp, para);
        break;
    
    case TBL_ACL_WORM_FILTER:
        ccs_cfg_build_app_worm_filter(fp, para);
        break;

    case TBL_CPU_MIRROR_GLOBAL:
        ccs_cfg_build_cpu_mirror_global(fp, para);
        break;

    case TBL_MIRROR:
        ccs_cfg_build_sys_mirror(fp, para);
        break;

    case TBL_MIRROR_MAC_ESCAPE:
        ccs_cfg_build_sys_mirror_mac_escape(fp, para);
        break;

    case TBL_QOS_GLOBAL:
        ccs_cfg_build_qos_global(fp, para);
        break;
    case TBL_COPP_CFG:
        ccs_cfg_build_copp_cfg(fp, para);
        break;
    case TBL_CPU_TRAFFIC:
        ccs_cfg_build_cpu_traffic(fp, para);
        break;
    case TBL_CPU_TRAFFIC_GROUP:
        ccs_cfg_build_cpu_traffic_group(fp, para);
        break;

    case TBL_QOS_MPLS_DOMAIN:
        ccs_cfg_build_qos_mpls_domain(fp, para);
        break;

    case TBL_QOS_DOMAIN:
        ccs_cfg_build_qos_domain(fp, para);
        break;

    case TBL_QOS_POLICER_ACTION_PROFILE:
        ccs_cfg_build_qos_policer_action_profile(fp, para);
        break;
        
    case TBL_QOS_POLICER_PROFILE:
        ccs_cfg_build_qos_policer_profile(fp, para);
        break;

    case TBL_QOS_DROP_PROFILE:
        ccs_cfg_build_qos_drop_profile(fp, para);
        break;

    case TBL_QOS_QUEUE_SHAPE_PROFILE:
        ccs_cfg_build_qos_scheduler(fp, para);
        break;

    case TBL_QOS_PORT_SHAPE_PROFILE:
        ccs_cfg_build_qos_port_shape_profile(fp, para);
        break;
        
    case TBL_QOS_QUEUE_SMART_BUFFER:
        ccs_cfg_build_qos_queue_smart_buffer(fp, para);
        break;

    case TBL_MANAGE_IF:
        ccs_cfg_build_sys_manage_if(fp, para);
        break;

    case TBL_MSTP_GLOBAL:
        ccs_cfg_build_mstp_global(fp, para);
        break;

    case TBL_ARP_FIB:
        break;
        
    case TBL_ARP:
        ccs_cfg_build_arp(fp, para);
        break;
	
	case TBL_BGP:
        ccs_cfg_build_router_bgp(fp, para);
        break;
    
    case TBL_BGP_NETWORK:
        ccs_cfg_build_router_bgp_network(fp, para);
        break;
    
    case TBL_BGP_NEIGHBOR:
        ccs_cfg_build_router_bgp_neighbor(fp, para);
        break;

	case TBL_RIP:
		ccs_cfg_build_router_rip(fp, para);
		break;

	case TBL_RIP_NETWORK:
		ccs_cfg_build_router_rip_network(fp, para);
		break;

	case TBL_RIP_NEIGHBOR:
		ccs_cfg_build_router_rip_neighbor(fp, para);
		break;
		
	case TBL_RIP_PASSIVE_IF:
		ccs_cfg_build_router_rip_passive_if(fp, para);
		break;
	
	case TBL_RIPNG:
		ccs_cfg_build_router_ripng(fp, para);
		break;

	case TBL_RIPNG_NETWORK:
		ccs_cfg_build_router_ripng_network(fp, para);
		break;

	case TBL_RIPNG_PASSIVE_IF:
		ccs_cfg_build_router_ripng_passive_if(fp, para);
		break;

    case TBL_OSPF:
        ccs_cfg_build_router_ospf(fp, para);
        break;

    case TBL_OSPF_NETWORK:
        ccs_cfg_build_router_ospf_network(fp, para);
        break;

    case TBL_OSPF_AREA_AUTH:
        ccs_cfg_build_router_ospf_area_auth(fp, para);
        break;

    case TBL_OSPF_AREA_RANGE:
        ccs_cfg_build_router_ospf_area_range(fp, para);
        break;
        
	case TBL_OSPF_AREA_STUB:
        ccs_cfg_build_router_ospf_area_stub(fp, para);
        break;
            
    case TBL_OSPF_AREA_NSSA:
        ccs_cfg_build_router_ospf_area_nssa(fp, para);
        break;
            
    case TBL_OSPF_AREA_VLINK:
        ccs_cfg_build_router_ospf_area_vlink(fp, para);
        break;

	case TBL_OSPF6:
        ccs_cfg_build_router_ospf6(fp, para);
        break;

	case TBL_OSPF6_INTERFACE:
        ccs_cfg_build_router_ospf6_interface(fp, para);
        break;

	case TBL_OSPF6_AREA_RANGE:
        ccs_cfg_build_router_ospf6_area_range(fp, para);
        break;

	case TBL_OSPF6_AREA_STUB:
        ccs_cfg_build_router_ospf6_area_stub(fp, para);
        break;
	
    case TBL_STATIC_ROUTE_CFG:
        ccs_cfg_build_rt_static_cfg(fp, para, 0);
        break;

    case TBL_INTERFACE:
        rc = ccs_cfg_build_interface(fp, key, para);
        break;

    case TBL_VLAN:
        rc = ccs_cfg_build_vlan(fp, para);
        break;

    case TBL_MLAG:
        ccs_cfg_build_mlag(fp, para);
        break;
        
    case TBL_IPSG_FIB:
        //ccs_cfg_build_ipsg(fp, para);
        break;

    case TBL_IPSG_S_MAC:
        ccs_cfg_build_ipsg_mac(fp, para);
        break;

    case TBL_IGSP_GLOBAL:
        ccs_cfg_build_igmp_snooping(fp, para);
        break;

    case TBL_LLDP_GLOBAL:
        ccs_cfg_build_lldp_global(fp, para);
        break;

    case TBL_FDB:
        ccs_cfg_build_fdb(fp, para);
        break;

    case TBL_MCFDB:
        ccs_cfg_build_l2mc_fdb(fp, para);
        break;

    case TBL_MACFILTER:
        ccs_cfg_build_macfilter(fp, para);
        break;

    case TBL_BRG_GLOBAL:
        ccs_cfg_build_brg_global(fp, para);
        break;

    case TBL_LAG_GLOBAL:
        ccs_cfg_build_lag_global(fp, para);
        break;

    case TBL_VLANCLASS_RULE:
        ccs_cfg_build_vlanclass_rule(fp, para);
        break;
        
    case TBL_VLANCLASS_GROUP:
        ccs_cfg_build_vlanclass_group(fp, para);
        break;

    case TBL_L2_PROTOCOL:
        ccs_cfg_build_l2_protocol(fp, para);
        break;

    case TBL_EVC_ENTRY:
        ccs_cfg_build_evc_entry(fp, para);
        break;
        
    case TBL_VLAN_GROUP:
        ccs_cfg_build_vlan_group(fp, para);
        break;

    case TBL_VLAN_MAPPING_TABLE:
        ccs_cfg_build_vlan_mapping_table(fp, para);
        break;

    case TBL_ACL_CONFIG:
        ccs_cfg_build_acl(fp, key, para);
        break;

    case TBL_ACL_UDF_ENTRY:
        ccs_cfg_build_acl_udf(fp, key, para);
        break;

    case TBL_CLASS_MAP_CONFIG:
        ccs_cfg_build_class_map(fp, key, para);
        break;
        
    case TBL_POLICY_MAP_CONFIG:
        ccs_cfg_build_policy_map(fp, key, para);
        break;
        
    case TBL_ARPACL_CONFIG:
        ccs_cfg_build_arpacl(fp, para);
        break;
        
    case TBL_ARPINSP:
        ccs_cfg_build_arpinsp(fp, para);
        break;
#ifdef TAPPRODUCT
    case TBL_TAP_GROUP:
        ccs_cfg_build_tap(fp, key, para);
        break;

    case TBL_TAP_DPI_GLOBAL:
        ccs_cfg_build_tapdpi(fp, key, para);
        break;
#endif
    case TBL_TIME_RANGE:
        ccs_cfg_build_time_range(fp, para);
        break;

    case TBL_SSH_CFG:
        ccs_cfg_build_app_ssh_cfg(fp, para);
        break;

    case TBL_SYSLOG_CFG:
        ccs_cfg_build_app_syslog_cfg(fp, para);
        break;

    case TBL_CLOCK:
        ccs_cfg_build_app_clock(fp, para);
        break;
    case TBL_TAG_SUMMER_CLOCK:
        ccs_cfg_build_app_summer_clock(fp, para);
        break;

    case TBL_AUTH_CFG:
        ccs_cfg_build_auth_cfg(fp, para);
        break;
        
    case TBL_AUTH_SERVER:
        ccs_cfg_build_auth_server(fp, para);
        ccs_cfg_build_tacacs_server(fp, para);
        break;
        
    case TBL_RSA:
        ccs_cfg_build_app_rsa(fp, para);
        break;

    case TBL_USER:
        ccs_cfg_build_app_user(fp, para);
        break;
        
    case TBL_LOGIN_SECURITY_GLB:
        ccs_cfg_build_app_login_security(fp, para);
        break;

    case TBL_AUTHEN:
        ccs_cfg_build_app_authen(fp, para);
        break;

    case TBL_AUTHOR:
        ccs_cfg_build_app_author(fp, para);
        break;

    case TBL_ACCOUNT:
        ccs_cfg_build_app_account(fp, para);
        break;

    case TBL_ACCOUNTCMD:
        ccs_cfg_build_app_accountcmd(fp, para);
        break;

    case TBL_DHCRELAY:
        ccs_cfg_build_dhcrelay(fp, para);
        break;

    case TBL_DHCSNOOPING:
        ccs_cfg_build_dhcsnooping(fp, para);
        break;
                
    case TBL_DHCSRVGRP:
        ccs_cfg_build_dhcsrvgrp(fp, para);
        break;

    case TBL_DHCLIENT:
        ccs_cfg_build_dhclient(fp, para);
        break;
    case TBL_G8032_RING:
        ccs_cfg_build_g8032_ring( fp, para);
        break;
    case TBL_CONSOLE:
        ccs_cfg_build_app_console(fp, para);
        break;

    case TBL_VTY:
        ccs_cfg_build_app_vty(fp, para);
        break;

    case TBL_SNMP_CFG:
        ccs_cfg_build_app_snmp_cfg(fp, para);
        break;

    case TBL_SNMP_VIEW:
        ccs_cfg_build_app_snmp_view(fp, para);
        break;

    case TBL_SNMP_COMMUNITY:
        ccs_cfg_build_app_snmp_community(fp, para);
        break;

    case TBL_SNMP_CONTEXT:
        ccs_cfg_build_app_snmp_context(fp, para);
        break;
        
    case TBL_SNMP_USM_USER:
        ccs_cfg_build_app_snmp_usm_user(fp, para);
        break;
        
    case TBL_SNMP_GROUP_USM:
        ccs_cfg_build_app_snmp_group_usm(fp, para);
        break;

    case TBL_SNMP_ACCESS_USM:
        ccs_cfg_build_app_snmp_access_usm(fp, para);
        break;
    case TBL_SNMP_NOTIFY:
        ccs_cfg_build_app_snmp_notify(fp, para);
        break;    
    case TBL_SNMP_TARGET_PARAMS:
        ccs_cfg_build_app_snmp_target_params(fp, para);
        break;
    case TBL_SNMP_TARGET_ADDR:
        ccs_cfg_build_app_snmp_target_addr(fp, para);
        break;
    case TBL_SNMP_TRAP:
        ccs_cfg_build_app_snmp_trap(fp, para);
        break; 

    case TBL_SNMP_INFORM:
        ccs_cfg_build_app_snmp_inform(fp, para);
        break;

    case TBL_RMON_EVENT:
        ccs_cfg_build_app_rmon_event(fp, para);
        break;

    case TBL_RMON_ALARM:
        ccs_cfg_build_app_rmon_alarm(fp, para);
        break;

    case TBL_NTP_CFG:
        ccs_cfg_build_app_ntp_cfg(fp, para);
        break;  

    case TBL_NTP_KEY:
        ccs_cfg_build_app_ntp_key(fp, para);
        break;
        
    case TBL_NTP_SERVER:
        ccs_cfg_build_app_ntp_server(fp, para);
        break;  

    case TBL_NTP_ACE:
        ccs_cfg_build_app_ntp_ace(fp, para);
        break;  

    case TBL_STATIC_DNS:
        ccs_cfg_build_app_static_dns(fp, para);
        break;
        
    case TBL_DYNAMIC_DNS_DOMAIN:
        ccs_cfg_build_app_dynamic_dns_domain(fp, para);
        break;
        
    case TBL_DYNAMIC_DNS_SERVER:
        ccs_cfg_build_app_dynamic_dns_server(fp, para);
        break;

    case TBL_CPU_LIMIT:
        ccs_cfg_build_app_cpu_limit(fp, para);
        break;

    case TBL_CARD:
        ccs_cfg_build_sys_card(fp, para);
        break;

    case TBL_PORT:
        ccs_cfg_build_sys_port(fp, para);
        break;

    case TBL_ERRDISABLE:
        ccs_cfg_build_sys_errdisable(fp, para);
        break;

    case TBL_NS_PORT_FORWARDING:
        ccs_cfg_build_sys_ns_port_forwarding(fp, para);
        break;

    case TBL_NS_ROUTE:
        ccs_cfg_build_sys_ns_route(fp, para);
        break;

    case TBL_LOG_GLOBAL:
        ccs_cfg_build_log_global(fp, para);
        break;

    case TBL_CEM:
        ccs_cfg_build_app_cem(fp, para);
        break; 

/* Modifed by weizj for tunnel: show running */
#ifdef OFPRODUCT
    case TBL_OPENFLOW:
        ccs_cfg_build_openflow(fp, para);
        break;
#endif

#ifdef OFDPAPRODUCT
    case TBL_CONTROLLER:
        ccs_cfg_build_controller(fp, para);
        break;
#endif

    case TBL_OPM_GLOBAL: 
        ccs_cfg_build_opm_global(fp, para);
        break;

    case TBL_ERPS_DOMAIN:
        ccs_cfg_build_erps_domain(fp, para);
        break;

    case TBL_DOT1X_GLOBAL:
        ccs_cfg_build_dot1x_global(fp, para);
        break;

    case TBL_DOT1X_RADIUS:
        ccs_cfg_build_dot1x_radius(fp, para);
        break;
    case TBL_ENABLE:
        ccs_cfg_build_sys_enable(fp, para);
        break;

    case TBL_FEA_TIME:
        ccs_cfg_fea_time(fp, para);
        break;

    case TBL_BHM_GLOBAL:
        ccs_cfg_bhm_global(fp, para);
        break;

    case TBL_SFLOW_GLOBAL:
        ccs_cfg_build_sflow_global(fp, para);
        break;
        
    case TBL_SFLOW_COLLECTOR:
        ccs_cfg_build_sflow_collector(fp, para);
        break;

    case TBL_SSM_GLOBAL:
        ccs_cfg_build_ssm_global(fp, para);
        break;

    case TBL_VRRP_GLOBAL:
        ccs_cfg_build_vrrp_global(fp, para);
        break;
        
    case TBL_VRRP_SESSION:
        ccs_cfg_build_vrrp_session(fp, para);
        break;
        
    case TBL_PTP_GLOBAL:
        ccs_cfg_build_ptp_global(fp, para);
        break;

    case TBL_IPSLA_ENTRY:
        ccs_cfg_build_ipsla_entry(fp, para);
        break;

    case TBL_TRACK_OBJECT:
        ccs_cfg_build_track_object(fp, para);
        break;

    case TBL_ECMP_GLOBAL:
        ccs_cfg_build_ecmp_global(fp, para);
        break;

    case TBL_RR_PREFIX:
        ccs_cfg_build_rr_prefix(fp, para);
        break;

    case TBL_IPMC_GLOBAL:
        ccs_cfg_build_ipmc_global(fp, para);
        break;

    case TBL_PIM_STATIC_RP:
        ccs_cfg_build_pim_static_rp(fp, para);
        break;

    case TBL_LSP_APS_GROUP:
        ccs_cfg_build_g8131_lsp_aps_group(fp, para);
        break;
    case TBL_PW_APS_GROUP:
        ccs_cfg_build_g8131_pw_aps_group(fp, para);
        break;

    case TBL_OAM_SESSION:
        ccs_cfg_build_tpoam_oam_session(fp, key, para);
        break;

    case TBL_TPOAM_GLOBAL:
        ccs_cfg_build_tpoam_global(fp, para);
        break;

    case TBL_ND_FIB:
        ccs_cfg_build_nd_fib(fp, para);
        break;
        
#ifdef TSINGMA
    case TBL_HASH_FIELD_PROFILE:
        ccs_cfg_build_hash_field_profile(fp, para);
        break;
    case TBL_HASH_VALUE_PROFILE:
        ccs_cfg_build_hash_value_profile(fp, para);
        break;
    case TBL_HASH_VALUE_GLOBAL:
        ccs_cfg_build_hash_value_global(fp, para);
        break;
#endif

    case TBL_NAT_POOL:
        ccs_cfg_build_nat_pool(fp, para);
        break;

    case TBL_NAT_RULE:
        ccs_cfg_build_nat_rule(fp, para);
        break;

    case TBL_NAT_SESSION_LIMIT:
        ccs_cfg_build_nat_session_limit(fp, para);
        break;

    case TBL_NAT_GLOBAL:
        ccs_cfg_build_nat_global(fp, para);
        break;

    case TBL_CFM_GLOBAL:
        ccs_cfg_build_cfm_global(fp, para);
        break;

    case TBL_CFM_MD:
        ccs_cfg_build_cfm_md(fp, para);
        break;

    case TBL_CFM_MA:
        ccs_cfg_build_cfm_ma(fp, para);
        break;
	/*add by zhw for show ipmc*/
	case TBL_MRT:
   		ccs_cfg_build_mrt(fp,para);
		break;
	
	case TBL_PIM_SM:
		ccs_cfg_build_router_pim_sm(fp, para);
		break;

	case TBL_PIM_DM:
		ccs_cfg_build_router_pim_dm(fp, para);
		break;
		
	/*add by zhw for igmp_Intf*/
   case TBL_IGMP_INTF:
		ccs_cfg_build_igmp_intf(fp,para);
		break;
	/*zhw end add*/
		
    default:
        //TODO_ASSERT(0);
        break;
    }

    return rc;
}

int32
ccs_cfg_build_cfg_cdb(FILE *fp, cfg_cmd_para_t *para)
{
    cdb_node_t *p_tbl_node = NULL;
    uint32 array_size = 0;
    uint32 i = 0;
    uint32 tbl_id = 0;

    array_size = sizeof(g_ccs_cfg_sort_array)/sizeof(g_ccs_cfg_sort_array[0]);
    for (i = 0; i < array_size; i++)
    {
        tbl_id = g_ccs_cfg_sort_array[i];
        p_tbl_node = cdb_get_tbl(tbl_id);

        _ccs_cfg_build_cfg_cdb_dispatch(fp, p_tbl_node, NULL, para);
    }
        
    return PM_E_NONE;
}

int32
ccs_cfg_build_cfg_qos_cdb(FILE *fp, cfg_cmd_para_t *para)
{
    cdb_node_t *p_tbl_node = NULL;
    uint32 i = 0;
    uint32 rc = PM_E_NONE;
    uint32 array[8] = {TBL_QOS_GLOBAL,    
    TBL_QOS_MPLS_DOMAIN, 
    TBL_QOS_DOMAIN,
    TBL_QOS_DROP_PROFILE,
    TBL_QOS_POLICER_ACTION_PROFILE,
    TBL_QOS_POLICER_PROFILE,
    TBL_QOS_QUEUE_SHAPE_PROFILE,    
    TBL_QOS_PORT_SHAPE_PROFILE};

    for (i = 0; i < 8; i++)
    {
        p_tbl_node = cdb_get_tbl(array[i]);
        rc = _ccs_cfg_build_cfg_cdb_dispatch(fp, p_tbl_node, NULL, para);
    }
        
    return rc;
}

int32
ccs_cfg_build_cfg(int32 sock, ctc_msg_t *p_msg, cdb_node_t* p_node)
{
    FILE *fp = NULL;
    char *key = NULL;
    cfg_parse_info_t parse_info;
    char           *para_str = NULL;
    cfg_cmd_para_t  para;
    int32 rs_len = 0;
    int32 rc = PM_E_NONE;

    para.sock = sock;
    para.oper = p_msg->u_hdr.cfg.oper;
    para.pid = p_msg->u_hdr.cfg.pid;
    para.p_msg = p_msg;
    sal_memset(para.ret_buf, 0, sizeof(para.ret_buf));
    para.p_rs = (cfg_result_t*)para.ret_buf;
    para.p_show = (cfg_show_t*)para.ret_buf;
    
    if (p_msg->data_len > MAX_CMD_STR_LEN)
    {
        SAL_ASSERT(0);
        return PM_E_EXCEED_CMD_LEN;
    }

    para_str = p_msg->data;
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Parse parameter fail, return code %d", rc);
        goto error_return;
    }

    if (parse_info.argc == 0)
    {
        sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "NULL build config path");
        goto error_return;
    }
    else if (parse_info.argc >= 2)
    {
        key = parse_info.argv[1];
    }

    if (sal_strcmp(parse_info.argv[0], "config-running_config_for_diff"))
    {
        sal_memset(para.p_show->path, 0, sizeof(para.p_show->path));
        sal_strncpy(para.p_show->path, TMP_DIR, MAX_CMD_SHOW_FNAME_LEN);
        sal_strncat(para.p_show->path, parse_info.argv[0], MAX_CMD_SHOW_FNAME_LEN);
        fp = sal_fopen(para.p_show->path, "w+");
    }
    else
    {
        fp = sal_fopen("/tmp/config-running_config_for_diff", "w+");
    }
    
    if (NULL == fp)
    {
        sal_snprintf(para.p_rs->desc, MAX_CMD_RET_LEN, "Temp file path is invalid");
        goto error_return;
    }
    
    if(!sal_strcmp(p_node->full_name,"/cdb/qos"))
    {
        rc = ccs_cfg_build_cfg_qos_cdb(fp,&para);
        if (rc < 0)
        {
            goto error_return;
        }
    }
    else
    {
        if (CDB_NODE_TBL == p_node->type)
        {
            rc = _ccs_cfg_build_cfg_cdb_dispatch(fp, p_node, key, &para);
            if (rc < 0)
            {
                goto error_return;
            }
        }
        else if (CDB_NODE_ACT == p_node->type)
        {
            rc = _ccs_cfg_build_cfg_cdb_action(fp, p_node, key, &para);
            if (rc < 0)
            {
                goto error_return;
            }
        }
        else
        {
            rc = ccs_cfg_build_cfg_cdb(fp, &para);
            if (rc < 0)
            {
                goto error_return;
            }
        }
    }
    
    sal_fclose(fp);
    rs_len = sizeof(cfg_show_t);
    return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);

error_return:
    para.p_rs->rc = PM_E_CFG_PM_ERROR;
    rs_len = sizeof(cfg_result_t);
    return ipc_pm_to_ccs(para.sock, para.p_msg, para.ret_buf, rs_len);
}

int32
ccs_show_vlan_range(FILE* fp, char *pszPrefix, char *pszVlan)
{
#define M_STR_LINE  50
    int len = sal_strlen(pszVlan);
    char *pszTok = NULL;
    char *pszTmp = NULL;
    char *pSavePtr = NULL;
    int cr;

    if (len <= M_STR_LINE || NULL == sal_strchr(pszVlan, ',')) {
        sal_fprintf(fp, "%s %s\n", pszPrefix, pszVlan);
        return 0;
    }

    pszTmp = sal_strdup(pszVlan);
    if (NULL == pszTmp) {
        sal_fprintf(fp, "%s %s\n", pszPrefix, pszVlan);
        return 0;
    }

    len = 0;
    cr = 0;
    sal_fprintf(fp, "%s ", pszPrefix);
    pszTok = strtok_r(pszTmp, ",", &pSavePtr);
    while (NULL != pszTok) {
        if ((len + sal_strlen(pszTok)) < M_STR_LINE) {
            sal_fprintf(fp, "%s%s", (0 == len) ? "" : ",", pszTok);
            len += sal_strlen(pszTok);
            cr = 0;
        } else {
            sal_fprintf(fp, "\n%s %s", pszPrefix, pszTok);
            len = sal_strlen(pszTok);
            cr = 1;
        }

        /* process next token */
        pszTok = strtok_r(NULL, ",", &pSavePtr);
    }

    if (!cr) {
        sal_fprintf(fp, "\n");
    }
    sal_free(pszTmp);
    return PM_E_NONE;
}
