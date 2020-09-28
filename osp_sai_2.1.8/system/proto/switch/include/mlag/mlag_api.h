
#ifndef __MLAG_API_H__
#define __MLAG_API_H__

#define __MLAG_DESC_API__
char*
mlag_msg_type_str(uint8 type);

/* __MLAG_IS_API__ */
uint32
mlag_api_is_peer_conf_none(tbl_mlag_port_t *p_db_port);
uint32
mlag_is_peer_addr_valid(tbl_mlag_peer_t *p_peer);
uint32
mlag_is_peer_ready();

/* __MLAG_CMD_USED_API__ */
int32
mlag_api_established_reload_recovery();
int32
mlag_api_set_reload_delay_interval(tbl_mlag_t *p_mlag);
int32
mlag_api_create(tbl_mlag_t *p_mlag);
int32
mlag_api_destory();
int32
mlag_api_create_peer(tbl_mlag_peer_t *p_peer);
int32
mlag_api_destory_peer();
int32
mlag_api_create_peer_link(tbl_interface_t *p_db_if);
int32
mlag_api_destory_peer_link();
int32
mlag_api_add_mlag_port(tbl_interface_t *p_db_if, uint32 mlag_id);
int32
mlag_api_del_mlag_port(tbl_interface_t *p_db_if);
int32
mlag_api_set_peer_timers(tbl_mlag_peer_t *p_peer);
int32
mlag_api_clear_counters();

/* __MLAG_FSM_PACKET_USED_API__ */
int32
mlag_api_sync_peer_conf_all_ports(tbl_mlag_peer_t *peer);
int32
mlag_api_clear_peer_conf();
int32
mlag_api_update_mlag_is_group(tbl_mlag_port_t *p_db_port);
int32
mlag_api_update_mlag_port_block(tbl_mlag_port_t *p_db_port);
int32
mlag_api_flush_fdb_sync_in();
int32
mlag_api_sync_peer_fdb(tbl_mlag_peer_t *peer);
int32
mlag_api_peer_fdb_flush();
int32
mlag_api_role_select();
int32
mlag_api_syspri_select();
int32
mlag_api_restart_socket(tbl_mlag_peer_t *peer);
int32
mlag_api_peer_session_close(tbl_mlag_peer_t *peer);
int32
mlag_api_port_peer_conf(uint32 mlag_id, uint8 is_up);
int32
mlag_api_port_peer_unconf(uint32 mlag_id);
int32
mlag_api_peer_failover(uint32 mlag_id, uint8 is_recover);
int32
mlag_api_get_reload_delay_remain_sec();

/* __MLAG_SWITCH_API_CALLBACK__ */
int32
mlag_api_if_del_cb(tbl_interface_t *p_db_if);
int32
mlag_api_if_up_cb(tbl_interface_t *p_db_if);
int32
mlag_api_if_down_cb(tbl_interface_t *p_db_if);
int32
mlag_api_set_lacp_system_priority_cb(uint16 priority);
int32
mlag_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add);
/* MLAG command API */
int32
mlag_cmd_process_mlag(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
mlag_cmd_process_mlag_peer(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
mlag_cmd_process_mlag_port(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
mlag_cmd_process_show_mlag(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
mlag_cmd_process_clear_counters(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mpf_process_event(mlag_event_t mpf_event);

#endif /* !__MLAG_API_H__ */

