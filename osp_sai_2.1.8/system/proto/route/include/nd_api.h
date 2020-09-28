
#ifndef __ND__API_H__
#define __ND__API_H__

int32
nd_api_add_static_route_to_frr_when_del_nd(tbl_nd_fib_t *p_db_nd_fib);

int32
nd_api_del_static_route_from_frr_when_add_nd(tbl_nd_fib_t *p_nd_fib);

int32
nd_neigh_add_static_entry(prefix_t* p_addr, uint8 *p_mac, tbl_interface_t  *p_if, tbl_route_if_t  *p_db_rtif, cfg_cmd_para_t *para);

int32
nd_neigh_del_static_entry(prefix_t* p_addr, tbl_interface_t  *p_if, cfg_cmd_para_t *para);

int32 nd_hop_limit_set(uint32 hop_limit, cfg_cmd_para_t *para);

int32
nd_ra_hop_limit_set(tbl_route_if_t *p_db_rtif, uint32 current_hop_limit, cfg_cmd_para_t *para);

int32
nd_ra_advertise_set(tbl_route_if_t *p_db_rtif, uint32 advertise, cfg_cmd_para_t *para);

int32
nd_ra_advertise_mtu_set(tbl_route_if_t *p_db_rtif, uint32 advertise_mtu, cfg_cmd_para_t *para);

int32
nd_ra_interval_set(tbl_route_if_t *p_db_rtif, uint32 ra_interval_max, uint32 ra_interval_min, cfg_cmd_para_t *para);

int32
nd_ra_lifetime_set(tbl_route_if_t *p_db_rtif, uint32 lifetime, cfg_cmd_para_t *para);

int32
nd_dad_attempts_set(tbl_route_if_t *p_db_rtif, uint32 dad_attempts, cfg_cmd_para_t *para);

int32
nd_ns_interval_set(tbl_route_if_t *p_db_rtif, uint32 ns_interval, cfg_cmd_para_t *para);

int32
nd_reachable_time_set(tbl_route_if_t *p_db_rtif, uint32 config_reachable_time, cfg_cmd_para_t *para);

int32
nd_managed_config_set(tbl_route_if_t *p_db_rtif, uint32 managed_config, cfg_cmd_para_t *para);

int32
nd_other_config_set(tbl_route_if_t *p_db_rtif, uint32 other_config, cfg_cmd_para_t *para);

void
nd_init_para_for_add_if(tbl_route_if_t *p_rt_if);

int32
nd_prefix_default_set(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_prefix_add_entry(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_prefix_del_entry(tbl_route_if_t *p_db_rtif, char** argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_address_install(tbl_route_if_t *p_db_rtif, void* ifc_v6);

void
nd_ifc_timer_handler(void *p_data);

int32 
nd_send_ns (tbl_route_if_t *p_db_rtif, addr_ipv6_t *target, uint32 unicast, uint32 unspecified);

int32
nd_send_na (tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_daddr, 
    struct sal_in6_addr *p_solicited_addr, uint8 router, uint8 solicited, uint8 override, uint8 inc_opt);

int32
nd_send_ra (tbl_route_if_t *p_db_rtif, struct sal_in6_addr *p_saddr, struct sal_in6_addr *p_daddr, uint32 router_lifetime);

int32
nd_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
nd_api_miss_trigger(addr_ipv6_t *ip_addr, uint32 kernel_ifindex);

int32
nd_start();

void*
nd_neigh_create(void  *p_nd_fib, tbl_interface_t  *p_db_if);

void
nd_neigh_timer_handler(void *p_data);

int32
nd_neigh_update_na(void *p_data, char *p_mac, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, uint8 router, uint8 solicited, uint8 override);

int32
nd_neigh_update_other(void *p_data, char *p_mac, tbl_route_if_t *p_db_rtif, tbl_interface_t  *p_db_if, uint8 type);

int32 
nd_check_interface(tbl_route_if_t *p_db_rtif, cfg_cmd_para_t *para);

int32
nd_neigh_delete(tbl_nd_fib_t  *p_nd_fib);

int32
nd_dad_failed(ds_connected_v6_t  *p_ifc, tbl_route_if_t *p_db_rtif);

int32
nd_if_stop(tbl_route_if_t *p_db_rtif, bool duplicate);

int32
neigh_ifc_down(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc, bool delete_addr);

int32
neigh_ifc_up(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc);

int32
nd_if_rtadv_start(tbl_route_if_t *p_db_rtif);

int32
nd_if_rtadv_stop(tbl_route_if_t *p_db_rtif);

int32
nd_if_rtadv_reset(tbl_route_if_t *p_db_rtif);

void 
nd_rtadv_if_ra_solicited(void *p_data);

void
nd_rtadv_if_timer(void *p_data);

int32
nd_if_rtadv_chage_ra_interval(tbl_route_if_t *p_db_rtif);

int32
nd_api_if_show(FILE *fp, tbl_route_if_t *p_db_rtif);

int32
nd_fib_make_key(tbl_nd_fib_key_t *p_fib_key, addr_ipv6_t* p_addr, tbl_interface_t  *p_db_if);

int32
nd_prefix_delete_all_entry(tbl_route_if_t *p_db_rtif);

int32
nd_neigh_delete_all_if_entries(uint32 ifindex);

int32 
nd_clear_nd_entries_and_config(tbl_route_if_t *p_db_rtif);

int32
neigh_ifc_add(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc);

int32
neigh_ifc_del(tbl_route_if_t *p_db_rtif, ds_connected_v6_t  *p_ifc);

int32
nd_neigh_route_add(tbl_nexthop_t   *pst_nexthop);

int32
nd_neigh_route_del(tbl_nexthop_t   *pst_nexthop);

int32
nd_api_if_up_pre_unset(tbl_route_if_t *p_db_rtif);

int32
nd_api_probe_for_fdb_age(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

uint32
nd_api_count_get_max();

int32
nd_api_kernel_update_nd(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

#endif /* !__ND_API_H__ */


