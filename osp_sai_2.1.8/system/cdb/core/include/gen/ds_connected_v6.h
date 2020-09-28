
#ifndef __DS_CONNECTED_V6_H__
#define __DS_CONNECTED_V6_H__

int32
ds_connected_v6_add_connected_v6_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, uint32 sync);

int32
ds_connected_v6_del_connected_v6_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, uint32 sync);

int32
ds_connected_v6_set_connected_v6_field_sync(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, ds_connected_v6_field_id_t field_id, uint32 sync);

int32
ds_connected_v6_add_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

int32
ds_connected_v6_del_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

int32
ds_connected_v6_set_connected_v6_field(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, ds_connected_v6_field_id_t field_id);

ds_connected_v6_t*
ds_connected_v6_get_connected_v6(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

int32
ds_connected_v6_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

char*
ds_connected_v6_key_name_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char* str);

char*
ds_connected_v6_key_value_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char* str);

char*
ds_connected_v6_field_name_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, int32 field_id, char* str);

char*
ds_connected_v6_field_value_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, int32 field_id, char* str);

char**
ds_connected_v6_table_dump(tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6, char** str);

int32
ds_connected_v6_field_value_parser(char* str, int32 field_id, tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

int32
ds_connected_v6_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_connected_v6_t *p_connect_v6);

int32
ds_connected_v6_dump_one(ds_connected_v6_t *p_connect_v6, tbl_iter_args_t *pargs);

#endif /* !__DS_CONNECTED_V6_H__ */

