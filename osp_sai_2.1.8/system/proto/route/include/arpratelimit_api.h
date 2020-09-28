#ifndef __ARPRATELIMIT_API_H__
#define __ARPRATELIMIT_API_H__

tbl_route_if_t*
tbl_route_if_get_route_if_by_name(char* ifname);

int32
arpratelimit_is_arpratelimit_l3_port(tbl_interface_t *p_db_if);

void
arpratelimit_init_para_for_add_if(tbl_route_if_t *p_rt_if);

int32
arpratelimit_set_exceed_action(tbl_route_if_t *p_db_rtif, uint8 action);

int32
arpratelimit_set_pkt_max(tbl_route_if_t *p_db_rtif, uint32 pktmax);

int32
arpratelimit_set_enable(tbl_route_if_t *p_db_rtif, uint8 enable);

#endif /* !__ARPRATELIMIT_API_H__ */