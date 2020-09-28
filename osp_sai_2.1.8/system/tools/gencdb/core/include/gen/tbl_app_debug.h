
#ifndef __TBL_APP_DEBUG_H__
#define __TBL_APP_DEBUG_H__

int32
tbl_app_debug_set_app_debug_field_sync(tbl_app_debug_t *p_appdbg, tbl_app_debug_field_id_t field_id, uint32 sync);

int32
tbl_app_debug_set_app_debug_field(tbl_app_debug_t *p_appdbg, tbl_app_debug_field_id_t field_id);

tbl_app_debug_t*
tbl_app_debug_get_app_debug();

char*
tbl_app_debug_key_val2str(tbl_app_debug_t *p_appdbg, char *str, uint32 str_len);

int32
tbl_app_debug_key_str2val(char *str, tbl_app_debug_t *p_appdbg);

int32
tbl_app_debug_dump_one(tbl_app_debug_t *p_appdbg, tbl_iter_args_t *pargs);

char*
tbl_app_debug_key_name_dump(tbl_app_debug_t *p_appdbg, char* str);

char*
tbl_app_debug_key_value_dump(tbl_app_debug_t *p_appdbg, char* str);

char*
tbl_app_debug_field_name_dump(tbl_app_debug_t *p_appdbg, int32 field_id, char* str);

char*
tbl_app_debug_field_value_dump(tbl_app_debug_t *p_appdbg, int32 field_id, char* str);

char**
tbl_app_debug_table_dump(tbl_app_debug_t *p_appdbg, char** str);

int32
tbl_app_debug_table_parser(char** array, char* key_value, tbl_app_debug_t *p_appdbg);

int32
tbl_app_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_app_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_app_debug_t*
tbl_app_debug_init_app_debug();

#endif /* !__TBL_APP_DEBUG_H__ */

