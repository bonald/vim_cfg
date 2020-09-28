#ifndef __RT_IF_H__
#define __RT_IF_H__

#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/ds_connected_define.h"
#include "gen/ds_connected.h"

#define RT_IF_CMD_CHECK_RT_IF(_ifname_, _para_, _pp_if_, _pp_rt_if_) \
do { \
    rc = rt_if_cmd_check_l3_cmd(_ifname_, _para_, _pp_if_, _pp_rt_if_); \
    if (rc < 0) \
    { \
        return rc; \
    } \
} while (0)

int32
rt_if_set_mode_set_before(uint8 mode_set_before, tbl_route_if_t *p_db_route_if, cfg_cmd_para_t *para);

int32
rt_if_indirect_ip_route_inc(tbl_interface_t *p_db_if, uint32 is_ipv4, uint32 is_link_local);

void
rt_if_indirect_ip_route_dec(tbl_interface_t *p_db_if, uint32 is_ipv4, uint32 is_link_local);

int32
ipv6_address_del(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, uint32 commit_frr);

tbl_route_if_t *
if_match_all_by_ipv6_address (addr_ipv6_t *addr_ipv6);

tbl_route_if_t*
rt_interface_check_rif_exist(tbl_interface_t *p_db_if);

int32
rt_if_set_mode(char *name, uint32 mode);

int32
rt_if_addr_del_update(tbl_route_if_t *p_db_rtif, ds_connected_t *ifc);

int32
rt_if_add_if(char *name);

int32
rt_if_del_if(char *name);

int32
static_delete_by_if (afi_t afi, char *ifname);

int32
rt_if_del_if_addr(char *ifname);

int32
if_get_ifc_ipv6_num(tbl_route_if_t * p_db_rtif);

ds_connected_v6_t *
if_lookup_ifc_ipv6_linklocal (tbl_route_if_t *ifp);

void
connected_invalid_ipv6 (tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn);

int32
rt_if_del_all_ipv4(tbl_route_if_t *p_db_rtif);

int32
rt_if_add_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn, mac_addr_t vmac);

int32
rt_if_add_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn, mac_addr_t vmac);

int32
rt_if_del_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_conn);

int32
rt_if_rt_entry_v6_delete_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv6_t *nh_addr);

int32
rt_if_del_all_ipv6(tbl_route_if_t *p_db_rtif, int32 disable, uint32 delete_by_cmd);

int32
rt_if_clear_rtif_urpf_en(tbl_route_if_t *p_db_rtif);

int32
rt_if_clear_rtif_urpf_en_v6(tbl_route_if_t *p_db_rtif);

int32 
rt_if_is_loopback(char *ifname);

ds_connected_v6_t *
connected_check_ipv6 (tbl_route_if_t *p_db_rtif, prefix_t *p);

int32
rt_if_del_ipv4(tbl_route_if_t *p_db_rtif, ds_connected_t *p_conn);

int32
rt_if_del_all_ipv4_secondary(tbl_route_if_t *p_db_rtif);

int32
rt_if_clear_if(tbl_interface_t *p_db_if);

int32
rt_if_clear_ospf_if(tbl_route_if_t *p_db_rtif);

int32
rt_if_clear_kernel_if_by_ifname(void *obj, tbl_iter_args_t *pargs);

int32
rt_if_ipv4_del_related_route_by_link(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, uint32 new_up_running);

int32
rt_if_ipv6_del_related_route_by_link(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, uint32 new_up_running);

int32
rt_if_up_update(char *ifname, uint32 new_up_running);

int32
rt_if_running_change(tbl_interface_t *p_db_if, uint32 old_running, uint32 new_running);

int32
rt_if_up_change(tbl_interface_t *p_db_if, uint32 old_up, uint32 new_up);

int32
rt_if_add_static_route_v6_by_syscmd(addr_ipv6_t *dest, addr_ipv6_t *gate,
                        uint8 prefixlen, uint32 distance, uint32 ifindex);

int32
rt_if_del_static_route_v6_by_syscmd(addr_ipv6_t *dest, addr_ipv6_t *gate,
                        uint8 prefixlen, uint32 ifindex);

int32
rt_if_add_static_route_by_syscmd(addr_ipv4_t *dest, addr_ipv4_t *gate,
                        uint8 prefixlen, uint32 distance);

int32
rt_if_del_static_route_by_syscmd(addr_ipv4_t *dest, addr_ipv4_t *gate, uint8 prefixlen);

int32
rt_if_rt_entry_by_ifaddr(tbl_route_if_t *p_db_rtif, addr_ipv4_t *nh_addr);

int32
rt_if_connected_down_ipv4(tbl_interface_t *p_db_if, tbl_route_if_t *p_db_rtif, ds_connected_t *p_db_conn);

int32
rt_if_cmd_process_route_if(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_if_set_mtu(char *ifname, uint32 mtu);

int32 
rt_if_mtu_update_agg_member(tbl_interface_t *pst_interface, uint32 mtu);

int32
rt_if_set_arp_retry_interval(char *ifname, uint32 retry_interval);

int32
rt_if_cmd_set_arp_ageing_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout);

int32
rt_if_set_arp_timeout(char *ifname, uint32 timeout);

int32
rt_if_set_ucast_rpf_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_ucast_rpf_en_v6(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_rip_recv_version(tbl_route_if_t *p_db_rtif, uint8 recv_version);

int32
rt_if_set_rip_send_version(tbl_route_if_t *p_db_rtif, uint8 send_version);

int32
rt_if_set_rip_v2_broadcast(tbl_route_if_t *p_db_rtif, uint8 v2_broadcast);

int32
rt_if_set_rip_split_horizon(tbl_route_if_t *p_db_rtif, uint8 split_horizon);

int32
rt_if_set_rip_poisoned_reverse(tbl_route_if_t *p_db_rtif, uint8 poisoned_reverse);

int32
rt_if_set_rip_auth_mode(tbl_route_if_t *p_db_rtif, uint8 auth_mode);

int32
rt_if_set_rip_auth_string(tbl_route_if_t *p_db_rtif, char* auth_string);

int32
rt_if_set_ip_unreachable_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_ospf_mtu_ignore_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_ospf_hello_interval(tbl_route_if_t *p_db_rtif, uint32 interval);

int32
rt_if_set_ospf_dead_interval(tbl_route_if_t *p_db_rtif, uint32 interval);

int32
rt_if_set_ospf_cost(tbl_route_if_t *p_db_rtif, uint32 cost);

int32
rt_if_set_ospf_network_type(tbl_route_if_t *p_db_rtif, uint32 type);

int32
rt_if_set_ospf_auth_type(tbl_route_if_t *p_db_rtif, uint32 type);

int32
rt_if_set_ospf_auth_key(tbl_route_if_t *p_db_rtif, char *key);

int32
rt_if_add_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth);

int32
rt_if_del_ospf_auth_md5_key(tbl_route_if_t *p_db_rtif, ds_ospf_auth_t *p_auth);

int32
rt_if_del_all_ospf_auth_md5(tbl_route_if_t *p_db_rtif);

int32
rt_if_set_ip_redirects_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_ospf_bandwidth(tbl_interface_t * p_db_if, uint32 ospf_bandwith);

int32
rt_if_set_local_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_arp_proxy(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_arp_retry_timeout(tbl_route_if_t *p_db_rtif, uint32 timeout);

int32
rt_if_set_dhcp_relay_option_trust(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_dhcp_relay_option_trust(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_dhcp_server_group(tbl_route_if_t *p_db_rtif, uint8 index);

int32
rt_if_set_dhcp_client_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_set_nei_retrans(tbl_route_if_t *p_db_rtif, uint8 timelen);

int32
rt_if_after_add_agg_member (tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

int32
rt_if_check_before_add_agg_member(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

int32
rt_if_before_destroy_agg(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param);

int32
rt_if_is_nonmirror_running(uint32 flags, uint32 mirror_enable);

int32
rt_if_del_all_arp(tbl_interface_t *p_db_if);

int32
rt_if_cmd_process_route_if_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
rt_if_set_vmac(char *ifname, mac_addr_t  vmac, bool add);

int32
rt_if_add_vip_and_arp(char *ifname, mac_addr_t vmac, ds_connected_t *p_conn);

int32
rt_if_del_vip_and_arp(char *ifname, ds_connected_t *p_conn);

ds_connected_v6_t *
if_match_ifc_ipv6_direct (tbl_route_if_t *ifp, prefix_t *p);

ds_connected_v6_t *
if_lookup_ifc_ipv6_linklocal (tbl_route_if_t *ifp);

int32
if_match_active_ifc_ipv6_num (tbl_route_if_t *p_db_rtif, prefix_t *p);

int32
rt_if_connected_valid_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_ifc);

int32
rt_if_connected_invalid_ipv6(tbl_route_if_t *p_db_rtif, ds_connected_v6_t *p_ifc, bool delete_addr);

int32
hal_l3if_set_ipv6_ucast_en(tbl_route_if_t * p_route_if, uint32 ipv6_unicast_en);

/* modified by liwh for bug 49365, 2018-11-27 */
int32
rt_if_set_dhcpv6_client_en(tbl_route_if_t *p_db_rtif, uint8 enable);

int32
rt_if_running_update(char *ifname, uint32 new_up_running);
/*liwh end */

int32
rt_if_set_nat_en(tbl_route_if_t *p_db_rtif, uint8 enable);

/*add by zhw for mc*/
int32
rt_if_set_pim_mode(tbl_route_if_t *p_db_rtif, uint8 pim_mode);

int32
rt_if_set_multicast(tbl_route_if_t *p_db_rtif, uint8 mode);
/*end add zhw*/
#endif /* !__RT_IF_H__ */

