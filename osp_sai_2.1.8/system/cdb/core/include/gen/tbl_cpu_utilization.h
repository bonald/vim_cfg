
#ifndef __TBL_CPU_UTILIZATION_H__
#define __TBL_CPU_UTILIZATION_H__

int32
tbl_cpu_utilization_add_cpu_utilization_sync(tbl_cpu_utilization_t *p_cpu_utilize, uint32 sync);

int32
tbl_cpu_utilization_del_cpu_utilization_sync(tbl_cpu_utilization_key_t *p_cpu_utilize_key, uint32 sync);

int32
tbl_cpu_utilization_set_cpu_utilization_field_sync(tbl_cpu_utilization_t *p_cpu_utilize, tbl_cpu_utilization_field_id_t field_id, uint32 sync);

int32
tbl_cpu_utilization_add_cpu_utilization(tbl_cpu_utilization_t *p_cpu_utilize);

int32
tbl_cpu_utilization_del_cpu_utilization(tbl_cpu_utilization_key_t *p_cpu_utilize_key);

int32
tbl_cpu_utilization_set_cpu_utilization_field(tbl_cpu_utilization_t *p_cpu_utilize, tbl_cpu_utilization_field_id_t field_id);

tbl_cpu_utilization_t*
tbl_cpu_utilization_get_cpu_utilization(tbl_cpu_utilization_key_t *p_cpu_utilize_key);

char*
tbl_cpu_utilization_key_val2str(tbl_cpu_utilization_t *p_cpu_utilize, char *str, uint32 str_len);

int32
tbl_cpu_utilization_key_str2val(char *str, tbl_cpu_utilization_t *p_cpu_utilize);

int32
tbl_cpu_utilization_dump_one(tbl_cpu_utilization_t *p_cpu_utilize, tbl_iter_args_t *pargs);

char*
tbl_cpu_utilization_key_name_dump(tbl_cpu_utilization_t *p_cpu_utilize, char* str);

char*
tbl_cpu_utilization_key_value_dump(tbl_cpu_utilization_t *p_cpu_utilize, char* str);

char*
tbl_cpu_utilization_field_name_dump(tbl_cpu_utilization_t *p_cpu_utilize, int32 field_id, char* str);

char*
tbl_cpu_utilization_field_value_dump(tbl_cpu_utilization_t *p_cpu_utilize, int32 field_id, char* str);

char**
tbl_cpu_utilization_table_dump(tbl_cpu_utilization_t *p_cpu_utilize, char** str);

int32
tbl_cpu_utilization_table_parser(char** array, char* key_value, tbl_cpu_utilization_t *p_cpu_utilize);

int32
tbl_cpu_utilization_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_utilization_master_t*
tbl_cpu_utilization_get_master();

tbl_cpu_utilization_master_t*
tbl_cpu_utilization_init_cpu_utilization();

#endif /* !__TBL_CPU_UTILIZATION_H__ */

