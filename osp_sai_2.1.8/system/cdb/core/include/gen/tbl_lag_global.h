
#ifndef __TBL_LAG_GLOBAL_H__
#define __TBL_LAG_GLOBAL_H__

int32
tbl_lag_global_set_lag_global_field_sync(tbl_lag_global_t *p_lag_glb, tbl_lag_global_field_id_t field_id, uint32 sync);

int32
tbl_lag_global_set_lag_global_field(tbl_lag_global_t *p_lag_glb, tbl_lag_global_field_id_t field_id);

tbl_lag_global_t*
tbl_lag_global_get_lag_global();

char*
tbl_lag_global_key_val2str(tbl_lag_global_t *p_lag_glb, char *str, uint32 str_len);

int32
tbl_lag_global_key_str2val(char *str, tbl_lag_global_t *p_lag_glb);

int32
tbl_lag_global_dump_one(tbl_lag_global_t *p_lag_glb, tbl_iter_args_t *pargs);

char*
tbl_lag_global_key_name_dump(tbl_lag_global_t *p_lag_glb, char* str);

char*
tbl_lag_global_key_value_dump(tbl_lag_global_t *p_lag_glb, char* str);

char*
tbl_lag_global_field_name_dump(tbl_lag_global_t *p_lag_glb, int32 field_id, char* str);

char*
tbl_lag_global_field_value_dump(tbl_lag_global_t *p_lag_glb, int32 field_id, char* str);

char**
tbl_lag_global_table_dump(tbl_lag_global_t *p_lag_glb, char** str);

int32
tbl_lag_global_table_parser(char** array, char* key_value, tbl_lag_global_t *p_lag_glb);

int32
tbl_lag_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_lag_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_lag_global_t*
tbl_lag_global_init_lag_global();

#endif /* !__TBL_LAG_GLOBAL_H__ */

