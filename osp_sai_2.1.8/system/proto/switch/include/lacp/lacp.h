#ifndef __LACP_H__
#define __LACP_H__
extern uint32 g_lacp_debug_flag;

#define LACP_LOG_DEBUG(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(LACP, lacp, typeenum, fmt, ##args);\
}while(0)

#define LACP_DEBUG_FUNC(typeenum) \
    LACP_LOG_DEBUG(typeenum, "Enter into %s.", __FUNCTION__)

#define LACP_DEBUG(typeenum, fmt, args...) \
do {\
    LACP_LOG_DEBUG(typeenum, fmt, ##args);\
}while(0)

int32
lacp_add_if_lacp(tbl_interface_t *p_db_if_agg, tbl_interface_t *p_db_if_mem);

int32
lacp_del_if_lacp(tbl_interface_t *p_db_if_mem);

int32
lacp_set_port_priority(tbl_interface_t *p_db_if, uint16 port_priority);

int32
lacp_set_port_short_timeout(tbl_interface_t *p_db_if, uint32 enable);

int32
lacp_set_system_priority(uint16 system_priority);

int32
lacp_set_system_id(const uint8 *system_id);

int32
lacp_set_hash_arithmetic(const uint8 hash_arithmetic_type);

int32
lacp_set_load_balance_inner_field_en(const uint32 load_balance_inner_en);

int32
lacp_set_load_balance(uint32 load_balance_flag);

int32
lacp_set_load_balance_mode(uint8 channel_group_id,uint8 load_balance_mode);

int32
lacp_clear_counters(uint32 lag_id);

int32
lacp_cmd_process_lag_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
lacp_cmd_process_clear_counters(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
lacp_init_default_config(uint8 *system_id);

int32
lacp_start(switch_master_t *master);

int32
lacp_stop(switch_master_t *master);

int32
lacp_if_change_process(tbl_interface_t *p_db_if);

int32
lacp_set_port_unselected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_record_pdu(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu);

int32
lacp_record_default(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_update_selected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu);

int32
lacp_update_default_selected(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_update_ntt(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp, lacp_pdu_t *p_pdu);

int32
lacp_enable_collecting_distributing(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_disable_collecting_distributing(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_attach_mux_to_aggregator(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_detach_mux_from_aggregator(tbl_interface_t *p_db_if, ds_lacp_t *p_db_lacp);

int32
lacp_set_mlag_sysid(uint8 *mlag_sysid);

int32
lacp_set_mlag_syspri(uint16 mlag_syspri);

int32
lacp_set_mlag_id(tbl_interface_t *p_db_if_agg, uint32 mlag_id);

#endif /* !__LACP_H__ */
