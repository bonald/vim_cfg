
#ifndef __IPTABLES_H__
#define __IPTABLES_H__

extern int32
ipt_update_service_acl_port(uint32 service_type, uint32 port);

extern int32
ipt_del_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type);

extern int32
ipt_add_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type);

extern int32
ipt_del_service_acl_chain(uint32 service_type);

extern int32
ipt_add_service_acl_chain(uint32 service_type);

extern int32
ipt_del_service_acl(uint32 service_type);

extern int32
ipt_add_service_acl(tbl_acl_config_t *p_acl_config, uint32 service_type);

extern int32
ipt_service_acl_start(switch_master_t *master);

extern int32
ipt_prevent_set_small_pkt(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_small_pkt_len(tbl_iptables_prevent_t* p_ipt_prevent, uint16 len_val);

extern int32
ipt_prevent_set_pkt_statistic_update(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_maceq(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_ipeq(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_smurf(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_fraggle(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_icmp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_icmp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val);

extern int32
ipt_prevent_set_tcp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_tcp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val);

extern int32
ipt_prevent_set_udp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable);

extern int32
ipt_prevent_set_udp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val);

extern int32
ipt_prevent_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
ipt_prevent_start(switch_master_t *master);

extern int32
ipt_prevent_stop(switch_master_t *master);

#endif /* !__SWITCH_H__ */

