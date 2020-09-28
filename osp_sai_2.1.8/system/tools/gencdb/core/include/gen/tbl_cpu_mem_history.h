
#ifndef __TBL_CPU_MEM_HISTORY_H__
#define __TBL_CPU_MEM_HISTORY_H__

int32
tbl_cpu_mem_history_set_cpu_mem_history_field_sync(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_cpu_mem_history_field_id_t field_id, uint32 sync);

int32
tbl_cpu_mem_history_set_cpu_mem_history_field(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_cpu_mem_history_field_id_t field_id);

tbl_cpu_mem_history_t*
tbl_cpu_mem_history_get_cpu_mem_history();

char*
tbl_cpu_mem_history_key_val2str(tbl_cpu_mem_history_t *p_cpu_mem_history, char *str, uint32 str_len);

int32
tbl_cpu_mem_history_key_str2val(char *str, tbl_cpu_mem_history_t *p_cpu_mem_history);

int32
tbl_cpu_mem_history_dump_one(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_iter_args_t *pargs);

char*
tbl_cpu_mem_history_key_name_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char* str);

char*
tbl_cpu_mem_history_key_value_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char* str);

char*
tbl_cpu_mem_history_field_name_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, int32 field_id, char* str);

char*
tbl_cpu_mem_history_field_value_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, int32 field_id, char* str);

char**
tbl_cpu_mem_history_table_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char** str);

int32
tbl_cpu_mem_history_table_parser(char** array, char* key_value, tbl_cpu_mem_history_t *p_cpu_mem_history);

int32
tbl_cpu_mem_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_cpu_mem_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_mem_history_t*
tbl_cpu_mem_history_init_cpu_mem_history();

#endif /* !__TBL_CPU_MEM_HISTORY_H__ */

