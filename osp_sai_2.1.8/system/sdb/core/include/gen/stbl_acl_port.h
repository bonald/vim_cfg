
#ifndef __STBL_ACL_PORT_H__
#define __STBL_ACL_PORT_H__

int32
stbl_acl_port_add_acl_port_sync(stbl_acl_port_t *p_port, uint32 sync);

int32
stbl_acl_port_del_acl_port_sync(stbl_acl_port_key_t *p_port_key, uint32 sync);

int32
stbl_acl_port_set_acl_port_field_sync(stbl_acl_port_t *p_port, stbl_acl_port_field_id_t field_id, uint32 sync);

int32
stbl_acl_port_add_acl_port(stbl_acl_port_t *p_port);

int32
stbl_acl_port_del_acl_port(stbl_acl_port_key_t *p_port_key);

int32
stbl_acl_port_set_acl_port_field(stbl_acl_port_t *p_port, stbl_acl_port_field_id_t field_id);

stbl_acl_port_t*
stbl_acl_port_get_acl_port(stbl_acl_port_key_t *p_port_key);

char*
stbl_acl_port_key_val2str(stbl_acl_port_t *p_port, char *str, uint32 str_len);

int32
stbl_acl_port_key_str2val(char *str, stbl_acl_port_t *p_port);

int32
stbl_acl_port_dump_one(stbl_acl_port_t *p_port, stbl_iter_args_t *pargs);

int32
stbl_acl_port_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_acl_port_master_t*
stbl_acl_port_get_master();

stbl_acl_port_master_t*
stbl_acl_port_init_acl_port();

#endif /* !__STBL_ACL_PORT_H__ */

