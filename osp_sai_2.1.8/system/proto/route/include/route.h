
#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_kernel_if_define.h"

typedef struct
{
    char key_name[64]; /* cmd key string */
    char key_value[64];    /* cmd value string */ 
}new_cmd_args_key_value_t;


/* Quagga / FRR bandwidth */
#define RT_BANDWIDTH_DIVIDE_QUAGGA     1
#define RT_BANDWIDTH_DIVIDE_FRR        1000
#define RT_BANDWIDTH_DIVIDE            RT_BANDWIDTH_DIVIDE_FRR

extern tbl_route_debug_t *_g_p_tbl_route_debug;


#define RT_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_route_debug->module & RTDBG_FLAG_##typeenum)

#define RT_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_ROUTE, E_ERROR, fmt, ##args);

#define RT_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (RT_DBG_IS_ON(module, typeenum)) \
        RT_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define RT_FEA_DEBUG(fmt, args...) \
do { \
    RT_LOG_DEBUG(route, ROUTE_EVENT, fmt, ##args); \
} while(0)

/* for testing static route entry rount check by wangjj, 2015-09-16 */
#define RT_STATIC_MAX_NUM    1024

typedef struct route_master_s
{
} route_master_t;

int32
rt_vty_if_cmd(char *ifname, char *cmd);

int32
rt_vty_router_ospf_cmd(char *cmd);

int32
rt_vty_config_cmd(char *cmd);

int32
rt_vty_exec_cmd(char *cmd);

int32
rt_vty_init();

int32
rt_sync_init();

void
route_set_ipv6_enable(uint32 ipv6_enable);

int32 
route_if_start();

int32
rt_fea_init(route_master_t *master);

int32
rt_fpm_init();

uint32
rt_api_is_ipv6_profile();

int32
rt_cmd_process_ecmp_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_cmd_show_ecmp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_rr_prefix(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_if_cmd_process_route_if(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_kernel_if(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_route_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_cmd_process_arp_fib(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_user_cmd_process_arp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_cmd_process_arp_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arp_cmd_process_flusharp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_route(char **argv, int32 argc, cfg_cmd_para_t *para);

//int32
//rt_cmd_add_static_route_by_syscmd(prefix_t *dest, prefix_t *gate,
//                        uint8 prefixlen, uint32 distance, uint32 ifindex);

//int32
//rt_cmd_del_static_route_by_syscmd(prefix_t *dest, addr_ipv4_t *gate, uint8 prefixlen, uint32 ifindex);

int32
rt_delete_recursive_static_route(prefix_t *dest, prefix_t *gate);

int32
rt_cmd_process_static_route_cfg(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_iproute_node_add_fail_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_iproute_node_add_fail_count_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_static_rt_cnt(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_bgp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_bgp_network(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_bgp_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_rip(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_rip_network(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_rip_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_rip_passive_if(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ripng(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ripng_network(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ripng_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ripng_passive_if(char **argv, int32 argc, cfg_cmd_para_t *para);


int32
rt_cmd_process_ospf(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_network(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_area_auth(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_area_range(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf6_area_range(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_area_stub(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf6_area_stub(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_area_nssa(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf_area_vlink(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf6(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_ospf6_interface(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_nexthop(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_nh_group(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_cmd_process_cfg_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_cmd_process_check_l3if_exist(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_cmd_process_check_l3if_addr(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
if_cmd_process_check_mgmt_if_ip(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_show_if_brief(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_show_v6_if_brief(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_ping_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_ping_mgmt_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_traceroute_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_traceroute_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_traceroute_mgmt_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_tftp_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_ftp_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_ssh_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_telnet_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_show_ipv6_route(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_show_ip_route(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_process_clear_ipv6_route_static(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_add_if_addr_v6_by_syscmd(char *name, addr_ipv6_t *addr, uint32 mask_len);

int32
rt_cmd_del_if_addr_v6_by_syscmd(char *name, addr_ipv6_t *addr, uint32 mask_len);

int32
rt_cmd_add_if_addr_by_syscmd(char *name, addr_ipv4_t *addr, uint32 mask_len, uint32 update);

int32
rt_cmd_del_if_addr_by_syscmd(char *name, addr_ipv4_t *addr, uint32 mask_len, uint32 update);

uint32 
rt_cli_get_ipv4_prefix(char *addr);

uint32 
rt_fea_get_route_count_global();

int32
rt_fea_static_max_cfg_check(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_cmd_process_nd_fib(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_cmd_show_nd(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
nd_cmd_clear_nd(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
route_cmd_ipv4_add_nhg(nexthop_ecmp_routed_t *nh_group, bool max_check, uint32 is_ipv4);

int32
route_cmd_ipv4_del_nhg(nexthop_ecmp_routed_t *nh_group, uint32 is_ipv4);

#endif /* !__ROUTE_H__ */

