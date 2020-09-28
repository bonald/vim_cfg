
#ifndef __TBL_PTP_GLOBAL_H__
#define __TBL_PTP_GLOBAL_H__

int32
tbl_ptp_global_set_ptp_global_field_sync(tbl_ptp_global_t *p_global, tbl_ptp_global_field_id_t field_id, uint32 sync);

int32
tbl_ptp_global_set_ptp_global_field(tbl_ptp_global_t *p_global, tbl_ptp_global_field_id_t field_id);

tbl_ptp_global_t*
tbl_ptp_global_get_ptp_global();

char*
tbl_ptp_global_key_val2str(tbl_ptp_global_t *p_global, char *str, uint32 str_len);

int32
tbl_ptp_global_key_str2val(char *str, tbl_ptp_global_t *p_global);

int32
tbl_ptp_global_dump_one(tbl_ptp_global_t *p_global, tbl_iter_args_t *pargs);

char*
tbl_ptp_global_key_name_dump(tbl_ptp_global_t *p_global, char* str);

char*
tbl_ptp_global_key_value_dump(tbl_ptp_global_t *p_global, char* str);

char*
tbl_ptp_global_field_name_dump(tbl_ptp_global_t *p_global, int32 field_id, char* str);

char*
tbl_ptp_global_field_value_dump(tbl_ptp_global_t *p_global, int32 field_id, char* str);

char**
tbl_ptp_global_table_dump(tbl_ptp_global_t *p_global, char** str);

int32
tbl_ptp_global_table_parser(char** array, char* key_value, tbl_ptp_global_t *p_global);

int32
tbl_ptp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ptp_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ptp_global_t*
tbl_ptp_global_init_ptp_global();

#endif /* !__TBL_PTP_GLOBAL_H__ */

