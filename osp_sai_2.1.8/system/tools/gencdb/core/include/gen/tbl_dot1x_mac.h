
#ifndef __TBL_DOT1X_MAC_H__
#define __TBL_DOT1X_MAC_H__

int32
tbl_dot1x_mac_add_dot1x_mac_sync(tbl_dot1x_mac_t *p_dot1x_mac, uint32 sync);

int32
tbl_dot1x_mac_del_dot1x_mac_sync(tbl_dot1x_mac_key_t *p_dot1x_mac_key, uint32 sync);

int32
tbl_dot1x_mac_set_dot1x_mac_field_sync(tbl_dot1x_mac_t *p_dot1x_mac, tbl_dot1x_mac_field_id_t field_id, uint32 sync);

int32
tbl_dot1x_mac_add_dot1x_mac(tbl_dot1x_mac_t *p_dot1x_mac);

int32
tbl_dot1x_mac_del_dot1x_mac(tbl_dot1x_mac_key_t *p_dot1x_mac_key);

int32
tbl_dot1x_mac_set_dot1x_mac_field(tbl_dot1x_mac_t *p_dot1x_mac, tbl_dot1x_mac_field_id_t field_id);

tbl_dot1x_mac_t*
tbl_dot1x_mac_get_dot1x_mac(tbl_dot1x_mac_key_t *p_dot1x_mac_key);

char*
tbl_dot1x_mac_key_val2str(tbl_dot1x_mac_t *p_dot1x_mac, char *str, uint32 str_len);

int32
tbl_dot1x_mac_key_str2val(char *str, tbl_dot1x_mac_t *p_dot1x_mac);

int32
tbl_dot1x_mac_dump_one(tbl_dot1x_mac_t *p_dot1x_mac, tbl_iter_args_t *pargs);

char*
tbl_dot1x_mac_key_name_dump(tbl_dot1x_mac_t *p_dot1x_mac, char* str);

char*
tbl_dot1x_mac_key_value_dump(tbl_dot1x_mac_t *p_dot1x_mac, char* str);

char*
tbl_dot1x_mac_field_name_dump(tbl_dot1x_mac_t *p_dot1x_mac, int32 field_id, char* str);

char*
tbl_dot1x_mac_field_value_dump(tbl_dot1x_mac_t *p_dot1x_mac, int32 field_id, char* str);

char**
tbl_dot1x_mac_table_dump(tbl_dot1x_mac_t *p_dot1x_mac, char** str);

int32
tbl_dot1x_mac_table_parser(char** array, char* key_value, tbl_dot1x_mac_t *p_dot1x_mac);

int32
tbl_dot1x_mac_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dot1x_mac_master_t*
tbl_dot1x_mac_get_master();

tbl_dot1x_mac_master_t*
tbl_dot1x_mac_init_dot1x_mac();

#endif /* !__TBL_DOT1X_MAC_H__ */

