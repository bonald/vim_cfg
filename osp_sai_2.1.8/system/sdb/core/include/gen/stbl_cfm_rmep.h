
#ifndef __STBL_CFM_RMEP_H__
#define __STBL_CFM_RMEP_H__

int32
stbl_cfm_rmep_add_cfm_rmep_sync(stbl_cfm_rmep_t *p_rmep, uint32 sync);

int32
stbl_cfm_rmep_del_cfm_rmep_sync(stbl_cfm_rmep_key_t *p_rmep_key, uint32 sync);

int32
stbl_cfm_rmep_set_cfm_rmep_field_sync(stbl_cfm_rmep_t *p_rmep, stbl_cfm_rmep_field_id_t field_id, uint32 sync);

int32
stbl_cfm_rmep_add_cfm_rmep(stbl_cfm_rmep_t *p_rmep);

int32
stbl_cfm_rmep_del_cfm_rmep(stbl_cfm_rmep_key_t *p_rmep_key);

int32
stbl_cfm_rmep_set_cfm_rmep_field(stbl_cfm_rmep_t *p_rmep, stbl_cfm_rmep_field_id_t field_id);

stbl_cfm_rmep_t*
stbl_cfm_rmep_get_cfm_rmep(stbl_cfm_rmep_key_t *p_rmep_key);

char*
stbl_cfm_rmep_key_val2str(stbl_cfm_rmep_t *p_rmep, char *str, uint32 str_len);

int32
stbl_cfm_rmep_key_str2val(char *str, stbl_cfm_rmep_t *p_rmep);

int32
stbl_cfm_rmep_dump_one(stbl_cfm_rmep_t *p_rmep, stbl_iter_args_t *pargs);

int32
stbl_cfm_rmep_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_cfm_rmep_master_t*
stbl_cfm_rmep_get_master();

stbl_cfm_rmep_master_t*
stbl_cfm_rmep_init_cfm_rmep();

#endif /* !__STBL_CFM_RMEP_H__ */

