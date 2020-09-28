
#ifndef __TBL_AUTH_SESSION_H__
#define __TBL_AUTH_SESSION_H__

int32
tbl_auth_session_add_auth_session_sync(tbl_auth_session_t *p_auth_session, uint32 sync);

int32
tbl_auth_session_del_auth_session_sync(tbl_auth_session_key_t *p_auth_session_key, uint32 sync);

int32
tbl_auth_session_set_auth_session_field_sync(tbl_auth_session_t *p_auth_session, tbl_auth_session_field_id_t field_id, uint32 sync);

int32
tbl_auth_session_add_auth_session(tbl_auth_session_t *p_auth_session);

int32
tbl_auth_session_del_auth_session(tbl_auth_session_key_t *p_auth_session_key);

int32
tbl_auth_session_set_auth_session_field(tbl_auth_session_t *p_auth_session, tbl_auth_session_field_id_t field_id);

tbl_auth_session_t*
tbl_auth_session_get_auth_session(tbl_auth_session_key_t *p_auth_session_key);

char*
tbl_auth_session_key_val2str(tbl_auth_session_t *p_auth_session, char *str, uint32 str_len);

int32
tbl_auth_session_key_str2val(char *str, tbl_auth_session_t *p_auth_session);

int32
tbl_auth_session_dump_one(tbl_auth_session_t *p_auth_session, tbl_iter_args_t *pargs);

char*
tbl_auth_session_key_name_dump(tbl_auth_session_t *p_auth_session, char* str);

char*
tbl_auth_session_key_value_dump(tbl_auth_session_t *p_auth_session, char* str);

char*
tbl_auth_session_field_name_dump(tbl_auth_session_t *p_auth_session, int32 field_id, char* str);

char*
tbl_auth_session_field_value_dump(tbl_auth_session_t *p_auth_session, int32 field_id, char* str);

char**
tbl_auth_session_table_dump(tbl_auth_session_t *p_auth_session, char** str);

int32
tbl_auth_session_table_parser(char** array, char* key_value, tbl_auth_session_t *p_auth_session);

int32
tbl_auth_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_auth_session_master_t*
tbl_auth_session_get_master();

tbl_auth_session_master_t*
tbl_auth_session_init_auth_session();

#endif /* !__TBL_AUTH_SESSION_H__ */

