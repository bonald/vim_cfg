
#ifndef __TBL_PORT_STATS_RATE_H__
#define __TBL_PORT_STATS_RATE_H__

int32
tbl_port_stats_rate_add_port_stats_rate_sync(tbl_port_stats_rate_t *p_port_stats_rate, uint32 sync);

int32
tbl_port_stats_rate_del_port_stats_rate_sync(tbl_port_stats_rate_key_t *p_port_stats_rate_key, uint32 sync);

int32
tbl_port_stats_rate_set_port_stats_rate_field_sync(tbl_port_stats_rate_t *p_port_stats_rate, tbl_port_stats_rate_field_id_t field_id, uint32 sync);

int32
tbl_port_stats_rate_add_port_stats_rate(tbl_port_stats_rate_t *p_port_stats_rate);

int32
tbl_port_stats_rate_del_port_stats_rate(tbl_port_stats_rate_key_t *p_port_stats_rate_key);

int32
tbl_port_stats_rate_set_port_stats_rate_field(tbl_port_stats_rate_t *p_port_stats_rate, tbl_port_stats_rate_field_id_t field_id);

tbl_port_stats_rate_t*
tbl_port_stats_rate_get_port_stats_rate(tbl_port_stats_rate_key_t *p_port_stats_rate_key);

char*
tbl_port_stats_rate_key_val2str(tbl_port_stats_rate_t *p_port_stats_rate, char *str, uint32 str_len);

int32
tbl_port_stats_rate_key_str2val(char *str, tbl_port_stats_rate_t *p_port_stats_rate);

int32
tbl_port_stats_rate_dump_one(tbl_port_stats_rate_t *p_port_stats_rate, tbl_iter_args_t *pargs);

char*
tbl_port_stats_rate_key_name_dump(tbl_port_stats_rate_t *p_port_stats_rate, char* str);

char*
tbl_port_stats_rate_key_value_dump(tbl_port_stats_rate_t *p_port_stats_rate, char* str);

char*
tbl_port_stats_rate_field_name_dump(tbl_port_stats_rate_t *p_port_stats_rate, int32 field_id, char* str);

char*
tbl_port_stats_rate_field_value_dump(tbl_port_stats_rate_t *p_port_stats_rate, int32 field_id, char* str);

char**
tbl_port_stats_rate_table_dump(tbl_port_stats_rate_t *p_port_stats_rate, char** str);

int32
tbl_port_stats_rate_table_parser(char** array, char* key_value, tbl_port_stats_rate_t *p_port_stats_rate);

int32
tbl_port_stats_rate_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_port_stats_rate_master_t*
tbl_port_stats_rate_get_master();

tbl_port_stats_rate_master_t*
tbl_port_stats_rate_init_port_stats_rate();

#endif /* !__TBL_PORT_STATS_RATE_H__ */

