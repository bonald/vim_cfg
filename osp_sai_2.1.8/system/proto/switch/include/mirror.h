#ifndef __MIRROR_H__
#define __MIRROR_H__

extern int32
mirror_create_session(uint32 sessionid);

extern int32
mirror_destroy_session(uint32 sessionid);

extern int32
mirror_add_source_interface_to_session(tbl_mirror_t *p_mirror, uint32 direction, tbl_interface_t *p_db_if, uint32 iter_member);

extern int32
mirror_del_source_interface_from_session(tbl_mirror_t *p_mirror, uint32 direction, tbl_interface_t *p_db_if, uint32 iter_member);

extern int32
mirror_add_dest_to_session(tbl_mirror_t *p_mirror, tbl_interface_t *p_db_if);

extern int32
mirror_del_dest_from_session(tbl_mirror_t *p_mirror, tbl_interface_t *p_db_if);

extern int32
mirror_del_cpu_dest_from_session(tbl_mirror_t *p_db_mirror);

extern int32
mirror_add_source_vlan_to_session(tbl_mirror_t *p_mirror, uint32 direction, vid_t vlanid);

extern int32
mirror_add_source_cpu_to_session(tbl_mirror_t *p_mirror, uint32 direction);

extern int32
mirror_del_source_vlan_from_session(tbl_mirror_t *p_mirror, uint32 direction, vid_t vlanid);

extern int32
mirror_add_remote_dest_to_session(tbl_mirror_t *p_mirror, uint32 vlanid, tbl_interface_t *p_db_if);

extern int32
mirror_del_remote_dest_from_session(tbl_mirror_t *p_mirror, vid_t vlanid, uint32 ifindex);

int32
mirror_add_multi_dest_to_session(tbl_mirror_t *p_mirror, uint32 groupid, tbl_interface_t *p_db_if);

int32
mirror_del_multi_dest_from_session(tbl_mirror_t *p_mirror, uint32 groupid, tbl_interface_t *p_db_if);

extern tbl_mirror_t*
mirror_get_session(uint32 sessionid);

extern int32
mirror_del_vlan_process(vid_t vlanid, bool is_vlanif);

extern int32
mirror_del_port_process(tbl_interface_t *p_db_if);

extern int32
mirror_clear_source_session(tbl_mirror_t *p_db_mirror);

extern int32
mirror_clear_session(tbl_mirror_t *p_db_mirror);

extern int32
mirror_start(switch_master_t *master);

extern int32
mirror_stop(switch_master_t *master);

extern int32
mirror_has_source_config(tbl_mirror_t *p_db_mirror);

int32
mirror_multi_dest_port_is_the_last_one(tbl_mirror_t *p_db_mirror);

extern int32
mirror_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
mirror_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
mirror_mac_escape_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
mirror_mac_escape_add_mac_escape(tbl_mirror_mac_escape_t *p_mac_escape);

int32
mirror_mac_escape_del_mac_escape(tbl_mirror_mac_escape_t *p_mac_escape);

int32
mirror_mac_escape_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);
#endif

