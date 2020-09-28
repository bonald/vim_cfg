#ifndef __ARP_API_H__
#define __ARP_API_H__

#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "route.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_ether.h"
#include "arp_sm.h"


#define ARP_INVALID_IF_INDEX    0

typedef int32 (*ARP_ENTRY_ITER_FUNC)(tbl_arp_t *p_db_arp, void *p_data, void *p_data1);

int32 rt_if_set_arp_ageing_timeout(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
        uint32 field_id, void *p_tbl, void *p_ds);

int32 arp_api_if_set_mirror_update(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
        uint32 field_id, void *p_tbl, void *p_ds);

int32 arp_api_ifc_match(addr_ipv4_t *addr, tbl_kernel_if_t *pst_kernel_if);

int32 arp_api_rt_ifc_match(addr_ipv4_t *addr, ds_connected_t *ifc);

int32 arp_db_synch_hw_os(tbl_arp_t *ae_p, arp_syn_op_t op);

uint32 arp_db_cache_overflow();

uint32
arp_api_count_get_max();

int32 arp_db_entry_create(tbl_interface_t *p_db_if, addr_ipv4_t *ip, 
             uint8 *ll_addr, arp_type_t type, 
             uint8 is_proxy, uint8 is_router, tbl_arp_t **entry);

int32
arp_db_entry_remove(tbl_arp_t *p_db_arp);

tbl_arp_t*
arp_db_entry_lookup(addr_ipv4_t *ip);

int32
arp_db_count_inc(tbl_arp_t *p_arp);

int32
arp_db_count_dec(tbl_arp_t *p_arp);

int32
rt_api_add_static_route_to_frr_when_del_arp(tbl_arp_t *p_db_arp);

int32
arp_api_connected_add(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, uint32 add_not_up, 
    uint32 new_up_running, mac_addr_t vmac);

int32
arp_api_connected_del(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn);

int32
arp_api_connected_up(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, uint32 new_up_running);

int32
arp_api_connected_down(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn);

int32
arp_api_get_kernel_if_by_tip(tbl_interface_t *p_db_if, addr_ipv4_t *tar_ip, tbl_kernel_if_t **pst_kernel_if);

int32
arp_api_get_ifc(tbl_interface_t *p_db_if, addr_ipv4_t *addr, tbl_kernel_if_t **pst_kernel_if);

int32
arp_api_get_rt_ifc(tbl_interface_t *p_db_if, addr_ipv4_t *addr, ds_connected_t **pp_conn);

int32
arp_api_get_ifc_for_shortest_prefix(tbl_interface_t *p_db_if, addr_ipv4_t *addr, tbl_kernel_if_t *pst_kernel_if);

int32
arp_api_get_rt_ifc_for_shortest_prefix(tbl_interface_t *p_db_if, addr_ipv4_t *addr, ds_connected_t **pp_conn);

ds_connected_t*
rt_if_lookup_ifc_ipv4(tbl_interface_t *p_db_if, addr_ipv4_t *addr);

int32
arp_api_fib_match(prefix_t *p, tbl_iproute_node_t *pst_iproute_node);

int32
arp_api_check_prefix_same_with_ifaddr(char *ifname, addr_ipv4_t *prefix);

tbl_kernel_if_t *
arp_api_check_addr_with_kernel_if_exist(addr_ipv4_t *addr_ip);

tbl_route_if_t *
arp_api_check_addr_with_rt_if_exist(tbl_interface_t *p_db_if, addr_ipv4_t *addr_ip);

tbl_route_if_t *
arp_api_check_addr_with_rt_if_exist_advance(addr_ipv4_t *addr_ip);

int32
arp_api_route_del(tbl_nexthop_t *p_db_nh);

int32
arp_api_route_del_nh_arp_by_nexthop_key(nexthop_key_t *pst_nexthop);

int32
arp_api_route_add(tbl_nexthop_t *p_db_nh);

int32
arp_api_update_static_arp(tbl_arp_t *p_db_arp, tbl_arp_t *p_arp, tbl_interface_t *p_db_if);

int32
arp_api_add_static_arp(tbl_arp_t *p_arp, tbl_interface_t *p_db_if);

int32
arp_api_del_static_arp(tbl_arp_t *p_arp);

int32
arp_api_miss_trigger(addr_ipv4_t *ip_addr, uint32 kernel_ifindex);

int32
arp_api_check_is_routemac(tbl_interface_t *p_db_if, uint8 *mac, int32 *is_routemac);

int32
_arp_api_if_set_mirror_update(tbl_interface_t *pst_interface, char *ifname);

int32
arp_api_aging_time_update(tbl_arp_t * p_db_arp, void *p_data, void *p_data1);

int32
_rt_if_set_arp_ageing_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout);

int32
arp_api_aging_time_init(tbl_arp_t *p_db_arp);

uint32
arp_api_entry_match(addr_ipv4_t *ip);

int32
arp_api_del_by_ifindex(tbl_arp_t *p_db_arp, void *p_data, void *p_data1);

int32
arp_api_flush_statistics();

int32
arp_api_trav_ifindex(uint32 ifindex, ARP_ENTRY_ITER_FUNC func, void *p_data, void *p_data1);

int32
arp_api_trav(ARP_ENTRY_ITER_FUNC func, void *p_data, void *p_data1);

int32
arp_api_init();

int32
arp_api_deinit();

int32
arp_cmd_process_arp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_cmd_process_flusharp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_api_process_flusharp_statistics(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
_arp_api_if_set_arp_numberlimit_enable(tbl_interface_t* p_db_if, uint32 arp_numberlimit_enable);

int32
_arp_api_if_set_arp_numberlimit_number(tbl_interface_t* p_db_if, uint32 arp_numberlimit_number);

int32
arp_update_vip_dynamic_arp_fib(addr_ipv4_t *dest);
#endif /* !__ARP_API_H__ */
