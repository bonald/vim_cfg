
#ifndef __TBL_ACL_WORM_FILTER_H__
#define __TBL_ACL_WORM_FILTER_H__

int32
tbl_acl_worm_filter_add_acl_worm_filter_sync(tbl_acl_worm_filter_t *p_acl_worm_filter, uint32 sync);

int32
tbl_acl_worm_filter_del_acl_worm_filter_sync(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key, uint32 sync);

int32
tbl_acl_worm_filter_set_acl_worm_filter_field_sync(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_acl_worm_filter_field_id_t field_id, uint32 sync);

int32
tbl_acl_worm_filter_add_acl_worm_filter(tbl_acl_worm_filter_t *p_acl_worm_filter);

int32
tbl_acl_worm_filter_del_acl_worm_filter(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key);

int32
tbl_acl_worm_filter_set_acl_worm_filter_field(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_acl_worm_filter_field_id_t field_id);

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_acl_worm_filter(tbl_acl_worm_filter_key_t *p_acl_worm_filter_key);

char*
tbl_acl_worm_filter_key_val2str(tbl_acl_worm_filter_t *p_acl_worm_filter, char *str, uint32 str_len);

int32
tbl_acl_worm_filter_key_str2val(char *str, tbl_acl_worm_filter_t *p_acl_worm_filter);

int32
tbl_acl_worm_filter_dump_one(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs);

char*
tbl_acl_worm_filter_key_name_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char* str);

char*
tbl_acl_worm_filter_key_value_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char* str);

char*
tbl_acl_worm_filter_field_name_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, int32 field_id, char* str);

char*
tbl_acl_worm_filter_field_value_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, int32 field_id, char* str);

char**
tbl_acl_worm_filter_table_dump(tbl_acl_worm_filter_t *p_acl_worm_filter, char** str);

int32
tbl_acl_worm_filter_table_parser(char** array, char* key_value, tbl_acl_worm_filter_t *p_acl_worm_filter);

int32
tbl_acl_worm_filter_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_worm_filter_master_t*
tbl_acl_worm_filter_get_master();

tbl_acl_worm_filter_master_t*
tbl_acl_worm_filter_init_acl_worm_filter();

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_name(char *name);

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_seq(uint32 seq_num);

int32
tbl_acl_worm_filter_get_range_count();

#endif /* !__TBL_ACL_WORM_FILTER_H__ */

