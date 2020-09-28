
#ifndef __VRRP_API_H__
#define __VRRP_API_H__

uint32 vrrp_compute_masterdowninterval(tbl_vrrp_session_t *p_db_sess, uint16 advt_interval, uint8 priority); 

int32 vrrp_reset_virtual_ip (tbl_vrrp_session_t *p_db_sess);

void vrrp_addr_subnet_match(char *ifname, addr_t *vip, bool *is_owner, bool *is_submatch);

int32 vrrp_shutdown_sess(tbl_vrrp_session_t *p_db_sess);

int32 vrrp_noshut_sess(tbl_vrrp_session_t *p_db_sess);

int32 
vrrp_sys_init_timer(tbl_vrrp_global_t *p_db_vrrp_glb, bool start);

int32 
vrrp_sys_init_msec_timer(tbl_vrrp_global_t *p_db_vrrp_glb, bool start);

int32
vrrp_send_advt(tbl_vrrp_session_t *p_db_sess);

void
vrrp_get_interface_primary_ip(char *ifname, uint32 *primary_ip);

int32
vrrp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int
vrrp_fe_state_change(tbl_vrrp_session_t  *p_db_sess, 
                        vrrp_state_t curr_state, 
                        vrrp_state_t new_state, 
                        uint8 owner);

int32 
vrrp_get_master_router_mac(tbl_interface_t *p_db_if, uint32 *addr, uint8 *vmac);

void
vrrp_packet_dump(struct vrrp_advt_info *p_vrrp_advt);

int32
vrrp_if_up(tbl_interface_t *p_db_if);

int32
vrrp_if_down(tbl_interface_t *p_db_if);

int32
vrrp_ip_address_interface_del(tbl_route_if_t *p_db_rtif);

uint32
vrrp_ip_address_check(tbl_route_if_t *p_db_rtif);

uint8
vrrp_adjust_priority (tbl_vrrp_session_t *p_db_sess);

int32
vrrp_api_set_vrrp_priority_bytrack(u_int16_t vrid, uint8 track_state);

int32
vrrp_disable_track_object(tbl_vrrp_session_t  *p_db_sess);

#endif /* !__VRRP_API_H__ */

