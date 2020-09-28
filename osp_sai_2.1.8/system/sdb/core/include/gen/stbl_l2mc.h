
#ifndef __STBL_L2MC_H__
#define __STBL_L2MC_H__

int32
stbl_l2mc_add_l2mc_sync(stbl_l2mc_t *p_l2mc, uint32 sync);

int32
stbl_l2mc_del_l2mc_sync(stbl_l2mc_key_t *p_l2mc_key, uint32 sync);

int32
stbl_l2mc_set_l2mc_field_sync(stbl_l2mc_t *p_l2mc, stbl_l2mc_field_id_t field_id, uint32 sync);

int32
stbl_l2mc_add_l2mc(stbl_l2mc_t *p_l2mc);

int32
stbl_l2mc_del_l2mc(stbl_l2mc_key_t *p_l2mc_key);

int32
stbl_l2mc_set_l2mc_field(stbl_l2mc_t *p_l2mc, stbl_l2mc_field_id_t field_id);

stbl_l2mc_t*
stbl_l2mc_get_l2mc(stbl_l2mc_key_t *p_l2mc_key);

char*
stbl_l2mc_key_val2str(stbl_l2mc_t *p_l2mc, char *str, uint32 str_len);

int32
stbl_l2mc_key_str2val(char *str, stbl_l2mc_t *p_l2mc);

int32
stbl_l2mc_dump_one(stbl_l2mc_t *p_l2mc, stbl_iter_args_t *pargs);

int32
stbl_l2mc_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_l2mc_master_t*
stbl_l2mc_get_master();

stbl_l2mc_master_t*
stbl_l2mc_init_l2mc();

#endif /* !__STBL_L2MC_H__ */

