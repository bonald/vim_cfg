
#ifndef __TBL_NTP_SERVER_H__
#define __TBL_NTP_SERVER_H__

int32
tbl_ntp_server_add_ntp_server_sync(tbl_ntp_server_t *p_ntp_server, uint32 sync);

int32
tbl_ntp_server_del_ntp_server_sync(tbl_ntp_server_key_t *p_ntp_server_key, uint32 sync);

int32
tbl_ntp_server_set_ntp_server_field_sync(tbl_ntp_server_t *p_ntp_server, tbl_ntp_server_field_id_t field_id, uint32 sync);

int32
tbl_ntp_server_add_ntp_server(tbl_ntp_server_t *p_ntp_server);

int32
tbl_ntp_server_del_ntp_server(tbl_ntp_server_key_t *p_ntp_server_key);

int32
tbl_ntp_server_set_ntp_server_field(tbl_ntp_server_t *p_ntp_server, tbl_ntp_server_field_id_t field_id);

tbl_ntp_server_t*
tbl_ntp_server_get_ntp_server(tbl_ntp_server_key_t *p_ntp_server_key);

char*
tbl_ntp_server_key_val2str(tbl_ntp_server_t *p_ntp_server, char *str, uint32 str_len);

int32
tbl_ntp_server_key_str2val(char *str, tbl_ntp_server_t *p_ntp_server);

int32
tbl_ntp_server_dump_one(tbl_ntp_server_t *p_ntp_server, tbl_iter_args_t *pargs);

char*
tbl_ntp_server_key_name_dump(tbl_ntp_server_t *p_ntp_server, char* str);

char*
tbl_ntp_server_key_value_dump(tbl_ntp_server_t *p_ntp_server, char* str);

char*
tbl_ntp_server_field_name_dump(tbl_ntp_server_t *p_ntp_server, int32 field_id, char* str);

char*
tbl_ntp_server_field_value_dump(tbl_ntp_server_t *p_ntp_server, int32 field_id, char* str);

char**
tbl_ntp_server_table_dump(tbl_ntp_server_t *p_ntp_server, char** str);

int32
tbl_ntp_server_table_parser(char** array, char* key_value, tbl_ntp_server_t *p_ntp_server);

int32
tbl_ntp_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ntp_server_master_t*
tbl_ntp_server_get_master();

tbl_ntp_server_master_t*
tbl_ntp_server_init_ntp_server();

#endif /* !__TBL_NTP_SERVER_H__ */

