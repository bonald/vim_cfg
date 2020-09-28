
#ifndef __RT_FEA_H__
#define __RT_FEA_H__

#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_nexthop_define.h"
#include "gen/tbl_nexthop.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"
#include "cdb_data_cmp.h"
#include "cdb_const.h"

int32
rt_indirect_ipv4_route_inc(vrf_id_t vrf_id, uint32 num);

int32
rt_indirect_ipv4_route_dec(vrf_id_t vrf_id, uint32 num);

int32
rt_indirect_ipv6_route_inc(vrf_id_t vrf_id, uint32 num);

int32
rt_indirect_ipv6_route_dec(vrf_id_t vrf_id, uint32 num);

uint32
rt_indirect_ipv4_route_count_max_check(uint32 num);

uint32
rt_indirect_ipv6_route_count_max_check(uint32 num);

int32
rt_neigh_counter_inc(vrf_id_t vrf_id);

int32
rt_neigh_counter_dec(vrf_id_t vrf_id);

int32
rt_ecmp_counter_inc(vrf_id_t vrf_id);

int32
rt_ecmp_counter_dec(vrf_id_t vrf_id);

int32
rt_ecmp_groups_counter_inc(vrf_id_t vrf_id);

int32
rt_ecmp_groups_counter_dec(vrf_id_t vrf_id);

int32
rt_ecmp_groups_v6_counter_inc(vrf_id_t vrf_id);

int32
rt_ecmp_groups_v6_counter_dec(vrf_id_t vrf_id);

int32
rt_route_counter_inc(vrf_id_t vrf_id);

int32
rt_route_counter_dec(vrf_id_t vrf_id);

int32
rt_static_v6_routes_inc(vrf_id_t vrf_id);

int32
rt_static_routes_inc(vrf_id_t vrf_id);

uint32
rt_fea_v6_ecmp_count_max_check();

uint32
rt_fea_v6_ecmp_groups_count_max_check();

uint32
rt_fea_ecmp_count_max_check();

uint32
rt_fea_ecmp_groups_count_max_check();

uint32
rt_fea_ecmp_count_get_max();

uint32
rt_fea_v6_ecmp_count_get_max();

int32
rt_static_routes_dec(vrf_id_t vrf_id);

int32
rt_static_v6_routes_dec(vrf_id_t vrf_id);

uint32
rt_fea_static_route_v6_count_get_max();

uint32
rt_fea_indirect_route_v6_count_get_max();

uint32
rt_static_route_count_max_check();

uint32
rt_static_v6_route_count_max_check();

uint32
rt_fea_static_route_count_get_max();

uint32
rt_fea_indirect_route_count_get_max();

tbl_nexthop_t *
rt_fea_nexthop_ifname_add(char *ifname);

int32
rt_fea_nexthop_ifname_del(char *ifname);

uint32
rt_fea_route_match_rtif_addr(addr_ipv4_t addr_ip, uint32 mask_len);

int32
rt_fea_arp_hw_syn(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static, uint32 op_flag);

int32
rt_fea_arp_add_static_by_loop(tbl_arp_t *pst_arp, tbl_iter_args_t *pargs);

int32
rt_fea_arp_del_static_by_loop(tbl_arp_t *pst_arp, tbl_iter_args_t *pargs);

tbl_nexthop_t *
rt_fea_nexthop_ipv4_get(char *ifname, addr_ipv4_t *ipv4);

tbl_nexthop_t *
rt_fea_nexthop_ipv4_add(char *ifname, addr_ipv4_t *ipv4);

int32
rt_fea_ipv6_del_from_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, addr_ipv6_t *gate, nexthop_key_t *nhg, uint32 add_nh_num);

int32
rt_fea_ipv4_del_from_iproute_node_add_fail
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num);

int32
rt_fea_ipv4_del_from_iproute_node_add_fail_count
    (vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, addr_ipv4_t *gate, nexthop_key_t *nhg, uint32 add_nh_num);

int32
rt_fea_ipv6_add_connect_route_by_link(vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, char *ifname);

int32
rt_fea_ipv6_del_connect_route_by_link(vrf_id_t vrf_id, addr_ipv6_t *dest, uint8 prefixlen, char *ifname);

int32
rt_fea_ipv4_add_connect_route_by_link(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, char *ifname);

int32
rt_fea_ipv4_del_connect_route_by_link(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 prefixlen, char *ifname);

int32
rt_fea_ipv4_add_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen, 
                  uint32 ifindex, uint32 distance, uint32 nexthop_type, uint32 route_type);

int32
rt_fea_ipv4_del_route_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen, 
                  uint32 ifindex, uint32 distance, uint32 nexthop_type, uint32 route_type);

int32
rt_fea_ipv6_del_route_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen, 
                  uint32 ifindex, uint32 distance, uint32 nexthop_type);

int32
rt_fea_ipv4_del_route_multi_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance, uint32 route_type);

 int32
rt_fea_ipv6_del_route_multi_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance);

int32 
rt_fea_del_route_by_ifname(void *obj, tbl_iter_args_t *pargs);

int32
rt_fea_nexthop_ipv4_del(char *ifname, addr_ipv4_t *ipv4);

tbl_nexthop_t *
rt_fea_nexthop_ipv6_add(char *ifname, addr_ipv6_t *ipv6);

int32
rt_fea_nexthop_ipv6_del(char *ifname, addr_ipv6_t *ipv6);

int32
rt_fea_ipv6_del_addr(vrf_id_t vrf_id, ds_connected_v6_t *p_db_conn, uint32 ifindex);

int32
rt_fea_ipv4_add_addr(vrf_id_t vrf_id, ds_connected_t *p_db_conn, uint32 ifindex);

int32
rt_fea_ipv4_del_addr(vrf_id_t vrf_id, ds_connected_t *p_db_conn, uint32 ifindex);

uint32
rt_fea_v6_ecmp_member_count_get_max();

uint32
rt_fea_ecmp_member_count_get_max();

/* deleted arp and nexthop relation by liwh for bug 45444, 2017-10-26 */
#if 0
int32
rt_fea_arp_ipv4_add_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, uint32 ifindex, uint8 is_static);

int32
rt_fea_arp_ipv4_del_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static);
#endif
/* liwh end */
int32
rt_fea_ipv4_add_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 *macaddr, uint32 ifindex, uint8 is_static, char *if_name);

int32
rt_fea_ipv4_del_neighbor(vrf_id_t vrf_id, addr_ipv4_t *dest, uint8 is_static);

int32
rt_fea_ipv4_add_route_multi_nl(vrf_id_t vrf_id, addr_ipv4_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance, uint32 route_type);

int32
rt_fea_ipv6_add_route_multi_nl(vrf_id_t vrf_id, addr_ipv6_t *dest, nexthop_key_t *nhg,
                    uint32 multipath, uint8 prefixlen, uint32 distance);
#endif /* !__RT_FEA_H__ */

