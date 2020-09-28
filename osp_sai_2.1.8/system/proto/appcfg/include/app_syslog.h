#ifndef __APP_SYSLOG_H__
#define __APP_SYSLOG_H__

int32
app_syslog_init();

int32
app_syslog_proc_startup_done();

int32
app_syslog_set_merge_timeout(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_merge_fifosize(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_merge(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_server(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_file(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_operate(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_trap(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_enable_diag(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_bufsz(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_server_addr(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_trap_facility(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_trap_severity(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_server_facility(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_server_severity(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_timestamp(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_file_severity(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_set_module_severity(tbl_syslog_cfg_t *p_cfg);

int32
app_syslog_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_syslog_show_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_syslog_act_process(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__APP_SYSLOG_H__ */

