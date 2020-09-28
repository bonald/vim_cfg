
#ifndef __TBL_FEA_ACL_TABLE_H__
#define __TBL_FEA_ACL_TABLE_H__

int32
tbl_fea_acl_table_add_fea_acl_table_sync(tbl_fea_acl_table_t *p_fea_acl_table, uint32 sync);

int32
tbl_fea_acl_table_del_fea_acl_table_sync(tbl_fea_acl_table_key_t *p_fea_acl_table_key, uint32 sync);

int32
tbl_fea_acl_table_set_fea_acl_table_field_sync(tbl_fea_acl_table_t *p_fea_acl_table, tbl_fea_acl_table_field_id_t field_id, uint32 sync);

int32
tbl_fea_acl_table_add_fea_acl_table(tbl_fea_acl_table_t *p_fea_acl_table);

int32
tbl_fea_acl_table_del_fea_acl_table(tbl_fea_acl_table_key_t *p_fea_acl_table_key);

int32
tbl_fea_acl_table_set_fea_acl_table_field(tbl_fea_acl_table_t *p_fea_acl_table, tbl_fea_acl_table_field_id_t field_id);

tbl_fea_acl_table_t*
tbl_fea_acl_table_get_fea_acl_table(tbl_fea_acl_table_key_t *p_fea_acl_table_key);

char*
tbl_fea_acl_table_key_val2str(tbl_fea_acl_table_t *p_fea_acl_table, char *str, uint32 str_len);

int32
tbl_fea_acl_table_key_str2val(char *str, tbl_fea_acl_table_t *p_fea_acl_table);

int32
tbl_fea_acl_table_dump_one(tbl_fea_acl_table_t *p_fea_acl_table, tbl_iter_args_t *pargs);

char*
tbl_fea_acl_table_key_name_dump(tbl_fea_acl_table_t *p_fea_acl_table, char* str);

char*
tbl_fea_acl_table_key_value_dump(tbl_fea_acl_table_t *p_fea_acl_table, char* str);

char*
tbl_fea_acl_table_field_name_dump(tbl_fea_acl_table_t *p_fea_acl_table, int32 field_id, char* str);

char*
tbl_fea_acl_table_field_value_dump(tbl_fea_acl_table_t *p_fea_acl_table, int32 field_id, char* str);

char**
tbl_fea_acl_table_table_dump(tbl_fea_acl_table_t *p_fea_acl_table, char** str);

int32
tbl_fea_acl_table_table_parser(char** array, char* key_value, tbl_fea_acl_table_t *p_fea_acl_table);

int32
tbl_fea_acl_table_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_acl_table_master_t*
tbl_fea_acl_table_get_master();

tbl_fea_acl_table_master_t*
tbl_fea_acl_table_init_fea_acl_table();

#endif /* !__TBL_FEA_ACL_TABLE_H__ */

