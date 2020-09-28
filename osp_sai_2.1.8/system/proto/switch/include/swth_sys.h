
#ifndef __SWITCH_SYS_H__
#define __SWITCH_SYS_H__

int32
swth_sys_pdu_set_port_action(uint32 ifindex, uint32 mode, uint32 set_mode);

int32
swth_sys_pdu_init();

int32
swth_sys_check_system_mac(const uint8 *mac);

int32
swth_sys_set_route_mac(uint8 *route_mac, uint32 mac_num);

int32
swth_sys_set_hostname(char *hostname);

int32
swth_sys_set_init_done();

int32
swth_sys_set_startup_done();

int32
swth_cmd_process_sys_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
swth_cmd_process_log_global(char **argv, int32 argc, cfg_cmd_para_t *para);

uint32
swth_sys_is_startup_done();

int32
swth_sys_set_ipg_global(char *type, char* value);

int32
swth_sys_set_aaa_new_model(int onoff);

int32
swth_sys_set_aaa_privilege(int privilege1, int privilege2, int privilege3);

int32
swth_sys_set_dhcp_service(int onoff);

int32
swth_sys_set_dhcp_relay(int onoff, cfg_cmd_para_t *para);

int32
swth_sys_set_dhcp_server(int onoff);

int32
swth_sys_set_dhcp_snooping(int onoff);

int32
swth_sys_set_http_service(int onoff);

int32
swth_sys_set_max_vty(int vtys, cfg_cmd_para_t *para);

int32
swth_sys_set_telnet_disable(uint32 disable, cfg_cmd_para_t *para);

int32
swth_sys_set_telnet_port(uint32 port);

int32
swth_sys_set_cut_through_enable(uint32 cut_through_enable);

int32
swth_sys_set_http_image_name(char *pszImage);

int32
swth_sys_set_service_password_encryption(int onoff);

int32
swth_sys_set_sflow_agent(addr_t *ip_address);

int32
swth_sys_set_sflow_counter_interval(uint32 counter_interval);

int32
swth_sys_get_sflow_enable();

int32
swth_sys_set_sflow_enable(uint32 enable);

int32
swth_sys_set_rpc_http_port(uint32 http_port);

int32
swth_sys_set_http_timeout(uint32 interval);

int32
swth_sys_set_tap_timestamp_ether(mac_addr_t macda, mac_addr_t macsa, uint32 ether_type);

int32
swth_sys_set_truncation_length(uint32 length);

int32
swth_sys_set_ptf_port(uint32 ptf_port);

int32
swth_cmd_show_resource(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
swth_sys_set_memory_threshold(uint32 mem_threshold1, uint32 mem_threshold2, uint32 mem_threshold3);

int32
swth_cmd_show_memory_threshold(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
swth_sys_get_reserve_and_total_memory(uint32 *sys_kernel_reserve, uint32 *sys_total);

void
swth_sys_get_memorycheck_states(void *p_argv);

char*
swth_sys_get_memchkstate_val2str(uint32 memchk_state);

int32
swth_sys_set_http_port(uint32 port);

int32
swth_sys_set_http_disable(uint32 disable, cfg_cmd_para_t *para);

int32
swth_sys_set_https_port(uint32 port);

int32
swth_sys_set_https_disable(uint32 disable, cfg_cmd_para_t *para);

int32
swth_sys_set_rpcapi_disable(uint32 disable, uint32 ssl_en, uint32 port, uint32 ssl_port, cfg_cmd_para_t *para);

int32
swth_sys_set_rpcapi_auth_mode(uint32 auth_mode, cfg_cmd_para_t *para);

int32
memory_threshold_check_init();

int32
swth_cmd_show_monitor(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
swth_cmd_ssh_cfg_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
                         uint32 field_id, void *p_tbl, void *p_ds);

int32
swth_cmd_clear_rpcapi_counter(char **argv, int32 argc, cfg_cmd_para_t *para);

void swth_sys_set_http_conf(uint32 port,uint32 disable);

int32
app_webui_en_file(uint32 en);

#endif /* !__SWITCH_SYS_H__ */

