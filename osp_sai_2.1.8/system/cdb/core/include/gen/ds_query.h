
#ifndef __DS_QUERY_H__
#define __DS_QUERY_H__

int32
ds_query_add_query_sync(tbl_igsp_intf_t *p_if, ds_query_t *p_query, uint32 sync);

int32
ds_query_del_query_sync(tbl_igsp_intf_t *p_if, ds_query_t *p_query, uint32 sync);

int32
ds_query_set_query_field_sync(tbl_igsp_intf_t *p_if, ds_query_t *p_query, ds_query_field_id_t field_id, uint32 sync);

int32
ds_query_add_query(tbl_igsp_intf_t *p_if, ds_query_t *p_query);

int32
ds_query_del_query(tbl_igsp_intf_t *p_if, ds_query_t *p_query);

int32
ds_query_set_query_field(tbl_igsp_intf_t *p_if, ds_query_t *p_query, ds_query_field_id_t field_id);

ds_query_t*
ds_query_get_query(tbl_igsp_intf_t *p_if, ds_query_t *p_query);

int32
ds_query_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_query_t *p_query);

char*
ds_query_key_name_dump(tbl_igsp_intf_t *p_if, ds_query_t *p_query, char* str);

char*
ds_query_key_value_dump(tbl_igsp_intf_t *p_if, ds_query_t *p_query, char* str);

char*
ds_query_field_name_dump(tbl_igsp_intf_t *p_if, ds_query_t *p_query, int32 field_id, char* str);

char*
ds_query_field_value_dump(tbl_igsp_intf_t *p_if, ds_query_t *p_query, int32 field_id, char* str);

char**
ds_query_table_dump(tbl_igsp_intf_t *p_if, ds_query_t *p_query, char** str);

int32
ds_query_field_value_parser(char* str, int32 field_id, tbl_igsp_intf_t *p_if, ds_query_t *p_query);

int32
ds_query_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_query_t *p_query);

int32
ds_query_dump_one(ds_query_t *p_query, tbl_iter_args_t *pargs);

#endif /* !__DS_QUERY_H__ */

