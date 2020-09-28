
#ifndef __DS_STATIC_ROUTE_DUP_IPV4_NH_H__
#define __DS_STATIC_ROUTE_DUP_IPV4_NH_H__

int32
ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, uint32 sync);

int32
ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, uint32 sync);

int32
ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field_sync(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, ds_static_route_dup_ipv4_nh_field_id_t field_id, uint32 sync);

int32
ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

int32
ds_static_route_dup_ipv4_nh_del_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

int32
ds_static_route_dup_ipv4_nh_set_static_route_dup_ipv4_nh_field(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, ds_static_route_dup_ipv4_nh_field_id_t field_id);

ds_static_route_dup_ipv4_nh_t*
ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

int32
ds_static_route_dup_ipv4_nh_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

char*
ds_static_route_dup_ipv4_nh_key_name_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char* str);

char*
ds_static_route_dup_ipv4_nh_key_value_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char* str);

char*
ds_static_route_dup_ipv4_nh_field_name_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, int32 field_id, char* str);

char*
ds_static_route_dup_ipv4_nh_field_value_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, int32 field_id, char* str);

char**
ds_static_route_dup_ipv4_nh_table_dump(tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, char** str);

int32
ds_static_route_dup_ipv4_nh_field_value_parser(char* str, int32 field_id, tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

int32
ds_static_route_dup_ipv4_nh_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_arp_t *p_arp, ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh);

int32
ds_static_route_dup_ipv4_nh_dump_one(ds_static_route_dup_ipv4_nh_t *p_static_route_dup_ipv4_nh, tbl_iter_args_t *pargs);

#endif /* !__DS_STATIC_ROUTE_DUP_IPV4_NH_H__ */

