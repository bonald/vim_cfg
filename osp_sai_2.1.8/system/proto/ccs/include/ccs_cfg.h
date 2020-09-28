
#ifndef __CCS_CFG_H__
#define __CCS_CFG_H__

int32
ccs_cfg_build_arp(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_arp_user(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_rt_static_cfg(FILE* fp, cfg_cmd_para_t *para, uint32 show_ipv4_ipv6);

/*add by luzilong*/
int32
ccs_cfg_build_router_bgp(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_bgp_network(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_bgp_neighbor(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_ospf(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_ospf_network(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_ospf_area_auth(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_ospf_area_range(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_route_global(FILE* fp, cfg_cmd_para_t *para, uint32 has_arp, uint32 show_ipv6);

int32
ccs_cfg_build_nd_fib(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_l2.c */
int32
ccs_cfg_build_vlan(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_fdb(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_ipsg(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_ipsg_mac(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_l2mc_fdb(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_macfilter(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_brg_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_lag_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_vlanclass_rule(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_vlanclass_group(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_evc_entry(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_vlan_group(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_vlan_mapping_table(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_g8032_ring(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_mlag(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_igmp_snooping(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_mstp.c */
int32
ccs_cfg_build_mstp_global(FILE* fp, cfg_cmd_para_t *para);


/* ccs_cfg_sys.c */
int32
ccs_cfg_build_sys_global(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_mem_summary(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_manage_if(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_cpu_mirror_global(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_mirror(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_mirror_mac_escape(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_sys_tap(FILE* fp, char *key, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_card(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_port(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_errdisable(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_log_global(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_ns_port_forwarding(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_sys_ns_route(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_fea_time(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_bhm_global(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_acl.c */
int32
ccs_cfg_build_cmap(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_pmap(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_pmap_if(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_acl(FILE* fp, char *key, cfg_cmd_para_t *para);
int32
ccs_cfg_build_acl_udf(FILE* fp, char *key, cfg_cmd_para_t *para);
int32
ccs_cfg_build_time_range(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_class_map(FILE* fp, char *key, cfg_cmd_para_t *para);
int32
ccs_cfg_build_policy_map(FILE* fp, char *key, cfg_cmd_para_t *para);
int32
ccs_cfg_build_flow(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_arpacl(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_arpinsp(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_intf.c */
int32
ccs_cfg_build_interface(FILE* fp, char *key, cfg_cmd_para_t *para);

int32
cdbclt_buildcfg_cpu_traffic(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_appcfg.c */
int32
ccs_cfg_build_app_ssh_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_syslog_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_auth_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_auth_server(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_tacacs_server(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_rsa(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_user(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_login_security(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_authen(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_author(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_account(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_accountcmd(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_console(FILE* cfg_fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_vty(FILE* cfg_fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_dhcsrvgrp(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_dhcrelay(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_dhcsnooping(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_dhclient(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_console(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_vty(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_sys_enable(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_app_snmp_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_view(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_community(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_context(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_usm_user(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_access_usm(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_group_usm(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_notify(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_target_addr(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_target_params(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_trap(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_snmp_inform(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_rmon_event(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_rmon_alarm(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_ntp_key(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_ntp_server(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_ntp_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_ntp_ace(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_static_dns(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_dynamic_dns_domain(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_dynamic_dns_server(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_cpu_limit(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_iptables_prevent(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_app_worm_filter(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_app_cem(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_clock(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_app_summer_clock(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_qos.c */
int32
ccs_cfg_build_qos_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_qos_mpls_domain(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_qos_domain(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_qos_policer_action_profile(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_qos_policer_profile(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_qos_drop_profile(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_qos_scheduler(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_qos_port_shape_profile(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_qos_queue_smart_buffer(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_copp_cfg(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_cpu_traffic(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_cpu_traffic_group(FILE* fp, cfg_cmd_para_t *para);
int32
ccs_cfg_build_qos_global(FILE* fp, cfg_cmd_para_t *para);

/* ccs_cfg_openflow.c */
int32
ccs_cfg_build_openflow(FILE* fp, cfg_cmd_para_t *para);


int32
ccs_show_vlan_range(FILE* fp, char *pszPrefix, char *pszVlan);
int32
ccs_cfg_build_cfg(int32 sock, ctc_msg_t *p_msg, cdb_node_t* p_node);

int32
ccs_cfg_build_erps_domain(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_opm_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_lldp_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_dot1x_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_dot1x_radius(FILE* fp, cfg_cmd_para_t *para);

#if defined TAPPRODUCT
int32
ccs_cfg_build_tap(FILE* fp, char *key, cfg_cmd_para_t *para);

int32
ccs_cfg_build_tapdpi(FILE* fp, char *key, cfg_cmd_para_t *para);
#endif

#ifdef CONFIG_SFLOW
int32
ccs_cfg_build_sflow_global(FILE * fp, cfg_cmd_para_t * para);
int32
ccs_cfg_build_sflow_collector(FILE * fp, cfg_cmd_para_t * para);
#endif

int32
ccs_cfg_build_ssm_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_vrrp_global(FILE * fp, cfg_cmd_para_t * para);
int32
ccs_cfg_build_vrrp_session(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_ptp_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_ipsla_entry(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_track_object(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_ecmp_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_rr_prefix(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_ipmc_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_pim_static_rp(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_g8131_lsp_aps_group(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_g8131_pw_aps_group(FILE* fp, cfg_cmd_para_t *para);

int32 
ccs_cfg_build_tpoam_oam_session(FILE* fp, char *key, cfg_cmd_para_t *para);

int32
ccs_cfg_build_tpoam_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_l2_protocol(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_hash_field_profile(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_hash_value_profile(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_hash_value_global(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_nat_pool(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_nat_rule(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_nat_session_limit(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_nat_global(FILE * fp, cfg_cmd_para_t * para);
int32
ccs_cfg_build_cfm_global(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_cfm_md(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_cfm_ma(FILE * fp, cfg_cmd_para_t * para);

int32
ccs_cfg_build_router_pim_sm(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_pim_sm_bsr(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_pim_sm_rp(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_router_pim_dm(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_igmp_intf(FILE* fp, cfg_cmd_para_t *para);

int32
ccs_cfg_build_mrt(FILE* fp, cfg_cmd_para_t *para);

#endif /* !__CCS_CFG_H__ */

