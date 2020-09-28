
#ifndef __SSM_H__
#define __SSM_H__


#define SSM_LOG_EMERG(fmt, args...)    log_sys(M_MOD_SSM, E_EMERGENCY, fmt, ##args)
#define SSM_LOG_ALERT(fmt, args...)    log_sys(M_MOD_SSM, E_ALERT, fmt, ##args)
#define SSM_LOG_CRIT(fmt, args...)     log_sys(M_MOD_SSM, E_CRITICAL, fmt, ##args)
#define SSM_LOG_ERR(fmt, args...)      log_sys(M_MOD_SSM, E_ERROR, fmt, ##args)
#define SSM_LOG_WARN(fmt, args...)     log_sys(M_MOD_SSM, E_WARNING, fmt, ##args)
#define SSM_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_SSM, E_NOTICE, fmt, ##args)
#define SSM_LOG_INFO(fmt, args...)     log_sys(M_MOD_SSM, E_INFORMATIONAL, fmt, ##args)

#define DBG_SSM_TYPE_FIELD 0
#define DBG_SSM_TYPE_ALL 1

int32
ssm_cmd_process_ssm_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ssm_cmd_process_ssm_port(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ssm_set_field_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
ssm_cmd_process_ssm_debug(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ssm_cmd_process_show_ssm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ssm_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
ssm_global_init();

int32
ssm_sync_init();
    
#endif /* !__SSM_H__ */

