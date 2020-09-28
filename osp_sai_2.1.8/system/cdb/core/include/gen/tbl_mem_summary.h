
#ifndef __TBL_MEM_SUMMARY_H__
#define __TBL_MEM_SUMMARY_H__

int32
tbl_mem_summary_set_mem_summary_field_sync(tbl_mem_summary_t *p_mem_info, tbl_mem_summary_field_id_t field_id, uint32 sync);

int32
tbl_mem_summary_set_mem_summary_field(tbl_mem_summary_t *p_mem_info, tbl_mem_summary_field_id_t field_id);

tbl_mem_summary_t*
tbl_mem_summary_get_mem_summary();

char*
tbl_mem_summary_key_val2str(tbl_mem_summary_t *p_mem_info, char *str, uint32 str_len);

int32
tbl_mem_summary_key_str2val(char *str, tbl_mem_summary_t *p_mem_info);

int32
tbl_mem_summary_dump_one(tbl_mem_summary_t *p_mem_info, tbl_iter_args_t *pargs);

char*
tbl_mem_summary_key_name_dump(tbl_mem_summary_t *p_mem_info, char* str);

char*
tbl_mem_summary_key_value_dump(tbl_mem_summary_t *p_mem_info, char* str);

char*
tbl_mem_summary_field_name_dump(tbl_mem_summary_t *p_mem_info, int32 field_id, char* str);

char*
tbl_mem_summary_field_value_dump(tbl_mem_summary_t *p_mem_info, int32 field_id, char* str);

char**
tbl_mem_summary_table_dump(tbl_mem_summary_t *p_mem_info, char** str);

int32
tbl_mem_summary_table_parser(char** array, char* key_value, tbl_mem_summary_t *p_mem_info);

int32
tbl_mem_summary_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_mem_summary_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mem_summary_t*
tbl_mem_summary_init_mem_summary();

#endif /* !__TBL_MEM_SUMMARY_H__ */

