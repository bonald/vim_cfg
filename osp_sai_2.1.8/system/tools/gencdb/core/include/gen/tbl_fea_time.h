
#ifndef __TBL_FEA_TIME_H__
#define __TBL_FEA_TIME_H__

int32
tbl_fea_time_set_fea_time_field_sync(tbl_fea_time_t *p_fea_time, tbl_fea_time_field_id_t field_id, uint32 sync);

int32
tbl_fea_time_set_fea_time_field(tbl_fea_time_t *p_fea_time, tbl_fea_time_field_id_t field_id);

tbl_fea_time_t*
tbl_fea_time_get_fea_time();

char*
tbl_fea_time_key_val2str(tbl_fea_time_t *p_fea_time, char *str, uint32 str_len);

int32
tbl_fea_time_key_str2val(char *str, tbl_fea_time_t *p_fea_time);

int32
tbl_fea_time_dump_one(tbl_fea_time_t *p_fea_time, tbl_iter_args_t *pargs);

char*
tbl_fea_time_key_name_dump(tbl_fea_time_t *p_fea_time, char* str);

char*
tbl_fea_time_key_value_dump(tbl_fea_time_t *p_fea_time, char* str);

char*
tbl_fea_time_field_name_dump(tbl_fea_time_t *p_fea_time, int32 field_id, char* str);

char*
tbl_fea_time_field_value_dump(tbl_fea_time_t *p_fea_time, int32 field_id, char* str);

char**
tbl_fea_time_table_dump(tbl_fea_time_t *p_fea_time, char** str);

int32
tbl_fea_time_table_parser(char** array, char* key_value, tbl_fea_time_t *p_fea_time);

int32
tbl_fea_time_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_fea_time_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_time_t*
tbl_fea_time_init_fea_time();

#endif /* !__TBL_FEA_TIME_H__ */

