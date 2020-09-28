
#ifndef __TBL_NAT_SESSION_LIMIT_H__
#define __TBL_NAT_SESSION_LIMIT_H__

int32
tbl_nat_session_limit_add_nat_session_limit_sync(tbl_nat_session_limit_t *p_nat_session_limit, uint32 sync);

int32
tbl_nat_session_limit_del_nat_session_limit_sync(tbl_nat_session_limit_key_t *p_nat_session_limit_key, uint32 sync);

int32
tbl_nat_session_limit_set_nat_session_limit_field_sync(tbl_nat_session_limit_t *p_nat_session_limit, tbl_nat_session_limit_field_id_t field_id, uint32 sync);

int32
tbl_nat_session_limit_add_nat_session_limit(tbl_nat_session_limit_t *p_nat_session_limit);

int32
tbl_nat_session_limit_del_nat_session_limit(tbl_nat_session_limit_key_t *p_nat_session_limit_key);

int32
tbl_nat_session_limit_set_nat_session_limit_field(tbl_nat_session_limit_t *p_nat_session_limit, tbl_nat_session_limit_field_id_t field_id);

tbl_nat_session_limit_t*
tbl_nat_session_limit_get_nat_session_limit(tbl_nat_session_limit_key_t *p_nat_session_limit_key);

char*
tbl_nat_session_limit_key_val2str(tbl_nat_session_limit_t *p_nat_session_limit, char *str, uint32 str_len);

int32
tbl_nat_session_limit_key_str2val(char *str, tbl_nat_session_limit_t *p_nat_session_limit);

int32
tbl_nat_session_limit_dump_one(tbl_nat_session_limit_t *p_nat_session_limit, tbl_iter_args_t *pargs);

char*
tbl_nat_session_limit_key_name_dump(tbl_nat_session_limit_t *p_nat_session_limit, char* str);

char*
tbl_nat_session_limit_key_value_dump(tbl_nat_session_limit_t *p_nat_session_limit, char* str);

char*
tbl_nat_session_limit_field_name_dump(tbl_nat_session_limit_t *p_nat_session_limit, int32 field_id, char* str);

char*
tbl_nat_session_limit_field_value_dump(tbl_nat_session_limit_t *p_nat_session_limit, int32 field_id, char* str);

char**
tbl_nat_session_limit_table_dump(tbl_nat_session_limit_t *p_nat_session_limit, char** str);

int32
tbl_nat_session_limit_table_parser(char** array, char* key_value, tbl_nat_session_limit_t *p_nat_session_limit);

int32
tbl_nat_session_limit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_session_limit_master_t*
tbl_nat_session_limit_get_master();

tbl_nat_session_limit_master_t*
tbl_nat_session_limit_init_nat_session_limit();

#endif /* !__TBL_NAT_SESSION_LIMIT_H__ */

