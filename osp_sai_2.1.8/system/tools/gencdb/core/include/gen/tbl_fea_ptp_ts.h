
#ifndef __TBL_FEA_PTP_TS_H__
#define __TBL_FEA_PTP_TS_H__

int32
tbl_fea_ptp_ts_add_fea_ptp_ts_sync(tbl_fea_ptp_ts_t *p_fea_ptp_ts, uint32 sync);

int32
tbl_fea_ptp_ts_del_fea_ptp_ts_sync(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key, uint32 sync);

int32
tbl_fea_ptp_ts_set_fea_ptp_ts_field_sync(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_fea_ptp_ts_field_id_t field_id, uint32 sync);

int32
tbl_fea_ptp_ts_add_fea_ptp_ts(tbl_fea_ptp_ts_t *p_fea_ptp_ts);

int32
tbl_fea_ptp_ts_del_fea_ptp_ts(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key);

int32
tbl_fea_ptp_ts_set_fea_ptp_ts_field(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_fea_ptp_ts_field_id_t field_id);

tbl_fea_ptp_ts_t*
tbl_fea_ptp_ts_get_fea_ptp_ts(tbl_fea_ptp_ts_key_t *p_fea_ptp_ts_key);

char*
tbl_fea_ptp_ts_key_val2str(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char *str, uint32 str_len);

int32
tbl_fea_ptp_ts_key_str2val(char *str, tbl_fea_ptp_ts_t *p_fea_ptp_ts);

int32
tbl_fea_ptp_ts_dump_one(tbl_fea_ptp_ts_t *p_fea_ptp_ts, tbl_iter_args_t *pargs);

char*
tbl_fea_ptp_ts_key_name_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char* str);

char*
tbl_fea_ptp_ts_key_value_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char* str);

char*
tbl_fea_ptp_ts_field_name_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, int32 field_id, char* str);

char*
tbl_fea_ptp_ts_field_value_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, int32 field_id, char* str);

char**
tbl_fea_ptp_ts_table_dump(tbl_fea_ptp_ts_t *p_fea_ptp_ts, char** str);

int32
tbl_fea_ptp_ts_table_parser(char** array, char* key_value, tbl_fea_ptp_ts_t *p_fea_ptp_ts);

int32
tbl_fea_ptp_ts_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_ptp_ts_master_t*
tbl_fea_ptp_ts_get_master();

tbl_fea_ptp_ts_master_t*
tbl_fea_ptp_ts_init_fea_ptp_ts();

#endif /* !__TBL_FEA_PTP_TS_H__ */

