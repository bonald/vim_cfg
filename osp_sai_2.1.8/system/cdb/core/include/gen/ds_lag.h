
#ifndef __DS_LAG_H__
#define __DS_LAG_H__

int32
ds_lag_add_lag_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 sync);

int32
ds_lag_del_lag_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_lag_set_lag_field_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, ds_lag_field_id_t field_id, uint32 sync);

int32
ds_lag_add_lag(tbl_interface_t *p_if, ds_lag_t *p_lag);

int32
ds_lag_del_lag(tbl_interface_t *p_if);

int32
ds_lag_set_lag_field(tbl_interface_t *p_if, ds_lag_t *p_lag, ds_lag_field_id_t field_id);

ds_lag_t*
ds_lag_get_lag(tbl_interface_t *p_if);

int32
ds_lag_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_lag_t *p_lag);

char*
ds_lag_key_name_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char* str);

char*
ds_lag_key_value_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char* str);

char*
ds_lag_field_name_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, int32 field_id, char* str);

char*
ds_lag_field_value_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, int32 field_id, char* str);

char**
ds_lag_table_dump(tbl_interface_t *p_if, ds_lag_t *p_lag, char** str);

int32
ds_lag_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_lag_t *p_lag);

int32
ds_lag_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_lag_t *p_lag);

int32
ds_lag_dump_one(ds_lag_t *p_lag, tbl_iter_args_t *pargs);

int32
ds_lag_set_member_ports_sync(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 add, void *p_object, uint32 sync);

int32
ds_lag_set_member_ports(tbl_interface_t *p_if, ds_lag_t *p_lag, uint32 add, void *p_object);

#endif /* !__DS_LAG_H__ */

