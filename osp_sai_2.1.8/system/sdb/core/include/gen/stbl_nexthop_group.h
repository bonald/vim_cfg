
#ifndef __STBL_NEXTHOP_GROUP_H__
#define __STBL_NEXTHOP_GROUP_H__

int32
stbl_nexthop_group_add_nexthop_group_sync(stbl_nexthop_group_t *p_nh_group, uint32 sync);

int32
stbl_nexthop_group_del_nexthop_group_sync(stbl_nexthop_group_key_t *p_nh_group_key, uint32 sync);

int32
stbl_nexthop_group_set_nexthop_group_field_sync(stbl_nexthop_group_t *p_nh_group, stbl_nexthop_group_field_id_t field_id, uint32 sync);

int32
stbl_nexthop_group_add_nexthop_group(stbl_nexthop_group_t *p_nh_group);

int32
stbl_nexthop_group_del_nexthop_group(stbl_nexthop_group_key_t *p_nh_group_key);

int32
stbl_nexthop_group_set_nexthop_group_field(stbl_nexthop_group_t *p_nh_group, stbl_nexthop_group_field_id_t field_id);

stbl_nexthop_group_t*
stbl_nexthop_group_get_nexthop_group(stbl_nexthop_group_key_t *p_nh_group_key);

char*
stbl_nexthop_group_key_val2str(stbl_nexthop_group_t *p_nh_group, char *str, uint32 str_len);

int32
stbl_nexthop_group_key_str2val(char *str, stbl_nexthop_group_t *p_nh_group);

int32
stbl_nexthop_group_dump_one(stbl_nexthop_group_t *p_nh_group, stbl_iter_args_t *pargs);

int32
stbl_nexthop_group_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_nexthop_group_master_t*
stbl_nexthop_group_get_master();

stbl_nexthop_group_master_t*
stbl_nexthop_group_init_nexthop_group();

int32
stbl_nexthop_group_set_nh_list_sync(stbl_nexthop_group_t *p_nh_group, uint32 add, void *p_object, uint32 sync);

int32
stbl_nexthop_group_set_nh_list(stbl_nexthop_group_t *p_nh_group, uint32 add, void *p_object);

#endif /* !__STBL_NEXTHOP_GROUP_H__ */

