
#ifndef __TBL_POLICY_MAP_CONFIG_H__
#define __TBL_POLICY_MAP_CONFIG_H__

int32
tbl_policy_map_config_add_policy_map_config_sync(tbl_policy_map_config_t *p_policy_map_config, uint32 sync);

int32
tbl_policy_map_config_del_policy_map_config_sync(tbl_policy_map_config_key_t *p_policy_map_config_key, uint32 sync);

int32
tbl_policy_map_config_set_policy_map_config_field_sync(tbl_policy_map_config_t *p_policy_map_config, tbl_policy_map_config_field_id_t field_id, uint32 sync);

int32
tbl_policy_map_config_add_policy_map_config(tbl_policy_map_config_t *p_policy_map_config);

int32
tbl_policy_map_config_del_policy_map_config(tbl_policy_map_config_key_t *p_policy_map_config_key);

int32
tbl_policy_map_config_set_policy_map_config_field(tbl_policy_map_config_t *p_policy_map_config, tbl_policy_map_config_field_id_t field_id);

tbl_policy_map_config_t*
tbl_policy_map_config_get_policy_map_config(tbl_policy_map_config_key_t *p_policy_map_config_key);

char*
tbl_policy_map_config_key_val2str(tbl_policy_map_config_t *p_policy_map_config, char *str, uint32 str_len);

int32
tbl_policy_map_config_key_str2val(char *str, tbl_policy_map_config_t *p_policy_map_config);

int32
tbl_policy_map_config_dump_one(tbl_policy_map_config_t *p_policy_map_config, tbl_iter_args_t *pargs);

char*
tbl_policy_map_config_key_name_dump(tbl_policy_map_config_t *p_policy_map_config, char* str);

char*
tbl_policy_map_config_key_value_dump(tbl_policy_map_config_t *p_policy_map_config, char* str);

char*
tbl_policy_map_config_field_name_dump(tbl_policy_map_config_t *p_policy_map_config, int32 field_id, char* str);

char*
tbl_policy_map_config_field_value_dump(tbl_policy_map_config_t *p_policy_map_config, int32 field_id, char* str);

char**
tbl_policy_map_config_table_dump(tbl_policy_map_config_t *p_policy_map_config, char** str);

int32
tbl_policy_map_config_table_parser(char** array, char* key_value, tbl_policy_map_config_t *p_policy_map_config);

int32
tbl_policy_map_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_policy_map_config_master_t*
tbl_policy_map_config_get_master();

tbl_policy_map_config_master_t*
tbl_policy_map_config_init_policy_map_config();

int32
tbl_policy_map_config_get_policy_map_config_count();

tbl_policy_map_config_t*
tbl_policy_map_config_get_policy_map_config_by_name(const char* policy_map_name);


#endif /* !__TBL_POLICY_MAP_CONFIG_H__ */

