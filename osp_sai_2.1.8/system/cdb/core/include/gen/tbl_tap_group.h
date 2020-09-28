
#ifndef __TBL_TAP_GROUP_H__
#define __TBL_TAP_GROUP_H__

int32
tbl_tap_group_add_tap_group_sync(tbl_tap_group_t *p_tap_group, uint32 sync);

int32
tbl_tap_group_del_tap_group_sync(tbl_tap_group_key_t *p_tap_group_key, uint32 sync);

int32
tbl_tap_group_set_tap_group_field_sync(tbl_tap_group_t *p_tap_group, tbl_tap_group_field_id_t field_id, uint32 sync);

int32
tbl_tap_group_add_tap_group(tbl_tap_group_t *p_tap_group);

int32
tbl_tap_group_del_tap_group(tbl_tap_group_key_t *p_tap_group_key);

int32
tbl_tap_group_set_tap_group_field(tbl_tap_group_t *p_tap_group, tbl_tap_group_field_id_t field_id);

tbl_tap_group_t*
tbl_tap_group_get_tap_group(tbl_tap_group_key_t *p_tap_group_key);

char*
tbl_tap_group_key_val2str(tbl_tap_group_t *p_tap_group, char *str, uint32 str_len);

int32
tbl_tap_group_key_str2val(char *str, tbl_tap_group_t *p_tap_group);

int32
tbl_tap_group_dump_one(tbl_tap_group_t *p_tap_group, tbl_iter_args_t *pargs);

char*
tbl_tap_group_key_name_dump(tbl_tap_group_t *p_tap_group, char* str);

char*
tbl_tap_group_key_value_dump(tbl_tap_group_t *p_tap_group, char* str);

char*
tbl_tap_group_field_name_dump(tbl_tap_group_t *p_tap_group, int32 field_id, char* str);

char*
tbl_tap_group_field_value_dump(tbl_tap_group_t *p_tap_group, int32 field_id, char* str);

char**
tbl_tap_group_table_dump(tbl_tap_group_t *p_tap_group, char** str);

int32
tbl_tap_group_table_parser(char** array, char* key_value, tbl_tap_group_t *p_tap_group);

int32
tbl_tap_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_group_master_t*
tbl_tap_group_get_master();

tbl_tap_group_master_t*
tbl_tap_group_init_tap_group();

int32
tbl_tap_group_get_truncation_rcs(tbl_tap_group_t *p_tap_grp, tbl_iter_args_t *pargs);


int32
tbl_tap_group_get_truncation_count();


#endif /* !__TBL_TAP_GROUP_H__ */

