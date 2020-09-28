
#ifndef __TBL_AUTH_SERVER_H__
#define __TBL_AUTH_SERVER_H__

int32
tbl_auth_server_add_auth_server_sync(tbl_auth_server_t *p_auth_server, uint32 sync);

int32
tbl_auth_server_del_auth_server_sync(tbl_auth_server_key_t *p_auth_server_key, uint32 sync);

int32
tbl_auth_server_set_auth_server_field_sync(tbl_auth_server_t *p_auth_server, tbl_auth_server_field_id_t field_id, uint32 sync);

int32
tbl_auth_server_add_auth_server(tbl_auth_server_t *p_auth_server);

int32
tbl_auth_server_del_auth_server(tbl_auth_server_key_t *p_auth_server_key);

int32
tbl_auth_server_set_auth_server_field(tbl_auth_server_t *p_auth_server, tbl_auth_server_field_id_t field_id);

tbl_auth_server_t*
tbl_auth_server_get_auth_server(tbl_auth_server_key_t *p_auth_server_key);

char*
tbl_auth_server_key_val2str(tbl_auth_server_t *p_auth_server, char *str, uint32 str_len);

int32
tbl_auth_server_key_str2val(char *str, tbl_auth_server_t *p_auth_server);

int32
tbl_auth_server_dump_one(tbl_auth_server_t *p_auth_server, tbl_iter_args_t *pargs);

char*
tbl_auth_server_key_name_dump(tbl_auth_server_t *p_auth_server, char* str);

char*
tbl_auth_server_key_value_dump(tbl_auth_server_t *p_auth_server, char* str);

char*
tbl_auth_server_field_name_dump(tbl_auth_server_t *p_auth_server, int32 field_id, char* str);

char*
tbl_auth_server_field_value_dump(tbl_auth_server_t *p_auth_server, int32 field_id, char* str);

char**
tbl_auth_server_table_dump(tbl_auth_server_t *p_auth_server, char** str);

int32
tbl_auth_server_table_parser(char** array, char* key_value, tbl_auth_server_t *p_auth_server);

int32
tbl_auth_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_auth_server_master_t*
tbl_auth_server_get_master();

tbl_auth_server_master_t*
tbl_auth_server_init_auth_server();

#endif /* !__TBL_AUTH_SERVER_H__ */

