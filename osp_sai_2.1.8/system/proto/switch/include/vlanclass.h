#ifndef __VLANCLASS_H__
#define __VLANCLASS_H__


int32
vlanclass_start(switch_master_t *master);

int32
vlanclass_stop(switch_master_t *master);

int32
vlanclass_add_rule(tbl_vlanclass_rule_t *p_vclass_rule);

int32
vlanclass_del_rule(uint32 rule_id);

int32
vlanclass_del_group(tbl_vlanclass_group_t* p_vclass_group);

int32
vlanclass_add_rule_to_group(uint32 group_id, tbl_vlanclass_rule_t *p_vclass_rule);

int32
vlanclass_del_rule_from_group(tbl_vlanclass_group_t* p_vclass_group, uint32 rule_id);

int32
vlanclass_set_entry_dynamic_count(tbl_vlanclass_rule_t* p_db_vclass_rule, bool add);

int32
vlanclass_cmd_process_vlanclass_rule(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanclass_cmd_process_vlanclass_group(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanclass_cmd_process_vlanclass_rule_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanclass_cmd_process_vlanclass_group_show(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanclass_cmd_process_vlanclass_if_show(char **argv, int32 argc, cfg_cmd_para_t *para);


#endif /* __VLANCLASS_H__ */

