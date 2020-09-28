
#ifndef __TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_H__
#define __TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_H__

int32
tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt_sync(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, uint32 sync);

int32
tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt_sync(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key, uint32 sync);

int32
tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field_sync(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_openflow_tunnel_local_ip_cnt_field_id_t field_id, uint32 sync);

int32
tbl_openflow_tunnel_local_ip_cnt_add_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip);

int32
tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key);

int32
tbl_openflow_tunnel_local_ip_cnt_set_openflow_tunnel_local_ip_cnt_field(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_openflow_tunnel_local_ip_cnt_field_id_t field_id);

tbl_openflow_tunnel_local_ip_cnt_t*
tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(tbl_openflow_tunnel_local_ip_cnt_key_t *p_openflow_tunnel_local_ip_key);

char*
tbl_openflow_tunnel_local_ip_cnt_key_val2str(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char *str, uint32 str_len);

int32
tbl_openflow_tunnel_local_ip_cnt_key_str2val(char *str, tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip);

int32
tbl_openflow_tunnel_local_ip_cnt_dump_one(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, tbl_iter_args_t *pargs);

char*
tbl_openflow_tunnel_local_ip_cnt_key_name_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char* str);

char*
tbl_openflow_tunnel_local_ip_cnt_key_value_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char* str);

char*
tbl_openflow_tunnel_local_ip_cnt_field_name_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, int32 field_id, char* str);

char*
tbl_openflow_tunnel_local_ip_cnt_field_value_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, int32 field_id, char* str);

char**
tbl_openflow_tunnel_local_ip_cnt_table_dump(tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip, char** str);

int32
tbl_openflow_tunnel_local_ip_cnt_table_parser(char** array, char* key_value, tbl_openflow_tunnel_local_ip_cnt_t *p_openflow_tunnel_local_ip);

int32
tbl_openflow_tunnel_local_ip_cnt_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_openflow_tunnel_local_ip_cnt_master_t*
tbl_openflow_tunnel_local_ip_cnt_get_master();

tbl_openflow_tunnel_local_ip_cnt_master_t*
tbl_openflow_tunnel_local_ip_cnt_init_openflow_tunnel_local_ip_cnt();


int32
tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt_count();
#endif /* !__TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_H__ */

