
#ifndef __TBL_HASH_VALUE_PROFILE_H__
#define __TBL_HASH_VALUE_PROFILE_H__

int32
tbl_hash_value_profile_add_hash_value_profile_sync(tbl_hash_value_profile_t *p_hash_value_profile, uint32 sync);

int32
tbl_hash_value_profile_del_hash_value_profile_sync(tbl_hash_value_profile_key_t *p_hash_value_profile_key, uint32 sync);

int32
tbl_hash_value_profile_set_hash_value_profile_field_sync(tbl_hash_value_profile_t *p_hash_value_profile, tbl_hash_value_profile_field_id_t field_id, uint32 sync);

int32
tbl_hash_value_profile_add_hash_value_profile(tbl_hash_value_profile_t *p_hash_value_profile);

int32
tbl_hash_value_profile_del_hash_value_profile(tbl_hash_value_profile_key_t *p_hash_value_profile_key);

int32
tbl_hash_value_profile_set_hash_value_profile_field(tbl_hash_value_profile_t *p_hash_value_profile, tbl_hash_value_profile_field_id_t field_id);

tbl_hash_value_profile_t*
tbl_hash_value_profile_get_hash_value_profile(tbl_hash_value_profile_key_t *p_hash_value_profile_key);

char*
tbl_hash_value_profile_key_val2str(tbl_hash_value_profile_t *p_hash_value_profile, char *str, uint32 str_len);

int32
tbl_hash_value_profile_key_str2val(char *str, tbl_hash_value_profile_t *p_hash_value_profile);

int32
tbl_hash_value_profile_dump_one(tbl_hash_value_profile_t *p_hash_value_profile, tbl_iter_args_t *pargs);

char*
tbl_hash_value_profile_key_name_dump(tbl_hash_value_profile_t *p_hash_value_profile, char* str);

char*
tbl_hash_value_profile_key_value_dump(tbl_hash_value_profile_t *p_hash_value_profile, char* str);

char*
tbl_hash_value_profile_field_name_dump(tbl_hash_value_profile_t *p_hash_value_profile, int32 field_id, char* str);

char*
tbl_hash_value_profile_field_value_dump(tbl_hash_value_profile_t *p_hash_value_profile, int32 field_id, char* str);

char**
tbl_hash_value_profile_table_dump(tbl_hash_value_profile_t *p_hash_value_profile, char** str);

int32
tbl_hash_value_profile_table_parser(char** array, char* key_value, tbl_hash_value_profile_t *p_hash_value_profile);

int32
tbl_hash_value_profile_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_hash_value_profile_master_t*
tbl_hash_value_profile_get_master();

tbl_hash_value_profile_master_t*
tbl_hash_value_profile_init_hash_value_profile();

#endif /* !__TBL_HASH_VALUE_PROFILE_H__ */

