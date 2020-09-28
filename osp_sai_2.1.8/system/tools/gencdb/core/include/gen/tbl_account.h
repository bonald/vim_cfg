
#ifndef __TBL_ACCOUNT_H__
#define __TBL_ACCOUNT_H__

int32
tbl_account_add_account_sync(tbl_account_t *p_account, uint32 sync);

int32
tbl_account_del_account_sync(tbl_account_key_t *p_account_key, uint32 sync);

int32
tbl_account_set_account_field_sync(tbl_account_t *p_account, tbl_account_field_id_t field_id, uint32 sync);

int32
tbl_account_add_account(tbl_account_t *p_account);

int32
tbl_account_del_account(tbl_account_key_t *p_account_key);

int32
tbl_account_set_account_field(tbl_account_t *p_account, tbl_account_field_id_t field_id);

tbl_account_t*
tbl_account_get_account(tbl_account_key_t *p_account_key);

char*
tbl_account_key_val2str(tbl_account_t *p_account, char *str, uint32 str_len);

int32
tbl_account_key_str2val(char *str, tbl_account_t *p_account);

int32
tbl_account_dump_one(tbl_account_t *p_account, tbl_iter_args_t *pargs);

char*
tbl_account_key_name_dump(tbl_account_t *p_account, char* str);

char*
tbl_account_key_value_dump(tbl_account_t *p_account, char* str);

char*
tbl_account_field_name_dump(tbl_account_t *p_account, int32 field_id, char* str);

char*
tbl_account_field_value_dump(tbl_account_t *p_account, int32 field_id, char* str);

char**
tbl_account_table_dump(tbl_account_t *p_account, char** str);

int32
tbl_account_table_parser(char** array, char* key_value, tbl_account_t *p_account);

int32
tbl_account_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_account_master_t*
tbl_account_get_master();

tbl_account_master_t*
tbl_account_init_account();

#endif /* !__TBL_ACCOUNT_H__ */

