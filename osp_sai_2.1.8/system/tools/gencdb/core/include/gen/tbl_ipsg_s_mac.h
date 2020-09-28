
#ifndef __TBL_IPSG_S_MAC_H__
#define __TBL_IPSG_S_MAC_H__

int32
tbl_ipsg_s_mac_add_ipsg_s_mac_sync(tbl_ipsg_s_mac_t *p_ipsg_s_mac, uint32 sync);

int32
tbl_ipsg_s_mac_del_ipsg_s_mac_sync(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key, uint32 sync);

int32
tbl_ipsg_s_mac_set_ipsg_s_mac_field_sync(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_ipsg_s_mac_field_id_t field_id, uint32 sync);

int32
tbl_ipsg_s_mac_add_ipsg_s_mac(tbl_ipsg_s_mac_t *p_ipsg_s_mac);

int32
tbl_ipsg_s_mac_del_ipsg_s_mac(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key);

int32
tbl_ipsg_s_mac_set_ipsg_s_mac_field(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_ipsg_s_mac_field_id_t field_id);

tbl_ipsg_s_mac_t*
tbl_ipsg_s_mac_get_ipsg_s_mac(tbl_ipsg_s_mac_key_t *p_ipsg_s_mac_key);

char*
tbl_ipsg_s_mac_key_val2str(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char *str, uint32 str_len);

int32
tbl_ipsg_s_mac_key_str2val(char *str, tbl_ipsg_s_mac_t *p_ipsg_s_mac);

int32
tbl_ipsg_s_mac_dump_one(tbl_ipsg_s_mac_t *p_ipsg_s_mac, tbl_iter_args_t *pargs);

char*
tbl_ipsg_s_mac_key_name_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char* str);

char*
tbl_ipsg_s_mac_key_value_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char* str);

char*
tbl_ipsg_s_mac_field_name_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, int32 field_id, char* str);

char*
tbl_ipsg_s_mac_field_value_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, int32 field_id, char* str);

char**
tbl_ipsg_s_mac_table_dump(tbl_ipsg_s_mac_t *p_ipsg_s_mac, char** str);

int32
tbl_ipsg_s_mac_table_parser(char** array, char* key_value, tbl_ipsg_s_mac_t *p_ipsg_s_mac);

int32
tbl_ipsg_s_mac_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipsg_s_mac_master_t*
tbl_ipsg_s_mac_get_master();

tbl_ipsg_s_mac_master_t*
tbl_ipsg_s_mac_init_ipsg_s_mac();


tbl_ipsg_s_mac_t*
tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(mac_addr_t mac);


#endif /* !__TBL_IPSG_S_MAC_H__ */

