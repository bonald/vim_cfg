
#ifndef __STBL_NEXTHOP_H__
#define __STBL_NEXTHOP_H__

int32
stbl_nexthop_add_nexthop_sync(stbl_nexthop_t *p_nexthop, uint32 sync);

int32
stbl_nexthop_del_nexthop_sync(stbl_nexthop_key_t *p_nexthop_key, uint32 sync);

int32
stbl_nexthop_set_nexthop_field_sync(stbl_nexthop_t *p_nexthop, stbl_nexthop_field_id_t field_id, uint32 sync);

int32
stbl_nexthop_add_nexthop(stbl_nexthop_t *p_nexthop);

int32
stbl_nexthop_del_nexthop(stbl_nexthop_key_t *p_nexthop_key);

int32
stbl_nexthop_set_nexthop_field(stbl_nexthop_t *p_nexthop, stbl_nexthop_field_id_t field_id);

stbl_nexthop_t*
stbl_nexthop_get_nexthop(stbl_nexthop_key_t *p_nexthop_key);

char*
stbl_nexthop_key_val2str(stbl_nexthop_t *p_nexthop, char *str, uint32 str_len);

int32
stbl_nexthop_key_str2val(char *str, stbl_nexthop_t *p_nexthop);

int32
stbl_nexthop_dump_one(stbl_nexthop_t *p_nexthop, stbl_iter_args_t *pargs);

int32
stbl_nexthop_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_nexthop_master_t*
stbl_nexthop_get_master();

stbl_nexthop_master_t*
stbl_nexthop_init_nexthop();

#endif /* !__STBL_NEXTHOP_H__ */

