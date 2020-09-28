
#ifndef __TBL_NS_ROUTE_IP_H__
#define __TBL_NS_ROUTE_IP_H__

int32
tbl_ns_route_ip_add_ns_route_ip_sync(tbl_ns_route_ip_t *p_ns_route_ip, uint32 sync);

int32
tbl_ns_route_ip_del_ns_route_ip_sync(tbl_ns_route_ip_key_t *p_ns_route_ip_key, uint32 sync);

int32
tbl_ns_route_ip_set_ns_route_ip_field_sync(tbl_ns_route_ip_t *p_ns_route_ip, tbl_ns_route_ip_field_id_t field_id, uint32 sync);

int32
tbl_ns_route_ip_add_ns_route_ip(tbl_ns_route_ip_t *p_ns_route_ip);

int32
tbl_ns_route_ip_del_ns_route_ip(tbl_ns_route_ip_key_t *p_ns_route_ip_key);

int32
tbl_ns_route_ip_set_ns_route_ip_field(tbl_ns_route_ip_t *p_ns_route_ip, tbl_ns_route_ip_field_id_t field_id);

tbl_ns_route_ip_t*
tbl_ns_route_ip_get_ns_route_ip(tbl_ns_route_ip_key_t *p_ns_route_ip_key);

char*
tbl_ns_route_ip_key_val2str(tbl_ns_route_ip_t *p_ns_route_ip, char *str, uint32 str_len);

int32
tbl_ns_route_ip_key_str2val(char *str, tbl_ns_route_ip_t *p_ns_route_ip);

int32
tbl_ns_route_ip_dump_one(tbl_ns_route_ip_t *p_ns_route_ip, tbl_iter_args_t *pargs);

char*
tbl_ns_route_ip_key_name_dump(tbl_ns_route_ip_t *p_ns_route_ip, char* str);

char*
tbl_ns_route_ip_key_value_dump(tbl_ns_route_ip_t *p_ns_route_ip, char* str);

char*
tbl_ns_route_ip_field_name_dump(tbl_ns_route_ip_t *p_ns_route_ip, int32 field_id, char* str);

char*
tbl_ns_route_ip_field_value_dump(tbl_ns_route_ip_t *p_ns_route_ip, int32 field_id, char* str);

char**
tbl_ns_route_ip_table_dump(tbl_ns_route_ip_t *p_ns_route_ip, char** str);

int32
tbl_ns_route_ip_table_parser(char** array, char* key_value, tbl_ns_route_ip_t *p_ns_route_ip);

int32
tbl_ns_route_ip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ns_route_ip_master_t*
tbl_ns_route_ip_get_master();

tbl_ns_route_ip_master_t*
tbl_ns_route_ip_init_ns_route_ip();

#endif /* !__TBL_NS_ROUTE_IP_H__ */

