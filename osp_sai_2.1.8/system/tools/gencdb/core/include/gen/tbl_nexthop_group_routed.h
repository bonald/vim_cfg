
#ifndef __TBL_NEXTHOP_GROUP_ROUTED_H__
#define __TBL_NEXTHOP_GROUP_ROUTED_H__

int32
tbl_nexthop_group_routed_add_nexthop_group_routed_sync(tbl_nexthop_group_routed_t *p_nhg_routed, uint32 sync);

int32
tbl_nexthop_group_routed_del_nexthop_group_routed_sync(tbl_nexthop_group_routed_key_t *p_nhg_routed_key, uint32 sync);

int32
tbl_nexthop_group_routed_set_nexthop_group_routed_field_sync(tbl_nexthop_group_routed_t *p_nhg_routed, tbl_nexthop_group_routed_field_id_t field_id, uint32 sync);

int32
tbl_nexthop_group_routed_add_nexthop_group_routed(tbl_nexthop_group_routed_t *p_nhg_routed);

int32
tbl_nexthop_group_routed_del_nexthop_group_routed(tbl_nexthop_group_routed_key_t *p_nhg_routed_key);

int32
tbl_nexthop_group_routed_set_nexthop_group_routed_field(tbl_nexthop_group_routed_t *p_nhg_routed, tbl_nexthop_group_routed_field_id_t field_id);

tbl_nexthop_group_routed_t*
tbl_nexthop_group_routed_get_nexthop_group_routed(tbl_nexthop_group_routed_key_t *p_nhg_routed_key);

char*
tbl_nexthop_group_routed_key_val2str(tbl_nexthop_group_routed_t *p_nhg_routed, char *str, uint32 str_len);

int32
tbl_nexthop_group_routed_key_str2val(char *str, tbl_nexthop_group_routed_t *p_nhg_routed);

int32
tbl_nexthop_group_routed_dump_one(tbl_nexthop_group_routed_t *p_nhg_routed, tbl_iter_args_t *pargs);

char*
tbl_nexthop_group_routed_key_name_dump(tbl_nexthop_group_routed_t *p_nhg_routed, char* str);

char*
tbl_nexthop_group_routed_key_value_dump(tbl_nexthop_group_routed_t *p_nhg_routed, char* str);

char*
tbl_nexthop_group_routed_field_name_dump(tbl_nexthop_group_routed_t *p_nhg_routed, int32 field_id, char* str);

char*
tbl_nexthop_group_routed_field_value_dump(tbl_nexthop_group_routed_t *p_nhg_routed, int32 field_id, char* str);

char**
tbl_nexthop_group_routed_table_dump(tbl_nexthop_group_routed_t *p_nhg_routed, char** str);

int32
tbl_nexthop_group_routed_table_parser(char** array, char* key_value, tbl_nexthop_group_routed_t *p_nhg_routed);

int32
tbl_nexthop_group_routed_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nexthop_group_routed_master_t*
tbl_nexthop_group_routed_get_master();

tbl_nexthop_group_routed_master_t*
tbl_nexthop_group_routed_init_nexthop_group_routed();

#endif /* !__TBL_NEXTHOP_GROUP_ROUTED_H__ */

