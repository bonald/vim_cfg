#ifndef __ARPACL_H__
#define __ARPACL_H__

int32
arpacl_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpacl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
arpacl_config_add_arpacl(char *arpacl_name);

int32
arpacl_config_del_arpacl(char *arpacl_name);

int32
arpacl_check_ace_config(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs);

int32
arpace_get_new_seq_ref(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs);

int32
arpace_config_add_arpace(tbl_arpace_config_t *p_arpace_config);

int32
arpace_config_del_arpace(char *arpacl_name, uint32 p_seq_no);

bool
arpacl_config_is_used(char *arpacl_name);

int32
arpacl_config_clear_arpace(char *arpacl_name);

int32
arpacl_access_list_filter(char* arpacl_name, arp_eth_t *arp_p);

int32
arpacl_start(route_master_t *master);

int32
arpacl_stop(route_master_t *master);

#endif /* !__ARPACL_H__ */
