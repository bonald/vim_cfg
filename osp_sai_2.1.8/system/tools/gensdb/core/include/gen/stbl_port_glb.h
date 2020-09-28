
#ifndef __STBL_PORT_GLB_H__
#define __STBL_PORT_GLB_H__

int32
stbl_port_glb_set_port_glb_field_sync(stbl_port_glb_t *p_port_glb, stbl_port_glb_field_id_t field_id, uint32 sync);

int32
stbl_port_glb_set_port_glb_field(stbl_port_glb_t *p_port_glb, stbl_port_glb_field_id_t field_id);

stbl_port_glb_t*
stbl_port_glb_get_port_glb();

char*
stbl_port_glb_key_val2str(stbl_port_glb_t *p_port_glb, char *str, uint32 str_len);

int32
stbl_port_glb_key_str2val(char *str, stbl_port_glb_t *p_port_glb);

int32
stbl_port_glb_dump_one(stbl_port_glb_t *p_port_glb, stbl_iter_args_t *pargs);

int32
stbl_port_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_port_glb_t*
stbl_port_glb_init_port_glb();

#endif /* !__STBL_PORT_GLB_H__ */

