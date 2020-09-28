
#ifndef __TBL_OPENFLOW_DEBUG_H__
#define __TBL_OPENFLOW_DEBUG_H__

int32
tbl_openflow_debug_set_openflow_debug_field_sync(tbl_openflow_debug_t *p_openflowdbg, tbl_openflow_debug_field_id_t field_id, uint32 sync);

int32
tbl_openflow_debug_set_openflow_debug_field(tbl_openflow_debug_t *p_openflowdbg, tbl_openflow_debug_field_id_t field_id);

tbl_openflow_debug_t*
tbl_openflow_debug_get_openflow_debug();

char*
tbl_openflow_debug_key_val2str(tbl_openflow_debug_t *p_openflowdbg, char *str, uint32 str_len);

int32
tbl_openflow_debug_key_str2val(char *str, tbl_openflow_debug_t *p_openflowdbg);

int32
tbl_openflow_debug_dump_one(tbl_openflow_debug_t *p_openflowdbg, tbl_iter_args_t *pargs);

char*
tbl_openflow_debug_key_name_dump(tbl_openflow_debug_t *p_openflowdbg, char* str);

char*
tbl_openflow_debug_key_value_dump(tbl_openflow_debug_t *p_openflowdbg, char* str);

char*
tbl_openflow_debug_field_name_dump(tbl_openflow_debug_t *p_openflowdbg, int32 field_id, char* str);

char*
tbl_openflow_debug_field_value_dump(tbl_openflow_debug_t *p_openflowdbg, int32 field_id, char* str);

char**
tbl_openflow_debug_table_dump(tbl_openflow_debug_t *p_openflowdbg, char** str);

int32
tbl_openflow_debug_table_parser(char** array, char* key_value, tbl_openflow_debug_t *p_openflowdbg);

int32
tbl_openflow_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_openflow_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_openflow_debug_t*
tbl_openflow_debug_init_openflow_debug();

#endif /* !__TBL_OPENFLOW_DEBUG_H__ */

