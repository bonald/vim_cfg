
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "gen/tbl_ipsg_s_ip_define.h"
#include "gen/tbl_ipsg_s_ip.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_iproute_node_add_fail_define.h"
#include "gen/tbl_iproute_node_add_fail.h"
#include "gen/tbl_iproute_node_add_fail_count_define.h"
#include "gen/tbl_iproute_node_add_fail_count.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"
#include "gen/tbl_nexthop_group_routed_define.h"
#include "gen/tbl_nexthop_group_routed.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_group_usm.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_context.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_fan_define.h"
#include "gen/tbl_psu_define.h"
#include "gen/tbl_led_define.h"
#include "gen/tbl_sensor_define.h"
#include "gen/tbl_vcm_define.h"
#include "gen/tbl_poe_define.h"
#include "gen/tbl_reboot_info_define.h"
#include "gen/ds_circuit_id_define.h"
#include "gen/ds_circuit_id.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_fea_port_if_define.h"
#include "gen/tbl_fea_port_if.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_aclqos_if_stats_define.h"
#include "gen/tbl_aclqos_if_stats.h"
#include "gen/tbl_fea_port_policer_apply_define.h"
#include "gen/tbl_fea_g8032_ring_define.h"
#include "gen/tbl_fea_g8032_ring.h"
#include "gen/tbl_fea_g8032_ring_mc_group_define.h"
#include "gen/tbl_fea_g8032_ring_mc_group.h"
#include "gen/tbl_fea_port_policer_apply.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_entry_define.h"
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_policy_map_config_define.h"
#include "gen/tbl_class_map_config_define.h"
#include "gen/tbl_class_in_policy_config_define.h"
#include "gen/tbl_acl_in_class_config_define.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_ospf_define.h"
#include "gen/tbl_ospf.h"
#include "gen/tbl_ospf6_define.h"
#include "gen/tbl_ospf6.h"
#include "gen/tbl_ospf6_interface_define.h"
#include "gen/tbl_ospf6_interface.h"
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "gen/tbl_ospf_area_range_define.h"
#include "gen/tbl_ospf_area_range.h"

#include "gen/tbl_ospf6_area_range_define.h"
#include "gen/tbl_ospf6_area_range.h"

#include "gen/tbl_ospf_area_stub_define.h"
#include "gen/tbl_ospf_area_stub.h"

#include "gen/tbl_ospf6_area_stub_define.h"
#include "gen/tbl_ospf6_area_stub.h"

#include "gen/tbl_ospf_area_nssa_define.h"
#include "gen/tbl_ospf_area_nssa.h"
#include "gen/tbl_ospf_area_vlink_define.h"
#include "gen/tbl_ospf_area_vlink.h"

#include "gen/tbl_bgp_define.h"
#include "gen/tbl_bgp.h"
#include "gen/tbl_bgp_network_define.h"
#include "gen/tbl_bgp_network.h"
#include "gen/tbl_bgp_neighbor_define.h"
#include "gen/tbl_bgp_neighbor.h"

#include "gen/tbl_rip_define.h"
#include "gen/tbl_rip.h"
#include "gen/tbl_rip_network_define.h"
#include "gen/tbl_rip_network.h"
#include "gen/tbl_rip_neighbor_define.h"
#include "gen/tbl_rip_neighbor.h"
#include "gen/tbl_rip_passive_if_define.h"
#include "gen/tbl_rip_passive_if.h"

#include "gen/tbl_ripng_define.h"
#include "gen/tbl_ripng.h"
#include "gen/tbl_ripng_network_define.h"
#include "gen/tbl_ripng_network.h"
#include "gen/tbl_ripng_passive_if_define.h"
#include "gen/tbl_ripng_passive_if.h"

#include "gen/tbl_controller_define.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_residence_time_cache_define.h"
#include "gen/tbl_fea_ptp_ts_define.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"

#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_five_tuple_define.h"
#include "gen/tbl_tap_five_tuple.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "gen/tbl_arpace_config_define.h"
#include "gen/tbl_arpace_config.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#include "gen/tbl_sflow_counter_port_define.h"
#include "gen/tbl_sflow_counter_port.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_ipsla_packet_stats_define.h"
#include "gen/tbl_ipsla_packet_stats.h"
#include "gen/tbl_ipsla_test_stats_define.h"
#include "gen/tbl_ipsla_test_stats.h"
#include "gen/tbl_fea_copp_trap_group_define.h"
#include "gen/tbl_fea_copp_trap_group.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_oam_session_upload_define.h"
#include "gen/tbl_oam_session_upload.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_oam_flow_info_define.h"
#include "gen/tbl_oam_flow_info.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
#include "gen/tbl_oam_inlabel_info_define.h"
#include "gen/tbl_oam_inlabel_info.h"
#include "gen/tbl_oam_mpls_aps_check_define.h"
#include "gen/tbl_oam_mpls_aps_check.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "gen/tbl_mapped_vlan_info_define.h"
#include "gen/tbl_mapped_vlan_info.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/ds_nd_prefix_define.h"
#include "gen/ds_nd_prefix.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"
#include "gen/tbl_login_record_define.h"
#include "gen/tbl_login_record.h"
#include "gen/ds_join_group_define.h"
#include "gen/ds_join_group.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "gen/tbl_nat_proxy_arp_define.h"
#include "gen/tbl_nat_proxy_arp.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"

#include "gen/tbl_hash_field_profile_define.h"
#include "gen/tbl_hash_field_profile.h"

#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"

#include "gen/tbl_hash_value_global_define.h"
#include "gen/tbl_hash_value_global.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_maid_define.h"
#include "gen/tbl_cfm_maid.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"

/*add by zhw for mc*/
#include "gen/tbl_pim_sm_define.h"
#include "gen/tbl_pim_sm.h"
#include "gen/tbl_pim_sm_bsr_define.h"
#include "gen/tbl_pim_sm_bsr.h"
#include "gen/tbl_pim_sm_rp_define.h"
#include "gen/tbl_pim_sm_rp.h"
#include "gen/tbl_pim_dm_define.h"
#include "gen/tbl_pim_dm.h"
#include "gen/tbl_igmp_intf_define.h"
#include "gen/tbl_igmp_intf.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_pim_sm_rp_gp_define.h"
#include "gen/tbl_pim_sm_rp_gp.h"
/*end add zhw*/
int32
tbl_auth_session_cmp(void *data1, void *data2)
{
    tbl_auth_session_t *p = (tbl_auth_session_t *)data1;
    tbl_auth_session_t *p2 = (tbl_auth_session_t *)data2;

    if (p->key.type != p2->key.type) {
        return p->key.type - p2->key.type;
    }

    if (p->key.index != p2->key.index) {
        return p->key.index - p2->key.index;
    }

    return sal_strcmp(p->key.name, p2->key.name);
}

/* order by type and key */
int32
tbl_auth_server_cmp(void *data1, void *data2)
{
    tbl_auth_server_t *p = (tbl_auth_server_t *)data1;
    tbl_auth_server_t *p2 = (tbl_auth_server_t *)data2;

    if (p->key.type != p2->key.type) {
        return p->key.type - p2->key.type;
    }

    if (p->key.sin_port != p2->key.sin_port) {
        return p->key.sin_port - p2->key.sin_port;
    }

    return sal_memcmp(&p->key.addr, &p2->key.addr, sizeof(addr_t));
}

/* order by lease time */
int32
tbl_dhcbinding_cmp(void *data1, void *data2)
{
    tbl_dhcbinding_t *p = (tbl_dhcbinding_t *)data1;
    tbl_dhcbinding_t *p2 = (tbl_dhcbinding_t *)data2;

    return (p->lease - p2->lease);
}

int32
ds_circuit_id_cmp(void *data1, void *data2)
{
    ds_circuit_id_t *p = (ds_circuit_id_t *)data1;
    ds_circuit_id_t *p2 = (ds_circuit_id_t *)data2;

    return (p->key.vid - p2->key.vid);
}

int32
tbl_snmp_view_cmp(void *data1, void *data2)
{
    tbl_snmp_view_t *p = (tbl_snmp_view_t *)data1;
    tbl_snmp_view_t *p2 = (tbl_snmp_view_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_snmp_view_key_t));
}

int32
tbl_snmp_community_cmp(void *data1, void *data2)
{
    tbl_snmp_community_t *p = (tbl_snmp_community_t *)data1;
    tbl_snmp_community_t *p2 = (tbl_snmp_community_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_login_cmp(void *data1, void *data2)
{
    tbl_login_t *p = (tbl_login_t *)data1;
    tbl_login_t *p2 = (tbl_login_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_authen_cmp(void *data1, void *data2)
{
    tbl_authen_t *p = (tbl_authen_t *)data1;
    tbl_authen_t *p2 = (tbl_authen_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_user_cmp(void *data1, void *data2)
{
    tbl_user_t *p = (tbl_user_t*)data1;
    tbl_user_t *p2 = (tbl_user_t*)data2;

    return sal_strcmp( p->key.name, p2->key.name);
}

int32
tbl_login_record_cmp(void *data1, void *data2)
{
    tbl_login_record_t *p_login_record1 = (tbl_login_record_t* )data1;
    tbl_login_record_t *p_login_record2 = (tbl_login_record_t* )data2;

    return sal_strcmp( p_login_record1->key.name, p_login_record2->key.name);
}

int32
tbl_rsa_cmp(void *data1, void *data2)
{
    tbl_rsa_t *p = (tbl_rsa_t*)data1;
    tbl_rsa_t *p2 = (tbl_rsa_t*)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_tap_five_tuple_cmp(void *data1, void *data2)
{
    tbl_tap_five_tuple_t *p1 = (tbl_tap_five_tuple_t *)data1;
    tbl_tap_five_tuple_t *p2 = (tbl_tap_five_tuple_t *)data2;

    return (sal_memcmp(&p1->key, &p2->key, sizeof(tbl_tap_five_tuple_key_t)));
}

int32
tbl_tap_group_cmp(void *data1, void *data2)
{
    tbl_tap_group_t *p = (tbl_tap_group_t *)data1;
    tbl_tap_group_t *p2 = (tbl_tap_group_t*)data2;

    if (0 == sal_strcmp(p->key.name, p2->key.name))
    {
        return 0;
    }

    return 1;
}

int32
tbl_tap_group_ingress_cmp(void *data1, void *data2)
{
    int32 rc = 0;
    tbl_tap_group_ingress_t *p_tap_group_ingress1 = (tbl_tap_group_ingress_t*)data1;
    tbl_tap_group_ingress_t *p_tap_group_ingress2 = (tbl_tap_group_ingress_t*)data2;

    rc = sal_memcmp(p_tap_group_ingress1->key.tap_group, p_tap_group_ingress2->key.tap_group, (sizeof(char))*TAP_NAME_SIZE);
    if (rc != 0)
        return rc;
    rc = sal_memcmp(p_tap_group_ingress1->key.if_name, p_tap_group_ingress2->key.if_name, (sizeof(char))*IFNAME_SIZE);

    return rc;
}

int32
tbl_tap_group_ingress_flow_cmp(void *data1, void *data2)
{
    int32 rc = 0;
    tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow1 = (tbl_tap_group_ingress_flow_t*)data1;
    tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow2 = (tbl_tap_group_ingress_flow_t*)data2;

    rc = sal_memcmp(p_tap_group_ingress_flow1->key.tap_group, p_tap_group_ingress_flow2->key.tap_group, (sizeof(char))*TAP_NAME_SIZE);
    if (rc != 0)
        return rc;
    rc = sal_memcmp(p_tap_group_ingress_flow1->key.if_name, p_tap_group_ingress_flow2->key.if_name, (sizeof(char))*IFNAME_SIZE);
    if (rc != 0)
        return rc;
    rc = sal_memcmp(p_tap_group_ingress_flow1->key.flow_name, p_tap_group_ingress_flow2->key.flow_name, (sizeof(char))*(ACL_NAME_FULL_NAME_SIZE+1));

    return rc;
}

int32
tbl_tap_group_egress_cmp(void *data1, void *data2)
{
    int32 rc = 0;
    tbl_tap_group_egress_t *p_tap_group_egress1 = (tbl_tap_group_egress_t*)data1;
    tbl_tap_group_egress_t *p_tap_group_egress2 = (tbl_tap_group_egress_t*)data2;

    rc = sal_memcmp(p_tap_group_egress1->key.tap_group, p_tap_group_egress2->key.tap_group, (sizeof(char))*TAP_NAME_SIZE);
    if (rc != 0)
        return rc;
    rc = sal_memcmp(p_tap_group_egress2->key.if_name, p_tap_group_egress2->key.if_name, (sizeof(char))*IFNAME_SIZE);

    return rc;
}

int32
tbl_interface_cmp(void *data1, void *data2)
{
    tbl_interface_t *p_if1 = (tbl_interface_t*)data1;
    tbl_interface_t *p_if2 = (tbl_interface_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;

    if (NULL == p_if1 || NULL == p_if2)
    {
        return -1;
    }
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_if1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_if2->key.name);
    return (ifindex1 - ifindex2);
}

int32
tbl_qos_mpls_domain_cmp(void *data1, void *data2)
{
    tbl_qos_mpls_domain_t *p_qos_mpls1 = (tbl_qos_mpls_domain_t*)data1;
    tbl_qos_mpls_domain_t *p_qos_mpls2 = (tbl_qos_mpls_domain_t*)data2;
    return (p_qos_mpls1->key.mpls_label - p_qos_mpls2->key.mpls_label);
}

int32
tbl_qos_queue_smart_buffer_cmp(void *data1, void *data2)
{
    tbl_qos_queue_smart_buffer_t *p_qos_queue1 = (tbl_qos_queue_smart_buffer_t*)data1;
    tbl_qos_queue_smart_buffer_t *p_qos_queue2 = (tbl_qos_queue_smart_buffer_t*)data2;
    return (p_qos_queue1->key.queue_id - p_qos_queue2->key.queue_id);
}

int32
tbl_aclqos_if_cmp(void *data1, void *data2)
{
    tbl_aclqos_if_t *p_aclqos_if1 = (tbl_aclqos_if_t*)data1;
    tbl_aclqos_if_t *p_aclqos_if2 = (tbl_aclqos_if_t*)data2;
    return (p_aclqos_if1->ifindex - p_aclqos_if2->ifindex);
}

int32
tbl_aclqos_if_stats_cmp(void *data1, void *data2)
{
    tbl_aclqos_if_stats_t *p_aclqos_if_stats1 = (tbl_aclqos_if_stats_t*)data1;
    tbl_aclqos_if_stats_t *p_aclqos_if_stats2 = (tbl_aclqos_if_stats_t*)data2;
    return sal_memcmp(p_aclqos_if_stats1->key.name, p_aclqos_if_stats2->key.name, (sizeof(char))*IFNAME_SIZE);
}

int32
tbl_fea_port_policer_apply_cmp(void *data1, void *data2)
{
    tbl_fea_port_policer_apply_t *p_fea_port_policer_apply1 = (tbl_fea_port_policer_apply_t*)data1;
    tbl_fea_port_policer_apply_t *p_fea_port_policer_apply2 = (tbl_fea_port_policer_apply_t*)data2;
    return sal_memcmp(p_fea_port_policer_apply1->key.name, p_fea_port_policer_apply2->key.name, (sizeof(char))*IFNAME_SIZE);
}

int32
tbl_macfilter_cmp(void *data1, void *data2)
{
    tbl_macfilter_t *p_macflt1 = (tbl_macfilter_t*)data1;
    tbl_macfilter_t *p_macflt2 = (tbl_macfilter_t*)data2;
    
    return sal_memcmp(p_macflt1->key.mac, p_macflt2->key.mac, MAC_ADDR_LEN);
}

int32
tbl_cpu_utilization_cmp(void *data1, void *data2)
{
    tbl_cpu_utilization_t *p_cpu_info1 = (tbl_cpu_utilization_t*)data1;
    tbl_cpu_utilization_t *p_cpu_info2 = (tbl_cpu_utilization_t*)data2;

    return sal_strcmp(p_cpu_info1->key.name, p_cpu_info2->key.name);
}

int32
tbl_cpu_limit_cmp(void *data1, void *data2)
{
    tbl_cpu_limit_t *p_cpu_limit1 = (tbl_cpu_limit_t*)data1;
    tbl_cpu_limit_t *p_cpu_limit2 = (tbl_cpu_limit_t*)data2;

    return sal_strcmp(p_cpu_limit1->key.name, p_cpu_limit2->key.name);
}

int32
tbl_pvlan_cmp(void *data1, void *data2)
{
    tbl_pvlan_t *p_pvlan1= (tbl_pvlan_t*)data1;
    tbl_pvlan_t *p_pvlan2 = (tbl_pvlan_t*)data2;
    
    return (p_pvlan1->key.primary_vid - p_pvlan2->key.primary_vid);
}
#if 0
int32
tbl_acl_cmp(void *data1, void *data2)
{
    tbl_acl_t *p_acl1 = (tbl_acl_t*)data1;
    tbl_acl_t *p_acl2 = (tbl_acl_t*)data2;
    return sal_strcmp(p_acl1->key.name, p_acl2->key.name);
}


int32
tbl_ace_cmp(void *data1, void *data2)
{
    tbl_ace_t *p_ace1 = (tbl_ace_t*)data1;
    tbl_ace_t *p_ace2 = (tbl_ace_t*)data2;
    int32 rc = 0;

    rc = sal_strcmp(p_ace1->key.acl_name, p_ace2->key.acl_name);
    if (rc < 0)
    {
        return rc;
    }
    return (p_ace1->key.seq_num - p_ace2->key.seq_num);
}


/* 1. pmap class should not be sorted
   2. class-default should be last */
int32
ds_pmap_class_cmp(void *data1, void *data2)
{
    ds_pmap_class_t *p_pmap_class1 = (ds_pmap_class_t*)data1;
    ds_pmap_class_t *p_pmap_class2 = (ds_pmap_class_t*)data2;
    if (0 == sal_strcmp(p_pmap_class1->key.name, p_pmap_class2->key.name))
    {
        return 0;
    }
    else if (0 == sal_strcmp(p_pmap_class2->key.name, "class-default"))
    {
        return -1;
    }
    else
    {
        return 1;
    }
}


/* return 0 means equal */
int32
ds_mac_ace_cmp(void *data1, void *data2)
{
    ds_mac_ace_t *p_mac_ace1 = (ds_mac_ace_t*)data1;
    ds_mac_ace_t *p_mac_ace2 = (ds_mac_ace_t*)data2;

    return (p_mac_ace1->key.seqno - p_mac_ace2->key.seqno);
}
#endif

int32
tbl_fea_g8032_ring_cmp(void *data1, void *data2)
{
    tbl_fea_g8032_ring_t *p_ring1 = (tbl_fea_g8032_ring_t*)data1;
    tbl_fea_g8032_ring_t *p_ring2 = (tbl_fea_g8032_ring_t*)data2;
  
    if (sal_memcmp(&p_ring1->key, &p_ring2->key, sizeof(tbl_fea_g8032_ring_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_g8032_ring_mc_group_cmp(void *data1, void *data2)
{
    tbl_fea_g8032_ring_mc_group_t *p_ring1 = ( tbl_fea_g8032_ring_mc_group_t *)data1;
     tbl_fea_g8032_ring_mc_group_t *p_ring2 = ( tbl_fea_g8032_ring_mc_group_t *)data2;
  
    return (p_ring1->key.ring_id-p_ring2->key.ring_id);
}

int32
tbl_fdb_cmp(void *p_arg1, void *p_arg2)
{
    tbl_fdb_t *p_fdb1 = (tbl_fdb_t*)p_arg1;
    tbl_fdb_t *p_fdb2 = (tbl_fdb_t*)p_arg2;

    if (sal_memcmp(&p_fdb1->key, &p_fdb2->key, sizeof(tbl_fdb_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsg_s_ip_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsg_s_ip_t *p_ipsg1 = (tbl_ipsg_s_ip_t*)p_arg1;
    tbl_ipsg_s_ip_t *p_ipsg2 = (tbl_ipsg_s_ip_t*)p_arg2;

    if (sal_memcmp(&p_ipsg1->key, &p_ipsg2->key, sizeof(tbl_ipsg_fib_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsg_s_mac_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsg_s_mac_t *p_ipsg1 = (tbl_ipsg_s_mac_t*)p_arg1;
    tbl_ipsg_s_mac_t *p_ipsg2 = (tbl_ipsg_s_mac_t*)p_arg2;

    if (sal_memcmp(&p_ipsg1->key, &p_ipsg2->key, sizeof(tbl_ipsg_fib_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsg_fib_cmp(void *p_arg1, void *p_arg2)
{
    tbl_ipsg_fib_t *p_ipsg1 = (tbl_ipsg_fib_t*)p_arg1;
    tbl_ipsg_fib_t *p_ipsg2 = (tbl_ipsg_fib_t*)p_arg2;

    if (sal_memcmp(&p_ipsg1->key, &p_ipsg2->key, sizeof(tbl_ipsg_fib_t)))
    {
        return TRUE;
    }

    return FALSE;
}

#if 0 
int32
ds_ip_ace_cmp(void *data1, void *data2)
{
    ds_ip_ace_t *p_ip_ace1 = (ds_ip_ace_t*)data1;
    ds_ip_ace_t *p_ip_ace2 = (ds_ip_ace_t*)data2;

    return (p_ip_ace1->key.seqno - p_ip_ace2->key.seqno);
}

int32
tbl_pmap_cmp(void *data1, void *data2)
{
    tbl_pmap_t *p_pmap1 = (tbl_pmap_t*)data1;
    tbl_pmap_t *p_pmap2 = (tbl_pmap_t*)data2;

    return sal_strcmp(p_pmap1->key.name, p_pmap2->key.name);
}

int32
tbl_cmap_cmp(void *data1, void *data2)
{
    tbl_cmap_t *p_cmap1 = (tbl_cmap_t*)data1;
    tbl_cmap_t *p_cmap2 = (tbl_cmap_t*)data2;

    return sal_strcmp(p_cmap1->key.name, p_cmap2->key.name);
}
#endif
int32
tbl_time_range_cmp(void *data1, void *data2)
{
    tbl_time_range_t *p_time_range1 = (tbl_time_range_t* )data1;
    tbl_time_range_t *p_time_range2 = (tbl_time_range_t* )data2;

    return sal_strcmp(p_time_range1->key.name, p_time_range2->key.name);
}

int32
tbl_acl_config_cmp(void *data1, void *data2)
{
    tbl_acl_config_t *p_acl_config1 = (tbl_acl_config_t* )data1;
    tbl_acl_config_t *p_acl_config2 = (tbl_acl_config_t* )data2;

    if (sal_strlen(p_acl_config1->key.name) != sal_strlen(p_acl_config2->key.name))
    {
        return sal_strlen(p_acl_config1->key.name)-sal_strlen(p_acl_config2->key.name);
    }

    return sal_strcmp(p_acl_config1->key.name, p_acl_config2->key.name);
}

int32
tbl_snmp_trap_cmp(void *data1, void *data2)
{
    tbl_snmp_trap_t *p_snmp_trap1 = (tbl_snmp_trap_t*)data1;
    tbl_snmp_trap_t *p_snmp_trap2 = (tbl_snmp_trap_t*)data2;
    
    return sal_memcmp(&p_snmp_trap1->key.trap, &p_snmp_trap2->key.trap, sizeof(snmp_trap_key_t));
}

int32
tbl_ntp_server_cmp(void *data1, void *data2)
{
    tbl_ntp_server_t *p1 = (tbl_ntp_server_t*)data1;
    tbl_ntp_server_t *p2 = (tbl_ntp_server_t*)data2;
    
    return sal_strncmp(p1->key.host, p2->key.host, (NTP_MAX_HOST_IP_SIZE));
}

int32
tbl_ntp_ace_cmp(void *data1, void *data2)
{
    tbl_ntp_ace_t *p1 = (tbl_ntp_ace_t*)data1;
    tbl_ntp_ace_t *p2 = (tbl_ntp_ace_t*)data2;
    
    if(! sal_strncmp(p1->key.host, p2->key.host, (NTP_MAX_HOST_IP_SIZE)))
    {
        return sal_strncmp(p1->key.mask, p2->key.mask, (NTP_MAX_HOST_IP_SIZE));
    }
    else 
    {
        return sal_strncmp(p1->key.host, p2->key.host, (NTP_MAX_HOST_IP_SIZE));
    }
    return 0;
}

int32
tbl_ntp_key_cmp(void *data1, void *data2)
{
    tbl_ntp_key_t *p1 = (tbl_ntp_key_t*)data1;
    tbl_ntp_key_t *p2 = (tbl_ntp_key_t*)data2;
    int rc = 0;
    
    rc = (int)(p1->key.keyid) - (int)(p2->key.keyid);
    return rc;
}

int32
tbl_snmp_inform_cmp(void *data1, void *data2)
{
    tbl_snmp_inform_t *p_snmp_inform1 = (tbl_snmp_inform_t*)data1;
    tbl_snmp_inform_t *p_snmp_inform2 = (tbl_snmp_inform_t*)data2;
    
    return sal_memcmp(&p_snmp_inform1->key.inform, &p_snmp_inform2->key.inform, sizeof(snmp_inform_key_t));
}

int32
tbl_static_dns_cmp(void *data1, void *data2)
{
    tbl_static_dns_t *p_static_dns1 = (tbl_static_dns_t*)data1;
    tbl_static_dns_t *p_static_dns2 = (tbl_static_dns_t*)data2;
    
    return sal_memcmp(p_static_dns1->key.name, p_static_dns2->key.name, sizeof(p_static_dns2->key.name));
}

int32
tbl_dynamic_dns_domain_cmp(void *data1, void *data2)
{
    tbl_dynamic_dns_domain_t *p_dynamic_dns1 = (tbl_dynamic_dns_domain_t*)data1;
    tbl_dynamic_dns_domain_t *p_dynamic_dns2 = (tbl_dynamic_dns_domain_t*)data2;
    
    return sal_memcmp(p_dynamic_dns1->key.name, p_dynamic_dns2->key.name, sizeof(p_dynamic_dns2->key.name));
}

int32
tbl_dynamic_dns_server_cmp(void *data1, void *data2)
{
    tbl_dynamic_dns_server_t *p_dynamic_dns_server1 = (tbl_dynamic_dns_server_t*)data1;
    tbl_dynamic_dns_server_t *p_dynamic_dns_server2 = (tbl_dynamic_dns_server_t*)data2;
    
    return sal_memcmp(&p_dynamic_dns_server1->key.ip_addr, &p_dynamic_dns_server2->key.ip_addr, 
        sizeof(addr_t));
}

int32
ds_pvlan_community_cmp(void *data1, void *data2)
{
    ds_pvlan_community_t *p_pvlan_cm1 = (ds_pvlan_community_t*)data1;
    ds_pvlan_community_t *p_pvlan_cm2 = (ds_pvlan_community_t*)data2;

    return (p_pvlan_cm1->key.community_vid - p_pvlan_cm2->key.community_vid);
}

int32
ds_connected_cmp(void *data1, void *data2)
{
    ds_connected_key_t *p_ifc1 = (ds_connected_key_t*)data1;
    ds_connected_key_t *p_ifc2 = (ds_connected_key_t*)data2;

    return sal_memcmp(p_ifc1, p_ifc2, sizeof(ds_connected_key_t));
}

int32
ds_connected_v6_cmp(void *data1, void *data2)
{
    ds_connected_v6_key_t *p_ifc1 = (ds_connected_v6_key_t*)data1;
    ds_connected_v6_key_t *p_ifc2 = (ds_connected_v6_key_t*)data2;

    return sal_memcmp(p_ifc1, p_ifc2, sizeof(ds_connected_v6_key_t));
}

int32
ds_static_route_dup_ipv4_nh_cmp(void *data1, void *data2)
{
    ds_static_route_dup_ipv4_nh_key_t *p_ifc1 = (ds_static_route_dup_ipv4_nh_key_t*)data1;
    ds_static_route_dup_ipv4_nh_key_t *p_ifc2 = (ds_static_route_dup_ipv4_nh_key_t*)data2;

    return sal_memcmp(p_ifc1, p_ifc2, sizeof(ds_static_route_dup_ipv4_nh_key_t));
}

int32
ds_ospf_auth_cmp(void *data1, void *data2)
{
    ds_ospf_auth_t *p_auth1 = (ds_ospf_auth_t*)data1;
    ds_ospf_auth_t *p_auth2 = (ds_ospf_auth_t*)data2;

    return sal_memcmp(&p_auth1->key, &p_auth2->key, sizeof(p_auth1->key));
}

int32
tbl_route_if_cmp(void *data1, void *data2)
{
    tbl_route_if_t *p_if1 = (tbl_route_if_t*)data1;
    tbl_route_if_t *p_if2 = (tbl_route_if_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_if1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_if2->key.name);
    return (ifindex1 - ifindex2);
}
int32
tbl_rip_cmp(void *data1, void *data2)
{
    tbl_rip_t *p_net1 = (tbl_rip_t*)data1;
    tbl_rip_t *p_net2 = (tbl_rip_t*)data2;

    return (p_net1->key.id - p_net2->key.id);
}

int32
tbl_rip_network_cmp(void *data1, void *data2)
{
    tbl_rip_network_t *p_net1 = (tbl_rip_network_t*)data1;
    tbl_rip_network_t *p_net2 = (tbl_rip_network_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(prefix_t)));
}

int32
tbl_rip_neighbor_cmp(void *data1, void *data2)
{
    tbl_rip_neighbor_t *p_net1 = (tbl_rip_neighbor_t*)data1;
    tbl_rip_neighbor_t *p_net2 = (tbl_rip_neighbor_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(addr_t)));
}

int32
tbl_ripng_cmp(void *data1, void *data2)
{
    tbl_ripng_t *p_net1 = (tbl_ripng_t*)data1;
    tbl_ripng_t *p_net2 = (tbl_ripng_t*)data2;

    return (p_net1->key.id - p_net2->key.id);
}

int32
tbl_ripng_network_cmp(void *data1, void *data2)
{
    tbl_ripng_network_t *p_net1 = (tbl_ripng_network_t*)data1;
    tbl_ripng_network_t *p_net2 = (tbl_ripng_network_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(prefix_t)));
}

int32
tbl_bgp_cmp(void *data1, void *data2)
{
    tbl_bgp_t *p_net1 = (tbl_bgp_t*)data1;
    tbl_bgp_t *p_net2 = (tbl_bgp_t*)data2;

    return (p_net1->key.as_id - p_net2->key.as_id);
}

int32
tbl_bgp_network_cmp(void *data1, void *data2)
{
    tbl_bgp_network_t *p_net1 = (tbl_bgp_network_t*)data1;
    tbl_bgp_network_t *p_net2 = (tbl_bgp_network_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(addr_ipv4_t)));
}

int32
tbl_bgp_neighbor_cmp(void *data1, void *data2)
{
    tbl_bgp_neighbor_t *p_net1 = (tbl_bgp_neighbor_t*)data1;
    tbl_bgp_neighbor_t *p_net2 = (tbl_bgp_neighbor_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(addr_ipv4_t)));
}

int32
tbl_rip_passive_if_cmp(void *data1, void *data2)
{
 	tbl_rip_passive_if_t *p_if1 = (tbl_rip_passive_if_t*)data1;
    tbl_rip_passive_if_t *p_if2 = (tbl_rip_passive_if_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_if1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_if2->key.name);
    return (ifindex1 - ifindex2);
}

int32
tbl_ripng_passive_if_cmp(void *data1, void *data2)
{
 	tbl_ripng_passive_if_t *p_if1 = (tbl_ripng_passive_if_t*)data1;
    tbl_ripng_passive_if_t *p_if2 = (tbl_ripng_passive_if_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_if1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_if2->key.name);
    return (ifindex1 - ifindex2);
}
int32
tbl_ospf_cmp(void *data1, void *data2)
{
    tbl_ospf_t *p_net1 = (tbl_ospf_t*)data1;
    tbl_ospf_t *p_net2 = (tbl_ospf_t*)data2;

    return (p_net1->key.id - p_net2->key.id);
}

int32
tbl_ospf6_cmp(void *data1, void *data2)
{
    tbl_ospf6_t *p_net1 = (tbl_ospf6_t*)data1;
    tbl_ospf6_t *p_net2 = (tbl_ospf6_t*)data2;

    return (p_net1->key.id - p_net2->key.id);
}

int32
tbl_ospf6_interface_cmp(void *data1, void *data2)
{
    tbl_ospf6_interface_t *p_net1 = (tbl_ospf6_interface_t*)data1;
    tbl_ospf6_interface_t *p_net2 = (tbl_ospf6_interface_t*)data2;
	int rc = 0;
	rc = sal_memcmp(p_net1->key.name, p_net2->key.name, (sizeof(char))*IFNAME_SIZE);

	return rc;
}

int32
tbl_nexthop_group_cmp(void *data1, void *data2)
{
    tbl_nexthop_group_t *p_net1 = (tbl_nexthop_group_t*)data1;
    tbl_nexthop_group_t *p_net2 = (tbl_nexthop_group_t*)data2;

    return sal_memcmp(&p_net1->key, &p_net2->key, sizeof(p_net2->key));
}

int32
tbl_nexthop_group_routed_cmp(void *data1, void *data2)
{
    tbl_nexthop_group_routed_t *p_net1 = (tbl_nexthop_group_routed_t*)data1;
    tbl_nexthop_group_routed_t *p_net2 = (tbl_nexthop_group_routed_t*)data2;

    return sal_memcmp(&p_net1->key, &p_net2->key, sizeof(p_net2->key));
}

int32
tbl_ospf_network_cmp(void *data1, void *data2)
{
    tbl_ospf_network_t *p_net1 = (tbl_ospf_network_t*)data1;
    tbl_ospf_network_t *p_net2 = (tbl_ospf_network_t*)data2;

    return (sal_memcmp(&p_net1->key, &p_net2->key, sizeof(prefix_t)));
}

int32
tbl_ospf_area_auth_cmp(void *data1, void *data2)
{
    tbl_ospf_area_auth_t *p_area1 = (tbl_ospf_area_auth_t*)data1;
    tbl_ospf_area_auth_t *p_area2 = (tbl_ospf_area_auth_t*)data2;

    if (p_area1->key.areaid > p_area2->key.areaid)
    {
        return 1;
    }
    else if (p_area1->key.areaid < p_area2->key.areaid)
    {
        return -1;
    }
    return (p_area1->key.areaid - p_area2->key.areaid);
}

int32
tbl_ospf_area_range_cmp(void *data1, void *data2)
{
    tbl_ospf_area_range_t *p_area_range1 = (tbl_ospf_area_range_t*)data1;
    tbl_ospf_area_range_t *p_area_range2 = (tbl_ospf_area_range_t*)data2;

    return sal_memcmp(&p_area_range1->key, &p_area_range2->key, sizeof(p_area_range1->key));
}

int32
tbl_ospf6_area_range_cmp(void *data1, void *data2)
{
    tbl_ospf6_area_range_t *p_area_range1 = (tbl_ospf6_area_range_t*)data1;
    tbl_ospf6_area_range_t *p_area_range2 = (tbl_ospf6_area_range_t*)data2;

    return sal_memcmp(&p_area_range1->key, &p_area_range2->key, sizeof(p_area_range1->key));
}

int32
tbl_arp_cmp(void *data1, void *data2)
{
    tbl_arp_t *p_arp1 = (tbl_arp_t*)data1;
    tbl_arp_t *p_arp2 = (tbl_arp_t*)data2;
    uint32 addr1 = 0;
    uint32 addr2 = 0;
    
    addr1 = p_arp1->key.ip.s_addr;
    addr2 = p_arp2->key.ip.s_addr;
    return (addr1 - addr2);
}

int32
tbl_kernel_if_cmp(void *data1, void *data2)
{
    tbl_kernel_if_t *p_if1 = (tbl_kernel_if_t*)data1;
    tbl_kernel_if_t *p_if2 = (tbl_kernel_if_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = p_if1->key.ifindex;
    ifindex2 = p_if2->key.ifindex;
    return (ifindex1 - ifindex2);
}

int32
tbl_iproute_node_cmp(void *data1, void *data2)
{
    tbl_iproute_node_t *p_iproute1 = (tbl_iproute_node_t*)data1;
    tbl_iproute_node_t *p_iproute2 = (tbl_iproute_node_t*)data2;
    
    return sal_memcmp(&p_iproute1->key, &p_iproute2->key, sizeof(p_iproute1->key));
}

int32
tbl_iproute_node_add_fail_cmp(void *data1, void *data2)
{
    tbl_iproute_node_add_fail_t *p_iproute1 = (tbl_iproute_node_add_fail_t*)data1;
    tbl_iproute_node_add_fail_t *p_iproute2 = (tbl_iproute_node_add_fail_t*)data2;
    
    return sal_memcmp(&p_iproute1->key, &p_iproute2->key, sizeof(p_iproute1->key));
}

int32
tbl_iproute_node_add_fail_count_cmp(void *data1, void *data2)
{
    tbl_iproute_node_add_fail_count_t *p_iproute1 = (tbl_iproute_node_add_fail_count_t*)data1;
    tbl_iproute_node_add_fail_count_t *p_iproute2 = (tbl_iproute_node_add_fail_count_t*)data2;
    
    return sal_memcmp(&p_iproute1->key, &p_iproute2->key, sizeof(p_iproute1->key));
}

int32
tbl_static_route_cfg_cmp(void *data1, void *data2)
{
    tbl_static_route_cfg_t *p_rt_s1 = (tbl_static_route_cfg_t*)data1;
    tbl_static_route_cfg_t *p_rt_s2 = (tbl_static_route_cfg_t*)data2;
    
    return sal_memcmp(&p_rt_s1->key, &p_rt_s2->key, sizeof(p_rt_s1->key));
}

int32
tbl_fea_port_if_cmp(void *data1, void *data2)
{
    tbl_fea_port_if_t *p_if1 = (tbl_fea_port_if_t*)data1;
    tbl_fea_port_if_t *p_if2 = (tbl_fea_port_if_t*)data2;
    uint64 port1 = 0;
    uint64 port2 = 0;
    
    port1 = p_if1->key.portid;
    port2 = p_if2->key.portid;
    return (port1 - port2);
}


int32
tbl_rif_cmp(void *data1, void *data2)
{
    tbl_route_if_t *p_if1 = (tbl_route_if_t*)data1;
    tbl_route_if_t *p_if2 = (tbl_route_if_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_if1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_if2->key.name);
    return (ifindex1 - ifindex2);
}

int32
tbl_mstp_port_cmp(void *data1, void *data2)
{
    tbl_mstp_port_t *p_port1 = (tbl_mstp_port_t*)data1;
    tbl_mstp_port_t *p_port2 = (tbl_mstp_port_t*)data2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    
    ifindex1 = tbl_interface_get_ifindex_by_name(p_port1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_port2->key.name);
    return (ifindex1 - ifindex2);
}


int32
tbl_fan_cmp(void *data1, void *data2)
{
    tbl_fan_t *p_fan1 = (tbl_fan_t*)data1;
    tbl_fan_t *p_fan2 = (tbl_fan_t*)data2;
    
    return (p_fan1->key.id - p_fan2->key.id);
} 

int32
tbl_psu_cmp(void *data1, void *data2)
{
    tbl_psu_t *p_psu1 = (tbl_psu_t*)data1;
    tbl_psu_t *p_psu2 = (tbl_psu_t*)data2;
    
    return (p_psu1->key.id - p_psu2->key.id);
} 

int32
tbl_led_cmp(void *data1, void *data2)
{
    tbl_led_t *p_led1 = (tbl_led_t*)data1;
    tbl_led_t *p_led2 = (tbl_led_t*)data2;
    
    return (p_led1->key.id - p_led2->key.id);
} 

int32
tbl_sensor_cmp(void *data1, void *data2)
{
    tbl_sensor_t *p_sensor1 = (tbl_sensor_t*)data1;
    tbl_sensor_t *p_sensor2 = (tbl_sensor_t*)data2;
    
    return (p_sensor1->key.id - p_sensor2->key.id);
} 

int32
tbl_vcm_cmp(void *data1, void *data2)
{
    tbl_vcm_t *p_vcm1 = (tbl_vcm_t*)data1;
    tbl_vcm_t *p_vcm2 = (tbl_vcm_t*)data2;
    
    return (p_vcm1->key.id - p_vcm2->key.id);
} 

int32
tbl_poe_cmp(void *data1, void *data2)
{
    tbl_poe_t *p_poe1 = (tbl_poe_t*)data1;
    tbl_poe_t *p_poe2 = (tbl_poe_t*)data2;
    
    return (p_poe1->key.id - p_poe2->key.id);
} 

int32
tbl_reboot_info_cmp(void *data1, void *data2)
{
    tbl_reboot_info_t *p_reboot1 = (tbl_reboot_info_t*)data1;
    tbl_reboot_info_t *p_reboot2 = (tbl_reboot_info_t*)data2;
    
    return (p_reboot1->key.index - p_reboot2->key.index);
} 


int32
tbl_ns_port_forwarding_cmp(void *data1, void *data2)
{
    tbl_ns_port_forwarding_t *p1 = (tbl_ns_port_forwarding_t*)data1;
    tbl_ns_port_forwarding_t *p2 = (tbl_ns_port_forwarding_t*)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_ns_port_forwarding_key_t));
}

int32
tbl_cem_cmp(void *data1, void *data2)
{
    tbl_cem_t *p_cem1 = (tbl_cem_t* )data1;
    tbl_cem_t *p_cem2 = (tbl_cem_t* )data2;

    return sal_memcmp(p_cem1->key.name, p_cem2->key.name, sizeof(p_cem1->key.name));
}

int32
tbl_mstp_instance_cmp(void *data1, void *data2)
{
    tbl_mstp_instance_t *p_inst1 = (tbl_mstp_instance_t*)data1;
    tbl_mstp_instance_t *p_inst2 = (tbl_mstp_instance_t*)data2;

    return (p_inst1->key.instance - p_inst2->key.instance);
}

int32
tbl_msti_port_cmp(void *p_arg1, void *p_arg2)
{
    tbl_msti_port_t *p_msti_port1 = (tbl_msti_port_t*)p_arg1;
    tbl_msti_port_t *p_msti_port2 = (tbl_msti_port_t*)p_arg2;
    uint32 ifindex1 = 0;
    uint32 ifindex2 = 0;
    int32 rc = 0;

    rc = sal_memcmp(&p_msti_port1->key, &p_msti_port2->key, sizeof(tbl_msti_port_key_t));
    if (0 == rc)
    {
        /* 1. same */
        return 0;
    }

    ifindex1 = tbl_interface_get_ifindex_by_name(p_msti_port1->key.name);
    ifindex2 = tbl_interface_get_ifindex_by_name(p_msti_port2->key.name);
    if (ifindex1 != ifindex2)
    {
        /* 2. not same port */
        return (ifindex1 - ifindex2);
    }
    else
    {
        /* 3. not same instance */
        return (p_msti_port1->key.instance - p_msti_port2->key.instance);
    }
}

int32
tbl_mlag_port_cmp(void *p_arg1, void *p_arg2)
{
    tbl_mlag_port_t *p_group1 = (tbl_mlag_port_t*)p_arg1;
    tbl_mlag_port_t *p_group2 = (tbl_mlag_port_t*)p_arg2;

    return (p_group1->key.id - p_group2->key.id);
}

int32
tbl_port_stats_cmp(void *data1, void *data2)
{
    tbl_port_stats_t *p_stat1 = (tbl_port_stats_t*)data1;
    tbl_port_stats_t *p_stat2 = (tbl_port_stats_t*)data2;
    
    return (p_stat1->key.ifindex - p_stat2->key.ifindex);
}

int32
tbl_port_stats_rate_cmp(void *data1, void *data2)
{
    tbl_port_stats_rate_t *p_stat1 = (tbl_port_stats_rate_t*)data1;
    tbl_port_stats_rate_t *p_stat2 = (tbl_port_stats_rate_t*)data2;
    
    return (p_stat1->key.ifindex - p_stat2->key.ifindex);
}
int32
tbl_acl_worm_filter_cmp(void *data1, void *data2)
{
    tbl_acl_worm_filter_t *p_worm_filter1 = (tbl_acl_worm_filter_t*)data1;
    tbl_acl_worm_filter_t *p_worm_filter2 = (tbl_acl_worm_filter_t*)data2;

    return sal_memcmp(p_worm_filter1->key.worm_filter_name, p_worm_filter2->key.worm_filter_name, sizeof(tbl_acl_worm_filter_key_t));
}


int32
tbl_ace_config_cmp(void *data1, void *data2)
{
    tbl_ace_config_t *p_ace1 = (tbl_ace_config_t*)data1;
    tbl_ace_config_t *p_ace2 = (tbl_ace_config_t*)data2;
    int32 rc = 0;

    rc = sal_strcmp(p_ace1->key.acl_name, p_ace2->key.acl_name);
    if (rc)
    {
        return rc;
    }
    else
    {
        return (p_ace1->key.seq_no - p_ace2->key.seq_no);
    }
}

int32
tbl_acl_entry_cmp(void *data1, void *data2)
{
    tbl_acl_entry_t *p_acl_entry1 = (tbl_acl_entry_t*)data1;
    tbl_acl_entry_t *p_acl_entry2 = (tbl_acl_entry_t*)data2;
    int32 rc = 0;
    
    if (p_acl_entry1->key.aclid > p_acl_entry2->key.aclid)
    {
        rc = 1;
    }
    else if (p_acl_entry1->key.aclid == p_acl_entry2->key.aclid)
    {
        rc = 0;
    }
    else
    {
        rc = -1;
    }
    return rc;
}

int32
tbl_policy_map_config_cmp(void *data1, void *data2)
{
    tbl_policy_map_config_t *p_policy_map1 = (tbl_policy_map_config_t*)data1;
    tbl_policy_map_config_t *p_policy_map2 = (tbl_policy_map_config_t*)data2;

    return sal_memcmp(p_policy_map1->key.name, p_policy_map2->key.name, (sizeof(char))*IFNAME_SIZE);
}

int32
tbl_class_map_config_cmp(void *data1, void *data2)
{
    tbl_class_map_config_t *p_class_map1 = (tbl_class_map_config_t*)data1;
    tbl_class_map_config_t *p_class_map2 = (tbl_class_map_config_t*)data2;

    return sal_memcmp(p_class_map1->key.name, p_class_map2->key.name, (sizeof(char))*IFNAME_SIZE);
}

int32
tbl_class_in_policy_config_cmp(void *data1, void *data2)
{
    /* SYSTEM MODIFIED by wangqj for bug 40047 at 2016/8/23 */
    return 1;
#if 0
    int32 rc = 0;

    tbl_class_in_policy_config_t *p_class_in_policy_config1 = (tbl_class_in_policy_config_t*)data1;
    tbl_class_in_policy_config_t *p_class_in_policy_config2 = (tbl_class_in_policy_config_t*)data2;


    rc = sal_memcmp(p_class_in_policy_config1->key.policy_name, p_class_in_policy_config2->key.policy_name, (sizeof(char))*IFNAME_SIZE);
    rc  += sal_memcmp(p_class_in_policy_config1->key.class_name, p_class_in_policy_config2->key.class_name, (sizeof(char))*IFNAME_SIZE);
    if (rc)
    {
        return rc;
    }
    else
    {
        return (p_class_in_policy_config1->class_priority - p_class_in_policy_config2->class_priority);
    }
#endif
}

int32
tbl_acl_in_class_config_cmp(void *data1, void *data2)
{
    /*modified by yejl for bug 49769, 2018-11-27*/
    return 0;
    #if 0
    int32 rc = 0;
    tbl_acl_in_class_config_t *p_acl_in_class_config1 = (tbl_acl_in_class_config_t*)data1;
    tbl_acl_in_class_config_t *p_acl_in_class_config2 = (tbl_acl_in_class_config_t*)data2;

    rc = sal_memcmp(p_acl_in_class_config1->key.class_name, p_acl_in_class_config2->key.class_name, (sizeof(char))*IFNAME_SIZE);
    rc += sal_memcmp(p_acl_in_class_config1->key.acl_name, p_acl_in_class_config2->key.acl_name, (sizeof(char))*IFNAME_SIZE);
    if (rc)
    {
        return rc;
    }
    else
    {
        return (p_acl_in_class_config1->acl_priority - p_acl_in_class_config1->acl_priority);
    }
    #endif
}

int32
tbl_class_map_action_config_cmp(void *data1, void *data2)
{
    int32 rc = 0;
    tbl_class_map_action_config_t *p_class_map_action_config1 = (tbl_class_map_action_config_t*)data1;
    tbl_class_map_action_config_t *p_class_map_action_config2 = (tbl_class_map_action_config_t*)data2;

    rc = sal_memcmp(p_class_map_action_config1->key.policy_name, p_class_map_action_config2->key.policy_name, (sizeof(char))*IFNAME_SIZE);
    rc += sal_memcmp(p_class_map_action_config1->key.class_name, p_class_map_action_config2->key.class_name, (sizeof(char))*IFNAME_SIZE);

    return rc;
}




int32
tbl_lldp_if_cmp(void *data1, void *data2)
{
    tbl_lldp_if_t *p_if1 = (tbl_lldp_if_t*)data1;
    tbl_lldp_if_t *p_if2 = (tbl_lldp_if_t*)data2;
    
    return (p_if1->key.ifindex - p_if2->key.ifindex);
}

int32
ds_lldp_neighbour_cmp(void *data1, void *data2)
{
    ds_lldp_neighbour_t *p_neigh1 = (ds_lldp_neighbour_t*)data1;
    ds_lldp_neighbour_t *p_neigh2 = (ds_lldp_neighbour_t*)data2;

    return sal_memcmp(&p_neigh1->key, &p_neigh2->key, sizeof(ds_lldp_neighbour_key_t));
/*    
    if (p_neigh1->key.chassis_id.id == p_neigh2->key.chassis_id.id
     && p_neigh1->key.chassis_id.len == p_neigh2->key.chassis_id.len
     && 0 == sal_memcmp(p_neigh1->key.chassis_id.id, p_neigh2->key.chassis_id.id, p_neigh1->key.chassis_id.len)
     && p_neigh1->key.port_id.id == p_neigh2->key.port_id.id
     && p_neigh1->key.port_id.len == p_neigh2->key.port_id.len
     && 0 == sal_memcmp(p_neigh1->key.port_id.id, p_neigh2->key.port_id.id, p_neigh1->key.port_id.len))
    {
        return 0;
    }

    return 1;
*/
}

int32
tbl_erps_ring_cmp(void *data1, void *data2)
{
    tbl_erps_ring_t *p_ring1 = (tbl_erps_ring_t*)data1;
    tbl_erps_ring_t *p_ring2 = (tbl_erps_ring_t*)data2;

    if (sal_memcmp(&p_ring1->key, &p_ring2->key, sizeof(tbl_erps_ring_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
ds_mrouter_port_cmp(void *data1, void *data2)
{
    ds_mrouter_port_t *p_mr1 = (ds_mrouter_port_t*)data1;
    ds_mrouter_port_t *p_mr2 = (ds_mrouter_port_t*)data2;

    if (sal_memcmp(&p_mr1->key, &p_mr2->key, sizeof(ds_mrouter_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
ds_query_cmp(void *data1, void *data2)
{
    ds_query_t *p_query1 = (ds_query_t*)data1;
    ds_query_t *p_query2 = (ds_query_t*)data2;

    if (sal_memcmp(&p_query1->key, &p_query2->key, sizeof(ds_query_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
ds_join_group_cmp(void *data1, void *data2)
{
    ds_join_group_t *p_query1 = (ds_join_group_t*)data1;
    ds_join_group_t *p_query2 = (ds_join_group_t*)data2;

    if (sal_memcmp(&p_query1->key, &p_query2->key, sizeof(ds_join_group_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_dot1x_port_cmp(void *data1, void *data2)
{
    tbl_dot1x_port_t *p_port1 = (tbl_dot1x_port_t*)data1;
    tbl_dot1x_port_t *p_port2 = (tbl_dot1x_port_t*)data2;

    return sal_memcmp(&p_port1->key, &p_port2->key, sizeof(tbl_dot1x_port_key_t));
}

int32
tbl_dot1x_radius_cmp(void *data1, void *data2)
{
    tbl_dot1x_radius_t *p_radius1 = (tbl_dot1x_radius_t*)data1;
    tbl_dot1x_radius_t *p_radius2 = (tbl_dot1x_radius_t*)data2;

    return sal_memcmp(&p_radius1->key, &p_radius2->key, sizeof(tbl_dot1x_radius_key_t));
}

int32
tbl_sflow_collector_cmp(void *data1, void *data2)
{
    tbl_sflow_collector_t *p_collector1 = (tbl_sflow_collector_t*)data1;
    tbl_sflow_collector_t *p_collector2 = (tbl_sflow_collector_t*)data2;

    return sal_memcmp(&p_collector1->key, &p_collector2->key, sizeof(tbl_sflow_collector_key_t));
}

int32
tbl_sflow_counter_port_cmp(void *data1, void *data2)
{
    tbl_sflow_counter_port_t *p_counter_port1 = (tbl_sflow_counter_port_t*)data1;
    tbl_sflow_counter_port_t *p_counter_port2 = (tbl_sflow_counter_port_t*)data2;

    return sal_memcmp(&p_counter_port1->key, &p_counter_port2->key, sizeof(tbl_sflow_counter_port_key_t));
}

int32
tbl_dot1x_mac_cmp(void *data1, void *data2)
{
    tbl_dot1x_mac_t *p_mac1 = (tbl_dot1x_mac_t*)data1;
    tbl_dot1x_mac_t *p_mac2 = (tbl_dot1x_mac_t*)data2;

    return sal_memcmp(&p_mac1->key, &p_mac2->key, sizeof(tbl_dot1x_mac_t));
}

int32
tbl_author_cmp(void *data1, void *data2)
{
    tbl_author_t *p = (tbl_author_t *)data1;
    tbl_author_t *p2 = (tbl_author_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_account_cmp(void *data1, void *data2)
{
    tbl_account_t *p = (tbl_account_t *)data1;
    tbl_account_t *p2 = (tbl_account_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_accountcmd_cmp(void *data1, void *data2)
{
    tbl_accountcmd_t *p = (tbl_accountcmd_t *)data1;
    tbl_accountcmd_t *p2 = (tbl_accountcmd_t *)data2;

    return sal_strcmp(p->key.name, p2->key.name);
}

int32
tbl_vlanclass_rule_cmp(void *data1, void *data2)
{
    tbl_vlanclass_rule_t *p1 = (tbl_vlanclass_rule_t *)data1;
    tbl_vlanclass_rule_t *p2 = (tbl_vlanclass_rule_t *)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_vlanclass_rule_key_t));
}

int32
tbl_vlanclass_group_cmp(void *data1, void *data2)
{
    tbl_vlanclass_group_t *p1 = (tbl_vlanclass_group_t *)data1;
    tbl_vlanclass_group_t *p2 = (tbl_vlanclass_group_t *)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_vlanclass_group_key_t));
}

int32
tbl_mirror_mac_escape_cmp(void *data1, void *data2)
{
    tbl_mirror_mac_escape_t *p1 = (tbl_mirror_mac_escape_t *)data1;
    tbl_mirror_mac_escape_t *p2 = (tbl_mirror_mac_escape_t *)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_mirror_mac_escape_key_t));
}

int32
tbl_controller_cmp(void *data1, void *data2)
{
    tbl_controller_t *p1 = (tbl_controller_t *)data1;
    tbl_controller_t *p2 = (tbl_controller_t *)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_controller_key_t));
}

int32
tbl_ns_route_cmp(void *p_arg1, void *p_arg2)
{
    return 0;
}

int32
tbl_ns_route_ip_cmp(void *p_arg1, void *p_arg2)
{
    return 0;
}

int32
tbl_openflow_interface_cmp(void *data1, void *data2)
{
    tbl_openflow_interface_t *p_rt_s1 = (tbl_openflow_interface_t*)data1;
    tbl_openflow_interface_t *p_rt_s2 = (tbl_openflow_interface_t*)data2;
    
    return sal_memcmp(&p_rt_s1->key, &p_rt_s2->key, sizeof(p_rt_s1->key));
}

int32
tbl_openflow_tunnel_interface_cmp(void *data1, void *data2)
{
    tbl_openflow_tunnel_interface_t *p_rt_s1 = (tbl_openflow_tunnel_interface_t*)data1;
    tbl_openflow_tunnel_interface_t *p_rt_s2 = (tbl_openflow_tunnel_interface_t*)data2;
    
    return sal_memcmp(&p_rt_s1->key, &p_rt_s2->key, sizeof(p_rt_s1->key));
}

int32
tbl_openflow_manager_cmp(void *p_arg1, void *p_arg2)
{
    return 0;
}

int32
tbl_ptp_port_cmp(void *data1, void *data2)
{
    tbl_ptp_port_t *p_ptp_port1 = (tbl_ptp_port_t*)data1;
    tbl_ptp_port_t *p_ptp_port2 = (tbl_ptp_port_t*)data2;
    
    //return (p_ptp_port1->ifindex - p_ptp_port2->ifindex);    
    return sal_memcmp(p_ptp_port1->key.name, p_ptp_port2->key.name, sizeof(p_ptp_port1->key.name));    
}

int32
tbl_ptp_foreign_cmp(void *data1, void *data2)
{
    tbl_ptp_foreign_t *p_ptp_foreign1 = (tbl_ptp_foreign_t*)data1;
    tbl_ptp_foreign_t *p_ptp_foreign2 = (tbl_ptp_foreign_t*)data2;
    //int32 clock_id_cmp = 0;
    /*
    clock_id_cmp = sal_strcmp((char*)p_ptp_foreign1->key.clock_id, (char*)p_ptp_foreign2->key.clock_id);
    if (0 != clock_id_cmp)
    {
        return clock_id_cmp;
    }
    
    return (p_ptp_foreign1->key.port_num - p_ptp_foreign2->key.port_num);*/
    return sal_memcmp(&p_ptp_foreign1->key, &p_ptp_foreign2->key, sizeof(p_ptp_foreign1->key));
}

int32
tbl_ptp_residence_time_cache_cmp(void *data1, void *data2)
{
    tbl_ptp_residence_time_cache_t *p_ptp_residence_time_cache1 = (tbl_ptp_residence_time_cache_t*)data1;
    tbl_ptp_residence_time_cache_t *p_ptp_residence_time_cache2 = (tbl_ptp_residence_time_cache_t*)data2;
    
    
    return sal_memcmp(&p_ptp_residence_time_cache1->key, &p_ptp_residence_time_cache2->key, sizeof(p_ptp_residence_time_cache1->key));
}

int32
tbl_fea_ptp_ts_cmp(void *data1, void *data2)
{
    tbl_fea_ptp_ts_t *p_db1 = (tbl_fea_ptp_ts_t*)data1;
    tbl_fea_ptp_ts_t *p_db2 = (tbl_fea_ptp_ts_t*)data2;
    uint32 intr_seq_id1 = 0;
    uint32 intr_seq_id2 = 0;
    
    intr_seq_id1 = p_db1->key.intr_seq_id;
    intr_seq_id2 = p_db2->key.intr_seq_id;
    return (intr_seq_id1 - intr_seq_id2);
}

int32
tbl_inband_snat_cmp(void *p_arg1, void *p_arg2)
{
    return 0;
}

int32
tbl_arpacl_config_cmp(void *data1, void *data2)
{
    tbl_arpacl_config_t *p_arpacl1 = (tbl_arpacl_config_t*)data1;
    tbl_arpacl_config_t *p_arpacl2 = (tbl_arpacl_config_t*)data2;
    return sal_strcmp(p_arpacl1->key.name, p_arpacl2->key.name);
}

int32
tbl_arpace_config_cmp(void *data1, void *data2)
{
    tbl_arpace_config_t *p_ace1 = (tbl_arpace_config_t*)data1;
    tbl_arpace_config_t *p_ace2 = (tbl_arpace_config_t*)data2;
    int32 rc = 0;

    rc = sal_strcmp(p_ace1->key.arpacl_name, p_ace2->key.arpacl_name);
    if (rc)
    {
        return rc;
    }
    else
    {
        return (p_ace1->key.seq_no - p_ace2->key.seq_no);
    }
}

int32
tbl_vrrp_vmac_cmp(void *data1, void *data2)
{
    tbl_vrrp_vmac_t *p_mac1 = (tbl_vrrp_vmac_t*)data1;
    tbl_vrrp_vmac_t *p_mac2 = (tbl_vrrp_vmac_t*)data2;

    return sal_memcmp(&p_mac1->key, &p_mac2->key, sizeof(tbl_vrrp_vmac_key_t));
}

int32
tbl_vrrp_vip_cmp(void *data1, void *data2)
{
    tbl_vrrp_vip_t *p_vip1 = (tbl_vrrp_vip_t*)data1;
    tbl_vrrp_vip_t *p_vip2 = (tbl_vrrp_vip_t*)data2;
    
    return sal_memcmp(&p_vip1->key, &p_vip2->key, sizeof(addr_t));
}

int32
tbl_vrrp_session_cmp(void *data1, void *data2)
{
    tbl_vrrp_session_t *p_sess1 = (tbl_vrrp_session_t*)data1;
    tbl_vrrp_session_t *p_sess2 = (tbl_vrrp_session_t*)data2;

    return sal_memcmp(&p_sess1->key, &p_sess2->key, sizeof(tbl_vrrp_session_key_t));
}

int32
tbl_ssm_port_cmp(void *data1, void *data2)
{
    tbl_ssm_port_t *p_sess1 = (tbl_ssm_port_t*)data1;
    tbl_ssm_port_t *p_sess2 = (tbl_ssm_port_t*)data2;
    return (p_sess1->ifindex - p_sess2->ifindex);
    //return sal_memcmp(&p_sess1->key, &p_sess2->key, sizeof(tbl_ssm_port_key_t));
}

int32
tbl_snmp_context_cmp(void *data1, void *data2)
{
    tbl_snmp_context_t *p_snmp_context1 = (tbl_snmp_context_t* )data1;
    tbl_snmp_context_t *p_snmp_context2 = (tbl_snmp_context_t* )data2;

    if (sal_strlen(p_snmp_context1->key.context_name) != sal_strlen(p_snmp_context2->key.context_name))
    {
        return sal_strlen(p_snmp_context1->key.context_name)-sal_strlen(p_snmp_context2->key.context_name);
    }

    return sal_strcmp(p_snmp_context1->key.context_name, p_snmp_context2->key.context_name);
}

int32
tbl_snmp_usm_user_cmp(void *data1, void *data2)
{
    tbl_snmp_usm_user_t *p_snmp_user1 = (tbl_snmp_usm_user_t* )data1;
    tbl_snmp_usm_user_t *p_snmp_user2 = (tbl_snmp_usm_user_t* )data2;

    if (sal_strlen(p_snmp_user1->key.usm_user_name) != sal_strlen(p_snmp_user2->key.usm_user_name))
    {
        return sal_strlen(p_snmp_user1->key.usm_user_name)-sal_strlen(p_snmp_user2->key.usm_user_name);
    }

    return sal_strcmp(p_snmp_user1->key.usm_user_name, p_snmp_user2->key.usm_user_name);
}

int32
tbl_snmp_group_usm_cmp(void *data1, void *data2)
{
    tbl_snmp_group_usm_t *p_snmp_user_group1 = (tbl_snmp_group_usm_t* )data1;
    tbl_snmp_group_usm_t *p_snmp_user_group2 = (tbl_snmp_group_usm_t* )data2;

    if (sal_strlen(p_snmp_user_group1->key.group_usm.grp_name) != sal_strlen(p_snmp_user_group2->key.group_usm.grp_name))
    {
        return sal_strlen(p_snmp_user_group1->key.group_usm.grp_name)-sal_strlen(p_snmp_user_group2->key.group_usm.grp_name);
    }
    else if (sal_strlen(p_snmp_user_group1->key.group_usm.grp_name) == sal_strlen(p_snmp_user_group2->key.group_usm.grp_name)
        && 0 == sal_strcmp(p_snmp_user_group1->key.group_usm.grp_name, p_snmp_user_group2->key.group_usm.grp_name))
    {
        if (sal_strlen(p_snmp_user_group1->key.group_usm.security_name) != sal_strlen(p_snmp_user_group2->key.group_usm.security_name))
        {
            return sal_strlen(p_snmp_user_group1->key.group_usm.security_name)-sal_strlen(p_snmp_user_group2->key.group_usm.security_name);
        }
        else
        {
            return sal_strcmp(p_snmp_user_group1->key.group_usm.security_name, p_snmp_user_group2->key.group_usm.security_name);
        }
    }

    return sal_strcmp(p_snmp_user_group1->key.group_usm.grp_name, p_snmp_user_group2->key.group_usm.grp_name);
}

int32
tbl_snmp_access_usm_cmp(void *data1, void *data2)
{
    tbl_snmp_access_usm_t *p_snmp_access_usm1 = (tbl_snmp_access_usm_t* )data1;
    tbl_snmp_access_usm_t *p_snmp_access_usm2 = (tbl_snmp_access_usm_t* )data2;

    if (sal_strlen(p_snmp_access_usm1->key.usm_access_name) != sal_strlen(p_snmp_access_usm2->key.usm_access_name))
    {
        return sal_strlen(p_snmp_access_usm1->key.usm_access_name)-sal_strlen(p_snmp_access_usm2->key.usm_access_name);
    }

    return sal_strcmp(p_snmp_access_usm1->key.usm_access_name, p_snmp_access_usm2->key.usm_access_name);
}

int32
tbl_snmp_notify_cmp(void *data1, void *data2)
{
    tbl_snmp_notify_t *p_snmp_notify1 = (tbl_snmp_notify_t* )data1;
    tbl_snmp_notify_t *p_snmp_notify2 = (tbl_snmp_notify_t* )data2;

    if (sal_strlen(p_snmp_notify1->key.notify_name) != sal_strlen(p_snmp_notify2->key.notify_name))
    {
        return sal_strlen(p_snmp_notify1->key.notify_name)-sal_strlen(p_snmp_notify2->key.notify_name);
    }

    return sal_strcmp(p_snmp_notify1->key.notify_name, p_snmp_notify2->key.notify_name);
}

int32
tbl_snmp_target_addr_cmp(void *data1, void *data2)
{
    tbl_snmp_target_addr_t *p_snmp_target_addr1 = (tbl_snmp_target_addr_t* )data1;
    tbl_snmp_target_addr_t *p_snmp_target_addr2 = (tbl_snmp_target_addr_t* )data2;

    if (sal_strlen(p_snmp_target_addr1->key.taraddr_name) != sal_strlen(p_snmp_target_addr2->key.taraddr_name))
    {
        return sal_strlen(p_snmp_target_addr1->key.taraddr_name)-sal_strlen(p_snmp_target_addr2->key.taraddr_name);
    }

    return sal_strcmp(p_snmp_target_addr1->key.taraddr_name, p_snmp_target_addr2->key.taraddr_name);
}

int32
tbl_snmp_target_params_cmp(void *data1, void *data2)
{
    tbl_snmp_target_params_t *p_snmp_target_param1 = (tbl_snmp_target_params_t* )data1;
    tbl_snmp_target_params_t *p_snmp_target_param2 = (tbl_snmp_target_params_t* )data2;

    if (sal_strlen(p_snmp_target_param1->key.params_name) != sal_strlen(p_snmp_target_param2->key.params_name))
    {
        return sal_strlen(p_snmp_target_param1->key.params_name)-sal_strlen(p_snmp_target_param2->key.params_name);
    }

    return sal_strcmp(p_snmp_target_param1->key.params_name, p_snmp_target_param2->key.params_name);
}

int32
tbl_rmon_event_cmp(void *data1, void *data2)
{
    tbl_rmon_event_t *p_result1 = (tbl_rmon_event_t*)data1;
    tbl_rmon_event_t *p_result2 = (tbl_rmon_event_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_rmon_event_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_alarm_cmp(void *data1, void *data2)
{
    tbl_rmon_alarm_t *p_result1 = (tbl_rmon_alarm_t*)data1;
    tbl_rmon_alarm_t *p_result2 = (tbl_rmon_alarm_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_rmon_alarm_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_ether_stats_cmp(void *data1, void *data2)
{
    tbl_rmon_ether_stats_t *p_result1 = (tbl_rmon_ether_stats_t*)data1;
    tbl_rmon_ether_stats_t *p_result2 = (tbl_rmon_ether_stats_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_rmon_ether_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_history_cmp(void *data1, void *data2)
{
    tbl_rmon_history_t *p_result1 = (tbl_rmon_history_t*)data1;
    tbl_rmon_history_t *p_result2 = (tbl_rmon_history_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_rmon_history_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_rmon_stats_history_cmp(void *data1, void *data2)
{
    tbl_rmon_stats_history_t *p_result1 = (tbl_rmon_stats_history_t*)data1;
    tbl_rmon_stats_history_t *p_result2 = (tbl_rmon_stats_history_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_rmon_stats_history_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsla_packet_stats_cmp(void *data1, void *data2)
{
    tbl_ipsla_packet_stats_t *p_result1 = (tbl_ipsla_packet_stats_t*)data1;
    tbl_ipsla_packet_stats_t *p_result2 = (tbl_ipsla_packet_stats_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_ipsla_packet_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_ipsla_test_stats_cmp(void *data1, void *data2)
{
    tbl_ipsla_test_stats_t *p_result1 = (tbl_ipsla_test_stats_t*)data1;
    tbl_ipsla_test_stats_t *p_result2 = (tbl_ipsla_test_stats_t*)data2;

    if (sal_memcmp(&p_result1->key, &p_result2->key, sizeof(tbl_ipsla_packet_stats_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_fea_copp_trap_group_cmp(void *data1, void *data2)
{
    tbl_fea_copp_trap_group_t *p = (tbl_fea_copp_trap_group_t *)data1;
    tbl_fea_copp_trap_group_t *p2 = (tbl_fea_copp_trap_group_t *)data2;

    return (p->key.copp_trap_reason_id - p2->key.copp_trap_reason_id);
}

int32
tbl_oam_session_cmp(void *data1, void *data2)
{
    tbl_oam_session_t *p = (tbl_oam_session_t *)data1;
    tbl_oam_session_t *p2 = (tbl_oam_session_t *)data2;

    return (p->key.session_id - p2->key.session_id);
}

int32
tbl_oam_session_upload_cmp(void *data1, void *data2)
{
    tbl_oam_session_upload_t *p = (tbl_oam_session_upload_t *)data1;
    tbl_oam_session_upload_t *p2 = (tbl_oam_session_upload_t *)data2;

    return (p->key.session_id - p2->key.session_id);
}

int32
tbl_lsp_aps_group_cmp(void *data1, void *data2)
{
    tbl_lsp_aps_group_t *p = (tbl_lsp_aps_group_t *)data1;
    tbl_lsp_aps_group_t *p2 = (tbl_lsp_aps_group_t *)data2;

    if (p->key.group_id > p2->key.group_id)
    {
        return 1;
    }
    else if (p->key.group_id < p2->key.group_id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int32
tbl_pw_aps_group_cmp(void *data1, void *data2)
{
    tbl_pw_aps_group_t *p = (tbl_pw_aps_group_t *)data1;
    tbl_pw_aps_group_t *p2 = (tbl_pw_aps_group_t *)data2;

    if (p->key.group_id > p2->key.group_id)
    {
        return 1;
    }
    else if (p->key.group_id < p2->key.group_id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int32
tbl_lsp_pe_cmp(void *data1, void *data2)
{
    tbl_lsp_pe_t *p  = (tbl_lsp_pe_t*)data1;
    tbl_lsp_pe_t *p2 = (tbl_lsp_pe_t*)data2;
    return sal_memcmp(p->key.name, p2->key.name, (sizeof(char))*LSPNAME_SIZE);
}

int32
tbl_lsp_p_cmp(void *data1, void *data2)
{
    tbl_lsp_p_t *p  = (tbl_lsp_p_t*)data1;
    tbl_lsp_p_t *p2 = (tbl_lsp_p_t*)data2;
    return sal_memcmp(p->key.name, p2->key.name, (sizeof(char))*LSPNAME_SIZE);
}

int32
tbl_pw_tpe_cmp(void *data1, void *data2)
{
    tbl_pw_tpe_t *p  = (tbl_pw_tpe_t*)data1;
    tbl_pw_tpe_t *p2 = (tbl_pw_tpe_t*)data2;
    return sal_memcmp(p->key.name, p2->key.name, (sizeof(char))*LSPNAME_SIZE);
}

int32
tbl_section_oam_cmp(void *data1, void *data2)
{
    tbl_section_oam_t *p  = (tbl_section_oam_t*)data1;
    tbl_section_oam_t *p2 = (tbl_section_oam_t*)data2;
    return (p->key.ifindex - p2->key.ifindex);
}

int32
tbl_oam_flow_info_cmp(void *data1, void *data2)
{
    tbl_oam_flow_info_t *p = (tbl_oam_flow_info_t *)data1;
    tbl_oam_flow_info_t *p2 = (tbl_oam_flow_info_t *)data2;

    return (p->key.flow_id - p2->key.flow_id);
}

int32
tbl_aps_group_info_cmp(void *data1, void *data2)
{
    tbl_aps_group_info_t *p = (tbl_aps_group_info_t *)data1;
    tbl_aps_group_info_t *p2 = (tbl_aps_group_info_t *)data2;

    if (p->key.aps_group_id > p2->key.aps_group_id)
    {
        return 1;
    }
    else if (p->key.aps_group_id < p2->key.aps_group_id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int32
tbl_oam_inlabel_info_cmp(void *data1, void *data2)
{
    tbl_oam_inlabel_info_t *p = (tbl_oam_inlabel_info_t *)data1;
    tbl_oam_inlabel_info_t *p2 = (tbl_oam_inlabel_info_t *)data2;

    return (p->key.inlabel- p2->key.inlabel);
}

int32
tbl_oam_mpls_aps_check_cmp(void *data1, void *data2)
{
    tbl_oam_mpls_aps_check_t *p = (tbl_oam_mpls_aps_check_t *)data1;
    tbl_oam_mpls_aps_check_t *p2 = (tbl_oam_mpls_aps_check_t *)data2;

    return sal_memcmp(p->key.name, p2->key.name, (sizeof(char))*LSPNAME_SIZE);
}

int32
tbl_mrt_cmp(void *data1, void *data2)
{
    tbl_mrt_t *p_mrt1 = (tbl_mrt_t*)data1;
    tbl_mrt_t *p_mrt2 = (tbl_mrt_t*)data2;

    return (sal_memcmp(&p_mrt1->key, &p_mrt2->key, sizeof(tbl_mrt_key_t)));
}

int32
tbl_ipmc_intf_cmp(void *data1, void *data2)
{
    tbl_ipmc_intf_t *p_intf1 = (tbl_ipmc_intf_t*)data1;
    tbl_ipmc_intf_t *p_intf2 = (tbl_ipmc_intf_t*)data2;

    return (sal_memcmp(&p_intf1->key, &p_intf2->key, sizeof(tbl_ipmc_intf_key_t)));
}
    
int32
tbl_pim_static_rp_cmp(void *data1, void *data2)
{
    tbl_pim_static_rp_t *p_rp1 = (tbl_pim_static_rp_t*)data1;
    tbl_pim_static_rp_t *p_rp2 = (tbl_pim_static_rp_t*)data2;

    return (sal_memcmp(&p_rp1->key, &p_rp2->key, sizeof(tbl_pim_static_rp_key_t)));
}

int32
tbl_vlan_mapping_table_cmp(void *data1, void *data2)
{
    tbl_vlan_mapping_table_t *p_vm_table1 = (tbl_vlan_mapping_table_t*)data1;
    tbl_vlan_mapping_table_t *p_vm_table2 = (tbl_vlan_mapping_table_t*)data2;

    return (sal_memcmp(&p_vm_table1->key, &p_vm_table2->key, sizeof(tbl_vlan_mapping_table_key_t)));
}

int32
tbl_vlan_mapping_entry_cmp(void *data1, void *data2)
{
    tbl_vlan_mapping_entry_t *p_vm_entry1 = (tbl_vlan_mapping_entry_t*)data1;
    tbl_vlan_mapping_entry_t *p_vm_entry2 = (tbl_vlan_mapping_entry_t*)data2;

    return (sal_memcmp(&p_vm_entry1->key, &p_vm_entry2->key, sizeof(tbl_vlan_mapping_entry_key_t)));
}

int32
tbl_egress_mapping_entry_cmp(void *data1, void *data2)
{
    tbl_egress_mapping_entry_t *p_vm_entry1 = (tbl_egress_mapping_entry_t*)data1;
    tbl_egress_mapping_entry_t *p_vm_entry2 = (tbl_egress_mapping_entry_t*)data2;

    return (sal_memcmp(&p_vm_entry1->key, &p_vm_entry2->key, sizeof(tbl_egress_mapping_entry_key_t)));
}

int32
tbl_vlan_group_cmp(void *data1, void *data2)
{
    tbl_vlan_group_t *p_vlan_group1 = (tbl_vlan_group_t*)data1;
    tbl_vlan_group_t *p_vlan_group2 = (tbl_vlan_group_t*)data2;

    return (sal_memcmp(&p_vlan_group1->key, &p_vlan_group2->key, sizeof(tbl_vlan_group_key_t)));
}

int32
tbl_evc_entry_cmp(void *data1, void *data2)
{
    tbl_evc_entry_t *p_evc_entry1 = (tbl_evc_entry_t*)data1;
    tbl_evc_entry_t *p_evc_entry2 = (tbl_evc_entry_t*)data2;

    return (sal_memcmp(&p_evc_entry1->key, &p_evc_entry2->key, sizeof(tbl_evc_entry_key_t)));
}

int32
tbl_mapped_vlan_info_cmp(void *data1, void *data2)
{
    tbl_mapped_vlan_info_t *p_mapped_info1 = (tbl_mapped_vlan_info_t*)data1;
    tbl_mapped_vlan_info_t *p_mapped_info2 = (tbl_mapped_vlan_info_t*)data2;

    return (sal_memcmp(&p_mapped_info1->key, &p_mapped_info2->key, sizeof(tbl_mapped_vlan_info_key_t)));
}

int32
tbl_l2_protocol_cmp(void *data1, void *data2)
{
    tbl_l2_protocol_t *p_l2_protocol1 = (tbl_l2_protocol_t*)data1;
    tbl_l2_protocol_t *p_l2_protocol2 = (tbl_l2_protocol_t*)data2;

    return sal_memcmp(&p_l2_protocol1->key, &p_l2_protocol2->key, sizeof(tbl_l2_protocol_key_t));
}

int32
ds_l2protocol_cmp(void *data1, void *data2)
{
    ds_l2protocol_t *p_ds_l2pro1 = (ds_l2protocol_t*)data1;
    ds_l2protocol_t *p_ds_l2pro2 = (ds_l2protocol_t*)data2;
    return sal_memcmp(&p_ds_l2pro1->key, &p_ds_l2pro2->key, sizeof(ds_l2protocol_key_t));
}

int32
tbl_nd_fib_cmp(void *data1, void *data2)
{
    tbl_nd_fib_t *p = (tbl_nd_fib_t *)data1;
    tbl_nd_fib_t *p2 = (tbl_nd_fib_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(addr_t));
}

int32
ds_nd_prefix_cmp(void *data1, void *data2)
{
    ds_nd_prefix_t *p = (ds_nd_prefix_t *)data1;
    ds_nd_prefix_t *p2 = (ds_nd_prefix_t *)data2;

    if (p->key.ifindex != p2->key.ifindex) {
        return p->key.ifindex - p2->key.ifindex;
    }

    return sal_memcmp(&p->key.addr, &p2->key.addr, sizeof(addr_t));
}

int32
tbl_hash_field_profile_cmp(void *data1, void *data2)
{
    tbl_hash_field_profile_t *p_hash_field_profile1 = (tbl_hash_field_profile_t*)data1;
    tbl_hash_field_profile_t *p_hash_field_profile2 = (tbl_hash_field_profile_t*)data2;

    return sal_memcmp(p_hash_field_profile1->key.name, p_hash_field_profile2->key.name,  sizeof(tbl_hash_field_profile_key_t));
}

int32
tbl_hash_value_profile_cmp(void *data1, void *data2)
{
    tbl_hash_value_profile_t *p_hash_value_profile1 = (tbl_hash_value_profile_t*)data1;
    tbl_hash_value_profile_t *p_hash_value_profile2 = (tbl_hash_value_profile_t*)data2;

    return sal_memcmp(p_hash_value_profile1->key.name, p_hash_value_profile2->key.name,  sizeof(tbl_hash_value_profile_key_t));
}

int32
tbl_nat_pool_cmp(void *data1, void *data2)
{
    tbl_nat_pool_t *p = (tbl_nat_pool_t *)data1;
    tbl_nat_pool_t *p2 = (tbl_nat_pool_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_pool_key_t));
}

int32
tbl_nat_session_cmp(void *data1, void *data2)
{
    tbl_nat_session_t *p = (tbl_nat_session_t *)data1;
    tbl_nat_session_t *p2 = (tbl_nat_session_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(nat_session_key_t));
}


int32
tbl_nat_rule_cmp(void *data1, void *data2)
{
    tbl_nat_rule_t *p = (tbl_nat_rule_t *)data1;
    tbl_nat_rule_t *p2 = (tbl_nat_rule_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_rule_t));
}

int32
tbl_nat_session_limit_cmp(void *data1, void *data2)
{
    tbl_nat_session_limit_t *p = (tbl_nat_session_limit_t *)data1;
    tbl_nat_session_limit_t *p2 = (tbl_nat_session_limit_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_session_limit_key_t));
}


int32
tbl_nat_proxy_arp_cmp(void *data1, void *data2)
{
    tbl_nat_proxy_arp_t *p = (tbl_nat_proxy_arp_t *)data1;
    tbl_nat_proxy_arp_t *p2 = (tbl_nat_proxy_arp_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_proxy_arp_key_t));
}

int32
tbl_nat_rtif_inside_cmp(void *data1, void *data2)
{
    tbl_nat_rtif_inside_t *p = (tbl_nat_rtif_inside_t *)data1;
    tbl_nat_rtif_inside_t *p2 = (tbl_nat_rtif_inside_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_rtif_inside_key_t));
}

int32
tbl_nat_rtif_outside_cmp(void *data1, void *data2)
{
    tbl_nat_rtif_outside_t *p = (tbl_nat_rtif_outside_t *)data1;
    tbl_nat_rtif_outside_t *p2 = (tbl_nat_rtif_outside_t *)data2;

    return sal_memcmp(&p->key, &p2->key, sizeof(tbl_nat_rtif_outside_key_t));
}

int32
tbl_g8032_ring_cmp(void *data1, void *data2)
{
    tbl_g8032_ring_t *p_ring1 = (tbl_g8032_ring_t*)data1;
    tbl_g8032_ring_t *p_ring2 = (tbl_g8032_ring_t*)data2;

    return p_ring1->key.ring_id - p_ring2->key.ring_id;
}

int32
tbl_cfm_md_cmp(void *data1, void *data2)
{
    tbl_cfm_md_t *p1 = (tbl_cfm_md_t*)data1;
    tbl_cfm_md_t *p2 = (tbl_cfm_md_t*)data2;

    return sal_strcmp(p1->key.name, p2->key.name);
}

int32
tbl_cfm_ma_cmp(void *data1, void *data2)
{
    tbl_cfm_ma_t *p1 = (tbl_cfm_ma_t*)data1;
    tbl_cfm_ma_t *p2 = (tbl_cfm_ma_t*)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_cfm_ma_t));
}

int32
tbl_cfm_maid_cmp(void *data1, void *data2)
{
    tbl_cfm_maid_t *p1 = (tbl_cfm_maid_t*)data1;
    tbl_cfm_maid_t *p2 = (tbl_cfm_maid_t*)data2;

    return sal_strcmp(p1->key.name, p2->key.name);
}

int32
tbl_cfm_lmep_cmp(void *data1, void *data2)
{
    tbl_cfm_lmep_t *p1 = (tbl_cfm_lmep_t*)data1;
    tbl_cfm_lmep_t *p2 = (tbl_cfm_lmep_t*)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_cfm_lmep_t));
}

int32
tbl_cfm_rmep_cmp(void *data1, void *data2)
{
    tbl_cfm_rmep_t *p1 = (tbl_cfm_rmep_t*)data1;
    tbl_cfm_rmep_t *p2 = (tbl_cfm_rmep_t*)data2;

    return sal_memcmp(&p1->key, &p2->key, sizeof(tbl_cfm_rmep_t));
}

int32
ds_cfm_error_cmp(void *data1, void *data2)
{
    ds_cfm_error_t *p_error1 = (ds_cfm_error_t*)data1;
    ds_cfm_error_t *p_error2 = (ds_cfm_error_t*)data2;

    if (sal_memcmp(&p_error1->key, &p_error2->key, sizeof(ds_cfm_error_t)))
    {
        return TRUE;
    }

    return FALSE;
}

/*add by zhw for mc*/
int32
tbl_pim_sm_cmp(void *data1, void *data2)
{
    tbl_pim_sm_t *p_sm1 = (tbl_pim_sm_t*)data1;
    tbl_pim_sm_t *p_sm2 = (tbl_pim_sm_t*)data2;
 
    return sal_memcmp(&p_sm1->key, &p_sm2->key, sizeof(tbl_pim_sm_t));
}
 
int32
tbl_pim_sm_bsr_cmp(void *data1, void *data2)
{
    tbl_pim_sm_bsr_t *p_sm_bsr1 = (tbl_pim_sm_bsr_t*)data1;
    tbl_pim_sm_bsr_t *p_sm_bsr2 = (tbl_pim_sm_bsr_t*)data2;
 
    return sal_memcmp(&p_sm_bsr1->key, &p_sm_bsr2->key, sizeof(tbl_pim_sm_bsr_t));
}
 
int32
tbl_pim_sm_rp_cmp(void *data1, void *data2)
{
    tbl_pim_sm_rp_t *p_sm_rp1 = (tbl_pim_sm_rp_t*)data1;
    tbl_pim_sm_rp_t *p_sm_rp2 = (tbl_pim_sm_rp_t*)data2;
 
    return sal_memcmp(&p_sm_rp1->key, &p_sm_rp2->key, sizeof(tbl_pim_sm_rp_t));
}

/*add by zhw for pim_sm group_prefix*/
int32
tbl_pim_sm_rp_gp_cmp(void *data1, void *data2)
{
    tbl_pim_sm_rp_gp_t *p_sm_rp_gp1 = (tbl_pim_sm_rp_gp_t*)data1;
    tbl_pim_sm_rp_gp_t *p_sm_rp_gp2 = (tbl_pim_sm_rp_gp_t*)data2;
 
    return sal_memcmp(&p_sm_rp_gp1->key, &p_sm_rp_gp2->key, sizeof(tbl_pim_sm_rp_gp_t));
}
 
 
int32
tbl_pim_dm_cmp(void *data1, void *data2)
{
    tbl_pim_dm_t *p_dm1 = (tbl_pim_dm_t*)data1;
    tbl_pim_dm_t *p_dm2 = (tbl_pim_dm_t*)data2;
 
    return sal_memcmp(&p_dm1->key, &p_dm2->key, sizeof(tbl_pim_dm_t));
}
 
/*add by zhw for igmp*/
int32
tbl_igmp_intf_cmp(void *data1, void *data2)
{
    tbl_igmp_intf_t *p_igmp_intf1 = (tbl_igmp_intf_t*)data1;
    tbl_igmp_intf_t *p_igmp_intf2 = (tbl_igmp_intf_t*)data2;
 
    return sal_memcmp(&p_igmp_intf1->key, &p_igmp_intf2->key, sizeof(tbl_igmp_intf_t));
}
/*end add zhw*/

int32
tbl_ospf_area_stub_cmp(void *data1, void *data2)
{
    tbl_ospf_area_stub_t *p_area1 = (tbl_ospf_area_stub_t*)data1;
    tbl_ospf_area_stub_t *p_area2 = (tbl_ospf_area_stub_t*)data2;

    if (p_area1->key.areaid > p_area2->key.areaid)
    {
        return 1;
    }
    else if (p_area1->key.areaid < p_area2->key.areaid)
    {
        return -1;
    }
    return (p_area1->key.areaid - p_area2->key.areaid);
}

int32
tbl_ospf6_area_stub_cmp(void *data1, void *data2)
{
    tbl_ospf6_area_stub_t *p_area1 = (tbl_ospf6_area_stub_t*)data1;
    tbl_ospf6_area_stub_t *p_area2 = (tbl_ospf6_area_stub_t*)data2;

    if (p_area1->key.areaid > p_area2->key.areaid)
    {
        return 1;
    }
    else if (p_area1->key.areaid < p_area2->key.areaid)
    {
        return -1;
    }
    return (p_area1->key.areaid - p_area2->key.areaid);
}

int32
tbl_ospf_area_nssa_cmp(void *data1, void *data2)
{
    tbl_ospf_area_nssa_t *p_area1 = (tbl_ospf_area_nssa_t*)data1;
    tbl_ospf_area_nssa_t *p_area2 = (tbl_ospf_area_nssa_t*)data2;

    if (p_area1->key.areaid > p_area2->key.areaid)
    {
        return 1;
    }
    else if (p_area1->key.areaid < p_area2->key.areaid)
    {
        return -1;
    }
    return (p_area1->key.areaid - p_area2->key.areaid);
}

int32
tbl_ospf_area_vlink_cmp(void *data1, void *data2)
{
    tbl_ospf_area_vlink_t *p_ospf_area_vlink1 = (tbl_ospf_area_vlink_t*)data1;
    tbl_ospf_area_vlink_t *p_ospf_area_vlink2 = (tbl_ospf_area_vlink_t*)data2;

    return sal_memcmp(&p_ospf_area_vlink1->key, &p_ospf_area_vlink2->key, sizeof(tbl_ospf_area_vlink_key_t));
}
