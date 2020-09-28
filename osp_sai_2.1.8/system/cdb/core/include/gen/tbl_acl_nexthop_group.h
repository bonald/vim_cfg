
#ifndef __TBL_ACL_NEXTHOP_GROUP_H__
#define __TBL_ACL_NEXTHOP_GROUP_H__

int32
tbl_acl_nexthop_group_add_acl_nexthop_group_sync(tbl_acl_nexthop_group_t *p_acl_nexthop_group, uint32 sync);

int32
tbl_acl_nexthop_group_del_acl_nexthop_group_sync(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key, uint32 sync);

int32
tbl_acl_nexthop_group_set_acl_nexthop_group_field_sync(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_acl_nexthop_group_field_id_t field_id, uint32 sync);

int32
tbl_acl_nexthop_group_add_acl_nexthop_group(tbl_acl_nexthop_group_t *p_acl_nexthop_group);

int32
tbl_acl_nexthop_group_del_acl_nexthop_group(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key);

int32
tbl_acl_nexthop_group_set_acl_nexthop_group_field(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_acl_nexthop_group_field_id_t field_id);

tbl_acl_nexthop_group_t*
tbl_acl_nexthop_group_get_acl_nexthop_group(tbl_acl_nexthop_group_key_t *p_acl_nexthop_group_key);

char*
tbl_acl_nexthop_group_key_val2str(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char *str, uint32 str_len);

int32
tbl_acl_nexthop_group_key_str2val(char *str, tbl_acl_nexthop_group_t *p_acl_nexthop_group);

int32
tbl_acl_nexthop_group_dump_one(tbl_acl_nexthop_group_t *p_acl_nexthop_group, tbl_iter_args_t *pargs);

char*
tbl_acl_nexthop_group_key_name_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char* str);

char*
tbl_acl_nexthop_group_key_value_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char* str);

char*
tbl_acl_nexthop_group_field_name_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, int32 field_id, char* str);

char*
tbl_acl_nexthop_group_field_value_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, int32 field_id, char* str);

char**
tbl_acl_nexthop_group_table_dump(tbl_acl_nexthop_group_t *p_acl_nexthop_group, char** str);

int32
tbl_acl_nexthop_group_table_parser(char** array, char* key_value, tbl_acl_nexthop_group_t *p_acl_nexthop_group);

int32
tbl_acl_nexthop_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_nexthop_group_master_t*
tbl_acl_nexthop_group_get_master();

tbl_acl_nexthop_group_master_t*
tbl_acl_nexthop_group_init_acl_nexthop_group();

#endif /* !__TBL_ACL_NEXTHOP_GROUP_H__ */

