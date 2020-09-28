
#ifndef __TBL_ACE_CONFIG_H__
#define __TBL_ACE_CONFIG_H__

int32
tbl_ace_config_add_ace_config_sync(tbl_ace_config_t *p_ace_config, uint32 sync);

int32
tbl_ace_config_del_ace_config_sync(tbl_ace_config_key_t *p_ace_config_key, uint32 sync);

int32
tbl_ace_config_set_ace_config_field_sync(tbl_ace_config_t *p_ace_config, tbl_ace_config_field_id_t field_id, uint32 sync);

int32
tbl_ace_config_add_ace_config(tbl_ace_config_t *p_ace_config);

int32
tbl_ace_config_del_ace_config(tbl_ace_config_key_t *p_ace_config_key);

int32
tbl_ace_config_set_ace_config_field(tbl_ace_config_t *p_ace_config, tbl_ace_config_field_id_t field_id);

tbl_ace_config_t*
tbl_ace_config_get_ace_config(tbl_ace_config_key_t *p_ace_config_key);

char*
tbl_ace_config_key_val2str(tbl_ace_config_t *p_ace_config, char *str, uint32 str_len);

int32
tbl_ace_config_key_str2val(char *str, tbl_ace_config_t *p_ace_config);

int32
tbl_ace_config_dump_one(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs);

char*
tbl_ace_config_key_name_dump(tbl_ace_config_t *p_ace_config, char* str);

char*
tbl_ace_config_key_value_dump(tbl_ace_config_t *p_ace_config, char* str);

char*
tbl_ace_config_field_name_dump(tbl_ace_config_t *p_ace_config, int32 field_id, char* str);

char*
tbl_ace_config_field_value_dump(tbl_ace_config_t *p_ace_config, int32 field_id, char* str);

char**
tbl_ace_config_table_dump(tbl_ace_config_t *p_ace_config, char** str);

int32
tbl_ace_config_table_parser(char** array, char* key_value, tbl_ace_config_t *p_ace_config);

int32
tbl_ace_config_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ace_config_master_t*
tbl_ace_config_get_master();

tbl_ace_config_master_t*
tbl_ace_config_init_ace_config();

int32
tbl_ace_config_get_ace_config_count();

tbl_ace_config_t*
tbl_ace_config_get_ace_config_by_name(const char* acl_name, const uint32 seq_no);


#endif /* !__TBL_ACE_CONFIG_H__ */

