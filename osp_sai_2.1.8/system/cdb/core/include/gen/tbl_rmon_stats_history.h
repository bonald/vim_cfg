
#ifndef __TBL_RMON_STATS_HISTORY_H__
#define __TBL_RMON_STATS_HISTORY_H__

int32
tbl_rmon_stats_history_add_rmon_stats_history_sync(tbl_rmon_stats_history_t *p_rmon_stats_history, uint32 sync);

int32
tbl_rmon_stats_history_del_rmon_stats_history_sync(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key, uint32 sync);

int32
tbl_rmon_stats_history_set_rmon_stats_history_field_sync(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_rmon_stats_history_field_id_t field_id, uint32 sync);

int32
tbl_rmon_stats_history_add_rmon_stats_history(tbl_rmon_stats_history_t *p_rmon_stats_history);

int32
tbl_rmon_stats_history_del_rmon_stats_history(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key);

int32
tbl_rmon_stats_history_set_rmon_stats_history_field(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_rmon_stats_history_field_id_t field_id);

tbl_rmon_stats_history_t*
tbl_rmon_stats_history_get_rmon_stats_history(tbl_rmon_stats_history_key_t *p_rmon_stats_history_key);

char*
tbl_rmon_stats_history_key_val2str(tbl_rmon_stats_history_t *p_rmon_stats_history, char *str, uint32 str_len);

int32
tbl_rmon_stats_history_key_str2val(char *str, tbl_rmon_stats_history_t *p_rmon_stats_history);

int32
tbl_rmon_stats_history_dump_one(tbl_rmon_stats_history_t *p_rmon_stats_history, tbl_iter_args_t *pargs);

char*
tbl_rmon_stats_history_key_name_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char* str);

char*
tbl_rmon_stats_history_key_value_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char* str);

char*
tbl_rmon_stats_history_field_name_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, int32 field_id, char* str);

char*
tbl_rmon_stats_history_field_value_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, int32 field_id, char* str);

char**
tbl_rmon_stats_history_table_dump(tbl_rmon_stats_history_t *p_rmon_stats_history, char** str);

int32
tbl_rmon_stats_history_table_parser(char** array, char* key_value, tbl_rmon_stats_history_t *p_rmon_stats_history);

int32
tbl_rmon_stats_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rmon_stats_history_master_t*
tbl_rmon_stats_history_get_master();

tbl_rmon_stats_history_master_t*
tbl_rmon_stats_history_init_rmon_stats_history();

#endif /* !__TBL_RMON_STATS_HISTORY_H__ */

