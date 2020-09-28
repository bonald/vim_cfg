
#ifndef __TBL_RIPNG_NETWORK_H__
#define __TBL_RIPNG_NETWORK_H__

int32
tbl_ripng_network_add_ripng_network_sync(tbl_ripng_network_t *p_ripng_network, uint32 sync);

int32
tbl_ripng_network_del_ripng_network_sync(tbl_ripng_network_key_t *p_ripng_network_key, uint32 sync);

int32
tbl_ripng_network_set_ripng_network_field_sync(tbl_ripng_network_t *p_ripng_network, tbl_ripng_network_field_id_t field_id, uint32 sync);

int32
tbl_ripng_network_add_ripng_network(tbl_ripng_network_t *p_ripng_network);

int32
tbl_ripng_network_del_ripng_network(tbl_ripng_network_key_t *p_ripng_network_key);

int32
tbl_ripng_network_set_ripng_network_field(tbl_ripng_network_t *p_ripng_network, tbl_ripng_network_field_id_t field_id);

tbl_ripng_network_t*
tbl_ripng_network_get_ripng_network(tbl_ripng_network_key_t *p_ripng_network_key);

char*
tbl_ripng_network_key_val2str(tbl_ripng_network_t *p_ripng_network, char *str, uint32 str_len);

int32
tbl_ripng_network_key_str2val(char *str, tbl_ripng_network_t *p_ripng_network);

int32
tbl_ripng_network_dump_one(tbl_ripng_network_t *p_ripng_network, tbl_iter_args_t *pargs);

char*
tbl_ripng_network_key_name_dump(tbl_ripng_network_t *p_ripng_network, char* str);

char*
tbl_ripng_network_key_value_dump(tbl_ripng_network_t *p_ripng_network, char* str);

char*
tbl_ripng_network_field_name_dump(tbl_ripng_network_t *p_ripng_network, int32 field_id, char* str);

char*
tbl_ripng_network_field_value_dump(tbl_ripng_network_t *p_ripng_network, int32 field_id, char* str);

char**
tbl_ripng_network_table_dump(tbl_ripng_network_t *p_ripng_network, char** str);

int32
tbl_ripng_network_table_parser(char** array, char* key_value, tbl_ripng_network_t *p_ripng_network);

int32
tbl_ripng_network_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ripng_network_master_t*
tbl_ripng_network_get_master();

tbl_ripng_network_master_t*
tbl_ripng_network_init_ripng_network();

#endif /* !__TBL_RIPNG_NETWORK_H__ */

