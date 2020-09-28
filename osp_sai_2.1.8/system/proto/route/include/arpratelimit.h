#ifndef __ARPRATELIMIT_H__
#define __ARPRATELIMIT_H__

tbl_route_if_t * 
arpratelimit_get_l3_if_from_msg(uint32 vlanid_pkt, tbl_interface_t *p_db_if_input);

int32 
arprateimit_currpktnum_exceed_action(tbl_interface_t *p_db_if, tbl_route_if_t *p_rt_if);

int32
arpratelimit_cmd_clear_statistics();

int32 
arpratelimit_packet_process(uint32 vlanid_pkt, tbl_interface_t *p_db_if_input);

int32
_arpratelimit_cmd_show_interface(tbl_interface_t* p_db_if_input, cfg_cmd_para_t *para);

#endif /* !__ARPRATELIMIT_H__ */