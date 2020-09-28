
#ifndef __STBL_NAT_FORWARD_H__
#define __STBL_NAT_FORWARD_H__

int32
stbl_nat_forward_add_nat_forward_sync(stbl_nat_forward_t *p_nat_forward, uint32 sync);

int32
stbl_nat_forward_del_nat_forward_sync(stbl_nat_forward_key_t *p_nat_forward_key, uint32 sync);

int32
stbl_nat_forward_set_nat_forward_field_sync(stbl_nat_forward_t *p_nat_forward, stbl_nat_forward_field_id_t field_id, uint32 sync);

int32
stbl_nat_forward_add_nat_forward(stbl_nat_forward_t *p_nat_forward);

int32
stbl_nat_forward_del_nat_forward(stbl_nat_forward_key_t *p_nat_forward_key);

int32
stbl_nat_forward_set_nat_forward_field(stbl_nat_forward_t *p_nat_forward, stbl_nat_forward_field_id_t field_id);

stbl_nat_forward_t*
stbl_nat_forward_get_nat_forward(stbl_nat_forward_key_t *p_nat_forward_key);

char*
stbl_nat_forward_key_val2str(stbl_nat_forward_t *p_nat_forward, char *str, uint32 str_len);

int32
stbl_nat_forward_key_str2val(char *str, stbl_nat_forward_t *p_nat_forward);

int32
stbl_nat_forward_dump_one(stbl_nat_forward_t *p_nat_forward, stbl_iter_args_t *pargs);

int32
stbl_nat_forward_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_nat_forward_master_t*
stbl_nat_forward_get_master();

stbl_nat_forward_master_t*
stbl_nat_forward_init_nat_forward();

#endif /* !__STBL_NAT_FORWARD_H__ */

