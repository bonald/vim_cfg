
#ifndef __TRACK_API_H__
#define __TRACK_API_H__

uint32
track_debug_is_on(uint32 flag);

int32
track_build_static_addrt_cmdstr(char *cmd_str, uint32 *static_rt_update_cnt, tbl_static_route_cfg_t  *p_db_static_route);

int32
track_build_static_delrt_cmdstr(char *cmd_str, uint32 *static_rt_update_cnt, tbl_static_route_cfg_t  *p_db_static_route);

int32
track_update_static_route(tbl_track_object_t *p_db_object, uint8 track_del);

int32
track_ipv4_route_unset(tbl_static_route_cfg_key_t *p_rst_cfg_key, cfg_cmd_para_t *para);

int32
track_ipv4_route_set(tbl_static_route_cfg_key_t *p_rst_cfg_key, uint16 track_id, uint16 *is_add, cfg_cmd_para_t *para);

int32
track_update_vrrp_priority(tbl_track_object_t *p_db_object);

int32
track_del_tracked_vrrp(u_int16_t trackobj_id, int32 vrid, cfg_cmd_para_t *para);

int32
track_add_tracked_vrrp(u_int16_t trackobj_id, int32 vrid, cfg_cmd_para_t *para);

int32
track_delay_up_process(tbl_track_object_t  *p_db_object, u_int16_t  track_timer_interval);

int32
track_delay_down_process(tbl_track_object_t  *p_db_object, u_int16_t  track_timer_interval);

int32
track_update_interface_status(tbl_track_object_t  *p_db_object, int32 if_state);

int32
track_update_rtr_status(tbl_track_object_t  *p_db_object);

int32
track_update_object_status(tbl_track_object_t  *p_db_object);

int32
track_quick_update_interface_status();

void
track_timer(tbl_track_global_t *p_db_track_glb);

void
track_sys_timer_update(void *p_data);

int32
track_sys_init_timer(tbl_track_global_t *p_db_track_glb, uint32 start);

int32
track_delete_object(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object);

int32
track_create_object_intf(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object,
        char *track_intf, uint32 object_id, cfg_cmd_para_t *para);

int32
track_create_object_rtr(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object,
        char *rtr_type, uint32 rtr_entry_id, uint32 object_id, cfg_cmd_para_t *para);

int32
track_start();

#endif /* !__TRACK_API_H__ */

