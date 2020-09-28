
#ifndef __TBL_VRRP_SESSION_H__
#define __TBL_VRRP_SESSION_H__

int32
tbl_vrrp_session_add_vrrp_session_sync(tbl_vrrp_session_t *p_vrrp_sess, uint32 sync);

int32
tbl_vrrp_session_del_vrrp_session_sync(tbl_vrrp_session_key_t *p_vrrp_sess_key, uint32 sync);

int32
tbl_vrrp_session_set_vrrp_session_field_sync(tbl_vrrp_session_t *p_vrrp_sess, tbl_vrrp_session_field_id_t field_id, uint32 sync);

int32
tbl_vrrp_session_add_vrrp_session(tbl_vrrp_session_t *p_vrrp_sess);

int32
tbl_vrrp_session_del_vrrp_session(tbl_vrrp_session_key_t *p_vrrp_sess_key);

int32
tbl_vrrp_session_set_vrrp_session_field(tbl_vrrp_session_t *p_vrrp_sess, tbl_vrrp_session_field_id_t field_id);

tbl_vrrp_session_t*
tbl_vrrp_session_get_vrrp_session(tbl_vrrp_session_key_t *p_vrrp_sess_key);

char*
tbl_vrrp_session_key_val2str(tbl_vrrp_session_t *p_vrrp_sess, char *str, uint32 str_len);

int32
tbl_vrrp_session_key_str2val(char *str, tbl_vrrp_session_t *p_vrrp_sess);

int32
tbl_vrrp_session_dump_one(tbl_vrrp_session_t *p_vrrp_sess, tbl_iter_args_t *pargs);

char*
tbl_vrrp_session_key_name_dump(tbl_vrrp_session_t *p_vrrp_sess, char* str);

char*
tbl_vrrp_session_key_value_dump(tbl_vrrp_session_t *p_vrrp_sess, char* str);

char*
tbl_vrrp_session_field_name_dump(tbl_vrrp_session_t *p_vrrp_sess, int32 field_id, char* str);

char*
tbl_vrrp_session_field_value_dump(tbl_vrrp_session_t *p_vrrp_sess, int32 field_id, char* str);

char**
tbl_vrrp_session_table_dump(tbl_vrrp_session_t *p_vrrp_sess, char** str);

int32
tbl_vrrp_session_table_parser(char** array, char* key_value, tbl_vrrp_session_t *p_vrrp_sess);

int32
tbl_vrrp_session_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vrrp_session_master_t*
tbl_vrrp_session_get_master();

tbl_vrrp_session_master_t*
tbl_vrrp_session_init_vrrp_session();

#endif /* !__TBL_VRRP_SESSION_H__ */

