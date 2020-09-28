
#ifndef __TBL_RIP_NEIGHBOR_H__
#define __TBL_RIP_NEIGHBOR_H__

int32
tbl_rip_neighbor_add_rip_neighbor_sync(tbl_rip_neighbor_t *p_rip_neighbor, uint32 sync);

int32
tbl_rip_neighbor_del_rip_neighbor_sync(tbl_rip_neighbor_key_t *p_rip_neighbor_key, uint32 sync);

int32
tbl_rip_neighbor_set_rip_neighbor_field_sync(tbl_rip_neighbor_t *p_rip_neighbor, tbl_rip_neighbor_field_id_t field_id, uint32 sync);

int32
tbl_rip_neighbor_add_rip_neighbor(tbl_rip_neighbor_t *p_rip_neighbor);

int32
tbl_rip_neighbor_del_rip_neighbor(tbl_rip_neighbor_key_t *p_rip_neighbor_key);

int32
tbl_rip_neighbor_set_rip_neighbor_field(tbl_rip_neighbor_t *p_rip_neighbor, tbl_rip_neighbor_field_id_t field_id);

tbl_rip_neighbor_t*
tbl_rip_neighbor_get_rip_neighbor(tbl_rip_neighbor_key_t *p_rip_neighbor_key);

char*
tbl_rip_neighbor_key_val2str(tbl_rip_neighbor_t *p_rip_neighbor, char *str, uint32 str_len);

int32
tbl_rip_neighbor_key_str2val(char *str, tbl_rip_neighbor_t *p_rip_neighbor);

int32
tbl_rip_neighbor_dump_one(tbl_rip_neighbor_t *p_rip_neighbor, tbl_iter_args_t *pargs);

char*
tbl_rip_neighbor_key_name_dump(tbl_rip_neighbor_t *p_rip_neighbor, char* str);

char*
tbl_rip_neighbor_key_value_dump(tbl_rip_neighbor_t *p_rip_neighbor, char* str);

char*
tbl_rip_neighbor_field_name_dump(tbl_rip_neighbor_t *p_rip_neighbor, int32 field_id, char* str);

char*
tbl_rip_neighbor_field_value_dump(tbl_rip_neighbor_t *p_rip_neighbor, int32 field_id, char* str);

char**
tbl_rip_neighbor_table_dump(tbl_rip_neighbor_t *p_rip_neighbor, char** str);

int32
tbl_rip_neighbor_table_parser(char** array, char* key_value, tbl_rip_neighbor_t *p_rip_neighbor);

int32
tbl_rip_neighbor_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rip_neighbor_master_t*
tbl_rip_neighbor_get_master();

tbl_rip_neighbor_master_t*
tbl_rip_neighbor_init_rip_neighbor();

#endif /* !__TBL_RIP_NEIGHBOR_H__ */

