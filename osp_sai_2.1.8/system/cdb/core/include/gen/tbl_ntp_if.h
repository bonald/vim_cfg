
#ifndef __TBL_NTP_IF_H__
#define __TBL_NTP_IF_H__

int32
tbl_ntp_if_set_ntp_if_field_sync(tbl_ntp_if_t *p_ntp_if, tbl_ntp_if_field_id_t field_id, uint32 sync);

int32
tbl_ntp_if_set_ntp_if_field(tbl_ntp_if_t *p_ntp_if, tbl_ntp_if_field_id_t field_id);

tbl_ntp_if_t*
tbl_ntp_if_get_ntp_if();

char*
tbl_ntp_if_key_val2str(tbl_ntp_if_t *p_ntp_if, char *str, uint32 str_len);

int32
tbl_ntp_if_key_str2val(char *str, tbl_ntp_if_t *p_ntp_if);

int32
tbl_ntp_if_dump_one(tbl_ntp_if_t *p_ntp_if, tbl_iter_args_t *pargs);

char*
tbl_ntp_if_key_name_dump(tbl_ntp_if_t *p_ntp_if, char* str);

char*
tbl_ntp_if_key_value_dump(tbl_ntp_if_t *p_ntp_if, char* str);

char*
tbl_ntp_if_field_name_dump(tbl_ntp_if_t *p_ntp_if, int32 field_id, char* str);

char*
tbl_ntp_if_field_value_dump(tbl_ntp_if_t *p_ntp_if, int32 field_id, char* str);

char**
tbl_ntp_if_table_dump(tbl_ntp_if_t *p_ntp_if, char** str);

int32
tbl_ntp_if_table_parser(char** array, char* key_value, tbl_ntp_if_t *p_ntp_if);

int32
tbl_ntp_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ntp_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ntp_if_t*
tbl_ntp_if_init_ntp_if();

#endif /* !__TBL_NTP_IF_H__ */

