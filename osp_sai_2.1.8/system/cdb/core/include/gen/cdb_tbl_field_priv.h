
#ifndef __CDB_TBL_FIELD_PRIV_H__
#define __CDB_TBL_FIELD_PRIV_H__

static cdb_field_t tbl_interface_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "portid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "hostifid" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "desc" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "speed" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "duplex" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "unidir" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS8,     -1,                  "flowcontrol_send" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS8,     -1,                  "flowcontrol_receive" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS8,     -1,                  "flowcontrol_oper_send" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS8,     -1,                  "flowcontrol_oper_receive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fec" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pfc_admin" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pfc_oper" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pfc_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "jumboframe_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "isolate_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_snooping_trust" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "phy_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "phy_loopback_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "internal_lp_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "pvlan_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "private_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "community_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ntp_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ntp_broadcast_client" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pvlan_group_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "support_speed" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "vlan_type" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "up" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "running" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mtu" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "oper_speed" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "prev_oper_speed" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "oper_duplex" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "oper_unidir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oper_fec" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oper_loopback" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bandwidth" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_used" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_ingress_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "tap" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_mark_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mirror_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mirror_igs_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mirror_egs_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mirror_dest_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_up" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_source_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_source_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_source_item_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "unicast_rpf_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_bandwidth" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bandwidth_may_change" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "errdisable_reason" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_occur" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "link_flap_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "static_security_num" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "errdisable_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_flow_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_flow_ingress" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_flow_egress" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_counter_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_sequence" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sflow_sample_pool" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "hw_mac_addr" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "hw_type" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "phy_type" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "media" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_combo" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS8,     -1,                  "phyport_loopback" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_eee" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_poe" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_master" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_slave" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "phyport_unidir" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_100m" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_1g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_10g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_40g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_100g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_2_5g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_5g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_25g" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS8,     -1,                  "support_50g" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_CIRCUIT_ID,       "circuit_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "igs_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "egs_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "igs_ipv6_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "egs_ipv6_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "igs_mac_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "egs_mac_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "igs_policy_map" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "egs_policy_map" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_BRGIF,            "brgif" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_LAG,              "lag" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_LACP,             "lacp" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_STORM_CONTROL,    "storm_control" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_ACLQOS_IF,        "aclqos_if" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_OPENFLOW_IF,      "openflow_if" },
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_DHCLIENT_IF,      "dhclient_if" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_L2PROTOCOL,       "l2protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "crc_error_check_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "crc_recalculation_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erps_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erps_pdu_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1x_enable" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_unauthorized" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_dir_in" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_dir_out" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_mode_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1x_guest_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlanclass_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlanclass_active_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlanclass_default_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1x_cfg_guest_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "egs_acl_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_group_igs_ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_group_egs_ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_flow_decap_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_flow_udf_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "metadata_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_trust_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_numberlimit_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_numberlimit_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_numberlimit_violate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_numberlimit_occur" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_rate_limit_errdisable_occur" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inband_snat_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "latency_event_high" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "latency_event_low" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "latency_event_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reflective_bridge_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "support_ssm_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_tpid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2pro_if_action_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2pro_uplink" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "static_agg_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "static_agg_is_active" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fec_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cl73_ability" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifg_stats_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_operation_agg" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_operation_agg_mem_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "carrier_down_holdtime_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_value_profile_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_value_profile_dir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_value_profile_id_agg" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_value_agg_hash" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "poe" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_g8032_ring_count" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "g8032_ring_first" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cfm_lmep_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cfm_rmep_num" },
};

static cdb_field_t tbl_route_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mtu" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "mode_set_before" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv4_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_retrans_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_timeout" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "kernel_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_retry_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_recv_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_send_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_v2_broadcast" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_split_horizon" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_poisoned_reverse" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rip_auth_mode" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "rip_auth_string" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ripng_split_horizon" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ripng_poisoned_reverse" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_cost" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_mtu_ignore" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_timer_hello" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_timer_dead" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_auth_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ospf_auth_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf_network_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_timer_hello" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_timer_dead" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_mtu_ignore" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_cost" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ospf6_network_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_proxy_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "local_arp_proxy_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "unicast_rpf_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "unicast_rpf_en_v6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "unicast_en_v6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_up_running" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_link_local_had_delete" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_unreachable_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_redirects_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_relay_option_trust" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_server_group" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "dhcp_pdu_enabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_client_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_client_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "dhcp_client_ipv4" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "dhcp_client_ipv4_mask" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_curr_dynamic" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "arp_attack_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_rate_limit_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_rate_limit_pkt_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_rate_limit_violation" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_rate_limit_pkt_curr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_rate_limit_port_abnormal_flag" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vmac1" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vmac2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vmac3" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrrp_session_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_currrent_hop_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_advertise" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_advertise_mtu" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_managed_config" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_other_config" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_interval_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_interval_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_min_delay" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_retransmit_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_router_lifetime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_base_reachable_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_reachable_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_delay_probe_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_dad_attempts" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nd_dst" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nd_last_ra_solicited" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nd_last_ra_unsolicited" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_ocount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_solicited_ocount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_init_unsolicited_ocount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_solicited_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_prefix_default_vlifetime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_prefix_default_plifetime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_prefix_default_onlink" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_ra_prefix_default_auto" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nd_rs_dest_addr" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ra_solicited" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ra_unsolicited" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_delete_all_v6_addr_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcpv6_client_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcpv6_client_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nat_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nat_category_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "set_mirror_enable" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ospf_cost_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_send_arp_timer" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_CONNECTED,        "ifc_ipv4" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_CONNECTED_V6,     "ifc_ipv6" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_OSPF_AUTH,        "ospf_md5_key" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_ND_PREFIX,        "nd_prefix" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "multicast" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "igmp" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "igmp_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "igmp_intf_limit_number" },
};

static cdb_field_t tbl_kernel_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "masklen" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr_prefix_length_v6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrf_id" },
};

static cdb_field_t tbl_fea_port_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "lag_member" },
};

static cdb_field_t tbl_vlan_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_mac_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cur_mac_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "instance" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcsnooping" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpsnooping" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "security_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_learning_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "op_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "member_port" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erps_domain_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1x_guest_vlan_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "g8032_ring_id" },
};

static cdb_field_t tbl_pvlan_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "community_group_id" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_PVLAN_COMMUNITY,  "community_groups" },
    { CDB_ACCESS_RW,  CDB_DT_REFERLIST,  -1,                  "isloate_ports" },
    { CDB_ACCESS_RW,  CDB_DT_REFERLIST,  -1,                  "promiscuous_ports" },
};

static cdb_field_t tbl_fdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "static" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "security" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "hybrid_flow" },
};

static cdb_field_t tbl_mcfdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "l2mc_port" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "action_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "manual" },
};

static cdb_field_t tbl_macfilter_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_psfdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "port_security" },
};

static cdb_field_t tbl_ipsg_s_ip_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
};

static cdb_field_t tbl_ipsg_s_mac_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
};

static cdb_field_t tbl_ipsg_fib_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipsg_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "set_dhcbinding_ipsg_tbl_exsit" },
};

static cdb_field_t tbl_brg_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_static_fdb" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_l2mcast_fdb" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_aging_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oper_fdb_aging_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hw_learning_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipsg_max_port_binding" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cur_ipv4_source_guard" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "port_isolate_mode" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "instance" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pvlan_isolate_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_instance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_pvlan_isolate_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_errdis_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_max_size" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_add_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_disc_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_loop_curr_size" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_delay_up" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_delay_down" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_all_flushing" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_based_vlan_class" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv4_based_vlan_class" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_based_vlan_class" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_mirror_mac_escape" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2protocol_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_mapping_entry_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_tpid_type1" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tpid_type1_ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_tpid_type2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tpid_type2_ref_cnt" },
};

static cdb_field_t tbl_mstp_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "adminPortPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "portPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forward_transitions" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tx_bpdu_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tx_error_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_bpdu_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_error_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_enable" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "bpdu_loop_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "restricted_role" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "restricted_tcn" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "admin_bpduguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_bpduguard" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "admin_bpdufilter" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_bpdufilter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_rootguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_rootguard" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_loopguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_loopguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cisco_cfg_format_id" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "adminP2PMac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "operP2PMac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "agree" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "agreed" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "adminForceVersion" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forceVersion" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "designated_rootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_rootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "designated_regionRootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_intRootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "designated_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_maxAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "disputed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fdbFlush" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forward" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forwarding" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "infoIs" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "learn" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "learning" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mcheck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "newInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "newInfoMsti" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "adminEdge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "operEdge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "portEnabled" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "portId" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_rootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_rootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_regionRootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_intRootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_maxAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "proposed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "proposing" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdBpdu" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "rcvdInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdInfoUpdateTimers" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdMsg" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdMSTP" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdRSTP" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdSTP" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTc" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTcAck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTcn" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reRoot" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reselect" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "role" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "selected" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "selectedRole" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sendRstp" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sync" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "synced" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcAck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcProp" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "txCount" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "updtInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reRooted" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rstpVersion" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stpVersion" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdInternal" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdInternalChange" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "hopCount" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "prx_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "ppm_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "bdm_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "ptx_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "pim_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "prt_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "pst_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "tcm_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fea_state" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "bpdu" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "edgeDelayWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "fdWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "helloWhen_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mdelayWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rbWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rcvdInfoWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rrWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "tcWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "hold_timer" },
    { CDB_ACCESS_RD,  CDB_DT_REFERLIST,  -1,                  "msti_list" },
};

static cdb_field_t tbl_msti_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "adminPortPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "portPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forward_transitions" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "newInfoMsti" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "bpdu_loop_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "restricted_role" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "restricted_tcn" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_rootguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_loopguard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "agree" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "agreed" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "adminForceVersion" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "designated_regionRootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_intRootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "designated_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "designated_maxAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "disputed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fdbFlush" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forward" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "forwarding" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "infoIs" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "learn" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "learning" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "newInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "portId" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_rootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_rootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "port_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_maxAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "proposed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "proposing" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdBpdu" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "rcvdInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdMsg" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdRSTP" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdSTP" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTc" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTcAck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcvdTcn" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reRoot" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reselect" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "role" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "selected" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "selectedRole" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sync" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "synced" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcAck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcProp" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "txCount" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "updtInfo" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reRooted" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "hopCount" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "ppm_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "pim_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "prt_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "pst_state" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "tcm_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fea_state" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "bpdu" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "fdWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mdelayWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rbWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rcvdInfoWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rrWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "tcWhile_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "hold_timer" },
};

static cdb_field_t tbl_mstp_instance_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "instance_oid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "bridge_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_portId" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "root_rootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_rootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "root_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_maxAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "prs_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "hopCount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "g8032_ring_id" },
    { CDB_ACCESS_RD,  CDB_DT_REFERLIST,  -1,                  "port_list" },
};

static cdb_field_t tbl_mstp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "no_mst_config" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "external_root_path_cost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "internal_root_path_cost" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_portId" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "root_rootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_rootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "root_regionRootId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_intRootPathCost" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "root_designatedBridgeId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_designatedPortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_bridgePortId" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_messageAge" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_fwdDelay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_helloTime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_maxAge" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_fwdDelay" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_helloTime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_maxAge" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_maxHops" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "bridge_id" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "last_tc_time" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "num_topo_changes" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "mlag_role" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "topology_change_detected" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bpduguard" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bpdufilter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_cisco" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "oper_cisco" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "txHoldCount" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "pathcost_standard" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tc_protection" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "hopCount" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "recv_tcn_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "recv_tcn_number" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "root_port_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "alternate_port_ifndex" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "prs_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mstp_format_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mstp_revision_level" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "mstp_region_name" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mstp_digest" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "tc_protection_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "of_exclude_all_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "of_include_vlan_bmp" },
};

static cdb_field_t tbl_lldp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "config_flag" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "system_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "management_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_hold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_delay" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reinit_delay" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_capability" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "system_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "system_desc" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "system_desc_default" },
};

static cdb_field_t tbl_lldp_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_error_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rx_discard_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tlv_discard_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tlv_unrecognized_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "aged_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tlv_enable_basic" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tlv_enable_8021" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tlv_enable_8023" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tlv_enable_med" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ppvid_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ppvid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protocol_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "vlan_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_name_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mdi_support" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pse_power_pair" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "power_class" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_capbility" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_dev_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_power_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_power_value" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "med_policy_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "med_lci_format" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "med_lci_coordinate" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "med_lci_elin" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "med_lci_civic" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tx_timer" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_LLDP_NEIGHBOUR,   "neigh_list" },
};

static cdb_field_t tbl_mlag_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "local_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reload_delay_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reload_delay_auto" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "role" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "local_sysid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "remote_sysid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mlag_sysid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "peer_routemac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "local_syspri" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "remote_syspri" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_syspri" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "peer_vlanif_bmp" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "peer_fdb_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "peer_link_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flush_fdb_mi" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "reload_delay_timer" },
};

static cdb_field_t tbl_mlag_peer_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "peer_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "set_timers" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "holdtime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "keepalive" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "server_sock" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "client_sock" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "v_auto_start" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "v_connect" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "v_holdtime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "v_keepalive" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "open_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "open_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "keepalive_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "keepalive_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "disconnect_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "disconnect_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fdbsync_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fdbsync_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "failover_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "failover_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "conf_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "conf_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "syspri_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "syspri_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "peer_fdb_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "peer_fdb_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_global_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_global_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_port_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_port_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_packet_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_packet_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_instance_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_instance_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_state_in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_state_out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "established" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "dropped" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ttl" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "uptime" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "lastread" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mpf_state" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_auto_start" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_holdtime" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_keepalive" },
};

static cdb_field_t tbl_mlag_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "peer_conf" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "peer_if_up" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "stp_port_enable" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "protect_en" },
};

static cdb_field_t tbl_isolation_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "isolation_oid" },
    { CDB_ACCESS_RD,  CDB_DT_REFERLIST,  -1,                  "ports" },
};

static cdb_field_t tbl_route_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "gratuitous_arp_learn_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_pkt_rx_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_pkt_tx_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_pkt_discard_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "arp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "multipath_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_static" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_error_ratelimit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_redirects_global_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_static_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_ecmp_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_ecmp_groups" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_remote_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_host_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_indirect_ipv4_route_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_static_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_ecmp_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_ecmp_v6_groups" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_remote_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "current_indirect_ipv6_route_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_local" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_static" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_connected" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_rip" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_ospf" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "active_bgp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_logbuf_entrynum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_logbuf_curnum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_logbuf_lognum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_logbuf_logsec" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arpinsp_validate_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fib_full" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_hop_limit" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vty_connected" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vty_nonconn_count" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "vty_mode" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "arp_aging_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "arpratelimit_timer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "arpratelimit_timer_start_time_sec" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "arpratelimit_timer_start_time_msec" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "buf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pkt_len" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nd_count" },
};

static cdb_field_t tbl_bgp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "router_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "redistribute" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "red_metric" },
};

static cdb_field_t tbl_bgp_network_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mask" },
};

static cdb_field_t tbl_bgp_neighbor_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "remote_as" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ebgp_multihop" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "password" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "intf_name" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "intf_addr" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "shutdown" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "next_hop_self" },
};

static cdb_field_t tbl_ospf_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "router_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_originate" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "redistribute" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ospf_redistribute_connected_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ospf_redistribute_static_timer" },
};

static cdb_field_t tbl_ospf6_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "router_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_originate" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "redistribute" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ospf6_redistribute_connected_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ospf6_redistribute_static_timer" },
};

static cdb_field_t tbl_ospf6_interface_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "interface" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "area_id" },
};

static cdb_field_t tbl_ospf_network_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "area_id" },
};

static cdb_field_t tbl_ospf_area_auth_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "auth_type" },
};

static cdb_field_t tbl_ospf_area_range_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "advertise" },
};

static cdb_field_t tbl_ospf6_area_range_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "advertise" },
};

static cdb_field_t tbl_iproute_node_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "route_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "distance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "multipath" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_group" },
};

static cdb_field_t tbl_iproute_node_add_fail_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "distance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "multipath" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_group" },
};

static cdb_field_t tbl_iproute_node_add_fail_count_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "distance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "multipath" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_group" },
};

static cdb_field_t tbl_static_route_cfg_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "distance" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "track_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "track_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nexthop_type_v6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_client" },
};

static cdb_field_t tbl_static_rt_cnt_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "refer_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "has_blackhole" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_group" },
};

static cdb_field_t tbl_arp_fib_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vrf_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_router" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "route_ref" },
};

static cdb_field_t tbl_arp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrf_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rif_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "status" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "route_ref" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "uptime" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "aging_delay" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_gratuitous" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_proxy" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_router" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "retry_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_bfd_ref" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_static_route_dup" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_STATIC_ROUTE_DUP_IPV4_NH, "static_route_dup_ipv4_nh" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "retry_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "refresh_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "gra_rsp_timer" },
};

static cdb_field_t tbl_nexthop_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "refcnt" },
};

static cdb_field_t tbl_nexthop_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nh_group" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nhg_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_hash" },
};

static cdb_field_t tbl_nexthop_group_routed_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nhg_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_hash" },
};

static cdb_field_t tbl_fea_nexthop_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nhid" },
};

static cdb_field_t tbl_mrt_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rpf_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "oif_index_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "nhg_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "packet_match" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "byte_match" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "static_mroute" },
};

static cdb_field_t tbl_ipmc_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mcast_route_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "register_suppress_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_packet_process" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "keep_alive_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rp_keep_alive_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "join_prune_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "switchover_mode" },
};

static cdb_field_t tbl_ipmc_intf_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "primary_address" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "igmp_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_enable" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "igmp_version" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "igmp_query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "igmp_query_max_response_time" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "pim_hello_period" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "pim_hello_holdtime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_generation_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_propagation_delay_msec" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_override_interval_msec" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pim_dr_priority" },
};

static cdb_field_t tbl_pim_static_rp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rp_addr" },
};

static cdb_field_t tbl_sys_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "hostname" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "route_mac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mac_num" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "curr_vlanif_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "init_done" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "startup_done" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_frame_size" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "jumbo_frame_size" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "reboot_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipg_shaping_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipg_policing_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipg_storm_control_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_new_model" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "service_password_encryption" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_vty" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_privilege1" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_privilege2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_privilege3" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_service_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_relay_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_snooping_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "http_service_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "http_image_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "errdisable_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trunction_length" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_ts_macda" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_ts_macsa" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ptf_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "telnet_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "telnet_port" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "telnet_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ssh_acl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "http_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "http_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "http_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "https_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "https_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpcapi_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpcapi_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpcapi_ssl_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpcapi_ssl_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpcapi_auth_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oper_rpcapi_auth_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cut_through_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protected_vlan_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_stats_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mib_port_stats_read_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mib_flow_stats_read_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "memory_threshold1" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "memory_threshold2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "memory_threshold3" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_memchk_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mgmt_if_running" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mgmt_if_timer" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "warmboot_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "warmboot_time" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "warmboot_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "system_time_update" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sflow_counter_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcp_server_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_gre_ace_entry_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_vxlan_entry_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_product" },
};

static cdb_field_t tbl_lag_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "system_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_system_priority" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mlag_system_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "load_balance" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "load_balance_inner_field_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_arithmetic" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "load_balance_mode" },
};

static cdb_field_t tbl_ecmp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_algorithm" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "load_balance_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dlb_mode_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rr_prefix_num" },
};

static cdb_field_t tbl_rr_prefix_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rr_prefix" },
};

static cdb_field_t tbl_mem_summary_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "total" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "used" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "free" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "buffer" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cached" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mem_high" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cpu_high" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cpu_high_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cpu_low_threshold" },
};

static cdb_field_t tbl_cpu_mem_history_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "polling_index" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "saving_index" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "hour_start" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "day_start" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "hour_last_save" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "day_last_save" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "cpu_polling_save" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "cpu_history_hour" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "cpu_history_day" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mem_polling_save" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mem_history_hour" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mem_history_day" },
};

static cdb_field_t tbl_chsm_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "chsm_chassis" },
};

static cdb_field_t tbl_switch_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_packet_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_packet_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_protocol_cist" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_protocol_msti" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mlag_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mlag_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mlag_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mlag_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lacp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lacp_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lacp_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lacp_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igsp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igsp_packet_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igsp_packet_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igsp_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igsp_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipsg_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipsg_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lldp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lldp_packet_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lldp_packet_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lldp_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "lldp_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rpcapi_request" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rpcapi_command" },
};

static cdb_field_t tbl_route_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "route_ipv4" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "route_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "route_netlink" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arp_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arp_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arp_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arpinspection_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arpinspection_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arpinspection_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "arpinspection_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vrrp_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vrrp_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vrrp_packet_send" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vrrp_packet_recv" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vrrp_packet_detail" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipsla_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipsla_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "track_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nd_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nd_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nd_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nd_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nat_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nat_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pimsm_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pimsm_pim" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pimsm_igmp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pimsm_timer" },
};

static cdb_field_t tbl_ssm_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ssm_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ssm_packet_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ssm_packet_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ssm_timer" },
};

static cdb_field_t tbl_ptp_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_communication_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_bmc_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_timer_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_synchronization_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_packet_rx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_packet_tx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_packet_discard_debug" },
};

static cdb_field_t tbl_quagga_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rip_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rip_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rip_zebra" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_ism_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_ism_status" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_ism_timers" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_lsa_flooding" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_lsa_generate" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_lsa_install" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_lsa_refresh" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_nsm_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_nsm_status" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_nsm_timers" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_nssa" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_packet_dd" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_packet_hello" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_packet_ls_ack" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_packet_ls_request" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_packet_ls_update" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_zebra_interface" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_zebra_redistribute" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_event_abr" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_event_ia" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_event_route" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ospf_event_spf" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_as4_segment" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_filters" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_fsm" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_keepalives" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_updates_in" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_updates_out" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "bgp_zebra" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igmp_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igmp_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "igmp_trace" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mroute_on" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mroute_detail" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_packetdump_send" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_packetdump_receive" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_trace" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "pim_zebra" },
};

static cdb_field_t tbl_lsrv_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "card" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "port" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "fiber" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "fan" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "psu" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "led" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "thermal" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vcm" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "poe" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "synce" },
};

static cdb_field_t tbl_hsrv_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vswitch" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "interface_l2if" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "interface_l3if" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "interface_ifdb" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "message_pm2hsrv" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "message_hsrv2pm" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_bpdu" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_slowproto" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_eapol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_lldp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_erps" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_macda" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_rip" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_ospf" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_bgp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_arp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_dhcp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_ipda" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_igmp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_maclimit" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_macmismatch" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_l3copycpu" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_other" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_raw" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cpu_ptp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ip_icmp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nexthop" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "vlan" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "mirror" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "fdb" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "l2mc" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "agg" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipuc" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ecmp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "acl" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stats" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "qos_class" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "qos_policer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "qos_process" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "qos_queue" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipsour" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "stp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_pkt_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_pkt_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_cfg" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ptp_interrupt" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipmc" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_common" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_update" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_pkt_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_pkt_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "l2protocol_pkt_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "l2protocol_pkt_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nat_rule" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nat_session" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "nat_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_event" },
};

static cdb_field_t tbl_app_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snmp_agent" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rmon_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rmon_alarm" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rmon_stats" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "rmon_history" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ntp_debug_level" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "netconf_rpc" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "netconf_rpc_clear" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "netconf_debug" },
};

static cdb_field_t tbl_openflow_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "openflow_vconn" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "openflow_rconn" },
};

static cdb_field_t tbl_rif_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rif_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "hostif_id" },
};

static cdb_field_t tbl_fea_lag_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "lag_id" },
};

static cdb_field_t tbl_fea_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "router_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_stats_ms" },
};

static cdb_field_t tbl_fea_acl_table_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "direct_in" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "table_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_table_id" },
};

static cdb_field_t tbl_fea_acl_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_mac_acl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_acl_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_acl_id_ipv4" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_acl_id_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "counter_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "stats_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "stats_byte" },
};

static cdb_field_t tbl_fea_fdb_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_source_ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "portid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fea_fail" },
};

static cdb_field_t tbl_fea_brg_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lag_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mac_learning_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_security_en" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "max_mac" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "curr_mac" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "curr_mac_static" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_peer_conf" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_peer_if_up" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mlag_is_group" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_mirror_dest" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "hw_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "portid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "disable_learn" },
};

static cdb_field_t tbl_acl_worm_filter_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "seq_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "istcp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dstport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "isrange" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_en" },
};

static cdb_field_t tbl_acl_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ace_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acev6_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acemac_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "intf_ref_in" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "intf_ref_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_port_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_flags_ref" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "remark" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "seq_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type_identifying" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ace_truncation_ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "decap_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "decap_inner_match_seq_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "decap_normal_seq_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "service_ipt_attached" },
};

static cdb_field_t tbl_ace_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_flags2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_ipv6_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_arp_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_op_code" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_op_code_mask" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "time_range" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "apply_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "in_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "out_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "untag_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_flow_label" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_flow_label_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_precedence" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_precedence_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_protocol_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_frag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "igmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_code" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_offset" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_value" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_value_valid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erspan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erspan_id_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "strip_header_pos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "strip_header_offset" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_vxlan_vni" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_vxlan_vni_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_key_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_add_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_key_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_key_len" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l3gre_add_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "deny" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "options" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "flow_match_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "action_strip_header" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_redirect" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_mark_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_untag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_trunction" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_dest_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_dest_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_src_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_ipda_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_ipda" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_ipsa_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_ipsa" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_snmp_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_vxlan_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_srcport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_destport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_vni" },
};

static cdb_field_t tbl_acl_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tblid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "entry_priority" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "time_range" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_flags2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_ipv6_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_arp_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_op_code" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_op_code_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "invalid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_copp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "in_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "out_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ether_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "svlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cvlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "untag_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_flow_label" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_flow_label_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_precedence" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_precedence_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_protocol_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_frag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "igmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_code" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_offset" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_value" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "udf_value_valid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erspan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erspan_id_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "strip_header_pos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "strip_header_offset" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_vxlan_vni" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_vxlan_vni_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_gre_key_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_add_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l2gre_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_key_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2gre_key_len" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l3gre_add_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "l3gre_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "category_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "deny" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "options" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "inner_key_flags" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "inner_key_ipv6_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_is_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ether_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ether_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_src_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_dst_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_svlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_svlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_svlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_svlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_cvlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_cvlan_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_cvlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_cvlan_cos_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_src_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inner_dst_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ipv6_flow_label" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ipv6_flow_label_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_dscp_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_precedence" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_precedence_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_protocol_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_src_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_src_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_src_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_dst_port_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_dst_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_l4_dst_port_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_ip_frag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_tcp_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_tcp_flags_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_igmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_icmp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_icmp_code" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inner_options" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "action_strip_header" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_redirect" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_mark_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_untag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_trunction" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_dest_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_dest_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_src_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_ipda_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_ipda" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_ipsa_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tap_action_edit_ipsa" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_action_edit_vlan_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tap_group_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_vxlan_header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vxlan_dst_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_srcport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_destport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_vni" },
};

static cdb_field_t tbl_acl_entry_action_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "action_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "deny" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_enale" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "policer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nexthop_group" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "logen" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "color" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "snat_cancel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_svlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_cvlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_scos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_ccos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "redirect_port_ifindex" },
};

static cdb_field_t tbl_acl_nexthop_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "acl_nexthop_key" },
};

static cdb_field_t tbl_acl_nexthop_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "edit_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ipv4" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dst_ipv4" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_src_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_dst_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref" },
};

static cdb_field_t tbl_time_range_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "time" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "running_timer" },
};

static cdb_field_t tbl_ssh_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_retry" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rekey_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "listen_port" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "hostkey" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "netconf_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "netconf_service" },
};

static cdb_field_t tbl_snmp_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "server_enable" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "engineid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "contact" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "location" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_coldstart" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_warmstart" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_linkdown" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_linkup" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_system" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_vrrp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_ptp_master_change" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_ptp_state_change" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_ptp_accuracy_change" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "trap_enable_all" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "watch_restart_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agent_stats" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_snmp_trap_all_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_snmp_enable_timer" },
};

static cdb_field_t tbl_snmp_view_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "access_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "refcnt" },
};

static cdb_field_t tbl_snmp_community_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "view" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "secidx" },
};

static cdb_field_t tbl_snmp_context_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
};

static cdb_field_t tbl_snmp_usm_user_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "remo_engid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "auth_pwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_auth_pwd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priv_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "priv_pwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_priv_pwd" },
};

static cdb_field_t tbl_snmp_group_usm_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "security_name" },
};

static cdb_field_t tbl_snmp_access_usm_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "usm_level" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "read_view" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "write_view" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "notify_view" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "context_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "isprefix" },
};

static cdb_field_t tbl_snmp_notify_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "tag_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "notify_type" },
};

static cdb_field_t tbl_snmp_target_addr_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "para_name" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "upd_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retries" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mgmt_if" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "tag_list" },
};

static cdb_field_t tbl_snmp_target_params_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "user_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "usm_level" },
};

static cdb_field_t tbl_snmp_trap_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
};

static cdb_field_t tbl_snmp_inform_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retries" },
};

static cdb_field_t tbl_snmp_trap_msg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "msg" },
};

static cdb_field_t tbl_rmon_event_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "conf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "last_sent" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "desc" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "community" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "owner" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_under_creation_timer" },
};

static cdb_field_t tbl_rmon_alarm_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "conf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "interval" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "variable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "variable_word" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sample_type" },
    { CDB_ACCESS_RW,  CDB_DT_INT64,      -1,                  "value" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "startup_alarm" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "rising_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "falling_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rising_event_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "falling_event_index" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "owner" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "last_event" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "prev_value" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "curr_value" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_alarm_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_under_creation_timer" },
};

static cdb_field_t tbl_rmon_ether_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "owner" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "data_source" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "data_source_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "drop_events" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "broadcast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "multicast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "crc_align_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "undersize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "oversize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "fragments" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "jabbers" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "collisions" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_64_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_65to127_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_128to255_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_256to511_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_512to1023_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_1024to1518_octets" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_under_creation_timer" },
};

static cdb_field_t tbl_rmon_history_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "owner" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "data" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "data_source" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buckets_requested" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buckets_granted" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_sample_no" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_under_creation_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_coll_history_timer" },
};

static cdb_field_t tbl_rmon_stats_history_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "interval_start" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "drop_events" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "broadcast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "multicast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "crc_align_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "undersize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "oversize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "fragments" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "jabbers" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "collisions" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "utilization" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_drop_events" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_octets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_broadcast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_multicast_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_crc_align_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_undersize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_oversize_pkts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_fragments" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_jabbers" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total_collisions" },
};

static cdb_field_t tbl_syslog_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_to_server" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_to_file" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_to_trap" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_to_diag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "logging_lines" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "server_addr" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "server_facility" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "server_severity" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "trap_facility" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "trap_severity" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "timestamp" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "file_severity" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "module_severity" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_merge" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_operate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "merge_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "merge_fifosize" },
};

static cdb_field_t tbl_ntp_server_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "keyid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "prefer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
};

static cdb_field_t tbl_ntp_ace_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "option" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "family" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "masklen" },
};

static cdb_field_t tbl_ntp_key_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "value" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trusted" },
};

static cdb_field_t tbl_ntp_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "aclCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ucastServerCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ucastClientCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "bcastServerCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "mcastClientCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "mcastServerCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "bcastDelay" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "minimumDistance" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "authentication" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "brefclock" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "refclock_stratum" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ifreloadCount" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "clearStats" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "debug_level" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ucastServerCount_inband" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ucastServerCount_outband" },
};

static cdb_field_t tbl_ntp_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "disable" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "broadcastClient" },
};

static cdb_field_t tbl_ntp_syncstatus_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "stratum" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "frequency" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "precision" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "reference_time" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "root_delay" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "root_dispersion" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "stability" },
};

static cdb_field_t tbl_static_dns_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_addr" },
};

static cdb_field_t tbl_dynamic_dns_domain_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_dynamic_dns_server_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_qos_domain_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "phb_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cfi_enable" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cos2pri" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cos2color" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cos2phb" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dscp2pri" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dscp2color" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dscp2phb" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "prec2pri" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "prec2color" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "prec2phb" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exp2pri" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exp2color" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exp2phb" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pri_color2cos" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pri_color2dscp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pri_color2exp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "phb2cos" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "phb2dscp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "phb2exp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cos2tc" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dscp2tc" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exp2tc" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tc_color2cos" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tc_color2dscp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tc_color2exp" },
};

static cdb_field_t tbl_qos_mpls_domain_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "exp_domain" },
};

static cdb_field_t tbl_qos_policer_res_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_qos_policer_action_profile_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "confirm_action" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exceed_action" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "violate_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_qos_policer_profile_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "flags_policer_agp" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_algorithm_mode" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_color_mode" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_cir" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_cbs" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_eir_or_pir" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_ebs_or_pbs" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "flags_drop_color" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "flags_use_l3_legth" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "flags_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_dump_stats" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "algorithm_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "color_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "drop_color" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "cir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cbs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "eir_or_pir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ebs_or_pbs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "confirm_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "confirm_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "exceed_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "exceed_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "violate_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "violate_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_pps" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "action_profile_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_qos_drop_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_green_max_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_green_min_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_green_probabilty" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_yellow_max_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_yellow_min_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_yellow_probabilty" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_red_max_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_red_min_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_red_probabilty" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_random_detect" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "green_max_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "green_min_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "green_probability" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "yellow_max_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "yellow_min_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "yellow_probability" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "red_max_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "red_min_threashold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "red_probability" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_qos_queue_shape_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cir" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "queue_weight" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_pir" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_cir" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_mode" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_weight" },
};

static cdb_field_t tbl_qos_port_shape_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pir" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_pir" },
};

static cdb_field_t tbl_qos_queue_smart_buffer_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "smart_buffer_enable" },
};

static cdb_field_t tbl_qos_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "qos_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "phb_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_policer_first_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "policer_stats_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cur_cpu_rate" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "def_cpu_rate" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "latency_range" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buffer_event_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buffer_event_high" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buffer_event_low" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "check_qos_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "qos_ipg_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "in_port_policer_res_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eg_port_policer_res_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "in_flow_policer_res_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eg_flow_policer_res_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "queue_shape_enable" },
};

static cdb_field_t tbl_mirror_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "source_igs_port_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "source_egs_port_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "source_igs_vlan_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "source_egs_vlan_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cpu_source_dir" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "dest_type" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dest_port_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dest_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dest_group" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dest_vlan" },
};

static cdb_field_t tbl_mirror_mac_escape_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_tap_group_ingress_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_mark_vlan_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_mark_vlan_vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_untag_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_trunction_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "ing_tap_group_member_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_dest_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ing_edit_dest_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_src_mac_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ing_edit_src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_ipda_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ing_edit_ipda" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_ipsa_en" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ing_edit_ipsa" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_vlan_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_edit_vlan" },
};

static cdb_field_t tbl_tap_group_ingress_flow_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tap_grp_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "if_idx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_untag_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_mark_vlan_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ing_mark_vlan_vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "ing_tap_group_member_oid" },
};

static cdb_field_t tbl_tap_group_egress_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timestamp_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tap_group_member_oid" },
};

static cdb_field_t tbl_tap_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flags" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "desc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "truncation_use" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "tap_group_oid" },
};

static cdb_field_t tbl_tap_five_tuple_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_entry_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "packet_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "byte_count" },
};

static cdb_field_t tbl_tap_dpi_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dpi_action" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "redir_if" },
};

static cdb_field_t tbl_user_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "uid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "privilege" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "rsakey" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "service_type" },
};

static cdb_field_t tbl_login_record_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "locked" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "first_fail_ts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lock_ts" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fail_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "invalid_user" },
};

static cdb_field_t tbl_login_security_glb_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_fail_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fail_period" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lock_duration" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "curr_invalid_user" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_invalid_user" },
};

static cdb_field_t tbl_vty_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_sec" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "protocol" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "login" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "privilege" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "auth_method" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "inuse" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "author_method" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "account_method" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "accountcmd_method" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ipv4_acl" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ipv6_acl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_privilege" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "config_privilege" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aaa_start_time" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cmd_buf" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "old_cmd_buf" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "acct_stop_ipaddr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acct_stop_pid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acct_stop_privilege" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_not_ssh_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_ssh_login" },
};

static cdb_field_t tbl_console_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_sec" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "login" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "privilege" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "auth_method" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stopbits" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "parity" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "databits" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "baudrate" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "inuse" },
};

static cdb_field_t tbl_authen_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "methods" },
};

static cdb_field_t tbl_login_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "user" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "vty" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "privilege" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ipaddr" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "dest_ipaddr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "remote_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dest_port" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "expire_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pid" },
};

static cdb_field_t tbl_rsa_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "refcnt" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pem" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
};

static cdb_field_t tbl_openflow_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_hash_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bond_hash_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bond_hash_use" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lacp_local_proc" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "local_vtep_ip" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "decap_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flow_drop_pkt_to_ingress_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_drop_pkt_to_ingress_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udf_parser" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "meter_ipg" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inband_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ptp_e2e_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inband_stag" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inband_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inband_gw" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mpls_parser" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "stpid" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ctpid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inband_downlink_port_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "inband_uplink_port_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flow_hash_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_hash_merge_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nvgre_hash_merge_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_tcp_only_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_granularity" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_detect_speed" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_detect_time_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_aging_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_flow_traffic_class" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_flow_color" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "efd_ipg_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "dpid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dpid_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "private_key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "certificate" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ca_cert" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_srcport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_srcport_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_destport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_destport_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_oam_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_oam_interval_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_oam_cycle_is_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_aps_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_aps_interval_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tp_oam_aps_cycle_is_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pw_not_standard_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "check_port_bind_oam" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_cycle_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_ccm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_lm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_dm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_packet_in_cycle_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_packet_in_event_path_change_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_cycle_packet_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_packet_ccm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_packet_lm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_packet_in_event_packet_dm_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_packet_in_cycle_packet_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_packet_in_event_path_change_packet_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_mac_learning_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "controller_affect_flow_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "async_msgid_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_flow_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fdb_flow_valid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ffgroup_switchback_en" },
};

static cdb_field_t tbl_cpu_traffic_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "class_id" },
};

static cdb_field_t tbl_cpu_traffic_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rate" },
};

static cdb_field_t tbl_cpu_utilization_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_DOUBLE,     -1,                  "cpu_usage" },
};

static cdb_field_t tbl_cpu_limit_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "percent" },
};

static cdb_field_t tbl_dhcrelay_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "drop_agent_mismatch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_agent_options" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agent_proc_option" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trust_all" },
};

static cdb_field_t tbl_dhcsrvgrp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cnt" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "addrs" },
};

static cdb_field_t tbl_dhcp_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "relay_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "relay_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "relay_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "relay_dump" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snooping_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snooping_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snooping_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snooping_dump" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snoopingv6_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snoopingv6_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snoopingv6_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "snoopingv6_dump" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "client_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "client_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "client_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "client_dump" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_dump" },
};

static cdb_field_t tbl_dhclient_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "distance" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "broadcast" },
};

static cdb_field_t tbl_dhcsnooping_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "verify_mac_address" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "add_agent_options" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "allow_untrusted" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hostname_as_remote_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "remote_id_string" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "database_save_interval" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vlans" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "snooping_acl_applied" },
};

static cdb_field_t tbl_dhcbinding_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname_new" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "client_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "client_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cIpsour" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lease" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lease_new" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "transid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipsg_tbl_exsit" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "state" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "type" },
};

static cdb_field_t tbl_iptables_prevent_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_icmp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_smurf" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_fraggle" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_udp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_tcp" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_small_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_maceq" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "ipt_cfg_ipeq" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icmp_rate_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcp_rate_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "udp_rate_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "small_pkt_length" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pkt_statistic_update" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "small_pkt_static_base" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "small_pkt_mgmt_static_base" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "icmp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "smurf_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fraggle_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "udp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcp_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "small_pkt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "icmp_mgmt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "smurf_mgmt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fraggle_mgmt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "udp_mgmt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcp_mgmt_count" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "small_pkt_mgmt_count" },
};

static cdb_field_t tbl_errdisable_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "errdisable_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "recovery_en" },
};

static cdb_field_t tbl_ns_port_forwarding_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip" },
};

static cdb_field_t tbl_log_global_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "log_to_cdb" },
};

static cdb_field_t tbl_log_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "log_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "severity" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "timestamp" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "data" },
};

static cdb_field_t tbl_sys_load_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sys_load_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cli_back_pressure_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cdb_queue_notice_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cdb_queue_warning_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cdb_queue_depth_switch" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cdb_queue_depth_routed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cdb_queue_depth_cds" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cdb_queue_depth_fea" },
};

static cdb_field_t tbl_cem_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "event" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "snmptrap" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "loadpath" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mail" },
};

static cdb_field_t tbl_clock_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timezone_positive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timezone_hour" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timezone_minute" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timezone_second" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "timezone_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cp_timezone_positive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cp_timezone_hour" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cp_timezone_minute" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cp_timezone_second" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "cp_timezone_name" },
};

static cdb_field_t tbl_tag_summer_clock_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tmstart" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tmend" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "iDstOffset" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "is_recurring" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "nStartFebDay" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "nEndFebDay" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "strZoneName" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "is_wday_tm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "wday_tm_start" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "wday_tm_end" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_summer_time_timer" },
};

static cdb_field_t tbl_port_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_id" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "octets_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "uc_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "brdc_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "mc_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "oam_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "undersize_pkts" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "oversize_pkts" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "jabber_pkts" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "mac_rcv_error" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bad_crc" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "frame_error" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "drop_events" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pause_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "octets_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "uc_pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "brdc_pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "mc_pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "oam_pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "underruns" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "mac_transmit_err" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pause_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "fcs_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "fcs_octets_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "fcs_pkts_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "fcs_octets_send" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "fragments_pkts" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bad_pkts_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bad_octets_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_64_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_65_to_127_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_128_to_255_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_256_to_511_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_512_to_1023_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pkts_1024_to_1518_rcv" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "last_clear_ts" },
};

static cdb_field_t tbl_vlan_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pktcntl2in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pktcntl2out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pktcntl3in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pktcntl3out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "pktcntacl" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bytecntl2in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bytecntl2out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bytecntl3in" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bytecntl3out" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "bytecntacl" },
};

static cdb_field_t tbl_port_stats_rate_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "octets_rcv_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_rcv_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "octets_send_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "pkts_send_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "load_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "input_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "input_rate_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "input_rate_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "input_rate_resume" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_rate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_rate_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_rate_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_rate_resume" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "output_discard_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_discard_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "output_discard_packets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "output_discard_total_packets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_discard_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "output_discard_running_interval" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_output_discard_timer" },
};

static cdb_field_t tbl_aclqos_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_type" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_domain" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_shape_profile" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_input_policer" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_output_policer" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_queue_shape_profile" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_queue_drop_profile" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_queue_drop_mode" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_replace_dscp" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_replace_cos" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_trust" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_tc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "domain" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cos_domain" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dscp_domain" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "port_shape_profile" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "input_policy_map" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "output_policy_map" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "input_policer" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "output_policer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_shape_profile" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_profile" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "replace_dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "replace_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trust" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_tc" },
};

static cdb_field_t tbl_l2_action_fields[] = 
{
    { CDB_ACCESS_SUB, CDB_DT_DS,         DS_FLUSH_FDB,        "flush_fdb" },
};

static cdb_field_t tbl_fea_qos_drop_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_drop_id" },
};

static cdb_field_t tbl_fea_qos_domain_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "dot1p_to_tc_color_map_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "dscp_to_tc_color_map_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "exp_to_tc_color_map_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tc_and_color_to_dot1p_map_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tc_and_color_to_dscp_map_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tc_and_color_to_exp_map_id" },
};

static cdb_field_t tbl_fea_g8032_ring_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mc_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nh_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nhoffset" },
};

static cdb_field_t tbl_fea_g8032_ring_mc_group_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mc_group_id" },
};

static cdb_field_t tbl_fea_qos_queue_shape_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_scheduler_id" },
};

static cdb_field_t tbl_fea_qos_port_shape_profile_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "sai_scheduler_id" },
};

static cdb_field_t tbl_fea_port_policer_apply_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "in_policer_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "out_policer_id" },
};

static cdb_field_t tbl_aclqos_if_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_get_input_policer_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_get_output_policer_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_clear_input_policer_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_port_clear_output_policer_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_get_queue_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_clear_queue_stats" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_transimt_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_transimt_byte" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_pkt" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "queue_drop_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_packet_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_byte_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_packet_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_byte_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_packet_out" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_byte_out" },
};

static cdb_field_t tbl_version_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "version" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "company" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "package" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "product" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "hw_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_cst" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "cst_company" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "cst_hw_type" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "cst_hw_info" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "cst_hw_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "cst_sw_ver" },
};

static cdb_field_t tbl_manage_if_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "gateway" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhclient_enable" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dhclient_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dhclient_gateway" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "addr_v6" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "gateway_v6" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "link_local_addr_str" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhclientv6_enable" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dhclientv6_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dhclientv6_gateway" },
};

static cdb_field_t tbl_bootimage_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fast" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "image" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "serverip" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ipaddr" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "bootcmd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "current_image" },
};

static cdb_field_t tbl_chassis_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "type" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot_num" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "cur_stm_mode" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "next_stm_mode" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "mac_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "sys_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "location_led" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reset_reboot_info" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "support_reboot_info" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "one_minute_reboot_alarm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "schedule_reboot_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "schedule_reboot_alarm_timer" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "schedule_reboot" },
};

static cdb_field_t tbl_ifname_info_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "connection" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "sub_connection" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "prefix_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "prefix" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "full_prefix" },
};

static cdb_field_t tbl_card_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "product_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "hardware_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "company_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "package_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "enterprise_oid" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "hardware_type" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "serial_no" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "bootrom_ver" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "epld_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "sw_ver" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "hardware_ver" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "board_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "board_series" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "flash_size" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "dram_size" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "port_num" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "uptime_day" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "uptime_hour" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "uptime_min" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "attach_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "attach_count" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "tmpr_low" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "tmpr_high" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "tmpr_critical" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "epld_status" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reboot" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "update_phy_firmware_port_idx" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "platform_type" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "bootup_diag_level" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "next_bootup_diag_level" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "fan_speed_bottom" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "fan_speed_low" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "fan_speed_high" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "fan_speed_full" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "bootcmd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "update_epld_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "update_bootrom_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "update_phy_firmware_name" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "glb_poe_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_poe_power_max" },
};

static cdb_field_t tbl_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot_no" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "panel_port_no" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "panel_sub_port_no" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "phy_type" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "phyinfo_flag" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "speed_capbility" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "logic_port_id" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "media" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "port_media_type" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "support_media_switch" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "split_type" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "bypass" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "bypass_slot" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "training_enable" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "training_status" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "present_status" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac" },
};

static cdb_field_t tbl_fiber_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "interface_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "fiber_type_name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "oui" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "pn" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "rev" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "sn" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "compliance_code" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tmpr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "voltage" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "bias" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tx_pwr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rx_pwr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tmpr2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "voltage2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "bias2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tx_pwr2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rx_pwr2" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "alarm_flag" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "warn_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "update_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "channel_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "channel_idx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_detail" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fiber_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "fiber_flag" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_copper" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "speed" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ddm_support" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "externally_calibrated" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "wavelength" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "length" },
};

static cdb_field_t tbl_sys_spec_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ucast_fdb" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "static_fdb" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mstp_instance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vlan_instance" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mac_filter" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mac_based_vlan_class" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ipv4_based_vlan_class" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ipv4_source_guard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vlan_mapping_entry_applied_port" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "dot1x_mac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "indirect_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "static_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "host_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ecmp_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ecmp_member_maximum" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "indirect_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "static_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "host_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ecmp_v6_routes" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ecmp_v6_member_maximum" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pbr_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l2mc_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l3mc_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l2mc_member_ports" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l3mc_member_ports" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vlan_member_ports" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "voice_vlan" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flow_entries_ingress" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flow_entries_egress" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flow6_entries_ingress" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flow6_entries_egress" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cfm_local_and_remote_meps" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "g8031_groups" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "g8032_rings" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "g8032_member_ports_per_ring" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ftn_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ilm_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mpls_lables" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vpws" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lsp_pe" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lsp_p" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vpls_peer" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vpls_ac" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vsi" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lsp_oam" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pw_oam" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "mpls_aps_tunnel" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "cfm_lck" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ip_tunnel" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ivi_peers" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pbr_v6_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l3mc_v6_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l2mc_v6_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "flow_v6_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ipv6_based_vlan_class" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ipv6_source_guard" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l3mc_v6_member_ports" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l2mc_v6_member_ports" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vlan_mapping_default_entry" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "bfd_sessions" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nvgre_tunnel_id_mappings" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nvgre_peers" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nvgre_tunnels" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "openflow_flow_entries" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "policer_profile_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "port_shape_profile_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "queue_shape_profile_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "drop_profile_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "lag_group_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ospf_area_range_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tap_group_profile_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tap_flow_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tap_flow_entry_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "l2protocol_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "openflow_dynamic_tunnel_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_pool_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_rule_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_inside_rtif_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_outside_rtif_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_session_upstream_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_session_dnstream_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_session_fast_path_entry_num" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "nat_session_limit_node_num" },
};

static cdb_field_t tbl_fan_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "tray" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "percent" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "absent" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "direction" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "mode" },
};

static cdb_field_t tbl_psu_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "absent" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "run_status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "alert_status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fixed_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "psu_health_status_change" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "support_pmbus" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_word" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_vout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_iout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_input" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_temperature" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_cml" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_other" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_mfr_specific" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_fans_1_2" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pmbus_status_fans_3_4" },
};

static cdb_field_t tbl_led_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "mode" },
};

static cdb_field_t tbl_sensor_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "temperature" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "position" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "status" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "tmpr_low" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "tmpr_high" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "tmpr_critical" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_chip" },
};

static cdb_field_t tbl_vcm_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vcm_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "vcm_channel_nums" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name0" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name1" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name2" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name3" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name4" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name5" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name6" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name7" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "channel_name8" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "flag" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "curr_depend_pin" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "volt_split_factor" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "low" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "high" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "volt" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "curr" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "tmpr" },
};

static cdb_field_t tbl_poe_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "slot" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_power_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_cur_consump" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_aver_consump" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "glb_peak_consump" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pse_channel_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "const_power_limit" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "pse_channel_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "panel_slot_no" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "panel_port_no" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "panel_sub_port_no" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "force_power" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "power_limit" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "class_level" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "cur_consump" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "aver_consump" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "peak_consump" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "current" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "voltage" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "temper" },
};

static cdb_field_t tbl_reboot_info_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "reboot_type" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "reboot_time" },
};

static cdb_field_t tbl_errdisable_flap_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "errdisable_rsn" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flap_cnt_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flap_time_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flap_time_cnt" },
};

static cdb_field_t tbl_opm_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_mode" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_unreload" },
};

static cdb_field_t tbl_erps_ring_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "erps_ring_state" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "erps_ring_level" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "instance_id_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "instance_id" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "node_role" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "edge_node_role" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erps_srpt_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "primary_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "secondary_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "edge_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "common_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "blocked_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "blocked" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "hello_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "failure_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "pre_forwarding_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "flush_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "edge_hello_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "edge_failure_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "edge_block_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hello_seq" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "expect_hello_seq" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_hello_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_ring_up_flush_fdb_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_ring_down_flush_fdb_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_link_down_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_edge_hello_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "rcv_major_fault_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_hello_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_ring_up_flush_fdb_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_ring_down_flush_fdb_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_link_down_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_edge_hello_cnt" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "send_major_fault_cnt" },
};

static cdb_field_t tbl_erps_domain_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "instance_id_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "instance_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "primary_control_vlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sub_control_vlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hello_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "failure_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pre_forwarding_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flush_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "edge_hello_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "edge_failure_timer_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "erps_enable" },
    { CDB_ACCESS_RW,  CDB_DT_REFERLIST,  -1,                  "erps_ring_list" },
};

static cdb_field_t tbl_opm_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "erps_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8131_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8131_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8131_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_lb_lbm_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_lb_lbr_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_lm_comm_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_lm_lmm_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_lm_lmr_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_dm_comm_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_dm_dmm_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_dm_dmr_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_dm_1dmtx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_dm_1dmrx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_packet_rx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_packet_tx_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "tpoam_event_debug" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_packet_tx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_packet_rx" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "g8032_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cfm_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cfm_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cfm_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "cfm_event" },
};

static cdb_field_t tbl_policy_map_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "intf_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "class_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "max_class_priority" },
};

static cdb_field_t tbl_class_map_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "policy_map_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "max_acl_priority" },
};

static cdb_field_t tbl_class_in_policy_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "class_priority" },
};

static cdb_field_t tbl_acl_in_class_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "acl_priority" },
};

static cdb_field_t tbl_class_map_action_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "action_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_tc" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "new_color" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_dscp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "policer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_svlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_cvlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_scos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "new_ccos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "redirect_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "red_unexist_agg" },
};

static cdb_field_t tbl_fea_acl_policy_action_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_class_get_policer_stats" },
    { CDB_ACCESS_RD,  CDB_DT_FLAGS32,    -1,                  "flags_calss_clear_policer_stats" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "policer_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "policer_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "policer_id_ref" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_byte" },
};

static cdb_field_t tbl_igsp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fast_leave" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "discard_unknown" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "report_suppress" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "querier_max_response_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lmqi" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lmqc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "robustness_var" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_member_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "curr_group_member" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcn_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcn_query_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcn_query_interval" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "tcn_query_current_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tcn_query_max_response_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "host_join_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "host_joined_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "global_src" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "all_hosts" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "all_routers" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "v3_routers" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_group_expire" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_tcn_query" },
};

static cdb_field_t tbl_igsp_intf_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "acl_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fast_leave" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "discard_unknown" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "discard_unknown_act" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "report_suppress" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "querier_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "querier_operate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_other_query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "other_query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "querier_max_response_time" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "querier_config_address" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "querier_oper_address" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "other_querier_address" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "vlan_if_address" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lmqi" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lmqc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_member_number" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "curr_group_member" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_member_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "robustness_var" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mrouter_aging_interval" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "startup_query_count" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_querier" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_other_querier" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_MROUTER_PORT,     "mrouter_list" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_QUERY,            "query_list" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_JOIN_GROUP,       "join_list" },
};

static cdb_field_t tbl_igsp_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "retx_group_lmqc" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "retx_group_source_lmqc" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "liveness" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "has_rx_leave" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "last_reporter_address" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "uptime" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "update_time" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "v1_update_time" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_v1_host_exist" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "inactive" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_v3_leave" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_retx_query" },
};

static cdb_field_t tbl_auth_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "secret" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_secret" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retries" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "deadtime" },
};

static cdb_field_t tbl_auth_server_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "secret" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_secret" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retries" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dead" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "reawake_time" },
};

static cdb_field_t tbl_auth_session_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "user" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
    { CDB_ACCESS_RD,  CDB_DT_ENUM8,      -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "authenticator" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oauthenticator" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dot1x_mac_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dot1x_mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_auth_bypass" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "author_start" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acct_start" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acct_stop" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acctcmd_start" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "next_acct_stop" },
};

static cdb_field_t tbl_authd_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_event" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_timer" },
};

static cdb_field_t tbl_dot1x_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "global_auth_enable" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "eapol_group_address" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_default_retry" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_default_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_default_deadtime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_user_count_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_user_count_current" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_user_count_db" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_port_count_current" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_default_shared_secret_valid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "radius_default_shared_secret" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "radius_default_shared_enc_secret" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "last_session_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "session_id_allocated" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_timer" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dot1x_event" },
};

static cdb_field_t tbl_dot1x_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_VOID,       -1,                  "current_radius" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_msg_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retry" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_wait" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sessiontype" },
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "buf" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dest_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "user_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authControlledPortControl" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protocol_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "guest_vlan_valid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "guest_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_user" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_user_count_db" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_user" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mac_auth_bypass" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "share_reauth" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authWhile" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aWhile" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "quietWhile" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reAuthWhen" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "txWhen" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authAbort" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authFail" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authStart" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authTimeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "authSuccess" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "initialize" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "portEnabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reAuthenticate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapNoReq" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eaprestart" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapResp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapReq" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapSuccess" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapFail" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retransmit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "currentId" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "portControl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "portStatus" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eapLogoff" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eapStart" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_portMode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_reAuthCount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_rxRespId" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_quietPeriod" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_reAuthMax" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_txPeriod" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_enter_connecting" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eaplogoff_while_connecting" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_enter_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_success_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_timeout_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_fail_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eapstart_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eaplogoff_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_reauths_while_authenticated" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eapstart_while_authenticated" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_eaplogoff_while_authenticated" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_pae_reauths_while_authenticating" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_xmit_keyAvailable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_xmit_keyTxEnabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_xmit_KeyRun" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_xmit_KeyDone" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_xmit_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reAuthPeriod" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reAuthEnabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_reqCount" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_idFromServer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_suppTimeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_serverTimeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_maxReq" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_responses" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_access_challenges" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_otherrequest_to_supplicant" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_auth_success" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_auth_fails" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_be_backend_nonnak_responses_from_supplicant" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_ctrl_dir_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_ctrl_dir_adminControlledDirections" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_ctrl_dir_operControlledDirections" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_ctrl_dir_bridgeDetected" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "auth_key_recv_rxKey" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_frames_tx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_start_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_logoff_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eap_respid_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eap_resp_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eap_reqid_frames_tx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eap_req_frames_tx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_invalid_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eap_len_error_frames_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "eapol_last_frame_version" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "eapol_last_frame_source" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_authentication_method" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_terminate_cause" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "session_user_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "class_id" },
};

static cdb_field_t tbl_dot1x_radius_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "identifier" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "server_string" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "server_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "radius_state_len" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "radius_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_retry" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "radius_defalt_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "radius_defalt_retransmit" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "radius_defalt_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "shared_secret_valid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "shared_secret" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "shared_enc_secret" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "authenticator" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oauthenticator" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_number" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "session_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "reactivate" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
};

static cdb_field_t tbl_dot1x_mac_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "user_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_id" },
    { CDB_ACCESS_RW,  CDB_DT_VOID,       -1,                  "current_radius" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_wait" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_reauth" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_reject" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_delete" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "entry_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_auth_bypass" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_mac_reject" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_mac_accept" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_mac_bypass" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_mac_reauth_accept" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "auth_mac_eapol_reauth" },
};

static cdb_field_t tbl_enable_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "enc_passwd" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "passwd" },
};

static cdb_field_t tbl_chip_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_ENUM32,     -1,                  "type" },
};

static cdb_field_t tbl_clear_acl_policy_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "counter_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "acl_id" },
};

static cdb_field_t tbl_author_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "methods" },
};

static cdb_field_t tbl_account_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "start_stop" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stop_only" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "methods" },
};

static cdb_field_t tbl_accountcmd_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "methods" },
};

static cdb_field_t tbl_vlanclass_rule_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rule_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_address" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protocol_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protocol_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rule_oid" },
};

static cdb_field_t tbl_vlanclass_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rule_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "action_rule_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "group_oid" },
};

static cdb_field_t tbl_acl_l4_port_range_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_src" },
};

static cdb_field_t tbl_acl_udf_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l3_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "offset" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "udf_match_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "udf_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "udf_group_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cnt" },
};

static cdb_field_t tbl_acl_udf_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "offset_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "offset_num" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "valid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "offset" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "key_flags" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ethtype" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ethtype_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlannum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlannum_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipproto" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipproto_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l4_type_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "srcport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "srcport_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dstport" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dstport_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mplsnum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mplsnum_mask" },
};

static cdb_field_t tbl_fea_pcap_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rx_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
};

static cdb_field_t tbl_controller_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port" },
};

static cdb_field_t tbl_cpu_packets_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "total" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_bpdu" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_slow_proto" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_eapol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_lldp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_erps" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_macda" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_rip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_ospf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_bgp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_arp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_dhcp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_ipda" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_igmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_mac_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_mac_mismatch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_l3copy_cpu" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_ttl_error" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_ptp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_tpoam" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_l2pro_protocol_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_l2pro_group_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_cpu_mirror" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_nd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_packetin" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_mtu_fail" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_g8032" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rx_other" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_igmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_bpdu" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_slow_proto" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_arp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_dhcp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_lldp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_erps" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_eapol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_ptp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_tpoam" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_l2pro_protocol_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_l2pro_group_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_cpu_mirror" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_nd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_g8032" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "tx_other" },
};

static cdb_field_t tbl_ns_route_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "cnt" },
};

static cdb_field_t tbl_ns_route_ip_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "cnt" },
};

static cdb_field_t tbl_openflow_interface_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "obey_vlan_filter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_instance_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_add_to_br0" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "protected_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protected_vlan_change" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protected_vlan_change_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ingress_add_native_vlan_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "egress_remove_native_vlan_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_tunnel_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_tunnel_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tunnel_mpls_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_bind_tunnel" },
};

static cdb_field_t tbl_openflow_manager_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_inband" },
};

static cdb_field_t tbl_ptp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "default_ds" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "current_ds" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "parent_ds" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "time_prop_ds" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "time_prop_intern" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "device_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "global_enable" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "master_min_delay_req_interval" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "c" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_internal_timer" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "current_slave_port" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "set_system_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "masterChangeTrap" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stateChangeTrap" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "accuracyChange" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "glb_enable_hw_msec_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "glb_enable_pf_msec_time" },
    { CDB_ACCESS_RW,  CDB_DT_INT64,      -1,                  "total_offset" },
    { CDB_ACCESS_RW,  CDB_DT_INT64,      -1,                  "hsrv_adjust_offset" },
    { CDB_ACCESS_RW,  CDB_DT_INT64,      -1,                  "hsrv_adjust_ffo" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "hsrv_update_system_time" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "hsrv_device_type" },
};

static cdb_field_t tbl_ptp_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_INT64,      -1,                  "hsrv_path_delay" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "port_ds" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "port_addr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "asymmetry_correction" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "ingress_latency" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "egress_latency" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "dump_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lag_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "link_up" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "drop_announce" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "packet_stats" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vlan_info" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "recommended_state" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "bmc_code" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM8,      -1,                  "bmc_expires" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "slave_change_master" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "synchronization_fault" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "master_clock_selected" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ptp_seq_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "uncalibrated_update_offset_times" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "uncalibrated_update_limit_times" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "p" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "c" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_announce_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_sync_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_delay_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_pdelay_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_announce_receipt_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ptm_qualification_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_timer_faulty_recover_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "msg_proc_ops" },
};

static cdb_field_t tbl_ptp_foreign_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "foreign_master_announce_messages" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "header" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "announce" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ptm_fm_qualification_timer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ptm_fm_announce_receipt_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "qualified" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "port_foreign_num" },
};

static cdb_field_t tbl_ptp_residence_time_cache_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sequence_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex_port_ingress" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "domain" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ptm_age_task_timer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "residence_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outdated" },
};

static cdb_field_t tbl_fea_ptp_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "global_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "device_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retry_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ready" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "blocked" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tx_seq_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "intr_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acl_update_time_range" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "hsrv_ptp_timer" },
};

static cdb_field_t tbl_fea_ptp_ts_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "intr_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ns" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "s_lsb" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "s_msb" },
};

static cdb_field_t tbl_fea_time_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "sync_type" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "sync_count" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "last_sync_time" },
};

static cdb_field_t tbl_bhm_global_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "hw_watchdog_fd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hw_watchdog_feed_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hw_watchdog_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sw_watchdog_enable" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "reactive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_reload" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_shutdown" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_warning" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "reboot" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sw_watchdog_timer" },
};

static cdb_field_t tbl_bhm_module_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "running" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "pid" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "death_count" },
};

static cdb_field_t tbl_openflow_tunnel_interface_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "openflow_enable" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "source_ip" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "remote_ip" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "bind_port_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "bind_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tunnel_status" },
};

static cdb_field_t tbl_openflow_tunnel_local_ip_cnt_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "local_ip_ref" },
};

static cdb_field_t tbl_inband_snat_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_handle" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "snat_timer" },
};

static cdb_field_t tbl_arpinsp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "acllog_filter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dhcplog_filter" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "filter_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable_state" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "forwarded" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "dropped" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "dhcp_dropped" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "acl_dropped" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "dhcp_permit" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "acl_permit" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "srcmac_failure" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "dstmac_failure" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "ip_failure" },
    { CDB_ACCESS_RD,  CDB_DT_UINT64,     -1,                  "invalid_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "filter" },
};

static cdb_field_t tbl_arpacl_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "seq_ref" },
};

static cdb_field_t tbl_arpace_config_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_type" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_mac_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_deny" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "arp_log" },
};

static cdb_field_t tbl_copp_cfg_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "copp_pname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "copp_ifidx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "copp_total_rate" },
};

static cdb_field_t tbl_fea_copp_trap_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "policer_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "trap_oid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "queue_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "green_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "yellow_byte" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_packet" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "red_byte" },
};

static cdb_field_t tbl_sflow_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "version" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "agent" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "agent_ipv6" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "counter_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "counter_port_num" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "counter_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "counter_next_port" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "sflow_all" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "sflow_counter" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "sflow_sample" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "sflow_packet" },
};

static cdb_field_t tbl_sflow_collector_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "buf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pkt_len" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sendfd" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "transdelay_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sequence" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_log_timer" },
};

static cdb_field_t tbl_sflow_counter_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "left_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sequence" },
};

static cdb_field_t tbl_vrrp_global_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_vrrp_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_vrrp_msec_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrrp_version" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrrp_session_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vrrp_msec_session_count" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "buf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pkt_len" },
};

static cdb_field_t tbl_vrrp_vmac_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
};

static cdb_field_t tbl_vrrp_vip_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vmac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "interface_mac" },
};

static cdb_field_t tbl_vrrp_session_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "state" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "owner" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "config_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "advt_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "time_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "preempt_delay" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "preempt_mode" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vmac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timer" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "old_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vip_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifp_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "shutdown_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "skew_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "master_down_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "num_ip_addrs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trackobj_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trackobj_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority_delta" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vrrp_uptime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rowstatus" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_become_master" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_advt_rcvd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_advt_interval_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_auth_failures" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_ip_ttl_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_priority_zero_pkts_rcvd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_priority_zero_pkts_sent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_invalid_type_pkts_rcvd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_addr_list_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_invalid_auth_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_auth_type_mismatch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "stats_pkt_len_errors" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "master_ip_status" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "master_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "master_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "master_advt_interval" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "primary_ip" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "learn_master" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "secondaryip_same_vip" },
};

static cdb_field_t tbl_ssm_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "device_clock_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "synce_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "local_priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "option_mode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "system_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "device_role" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ssm_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ssm_extend_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "holdover_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "freerun_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "select_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dpll_selected_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dpll_current_status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "force_quality_option1_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "force_quality_option2_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "clock_recovery_select" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "next_selected_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "select_if_linkup_restore_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "select_if_priority_change_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "select_if_quality_change_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "holdoff_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wtr_time" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "holdoff_send_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "wtr_send_timer" },
};

static cdb_field_t tbl_ssm_port_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "synce_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ssm_pdu_direct" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "replace_clockid_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "receive_pdu_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "send_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "receive_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "force_tx_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "force_rx_quality" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dnu_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hop" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "receive_clock_id" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ssm_timeout_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "ssm_send_timer" },
};

static cdb_field_t tbl_ipsla_global_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "ipsla_entry_num" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ipsla_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_test_failure" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_test_success" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_pkt_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "trap_overthreshold" },
};

static cdb_field_t tbl_ipsla_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "admin_state" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "desc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "frequency" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "packets_per_test" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fail_percent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "data_size" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "data_pattern" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "inaddr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "src_inaddr" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "src_ifname" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "sock" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "send_sock" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "sock_flag" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "buf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "packet_length" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_ipsla_read" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "testid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_frequency" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_probe_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sequence" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "send_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "last_send_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "send_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "receive_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "overthreshold_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sendfail_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_rtt_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_rtt_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_rtt_sum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_success" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_fail_percent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "returncode" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "echo_reply" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "total_receive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "total_send" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rtt_sum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "probe_result_config_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "probe_result_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "test_result_config_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "test_result_index" },
};

static cdb_field_t tbl_ipsla_packet_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "send_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "testid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "status" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "threshold" },
};

static cdb_field_t tbl_ipsla_test_stats_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "last_send_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "testid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "send_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "receive_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "overthreshold_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sendfail_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt_min" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt_max" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtt_sum" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "success" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fail_percent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
};

static cdb_field_t tbl_track_global_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "track_object_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "track_timer_interval" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_track_timer" },
};

static cdb_field_t tbl_track_object_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "state" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "track_if" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rtr_entryid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "delay_up" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_delay_up" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "delay_down" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "running_delay_down" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "tracked_vrid_bmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "static_rt_refcnt" },
};

static cdb_field_t tbl_controller_attr_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "controller_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "max_backoff" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inactivity_probe" },
};

static cdb_field_t tbl_oam_session_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "creat_by_flow" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_type" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "pw_type" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "mp_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "service_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "service_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "megid_str" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mepid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rmepid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cc_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cc_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cc_interval_cfg" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lm_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dm_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "level" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_oam_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_oam_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "icclen" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "description" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_dm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_lm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_lb" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_event" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_t_send_dm_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dm_active" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_status" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "signal_status" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_stats" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_list" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flag_add_flow" },
};

static cdb_field_t tbl_oam_session_upload_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bind_oam_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upload_dm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upload_dm_event" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upload_lm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upload_lm_event" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upload_ccm" },
};

static cdb_field_t tbl_lsp_aps_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "working_lsp_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "protection_lsp_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flow_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wait_to_restore_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_revertive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "active_path" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_permanent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_state" },
};

static cdb_field_t tbl_pw_aps_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "working_pw_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "protection_pw_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flow_flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wait_to_restore_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_revertive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "active_path" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_permanent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_state" },
};

static cdb_field_t tbl_lsp_pe_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_session_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lsp_aps_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nexthop_mac" },
};

static cdb_field_t tbl_lsp_p_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_inlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_outlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_vid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "west_nexthop_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_outlabel_exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_outlabel_exp_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_outlabel_ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_inlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_outlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_outlabel_exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_outlabel_exp_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_outlabel_ttl" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "east_nexthop_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_session_id" },
};

static cdb_field_t tbl_pw_tpe_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "tunnel_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "lsp_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lsp_aps_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pw_aps_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_session_id" },
};

static cdb_field_t tbl_section_oam_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_session_id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nexthop_mac" },
};

static cdb_field_t tbl_oam_flow_info_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inlabel_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "inlabel_group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
};

static cdb_field_t tbl_tpoam_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "node_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "channel_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fast_aps_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rdi_reason" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rdi_reason_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sf_reason_mask" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sd_reason_mask" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "resouce_count_max" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "resouce_count_curr" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "event_rule_map" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "oam_glb_stats" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "trap_cache" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "trap" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "log" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dm_enable_global" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dm_delay_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dm_calc_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lm_enable_global" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lm_local_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lm_remote_threshold" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lm_calc_interval" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "lm_calc_interval_timer" },
};

static cdb_field_t tbl_tpoam_exec_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "test_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oam_session_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sock" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_lm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_lb_rmep" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_lb_mip" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_lb_discovery" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_dmm" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "exec_1dm" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_t_send_lb_msg_timer" },
};

static cdb_field_t tbl_aps_group_info_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "working_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "protection_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "select_name" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "aps_group_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wait_to_restore_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_revertive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "active_path" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_permanent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_event" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dfop_reason" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sc_aps_intervel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "fast_aps_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_channel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "requested_signal" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "direction" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bridge_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "last_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "far_end_last_event" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "wtr_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sf_hold_off_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sfp_hold_off_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sd_hold_off_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sdp_hold_off_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "sc_out_aps_timer" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "aps_tx_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "sc_aps_out_pdu_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_expired" },
};

static cdb_field_t tbl_oam_inlabel_info_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_oam_mpls_aps_check_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lsp_label" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_aps_action" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "nexthop_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_exp_is_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "outlabel_ttl" },
};

static cdb_field_t tbl_vlan_mapping_table_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "applied_ports" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "entry_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "egress_entry_num" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "group_mapped_svid" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "group_evc_entry_name" },
};

static cdb_field_t tbl_vlan_mapping_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_svid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_cvid" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "evc_entry" },
};

static cdb_field_t tbl_egress_mapping_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_vid" },
};

static cdb_field_t tbl_vlan_group_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "valid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "min_vid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "max_vid" },
};

static cdb_field_t tbl_evc_entry_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_svid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_cvid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "evc_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_count" },
};

static cdb_field_t tbl_mapped_vlan_info_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_svid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mapped_cvid" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "raw_vid_bmp" },
};

static cdb_field_t tbl_l2_protocol_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "protocol_mac" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "group_mac" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_standard_pro" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protocol_index" },
};

static cdb_field_t tbl_nd_fib_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "state" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "permanent" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "active" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "router" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "fdb_age" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "uptime" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "probes" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "t_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipackets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "opackets" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "route_ref" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "is_static_route_dup" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "staic_route_dup_ipv6_nh" },
};

static cdb_field_t tbl_nd_stats_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_error" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_tooshort" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_invalid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_inconsistent" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_rs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_ra" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_ns" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_na" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_redirect" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "overflow_failed" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_miss" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_miss_conflict" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_static_conflict" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "allocs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "destroys" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "error" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "intf_invalid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "res_failed" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_probes_mcast" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rcv_probes_ucast" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "snd_solicited_na" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "snd_unsolicited_na" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "snd_dad_ns" },
};

static cdb_field_t tbl_cpu_mirror_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "strategy" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "buf_size" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "buf_list" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "write_file" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "flash_size_threshold" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "buf_mutex" },
    { CDB_ACCESS_RD,  CDB_DT_VOID,       -1,                  "capture_fp" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "capture_timer" },
};

static cdb_field_t tbl_tap_server_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "server_sock" },
    { CDB_ACCESS_RD,  CDB_DT_INT,        -1,                  "client_sock" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "peer_addr" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "lastread" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "debug" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "client_peer_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "client_test" },
};

static cdb_field_t tbl_dhcpv6_debug_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_error" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_events" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_packet" },
    { CDB_ACCESS_RW,  CDB_DT_FLAGS32,    -1,                  "clientv6_dump" },
};

static cdb_field_t tbl_l2edit_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "count" },
};

static cdb_field_t tbl_hash_field_profile_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "description" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_data" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flag" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l2_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ip_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ipv6_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "gre_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nvgre_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mpls_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vpws_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "l3vpn_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vxlan_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vpls_set" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_arithmetic" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hash_seed_value" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mode_symmetry" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_disable" },
};

static cdb_field_t tbl_hash_value_profile_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "description" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_data" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_command" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_all" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_input" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_static" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_resilient" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_all" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_l3" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_mpls" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_nvgre" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_vxlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_dynamic_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ref_cnt" },
};

static cdb_field_t tbl_hash_value_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_data" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "bitmap_command" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "agg_hash" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_all" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_l3" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_mpls" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_nvgre" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_static_vxlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_dynamic" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ecmp_dynamic_flow_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "lbid" },
};

static cdb_field_t tbl_nat_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_icmp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_tcp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timeout_udp" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_upstream_if" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_connlimit_post" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_connlimit_pre" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_upstream_filter_nat_rule" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "rule_id_opb" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "session_id_opb" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "category_id_opb" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "aging_timer" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "timer_mark" },
    { CDB_ACCESS_RW,  CDB_DT_VOID,       -1,                  "nfct_handle" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "rule_cfg_order" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_cnt_upstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_cnt_dnstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_fast_fwd_cnt_upstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_fast_fwd_cnt_dnstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nfct_kernel_notify_add_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nfct_kernel_notify_del_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nfct_notify_kernel_add_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nfct_notify_kernel_del_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_aging_del_cnt" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "session_aging_upd_ttl_cnt" },
};

static cdb_field_t tbl_nat_pool_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_lo" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_hi" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "ip_mask" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "refer_oif" },
};

static cdb_field_t tbl_nat_rule_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "handle_postrouting" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "handle_prerouting" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "category_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "refer_iif" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "refer_oif" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_chain_id_acl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_chain_id_rule_pre" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_chain_id_rule_post" },
    { CDB_ACCESS_RW,  CDB_DT_UINT64,     -1,                  "cfg_order" },
};

static cdb_field_t tbl_nat_session_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "id" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "timestamp" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "hit" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "upd_kernel_ttl_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ttl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "iif_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "oif_index" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "helper" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "slow_path_fwd" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "creator" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "set_sai_status" },
};

static cdb_field_t tbl_nat_session_limit_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "limit" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "count_upstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "count_dnstream" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_post" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "nft_hdl_pre" },
};

static cdb_field_t tbl_nat_proxy_arp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "enable" },
};

static cdb_field_t tbl_nat_rtif_inside_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_nat_rtif_outside_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_g8032_ring_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpl_owner" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpl_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rpl_next_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ring_level_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protection_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "config_sync" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "aps_protection_en" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protection_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mode_revertive" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "virtual_channel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "g8032_major_ring_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_sub_ring" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "instance_count" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "instance" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "sub_ring_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_local_fs" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_local_ms" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_rpl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_rpl_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_rpl_next_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_force_switch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_manual_switch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_blocked" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_flush_fdb" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_is_fail" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "east_port_rec_node_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_port_rec_bpr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_rpl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_rpl_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_rpl_next_neighbor" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_force_switch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_manual_switch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_blocked" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_flush_fdb" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_is_fail" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "west_port_rec_node_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_port_rec_bpr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "control_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_sub_control_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "is_fwdtocpu" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "raps_mel" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "raps_dnf" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "protection_trigger" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "primary_vlan" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "current_node_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority_request" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_expired" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wtr_timeout_val" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "wtb_timeout_val" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "guard_timeout_val" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hold_off_timeout_val" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "flush_fdb_timeout_val" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "cfm_md_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "cfm_ma_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "level" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "vlan_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "east_interface_cfm_state" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "west_interface_cfm_state" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_raps_tx_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_wtr_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_wtb_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_hold_off_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_guard_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_flush_fdb_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_tx_raps_nr_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_tx_raps_sf_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_tx_raps_fs_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_tx_raps_ms_while" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "p_tx_raps_nr_nb_while" },
};

static cdb_field_t tbl_cfm_global_fields[] = 
{
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cfm_enable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cfm_mode" },
};

static cdb_field_t tbl_cfm_md_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "level" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "name_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "pkt_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "pkt_name_length" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "pkt_name_input" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "service_number" },
    { CDB_ACCESS_SUB, CDB_DT_REFERLIST,  DS_CFM_ERROR,        "cfm_error" },
};

static cdb_field_t tbl_cfm_ma_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "name_type" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cci_enabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ccm_vlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "pkt_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "maid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "error_write_disable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "error_number" },
};

static cdb_field_t tbl_cfm_maid_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_cfm_lmep_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mep_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "maid" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "md_name" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "direction" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "d_unexp_mep" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "d_mismerge" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "d_meg_lvl" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cci_enabled" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "cci_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rdi" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "if_mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ccm_vlan_cos" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ring_id" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "mep_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rmep_num" },
};

static cdb_field_t tbl_cfm_rmep_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ifindex" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "md_name" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "maid" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "mac_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "first_pkt_rx" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "d_loc" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "d_unexp_period" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "ma_sa_mismatch" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "rmep_last_rdi" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "ifname" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "local_mep_id" },
};

static cdb_field_t tbl_pim_sm_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "rp_addr" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hello_time" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "spt_infinity" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "jp_timer" },
};

static cdb_field_t tbl_pim_sm_bsr_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dr_pri" },
};

static cdb_field_t tbl_pim_sm_rp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "priority" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "interval" },
};

static cdb_field_t tbl_pim_sm_rp_gp_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_COMPOSE,    -1,                  "group_prefix" },
};

static cdb_field_t tbl_pim_dm_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
};

static cdb_field_t tbl_igmp_intf_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "querier_timeout" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "robustness_variable" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "layer3_protocol_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "layer3_protocol" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "entry_valid" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "action_index" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "last_member_query_count" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "last_member_query_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "global_limit_number" },
};

static cdb_field_t tbl_rip_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "version" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "allow_ecmp" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "timers" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_information" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "redistribute" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "red_metric" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_metric" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "passive_if_default" },
};

static cdb_field_t tbl_ripng_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "allow_ecmp" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "timers" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_information" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "redistribute" },
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "red_metric" },
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "default_metric" },
};

static cdb_field_t tbl_rip_network_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_ripng_network_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_rip_neighbor_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
};

static cdb_field_t tbl_rip_passive_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_ripng_passive_if_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_STRING,     -1,                  "key" },
};

static cdb_field_t tbl_ospf_area_stub_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "no_summary" },
};

static cdb_field_t tbl_ospf6_area_stub_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "no_summary" },
};

static cdb_field_t tbl_ospf_area_nssa_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_UINT,       -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "nssa_role" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "no_summary" },
};

static cdb_field_t tbl_ospf_area_vlink_fields[] = 
{
    { CDB_ACCESS_RD,  CDB_DT_COMPOSE,    -1,                  "key" },
    { CDB_ACCESS_RW,  CDB_DT_INT,        -1,                  "auth_flag" },
    { CDB_ACCESS_RW,  CDB_DT_ENUM32,     -1,                  "auth_type" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "auth_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "md5_id" },
    { CDB_ACCESS_RW,  CDB_DT_STRING,     -1,                  "md5_key" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "hello_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "dead_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "retransmit_interval" },
    { CDB_ACCESS_RW,  CDB_DT_UINT,       -1,                  "transmit_delay" },
};

#endif /* !__CDB_TBL_FIELD_PRIV_H__ */

