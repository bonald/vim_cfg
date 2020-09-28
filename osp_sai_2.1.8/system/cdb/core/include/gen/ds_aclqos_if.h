
#ifndef __DS_ACLQOS_IF_H__
#define __DS_ACLQOS_IF_H__

int32
ds_aclqos_if_add_aclqos_if_sync(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, uint32 sync);

int32
ds_aclqos_if_del_aclqos_if_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_aclqos_if_set_aclqos_if_field_sync(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, ds_aclqos_if_field_id_t field_id, uint32 sync);

int32
ds_aclqos_if_add_aclqos_if(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if);

int32
ds_aclqos_if_del_aclqos_if(tbl_interface_t *p_if);

int32
ds_aclqos_if_set_aclqos_if_field(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, ds_aclqos_if_field_id_t field_id);

ds_aclqos_if_t*
ds_aclqos_if_get_aclqos_if(tbl_interface_t *p_if);

int32
ds_aclqos_if_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if);

char*
ds_aclqos_if_key_name_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char* str);

char*
ds_aclqos_if_key_value_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char* str);

char*
ds_aclqos_if_field_name_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, int32 field_id, char* str);

char*
ds_aclqos_if_field_value_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, int32 field_id, char* str);

char**
ds_aclqos_if_table_dump(tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if, char** str);

int32
ds_aclqos_if_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if);

int32
ds_aclqos_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_aclqos_if_t *p_aclqos_if);

int32
ds_aclqos_if_dump_one(ds_aclqos_if_t *p_aclqos_if, tbl_iter_args_t *pargs);

int32
ds_aclqos_if_get_aclqos_if_field(tbl_interface_t *p_db_if, ds_aclqos_if_field_id_t field_id, void* field_value);

#endif /* !__DS_ACLQOS_IF_H__ */

