
#ifndef __TBL_APS_GROUP_INFO_H__
#define __TBL_APS_GROUP_INFO_H__

int32
tbl_aps_group_info_add_aps_group_info_sync(tbl_aps_group_info_t *p_aps_group_info, uint32 sync);

int32
tbl_aps_group_info_del_aps_group_info_sync(tbl_aps_group_info_key_t *p_aps_group_info_key, uint32 sync);

int32
tbl_aps_group_info_set_aps_group_info_field_sync(tbl_aps_group_info_t *p_aps_group_info, tbl_aps_group_info_field_id_t field_id, uint32 sync);

int32
tbl_aps_group_info_add_aps_group_info(tbl_aps_group_info_t *p_aps_group_info);

int32
tbl_aps_group_info_del_aps_group_info(tbl_aps_group_info_key_t *p_aps_group_info_key);

int32
tbl_aps_group_info_set_aps_group_info_field(tbl_aps_group_info_t *p_aps_group_info, tbl_aps_group_info_field_id_t field_id);

tbl_aps_group_info_t*
tbl_aps_group_info_get_aps_group_info(tbl_aps_group_info_key_t *p_aps_group_info_key);

char*
tbl_aps_group_info_key_val2str(tbl_aps_group_info_t *p_aps_group_info, char *str, uint32 str_len);

int32
tbl_aps_group_info_key_str2val(char *str, tbl_aps_group_info_t *p_aps_group_info);

int32
tbl_aps_group_info_dump_one(tbl_aps_group_info_t *p_aps_group_info, tbl_iter_args_t *pargs);

char*
tbl_aps_group_info_key_name_dump(tbl_aps_group_info_t *p_aps_group_info, char* str);

char*
tbl_aps_group_info_key_value_dump(tbl_aps_group_info_t *p_aps_group_info, char* str);

char*
tbl_aps_group_info_field_name_dump(tbl_aps_group_info_t *p_aps_group_info, int32 field_id, char* str);

char*
tbl_aps_group_info_field_value_dump(tbl_aps_group_info_t *p_aps_group_info, int32 field_id, char* str);

char**
tbl_aps_group_info_table_dump(tbl_aps_group_info_t *p_aps_group_info, char** str);

int32
tbl_aps_group_info_table_parser(char** array, char* key_value, tbl_aps_group_info_t *p_aps_group_info);

int32
tbl_aps_group_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_aps_group_info_master_t*
tbl_aps_group_info_get_master();

tbl_aps_group_info_master_t*
tbl_aps_group_info_init_aps_group_info();

#endif /* !__TBL_APS_GROUP_INFO_H__ */

