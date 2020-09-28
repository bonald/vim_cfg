
#ifndef __TBL_ND_STATS_H__
#define __TBL_ND_STATS_H__

int32
tbl_nd_stats_set_nd_stats_field_sync(tbl_nd_stats_t *p_nd_stats, tbl_nd_stats_field_id_t field_id, uint32 sync);

int32
tbl_nd_stats_set_nd_stats_field(tbl_nd_stats_t *p_nd_stats, tbl_nd_stats_field_id_t field_id);

tbl_nd_stats_t*
tbl_nd_stats_get_nd_stats();

char*
tbl_nd_stats_key_val2str(tbl_nd_stats_t *p_nd_stats, char *str, uint32 str_len);

int32
tbl_nd_stats_key_str2val(char *str, tbl_nd_stats_t *p_nd_stats);

int32
tbl_nd_stats_dump_one(tbl_nd_stats_t *p_nd_stats, tbl_iter_args_t *pargs);

char*
tbl_nd_stats_key_name_dump(tbl_nd_stats_t *p_nd_stats, char* str);

char*
tbl_nd_stats_key_value_dump(tbl_nd_stats_t *p_nd_stats, char* str);

char*
tbl_nd_stats_field_name_dump(tbl_nd_stats_t *p_nd_stats, int32 field_id, char* str);

char*
tbl_nd_stats_field_value_dump(tbl_nd_stats_t *p_nd_stats, int32 field_id, char* str);

char**
tbl_nd_stats_table_dump(tbl_nd_stats_t *p_nd_stats, char** str);

int32
tbl_nd_stats_table_parser(char** array, char* key_value, tbl_nd_stats_t *p_nd_stats);

int32
tbl_nd_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_nd_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nd_stats_t*
tbl_nd_stats_init_nd_stats();

#endif /* !__TBL_ND_STATS_H__ */

