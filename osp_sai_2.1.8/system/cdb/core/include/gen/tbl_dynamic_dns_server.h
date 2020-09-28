
#ifndef __TBL_DYNAMIC_DNS_SERVER_H__
#define __TBL_DYNAMIC_DNS_SERVER_H__

int32
tbl_dynamic_dns_server_add_dynamic_dns_server_sync(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, uint32 sync);

int32
tbl_dynamic_dns_server_del_dynamic_dns_server_sync(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key, uint32 sync);

int32
tbl_dynamic_dns_server_set_dynamic_dns_server_field_sync(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_dynamic_dns_server_field_id_t field_id, uint32 sync);

int32
tbl_dynamic_dns_server_add_dynamic_dns_server(tbl_dynamic_dns_server_t *p_dynamic_server_cfg);

int32
tbl_dynamic_dns_server_del_dynamic_dns_server(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key);

int32
tbl_dynamic_dns_server_set_dynamic_dns_server_field(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_dynamic_dns_server_field_id_t field_id);

tbl_dynamic_dns_server_t*
tbl_dynamic_dns_server_get_dynamic_dns_server(tbl_dynamic_dns_server_key_t *p_dynamic_server_cfg_key);

char*
tbl_dynamic_dns_server_key_val2str(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char *str, uint32 str_len);

int32
tbl_dynamic_dns_server_key_str2val(char *str, tbl_dynamic_dns_server_t *p_dynamic_server_cfg);

int32
tbl_dynamic_dns_server_dump_one(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, tbl_iter_args_t *pargs);

char*
tbl_dynamic_dns_server_key_name_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char* str);

char*
tbl_dynamic_dns_server_key_value_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char* str);

char*
tbl_dynamic_dns_server_field_name_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, int32 field_id, char* str);

char*
tbl_dynamic_dns_server_field_value_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, int32 field_id, char* str);

char**
tbl_dynamic_dns_server_table_dump(tbl_dynamic_dns_server_t *p_dynamic_server_cfg, char** str);

int32
tbl_dynamic_dns_server_table_parser(char** array, char* key_value, tbl_dynamic_dns_server_t *p_dynamic_server_cfg);

int32
tbl_dynamic_dns_server_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dynamic_dns_server_master_t*
tbl_dynamic_dns_server_get_master();

tbl_dynamic_dns_server_master_t*
tbl_dynamic_dns_server_init_dynamic_dns_server();

#endif /* !__TBL_DYNAMIC_DNS_SERVER_H__ */

