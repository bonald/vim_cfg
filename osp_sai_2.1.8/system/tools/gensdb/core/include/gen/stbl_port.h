
#ifndef __STBL_PORT_H__
#define __STBL_PORT_H__

int32
stbl_port_add_port_sync(stbl_port_t *p_port, uint32 sync);

int32
stbl_port_del_port_sync(stbl_port_key_t *p_port_key, uint32 sync);

int32
stbl_port_set_port_field_sync(stbl_port_t *p_port, stbl_port_field_id_t field_id, uint32 sync);

int32
stbl_port_add_port(stbl_port_t *p_port);

int32
stbl_port_del_port(stbl_port_key_t *p_port_key);

int32
stbl_port_set_port_field(stbl_port_t *p_port, stbl_port_field_id_t field_id);

stbl_port_t*
stbl_port_get_port(stbl_port_key_t *p_port_key);

char*
stbl_port_key_val2str(stbl_port_t *p_port, char *str, uint32 str_len);

int32
stbl_port_key_str2val(char *str, stbl_port_t *p_port);

int32
stbl_port_dump_one(stbl_port_t *p_port, stbl_iter_args_t *pargs);

int32
stbl_port_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_port_master_t*
stbl_port_get_master();

stbl_port_master_t*
stbl_port_init_port();

#endif /* !__STBL_PORT_H__ */

