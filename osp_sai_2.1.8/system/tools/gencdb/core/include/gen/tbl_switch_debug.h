
#ifndef __TBL_SWITCH_DEBUG_H__
#define __TBL_SWITCH_DEBUG_H__

int32
tbl_switch_debug_set_switch_debug_field_sync(tbl_switch_debug_t *p_swthdbg, tbl_switch_debug_field_id_t field_id, uint32 sync);

int32
tbl_switch_debug_set_switch_debug_field(tbl_switch_debug_t *p_swthdbg, tbl_switch_debug_field_id_t field_id);

tbl_switch_debug_t*
tbl_switch_debug_get_switch_debug();

char*
tbl_switch_debug_key_val2str(tbl_switch_debug_t *p_swthdbg, char *str, uint32 str_len);

int32
tbl_switch_debug_key_str2val(char *str, tbl_switch_debug_t *p_swthdbg);

int32
tbl_switch_debug_dump_one(tbl_switch_debug_t *p_swthdbg, tbl_iter_args_t *pargs);

char*
tbl_switch_debug_key_name_dump(tbl_switch_debug_t *p_swthdbg, char* str);

char*
tbl_switch_debug_key_value_dump(tbl_switch_debug_t *p_swthdbg, char* str);

char*
tbl_switch_debug_field_name_dump(tbl_switch_debug_t *p_swthdbg, int32 field_id, char* str);

char*
tbl_switch_debug_field_value_dump(tbl_switch_debug_t *p_swthdbg, int32 field_id, char* str);

char**
tbl_switch_debug_table_dump(tbl_switch_debug_t *p_swthdbg, char** str);

int32
tbl_switch_debug_table_parser(char** array, char* key_value, tbl_switch_debug_t *p_swthdbg);

int32
tbl_switch_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_switch_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_switch_debug_t*
tbl_switch_debug_init_switch_debug();

#endif /* !__TBL_SWITCH_DEBUG_H__ */

