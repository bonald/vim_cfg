
#ifndef __TBL_NEXTHOP_H__
#define __TBL_NEXTHOP_H__

int32
tbl_nexthop_add_nexthop_sync(tbl_nexthop_t *p_nh, uint32 sync);

int32
tbl_nexthop_del_nexthop_sync(tbl_nexthop_key_t *p_nh_key, uint32 sync);

int32
tbl_nexthop_set_nexthop_field_sync(tbl_nexthop_t *p_nh, tbl_nexthop_field_id_t field_id, uint32 sync);

int32
tbl_nexthop_add_nexthop(tbl_nexthop_t *p_nh);

int32
tbl_nexthop_del_nexthop(tbl_nexthop_key_t *p_nh_key);

int32
tbl_nexthop_set_nexthop_field(tbl_nexthop_t *p_nh, tbl_nexthop_field_id_t field_id);

tbl_nexthop_t*
tbl_nexthop_get_nexthop(tbl_nexthop_key_t *p_nh_key);

char*
tbl_nexthop_key_val2str(tbl_nexthop_t *p_nh, char *str, uint32 str_len);

int32
tbl_nexthop_key_str2val(char *str, tbl_nexthop_t *p_nh);

int32
tbl_nexthop_dump_one(tbl_nexthop_t *p_nh, tbl_iter_args_t *pargs);

char*
tbl_nexthop_key_name_dump(tbl_nexthop_t *p_nh, char* str);

char*
tbl_nexthop_key_value_dump(tbl_nexthop_t *p_nh, char* str);

char*
tbl_nexthop_field_name_dump(tbl_nexthop_t *p_nh, int32 field_id, char* str);

char*
tbl_nexthop_field_value_dump(tbl_nexthop_t *p_nh, int32 field_id, char* str);

char**
tbl_nexthop_table_dump(tbl_nexthop_t *p_nh, char** str);

int32
tbl_nexthop_table_parser(char** array, char* key_value, tbl_nexthop_t *p_nh);

int32
tbl_nexthop_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nexthop_master_t*
tbl_nexthop_get_master();

tbl_nexthop_master_t*
tbl_nexthop_init_nexthop();

#endif /* !__TBL_NEXTHOP_H__ */

