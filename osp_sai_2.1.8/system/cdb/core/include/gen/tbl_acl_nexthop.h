
#ifndef __TBL_ACL_NEXTHOP_H__
#define __TBL_ACL_NEXTHOP_H__

int32
tbl_acl_nexthop_add_acl_nexthop_sync(tbl_acl_nexthop_t *p_acl_nexthop, uint32 sync);

int32
tbl_acl_nexthop_del_acl_nexthop_sync(tbl_acl_nexthop_key_t *p_acl_nexthop_key, uint32 sync);

int32
tbl_acl_nexthop_set_acl_nexthop_field_sync(tbl_acl_nexthop_t *p_acl_nexthop, tbl_acl_nexthop_field_id_t field_id, uint32 sync);

int32
tbl_acl_nexthop_add_acl_nexthop(tbl_acl_nexthop_t *p_acl_nexthop);

int32
tbl_acl_nexthop_del_acl_nexthop(tbl_acl_nexthop_key_t *p_acl_nexthop_key);

int32
tbl_acl_nexthop_set_acl_nexthop_field(tbl_acl_nexthop_t *p_acl_nexthop, tbl_acl_nexthop_field_id_t field_id);

tbl_acl_nexthop_t*
tbl_acl_nexthop_get_acl_nexthop(tbl_acl_nexthop_key_t *p_acl_nexthop_key);

char*
tbl_acl_nexthop_key_val2str(tbl_acl_nexthop_t *p_acl_nexthop, char *str, uint32 str_len);

int32
tbl_acl_nexthop_key_str2val(char *str, tbl_acl_nexthop_t *p_acl_nexthop);

int32
tbl_acl_nexthop_dump_one(tbl_acl_nexthop_t *p_acl_nexthop, tbl_iter_args_t *pargs);

char*
tbl_acl_nexthop_key_name_dump(tbl_acl_nexthop_t *p_acl_nexthop, char* str);

char*
tbl_acl_nexthop_key_value_dump(tbl_acl_nexthop_t *p_acl_nexthop, char* str);

char*
tbl_acl_nexthop_field_name_dump(tbl_acl_nexthop_t *p_acl_nexthop, int32 field_id, char* str);

char*
tbl_acl_nexthop_field_value_dump(tbl_acl_nexthop_t *p_acl_nexthop, int32 field_id, char* str);

char**
tbl_acl_nexthop_table_dump(tbl_acl_nexthop_t *p_acl_nexthop, char** str);

int32
tbl_acl_nexthop_table_parser(char** array, char* key_value, tbl_acl_nexthop_t *p_acl_nexthop);

int32
tbl_acl_nexthop_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_nexthop_master_t*
tbl_acl_nexthop_get_master();

tbl_acl_nexthop_master_t*
tbl_acl_nexthop_init_acl_nexthop();

#endif /* !__TBL_ACL_NEXTHOP_H__ */
