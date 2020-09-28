
#ifndef __TBL_HASH_VALUE_GLOBAL_H__
#define __TBL_HASH_VALUE_GLOBAL_H__

int32
tbl_hash_value_global_set_hash_value_global_field_sync(tbl_hash_value_global_t *p_hash_value_global, tbl_hash_value_global_field_id_t field_id, uint32 sync);

int32
tbl_hash_value_global_set_hash_value_global_field(tbl_hash_value_global_t *p_hash_value_global, tbl_hash_value_global_field_id_t field_id);

tbl_hash_value_global_t*
tbl_hash_value_global_get_hash_value_global();

char*
tbl_hash_value_global_key_val2str(tbl_hash_value_global_t *p_hash_value_global, char *str, uint32 str_len);

int32
tbl_hash_value_global_key_str2val(char *str, tbl_hash_value_global_t *p_hash_value_global);

int32
tbl_hash_value_global_dump_one(tbl_hash_value_global_t *p_hash_value_global, tbl_iter_args_t *pargs);

char*
tbl_hash_value_global_key_name_dump(tbl_hash_value_global_t *p_hash_value_global, char* str);

char*
tbl_hash_value_global_key_value_dump(tbl_hash_value_global_t *p_hash_value_global, char* str);

char*
tbl_hash_value_global_field_name_dump(tbl_hash_value_global_t *p_hash_value_global, int32 field_id, char* str);

char*
tbl_hash_value_global_field_value_dump(tbl_hash_value_global_t *p_hash_value_global, int32 field_id, char* str);

char**
tbl_hash_value_global_table_dump(tbl_hash_value_global_t *p_hash_value_global, char** str);

int32
tbl_hash_value_global_table_parser(char** array, char* key_value, tbl_hash_value_global_t *p_hash_value_global);

int32
tbl_hash_value_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_hash_value_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_hash_value_global_t*
tbl_hash_value_global_init_hash_value_global();

#endif /* !__TBL_HASH_VALUE_GLOBAL_H__ */

