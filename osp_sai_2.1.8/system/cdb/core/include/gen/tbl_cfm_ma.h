
#ifndef __TBL_CFM_MA_H__
#define __TBL_CFM_MA_H__

int32
tbl_cfm_ma_add_cfm_ma_sync(tbl_cfm_ma_t *p_ma, uint32 sync);

int32
tbl_cfm_ma_del_cfm_ma_sync(tbl_cfm_ma_key_t *p_ma_key, uint32 sync);

int32
tbl_cfm_ma_set_cfm_ma_field_sync(tbl_cfm_ma_t *p_ma, tbl_cfm_ma_field_id_t field_id, uint32 sync);

int32
tbl_cfm_ma_add_cfm_ma(tbl_cfm_ma_t *p_ma);

int32
tbl_cfm_ma_del_cfm_ma(tbl_cfm_ma_key_t *p_ma_key);

int32
tbl_cfm_ma_set_cfm_ma_field(tbl_cfm_ma_t *p_ma, tbl_cfm_ma_field_id_t field_id);

tbl_cfm_ma_t*
tbl_cfm_ma_get_cfm_ma(tbl_cfm_ma_key_t *p_ma_key);

char*
tbl_cfm_ma_key_val2str(tbl_cfm_ma_t *p_ma, char *str, uint32 str_len);

int32
tbl_cfm_ma_key_str2val(char *str, tbl_cfm_ma_t *p_ma);

int32
tbl_cfm_ma_dump_one(tbl_cfm_ma_t *p_ma, tbl_iter_args_t *pargs);

char*
tbl_cfm_ma_key_name_dump(tbl_cfm_ma_t *p_ma, char* str);

char*
tbl_cfm_ma_key_value_dump(tbl_cfm_ma_t *p_ma, char* str);

char*
tbl_cfm_ma_field_name_dump(tbl_cfm_ma_t *p_ma, int32 field_id, char* str);

char*
tbl_cfm_ma_field_value_dump(tbl_cfm_ma_t *p_ma, int32 field_id, char* str);

char**
tbl_cfm_ma_table_dump(tbl_cfm_ma_t *p_ma, char** str);

int32
tbl_cfm_ma_table_parser(char** array, char* key_value, tbl_cfm_ma_t *p_ma);

int32
tbl_cfm_ma_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_ma_master_t*
tbl_cfm_ma_get_master();

tbl_cfm_ma_master_t*
tbl_cfm_ma_init_cfm_ma();

#endif /* !__TBL_CFM_MA_H__ */

