
#ifndef __TBL_OSPF_NETWORK_H__
#define __TBL_OSPF_NETWORK_H__

int32
tbl_ospf_network_add_ospf_network_sync(tbl_ospf_network_t *p_ospf_network, uint32 sync);

int32
tbl_ospf_network_del_ospf_network_sync(tbl_ospf_network_key_t *p_ospf_network_key, uint32 sync);

int32
tbl_ospf_network_set_ospf_network_field_sync(tbl_ospf_network_t *p_ospf_network, tbl_ospf_network_field_id_t field_id, uint32 sync);

int32
tbl_ospf_network_add_ospf_network(tbl_ospf_network_t *p_ospf_network);

int32
tbl_ospf_network_del_ospf_network(tbl_ospf_network_key_t *p_ospf_network_key);

int32
tbl_ospf_network_set_ospf_network_field(tbl_ospf_network_t *p_ospf_network, tbl_ospf_network_field_id_t field_id);

tbl_ospf_network_t*
tbl_ospf_network_get_ospf_network(tbl_ospf_network_key_t *p_ospf_network_key);

char*
tbl_ospf_network_key_val2str(tbl_ospf_network_t *p_ospf_network, char *str, uint32 str_len);

int32
tbl_ospf_network_key_str2val(char *str, tbl_ospf_network_t *p_ospf_network);

int32
tbl_ospf_network_dump_one(tbl_ospf_network_t *p_ospf_network, tbl_iter_args_t *pargs);

char*
tbl_ospf_network_key_name_dump(tbl_ospf_network_t *p_ospf_network, char* str);

char*
tbl_ospf_network_key_value_dump(tbl_ospf_network_t *p_ospf_network, char* str);

char*
tbl_ospf_network_field_name_dump(tbl_ospf_network_t *p_ospf_network, int32 field_id, char* str);

char*
tbl_ospf_network_field_value_dump(tbl_ospf_network_t *p_ospf_network, int32 field_id, char* str);

char**
tbl_ospf_network_table_dump(tbl_ospf_network_t *p_ospf_network, char** str);

int32
tbl_ospf_network_table_parser(char** array, char* key_value, tbl_ospf_network_t *p_ospf_network);

int32
tbl_ospf_network_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_network_master_t*
tbl_ospf_network_get_master();

tbl_ospf_network_master_t*
tbl_ospf_network_init_ospf_network();

#endif /* !__TBL_OSPF_NETWORK_H__ */

