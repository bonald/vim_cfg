
#ifndef __TBL_CPU_LIMIT_H__
#define __TBL_CPU_LIMIT_H__

int32
tbl_cpu_limit_add_cpu_limit_sync(tbl_cpu_limit_t *p_cpu_limit, uint32 sync);

int32
tbl_cpu_limit_del_cpu_limit_sync(tbl_cpu_limit_key_t *p_cpu_limit_key, uint32 sync);

int32
tbl_cpu_limit_set_cpu_limit_field_sync(tbl_cpu_limit_t *p_cpu_limit, tbl_cpu_limit_field_id_t field_id, uint32 sync);

int32
tbl_cpu_limit_add_cpu_limit(tbl_cpu_limit_t *p_cpu_limit);

int32
tbl_cpu_limit_del_cpu_limit(tbl_cpu_limit_key_t *p_cpu_limit_key);

int32
tbl_cpu_limit_set_cpu_limit_field(tbl_cpu_limit_t *p_cpu_limit, tbl_cpu_limit_field_id_t field_id);

tbl_cpu_limit_t*
tbl_cpu_limit_get_cpu_limit(tbl_cpu_limit_key_t *p_cpu_limit_key);

char*
tbl_cpu_limit_key_val2str(tbl_cpu_limit_t *p_cpu_limit, char *str, uint32 str_len);

int32
tbl_cpu_limit_key_str2val(char *str, tbl_cpu_limit_t *p_cpu_limit);

int32
tbl_cpu_limit_dump_one(tbl_cpu_limit_t *p_cpu_limit, tbl_iter_args_t *pargs);

char*
tbl_cpu_limit_key_name_dump(tbl_cpu_limit_t *p_cpu_limit, char* str);

char*
tbl_cpu_limit_key_value_dump(tbl_cpu_limit_t *p_cpu_limit, char* str);

char*
tbl_cpu_limit_field_name_dump(tbl_cpu_limit_t *p_cpu_limit, int32 field_id, char* str);

char*
tbl_cpu_limit_field_value_dump(tbl_cpu_limit_t *p_cpu_limit, int32 field_id, char* str);

char**
tbl_cpu_limit_table_dump(tbl_cpu_limit_t *p_cpu_limit, char** str);

int32
tbl_cpu_limit_table_parser(char** array, char* key_value, tbl_cpu_limit_t *p_cpu_limit);

int32
tbl_cpu_limit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_limit_master_t*
tbl_cpu_limit_get_master();

tbl_cpu_limit_master_t*
tbl_cpu_limit_init_cpu_limit();

#endif /* !__TBL_CPU_LIMIT_H__ */

