
#ifndef __TBL_OSPF_AREA_AUTH_H__
#define __TBL_OSPF_AREA_AUTH_H__

int32
tbl_ospf_area_auth_add_ospf_area_auth_sync(tbl_ospf_area_auth_t *p_ospf_area_auth, uint32 sync);

int32
tbl_ospf_area_auth_del_ospf_area_auth_sync(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key, uint32 sync);

int32
tbl_ospf_area_auth_set_ospf_area_auth_field_sync(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_ospf_area_auth_field_id_t field_id, uint32 sync);

int32
tbl_ospf_area_auth_add_ospf_area_auth(tbl_ospf_area_auth_t *p_ospf_area_auth);

int32
tbl_ospf_area_auth_del_ospf_area_auth(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key);

int32
tbl_ospf_area_auth_set_ospf_area_auth_field(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_ospf_area_auth_field_id_t field_id);

tbl_ospf_area_auth_t*
tbl_ospf_area_auth_get_ospf_area_auth(tbl_ospf_area_auth_key_t *p_ospf_area_auth_key);

char*
tbl_ospf_area_auth_key_val2str(tbl_ospf_area_auth_t *p_ospf_area_auth, char *str, uint32 str_len);

int32
tbl_ospf_area_auth_key_str2val(char *str, tbl_ospf_area_auth_t *p_ospf_area_auth);

int32
tbl_ospf_area_auth_dump_one(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_iter_args_t *pargs);

char*
tbl_ospf_area_auth_key_name_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char* str);

char*
tbl_ospf_area_auth_key_value_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char* str);

char*
tbl_ospf_area_auth_field_name_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, int32 field_id, char* str);

char*
tbl_ospf_area_auth_field_value_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, int32 field_id, char* str);

char**
tbl_ospf_area_auth_table_dump(tbl_ospf_area_auth_t *p_ospf_area_auth, char** str);

int32
tbl_ospf_area_auth_table_parser(char** array, char* key_value, tbl_ospf_area_auth_t *p_ospf_area_auth);

int32
tbl_ospf_area_auth_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_area_auth_master_t*
tbl_ospf_area_auth_get_master();

tbl_ospf_area_auth_master_t*
tbl_ospf_area_auth_init_ospf_area_auth();

#endif /* !__TBL_OSPF_AREA_AUTH_H__ */

