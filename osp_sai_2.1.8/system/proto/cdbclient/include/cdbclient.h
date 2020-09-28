
#ifndef __CDB_CLIENT_H__
#define __CDB_CLIENT_H__

typedef struct app_cpu_utilization_s
{
    char  name[CMD_BUF_64];
    float cpu_usage;
}app_cpu_utilization_t;

typedef struct
{
    char  tbl_line[FILE_READ_LINE_32K_LEN];
    char  ds_line[FILE_READ_LINE_32K_LEN];
    char  ds2_line[FILE_READ_LINE_32K_LEN];
} cdbctl_line_t;

typedef int32 (*CDB_SUBSCRIBE_ACTION_FUNC)(cdb_subscribe_format_t format, char *output);

typedef int32 (*CDB_SUBSCRIBE_USER_FUNC)(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg);

int32
cdbctl_set_grep_str(char *grep_str);

const char*
interface_type_str(const char *mode, const char *vlan_type);

const char*
interface_get_status(uint32 flags, uint32 is_short, uint32 errdisable_rsn, uint32 mirror_dest_flag);

int32
cdbclt_get_tbl_data(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, void *p_tbl, uint32 tbl_size);

int32
cdbclt_get_tbl(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, char *path);

int32
cdbclt_get_tbl_line(cdb_tbl_t tbl_id, void *p_data, uint32 data_len, char *line, uint32 line_len);

int32
cdbclt_process_cmd_cfg(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info);

int32
cdbclt_process_cmd_batch_cfg(cdb_node_t* p_bat_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info);

int32
cdbclt_process_cmd_get(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info);

int32
cdbclt_timer_get_remain_seconds(char *str);

int32 
cdbctl_subscribe_action_log(cdb_subscribe_format_t format, char *output);

int32 
cdbctl_subscribe_action_print(cdb_subscribe_format_t format, char *output);

int32 
cdbctl_subscribe_action_script(cdb_subscribe_format_t format, char *output);

typedef struct
{
    char *ip_addr;
    char *dest_addr;
    char *ip_addr_2nd[RT_IF_ADDRESS_MAX];
    char *dest_addr_2nd[RT_IF_ADDRESS_MAX];
    uint32 count_2nd;
} ip_addr_array_t;

int32
cdbctl_get_primary_ip_addr(char *conn_ipv4, ip_addr_array_t *array);

/* cdbclient_show_intf.c */
int32
cdbctl_show_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_aclqos_if_stats(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

int32
cdbctl_show_aclqos_if(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

int32
cdbctl_show_lag_global_sysid(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

int32
cdbctl_show_lag_global_load_balance(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_l2.c */
int32
cdbctl_show_fdb(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_psfdb(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_psfdb_status(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_psfdb_current_port_count(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_psfdb_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_macfilter(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_vlan(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_pvlan(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_brg_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_lag_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_mstp.c */
int32
cdbctl_show_mstp_port(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_mstp_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_mstp_instance(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_l3.c */
int32
cdbctl_show_arp(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

int32
cdbctl_show_ip_interface(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

int32
cdbctl_show_iproute_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_sys.c */
int32
cdbctl_show_version(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_sys_spec(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_mirror(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_manage_if(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_bootimage(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_rebootinfo(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_sys_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_transceiver(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_memory_summary(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_log_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_log(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_chassis(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_debug.c */
int32
cdbctl_show_switch_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_route_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_ntp_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_ssm_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_openflow_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_ptp_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_quagga_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_dhcp_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_opm_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_lsrv_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_hsrv_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_auth_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_dhcpv6_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_acl.c */
int32
cdbctl_show_acl(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_cmap(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_pmap_if_stats(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_pmap(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_time_range(FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show_qos.c */
int32
cdbclt_show_qos_global(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_policer_action_profile(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_policer_profile(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_drop_profile(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_queue_shape_profile(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_port_shape_profile(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_qos_mpls_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);


/* cdbclient_show_appcfg.c */
int32
cdbctl_show_app_dhcsrvgrp(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dhcrelay(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dhclient(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dhcsnooping(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dhcbinding(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_ssh_cfg(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_syslog_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_user(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_rsa(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_authen(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_author(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_account(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_accountcmd(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_console(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_vty(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_sys_enable(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_login(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_view(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_community(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_context(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_usm_user(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_group_usm(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_access_usm(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_notify(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_target_addr(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_target_params(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_trap(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_snmp_inform(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_static_dns(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_ntp_key(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_ntp_ace(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_ntp_server(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_ntp_cfg(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dynamic_dns_domain(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_dynamic_dns_server(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_app_cpu_utilization(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_copp_cfg(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbclt_show_cpu_traffic_group(FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_iptables_prevent(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_errdisable(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_nat_pool_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_nat_rule_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_nat_session_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);
int32
cdbctl_show_nat_session_limit_node(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info);

/* cdbclient_show.c */
int32
cdbclt_process_cmd_show(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info);

/* cdbclient_buildcfg_l2.c */
int32
cdbclt_buildcfg_build_vlan(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_fdb(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_l2mc_fdb(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_macfilter(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_brg_global(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_lag_global(FILE* cfg_fp, FILE *fp);

/* cdbclient_buildcfg_mstp.c */
int32
cdbclt_buildcfg_build_mstp_global(FILE* cfg_fp, FILE *fp);

/* cdbclient_buildcfg_l3.c */
int32
cdbclt_buildcfg_build_route_global(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_iproute_node(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_arp(FILE* cfg_fp, FILE *fp);

/* cdbclient_buildcfg_sys.c */
int32
cdbclt_buildcfg_build_sys_global(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_sys_manage_if(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_sys_mirror(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_sys_tap(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_sys_card(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_sys_port(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_sys_errdisable(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_ns_port_forwarding(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_log_global(FILE* cfg_fp, FILE *fp);

/* cdbclient_buildcfg_acl.c */
int32
cdbclt_buildcfg_build_cmap(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_pmap(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_pmap_if(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_time_range(FILE* cfg_fp, FILE *fp);
int32
cdbclt_buildcfg_build_acl(FILE* cfg_fp, FILE *fp, char *cfg_file_name);
int32
cdbclt_buildcfg_build_flow(FILE* cfg_fp, FILE *fp, char *cfg_file_name);

/* cdbclient_buildcfg_intf.c */
int32
cdbclt_buildcfg_build_interface(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_cpu_traffic(FILE* cfg_fp, FILE *fp);

/* cdbclient_buildcfg_appcfg.c */
int32
cdbctl_buildcfg_build_app_ssh_cfg(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_syslog_cfg(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_rsa(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_user(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_authen(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_author(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_account(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_accountcmd(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_dhcsrvgrp(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_dhcrelay(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_dhcsnooping(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_console(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_vty(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_snmp_cfg(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_snmp_view(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_snmp_community(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_snmp_trap(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_snmp_inform(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_ntp_key(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_ntp_server(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_ntp_cfg(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_ntp_ace(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_static_dns(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_dynamic_dns_domain(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_dynamic_dns_server(FILE* cfg_fp, FILE *fp);
int32
cdbctl_buildcfg_build_app_cpu_limit(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_app_iptables_prevent(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_global(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_domain(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_policer_profile(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_drop_profile(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_shape(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_qos_port_shape(FILE* cfg_fp, FILE *fp);

int32
cdbclt_buildcfg_build_openflow(FILE* cfg_fp, FILE *fp);

int32
cdbclt_process_cmd_local(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info);

int32
cdbclt_process_cmd(char *raw_path);

int32
cdbclient_execvp(char **argv);

int32
get_last_exec_result();

int32
cdbclt_sigint_clean();

int32
cdbclt_buildcfg_build_erps_domain(FILE* cfg_fp, FILE *fp);

#endif /* !__CDB_CLIENT_H__ */

