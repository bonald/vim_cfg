
#ifndef __TBL_NTP_ACE_H__
#define __TBL_NTP_ACE_H__

int32
tbl_ntp_ace_add_ntp_ace_sync(tbl_ntp_ace_t *p_ntp_ace, uint32 sync);

int32
tbl_ntp_ace_del_ntp_ace_sync(tbl_ntp_ace_key_t *p_ntp_ace_key, uint32 sync);

int32
tbl_ntp_ace_set_ntp_ace_field_sync(tbl_ntp_ace_t *p_ntp_ace, tbl_ntp_ace_field_id_t field_id, uint32 sync);

int32
tbl_ntp_ace_add_ntp_ace(tbl_ntp_ace_t *p_ntp_ace);

int32
tbl_ntp_ace_del_ntp_ace(tbl_ntp_ace_key_t *p_ntp_ace_key);

int32
tbl_ntp_ace_set_ntp_ace_field(tbl_ntp_ace_t *p_ntp_ace, tbl_ntp_ace_field_id_t field_id);

tbl_ntp_ace_t*
tbl_ntp_ace_get_ntp_ace(tbl_ntp_ace_key_t *p_ntp_ace_key);

char*
tbl_ntp_ace_key_val2str(tbl_ntp_ace_t *p_ntp_ace, char *str, uint32 str_len);

int32
tbl_ntp_ace_key_str2val(char *str, tbl_ntp_ace_t *p_ntp_ace);

int32
tbl_ntp_ace_dump_one(tbl_ntp_ace_t *p_ntp_ace, tbl_iter_args_t *pargs);

char*
tbl_ntp_ace_key_name_dump(tbl_ntp_ace_t *p_ntp_ace, char* str);

char*
tbl_ntp_ace_key_value_dump(tbl_ntp_ace_t *p_ntp_ace, char* str);

char*
tbl_ntp_ace_field_name_dump(tbl_ntp_ace_t *p_ntp_ace, int32 field_id, char* str);

char*
tbl_ntp_ace_field_value_dump(tbl_ntp_ace_t *p_ntp_ace, int32 field_id, char* str);

char**
tbl_ntp_ace_table_dump(tbl_ntp_ace_t *p_ntp_ace, char** str);

int32
tbl_ntp_ace_table_parser(char** array, char* key_value, tbl_ntp_ace_t *p_ntp_ace);

int32
tbl_ntp_ace_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ntp_ace_master_t*
tbl_ntp_ace_get_master();

tbl_ntp_ace_master_t*
tbl_ntp_ace_init_ntp_ace();

#endif /* !__TBL_NTP_ACE_H__ */

