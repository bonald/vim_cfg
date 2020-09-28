
#ifndef __TBL_LOGIN_H__
#define __TBL_LOGIN_H__

int32
tbl_login_add_login_sync(tbl_login_t *p_login, uint32 sync);

int32
tbl_login_del_login_sync(tbl_login_key_t *p_login_key, uint32 sync);

int32
tbl_login_set_login_field_sync(tbl_login_t *p_login, tbl_login_field_id_t field_id, uint32 sync);

int32
tbl_login_add_login(tbl_login_t *p_login);

int32
tbl_login_del_login(tbl_login_key_t *p_login_key);

int32
tbl_login_set_login_field(tbl_login_t *p_login, tbl_login_field_id_t field_id);

tbl_login_t*
tbl_login_get_login(tbl_login_key_t *p_login_key);

char*
tbl_login_key_val2str(tbl_login_t *p_login, char *str, uint32 str_len);

int32
tbl_login_key_str2val(char *str, tbl_login_t *p_login);

int32
tbl_login_dump_one(tbl_login_t *p_login, tbl_iter_args_t *pargs);

char*
tbl_login_key_name_dump(tbl_login_t *p_login, char* str);

char*
tbl_login_key_value_dump(tbl_login_t *p_login, char* str);

char*
tbl_login_field_name_dump(tbl_login_t *p_login, int32 field_id, char* str);

char*
tbl_login_field_value_dump(tbl_login_t *p_login, int32 field_id, char* str);

char**
tbl_login_table_dump(tbl_login_t *p_login, char** str);

int32
tbl_login_table_parser(char** array, char* key_value, tbl_login_t *p_login);

int32
tbl_login_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_login_master_t*
tbl_login_get_master();

tbl_login_master_t*
tbl_login_init_login();

#endif /* !__TBL_LOGIN_H__ */

