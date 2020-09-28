
#ifndef __STBL_RIF_H__
#define __STBL_RIF_H__

int32
stbl_rif_add_rif_sync(stbl_rif_t *p_rif, uint32 sync);

int32
stbl_rif_del_rif_sync(stbl_rif_key_t *p_rif_key, uint32 sync);

int32
stbl_rif_set_rif_field_sync(stbl_rif_t *p_rif, stbl_rif_field_id_t field_id, uint32 sync);

int32
stbl_rif_add_rif(stbl_rif_t *p_rif);

int32
stbl_rif_del_rif(stbl_rif_key_t *p_rif_key);

int32
stbl_rif_set_rif_field(stbl_rif_t *p_rif, stbl_rif_field_id_t field_id);

stbl_rif_t*
stbl_rif_get_rif(stbl_rif_key_t *p_rif_key);

char*
stbl_rif_key_val2str(stbl_rif_t *p_rif, char *str, uint32 str_len);

int32
stbl_rif_key_str2val(char *str, stbl_rif_t *p_rif);

int32
stbl_rif_dump_one(stbl_rif_t *p_rif, stbl_iter_args_t *pargs);

int32
stbl_rif_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_rif_master_t*
stbl_rif_get_master();

stbl_rif_master_t*
stbl_rif_init_rif();

#endif /* !__STBL_RIF_H__ */

