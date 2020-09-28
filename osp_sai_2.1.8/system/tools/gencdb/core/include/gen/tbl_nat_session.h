
#ifndef __TBL_NAT_SESSION_H__
#define __TBL_NAT_SESSION_H__

int32
tbl_nat_session_add_nat_session_sync(tbl_nat_session_t *p_nat_session, uint32 sync);

int32
tbl_nat_session_del_nat_session_sync(tbl_nat_session_key_t *p_nat_session_key, uint32 sync);

int32
tbl_nat_session_set_nat_session_field_sync(tbl_nat_session_t *p_nat_session, tbl_nat_session_field_id_t field_id, uint32 sync);

int32
tbl_nat_session_add_nat_session(tbl_nat_session_t *p_nat_session);

int32
tbl_nat_session_del_nat_session(tbl_nat_session_key_t *p_nat_session_key);

int32
tbl_nat_session_set_nat_session_field(tbl_nat_session_t *p_nat_session, tbl_nat_session_field_id_t field_id);

tbl_nat_session_t*
tbl_nat_session_get_nat_session(tbl_nat_session_key_t *p_nat_session_key);

char*
tbl_nat_session_key_val2str(tbl_nat_session_t *p_nat_session, char *str, uint32 str_len);

int32
tbl_nat_session_key_str2val(char *str, tbl_nat_session_t *p_nat_session);

int32
tbl_nat_session_dump_one(tbl_nat_session_t *p_nat_session, tbl_iter_args_t *pargs);

char*
tbl_nat_session_key_name_dump(tbl_nat_session_t *p_nat_session, char* str);

char*
tbl_nat_session_key_value_dump(tbl_nat_session_t *p_nat_session, char* str);

char*
tbl_nat_session_field_name_dump(tbl_nat_session_t *p_nat_session, int32 field_id, char* str);

char*
tbl_nat_session_field_value_dump(tbl_nat_session_t *p_nat_session, int32 field_id, char* str);

char**
tbl_nat_session_table_dump(tbl_nat_session_t *p_nat_session, char** str);

int32
tbl_nat_session_table_parser(char** array, char* key_value, tbl_nat_session_t *p_nat_session);

int32
tbl_nat_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_session_master_t*
tbl_nat_session_get_master();

tbl_nat_session_master_t*
tbl_nat_session_init_nat_session();

#endif /* !__TBL_NAT_SESSION_H__ */

