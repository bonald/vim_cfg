
#ifndef __IGSP_API_H__
#define __IGSP_API_H__

#include "igmp_define.h"

void
igsp_api_max_resp_time_fn(void *p_data);

int32
igsp_api_intf_add(vid_t vid);

int32
igsp_api_intf_del(vid_t vid);

int32
igsp_api_intf_set_fast_leave(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_acl_name(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_enable(tbl_igsp_intf_t *p_igsp_if);
        
int32
igsp_api_intf_disable(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_version(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_lmqi(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_lmqc(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_max_member_number(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_robustness_var(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_discard_unknown(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_report_suppress(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_querier_operate(tbl_igsp_intf_t *p_igsp_if, uint32 operate, uint32 other_querier_addr);

int32
igsp_api_intf_set_querier_enable(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_query_interval(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_update_other_query_interval_group_member_interval(tbl_igsp_intf_t *p_db_igsp_if);

int32
igsp_api_intf_set_admin_other_query_interval(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_querier_max_response_time(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_set_querier_config_addr(tbl_igsp_intf_t *p_igsp_if,tbl_igsp_intf_t *p_db_igsp_if);

int32
igsp_api_intf_set_mrouter_aging_interval(tbl_igsp_intf_t *p_igsp_if);

int32
igsp_api_intf_add_static_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_mr_port);

int32
igsp_api_intf_add_dynamic_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_mr_port);

int32
igsp_api_intf_del_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_db_mr_port);

int32
igsp_api_intf_del_query_port(tbl_igsp_intf_t *p_db_igsp_if, ds_query_t *p_db_query);

int32
igsp_api_intf_add_query(ds_query_t *p_query, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address, uint32 max_resp_time);

int32
igsp_api_intf_update_querier_oper_addr(tbl_igsp_intf_t *p_igsp_if);
void
igsp_api_timer_mrouter_expire_fn(void *p_data);
uint32
igsp_api_intf_get_static_mrouter_count(tbl_igsp_intf_t *p_db_igsp_if);

int32
igsp_api_intf_update_querier_oper_addr_all();

void
igsp_api_timer_other_querier_fn(void *p_data);

int32
igsp_api_get_vlan_erps_enable(vid_t vid);

int32
igsp_api_global_enable();

int32
igsp_api_global_disable();

int32
igsp_api_global_set_fast_leave(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_discard_unknown(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_report_suppress(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_host_join(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb);

int32
igsp_api_global_set_version(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb);

int32
igsp_api_global_set_querier_max_response_time(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_query_interval(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_lmqc(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_lmqi(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_robustness_var(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_max_member_number(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_global_src(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_tcn_enable(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_tcn_query_count(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_tcn_query_interval(tbl_igsp_global_t *p_glb);

int32
igsp_api_global_set_tcn_query_max_response_time(tbl_igsp_global_t *p_glb);

int32
igsp_api_group_add_static(tbl_igsp_group_t *p_igsp_grp, tbl_igsp_intf_t *p_db_igsp_if);

int32
igsp_api_group_del_static(tbl_igsp_group_t *p_igsp_grp);

int32
igsp_api_add_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_group);

int32
igsp_api_del_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_db_group);

int32
igsp_api_group_add_dynamic(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    uint32 group_address, uint32 src_address, uint32 is_v1, uint32 is_v3, const ctc_msg_t *p_msg);

int32
igsp_api_group_del_dynamic(tbl_igsp_group_t *p_db_igsp_grp);

uint32
igsp_api_get_group_count(tbl_igsp_group_t *p_lookup_grp);

int32
igsp_api_group_rx_leave(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address, uint32 is_v3_leave);

int32
igsp_api_clear_groups_by_vlan(addr_ipv4_t *group_addr, vid_t vid);

int32
igsp_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
igsp_cmd_process_intf(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
igsp_cmd_process_group(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
igsp_cmd_process_act_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
igsp_cmd_process_act_clear(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
igsp_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add);

int32
igsp_api_if_down_cb(tbl_interface_t *p_db_if);

int32
igsp_api_if_del_cb(tbl_interface_t *p_db_if);

int32
igsp_api_if_add_to_agg_cb(tbl_interface_t *p_db_if);

int32
igsp_api_vlan_disable_cb(tbl_vlan_t *p_db_vlan);

uint32
igsp_api_is_igmp_snooping_en(vid_t vid);

int32
igsp_api_mstp_topology_change_cb(uint32 new_topology_change_detected);

int32
igsp_api_intf_acl_prefix_check(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address, uint32 src_address);

int32
igsp_init();

int32
igsp_start(switch_master_t *master);

int32
igsp_stop(switch_master_t *master);

#endif /* __IGSP_API_H__ */

