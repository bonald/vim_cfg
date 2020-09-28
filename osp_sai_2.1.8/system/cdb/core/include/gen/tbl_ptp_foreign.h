
#ifndef __TBL_PTP_FOREIGN_H__
#define __TBL_PTP_FOREIGN_H__

int32
tbl_ptp_foreign_add_ptp_foreign_sync(tbl_ptp_foreign_t *p_foreign, uint32 sync);

int32
tbl_ptp_foreign_del_ptp_foreign_sync(tbl_ptp_foreign_key_t *p_foreign_key, uint32 sync);

int32
tbl_ptp_foreign_set_ptp_foreign_field_sync(tbl_ptp_foreign_t *p_foreign, tbl_ptp_foreign_field_id_t field_id, uint32 sync);

int32
tbl_ptp_foreign_add_ptp_foreign(tbl_ptp_foreign_t *p_foreign);

int32
tbl_ptp_foreign_del_ptp_foreign(tbl_ptp_foreign_key_t *p_foreign_key);

int32
tbl_ptp_foreign_set_ptp_foreign_field(tbl_ptp_foreign_t *p_foreign, tbl_ptp_foreign_field_id_t field_id);

tbl_ptp_foreign_t*
tbl_ptp_foreign_get_ptp_foreign(tbl_ptp_foreign_key_t *p_foreign_key);

char*
tbl_ptp_foreign_key_val2str(tbl_ptp_foreign_t *p_foreign, char *str, uint32 str_len);

int32
tbl_ptp_foreign_key_str2val(char *str, tbl_ptp_foreign_t *p_foreign);

int32
tbl_ptp_foreign_dump_one(tbl_ptp_foreign_t *p_foreign, tbl_iter_args_t *pargs);

char*
tbl_ptp_foreign_key_name_dump(tbl_ptp_foreign_t *p_foreign, char* str);

char*
tbl_ptp_foreign_key_value_dump(tbl_ptp_foreign_t *p_foreign, char* str);

char*
tbl_ptp_foreign_field_name_dump(tbl_ptp_foreign_t *p_foreign, int32 field_id, char* str);

char*
tbl_ptp_foreign_field_value_dump(tbl_ptp_foreign_t *p_foreign, int32 field_id, char* str);

char**
tbl_ptp_foreign_table_dump(tbl_ptp_foreign_t *p_foreign, char** str);

int32
tbl_ptp_foreign_table_parser(char** array, char* key_value, tbl_ptp_foreign_t *p_foreign);

int32
tbl_ptp_foreign_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ptp_foreign_master_t*
tbl_ptp_foreign_get_master();

tbl_ptp_foreign_master_t*
tbl_ptp_foreign_init_ptp_foreign();

#endif /* !__TBL_PTP_FOREIGN_H__ */

