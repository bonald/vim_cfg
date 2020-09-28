
#ifndef __STBL_ACL_GLB_H__
#define __STBL_ACL_GLB_H__

int32
stbl_acl_glb_set_acl_glb_field_sync(stbl_acl_glb_t *p_acl_glb, stbl_acl_glb_field_id_t field_id, uint32 sync);

int32
stbl_acl_glb_set_acl_glb_field(stbl_acl_glb_t *p_acl_glb, stbl_acl_glb_field_id_t field_id);

stbl_acl_glb_t*
stbl_acl_glb_get_acl_glb();

char*
stbl_acl_glb_key_val2str(stbl_acl_glb_t *p_acl_glb, char *str, uint32 str_len);

int32
stbl_acl_glb_key_str2val(char *str, stbl_acl_glb_t *p_acl_glb);

int32
stbl_acl_glb_dump_one(stbl_acl_glb_t *p_acl_glb, stbl_iter_args_t *pargs);

int32
stbl_acl_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_glb_t*
stbl_acl_glb_init_acl_glb();

#endif /* !__STBL_ACL_GLB_H__ */

