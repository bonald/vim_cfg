
#ifndef __TBL_IGSP_GROUP_H__
#define __TBL_IGSP_GROUP_H__

int32
tbl_igsp_group_add_igsp_group_sync(tbl_igsp_group_t *p_group, uint32 sync);

int32
tbl_igsp_group_del_igsp_group_sync(tbl_igsp_group_key_t *p_group_key, uint32 sync);

int32
tbl_igsp_group_set_igsp_group_field_sync(tbl_igsp_group_t *p_group, tbl_igsp_group_field_id_t field_id, uint32 sync);

int32
tbl_igsp_group_add_igsp_group(tbl_igsp_group_t *p_group);

int32
tbl_igsp_group_del_igsp_group(tbl_igsp_group_key_t *p_group_key);

int32
tbl_igsp_group_set_igsp_group_field(tbl_igsp_group_t *p_group, tbl_igsp_group_field_id_t field_id);

tbl_igsp_group_t*
tbl_igsp_group_get_igsp_group(tbl_igsp_group_key_t *p_group_key);

char*
tbl_igsp_group_key_val2str(tbl_igsp_group_t *p_group, char *str, uint32 str_len);

int32
tbl_igsp_group_key_str2val(char *str, tbl_igsp_group_t *p_group);

int32
tbl_igsp_group_dump_one(tbl_igsp_group_t *p_group, tbl_iter_args_t *pargs);

char*
tbl_igsp_group_key_name_dump(tbl_igsp_group_t *p_group, char* str);

char*
tbl_igsp_group_key_value_dump(tbl_igsp_group_t *p_group, char* str);

char*
tbl_igsp_group_field_name_dump(tbl_igsp_group_t *p_group, int32 field_id, char* str);

char*
tbl_igsp_group_field_value_dump(tbl_igsp_group_t *p_group, int32 field_id, char* str);

char**
tbl_igsp_group_table_dump(tbl_igsp_group_t *p_group, char** str);

int32
tbl_igsp_group_table_parser(char** array, char* key_value, tbl_igsp_group_t *p_group);

int32
tbl_igsp_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_igsp_group_master_t*
tbl_igsp_group_get_master();

tbl_igsp_group_master_t*
tbl_igsp_group_init_igsp_group();

#endif /* !__TBL_IGSP_GROUP_H__ */

