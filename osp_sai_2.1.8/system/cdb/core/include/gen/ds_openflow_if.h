
#ifndef __DS_OPENFLOW_IF_H__
#define __DS_OPENFLOW_IF_H__

int32
ds_openflow_if_add_openflow_if_sync(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, uint32 sync);

int32
ds_openflow_if_del_openflow_if_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_openflow_if_set_openflow_if_field_sync(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, ds_openflow_if_field_id_t field_id, uint32 sync);

int32
ds_openflow_if_add_openflow_if(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow);

int32
ds_openflow_if_del_openflow_if(tbl_interface_t *p_if);

int32
ds_openflow_if_set_openflow_if_field(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, ds_openflow_if_field_id_t field_id);

ds_openflow_if_t*
ds_openflow_if_get_openflow_if(tbl_interface_t *p_if);

int32
ds_openflow_if_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_openflow_if_t *p_openflow);

char*
ds_openflow_if_key_name_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char* str);

char*
ds_openflow_if_key_value_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char* str);

char*
ds_openflow_if_field_name_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, int32 field_id, char* str);

char*
ds_openflow_if_field_value_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, int32 field_id, char* str);

char**
ds_openflow_if_table_dump(tbl_interface_t *p_if, ds_openflow_if_t *p_openflow, char** str);

int32
ds_openflow_if_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_openflow_if_t *p_openflow);

int32
ds_openflow_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_openflow_if_t *p_openflow);

int32
ds_openflow_if_dump_one(ds_openflow_if_t *p_openflow, tbl_iter_args_t *pargs);

#endif /* !__DS_OPENFLOW_IF_H__ */

