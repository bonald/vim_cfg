
#ifndef __STBL_COPP_TRAP_H__
#define __STBL_COPP_TRAP_H__

int32
stbl_copp_trap_add_copp_trap_sync(stbl_copp_trap_t *p_trap, uint32 sync);

int32
stbl_copp_trap_del_copp_trap_sync(stbl_copp_trap_key_t *p_trap_key, uint32 sync);

int32
stbl_copp_trap_set_copp_trap_field_sync(stbl_copp_trap_t *p_trap, stbl_copp_trap_field_id_t field_id, uint32 sync);

int32
stbl_copp_trap_add_copp_trap(stbl_copp_trap_t *p_trap);

int32
stbl_copp_trap_del_copp_trap(stbl_copp_trap_key_t *p_trap_key);

int32
stbl_copp_trap_set_copp_trap_field(stbl_copp_trap_t *p_trap, stbl_copp_trap_field_id_t field_id);

stbl_copp_trap_t*
stbl_copp_trap_get_copp_trap(stbl_copp_trap_key_t *p_trap_key);

char*
stbl_copp_trap_key_val2str(stbl_copp_trap_t *p_trap, char *str, uint32 str_len);

int32
stbl_copp_trap_key_str2val(char *str, stbl_copp_trap_t *p_trap);

int32
stbl_copp_trap_dump_one(stbl_copp_trap_t *p_trap, stbl_iter_args_t *pargs);

int32
stbl_copp_trap_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_copp_trap_master_t*
stbl_copp_trap_get_master();

stbl_copp_trap_master_t*
stbl_copp_trap_init_copp_trap();

#endif /* !__STBL_COPP_TRAP_H__ */

