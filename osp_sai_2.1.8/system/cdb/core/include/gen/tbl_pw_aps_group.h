
#ifndef __TBL_PW_APS_GROUP_H__
#define __TBL_PW_APS_GROUP_H__

int32
tbl_pw_aps_group_add_pw_aps_group_sync(tbl_pw_aps_group_t *p_pw_aps_group, uint32 sync);

int32
tbl_pw_aps_group_del_pw_aps_group_sync(tbl_pw_aps_group_key_t *p_pw_aps_group_key, uint32 sync);

int32
tbl_pw_aps_group_set_pw_aps_group_field_sync(tbl_pw_aps_group_t *p_pw_aps_group, tbl_pw_aps_group_field_id_t field_id, uint32 sync);

int32
tbl_pw_aps_group_add_pw_aps_group(tbl_pw_aps_group_t *p_pw_aps_group);

int32
tbl_pw_aps_group_del_pw_aps_group(tbl_pw_aps_group_key_t *p_pw_aps_group_key);

int32
tbl_pw_aps_group_set_pw_aps_group_field(tbl_pw_aps_group_t *p_pw_aps_group, tbl_pw_aps_group_field_id_t field_id);

tbl_pw_aps_group_t*
tbl_pw_aps_group_get_pw_aps_group(tbl_pw_aps_group_key_t *p_pw_aps_group_key);

char*
tbl_pw_aps_group_key_val2str(tbl_pw_aps_group_t *p_pw_aps_group, char *str, uint32 str_len);

int32
tbl_pw_aps_group_key_str2val(char *str, tbl_pw_aps_group_t *p_pw_aps_group);

int32
tbl_pw_aps_group_dump_one(tbl_pw_aps_group_t *p_pw_aps_group, tbl_iter_args_t *pargs);

char*
tbl_pw_aps_group_key_name_dump(tbl_pw_aps_group_t *p_pw_aps_group, char* str);

char*
tbl_pw_aps_group_key_value_dump(tbl_pw_aps_group_t *p_pw_aps_group, char* str);

char*
tbl_pw_aps_group_field_name_dump(tbl_pw_aps_group_t *p_pw_aps_group, int32 field_id, char* str);

char*
tbl_pw_aps_group_field_value_dump(tbl_pw_aps_group_t *p_pw_aps_group, int32 field_id, char* str);

char**
tbl_pw_aps_group_table_dump(tbl_pw_aps_group_t *p_pw_aps_group, char** str);

int32
tbl_pw_aps_group_table_parser(char** array, char* key_value, tbl_pw_aps_group_t *p_pw_aps_group);

int32
tbl_pw_aps_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pw_aps_group_master_t*
tbl_pw_aps_group_get_master();

tbl_pw_aps_group_master_t*
tbl_pw_aps_group_init_pw_aps_group();

#endif /* !__TBL_PW_APS_GROUP_H__ */
