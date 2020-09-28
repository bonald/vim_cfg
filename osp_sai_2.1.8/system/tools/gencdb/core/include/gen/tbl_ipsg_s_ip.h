
#ifndef __TBL_IPSG_S_IP_H__
#define __TBL_IPSG_S_IP_H__

int32
tbl_ipsg_s_ip_add_ipsg_s_ip_sync(tbl_ipsg_s_ip_t *p_ipsg_s_ip, uint32 sync);

int32
tbl_ipsg_s_ip_del_ipsg_s_ip_sync(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key, uint32 sync);

int32
tbl_ipsg_s_ip_set_ipsg_s_ip_field_sync(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_ipsg_s_ip_field_id_t field_id, uint32 sync);

int32
tbl_ipsg_s_ip_add_ipsg_s_ip(tbl_ipsg_s_ip_t *p_ipsg_s_ip);

int32
tbl_ipsg_s_ip_del_ipsg_s_ip(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key);

int32
tbl_ipsg_s_ip_set_ipsg_s_ip_field(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_ipsg_s_ip_field_id_t field_id);

tbl_ipsg_s_ip_t*
tbl_ipsg_s_ip_get_ipsg_s_ip(tbl_ipsg_s_ip_key_t *p_ipsg_s_ip_key);

char*
tbl_ipsg_s_ip_key_val2str(tbl_ipsg_s_ip_t *p_ipsg_s_ip, char *str, uint32 str_len);

int32
tbl_ipsg_s_ip_key_str2val(char *str, tbl_ipsg_s_ip_t *p_ipsg_s_ip);

int32
tbl_ipsg_s_ip_dump_one(tbl_ipsg_s_ip_t *p_ipsg_s_ip, tbl_iter_args_t *pargs);

char*
tbl_ipsg_s_ip_key_name_dump(tbl_ipsg_s_ip_t *p_ipsg_s_ip, char* str);

char*
tbl_ipsg_s_ip_key_value_dump(tbl_ipsg_s_ip_t *p_ipsg_s_ip, char* str);

char*
tbl_ipsg_s_ip_field_name_dump(tbl_ipsg_s_ip_t *p_ipsg_s_ip, int32 field_id, char* str);

char*
tbl_ipsg_s_ip_field_value_dump(tbl_ipsg_s_ip_t *p_ipsg_s_ip, int32 field_id, char* str);

char**
tbl_ipsg_s_ip_table_dump(tbl_ipsg_s_ip_t *p_ipsg_s_ip, char** str);

int32
tbl_ipsg_s_ip_table_parser(char** array, char* key_value, tbl_ipsg_s_ip_t *p_ipsg_s_ip);

int32
tbl_ipsg_s_ip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipsg_s_ip_master_t*
tbl_ipsg_s_ip_get_master();

tbl_ipsg_s_ip_master_t*
tbl_ipsg_s_ip_init_ipsg_s_ip();


tbl_ipsg_s_ip_t*
tbl_ipsg_s_ip_get_ipsg_s_ip_by_ipaddr(prefix_t *preifx);


#endif /* !__TBL_IPSG_S_IP_H__ */

