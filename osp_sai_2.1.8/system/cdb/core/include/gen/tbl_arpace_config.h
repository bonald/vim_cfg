
#ifndef __TBL_ARPACE_CONFIG_H__
#define __TBL_ARPACE_CONFIG_H__

int32
tbl_arpace_config_add_arpace_config_sync(tbl_arpace_config_t *p_arpace_config, uint32 sync);

int32
tbl_arpace_config_del_arpace_config_sync(tbl_arpace_config_key_t *p_arpace_config_key, uint32 sync);

int32
tbl_arpace_config_set_arpace_config_field_sync(tbl_arpace_config_t *p_arpace_config, tbl_arpace_config_field_id_t field_id, uint32 sync);

int32
tbl_arpace_config_add_arpace_config(tbl_arpace_config_t *p_arpace_config);

int32
tbl_arpace_config_del_arpace_config(tbl_arpace_config_key_t *p_arpace_config_key);

int32
tbl_arpace_config_set_arpace_config_field(tbl_arpace_config_t *p_arpace_config, tbl_arpace_config_field_id_t field_id);

tbl_arpace_config_t*
tbl_arpace_config_get_arpace_config(tbl_arpace_config_key_t *p_arpace_config_key);

char*
tbl_arpace_config_key_val2str(tbl_arpace_config_t *p_arpace_config, char *str, uint32 str_len);

int32
tbl_arpace_config_key_str2val(char *str, tbl_arpace_config_t *p_arpace_config);

int32
tbl_arpace_config_dump_one(tbl_arpace_config_t *p_arpace_config, tbl_iter_args_t *pargs);

char*
tbl_arpace_config_key_name_dump(tbl_arpace_config_t *p_arpace_config, char* str);

char*
tbl_arpace_config_key_value_dump(tbl_arpace_config_t *p_arpace_config, char* str);

char*
tbl_arpace_config_field_name_dump(tbl_arpace_config_t *p_arpace_config, int32 field_id, char* str);

char*
tbl_arpace_config_field_value_dump(tbl_arpace_config_t *p_arpace_config, int32 field_id, char* str);

char**
tbl_arpace_config_table_dump(tbl_arpace_config_t *p_arpace_config, char** str);

int32
tbl_arpace_config_table_parser(char** array, char* key_value, tbl_arpace_config_t *p_arpace_config);

int32
tbl_arpace_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_arpace_config_master_t*
tbl_arpace_config_get_master();

tbl_arpace_config_master_t*
tbl_arpace_config_init_arpace_config();

#endif /* !__TBL_ARPACE_CONFIG_H__ */

