
#ifndef __HAL_API_QOS_H__
#define __HAL_API_QOS_H__

int32
fei_api_qos_get_flow_rule_stats(fei_qos_get_flow_rule_stats_req_t *pst_req);

int32
fei_api_qos_update_agp(fei_qos_update_agp_req_t *pst_req);

int32
fei_api_qos_del_agp(fei_qos_remove_agp_req_t *pst_req);

int32
fei_api_qos_create_flow_action(fei_qos_create_flow_action_req_t *pst_req);

int32
fei_api_qos_update_flow_action(fei_qos_update_flow_action_req_t *pst_req);

int32
fei_api_qos_set_flow_processing_flag(fei_qos_set_flow_processing_flag_req_t *pst_req);

int32
fei_api_qos_add_flow_rules(fei_qos_add_flow_rules_req_t *pst_req);

int32
fei_api_qos_remove_flow_rule(fei_qos_remove_flow_rule_req_t *pst_req);

int32
fei_api_qos_remove_rule_group(fei_qos_remove_rule_group_req_t *pst_req);

int32
fei_api_qos_remove_flow_entry(fei_qos_remove_flow_entry_req_t *pst_req);

int32
fei_api_qos_remove_flow_table(fei_qos_remove_flow_table_req_t *pst_req);

int32
fei_api_qos_apply_flow_table_to_intf(fei_qos_apply_flow_table_to_intf_req_t *pst_req);

int32
fei_api_qos_unapply_flow_table_from_intf(fei_qos_unapply_flow_table_from_intf_req_t *pst_req);

int32
fei_api_qos_flow_update(fei_qos_flow_update_req_t *pst_req);

int32
fei_api_qos_set_port_queue_class(fei_qos_set_port_queue_class_req_t *pst_req);

int32
fei_api_qos_set_port_queue_shape(fei_qos_set_port_queue_shape_req_t *pst_req);

int32
fei_api_qos_unset_port_queue_shape(fei_qos_unset_port_queue_shape_req_t *pst_req);

int32
fei_api_qos_set_port_shape(fei_qos_set_port_shape_req_t *pst_req);

int32
fei_api_qos_unset_port_shape(uint32 *pst_req);

int32
fei_api_qos_set_port_queue_drop(fei_qos_set_port_queue_drop_req_t *pst_req);

int32
fei_api_qos_set_port_queue_stats(fei_qos_set_port_queue_stats_t *pst_req);

int32
fei_api_qos_set_port_queue_wdrr_weight(fei_qos_set_port_queue_wdrr_weight_req_t *pst_req);

int32
fei_api_qos_set_port_queue_wdrr_weight_mtu(fei_qos_set_port_queue_wdrr_weight_mtu_req_t *pst_req);

int32
fei_api_qos_get_port_policer_stats(fei_qos_get_port_policer_stats_req_t *pst_req);

int32
fei_api_qos_clear_port_policer_stats(fei_qos_clear_port_policer_stats_req_t *pst_req);

int32
fei_api_qos_get_flow_policer_stats(fei_qos_get_flow_policer_stats_req_t *pst_req);

int32
fei_api_qos_clear_flow_policer_stats(fei_qos_clear_flow_policer_stats_req_t *pst_req);

int32
fei_api_qos_clear_flow_rule_stats(fei_qos_clear_flow_rule_stats_req_t *pst_req);

int32
fei_api_qos_clear_agp_stats(fei_qos_clear_agp_stats_req_t *pst_req);

int32
fei_api_qos_get_agp_stats(fei_qos_get_agp_stats_req_t *pst_req);

int32
fei_api_qos_get_port_queue_depth(fei_qos_get_port_queue_depth_req_t *pst_req);

int32
fei_api_qos_get_port_queue_stats(fei_qos_get_port_queue_stats_req_t *pst_req);

int32
fei_api_qos_clear_port_queue_stats(fei_qos_clear_port_queue_stats_req_t *pst_req);

int32
fei_api_qos_intf_del_cb(uint32 *pst_req);

int32
fei_api_qos_init_qos_port_cfg(glb_qos_port_init_cfg_t *pst_req);

int32
fei_api_qos_deinit_qos_port_cfg(glb_qos_port_deinit_cfg_t *pst_req);

int32
fei_api_qos_linkagg_mem_join_update(fei_qos_linkagg_mem_join_req_t *pst_req);

int32
fei_api_qos_linkagg_mem_leave_update(fei_qos_linkagg_mem_leave_req_t *pst_req);

int32
fei_api_qos_set_qos_global_enable_flags(fei_qos_set_global_flag_req_t *pst_req);

int32
fei_api_qos_set_pri_color_map_table(fei_qos_set_pri_color_map_req_t *pst_req);

int32
fei_api_qos_set_cfi_enable(fei_qos_domain_set_cfi_enable_req_t *pst_req);

int32
fei_api_qos_set_port_attribute(fei_qos_set_port_attr_req_t *pst_req);

int32
fei_api_qos_set_port_policer(fei_qos_set_port_policer_req_t *pst_req);

int32
fei_api_qos_unset_port_policer(fei_qos_unset_port_policer_req_t *pst_req);

#endif /* !__HAL_API_QOS_H__ */

