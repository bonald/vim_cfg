
#ifndef __TBL_ARPACL_CONFIG_H__
#define __TBL_ARPACL_CONFIG_H__

int32
tbl_arpacl_config_add_arpacl_config_sync(tbl_arpacl_config_t *p_arpacl_config, uint32 sync);

int32
tbl_arpacl_config_del_arpacl_config_sync(tbl_arpacl_config_key_t *p_arpacl_config_key, uint32 sync);

int32
tbl_arpacl_config_set_arpacl_config_field_sync(tbl_arpacl_config_t *p_arpacl_config, tbl_arpacl_config_field_id_t field_id, uint32 sync);

int32
tbl_arpacl_config_add_arpacl_config(tbl_arpacl_config_t *p_arpacl_config);

int32
tbl_arpacl_config_del_arpacl_config(tbl_arpacl_config_key_t *p_arpacl_config_key);

int32
tbl_arpacl_config_set_arpacl_config_field(tbl_arpacl_config_t *p_arpacl_config, tbl_arpacl_config_field_id_t field_id);

tbl_arpacl_config_t*
tbl_arpacl_config_get_arpacl_config(tbl_arpacl_config_key_t *p_arpacl_config_key);

char*
tbl_arpacl_config_key_val2str(tbl_arpacl_config_t *p_arpacl_config, char *str, uint32 str_len);

int32
tbl_arpacl_config_key_str2val(char *str, tbl_arpacl_config_t *p_arpacl_config);

int32
tbl_arpacl_config_dump_one(tbl_arpacl_config_t *p_arpacl_config, tbl_iter_args_t *pargs);

char*
tbl_arpacl_config_key_name_dump(tbl_arpacl_config_t *p_arpacl_config, char* str);

char*
tbl_arpacl_config_key_value_dump(tbl_arpacl_config_t *p_arpacl_config, char* str);

char*
tbl_arpacl_config_field_name_dump(tbl_arpacl_config_t *p_arpacl_config, int32 field_id, char* str);

char*
tbl_arpacl_config_field_value_dump(tbl_arpacl_config_t *p_arpacl_config, int32 field_id, char* str);

char**
tbl_arpacl_config_table_dump(tbl_arpacl_config_t *p_arpacl_config, char** str);

int32
tbl_arpacl_config_table_parser(char** array, char* key_value, tbl_arpacl_config_t *p_arpacl_config);

int32
tbl_arpacl_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_arpacl_config_master_t*
tbl_arpacl_config_get_master();

tbl_arpacl_config_master_t*
tbl_arpacl_config_init_arpacl_config();

#endif /* !__TBL_ARPACL_CONFIG_H__ */

