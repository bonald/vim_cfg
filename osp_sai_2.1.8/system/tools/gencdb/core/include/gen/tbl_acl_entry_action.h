
#ifndef __TBL_ACL_ENTRY_ACTION_H__
#define __TBL_ACL_ENTRY_ACTION_H__

int32
tbl_acl_entry_action_add_acl_entry_action_sync(tbl_acl_entry_action_t *p_acl_entry_action, uint32 sync);

int32
tbl_acl_entry_action_del_acl_entry_action_sync(tbl_acl_entry_action_key_t *p_acl_entry_action_key, uint32 sync);

int32
tbl_acl_entry_action_set_acl_entry_action_field_sync(tbl_acl_entry_action_t *p_acl_entry_action, tbl_acl_entry_action_field_id_t field_id, uint32 sync);

int32
tbl_acl_entry_action_add_acl_entry_action(tbl_acl_entry_action_t *p_acl_entry_action);

int32
tbl_acl_entry_action_del_acl_entry_action(tbl_acl_entry_action_key_t *p_acl_entry_action_key);

int32
tbl_acl_entry_action_set_acl_entry_action_field(tbl_acl_entry_action_t *p_acl_entry_action, tbl_acl_entry_action_field_id_t field_id);

tbl_acl_entry_action_t*
tbl_acl_entry_action_get_acl_entry_action(tbl_acl_entry_action_key_t *p_acl_entry_action_key);

char*
tbl_acl_entry_action_key_val2str(tbl_acl_entry_action_t *p_acl_entry_action, char *str, uint32 str_len);

int32
tbl_acl_entry_action_key_str2val(char *str, tbl_acl_entry_action_t *p_acl_entry_action);

int32
tbl_acl_entry_action_dump_one(tbl_acl_entry_action_t *p_acl_entry_action, tbl_iter_args_t *pargs);

char*
tbl_acl_entry_action_key_name_dump(tbl_acl_entry_action_t *p_acl_entry_action, char* str);

char*
tbl_acl_entry_action_key_value_dump(tbl_acl_entry_action_t *p_acl_entry_action, char* str);

char*
tbl_acl_entry_action_field_name_dump(tbl_acl_entry_action_t *p_acl_entry_action, int32 field_id, char* str);

char*
tbl_acl_entry_action_field_value_dump(tbl_acl_entry_action_t *p_acl_entry_action, int32 field_id, char* str);

char**
tbl_acl_entry_action_table_dump(tbl_acl_entry_action_t *p_acl_entry_action, char** str);

int32
tbl_acl_entry_action_table_parser(char** array, char* key_value, tbl_acl_entry_action_t *p_acl_entry_action);

int32
tbl_acl_entry_action_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_entry_action_master_t*
tbl_acl_entry_action_get_master();

tbl_acl_entry_action_master_t*
tbl_acl_entry_action_init_acl_entry_action();

#endif /* !__TBL_ACL_ENTRY_ACTION_H__ */

