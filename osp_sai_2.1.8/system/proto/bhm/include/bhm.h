
#ifndef __BHM_H__
#define __BHM_H__


int32
bhm_api_hw_feed_disable();

int32
bhm_api_set_hw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable);

int32
bhm_api_set_sw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable);

int32
bhm_api_set_sw_reactive(tbl_bhm_global_t *p_db_bhm, uint32 reactive);

int32
bhm_module_init();

int32
bhm_sock_deinit();

int32
bhm_cmd_process_bhm_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
bhm_cmd_process_bhm_module(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
bhm_cmd_process_reboot(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__BHM_H__ */

