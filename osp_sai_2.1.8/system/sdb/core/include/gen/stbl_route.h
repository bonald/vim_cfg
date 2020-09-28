
#ifndef __STBL_ROUTE_H__
#define __STBL_ROUTE_H__

int32
stbl_route_add_route_sync(stbl_route_t *p_route, uint32 sync);

int32
stbl_route_del_route_sync(stbl_route_key_t *p_route_key, uint32 sync);

int32
stbl_route_set_route_field_sync(stbl_route_t *p_route, stbl_route_field_id_t field_id, uint32 sync);

int32
stbl_route_add_route(stbl_route_t *p_route);

int32
stbl_route_del_route(stbl_route_key_t *p_route_key);

int32
stbl_route_set_route_field(stbl_route_t *p_route, stbl_route_field_id_t field_id);

stbl_route_t*
stbl_route_get_route(stbl_route_key_t *p_route_key);

char*
stbl_route_key_val2str(stbl_route_t *p_route, char *str, uint32 str_len);

int32
stbl_route_key_str2val(char *str, stbl_route_t *p_route);

int32
stbl_route_dump_one(stbl_route_t *p_route, stbl_iter_args_t *pargs);

int32
stbl_route_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_route_master_t*
stbl_route_get_master();

stbl_route_master_t*
stbl_route_init_route();

#endif /* !__STBL_ROUTE_H__ */

