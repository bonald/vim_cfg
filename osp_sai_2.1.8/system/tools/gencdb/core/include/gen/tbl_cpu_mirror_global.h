
#ifndef __TBL_CPU_MIRROR_GLOBAL_H__
#define __TBL_CPU_MIRROR_GLOBAL_H__

int32
tbl_cpu_mirror_global_set_cpu_mirror_global_field_sync(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_cpu_mirror_global_field_id_t field_id, uint32 sync);

int32
tbl_cpu_mirror_global_set_cpu_mirror_global_field(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_cpu_mirror_global_field_id_t field_id);

tbl_cpu_mirror_global_t*
tbl_cpu_mirror_global_get_cpu_mirror_global();

char*
tbl_cpu_mirror_global_key_val2str(tbl_cpu_mirror_global_t *p_cpu_mirror, char *str, uint32 str_len);

int32
tbl_cpu_mirror_global_key_str2val(char *str, tbl_cpu_mirror_global_t *p_cpu_mirror);

int32
tbl_cpu_mirror_global_dump_one(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_iter_args_t *pargs);

char*
tbl_cpu_mirror_global_key_name_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char* str);

char*
tbl_cpu_mirror_global_key_value_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char* str);

char*
tbl_cpu_mirror_global_field_name_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, int32 field_id, char* str);

char*
tbl_cpu_mirror_global_field_value_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, int32 field_id, char* str);

char**
tbl_cpu_mirror_global_table_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char** str);

int32
tbl_cpu_mirror_global_table_parser(char** array, char* key_value, tbl_cpu_mirror_global_t *p_cpu_mirror);

int32
tbl_cpu_mirror_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_cpu_mirror_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_mirror_global_t*
tbl_cpu_mirror_global_init_cpu_mirror_global();

#endif /* !__TBL_CPU_MIRROR_GLOBAL_H__ */

