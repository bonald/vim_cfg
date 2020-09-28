
#ifndef __TBL_NS_ROUTE_H__
#define __TBL_NS_ROUTE_H__

int32
tbl_ns_route_add_ns_route_sync(tbl_ns_route_t *p_ns_route, uint32 sync);

int32
tbl_ns_route_del_ns_route_sync(tbl_ns_route_key_t *p_ns_route_key, uint32 sync);

int32
tbl_ns_route_set_ns_route_field_sync(tbl_ns_route_t *p_ns_route, tbl_ns_route_field_id_t field_id, uint32 sync);

int32
tbl_ns_route_add_ns_route(tbl_ns_route_t *p_ns_route);

int32
tbl_ns_route_del_ns_route(tbl_ns_route_key_t *p_ns_route_key);

int32
tbl_ns_route_set_ns_route_field(tbl_ns_route_t *p_ns_route, tbl_ns_route_field_id_t field_id);

tbl_ns_route_t*
tbl_ns_route_get_ns_route(tbl_ns_route_key_t *p_ns_route_key);

char*
tbl_ns_route_key_val2str(tbl_ns_route_t *p_ns_route, char *str, uint32 str_len);

int32
tbl_ns_route_key_str2val(char *str, tbl_ns_route_t *p_ns_route);

int32
tbl_ns_route_dump_one(tbl_ns_route_t *p_ns_route, tbl_iter_args_t *pargs);

char*
tbl_ns_route_key_name_dump(tbl_ns_route_t *p_ns_route, char* str);

char*
tbl_ns_route_key_value_dump(tbl_ns_route_t *p_ns_route, char* str);

char*
tbl_ns_route_field_name_dump(tbl_ns_route_t *p_ns_route, int32 field_id, char* str);

char*
tbl_ns_route_field_value_dump(tbl_ns_route_t *p_ns_route, int32 field_id, char* str);

char**
tbl_ns_route_table_dump(tbl_ns_route_t *p_ns_route, char** str);

int32
tbl_ns_route_table_parser(char** array, char* key_value, tbl_ns_route_t *p_ns_route);

int32
tbl_ns_route_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ns_route_master_t*
tbl_ns_route_get_master();

tbl_ns_route_master_t*
tbl_ns_route_init_ns_route();

#endif /* !__TBL_NS_ROUTE_H__ */

