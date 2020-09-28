
#ifndef __TBL_SYS_LOAD_H__
#define __TBL_SYS_LOAD_H__

int32
tbl_sys_load_set_sys_load_field_sync(tbl_sys_load_t *p_sys_load, tbl_sys_load_field_id_t field_id, uint32 sync);

int32
tbl_sys_load_set_sys_load_field(tbl_sys_load_t *p_sys_load, tbl_sys_load_field_id_t field_id);

tbl_sys_load_t*
tbl_sys_load_get_sys_load();

char*
tbl_sys_load_key_val2str(tbl_sys_load_t *p_sys_load, char *str, uint32 str_len);

int32
tbl_sys_load_key_str2val(char *str, tbl_sys_load_t *p_sys_load);

int32
tbl_sys_load_dump_one(tbl_sys_load_t *p_sys_load, tbl_iter_args_t *pargs);

char*
tbl_sys_load_key_name_dump(tbl_sys_load_t *p_sys_load, char* str);

char*
tbl_sys_load_key_value_dump(tbl_sys_load_t *p_sys_load, char* str);

char*
tbl_sys_load_field_name_dump(tbl_sys_load_t *p_sys_load, int32 field_id, char* str);

char*
tbl_sys_load_field_value_dump(tbl_sys_load_t *p_sys_load, int32 field_id, char* str);

char**
tbl_sys_load_table_dump(tbl_sys_load_t *p_sys_load, char** str);

int32
tbl_sys_load_table_parser(char** array, char* key_value, tbl_sys_load_t *p_sys_load);

int32
tbl_sys_load_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_sys_load_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_sys_load_t*
tbl_sys_load_init_sys_load();

#endif /* !__TBL_SYS_LOAD_H__ */

