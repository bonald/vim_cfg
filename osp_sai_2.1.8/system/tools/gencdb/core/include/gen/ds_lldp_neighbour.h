
#ifndef __DS_LLDP_NEIGHBOUR_H__
#define __DS_LLDP_NEIGHBOUR_H__

int32
ds_lldp_neighbour_add_lldp_neighbour_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, uint32 sync);

int32
ds_lldp_neighbour_del_lldp_neighbour_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, uint32 sync);

int32
ds_lldp_neighbour_set_lldp_neighbour_field_sync(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, ds_lldp_neighbour_field_id_t field_id, uint32 sync);

int32
ds_lldp_neighbour_add_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

int32
ds_lldp_neighbour_del_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

int32
ds_lldp_neighbour_set_lldp_neighbour_field(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, ds_lldp_neighbour_field_id_t field_id);

ds_lldp_neighbour_t*
ds_lldp_neighbour_get_lldp_neighbour(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

int32
ds_lldp_neighbour_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

char*
ds_lldp_neighbour_key_name_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char* str);

char*
ds_lldp_neighbour_key_value_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char* str);

char*
ds_lldp_neighbour_field_name_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, int32 field_id, char* str);

char*
ds_lldp_neighbour_field_value_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, int32 field_id, char* str);

char**
ds_lldp_neighbour_table_dump(tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh, char** str);

int32
ds_lldp_neighbour_field_value_parser(char* str, int32 field_id, tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

int32
ds_lldp_neighbour_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_lldp_if_t *p_lldp_if, ds_lldp_neighbour_t *p_neigh);

int32
ds_lldp_neighbour_dump_one(ds_lldp_neighbour_t *p_neigh, tbl_iter_args_t *pargs);

#endif /* !__DS_LLDP_NEIGHBOUR_H__ */

