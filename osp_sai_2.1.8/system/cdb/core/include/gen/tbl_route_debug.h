
#ifndef __TBL_ROUTE_DEBUG_H__
#define __TBL_ROUTE_DEBUG_H__

int32
tbl_route_debug_set_route_debug_field_sync(tbl_route_debug_t *p_rtdbg, tbl_route_debug_field_id_t field_id, uint32 sync);

int32
tbl_route_debug_set_route_debug_field(tbl_route_debug_t *p_rtdbg, tbl_route_debug_field_id_t field_id);

tbl_route_debug_t*
tbl_route_debug_get_route_debug();

char*
tbl_route_debug_key_val2str(tbl_route_debug_t *p_rtdbg, char *str, uint32 str_len);

int32
tbl_route_debug_key_str2val(char *str, tbl_route_debug_t *p_rtdbg);

int32
tbl_route_debug_dump_one(tbl_route_debug_t *p_rtdbg, tbl_iter_args_t *pargs);

char*
tbl_route_debug_key_name_dump(tbl_route_debug_t *p_rtdbg, char* str);

char*
tbl_route_debug_key_value_dump(tbl_route_debug_t *p_rtdbg, char* str);

char*
tbl_route_debug_field_name_dump(tbl_route_debug_t *p_rtdbg, int32 field_id, char* str);

char*
tbl_route_debug_field_value_dump(tbl_route_debug_t *p_rtdbg, int32 field_id, char* str);

char**
tbl_route_debug_table_dump(tbl_route_debug_t *p_rtdbg, char** str);

int32
tbl_route_debug_table_parser(char** array, char* key_value, tbl_route_debug_t *p_rtdbg);

int32
tbl_route_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_route_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_route_debug_t*
tbl_route_debug_init_route_debug();

#endif /* !__TBL_ROUTE_DEBUG_H__ */

