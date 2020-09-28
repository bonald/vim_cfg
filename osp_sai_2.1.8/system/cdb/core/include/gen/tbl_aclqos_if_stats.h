
#ifndef __TBL_ACLQOS_IF_STATS_H__
#define __TBL_ACLQOS_IF_STATS_H__

int32
tbl_aclqos_if_stats_add_aclqos_if_stats_sync(tbl_aclqos_if_stats_t *p_aclqos_if_stats, uint32 sync);

int32
tbl_aclqos_if_stats_del_aclqos_if_stats_sync(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key, uint32 sync);

int32
tbl_aclqos_if_stats_set_aclqos_if_stats_field_sync(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_aclqos_if_stats_field_id_t field_id, uint32 sync);

int32
tbl_aclqos_if_stats_add_aclqos_if_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats);

int32
tbl_aclqos_if_stats_del_aclqos_if_stats(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key);

int32
tbl_aclqos_if_stats_set_aclqos_if_stats_field(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_aclqos_if_stats_field_id_t field_id);

tbl_aclqos_if_stats_t*
tbl_aclqos_if_stats_get_aclqos_if_stats(tbl_aclqos_if_stats_key_t *p_aclqos_if_stats_key);

char*
tbl_aclqos_if_stats_key_val2str(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char *str, uint32 str_len);

int32
tbl_aclqos_if_stats_key_str2val(char *str, tbl_aclqos_if_stats_t *p_aclqos_if_stats);

int32
tbl_aclqos_if_stats_dump_one(tbl_aclqos_if_stats_t *p_aclqos_if_stats, tbl_iter_args_t *pargs);

char*
tbl_aclqos_if_stats_key_name_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char* str);

char*
tbl_aclqos_if_stats_key_value_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char* str);

char*
tbl_aclqos_if_stats_field_name_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, int32 field_id, char* str);

char*
tbl_aclqos_if_stats_field_value_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, int32 field_id, char* str);

char**
tbl_aclqos_if_stats_table_dump(tbl_aclqos_if_stats_t *p_aclqos_if_stats, char** str);

int32
tbl_aclqos_if_stats_table_parser(char** array, char* key_value, tbl_aclqos_if_stats_t *p_aclqos_if_stats);

int32
tbl_aclqos_if_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_aclqos_if_stats_master_t*
tbl_aclqos_if_stats_get_master();

tbl_aclqos_if_stats_master_t*
tbl_aclqos_if_stats_init_aclqos_if_stats();

tbl_aclqos_if_stats_t*
tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(const char* ifname);
#endif /* !__TBL_ACLQOS_IF_STATS_H__ */

