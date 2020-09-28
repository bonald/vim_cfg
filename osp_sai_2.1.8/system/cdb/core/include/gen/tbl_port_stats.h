
#ifndef __TBL_PORT_STATS_H__
#define __TBL_PORT_STATS_H__

int32
tbl_port_stats_add_port_stats_sync(tbl_port_stats_t *p_port_stats, uint32 sync);

int32
tbl_port_stats_del_port_stats_sync(tbl_port_stats_key_t *p_port_stats_key, uint32 sync);

int32
tbl_port_stats_set_port_stats_field_sync(tbl_port_stats_t *p_port_stats, tbl_port_stats_field_id_t field_id, uint32 sync);

int32
tbl_port_stats_add_port_stats(tbl_port_stats_t *p_port_stats);

int32
tbl_port_stats_del_port_stats(tbl_port_stats_key_t *p_port_stats_key);

int32
tbl_port_stats_set_port_stats_field(tbl_port_stats_t *p_port_stats, tbl_port_stats_field_id_t field_id);

tbl_port_stats_t*
tbl_port_stats_get_port_stats(tbl_port_stats_key_t *p_port_stats_key);

char*
tbl_port_stats_key_val2str(tbl_port_stats_t *p_port_stats, char *str, uint32 str_len);

int32
tbl_port_stats_key_str2val(char *str, tbl_port_stats_t *p_port_stats);

int32
tbl_port_stats_dump_one(tbl_port_stats_t *p_port_stats, tbl_iter_args_t *pargs);

char*
tbl_port_stats_key_name_dump(tbl_port_stats_t *p_port_stats, char* str);

char*
tbl_port_stats_key_value_dump(tbl_port_stats_t *p_port_stats, char* str);

char*
tbl_port_stats_field_name_dump(tbl_port_stats_t *p_port_stats, int32 field_id, char* str);

char*
tbl_port_stats_field_value_dump(tbl_port_stats_t *p_port_stats, int32 field_id, char* str);

char**
tbl_port_stats_table_dump(tbl_port_stats_t *p_port_stats, char** str);

int32
tbl_port_stats_table_parser(char** array, char* key_value, tbl_port_stats_t *p_port_stats);

int32
tbl_port_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_port_stats_master_t*
tbl_port_stats_get_master();

tbl_port_stats_master_t*
tbl_port_stats_init_port_stats();

#endif /* !__TBL_PORT_STATS_H__ */

