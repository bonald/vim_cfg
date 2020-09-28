
#ifndef __PTP_H__
#define __PTP_H__

int32
ptp_sync_init();

int32
ptp_cmd_process_ptp_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_ptp_port(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_ptp_foreign(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_ptp_residence_time_cache(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_ptp_debug(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ptp_cmd_process_clear(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__PTP_H__ */

