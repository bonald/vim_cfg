
#ifndef __TBL_SNMP_USM_USER_H__
#define __TBL_SNMP_USM_USER_H__

int32
tbl_snmp_usm_user_add_snmp_usm_user_sync(tbl_snmp_usm_user_t *p_snmp_usm_user, uint32 sync);

int32
tbl_snmp_usm_user_del_snmp_usm_user_sync(tbl_snmp_usm_user_key_t *p_snmp_usm_user_key, uint32 sync);

int32
tbl_snmp_usm_user_set_snmp_usm_user_field_sync(tbl_snmp_usm_user_t *p_snmp_usm_user, tbl_snmp_usm_user_field_id_t field_id, uint32 sync);

int32
tbl_snmp_usm_user_add_snmp_usm_user(tbl_snmp_usm_user_t *p_snmp_usm_user);

int32
tbl_snmp_usm_user_del_snmp_usm_user(tbl_snmp_usm_user_key_t *p_snmp_usm_user_key);

int32
tbl_snmp_usm_user_set_snmp_usm_user_field(tbl_snmp_usm_user_t *p_snmp_usm_user, tbl_snmp_usm_user_field_id_t field_id);

tbl_snmp_usm_user_t*
tbl_snmp_usm_user_get_snmp_usm_user(tbl_snmp_usm_user_key_t *p_snmp_usm_user_key);

char*
tbl_snmp_usm_user_key_val2str(tbl_snmp_usm_user_t *p_snmp_usm_user, char *str, uint32 str_len);

int32
tbl_snmp_usm_user_key_str2val(char *str, tbl_snmp_usm_user_t *p_snmp_usm_user);

int32
tbl_snmp_usm_user_dump_one(tbl_snmp_usm_user_t *p_snmp_usm_user, tbl_iter_args_t *pargs);

char*
tbl_snmp_usm_user_key_name_dump(tbl_snmp_usm_user_t *p_snmp_usm_user, char* str);

char*
tbl_snmp_usm_user_key_value_dump(tbl_snmp_usm_user_t *p_snmp_usm_user, char* str);

char*
tbl_snmp_usm_user_field_name_dump(tbl_snmp_usm_user_t *p_snmp_usm_user, int32 field_id, char* str);

char*
tbl_snmp_usm_user_field_value_dump(tbl_snmp_usm_user_t *p_snmp_usm_user, int32 field_id, char* str);

char**
tbl_snmp_usm_user_table_dump(tbl_snmp_usm_user_t *p_snmp_usm_user, char** str);

int32
tbl_snmp_usm_user_table_parser(char** array, char* key_value, tbl_snmp_usm_user_t *p_snmp_usm_user);

int32
tbl_snmp_usm_user_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_usm_user_master_t*
tbl_snmp_usm_user_get_master();

tbl_snmp_usm_user_master_t*
tbl_snmp_usm_user_init_snmp_usm_user();

int32
tbl_snmp_usm_user_num_get();



#endif /* !__TBL_SNMP_USM_USER_H__ */

