
#ifndef __TBL_DHCRELAY_H__
#define __TBL_DHCRELAY_H__

int32
tbl_dhcrelay_set_dhcrelay_field_sync(tbl_dhcrelay_t *p_dhcrelay, tbl_dhcrelay_field_id_t field_id, uint32 sync);

int32
tbl_dhcrelay_set_dhcrelay_field(tbl_dhcrelay_t *p_dhcrelay, tbl_dhcrelay_field_id_t field_id);

tbl_dhcrelay_t*
tbl_dhcrelay_get_dhcrelay();

char*
tbl_dhcrelay_key_val2str(tbl_dhcrelay_t *p_dhcrelay, char *str, uint32 str_len);

int32
tbl_dhcrelay_key_str2val(char *str, tbl_dhcrelay_t *p_dhcrelay);

int32
tbl_dhcrelay_dump_one(tbl_dhcrelay_t *p_dhcrelay, tbl_iter_args_t *pargs);

char*
tbl_dhcrelay_key_name_dump(tbl_dhcrelay_t *p_dhcrelay, char* str);

char*
tbl_dhcrelay_key_value_dump(tbl_dhcrelay_t *p_dhcrelay, char* str);

char*
tbl_dhcrelay_field_name_dump(tbl_dhcrelay_t *p_dhcrelay, int32 field_id, char* str);

char*
tbl_dhcrelay_field_value_dump(tbl_dhcrelay_t *p_dhcrelay, int32 field_id, char* str);

char**
tbl_dhcrelay_table_dump(tbl_dhcrelay_t *p_dhcrelay, char** str);

int32
tbl_dhcrelay_table_parser(char** array, char* key_value, tbl_dhcrelay_t *p_dhcrelay);

int32
tbl_dhcrelay_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_dhcrelay_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhcrelay_t*
tbl_dhcrelay_init_dhcrelay();

#endif /* !__TBL_DHCRELAY_H__ */

