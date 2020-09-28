
#ifndef __TBL_IPSLA_TEST_STATS_H__
#define __TBL_IPSLA_TEST_STATS_H__

int32
tbl_ipsla_test_stats_add_ipsla_test_stats_sync(tbl_ipsla_test_stats_t *p_ipsla_test_stats, uint32 sync);

int32
tbl_ipsla_test_stats_del_ipsla_test_stats_sync(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key, uint32 sync);

int32
tbl_ipsla_test_stats_set_ipsla_test_stats_field_sync(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_ipsla_test_stats_field_id_t field_id, uint32 sync);

int32
tbl_ipsla_test_stats_add_ipsla_test_stats(tbl_ipsla_test_stats_t *p_ipsla_test_stats);

int32
tbl_ipsla_test_stats_del_ipsla_test_stats(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key);

int32
tbl_ipsla_test_stats_set_ipsla_test_stats_field(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_ipsla_test_stats_field_id_t field_id);

tbl_ipsla_test_stats_t*
tbl_ipsla_test_stats_get_ipsla_test_stats(tbl_ipsla_test_stats_key_t *p_ipsla_test_stats_key);

char*
tbl_ipsla_test_stats_key_val2str(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char *str, uint32 str_len);

int32
tbl_ipsla_test_stats_key_str2val(char *str, tbl_ipsla_test_stats_t *p_ipsla_test_stats);

int32
tbl_ipsla_test_stats_dump_one(tbl_ipsla_test_stats_t *p_ipsla_test_stats, tbl_iter_args_t *pargs);

char*
tbl_ipsla_test_stats_key_name_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char* str);

char*
tbl_ipsla_test_stats_key_value_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char* str);

char*
tbl_ipsla_test_stats_field_name_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, int32 field_id, char* str);

char*
tbl_ipsla_test_stats_field_value_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, int32 field_id, char* str);

char**
tbl_ipsla_test_stats_table_dump(tbl_ipsla_test_stats_t *p_ipsla_test_stats, char** str);

int32
tbl_ipsla_test_stats_table_parser(char** array, char* key_value, tbl_ipsla_test_stats_t *p_ipsla_test_stats);

int32
tbl_ipsla_test_stats_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipsla_test_stats_master_t*
tbl_ipsla_test_stats_get_master();

tbl_ipsla_test_stats_master_t*
tbl_ipsla_test_stats_init_ipsla_test_stats();

#endif /* !__TBL_IPSLA_TEST_STATS_H__ */

