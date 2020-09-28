
#ifndef __CDB_ACT_FIELD_PRIV_H__
#define __CDB_ACT_FIELD_PRIV_H__

static cdb_field_t act_show_common_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "memory" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "socket" },
};

static cdb_field_t act_set_socket_fields[] = 
{
};

static cdb_field_t act_copy_file_fields[] = 
{
};

static cdb_field_t act_flush_fdb_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "static" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "multicast" },
};

static cdb_field_t act_show_vlan_fields[] = 
{
};

static cdb_field_t act_show_fdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "dynamic" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "static" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "multicast" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "all" },
};

static cdb_field_t act_show_hardware_fdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "dynamic" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "static" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "multicast" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "all" },
};

static cdb_field_t act_flush_macfilter_fields[] = 
{
};

static cdb_field_t act_show_acl_fields[] = 
{
};

static cdb_field_t act_show_acl_udf_fields[] = 
{
};

static cdb_field_t act_show_tap_fields[] = 
{
};

static cdb_field_t act_show_tap_tuple5_fields[] = 
{
};

static cdb_field_t act_clear_tap_tuple5_fields[] = 
{
};

static cdb_field_t act_clear_tap_dpi_stats_fields[] = 
{
};

static cdb_field_t act_show_timerange_info_fields[] = 
{
};

static cdb_field_t act_show_acl_counter_fields[] = 
{
};

static cdb_field_t act_clear_acl_counter_fields[] = 
{
};

static cdb_field_t act_show_fea_copp_trap_group_counter_fields[] = 
{
};

static cdb_field_t act_clear_fea_copp_trap_group_counter_fields[] = 
{
};

static cdb_field_t act_show_copp_class_counter_fields[] = 
{
};

static cdb_field_t act_clear_copp_class_counter_fields[] = 
{
};

static cdb_field_t act_show_acl_worm_filter_counter_fields[] = 
{
};

static cdb_field_t act_clear_acl_worm_filter_counter_fields[] = 
{
};

static cdb_field_t act_show_tap_flow_counter_fields[] = 
{
};

static cdb_field_t act_clear_tap_flow_counter_fields[] = 
{
};

static cdb_field_t act_show_tap_counter_fields[] = 
{
};

static cdb_field_t act_show_stm_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "default" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "vlan" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "ipv6" },
};

static cdb_field_t act_flush_psfdb_fields[] = 
{
};

static cdb_field_t act_show_psfdb_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "static" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "all" },
};

static cdb_field_t act_clear_lacp_counters_fields[] = 
{
};

static cdb_field_t act_clear_stp_fields[] = 
{
};

static cdb_field_t act_show_mlag_fields[] = 
{
};

static cdb_field_t act_clear_mlag_counters_fields[] = 
{
};

static cdb_field_t act_show_ecmp_fields[] = 
{
};

static cdb_field_t act_flush_arp_fields[] = 
{
};

static cdb_field_t act_if_addr_cfg_check_fields[] = 
{
};

static cdb_field_t act_rt_static_max_check_fields[] = 
{
};

static cdb_field_t act_show_arp_fields[] = 
{
};

static cdb_field_t act_show_ip_route_fields[] = 
{
};

static cdb_field_t act_show_ipsg_fields[] = 
{
};

static cdb_field_t act_show_ip_if_brief_fields[] = 
{
};

static cdb_field_t act_show_ipv6_if_brief_fields[] = 
{
};

static cdb_field_t act_show_ipv6_route_fields[] = 
{
};

static cdb_field_t act_clear_ipv6_route_static_fields[] = 
{
};

static cdb_field_t act_ping_ipv6_fields[] = 
{
};

static cdb_field_t act_ping_mgmt_ipv6_fields[] = 
{
};

static cdb_field_t act_traceroute_fields[] = 
{
};

static cdb_field_t act_traceroute_ipv6_fields[] = 
{
};

static cdb_field_t act_traceroute_mgmt_ipv6_fields[] = 
{
};

static cdb_field_t act_tftp_check_fields[] = 
{
};

static cdb_field_t act_ftp_check_fields[] = 
{
};

static cdb_field_t act_ssh_check_fields[] = 
{
};

static cdb_field_t act_telnet_check_fields[] = 
{
};

static cdb_field_t act_flush_arp_statistics_fields[] = 
{
};

static cdb_field_t act_show_add_failed_fib_fields[] = 
{
};

static cdb_field_t act_show_hsrv_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "task" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "message" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "cpu" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "interface" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "nexthop" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "vlan" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "fdb" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "l2mc" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "storm_control" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "vlan_mapping" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "ipv4" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "ipv6" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "ecmpv4" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "ecmpv6" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "peth" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "kmpeth" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "acl" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "qos" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "mpls" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "mplstp" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "mplstp_oam" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "tunnel" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "sflow" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "brg" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "openflow" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "tap" },
};

static cdb_field_t act_clear_hsrv_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "peth" },
};

static cdb_field_t act_dump_cdb_fields[] = 
{
};

static cdb_field_t act_dump_ccs_subscriber_fields[] = 
{
};

static cdb_field_t act_dump_cds_subscriber_fields[] = 
{
};

static cdb_field_t act_update_if_stats_fields[] = 
{
};

static cdb_field_t act_clear_if_stats_rate_fields[] = 
{
};

static cdb_field_t act_show_bandwidth_usage_fields[] = 
{
};

static cdb_field_t act_terminal_monitor_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "on" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "off" },
};

static cdb_field_t act_clear_qos_stats_fields[] = 
{
};

static cdb_field_t act_get_qos_queuemap_fields[] = 
{
};

static cdb_field_t act_show_resource_fields[] = 
{
};

static cdb_field_t act_clear_openflow_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "flow" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "meters" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "groups" },
};

static cdb_field_t act_show_openflow_fields[] = 
{
};

static cdb_field_t act_sync_openflow_db_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "reset" },
};

static cdb_field_t act_openflow_set_controller_fields[] = 
{
};

static cdb_field_t act_clock_fields[] = 
{
};

static cdb_field_t act_update_epld_fields[] = 
{
};

static cdb_field_t act_update_bootrom_fields[] = 
{
};

static cdb_field_t act_generate_oem_file_fields[] = 
{
};

static cdb_field_t act_update_oem_file_fields[] = 
{
};

static cdb_field_t act_bootupdiag_fields[] = 
{
};

static cdb_field_t act_update_phy_firmware_fields[] = 
{
};

static cdb_field_t act_dhcsnooping_db_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "save" },
    { CDB_ACCESS_RW,  CDB_DT_NONE,       -1,                  "load" },
};

static cdb_field_t act_errdisable_show_fields[] = 
{
};

static cdb_field_t act_web_login_fields[] = 
{
};

static cdb_field_t act_show_login_security_fields[] = 
{
};

static cdb_field_t act_reboot_fields[] = 
{
};

static cdb_field_t act_show_schedule_reboot_fields[] = 
{
};

static cdb_field_t act_send_trap_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "write_success" },
    { CDB_ACCESS_RD,  CDB_DT_NONE,       -1,                  "write_failure" },
};

static cdb_field_t act_update_port_connection_fields[] = 
{
};

static cdb_field_t act_update_oem_info_fields[] = 
{
};

static cdb_field_t act_dump_cdb_ds_fields[] = 
{
};

static cdb_field_t act_show_mirror_fields[] = 
{
};

static cdb_field_t act_show_mirror_mac_escape_fields[] = 
{
};

static cdb_field_t act_show_environment_fields[] = 
{
};

static cdb_field_t act_show_psu_pmbus_info_fields[] = 
{
};

static cdb_field_t act_show_vcm_fields[] = 
{
};

static cdb_field_t act_show_interface_fields[] = 
{
};

static cdb_field_t act_show_poe_fields[] = 
{
};

static cdb_field_t act_show_isolation_fields[] = 
{
};

static cdb_field_t act_show_pvlan_fields[] = 
{
};

static cdb_field_t act_show_pfc_fields[] = 
{
};

static cdb_field_t act_ptf_sai_fields[] = 
{
};

static cdb_field_t act_show_lldp_local_fields[] = 
{
};

static cdb_field_t act_show_lldp_neighbor_fields[] = 
{
};

static cdb_field_t act_show_lldp_statistics_fields[] = 
{
};

static cdb_field_t act_clear_lldp_statistics_fields[] = 
{
};

static cdb_field_t act_show_erps_list_fields[] = 
{
};

static cdb_field_t act_show_erps_fields[] = 
{
};

static cdb_field_t act_clear_erps_counters_fields[] = 
{
};

static cdb_field_t act_show_acl_policy_counters_fields[] = 
{
};

static cdb_field_t act_show_igsp_fields[] = 
{
};

static cdb_field_t act_clear_igsp_fields[] = 
{
};

static cdb_field_t act_show_radius_server_fields[] = 
{
};

static cdb_field_t act_show_dot1x_fields[] = 
{
};

static cdb_field_t act_clear_dot1x_fields[] = 
{
};

static cdb_field_t act_show_macfilter_fields[] = 
{
};

static cdb_field_t act_change_username_fields[] = 
{
};

static cdb_field_t act_check_l3if_exist_fields[] = 
{
};

static cdb_field_t act_check_l3if_addr_fields[] = 
{
};

static cdb_field_t act_show_vlanclass_rule_fields[] = 
{
};

static cdb_field_t act_show_vlanclass_group_fields[] = 
{
};

static cdb_field_t act_show_vlanclass_if_fields[] = 
{
};

static cdb_field_t act_show_mstp_fields[] = 
{
};

static cdb_field_t act_show_inst_state_fields[] = 
{
};

static cdb_field_t act_show_hybrid_stp_fields[] = 
{
};

static cdb_field_t act_sync_systime_fields[] = 
{
};

static cdb_field_t act_show_protected_vlan_fields[] = 
{
};

static cdb_field_t act_show_memory_threshold_fields[] = 
{
};

static cdb_field_t act_show_cpu_mem_history_fields[] = 
{
};

static cdb_field_t act_show_openflow_resource_fields[] = 
{
};

static cdb_field_t act_build_config_fields[] = 
{
};

static cdb_field_t act_show_arpinsp_fields[] = 
{
};

static cdb_field_t act_show_arpacl_fields[] = 
{
};

static cdb_field_t act_clear_arpinsp_fields[] = 
{
};

static cdb_field_t act_show_ptp_fields[] = 
{
};

static cdb_field_t act_show_fea_ptp_fields[] = 
{
};

static cdb_field_t act_clear_ptp_fields[] = 
{
};

static cdb_field_t act_show_lic_fields[] = 
{
};

static cdb_field_t act_show_mib_stats_fields[] = 
{
};

static cdb_field_t act_show_sflow_fields[] = 
{
};

static cdb_field_t act_check_mgmt_if_ip_fields[] = 
{
};

static cdb_field_t act_show_copp_reason_fields[] = 
{
};

static cdb_field_t act_show_dhclient_fields[] = 
{
};

static cdb_field_t act_show_combo_fields[] = 
{
};

static cdb_field_t act_carrier_blink_down_fields[] = 
{
};

static cdb_field_t act_show_monitor_fields[] = 
{
};

static cdb_field_t act_show_policy_map_fields[] = 
{
};

static cdb_field_t act_show_class_map_fields[] = 
{
};

static cdb_field_t act_show_management_if_fields[] = 
{
};

static cdb_field_t act_acl_iotime_fields[] = 
{
};

static cdb_field_t act_factory_config_fields[] = 
{
};

static cdb_field_t act_show_vrrp_fields[] = 
{
};

static cdb_field_t act_l3if_pre_down_fields[] = 
{
};

static cdb_field_t act_show_ssm_fields[] = 
{
};

static cdb_field_t act_show_ipsla_fields[] = 
{
};

static cdb_field_t act_clear_ipsla_stats_fields[] = 
{
};

static cdb_field_t act_show_track_fields[] = 
{
};

static cdb_field_t act_show_fiber_fields[] = 
{
};

static cdb_field_t act_warmboot_fields[] = 
{
};

static cdb_field_t act_show_tpoam_fields[] = 
{
};

static cdb_field_t act_clear_tpoam_fields[] = 
{
};

static cdb_field_t act_exec_tpoam_fields[] = 
{
};

static cdb_field_t act_get_auth_fields[] = 
{
};

static cdb_field_t act_show_rmon_event_fields[] = 
{
};

static cdb_field_t act_show_rmon_alarm_fields[] = 
{
};

static cdb_field_t act_show_rmon_ether_stats_fields[] = 
{
};

static cdb_field_t act_show_rmon_history_fields[] = 
{
};

static cdb_field_t act_line_acl_check_fields[] = 
{
};

static cdb_field_t act_show_g8131_fields[] = 
{
};

static cdb_field_t act_show_oam_packet_in_stats_fields[] = 
{
};

static cdb_field_t act_clear_oam_packet_in_stats_fields[] = 
{
};

static cdb_field_t act_show_openflow_flow_match_info_fields[] = 
{
};

static cdb_field_t act_show_vlan_mapping_table_fields[] = 
{
};

static cdb_field_t act_show_evc_entry_fields[] = 
{
};

static cdb_field_t act_show_l2_protocol_fields[] = 
{
};

static cdb_field_t act_show_dot1q_ethertype_fields[] = 
{
};

static cdb_field_t act_show_nd_fields[] = 
{
};

static cdb_field_t act_clear_nd_fields[] = 
{
};

static cdb_field_t act_check_openflow_fields[] = 
{
};

static cdb_field_t act_cpu_mirror_fields[] = 
{
};

static cdb_field_t act_show_hsrv_tunnel_fields[] = 
{
};

static cdb_field_t act_show_tacacs_server_fields[] = 
{
};

static cdb_field_t act_show_dhclient6_fields[] = 
{
};

static cdb_field_t act_dhcp_l3if_pre_delete_fields[] = 
{
};

static cdb_field_t act_dhcpv6_l3if_pre_delete_fields[] = 
{
};

static cdb_field_t act_dhcp_l3if_pre_shutdown_fields[] = 
{
};

static cdb_field_t act_dhcpv6_l3if_pre_shutdown_fields[] = 
{
};

static cdb_field_t act_show_hash_field_fields[] = 
{
};

static cdb_field_t act_show_hash_value_fields[] = 
{
};

static cdb_field_t act_clear_rpcapi_counter_fields[] = 
{
};

static cdb_field_t act_show_nat_session_fields[] = 
{
};

static cdb_field_t act_clear_nat_session_fields[] = 
{
};

static cdb_field_t act_show_g8032_fields[] = 
{
};

static cdb_field_t act_update_sw_version_fields[] = 
{
};

static cdb_field_t act_show_cfm_domain_fields[] = 
{
};

static cdb_field_t act_show_cfm_cc_config_fields[] = 
{
};

static cdb_field_t act_show_cfm_errors_fields[] = 
{
};

static cdb_field_t act_clear_cfm_errors_fields[] = 
{
};

static cdb_field_t act_show_cfm_mep_fields[] = 
{
};

static cdb_field_t act_show_cfm_fields[] = 
{
};

static cdb_field_t act_show_queue_drop_fields[] = 
{
};

static cdb_field_t act_show_ip_mroute_fields[] = 
{
};

#endif /* !__CDB_ACT_FIELD_PRIV_H__ */

