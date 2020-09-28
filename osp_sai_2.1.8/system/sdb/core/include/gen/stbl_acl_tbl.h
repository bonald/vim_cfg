
#ifndef __STBL_ACL_TBL_H__
#define __STBL_ACL_TBL_H__

int32
stbl_acl_tbl_add_acl_tbl_sync(stbl_acl_tbl_t *p_acl, uint32 sync);

int32
stbl_acl_tbl_del_acl_tbl_sync(stbl_acl_tbl_key_t *p_acl_key, uint32 sync);

int32
stbl_acl_tbl_set_acl_tbl_field_sync(stbl_acl_tbl_t *p_acl, stbl_acl_tbl_field_id_t field_id, uint32 sync);

int32
stbl_acl_tbl_add_acl_tbl(stbl_acl_tbl_t *p_acl);

int32
stbl_acl_tbl_del_acl_tbl(stbl_acl_tbl_key_t *p_acl_key);

int32
stbl_acl_tbl_set_acl_tbl_field(stbl_acl_tbl_t *p_acl, stbl_acl_tbl_field_id_t field_id);

stbl_acl_tbl_t*
stbl_acl_tbl_get_acl_tbl(stbl_acl_tbl_key_t *p_acl_key);

char*
stbl_acl_tbl_key_val2str(stbl_acl_tbl_t *p_acl, char *str, uint32 str_len);

int32
stbl_acl_tbl_key_str2val(char *str, stbl_acl_tbl_t *p_acl);

int32
stbl_acl_tbl_dump_one(stbl_acl_tbl_t *p_acl, stbl_iter_args_t *pargs);

int32
stbl_acl_tbl_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_tbl_master_t*
stbl_acl_tbl_get_master();

stbl_acl_tbl_master_t*
stbl_acl_tbl_init_acl_tbl();

#endif /* !__STBL_ACL_TBL_H__ */

