
#ifndef __TBL_TAP_FIVE_TUPLE_H__
#define __TBL_TAP_FIVE_TUPLE_H__

int32
tbl_tap_five_tuple_add_tap_five_tuple_sync(tbl_tap_five_tuple_t *p_tap_five_tuple, uint32 sync);

int32
tbl_tap_five_tuple_del_tap_five_tuple_sync(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key, uint32 sync);

int32
tbl_tap_five_tuple_set_tap_five_tuple_field_sync(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_tap_five_tuple_field_id_t field_id, uint32 sync);

int32
tbl_tap_five_tuple_add_tap_five_tuple(tbl_tap_five_tuple_t *p_tap_five_tuple);

int32
tbl_tap_five_tuple_del_tap_five_tuple(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key);

int32
tbl_tap_five_tuple_set_tap_five_tuple_field(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_tap_five_tuple_field_id_t field_id);

tbl_tap_five_tuple_t*
tbl_tap_five_tuple_get_tap_five_tuple(tbl_tap_five_tuple_key_t *p_tap_five_tuple_key);

char*
tbl_tap_five_tuple_key_val2str(tbl_tap_five_tuple_t *p_tap_five_tuple, char *str, uint32 str_len);

int32
tbl_tap_five_tuple_key_str2val(char *str, tbl_tap_five_tuple_t *p_tap_five_tuple);

int32
tbl_tap_five_tuple_dump_one(tbl_tap_five_tuple_t *p_tap_five_tuple, tbl_iter_args_t *pargs);

char*
tbl_tap_five_tuple_key_name_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char* str);

char*
tbl_tap_five_tuple_key_value_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char* str);

char*
tbl_tap_five_tuple_field_name_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, int32 field_id, char* str);

char*
tbl_tap_five_tuple_field_value_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, int32 field_id, char* str);

char**
tbl_tap_five_tuple_table_dump(tbl_tap_five_tuple_t *p_tap_five_tuple, char** str);

int32
tbl_tap_five_tuple_table_parser(char** array, char* key_value, tbl_tap_five_tuple_t *p_tap_five_tuple);

int32
tbl_tap_five_tuple_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_five_tuple_master_t*
tbl_tap_five_tuple_get_master();

tbl_tap_five_tuple_master_t*
tbl_tap_five_tuple_init_tap_five_tuple();

#endif /* !__TBL_TAP_FIVE_TUPLE_H__ */

