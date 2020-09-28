
#ifndef __TBL_ROUTE_GLOBAL_H__
#define __TBL_ROUTE_GLOBAL_H__

int32
tbl_route_global_set_route_global_field_sync(tbl_route_global_t *p_rt_glb, tbl_route_global_field_id_t field_id, uint32 sync);

int32
tbl_route_global_set_route_global_field(tbl_route_global_t *p_rt_glb, tbl_route_global_field_id_t field_id);

tbl_route_global_t*
tbl_route_global_get_route_global();

char*
tbl_route_global_key_val2str(tbl_route_global_t *p_rt_glb, char *str, uint32 str_len);

int32
tbl_route_global_key_str2val(char *str, tbl_route_global_t *p_rt_glb);

int32
tbl_route_global_dump_one(tbl_route_global_t *p_rt_glb, tbl_iter_args_t *pargs);

char*
tbl_route_global_key_name_dump(tbl_route_global_t *p_rt_glb, char* str);

char*
tbl_route_global_key_value_dump(tbl_route_global_t *p_rt_glb, char* str);

char*
tbl_route_global_field_name_dump(tbl_route_global_t *p_rt_glb, int32 field_id, char* str);

char*
tbl_route_global_field_value_dump(tbl_route_global_t *p_rt_glb, int32 field_id, char* str);

char**
tbl_route_global_table_dump(tbl_route_global_t *p_rt_glb, char** str);

int32
tbl_route_global_table_parser(char** array, char* key_value, tbl_route_global_t *p_rt_glb);

int32
tbl_route_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_route_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_route_global_t*
tbl_route_global_init_route_global();

#endif /* !__TBL_ROUTE_GLOBAL_H__ */

