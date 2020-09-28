
#include "proto.h"
#include "cdb_store.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_fea_port_if_define.h"
#include "gen/tbl_fea_port_if.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_ipsg_s_ip_define.h"
#include "gen/tbl_ipsg_s_ip.h"
#include "gen/tbl_ipsg_s_mac_define.h"
#include "gen/tbl_ipsg_s_mac.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_isolation_define.h"
#include "gen/tbl_isolation.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_bgp_define.h"
#include "gen/tbl_bgp.h"
#include "gen/tbl_bgp_network_define.h"
#include "gen/tbl_bgp_network.h"
#include "gen/tbl_bgp_neighbor_define.h"
#include "gen/tbl_bgp_neighbor.h"
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
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_iproute_node_add_fail_define.h"
#include "gen/tbl_iproute_node_add_fail.h"
#include "gen/tbl_iproute_node_add_fail_count_define.h"
#include "gen/tbl_iproute_node_add_fail_count.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_static_rt_cnt_define.h"
#include "gen/tbl_static_rt_cnt.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"
#include "gen/tbl_nexthop_group_routed_define.h"
#include "gen/tbl_nexthop_group_routed.h"
#include "gen/tbl_fea_nexthop_define.h"
#include "gen/tbl_fea_nexthop.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "gen/tbl_rr_prefix_define.h"
#include "gen/tbl_rr_prefix.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_cpu_mem_history_define.h"
#include "gen/tbl_cpu_mem_history.h"
#include "gen/tbl_chsm_debug_define.h"
#include "gen/tbl_chsm_debug.h"
#include "gen/tbl_switch_debug_define.h"
#include "gen/tbl_switch_debug.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "gen/tbl_ssm_debug_define.h"
#include "gen/tbl_ssm_debug.h"
#include "gen/tbl_ptp_debug_define.h"
#include "gen/tbl_ptp_debug.h"
#include "gen/tbl_quagga_debug_define.h"
#include "gen/tbl_quagga_debug.h"
#include "gen/tbl_lsrv_debug_define.h"
#include "gen/tbl_lsrv_debug.h"
#include "gen/tbl_hsrv_debug_define.h"
#include "gen/tbl_hsrv_debug.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_app_debug.h"
#include "gen/tbl_openflow_debug_define.h"
#include "gen/tbl_openflow_debug.h"
#include "gen/tbl_rif_define.h"
#include "gen/tbl_rif.h"
#include "gen/tbl_fea_lag_define.h"
#include "gen/tbl_fea_lag.h"
#include "gen/tbl_fea_global_define.h"
#include "gen/tbl_fea_global.h"
#include "gen/tbl_fea_acl_table_define.h"
#include "gen/tbl_fea_acl_table.h"
#include "gen/tbl_fea_acl_define.h"
#include "gen/tbl_fea_acl.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "gen/tbl_fea_brg_if_define.h"
#include "gen/tbl_fea_brg_if.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_entry_define.h"
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_entry_action_define.h"
#include "gen/tbl_acl_entry_action.h"
#include "gen/tbl_acl_nexthop_group_define.h"
#include "gen/tbl_acl_nexthop_group.h"
#include "gen/tbl_acl_nexthop_define.h"
#include "gen/tbl_acl_nexthop.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_context.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_group_usm.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "gen/tbl_snmp_trap_msg_define.h"
#include "gen/tbl_snmp_trap_msg.h"
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
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_server.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_ace.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_key.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_cfg.h"
#include "gen/tbl_ntp_if_define.h"
#include "gen/tbl_ntp_if.h"
#include "gen/tbl_ntp_syncstatus_define.h"
#include "gen/tbl_ntp_syncstatus.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "gen/tbl_qos_domain_define.h"
#include "gen/tbl_qos_domain.h"
#include "gen/tbl_qos_mpls_domain_define.h"
#include "gen/tbl_qos_mpls_domain.h"
#include "gen/tbl_qos_policer_res_define.h"
#include "gen/tbl_qos_policer_res.h"
#include "gen/tbl_qos_policer_action_profile_define.h"
#include "gen/tbl_qos_policer_action_profile.h"
#include "gen/tbl_qos_policer_profile_define.h"
#include "gen/tbl_qos_policer_profile.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "gen/tbl_qos_drop_profile.h"
#include "gen/tbl_qos_queue_shape_profile_define.h"
#include "gen/tbl_qos_queue_shape_profile.h"
#include "gen/tbl_qos_port_shape_profile_define.h"
#include "gen/tbl_qos_port_shape_profile.h"
#include "gen/tbl_qos_queue_smart_buffer_define.h"
#include "gen/tbl_qos_queue_smart_buffer.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
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
#include "gen/tbl_tap_dpi_global_define.h"
#include "gen/tbl_tap_dpi_global.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_login_record_define.h"
#include "gen/tbl_login_record.h"
#include "gen/tbl_login_security_glb_define.h"
#include "gen/tbl_login_security_glb.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include "gen/tbl_cpu_traffic_define.h"
#include "gen/tbl_cpu_traffic.h"
#include "gen/tbl_cpu_traffic_group_define.h"
#include "gen/tbl_cpu_traffic_group.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcrelay.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_iptables_prevent_define.h"
#include "gen/tbl_iptables_prevent.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"
#include "gen/tbl_log_global_define.h"
#include "gen/tbl_log_global.h"
#include "gen/tbl_log_define.h"
#include "gen/tbl_log.h"
#include "gen/tbl_sys_load_define.h"
#include "gen/tbl_sys_load.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "gen/tbl_tag_summer_clock_define.h"
#include "gen/tbl_tag_summer_clock.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_vlan_stats_define.h"
#include "gen/tbl_vlan_stats.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_aclqos_if_define.h"
#include "gen/tbl_aclqos_if.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_fea_qos_drop_profile_define.h"
#include "gen/tbl_fea_qos_drop_profile.h"
#include "gen/tbl_fea_qos_domain_define.h"
#include "gen/tbl_fea_qos_domain.h"
#include "gen/tbl_fea_g8032_ring_define.h"
#include "gen/tbl_fea_g8032_ring.h"
#include "gen/tbl_fea_g8032_ring_mc_group_define.h"
#include "gen/tbl_fea_g8032_ring_mc_group.h"
#include "gen/tbl_fea_qos_queue_shape_profile_define.h"
#include "gen/tbl_fea_qos_queue_shape_profile.h"
#include "gen/tbl_fea_qos_port_shape_profile_define.h"
#include "gen/tbl_fea_qos_port_shape_profile.h"
#include "gen/tbl_fea_port_policer_apply_define.h"
#include "gen/tbl_fea_port_policer_apply.h"
#include "gen/tbl_aclqos_if_stats_define.h"
#include "gen/tbl_aclqos_if_stats.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_manage_if.h"
#include "gen/tbl_bootimage_define.h"
#include "gen/tbl_bootimage.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_ifname_info_define.h"
#include "gen/tbl_ifname_info.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_port_define.h"
#include "gen/tbl_port.h"
#include "gen/tbl_fiber_define.h"
#include "gen/tbl_fiber.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_fan_define.h"
#include "gen/tbl_fan.h"
#include "gen/tbl_psu_define.h"
#include "gen/tbl_psu.h"
#include "gen/tbl_led_define.h"
#include "gen/tbl_led.h"
#include "gen/tbl_sensor_define.h"
#include "gen/tbl_sensor.h"
#include "gen/tbl_vcm_define.h"
#include "gen/tbl_vcm.h"
#include "gen/tbl_poe_define.h"
#include "gen/tbl_poe.h"
#include "gen/tbl_reboot_info_define.h"
#include "gen/tbl_reboot_info.h"
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"
#include "gen/tbl_opm_global_define.h"
#include "gen/tbl_opm_global.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_opm_debug.h"
#include "gen/tbl_policy_map_config_define.h"
#include "gen/tbl_policy_map_config.h"
#include "gen/tbl_class_map_config_define.h"
#include "gen/tbl_class_map_config.h"
#include "gen/tbl_class_in_policy_config_define.h"
#include "gen/tbl_class_in_policy_config.h"
#include "gen/tbl_acl_in_class_config_define.h"
#include "gen/tbl_acl_in_class_config.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_class_map_action_config.h"
#include "gen/tbl_fea_acl_policy_action_define.h"
#include "gen/tbl_fea_acl_policy_action.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_authd_debug.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_enable.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_clear_acl_policy_define.h"
#include "gen/tbl_clear_acl_policy.h"
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
#include "gen/tbl_acl_l4_port_range_define.h"
#include "gen/tbl_acl_l4_port_range.h"
#include "gen/tbl_acl_udf_group_define.h"
#include "gen/tbl_acl_udf_group.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_fea_pcap_define.h"
#include "gen/tbl_fea_pcap.h"
#include "gen/tbl_controller_define.h"
#include "gen/tbl_controller.h"
#include "gen/tbl_cpu_packets_define.h"
#include "gen/tbl_cpu_packets.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_ns_route_ip_define.h"
#include "gen/tbl_ns_route_ip.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "gen/tbl_openflow_manager_define.h"
#include "gen/tbl_openflow_manager.h"
#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"
#include "gen/tbl_ptp_residence_time_cache_define.h"
#include "gen/tbl_ptp_residence_time_cache.h"
#include "gen/tbl_fea_ptp_global_define.h"
#include "gen/tbl_fea_ptp_global.h"
#include "gen/tbl_fea_ptp_ts_define.h"
#include "gen/tbl_fea_ptp_ts.h"
#include "gen/tbl_fea_time_define.h"
#include "gen/tbl_fea_time.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "gen/tbl_bhm_module_define.h"
#include "gen/tbl_bhm_module.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt_define.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "gen/tbl_arpace_config_define.h"
#include "gen/tbl_arpace_config.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "gen/tbl_fea_copp_trap_group_define.h"
#include "gen/tbl_fea_copp_trap_group.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "gen/tbl_sflow_collector_define.h"
#include "gen/tbl_sflow_collector.h"
#include "gen/tbl_sflow_counter_port_define.h"
#include "gen/tbl_sflow_counter_port.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
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
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_controller_attr_define.h"
#include "gen/tbl_controller_attr.h"
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
#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_tpoam_exec_global_define.h"
#include "gen/tbl_tpoam_exec_global.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
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
#include "gen/tbl_nd_stats_define.h"
#include "gen/tbl_nd_stats.h"
#include "gen/tbl_cpu_mirror_global_define.h"
#include "gen/tbl_cpu_mirror_global.h"
#include "gen/tbl_tap_server_define.h"
#include "gen/tbl_tap_server.h"
#include "gen/tbl_dhcpv6_debug_define.h"
#include "gen/tbl_dhcpv6_debug.h"
#include "gen/tbl_l2edit_define.h"
#include "gen/tbl_l2edit.h"
#include "gen/tbl_hash_field_profile_define.h"
#include "gen/tbl_hash_field_profile.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_hash_value_global_define.h"
#include "gen/tbl_hash_value_global.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
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
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
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
#include "gen/tbl_pim_sm_define.h"
#include "gen/tbl_pim_sm.h"
#include "gen/tbl_pim_sm_bsr_define.h"
#include "gen/tbl_pim_sm_bsr.h"
#include "gen/tbl_pim_sm_rp_define.h"
#include "gen/tbl_pim_sm_rp.h"
#include "gen/tbl_pim_sm_rp_gp_define.h"
#include "gen/tbl_pim_sm_rp_gp.h"
#include "gen/tbl_pim_dm_define.h"
#include "gen/tbl_pim_dm.h"
#include "gen/tbl_igmp_intf_define.h"
#include "gen/tbl_igmp_intf.h"
#include "gen/tbl_rip_define.h"
#include "gen/tbl_rip.h"
#include "gen/tbl_ripng_define.h"
#include "gen/tbl_ripng.h"
#include "gen/tbl_rip_network_define.h"
#include "gen/tbl_rip_network.h"
#include "gen/tbl_ripng_network_define.h"
#include "gen/tbl_ripng_network.h"
#include "gen/tbl_rip_neighbor_define.h"
#include "gen/tbl_rip_neighbor.h"
#include "gen/tbl_rip_passive_if_define.h"
#include "gen/tbl_rip_passive_if.h"
#include "gen/tbl_ripng_passive_if_define.h"
#include "gen/tbl_ripng_passive_if.h"
#include "gen/tbl_ospf_area_stub_define.h"
#include "gen/tbl_ospf_area_stub.h"
#include "gen/tbl_ospf6_area_stub_define.h"
#include "gen/tbl_ospf6_area_stub.h"
#include "gen/tbl_ospf_area_nssa_define.h"
#include "gen/tbl_ospf_area_nssa.h"
#include "gen/tbl_ospf_area_vlink_define.h"
#include "gen/tbl_ospf_area_vlink.h"

static int32
_cdb_format_tbl_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_interface_t *p_if = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_if = (tbl_interface_t*)p_tbl;

    p_if->errdisable_timer = NULL;
    p_if->circuit_id.obj_list = NULL;
    p_if->brgif = NULL;
    p_if->lag = NULL;
    p_if->lacp = NULL;
    p_if->storm_control = NULL;
    p_if->aclqos_if = NULL;
    p_if->openflow_if = NULL;
    p_if->dhclient_if = NULL;
    p_if->l2protocol.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_route_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_route_if_t *p_rtif = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rtif = (tbl_route_if_t*)p_tbl;

    p_rtif->t_ra_solicited = NULL;
    p_rtif->t_ra_unsolicited = NULL;
    p_rtif->t_delete_all_v6_addr_timer = NULL;
    p_rtif->t_ospf_cost_timer = NULL;
    p_rtif->t_send_arp_timer = NULL;
    p_rtif->ifc_ipv4.obj_list = NULL;
    p_rtif->ifc_ipv6.obj_list = NULL;
    p_rtif->ospf_md5_key.obj_list = NULL;
    p_rtif->nd_prefix.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_kernel_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_port_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pvlan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pvlan_t *p_pvlan = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_pvlan = (tbl_pvlan_t*)p_tbl;

    p_pvlan->community_groups.obj_list = NULL;
    p_pvlan->isloate_ports.obj_list = NULL;
    p_pvlan->promiscuous_ports.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mcfdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_macfilter(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_psfdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsg_s_ip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsg_s_mac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsg_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_brg_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mstp_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_port_t *p_mstp_port = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_mstp_port = (tbl_mstp_port_t*)p_tbl;

    p_mstp_port->bpdu = NULL;
    p_mstp_port->port_timer = NULL;
    p_mstp_port->edgeDelayWhile_timer = NULL;
    p_mstp_port->fdWhile_timer = NULL;
    p_mstp_port->helloWhen_timer = NULL;
    p_mstp_port->mdelayWhile_timer = NULL;
    p_mstp_port->rbWhile_timer = NULL;
    p_mstp_port->rcvdInfoWhile_timer = NULL;
    p_mstp_port->rrWhile_timer = NULL;
    p_mstp_port->tcWhile_timer = NULL;
    p_mstp_port->hold_timer = NULL;
    p_mstp_port->msti_list.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_msti_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_msti_port_t *p_msti_port = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_msti_port = (tbl_msti_port_t*)p_tbl;

    p_msti_port->bpdu = NULL;
    p_msti_port->fdWhile_timer = NULL;
    p_msti_port->mdelayWhile_timer = NULL;
    p_msti_port->rbWhile_timer = NULL;
    p_msti_port->rcvdInfoWhile_timer = NULL;
    p_msti_port->rrWhile_timer = NULL;
    p_msti_port->tcWhile_timer = NULL;
    p_msti_port->hold_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mstp_instance(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_instance_t *p_inst = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_inst = (tbl_mstp_instance_t*)p_tbl;

    p_inst->port_list.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mstp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_global_t *p_mstp_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_mstp_glb = (tbl_mstp_global_t*)p_tbl;

    p_mstp_glb->tc_protection_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lldp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lldp_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lldp_if_t *p_lldp_if = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_lldp_if = (tbl_lldp_if_t*)p_tbl;

    p_lldp_if->tx_timer = NULL;
    p_lldp_if->neigh_list.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mlag(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mlag_t *p_mlag = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_mlag = (tbl_mlag_t*)p_tbl;

    p_mlag->reload_delay_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mlag_peer(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mlag_peer_t *p_peer = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_peer = (tbl_mlag_peer_t*)p_tbl;

    p_peer->t_auto_start = NULL;
    p_peer->t_holdtime = NULL;
    p_peer->t_keepalive = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mlag_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_isolation(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_isolation_t *p_iso = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_iso = (tbl_isolation_t*)p_tbl;

    p_iso->ports.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_route_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_route_global_t *p_rt_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rt_glb = (tbl_route_global_t*)p_tbl;

    p_rt_glb->arp_aging_timer = NULL;
    p_rt_glb->arpratelimit_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bgp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bgp_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bgp_neighbor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_t *p_ospf = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ospf = (tbl_ospf_t*)p_tbl;

    p_ospf->t_ospf_redistribute_connected_timer = NULL;
    p_ospf->t_ospf_redistribute_static_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf6(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf6_t *p_ospf6 = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ospf6 = (tbl_ospf6_t*)p_tbl;

    p_ospf6->t_ospf6_redistribute_connected_timer = NULL;
    p_ospf6->t_ospf6_redistribute_static_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf6_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_area_auth(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_area_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf6_area_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_iproute_node(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_iproute_node_add_fail(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_iproute_node_add_fail_count(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_static_route_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_static_rt_cnt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_arp_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_arp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arp_t *p_arp = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_arp = (tbl_arp_t*)p_tbl;

    p_arp->static_route_dup_ipv4_nh.obj_list = NULL;
    p_arp->retry_timer = NULL;
    p_arp->refresh_timer = NULL;
    p_arp->gra_rsp_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nexthop_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nexthop_group_routed(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mrt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipmc_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipmc_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_static_rp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sys_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sys_global_t *p_sys_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_sys_glb = (tbl_sys_global_t*)p_tbl;

    p_sys_glb->mgmt_if_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lag_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ecmp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rr_prefix(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mem_summary(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_mem_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_chsm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_switch_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_route_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ssm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ptp_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_quagga_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lsrv_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_hsrv_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_app_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rif(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_lag(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_acl_table(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_acl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_fdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_brg_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_worm_filter(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ace_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_entry_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_nexthop_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_time_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ssh_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_cfg_t *p_snmp_cfg = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_snmp_cfg = (tbl_snmp_cfg_t*)p_tbl;

    p_snmp_cfg->t_snmp_trap_all_timer = NULL;
    p_snmp_cfg->t_snmp_enable_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_view(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_community(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_context(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_usm_user(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_group_usm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_access_usm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_notify(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_target_addr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_target_params(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_trap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_inform(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_snmp_trap_msg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rmon_event(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_event_t *p_rmon_event = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rmon_event = (tbl_rmon_event_t*)p_tbl;

    p_rmon_event->t_under_creation_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rmon_alarm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_alarm_t *p_rmon_alarm = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rmon_alarm = (tbl_rmon_alarm_t*)p_tbl;

    p_rmon_alarm->t_alarm_timer = NULL;
    p_rmon_alarm->t_under_creation_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rmon_ether_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_ether_stats_t *p_rmon_ether_stats = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rmon_ether_stats = (tbl_rmon_ether_stats_t*)p_tbl;

    p_rmon_ether_stats->t_under_creation_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rmon_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_history_t *p_rmon_history = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_rmon_history = (tbl_rmon_history_t*)p_tbl;

    p_rmon_history->t_under_creation_timer = NULL;
    p_rmon_history->t_coll_history_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rmon_stats_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_syslog_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_ace(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_key(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ntp_syncstatus(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_static_dns(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dynamic_dns_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dynamic_dns_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_mpls_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_policer_res(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_policer_action_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_policer_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_drop_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_queue_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_port_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_queue_smart_buffer(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_qos_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mirror(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mirror_mac_escape(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_group_ingress(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_group_ingress_flow(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_group_egress(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_five_tuple(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_dpi_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_user(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_login_record(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_login_security_glb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vty(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_console(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_authen(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_login(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rsa(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_traffic(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_traffic_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_utilization(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_limit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcrelay(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcsrvgrp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcp_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhclient(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcsnooping(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcbinding(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_iptables_prevent(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_errdisable(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ns_port_forwarding(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_log_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_log(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sys_load(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cem(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_clock(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tag_summer_clock(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tag_summer_clock_t *p_tag_summer_clk = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_tag_summer_clk = (tbl_tag_summer_clock_t*)p_tbl;

    p_tag_summer_clk->t_summer_time_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_port_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlan_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_port_stats_rate(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_port_stats_rate_t *p_port_stats_rate = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_port_stats_rate = (tbl_port_stats_rate_t*)p_tbl;

    p_port_stats_rate->t_output_discard_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_aclqos_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_l2_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_l2_action_t *p_l2_act = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_l2_act = (tbl_l2_action_t*)p_tbl;

    p_l2_act->flush_fdb = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_qos_drop_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_qos_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_g8032_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_g8032_ring_mc_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_qos_queue_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_qos_port_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_port_policer_apply(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_aclqos_if_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_version(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_manage_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bootimage(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_chassis(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_chassis_t *p_chassis = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_chassis = (tbl_chassis_t*)p_tbl;

    p_chassis->schedule_reboot_timer = NULL;
    p_chassis->schedule_reboot_alarm_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ifname_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_card(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fiber(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sys_spec(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_psu(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_led(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sensor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vcm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_poe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_reboot_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_errdisable_flap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_opm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_erps_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_erps_ring_t *p_erps_ring = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_erps_ring = (tbl_erps_ring_t*)p_tbl;

    p_erps_ring->hello_timer = NULL;
    p_erps_ring->failure_timer = NULL;
    p_erps_ring->pre_forwarding_timer = NULL;
    p_erps_ring->flush_timer = NULL;
    p_erps_ring->edge_hello_timer = NULL;
    p_erps_ring->edge_failure_timer = NULL;
    p_erps_ring->edge_block_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_erps_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_erps_domain_t *p_erps_domain = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_erps_domain = (tbl_erps_domain_t*)p_tbl;

    p_erps_domain->erps_ring_list.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_opm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_policy_map_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_class_map_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_class_in_policy_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_in_class_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_class_map_action_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_acl_policy_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_igsp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_global_t *p_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_glb = (tbl_igsp_global_t*)p_tbl;

    p_glb->t_group_expire = NULL;
    p_glb->t_tcn_query = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_igsp_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_intf_t *p_if = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_if = (tbl_igsp_intf_t*)p_tbl;

    p_if->t_querier = NULL;
    p_if->t_other_querier = NULL;
    p_if->mrouter_list.obj_list = NULL;
    p_if->query_list.obj_list = NULL;
    p_if->join_list.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_igsp_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_group_t *p_group = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_group = (tbl_igsp_group_t*)p_tbl;

    p_group->t_retx_query = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_auth_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_auth_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_auth_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_authd_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dot1x_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dot1x_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_port_t *p_dot1x_port = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_dot1x_port = (tbl_dot1x_port_t*)p_tbl;

    p_dot1x_port->current_radius = NULL;
    p_dot1x_port->t_wait = NULL;
    p_dot1x_port->t_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dot1x_radius(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dot1x_mac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_mac_t *p_dot1x_mac = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_dot1x_mac = (tbl_dot1x_mac_t*)p_tbl;

    p_dot1x_mac->current_radius = NULL;
    p_dot1x_mac->t_wait = NULL;
    p_dot1x_mac->t_reauth = NULL;
    p_dot1x_mac->t_reject = NULL;
    p_dot1x_mac->t_delete = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_enable(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_chip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_clear_acl_policy(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_author(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_account(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_accountcmd(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlanclass_rule(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlanclass_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_l4_port_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_udf_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_acl_udf_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_pcap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_controller(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_packets(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ns_route(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ns_route_ip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow_manager(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ptp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_global_t *p_global = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_global = (tbl_ptp_global_t*)p_tbl;

    p_global->p_internal_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ptp_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_port_t *p_port = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_port = (tbl_ptp_port_t*)p_tbl;

    p_port->ptm_announce_timer = NULL;
    p_port->ptm_sync_timer = NULL;
    p_port->ptm_delay_timer = NULL;
    p_port->ptm_pdelay_timer = NULL;
    p_port->ptm_announce_receipt_timer = NULL;
    p_port->ptm_qualification_timer = NULL;
    p_port->p_timer_faulty_recover_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ptp_foreign(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_foreign_t *p_foreign = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_foreign = (tbl_ptp_foreign_t*)p_tbl;

    p_foreign->ptm_fm_qualification_timer = NULL;
    p_foreign->ptm_fm_announce_receipt_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ptp_residence_time_cache(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_residence_time_cache_t *p_residence_time_cache = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_residence_time_cache = (tbl_ptp_residence_time_cache_t*)p_tbl;

    p_residence_time_cache->ptm_age_task_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_ptp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_fea_ptp_global = (tbl_fea_ptp_global_t*)p_tbl;

    p_fea_ptp_global->hsrv_ptp_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_ptp_ts(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_time(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bhm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bhm_global_t *p_bhm_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_bhm_glb = (tbl_bhm_global_t*)p_tbl;

    p_bhm_glb->sw_watchdog_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_bhm_module(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow_tunnel_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_openflow_tunnel_local_ip_cnt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_inband_snat(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_inband_snat_t *p_inband_snat = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_inband_snat = (tbl_inband_snat_t*)p_tbl;

    p_inband_snat->snat_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_arpinsp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_arpacl_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_arpace_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_copp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_fea_copp_trap_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sflow_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_sflow_glb = (tbl_sflow_global_t*)p_tbl;

    p_sflow_glb->counter_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sflow_collector(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sflow_collector_t *p_sflow_collector = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_sflow_collector = (tbl_sflow_collector_t*)p_tbl;

    p_sflow_collector->transdelay_timer = NULL;
    p_sflow_collector->t_log_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_sflow_counter_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vrrp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vrrp_global_t *p_vrrp_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_vrrp_glb = (tbl_vrrp_global_t*)p_tbl;

    p_vrrp_glb->t_vrrp_timer = NULL;
    p_vrrp_glb->t_vrrp_msec_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vrrp_vmac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vrrp_vip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vrrp_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ssm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssm_global_t *p_ssm_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ssm_glb = (tbl_ssm_global_t*)p_tbl;

    p_ssm_glb->holdoff_send_timer = NULL;
    p_ssm_glb->wtr_send_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ssm_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssm_port_t *p_ssm_port = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ssm_port = (tbl_ssm_port_t*)p_tbl;

    p_ssm_port->ssm_timeout_timer = NULL;
    p_ssm_port->ssm_send_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsla_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_global_t *p_ipsla_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ipsla_glb = (tbl_ipsla_global_t*)p_tbl;

    p_ipsla_glb->t_ipsla_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsla_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_entry_t *p_ipsla_entry = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_ipsla_entry = (tbl_ipsla_entry_t*)p_tbl;

    p_ipsla_entry->t_ipsla_read = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsla_packet_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ipsla_test_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_track_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_track_global_t *p_track_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_track_glb = (tbl_track_global_t*)p_tbl;

    p_track_glb->t_track_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_track_object(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_controller_attr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_oam_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_session_t *p_oam_session = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_oam_session = (tbl_oam_session_t*)p_tbl;

    p_oam_session->p_t_send_dm_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_oam_session_upload(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lsp_aps_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pw_aps_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lsp_pe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_lsp_p(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pw_tpe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_section_oam(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_oam_flow_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tpoam_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tpoam_global_t *p_tpoam_global = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_tpoam_global = (tbl_tpoam_global_t*)p_tbl;

    p_tpoam_global->lm_calc_interval_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tpoam_exec_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_tpoam_exec_global = (tbl_tpoam_exec_global_t*)p_tbl;

    p_tpoam_exec_global->p_t_send_lb_msg_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_aps_group_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_aps_group_info_t *p_aps_group_info = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_aps_group_info = (tbl_aps_group_info_t*)p_tbl;

    p_aps_group_info->wtr_timer = NULL;
    p_aps_group_info->sf_hold_off_timer = NULL;
    p_aps_group_info->sfp_hold_off_timer = NULL;
    p_aps_group_info->sd_hold_off_timer = NULL;
    p_aps_group_info->sdp_hold_off_timer = NULL;
    p_aps_group_info->sc_out_aps_timer = NULL;
    p_aps_group_info->aps_tx_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_oam_inlabel_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_oam_mpls_aps_check(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlan_mapping_table(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlan_mapping_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_egress_mapping_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_vlan_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_evc_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_mapped_vlan_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_l2_protocol(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nd_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nd_fib_t *p_nd_fib = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_nd_fib = (tbl_nd_fib_t*)p_tbl;

    p_nd_fib->t_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nd_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cpu_mirror_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_mirror_global_t *p_cpu_mirror = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_cpu_mirror = (tbl_cpu_mirror_global_t*)p_tbl;

    p_cpu_mirror->buf_list = NULL;
    p_cpu_mirror->buf_mutex = NULL;
    p_cpu_mirror->capture_fp = NULL;
    p_cpu_mirror->capture_timer = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_tap_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_dhcpv6_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_l2edit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_hash_field_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_hash_value_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_hash_value_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_global_t *p_nat_glb = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_nat_glb = (tbl_nat_global_t*)p_tbl;

    p_nat_glb->aging_timer = NULL;
    p_nat_glb->nfct_handle = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_pool(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_rule(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_session_limit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_proxy_arp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_rtif_inside(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_nat_rtif_outside(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_g8032_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_g8032_ring_t *p_g8032_ring = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_g8032_ring = (tbl_g8032_ring_t*)p_tbl;

    p_g8032_ring->p_raps_tx_while = NULL;
    p_g8032_ring->p_wtr_while = NULL;
    p_g8032_ring->p_wtb_while = NULL;
    p_g8032_ring->p_hold_off_while = NULL;
    p_g8032_ring->p_guard_while = NULL;
    p_g8032_ring->p_flush_fdb_while = NULL;
    p_g8032_ring->p_tx_raps_nr_while = NULL;
    p_g8032_ring->p_tx_raps_sf_while = NULL;
    p_g8032_ring->p_tx_raps_fs_while = NULL;
    p_g8032_ring->p_tx_raps_ms_while = NULL;
    p_g8032_ring->p_tx_raps_nr_nb_while = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_md(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_md_t *p_md = NULL;

    if (CDB_OPER_DEL == oper)
    {
        return PM_E_NONE;
    }
    p_md = (tbl_cfm_md_t*)p_tbl;

    p_md->cfm_error.obj_list = NULL;

    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_ma(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_maid(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_lmep(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_cfm_rmep(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_sm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_sm_bsr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_sm_rp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_sm_rp_gp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_pim_dm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_igmp_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ripng(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rip_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ripng_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rip_neighbor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_rip_passive_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ripng_passive_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_area_stub(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf6_area_stub(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_area_nssa(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

static int32
_cdb_format_tbl_ospf_area_vlink(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    return PM_E_NONE;
}

CDB_STORE_TBL_FUNC g_cdb_format_tbl_func[TBL_MAX] = 
{
    _cdb_format_tbl_interface,
    _cdb_format_tbl_route_if,
    _cdb_format_tbl_kernel_if,
    _cdb_format_tbl_fea_port_if,
    _cdb_format_tbl_vlan,
    _cdb_format_tbl_pvlan,
    _cdb_format_tbl_fdb,
    _cdb_format_tbl_mcfdb,
    _cdb_format_tbl_macfilter,
    _cdb_format_tbl_psfdb,
    _cdb_format_tbl_ipsg_s_ip,
    _cdb_format_tbl_ipsg_s_mac,
    _cdb_format_tbl_ipsg_fib,
    _cdb_format_tbl_brg_global,
    _cdb_format_tbl_mstp_port,
    _cdb_format_tbl_msti_port,
    _cdb_format_tbl_mstp_instance,
    _cdb_format_tbl_mstp_global,
    _cdb_format_tbl_lldp_global,
    _cdb_format_tbl_lldp_if,
    _cdb_format_tbl_mlag,
    _cdb_format_tbl_mlag_peer,
    _cdb_format_tbl_mlag_port,
    _cdb_format_tbl_isolation,
    _cdb_format_tbl_route_global,
    _cdb_format_tbl_bgp,
    _cdb_format_tbl_bgp_network,
    _cdb_format_tbl_bgp_neighbor,
    _cdb_format_tbl_ospf,
    _cdb_format_tbl_ospf6,
    _cdb_format_tbl_ospf6_interface,
    _cdb_format_tbl_ospf_network,
    _cdb_format_tbl_ospf_area_auth,
    _cdb_format_tbl_ospf_area_range,
    _cdb_format_tbl_ospf6_area_range,
    _cdb_format_tbl_iproute_node,
    _cdb_format_tbl_iproute_node_add_fail,
    _cdb_format_tbl_iproute_node_add_fail_count,
    _cdb_format_tbl_static_route_cfg,
    _cdb_format_tbl_static_rt_cnt,
    _cdb_format_tbl_arp_fib,
    _cdb_format_tbl_arp,
    _cdb_format_tbl_nexthop,
    _cdb_format_tbl_nexthop_group,
    _cdb_format_tbl_nexthop_group_routed,
    _cdb_format_tbl_fea_nexthop,
    _cdb_format_tbl_mrt,
    _cdb_format_tbl_ipmc_global,
    _cdb_format_tbl_ipmc_intf,
    _cdb_format_tbl_pim_static_rp,
    _cdb_format_tbl_sys_global,
    _cdb_format_tbl_lag_global,
    _cdb_format_tbl_ecmp_global,
    _cdb_format_tbl_rr_prefix,
    _cdb_format_tbl_mem_summary,
    _cdb_format_tbl_cpu_mem_history,
    _cdb_format_tbl_chsm_debug,
    _cdb_format_tbl_switch_debug,
    _cdb_format_tbl_route_debug,
    _cdb_format_tbl_ssm_debug,
    _cdb_format_tbl_ptp_debug,
    _cdb_format_tbl_quagga_debug,
    _cdb_format_tbl_lsrv_debug,
    _cdb_format_tbl_hsrv_debug,
    _cdb_format_tbl_app_debug,
    _cdb_format_tbl_openflow_debug,
    _cdb_format_tbl_rif,
    _cdb_format_tbl_fea_lag,
    _cdb_format_tbl_fea_global,
    _cdb_format_tbl_fea_acl_table,
    _cdb_format_tbl_fea_acl,
    _cdb_format_tbl_fea_fdb,
    _cdb_format_tbl_fea_brg_if,
    _cdb_format_tbl_acl_worm_filter,
    _cdb_format_tbl_acl_config,
    _cdb_format_tbl_ace_config,
    _cdb_format_tbl_acl_entry,
    _cdb_format_tbl_acl_entry_action,
    _cdb_format_tbl_acl_nexthop_group,
    _cdb_format_tbl_acl_nexthop,
    _cdb_format_tbl_time_range,
    _cdb_format_tbl_ssh_cfg,
    _cdb_format_tbl_snmp_cfg,
    _cdb_format_tbl_snmp_view,
    _cdb_format_tbl_snmp_community,
    _cdb_format_tbl_snmp_context,
    _cdb_format_tbl_snmp_usm_user,
    _cdb_format_tbl_snmp_group_usm,
    _cdb_format_tbl_snmp_access_usm,
    _cdb_format_tbl_snmp_notify,
    _cdb_format_tbl_snmp_target_addr,
    _cdb_format_tbl_snmp_target_params,
    _cdb_format_tbl_snmp_trap,
    _cdb_format_tbl_snmp_inform,
    _cdb_format_tbl_snmp_trap_msg,
    _cdb_format_tbl_rmon_event,
    _cdb_format_tbl_rmon_alarm,
    _cdb_format_tbl_rmon_ether_stats,
    _cdb_format_tbl_rmon_history,
    _cdb_format_tbl_rmon_stats_history,
    _cdb_format_tbl_syslog_cfg,
    _cdb_format_tbl_ntp_server,
    _cdb_format_tbl_ntp_ace,
    _cdb_format_tbl_ntp_key,
    _cdb_format_tbl_ntp_cfg,
    _cdb_format_tbl_ntp_if,
    _cdb_format_tbl_ntp_syncstatus,
    _cdb_format_tbl_static_dns,
    _cdb_format_tbl_dynamic_dns_domain,
    _cdb_format_tbl_dynamic_dns_server,
    _cdb_format_tbl_qos_domain,
    _cdb_format_tbl_qos_mpls_domain,
    _cdb_format_tbl_qos_policer_res,
    _cdb_format_tbl_qos_policer_action_profile,
    _cdb_format_tbl_qos_policer_profile,
    _cdb_format_tbl_qos_drop_profile,
    _cdb_format_tbl_qos_queue_shape_profile,
    _cdb_format_tbl_qos_port_shape_profile,
    _cdb_format_tbl_qos_queue_smart_buffer,
    _cdb_format_tbl_qos_global,
    _cdb_format_tbl_mirror,
    _cdb_format_tbl_mirror_mac_escape,
    _cdb_format_tbl_tap_group_ingress,
    _cdb_format_tbl_tap_group_ingress_flow,
    _cdb_format_tbl_tap_group_egress,
    _cdb_format_tbl_tap_group,
    _cdb_format_tbl_tap_five_tuple,
    _cdb_format_tbl_tap_dpi_global,
    _cdb_format_tbl_user,
    _cdb_format_tbl_login_record,
    _cdb_format_tbl_login_security_glb,
    _cdb_format_tbl_vty,
    _cdb_format_tbl_console,
    _cdb_format_tbl_authen,
    _cdb_format_tbl_login,
    _cdb_format_tbl_rsa,
    _cdb_format_tbl_openflow,
    _cdb_format_tbl_cpu_traffic,
    _cdb_format_tbl_cpu_traffic_group,
    _cdb_format_tbl_cpu_utilization,
    _cdb_format_tbl_cpu_limit,
    _cdb_format_tbl_dhcrelay,
    _cdb_format_tbl_dhcsrvgrp,
    _cdb_format_tbl_dhcp_debug,
    _cdb_format_tbl_dhclient,
    _cdb_format_tbl_dhcsnooping,
    _cdb_format_tbl_dhcbinding,
    _cdb_format_tbl_iptables_prevent,
    _cdb_format_tbl_errdisable,
    _cdb_format_tbl_ns_port_forwarding,
    _cdb_format_tbl_log_global,
    _cdb_format_tbl_log,
    _cdb_format_tbl_sys_load,
    _cdb_format_tbl_cem,
    _cdb_format_tbl_clock,
    _cdb_format_tbl_tag_summer_clock,
    _cdb_format_tbl_port_stats,
    _cdb_format_tbl_vlan_stats,
    _cdb_format_tbl_port_stats_rate,
    _cdb_format_tbl_aclqos_if,
    _cdb_format_tbl_l2_action,
    _cdb_format_tbl_fea_qos_drop_profile,
    _cdb_format_tbl_fea_qos_domain,
    _cdb_format_tbl_fea_g8032_ring,
    _cdb_format_tbl_fea_g8032_ring_mc_group,
    _cdb_format_tbl_fea_qos_queue_shape_profile,
    _cdb_format_tbl_fea_qos_port_shape_profile,
    _cdb_format_tbl_fea_port_policer_apply,
    _cdb_format_tbl_aclqos_if_stats,
    _cdb_format_tbl_version,
    _cdb_format_tbl_manage_if,
    _cdb_format_tbl_bootimage,
    _cdb_format_tbl_chassis,
    _cdb_format_tbl_ifname_info,
    _cdb_format_tbl_card,
    _cdb_format_tbl_port,
    _cdb_format_tbl_fiber,
    _cdb_format_tbl_sys_spec,
    _cdb_format_tbl_fan,
    _cdb_format_tbl_psu,
    _cdb_format_tbl_led,
    _cdb_format_tbl_sensor,
    _cdb_format_tbl_vcm,
    _cdb_format_tbl_poe,
    _cdb_format_tbl_reboot_info,
    _cdb_format_tbl_errdisable_flap,
    _cdb_format_tbl_opm_global,
    _cdb_format_tbl_erps_ring,
    _cdb_format_tbl_erps_domain,
    _cdb_format_tbl_opm_debug,
    _cdb_format_tbl_policy_map_config,
    _cdb_format_tbl_class_map_config,
    _cdb_format_tbl_class_in_policy_config,
    _cdb_format_tbl_acl_in_class_config,
    _cdb_format_tbl_class_map_action_config,
    _cdb_format_tbl_fea_acl_policy_action,
    _cdb_format_tbl_igsp_global,
    _cdb_format_tbl_igsp_intf,
    _cdb_format_tbl_igsp_group,
    _cdb_format_tbl_auth_cfg,
    _cdb_format_tbl_auth_server,
    _cdb_format_tbl_auth_session,
    _cdb_format_tbl_authd_debug,
    _cdb_format_tbl_dot1x_global,
    _cdb_format_tbl_dot1x_port,
    _cdb_format_tbl_dot1x_radius,
    _cdb_format_tbl_dot1x_mac,
    _cdb_format_tbl_enable,
    _cdb_format_tbl_chip,
    _cdb_format_tbl_clear_acl_policy,
    _cdb_format_tbl_author,
    _cdb_format_tbl_account,
    _cdb_format_tbl_accountcmd,
    _cdb_format_tbl_vlanclass_rule,
    _cdb_format_tbl_vlanclass_group,
    _cdb_format_tbl_acl_l4_port_range,
    _cdb_format_tbl_acl_udf_group,
    _cdb_format_tbl_acl_udf_entry,
    _cdb_format_tbl_fea_pcap,
    _cdb_format_tbl_controller,
    _cdb_format_tbl_cpu_packets,
    _cdb_format_tbl_ns_route,
    _cdb_format_tbl_ns_route_ip,
    _cdb_format_tbl_openflow_interface,
    _cdb_format_tbl_openflow_manager,
    _cdb_format_tbl_ptp_global,
    _cdb_format_tbl_ptp_port,
    _cdb_format_tbl_ptp_foreign,
    _cdb_format_tbl_ptp_residence_time_cache,
    _cdb_format_tbl_fea_ptp_global,
    _cdb_format_tbl_fea_ptp_ts,
    _cdb_format_tbl_fea_time,
    _cdb_format_tbl_bhm_global,
    _cdb_format_tbl_bhm_module,
    _cdb_format_tbl_openflow_tunnel_interface,
    _cdb_format_tbl_openflow_tunnel_local_ip_cnt,
    _cdb_format_tbl_inband_snat,
    _cdb_format_tbl_arpinsp,
    _cdb_format_tbl_arpacl_config,
    _cdb_format_tbl_arpace_config,
    _cdb_format_tbl_copp_cfg,
    _cdb_format_tbl_fea_copp_trap_group,
    _cdb_format_tbl_sflow_global,
    _cdb_format_tbl_sflow_collector,
    _cdb_format_tbl_sflow_counter_port,
    _cdb_format_tbl_vrrp_global,
    _cdb_format_tbl_vrrp_vmac,
    _cdb_format_tbl_vrrp_vip,
    _cdb_format_tbl_vrrp_session,
    _cdb_format_tbl_ssm_global,
    _cdb_format_tbl_ssm_port,
    _cdb_format_tbl_ipsla_global,
    _cdb_format_tbl_ipsla_entry,
    _cdb_format_tbl_ipsla_packet_stats,
    _cdb_format_tbl_ipsla_test_stats,
    _cdb_format_tbl_track_global,
    _cdb_format_tbl_track_object,
    _cdb_format_tbl_controller_attr,
    _cdb_format_tbl_oam_session,
    _cdb_format_tbl_oam_session_upload,
    _cdb_format_tbl_lsp_aps_group,
    _cdb_format_tbl_pw_aps_group,
    _cdb_format_tbl_lsp_pe,
    _cdb_format_tbl_lsp_p,
    _cdb_format_tbl_pw_tpe,
    _cdb_format_tbl_section_oam,
    _cdb_format_tbl_oam_flow_info,
    _cdb_format_tbl_tpoam_global,
    _cdb_format_tbl_tpoam_exec_global,
    _cdb_format_tbl_aps_group_info,
    _cdb_format_tbl_oam_inlabel_info,
    _cdb_format_tbl_oam_mpls_aps_check,
    _cdb_format_tbl_vlan_mapping_table,
    _cdb_format_tbl_vlan_mapping_entry,
    _cdb_format_tbl_egress_mapping_entry,
    _cdb_format_tbl_vlan_group,
    _cdb_format_tbl_evc_entry,
    _cdb_format_tbl_mapped_vlan_info,
    _cdb_format_tbl_l2_protocol,
    _cdb_format_tbl_nd_fib,
    _cdb_format_tbl_nd_stats,
    _cdb_format_tbl_cpu_mirror_global,
    _cdb_format_tbl_tap_server,
    _cdb_format_tbl_dhcpv6_debug,
    _cdb_format_tbl_l2edit,
    _cdb_format_tbl_hash_field_profile,
    _cdb_format_tbl_hash_value_profile,
    _cdb_format_tbl_hash_value_global,
    _cdb_format_tbl_nat_global,
    _cdb_format_tbl_nat_pool,
    _cdb_format_tbl_nat_rule,
    _cdb_format_tbl_nat_session,
    _cdb_format_tbl_nat_session_limit,
    _cdb_format_tbl_nat_proxy_arp,
    _cdb_format_tbl_nat_rtif_inside,
    _cdb_format_tbl_nat_rtif_outside,
    _cdb_format_tbl_g8032_ring,
    _cdb_format_tbl_cfm_global,
    _cdb_format_tbl_cfm_md,
    _cdb_format_tbl_cfm_ma,
    _cdb_format_tbl_cfm_maid,
    _cdb_format_tbl_cfm_lmep,
    _cdb_format_tbl_cfm_rmep,
    _cdb_format_tbl_pim_sm,
    _cdb_format_tbl_pim_sm_bsr,
    _cdb_format_tbl_pim_sm_rp,
    _cdb_format_tbl_pim_sm_rp_gp,
    _cdb_format_tbl_pim_dm,
    _cdb_format_tbl_igmp_intf,
    _cdb_format_tbl_rip,
    _cdb_format_tbl_ripng,
    _cdb_format_tbl_rip_network,
    _cdb_format_tbl_ripng_network,
    _cdb_format_tbl_rip_neighbor,
    _cdb_format_tbl_rip_passive_if,
    _cdb_format_tbl_ripng_passive_if,
    _cdb_format_tbl_ospf_area_stub,
    _cdb_format_tbl_ospf6_area_stub,
    _cdb_format_tbl_ospf_area_nssa,
    _cdb_format_tbl_ospf_area_vlink,
};

int32
cdb_format_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    if (NULL == g_cdb_format_tbl_func[p_tbl_node->id])
    {
        return PM_E_NOT_SUPPORT;
    }

    return g_cdb_format_tbl_func[p_tbl_node->id](oper, fields, p_tbl_node, p_tbl, tbl_len);
}

