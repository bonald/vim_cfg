
#ifndef _IPMC_H
#define _IPMC_H

#include "ipmc_define.h"

int32
ipmc_api_set_ipmc_global_field(tbl_ipmc_global_t *p_db_glb, int32 field_id, cfg_cmd_para_t *para);

int32
ipmc_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipmc_cmd_process_intf(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipmc_cmd_process_static_rp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipmc_cmd_process_mrt(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipmc_start();

/*add by zhw for ipmc*/
int32
rt_cmd_process_show_ip_mroute(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_pim_sm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_pim_sm_bsr(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_pim_sm_rp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_pim_sm_rp_gp(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_pim_dm(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
rt_cmd_process_igmp_intf(char **argv, int32 argc, cfg_cmd_para_t *para);

/*end add zhw*/
#endif /* _IPMC_H */

