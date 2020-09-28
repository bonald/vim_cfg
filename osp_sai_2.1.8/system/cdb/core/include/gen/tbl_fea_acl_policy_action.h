
#ifndef __TBL_FEA_ACL_POLICY_ACTION_H__
#define __TBL_FEA_ACL_POLICY_ACTION_H__

int32
tbl_fea_acl_policy_action_add_fea_acl_policy_action_sync(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, uint32 sync);

int32
tbl_fea_acl_policy_action_del_fea_acl_policy_action_sync(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key, uint32 sync);

int32
tbl_fea_acl_policy_action_set_fea_acl_policy_action_field_sync(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_fea_acl_policy_action_field_id_t field_id, uint32 sync);

int32
tbl_fea_acl_policy_action_add_fea_acl_policy_action(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action);

int32
tbl_fea_acl_policy_action_del_fea_acl_policy_action(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key);

int32
tbl_fea_acl_policy_action_set_fea_acl_policy_action_field(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_fea_acl_policy_action_field_id_t field_id);

tbl_fea_acl_policy_action_t*
tbl_fea_acl_policy_action_get_fea_acl_policy_action(tbl_fea_acl_policy_action_key_t *p_fea_acl_policy_action_key);

char*
tbl_fea_acl_policy_action_key_val2str(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char *str, uint32 str_len);

int32
tbl_fea_acl_policy_action_key_str2val(char *str, tbl_fea_acl_policy_action_t *p_fea_acl_policy_action);

int32
tbl_fea_acl_policy_action_dump_one(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, tbl_iter_args_t *pargs);

char*
tbl_fea_acl_policy_action_key_name_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char* str);

char*
tbl_fea_acl_policy_action_key_value_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char* str);

char*
tbl_fea_acl_policy_action_field_name_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, int32 field_id, char* str);

char*
tbl_fea_acl_policy_action_field_value_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, int32 field_id, char* str);

char**
tbl_fea_acl_policy_action_table_dump(tbl_fea_acl_policy_action_t *p_fea_acl_policy_action, char** str);

int32
tbl_fea_acl_policy_action_table_parser(char** array, char* key_value, tbl_fea_acl_policy_action_t *p_fea_acl_policy_action);

int32
tbl_fea_acl_policy_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_acl_policy_action_master_t*
tbl_fea_acl_policy_action_get_master();

tbl_fea_acl_policy_action_master_t*
tbl_fea_acl_policy_action_init_fea_acl_policy_action();

#endif /* !__TBL_FEA_ACL_POLICY_ACTION_H__ */

