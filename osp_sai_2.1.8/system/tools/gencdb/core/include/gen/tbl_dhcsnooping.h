
#ifndef __TBL_DHCSNOOPING_H__
#define __TBL_DHCSNOOPING_H__

int32
tbl_dhcsnooping_set_dhcsnooping_field_sync(tbl_dhcsnooping_t *p_dhcsnooping, tbl_dhcsnooping_field_id_t field_id, uint32 sync);

int32
tbl_dhcsnooping_set_dhcsnooping_field(tbl_dhcsnooping_t *p_dhcsnooping, tbl_dhcsnooping_field_id_t field_id);

tbl_dhcsnooping_t*
tbl_dhcsnooping_get_dhcsnooping();

char*
tbl_dhcsnooping_key_val2str(tbl_dhcsnooping_t *p_dhcsnooping, char *str, uint32 str_len);

int32
tbl_dhcsnooping_key_str2val(char *str, tbl_dhcsnooping_t *p_dhcsnooping);

int32
tbl_dhcsnooping_dump_one(tbl_dhcsnooping_t *p_dhcsnooping, tbl_iter_args_t *pargs);

char*
tbl_dhcsnooping_key_name_dump(tbl_dhcsnooping_t *p_dhcsnooping, char* str);

char*
tbl_dhcsnooping_key_value_dump(tbl_dhcsnooping_t *p_dhcsnooping, char* str);

char*
tbl_dhcsnooping_field_name_dump(tbl_dhcsnooping_t *p_dhcsnooping, int32 field_id, char* str);

char*
tbl_dhcsnooping_field_value_dump(tbl_dhcsnooping_t *p_dhcsnooping, int32 field_id, char* str);

char**
tbl_dhcsnooping_table_dump(tbl_dhcsnooping_t *p_dhcsnooping, char** str);

int32
tbl_dhcsnooping_table_parser(char** array, char* key_value, tbl_dhcsnooping_t *p_dhcsnooping);

int32
tbl_dhcsnooping_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_dhcsnooping_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhcsnooping_t*
tbl_dhcsnooping_init_dhcsnooping();

#endif /* !__TBL_DHCSNOOPING_H__ */

