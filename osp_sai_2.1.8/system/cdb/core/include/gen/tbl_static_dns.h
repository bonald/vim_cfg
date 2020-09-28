
#ifndef __TBL_STATIC_DNS_H__
#define __TBL_STATIC_DNS_H__

int32
tbl_static_dns_add_static_dns_sync(tbl_static_dns_t *p_static_cfg, uint32 sync);

int32
tbl_static_dns_del_static_dns_sync(tbl_static_dns_key_t *p_static_cfg_key, uint32 sync);

int32
tbl_static_dns_set_static_dns_field_sync(tbl_static_dns_t *p_static_cfg, tbl_static_dns_field_id_t field_id, uint32 sync);

int32
tbl_static_dns_add_static_dns(tbl_static_dns_t *p_static_cfg);

int32
tbl_static_dns_del_static_dns(tbl_static_dns_key_t *p_static_cfg_key);

int32
tbl_static_dns_set_static_dns_field(tbl_static_dns_t *p_static_cfg, tbl_static_dns_field_id_t field_id);

tbl_static_dns_t*
tbl_static_dns_get_static_dns(tbl_static_dns_key_t *p_static_cfg_key);

char*
tbl_static_dns_key_val2str(tbl_static_dns_t *p_static_cfg, char *str, uint32 str_len);

int32
tbl_static_dns_key_str2val(char *str, tbl_static_dns_t *p_static_cfg);

int32
tbl_static_dns_dump_one(tbl_static_dns_t *p_static_cfg, tbl_iter_args_t *pargs);

char*
tbl_static_dns_key_name_dump(tbl_static_dns_t *p_static_cfg, char* str);

char*
tbl_static_dns_key_value_dump(tbl_static_dns_t *p_static_cfg, char* str);

char*
tbl_static_dns_field_name_dump(tbl_static_dns_t *p_static_cfg, int32 field_id, char* str);

char*
tbl_static_dns_field_value_dump(tbl_static_dns_t *p_static_cfg, int32 field_id, char* str);

char**
tbl_static_dns_table_dump(tbl_static_dns_t *p_static_cfg, char** str);

int32
tbl_static_dns_table_parser(char** array, char* key_value, tbl_static_dns_t *p_static_cfg);

int32
tbl_static_dns_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_static_dns_master_t*
tbl_static_dns_get_master();

tbl_static_dns_master_t*
tbl_static_dns_init_static_dns();

#endif /* !__TBL_STATIC_DNS_H__ */

