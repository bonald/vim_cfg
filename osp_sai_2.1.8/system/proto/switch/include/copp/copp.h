#ifndef __COPP_H__
#define __COPP_H__


#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"

int32
copp_start(switch_master_t *master);

int32
copp_stop(switch_master_t *master);

int32
copp_cfg_global_set_total_rate(uint32 rate);

int32
copp_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__COPP_H__ */

