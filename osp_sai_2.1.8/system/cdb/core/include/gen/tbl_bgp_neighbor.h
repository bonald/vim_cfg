
#ifndef __TBL_BGP_NEIGHBOR_H__
#define __TBL_BGP_NEIGHBOR_H__

int32
tbl_bgp_neighbor_add_bgp_neighbor_sync(tbl_bgp_neighbor_t *p_bgp_neighbor, uint32 sync);

int32
tbl_bgp_neighbor_del_bgp_neighbor_sync(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key, uint32 sync);

int32
tbl_bgp_neighbor_set_bgp_neighbor_field_sync(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_bgp_neighbor_field_id_t field_id, uint32 sync);

int32
tbl_bgp_neighbor_add_bgp_neighbor(tbl_bgp_neighbor_t *p_bgp_neighbor);

int32
tbl_bgp_neighbor_del_bgp_neighbor(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key);

int32
tbl_bgp_neighbor_set_bgp_neighbor_field(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_bgp_neighbor_field_id_t field_id);

tbl_bgp_neighbor_t*
tbl_bgp_neighbor_get_bgp_neighbor(tbl_bgp_neighbor_key_t *p_bgp_neighbor_key);

char*
tbl_bgp_neighbor_key_val2str(tbl_bgp_neighbor_t *p_bgp_neighbor, char *str, uint32 str_len);

int32
tbl_bgp_neighbor_key_str2val(char *str, tbl_bgp_neighbor_t *p_bgp_neighbor);

int32
tbl_bgp_neighbor_dump_one(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_iter_args_t *pargs);

char*
tbl_bgp_neighbor_key_name_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char* str);

char*
tbl_bgp_neighbor_key_value_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char* str);

char*
tbl_bgp_neighbor_field_name_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, int32 field_id, char* str);

char*
tbl_bgp_neighbor_field_value_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, int32 field_id, char* str);

char**
tbl_bgp_neighbor_table_dump(tbl_bgp_neighbor_t *p_bgp_neighbor, char** str);

int32
tbl_bgp_neighbor_table_parser(char** array, char* key_value, tbl_bgp_neighbor_t *p_bgp_neighbor);

int32
tbl_bgp_neighbor_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_bgp_neighbor_master_t*
tbl_bgp_neighbor_get_master();

tbl_bgp_neighbor_master_t*
tbl_bgp_neighbor_init_bgp_neighbor();

#endif /* !__TBL_BGP_NEIGHBOR_H__ */

