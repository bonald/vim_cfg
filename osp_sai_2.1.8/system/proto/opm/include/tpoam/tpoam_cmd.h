#ifndef __TPOAM_CMD_H__
#define __TPOAM_CMD_H__

int32
tpoam_cmd_clear_statistics_one(void *obj, tbl_iter_args_t *pargs);

int32
tpoam_cmd_process_tpoam_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
tpoam_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
tpoam_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
tpoam_cmd_process_exec(char **argv, int32 argc, cfg_cmd_para_t *para);

void
_tpoam_cli_lb_mep_reply_cb(void *p_data);

void
_tpoam_cli_lb_mip_reply_cb(void *p_data);

void
_tpoam_cli_lb_discovery_reply_cb(void *p_data);

int32
tpoam_cmd_process_oam_session_upload(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
tpoam_cmd_process_tpoam_exec_global(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__TPOAM_CMD_H__ */