
#ifndef __TBL_CLEAR_ACL_POLICY_H__
#define __TBL_CLEAR_ACL_POLICY_H__

int32
tbl_clear_acl_policy_set_clear_acl_policy_field_sync(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_clear_acl_policy_field_id_t field_id, uint32 sync);

int32
tbl_clear_acl_policy_set_clear_acl_policy_field(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_clear_acl_policy_field_id_t field_id);

tbl_clear_acl_policy_t*
tbl_clear_acl_policy_get_clear_acl_policy();

char*
tbl_clear_acl_policy_key_val2str(tbl_clear_acl_policy_t *p_clear_acl_policy, char *str, uint32 str_len);

int32
tbl_clear_acl_policy_key_str2val(char *str, tbl_clear_acl_policy_t *p_clear_acl_policy);

int32
tbl_clear_acl_policy_dump_one(tbl_clear_acl_policy_t *p_clear_acl_policy, tbl_iter_args_t *pargs);

char*
tbl_clear_acl_policy_key_name_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char* str);

char*
tbl_clear_acl_policy_key_value_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char* str);

char*
tbl_clear_acl_policy_field_name_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, int32 field_id, char* str);

char*
tbl_clear_acl_policy_field_value_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, int32 field_id, char* str);

char**
tbl_clear_acl_policy_table_dump(tbl_clear_acl_policy_t *p_clear_acl_policy, char** str);

int32
tbl_clear_acl_policy_table_parser(char** array, char* key_value, tbl_clear_acl_policy_t *p_clear_acl_policy);

int32
tbl_clear_acl_policy_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_clear_acl_policy_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_clear_acl_policy_t*
tbl_clear_acl_policy_init_clear_acl_policy();

#endif /* !__TBL_CLEAR_ACL_POLICY_H__ */

