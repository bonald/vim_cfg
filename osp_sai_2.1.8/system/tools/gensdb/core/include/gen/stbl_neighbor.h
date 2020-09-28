
#ifndef __STBL_NEIGHBOR_H__
#define __STBL_NEIGHBOR_H__

int32
stbl_neighbor_add_neighbor_sync(stbl_neighbor_t *p_neigh, uint32 sync);

int32
stbl_neighbor_del_neighbor_sync(stbl_neighbor_key_t *p_neigh_key, uint32 sync);

int32
stbl_neighbor_set_neighbor_field_sync(stbl_neighbor_t *p_neigh, stbl_neighbor_field_id_t field_id, uint32 sync);

int32
stbl_neighbor_add_neighbor(stbl_neighbor_t *p_neigh);

int32
stbl_neighbor_del_neighbor(stbl_neighbor_key_t *p_neigh_key);

int32
stbl_neighbor_set_neighbor_field(stbl_neighbor_t *p_neigh, stbl_neighbor_field_id_t field_id);

stbl_neighbor_t*
stbl_neighbor_get_neighbor(stbl_neighbor_key_t *p_neigh_key);

char*
stbl_neighbor_key_val2str(stbl_neighbor_t *p_neigh, char *str, uint32 str_len);

int32
stbl_neighbor_key_str2val(char *str, stbl_neighbor_t *p_neigh);

int32
stbl_neighbor_dump_one(stbl_neighbor_t *p_neigh, stbl_iter_args_t *pargs);

int32
stbl_neighbor_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_neighbor_master_t*
stbl_neighbor_get_master();

stbl_neighbor_master_t*
stbl_neighbor_init_neighbor();

#endif /* !__STBL_NEIGHBOR_H__ */

