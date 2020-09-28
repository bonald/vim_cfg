
#ifndef __DS_L2PROTOCOL_H__
#define __DS_L2PROTOCOL_H__

int32
ds_l2protocol_add_l2protocol_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, uint32 sync);

int32
ds_l2protocol_del_l2protocol_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, uint32 sync);

int32
ds_l2protocol_set_l2protocol_field_sync(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, ds_l2protocol_field_id_t field_id, uint32 sync);

int32
ds_l2protocol_add_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

int32
ds_l2protocol_del_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

int32
ds_l2protocol_set_l2protocol_field(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, ds_l2protocol_field_id_t field_id);

ds_l2protocol_t*
ds_l2protocol_get_l2protocol(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

int32
ds_l2protocol_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

char*
ds_l2protocol_key_name_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char* str);

char*
ds_l2protocol_key_value_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char* str);

char*
ds_l2protocol_field_name_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, int32 field_id, char* str);

char*
ds_l2protocol_field_value_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, int32 field_id, char* str);

char**
ds_l2protocol_table_dump(tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol, char** str);

int32
ds_l2protocol_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

int32
ds_l2protocol_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_l2protocol_t *p_l2protocol);

int32
ds_l2protocol_dump_one(ds_l2protocol_t *p_l2protocol, tbl_iter_args_t *pargs);

#endif /* !__DS_L2PROTOCOL_H__ */

