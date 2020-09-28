
#ifndef __TBL_RMON_ETHER_STATS_H__
#define __TBL_RMON_ETHER_STATS_H__

int32
tbl_rmon_ether_stats_add_rmon_ether_stats_sync(tbl_rmon_ether_stats_t *p_rmon_ether_stats, uint32 sync);

int32
tbl_rmon_ether_stats_del_rmon_ether_stats_sync(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key, uint32 sync);

int32
tbl_rmon_ether_stats_set_rmon_ether_stats_field_sync(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_rmon_ether_stats_field_id_t field_id, uint32 sync);

int32
tbl_rmon_ether_stats_add_rmon_ether_stats(tbl_rmon_ether_stats_t *p_rmon_ether_stats);

int32
tbl_rmon_ether_stats_del_rmon_ether_stats(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key);

int32
tbl_rmon_ether_stats_set_rmon_ether_stats_field(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_rmon_ether_stats_field_id_t field_id);

tbl_rmon_ether_stats_t*
tbl_rmon_ether_stats_get_rmon_ether_stats(tbl_rmon_ether_stats_key_t *p_rmon_ether_stats_key);

char*
tbl_rmon_ether_stats_key_val2str(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char *str, uint32 str_len);

int32
tbl_rmon_ether_stats_key_str2val(char *str, tbl_rmon_ether_stats_t *p_rmon_ether_stats);

int32
tbl_rmon_ether_stats_dump_one(tbl_rmon_ether_stats_t *p_rmon_ether_stats, tbl_iter_args_t *pargs);

char*
tbl_rmon_ether_stats_key_name_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char* str);

char*
tbl_rmon_ether_stats_key_value_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char* str);

char*
tbl_rmon_ether_stats_field_name_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, int32 field_id, char* str);

char*
tbl_rmon_ether_stats_field_value_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, int32 field_id, char* str);

char**
tbl_rmon_ether_stats_table_dump(tbl_rmon_ether_stats_t *p_rmon_ether_stats, char** str);

int32
tbl_rmon_ether_stats_table_parser(char** array, char* key_value, tbl_rmon_ether_stats_t *p_rmon_ether_stats);

int32
tbl_rmon_ether_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rmon_ether_stats_master_t*
tbl_rmon_ether_stats_get_master();

tbl_rmon_ether_stats_master_t*
tbl_rmon_ether_stats_init_rmon_ether_stats();

#endif /* !__TBL_RMON_ETHER_STATS_H__ */

