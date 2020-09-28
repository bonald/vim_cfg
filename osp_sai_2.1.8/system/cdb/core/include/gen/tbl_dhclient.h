
#ifndef __TBL_DHCLIENT_H__
#define __TBL_DHCLIENT_H__

int32
tbl_dhclient_set_dhclient_field_sync(tbl_dhclient_t *p_dhclient, tbl_dhclient_field_id_t field_id, uint32 sync);

int32
tbl_dhclient_set_dhclient_field(tbl_dhclient_t *p_dhclient, tbl_dhclient_field_id_t field_id);

tbl_dhclient_t*
tbl_dhclient_get_dhclient();

char*
tbl_dhclient_key_val2str(tbl_dhclient_t *p_dhclient, char *str, uint32 str_len);

int32
tbl_dhclient_key_str2val(char *str, tbl_dhclient_t *p_dhclient);

int32
tbl_dhclient_dump_one(tbl_dhclient_t *p_dhclient, tbl_iter_args_t *pargs);

char*
tbl_dhclient_key_name_dump(tbl_dhclient_t *p_dhclient, char* str);

char*
tbl_dhclient_key_value_dump(tbl_dhclient_t *p_dhclient, char* str);

char*
tbl_dhclient_field_name_dump(tbl_dhclient_t *p_dhclient, int32 field_id, char* str);

char*
tbl_dhclient_field_value_dump(tbl_dhclient_t *p_dhclient, int32 field_id, char* str);

char**
tbl_dhclient_table_dump(tbl_dhclient_t *p_dhclient, char** str);

int32
tbl_dhclient_table_parser(char** array, char* key_value, tbl_dhclient_t *p_dhclient);

int32
tbl_dhclient_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_dhclient_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhclient_t*
tbl_dhclient_init_dhclient();

#endif /* !__TBL_DHCLIENT_H__ */

