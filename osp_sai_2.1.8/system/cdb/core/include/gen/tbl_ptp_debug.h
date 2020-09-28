
#ifndef __TBL_PTP_DEBUG_H__
#define __TBL_PTP_DEBUG_H__

int32
tbl_ptp_debug_set_ptp_debug_field_sync(tbl_ptp_debug_t *p_ptpdbg, tbl_ptp_debug_field_id_t field_id, uint32 sync);

int32
tbl_ptp_debug_set_ptp_debug_field(tbl_ptp_debug_t *p_ptpdbg, tbl_ptp_debug_field_id_t field_id);

tbl_ptp_debug_t*
tbl_ptp_debug_get_ptp_debug();

char*
tbl_ptp_debug_key_val2str(tbl_ptp_debug_t *p_ptpdbg, char *str, uint32 str_len);

int32
tbl_ptp_debug_key_str2val(char *str, tbl_ptp_debug_t *p_ptpdbg);

int32
tbl_ptp_debug_dump_one(tbl_ptp_debug_t *p_ptpdbg, tbl_iter_args_t *pargs);

char*
tbl_ptp_debug_key_name_dump(tbl_ptp_debug_t *p_ptpdbg, char* str);

char*
tbl_ptp_debug_key_value_dump(tbl_ptp_debug_t *p_ptpdbg, char* str);

char*
tbl_ptp_debug_field_name_dump(tbl_ptp_debug_t *p_ptpdbg, int32 field_id, char* str);

char*
tbl_ptp_debug_field_value_dump(tbl_ptp_debug_t *p_ptpdbg, int32 field_id, char* str);

char**
tbl_ptp_debug_table_dump(tbl_ptp_debug_t *p_ptpdbg, char** str);

int32
tbl_ptp_debug_table_parser(char** array, char* key_value, tbl_ptp_debug_t *p_ptpdbg);

int32
tbl_ptp_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ptp_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ptp_debug_t*
tbl_ptp_debug_init_ptp_debug();

#endif /* !__TBL_PTP_DEBUG_H__ */

