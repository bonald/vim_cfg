
#ifndef __DS_OSPF_AUTH_H__
#define __DS_OSPF_AUTH_H__

int32
ds_ospf_auth_add_ospf_auth_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, uint32 sync);

int32
ds_ospf_auth_del_ospf_auth_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, uint32 sync);

int32
ds_ospf_auth_set_ospf_auth_field_sync(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, ds_ospf_auth_field_id_t field_id, uint32 sync);

int32
ds_ospf_auth_add_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

int32
ds_ospf_auth_del_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

int32
ds_ospf_auth_set_ospf_auth_field(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, ds_ospf_auth_field_id_t field_id);

ds_ospf_auth_t*
ds_ospf_auth_get_ospf_auth(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

int32
ds_ospf_auth_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

char*
ds_ospf_auth_key_name_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char* str);

char*
ds_ospf_auth_key_value_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char* str);

char*
ds_ospf_auth_field_name_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, int32 field_id, char* str);

char*
ds_ospf_auth_field_value_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, int32 field_id, char* str);

char**
ds_ospf_auth_table_dump(tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth, char** str);

int32
ds_ospf_auth_field_value_parser(char* str, int32 field_id, tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

int32
ds_ospf_auth_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_route_if_t *p_rtif, ds_ospf_auth_t *p_ospf_auth);

int32
ds_ospf_auth_dump_one(ds_ospf_auth_t *p_ospf_auth, tbl_iter_args_t *pargs);

#endif /* !__DS_OSPF_AUTH_H__ */

