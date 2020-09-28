
#ifndef __TBL_AUTH_CFG_H__
#define __TBL_AUTH_CFG_H__

int32
tbl_auth_cfg_set_auth_cfg_field_sync(tbl_auth_cfg_t *p_auth_cfg, tbl_auth_cfg_field_id_t field_id, uint32 sync);

int32
tbl_auth_cfg_set_auth_cfg_field(tbl_auth_cfg_t *p_auth_cfg, tbl_auth_cfg_field_id_t field_id);

tbl_auth_cfg_t*
tbl_auth_cfg_get_auth_cfg();

char*
tbl_auth_cfg_key_val2str(tbl_auth_cfg_t *p_auth_cfg, char *str, uint32 str_len);

int32
tbl_auth_cfg_key_str2val(char *str, tbl_auth_cfg_t *p_auth_cfg);

int32
tbl_auth_cfg_dump_one(tbl_auth_cfg_t *p_auth_cfg, tbl_iter_args_t *pargs);

char*
tbl_auth_cfg_key_name_dump(tbl_auth_cfg_t *p_auth_cfg, char* str);

char*
tbl_auth_cfg_key_value_dump(tbl_auth_cfg_t *p_auth_cfg, char* str);

char*
tbl_auth_cfg_field_name_dump(tbl_auth_cfg_t *p_auth_cfg, int32 field_id, char* str);

char*
tbl_auth_cfg_field_value_dump(tbl_auth_cfg_t *p_auth_cfg, int32 field_id, char* str);

char**
tbl_auth_cfg_table_dump(tbl_auth_cfg_t *p_auth_cfg, char** str);

int32
tbl_auth_cfg_table_parser(char** array, char* key_value, tbl_auth_cfg_t *p_auth_cfg);

int32
tbl_auth_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_auth_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_auth_cfg_t*
tbl_auth_cfg_init_auth_cfg();

#endif /* !__TBL_AUTH_CFG_H__ */

