
#ifndef __STBL_COPP_GROUP_H__
#define __STBL_COPP_GROUP_H__

int32
stbl_copp_group_add_copp_group_sync(stbl_copp_group_t *p_group, uint32 sync);

int32
stbl_copp_group_del_copp_group_sync(stbl_copp_group_key_t *p_group_key, uint32 sync);

int32
stbl_copp_group_set_copp_group_field_sync(stbl_copp_group_t *p_group, stbl_copp_group_field_id_t field_id, uint32 sync);

int32
stbl_copp_group_add_copp_group(stbl_copp_group_t *p_group);

int32
stbl_copp_group_del_copp_group(stbl_copp_group_key_t *p_group_key);

int32
stbl_copp_group_set_copp_group_field(stbl_copp_group_t *p_group, stbl_copp_group_field_id_t field_id);

stbl_copp_group_t*
stbl_copp_group_get_copp_group(stbl_copp_group_key_t *p_group_key);

char*
stbl_copp_group_key_val2str(stbl_copp_group_t *p_group, char *str, uint32 str_len);

int32
stbl_copp_group_key_str2val(char *str, stbl_copp_group_t *p_group);

int32
stbl_copp_group_dump_one(stbl_copp_group_t *p_group, stbl_iter_args_t *pargs);

int32
stbl_copp_group_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_copp_group_master_t*
stbl_copp_group_get_master();

stbl_copp_group_master_t*
stbl_copp_group_init_copp_group();

#endif /* !__STBL_COPP_GROUP_H__ */

