
#ifndef __TBL_FEA_ACL_H__
#define __TBL_FEA_ACL_H__

int32
tbl_fea_acl_add_fea_acl_sync(tbl_fea_acl_t *p_fea_acl, uint32 sync);

int32
tbl_fea_acl_del_fea_acl_sync(tbl_fea_acl_key_t *p_fea_acl_key, uint32 sync);

int32
tbl_fea_acl_set_fea_acl_field_sync(tbl_fea_acl_t *p_fea_acl, tbl_fea_acl_field_id_t field_id, uint32 sync);

int32
tbl_fea_acl_add_fea_acl(tbl_fea_acl_t *p_fea_acl);

int32
tbl_fea_acl_del_fea_acl(tbl_fea_acl_key_t *p_fea_acl_key);

int32
tbl_fea_acl_set_fea_acl_field(tbl_fea_acl_t *p_fea_acl, tbl_fea_acl_field_id_t field_id);

tbl_fea_acl_t*
tbl_fea_acl_get_fea_acl(tbl_fea_acl_key_t *p_fea_acl_key);

char*
tbl_fea_acl_key_val2str(tbl_fea_acl_t *p_fea_acl, char *str, uint32 str_len);

int32
tbl_fea_acl_key_str2val(char *str, tbl_fea_acl_t *p_fea_acl);

int32
tbl_fea_acl_dump_one(tbl_fea_acl_t *p_fea_acl, tbl_iter_args_t *pargs);

char*
tbl_fea_acl_key_name_dump(tbl_fea_acl_t *p_fea_acl, char* str);

char*
tbl_fea_acl_key_value_dump(tbl_fea_acl_t *p_fea_acl, char* str);

char*
tbl_fea_acl_field_name_dump(tbl_fea_acl_t *p_fea_acl, int32 field_id, char* str);

char*
tbl_fea_acl_field_value_dump(tbl_fea_acl_t *p_fea_acl, int32 field_id, char* str);

char**
tbl_fea_acl_table_dump(tbl_fea_acl_t *p_fea_acl, char** str);

int32
tbl_fea_acl_table_parser(char** array, char* key_value, tbl_fea_acl_t *p_fea_acl);

int32
tbl_fea_acl_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_acl_master_t*
tbl_fea_acl_get_master();

tbl_fea_acl_master_t*
tbl_fea_acl_init_fea_acl();

#endif /* !__TBL_FEA_ACL_H__ */

