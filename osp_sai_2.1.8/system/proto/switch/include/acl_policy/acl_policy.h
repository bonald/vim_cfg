#ifndef __ACL_POLICY__
#define __ACL_POLICY__

extern int32
policy_map_attach_interface(char *policy_map_name, char *interface_name, bool direct_in);

extern int32
policy_map_detach_interface(char *policy_map_name, char *interface_name, bool direct_in);

extern int32
class_attach_policy(char *policy_map_name, char *class_map_name, cfg_cmd_para_t *para);

extern int32
class_detach_policy(char *policy_map_name, char *class_map_name);

extern int32
acl_attach_class(char *class_map_name, char *acl_group_name, cfg_cmd_para_t *para);

extern int32
acl_detach_class(char *class_map_name, char *acl_group_name);

extern int32
class_map_action_show(cfg_cmd_para_t *para);

extern int32
class_map_action_update(char *policy_map_name, char *class_map_name, uint32 field_id, char* value, cfg_cmd_para_t *para);

extern int32 
acl_policy_show_acl_policy(FILE* fp, char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_policy_clear_acl_policy(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
policy_map_config_add_policy_map(char *policy_map_name);

extern int32
policy_map_config_del_policy_map(char *policy_map_name);

extern int32
class_map_config_add_class_map(char *class_map_name);

extern int32
class_map_config_del_class_map(char *class_map_name);

extern int32
acl_policy_operation(char* interface_name, uint32 direct_in, char* policy_map_name, char* class_map_name, char* acl_name, uint32 seq_no,
                    glb_acl_policy_operation_type_t op_type, glb_acl_policy_op_action_type_t op_action);

extern int32
acl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
policy_map_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
class_map_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
class_in_policy_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_in_class_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
class_map_action_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
clear_acl_policy_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_cmd_show_acl_policy_stats(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_cmd_show_policy_map(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_cmd_show_class_map(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
acl_is_redirected_interface(tbl_interface_t *p_db_if);

extern int32 acl_policy_start(switch_master_t *master);

extern int32 acl_policy_clear_statistics(char *ifname, bool is_input, cfg_cmd_para_t *para);

int32
acl_api_if_del_cb(tbl_interface_t *p_db_if);

int32
acl_policy_show_policy_map(tbl_policy_map_config_t *p_db_pmap_config, cfg_cmd_para_t *para);

int32
acl_policy_show_class_map(tbl_class_map_config_t *p_db_cmap_config, cfg_cmd_para_t *para);

int32
acl_policy_get_tcp_flags_config_count();

#endif


