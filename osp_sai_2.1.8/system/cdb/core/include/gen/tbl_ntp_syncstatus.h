
#ifndef __TBL_NTP_SYNCSTATUS_H__
#define __TBL_NTP_SYNCSTATUS_H__

int32
tbl_ntp_syncstatus_set_ntp_syncstatus_field_sync(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_ntp_syncstatus_field_id_t field_id, uint32 sync);

int32
tbl_ntp_syncstatus_set_ntp_syncstatus_field(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_ntp_syncstatus_field_id_t field_id);

tbl_ntp_syncstatus_t*
tbl_ntp_syncstatus_get_ntp_syncstatus();

char*
tbl_ntp_syncstatus_key_val2str(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char *str, uint32 str_len);

int32
tbl_ntp_syncstatus_key_str2val(char *str, tbl_ntp_syncstatus_t *p_ntp_syncstatus);

int32
tbl_ntp_syncstatus_dump_one(tbl_ntp_syncstatus_t *p_ntp_syncstatus, tbl_iter_args_t *pargs);

char*
tbl_ntp_syncstatus_key_name_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char* str);

char*
tbl_ntp_syncstatus_key_value_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char* str);

char*
tbl_ntp_syncstatus_field_name_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, int32 field_id, char* str);

char*
tbl_ntp_syncstatus_field_value_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, int32 field_id, char* str);

char**
tbl_ntp_syncstatus_table_dump(tbl_ntp_syncstatus_t *p_ntp_syncstatus, char** str);

int32
tbl_ntp_syncstatus_table_parser(char** array, char* key_value, tbl_ntp_syncstatus_t *p_ntp_syncstatus);

int32
tbl_ntp_syncstatus_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ntp_syncstatus_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ntp_syncstatus_t*
tbl_ntp_syncstatus_init_ntp_syncstatus();

#endif /* !__TBL_NTP_SYNCSTATUS_H__ */

