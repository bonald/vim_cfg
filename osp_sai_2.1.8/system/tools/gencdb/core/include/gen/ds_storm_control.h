
#ifndef __DS_STORM_CONTROL_H__
#define __DS_STORM_CONTROL_H__

int32
ds_storm_control_add_storm_control_sync(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, uint32 sync);

int32
ds_storm_control_del_storm_control_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_storm_control_set_storm_control_field_sync(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, ds_storm_control_field_id_t field_id, uint32 sync);

int32
ds_storm_control_add_storm_control(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control);

int32
ds_storm_control_del_storm_control(tbl_interface_t *p_if);

int32
ds_storm_control_set_storm_control_field(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, ds_storm_control_field_id_t field_id);

ds_storm_control_t*
ds_storm_control_get_storm_control(tbl_interface_t *p_if);

int32
ds_storm_control_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_storm_control_t *p_storm_control);

char*
ds_storm_control_key_name_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char* str);

char*
ds_storm_control_key_value_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char* str);

char*
ds_storm_control_field_name_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, int32 field_id, char* str);

char*
ds_storm_control_field_value_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, int32 field_id, char* str);

char**
ds_storm_control_table_dump(tbl_interface_t *p_if, ds_storm_control_t *p_storm_control, char** str);

int32
ds_storm_control_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_storm_control_t *p_storm_control);

int32
ds_storm_control_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_storm_control_t *p_storm_control);

int32
ds_storm_control_dump_one(ds_storm_control_t *p_storm_control, tbl_iter_args_t *pargs);

#endif /* !__DS_STORM_CONTROL_H__ */

