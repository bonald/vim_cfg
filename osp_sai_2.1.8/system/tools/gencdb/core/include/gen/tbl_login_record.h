
#ifndef __TBL_LOGIN_RECORD_H__
#define __TBL_LOGIN_RECORD_H__

int32
tbl_login_record_add_login_record_sync(tbl_login_record_t *p_login_record, uint32 sync);

int32
tbl_login_record_del_login_record_sync(tbl_login_record_key_t *p_login_record_key, uint32 sync);

int32
tbl_login_record_set_login_record_field_sync(tbl_login_record_t *p_login_record, tbl_login_record_field_id_t field_id, uint32 sync);

int32
tbl_login_record_add_login_record(tbl_login_record_t *p_login_record);

int32
tbl_login_record_del_login_record(tbl_login_record_key_t *p_login_record_key);

int32
tbl_login_record_set_login_record_field(tbl_login_record_t *p_login_record, tbl_login_record_field_id_t field_id);

tbl_login_record_t*
tbl_login_record_get_login_record(tbl_login_record_key_t *p_login_record_key);

char*
tbl_login_record_key_val2str(tbl_login_record_t *p_login_record, char *str, uint32 str_len);

int32
tbl_login_record_key_str2val(char *str, tbl_login_record_t *p_login_record);

int32
tbl_login_record_dump_one(tbl_login_record_t *p_login_record, tbl_iter_args_t *pargs);

char*
tbl_login_record_key_name_dump(tbl_login_record_t *p_login_record, char* str);

char*
tbl_login_record_key_value_dump(tbl_login_record_t *p_login_record, char* str);

char*
tbl_login_record_field_name_dump(tbl_login_record_t *p_login_record, int32 field_id, char* str);

char*
tbl_login_record_field_value_dump(tbl_login_record_t *p_login_record, int32 field_id, char* str);

char**
tbl_login_record_table_dump(tbl_login_record_t *p_login_record, char** str);

int32
tbl_login_record_table_parser(char** array, char* key_value, tbl_login_record_t *p_login_record);

int32
tbl_login_record_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_login_record_master_t*
tbl_login_record_get_master();

tbl_login_record_master_t*
tbl_login_record_init_login_record();

#endif /* !__TBL_LOGIN_RECORD_H__ */

