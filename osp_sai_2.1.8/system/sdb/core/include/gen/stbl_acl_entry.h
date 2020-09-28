
#ifndef __STBL_ACL_ENTRY_H__
#define __STBL_ACL_ENTRY_H__

int32
stbl_acl_entry_add_acl_entry_sync(stbl_acl_entry_t *p_ace, uint32 sync);

int32
stbl_acl_entry_del_acl_entry_sync(stbl_acl_entry_key_t *p_ace_key, uint32 sync);

int32
stbl_acl_entry_set_acl_entry_field_sync(stbl_acl_entry_t *p_ace, stbl_acl_entry_field_id_t field_id, uint32 sync);

int32
stbl_acl_entry_add_acl_entry(stbl_acl_entry_t *p_ace);

int32
stbl_acl_entry_del_acl_entry(stbl_acl_entry_key_t *p_ace_key);

int32
stbl_acl_entry_set_acl_entry_field(stbl_acl_entry_t *p_ace, stbl_acl_entry_field_id_t field_id);

stbl_acl_entry_t*
stbl_acl_entry_get_acl_entry(stbl_acl_entry_key_t *p_ace_key);

char*
stbl_acl_entry_key_val2str(stbl_acl_entry_t *p_ace, char *str, uint32 str_len);

int32
stbl_acl_entry_key_str2val(char *str, stbl_acl_entry_t *p_ace);

int32
stbl_acl_entry_dump_one(stbl_acl_entry_t *p_ace, stbl_iter_args_t *pargs);

int32
stbl_acl_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_entry_master_t*
stbl_acl_entry_get_master();

stbl_acl_entry_master_t*
stbl_acl_entry_init_acl_entry();

#endif /* !__STBL_ACL_ENTRY_H__ */

