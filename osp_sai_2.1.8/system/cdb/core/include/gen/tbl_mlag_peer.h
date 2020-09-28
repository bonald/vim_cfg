
#ifndef __TBL_MLAG_PEER_H__
#define __TBL_MLAG_PEER_H__

int32
tbl_mlag_peer_set_mlag_peer_field_sync(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_field_id_t field_id, uint32 sync);

int32
tbl_mlag_peer_set_mlag_peer_field(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_field_id_t field_id);

tbl_mlag_peer_t*
tbl_mlag_peer_get_mlag_peer();

char*
tbl_mlag_peer_key_val2str(tbl_mlag_peer_t *p_peer, char *str, uint32 str_len);

int32
tbl_mlag_peer_key_str2val(char *str, tbl_mlag_peer_t *p_peer);

int32
tbl_mlag_peer_dump_one(tbl_mlag_peer_t *p_peer, tbl_iter_args_t *pargs);

char*
tbl_mlag_peer_key_name_dump(tbl_mlag_peer_t *p_peer, char* str);

char*
tbl_mlag_peer_key_value_dump(tbl_mlag_peer_t *p_peer, char* str);

char*
tbl_mlag_peer_field_name_dump(tbl_mlag_peer_t *p_peer, int32 field_id, char* str);

char*
tbl_mlag_peer_field_value_dump(tbl_mlag_peer_t *p_peer, int32 field_id, char* str);

char**
tbl_mlag_peer_table_dump(tbl_mlag_peer_t *p_peer, char** str);

int32
tbl_mlag_peer_table_parser(char** array, char* key_value, tbl_mlag_peer_t *p_peer);

int32
tbl_mlag_peer_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_mlag_peer_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mlag_peer_t*
tbl_mlag_peer_init_mlag_peer();

#endif /* !__TBL_MLAG_PEER_H__ */

