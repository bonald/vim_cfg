
#ifndef __DS_DHCLIENT_IF_H__
#define __DS_DHCLIENT_IF_H__

int32
ds_dhclient_if_add_dhclient_if_sync(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, uint32 sync);

int32
ds_dhclient_if_del_dhclient_if_sync(tbl_interface_t *p_if, uint32 sync);

int32
ds_dhclient_if_set_dhclient_if_field_sync(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, ds_dhclient_if_field_id_t field_id, uint32 sync);

int32
ds_dhclient_if_add_dhclient_if(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient);

int32
ds_dhclient_if_del_dhclient_if(tbl_interface_t *p_if);

int32
ds_dhclient_if_set_dhclient_if_field(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, ds_dhclient_if_field_id_t field_id);

ds_dhclient_if_t*
ds_dhclient_if_get_dhclient_if(tbl_interface_t *p_if);

int32
ds_dhclient_if_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient);

char*
ds_dhclient_if_key_name_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char* str);

char*
ds_dhclient_if_key_value_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char* str);

char*
ds_dhclient_if_field_name_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, int32 field_id, char* str);

char*
ds_dhclient_if_field_value_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, int32 field_id, char* str);

char**
ds_dhclient_if_table_dump(tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient, char** str);

int32
ds_dhclient_if_field_value_parser(char* str, int32 field_id, tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient);

int32
ds_dhclient_if_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_interface_t *p_if, ds_dhclient_if_t *p_dhclient);

int32
ds_dhclient_if_dump_one(ds_dhclient_if_t *p_dhclient, tbl_iter_args_t *pargs);

#endif /* !__DS_DHCLIENT_IF_H__ */

