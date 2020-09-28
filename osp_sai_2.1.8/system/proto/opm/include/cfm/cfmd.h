#ifndef __CFMD_H__
#define __CFMD_H__

#define CFM_LOG_EMERG(fmt, args...)    log_sys(M_MOD_INTERNAL, E_EMERGENCY, fmt, ##args)
#define CFM_LOG_ALERT(fmt, args...)    log_sys(M_MOD_INTERNAL, E_ALERT, fmt, ##args)
#define CFM_LOG_CRIT(fmt, args...)     log_sys(M_MOD_INTERNAL, E_CRITICAL, fmt, ##args)
#define CFM_LOG_ERR(fmt, args...)      log_sys(M_MOD_INTERNAL, E_ERROR, fmt, ##args)
#define CFM_LOG_WARN(fmt, args...)     log_sys(M_MOD_INTERNAL, E_WARNING, fmt, ##args)
#define CFM_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_INTERNAL, E_NOTICE, fmt, ##args)
#define CFM_LOG_INFO(fmt, args...)     log_sys(M_MOD_INTERNAL, E_INFORMATIONAL, fmt, ##args)

bool cfm_debug_is_on(uint32 flag);

#define CFM_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (cfm_debug_is_on(flag)) \
        log_sys(M_MOD_LOG, E_INFORMATIONAL, fmt, ##args); \
} while (0)

#define CFM_ERROR_INDEX_MAX  200

int32
cfm_cmd_process_cfm_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_process_cfm_md(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_process_cfm_ma(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_process_cfm_maid(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_process_cfm_lmep(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_process_cfm_rmep(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_show_cfm_domain(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_show_cfm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_show_cfm_cc_config(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_show_cfm_errors(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_clear_cfm_errors(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_show_cfm_mep(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
cfm_cmd_pm_process_cfm_rmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
cfm_cmd_pm_process_cfm_lmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
cfm_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);
#endif /* !__CFMD_H__ */
