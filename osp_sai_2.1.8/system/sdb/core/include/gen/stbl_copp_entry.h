
#ifndef __STBL_COPP_ENTRY_H__
#define __STBL_COPP_ENTRY_H__

int32
stbl_copp_entry_add_copp_entry_sync(stbl_copp_entry_t *p_copp_entry, uint32 sync);

int32
stbl_copp_entry_del_copp_entry_sync(stbl_copp_entry_key_t *p_copp_entry_key, uint32 sync);

int32
stbl_copp_entry_set_copp_entry_field_sync(stbl_copp_entry_t *p_copp_entry, stbl_copp_entry_field_id_t field_id, uint32 sync);

int32
stbl_copp_entry_add_copp_entry(stbl_copp_entry_t *p_copp_entry);

int32
stbl_copp_entry_del_copp_entry(stbl_copp_entry_key_t *p_copp_entry_key);

int32
stbl_copp_entry_set_copp_entry_field(stbl_copp_entry_t *p_copp_entry, stbl_copp_entry_field_id_t field_id);

stbl_copp_entry_t*
stbl_copp_entry_get_copp_entry(stbl_copp_entry_key_t *p_copp_entry_key);

char*
stbl_copp_entry_key_val2str(stbl_copp_entry_t *p_copp_entry, char *str, uint32 str_len);

int32
stbl_copp_entry_key_str2val(char *str, stbl_copp_entry_t *p_copp_entry);

int32
stbl_copp_entry_dump_one(stbl_copp_entry_t *p_copp_entry, stbl_iter_args_t *pargs);

int32
stbl_copp_entry_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_copp_entry_master_t*
stbl_copp_entry_get_master();

stbl_copp_entry_master_t*
stbl_copp_entry_init_copp_entry();

#endif /* !__STBL_COPP_ENTRY_H__ */

