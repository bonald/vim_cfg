#ifndef __CDB_DATA_CMP_H__
#define __CDB_DATA_CMP_H__

int32
tbl_auth_session_cmp(void *data1, void *data2);

int32
tbl_auth_server_cmp(void *data1, void *data2);

int32
tbl_dhcbinding_cmp(void *data1, void *data2);

int32
ds_circuit_id_cmp(void *data1, void *data2);

int32
tbl_snmp_view_cmp(void *data1, void *data2);

int32
tbl_snmp_community_cmp(void *data1, void *data2);

int32
tbl_login_cmp(void *data1, void *data2);

int32
tbl_authen_cmp(void *data1, void *data2);

int32
tbl_login_record_cmp(void *data1, void *data2);

int32
tbl_user_cmp(void *data1, void *data2);

int32
tbl_rsa_cmp(void *data1, void *data2);

int32
tbl_fea_g8032_ring_cmp(void *data1, void *data2);

int32
tbl_fea_g8032_ring_mc_group_cmp(void *data1, void *data2);

int32
tbl_tap_five_tuple_cmp(void *data1, void *data2);

int32
tbl_tap_group_cmp(void *data1, void *data2);

int32
tbl_tap_group_ingress_cmp(void *data1, void *data2);

int32
tbl_tap_group_ingress_flow_cmp(void *data1, void *data2);

int32
tbl_tap_group_egress_cmp(void *data1, void *data2);

int32
tbl_interface_cmp(void *data1, void *data2);

int32
tbl_qos_mpls_domain_cmp(void *data1, void *data2);

int32
tbl_qos_queue_smart_buffer_cmp(void *data1, void *data2);

int32
tbl_aclqos_if_cmp(void *data1, void *data2);

int32
tbl_aclqos_if_stats_cmp(void *data1, void *data2);

int32
tbl_fea_port_policer_apply_cmp(void *data1, void *data2);

int32
tbl_macfilter_cmp(void *data1, void *data2);

int32
tbl_cpu_utilization_cmp(void *data1, void *data2);

int32
tbl_cpu_limit_cmp(void *data1, void *data2);

int32
tbl_pvlan_cmp(void *data1, void *data2);

int32
tbl_snmp_trap_cmp(void *data1, void *data2);

int32
tbl_ntp_server_cmp(void *data1, void *data2);
int32
tbl_ntp_ace_cmp(void *data1, void *data2);
int32
tbl_ntp_key_cmp(void *data1, void *data2);

int32
tbl_snmp_inform_cmp(void *data1, void *data2);

int32
tbl_static_dns_cmp(void *data1, void *data2);

int32
tbl_dynamic_dns_domain_cmp(void *data1, void *data2);

int32
tbl_dynamic_dns_server_cmp(void *data1, void *data2);

int32
tbl_acl_cmp(void *data1, void *data2);

int32
ds_pmap_class_cmp(void *data1, void *data2);

int32
ds_mac_ace_cmp(void *data1, void *data2);

int32
tbl_fdb_cmp(void *p_arg1, void *p_arg2);

int32
tbl_ipsg_s_ip_cmp(void *p_arg1, void *p_arg2);

int32
tbl_ipsg_s_mac_cmp(void *p_arg1, void *p_arg2);

int32
tbl_ipsg_fib_cmp(void *p_arg1, void *p_arg2);

int32
ds_ip_ace_cmp(void *data1, void *data2);

int32
ds_pmap_if_cmp(void *data1, void *data2);

int32
tbl_pmap_cmp(void *data1, void *data2);

int32
tbl_cmap_cmp(void *data1, void *data2);

int32
tbl_pmap_if_cmp(void *data1, void *data2);

int32
tbl_time_range_cmp(void *data1, void *data2);

int32
tbl_acl_config_cmp(void *data1, void *data2);

int32
tbl_route_if_cmp(void *data1, void *data2);

int32
tbl_nexthop_group_cmp(void *data1, void *data2);

int32
tbl_rip_cmp(void *data1, void *data2);

int32
tbl_rip_network_cmp(void *data1, void *data2);

int32
tbl_rip_neighbor_cmp(void *data1, void *data2);

int32
tbl_rip_passive_if_cmp(void *data1, void *data2);

int32
tbl_ripng_cmp(void *data1, void *data2);

int32
tbl_ripng_network_cmp(void *data1, void *data2);

int32
tbl_ripng_passive_if_cmp(void *data1, void *data2);


int32
tbl_nexthop_group_routed_cmp(void *data1, void *data2);

int32
tbl_bgp_cmp(void *data1, void *data2);

int32
tbl_bgp_network_cmp(void *data1, void *data2);

int32
tbl_bgp_neighbor_cmp(void *data1, void *data2);

int32
tbl_ospf_cmp(void *data1, void *data2);

int32
tbl_ospf6_cmp(void *data1, void *data2);

int32
tbl_ospf6_interface_cmp(void *data1, void *data2);

int32
tbl_ospf_network_cmp(void *data1, void *data2);

int32
tbl_ospf_area_auth_cmp(void *data1, void *data2);

int32
tbl_ospf_area_stub_cmp(void *data1, void *data2);

int32
tbl_ospf6_area_stub_cmp(void *data1, void *data2);

int32
tbl_ospf_area_nssa_cmp(void *data1, void *data2);

int32
tbl_ospf_area_vlink_cmp(void *data1, void *data2);

int32
tbl_kernel_if_cmp(void *data1, void *data2);

int32
tbl_arp_cmp(void *data1, void *data2);

int32
tbl_iproute_node_cmp(void *data1, void *data2);

int32
tbl_iproute_node_add_fail_cmp(void *data1, void *data2);

int32
tbl_iproute_node_add_fail_count_cmp(void *data1, void *data2);

int32
tbl_static_route_cfg_cmp(void *data1, void *data2);

int32
tbl_fea_port_if_cmp(void *data1, void *data2);

int32
tbl_rif_cmp(void *data1, void *data2);

int32
tbl_mstp_port_cmp(void *data1, void *data2);

int32
ds_pvlan_community_cmp(void *data1, void *data2);

int32
ds_connected_cmp(void *data1, void *data2);

int32
ds_connected_v6_cmp(void *data1, void *data2);

int32
ds_static_route_dup_ipv4_nh_cmp(void *data1, void *data2);

int32
ds_ospf_auth_cmp(void *data1, void *data2);

int32
tbl_ospf_area_range_cmp(void *data1, void *data2);

int32
tbl_ospf6_area_range_cmp(void *data1, void *data2);

int32
tbl_fan_cmp(void *data1, void *data2);

int32
tbl_psu_cmp(void *data1, void *data2); 

int32
tbl_led_cmp(void *data1, void *data2);

int32
tbl_sensor_cmp(void *data1, void *data2);
 
int32
tbl_vcm_cmp(void *data1, void *data2);

int32
tbl_poe_cmp(void *data1, void *data2);

int32
tbl_reboot_info_cmp(void *data1, void *data2);

int32
tbl_ns_port_forwarding_cmp(void *data1, void *data2);

int32
tbl_cem_cmp(void *data1, void *data2);

int32
tbl_mstp_instance_cmp(void *data1, void *data2);

int32
tbl_msti_port_cmp(void *p_arg1, void *p_arg2);

int32
tbl_mlag_port_cmp(void *p_arg1, void *p_arg2);

int32
tbl_port_stats_cmp(void *p_arg1, void *p_arg2);

int32
tbl_port_stats_rate_cmp(void *data1, void *data2);

int32
tbl_acl_worm_filter_cmp(void *data1, void *data2);

int32
tbl_ace_config_cmp(void *data1, void *data2);

int32
tbl_acl_entry_cmp(void *data1, void *data2);

int32
tbl_policy_map_config_cmp(void *data1, void *data2);

int32
tbl_class_map_config_cmp(void *data1, void *data2);

int32
tbl_class_in_policy_config_cmp(void *data1, void *data2);

int32
tbl_acl_in_class_config_cmp(void *data1, void *data2);

int32
tbl_class_map_action_config_cmp(void *data1, void *data2);

int32
tbl_lldp_if_cmp(void *data1, void *data2);

int32
ds_lldp_neighbour_cmp(void *data1, void *data2);

int32
tbl_erps_ring_cmp(void *data1, void *data2);

int32
ds_mrouter_port_cmp(void *data1, void *data2);

int32
ds_query_cmp(void *data1, void *data2);

int32
ds_join_group_cmp(void *data1, void *data2);

int32
tbl_dot1x_port_cmp(void *data1, void *data2);

int32
tbl_dot1x_radius_cmp(void *data1, void *data2);

int32
tbl_sflow_collector_cmp(void *data1, void *data2);

int32
tbl_sflow_counter_port_cmp(void *data1, void *data2);

int32
tbl_dot1x_mac_cmp(void *data1, void *data2);

int32
tbl_author_cmp(void *data1, void *data2);

int32
tbl_account_cmp(void *data1, void *data2);

int32
tbl_accountcmd_cmp(void *data1, void *data2);

int32
tbl_vlanclass_rule_cmp(void *data1, void *data2);

int32
tbl_vlanclass_group_cmp(void *data1, void *data2);

int32
tbl_mirror_mac_escape_cmp(void *data1, void *data2);

int32
tbl_controller_cmp(void *data1, void *data2);

int32
tbl_ns_route_cmp(void *p_arg1, void *p_arg2);

int32
tbl_ns_route_ip_cmp(void *p_arg1, void *p_arg2);
int32
tbl_openflow_interface_cmp(void *data1, void *data2);

int32
tbl_openflow_tunnel_interface_cmp(void *data1, void *data2);

int32
tbl_openflow_manager_cmp(void *p_arg1, void *p_arg2);

int32
tbl_ptp_port_cmp(void *data1, void *data2);

int32
tbl_ptp_foreign_cmp(void *data1, void *data2);

int32
tbl_ptp_residence_time_cache_cmp(void *data1, void *data2);

int32
tbl_fea_ptp_ts_cmp(void *data1, void *data2);

int32
tbl_inband_snat_cmp(void *p_arg1, void *p_arg2);

int32
tbl_arpacl_config_cmp(void *data1, void *data2);

int32
tbl_arpace_config_cmp(void *data1, void *data2);

int32
tbl_vrrp_vmac_cmp(void *data1, void *data2);

int32
tbl_vrrp_vip_cmp(void *data1, void *data2);

int32
tbl_vrrp_session_cmp(void *data1, void *data2);

int32
tbl_ssm_port_cmp(void *data1, void *data2);

int32
tbl_snmp_context_cmp(void *data1, void *data2);

int32
tbl_snmp_usm_user_cmp(void *data1, void *data2);

int32
tbl_snmp_group_usm_cmp(void *data1, void *data2);

int32
tbl_snmp_access_usm_cmp(void *data1, void *data2);

int32
tbl_snmp_notify_cmp(void *data1, void *data2);

int32
tbl_snmp_target_addr_cmp(void *data1, void *data2);

int32
tbl_snmp_target_params_cmp(void *data1, void *data2);

int32
tbl_rmon_event_cmp(void *data1, void *data2);

int32
tbl_rmon_alarm_cmp(void *data1, void *data2);

int32
tbl_rmon_ether_stats_cmp(void *data1, void *data2);

int32
tbl_rmon_history_cmp(void *data1, void *data2);

int32
tbl_rmon_stats_history_cmp(void *data1, void *data2);

int32
tbl_ipsla_packet_stats_cmp(void *data1, void *data2);

int32
tbl_ipsla_test_stats_cmp(void *data1, void *data2);

int32
tbl_fea_copp_trap_group_cmp(void *data1, void *data2);

int32
tbl_oam_session_cmp(void *data1, void *data2);

int32
tbl_oam_session_upload_cmp(void *data1, void *data2);

int32
tbl_lsp_aps_group_cmp(void *data1, void *data2);

int32
tbl_pw_aps_group_cmp(void *data1, void *data2);

int32
tbl_lsp_pe_cmp(void *data1, void *data2);

int32
tbl_lsp_p_cmp(void *data1, void *data2);

int32
tbl_pw_tpe_cmp(void *data1, void *data2);

int32
tbl_section_oam_cmp(void *data1, void *data2);

int32
tbl_oam_flow_info_cmp(void *data1, void *data2);

int32
tbl_aps_group_info_cmp(void *data1, void *data2);

int32
tbl_oam_inlabel_info_cmp(void *data1, void *data2);

int32
tbl_oam_mpls_aps_check_cmp(void *data1, void *data2);

int32
tbl_mrt_cmp(void *data1, void *data2);

int32
tbl_ipmc_intf_cmp(void *data1, void *data2);

int32
tbl_pim_static_rp_cmp(void *data1, void *data2);

int32
tbl_vlan_mapping_table_cmp(void *data1, void *data2);

int32
tbl_vlan_mapping_entry_cmp(void *data1, void *data2);

int32
tbl_egress_mapping_entry_cmp(void *data1, void *data2);

int32
tbl_vlan_group_cmp(void *data1, void *data2);

int32
tbl_evc_entry_cmp(void *data1, void *data2);

int32
tbl_mapped_vlan_info_cmp(void *data1, void *data2);

int32
tbl_l2_protocol_cmp(void *data1, void *data2);

int32
ds_l2protocol_cmp(void *data1, void *data2);

int32
tbl_nd_fib_cmp(void *data1, void *data2);

int32
ds_nd_prefix_cmp(void *data1, void *data2);

int32
tbl_hash_field_profile_cmp(void *data1, void *data2);

int32
tbl_hash_value_profile_cmp(void *data1, void *data2);

int32
tbl_nat_pool_cmp(void *data1, void *data2);

int32
tbl_nat_session_cmp(void *data1, void *data2);

int32
tbl_nat_rule_cmp(void *data1, void *data2);

int32
tbl_nat_session_limit_cmp(void *data1, void *data2);

int32
tbl_nat_proxy_arp_cmp(void *data1, void *data2);

int32
tbl_g8032_ring_cmp(void *data1, void *data2);

int32
tbl_nat_rtif_inside_cmp(void *data1, void *data2);

int32
tbl_nat_rtif_outside_cmp(void *data1, void *data2);

int32
tbl_cfm_md_cmp(void *data1, void *data2);
int32
tbl_cfm_ma_cmp(void *data1, void *data2);

int32
tbl_cfm_maid_cmp(void *data1, void *data2);

int32
tbl_cfm_lmep_cmp(void *data1, void *data2);

int32
tbl_cfm_rmep_cmp(void *data1, void *data2);

int32
ds_cfm_error_cmp(void *data1, void *data2);

/*add by zhw for mc*/
int32
tbl_pim_sm_cmp(void *data1, void *data2);
 
int32
tbl_pim_sm_bsr_cmp(void *data1, void *data2);
 
int32
tbl_pim_sm_rp_cmp(void *data1, void *data2);
 
int32
tbl_pim_sm_rp_gp_cmp(void *data1, void *data2);
 
int32
tbl_pim_dm_cmp(void *data1, void *data2);
 
int32
tbl_igmp_intf_cmp(void *data1, void *data2);
/*end add zhw*/
#endif /* !__CDB_DATA_CMP_H__ */

