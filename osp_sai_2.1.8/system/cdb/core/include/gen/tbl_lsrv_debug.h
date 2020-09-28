
#ifndef __TBL_LSRV_DEBUG_H__
#define __TBL_LSRV_DEBUG_H__

int32
tbl_lsrv_debug_set_lsrv_debug_field_sync(tbl_lsrv_debug_t *p_lsrvdbg, tbl_lsrv_debug_field_id_t field_id, uint32 sync);

int32
tbl_lsrv_debug_set_lsrv_debug_field(tbl_lsrv_debug_t *p_lsrvdbg, tbl_lsrv_debug_field_id_t field_id);

tbl_lsrv_debug_t*
tbl_lsrv_debug_get_lsrv_debug();

char*
tbl_lsrv_debug_key_val2str(tbl_lsrv_debug_t *p_lsrvdbg, char *str, uint32 str_len);

int32
tbl_lsrv_debug_key_str2val(char *str, tbl_lsrv_debug_t *p_lsrvdbg);

int32
tbl_lsrv_debug_dump_one(tbl_lsrv_debug_t *p_lsrvdbg, tbl_iter_args_t *pargs);

char*
tbl_lsrv_debug_key_name_dump(tbl_lsrv_debug_t *p_lsrvdbg, char* str);

char*
tbl_lsrv_debug_key_value_dump(tbl_lsrv_debug_t *p_lsrvdbg, char* str);

char*
tbl_lsrv_debug_field_name_dump(tbl_lsrv_debug_t *p_lsrvdbg, int32 field_id, char* str);

char*
tbl_lsrv_debug_field_value_dump(tbl_lsrv_debug_t *p_lsrvdbg, int32 field_id, char* str);

char**
tbl_lsrv_debug_table_dump(tbl_lsrv_debug_t *p_lsrvdbg, char** str);

int32
tbl_lsrv_debug_table_parser(char** array, char* key_value, tbl_lsrv_debug_t *p_lsrvdbg);

int32
tbl_lsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_lsrv_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_lsrv_debug_t*
tbl_lsrv_debug_init_lsrv_debug();

#endif /* !__TBL_LSRV_DEBUG_H__ */

