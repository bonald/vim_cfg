
#ifndef __TBL_TPOAM_EXEC_GLOBAL_H__
#define __TBL_TPOAM_EXEC_GLOBAL_H__

int32
tbl_tpoam_exec_global_set_tpoam_exec_global_field_sync(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_tpoam_exec_global_field_id_t field_id, uint32 sync);

int32
tbl_tpoam_exec_global_set_tpoam_exec_global_field(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_tpoam_exec_global_field_id_t field_id);

tbl_tpoam_exec_global_t*
tbl_tpoam_exec_global_get_tpoam_exec_global();

char*
tbl_tpoam_exec_global_key_val2str(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char *str, uint32 str_len);

int32
tbl_tpoam_exec_global_key_str2val(char *str, tbl_tpoam_exec_global_t *p_tpoam_exec_global);

int32
tbl_tpoam_exec_global_dump_one(tbl_tpoam_exec_global_t *p_tpoam_exec_global, tbl_iter_args_t *pargs);

char*
tbl_tpoam_exec_global_key_name_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char* str);

char*
tbl_tpoam_exec_global_key_value_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char* str);

char*
tbl_tpoam_exec_global_field_name_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, int32 field_id, char* str);

char*
tbl_tpoam_exec_global_field_value_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, int32 field_id, char* str);

char**
tbl_tpoam_exec_global_table_dump(tbl_tpoam_exec_global_t *p_tpoam_exec_global, char** str);

int32
tbl_tpoam_exec_global_table_parser(char** array, char* key_value, tbl_tpoam_exec_global_t *p_tpoam_exec_global);

int32
tbl_tpoam_exec_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_tpoam_exec_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tpoam_exec_global_t*
tbl_tpoam_exec_global_init_tpoam_exec_global();

#endif /* !__TBL_TPOAM_EXEC_GLOBAL_H__ */

