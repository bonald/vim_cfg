
#ifndef __STBL_ACL_UDF_ENTRY_H__
#define __STBL_ACL_UDF_ENTRY_H__

int32
stbl_acl_udf_entry_add_acl_udf_entry_sync(stbl_acl_udf_entry_t *p_acl_udf_entry, uint32 sync);

int32
stbl_acl_udf_entry_del_acl_udf_entry_sync(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key, uint32 sync);

int32
stbl_acl_udf_entry_set_acl_udf_entry_field_sync(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_acl_udf_entry_field_id_t field_id, uint32 sync);

int32
stbl_acl_udf_entry_add_acl_udf_entry(stbl_acl_udf_entry_t *p_acl_udf_entry);

int32
stbl_acl_udf_entry_del_acl_udf_entry(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key);

int32
stbl_acl_udf_entry_set_acl_udf_entry_field(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_acl_udf_entry_field_id_t field_id);

stbl_acl_udf_entry_t*
stbl_acl_udf_entry_get_acl_udf_entry(stbl_acl_udf_entry_key_t *p_acl_udf_entry_key);

char*
stbl_acl_udf_entry_key_val2str(stbl_acl_udf_entry_t *p_acl_udf_entry, char *str, uint32 str_len);

int32
stbl_acl_udf_entry_key_str2val(char *str, stbl_acl_udf_entry_t *p_acl_udf_entry);

int32
stbl_acl_udf_entry_dump_one(stbl_acl_udf_entry_t *p_acl_udf_entry, stbl_iter_args_t *pargs);

int32
stbl_acl_udf_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_udf_entry_master_t*
stbl_acl_udf_entry_get_master();

stbl_acl_udf_entry_master_t*
stbl_acl_udf_entry_init_acl_udf_entry();

#endif /* !__STBL_ACL_UDF_ENTRY_H__ */

