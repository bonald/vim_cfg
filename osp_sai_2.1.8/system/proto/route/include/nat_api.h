
#ifndef __NAT_API_H__
#define __NAT_API_H__


extern int32 nat_api_set_if_nat(tbl_route_if_t *p_db_rt_if, uint8 enable_old, uint8 enable_new);
extern int32 nat_api_init(route_master_t *master);
extern bool  nat_api_if_need_proxy_arp(tbl_route_if_t *p_rtif, addr_ipv4_t *p_addr);
extern int32 nat_api_rtif_del(tbl_route_if_t *p_rt_if);
extern int32 nat_api_rtif_connected_add(tbl_interface_t *p_db_if, tbl_route_if_t *p_rt_if, ds_connected_t *p_conn);
extern int32 nat_api_rtif_connected_del(tbl_interface_t *p_db_if, tbl_route_if_t *p_rt_if, ds_connected_t *p_conn);

#endif /* !__NAT_API_H__ */

