
#ifndef __STBL_SAI_GLB_H__
#define __STBL_SAI_GLB_H__

int32
stbl_sai_glb_set_sai_glb_field_sync(stbl_sai_glb_t *p_sai_glb, stbl_sai_glb_field_id_t field_id, uint32 sync);

int32
stbl_sai_glb_set_sai_glb_field(stbl_sai_glb_t *p_sai_glb, stbl_sai_glb_field_id_t field_id);

stbl_sai_glb_t*
stbl_sai_glb_get_sai_glb();

char*
stbl_sai_glb_key_val2str(stbl_sai_glb_t *p_sai_glb, char *str, uint32 str_len);

int32
stbl_sai_glb_key_str2val(char *str, stbl_sai_glb_t *p_sai_glb);

int32
stbl_sai_glb_dump_one(stbl_sai_glb_t *p_sai_glb, stbl_iter_args_t *pargs);

int32
stbl_sai_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_sai_glb_t*
stbl_sai_glb_init_sai_glb();

#endif /* !__STBL_SAI_GLB_H__ */

