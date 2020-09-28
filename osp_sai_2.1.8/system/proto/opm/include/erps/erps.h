#ifndef __ERPS_H__
#define __ERPS_H__

#define ERPS_LOG_EMERG(fmt, args...)    log_sys(M_MOD_INTERNAL, E_EMERGENCY, fmt, ##args)
#define ERPS_LOG_ALERT(fmt, args...)    log_sys(M_MOD_INTERNAL, E_ALERT, fmt, ##args)
#define ERPS_LOG_CRIT(fmt, args...)     log_sys(M_MOD_INTERNAL, E_CRITICAL, fmt, ##args)
#define ERPS_LOG_ERR(fmt, args...)      log_sys(M_MOD_INTERNAL, E_ERROR, fmt, ##args)
#define ERPS_LOG_WARN(fmt, args...)     log_sys(M_MOD_INTERNAL, E_WARNING, fmt, ##args)
#define ERPS_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_INTERNAL, E_NOTICE, fmt, ##args)
#define ERPS_LOG_INFO(fmt, args...)     log_sys(M_MOD_INTERNAL, E_INFORMATIONAL, fmt, ##args)

bool erps_debug_is_on(uint32 flag);

#define ERPS_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (erps_debug_is_on(flag)) \
        log_sys(M_MOD_LOG, E_INFORMATIONAL, fmt, ##args); \
} while (0)

bool erps_mode_is_huawei();

int32
erps_cmd_process_erps_domain(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
erps_add_erps_domain(uint16 domain_id, char* name);

int32
erps_del_erps_domain(uint16 domain_id);

int32
erps_get_check_erps_domain_ring_running(tbl_erps_domain_t *p_domain);

int32
erps_set_erps_domain_hello_time(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_set_erps_domain_fail_time(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_set_erps_domain_instance(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_get_erps_control_vlan_igsp_enable(vid_t vlanid);

int32
erps_set_erps_domain_primary_control_vlan(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_set_erps_domain_sub_control_vlan(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_set_erps_domain_enable(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain);

int32
erps_cmd_process_erps_ring(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
erps_add_erps_ring(tbl_erps_domain_t *p_db_domain, uint32 ring_id, uint32 level);

int32
erps_del_erps_ring(tbl_erps_domain_t *p_db_domain, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_node_role(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_edge_node_role(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_srpt_enable(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_state(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_primary_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_secondary_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_edge_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_common_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring);

int32
erps_set_erps_ring_blocked_interface(tbl_erps_ring_t *p_ring, tbl_erps_ring_t *p_db_ring, bool blocked);

int32
erps_clear_erps_ring_counters(tbl_erps_ring_t  *p_db_ring);

bool
erps_ring_check_edge_node_role_mux(uint8 role, erps_node_check_t check_type);

int32
erps_cmd_show_erps_list(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
erps_cmd_show_erps(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
erps_cmd_clear_erps_counters(char **argv, int32 argc, cfg_cmd_para_t *para);

uint32
erps_get_erps_domain_num(void);

int32 erps_cmd_set_erps_ring_enable(tbl_erps_ring_t *p_ring, 
    tbl_erps_ring_t *p_db_ring, cfg_cmd_para_t *para);

int32
erps_cmd_del_erps_ring(tbl_erps_domain_t *p_db_domain, uint32 ring_id, cfg_cmd_para_t *para);

int32
erps_fdb_flush_dynamic_fdb_port(uint32 ifindex);

int32
erps_fdb_flush_dynamic_fdb_all(void);

void erps_timer_hello_timer_fn(void *p_data);

void erps_timer_failure_timeout_pre_fn(void *p_data);

void erps_timer_failure_timer_fn(void *p_data);

void erps_timer_edge_hello_timer_fn(void *p_data);

void erps_timer_edge_failure_timer_fn(void *p_data);

void erps_msg_send_pkt(erps_msg_type_t type, tbl_interface_t* p_if, tbl_erps_domain_t *p_domain, tbl_erps_ring_t *p_ring);

char* erps_desc_node_role(erps_node_role_t role);

char* erps_desc_node_state(erps_state_t state);

void erps_timer_pre_forwarding_timer_fn(void *p_data);

int32 erps_check_one_domain_instnace(tbl_erps_domain_t *p_domain, tbl_erps_domain_t *p_db_domain,
    uint32 instance_id, bool add);

#endif /* !__ERPS_DEFINE_H__ */
