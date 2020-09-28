
#ifndef __STBL_HOSTIF_H__
#define __STBL_HOSTIF_H__

int32
stbl_hostif_add_hostif_sync(stbl_hostif_t *p_hif, uint32 sync);

int32
stbl_hostif_del_hostif_sync(stbl_hostif_key_t *p_hif_key, uint32 sync);

int32
stbl_hostif_set_hostif_field_sync(stbl_hostif_t *p_hif, stbl_hostif_field_id_t field_id, uint32 sync);

int32
stbl_hostif_add_hostif(stbl_hostif_t *p_hif);

int32
stbl_hostif_del_hostif(stbl_hostif_key_t *p_hif_key);

int32
stbl_hostif_set_hostif_field(stbl_hostif_t *p_hif, stbl_hostif_field_id_t field_id);

stbl_hostif_t*
stbl_hostif_get_hostif(stbl_hostif_key_t *p_hif_key);

char*
stbl_hostif_key_val2str(stbl_hostif_t *p_hif, char *str, uint32 str_len);

int32
stbl_hostif_key_str2val(char *str, stbl_hostif_t *p_hif);

int32
stbl_hostif_dump_one(stbl_hostif_t *p_hif, stbl_iter_args_t *pargs);

int32
stbl_hostif_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_hostif_master_t*
stbl_hostif_get_master();

stbl_hostif_master_t*
stbl_hostif_init_hostif();

#endif /* !__STBL_HOSTIF_H__ */

