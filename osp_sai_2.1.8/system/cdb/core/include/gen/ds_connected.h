
#ifndef __DS_CONNECTED_H__
#define __DS_CONNECTED_H__

int32
ds_connected_add_connected_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, uint32 sync);

int32
ds_connected_del_connected_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, uint32 sync);

int32
ds_connected_set_connected_field_sync(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, ds_connected_field_id_t field_id, uint32 sync);

int32
ds_connected_add_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

int32
ds_connected_del_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

int32
ds_connected_set_connected_field(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, ds_connected_field_id_t field_id);

ds_connected_t*
ds_connected_get_connected(tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

int32
ds_connected_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

char*
ds_connected_key_name_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char* str);

char*
ds_connected_key_value_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char* str);

char*
ds_connected_field_name_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, int32 field_id, char* str);

char*
ds_connected_field_value_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, int32 field_id, char* str);

char**
ds_connected_table_dump(tbl_route_if_t *p_rtif, ds_connected_t *p_connect, char** str);

int32
ds_connected_field_value_parser(char* str, int32 field_id, tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

int32
ds_connected_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_connected_t *p_connect);

int32
ds_connected_dump_one(ds_connected_t *p_connect, tbl_iter_args_t *pargs);

int32
route_if_addr_exists(addr_t *p_addr);

#endif /* !__DS_CONNECTED_H__ */

