
#ifndef __DS_JOIN_GROUP_H__
#define __DS_JOIN_GROUP_H__

int32
ds_join_group_add_join_group_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, uint32 sync);

int32
ds_join_group_del_join_group_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, uint32 sync);

int32
ds_join_group_set_join_group_field_sync(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, ds_join_group_field_id_t field_id, uint32 sync);

int32
ds_join_group_add_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

int32
ds_join_group_del_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

int32
ds_join_group_set_join_group_field(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, ds_join_group_field_id_t field_id);

ds_join_group_t*
ds_join_group_get_join_group(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

int32
ds_join_group_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

char*
ds_join_group_key_name_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char* str);

char*
ds_join_group_key_value_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char* str);

char*
ds_join_group_field_name_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, int32 field_id, char* str);

char*
ds_join_group_field_value_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, int32 field_id, char* str);

char**
ds_join_group_table_dump(tbl_igsp_intf_t *p_if, ds_join_group_t *p_join, char** str);

int32
ds_join_group_field_value_parser(char* str, int32 field_id, tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

int32
ds_join_group_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_join_group_t *p_join);

int32
ds_join_group_dump_one(ds_join_group_t *p_join, tbl_iter_args_t *pargs);

#endif /* !__DS_JOIN_GROUP_H__ */

