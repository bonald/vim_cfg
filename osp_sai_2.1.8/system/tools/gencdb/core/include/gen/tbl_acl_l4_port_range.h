
#ifndef __TBL_ACL_L4_PORT_RANGE_H__
#define __TBL_ACL_L4_PORT_RANGE_H__

int32
tbl_acl_l4_port_range_add_acl_l4_port_range_sync(tbl_acl_l4_port_range_t *p_acl_l4_port_range, uint32 sync);

int32
tbl_acl_l4_port_range_del_acl_l4_port_range_sync(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key, uint32 sync);

int32
tbl_acl_l4_port_range_set_acl_l4_port_range_field_sync(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_acl_l4_port_range_field_id_t field_id, uint32 sync);

int32
tbl_acl_l4_port_range_add_acl_l4_port_range(tbl_acl_l4_port_range_t *p_acl_l4_port_range);

int32
tbl_acl_l4_port_range_del_acl_l4_port_range(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key);

int32
tbl_acl_l4_port_range_set_acl_l4_port_range_field(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_acl_l4_port_range_field_id_t field_id);

tbl_acl_l4_port_range_t*
tbl_acl_l4_port_range_get_acl_l4_port_range(tbl_acl_l4_port_range_key_t *p_acl_l4_port_range_key);

char*
tbl_acl_l4_port_range_key_val2str(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char *str, uint32 str_len);

int32
tbl_acl_l4_port_range_key_str2val(char *str, tbl_acl_l4_port_range_t *p_acl_l4_port_range);

int32
tbl_acl_l4_port_range_dump_one(tbl_acl_l4_port_range_t *p_acl_l4_port_range, tbl_iter_args_t *pargs);

char*
tbl_acl_l4_port_range_key_name_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char* str);

char*
tbl_acl_l4_port_range_key_value_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char* str);

char*
tbl_acl_l4_port_range_field_name_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, int32 field_id, char* str);

char*
tbl_acl_l4_port_range_field_value_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, int32 field_id, char* str);

char**
tbl_acl_l4_port_range_table_dump(tbl_acl_l4_port_range_t *p_acl_l4_port_range, char** str);

int32
tbl_acl_l4_port_range_table_parser(char** array, char* key_value, tbl_acl_l4_port_range_t *p_acl_l4_port_range);

int32
tbl_acl_l4_port_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_acl_l4_port_range_master_t*
tbl_acl_l4_port_range_get_master();

tbl_acl_l4_port_range_master_t*
tbl_acl_l4_port_range_init_acl_l4_port_range();

#endif /* !__TBL_ACL_L4_PORT_RANGE_H__ */

