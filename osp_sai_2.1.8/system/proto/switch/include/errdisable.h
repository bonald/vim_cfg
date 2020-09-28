#ifndef __ERRDISABLE_H__
#define __ERRDISABLE_H__
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"

#define GLB_ERRDISABLE_TIMER_INTERVAL  1

int32
errdisable_start(switch_master_t *master);

int32
errdisable_stop(switch_master_t *master);

int32
errdisable_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
errdisable_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
errdisable_handle_exception(errdis_rsn_t reason, tbl_interface_t *p_if, uint32 enable);

int32
errdisable_if_is_errdisable (tbl_interface_t *p_if);

int32
errdisable_is_recovery_en(errdis_rsn_t reason);

int32
errdisable_is_detect_en(errdis_rsn_t reason);

int32
errdisable_lacp_inherit_parent(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem);

int32 
errdisable_set_fdb_loop_detect_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable);

int32
errdisable_set_detect_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable);

int32
errdisable_set_recovery_en(tbl_errdisable_t *p_db_errdisable, uint32 is_enable);

int32
errdisable_set_errdisable_interval(uint32 interval);

int32
errdisable_set_flap(tbl_errdisable_flap_t *p_db_errdis_flap, uint32 count, uint32 second);

int32
errdisable_set_fdb_loop(tbl_errdisable_t *p_db_errdisable, uint32 maxsize, uint32 rate);

int32
errdisable_flap_init();

#endif
