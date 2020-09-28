
#ifndef __STBL_COPP_REASON_H__
#define __STBL_COPP_REASON_H__

int32
stbl_copp_reason_add_copp_reason_sync(stbl_copp_reason_t *p_reason, uint32 sync);

int32
stbl_copp_reason_del_copp_reason_sync(stbl_copp_reason_key_t *p_reason_key, uint32 sync);

int32
stbl_copp_reason_set_copp_reason_field_sync(stbl_copp_reason_t *p_reason, stbl_copp_reason_field_id_t field_id, uint32 sync);

int32
stbl_copp_reason_add_copp_reason(stbl_copp_reason_t *p_reason);

int32
stbl_copp_reason_del_copp_reason(stbl_copp_reason_key_t *p_reason_key);

int32
stbl_copp_reason_set_copp_reason_field(stbl_copp_reason_t *p_reason, stbl_copp_reason_field_id_t field_id);

stbl_copp_reason_t*
stbl_copp_reason_get_copp_reason(stbl_copp_reason_key_t *p_reason_key);

char*
stbl_copp_reason_key_val2str(stbl_copp_reason_t *p_reason, char *str, uint32 str_len);

int32
stbl_copp_reason_key_str2val(char *str, stbl_copp_reason_t *p_reason);

int32
stbl_copp_reason_dump_one(stbl_copp_reason_t *p_reason, stbl_iter_args_t *pargs);

int32
stbl_copp_reason_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_copp_reason_master_t*
stbl_copp_reason_get_master();

stbl_copp_reason_master_t*
stbl_copp_reason_init_copp_reason();

#endif /* !__STBL_COPP_REASON_H__ */

