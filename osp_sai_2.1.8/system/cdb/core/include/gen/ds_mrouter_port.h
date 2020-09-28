
#ifndef __DS_MROUTER_PORT_H__
#define __DS_MROUTER_PORT_H__

int32
ds_mrouter_port_add_mrouter_port_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, uint32 sync);

int32
ds_mrouter_port_del_mrouter_port_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, uint32 sync);

int32
ds_mrouter_port_set_mrouter_port_field_sync(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, ds_mrouter_port_field_id_t field_id, uint32 sync);

int32
ds_mrouter_port_add_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

int32
ds_mrouter_port_del_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

int32
ds_mrouter_port_set_mrouter_port_field(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, ds_mrouter_port_field_id_t field_id);

ds_mrouter_port_t*
ds_mrouter_port_get_mrouter_port(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

int32
ds_mrouter_port_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

char*
ds_mrouter_port_key_name_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char* str);

char*
ds_mrouter_port_key_value_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char* str);

char*
ds_mrouter_port_field_name_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, int32 field_id, char* str);

char*
ds_mrouter_port_field_value_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, int32 field_id, char* str);

char**
ds_mrouter_port_table_dump(tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port, char** str);

int32
ds_mrouter_port_field_value_parser(char* str, int32 field_id, tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

int32
ds_mrouter_port_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_igsp_intf_t *p_if, ds_mrouter_port_t *p_mr_port);

int32
ds_mrouter_port_dump_one(ds_mrouter_port_t *p_mr_port, tbl_iter_args_t *pargs);

#endif /* !__DS_MROUTER_PORT_H__ */

