
#ifndef __IPSLA_API_H__
#define __IPSLA_API_H__

uint32
ipsla_debug_is_on(uint32 flag);

int32
ipsla_check_ip(struct sal_in4_addr *addr);

uint16
ipsla_cal_chksum(unsigned short *addr,int len);

int32
ipsla_adjust_probe_result(tbl_ipsla_entry_t *p_db_entry, int32 config_cnt);

int32
ipsla_adjust_test_result(tbl_ipsla_entry_t *p_db_entry, int32 config_cnt);

void
ipsla_process_probe_result(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_save_test_result(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_send_icmp_packet(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_probe_start (tbl_ipsla_entry_t *p_db_entry);

void
ipsla_update_entry_status(tbl_ipsla_entry_t *p_db_entry);

void
ipsla_timer(tbl_ipsla_global_t *p_db_ipsla_glb);

void
ipsla_sys_timer_update(void *p_data);

int32
ipsla_sys_init_timer(tbl_ipsla_global_t *p_db_ipsla_glb, bool start);

int32
ipsla_enable_entry(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_disable_entry(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_process_src_intf_delete(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_update_src_intf_delete(tbl_interface_t *p_db_if);

int32
ipsla_create_socket(tbl_ipsla_entry_t *p_db_entry);

int32
ipsla_packet_new(tbl_ipsla_entry_t *p_db_entry);

void
ipsla_ip_recv_packet(void *p_data);

int32
ipsla_start();

#endif /* !__IPSLA_API_H__ */

