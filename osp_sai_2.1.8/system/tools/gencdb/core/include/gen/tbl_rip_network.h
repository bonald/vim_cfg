
#ifndef __TBL_RIP_NETWORK_H__
#define __TBL_RIP_NETWORK_H__

int32
tbl_rip_network_add_rip_network_sync(tbl_rip_network_t *p_rip_network, uint32 sync);

int32
tbl_rip_network_del_rip_network_sync(tbl_rip_network_key_t *p_rip_network_key, uint32 sync);

int32
tbl_rip_network_set_rip_network_field_sync(tbl_rip_network_t *p_rip_network, tbl_rip_network_field_id_t field_id, uint32 sync);

int32
tbl_rip_network_add_rip_network(tbl_rip_network_t *p_rip_network);

int32
tbl_rip_network_del_rip_network(tbl_rip_network_key_t *p_rip_network_key);

int32
tbl_rip_network_set_rip_network_field(tbl_rip_network_t *p_rip_network, tbl_rip_network_field_id_t field_id);

tbl_rip_network_t*
tbl_rip_network_get_rip_network(tbl_rip_network_key_t *p_rip_network_key);

char*
tbl_rip_network_key_val2str(tbl_rip_network_t *p_rip_network, char *str, uint32 str_len);

int32
tbl_rip_network_key_str2val(char *str, tbl_rip_network_t *p_rip_network);

int32
tbl_rip_network_dump_one(tbl_rip_network_t *p_rip_network, tbl_iter_args_t *pargs);

char*
tbl_rip_network_key_name_dump(tbl_rip_network_t *p_rip_network, char* str);

char*
tbl_rip_network_key_value_dump(tbl_rip_network_t *p_rip_network, char* str);

char*
tbl_rip_network_field_name_dump(tbl_rip_network_t *p_rip_network, int32 field_id, char* str);

char*
tbl_rip_network_field_value_dump(tbl_rip_network_t *p_rip_network, int32 field_id, char* str);

char**
tbl_rip_network_table_dump(tbl_rip_network_t *p_rip_network, char** str);

int32
tbl_rip_network_table_parser(char** array, char* key_value, tbl_rip_network_t *p_rip_network);

int32
tbl_rip_network_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rip_network_master_t*
tbl_rip_network_get_master();

tbl_rip_network_master_t*
tbl_rip_network_init_rip_network();

#endif /* !__TBL_RIP_NETWORK_H__ */

