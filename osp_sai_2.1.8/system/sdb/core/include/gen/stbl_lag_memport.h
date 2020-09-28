
#ifndef __STBL_LAG_MEMPORT_H__
#define __STBL_LAG_MEMPORT_H__

int32
stbl_lag_memport_add_lag_memport_sync(stbl_lag_memport_t *p_memport, uint32 sync);

int32
stbl_lag_memport_del_lag_memport_sync(stbl_lag_memport_key_t *p_memport_key, uint32 sync);

int32
stbl_lag_memport_set_lag_memport_field_sync(stbl_lag_memport_t *p_memport, stbl_lag_memport_field_id_t field_id, uint32 sync);

int32
stbl_lag_memport_add_lag_memport(stbl_lag_memport_t *p_memport);

int32
stbl_lag_memport_del_lag_memport(stbl_lag_memport_key_t *p_memport_key);

int32
stbl_lag_memport_set_lag_memport_field(stbl_lag_memport_t *p_memport, stbl_lag_memport_field_id_t field_id);

stbl_lag_memport_t*
stbl_lag_memport_get_lag_memport(stbl_lag_memport_key_t *p_memport_key);

char*
stbl_lag_memport_key_val2str(stbl_lag_memport_t *p_memport, char *str, uint32 str_len);

int32
stbl_lag_memport_key_str2val(char *str, stbl_lag_memport_t *p_memport);

int32
stbl_lag_memport_dump_one(stbl_lag_memport_t *p_memport, stbl_iter_args_t *pargs);

int32
stbl_lag_memport_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_lag_memport_master_t*
stbl_lag_memport_get_master();

stbl_lag_memport_master_t*
stbl_lag_memport_init_lag_memport();

#endif /* !__STBL_LAG_MEMPORT_H__ */

