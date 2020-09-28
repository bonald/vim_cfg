
#ifndef __CDB_DS_H__
#define __CDB_DS_H__

#define CDB_NAME_ERR_NONE           0
#define CDB_NAME_ERR_EXCEED_LEN     -1
#define CDB_NAME_ERR_FIRST_CHAR     -2
#define CDB_NAME_ERR_LAST_CHAR      -3
#define CDB_NAME_ERR_INVALID_CHAR   -4

void
_cmd_print_para(char** argv , uint8 argc);

#define IS_LEAP_YEAR(y)    (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

extern const char *glb_bhm_action_strs[];
extern const char *glb_time_sync_type_strs[];
extern const char *glb_chip_type_strs[];
extern const char *cdb_sub_format_str[];
extern const char *g_ipc_dest_desc[];
extern const char *cdb_dt_type_str_array[];
extern const char *cdb_oper_str_array[];
extern const char *cdb_access_str_array[];

extern const char *glb_port_speed_strs[];
extern const char *glb_port_duplex_strs[];
extern const char *glb_port_unidir_strs[];
extern const char *glb_port_pfc_mode[];
extern const char *glb_if_mode_strs[];
extern const char *glb_vlan_port_type_strs[];
extern const char *glb_vlan_qinq_type_strs[];
extern const char *glb_agg_mode_strs[];
extern const char *glb_if_pvlan_type_strs[];

extern const char *glb_if_type_strs[];
extern const char *glb_phy_type_strs[];
extern const char *glb_ppt_numchg_type_strs[];
extern const char *glb_port_media_type_strs[];
extern const char *glb_card_state_strs[];
extern const char *glb_chassis_type_strs[];
extern const char *glb_port_media_strs[];
extern const char* glb_port_training_status_strs[];
extern const char *ds_brgif_allowed_vlan_act_strs[];
extern const char *glb_agg_load_balance_strs[];
extern const char *boot_mode_strs[];
extern const char *mlag_role_strs[];
extern const char *glb_flow_ip_frag_strs[];
extern const char *glb_port_isolate_strs[];
extern const char *glb_pdu_l2pdu_port_action_strs[];
extern const char *glb_vlan_security_action_strs[];
extern const char *glb_port_security_violation_strs[];
extern const char *glb_port_security_enable_strs[];
extern const char *arp_inspection_trust_enable_strs[];
extern const char *arp_port_numberlimit_enable_strs[];
extern const char *arp_port_numberlimit_violate_strs[];
extern const char *arpratelimit_exceed_action_str[];
extern const char *arpratelimit_abnormal_str[];
extern const char *arpratelimit_enable_str[];

extern const char *ptp_state_strs[];
extern const char *ptp_bmc_code_strs[];
extern const char *ptp_bmc_expires_strs[];
extern const char *ptp_device_type_strs[];

extern const char *tpoam_mp_type_strs[];
extern const char *tpoam_pw_type_strs[];
extern const char *tpoam_status_strs[];

extern const char *arp_type_strs[];
extern const char *route_type_strs[];
extern const char *as_strs[];
extern const char *ssh_version_strs[];
extern const char *syslog_facility_strs[];
extern const char *syslog_severity_strs[];
extern const char *frr_syslog_severity_strs[];
extern const char *syslog_timestamp_strs[];
extern const char *login_method_strs[];
extern const char *parity_strs[];
extern const char *authen_method_strs[];
extern const char *author_method_strs[];
extern const char *account_method_strs[];
extern const char *accountcmd_method_strs[];
extern const char *key_type_strs[];
extern const char *agent_proc_option_strs[];
extern const char *snmp_version_strs[];
extern const char *dhcbinding_state_strs[];
extern const char *dhcbinding_type_strs[];
extern const char *auth_server_type_strs[];
extern const char *auth_session_status_strs[];
extern const char *snmp_view_class_strs[];
extern const char *snmp_community_strs[];
extern const char *snmp_authen_strs[];
extern const char *snmp_privacy_strs[];
extern const char *snmp_usm_level_strs[];
extern const char *scheduler_mode_strs[];
extern const char *errdis_rsn_strs[];
extern const char *lldp_if_mode_strs[];
extern const char *glb_reboot_type_strs[];
extern const char *glb_warmboot_type_strs[];
extern const char *flush_fdb_type_strs[];
extern const char *flush_fdb_subtype_strs[];
extern const char *flush_fdb_mode_strs[];
extern const char *glb_br_port_state_strs[];
extern const char *erps_state_strs[];
extern const char *erps_msg_type_strs[];
extern const char *erps_node_role_strs[];
extern const char *erps_edge_node_role_strs[];
extern const char *erps_ring_level_strs[];
extern const char *vlanclass_rule_protype_strs[];
extern const char *glb_acl_type_str[];
extern const char *glb_acl_show_running_str[];
extern const char *glb_vty_mode_strs[];
extern const char *ospf_nssa_translate_role_strs[];
extern const char *ospf_vlink_auth_type_strs[];

void
ptp_desc_ptp_sockaddr(ptp_sockaddr_t *p_skaddr);

char*
_ptp_common_message_header_val2str(char *str, uint32 str_len, const ptp_common_message_header_t *p_data);

char *
ptp_desc_oper(uint8 oper);

char *
ptp_desc_msg_type(ptp_msg_type_t type);

char *
ptp_desc_clockid(const glb_ptp_clock_id_t clockid);

char *
ptp_desc_time_source(ptp_time_source_t time_source);

const char*
cdb_if_speed_str(uint32 speed, uint32 oper_speed);

const char*
cdb_if_duplex_str(uint32 duplex, uint32 oper_duplex);

int32
cdb_int_str2val(const char *str, int32 *ret);

uint32
cdb_uint_str2val(const char *str, int32 *ret);

int64
cdb_int64_str2val(const char *str, int32 *ret);

uint64
cdb_uint64_str2val(const char *str, int32 *ret);

double
cdb_double_str2val(const char *str, int32 *ret);

int32
cdb_enum_str2val(const char **array, uint32 array_size, const char *str);

const char*
cdb_enum_val2str(const char **array, uint32 array_size, uint32 value);

int32
cdb_bitmap_str2val(const char *str, uint32 min, uint32 max, uint32 *bmp, uint32 bmp_len, char *err_desc);

char*
cdb_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len);

int32
cdb_mpls_label_bitmap_str2val(const char *str, uint32 min, uint32 max, uint32 *bmp, uint32 bmp_len, char *err_desc);

char*
cdb_mpls_label_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len);

int32
cdb_mac_addr_str2val(uint8 *mac, const char *str);

int32
cdb_mac_addr_masked(uint8 *mac, uint8 *mask);

int32
cdb_ip_addr_masked(uint32 *add, uint32 *add_mask);

char*
cdb_mac_addr_val2str(char *str, uint32 str_len, const uint8 *mac);

char*
cdb_mac_addr_vtysh_val2str(char *str, uint32 str_len, const uint8 *mac);

char*
cdb_addr_ipv4_val2str(char *str, uint32 str_len, const addr_ipv4_t *addr);

int32
cdb_addr_ipv4_str2val(addr_ipv4_t *addr, const char *str, uint32 str_len);

int32
cdb_prefix_ipv4_str2val(char *addr_str, char *masklen_str, prefix_ipv4_t *p_prefix, char *err_desc);

int32
cdb_prefix_ipv4_str2val_wo_masklen(char *str, prefix_ipv4_t *p_prefix);

char*
cdb_addr_ipv6_val2str(char *str, uint32 str_len, const addr_ipv6_t *addr);

int32
cdb_addr_ipv6_str2val(const char *str, addr_ipv6_t *addr);

int32
cdb_prefix_str2val(char *addr_str, char *masklen_str, prefix_t *p_prefix, char *err_desc);

int32
cdb_prefix_str2val_wo_masklen(char *str, prefix_t *p_prefix);

char*
cdb_prefix_ipv4_val2str(char *str, uint32 str_len, const prefix_ipv4_t *p_ipv4);

char*
cdb_prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix);

char*
cdb_rsa_keystr_val2str(char *str, uint32 str_len, const rsa_keystr_t *prefix);

char*
cdb_mstp_brg_id_val2str(char *str, uint32 str_len, const mstp_brg_id_t *brg_id);

int32
cdb_mstp_brg_id_str2val(char *str,  mstp_brg_id_t *brg_id);

char*
cdb_addr_val2str(char *str, uint32 str_len, const addr_t *addr);

int32
cdb_addr_str2val(addr_t *addr, const char *str, uint32 str_len);

char*
cdb_nexthop_key_u_val2str(char *str, uint32 str_len, const nexthop_key_u *prefix, uint8 family);

int32
cdb_mac_str2val_dhcs(uint8 *mac, const char *str, uint32 str_len);

int32
cdb_fdb_key_str2val(fdb_key_t *fdb_key, const char *str, char *err_desc);

char*
cdb_fdb_key_val2str(char *str, uint32 str_len, const fdb_key_t *fdb_key);

int32
cdb_l2edit_key_str2val(const char *str, l2edit_key_t *l2edit_key);

char*
cdb_l2edit_key_val2str(char *str, uint32 str_len, const l2edit_key_t *l2edit_key);

char*
cdb_g8032_fea_ring_val2str(char *str, uint32 str_len, const fea_g8032_ring_key_t  *g8032_fea_ring_key);

int32
cdb_g8032_fea_ring_str2val(char *str, fea_g8032_ring_key_t  *g8032_fea_ring_key);

char*
cdb_igsp_group_val2str(char *str, uint32 str_len, const igsp_group_key_t *igsp_group_key);

int32
cdb_igsp_group_str2val(char *str, igsp_group_key_t *igsp_group_key);

char*
cdb_lldp_msap_id_val2str(char *str, uint32 str_len, const lldp_msap_id_t *msap_id);

int32
cdb_ptp_foreign_master_str2val(char *str, const ptp_foreign_master_t *ptp_foreign_master);

char*
cdb_ptp_foreign_master_val2str(char *str, uint32 str_len, const ptp_foreign_master_t *ptp_foreign_master);

int32
cdb_ptp_residence_time_cache_str2val(char *str, const ptp_residence_time_cache_t *ptp_residence_time_cache);

char*
cdb_ptp_residence_time_cache_val2str(char *str, uint32 str_len, const ptp_residence_time_cache_t *p_data);

int32
cdb_lldp_msap_id_str2val(char *str, lldp_msap_id_t *msap_id);

char *
cdb_ace_config_key_val2str(char *str, uint32 str_len, const ace_config_key_t *ace_config_key);

int32
cdb_ace_config_key_str2val(char *str, ace_config_key_t *ace_config_key);

char*
cdb_class_in_policy_config_key_val2str(char *str, uint32 str_len, class_in_policy_config_key_t *class_in_policy_config_key);

int32
cdb_class_in_policy_config_key_str2val(char *str, class_in_policy_config_key_t *class_in_policy_config_key);

char*
cdb_acl_in_class_config_key_val2str(char *str, uint32 str_len, acl_in_class_config_key_t *acl_in_class_config_key);

int32
cdb_acl_in_class_config_key_str2val(char *str, acl_in_class_config_key_t *acl_in_class_config_key);

char*
cdb_class_map_action_config_key_val2str(char *str, uint32 str_len, class_map_action_config_key_t *class_map_action_config_key);

int32
cdb_class_map_action_config_key_str2val(char *str, class_map_action_config_key_t *class_map_action_config_key);

char*
cdb_tap_group_ingress_key_val2str(char *str, uint32 str_len, tap_group_ingress_key_t *tap_group_ingress_key);

char*
cdb_tap_group_ingress_flow_key_val2str(char *str, uint32 str_len, tap_group_ingress_flow_key_t *tap_group_ingress_flow_key);

int32
cdb_tap_group_ingress_key_str2val(char *str, tap_group_ingress_key_t *tap_group_ingress_key);

int32
cdb_tap_group_ingress_flow_key_str2val(char *str, tap_group_ingress_flow_key_t *tap_group_ingress_flow_key);

char*
cdb_tap_group_egress_key_val2str(char *str, uint32 str_len, tap_group_egress_key_t *tap_group_egress_key);

int32
cdb_tap_group_egress_key_str2val(char *str, tap_group_egress_key_t *tap_group_egress_key);

char*
cdb_tap_five_tuple_key_val2str(char *str, uint32 str_len, tap_five_tuple_key_t *tap_five_tuple_key);

int32
cdb_tap_five_tuple_key_str2val(char *str, tap_five_tuple_key_t *tap_five_tuple_key);

char*
cdb_tap_tuple_acl_entry_id_val2str(char *str, uint32 str_len, tap_tuple_acl_entry_id_t *acl_entry_id);

int32
cdb_tap_tuple_acl_entry_id_str2val(char *str, tap_tuple_acl_entry_id_t *acl_entry_id);

char*
cdb_msti_port_key_val2str(char *str, uint32 str_len, const msti_port_key_t *msti_key);

int32
cdb_msti_port_key_str2val(char *str, msti_port_key_t *msti_key);

char*
cdb_ptp_port_id_val2str(char *str, uint32 str_len, const glb_ptp_port_id_t *ptp_port_id);

int32
cdb_ptp_port_id_str2val(char *str, glb_ptp_port_id_t *ptp_port_id);

int32
cdb_auth_server_key_str2val(auth_server_key_t *key, const char *str, char *err_desc);

char*
cdb_auth_server_key_val2str(char *str, uint32 str_len, const auth_server_key_t *key);

int32
cdb_auth_session_key_str2val(auth_session_key_t *key, const char *str, char *err_desc);

char*
cdb_auth_session_key_val2str(char *str, uint32 str_len, const auth_session_key_t *key);

int32
cdb_binding_key_str2val(binding_key_t *key, const char *str, char *err_desc);

char*
cdb_binding_key_val2str(char *str, uint32 str_len, const binding_key_t *key);

char*
cdb_route_node_key_val2str(char *str, uint32 str_len, const route_node_key_t *rn_key);

int32
cdb_route_node_key_with_ad_str2val(char *str, route_node_key_with_ad_t *rn_key);

char*
cdb_route_node_key_with_ad_val2str(char *str, uint32 str_len, const route_node_key_with_ad_t *rn_key);

char*
cdb_ospf_area_range_key_val2str(char *str, uint32 str_len, const ospf_area_range_key_t *ospf_area_range_key);

char*
cdb_ospf6_area_range_key_val2str(char *str, uint32 str_len, const ospf6_area_range_key_t *ospf6_area_range_key);

int32
cdb_ospf_area_range_key_str2val(char *str, ospf_area_range_key_t *ospf_area_range_key);

int32
cdb_ospf6_area_range_key_str2val(char *str, ospf6_area_range_key_t *ospf6_area_range_key);

char*
cdb_ospf_area_vlink_key_val2str(char *str, uint32 str_len, const ospf_area_vlink_key_t *ospf_area_vlink_key);

int32
cdb_ospf_area_vlink_key_str2val(char *str, ospf_area_vlink_key_t *ospf_area_vlink_key);

char*
cdb_rip_timers_val2str(char *str, uint32 str_len, const rip_timers_t *timers);

int32
cdb_rip_timers_str2val(char *str, const rip_timers_t *timers);

char*
cdb_nexthop_key_val2str(char *str, uint32 str_len, const nexthop_key_t *nh_key);

int32
cdb_nexthop_key_str2val(char *str, nexthop_key_t *nh_key);

char*
cdb_nexthop_ecmp_val2str(char *str, uint32 str_len, const nexthop_ecmp_t *nh_group_key);

int32
cdb_nexthop_ecmp_str2val(char *str, nexthop_ecmp_t *nh_group_key);

char*
cdb_nexthop_ecmp_routed_val2str(char *str, uint32 str_len, const nexthop_ecmp_routed_t *nh_group_key);

int32
cdb_nexthop_ecmp_routed_str2val(char *str, nexthop_ecmp_routed_t *nh_group_key);

int32
cdb_mac_str2val(uint8 *mac, const char *str);

int32
cdb_fdb_mac_address_is_valid(const char *mac_addr);

int32
cdb_arp_mac_address_is_valid(const char *mac_addr);

int32
cdb_clock_id_address_is_valid(const char *clockid_addr);

char*
cdb_mac_val2str(char *str, uint32 str_len, const uint8 *mac);

int32
cdb_mstp_digest_str2val(mstp_digest_t *digest, const char *str);

char*
cdb_mstp_digest_val2str(char *str, uint32 str_len, const mstp_digest_t *digest);

int32
cdb_ptp_default_str2val(const char *str, ptp_default_t *p_data);

char*
cdb_ptp_default_val2str(char *str, uint32 str_len, const ptp_default_t *p_data);

int32
cdb_ptp_current_str2val(const char *str, ptp_current_t *p_data);

char*
cdb_ptp_current_val2str(char *str, uint32 str_len, const ptp_current_t *p_data);

int32
cdb_ptp_parent_str2val(const char *str, ptp_parent_t *p_data);

char*
cdb_ptp_parent_val2str(char *str, uint32 str_len, const ptp_parent_t *p_data);

int32
cdb_ptp_time_properties_str2val(const char *str, ptp_time_properties_t *p_data);

char*
cdb_ptp_time_properties_val2str(char *str, uint32 str_len, const ptp_time_properties_t *p_data);

int32
cdb_ptp_global_c_str2val(const char *str, ptp_global_c_t *p_data);

char*
cdb_ptp_global_c_val2str(char *str, uint32 str_len, const ptp_global_c_t *p_data);

int32
cdb_ptp_port_str2val(const char *str, ptp_port_t *p_data);

char*
cdb_ptp_port_val2str(char *str, uint32 str_len, const ptp_port_t *p_data);

int32
cdb_glb_ptp_port_address_str2val(const char *str, glb_ptp_port_address_t *p_data);

char*
cdb_glb_ptp_port_address_val2str(char *str, uint32 str_len, const glb_ptp_port_address_t *p_data);

int32
cdb_glb_ptp_timeinterval_str2val(const char *str, glb_ptp_timeinterval_t *p_data);

char*
cdb_glb_ptp_timeinterval_val2str(char *str, uint32 str_len, const glb_ptp_timeinterval_t *p_data);

int32
cdb_ptp_vlan_info_str2val(const char *str, ptp_vlan_info_t *p_data);

char*
cdb_ptp_vlan_info_val2str(char *str, uint32 str_len, const ptp_vlan_info_t *p_data);

char*
cdb_ptp_dump_flag_val2str(char *str, uint32 str_len, const ptp_dump_flag_t *p_data);

int32
cdb_ptp_dump_flag_str2val(const char *str, ptp_dump_flag_t *p_data);

char*
cdb_ptp_port_p_val2str(char *str, uint32 str_len, const ptp_port_p_t *p_data);

int32
cdb_ptp_port_p_str2val(const char *str, ptp_port_p_t *p_data);

char*
cdb_ptp_port_c_val2str(char *str, uint32 str_len, const ptp_port_c_t *p_data);

int32
cdb_ptp_port_c_str2val(const char *str, ptp_port_c_t *p_data);

char*
cdb_ptp_port_stats_val2str(char *str, uint32 str_len, ptp_port_stats_t *p_data);

int32
cdb_ptp_port_stats_str2val(const char *str, ptp_port_stats_t *p_data);

char*
cdb_ptp_msg_proc_ops_val2str(char *str, uint32 str_len, const ptp_msg_proc_ops_t *p_data);

int32
cdb_ptp_msg_proc_ops_str2val(const char *str, ptp_msg_proc_ops_t *p_data);

char*
cdb_kernel_if_ipv6_addr_val2str(char *str, uint32 str_len, const kernel_if_ipv6_addr_t *p_data);

int32
cdb_kernel_if_ipv6_addr_str2val(const char *str, kernel_if_ipv6_addr_t *p_data);

char*
cdb_brg_allowed_vlan_val2str(char *str, uint32 str_len, const brg_allowed_vlan_t *allowed_vlan);

int32
cdb_brg_allowed_vlan_str2val(char *str, brg_allowed_vlan_t *allowed_vlan);

int32
cdb_ptp_common_message_header_str2val(char *str, const ptp_common_message_header_t *p_data);

char*
cdb_ptp_common_message_header_val2str(char *str, uint32 str_len, const ptp_common_message_header_t *p_data);

int32
cdb_ptp_msg_announce_str2val(char *str, const ptp_msg_announce_t *p_data);

char*
cdb_ptp_msg_announce_val2str(char *str, uint32 str_len, const ptp_msg_announce_t *p_data);

char*
cdb_sal_time_val2str(char *str, uint32 str_len, const sal_time_t *time);

int32
cdb_sal_time_str2val(char *str, sal_time_t *time);

char*
cdb_timestamp_val2str(char *str, uint32 str_len, const timestamp_t *timestamp);

int32
cdb_time_range_timer_str2val(char *str, time_range_timer_t* value);

int32
cdb_timestamp_str2val(char *str, timestamp_t *timestamp);

char*
cdb_vlan_member_port_val2str(char *str, uint32 str_len, vlan_memberport_t *bmp);

int32
cdb_vlan_member_port_str2val(char *str, vlan_memberport_t *bmp);

int32
cdb_board_type_str2val(char *str, glb_board_type_t *type);

char*
cdb_board_type_val2str(char *str, uint32 str_len, const glb_board_type_t *type);

char*
cdb_int8_array_val2str(char *str, uint32 str_len, const int8 *array, uint32 len);

int32
cdb_int8_array_str2val(char *str, int8 *array);

char*
cdb_uint8_array_val2str(char *str, uint32 str_len, const uint8 *array, uint32 len);

int32
cdb_uint8_array_str2val(char *str, uint8 *array);

char*
cdb_int32_array_val2str(char *str, uint32 str_len, const int32 *array, uint32 len);

int32
cdb_int32_array_str2val(char *str, int32 *array);

char*
cdb_uint32_array_val2str(char *str, uint32 str_len, const uint32 *array, uint32 len);

int32
cdb_uint32_array_str2val(char *str, uint32 *array);

char*
cdb_uint64_array_val2str(char *str, uint32 str_len, const uint64 *array, uint32 len);

int32
cdb_uint64_array_str2val(char *str, uint64 *array);

char*
cdb_double_array_val2str(char *str, uint32 str_len, const double *array, uint32 len);

int32
cdb_double_array_str2val(char *str, double *array);

int32
cdb_string_array_str2val(char *str, char **array);

char*
cdb_string_array_val2str(char *str, uint32 str_len, const char *array, uint32 len, uint32 num);

int32
cdb_check_str(const char *name, int32 max_len, char *err_desc);

int32
cdb_check_name(const char *name, int32 max_len, char *err_desc);

int32
cdb_check_snmp_contact(const char *name, int32 max_len, char *err_desc);

char*
cdb_reference_val2str(char *str, uint32 str_len, cdb_reference_t *ref);

char*
cdb_reference_list_val2str(char *str, uint32 str_len, cdb_reference_list_t *ref_list);

char*
cdb_time_range_value_val2str(char *str, uint32 str_len, time_range_value_t *value);

int32
cdb_time_range_value_str2val(char *str, time_range_value_t *value);

char *
cdb_str_tolower (char *str);

int
cdb_month_str2digit (char *month_str);

int32
cdb_get_weekday(char *str, weekday_t *day);

sal_time_t 
cdb_time_range_str2time (char *time_str, char *day_str,
                char *month_str, char *year_str);

int32
cdb_time_range_get_time(char *str, uint8 *hour, uint8 *min);

char*
cdb_time_range_timer_val2str(char *str, uint32 str_len, time_range_timer_t* value);

char*
cdb_glb_stats_val2str(char *str, uint32 str_len, const glb_stats_t* value);

char*
cdb_glb_if_stats_val2str(char *str, uint32 str_len, const glb_if_stats_t* value);

int32
cdb_glb_if_stats_str2val(char *str, glb_if_stats_t* value);

char*
cdb_ds_connected_key_val2str(char *str, uint32 str_len, const void *ifc_key);

int32
cdb_ds_connected_key_str2val(char *str, void *ifc_key);

char*
cdb_ds_connected_v6_key_val2str(char *str, uint32 str_len, const void *ifc_key);

int32
cdb_ds_connected_v6_key_str2val(char *str, void *ifc_key);

char*
cdb_ds_static_route_dup_ipv4_nh_key_val2str(char *str, uint32 str_len, const void *ifc_key);

int32
cdb_ds_static_route_dup_ipv4_nh_key_str2val(char *str, void *ifc_key);

int
cds_uint64_to_str(uint64_t src, char dest[UINT64_STR_LEN]);

char*
cdb_l4_port_val2str(char *str, uint32 str_len, const glb_flow_l4_port_t *value);

int32
cdb_l4_port_str2val(char *str, glb_flow_l4_port_t *l4_port);

char*
cdb_tcp_flag_val2str(char *str, uint32 str_len, const glb_flow_tcp_flag_t *value);

int32
cdb_tcp_flag_str2val(char *str, glb_flow_tcp_flag_t *flag);

char*
cdb_aclqos_if_queue_val2str(char *str, uint32 str_len, const aclqos_if_queue_t *queue);

int32
cdb_aclqos_if_queue_str2val(char *str, aclqos_if_queue_t *queue);

char*
cdb_snmp_trap_val2str(char *str, uint32 str_len, const snmp_trap_key_t *snmp_trap);

int32
cdb_snmp_trap_str2val(char *str, snmp_trap_key_t *snmp_trap);

char*
cdb_snmp_view_val2str(char *str, uint32 str_len, const snmp_view_key_t *snmp_view);

int32
cdb_snmp_view_str2val(char *str, snmp_view_key_t *snmp_view);

char*
cdb_snmp_group_usm_val2str(char *str, uint32 str_len, const snmp_group_usm_key_t *snmp_usm_group);

int32
cdb_snmp_group_usm_str2val(char *str, snmp_group_usm_key_t *snmp_usm_group);

char*
cdb_snmp_info_val2str(char *str, uint32 str_len, const snmp_info_t *item);

int32
cdb_snmp_info_str2val(snmp_info_t *item, const char *str, char *err_desc);

char*
cdb_user_val2str(char *str, uint32 str_len, const user_t *item);

int32
cdb_user_str2val(user_t *item, const char *str, char *err_desc);

char*
cdb_snmp_inform_val2str(char *str, uint32 str_len, const snmp_inform_key_t *snmp_inform);

int32
cdb_snmp_inform_str2val(char *str, snmp_inform_key_t *snmp_inform);

char*
cdb_cem_mail_val2str(char *str, uint32 str_len, const mail_t *cem_mail);

int32
cdb_cem_mail_str2val(char *str, mail_t *cem_mail) ;


int32
cdb_rsa_keystr_str2val(char *str, rsa_keystr_t *keystr);

int32
cdb_mstp_brg_id_str2val(char *str, mstp_brg_id_t *brg_id);

int32
cdb_ns_port_forwarding_key_str2val(ns_port_forwarding_key_t *key, const char *str, char *err_desc);

char*
cdb_ns_port_forwarding_key_val2str(char *str, uint32 str_len, const ns_port_forwarding_key_t *key);

char*
cdb_erps_ring_key_val2str(char *str, uint32 str_len, const erps_ring_key_t *erps_ring_key);

int32
cdb_erps_ring_key_str2val(char *str,  erps_ring_key_t *erps_ring_key);

char*
cdb_lldp_civic_address_t_val2str(char *str, uint32 str_len, const lldp_civic_address_t *civic);

int32
cdb_lldp_civic_address_t_str2val(char *str, lldp_civic_address_t *civic);

char*
cdb_dot1x_radius_key_val2str(char *str, uint32 str_len, const dot1x_radius_key_t *key);

int32
cdb_dot1x_radius_key_str2val(char *str, dot1x_radius_key_t *key);

char*
cdb_sflow_collector_key_val2str(char *str, uint32 str_len, const sflow_collector_key_t *key);

char*
cdb_dot1x_mac_key_val2str(char *str, uint32 str_len, const dot1x_mac_key_t *key);

int32
cdb_sflow_collector_key_str2val(char *str, sflow_collector_key_t *key);

char*
cdb_mirror_mac_escape_key_val2str(char *str, uint32 str_len, const mirror_mac_escape_key_t *key);

int32
cdb_dot1x_mac_key_str2val(char *str, dot1x_mac_key_t *key);

int32
cdb_mirror_mac_escape_key_str2val(char *str, mirror_mac_escape_key_t *key);

char*
cdb_ns_route_key_val2str(char *str, uint32 str_len, const ns_route_key_t *key);

int32
cdb_ns_route_key_str2val(char *str, ns_route_key_t *key);

char*
cdb_openflow_manager_key_val2str(char *str, uint32 str_len, const openflow_manager_key_t *key);

int32
cdb_openflow_manager_key_str2val(char *str, openflow_manager_key_t *key);

char*
cdb_inband_snat_key_val2str(char *str, uint32 str_len, const inband_snat_key_t *key);

int32
cdb_inband_snat_key_str2val(char *str, inband_snat_key_t *key);

char *
cdb_arpace_config_key_val2str(char *str, uint32 str_len, const arpace_config_key_t *arpace_config_key);

char*
cdb_ssm_clock_id_val2str(char *str, uint32 str_len, const uint8 *mac);

int32
cdb_ssm_clock_id_str2val(char *str, ssm_clock_id_t clock_id);

int32
cdb_arpace_config_key_str2val(char *str, arpace_config_key_t *arpace_config_key);

int32
cdb_route_node_key_str2val(char *str, route_node_key_t *rn_key);

char*
cdb_rmon_stats_history_key_val2str(char *str, uint32 str_len,
                               const rmon_stats_history_key_t *rmon_stats_history_key);
int32
cdb_rmon_stats_history_key_str2val(char *str, rmon_stats_history_key_t *rmon_stats_history_key);

char*
cdb_ipsla_packet_stats_key_val2str(char *str, uint32 str_len, const ipsla_packet_stats_key_t *ipsla_packet_statistics_key);

int32
cdb_ipsla_packet_stats_key_str2val(char *str, ipsla_packet_stats_key_t *ipsla_packet_stats_key);

char*
cdb_ipsla_test_stats_key_val2str(char *str, uint32 str_len, const ipsla_test_stats_key_t *ipsla_test_statistics_key);

int32
cdb_ipsla_test_stats_key_str2val(char *str, ipsla_test_stats_key_t *ipsla_test_stats_key);

char*
cdb_mrt_val2str(char *str, uint32 str_len, const mrt_key_t *mrt_key);

int32
cdb_mrt_str2val(char *str, mrt_key_t *mrt_key);

char*
cdb_mrt_key_val2str(char *str, uint32 str_len, const mrt_key_t *mrt_key);

int32
cdb_mrt_key_str2val(char *str, mrt_key_t *mrt_key);

int32 
cdb_percent_point_str2value(char *str, uint32 *value_out);

char*
tpoam_api_signal_status_desc(tpoam_status_t signal_status);

char*
tpoam_api_event_ext_desc(uint32 event);

int32
cdb_tpoam_dm_str2val(const char *str, tpoam_dm_t*p_data);

char*
cdb_tpoam_dm_val2str(char *str, uint32 str_len, const tpoam_dm_t*p_data);

int32
cdb_tpoam_lm_str2val(const char *str, tpoam_lm_t*p_data);

char*
cdb_tpoam_lm_val2str(char *str, uint32 str_len, const tpoam_lm_t*p_data);

int32
cdb_tpoam_lb_str2val(const char *str, tpoam_lb_t*p_data);

char*
cdb_tpoam_lb_val2str(char *str, uint32 str_len, const tpoam_lb_t*p_data);

int32
cdb_tpoam_event_str2val(const char *str, tpoam_event_t*p_data);

char*
cdb_tpoam_event_val2str(char *str, uint32 str_len, const tpoam_event_t*p_data);

int32
cdb_tpoam_status_str2val(const char *str, tpoam_status_t*p_data);

char*
cdb_tpoam_status_val2str(char *str, uint32 str_len, const tpoam_status_t*p_data);

int32
cdb_tpoam_stats_str2val(const char *str, tpoam_stats_t*p_data);

char*
cdb_tpoam_stats_val2str(char *str, uint32 str_len, const tpoam_stats_t*p_data);

int32
cdb_tpoam_list_str2val(const char *str, tpoam_list_t*p_data);

char*
cdb_tpoam_list_val2str(char *str, uint32 str_len, const tpoam_list_t*p_data);

int32
cdb_tpoam_rc_count_str2val(const char *str, tpoam_rc_count_t*p_data);

char*
cdb_tpoam_rc_count_val2str(char *str, uint32 str_len, const tpoam_rc_count_t*p_data);

int32
cdb_tpoam_exec_str2val(const char *str, tpoam_exec_t*p_data);

char*
cdb_tpoam_exec_val2str(char *str, uint32 str_len, const tpoam_exec_t*p_data);

int32
cdb_tpoam_trap_cache_str2val(const char *str, tpoam_trap_cache_t*p_data);

char*
cdb_tpoam_trap_cache_val2str(char *str, uint32 str_len, const tpoam_trap_cache_t*p_data);

int32
cdb_tpoam_ratelimit_str2val(const char *str, tpoam_ratelimit_t*p_data);

char*
cdb_tpoam_ratelimit_val2str(char *str, uint32 str_len, const tpoam_ratelimit_t*p_data);

int32
cdb_tpoam_lmm_exec_str2val(const char *str, tpoam_lmm_exec_t*p_data);

char*
cdb_tpoam_lmm_exec_val2str(char *str, uint32 str_len, const tpoam_lmm_exec_t*p_data);

int32
cdb_tpoam_lb_exec_str2val(const char *str, tpoam_lb_exec_t*p_data);

char*
cdb_tpoam_lb_exec_val2str(char *str, uint32 str_len, const tpoam_lb_exec_t*p_data);

int32
cdb_tpoam_1dm_dmm_exec_str2val(const char *str, tpoam_1dm_dmm_exec_t*p_data);

char*
cdb_tpoam_1dm_dmm_exec_val2str(char *str, uint32 str_len, const tpoam_1dm_dmm_exec_t*p_data);

int32
cdb_upload_tpoam_dm_str2val(const char *str, upload_tpoam_dm_t*p_data);

char*
cdb_upload_tpoam_dm_val2str(char *str, uint32 str_len, const upload_tpoam_dm_t*p_data);

int32
cdb_upload_tpoam_lm_str2val(const char *str, upload_tpoam_lm_t*p_data);

char*
cdb_upload_tpoam_lm_val2str(char *str, uint32 str_len, const upload_tpoam_lm_t*p_data);

int32
cdb_upload_tpoam_ccm_str2val(const char *str, upload_tpoam_ccm_t*p_data);

char*
cdb_upload_tpoam_ccm_val2str(char *str, uint32 str_len, const upload_tpoam_ccm_t*p_data);

char *
tpoam_desc_dm_interval_val2str(uint32 interval);

char *
tpoam_desc_dm_interval_val2str_without_s(uint32 interval);

 int32
tpoam_desc_dm_interval_str2val(char* interval_str,  uint32* interval);

char*
cdb_vlan_mapping_entry_key_val2str(char *str, uint32 str_len,
                               const vlan_mapping_entry_key_t *vm_entry_key);
int32
cdb_vlan_mapping_entry_key_str2val(char *str, vlan_mapping_entry_key_t *vm_entry_key);

char*
cdb_egress_mapping_entry_key_val2str(char *str, uint32 str_len,
                               const egress_mapping_entry_key_t *vm_entry_key);
int32
cdb_egress_mapping_entry_key_str2val(char *str, egress_mapping_entry_key_t *vm_entry_key);

char*
cdb_mapped_vlan_info_key_val2str(char *str, uint32 str_len,
                               const mapped_vlan_info_key_t *mapped_info_key);

char *
cdb_summer_time_val2str(char *str, uint32 str_length,summer_time_t*p_data);

char *
cdb_which_summer_time_val2str(char *str, uint32 str_length,which_summer_time_t*p_data);
char *
cdb_summer_time_str2val(const char *str,summer_time_t*p_data);
char *
cdb_which_summer_time_str2val(const char *str,which_summer_time_t*p_data);
int32
cdb_mapped_vlan_info_key_str2val(char *str, mapped_vlan_info_key_t *mapped_info_key);

int32
cdb_nd_prefix_key_str2val(const char *str, nd_prefix_key_t *key);

char*
cdb_nd_prefix_key_val2str(char *str, uint32 str_len, const nd_prefix_key_t *key);

char*
cdb_l2pro_if_action_vlan_val2str(char *str, uint32 str_len, const l2pro_if_action_vlan_t *p_data);

int32
cdb_l2pro_if_action_vlan_str2val(const char *str, l2pro_if_action_vlan_t *p_data);

char*
cdb_qos_color_action_val2str(char *str, uint32 str_len, glb_qos_color_action_t *p_data);

int32
cdb_qos_color_action_str2val(const char *str, glb_qos_color_action_t *p_data);

char *
cdb_qos_policer_res_key_val2str(char *str, uint32 str_len, qos_policer_res_key_t *p_data);

int32
cdb_qos_policer_res_key_str2val(const char *str, qos_policer_res_key_t *p_data);

char *
cdb_nat_session_key_val2str(char *str, uint32 str_len, nat_session_key_t *p_data);

int32
cdb_nat_session_key_str2val(const char *str, nat_session_key_t *p_data);

char *
cdb_poe_cfg_val2str(char *str,uint32 str_len, glb_poe_cfg_t *p_data);

int32
cdb_poe_cfg_str2val(const char *str, glb_poe_cfg_t *p_data);

char *
cdb_nat_rule_key_val2str(char *str, uint32 str_len, nat_rule_key_t *p_data);

int32
cdb_nat_rule_key_str2val(const char *str, nat_rule_key_t *p_data);

char*
cdb_opb_val2str(char *str, uint32 str_len, const ctclib_opb_t *opb);

int32
cdb_opb_str2val(const char *str, ctclib_opb_t *p_data);

int32
cdb_nat_proxy_arp_key_str2val(const char *str, nat_proxy_arp_key_t *p_data);

char *
cdb_nat_proxy_arp_key_val2str(char *str, uint32 str_len, nat_proxy_arp_key_t *p_data);

char *
cdb_cfm_ma_key_val2str(char *str, uint32 str_len, cfm_ma_key_t *p_data);

int32
cdb_cfm_ma_key_str2val(const char *str, cfm_ma_key_t *p_data);

char *
cdb_cfm_lmep_key_val2str(char *str, uint32 str_len, cfm_lmep_key_t *p_data);

int32
cdb_cfm_lmep_key_str2val(const char *str, cfm_lmep_key_t *p_data);

char *
cdb_cfm_rmep_key_val2str(char *str, uint32 str_len, cfm_rmep_key_t *p_data);

int32
cdb_cfm_rmep_key_str2val(const char *str, cfm_rmep_key_t *p_data);

char *
cdb_cfm_time_key_val2str(char *str, uint32 str_len, cfm_time_key_t *p_data);

int32
cdb_cfm_time_key_str2val(const char *str, cfm_time_key_t *p_data);

char*                                                               
cdb_pim_sm_gprefix_val2str(char *str, uint32 str_len, const pim_sm_gprefix_t *gprefix);
                                                                    
char*                                                               
cdb_pim_static_rp_addr_val2str(char *str, uint32 str_len, const pim_static_rp_addr_t *gprefix); 
#endif /* !__CDB_DS_H__ */

