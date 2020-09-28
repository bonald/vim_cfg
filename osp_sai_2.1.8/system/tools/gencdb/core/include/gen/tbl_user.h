
#ifndef __TBL_USER_H__
#define __TBL_USER_H__

int32
tbl_user_add_user_sync(tbl_user_t *p_user, uint32 sync);

int32
tbl_user_del_user_sync(tbl_user_key_t *p_user_key, uint32 sync);

int32
tbl_user_set_user_field_sync(tbl_user_t *p_user, tbl_user_field_id_t field_id, uint32 sync);

int32
tbl_user_add_user(tbl_user_t *p_user);

int32
tbl_user_del_user(tbl_user_key_t *p_user_key);

int32
tbl_user_set_user_field(tbl_user_t *p_user, tbl_user_field_id_t field_id);

tbl_user_t*
tbl_user_get_user(tbl_user_key_t *p_user_key);

char*
tbl_user_key_val2str(tbl_user_t *p_user, char *str, uint32 str_len);

int32
tbl_user_key_str2val(char *str, tbl_user_t *p_user);

int32
tbl_user_dump_one(tbl_user_t *p_user, tbl_iter_args_t *pargs);

char*
tbl_user_key_name_dump(tbl_user_t *p_user, char* str);

char*
tbl_user_key_value_dump(tbl_user_t *p_user, char* str);

char*
tbl_user_field_name_dump(tbl_user_t *p_user, int32 field_id, char* str);

char*
tbl_user_field_value_dump(tbl_user_t *p_user, int32 field_id, char* str);

char**
tbl_user_table_dump(tbl_user_t *p_user, char** str);

int32
tbl_user_table_parser(char** array, char* key_value, tbl_user_t *p_user);

int32
tbl_user_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_user_master_t*
tbl_user_get_master();

tbl_user_master_t*
tbl_user_init_user();

#endif /* !__TBL_USER_H__ */

