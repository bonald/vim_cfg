
#ifndef __STBL_VR_H__
#define __STBL_VR_H__

int32
stbl_vr_add_vr_sync(stbl_vr_t *p_vr, uint32 sync);

int32
stbl_vr_del_vr_sync(stbl_vr_key_t *p_vr_key, uint32 sync);

int32
stbl_vr_set_vr_field_sync(stbl_vr_t *p_vr, stbl_vr_field_id_t field_id, uint32 sync);

int32
stbl_vr_add_vr(stbl_vr_t *p_vr);

int32
stbl_vr_del_vr(stbl_vr_key_t *p_vr_key);

int32
stbl_vr_set_vr_field(stbl_vr_t *p_vr, stbl_vr_field_id_t field_id);

stbl_vr_t*
stbl_vr_get_vr(stbl_vr_key_t *p_vr_key);

char*
stbl_vr_key_val2str(stbl_vr_t *p_vr, char *str, uint32 str_len);

int32
stbl_vr_key_str2val(char *str, stbl_vr_t *p_vr);

int32
stbl_vr_dump_one(stbl_vr_t *p_vr, stbl_iter_args_t *pargs);

int32
stbl_vr_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_vr_master_t*
stbl_vr_get_master();

stbl_vr_master_t*
stbl_vr_init_vr();

int32
stbl_vr_set_rif_list_sync(stbl_vr_t *p_vr, uint32 add, void *p_object, uint32 sync);

int32
stbl_vr_set_rif_list(stbl_vr_t *p_vr, uint32 add, void *p_object);

#endif /* !__STBL_VR_H__ */

