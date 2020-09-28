#ifndef __DOT1X_ARI_H__
#define __DOT1X_ARI_H__

void dot1x_run_sm(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_port_timer_fn(void *p_data);

void dot1x_be_initialize(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_port_check_dot1x_interface(tbl_dot1x_port_t *p_db_dot1x_port, cfg_cmd_para_t *para);

void dot1x_set_fea_mode(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_unset_fea_mode(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_port_clear_dead_radius(void);

void dot1x_port_active_radius_server(tbl_dot1x_radius_t  *p_db_dot1x_radius);

void
dot1x_set_auth_cfg_retry(tbl_dot1x_global_t *p_glb);

void
dot1x_set_auth_cfg_timeout(tbl_dot1x_global_t *p_glb);

void
dot1x_set_auth_cfg_deadtime(tbl_dot1x_global_t *p_glb);

void
dot1x_set_auth_cfg_shared_secret(tbl_dot1x_global_t *p_glb);

void
dot1x_set_auth_server_retry(tbl_dot1x_radius_t  *p_db_dot1x_radius);

void
dot1x_set_auth_server_timeout(tbl_dot1x_radius_t  *p_db_dot1x_radius);

void
dot1x_set_auth_server_shared_secret(tbl_dot1x_radius_t  *p_db_dot1x_radius);

void dot1x_reauth_timer_fn(void *p_data);

void dot1x_reject_timer_fn(void *p_data);

void dot1x_api_user_off_line(tbl_dot1x_port_t *p_db_dot1x_port, tbl_dot1x_mac_t  *p_db_dot1x_mac);

void
dot1x_api_user_off_line_whole_port(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_in_or_both(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_set_fea_port_authorize(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_set_fea_port_unauthorize(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_set_fea_port_enable(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_set_fea_port_disable(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_set_fea_port_ctrl(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_set_fea_port_guest_vlan(tbl_dot1x_port_t *p_db_dot1x_port);

void
dot1x_set_fea_mac_guest_vlan(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_cmd_set_port_guest_vlan(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    tbl_interface_t  *p_db_if, int32 field_id, cfg_cmd_para_t *para, bool check);

int32
dot1x_cmd_set_port_mac_auth_bypass(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para);

int32
dot1x_cmd_set_port_authmode(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_t *p_db_dot1x_port, 
    int32 field_id, cfg_cmd_para_t *para, bool check);

int32 dot1x_port_report_link_state(tbl_interface_t *p_if);

int32 dot1x_port_init(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_port_up(tbl_dot1x_port_t *p_db_dot1x_port);

int32 dot1x_port_down(tbl_dot1x_port_t *p_db_dot1x_port);

void
dot1x_set_fea_port_mac_enable(tbl_dot1x_port_t *p_db_dot1x_port);

void
dot1x_set_fea_port_mac_disable(tbl_dot1x_port_t *p_db_dot1x_port);

void dot1x_set_fea_add_mac(tbl_dot1x_mac_t *p_dot1x_mac);

void dot1x_set_fea_delete_mac(tbl_dot1x_mac_t *p_dot1x_mac);

uint32 
dot1x_allocate_session_id(void);

void 
dot1x_free_session(uint32 session_id);

void
dot1x_proc_radius(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess);

void
dot1x_radius_end_association(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_radius_end_mac_association(tbl_dot1x_mac_t *p_db_dot1x_mac);

tbl_dot1x_mac_t * 
dot1x_db_add_mac(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_dot1x_mac_entry);

void
dot1x_db_del_mac(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_dot1x_mac_entry);

void
dot1x_db_mac_inc_count(tbl_dot1x_port_t  *p_db_dot1x_port);

void
dot1x_db_mac_dec_count(tbl_dot1x_port_t  *p_db_dot1x_port);

int32
dot1x_db_check_count(tbl_dot1x_port_t  *p_db_dot1x_port, bool check_sw_db);

void
dot1x_mac_connect_session(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_auth_session_t  *p_auth_session);

void
dot1x_radius_set_identifier(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess);

void
dot1x_radius_release_session(tbl_auth_session_t *pSess);

void
dot1x_delete_session(uint32  index);

void
dot1x_mac_access_reject(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_db_dot1x_mac);

void dot1x_set_port_mode(tbl_dot1x_global_t  *p_db_glb, tbl_dot1x_port_t   *p_db_dot1x_port);

void
dot1x_radius_key_encryption(void);
#endif /* !__DOT1X_API_H__ */
