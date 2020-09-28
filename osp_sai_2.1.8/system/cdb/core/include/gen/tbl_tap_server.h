
#ifndef __TBL_TAP_SERVER_H__
#define __TBL_TAP_SERVER_H__

int32
tbl_tap_server_set_tap_server_field_sync(tbl_tap_server_t *p_tap_srv, tbl_tap_server_field_id_t field_id, uint32 sync);

int32
tbl_tap_server_set_tap_server_field(tbl_tap_server_t *p_tap_srv, tbl_tap_server_field_id_t field_id);

tbl_tap_server_t*
tbl_tap_server_get_tap_server();

char*
tbl_tap_server_key_val2str(tbl_tap_server_t *p_tap_srv, char *str, uint32 str_len);

int32
tbl_tap_server_key_str2val(char *str, tbl_tap_server_t *p_tap_srv);

int32
tbl_tap_server_dump_one(tbl_tap_server_t *p_tap_srv, tbl_iter_args_t *pargs);

char*
tbl_tap_server_key_name_dump(tbl_tap_server_t *p_tap_srv, char* str);

char*
tbl_tap_server_key_value_dump(tbl_tap_server_t *p_tap_srv, char* str);

char*
tbl_tap_server_field_name_dump(tbl_tap_server_t *p_tap_srv, int32 field_id, char* str);

char*
tbl_tap_server_field_value_dump(tbl_tap_server_t *p_tap_srv, int32 field_id, char* str);

char**
tbl_tap_server_table_dump(tbl_tap_server_t *p_tap_srv, char** str);

int32
tbl_tap_server_table_parser(char** array, char* key_value, tbl_tap_server_t *p_tap_srv);

int32
tbl_tap_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_tap_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_server_t*
tbl_tap_server_init_tap_server();

#endif /* !__TBL_TAP_SERVER_H__ */

