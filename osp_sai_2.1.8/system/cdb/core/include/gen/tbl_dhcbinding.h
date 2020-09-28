
#ifndef __TBL_DHCBINDING_H__
#define __TBL_DHCBINDING_H__

int32
tbl_dhcbinding_add_dhcbinding_sync(tbl_dhcbinding_t *p_dhcbinding, uint32 sync);

int32
tbl_dhcbinding_del_dhcbinding_sync(tbl_dhcbinding_key_t *p_dhcbinding_key, uint32 sync);

int32
tbl_dhcbinding_set_dhcbinding_field_sync(tbl_dhcbinding_t *p_dhcbinding, tbl_dhcbinding_field_id_t field_id, uint32 sync);

int32
tbl_dhcbinding_add_dhcbinding(tbl_dhcbinding_t *p_dhcbinding);

int32
tbl_dhcbinding_del_dhcbinding(tbl_dhcbinding_key_t *p_dhcbinding_key);

int32
tbl_dhcbinding_set_dhcbinding_field(tbl_dhcbinding_t *p_dhcbinding, tbl_dhcbinding_field_id_t field_id);

tbl_dhcbinding_t*
tbl_dhcbinding_get_dhcbinding(tbl_dhcbinding_key_t *p_dhcbinding_key);

char*
tbl_dhcbinding_key_val2str(tbl_dhcbinding_t *p_dhcbinding, char *str, uint32 str_len);

int32
tbl_dhcbinding_key_str2val(char *str, tbl_dhcbinding_t *p_dhcbinding);

int32
tbl_dhcbinding_dump_one(tbl_dhcbinding_t *p_dhcbinding, tbl_iter_args_t *pargs);

char*
tbl_dhcbinding_key_name_dump(tbl_dhcbinding_t *p_dhcbinding, char* str);

char*
tbl_dhcbinding_key_value_dump(tbl_dhcbinding_t *p_dhcbinding, char* str);

char*
tbl_dhcbinding_field_name_dump(tbl_dhcbinding_t *p_dhcbinding, int32 field_id, char* str);

char*
tbl_dhcbinding_field_value_dump(tbl_dhcbinding_t *p_dhcbinding, int32 field_id, char* str);

char**
tbl_dhcbinding_table_dump(tbl_dhcbinding_t *p_dhcbinding, char** str);

int32
tbl_dhcbinding_table_parser(char** array, char* key_value, tbl_dhcbinding_t *p_dhcbinding);

int32
tbl_dhcbinding_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhcbinding_master_t*
tbl_dhcbinding_get_master();

tbl_dhcbinding_master_t*
tbl_dhcbinding_init_dhcbinding();

#endif /* !__TBL_DHCBINDING_H__ */

