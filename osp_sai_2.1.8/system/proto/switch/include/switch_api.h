
#ifndef __SWITCH_API_H__
#define __SWITCH_API_H__

/* MSTP APIs */
int32
swth_api_add_vlan_cb(vid_t vid);

int32
swth_api_del_vlan_cb(vid_t vid);

int32
swth_mstp_api_add_port(tbl_interface_t *p_db_if);

int32
swth_mstp_api_del_port(tbl_interface_t *p_db_if);

int32
swth_mstp_api_update_if_path_cost(tbl_interface_t *p_db_if);

int32
swth_mstp_api_update_if_running(tbl_interface_t *p_db_if);

int32
swth_mstp_api_update_if_p2p_mac(tbl_interface_t *p_db_if);

int32
swth_mstp_api_update_system_mac(uint8 *system_mac);

int32
swth_module_init();

int32
swth_module_start(switch_master_t *master);

int32
switch_is_mlag_peer_link(uint32 ifindex);

int32
swth_api_igsp_mroute_del_port(tbl_interface_t *p_db_if);

int32
swth_if_del_cb(tbl_interface_t *p_db_if);
int32
swth_if_up_cb(tbl_interface_t *p_db_if);
int32
swth_if_down_cb(tbl_interface_t *p_db_if);
int32
swth_if_set_allowed_vlan_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add);
int32
swth_if_add_to_agg_cb(tbl_interface_t *p_db_if);

int32
swth_set_lacp_priority(uint16 priority);

int32
swth_vlan_disable_cb(tbl_vlan_t *p_db_vlan);

uint32
swth_api_is_igmp_snooping_en(vid_t vid);

int32
swth_api_mstp_topology_change_cb(uint32 new_topology_change_detected, uint32 ageing_time);

int32
swth_api_get_mlag_remain_sec();

int32
swth_api_is_mlag_peer_link(uint32 ifindex);

#endif /* !__SWITCH_API_H__ */

