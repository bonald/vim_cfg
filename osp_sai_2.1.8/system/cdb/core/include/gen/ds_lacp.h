
#ifndef __DS_LACP_H__
#define __DS_LACP_H__

int32
ds_lacp_add_lacp_sync(tbl_interface_t *p_if, ds_lacp_t *p_lacp, uint32 sync);

int32
ds_lacp_del_lacp_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_lacp_set_lacp_field_sync(tbl_interface_t *p_if, ds_lacp_t *p_lacp, ds_lacp_field_id_t field_id, uint32 sync);

int32
ds_lacp_add_lacp(tbl_interface_t *p_if, ds_lacp_t *p_lacp);

int32
ds_lacp_del_lacp(tbl_interface_t *p_if);

int32
ds_lacp_set_lacp_field(tbl_interface_t *p_if, ds_lacp_t *p_lacp, ds_lacp_field_id_t field_id);

ds_lacp_t*
ds_lacp_get_lacp(tbl_interface_t *p_if);

int32
ds_lacp_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_lacp_t *p_lacp);

char*
ds_lacp_key_name_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char* str);

char*
ds_lacp_key_value_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char* str);

char*
ds_lacp_field_name_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, int32 field_id, char* str);

char*
ds_lacp_field_value_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, int32 field_id, char* str);

char**
ds_lacp_table_dump(tbl_interface_t *p_if, ds_lacp_t *p_lacp, char** str);

int32
ds_lacp_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_lacp_t *p_lacp);

int32
ds_lacp_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_lacp_t *p_lacp);

int32
ds_lacp_dump_one(ds_lacp_t *p_lacp, tbl_iter_args_t *pargs);

#endif /* !__DS_LACP_H__ */

