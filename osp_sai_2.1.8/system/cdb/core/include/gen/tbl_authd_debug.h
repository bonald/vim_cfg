
#ifndef __TBL_AUTHD_DEBUG_H__
#define __TBL_AUTHD_DEBUG_H__

int32
tbl_authd_debug_set_authd_debug_field_sync(tbl_authd_debug_t *p_authd_debug, tbl_authd_debug_field_id_t field_id, uint32 sync);

int32
tbl_authd_debug_set_authd_debug_field(tbl_authd_debug_t *p_authd_debug, tbl_authd_debug_field_id_t field_id);

tbl_authd_debug_t*
tbl_authd_debug_get_authd_debug();

char*
tbl_authd_debug_key_val2str(tbl_authd_debug_t *p_authd_debug, char *str, uint32 str_len);

int32
tbl_authd_debug_key_str2val(char *str, tbl_authd_debug_t *p_authd_debug);

int32
tbl_authd_debug_dump_one(tbl_authd_debug_t *p_authd_debug, tbl_iter_args_t *pargs);

char*
tbl_authd_debug_key_name_dump(tbl_authd_debug_t *p_authd_debug, char* str);

char*
tbl_authd_debug_key_value_dump(tbl_authd_debug_t *p_authd_debug, char* str);

char*
tbl_authd_debug_field_name_dump(tbl_authd_debug_t *p_authd_debug, int32 field_id, char* str);

char*
tbl_authd_debug_field_value_dump(tbl_authd_debug_t *p_authd_debug, int32 field_id, char* str);

char**
tbl_authd_debug_table_dump(tbl_authd_debug_t *p_authd_debug, char** str);

int32
tbl_authd_debug_table_parser(char** array, char* key_value, tbl_authd_debug_t *p_authd_debug);

int32
tbl_authd_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_authd_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_authd_debug_t*
tbl_authd_debug_init_authd_debug();

#endif /* !__TBL_AUTHD_DEBUG_H__ */

