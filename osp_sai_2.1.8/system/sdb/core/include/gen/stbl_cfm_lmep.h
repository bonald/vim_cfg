
#ifndef __STBL_CFM_LMEP_H__
#define __STBL_CFM_LMEP_H__

int32
stbl_cfm_lmep_add_cfm_lmep_sync(stbl_cfm_lmep_t *p_lmep, uint32 sync);

int32
stbl_cfm_lmep_del_cfm_lmep_sync(stbl_cfm_lmep_key_t *p_lmep_key, uint32 sync);

int32
stbl_cfm_lmep_set_cfm_lmep_field_sync(stbl_cfm_lmep_t *p_lmep, stbl_cfm_lmep_field_id_t field_id, uint32 sync);

int32
stbl_cfm_lmep_add_cfm_lmep(stbl_cfm_lmep_t *p_lmep);

int32
stbl_cfm_lmep_del_cfm_lmep(stbl_cfm_lmep_key_t *p_lmep_key);

int32
stbl_cfm_lmep_set_cfm_lmep_field(stbl_cfm_lmep_t *p_lmep, stbl_cfm_lmep_field_id_t field_id);

stbl_cfm_lmep_t*
stbl_cfm_lmep_get_cfm_lmep(stbl_cfm_lmep_key_t *p_lmep_key);

char*
stbl_cfm_lmep_key_val2str(stbl_cfm_lmep_t *p_lmep, char *str, uint32 str_len);

int32
stbl_cfm_lmep_key_str2val(char *str, stbl_cfm_lmep_t *p_lmep);

int32
stbl_cfm_lmep_dump_one(stbl_cfm_lmep_t *p_lmep, stbl_iter_args_t *pargs);

int32
stbl_cfm_lmep_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_cfm_lmep_master_t*
stbl_cfm_lmep_get_master();

stbl_cfm_lmep_master_t*
stbl_cfm_lmep_init_cfm_lmep();

#endif /* !__STBL_CFM_LMEP_H__ */

