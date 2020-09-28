
#ifndef __MSTP_API_H__
#define __MSTP_API_H__

/* msti port API */

int32
mstp_api_set_msti_port_path_cost(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port);
int32
mstp_api_set_msti_port_priority(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port);
int32
mstp_api_set_msti_port_restricted_role(tbl_msti_port_t *p_port);
int32
mstp_api_set_msti_port_restricted_tcn(tbl_msti_port_t *p_port);

/* msti API */

int32
mstp_make_bridge_id(mstp_brg_id_t *bridge_id, uint8 *address, uint16 new_priority, uint32 instance_id);

int32
mstp_make_bridge_id_priority(mstp_brg_id_t *bridge_id, uint16 new_priority, uint32 instance_id);

int32
mstp_api_set_msti_priority(tbl_mstp_instance_t *p_db_inst, uint32 priority);

tbl_msti_port_t*
mstp_api_get_msti_port(tbl_mstp_port_t *p_db_port, uint16 instance_id);

tbl_mstp_instance_t*
mstp_api_add_msti(tbl_mstp_instance_t *p_inst);

int32
mstp_api_del_msti(tbl_mstp_instance_t *p_inst);

/* port API */
int32
mstp_api_set_port_admin_force_version(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_force_version(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_path_cost(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_enable(tbl_mstp_port_t *p_db_port, uint32 enable, uint32 del_port);

int32
mstp_api_set_port_priority(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_bpdufilter(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_bpduguard(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_rootguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port);

int32
mstp_api_set_port_loopguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port);

int32
mstp_api_set_port_admin_edge(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_admin_p2pmac(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_restricted_role(tbl_mstp_port_t *p_port);

int32
mstp_api_set_port_restricted_tcn(tbl_mstp_port_t *p_port);

int32
mstp_api_add_port(tbl_interface_t *p_db_if);

int32
mstp_api_del_port(tbl_interface_t *p_db_if);

int32
mstp_api_clear_counters(tbl_mstp_port_t *p_db_port);

int32
mstp_api_set_port_mcheck(tbl_mstp_port_t *p_db_port);

int32
mstp_api_clear_all_disabled_port();

/* if update API */
int32
mstp_api_update_if_running(tbl_interface_t *p_db_if);

int32
mstp_api_update_if_path_cost(tbl_interface_t *p_db_if);

int32
mstp_api_update_if_p2p_mac(tbl_interface_t *p_db_if);

int32
mstp_api_flush_fdb(tbl_mstp_port_t *p_db_port);

/* global API */
int32
mstp_api_set_glb_tc_protection(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_tx_hold_count(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_priority(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_pathcost_standard(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_fwd_delay(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_hello_time(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_max_age(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_max_hops(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_bpduguard(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_bpdufilter(tbl_mstp_global_t *p_glb);

int32
mstp_api_clear_mst_config();

int32
mstp_api_set_glb_type(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_enable(tbl_mstp_global_t *p_glb, uint32 force_blocking);

int32
mstp_api_set_glb_region_name(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_revision_level(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_of_exclude_all_vlan(tbl_mstp_global_t *p_glb);

int32
mstp_api_update_system_mac(uint8 *system_mac);

int32
mstp_api_reselect();

int32
mstp_api_reselect_all_msti();

int32
mstp_api_port_reselect_all_msti(tbl_mstp_port_t *p_db_port);

int32
mstp_api_reselect_msti_port(tbl_msti_port_t *p_db_port);

int32
mstp_api_msti_reselect(tbl_mstp_instance_t *p_db_inst);

int32
mstp_api_set_port_fei_state(tbl_mstp_port_t *p_db_port, uint32 state);

int32
mstp_api_set_msti_fei_state(tbl_msti_port_t *p_db_port, uint32 state);

int32
mstp_api_set_global_enable_fei(uint32 enable, uint32 force_blocking);

tbl_mstp_instance_t*
mstp_api_instance_lookup_vlan(vid_t vid);

int32
mstp_api_instance_add_vlan(vid_t vid);

int32
mstp_api_instance_del_vlan(vid_t vid);
int32
mstp_api_of_del_vlan(vid_t vid);

int32
mstp_api_instance_change_to_vlan(vid_t vid, tbl_mstp_instance_t *p_db_inst_new);

int32
mstp_api_instance_change_to_default_vlan(vid_t vid, tbl_mstp_instance_t *p_db_inst_old);

int32
mstp_api_instance_set_vlan_db(tbl_mstp_instance_t *p_inst);

int32
mstp_api_tx();

int32
mstp_api_all_port_tx_pdu();

int32
mstp_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add);

/* MLAG APIs */
int32
mstp_api_set_mlag_enable(uint32 mlag_enable);

int32
mstp_api_set_mlag_role(mlag_role_t mlag_role);

/* cmd callback */
int32
mstp_cmd_process_mstp_port(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_cmd_process_msti_port(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_cmd_process_mstp_instance(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_cmd_process_mstp_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_cmd_process_mstp_clear_stp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_cmd_process_mstp_show_mstp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mstp_sm_role_selection(void);

int32 
newTcWhile(tbl_mstp_port_t *p_db_port);

int32
mstp_api_set_glb_region_name(tbl_mstp_global_t *p_glb);

int32
mstp_api_set_glb_revision_level(tbl_mstp_global_t *p_glb);

int32
mstp_api_update_digest();

#endif /* !__MSTP_API_H__ */
