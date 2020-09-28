
#ifndef __TBL_NTP_KEY_H__
#define __TBL_NTP_KEY_H__

int32
tbl_ntp_key_add_ntp_key_sync(tbl_ntp_key_t *p_ntp_key, uint32 sync);

int32
tbl_ntp_key_del_ntp_key_sync(tbl_ntp_key_key_t *p_ntp_key_key, uint32 sync);

int32
tbl_ntp_key_set_ntp_key_field_sync(tbl_ntp_key_t *p_ntp_key, tbl_ntp_key_field_id_t field_id, uint32 sync);

int32
tbl_ntp_key_add_ntp_key(tbl_ntp_key_t *p_ntp_key);

int32
tbl_ntp_key_del_ntp_key(tbl_ntp_key_key_t *p_ntp_key_key);

int32
tbl_ntp_key_set_ntp_key_field(tbl_ntp_key_t *p_ntp_key, tbl_ntp_key_field_id_t field_id);

tbl_ntp_key_t*
tbl_ntp_key_get_ntp_key(tbl_ntp_key_key_t *p_ntp_key_key);

char*
tbl_ntp_key_key_val2str(tbl_ntp_key_t *p_ntp_key, char *str, uint32 str_len);

int32
tbl_ntp_key_key_str2val(char *str, tbl_ntp_key_t *p_ntp_key);

int32
tbl_ntp_key_dump_one(tbl_ntp_key_t *p_ntp_key, tbl_iter_args_t *pargs);

char*
tbl_ntp_key_key_name_dump(tbl_ntp_key_t *p_ntp_key, char* str);

char*
tbl_ntp_key_key_value_dump(tbl_ntp_key_t *p_ntp_key, char* str);

char*
tbl_ntp_key_field_name_dump(tbl_ntp_key_t *p_ntp_key, int32 field_id, char* str);

char*
tbl_ntp_key_field_value_dump(tbl_ntp_key_t *p_ntp_key, int32 field_id, char* str);

char**
tbl_ntp_key_table_dump(tbl_ntp_key_t *p_ntp_key, char** str);

int32
tbl_ntp_key_table_parser(char** array, char* key_value, tbl_ntp_key_t *p_ntp_key);

int32
tbl_ntp_key_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ntp_key_master_t*
tbl_ntp_key_get_master();

tbl_ntp_key_master_t*
tbl_ntp_key_init_ntp_key();

#endif /* !__TBL_NTP_KEY_H__ */

