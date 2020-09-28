
#ifndef __TBL_ACL_CONFIG_H__
#define __TBL_ACL_CONFIG_H__

int32
tbl_acl_config_add_acl_config_sync(tbl_acl_config_t *p_acl_config, uint32 sync);

int32
tbl_acl_config_del_acl_config_sync(tbl_acl_config_key_t *p_acl_config_key, uint32 sync);

int32
tbl_acl_config_set_acl_config_field_sync(tbl_acl_config_t *p_acl_config, tbl_acl_config_field_id_t field_id, uint32 sync);

int32
tbl_acl_config_add_acl_config(tbl_acl_config_t *p_acl_config);

int32
tbl_acl_config_del_acl_config(tbl_acl_config_key_t *p_acl_config_key);

int32
tbl_acl_config_set_acl_config_field(tbl_acl_config_t *p_acl_config, tbl_acl_config_field_id_t field_id);

tbl_acl_config_t*
tbl_acl_config_get_acl_config(tbl_acl_config_key_t *p_acl_config_key);

char*
tbl_acl_config_key_val2str(tbl_acl_config_t *p_acl_config, char *str, uint32 str_len);

int32
tbl_acl_config_key_str2val(char *str, tbl_acl_config_t *p_acl_config);

int32
tbl_acl_config_dump_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs);

char*
tbl_acl_config_key_name_dump(tbl_acl_config_t *p_acl_config, char* str);

char*
tbl_acl_config_key_value_dump(tbl_acl_config_t *p_acl_config, char* str);

char*
tbl_acl_config_field_name_dump(tbl_acl_config_t *p_acl_config, int32 field_id, char* str);

char*
tbl_acl_config_field_value_dump(tbl_acl_config_t *p_acl_config, int32 field_id, char* str);

char**
tbl_acl_config_table_dump(tbl_acl_config_t *p_acl_config, char** str);

int32
tbl_acl_config_table_parser(char** array, char* key_value, tbl_acl_config_t *p_acl_config);

int32
tbl_acl_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_config_master_t*
tbl_acl_config_get_master();

tbl_acl_config_master_t*
tbl_acl_config_init_acl_config();

int32
tbl_acl_config_get_acl_config_count();

int32
tbl_acl_config_get_l4port_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs);

int32
tbl_acl_config_get_l4port_config_count();

int32
tbl_acl_config_get_tcp_flags_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs);

int32
tbl_acl_config_get_tcp_flags_config_count();


#endif /* !__TBL_ACL_CONFIG_H__ */

