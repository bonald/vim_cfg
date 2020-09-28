
#ifndef __HAL_API_IF_H__
#define __HAL_API_IF_H__

int32
fei_api_if_create_port_if(fei_create_port_if_req_t *pst_req, uint32 *p_kernel_ifindex);

int32
fei_api_if_destroy_port_if(fei_destroy_port_if_req_t *pst_req);

int32
fei_api_if_set_phy_en(fei_if_set_en_req_t *pst_req);

int32
fei_api_if_set_mode(fei_if_set_mode_req_t *pst_req);

int32
fei_api_if_set_speed(fei_if_set_speed_req_t *pst_req);

int32
fei_api_if_set_duplex(fei_if_set_duplex_req_t *pst_req);

int32
fei_api_if_set_unidir(fei_if_set_unidir_req_t *pst_req);

int32
fei_api_if_set_fec_enable(fei_if_set_fec_enable_req_t *pst_req);

int32
fei_api_if_set_flow_ctl(fei_if_set_flowctl_req_t *pst_req);

int32
fei_api_if_set_jumboframe_en(fei_if_set_jumboframe_en_req_t *pst_req);

int32
fei_api_if_set_pvid(fei_if_set_pvid_req_t *pst_req);

int32
fei_api_if_set_flags(fei_if_set_flags_req_t *pst_req);

int32
fei_api_if_set_vlan_filter(fei_if_set_vlan_filter_req_t *pst_req);

int32
fei_api_if_set_vlan_type(fei_if_set_vlan_type_req_t *pst_req);

int32
fei_api_if_set_phy_state(fei_if_phy_state_notify_t* pst_notify);

int32
fei_api_if_crc_error_log(fei_if_crc_err_log_notify_t* pst_notify);

int32
fei_api_if_set_tpid_value(fei_if_set_tpid_req_t *pst_req);

int32
fei_api_if_set_isolate_group(fei_if_set_port_isolate_group_t *pst_req);

int32
fei_api_if_set_isolate_mode(fei_if_set_port_isolate_mode_t *pst_req);

int32
fei_api_if_set_pvlan(fei_if_set_pvlan_t *pst_req);

int32
fei_api_if_set_mac_learn_en(fei_if_set_mac_learn_en_t *pst_req);

int32
fei_api_if_set_max_frame_size(fei_if_set_max_frame_size_req_t *pst_req);

int32
fei_api_if_get_count(fei_if_get_counters_req_t *pst_req);

int32
fei_api_if_clear_count(fei_if_clear_req_t *pst_req);

int32
fei_api_if_set_crc_check_enable (fei_if_crc_check_enable_req_t* pst_req);

int32
fei_api_if_set_crc_recalculation_enable (fei_if_crc_recalculation_enable_req_t* pst_req);

#endif /* !__HAL_API_IF_H__ */

