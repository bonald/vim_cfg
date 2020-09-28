
#ifndef __TBL_TIME_RANGE_H__
#define __TBL_TIME_RANGE_H__

int32
tbl_time_range_add_time_range_sync(tbl_time_range_t *p_time_range, uint32 sync);

int32
tbl_time_range_del_time_range_sync(tbl_time_range_key_t *p_time_range_key, uint32 sync);

int32
tbl_time_range_set_time_range_field_sync(tbl_time_range_t *p_time_range, tbl_time_range_field_id_t field_id, uint32 sync);

int32
tbl_time_range_add_time_range(tbl_time_range_t *p_time_range);

int32
tbl_time_range_del_time_range(tbl_time_range_key_t *p_time_range_key);

int32
tbl_time_range_set_time_range_field(tbl_time_range_t *p_time_range, tbl_time_range_field_id_t field_id);

tbl_time_range_t*
tbl_time_range_get_time_range(tbl_time_range_key_t *p_time_range_key);

char*
tbl_time_range_key_val2str(tbl_time_range_t *p_time_range, char *str, uint32 str_len);

int32
tbl_time_range_key_str2val(char *str, tbl_time_range_t *p_time_range);

int32
tbl_time_range_dump_one(tbl_time_range_t *p_time_range, tbl_iter_args_t *pargs);

char*
tbl_time_range_key_name_dump(tbl_time_range_t *p_time_range, char* str);

char*
tbl_time_range_key_value_dump(tbl_time_range_t *p_time_range, char* str);

char*
tbl_time_range_field_name_dump(tbl_time_range_t *p_time_range, int32 field_id, char* str);

char*
tbl_time_range_field_value_dump(tbl_time_range_t *p_time_range, int32 field_id, char* str);

char**
tbl_time_range_table_dump(tbl_time_range_t *p_time_range, char** str);

int32
tbl_time_range_table_parser(char** array, char* key_value, tbl_time_range_t *p_time_range);

int32
tbl_time_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_time_range_master_t*
tbl_time_range_get_master();

tbl_time_range_master_t*
tbl_time_range_init_time_range();

tbl_time_range_t*
tbl_time_range_get_time_range_by_name(char *name);

int32
tbl_time_range_get_time_range_count();


#endif /* !__TBL_TIME_RANGE_H__ */

