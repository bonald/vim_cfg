
#ifndef __TBL_FEA_LAG_H__
#define __TBL_FEA_LAG_H__

int32
tbl_fea_lag_add_fea_lag_sync(tbl_fea_lag_t *p_fea_lag, uint32 sync);

int32
tbl_fea_lag_del_fea_lag_sync(tbl_fea_lag_key_t *p_fea_lag_key, uint32 sync);

int32
tbl_fea_lag_set_fea_lag_field_sync(tbl_fea_lag_t *p_fea_lag, tbl_fea_lag_field_id_t field_id, uint32 sync);

int32
tbl_fea_lag_add_fea_lag(tbl_fea_lag_t *p_fea_lag);

int32
tbl_fea_lag_del_fea_lag(tbl_fea_lag_key_t *p_fea_lag_key);

int32
tbl_fea_lag_set_fea_lag_field(tbl_fea_lag_t *p_fea_lag, tbl_fea_lag_field_id_t field_id);

tbl_fea_lag_t*
tbl_fea_lag_get_fea_lag(tbl_fea_lag_key_t *p_fea_lag_key);

char*
tbl_fea_lag_key_val2str(tbl_fea_lag_t *p_fea_lag, char *str, uint32 str_len);

int32
tbl_fea_lag_key_str2val(char *str, tbl_fea_lag_t *p_fea_lag);

int32
tbl_fea_lag_dump_one(tbl_fea_lag_t *p_fea_lag, tbl_iter_args_t *pargs);

char*
tbl_fea_lag_key_name_dump(tbl_fea_lag_t *p_fea_lag, char* str);

char*
tbl_fea_lag_key_value_dump(tbl_fea_lag_t *p_fea_lag, char* str);

char*
tbl_fea_lag_field_name_dump(tbl_fea_lag_t *p_fea_lag, int32 field_id, char* str);

char*
tbl_fea_lag_field_value_dump(tbl_fea_lag_t *p_fea_lag, int32 field_id, char* str);

char**
tbl_fea_lag_table_dump(tbl_fea_lag_t *p_fea_lag, char** str);

int32
tbl_fea_lag_table_parser(char** array, char* key_value, tbl_fea_lag_t *p_fea_lag);

int32
tbl_fea_lag_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_lag_master_t*
tbl_fea_lag_get_master();

tbl_fea_lag_master_t*
tbl_fea_lag_init_fea_lag();

#endif /* !__TBL_FEA_LAG_H__ */

