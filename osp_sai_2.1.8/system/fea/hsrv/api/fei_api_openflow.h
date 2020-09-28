#ifndef __HAL_API_OPENFLOW_H__
#define __HAL_API_OPENFLOW_H__

int32 
fei_api_openflow_post_sys_startup(fei_openflow_post_sys_startup_t *pst_req);

int32 
fei_api_openflow_port_enable(fei_openflow_port_enable_t *pst_req);

int32 
fei_api_openflow_port_set_protected_vlan(fei_openflow_port_set_protected_vlan_t *pst_req);

int32 
fei_api_openflow_port_ignore_vlan_filter(fei_openflow_port_ignore_vlan_filter_t *pst_req);

int32 
fei_api_openflow_port_tunnel_mpls_disable(fei_openflow_port_tunnel_mpls_disable_t *pst_req);

int32 
fei_api_openflow_port_native_vlan_enable(fei_openflow_port_native_vlan_config_t *pst_req);

int32 
fei_api_openflow_port_set_port_action(fei_openflow_port_pkt_action_t *pst_req);

int32 
fei_api_openflow_port_notify_link(fei_openflow_port_notify_link_t *pst_req);

int32
fei_api_openflow_port_get_link(uint32 ifindex, bool *link);

int32 
fei_api_openflow_port_set_config(fei_openflow_port_set_config_t *pst_req);

int32 
fei_api_openflow_port_add_to_bridge(fei_openflow_port_add_to_bridge_req_t *pst_req);

int32 
fei_api_openflow_bond_create(fei_openflow_bond_create_t *pst_req);

int32 
fei_api_openflow_bond_update_slave(fei_openflow_bond_update_slave_t *pst_req);

int32 
fei_api_openflow_bond_attach_slave(fei_openflow_bond_attach_slave_t *pst_req);

int32 
fei_api_openflow_bond_set_mac(fei_openflow_bond_set_mac_t *pst_req);

int32 
fei_api_openflow_add_flow(fei_openflow_add_flow_req_t *pst_req);

int32 
fei_api_openflow_mod_flow(fei_openflow_mod_flow_req_t *pst_req);

int32 
fei_api_openflow_del_flow(fei_openflow_del_flow_req_t *pst_req);

int32 
fei_api_openflow_reset_flow(fei_openflow_reset_flow_req_t *pst_req);

/* Added by weizj for tunnel bug 33724 */
int32 
fei_api_openflow_set_flow_drop_ingress(fei_openflow_set_drop_ingress *pst_req);

/* Added by weizj for bug 35135 */
int32 
fei_api_openflow_set_group_drop_ingress(fei_openflow_set_drop_ingress *pst_req);

/* Added by weizj for udf */
int32 
fei_api_openflow_udf_enable(fei_openflow_set_udf_parser *pst_req);

int32 
fei_api_openflow_set_udf_parser(fei_openflow_set_udf_parser *pst_req);
/* End by weizj for udf */

int32
fei_api_openflow_clear_flow_stats(fei_openflow_clear_stats_req_t *pst_req);

int32
fei_api_openflow_flow_update_all_action_list(fei_openflow_update_all_action_list_req_t *pst_req);

int32
fei_api_openflow_get_flow_stats(fei_openflow_get_stats_req_t *pst_req);

/* group */
int32
fei_api_openflow_add_group(glb_openflow_group_t *p_fei_group);

int32
fei_api_openflow_del_group(fei_openflow_del_group_req_t *p_fei_group);

int32
fei_api_openflow_clear_group_stats(fei_openflow_clear_stats_req_t *pst_req);

int32
fei_api_openflow_get_group_stats(fei_openflow_get_stats_req_t *pst_req);

/* meter */
/* Added by weizj for meter */
int32
fei_api_openflow_add_meter(fei_openflow_add_meter_req_t* pst_req);

int32
fei_api_openflow_mod_meter(fei_openflow_mod_meter_req_t *pst_req);

int32
fei_api_openflow_del_meter(fei_openflow_del_meter_req_t *pst_req);

int32
fei_api_openflow_mod_meter_ipg_status(fei_openflow_mod_meter_ipg_req_t* pst_req);

int32
fei_api_openflow_clear_meter_stats(fei_openflow_clear_stats_req_t *pst_req);
/* End by weizj for meter */

int32 
fei_api_openflow_add_queue_shape(fei_openflow_add_queue_shape_req_t* pst_req);

/* inband */
/* Added by weizj for inband */
int32
fei_api_openflow_inband_set_config(fei_openflow_inband_set_config_req_t *pst_req);

int32
fei_api_openflow_inband_create_vif(fei_openflow_inband_vif_req_t *pst_req);

int32
fei_api_openflow_inband_destroy_vif(fei_openflow_inband_vif_req_t *pst_req);
/* End by weizj for inband */
#ifdef V580_OPENFLOW
int32
fei_api_openflow_inband_set_downlink_port(fei_openflow_inband_downlink_port_req_t *pst_req);

int32
fei_api_openflow_inband_set_uplink_port(fei_openflow_inband_uplink_port_req_t *pst_req);

int32
fei_api_openflow_inband_unset_uplink_port(fei_openflow_inband_unset_uplink_port_req_t *pst_req);

int32
fei_api_openflow_inband_unset_downlink_port(fei_openflow_inband_unset_downlink_port_req_t *pst_req);
#endif
/* tunnel */
/* Added by weizj for tunnel */
int32
fei_api_openflow_tunnel_port_decap_en(fei_openflow_port_decap_req_t* p_req);

int32
fei_api_openflow_tunnel_update_decap_mode(fei_openflow_tunnel_decap_mode_req_t* p_req);

int32
fei_api_openflow_tunnel_update_vtep_ip(fei_openflow_vtep_ip_req_t* p_req);

int32
fei_api_openflow_tunnel_add_port(fei_openflow_add_tunnel_req_t* p_req);

int32
fei_api_openflow_tunnel_delete_port(fei_openflow_del_tunnel_req_t* p_req);
/* End by weizj for tunnel */

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
int32 
fei_api_openflow_add_tpoam(fei_openflow_add_tpoam_req_t* p_msg_req);

int32
fei_api_openflow_del_tpoam(fei_openflow_del_tpoam_req_t*   p_msg_req);

int32
fei_api_openflow_update_tpoam(fei_openflow_update_tpoam_req_t*   p_msg_req);

int32
fei_api_openflow_g8131_fast_aps_en(fei_g8131_mpls_fast_aps_req_t*  p_msg_req);

int32
fei_api_openflow_g8131_protection_en(fei_g8131_mpls_protection_en_req_t*   p_msg_req);

int32
fei_api_set_pw_mode(uint32*   p_msg_req);
/* liwh end */

int32
hsrv_tpoam_rx_hagt_event(void* pv_msg_data);

int32
hsrv_tpoam_rx_hagt_lm_dual_counters(void* pv_msg_data);

int32
hsrv_tpoam_rx_hagt_lm_sd(void* pv_msg_data);
#endif /* !__HAL_API_OPENFLOW_H__ */
