
#ifndef __STBL_ACL_TUNNEL_H__
#define __STBL_ACL_TUNNEL_H__

int32
stbl_acl_tunnel_add_acl_tunnel_sync(stbl_acl_tunnel_t *p_tunnel, uint32 sync);

int32
stbl_acl_tunnel_del_acl_tunnel_sync(stbl_acl_tunnel_key_t *p_tunnel_key, uint32 sync);

int32
stbl_acl_tunnel_set_acl_tunnel_field_sync(stbl_acl_tunnel_t *p_tunnel, stbl_acl_tunnel_field_id_t field_id, uint32 sync);

int32
stbl_acl_tunnel_add_acl_tunnel(stbl_acl_tunnel_t *p_tunnel);

int32
stbl_acl_tunnel_del_acl_tunnel(stbl_acl_tunnel_key_t *p_tunnel_key);

int32
stbl_acl_tunnel_set_acl_tunnel_field(stbl_acl_tunnel_t *p_tunnel, stbl_acl_tunnel_field_id_t field_id);

stbl_acl_tunnel_t*
stbl_acl_tunnel_get_acl_tunnel(stbl_acl_tunnel_key_t *p_tunnel_key);

char*
stbl_acl_tunnel_key_val2str(stbl_acl_tunnel_t *p_tunnel, char *str, uint32 str_len);

int32
stbl_acl_tunnel_key_str2val(char *str, stbl_acl_tunnel_t *p_tunnel);

int32
stbl_acl_tunnel_dump_one(stbl_acl_tunnel_t *p_tunnel, stbl_iter_args_t *pargs);

int32
stbl_acl_tunnel_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_tunnel_master_t*
stbl_acl_tunnel_get_master();

stbl_acl_tunnel_master_t*
stbl_acl_tunnel_init_acl_tunnel();

#endif /* !__STBL_ACL_TUNNEL_H__ */

