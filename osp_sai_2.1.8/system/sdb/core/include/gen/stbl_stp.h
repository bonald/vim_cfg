
#ifndef __STBL_STP_H__
#define __STBL_STP_H__

int32
stbl_stp_add_stp_sync(stbl_stp_t *p_stp, uint32 sync);

int32
stbl_stp_del_stp_sync(stbl_stp_key_t *p_stp_key, uint32 sync);

int32
stbl_stp_set_stp_field_sync(stbl_stp_t *p_stp, stbl_stp_field_id_t field_id, uint32 sync);

int32
stbl_stp_add_stp(stbl_stp_t *p_stp);

int32
stbl_stp_del_stp(stbl_stp_key_t *p_stp_key);

int32
stbl_stp_set_stp_field(stbl_stp_t *p_stp, stbl_stp_field_id_t field_id);

stbl_stp_t*
stbl_stp_get_stp(stbl_stp_key_t *p_stp_key);

char*
stbl_stp_key_val2str(stbl_stp_t *p_stp, char *str, uint32 str_len);

int32
stbl_stp_key_str2val(char *str, stbl_stp_t *p_stp);

int32
stbl_stp_dump_one(stbl_stp_t *p_stp, stbl_iter_args_t *pargs);

int32
stbl_stp_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_stp_master_t*
stbl_stp_get_master();

stbl_stp_master_t*
stbl_stp_init_stp();

#endif /* !__STBL_STP_H__ */

