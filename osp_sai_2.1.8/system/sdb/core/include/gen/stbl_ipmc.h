
#ifndef __STBL_IPMC_H__
#define __STBL_IPMC_H__

int32
stbl_ipmc_add_ipmc_sync(stbl_ipmc_t *p_ipmc, uint32 sync);

int32
stbl_ipmc_del_ipmc_sync(stbl_ipmc_key_t *p_ipmc_key, uint32 sync);

int32
stbl_ipmc_set_ipmc_field_sync(stbl_ipmc_t *p_ipmc, stbl_ipmc_field_id_t field_id, uint32 sync);

int32
stbl_ipmc_add_ipmc(stbl_ipmc_t *p_ipmc);

int32
stbl_ipmc_del_ipmc(stbl_ipmc_key_t *p_ipmc_key);

int32
stbl_ipmc_set_ipmc_field(stbl_ipmc_t *p_ipmc, stbl_ipmc_field_id_t field_id);

stbl_ipmc_t*
stbl_ipmc_get_ipmc(stbl_ipmc_key_t *p_ipmc_key);

char*
stbl_ipmc_key_val2str(stbl_ipmc_t *p_ipmc, char *str, uint32 str_len);

int32
stbl_ipmc_key_str2val(char *str, stbl_ipmc_t *p_ipmc);

int32
stbl_ipmc_dump_one(stbl_ipmc_t *p_ipmc, stbl_iter_args_t *pargs);

int32
stbl_ipmc_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_ipmc_master_t*
stbl_ipmc_get_master();

stbl_ipmc_master_t*
stbl_ipmc_init_ipmc();

#endif /* !__STBL_IPMC_H__ */

