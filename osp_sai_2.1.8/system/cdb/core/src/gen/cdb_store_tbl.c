
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
_cdb_store_tbl_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_interface_t*)p_tbl;
        rc = tbl_interface_add_interface_sync(p_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        rc = tbl_interface_del_interface_sync(p_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        rc = tbl_interface_set_interface_field_sync(p_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_route_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_route_if_key_t *p_rtif_key = NULL;
    tbl_route_if_t *p_rtif = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rtif = (tbl_route_if_t*)p_tbl;
        rc = tbl_route_if_add_route_if_sync(p_rtif, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rtif_key = (tbl_route_if_key_t*)p_tbl;
        rc = tbl_route_if_del_route_if_sync(p_rtif_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rtif = (tbl_route_if_t*)p_tbl;
        rc = tbl_route_if_set_route_if_field_sync(p_rtif, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_kernel_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_kernel_if_key_t *p_kernel_if_key = NULL;
    tbl_kernel_if_t *p_kernel_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_kernel_if = (tbl_kernel_if_t*)p_tbl;
        rc = tbl_kernel_if_add_kernel_if_sync(p_kernel_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_kernel_if_key = (tbl_kernel_if_key_t*)p_tbl;
        rc = tbl_kernel_if_del_kernel_if_sync(p_kernel_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_kernel_if = (tbl_kernel_if_t*)p_tbl;
        rc = tbl_kernel_if_set_kernel_if_field_sync(p_kernel_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_port_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_port_if_key_t *p_portif_key = NULL;
    tbl_fea_port_if_t *p_portif = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_portif = (tbl_fea_port_if_t*)p_tbl;
        rc = tbl_fea_port_if_add_fea_port_if_sync(p_portif, FALSE);
        break;

    case CDB_OPER_DEL:
        p_portif_key = (tbl_fea_port_if_key_t*)p_tbl;
        rc = tbl_fea_port_if_del_fea_port_if_sync(p_portif_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_portif = (tbl_fea_port_if_t*)p_tbl;
        rc = tbl_fea_port_if_set_fea_port_if_field_sync(p_portif, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlan_key_t *p_vlan_key = NULL;
    tbl_vlan_t *p_vlan = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan = (tbl_vlan_t*)p_tbl;
        rc = tbl_vlan_add_vlan_sync(p_vlan, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlan_key = (tbl_vlan_key_t*)p_tbl;
        rc = tbl_vlan_del_vlan_sync(p_vlan_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlan = (tbl_vlan_t*)p_tbl;
        rc = tbl_vlan_set_vlan_field_sync(p_vlan, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pvlan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pvlan_key_t *p_pvlan_key = NULL;
    tbl_pvlan_t *p_pvlan = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pvlan = (tbl_pvlan_t*)p_tbl;
        rc = tbl_pvlan_add_pvlan_sync(p_pvlan, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pvlan_key = (tbl_pvlan_key_t*)p_tbl;
        rc = tbl_pvlan_del_pvlan_sync(p_pvlan_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pvlan = (tbl_pvlan_t*)p_tbl;
        rc = tbl_pvlan_set_pvlan_field_sync(p_pvlan, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fdb_key_t *p_fdb_key = NULL;
    tbl_fdb_t *p_fdb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fdb = (tbl_fdb_t*)p_tbl;
        rc = tbl_fdb_add_fdb_sync(p_fdb, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fdb_key = (tbl_fdb_key_t*)p_tbl;
        rc = tbl_fdb_del_fdb_sync(p_fdb_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fdb = (tbl_fdb_t*)p_tbl;
        rc = tbl_fdb_set_fdb_field_sync(p_fdb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mcfdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mcfdb_key_t *p_mcfdb_key = NULL;
    tbl_mcfdb_t *p_mcfdb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mcfdb = (tbl_mcfdb_t*)p_tbl;
        rc = tbl_mcfdb_add_mcfdb_sync(p_mcfdb, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mcfdb_key = (tbl_mcfdb_key_t*)p_tbl;
        rc = tbl_mcfdb_del_mcfdb_sync(p_mcfdb_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mcfdb = (tbl_mcfdb_t*)p_tbl;
        rc = tbl_mcfdb_set_mcfdb_field_sync(p_mcfdb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_macfilter(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_macfilter_key_t *p_macflt_key = NULL;
    tbl_macfilter_t *p_macflt = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_macflt = (tbl_macfilter_t*)p_tbl;
        rc = tbl_macfilter_add_macfilter_sync(p_macflt, FALSE);
        break;

    case CDB_OPER_DEL:
        p_macflt_key = (tbl_macfilter_key_t*)p_tbl;
        rc = tbl_macfilter_del_macfilter_sync(p_macflt_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_macflt = (tbl_macfilter_t*)p_tbl;
        rc = tbl_macfilter_set_macfilter_field_sync(p_macflt, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_psfdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_psfdb_key_t *p_psfdb_key = NULL;
    tbl_psfdb_t *p_psfdb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_psfdb = (tbl_psfdb_t*)p_tbl;
        rc = tbl_psfdb_add_psfdb_sync(p_psfdb, FALSE);
        break;

    case CDB_OPER_DEL:
        p_psfdb_key = (tbl_psfdb_key_t*)p_tbl;
        rc = tbl_psfdb_del_psfdb_sync(p_psfdb_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_psfdb = (tbl_psfdb_t*)p_tbl;
        rc = tbl_psfdb_set_psfdb_field_sync(p_psfdb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsg_s_ip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key = NULL;
    tbl_ipsg_s_ip_t *p_ipsg_s_ip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsg_s_ip = (tbl_ipsg_s_ip_t*)p_tbl;
        rc = tbl_ipsg_s_ip_add_ipsg_s_ip_sync(p_ipsg_s_ip, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsg_s_ip_key = (tbl_ipsg_s_ip_key_t*)p_tbl;
        rc = tbl_ipsg_s_ip_del_ipsg_s_ip_sync(p_ipsg_s_ip_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsg_s_ip = (tbl_ipsg_s_ip_t*)p_tbl;
        rc = tbl_ipsg_s_ip_set_ipsg_s_ip_field_sync(p_ipsg_s_ip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsg_s_mac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key = NULL;
    tbl_ipsg_s_mac_t *p_ipsg_s_mac = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsg_s_mac = (tbl_ipsg_s_mac_t*)p_tbl;
        rc = tbl_ipsg_s_mac_add_ipsg_s_mac_sync(p_ipsg_s_mac, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsg_s_mac_key = (tbl_ipsg_s_mac_key_t*)p_tbl;
        rc = tbl_ipsg_s_mac_del_ipsg_s_mac_sync(p_ipsg_s_mac_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsg_s_mac = (tbl_ipsg_s_mac_t*)p_tbl;
        rc = tbl_ipsg_s_mac_set_ipsg_s_mac_field_sync(p_ipsg_s_mac, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsg_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsg_fib_key_t *p_ipsg_fib_key = NULL;
    tbl_ipsg_fib_t *p_ipsg_fib = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsg_fib = (tbl_ipsg_fib_t*)p_tbl;
        rc = tbl_ipsg_fib_add_ipsg_fib_sync(p_ipsg_fib, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsg_fib_key = (tbl_ipsg_fib_key_t*)p_tbl;
        rc = tbl_ipsg_fib_del_ipsg_fib_sync(p_ipsg_fib_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsg_fib = (tbl_ipsg_fib_t*)p_tbl;
        rc = tbl_ipsg_fib_set_ipsg_fib_field_sync(p_ipsg_fib, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_brg_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_brg_global_t *p_brg_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_brg_glb = (tbl_brg_global_t*)p_tbl;
        rc = tbl_brg_global_set_brg_global_field_sync(p_brg_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mstp_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_port_key_t *p_mstp_port_key = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mstp_port = (tbl_mstp_port_t*)p_tbl;
        rc = tbl_mstp_port_add_mstp_port_sync(p_mstp_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mstp_port_key = (tbl_mstp_port_key_t*)p_tbl;
        rc = tbl_mstp_port_del_mstp_port_sync(p_mstp_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mstp_port = (tbl_mstp_port_t*)p_tbl;
        rc = tbl_mstp_port_set_mstp_port_field_sync(p_mstp_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_msti_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_msti_port_key_t *p_msti_port_key = NULL;
    tbl_msti_port_t *p_msti_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_msti_port = (tbl_msti_port_t*)p_tbl;
        rc = tbl_msti_port_add_msti_port_sync(p_msti_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_msti_port_key = (tbl_msti_port_key_t*)p_tbl;
        rc = tbl_msti_port_del_msti_port_sync(p_msti_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_msti_port = (tbl_msti_port_t*)p_tbl;
        rc = tbl_msti_port_set_msti_port_field_sync(p_msti_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mstp_instance(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_instance_key_t *p_inst_key = NULL;
    tbl_mstp_instance_t *p_inst = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_inst = (tbl_mstp_instance_t*)p_tbl;
        rc = tbl_mstp_instance_add_mstp_instance_sync(p_inst, FALSE);
        break;

    case CDB_OPER_DEL:
        p_inst_key = (tbl_mstp_instance_key_t*)p_tbl;
        rc = tbl_mstp_instance_del_mstp_instance_sync(p_inst_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_inst = (tbl_mstp_instance_t*)p_tbl;
        rc = tbl_mstp_instance_set_mstp_instance_field_sync(p_inst, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mstp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mstp_global_t *p_mstp_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_mstp_glb = (tbl_mstp_global_t*)p_tbl;
        rc = tbl_mstp_global_set_mstp_global_field_sync(p_mstp_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lldp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lldp_global_t *p_lldp_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_lldp_glb = (tbl_lldp_global_t*)p_tbl;
        rc = tbl_lldp_global_set_lldp_global_field_sync(p_lldp_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lldp_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lldp_if_key_t *p_lldp_if_key = NULL;
    tbl_lldp_if_t *p_lldp_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_lldp_if = (tbl_lldp_if_t*)p_tbl;
        rc = tbl_lldp_if_add_lldp_if_sync(p_lldp_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_lldp_if_key = (tbl_lldp_if_key_t*)p_tbl;
        rc = tbl_lldp_if_del_lldp_if_sync(p_lldp_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_lldp_if = (tbl_lldp_if_t*)p_tbl;
        rc = tbl_lldp_if_set_lldp_if_field_sync(p_lldp_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mlag(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mlag_t *p_mlag = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_mlag = (tbl_mlag_t*)p_tbl;
        rc = tbl_mlag_set_mlag_field_sync(p_mlag, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mlag_peer(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mlag_peer_t *p_peer = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_peer = (tbl_mlag_peer_t*)p_tbl;
        rc = tbl_mlag_peer_set_mlag_peer_field_sync(p_peer, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mlag_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mlag_port_key_t *p_port_key = NULL;
    tbl_mlag_port_t *p_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port = (tbl_mlag_port_t*)p_tbl;
        rc = tbl_mlag_port_add_mlag_port_sync(p_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_port_key = (tbl_mlag_port_key_t*)p_tbl;
        rc = tbl_mlag_port_del_mlag_port_sync(p_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_port = (tbl_mlag_port_t*)p_tbl;
        rc = tbl_mlag_port_set_mlag_port_field_sync(p_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_isolation(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_isolation_key_t *p_iso_key = NULL;
    tbl_isolation_t *p_iso = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_iso = (tbl_isolation_t*)p_tbl;
        rc = tbl_isolation_add_isolation_sync(p_iso, FALSE);
        break;

    case CDB_OPER_DEL:
        p_iso_key = (tbl_isolation_key_t*)p_tbl;
        rc = tbl_isolation_del_isolation_sync(p_iso_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_iso = (tbl_isolation_t*)p_tbl;
        rc = tbl_isolation_set_isolation_field_sync(p_iso, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_route_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_route_global_t *p_rt_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_rt_glb = (tbl_route_global_t*)p_tbl;
        rc = tbl_route_global_set_route_global_field_sync(p_rt_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bgp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bgp_key_t *p_bgp_key = NULL;
    tbl_bgp_t *p_bgp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_bgp = (tbl_bgp_t*)p_tbl;
        rc = tbl_bgp_add_bgp_sync(p_bgp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_bgp_key = (tbl_bgp_key_t*)p_tbl;
        rc = tbl_bgp_del_bgp_sync(p_bgp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_bgp = (tbl_bgp_t*)p_tbl;
        rc = tbl_bgp_set_bgp_field_sync(p_bgp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bgp_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bgp_network_key_t *p_bgp_network_key = NULL;
    tbl_bgp_network_t *p_bgp_network = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_bgp_network = (tbl_bgp_network_t*)p_tbl;
        rc = tbl_bgp_network_add_bgp_network_sync(p_bgp_network, FALSE);
        break;

    case CDB_OPER_DEL:
        p_bgp_network_key = (tbl_bgp_network_key_t*)p_tbl;
        rc = tbl_bgp_network_del_bgp_network_sync(p_bgp_network_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_bgp_network = (tbl_bgp_network_t*)p_tbl;
        rc = tbl_bgp_network_set_bgp_network_field_sync(p_bgp_network, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bgp_neighbor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bgp_neighbor_key_t *p_bgp_neighbor_key = NULL;
    tbl_bgp_neighbor_t *p_bgp_neighbor = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_bgp_neighbor = (tbl_bgp_neighbor_t*)p_tbl;
        rc = tbl_bgp_neighbor_add_bgp_neighbor_sync(p_bgp_neighbor, FALSE);
        break;

    case CDB_OPER_DEL:
        p_bgp_neighbor_key = (tbl_bgp_neighbor_key_t*)p_tbl;
        rc = tbl_bgp_neighbor_del_bgp_neighbor_sync(p_bgp_neighbor_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_bgp_neighbor = (tbl_bgp_neighbor_t*)p_tbl;
        rc = tbl_bgp_neighbor_set_bgp_neighbor_field_sync(p_bgp_neighbor, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_key_t *p_ospf_key = NULL;
    tbl_ospf_t *p_ospf = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf = (tbl_ospf_t*)p_tbl;
        rc = tbl_ospf_add_ospf_sync(p_ospf, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_key = (tbl_ospf_key_t*)p_tbl;
        rc = tbl_ospf_del_ospf_sync(p_ospf_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf = (tbl_ospf_t*)p_tbl;
        rc = tbl_ospf_set_ospf_field_sync(p_ospf, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf6(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf6_key_t *p_ospf6_key = NULL;
    tbl_ospf6_t *p_ospf6 = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf6 = (tbl_ospf6_t*)p_tbl;
        rc = tbl_ospf6_add_ospf6_sync(p_ospf6, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf6_key = (tbl_ospf6_key_t*)p_tbl;
        rc = tbl_ospf6_del_ospf6_sync(p_ospf6_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf6 = (tbl_ospf6_t*)p_tbl;
        rc = tbl_ospf6_set_ospf6_field_sync(p_ospf6, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf6_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf6_interface_key_t *p_ospf6_interface_key = NULL;
    tbl_ospf6_interface_t *p_ospf6_interface = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf6_interface = (tbl_ospf6_interface_t*)p_tbl;
        rc = tbl_ospf6_interface_add_ospf6_interface_sync(p_ospf6_interface, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf6_interface_key = (tbl_ospf6_interface_key_t*)p_tbl;
        rc = tbl_ospf6_interface_del_ospf6_interface_sync(p_ospf6_interface_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf6_interface = (tbl_ospf6_interface_t*)p_tbl;
        rc = tbl_ospf6_interface_set_ospf6_interface_field_sync(p_ospf6_interface, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_network_key_t *p_ospf_network_key = NULL;
    tbl_ospf_network_t *p_ospf_network = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_network = (tbl_ospf_network_t*)p_tbl;
        rc = tbl_ospf_network_add_ospf_network_sync(p_ospf_network, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_network_key = (tbl_ospf_network_key_t*)p_tbl;
        rc = tbl_ospf_network_del_ospf_network_sync(p_ospf_network_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_network = (tbl_ospf_network_t*)p_tbl;
        rc = tbl_ospf_network_set_ospf_network_field_sync(p_ospf_network, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_area_auth(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_area_auth_key_t *p_ospf_area_auth_key = NULL;
    tbl_ospf_area_auth_t *p_ospf_area_auth = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_area_auth = (tbl_ospf_area_auth_t*)p_tbl;
        rc = tbl_ospf_area_auth_add_ospf_area_auth_sync(p_ospf_area_auth, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_area_auth_key = (tbl_ospf_area_auth_key_t*)p_tbl;
        rc = tbl_ospf_area_auth_del_ospf_area_auth_sync(p_ospf_area_auth_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_area_auth = (tbl_ospf_area_auth_t*)p_tbl;
        rc = tbl_ospf_area_auth_set_ospf_area_auth_field_sync(p_ospf_area_auth, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_area_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_area_range_key_t *p_ospf_area_range_key = NULL;
    tbl_ospf_area_range_t *p_ospf_area_range = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_area_range = (tbl_ospf_area_range_t*)p_tbl;
        rc = tbl_ospf_area_range_add_ospf_area_range_sync(p_ospf_area_range, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_area_range_key = (tbl_ospf_area_range_key_t*)p_tbl;
        rc = tbl_ospf_area_range_del_ospf_area_range_sync(p_ospf_area_range_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_area_range = (tbl_ospf_area_range_t*)p_tbl;
        rc = tbl_ospf_area_range_set_ospf_area_range_field_sync(p_ospf_area_range, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf6_area_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf6_area_range_key_t *p_ospf6_area_range_key = NULL;
    tbl_ospf6_area_range_t *p_ospf6_area_range = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf6_area_range = (tbl_ospf6_area_range_t*)p_tbl;
        rc = tbl_ospf6_area_range_add_ospf6_area_range_sync(p_ospf6_area_range, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf6_area_range_key = (tbl_ospf6_area_range_key_t*)p_tbl;
        rc = tbl_ospf6_area_range_del_ospf6_area_range_sync(p_ospf6_area_range_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf6_area_range = (tbl_ospf6_area_range_t*)p_tbl;
        rc = tbl_ospf6_area_range_set_ospf6_area_range_field_sync(p_ospf6_area_range, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_iproute_node(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_iproute_node_key_t *p_rn_key = NULL;
    tbl_iproute_node_t *p_rn = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rn = (tbl_iproute_node_t*)p_tbl;
        rc = tbl_iproute_node_add_iproute_node_sync(p_rn, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rn_key = (tbl_iproute_node_key_t*)p_tbl;
        rc = tbl_iproute_node_del_iproute_node_sync(p_rn_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rn = (tbl_iproute_node_t*)p_tbl;
        rc = tbl_iproute_node_set_iproute_node_field_sync(p_rn, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_iproute_node_add_fail(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_iproute_node_add_fail_key_t *p_rn_key = NULL;
    tbl_iproute_node_add_fail_t *p_rn = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rn = (tbl_iproute_node_add_fail_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_add_iproute_node_add_fail_sync(p_rn, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rn_key = (tbl_iproute_node_add_fail_key_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_del_iproute_node_add_fail_sync(p_rn_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rn = (tbl_iproute_node_add_fail_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_set_iproute_node_add_fail_field_sync(p_rn, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_iproute_node_add_fail_count(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_iproute_node_add_fail_count_key_t *p_rn_key = NULL;
    tbl_iproute_node_add_fail_count_t *p_rn = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rn = (tbl_iproute_node_add_fail_count_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_count_add_iproute_node_add_fail_count_sync(p_rn, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rn_key = (tbl_iproute_node_add_fail_count_key_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_count_del_iproute_node_add_fail_count_sync(p_rn_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rn = (tbl_iproute_node_add_fail_count_t*)p_tbl;
        rc = tbl_iproute_node_add_fail_count_set_iproute_node_add_fail_count_field_sync(p_rn, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_static_route_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_static_route_cfg_key_t *p_rst_cfg_key = NULL;
    tbl_static_route_cfg_t *p_rst_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rst_cfg = (tbl_static_route_cfg_t*)p_tbl;
        rc = tbl_static_route_cfg_add_static_route_cfg_sync(p_rst_cfg, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rst_cfg_key = (tbl_static_route_cfg_key_t*)p_tbl;
        rc = tbl_static_route_cfg_del_static_route_cfg_sync(p_rst_cfg_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rst_cfg = (tbl_static_route_cfg_t*)p_tbl;
        rc = tbl_static_route_cfg_set_static_route_cfg_field_sync(p_rst_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_static_rt_cnt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_static_rt_cnt_key_t *p_rt_cnt_key = NULL;
    tbl_static_rt_cnt_t *p_rt_cnt = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rt_cnt = (tbl_static_rt_cnt_t*)p_tbl;
        rc = tbl_static_rt_cnt_add_static_rt_cnt_sync(p_rt_cnt, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rt_cnt_key = (tbl_static_rt_cnt_key_t*)p_tbl;
        rc = tbl_static_rt_cnt_del_static_rt_cnt_sync(p_rt_cnt_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rt_cnt = (tbl_static_rt_cnt_t*)p_tbl;
        rc = tbl_static_rt_cnt_set_static_rt_cnt_field_sync(p_rt_cnt, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_arp_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arp_fib_key_t *p_arp_fib_key = NULL;
    tbl_arp_fib_t *p_arp_fib = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arp_fib = (tbl_arp_fib_t*)p_tbl;
        rc = tbl_arp_fib_add_arp_fib_sync(p_arp_fib, FALSE);
        break;

    case CDB_OPER_DEL:
        p_arp_fib_key = (tbl_arp_fib_key_t*)p_tbl;
        rc = tbl_arp_fib_del_arp_fib_sync(p_arp_fib_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_arp_fib = (tbl_arp_fib_t*)p_tbl;
        rc = tbl_arp_fib_set_arp_fib_field_sync(p_arp_fib, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_arp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arp_key_t *p_arp_key = NULL;
    tbl_arp_t *p_arp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arp = (tbl_arp_t*)p_tbl;
        rc = tbl_arp_add_arp_sync(p_arp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_arp_key = (tbl_arp_key_t*)p_tbl;
        rc = tbl_arp_del_arp_sync(p_arp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_arp = (tbl_arp_t*)p_tbl;
        rc = tbl_arp_set_arp_field_sync(p_arp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nexthop_key_t *p_nh_key = NULL;
    tbl_nexthop_t *p_nh = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nh = (tbl_nexthop_t*)p_tbl;
        rc = tbl_nexthop_add_nexthop_sync(p_nh, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nh_key = (tbl_nexthop_key_t*)p_tbl;
        rc = tbl_nexthop_del_nexthop_sync(p_nh_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nh = (tbl_nexthop_t*)p_tbl;
        rc = tbl_nexthop_set_nexthop_field_sync(p_nh, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nexthop_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nexthop_group_key_t *p_nhg_key = NULL;
    tbl_nexthop_group_t *p_nhg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nhg = (tbl_nexthop_group_t*)p_tbl;
        rc = tbl_nexthop_group_add_nexthop_group_sync(p_nhg, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nhg_key = (tbl_nexthop_group_key_t*)p_tbl;
        rc = tbl_nexthop_group_del_nexthop_group_sync(p_nhg_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nhg = (tbl_nexthop_group_t*)p_tbl;
        rc = tbl_nexthop_group_set_nexthop_group_field_sync(p_nhg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nexthop_group_routed(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nexthop_group_routed_key_t *p_nhg_routed_key = NULL;
    tbl_nexthop_group_routed_t *p_nhg_routed = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nhg_routed = (tbl_nexthop_group_routed_t*)p_tbl;
        rc = tbl_nexthop_group_routed_add_nexthop_group_routed_sync(p_nhg_routed, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nhg_routed_key = (tbl_nexthop_group_routed_key_t*)p_tbl;
        rc = tbl_nexthop_group_routed_del_nexthop_group_routed_sync(p_nhg_routed_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nhg_routed = (tbl_nexthop_group_routed_t*)p_tbl;
        rc = tbl_nexthop_group_routed_set_nexthop_group_routed_field_sync(p_nhg_routed, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_nexthop_key_t *p_nh_key = NULL;
    tbl_fea_nexthop_t *p_nh = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nh = (tbl_fea_nexthop_t*)p_tbl;
        rc = tbl_fea_nexthop_add_fea_nexthop_sync(p_nh, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nh_key = (tbl_fea_nexthop_key_t*)p_tbl;
        rc = tbl_fea_nexthop_del_fea_nexthop_sync(p_nh_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nh = (tbl_fea_nexthop_t*)p_tbl;
        rc = tbl_fea_nexthop_set_fea_nexthop_field_sync(p_nh, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mrt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mrt_key_t *p_mrt_key = NULL;
    tbl_mrt_t *p_mrt = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mrt = (tbl_mrt_t*)p_tbl;
        rc = tbl_mrt_add_mrt_sync(p_mrt, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mrt_key = (tbl_mrt_key_t*)p_tbl;
        rc = tbl_mrt_del_mrt_sync(p_mrt_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mrt = (tbl_mrt_t*)p_tbl;
        rc = tbl_mrt_set_mrt_field_sync(p_mrt, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipmc_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipmc_global_t *p_pim_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_pim_glb = (tbl_ipmc_global_t*)p_tbl;
        rc = tbl_ipmc_global_set_ipmc_global_field_sync(p_pim_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipmc_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipmc_intf_key_t *p_pim_intf_key = NULL;
    tbl_ipmc_intf_t *p_pim_intf = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_intf = (tbl_ipmc_intf_t*)p_tbl;
        rc = tbl_ipmc_intf_add_ipmc_intf_sync(p_pim_intf, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_intf_key = (tbl_ipmc_intf_key_t*)p_tbl;
        rc = tbl_ipmc_intf_del_ipmc_intf_sync(p_pim_intf_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_intf = (tbl_ipmc_intf_t*)p_tbl;
        rc = tbl_ipmc_intf_set_ipmc_intf_field_sync(p_pim_intf, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_static_rp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_static_rp_key_t *p_pim_static_rp_key = NULL;
    tbl_pim_static_rp_t *p_pim_static_rp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_static_rp = (tbl_pim_static_rp_t*)p_tbl;
        rc = tbl_pim_static_rp_add_pim_static_rp_sync(p_pim_static_rp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_static_rp_key = (tbl_pim_static_rp_key_t*)p_tbl;
        rc = tbl_pim_static_rp_del_pim_static_rp_sync(p_pim_static_rp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_static_rp = (tbl_pim_static_rp_t*)p_tbl;
        rc = tbl_pim_static_rp_set_pim_static_rp_field_sync(p_pim_static_rp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sys_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sys_global_t *p_sys_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_sys_glb = (tbl_sys_global_t*)p_tbl;
        rc = tbl_sys_global_set_sys_global_field_sync(p_sys_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lag_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lag_global_t *p_lag_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_lag_glb = (tbl_lag_global_t*)p_tbl;
        rc = tbl_lag_global_set_lag_global_field_sync(p_lag_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ecmp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ecmp_global_t *p_ecmp_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ecmp_glb = (tbl_ecmp_global_t*)p_tbl;
        rc = tbl_ecmp_global_set_ecmp_global_field_sync(p_ecmp_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rr_prefix(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rr_prefix_key_t *p_rr_prefix_key = NULL;
    tbl_rr_prefix_t *p_rr_prefix = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rr_prefix = (tbl_rr_prefix_t*)p_tbl;
        rc = tbl_rr_prefix_add_rr_prefix_sync(p_rr_prefix, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rr_prefix_key = (tbl_rr_prefix_key_t*)p_tbl;
        rc = tbl_rr_prefix_del_rr_prefix_sync(p_rr_prefix_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rr_prefix = (tbl_rr_prefix_t*)p_tbl;
        rc = tbl_rr_prefix_set_rr_prefix_field_sync(p_rr_prefix, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mem_summary(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mem_summary_t *p_mem_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_mem_info = (tbl_mem_summary_t*)p_tbl;
        rc = tbl_mem_summary_set_mem_summary_field_sync(p_mem_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_mem_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_mem_history_t *p_cpu_mem_history = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cpu_mem_history = (tbl_cpu_mem_history_t*)p_tbl;
        rc = tbl_cpu_mem_history_set_cpu_mem_history_field_sync(p_cpu_mem_history, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_chsm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_chsm_debug_t *p_chsmdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_chsmdbg = (tbl_chsm_debug_t*)p_tbl;
        rc = tbl_chsm_debug_set_chsm_debug_field_sync(p_chsmdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_switch_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_switch_debug_t *p_swthdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_swthdbg = (tbl_switch_debug_t*)p_tbl;
        rc = tbl_switch_debug_set_switch_debug_field_sync(p_swthdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_route_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_route_debug_t *p_rtdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_rtdbg = (tbl_route_debug_t*)p_tbl;
        rc = tbl_route_debug_set_route_debug_field_sync(p_rtdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ssm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssm_debug_t *p_ssmdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ssmdbg = (tbl_ssm_debug_t*)p_tbl;
        rc = tbl_ssm_debug_set_ssm_debug_field_sync(p_ssmdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ptp_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_debug_t *p_ptpdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ptpdbg = (tbl_ptp_debug_t*)p_tbl;
        rc = tbl_ptp_debug_set_ptp_debug_field_sync(p_ptpdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_quagga_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_quagga_debug_t *p_dbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dbg = (tbl_quagga_debug_t*)p_tbl;
        rc = tbl_quagga_debug_set_quagga_debug_field_sync(p_dbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lsrv_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lsrv_debug_t *p_lsrvdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_lsrvdbg = (tbl_lsrv_debug_t*)p_tbl;
        rc = tbl_lsrv_debug_set_lsrv_debug_field_sync(p_lsrvdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_hsrv_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_hsrv_debug_t *p_hsrvdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_hsrvdbg = (tbl_hsrv_debug_t*)p_tbl;
        rc = tbl_hsrv_debug_set_hsrv_debug_field_sync(p_hsrvdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_app_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_app_debug_t *p_appdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_appdbg = (tbl_app_debug_t*)p_tbl;
        rc = tbl_app_debug_set_app_debug_field_sync(p_appdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_debug_t *p_openflowdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_openflowdbg = (tbl_openflow_debug_t*)p_tbl;
        rc = tbl_openflow_debug_set_openflow_debug_field_sync(p_openflowdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rif(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rif_key_t *p_rif_key = NULL;
    tbl_rif_t *p_rif = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rif = (tbl_rif_t*)p_tbl;
        rc = tbl_rif_add_rif_sync(p_rif, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rif_key = (tbl_rif_key_t*)p_tbl;
        rc = tbl_rif_del_rif_sync(p_rif_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rif = (tbl_rif_t*)p_tbl;
        rc = tbl_rif_set_rif_field_sync(p_rif, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_lag(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_lag_key_t *p_fea_lag_key = NULL;
    tbl_fea_lag_t *p_fea_lag = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_lag = (tbl_fea_lag_t*)p_tbl;
        rc = tbl_fea_lag_add_fea_lag_sync(p_fea_lag, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_lag_key = (tbl_fea_lag_key_t*)p_tbl;
        rc = tbl_fea_lag_del_fea_lag_sync(p_fea_lag_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_lag = (tbl_fea_lag_t*)p_tbl;
        rc = tbl_fea_lag_set_fea_lag_field_sync(p_fea_lag, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_global_t *p_fea_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_fea_global = (tbl_fea_global_t*)p_tbl;
        rc = tbl_fea_global_set_fea_global_field_sync(p_fea_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_acl_table(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_acl_table_key_t *p_fea_acl_table_key = NULL;
    tbl_fea_acl_table_t *p_fea_acl_table = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_acl_table = (tbl_fea_acl_table_t*)p_tbl;
        rc = tbl_fea_acl_table_add_fea_acl_table_sync(p_fea_acl_table, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_acl_table_key = (tbl_fea_acl_table_key_t*)p_tbl;
        rc = tbl_fea_acl_table_del_fea_acl_table_sync(p_fea_acl_table_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_acl_table = (tbl_fea_acl_table_t*)p_tbl;
        rc = tbl_fea_acl_table_set_fea_acl_table_field_sync(p_fea_acl_table, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_acl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_acl_key_t *p_fea_acl_key = NULL;
    tbl_fea_acl_t *p_fea_acl = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_acl = (tbl_fea_acl_t*)p_tbl;
        rc = tbl_fea_acl_add_fea_acl_sync(p_fea_acl, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_acl_key = (tbl_fea_acl_key_t*)p_tbl;
        rc = tbl_fea_acl_del_fea_acl_sync(p_fea_acl_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_acl = (tbl_fea_acl_t*)p_tbl;
        rc = tbl_fea_acl_set_fea_acl_field_sync(p_fea_acl, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_fdb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_fdb_key_t *p_fdb_key = NULL;
    tbl_fea_fdb_t *p_fdb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fdb = (tbl_fea_fdb_t*)p_tbl;
        rc = tbl_fea_fdb_add_fea_fdb_sync(p_fdb, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fdb_key = (tbl_fea_fdb_key_t*)p_tbl;
        rc = tbl_fea_fdb_del_fea_fdb_sync(p_fdb_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fdb = (tbl_fea_fdb_t*)p_tbl;
        rc = tbl_fea_fdb_set_fea_fdb_field_sync(p_fdb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_brg_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_brg_if_key_t *p_brgif_key = NULL;
    tbl_fea_brg_if_t *p_brgif = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_brgif = (tbl_fea_brg_if_t*)p_tbl;
        rc = tbl_fea_brg_if_add_fea_brg_if_sync(p_brgif, FALSE);
        break;

    case CDB_OPER_DEL:
        p_brgif_key = (tbl_fea_brg_if_key_t*)p_tbl;
        rc = tbl_fea_brg_if_del_fea_brg_if_sync(p_brgif_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_brgif = (tbl_fea_brg_if_t*)p_tbl;
        rc = tbl_fea_brg_if_set_fea_brg_if_field_sync(p_brgif, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_worm_filter(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_worm_filter_key_t *p_acl_worm_filter_key = NULL;
    tbl_acl_worm_filter_t *p_acl_worm_filter = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_worm_filter = (tbl_acl_worm_filter_t*)p_tbl;
        rc = tbl_acl_worm_filter_add_acl_worm_filter_sync(p_acl_worm_filter, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_worm_filter_key = (tbl_acl_worm_filter_key_t*)p_tbl;
        rc = tbl_acl_worm_filter_del_acl_worm_filter_sync(p_acl_worm_filter_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_worm_filter = (tbl_acl_worm_filter_t*)p_tbl;
        rc = tbl_acl_worm_filter_set_acl_worm_filter_field_sync(p_acl_worm_filter, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_config_key_t *p_acl_config_key = NULL;
    tbl_acl_config_t *p_acl_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_config = (tbl_acl_config_t*)p_tbl;
        rc = tbl_acl_config_add_acl_config_sync(p_acl_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_config_key = (tbl_acl_config_key_t*)p_tbl;
        rc = tbl_acl_config_del_acl_config_sync(p_acl_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_config = (tbl_acl_config_t*)p_tbl;
        rc = tbl_acl_config_set_acl_config_field_sync(p_acl_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ace_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ace_config_key_t *p_ace_config_key = NULL;
    tbl_ace_config_t *p_ace_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ace_config = (tbl_ace_config_t*)p_tbl;
        rc = tbl_ace_config_add_ace_config_sync(p_ace_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ace_config_key = (tbl_ace_config_key_t*)p_tbl;
        rc = tbl_ace_config_del_ace_config_sync(p_ace_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ace_config = (tbl_ace_config_t*)p_tbl;
        rc = tbl_ace_config_set_ace_config_field_sync(p_ace_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_entry_key_t *p_acl_entry_key = NULL;
    tbl_acl_entry_t *p_acl_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_entry = (tbl_acl_entry_t*)p_tbl;
        rc = tbl_acl_entry_add_acl_entry_sync(p_acl_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_entry_key = (tbl_acl_entry_key_t*)p_tbl;
        rc = tbl_acl_entry_del_acl_entry_sync(p_acl_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_entry = (tbl_acl_entry_t*)p_tbl;
        rc = tbl_acl_entry_set_acl_entry_field_sync(p_acl_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_entry_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_entry_action_key_t *p_acl_entry_action_key = NULL;
    tbl_acl_entry_action_t *p_acl_entry_action = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_entry_action = (tbl_acl_entry_action_t*)p_tbl;
        rc = tbl_acl_entry_action_add_acl_entry_action_sync(p_acl_entry_action, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_entry_action_key = (tbl_acl_entry_action_key_t*)p_tbl;
        rc = tbl_acl_entry_action_del_acl_entry_action_sync(p_acl_entry_action_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_entry_action = (tbl_acl_entry_action_t*)p_tbl;
        rc = tbl_acl_entry_action_set_acl_entry_action_field_sync(p_acl_entry_action, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_nexthop_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key = NULL;
    tbl_acl_nexthop_group_t *p_acl_nexthop_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_nexthop_group = (tbl_acl_nexthop_group_t*)p_tbl;
        rc = tbl_acl_nexthop_group_add_acl_nexthop_group_sync(p_acl_nexthop_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_nexthop_group_key = (tbl_acl_nexthop_group_key_t*)p_tbl;
        rc = tbl_acl_nexthop_group_del_acl_nexthop_group_sync(p_acl_nexthop_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_nexthop_group = (tbl_acl_nexthop_group_t*)p_tbl;
        rc = tbl_acl_nexthop_group_set_acl_nexthop_group_field_sync(p_acl_nexthop_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_nexthop(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_nexthop_key_t *p_acl_nexthop_key = NULL;
    tbl_acl_nexthop_t *p_acl_nexthop = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_nexthop = (tbl_acl_nexthop_t*)p_tbl;
        rc = tbl_acl_nexthop_add_acl_nexthop_sync(p_acl_nexthop, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_nexthop_key = (tbl_acl_nexthop_key_t*)p_tbl;
        rc = tbl_acl_nexthop_del_acl_nexthop_sync(p_acl_nexthop_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_nexthop = (tbl_acl_nexthop_t*)p_tbl;
        rc = tbl_acl_nexthop_set_acl_nexthop_field_sync(p_acl_nexthop, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_time_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_time_range_key_t *p_time_range_key = NULL;
    tbl_time_range_t *p_time_range = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_time_range = (tbl_time_range_t*)p_tbl;
        rc = tbl_time_range_add_time_range_sync(p_time_range, FALSE);
        break;

    case CDB_OPER_DEL:
        p_time_range_key = (tbl_time_range_key_t*)p_tbl;
        rc = tbl_time_range_del_time_range_sync(p_time_range_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_time_range = (tbl_time_range_t*)p_tbl;
        rc = tbl_time_range_set_time_range_field_sync(p_time_range, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ssh_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssh_cfg_t *p_ssh_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ssh_cfg = (tbl_ssh_cfg_t*)p_tbl;
        rc = tbl_ssh_cfg_set_ssh_cfg_field_sync(p_ssh_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_cfg_t *p_snmp_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_snmp_cfg = (tbl_snmp_cfg_t*)p_tbl;
        rc = tbl_snmp_cfg_set_snmp_cfg_field_sync(p_snmp_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_view(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_view_key_t *p_snmp_view_key = NULL;
    tbl_snmp_view_t *p_snmp_view = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_view = (tbl_snmp_view_t*)p_tbl;
        rc = tbl_snmp_view_add_snmp_view_sync(p_snmp_view, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_view_key = (tbl_snmp_view_key_t*)p_tbl;
        rc = tbl_snmp_view_del_snmp_view_sync(p_snmp_view_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_view = (tbl_snmp_view_t*)p_tbl;
        rc = tbl_snmp_view_set_snmp_view_field_sync(p_snmp_view, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_community(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_community_key_t *p_snmp_community_key = NULL;
    tbl_snmp_community_t *p_snmp_community = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_community = (tbl_snmp_community_t*)p_tbl;
        rc = tbl_snmp_community_add_snmp_community_sync(p_snmp_community, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_community_key = (tbl_snmp_community_key_t*)p_tbl;
        rc = tbl_snmp_community_del_snmp_community_sync(p_snmp_community_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_community = (tbl_snmp_community_t*)p_tbl;
        rc = tbl_snmp_community_set_snmp_community_field_sync(p_snmp_community, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_context(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_context_key_t *p_snmp_context_key = NULL;
    tbl_snmp_context_t *p_snmp_context = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_context = (tbl_snmp_context_t*)p_tbl;
        rc = tbl_snmp_context_add_snmp_context_sync(p_snmp_context, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_context_key = (tbl_snmp_context_key_t*)p_tbl;
        rc = tbl_snmp_context_del_snmp_context_sync(p_snmp_context_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_context = (tbl_snmp_context_t*)p_tbl;
        rc = tbl_snmp_context_set_snmp_context_field_sync(p_snmp_context, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_usm_user(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_usm_user_key_t *p_snmp_usm_user_key = NULL;
    tbl_snmp_usm_user_t *p_snmp_usm_user = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_usm_user = (tbl_snmp_usm_user_t*)p_tbl;
        rc = tbl_snmp_usm_user_add_snmp_usm_user_sync(p_snmp_usm_user, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_usm_user_key = (tbl_snmp_usm_user_key_t*)p_tbl;
        rc = tbl_snmp_usm_user_del_snmp_usm_user_sync(p_snmp_usm_user_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_usm_user = (tbl_snmp_usm_user_t*)p_tbl;
        rc = tbl_snmp_usm_user_set_snmp_usm_user_field_sync(p_snmp_usm_user, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_group_usm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_group_usm_key_t *p_snmp_group_usm_key = NULL;
    tbl_snmp_group_usm_t *p_snmp_group_usm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_group_usm = (tbl_snmp_group_usm_t*)p_tbl;
        rc = tbl_snmp_group_usm_add_snmp_group_usm_sync(p_snmp_group_usm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_group_usm_key = (tbl_snmp_group_usm_key_t*)p_tbl;
        rc = tbl_snmp_group_usm_del_snmp_group_usm_sync(p_snmp_group_usm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_group_usm = (tbl_snmp_group_usm_t*)p_tbl;
        rc = tbl_snmp_group_usm_set_snmp_group_usm_field_sync(p_snmp_group_usm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_access_usm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_access_usm_key_t *p_snmp_access_usm_key = NULL;
    tbl_snmp_access_usm_t *p_snmp_access_usm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_access_usm = (tbl_snmp_access_usm_t*)p_tbl;
        rc = tbl_snmp_access_usm_add_snmp_access_usm_sync(p_snmp_access_usm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_access_usm_key = (tbl_snmp_access_usm_key_t*)p_tbl;
        rc = tbl_snmp_access_usm_del_snmp_access_usm_sync(p_snmp_access_usm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_access_usm = (tbl_snmp_access_usm_t*)p_tbl;
        rc = tbl_snmp_access_usm_set_snmp_access_usm_field_sync(p_snmp_access_usm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_notify(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_notify_key_t *p_snmp_notify_key = NULL;
    tbl_snmp_notify_t *p_snmp_notify = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_notify = (tbl_snmp_notify_t*)p_tbl;
        rc = tbl_snmp_notify_add_snmp_notify_sync(p_snmp_notify, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_notify_key = (tbl_snmp_notify_key_t*)p_tbl;
        rc = tbl_snmp_notify_del_snmp_notify_sync(p_snmp_notify_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_notify = (tbl_snmp_notify_t*)p_tbl;
        rc = tbl_snmp_notify_set_snmp_notify_field_sync(p_snmp_notify, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_target_addr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_target_addr_key_t *p_snmp_target_addr_key = NULL;
    tbl_snmp_target_addr_t *p_snmp_target_addr = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_target_addr = (tbl_snmp_target_addr_t*)p_tbl;
        rc = tbl_snmp_target_addr_add_snmp_target_addr_sync(p_snmp_target_addr, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_target_addr_key = (tbl_snmp_target_addr_key_t*)p_tbl;
        rc = tbl_snmp_target_addr_del_snmp_target_addr_sync(p_snmp_target_addr_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_target_addr = (tbl_snmp_target_addr_t*)p_tbl;
        rc = tbl_snmp_target_addr_set_snmp_target_addr_field_sync(p_snmp_target_addr, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_target_params(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_target_params_key_t *p_snmp_target_params_key = NULL;
    tbl_snmp_target_params_t *p_snmp_target_params = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_target_params = (tbl_snmp_target_params_t*)p_tbl;
        rc = tbl_snmp_target_params_add_snmp_target_params_sync(p_snmp_target_params, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_target_params_key = (tbl_snmp_target_params_key_t*)p_tbl;
        rc = tbl_snmp_target_params_del_snmp_target_params_sync(p_snmp_target_params_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_target_params = (tbl_snmp_target_params_t*)p_tbl;
        rc = tbl_snmp_target_params_set_snmp_target_params_field_sync(p_snmp_target_params, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_trap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_trap_key_t *p_snmp_trap_key = NULL;
    tbl_snmp_trap_t *p_snmp_trap = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_trap = (tbl_snmp_trap_t*)p_tbl;
        rc = tbl_snmp_trap_add_snmp_trap_sync(p_snmp_trap, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_trap_key = (tbl_snmp_trap_key_t*)p_tbl;
        rc = tbl_snmp_trap_del_snmp_trap_sync(p_snmp_trap_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_trap = (tbl_snmp_trap_t*)p_tbl;
        rc = tbl_snmp_trap_set_snmp_trap_field_sync(p_snmp_trap, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_inform(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_inform_key_t *p_snmp_inform_key = NULL;
    tbl_snmp_inform_t *p_snmp_inform = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_snmp_inform = (tbl_snmp_inform_t*)p_tbl;
        rc = tbl_snmp_inform_add_snmp_inform_sync(p_snmp_inform, FALSE);
        break;

    case CDB_OPER_DEL:
        p_snmp_inform_key = (tbl_snmp_inform_key_t*)p_tbl;
        rc = tbl_snmp_inform_del_snmp_inform_sync(p_snmp_inform_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_snmp_inform = (tbl_snmp_inform_t*)p_tbl;
        rc = tbl_snmp_inform_set_snmp_inform_field_sync(p_snmp_inform, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_snmp_trap_msg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_snmp_trap_msg_t *p_trap = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_trap = (tbl_snmp_trap_msg_t*)p_tbl;
        rc = tbl_snmp_trap_msg_set_snmp_trap_msg_field_sync(p_trap, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rmon_event(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_event_key_t *p_rmon_event_key = NULL;
    tbl_rmon_event_t *p_rmon_event = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmon_event = (tbl_rmon_event_t*)p_tbl;
        rc = tbl_rmon_event_add_rmon_event_sync(p_rmon_event, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmon_event_key = (tbl_rmon_event_key_t*)p_tbl;
        rc = tbl_rmon_event_del_rmon_event_sync(p_rmon_event_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmon_event = (tbl_rmon_event_t*)p_tbl;
        rc = tbl_rmon_event_set_rmon_event_field_sync(p_rmon_event, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rmon_alarm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_alarm_key_t *p_rmon_alarm_key = NULL;
    tbl_rmon_alarm_t *p_rmon_alarm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmon_alarm = (tbl_rmon_alarm_t*)p_tbl;
        rc = tbl_rmon_alarm_add_rmon_alarm_sync(p_rmon_alarm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmon_alarm_key = (tbl_rmon_alarm_key_t*)p_tbl;
        rc = tbl_rmon_alarm_del_rmon_alarm_sync(p_rmon_alarm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmon_alarm = (tbl_rmon_alarm_t*)p_tbl;
        rc = tbl_rmon_alarm_set_rmon_alarm_field_sync(p_rmon_alarm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rmon_ether_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key = NULL;
    tbl_rmon_ether_stats_t *p_rmon_ether_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmon_ether_stats = (tbl_rmon_ether_stats_t*)p_tbl;
        rc = tbl_rmon_ether_stats_add_rmon_ether_stats_sync(p_rmon_ether_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmon_ether_stats_key = (tbl_rmon_ether_stats_key_t*)p_tbl;
        rc = tbl_rmon_ether_stats_del_rmon_ether_stats_sync(p_rmon_ether_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmon_ether_stats = (tbl_rmon_ether_stats_t*)p_tbl;
        rc = tbl_rmon_ether_stats_set_rmon_ether_stats_field_sync(p_rmon_ether_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rmon_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_history_key_t *p_rmon_history_key = NULL;
    tbl_rmon_history_t *p_rmon_history = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmon_history = (tbl_rmon_history_t*)p_tbl;
        rc = tbl_rmon_history_add_rmon_history_sync(p_rmon_history, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmon_history_key = (tbl_rmon_history_key_t*)p_tbl;
        rc = tbl_rmon_history_del_rmon_history_sync(p_rmon_history_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmon_history = (tbl_rmon_history_t*)p_tbl;
        rc = tbl_rmon_history_set_rmon_history_field_sync(p_rmon_history, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rmon_stats_history(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rmon_stats_history_key_t *p_rmon_stats_history_key = NULL;
    tbl_rmon_stats_history_t *p_rmon_stats_history = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmon_stats_history = (tbl_rmon_stats_history_t*)p_tbl;
        rc = tbl_rmon_stats_history_add_rmon_stats_history_sync(p_rmon_stats_history, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmon_stats_history_key = (tbl_rmon_stats_history_key_t*)p_tbl;
        rc = tbl_rmon_stats_history_del_rmon_stats_history_sync(p_rmon_stats_history_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmon_stats_history = (tbl_rmon_stats_history_t*)p_tbl;
        rc = tbl_rmon_stats_history_set_rmon_stats_history_field_sync(p_rmon_stats_history, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_syslog_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_syslog_cfg_t *p_syslog_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_syslog_cfg = (tbl_syslog_cfg_t*)p_tbl;
        rc = tbl_syslog_cfg_set_syslog_cfg_field_sync(p_syslog_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_server_key_t *p_ntp_server_key = NULL;
    tbl_ntp_server_t *p_ntp_server = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ntp_server = (tbl_ntp_server_t*)p_tbl;
        rc = tbl_ntp_server_add_ntp_server_sync(p_ntp_server, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ntp_server_key = (tbl_ntp_server_key_t*)p_tbl;
        rc = tbl_ntp_server_del_ntp_server_sync(p_ntp_server_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ntp_server = (tbl_ntp_server_t*)p_tbl;
        rc = tbl_ntp_server_set_ntp_server_field_sync(p_ntp_server, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_ace(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_ace_key_t *p_ntp_ace_key = NULL;
    tbl_ntp_ace_t *p_ntp_ace = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ntp_ace = (tbl_ntp_ace_t*)p_tbl;
        rc = tbl_ntp_ace_add_ntp_ace_sync(p_ntp_ace, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ntp_ace_key = (tbl_ntp_ace_key_t*)p_tbl;
        rc = tbl_ntp_ace_del_ntp_ace_sync(p_ntp_ace_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ntp_ace = (tbl_ntp_ace_t*)p_tbl;
        rc = tbl_ntp_ace_set_ntp_ace_field_sync(p_ntp_ace, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_key(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_key_key_t *p_ntp_key_key = NULL;
    tbl_ntp_key_t *p_ntp_key = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ntp_key = (tbl_ntp_key_t*)p_tbl;
        rc = tbl_ntp_key_add_ntp_key_sync(p_ntp_key, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ntp_key_key = (tbl_ntp_key_key_t*)p_tbl;
        rc = tbl_ntp_key_del_ntp_key_sync(p_ntp_key_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ntp_key = (tbl_ntp_key_t*)p_tbl;
        rc = tbl_ntp_key_set_ntp_key_field_sync(p_ntp_key, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_cfg_t *p_ntp_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ntp_cfg = (tbl_ntp_cfg_t*)p_tbl;
        rc = tbl_ntp_cfg_set_ntp_cfg_field_sync(p_ntp_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_if_t *p_ntp_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ntp_if = (tbl_ntp_if_t*)p_tbl;
        rc = tbl_ntp_if_set_ntp_if_field_sync(p_ntp_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ntp_syncstatus(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ntp_syncstatus_t *p_ntp_syncstatus = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ntp_syncstatus = (tbl_ntp_syncstatus_t*)p_tbl;
        rc = tbl_ntp_syncstatus_set_ntp_syncstatus_field_sync(p_ntp_syncstatus, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_static_dns(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_static_dns_key_t *p_static_cfg_key = NULL;
    tbl_static_dns_t *p_static_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_static_cfg = (tbl_static_dns_t*)p_tbl;
        rc = tbl_static_dns_add_static_dns_sync(p_static_cfg, FALSE);
        break;

    case CDB_OPER_DEL:
        p_static_cfg_key = (tbl_static_dns_key_t*)p_tbl;
        rc = tbl_static_dns_del_static_dns_sync(p_static_cfg_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_static_cfg = (tbl_static_dns_t*)p_tbl;
        rc = tbl_static_dns_set_static_dns_field_sync(p_static_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dynamic_dns_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dynamic_dns_domain_key_t *p_dynamic_domain_cfg_key = NULL;
    tbl_dynamic_dns_domain_t *p_dynamic_domain_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dynamic_domain_cfg = (tbl_dynamic_dns_domain_t*)p_tbl;
        rc = tbl_dynamic_dns_domain_add_dynamic_dns_domain_sync(p_dynamic_domain_cfg, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dynamic_domain_cfg_key = (tbl_dynamic_dns_domain_key_t*)p_tbl;
        rc = tbl_dynamic_dns_domain_del_dynamic_dns_domain_sync(p_dynamic_domain_cfg_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dynamic_domain_cfg = (tbl_dynamic_dns_domain_t*)p_tbl;
        rc = tbl_dynamic_dns_domain_set_dynamic_dns_domain_field_sync(p_dynamic_domain_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dynamic_dns_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key = NULL;
    tbl_dynamic_dns_server_t *p_dynamic_server_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dynamic_server_cfg = (tbl_dynamic_dns_server_t*)p_tbl;
        rc = tbl_dynamic_dns_server_add_dynamic_dns_server_sync(p_dynamic_server_cfg, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dynamic_server_cfg_key = (tbl_dynamic_dns_server_key_t*)p_tbl;
        rc = tbl_dynamic_dns_server_del_dynamic_dns_server_sync(p_dynamic_server_cfg_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dynamic_server_cfg = (tbl_dynamic_dns_server_t*)p_tbl;
        rc = tbl_dynamic_dns_server_set_dynamic_dns_server_field_sync(p_dynamic_server_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_domain_key_t *p_qos_domain_key = NULL;
    tbl_qos_domain_t *p_qos_domain = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_domain = (tbl_qos_domain_t*)p_tbl;
        rc = tbl_qos_domain_add_qos_domain_sync(p_qos_domain, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_domain_key = (tbl_qos_domain_key_t*)p_tbl;
        rc = tbl_qos_domain_del_qos_domain_sync(p_qos_domain_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_domain = (tbl_qos_domain_t*)p_tbl;
        rc = tbl_qos_domain_set_qos_domain_field_sync(p_qos_domain, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_mpls_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key = NULL;
    tbl_qos_mpls_domain_t *p_qos_mpls_domain = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_mpls_domain = (tbl_qos_mpls_domain_t*)p_tbl;
        rc = tbl_qos_mpls_domain_add_qos_mpls_domain_sync(p_qos_mpls_domain, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_mpls_domain_key = (tbl_qos_mpls_domain_key_t*)p_tbl;
        rc = tbl_qos_mpls_domain_del_qos_mpls_domain_sync(p_qos_mpls_domain_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_mpls_domain = (tbl_qos_mpls_domain_t*)p_tbl;
        rc = tbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(p_qos_mpls_domain, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_policer_res(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_policer_res_key_t *p_qos_policer_res_key = NULL;
    tbl_qos_policer_res_t *p_qos_policer_res = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_policer_res = (tbl_qos_policer_res_t*)p_tbl;
        rc = tbl_qos_policer_res_add_qos_policer_res_sync(p_qos_policer_res, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_policer_res_key = (tbl_qos_policer_res_key_t*)p_tbl;
        rc = tbl_qos_policer_res_del_qos_policer_res_sync(p_qos_policer_res_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_policer_res = (tbl_qos_policer_res_t*)p_tbl;
        rc = tbl_qos_policer_res_set_qos_policer_res_field_sync(p_qos_policer_res, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_policer_action_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_policer_action_profile_key_t *p_qos_policer_action_profile_key = NULL;
    tbl_qos_policer_action_profile_t *p_qos_policer_action_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_policer_action_profile = (tbl_qos_policer_action_profile_t*)p_tbl;
        rc = tbl_qos_policer_action_profile_add_qos_policer_action_profile_sync(p_qos_policer_action_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_policer_action_profile_key = (tbl_qos_policer_action_profile_key_t*)p_tbl;
        rc = tbl_qos_policer_action_profile_del_qos_policer_action_profile_sync(p_qos_policer_action_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_policer_action_profile = (tbl_qos_policer_action_profile_t*)p_tbl;
        rc = tbl_qos_policer_action_profile_set_qos_policer_action_profile_field_sync(p_qos_policer_action_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_policer_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_policer_profile_key_t *p_qos_policer_profile_key = NULL;
    tbl_qos_policer_profile_t *p_qos_policer_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_policer_profile = (tbl_qos_policer_profile_t*)p_tbl;
        rc = tbl_qos_policer_profile_add_qos_policer_profile_sync(p_qos_policer_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_policer_profile_key = (tbl_qos_policer_profile_key_t*)p_tbl;
        rc = tbl_qos_policer_profile_del_qos_policer_profile_sync(p_qos_policer_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_policer_profile = (tbl_qos_policer_profile_t*)p_tbl;
        rc = tbl_qos_policer_profile_set_qos_policer_profile_field_sync(p_qos_policer_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_drop_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_drop_profile_key_t *p_qos_drop_profile_key = NULL;
    tbl_qos_drop_profile_t *p_qos_drop_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_drop_profile = (tbl_qos_drop_profile_t*)p_tbl;
        rc = tbl_qos_drop_profile_add_qos_drop_profile_sync(p_qos_drop_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_drop_profile_key = (tbl_qos_drop_profile_key_t*)p_tbl;
        rc = tbl_qos_drop_profile_del_qos_drop_profile_sync(p_qos_drop_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_drop_profile = (tbl_qos_drop_profile_t*)p_tbl;
        rc = tbl_qos_drop_profile_set_qos_drop_profile_field_sync(p_qos_drop_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_queue_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_queue_shape_profile_key_t *p_qos_queue_shape_profile_key = NULL;
    tbl_qos_queue_shape_profile_t *p_qos_queue_shape_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_queue_shape_profile = (tbl_qos_queue_shape_profile_t*)p_tbl;
        rc = tbl_qos_queue_shape_profile_add_qos_queue_shape_profile_sync(p_qos_queue_shape_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_queue_shape_profile_key = (tbl_qos_queue_shape_profile_key_t*)p_tbl;
        rc = tbl_qos_queue_shape_profile_del_qos_queue_shape_profile_sync(p_qos_queue_shape_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_queue_shape_profile = (tbl_qos_queue_shape_profile_t*)p_tbl;
        rc = tbl_qos_queue_shape_profile_set_qos_queue_shape_profile_field_sync(p_qos_queue_shape_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_port_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_port_shape_profile_key_t *p_qos_port_shape_profile_key = NULL;
    tbl_qos_port_shape_profile_t *p_qos_port_shape_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_port_shape_profile = (tbl_qos_port_shape_profile_t*)p_tbl;
        rc = tbl_qos_port_shape_profile_add_qos_port_shape_profile_sync(p_qos_port_shape_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_port_shape_profile_key = (tbl_qos_port_shape_profile_key_t*)p_tbl;
        rc = tbl_qos_port_shape_profile_del_qos_port_shape_profile_sync(p_qos_port_shape_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_port_shape_profile = (tbl_qos_port_shape_profile_t*)p_tbl;
        rc = tbl_qos_port_shape_profile_set_qos_port_shape_profile_field_sync(p_qos_port_shape_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_queue_smart_buffer(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_queue_smart_buffer_key_t *p_qos_queue_smart_buffer_key = NULL;
    tbl_qos_queue_smart_buffer_t *p_qos_queue_smart_buffer = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_qos_queue_smart_buffer = (tbl_qos_queue_smart_buffer_t*)p_tbl;
        rc = tbl_qos_queue_smart_buffer_add_qos_queue_smart_buffer_sync(p_qos_queue_smart_buffer, FALSE);
        break;

    case CDB_OPER_DEL:
        p_qos_queue_smart_buffer_key = (tbl_qos_queue_smart_buffer_key_t*)p_tbl;
        rc = tbl_qos_queue_smart_buffer_del_qos_queue_smart_buffer_sync(p_qos_queue_smart_buffer_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_qos_queue_smart_buffer = (tbl_qos_queue_smart_buffer_t*)p_tbl;
        rc = tbl_qos_queue_smart_buffer_set_qos_queue_smart_buffer_field_sync(p_qos_queue_smart_buffer, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_qos_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_qos_global_t *p_qos_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_qos_glb = (tbl_qos_global_t*)p_tbl;
        rc = tbl_qos_global_set_qos_global_field_sync(p_qos_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mirror(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mirror_key_t *p_mirror_key = NULL;
    tbl_mirror_t *p_mirror = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mirror = (tbl_mirror_t*)p_tbl;
        rc = tbl_mirror_add_mirror_sync(p_mirror, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mirror_key = (tbl_mirror_key_t*)p_tbl;
        rc = tbl_mirror_del_mirror_sync(p_mirror_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mirror = (tbl_mirror_t*)p_tbl;
        rc = tbl_mirror_set_mirror_field_sync(p_mirror, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mirror_mac_escape(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key = NULL;
    tbl_mirror_mac_escape_t *p_mirror_mac_escape = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mirror_mac_escape = (tbl_mirror_mac_escape_t*)p_tbl;
        rc = tbl_mirror_mac_escape_add_mirror_mac_escape_sync(p_mirror_mac_escape, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mirror_mac_escape_key = (tbl_mirror_mac_escape_key_t*)p_tbl;
        rc = tbl_mirror_mac_escape_del_mirror_mac_escape_sync(p_mirror_mac_escape_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mirror_mac_escape = (tbl_mirror_mac_escape_t*)p_tbl;
        rc = tbl_mirror_mac_escape_set_mirror_mac_escape_field_sync(p_mirror_mac_escape, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_group_ingress(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_group_ingress_key_t *p_tap_group_ingress_key = NULL;
    tbl_tap_group_ingress_t *p_tap_group_ingress = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tap_group_ingress = (tbl_tap_group_ingress_t*)p_tbl;
        rc = tbl_tap_group_ingress_add_tap_group_ingress_sync(p_tap_group_ingress, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tap_group_ingress_key = (tbl_tap_group_ingress_key_t*)p_tbl;
        rc = tbl_tap_group_ingress_del_tap_group_ingress_sync(p_tap_group_ingress_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tap_group_ingress = (tbl_tap_group_ingress_t*)p_tbl;
        rc = tbl_tap_group_ingress_set_tap_group_ingress_field_sync(p_tap_group_ingress, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_group_ingress_flow(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key = NULL;
    tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tap_group_ingress_flow = (tbl_tap_group_ingress_flow_t*)p_tbl;
        rc = tbl_tap_group_ingress_flow_add_tap_group_ingress_flow_sync(p_tap_group_ingress_flow, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tap_group_ingress_flow_key = (tbl_tap_group_ingress_flow_key_t*)p_tbl;
        rc = tbl_tap_group_ingress_flow_del_tap_group_ingress_flow_sync(p_tap_group_ingress_flow_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tap_group_ingress_flow = (tbl_tap_group_ingress_flow_t*)p_tbl;
        rc = tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field_sync(p_tap_group_ingress_flow, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_group_egress(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_group_egress_key_t *p_tap_group_egress_key = NULL;
    tbl_tap_group_egress_t *p_tap_group_egress = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tap_group_egress = (tbl_tap_group_egress_t*)p_tbl;
        rc = tbl_tap_group_egress_add_tap_group_egress_sync(p_tap_group_egress, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tap_group_egress_key = (tbl_tap_group_egress_key_t*)p_tbl;
        rc = tbl_tap_group_egress_del_tap_group_egress_sync(p_tap_group_egress_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tap_group_egress = (tbl_tap_group_egress_t*)p_tbl;
        rc = tbl_tap_group_egress_set_tap_group_egress_field_sync(p_tap_group_egress, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_group_key_t *p_tap_group_key = NULL;
    tbl_tap_group_t *p_tap_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tap_group = (tbl_tap_group_t*)p_tbl;
        rc = tbl_tap_group_add_tap_group_sync(p_tap_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tap_group_key = (tbl_tap_group_key_t*)p_tbl;
        rc = tbl_tap_group_del_tap_group_sync(p_tap_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tap_group = (tbl_tap_group_t*)p_tbl;
        rc = tbl_tap_group_set_tap_group_field_sync(p_tap_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_five_tuple(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_five_tuple_key_t *p_tap_five_tuple_key = NULL;
    tbl_tap_five_tuple_t *p_tap_five_tuple = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tap_five_tuple = (tbl_tap_five_tuple_t*)p_tbl;
        rc = tbl_tap_five_tuple_add_tap_five_tuple_sync(p_tap_five_tuple, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tap_five_tuple_key = (tbl_tap_five_tuple_key_t*)p_tbl;
        rc = tbl_tap_five_tuple_del_tap_five_tuple_sync(p_tap_five_tuple_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tap_five_tuple = (tbl_tap_five_tuple_t*)p_tbl;
        rc = tbl_tap_five_tuple_set_tap_five_tuple_field_sync(p_tap_five_tuple, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_dpi_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_dpi_global_t *p_tap_dpi_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_tap_dpi_global = (tbl_tap_dpi_global_t*)p_tbl;
        rc = tbl_tap_dpi_global_set_tap_dpi_global_field_sync(p_tap_dpi_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_user(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_user_key_t *p_user_key = NULL;
    tbl_user_t *p_user = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_user = (tbl_user_t*)p_tbl;
        rc = tbl_user_add_user_sync(p_user, FALSE);
        break;

    case CDB_OPER_DEL:
        p_user_key = (tbl_user_key_t*)p_tbl;
        rc = tbl_user_del_user_sync(p_user_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_user = (tbl_user_t*)p_tbl;
        rc = tbl_user_set_user_field_sync(p_user, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_login_record(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_login_record_key_t *p_login_record_key = NULL;
    tbl_login_record_t *p_login_record = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_login_record = (tbl_login_record_t*)p_tbl;
        rc = tbl_login_record_add_login_record_sync(p_login_record, FALSE);
        break;

    case CDB_OPER_DEL:
        p_login_record_key = (tbl_login_record_key_t*)p_tbl;
        rc = tbl_login_record_del_login_record_sync(p_login_record_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_login_record = (tbl_login_record_t*)p_tbl;
        rc = tbl_login_record_set_login_record_field_sync(p_login_record, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_login_security_glb(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_login_security_glb_t *p_login_security_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_login_security_glb = (tbl_login_security_glb_t*)p_tbl;
        rc = tbl_login_security_glb_set_login_security_glb_field_sync(p_login_security_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vty(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vty_key_t *p_vty_key = NULL;
    tbl_vty_t *p_vty = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vty = (tbl_vty_t*)p_tbl;
        rc = tbl_vty_add_vty_sync(p_vty, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vty_key = (tbl_vty_key_t*)p_tbl;
        rc = tbl_vty_del_vty_sync(p_vty_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vty = (tbl_vty_t*)p_tbl;
        rc = tbl_vty_set_vty_field_sync(p_vty, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_console(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_console_t *p_console = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_console = (tbl_console_t*)p_tbl;
        rc = tbl_console_set_console_field_sync(p_console, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_authen(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_authen_key_t *p_authen_key = NULL;
    tbl_authen_t *p_authen = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_authen = (tbl_authen_t*)p_tbl;
        rc = tbl_authen_add_authen_sync(p_authen, FALSE);
        break;

    case CDB_OPER_DEL:
        p_authen_key = (tbl_authen_key_t*)p_tbl;
        rc = tbl_authen_del_authen_sync(p_authen_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_authen = (tbl_authen_t*)p_tbl;
        rc = tbl_authen_set_authen_field_sync(p_authen, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_login(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_login_key_t *p_login_key = NULL;
    tbl_login_t *p_login = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_login = (tbl_login_t*)p_tbl;
        rc = tbl_login_add_login_sync(p_login, FALSE);
        break;

    case CDB_OPER_DEL:
        p_login_key = (tbl_login_key_t*)p_tbl;
        rc = tbl_login_del_login_sync(p_login_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_login = (tbl_login_t*)p_tbl;
        rc = tbl_login_set_login_field_sync(p_login, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rsa(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rsa_key_t *p_rsa_key = NULL;
    tbl_rsa_t *p_rsa = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rsa = (tbl_rsa_t*)p_tbl;
        rc = tbl_rsa_add_rsa_sync(p_rsa, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rsa_key = (tbl_rsa_key_t*)p_tbl;
        rc = tbl_rsa_del_rsa_sync(p_rsa_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rsa = (tbl_rsa_t*)p_tbl;
        rc = tbl_rsa_set_rsa_field_sync(p_rsa, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_t *p_openflow = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_openflow = (tbl_openflow_t*)p_tbl;
        rc = tbl_openflow_set_openflow_field_sync(p_openflow, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_traffic(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_traffic_key_t *p_cpu_traffic_key = NULL;
    tbl_cpu_traffic_t *p_cpu_traffic = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cpu_traffic = (tbl_cpu_traffic_t*)p_tbl;
        rc = tbl_cpu_traffic_add_cpu_traffic_sync(p_cpu_traffic, FALSE);
        break;

    case CDB_OPER_DEL:
        p_cpu_traffic_key = (tbl_cpu_traffic_key_t*)p_tbl;
        rc = tbl_cpu_traffic_del_cpu_traffic_sync(p_cpu_traffic_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_cpu_traffic = (tbl_cpu_traffic_t*)p_tbl;
        rc = tbl_cpu_traffic_set_cpu_traffic_field_sync(p_cpu_traffic, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_traffic_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key = NULL;
    tbl_cpu_traffic_group_t *p_cpu_traffic_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cpu_traffic_group = (tbl_cpu_traffic_group_t*)p_tbl;
        rc = tbl_cpu_traffic_group_add_cpu_traffic_group_sync(p_cpu_traffic_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_cpu_traffic_group_key = (tbl_cpu_traffic_group_key_t*)p_tbl;
        rc = tbl_cpu_traffic_group_del_cpu_traffic_group_sync(p_cpu_traffic_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_cpu_traffic_group = (tbl_cpu_traffic_group_t*)p_tbl;
        rc = tbl_cpu_traffic_group_set_cpu_traffic_group_field_sync(p_cpu_traffic_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_utilization(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_utilization_key_t *p_cpu_utilize_key = NULL;
    tbl_cpu_utilization_t *p_cpu_utilize = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cpu_utilize = (tbl_cpu_utilization_t*)p_tbl;
        rc = tbl_cpu_utilization_add_cpu_utilization_sync(p_cpu_utilize, FALSE);
        break;

    case CDB_OPER_DEL:
        p_cpu_utilize_key = (tbl_cpu_utilization_key_t*)p_tbl;
        rc = tbl_cpu_utilization_del_cpu_utilization_sync(p_cpu_utilize_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_cpu_utilize = (tbl_cpu_utilization_t*)p_tbl;
        rc = tbl_cpu_utilization_set_cpu_utilization_field_sync(p_cpu_utilize, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_limit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_limit_key_t *p_cpu_limit_key = NULL;
    tbl_cpu_limit_t *p_cpu_limit = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cpu_limit = (tbl_cpu_limit_t*)p_tbl;
        rc = tbl_cpu_limit_add_cpu_limit_sync(p_cpu_limit, FALSE);
        break;

    case CDB_OPER_DEL:
        p_cpu_limit_key = (tbl_cpu_limit_key_t*)p_tbl;
        rc = tbl_cpu_limit_del_cpu_limit_sync(p_cpu_limit_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_cpu_limit = (tbl_cpu_limit_t*)p_tbl;
        rc = tbl_cpu_limit_set_cpu_limit_field_sync(p_cpu_limit, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcrelay(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcrelay_t *p_dhcrelay = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dhcrelay = (tbl_dhcrelay_t*)p_tbl;
        rc = tbl_dhcrelay_set_dhcrelay_field_sync(p_dhcrelay, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcsrvgrp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key = NULL;
    tbl_dhcsrvgrp_t *p_dhcsrvgrp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dhcsrvgrp = (tbl_dhcsrvgrp_t*)p_tbl;
        rc = tbl_dhcsrvgrp_add_dhcsrvgrp_sync(p_dhcsrvgrp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dhcsrvgrp_key = (tbl_dhcsrvgrp_key_t*)p_tbl;
        rc = tbl_dhcsrvgrp_del_dhcsrvgrp_sync(p_dhcsrvgrp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dhcsrvgrp = (tbl_dhcsrvgrp_t*)p_tbl;
        rc = tbl_dhcsrvgrp_set_dhcsrvgrp_field_sync(p_dhcsrvgrp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcp_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcp_debug_t *p_dhcp_debug = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dhcp_debug = (tbl_dhcp_debug_t*)p_tbl;
        rc = tbl_dhcp_debug_set_dhcp_debug_field_sync(p_dhcp_debug, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhclient(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhclient_t *p_dhclient = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dhclient = (tbl_dhclient_t*)p_tbl;
        rc = tbl_dhclient_set_dhclient_field_sync(p_dhclient, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcsnooping(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcsnooping_t *p_dhcsnooping = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dhcsnooping = (tbl_dhcsnooping_t*)p_tbl;
        rc = tbl_dhcsnooping_set_dhcsnooping_field_sync(p_dhcsnooping, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcbinding(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcbinding_key_t *p_dhcbinding_key = NULL;
    tbl_dhcbinding_t *p_dhcbinding = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dhcbinding = (tbl_dhcbinding_t*)p_tbl;
        rc = tbl_dhcbinding_add_dhcbinding_sync(p_dhcbinding, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dhcbinding_key = (tbl_dhcbinding_key_t*)p_tbl;
        rc = tbl_dhcbinding_del_dhcbinding_sync(p_dhcbinding_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dhcbinding = (tbl_dhcbinding_t*)p_tbl;
        rc = tbl_dhcbinding_set_dhcbinding_field_sync(p_dhcbinding, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_iptables_prevent(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_iptables_prevent_t *p_iptables_prevent = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_iptables_prevent = (tbl_iptables_prevent_t*)p_tbl;
        rc = tbl_iptables_prevent_set_iptables_prevent_field_sync(p_iptables_prevent, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_errdisable(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_errdisable_key_t *p_errdisable_key = NULL;
    tbl_errdisable_t *p_errdisable = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_errdisable = (tbl_errdisable_t*)p_tbl;
        rc = tbl_errdisable_add_errdisable_sync(p_errdisable, FALSE);
        break;

    case CDB_OPER_DEL:
        p_errdisable_key = (tbl_errdisable_key_t*)p_tbl;
        rc = tbl_errdisable_del_errdisable_sync(p_errdisable_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_errdisable = (tbl_errdisable_t*)p_tbl;
        rc = tbl_errdisable_set_errdisable_field_sync(p_errdisable, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ns_port_forwarding(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ns_port_forwarding_key_t *p_ns_port_forwarding_key = NULL;
    tbl_ns_port_forwarding_t *p_ns_port_forwarding = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ns_port_forwarding = (tbl_ns_port_forwarding_t*)p_tbl;
        rc = tbl_ns_port_forwarding_add_ns_port_forwarding_sync(p_ns_port_forwarding, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ns_port_forwarding_key = (tbl_ns_port_forwarding_key_t*)p_tbl;
        rc = tbl_ns_port_forwarding_del_ns_port_forwarding_sync(p_ns_port_forwarding_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ns_port_forwarding = (tbl_ns_port_forwarding_t*)p_tbl;
        rc = tbl_ns_port_forwarding_set_ns_port_forwarding_field_sync(p_ns_port_forwarding, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_log_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_log_global_t *p_log_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_log_glb = (tbl_log_global_t*)p_tbl;
        rc = tbl_log_global_set_log_global_field_sync(p_log_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_log(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_log_key_t *p_log_key = NULL;
    tbl_log_t *p_log = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_log = (tbl_log_t*)p_tbl;
        rc = tbl_log_add_log_sync(p_log, FALSE);
        break;

    case CDB_OPER_DEL:
        p_log_key = (tbl_log_key_t*)p_tbl;
        rc = tbl_log_del_log_sync(p_log_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_log = (tbl_log_t*)p_tbl;
        rc = tbl_log_set_log_field_sync(p_log, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sys_load(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sys_load_t *p_sys_load = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_sys_load = (tbl_sys_load_t*)p_tbl;
        rc = tbl_sys_load_set_sys_load_field_sync(p_sys_load, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cem(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cem_key_t *p_cem_key = NULL;
    tbl_cem_t *p_cem = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cem = (tbl_cem_t*)p_tbl;
        rc = tbl_cem_add_cem_sync(p_cem, FALSE);
        break;

    case CDB_OPER_DEL:
        p_cem_key = (tbl_cem_key_t*)p_tbl;
        rc = tbl_cem_del_cem_sync(p_cem_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_cem = (tbl_cem_t*)p_tbl;
        rc = tbl_cem_set_cem_field_sync(p_cem, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_clock(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_clock_t *p_clk = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_clk = (tbl_clock_t*)p_tbl;
        rc = tbl_clock_set_clock_field_sync(p_clk, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tag_summer_clock(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tag_summer_clock_t *p_tag_summer_clk = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_tag_summer_clk = (tbl_tag_summer_clock_t*)p_tbl;
        rc = tbl_tag_summer_clock_set_tag_summer_clock_field_sync(p_tag_summer_clk, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_port_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_port_stats_key_t *p_port_stats_key = NULL;
    tbl_port_stats_t *p_port_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port_stats = (tbl_port_stats_t*)p_tbl;
        rc = tbl_port_stats_add_port_stats_sync(p_port_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_port_stats_key = (tbl_port_stats_key_t*)p_tbl;
        rc = tbl_port_stats_del_port_stats_sync(p_port_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_port_stats = (tbl_port_stats_t*)p_tbl;
        rc = tbl_port_stats_set_port_stats_field_sync(p_port_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlan_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlan_stats_key_t *p_vlan_stats_key = NULL;
    tbl_vlan_stats_t *p_vlan_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan_stats = (tbl_vlan_stats_t*)p_tbl;
        rc = tbl_vlan_stats_add_vlan_stats_sync(p_vlan_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlan_stats_key = (tbl_vlan_stats_key_t*)p_tbl;
        rc = tbl_vlan_stats_del_vlan_stats_sync(p_vlan_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlan_stats = (tbl_vlan_stats_t*)p_tbl;
        rc = tbl_vlan_stats_set_vlan_stats_field_sync(p_vlan_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_port_stats_rate(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_port_stats_rate_key_t *p_port_stats_rate_key = NULL;
    tbl_port_stats_rate_t *p_port_stats_rate = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port_stats_rate = (tbl_port_stats_rate_t*)p_tbl;
        rc = tbl_port_stats_rate_add_port_stats_rate_sync(p_port_stats_rate, FALSE);
        break;

    case CDB_OPER_DEL:
        p_port_stats_rate_key = (tbl_port_stats_rate_key_t*)p_tbl;
        rc = tbl_port_stats_rate_del_port_stats_rate_sync(p_port_stats_rate_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_port_stats_rate = (tbl_port_stats_rate_t*)p_tbl;
        rc = tbl_port_stats_rate_set_port_stats_rate_field_sync(p_port_stats_rate, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_aclqos_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_aclqos_if_key_t *p_aclqos_if_key = NULL;
    tbl_aclqos_if_t *p_aclqos_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_aclqos_if = (tbl_aclqos_if_t*)p_tbl;
        rc = tbl_aclqos_if_add_aclqos_if_sync(p_aclqos_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_aclqos_if_key = (tbl_aclqos_if_key_t*)p_tbl;
        rc = tbl_aclqos_if_del_aclqos_if_sync(p_aclqos_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_aclqos_if = (tbl_aclqos_if_t*)p_tbl;
        rc = tbl_aclqos_if_set_aclqos_if_field_sync(p_aclqos_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_l2_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_l2_action_t *p_l2_act = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_l2_act = (tbl_l2_action_t*)p_tbl;
        rc = tbl_l2_action_set_l2_action_field_sync(p_l2_act, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_qos_drop_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_qos_drop_profile_key_t *p_fea_qos_drop_profile_key = NULL;
    tbl_fea_qos_drop_profile_t *p_fea_qos_drop_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_qos_drop_profile = (tbl_fea_qos_drop_profile_t*)p_tbl;
        rc = tbl_fea_qos_drop_profile_add_fea_qos_drop_profile_sync(p_fea_qos_drop_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_qos_drop_profile_key = (tbl_fea_qos_drop_profile_key_t*)p_tbl;
        rc = tbl_fea_qos_drop_profile_del_fea_qos_drop_profile_sync(p_fea_qos_drop_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_qos_drop_profile = (tbl_fea_qos_drop_profile_t*)p_tbl;
        rc = tbl_fea_qos_drop_profile_set_fea_qos_drop_profile_field_sync(p_fea_qos_drop_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_qos_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_qos_domain_key_t *p_fea_qos_domain_key = NULL;
    tbl_fea_qos_domain_t *p_fea_qos_domain = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_qos_domain = (tbl_fea_qos_domain_t*)p_tbl;
        rc = tbl_fea_qos_domain_add_fea_qos_domain_sync(p_fea_qos_domain, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_qos_domain_key = (tbl_fea_qos_domain_key_t*)p_tbl;
        rc = tbl_fea_qos_domain_del_fea_qos_domain_sync(p_fea_qos_domain_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_qos_domain = (tbl_fea_qos_domain_t*)p_tbl;
        rc = tbl_fea_qos_domain_set_fea_qos_domain_field_sync(p_fea_qos_domain, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_g8032_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key = NULL;
    tbl_fea_g8032_ring_t *p_fea_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_g8032_ring = (tbl_fea_g8032_ring_t*)p_tbl;
        rc = tbl_fea_g8032_ring_add_fea_g8032_ring_sync(p_fea_g8032_ring, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_g8032_ring_key = (tbl_fea_g8032_ring_key_t*)p_tbl;
        rc = tbl_fea_g8032_ring_del_fea_g8032_ring_sync(p_fea_g8032_ring_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_g8032_ring = (tbl_fea_g8032_ring_t*)p_tbl;
        rc = tbl_fea_g8032_ring_set_fea_g8032_ring_field_sync(p_fea_g8032_ring, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_g8032_ring_mc_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_g8032_ring_mc_group_key_t *p_fea_g8032_ring_mc_group_key = NULL;
    tbl_fea_g8032_ring_mc_group_t *p_fea_g8032_ring_mc_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_g8032_ring_mc_group = (tbl_fea_g8032_ring_mc_group_t*)p_tbl;
        rc = tbl_fea_g8032_ring_mc_group_add_fea_g8032_ring_mc_group_sync(p_fea_g8032_ring_mc_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_g8032_ring_mc_group_key = (tbl_fea_g8032_ring_mc_group_key_t*)p_tbl;
        rc = tbl_fea_g8032_ring_mc_group_del_fea_g8032_ring_mc_group_sync(p_fea_g8032_ring_mc_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_g8032_ring_mc_group = (tbl_fea_g8032_ring_mc_group_t*)p_tbl;
        rc = tbl_fea_g8032_ring_mc_group_set_fea_g8032_ring_mc_group_field_sync(p_fea_g8032_ring_mc_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_qos_queue_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_qos_queue_shape_profile_key_t *p_fea_qos_queue_shape_profile_key = NULL;
    tbl_fea_qos_queue_shape_profile_t *p_fea_qos_queue_shape_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_qos_queue_shape_profile = (tbl_fea_qos_queue_shape_profile_t*)p_tbl;
        rc = tbl_fea_qos_queue_shape_profile_add_fea_qos_queue_shape_profile_sync(p_fea_qos_queue_shape_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_qos_queue_shape_profile_key = (tbl_fea_qos_queue_shape_profile_key_t*)p_tbl;
        rc = tbl_fea_qos_queue_shape_profile_del_fea_qos_queue_shape_profile_sync(p_fea_qos_queue_shape_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_qos_queue_shape_profile = (tbl_fea_qos_queue_shape_profile_t*)p_tbl;
        rc = tbl_fea_qos_queue_shape_profile_set_fea_qos_queue_shape_profile_field_sync(p_fea_qos_queue_shape_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_qos_port_shape_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_qos_port_shape_profile_key_t *p_fea_qos_port_shape_profile_key = NULL;
    tbl_fea_qos_port_shape_profile_t *p_fea_qos_port_shape_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_qos_port_shape_profile = (tbl_fea_qos_port_shape_profile_t*)p_tbl;
        rc = tbl_fea_qos_port_shape_profile_add_fea_qos_port_shape_profile_sync(p_fea_qos_port_shape_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_qos_port_shape_profile_key = (tbl_fea_qos_port_shape_profile_key_t*)p_tbl;
        rc = tbl_fea_qos_port_shape_profile_del_fea_qos_port_shape_profile_sync(p_fea_qos_port_shape_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_qos_port_shape_profile = (tbl_fea_qos_port_shape_profile_t*)p_tbl;
        rc = tbl_fea_qos_port_shape_profile_set_fea_qos_port_shape_profile_field_sync(p_fea_qos_port_shape_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_port_policer_apply(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_port_policer_apply_key_t *p_fea_port_policer_apply_key = NULL;
    tbl_fea_port_policer_apply_t *p_fea_port_policer_apply = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_port_policer_apply = (tbl_fea_port_policer_apply_t*)p_tbl;
        rc = tbl_fea_port_policer_apply_add_fea_port_policer_apply_sync(p_fea_port_policer_apply, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_port_policer_apply_key = (tbl_fea_port_policer_apply_key_t*)p_tbl;
        rc = tbl_fea_port_policer_apply_del_fea_port_policer_apply_sync(p_fea_port_policer_apply_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_port_policer_apply = (tbl_fea_port_policer_apply_t*)p_tbl;
        rc = tbl_fea_port_policer_apply_set_fea_port_policer_apply_field_sync(p_fea_port_policer_apply, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_aclqos_if_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key = NULL;
    tbl_aclqos_if_stats_t *p_aclqos_if_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_aclqos_if_stats = (tbl_aclqos_if_stats_t*)p_tbl;
        rc = tbl_aclqos_if_stats_add_aclqos_if_stats_sync(p_aclqos_if_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_aclqos_if_stats_key = (tbl_aclqos_if_stats_key_t*)p_tbl;
        rc = tbl_aclqos_if_stats_del_aclqos_if_stats_sync(p_aclqos_if_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_aclqos_if_stats = (tbl_aclqos_if_stats_t*)p_tbl;
        rc = tbl_aclqos_if_stats_set_aclqos_if_stats_field_sync(p_aclqos_if_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_version(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_version_t *p_ver = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ver = (tbl_version_t*)p_tbl;
        rc = tbl_version_set_version_field_sync(p_ver, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_manage_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_manage_if_t *p_mng_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_mng_if = (tbl_manage_if_t*)p_tbl;
        rc = tbl_manage_if_set_manage_if_field_sync(p_mng_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bootimage(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bootimage_t *p_boot = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_boot = (tbl_bootimage_t*)p_tbl;
        rc = tbl_bootimage_set_bootimage_field_sync(p_boot, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_chassis(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_chassis_t *p_chassis = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_chassis = (tbl_chassis_t*)p_tbl;
        rc = tbl_chassis_set_chassis_field_sync(p_chassis, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ifname_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ifname_info_t *p_ifname_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ifname_info = (tbl_ifname_info_t*)p_tbl;
        rc = tbl_ifname_info_set_ifname_info_field_sync(p_ifname_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_card(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_card_key_t *p_card_key = NULL;
    tbl_card_t *p_card = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_card = (tbl_card_t*)p_tbl;
        rc = tbl_card_add_card_sync(p_card, FALSE);
        break;

    case CDB_OPER_DEL:
        p_card_key = (tbl_card_key_t*)p_tbl;
        rc = tbl_card_del_card_sync(p_card_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_card = (tbl_card_t*)p_tbl;
        rc = tbl_card_set_card_field_sync(p_card, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_port_key_t *p_port_key = NULL;
    tbl_port_t *p_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port = (tbl_port_t*)p_tbl;
        rc = tbl_port_add_port_sync(p_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_port_key = (tbl_port_key_t*)p_tbl;
        rc = tbl_port_del_port_sync(p_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_port = (tbl_port_t*)p_tbl;
        rc = tbl_port_set_port_field_sync(p_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fiber(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fiber_key_t *p_fiber_key = NULL;
    tbl_fiber_t *p_fiber = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fiber = (tbl_fiber_t*)p_tbl;
        rc = tbl_fiber_add_fiber_sync(p_fiber, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fiber_key = (tbl_fiber_key_t*)p_tbl;
        rc = tbl_fiber_del_fiber_sync(p_fiber_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fiber = (tbl_fiber_t*)p_tbl;
        rc = tbl_fiber_set_fiber_field_sync(p_fiber, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sys_spec(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sys_spec_key_t *p_sys_spec_key = NULL;
    tbl_sys_spec_t *p_sys_spec = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_sys_spec = (tbl_sys_spec_t*)p_tbl;
        rc = tbl_sys_spec_add_sys_spec_sync(p_sys_spec, FALSE);
        break;

    case CDB_OPER_DEL:
        p_sys_spec_key = (tbl_sys_spec_key_t*)p_tbl;
        rc = tbl_sys_spec_del_sys_spec_sync(p_sys_spec_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_sys_spec = (tbl_sys_spec_t*)p_tbl;
        rc = tbl_sys_spec_set_sys_spec_field_sync(p_sys_spec, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fan(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fan_key_t *p_fan_key = NULL;
    tbl_fan_t *p_fan = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fan = (tbl_fan_t*)p_tbl;
        rc = tbl_fan_add_fan_sync(p_fan, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fan_key = (tbl_fan_key_t*)p_tbl;
        rc = tbl_fan_del_fan_sync(p_fan_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fan = (tbl_fan_t*)p_tbl;
        rc = tbl_fan_set_fan_field_sync(p_fan, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_psu(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_psu_key_t *p_psu_key = NULL;
    tbl_psu_t *p_psu = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_psu = (tbl_psu_t*)p_tbl;
        rc = tbl_psu_add_psu_sync(p_psu, FALSE);
        break;

    case CDB_OPER_DEL:
        p_psu_key = (tbl_psu_key_t*)p_tbl;
        rc = tbl_psu_del_psu_sync(p_psu_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_psu = (tbl_psu_t*)p_tbl;
        rc = tbl_psu_set_psu_field_sync(p_psu, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_led(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_led_key_t *p_led_key = NULL;
    tbl_led_t *p_led = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_led = (tbl_led_t*)p_tbl;
        rc = tbl_led_add_led_sync(p_led, FALSE);
        break;

    case CDB_OPER_DEL:
        p_led_key = (tbl_led_key_t*)p_tbl;
        rc = tbl_led_del_led_sync(p_led_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_led = (tbl_led_t*)p_tbl;
        rc = tbl_led_set_led_field_sync(p_led, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sensor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sensor_key_t *p_sensor_key = NULL;
    tbl_sensor_t *p_sensor = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_sensor = (tbl_sensor_t*)p_tbl;
        rc = tbl_sensor_add_sensor_sync(p_sensor, FALSE);
        break;

    case CDB_OPER_DEL:
        p_sensor_key = (tbl_sensor_key_t*)p_tbl;
        rc = tbl_sensor_del_sensor_sync(p_sensor_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_sensor = (tbl_sensor_t*)p_tbl;
        rc = tbl_sensor_set_sensor_field_sync(p_sensor, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vcm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vcm_key_t *p_vcm_key = NULL;
    tbl_vcm_t *p_vcm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vcm = (tbl_vcm_t*)p_tbl;
        rc = tbl_vcm_add_vcm_sync(p_vcm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vcm_key = (tbl_vcm_key_t*)p_tbl;
        rc = tbl_vcm_del_vcm_sync(p_vcm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vcm = (tbl_vcm_t*)p_tbl;
        rc = tbl_vcm_set_vcm_field_sync(p_vcm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_poe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_poe_key_t *p_poe_key = NULL;
    tbl_poe_t *p_poe = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_poe = (tbl_poe_t*)p_tbl;
        rc = tbl_poe_add_poe_sync(p_poe, FALSE);
        break;

    case CDB_OPER_DEL:
        p_poe_key = (tbl_poe_key_t*)p_tbl;
        rc = tbl_poe_del_poe_sync(p_poe_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_poe = (tbl_poe_t*)p_tbl;
        rc = tbl_poe_set_poe_field_sync(p_poe, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_reboot_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_reboot_info_key_t *p_reboot_info_key = NULL;
    tbl_reboot_info_t *p_reboot_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_reboot_info = (tbl_reboot_info_t*)p_tbl;
        rc = tbl_reboot_info_add_reboot_info_sync(p_reboot_info, FALSE);
        break;

    case CDB_OPER_DEL:
        p_reboot_info_key = (tbl_reboot_info_key_t*)p_tbl;
        rc = tbl_reboot_info_del_reboot_info_sync(p_reboot_info_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_reboot_info = (tbl_reboot_info_t*)p_tbl;
        rc = tbl_reboot_info_set_reboot_info_field_sync(p_reboot_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_errdisable_flap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_errdisable_flap_key_t *p_errdisable_flap_key = NULL;
    tbl_errdisable_flap_t *p_errdisable_flap = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_errdisable_flap = (tbl_errdisable_flap_t*)p_tbl;
        rc = tbl_errdisable_flap_add_errdisable_flap_sync(p_errdisable_flap, FALSE);
        break;

    case CDB_OPER_DEL:
        p_errdisable_flap_key = (tbl_errdisable_flap_key_t*)p_tbl;
        rc = tbl_errdisable_flap_del_errdisable_flap_sync(p_errdisable_flap_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_errdisable_flap = (tbl_errdisable_flap_t*)p_tbl;
        rc = tbl_errdisable_flap_set_errdisable_flap_field_sync(p_errdisable_flap, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_opm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_opm_global_t *p_opmglb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_opmglb = (tbl_opm_global_t*)p_tbl;
        rc = tbl_opm_global_set_opm_global_field_sync(p_opmglb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_erps_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_erps_ring_key_t *p_erps_ring_key = NULL;
    tbl_erps_ring_t *p_erps_ring = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_erps_ring = (tbl_erps_ring_t*)p_tbl;
        rc = tbl_erps_ring_add_erps_ring_sync(p_erps_ring, FALSE);
        break;

    case CDB_OPER_DEL:
        p_erps_ring_key = (tbl_erps_ring_key_t*)p_tbl;
        rc = tbl_erps_ring_del_erps_ring_sync(p_erps_ring_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_erps_ring = (tbl_erps_ring_t*)p_tbl;
        rc = tbl_erps_ring_set_erps_ring_field_sync(p_erps_ring, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_erps_domain(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_erps_domain_key_t *p_erps_domain_key = NULL;
    tbl_erps_domain_t *p_erps_domain = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_erps_domain = (tbl_erps_domain_t*)p_tbl;
        rc = tbl_erps_domain_add_erps_domain_sync(p_erps_domain, FALSE);
        break;

    case CDB_OPER_DEL:
        p_erps_domain_key = (tbl_erps_domain_key_t*)p_tbl;
        rc = tbl_erps_domain_del_erps_domain_sync(p_erps_domain_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_erps_domain = (tbl_erps_domain_t*)p_tbl;
        rc = tbl_erps_domain_set_erps_domain_field_sync(p_erps_domain, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_opm_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_opm_debug_t *p_opmdbg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_opmdbg = (tbl_opm_debug_t*)p_tbl;
        rc = tbl_opm_debug_set_opm_debug_field_sync(p_opmdbg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_policy_map_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_policy_map_config_key_t *p_policy_map_config_key = NULL;
    tbl_policy_map_config_t *p_policy_map_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_policy_map_config = (tbl_policy_map_config_t*)p_tbl;
        rc = tbl_policy_map_config_add_policy_map_config_sync(p_policy_map_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_policy_map_config_key = (tbl_policy_map_config_key_t*)p_tbl;
        rc = tbl_policy_map_config_del_policy_map_config_sync(p_policy_map_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_policy_map_config = (tbl_policy_map_config_t*)p_tbl;
        rc = tbl_policy_map_config_set_policy_map_config_field_sync(p_policy_map_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_class_map_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_class_map_config_key_t *p_class_map_config_key = NULL;
    tbl_class_map_config_t *p_class_map_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_class_map_config = (tbl_class_map_config_t*)p_tbl;
        rc = tbl_class_map_config_add_class_map_config_sync(p_class_map_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_class_map_config_key = (tbl_class_map_config_key_t*)p_tbl;
        rc = tbl_class_map_config_del_class_map_config_sync(p_class_map_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_class_map_config = (tbl_class_map_config_t*)p_tbl;
        rc = tbl_class_map_config_set_class_map_config_field_sync(p_class_map_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_class_in_policy_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_class_in_policy_config_key_t *p_class_in_policy_config_key = NULL;
    tbl_class_in_policy_config_t *p_class_in_policy_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_class_in_policy_config = (tbl_class_in_policy_config_t*)p_tbl;
        rc = tbl_class_in_policy_config_add_class_in_policy_config_sync(p_class_in_policy_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_class_in_policy_config_key = (tbl_class_in_policy_config_key_t*)p_tbl;
        rc = tbl_class_in_policy_config_del_class_in_policy_config_sync(p_class_in_policy_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_class_in_policy_config = (tbl_class_in_policy_config_t*)p_tbl;
        rc = tbl_class_in_policy_config_set_class_in_policy_config_field_sync(p_class_in_policy_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_in_class_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_in_class_config_key_t *p_acl_in_class_config_key = NULL;
    tbl_acl_in_class_config_t *p_acl_in_class_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_in_class_config = (tbl_acl_in_class_config_t*)p_tbl;
        rc = tbl_acl_in_class_config_add_acl_in_class_config_sync(p_acl_in_class_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_in_class_config_key = (tbl_acl_in_class_config_key_t*)p_tbl;
        rc = tbl_acl_in_class_config_del_acl_in_class_config_sync(p_acl_in_class_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_in_class_config = (tbl_acl_in_class_config_t*)p_tbl;
        rc = tbl_acl_in_class_config_set_acl_in_class_config_field_sync(p_acl_in_class_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_class_map_action_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_class_map_action_config_key_t *p_class_map_action_config_key = NULL;
    tbl_class_map_action_config_t *p_class_map_action_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_class_map_action_config = (tbl_class_map_action_config_t*)p_tbl;
        rc = tbl_class_map_action_config_add_class_map_action_config_sync(p_class_map_action_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_class_map_action_config_key = (tbl_class_map_action_config_key_t*)p_tbl;
        rc = tbl_class_map_action_config_del_class_map_action_config_sync(p_class_map_action_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_class_map_action_config = (tbl_class_map_action_config_t*)p_tbl;
        rc = tbl_class_map_action_config_set_class_map_action_config_field_sync(p_class_map_action_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_acl_policy_action(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key = NULL;
    tbl_fea_acl_policy_action_t *p_fea_acl_policy_action = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_acl_policy_action = (tbl_fea_acl_policy_action_t*)p_tbl;
        rc = tbl_fea_acl_policy_action_add_fea_acl_policy_action_sync(p_fea_acl_policy_action, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_acl_policy_action_key = (tbl_fea_acl_policy_action_key_t*)p_tbl;
        rc = tbl_fea_acl_policy_action_del_fea_acl_policy_action_sync(p_fea_acl_policy_action_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_acl_policy_action = (tbl_fea_acl_policy_action_t*)p_tbl;
        rc = tbl_fea_acl_policy_action_set_fea_acl_policy_action_field_sync(p_fea_acl_policy_action, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_igsp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_global_t *p_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_glb = (tbl_igsp_global_t*)p_tbl;
        rc = tbl_igsp_global_set_igsp_global_field_sync(p_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_igsp_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_intf_key_t *p_if_key = NULL;
    tbl_igsp_intf_t *p_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_igsp_intf_t*)p_tbl;
        rc = tbl_igsp_intf_add_igsp_intf_sync(p_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_igsp_intf_key_t*)p_tbl;
        rc = tbl_igsp_intf_del_igsp_intf_sync(p_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_if = (tbl_igsp_intf_t*)p_tbl;
        rc = tbl_igsp_intf_set_igsp_intf_field_sync(p_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_igsp_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igsp_group_key_t *p_group_key = NULL;
    tbl_igsp_group_t *p_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_group = (tbl_igsp_group_t*)p_tbl;
        rc = tbl_igsp_group_add_igsp_group_sync(p_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_group_key = (tbl_igsp_group_key_t*)p_tbl;
        rc = tbl_igsp_group_del_igsp_group_sync(p_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_group = (tbl_igsp_group_t*)p_tbl;
        rc = tbl_igsp_group_set_igsp_group_field_sync(p_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_auth_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_auth_cfg_t *p_auth_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_auth_cfg = (tbl_auth_cfg_t*)p_tbl;
        rc = tbl_auth_cfg_set_auth_cfg_field_sync(p_auth_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_auth_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_auth_server_key_t *p_auth_server_key = NULL;
    tbl_auth_server_t *p_auth_server = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_auth_server = (tbl_auth_server_t*)p_tbl;
        rc = tbl_auth_server_add_auth_server_sync(p_auth_server, FALSE);
        break;

    case CDB_OPER_DEL:
        p_auth_server_key = (tbl_auth_server_key_t*)p_tbl;
        rc = tbl_auth_server_del_auth_server_sync(p_auth_server_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_auth_server = (tbl_auth_server_t*)p_tbl;
        rc = tbl_auth_server_set_auth_server_field_sync(p_auth_server, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_auth_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_auth_session_key_t *p_auth_session_key = NULL;
    tbl_auth_session_t *p_auth_session = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_auth_session = (tbl_auth_session_t*)p_tbl;
        rc = tbl_auth_session_add_auth_session_sync(p_auth_session, FALSE);
        break;

    case CDB_OPER_DEL:
        p_auth_session_key = (tbl_auth_session_key_t*)p_tbl;
        rc = tbl_auth_session_del_auth_session_sync(p_auth_session_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_auth_session = (tbl_auth_session_t*)p_tbl;
        rc = tbl_auth_session_set_auth_session_field_sync(p_auth_session, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_authd_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_authd_debug_t *p_authd_debug = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_authd_debug = (tbl_authd_debug_t*)p_tbl;
        rc = tbl_authd_debug_set_authd_debug_field_sync(p_authd_debug, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dot1x_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_global_t *p_dot1x_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dot1x_global = (tbl_dot1x_global_t*)p_tbl;
        rc = tbl_dot1x_global_set_dot1x_global_field_sync(p_dot1x_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dot1x_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_port_key_t *p_dot1x_port_key = NULL;
    tbl_dot1x_port_t *p_dot1x_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dot1x_port = (tbl_dot1x_port_t*)p_tbl;
        rc = tbl_dot1x_port_add_dot1x_port_sync(p_dot1x_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dot1x_port_key = (tbl_dot1x_port_key_t*)p_tbl;
        rc = tbl_dot1x_port_del_dot1x_port_sync(p_dot1x_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dot1x_port = (tbl_dot1x_port_t*)p_tbl;
        rc = tbl_dot1x_port_set_dot1x_port_field_sync(p_dot1x_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dot1x_radius(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_radius_key_t *p_dot1x_radius_key = NULL;
    tbl_dot1x_radius_t *p_dot1x_radius = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dot1x_radius = (tbl_dot1x_radius_t*)p_tbl;
        rc = tbl_dot1x_radius_add_dot1x_radius_sync(p_dot1x_radius, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dot1x_radius_key = (tbl_dot1x_radius_key_t*)p_tbl;
        rc = tbl_dot1x_radius_del_dot1x_radius_sync(p_dot1x_radius_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dot1x_radius = (tbl_dot1x_radius_t*)p_tbl;
        rc = tbl_dot1x_radius_set_dot1x_radius_field_sync(p_dot1x_radius, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dot1x_mac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dot1x_mac_key_t *p_dot1x_mac_key = NULL;
    tbl_dot1x_mac_t *p_dot1x_mac = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_dot1x_mac = (tbl_dot1x_mac_t*)p_tbl;
        rc = tbl_dot1x_mac_add_dot1x_mac_sync(p_dot1x_mac, FALSE);
        break;

    case CDB_OPER_DEL:
        p_dot1x_mac_key = (tbl_dot1x_mac_key_t*)p_tbl;
        rc = tbl_dot1x_mac_del_dot1x_mac_sync(p_dot1x_mac_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_dot1x_mac = (tbl_dot1x_mac_t*)p_tbl;
        rc = tbl_dot1x_mac_set_dot1x_mac_field_sync(p_dot1x_mac, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_enable(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_enable_key_t *p_enable_key = NULL;
    tbl_enable_t *p_enable = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_enable = (tbl_enable_t*)p_tbl;
        rc = tbl_enable_add_enable_sync(p_enable, FALSE);
        break;

    case CDB_OPER_DEL:
        p_enable_key = (tbl_enable_key_t*)p_tbl;
        rc = tbl_enable_del_enable_sync(p_enable_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_enable = (tbl_enable_t*)p_tbl;
        rc = tbl_enable_set_enable_field_sync(p_enable, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_chip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_chip_t *p_chip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_chip = (tbl_chip_t*)p_tbl;
        rc = tbl_chip_set_chip_field_sync(p_chip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_clear_acl_policy(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_clear_acl_policy_t *p_clear_acl_policy = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_clear_acl_policy = (tbl_clear_acl_policy_t*)p_tbl;
        rc = tbl_clear_acl_policy_set_clear_acl_policy_field_sync(p_clear_acl_policy, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_author(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_author_key_t *p_author_key = NULL;
    tbl_author_t *p_author = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_author = (tbl_author_t*)p_tbl;
        rc = tbl_author_add_author_sync(p_author, FALSE);
        break;

    case CDB_OPER_DEL:
        p_author_key = (tbl_author_key_t*)p_tbl;
        rc = tbl_author_del_author_sync(p_author_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_author = (tbl_author_t*)p_tbl;
        rc = tbl_author_set_author_field_sync(p_author, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_account(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_account_key_t *p_account_key = NULL;
    tbl_account_t *p_account = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_account = (tbl_account_t*)p_tbl;
        rc = tbl_account_add_account_sync(p_account, FALSE);
        break;

    case CDB_OPER_DEL:
        p_account_key = (tbl_account_key_t*)p_tbl;
        rc = tbl_account_del_account_sync(p_account_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_account = (tbl_account_t*)p_tbl;
        rc = tbl_account_set_account_field_sync(p_account, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_accountcmd(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_accountcmd_key_t *p_accountcmd_key = NULL;
    tbl_accountcmd_t *p_accountcmd = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_accountcmd = (tbl_accountcmd_t*)p_tbl;
        rc = tbl_accountcmd_add_accountcmd_sync(p_accountcmd, FALSE);
        break;

    case CDB_OPER_DEL:
        p_accountcmd_key = (tbl_accountcmd_key_t*)p_tbl;
        rc = tbl_accountcmd_del_accountcmd_sync(p_accountcmd_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_accountcmd = (tbl_accountcmd_t*)p_tbl;
        rc = tbl_accountcmd_set_accountcmd_field_sync(p_accountcmd, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlanclass_rule(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlanclass_rule_key_t *p_vlanclass_rule_key = NULL;
    tbl_vlanclass_rule_t *p_vlanclass_rule = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlanclass_rule = (tbl_vlanclass_rule_t*)p_tbl;
        rc = tbl_vlanclass_rule_add_vlanclass_rule_sync(p_vlanclass_rule, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlanclass_rule_key = (tbl_vlanclass_rule_key_t*)p_tbl;
        rc = tbl_vlanclass_rule_del_vlanclass_rule_sync(p_vlanclass_rule_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlanclass_rule = (tbl_vlanclass_rule_t*)p_tbl;
        rc = tbl_vlanclass_rule_set_vlanclass_rule_field_sync(p_vlanclass_rule, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlanclass_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlanclass_group_key_t *p_vlanclass_group_key = NULL;
    tbl_vlanclass_group_t *p_vlanclass_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlanclass_group = (tbl_vlanclass_group_t*)p_tbl;
        rc = tbl_vlanclass_group_add_vlanclass_group_sync(p_vlanclass_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlanclass_group_key = (tbl_vlanclass_group_key_t*)p_tbl;
        rc = tbl_vlanclass_group_del_vlanclass_group_sync(p_vlanclass_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlanclass_group = (tbl_vlanclass_group_t*)p_tbl;
        rc = tbl_vlanclass_group_set_vlanclass_group_field_sync(p_vlanclass_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_l4_port_range(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key = NULL;
    tbl_acl_l4_port_range_t *p_acl_l4_port_range = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_l4_port_range = (tbl_acl_l4_port_range_t*)p_tbl;
        rc = tbl_acl_l4_port_range_add_acl_l4_port_range_sync(p_acl_l4_port_range, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_l4_port_range_key = (tbl_acl_l4_port_range_key_t*)p_tbl;
        rc = tbl_acl_l4_port_range_del_acl_l4_port_range_sync(p_acl_l4_port_range_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_l4_port_range = (tbl_acl_l4_port_range_t*)p_tbl;
        rc = tbl_acl_l4_port_range_set_acl_l4_port_range_field_sync(p_acl_l4_port_range, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_udf_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_udf_group_key_t *p_acl_udf_group_key = NULL;
    tbl_acl_udf_group_t *p_acl_udf_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_udf_group = (tbl_acl_udf_group_t*)p_tbl;
        rc = tbl_acl_udf_group_add_acl_udf_group_sync(p_acl_udf_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_udf_group_key = (tbl_acl_udf_group_key_t*)p_tbl;
        rc = tbl_acl_udf_group_del_acl_udf_group_sync(p_acl_udf_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_udf_group = (tbl_acl_udf_group_t*)p_tbl;
        rc = tbl_acl_udf_group_set_acl_udf_group_field_sync(p_acl_udf_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_acl_udf_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_acl_udf_entry_key_t *p_acl_udf_entry_key = NULL;
    tbl_acl_udf_entry_t *p_acl_udf_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_acl_udf_entry = (tbl_acl_udf_entry_t*)p_tbl;
        rc = tbl_acl_udf_entry_add_acl_udf_entry_sync(p_acl_udf_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_acl_udf_entry_key = (tbl_acl_udf_entry_key_t*)p_tbl;
        rc = tbl_acl_udf_entry_del_acl_udf_entry_sync(p_acl_udf_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_acl_udf_entry = (tbl_acl_udf_entry_t*)p_tbl;
        rc = tbl_acl_udf_entry_set_acl_udf_entry_field_sync(p_acl_udf_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_pcap(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_pcap_t *p_pcap = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_pcap = (tbl_fea_pcap_t*)p_tbl;
        rc = tbl_fea_pcap_set_fea_pcap_field_sync(p_pcap, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_controller(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_controller_key_t *p_controller_key = NULL;
    tbl_controller_t *p_controller = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_controller = (tbl_controller_t*)p_tbl;
        rc = tbl_controller_add_controller_sync(p_controller, FALSE);
        break;

    case CDB_OPER_DEL:
        p_controller_key = (tbl_controller_key_t*)p_tbl;
        rc = tbl_controller_del_controller_sync(p_controller_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_controller = (tbl_controller_t*)p_tbl;
        rc = tbl_controller_set_controller_field_sync(p_controller, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_packets(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_packets_t *p_cpu_packets = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cpu_packets = (tbl_cpu_packets_t*)p_tbl;
        rc = tbl_cpu_packets_set_cpu_packets_field_sync(p_cpu_packets, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ns_route(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ns_route_key_t *p_ns_route_key = NULL;
    tbl_ns_route_t *p_ns_route = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ns_route = (tbl_ns_route_t*)p_tbl;
        rc = tbl_ns_route_add_ns_route_sync(p_ns_route, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ns_route_key = (tbl_ns_route_key_t*)p_tbl;
        rc = tbl_ns_route_del_ns_route_sync(p_ns_route_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ns_route = (tbl_ns_route_t*)p_tbl;
        rc = tbl_ns_route_set_ns_route_field_sync(p_ns_route, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ns_route_ip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ns_route_ip_key_t *p_ns_route_ip_key = NULL;
    tbl_ns_route_ip_t *p_ns_route_ip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ns_route_ip = (tbl_ns_route_ip_t*)p_tbl;
        rc = tbl_ns_route_ip_add_ns_route_ip_sync(p_ns_route_ip, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ns_route_ip_key = (tbl_ns_route_ip_key_t*)p_tbl;
        rc = tbl_ns_route_ip_del_ns_route_ip_sync(p_ns_route_ip_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ns_route_ip = (tbl_ns_route_ip_t*)p_tbl;
        rc = tbl_ns_route_ip_set_ns_route_ip_field_sync(p_ns_route_ip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_interface_key_t *p_if_key = NULL;
    tbl_openflow_interface_t *p_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if = (tbl_openflow_interface_t*)p_tbl;
        rc = tbl_openflow_interface_add_openflow_interface_sync(p_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_openflow_interface_key_t*)p_tbl;
        rc = tbl_openflow_interface_del_openflow_interface_sync(p_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_if = (tbl_openflow_interface_t*)p_tbl;
        rc = tbl_openflow_interface_set_openflow_interface_field_sync(p_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow_manager(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_manager_key_t *p_openflow_manager_key = NULL;
    tbl_openflow_manager_t *p_openflow_manager = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_openflow_manager = (tbl_openflow_manager_t*)p_tbl;
        rc = tbl_openflow_manager_add_openflow_manager_sync(p_openflow_manager, FALSE);
        break;

    case CDB_OPER_DEL:
        p_openflow_manager_key = (tbl_openflow_manager_key_t*)p_tbl;
        rc = tbl_openflow_manager_del_openflow_manager_sync(p_openflow_manager_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_openflow_manager = (tbl_openflow_manager_t*)p_tbl;
        rc = tbl_openflow_manager_set_openflow_manager_field_sync(p_openflow_manager, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ptp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_global_t *p_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_global = (tbl_ptp_global_t*)p_tbl;
        rc = tbl_ptp_global_set_ptp_global_field_sync(p_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ptp_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_port_key_t *p_port_key = NULL;
    tbl_ptp_port_t *p_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port = (tbl_ptp_port_t*)p_tbl;
        rc = tbl_ptp_port_add_ptp_port_sync(p_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_port_key = (tbl_ptp_port_key_t*)p_tbl;
        rc = tbl_ptp_port_del_ptp_port_sync(p_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_port = (tbl_ptp_port_t*)p_tbl;
        rc = tbl_ptp_port_set_ptp_port_field_sync(p_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ptp_foreign(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_foreign_key_t *p_foreign_key = NULL;
    tbl_ptp_foreign_t *p_foreign = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_foreign = (tbl_ptp_foreign_t*)p_tbl;
        rc = tbl_ptp_foreign_add_ptp_foreign_sync(p_foreign, FALSE);
        break;

    case CDB_OPER_DEL:
        p_foreign_key = (tbl_ptp_foreign_key_t*)p_tbl;
        rc = tbl_ptp_foreign_del_ptp_foreign_sync(p_foreign_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_foreign = (tbl_ptp_foreign_t*)p_tbl;
        rc = tbl_ptp_foreign_set_ptp_foreign_field_sync(p_foreign, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ptp_residence_time_cache(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ptp_residence_time_cache_key_t *p_residence_time_cache_key = NULL;
    tbl_ptp_residence_time_cache_t *p_residence_time_cache = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_residence_time_cache = (tbl_ptp_residence_time_cache_t*)p_tbl;
        rc = tbl_ptp_residence_time_cache_add_ptp_residence_time_cache_sync(p_residence_time_cache, FALSE);
        break;

    case CDB_OPER_DEL:
        p_residence_time_cache_key = (tbl_ptp_residence_time_cache_key_t*)p_tbl;
        rc = tbl_ptp_residence_time_cache_del_ptp_residence_time_cache_sync(p_residence_time_cache_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_residence_time_cache = (tbl_ptp_residence_time_cache_t*)p_tbl;
        rc = tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field_sync(p_residence_time_cache, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_ptp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_fea_ptp_global = (tbl_fea_ptp_global_t*)p_tbl;
        rc = tbl_fea_ptp_global_set_fea_ptp_global_field_sync(p_fea_ptp_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_ptp_ts(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key = NULL;
    tbl_fea_ptp_ts_t *p_fea_ptp_ts = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_ptp_ts = (tbl_fea_ptp_ts_t*)p_tbl;
        rc = tbl_fea_ptp_ts_add_fea_ptp_ts_sync(p_fea_ptp_ts, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_ptp_ts_key = (tbl_fea_ptp_ts_key_t*)p_tbl;
        rc = tbl_fea_ptp_ts_del_fea_ptp_ts_sync(p_fea_ptp_ts_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_ptp_ts = (tbl_fea_ptp_ts_t*)p_tbl;
        rc = tbl_fea_ptp_ts_set_fea_ptp_ts_field_sync(p_fea_ptp_ts, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_time(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_time_t *p_fea_time = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_fea_time = (tbl_fea_time_t*)p_tbl;
        rc = tbl_fea_time_set_fea_time_field_sync(p_fea_time, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bhm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bhm_global_t *p_bhm_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_bhm_glb = (tbl_bhm_global_t*)p_tbl;
        rc = tbl_bhm_global_set_bhm_global_field_sync(p_bhm_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_bhm_module(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_bhm_module_key_t *p_bhm_module_key = NULL;
    tbl_bhm_module_t *p_bhm_module = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_bhm_module = (tbl_bhm_module_t*)p_tbl;
        rc = tbl_bhm_module_add_bhm_module_sync(p_bhm_module, FALSE);
        break;

    case CDB_OPER_DEL:
        p_bhm_module_key = (tbl_bhm_module_key_t*)p_tbl;
        rc = tbl_bhm_module_del_bhm_module_sync(p_bhm_module_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_bhm_module = (tbl_bhm_module_t*)p_tbl;
        rc = tbl_bhm_module_set_bhm_module_field_sync(p_bhm_module, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow_tunnel_interface(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_tunnel_interface_key_t *p_tunnel_if_key = NULL;
    tbl_openflow_tunnel_interface_t *p_tunnel_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_tunnel_if = (tbl_openflow_tunnel_interface_t*)p_tbl;
        rc = tbl_openflow_tunnel_interface_add_openflow_tunnel_interface_sync(p_tunnel_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_tunnel_if_key = (tbl_openflow_tunnel_interface_key_t*)p_tbl;
        rc = tbl_openflow_tunnel_interface_del_openflow_tunnel_interface_sync(p_tunnel_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_tunnel_if = (tbl_openflow_tunnel_interface_t*)p_tbl;
        rc = tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field_sync(p_tunnel_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_openflow_tunnel_local_ip_cnt(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key = NULL;
    tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_openflow_tunnel_local_ip = (tbl_openflow_tunnel_local_ip_cnt_t*)p_tbl;
        rc = tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt_sync(p_openflow_tunnel_local_ip, FALSE);
        break;

    case CDB_OPER_DEL:
        p_openflow_tunnel_local_ip_key = (tbl_openflow_tunnel_local_ip_cnt_key_t*)p_tbl;
        rc = tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt_sync(p_openflow_tunnel_local_ip_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_openflow_tunnel_local_ip = (tbl_openflow_tunnel_local_ip_cnt_t*)p_tbl;
        rc = tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field_sync(p_openflow_tunnel_local_ip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_inband_snat(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_inband_snat_key_t *p_inband_snat_key = NULL;
    tbl_inband_snat_t *p_inband_snat = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_inband_snat = (tbl_inband_snat_t*)p_tbl;
        rc = tbl_inband_snat_add_inband_snat_sync(p_inband_snat, FALSE);
        break;

    case CDB_OPER_DEL:
        p_inband_snat_key = (tbl_inband_snat_key_t*)p_tbl;
        rc = tbl_inband_snat_del_inband_snat_sync(p_inband_snat_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_inband_snat = (tbl_inband_snat_t*)p_tbl;
        rc = tbl_inband_snat_set_inband_snat_field_sync(p_inband_snat, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_arpinsp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arpinsp_key_t *p_arpinsp_key = NULL;
    tbl_arpinsp_t *p_arpinsp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arpinsp = (tbl_arpinsp_t*)p_tbl;
        rc = tbl_arpinsp_add_arpinsp_sync(p_arpinsp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_arpinsp_key = (tbl_arpinsp_key_t*)p_tbl;
        rc = tbl_arpinsp_del_arpinsp_sync(p_arpinsp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_arpinsp = (tbl_arpinsp_t*)p_tbl;
        rc = tbl_arpinsp_set_arpinsp_field_sync(p_arpinsp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_arpacl_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arpacl_config_key_t *p_arpacl_config_key = NULL;
    tbl_arpacl_config_t *p_arpacl_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arpacl_config = (tbl_arpacl_config_t*)p_tbl;
        rc = tbl_arpacl_config_add_arpacl_config_sync(p_arpacl_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_arpacl_config_key = (tbl_arpacl_config_key_t*)p_tbl;
        rc = tbl_arpacl_config_del_arpacl_config_sync(p_arpacl_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_arpacl_config = (tbl_arpacl_config_t*)p_tbl;
        rc = tbl_arpacl_config_set_arpacl_config_field_sync(p_arpacl_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_arpace_config(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_arpace_config_key_t *p_arpace_config_key = NULL;
    tbl_arpace_config_t *p_arpace_config = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_arpace_config = (tbl_arpace_config_t*)p_tbl;
        rc = tbl_arpace_config_add_arpace_config_sync(p_arpace_config, FALSE);
        break;

    case CDB_OPER_DEL:
        p_arpace_config_key = (tbl_arpace_config_key_t*)p_tbl;
        rc = tbl_arpace_config_del_arpace_config_sync(p_arpace_config_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_arpace_config = (tbl_arpace_config_t*)p_tbl;
        rc = tbl_arpace_config_set_arpace_config_field_sync(p_arpace_config, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_copp_cfg(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_copp_cfg_t *p_copp_cfg = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_copp_cfg = (tbl_copp_cfg_t*)p_tbl;
        rc = tbl_copp_cfg_set_copp_cfg_field_sync(p_copp_cfg, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_fea_copp_trap_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_fea_copp_trap_group_key_t *p_fea_copp_trap_group_key = NULL;
    tbl_fea_copp_trap_group_t *p_fea_copp_trap_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fea_copp_trap_group = (tbl_fea_copp_trap_group_t*)p_tbl;
        rc = tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(p_fea_copp_trap_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_fea_copp_trap_group_key = (tbl_fea_copp_trap_group_key_t*)p_tbl;
        rc = tbl_fea_copp_trap_group_del_fea_copp_trap_group_sync(p_fea_copp_trap_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_fea_copp_trap_group = (tbl_fea_copp_trap_group_t*)p_tbl;
        rc = tbl_fea_copp_trap_group_set_fea_copp_trap_group_field_sync(p_fea_copp_trap_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sflow_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sflow_global_t *p_sflow_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_sflow_glb = (tbl_sflow_global_t*)p_tbl;
        rc = tbl_sflow_global_set_sflow_global_field_sync(p_sflow_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sflow_collector(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sflow_collector_key_t *p_sflow_collector_key = NULL;
    tbl_sflow_collector_t *p_sflow_collector = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_sflow_collector = (tbl_sflow_collector_t*)p_tbl;
        rc = tbl_sflow_collector_add_sflow_collector_sync(p_sflow_collector, FALSE);
        break;

    case CDB_OPER_DEL:
        p_sflow_collector_key = (tbl_sflow_collector_key_t*)p_tbl;
        rc = tbl_sflow_collector_del_sflow_collector_sync(p_sflow_collector_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_sflow_collector = (tbl_sflow_collector_t*)p_tbl;
        rc = tbl_sflow_collector_set_sflow_collector_field_sync(p_sflow_collector, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_sflow_counter_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_sflow_counter_port_key_t *p_sflow_counter_port_key = NULL;
    tbl_sflow_counter_port_t *p_sflow_counter_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_sflow_counter_port = (tbl_sflow_counter_port_t*)p_tbl;
        rc = tbl_sflow_counter_port_add_sflow_counter_port_sync(p_sflow_counter_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_sflow_counter_port_key = (tbl_sflow_counter_port_key_t*)p_tbl;
        rc = tbl_sflow_counter_port_del_sflow_counter_port_sync(p_sflow_counter_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_sflow_counter_port = (tbl_sflow_counter_port_t*)p_tbl;
        rc = tbl_sflow_counter_port_set_sflow_counter_port_field_sync(p_sflow_counter_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vrrp_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vrrp_global_t *p_vrrp_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_vrrp_glb = (tbl_vrrp_global_t*)p_tbl;
        rc = tbl_vrrp_global_set_vrrp_global_field_sync(p_vrrp_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vrrp_vmac(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vrrp_vmac_key_t *p_vrrp_vmac_key = NULL;
    tbl_vrrp_vmac_t *p_vrrp_vmac = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vrrp_vmac = (tbl_vrrp_vmac_t*)p_tbl;
        rc = tbl_vrrp_vmac_add_vrrp_vmac_sync(p_vrrp_vmac, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vrrp_vmac_key = (tbl_vrrp_vmac_key_t*)p_tbl;
        rc = tbl_vrrp_vmac_del_vrrp_vmac_sync(p_vrrp_vmac_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vrrp_vmac = (tbl_vrrp_vmac_t*)p_tbl;
        rc = tbl_vrrp_vmac_set_vrrp_vmac_field_sync(p_vrrp_vmac, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vrrp_vip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vrrp_vip_key_t *p_vrrp_vip_key = NULL;
    tbl_vrrp_vip_t *p_vrrp_vip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vrrp_vip = (tbl_vrrp_vip_t*)p_tbl;
        rc = tbl_vrrp_vip_add_vrrp_vip_sync(p_vrrp_vip, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vrrp_vip_key = (tbl_vrrp_vip_key_t*)p_tbl;
        rc = tbl_vrrp_vip_del_vrrp_vip_sync(p_vrrp_vip_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vrrp_vip = (tbl_vrrp_vip_t*)p_tbl;
        rc = tbl_vrrp_vip_set_vrrp_vip_field_sync(p_vrrp_vip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vrrp_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vrrp_session_key_t *p_vrrp_sess_key = NULL;
    tbl_vrrp_session_t *p_vrrp_sess = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vrrp_sess = (tbl_vrrp_session_t*)p_tbl;
        rc = tbl_vrrp_session_add_vrrp_session_sync(p_vrrp_sess, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vrrp_sess_key = (tbl_vrrp_session_key_t*)p_tbl;
        rc = tbl_vrrp_session_del_vrrp_session_sync(p_vrrp_sess_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vrrp_sess = (tbl_vrrp_session_t*)p_tbl;
        rc = tbl_vrrp_session_set_vrrp_session_field_sync(p_vrrp_sess, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ssm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssm_global_t *p_ssm_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ssm_glb = (tbl_ssm_global_t*)p_tbl;
        rc = tbl_ssm_global_set_ssm_global_field_sync(p_ssm_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ssm_port(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ssm_port_key_t *p_ssm_port_key = NULL;
    tbl_ssm_port_t *p_ssm_port = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ssm_port = (tbl_ssm_port_t*)p_tbl;
        rc = tbl_ssm_port_add_ssm_port_sync(p_ssm_port, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ssm_port_key = (tbl_ssm_port_key_t*)p_tbl;
        rc = tbl_ssm_port_del_ssm_port_sync(p_ssm_port_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ssm_port = (tbl_ssm_port_t*)p_tbl;
        rc = tbl_ssm_port_set_ssm_port_field_sync(p_ssm_port, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsla_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_global_t *p_ipsla_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ipsla_glb = (tbl_ipsla_global_t*)p_tbl;
        rc = tbl_ipsla_global_set_ipsla_global_field_sync(p_ipsla_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsla_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_entry_key_t *p_ipsla_entry_key = NULL;
    tbl_ipsla_entry_t *p_ipsla_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsla_entry = (tbl_ipsla_entry_t*)p_tbl;
        rc = tbl_ipsla_entry_add_ipsla_entry_sync(p_ipsla_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsla_entry_key = (tbl_ipsla_entry_key_t*)p_tbl;
        rc = tbl_ipsla_entry_del_ipsla_entry_sync(p_ipsla_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsla_entry = (tbl_ipsla_entry_t*)p_tbl;
        rc = tbl_ipsla_entry_set_ipsla_entry_field_sync(p_ipsla_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsla_packet_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_packet_stats_key_t *p_ipsla_packet_stats_key = NULL;
    tbl_ipsla_packet_stats_t *p_ipsla_packet_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsla_packet_stats = (tbl_ipsla_packet_stats_t*)p_tbl;
        rc = tbl_ipsla_packet_stats_add_ipsla_packet_stats_sync(p_ipsla_packet_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsla_packet_stats_key = (tbl_ipsla_packet_stats_key_t*)p_tbl;
        rc = tbl_ipsla_packet_stats_del_ipsla_packet_stats_sync(p_ipsla_packet_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsla_packet_stats = (tbl_ipsla_packet_stats_t*)p_tbl;
        rc = tbl_ipsla_packet_stats_set_ipsla_packet_stats_field_sync(p_ipsla_packet_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ipsla_test_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key = NULL;
    tbl_ipsla_test_stats_t *p_ipsla_test_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ipsla_test_stats = (tbl_ipsla_test_stats_t*)p_tbl;
        rc = tbl_ipsla_test_stats_add_ipsla_test_stats_sync(p_ipsla_test_stats, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ipsla_test_stats_key = (tbl_ipsla_test_stats_key_t*)p_tbl;
        rc = tbl_ipsla_test_stats_del_ipsla_test_stats_sync(p_ipsla_test_stats_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ipsla_test_stats = (tbl_ipsla_test_stats_t*)p_tbl;
        rc = tbl_ipsla_test_stats_set_ipsla_test_stats_field_sync(p_ipsla_test_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_track_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_track_global_t *p_track_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_track_glb = (tbl_track_global_t*)p_tbl;
        rc = tbl_track_global_set_track_global_field_sync(p_track_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_track_object(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_track_object_key_t *p_track_object_key = NULL;
    tbl_track_object_t *p_track_object = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_track_object = (tbl_track_object_t*)p_tbl;
        rc = tbl_track_object_add_track_object_sync(p_track_object, FALSE);
        break;

    case CDB_OPER_DEL:
        p_track_object_key = (tbl_track_object_key_t*)p_tbl;
        rc = tbl_track_object_del_track_object_sync(p_track_object_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_track_object = (tbl_track_object_t*)p_tbl;
        rc = tbl_track_object_set_track_object_field_sync(p_track_object, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_controller_attr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_controller_attr_key_t *p_controller_attr_key = NULL;
    tbl_controller_attr_t *p_controller_attr = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_controller_attr = (tbl_controller_attr_t*)p_tbl;
        rc = tbl_controller_attr_add_controller_attr_sync(p_controller_attr, FALSE);
        break;

    case CDB_OPER_DEL:
        p_controller_attr_key = (tbl_controller_attr_key_t*)p_tbl;
        rc = tbl_controller_attr_del_controller_attr_sync(p_controller_attr_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_controller_attr = (tbl_controller_attr_t*)p_tbl;
        rc = tbl_controller_attr_set_controller_attr_field_sync(p_controller_attr, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_oam_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_session_key_t *p_oam_session_key = NULL;
    tbl_oam_session_t *p_oam_session = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_session = (tbl_oam_session_t*)p_tbl;
        rc = tbl_oam_session_add_oam_session_sync(p_oam_session, FALSE);
        break;

    case CDB_OPER_DEL:
        p_oam_session_key = (tbl_oam_session_key_t*)p_tbl;
        rc = tbl_oam_session_del_oam_session_sync(p_oam_session_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_oam_session = (tbl_oam_session_t*)p_tbl;
        rc = tbl_oam_session_set_oam_session_field_sync(p_oam_session, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_oam_session_upload(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_session_upload_key_t *p_oam_session_upload_key = NULL;
    tbl_oam_session_upload_t *p_oam_session_upload = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_session_upload = (tbl_oam_session_upload_t*)p_tbl;
        rc = tbl_oam_session_upload_add_oam_session_upload_sync(p_oam_session_upload, FALSE);
        break;

    case CDB_OPER_DEL:
        p_oam_session_upload_key = (tbl_oam_session_upload_key_t*)p_tbl;
        rc = tbl_oam_session_upload_del_oam_session_upload_sync(p_oam_session_upload_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_oam_session_upload = (tbl_oam_session_upload_t*)p_tbl;
        rc = tbl_oam_session_upload_set_oam_session_upload_field_sync(p_oam_session_upload, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lsp_aps_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lsp_aps_group_key_t *p_lsp_aps_group_key = NULL;
    tbl_lsp_aps_group_t *p_lsp_aps_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_lsp_aps_group = (tbl_lsp_aps_group_t*)p_tbl;
        rc = tbl_lsp_aps_group_add_lsp_aps_group_sync(p_lsp_aps_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_lsp_aps_group_key = (tbl_lsp_aps_group_key_t*)p_tbl;
        rc = tbl_lsp_aps_group_del_lsp_aps_group_sync(p_lsp_aps_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_lsp_aps_group = (tbl_lsp_aps_group_t*)p_tbl;
        rc = tbl_lsp_aps_group_set_lsp_aps_group_field_sync(p_lsp_aps_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pw_aps_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pw_aps_group_key_t *p_pw_aps_group_key = NULL;
    tbl_pw_aps_group_t *p_pw_aps_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pw_aps_group = (tbl_pw_aps_group_t*)p_tbl;
        rc = tbl_pw_aps_group_add_pw_aps_group_sync(p_pw_aps_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pw_aps_group_key = (tbl_pw_aps_group_key_t*)p_tbl;
        rc = tbl_pw_aps_group_del_pw_aps_group_sync(p_pw_aps_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pw_aps_group = (tbl_pw_aps_group_t*)p_tbl;
        rc = tbl_pw_aps_group_set_pw_aps_group_field_sync(p_pw_aps_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lsp_pe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lsp_pe_key_t *p_lsp_pe_key = NULL;
    tbl_lsp_pe_t *p_lsp_pe = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_lsp_pe = (tbl_lsp_pe_t*)p_tbl;
        rc = tbl_lsp_pe_add_lsp_pe_sync(p_lsp_pe, FALSE);
        break;

    case CDB_OPER_DEL:
        p_lsp_pe_key = (tbl_lsp_pe_key_t*)p_tbl;
        rc = tbl_lsp_pe_del_lsp_pe_sync(p_lsp_pe_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_lsp_pe = (tbl_lsp_pe_t*)p_tbl;
        rc = tbl_lsp_pe_set_lsp_pe_field_sync(p_lsp_pe, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_lsp_p(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_lsp_p_key_t *p_lsp_p_key = NULL;
    tbl_lsp_p_t *p_lsp_p = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_lsp_p = (tbl_lsp_p_t*)p_tbl;
        rc = tbl_lsp_p_add_lsp_p_sync(p_lsp_p, FALSE);
        break;

    case CDB_OPER_DEL:
        p_lsp_p_key = (tbl_lsp_p_key_t*)p_tbl;
        rc = tbl_lsp_p_del_lsp_p_sync(p_lsp_p_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_lsp_p = (tbl_lsp_p_t*)p_tbl;
        rc = tbl_lsp_p_set_lsp_p_field_sync(p_lsp_p, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pw_tpe(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pw_tpe_key_t *p_pw_tpe_key = NULL;
    tbl_pw_tpe_t *p_pw_tpe = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pw_tpe = (tbl_pw_tpe_t*)p_tbl;
        rc = tbl_pw_tpe_add_pw_tpe_sync(p_pw_tpe, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pw_tpe_key = (tbl_pw_tpe_key_t*)p_tbl;
        rc = tbl_pw_tpe_del_pw_tpe_sync(p_pw_tpe_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pw_tpe = (tbl_pw_tpe_t*)p_tbl;
        rc = tbl_pw_tpe_set_pw_tpe_field_sync(p_pw_tpe, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_section_oam(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_section_oam_key_t *p_section_oam_key = NULL;
    tbl_section_oam_t *p_section_oam = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_section_oam = (tbl_section_oam_t*)p_tbl;
        rc = tbl_section_oam_add_section_oam_sync(p_section_oam, FALSE);
        break;

    case CDB_OPER_DEL:
        p_section_oam_key = (tbl_section_oam_key_t*)p_tbl;
        rc = tbl_section_oam_del_section_oam_sync(p_section_oam_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_section_oam = (tbl_section_oam_t*)p_tbl;
        rc = tbl_section_oam_set_section_oam_field_sync(p_section_oam, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_oam_flow_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_flow_info_key_t *p_oam_flow_info_key = NULL;
    tbl_oam_flow_info_t *p_oam_flow_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_flow_info = (tbl_oam_flow_info_t*)p_tbl;
        rc = tbl_oam_flow_info_add_oam_flow_info_sync(p_oam_flow_info, FALSE);
        break;

    case CDB_OPER_DEL:
        p_oam_flow_info_key = (tbl_oam_flow_info_key_t*)p_tbl;
        rc = tbl_oam_flow_info_del_oam_flow_info_sync(p_oam_flow_info_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_oam_flow_info = (tbl_oam_flow_info_t*)p_tbl;
        rc = tbl_oam_flow_info_set_oam_flow_info_field_sync(p_oam_flow_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tpoam_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tpoam_global_t *p_tpoam_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_tpoam_global = (tbl_tpoam_global_t*)p_tbl;
        rc = tbl_tpoam_global_set_tpoam_global_field_sync(p_tpoam_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tpoam_exec_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tpoam_exec_global_t *p_tpoam_exec_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_tpoam_exec_global = (tbl_tpoam_exec_global_t*)p_tbl;
        rc = tbl_tpoam_exec_global_set_tpoam_exec_global_field_sync(p_tpoam_exec_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_aps_group_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_aps_group_info_key_t *p_aps_group_info_key = NULL;
    tbl_aps_group_info_t *p_aps_group_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_aps_group_info = (tbl_aps_group_info_t*)p_tbl;
        rc = tbl_aps_group_info_add_aps_group_info_sync(p_aps_group_info, FALSE);
        break;

    case CDB_OPER_DEL:
        p_aps_group_info_key = (tbl_aps_group_info_key_t*)p_tbl;
        rc = tbl_aps_group_info_del_aps_group_info_sync(p_aps_group_info_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_aps_group_info = (tbl_aps_group_info_t*)p_tbl;
        rc = tbl_aps_group_info_set_aps_group_info_field_sync(p_aps_group_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_oam_inlabel_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key = NULL;
    tbl_oam_inlabel_info_t *p_oam_inlabel_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_inlabel_info = (tbl_oam_inlabel_info_t*)p_tbl;
        rc = tbl_oam_inlabel_info_add_oam_inlabel_info_sync(p_oam_inlabel_info, FALSE);
        break;

    case CDB_OPER_DEL:
        p_oam_inlabel_info_key = (tbl_oam_inlabel_info_key_t*)p_tbl;
        rc = tbl_oam_inlabel_info_del_oam_inlabel_info_sync(p_oam_inlabel_info_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_oam_inlabel_info = (tbl_oam_inlabel_info_t*)p_tbl;
        rc = tbl_oam_inlabel_info_set_oam_inlabel_info_field_sync(p_oam_inlabel_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_oam_mpls_aps_check(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_oam_mpls_aps_check_key_t *p_oam_mpls_aps_check_key = NULL;
    tbl_oam_mpls_aps_check_t *p_oam_mpls_aps_check = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_oam_mpls_aps_check = (tbl_oam_mpls_aps_check_t*)p_tbl;
        rc = tbl_oam_mpls_aps_check_add_oam_mpls_aps_check_sync(p_oam_mpls_aps_check, FALSE);
        break;

    case CDB_OPER_DEL:
        p_oam_mpls_aps_check_key = (tbl_oam_mpls_aps_check_key_t*)p_tbl;
        rc = tbl_oam_mpls_aps_check_del_oam_mpls_aps_check_sync(p_oam_mpls_aps_check_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_oam_mpls_aps_check = (tbl_oam_mpls_aps_check_t*)p_tbl;
        rc = tbl_oam_mpls_aps_check_set_oam_mpls_aps_check_field_sync(p_oam_mpls_aps_check, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlan_mapping_table(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key = NULL;
    tbl_vlan_mapping_table_t *p_vlan_mapping_table = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan_mapping_table = (tbl_vlan_mapping_table_t*)p_tbl;
        rc = tbl_vlan_mapping_table_add_vlan_mapping_table_sync(p_vlan_mapping_table, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlan_mapping_table_key = (tbl_vlan_mapping_table_key_t*)p_tbl;
        rc = tbl_vlan_mapping_table_del_vlan_mapping_table_sync(p_vlan_mapping_table_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlan_mapping_table = (tbl_vlan_mapping_table_t*)p_tbl;
        rc = tbl_vlan_mapping_table_set_vlan_mapping_table_field_sync(p_vlan_mapping_table, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlan_mapping_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlan_mapping_entry_key_t *p_vlan_mapping_entry_key = NULL;
    tbl_vlan_mapping_entry_t *p_vlan_mapping_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan_mapping_entry = (tbl_vlan_mapping_entry_t*)p_tbl;
        rc = tbl_vlan_mapping_entry_add_vlan_mapping_entry_sync(p_vlan_mapping_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlan_mapping_entry_key = (tbl_vlan_mapping_entry_key_t*)p_tbl;
        rc = tbl_vlan_mapping_entry_del_vlan_mapping_entry_sync(p_vlan_mapping_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlan_mapping_entry = (tbl_vlan_mapping_entry_t*)p_tbl;
        rc = tbl_vlan_mapping_entry_set_vlan_mapping_entry_field_sync(p_vlan_mapping_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_egress_mapping_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key = NULL;
    tbl_egress_mapping_entry_t *p_egress_mapping_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_egress_mapping_entry = (tbl_egress_mapping_entry_t*)p_tbl;
        rc = tbl_egress_mapping_entry_add_egress_mapping_entry_sync(p_egress_mapping_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_egress_mapping_entry_key = (tbl_egress_mapping_entry_key_t*)p_tbl;
        rc = tbl_egress_mapping_entry_del_egress_mapping_entry_sync(p_egress_mapping_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_egress_mapping_entry = (tbl_egress_mapping_entry_t*)p_tbl;
        rc = tbl_egress_mapping_entry_set_egress_mapping_entry_field_sync(p_egress_mapping_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_vlan_group(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_vlan_group_key_t *p_vlan_group_key = NULL;
    tbl_vlan_group_t *p_vlan_group = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan_group = (tbl_vlan_group_t*)p_tbl;
        rc = tbl_vlan_group_add_vlan_group_sync(p_vlan_group, FALSE);
        break;

    case CDB_OPER_DEL:
        p_vlan_group_key = (tbl_vlan_group_key_t*)p_tbl;
        rc = tbl_vlan_group_del_vlan_group_sync(p_vlan_group_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_vlan_group = (tbl_vlan_group_t*)p_tbl;
        rc = tbl_vlan_group_set_vlan_group_field_sync(p_vlan_group, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_evc_entry(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_evc_entry_key_t *p_evc_entry_key = NULL;
    tbl_evc_entry_t *p_evc_entry = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_evc_entry = (tbl_evc_entry_t*)p_tbl;
        rc = tbl_evc_entry_add_evc_entry_sync(p_evc_entry, FALSE);
        break;

    case CDB_OPER_DEL:
        p_evc_entry_key = (tbl_evc_entry_key_t*)p_tbl;
        rc = tbl_evc_entry_del_evc_entry_sync(p_evc_entry_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_evc_entry = (tbl_evc_entry_t*)p_tbl;
        rc = tbl_evc_entry_set_evc_entry_field_sync(p_evc_entry, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_mapped_vlan_info(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_mapped_vlan_info_key_t *p_mapped_vlan_info_key = NULL;
    tbl_mapped_vlan_info_t *p_mapped_vlan_info = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mapped_vlan_info = (tbl_mapped_vlan_info_t*)p_tbl;
        rc = tbl_mapped_vlan_info_add_mapped_vlan_info_sync(p_mapped_vlan_info, FALSE);
        break;

    case CDB_OPER_DEL:
        p_mapped_vlan_info_key = (tbl_mapped_vlan_info_key_t*)p_tbl;
        rc = tbl_mapped_vlan_info_del_mapped_vlan_info_sync(p_mapped_vlan_info_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_mapped_vlan_info = (tbl_mapped_vlan_info_t*)p_tbl;
        rc = tbl_mapped_vlan_info_set_mapped_vlan_info_field_sync(p_mapped_vlan_info, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_l2_protocol(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_l2_protocol_key_t *p_l2_protocol_key = NULL;
    tbl_l2_protocol_t *p_l2_protocol = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_l2_protocol = (tbl_l2_protocol_t*)p_tbl;
        rc = tbl_l2_protocol_add_l2_protocol_sync(p_l2_protocol, FALSE);
        break;

    case CDB_OPER_DEL:
        p_l2_protocol_key = (tbl_l2_protocol_key_t*)p_tbl;
        rc = tbl_l2_protocol_del_l2_protocol_sync(p_l2_protocol_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_l2_protocol = (tbl_l2_protocol_t*)p_tbl;
        rc = tbl_l2_protocol_set_l2_protocol_field_sync(p_l2_protocol, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nd_fib(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nd_fib_key_t *p_nd_fib_key = NULL;
    tbl_nd_fib_t *p_nd_fib = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nd_fib = (tbl_nd_fib_t*)p_tbl;
        rc = tbl_nd_fib_add_nd_fib_sync(p_nd_fib, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nd_fib_key = (tbl_nd_fib_key_t*)p_tbl;
        rc = tbl_nd_fib_del_nd_fib_sync(p_nd_fib_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nd_fib = (tbl_nd_fib_t*)p_tbl;
        rc = tbl_nd_fib_set_nd_fib_field_sync(p_nd_fib, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nd_stats(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nd_stats_t *p_nd_stats = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_nd_stats = (tbl_nd_stats_t*)p_tbl;
        rc = tbl_nd_stats_set_nd_stats_field_sync(p_nd_stats, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cpu_mirror_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cpu_mirror_global_t *p_cpu_mirror = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cpu_mirror = (tbl_cpu_mirror_global_t*)p_tbl;
        rc = tbl_cpu_mirror_global_set_cpu_mirror_global_field_sync(p_cpu_mirror, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_tap_server(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_tap_server_t *p_tap_srv = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_tap_srv = (tbl_tap_server_t*)p_tbl;
        rc = tbl_tap_server_set_tap_server_field_sync(p_tap_srv, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_dhcpv6_debug(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_dhcpv6_debug_t *p_dhcpv6_debug = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_dhcpv6_debug = (tbl_dhcpv6_debug_t*)p_tbl;
        rc = tbl_dhcpv6_debug_set_dhcpv6_debug_field_sync(p_dhcpv6_debug, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_l2edit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_l2edit_key_t *p_l2edit_key = NULL;
    tbl_l2edit_t *p_l2edit = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_l2edit = (tbl_l2edit_t*)p_tbl;
        rc = tbl_l2edit_add_l2edit_sync(p_l2edit, FALSE);
        break;

    case CDB_OPER_DEL:
        p_l2edit_key = (tbl_l2edit_key_t*)p_tbl;
        rc = tbl_l2edit_del_l2edit_sync(p_l2edit_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_l2edit = (tbl_l2edit_t*)p_tbl;
        rc = tbl_l2edit_set_l2edit_field_sync(p_l2edit, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_hash_field_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_hash_field_profile_key_t *p_hash_field_profile_key = NULL;
    tbl_hash_field_profile_t *p_hash_field_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_hash_field_profile = (tbl_hash_field_profile_t*)p_tbl;
        rc = tbl_hash_field_profile_add_hash_field_profile_sync(p_hash_field_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_hash_field_profile_key = (tbl_hash_field_profile_key_t*)p_tbl;
        rc = tbl_hash_field_profile_del_hash_field_profile_sync(p_hash_field_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_hash_field_profile = (tbl_hash_field_profile_t*)p_tbl;
        rc = tbl_hash_field_profile_set_hash_field_profile_field_sync(p_hash_field_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_hash_value_profile(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_hash_value_profile_key_t *p_hash_value_profile_key = NULL;
    tbl_hash_value_profile_t *p_hash_value_profile = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_hash_value_profile = (tbl_hash_value_profile_t*)p_tbl;
        rc = tbl_hash_value_profile_add_hash_value_profile_sync(p_hash_value_profile, FALSE);
        break;

    case CDB_OPER_DEL:
        p_hash_value_profile_key = (tbl_hash_value_profile_key_t*)p_tbl;
        rc = tbl_hash_value_profile_del_hash_value_profile_sync(p_hash_value_profile_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_hash_value_profile = (tbl_hash_value_profile_t*)p_tbl;
        rc = tbl_hash_value_profile_set_hash_value_profile_field_sync(p_hash_value_profile, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_hash_value_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_hash_value_global_t *p_hash_value_global = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_hash_value_global = (tbl_hash_value_global_t*)p_tbl;
        rc = tbl_hash_value_global_set_hash_value_global_field_sync(p_hash_value_global, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_global_t *p_nat_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_nat_glb = (tbl_nat_global_t*)p_tbl;
        rc = tbl_nat_global_set_nat_global_field_sync(p_nat_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_pool(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_pool_key_t *p_nat_pool_key = NULL;
    tbl_nat_pool_t *p_nat_pool = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_pool = (tbl_nat_pool_t*)p_tbl;
        rc = tbl_nat_pool_add_nat_pool_sync(p_nat_pool, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_pool_key = (tbl_nat_pool_key_t*)p_tbl;
        rc = tbl_nat_pool_del_nat_pool_sync(p_nat_pool_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_pool = (tbl_nat_pool_t*)p_tbl;
        rc = tbl_nat_pool_set_nat_pool_field_sync(p_nat_pool, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_rule(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_rule_key_t *p_nat_rule_key = NULL;
    tbl_nat_rule_t *p_nat_rule = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_rule = (tbl_nat_rule_t*)p_tbl;
        rc = tbl_nat_rule_add_nat_rule_sync(p_nat_rule, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_rule_key = (tbl_nat_rule_key_t*)p_tbl;
        rc = tbl_nat_rule_del_nat_rule_sync(p_nat_rule_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_rule = (tbl_nat_rule_t*)p_tbl;
        rc = tbl_nat_rule_set_nat_rule_field_sync(p_nat_rule, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_session(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_session_key_t *p_nat_session_key = NULL;
    tbl_nat_session_t *p_nat_session = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_session = (tbl_nat_session_t*)p_tbl;
        rc = tbl_nat_session_add_nat_session_sync(p_nat_session, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_session_key = (tbl_nat_session_key_t*)p_tbl;
        rc = tbl_nat_session_del_nat_session_sync(p_nat_session_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_session = (tbl_nat_session_t*)p_tbl;
        rc = tbl_nat_session_set_nat_session_field_sync(p_nat_session, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_session_limit(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_session_limit_key_t *p_nat_session_limit_key = NULL;
    tbl_nat_session_limit_t *p_nat_session_limit = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_session_limit = (tbl_nat_session_limit_t*)p_tbl;
        rc = tbl_nat_session_limit_add_nat_session_limit_sync(p_nat_session_limit, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_session_limit_key = (tbl_nat_session_limit_key_t*)p_tbl;
        rc = tbl_nat_session_limit_del_nat_session_limit_sync(p_nat_session_limit_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_session_limit = (tbl_nat_session_limit_t*)p_tbl;
        rc = tbl_nat_session_limit_set_nat_session_limit_field_sync(p_nat_session_limit, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_proxy_arp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_proxy_arp_key_t *p_nat_proxy_arp_key = NULL;
    tbl_nat_proxy_arp_t *p_nat_proxy_arp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_proxy_arp = (tbl_nat_proxy_arp_t*)p_tbl;
        rc = tbl_nat_proxy_arp_add_nat_proxy_arp_sync(p_nat_proxy_arp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_proxy_arp_key = (tbl_nat_proxy_arp_key_t*)p_tbl;
        rc = tbl_nat_proxy_arp_del_nat_proxy_arp_sync(p_nat_proxy_arp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_proxy_arp = (tbl_nat_proxy_arp_t*)p_tbl;
        rc = tbl_nat_proxy_arp_set_nat_proxy_arp_field_sync(p_nat_proxy_arp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_rtif_inside(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_rtif_inside_key_t *p_nat_rtif_inside_key = NULL;
    tbl_nat_rtif_inside_t *p_nat_rtif_inside = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_rtif_inside = (tbl_nat_rtif_inside_t*)p_tbl;
        rc = tbl_nat_rtif_inside_add_nat_rtif_inside_sync(p_nat_rtif_inside, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_rtif_inside_key = (tbl_nat_rtif_inside_key_t*)p_tbl;
        rc = tbl_nat_rtif_inside_del_nat_rtif_inside_sync(p_nat_rtif_inside_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_rtif_inside = (tbl_nat_rtif_inside_t*)p_tbl;
        rc = tbl_nat_rtif_inside_set_nat_rtif_inside_field_sync(p_nat_rtif_inside, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_nat_rtif_outside(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_nat_rtif_outside_key_t *p_nat_rtif_outside_key = NULL;
    tbl_nat_rtif_outside_t *p_nat_rtif_outside = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_nat_rtif_outside = (tbl_nat_rtif_outside_t*)p_tbl;
        rc = tbl_nat_rtif_outside_add_nat_rtif_outside_sync(p_nat_rtif_outside, FALSE);
        break;

    case CDB_OPER_DEL:
        p_nat_rtif_outside_key = (tbl_nat_rtif_outside_key_t*)p_tbl;
        rc = tbl_nat_rtif_outside_del_nat_rtif_outside_sync(p_nat_rtif_outside_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_nat_rtif_outside = (tbl_nat_rtif_outside_t*)p_tbl;
        rc = tbl_nat_rtif_outside_set_nat_rtif_outside_field_sync(p_nat_rtif_outside, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_g8032_ring(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_g8032_ring_key_t *p_g8032_ring_key = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_g8032_ring = (tbl_g8032_ring_t*)p_tbl;
        rc = tbl_g8032_ring_add_g8032_ring_sync(p_g8032_ring, FALSE);
        break;

    case CDB_OPER_DEL:
        p_g8032_ring_key = (tbl_g8032_ring_key_t*)p_tbl;
        rc = tbl_g8032_ring_del_g8032_ring_sync(p_g8032_ring_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_g8032_ring = (tbl_g8032_ring_t*)p_tbl;
        rc = tbl_g8032_ring_set_g8032_ring_field_sync(p_g8032_ring, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_global(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_global_t *p_cfm_glb = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cfm_glb = (tbl_cfm_global_t*)p_tbl;
        rc = tbl_cfm_global_set_cfm_global_field_sync(p_cfm_glb, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_md(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_md_key_t *p_md_key = NULL;
    tbl_cfm_md_t *p_md = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_md = (tbl_cfm_md_t*)p_tbl;
        rc = tbl_cfm_md_add_cfm_md_sync(p_md, FALSE);
        break;

    case CDB_OPER_DEL:
        p_md_key = (tbl_cfm_md_key_t*)p_tbl;
        rc = tbl_cfm_md_del_cfm_md_sync(p_md_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_md = (tbl_cfm_md_t*)p_tbl;
        rc = tbl_cfm_md_set_cfm_md_field_sync(p_md, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_ma(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_ma_key_t *p_ma_key = NULL;
    tbl_cfm_ma_t *p_ma = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ma = (tbl_cfm_ma_t*)p_tbl;
        rc = tbl_cfm_ma_add_cfm_ma_sync(p_ma, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ma_key = (tbl_cfm_ma_key_t*)p_tbl;
        rc = tbl_cfm_ma_del_cfm_ma_sync(p_ma_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ma = (tbl_cfm_ma_t*)p_tbl;
        rc = tbl_cfm_ma_set_cfm_ma_field_sync(p_ma, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_maid(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_maid_key_t *p_maid_key = NULL;
    tbl_cfm_maid_t *p_maid = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_maid = (tbl_cfm_maid_t*)p_tbl;
        rc = tbl_cfm_maid_add_cfm_maid_sync(p_maid, FALSE);
        break;

    case CDB_OPER_DEL:
        p_maid_key = (tbl_cfm_maid_key_t*)p_tbl;
        rc = tbl_cfm_maid_del_cfm_maid_sync(p_maid_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_maid = (tbl_cfm_maid_t*)p_tbl;
        rc = tbl_cfm_maid_set_cfm_maid_field_sync(p_maid, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_lmep(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_lmep_key_t *p_lmep_key = NULL;
    tbl_cfm_lmep_t *p_lmep = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_lmep = (tbl_cfm_lmep_t*)p_tbl;
        rc = tbl_cfm_lmep_add_cfm_lmep_sync(p_lmep, FALSE);
        break;

    case CDB_OPER_DEL:
        p_lmep_key = (tbl_cfm_lmep_key_t*)p_tbl;
        rc = tbl_cfm_lmep_del_cfm_lmep_sync(p_lmep_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_lmep = (tbl_cfm_lmep_t*)p_tbl;
        rc = tbl_cfm_lmep_set_cfm_lmep_field_sync(p_lmep, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_cfm_rmep(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_cfm_rmep_key_t *p_rmep_key = NULL;
    tbl_cfm_rmep_t *p_rmep = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rmep = (tbl_cfm_rmep_t*)p_tbl;
        rc = tbl_cfm_rmep_add_cfm_rmep_sync(p_rmep, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rmep_key = (tbl_cfm_rmep_key_t*)p_tbl;
        rc = tbl_cfm_rmep_del_cfm_rmep_sync(p_rmep_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rmep = (tbl_cfm_rmep_t*)p_tbl;
        rc = tbl_cfm_rmep_set_cfm_rmep_field_sync(p_rmep, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_sm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_sm_key_t *p_pim_sm_key = NULL;
    tbl_pim_sm_t *p_pim_sm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_sm = (tbl_pim_sm_t*)p_tbl;
        rc = tbl_pim_sm_add_pim_sm_sync(p_pim_sm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_sm_key = (tbl_pim_sm_key_t*)p_tbl;
        rc = tbl_pim_sm_del_pim_sm_sync(p_pim_sm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_sm = (tbl_pim_sm_t*)p_tbl;
        rc = tbl_pim_sm_set_pim_sm_field_sync(p_pim_sm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_sm_bsr(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_sm_bsr_key_t *p_pim_sm_bsr_key = NULL;
    tbl_pim_sm_bsr_t *p_pim_sm_bsr = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_sm_bsr = (tbl_pim_sm_bsr_t*)p_tbl;
        rc = tbl_pim_sm_bsr_add_pim_sm_bsr_sync(p_pim_sm_bsr, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_sm_bsr_key = (tbl_pim_sm_bsr_key_t*)p_tbl;
        rc = tbl_pim_sm_bsr_del_pim_sm_bsr_sync(p_pim_sm_bsr_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_sm_bsr = (tbl_pim_sm_bsr_t*)p_tbl;
        rc = tbl_pim_sm_bsr_set_pim_sm_bsr_field_sync(p_pim_sm_bsr, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_sm_rp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_sm_rp_key_t *p_pim_sm_rp_key = NULL;
    tbl_pim_sm_rp_t *p_pim_sm_rp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_sm_rp = (tbl_pim_sm_rp_t*)p_tbl;
        rc = tbl_pim_sm_rp_add_pim_sm_rp_sync(p_pim_sm_rp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_sm_rp_key = (tbl_pim_sm_rp_key_t*)p_tbl;
        rc = tbl_pim_sm_rp_del_pim_sm_rp_sync(p_pim_sm_rp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_sm_rp = (tbl_pim_sm_rp_t*)p_tbl;
        rc = tbl_pim_sm_rp_set_pim_sm_rp_field_sync(p_pim_sm_rp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_sm_rp_gp(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_sm_rp_gp_key_t *p_pim_sm_rp_gp_key = NULL;
    tbl_pim_sm_rp_gp_t *p_pim_sm_rp_gp = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_sm_rp_gp = (tbl_pim_sm_rp_gp_t*)p_tbl;
        rc = tbl_pim_sm_rp_gp_add_pim_sm_rp_gp_sync(p_pim_sm_rp_gp, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_sm_rp_gp_key = (tbl_pim_sm_rp_gp_key_t*)p_tbl;
        rc = tbl_pim_sm_rp_gp_del_pim_sm_rp_gp_sync(p_pim_sm_rp_gp_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_sm_rp_gp = (tbl_pim_sm_rp_gp_t*)p_tbl;
        rc = tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field_sync(p_pim_sm_rp_gp, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_pim_dm(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_pim_dm_key_t *p_pim_dm_key = NULL;
    tbl_pim_dm_t *p_pim_dm = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_pim_dm = (tbl_pim_dm_t*)p_tbl;
        rc = tbl_pim_dm_add_pim_dm_sync(p_pim_dm, FALSE);
        break;

    case CDB_OPER_DEL:
        p_pim_dm_key = (tbl_pim_dm_key_t*)p_tbl;
        rc = tbl_pim_dm_del_pim_dm_sync(p_pim_dm_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_pim_dm = (tbl_pim_dm_t*)p_tbl;
        rc = tbl_pim_dm_set_pim_dm_field_sync(p_pim_dm, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_igmp_intf(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_igmp_intf_key_t *p_igmp_intf_key = NULL;
    tbl_igmp_intf_t *p_igmp_intf = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_igmp_intf = (tbl_igmp_intf_t*)p_tbl;
        rc = tbl_igmp_intf_add_igmp_intf_sync(p_igmp_intf, FALSE);
        break;

    case CDB_OPER_DEL:
        p_igmp_intf_key = (tbl_igmp_intf_key_t*)p_tbl;
        rc = tbl_igmp_intf_del_igmp_intf_sync(p_igmp_intf_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_igmp_intf = (tbl_igmp_intf_t*)p_tbl;
        rc = tbl_igmp_intf_set_igmp_intf_field_sync(p_igmp_intf, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rip(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rip_key_t *p_rip_key = NULL;
    tbl_rip_t *p_rip = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rip = (tbl_rip_t*)p_tbl;
        rc = tbl_rip_add_rip_sync(p_rip, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rip_key = (tbl_rip_key_t*)p_tbl;
        rc = tbl_rip_del_rip_sync(p_rip_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rip = (tbl_rip_t*)p_tbl;
        rc = tbl_rip_set_rip_field_sync(p_rip, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ripng(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ripng_key_t *p_ripng_key = NULL;
    tbl_ripng_t *p_ripng = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ripng = (tbl_ripng_t*)p_tbl;
        rc = tbl_ripng_add_ripng_sync(p_ripng, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ripng_key = (tbl_ripng_key_t*)p_tbl;
        rc = tbl_ripng_del_ripng_sync(p_ripng_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ripng = (tbl_ripng_t*)p_tbl;
        rc = tbl_ripng_set_ripng_field_sync(p_ripng, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rip_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rip_network_key_t *p_rip_network_key = NULL;
    tbl_rip_network_t *p_rip_network = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rip_network = (tbl_rip_network_t*)p_tbl;
        rc = tbl_rip_network_add_rip_network_sync(p_rip_network, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rip_network_key = (tbl_rip_network_key_t*)p_tbl;
        rc = tbl_rip_network_del_rip_network_sync(p_rip_network_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rip_network = (tbl_rip_network_t*)p_tbl;
        rc = tbl_rip_network_set_rip_network_field_sync(p_rip_network, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ripng_network(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ripng_network_key_t *p_ripng_network_key = NULL;
    tbl_ripng_network_t *p_ripng_network = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ripng_network = (tbl_ripng_network_t*)p_tbl;
        rc = tbl_ripng_network_add_ripng_network_sync(p_ripng_network, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ripng_network_key = (tbl_ripng_network_key_t*)p_tbl;
        rc = tbl_ripng_network_del_ripng_network_sync(p_ripng_network_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ripng_network = (tbl_ripng_network_t*)p_tbl;
        rc = tbl_ripng_network_set_ripng_network_field_sync(p_ripng_network, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rip_neighbor(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rip_neighbor_key_t *p_rip_neighbor_key = NULL;
    tbl_rip_neighbor_t *p_rip_neighbor = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rip_neighbor = (tbl_rip_neighbor_t*)p_tbl;
        rc = tbl_rip_neighbor_add_rip_neighbor_sync(p_rip_neighbor, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rip_neighbor_key = (tbl_rip_neighbor_key_t*)p_tbl;
        rc = tbl_rip_neighbor_del_rip_neighbor_sync(p_rip_neighbor_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rip_neighbor = (tbl_rip_neighbor_t*)p_tbl;
        rc = tbl_rip_neighbor_set_rip_neighbor_field_sync(p_rip_neighbor, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_rip_passive_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_rip_passive_if_key_t *p_rip_passive_if_key = NULL;
    tbl_rip_passive_if_t *p_rip_passive_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_rip_passive_if = (tbl_rip_passive_if_t*)p_tbl;
        rc = tbl_rip_passive_if_add_rip_passive_if_sync(p_rip_passive_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_rip_passive_if_key = (tbl_rip_passive_if_key_t*)p_tbl;
        rc = tbl_rip_passive_if_del_rip_passive_if_sync(p_rip_passive_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_rip_passive_if = (tbl_rip_passive_if_t*)p_tbl;
        rc = tbl_rip_passive_if_set_rip_passive_if_field_sync(p_rip_passive_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ripng_passive_if(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ripng_passive_if_key_t *p_ripng_passive_if_key = NULL;
    tbl_ripng_passive_if_t *p_ripng_passive_if = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ripng_passive_if = (tbl_ripng_passive_if_t*)p_tbl;
        rc = tbl_ripng_passive_if_add_ripng_passive_if_sync(p_ripng_passive_if, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ripng_passive_if_key = (tbl_ripng_passive_if_key_t*)p_tbl;
        rc = tbl_ripng_passive_if_del_ripng_passive_if_sync(p_ripng_passive_if_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ripng_passive_if = (tbl_ripng_passive_if_t*)p_tbl;
        rc = tbl_ripng_passive_if_set_ripng_passive_if_field_sync(p_ripng_passive_if, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_area_stub(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_area_stub_key_t *p_ospf_area_stub_key = NULL;
    tbl_ospf_area_stub_t *p_ospf_area_stub = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_area_stub = (tbl_ospf_area_stub_t*)p_tbl;
        rc = tbl_ospf_area_stub_add_ospf_area_stub_sync(p_ospf_area_stub, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_area_stub_key = (tbl_ospf_area_stub_key_t*)p_tbl;
        rc = tbl_ospf_area_stub_del_ospf_area_stub_sync(p_ospf_area_stub_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_area_stub = (tbl_ospf_area_stub_t*)p_tbl;
        rc = tbl_ospf_area_stub_set_ospf_area_stub_field_sync(p_ospf_area_stub, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf6_area_stub(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf6_area_stub_key_t *p_ospf6_area_stub_key = NULL;
    tbl_ospf6_area_stub_t *p_ospf6_area_stub = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf6_area_stub = (tbl_ospf6_area_stub_t*)p_tbl;
        rc = tbl_ospf6_area_stub_add_ospf6_area_stub_sync(p_ospf6_area_stub, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf6_area_stub_key = (tbl_ospf6_area_stub_key_t*)p_tbl;
        rc = tbl_ospf6_area_stub_del_ospf6_area_stub_sync(p_ospf6_area_stub_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf6_area_stub = (tbl_ospf6_area_stub_t*)p_tbl;
        rc = tbl_ospf6_area_stub_set_ospf6_area_stub_field_sync(p_ospf6_area_stub, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_area_nssa(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_area_nssa_key_t *p_ospf_area_nssa_key = NULL;
    tbl_ospf_area_nssa_t *p_ospf_area_nssa = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_area_nssa = (tbl_ospf_area_nssa_t*)p_tbl;
        rc = tbl_ospf_area_nssa_add_ospf_area_nssa_sync(p_ospf_area_nssa, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_area_nssa_key = (tbl_ospf_area_nssa_key_t*)p_tbl;
        rc = tbl_ospf_area_nssa_del_ospf_area_nssa_sync(p_ospf_area_nssa_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_area_nssa = (tbl_ospf_area_nssa_t*)p_tbl;
        rc = tbl_ospf_area_nssa_set_ospf_area_nssa_field_sync(p_ospf_area_nssa, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_cdb_store_tbl_ospf_area_vlink(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key = NULL;
    tbl_ospf_area_vlink_t *p_ospf_area_vlink = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_ospf_area_vlink = (tbl_ospf_area_vlink_t*)p_tbl;
        rc = tbl_ospf_area_vlink_add_ospf_area_vlink_sync(p_ospf_area_vlink, FALSE);
        break;

    case CDB_OPER_DEL:
        p_ospf_area_vlink_key = (tbl_ospf_area_vlink_key_t*)p_tbl;
        rc = tbl_ospf_area_vlink_del_ospf_area_vlink_sync(p_ospf_area_vlink_key, FALSE);
        break;

    case CDB_OPER_SET:
        p_ospf_area_vlink = (tbl_ospf_area_vlink_t*)p_tbl;
        rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field_sync(p_ospf_area_vlink, fields[0], FALSE);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

CDB_STORE_TBL_FUNC g_cdb_store_tbl_func[TBL_MAX] = 
{
    _cdb_store_tbl_interface,
    _cdb_store_tbl_route_if,
    _cdb_store_tbl_kernel_if,
    _cdb_store_tbl_fea_port_if,
    _cdb_store_tbl_vlan,
    _cdb_store_tbl_pvlan,
    _cdb_store_tbl_fdb,
    _cdb_store_tbl_mcfdb,
    _cdb_store_tbl_macfilter,
    _cdb_store_tbl_psfdb,
    _cdb_store_tbl_ipsg_s_ip,
    _cdb_store_tbl_ipsg_s_mac,
    _cdb_store_tbl_ipsg_fib,
    _cdb_store_tbl_brg_global,
    _cdb_store_tbl_mstp_port,
    _cdb_store_tbl_msti_port,
    _cdb_store_tbl_mstp_instance,
    _cdb_store_tbl_mstp_global,
    _cdb_store_tbl_lldp_global,
    _cdb_store_tbl_lldp_if,
    _cdb_store_tbl_mlag,
    _cdb_store_tbl_mlag_peer,
    _cdb_store_tbl_mlag_port,
    _cdb_store_tbl_isolation,
    _cdb_store_tbl_route_global,
    _cdb_store_tbl_bgp,
    _cdb_store_tbl_bgp_network,
    _cdb_store_tbl_bgp_neighbor,
    _cdb_store_tbl_ospf,
    _cdb_store_tbl_ospf6,
    _cdb_store_tbl_ospf6_interface,
    _cdb_store_tbl_ospf_network,
    _cdb_store_tbl_ospf_area_auth,
    _cdb_store_tbl_ospf_area_range,
    _cdb_store_tbl_ospf6_area_range,
    _cdb_store_tbl_iproute_node,
    _cdb_store_tbl_iproute_node_add_fail,
    _cdb_store_tbl_iproute_node_add_fail_count,
    _cdb_store_tbl_static_route_cfg,
    _cdb_store_tbl_static_rt_cnt,
    _cdb_store_tbl_arp_fib,
    _cdb_store_tbl_arp,
    _cdb_store_tbl_nexthop,
    _cdb_store_tbl_nexthop_group,
    _cdb_store_tbl_nexthop_group_routed,
    _cdb_store_tbl_fea_nexthop,
    _cdb_store_tbl_mrt,
    _cdb_store_tbl_ipmc_global,
    _cdb_store_tbl_ipmc_intf,
    _cdb_store_tbl_pim_static_rp,
    _cdb_store_tbl_sys_global,
    _cdb_store_tbl_lag_global,
    _cdb_store_tbl_ecmp_global,
    _cdb_store_tbl_rr_prefix,
    _cdb_store_tbl_mem_summary,
    _cdb_store_tbl_cpu_mem_history,
    _cdb_store_tbl_chsm_debug,
    _cdb_store_tbl_switch_debug,
    _cdb_store_tbl_route_debug,
    _cdb_store_tbl_ssm_debug,
    _cdb_store_tbl_ptp_debug,
    _cdb_store_tbl_quagga_debug,
    _cdb_store_tbl_lsrv_debug,
    _cdb_store_tbl_hsrv_debug,
    _cdb_store_tbl_app_debug,
    _cdb_store_tbl_openflow_debug,
    _cdb_store_tbl_rif,
    _cdb_store_tbl_fea_lag,
    _cdb_store_tbl_fea_global,
    _cdb_store_tbl_fea_acl_table,
    _cdb_store_tbl_fea_acl,
    _cdb_store_tbl_fea_fdb,
    _cdb_store_tbl_fea_brg_if,
    _cdb_store_tbl_acl_worm_filter,
    _cdb_store_tbl_acl_config,
    _cdb_store_tbl_ace_config,
    _cdb_store_tbl_acl_entry,
    _cdb_store_tbl_acl_entry_action,
    _cdb_store_tbl_acl_nexthop_group,
    _cdb_store_tbl_acl_nexthop,
    _cdb_store_tbl_time_range,
    _cdb_store_tbl_ssh_cfg,
    _cdb_store_tbl_snmp_cfg,
    _cdb_store_tbl_snmp_view,
    _cdb_store_tbl_snmp_community,
    _cdb_store_tbl_snmp_context,
    _cdb_store_tbl_snmp_usm_user,
    _cdb_store_tbl_snmp_group_usm,
    _cdb_store_tbl_snmp_access_usm,
    _cdb_store_tbl_snmp_notify,
    _cdb_store_tbl_snmp_target_addr,
    _cdb_store_tbl_snmp_target_params,
    _cdb_store_tbl_snmp_trap,
    _cdb_store_tbl_snmp_inform,
    _cdb_store_tbl_snmp_trap_msg,
    _cdb_store_tbl_rmon_event,
    _cdb_store_tbl_rmon_alarm,
    _cdb_store_tbl_rmon_ether_stats,
    _cdb_store_tbl_rmon_history,
    _cdb_store_tbl_rmon_stats_history,
    _cdb_store_tbl_syslog_cfg,
    _cdb_store_tbl_ntp_server,
    _cdb_store_tbl_ntp_ace,
    _cdb_store_tbl_ntp_key,
    _cdb_store_tbl_ntp_cfg,
    _cdb_store_tbl_ntp_if,
    _cdb_store_tbl_ntp_syncstatus,
    _cdb_store_tbl_static_dns,
    _cdb_store_tbl_dynamic_dns_domain,
    _cdb_store_tbl_dynamic_dns_server,
    _cdb_store_tbl_qos_domain,
    _cdb_store_tbl_qos_mpls_domain,
    _cdb_store_tbl_qos_policer_res,
    _cdb_store_tbl_qos_policer_action_profile,
    _cdb_store_tbl_qos_policer_profile,
    _cdb_store_tbl_qos_drop_profile,
    _cdb_store_tbl_qos_queue_shape_profile,
    _cdb_store_tbl_qos_port_shape_profile,
    _cdb_store_tbl_qos_queue_smart_buffer,
    _cdb_store_tbl_qos_global,
    _cdb_store_tbl_mirror,
    _cdb_store_tbl_mirror_mac_escape,
    _cdb_store_tbl_tap_group_ingress,
    _cdb_store_tbl_tap_group_ingress_flow,
    _cdb_store_tbl_tap_group_egress,
    _cdb_store_tbl_tap_group,
    _cdb_store_tbl_tap_five_tuple,
    _cdb_store_tbl_tap_dpi_global,
    _cdb_store_tbl_user,
    _cdb_store_tbl_login_record,
    _cdb_store_tbl_login_security_glb,
    _cdb_store_tbl_vty,
    _cdb_store_tbl_console,
    _cdb_store_tbl_authen,
    _cdb_store_tbl_login,
    _cdb_store_tbl_rsa,
    _cdb_store_tbl_openflow,
    _cdb_store_tbl_cpu_traffic,
    _cdb_store_tbl_cpu_traffic_group,
    _cdb_store_tbl_cpu_utilization,
    _cdb_store_tbl_cpu_limit,
    _cdb_store_tbl_dhcrelay,
    _cdb_store_tbl_dhcsrvgrp,
    _cdb_store_tbl_dhcp_debug,
    _cdb_store_tbl_dhclient,
    _cdb_store_tbl_dhcsnooping,
    _cdb_store_tbl_dhcbinding,
    _cdb_store_tbl_iptables_prevent,
    _cdb_store_tbl_errdisable,
    _cdb_store_tbl_ns_port_forwarding,
    _cdb_store_tbl_log_global,
    _cdb_store_tbl_log,
    _cdb_store_tbl_sys_load,
    _cdb_store_tbl_cem,
    _cdb_store_tbl_clock,
    _cdb_store_tbl_tag_summer_clock,
    _cdb_store_tbl_port_stats,
    _cdb_store_tbl_vlan_stats,
    _cdb_store_tbl_port_stats_rate,
    _cdb_store_tbl_aclqos_if,
    _cdb_store_tbl_l2_action,
    _cdb_store_tbl_fea_qos_drop_profile,
    _cdb_store_tbl_fea_qos_domain,
    _cdb_store_tbl_fea_g8032_ring,
    _cdb_store_tbl_fea_g8032_ring_mc_group,
    _cdb_store_tbl_fea_qos_queue_shape_profile,
    _cdb_store_tbl_fea_qos_port_shape_profile,
    _cdb_store_tbl_fea_port_policer_apply,
    _cdb_store_tbl_aclqos_if_stats,
    _cdb_store_tbl_version,
    _cdb_store_tbl_manage_if,
    _cdb_store_tbl_bootimage,
    _cdb_store_tbl_chassis,
    _cdb_store_tbl_ifname_info,
    _cdb_store_tbl_card,
    _cdb_store_tbl_port,
    _cdb_store_tbl_fiber,
    _cdb_store_tbl_sys_spec,
    _cdb_store_tbl_fan,
    _cdb_store_tbl_psu,
    _cdb_store_tbl_led,
    _cdb_store_tbl_sensor,
    _cdb_store_tbl_vcm,
    _cdb_store_tbl_poe,
    _cdb_store_tbl_reboot_info,
    _cdb_store_tbl_errdisable_flap,
    _cdb_store_tbl_opm_global,
    _cdb_store_tbl_erps_ring,
    _cdb_store_tbl_erps_domain,
    _cdb_store_tbl_opm_debug,
    _cdb_store_tbl_policy_map_config,
    _cdb_store_tbl_class_map_config,
    _cdb_store_tbl_class_in_policy_config,
    _cdb_store_tbl_acl_in_class_config,
    _cdb_store_tbl_class_map_action_config,
    _cdb_store_tbl_fea_acl_policy_action,
    _cdb_store_tbl_igsp_global,
    _cdb_store_tbl_igsp_intf,
    _cdb_store_tbl_igsp_group,
    _cdb_store_tbl_auth_cfg,
    _cdb_store_tbl_auth_server,
    _cdb_store_tbl_auth_session,
    _cdb_store_tbl_authd_debug,
    _cdb_store_tbl_dot1x_global,
    _cdb_store_tbl_dot1x_port,
    _cdb_store_tbl_dot1x_radius,
    _cdb_store_tbl_dot1x_mac,
    _cdb_store_tbl_enable,
    _cdb_store_tbl_chip,
    _cdb_store_tbl_clear_acl_policy,
    _cdb_store_tbl_author,
    _cdb_store_tbl_account,
    _cdb_store_tbl_accountcmd,
    _cdb_store_tbl_vlanclass_rule,
    _cdb_store_tbl_vlanclass_group,
    _cdb_store_tbl_acl_l4_port_range,
    _cdb_store_tbl_acl_udf_group,
    _cdb_store_tbl_acl_udf_entry,
    _cdb_store_tbl_fea_pcap,
    _cdb_store_tbl_controller,
    _cdb_store_tbl_cpu_packets,
    _cdb_store_tbl_ns_route,
    _cdb_store_tbl_ns_route_ip,
    _cdb_store_tbl_openflow_interface,
    _cdb_store_tbl_openflow_manager,
    _cdb_store_tbl_ptp_global,
    _cdb_store_tbl_ptp_port,
    _cdb_store_tbl_ptp_foreign,
    _cdb_store_tbl_ptp_residence_time_cache,
    _cdb_store_tbl_fea_ptp_global,
    _cdb_store_tbl_fea_ptp_ts,
    _cdb_store_tbl_fea_time,
    _cdb_store_tbl_bhm_global,
    _cdb_store_tbl_bhm_module,
    _cdb_store_tbl_openflow_tunnel_interface,
    _cdb_store_tbl_openflow_tunnel_local_ip_cnt,
    _cdb_store_tbl_inband_snat,
    _cdb_store_tbl_arpinsp,
    _cdb_store_tbl_arpacl_config,
    _cdb_store_tbl_arpace_config,
    _cdb_store_tbl_copp_cfg,
    _cdb_store_tbl_fea_copp_trap_group,
    _cdb_store_tbl_sflow_global,
    _cdb_store_tbl_sflow_collector,
    _cdb_store_tbl_sflow_counter_port,
    _cdb_store_tbl_vrrp_global,
    _cdb_store_tbl_vrrp_vmac,
    _cdb_store_tbl_vrrp_vip,
    _cdb_store_tbl_vrrp_session,
    _cdb_store_tbl_ssm_global,
    _cdb_store_tbl_ssm_port,
    _cdb_store_tbl_ipsla_global,
    _cdb_store_tbl_ipsla_entry,
    _cdb_store_tbl_ipsla_packet_stats,
    _cdb_store_tbl_ipsla_test_stats,
    _cdb_store_tbl_track_global,
    _cdb_store_tbl_track_object,
    _cdb_store_tbl_controller_attr,
    _cdb_store_tbl_oam_session,
    _cdb_store_tbl_oam_session_upload,
    _cdb_store_tbl_lsp_aps_group,
    _cdb_store_tbl_pw_aps_group,
    _cdb_store_tbl_lsp_pe,
    _cdb_store_tbl_lsp_p,
    _cdb_store_tbl_pw_tpe,
    _cdb_store_tbl_section_oam,
    _cdb_store_tbl_oam_flow_info,
    _cdb_store_tbl_tpoam_global,
    _cdb_store_tbl_tpoam_exec_global,
    _cdb_store_tbl_aps_group_info,
    _cdb_store_tbl_oam_inlabel_info,
    _cdb_store_tbl_oam_mpls_aps_check,
    _cdb_store_tbl_vlan_mapping_table,
    _cdb_store_tbl_vlan_mapping_entry,
    _cdb_store_tbl_egress_mapping_entry,
    _cdb_store_tbl_vlan_group,
    _cdb_store_tbl_evc_entry,
    _cdb_store_tbl_mapped_vlan_info,
    _cdb_store_tbl_l2_protocol,
    _cdb_store_tbl_nd_fib,
    _cdb_store_tbl_nd_stats,
    _cdb_store_tbl_cpu_mirror_global,
    _cdb_store_tbl_tap_server,
    _cdb_store_tbl_dhcpv6_debug,
    _cdb_store_tbl_l2edit,
    _cdb_store_tbl_hash_field_profile,
    _cdb_store_tbl_hash_value_profile,
    _cdb_store_tbl_hash_value_global,
    _cdb_store_tbl_nat_global,
    _cdb_store_tbl_nat_pool,
    _cdb_store_tbl_nat_rule,
    _cdb_store_tbl_nat_session,
    _cdb_store_tbl_nat_session_limit,
    _cdb_store_tbl_nat_proxy_arp,
    _cdb_store_tbl_nat_rtif_inside,
    _cdb_store_tbl_nat_rtif_outside,
    _cdb_store_tbl_g8032_ring,
    _cdb_store_tbl_cfm_global,
    _cdb_store_tbl_cfm_md,
    _cdb_store_tbl_cfm_ma,
    _cdb_store_tbl_cfm_maid,
    _cdb_store_tbl_cfm_lmep,
    _cdb_store_tbl_cfm_rmep,
    _cdb_store_tbl_pim_sm,
    _cdb_store_tbl_pim_sm_bsr,
    _cdb_store_tbl_pim_sm_rp,
    _cdb_store_tbl_pim_sm_rp_gp,
    _cdb_store_tbl_pim_dm,
    _cdb_store_tbl_igmp_intf,
    _cdb_store_tbl_rip,
    _cdb_store_tbl_ripng,
    _cdb_store_tbl_rip_network,
    _cdb_store_tbl_ripng_network,
    _cdb_store_tbl_rip_neighbor,
    _cdb_store_tbl_rip_passive_if,
    _cdb_store_tbl_ripng_passive_if,
    _cdb_store_tbl_ospf_area_stub,
    _cdb_store_tbl_ospf6_area_stub,
    _cdb_store_tbl_ospf_area_nssa,
    _cdb_store_tbl_ospf_area_vlink,
};

int32
cdb_store_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)
{
    switch (oper)
    {
    case CDB_OPER_ADD:
        SAL_ASSERT(p_tbl_node->tbl_info->data_size == tbl_len);
        break;

    case CDB_OPER_DEL:
        SAL_ASSERT(p_tbl_node->tbl_info->key_size == tbl_len);
        break;

    case CDB_OPER_SET:
        SAL_ASSERT(p_tbl_node->tbl_info->data_size == tbl_len);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    if (NULL == g_cdb_store_tbl_func[p_tbl_node->id])
    {
        return PM_E_NOT_SUPPORT;
    }

    return g_cdb_store_tbl_func[p_tbl_node->id](oper, fields, p_tbl_node, p_tbl, tbl_len);
}

int32
cdb_store_tbl_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, 
                    void *p_data, uint32 data_len)
{
    void *p_tbl = NULL;
    void *p_left = NULL;
    cdb_sync_refer_t *p_refer = NULL;
    void *p_refer_obj = NULL;
    uint32 add = FALSE;
    int32 rc = PM_E_NONE;

    p_tbl = p_data;
    p_left = p_data + p_tbl_node->tbl_info->key_size;
    p_refer = p_left;
    p_refer_obj = &(p_refer->object);
    SAL_ASSERT(p_tbl_node->tbl_info->key_size + sizeof(cdb_sync_refer_hdr_t) + p_refer->hdr.size == data_len);
    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;

    if (CDB_REFER_TBL == p_refer->hdr.type)
    {
        switch (p_tbl_node->id)
        {
        case TBL_PVLAN:
            switch (fields[0])
            {
            case TBL_PVLAN_FLD_ISLOATE_PORTS:
                if (tbl_interface_get_master())
                {
                    p_refer_obj = tbl_interface_get_interface(p_refer_obj);
                    PM_E_RETURN(tbl_pvlan_set_isloate_ports_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            case TBL_PVLAN_FLD_PROMISCUOUS_PORTS:
                if (tbl_interface_get_master())
                {
                    p_refer_obj = tbl_interface_get_interface(p_refer_obj);
                    PM_E_RETURN(tbl_pvlan_set_promiscuous_ports_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        case TBL_MSTP_PORT:
            switch (fields[0])
            {
            case TBL_MSTP_PORT_FLD_MSTI_LIST:
                if (tbl_msti_port_get_master())
                {
                    p_refer_obj = tbl_msti_port_get_msti_port(p_refer_obj);
                    PM_E_RETURN(tbl_mstp_port_set_msti_list_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        case TBL_MSTP_INSTANCE:
            switch (fields[0])
            {
            case TBL_MSTP_INSTANCE_FLD_PORT_LIST:
                if (tbl_msti_port_get_master())
                {
                    p_refer_obj = tbl_msti_port_get_msti_port(p_refer_obj);
                    PM_E_RETURN(tbl_mstp_instance_set_port_list_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        case TBL_ISOLATION:
            switch (fields[0])
            {
            case TBL_ISOLATION_FLD_PORTS:
                if (tbl_interface_get_master())
                {
                    p_refer_obj = tbl_interface_get_interface(p_refer_obj);
                    PM_E_RETURN(tbl_isolation_set_ports_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        case TBL_ERPS_DOMAIN:
            switch (fields[0])
            {
            case TBL_ERPS_DOMAIN_FLD_ERPS_RING_LIST:
                if (tbl_erps_ring_get_master())
                {
                    p_refer_obj = tbl_erps_ring_get_erps_ring(p_refer_obj);
                    PM_E_RETURN(tbl_erps_domain_set_erps_ring_list_sync(p_tbl, add, p_refer_obj, FALSE));
                }
                break;
            default:
                break;
            }
            break;
        default:
            return PM_E_INVALID_PARAM;
        }
    }
    else if (CDB_REFER_TIMER == p_refer->hdr.type)
    {
    }

    return PM_E_NONE;
}

