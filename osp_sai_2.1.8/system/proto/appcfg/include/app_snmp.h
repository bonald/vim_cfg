#ifndef __APP_SNMP_H__
#define __APP_SNMP_H__

int32
app_snmp_init();

int app_snmp_free_secidx(int idx);
int app_snmp_alloc_secidx();

int32
app_snmp_proc_startup_done();

int32
app_snmp_community_set(tbl_snmp_community_field_id_t field_id, tbl_snmp_community_t *pitem);

int32
app_snmp_view_set(tbl_snmp_view_field_id_t field_id, tbl_snmp_view_t *pitem);

int32
app_snmp_set(tbl_snmp_cfg_field_id_t field_id, tbl_snmp_cfg_t *pitem);

int32
app_snmp_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_view_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_community_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_trap_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_inform_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_context_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_usm_user_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_group_usm_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_access_usm_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_notify_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_target_addr_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_snmp_target_params_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);


#endif /* !__APP_SNMP_H__ */

