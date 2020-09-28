
#ifndef __DS_ND_PREFIX_H__
#define __DS_ND_PREFIX_H__

int32
ds_nd_prefix_add_nd_prefix_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, uint32 sync);

int32
ds_nd_prefix_del_nd_prefix_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, uint32 sync);

int32
ds_nd_prefix_set_nd_prefix_field_sync(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, ds_nd_prefix_field_id_t field_id, uint32 sync);

int32
ds_nd_prefix_add_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

int32
ds_nd_prefix_del_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

int32
ds_nd_prefix_set_nd_prefix_field(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, ds_nd_prefix_field_id_t field_id);

ds_nd_prefix_t*
ds_nd_prefix_get_nd_prefix(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

int32
ds_nd_prefix_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

char*
ds_nd_prefix_key_name_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char* str);

char*
ds_nd_prefix_key_value_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char* str);

char*
ds_nd_prefix_field_name_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, int32 field_id, char* str);

char*
ds_nd_prefix_field_value_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, int32 field_id, char* str);

char**
ds_nd_prefix_table_dump(tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix, char** str);

int32
ds_nd_prefix_field_value_parser(char* str, int32 field_id, tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

int32
ds_nd_prefix_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_nd_prefix_t *p_nd_prefix);

int32
ds_nd_prefix_dump_one(ds_nd_prefix_t *p_nd_prefix, tbl_iter_args_t *pargs);

#endif /* !__DS_ND_PREFIX_H__ */

