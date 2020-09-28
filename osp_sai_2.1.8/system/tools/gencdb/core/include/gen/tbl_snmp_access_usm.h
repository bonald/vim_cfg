
#ifndef __TBL_SNMP_ACCESS_USM_H__
#define __TBL_SNMP_ACCESS_USM_H__

int32
tbl_snmp_access_usm_add_snmp_access_usm_sync(tbl_snmp_access_usm_t *p_snmp_access_usm, uint32 sync);

int32
tbl_snmp_access_usm_del_snmp_access_usm_sync(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key, uint32 sync);

int32
tbl_snmp_access_usm_set_snmp_access_usm_field_sync(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_snmp_access_usm_field_id_t field_id, uint32 sync);

int32
tbl_snmp_access_usm_add_snmp_access_usm(tbl_snmp_access_usm_t *p_snmp_access_usm);

int32
tbl_snmp_access_usm_del_snmp_access_usm(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key);

int32
tbl_snmp_access_usm_set_snmp_access_usm_field(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_snmp_access_usm_field_id_t field_id);

tbl_snmp_access_usm_t*
tbl_snmp_access_usm_get_snmp_access_usm(tbl_snmp_access_usm_key_t *p_snmp_access_usm_key);

char*
tbl_snmp_access_usm_key_val2str(tbl_snmp_access_usm_t *p_snmp_access_usm, char *str, uint32 str_len);

int32
tbl_snmp_access_usm_key_str2val(char *str, tbl_snmp_access_usm_t *p_snmp_access_usm);

int32
tbl_snmp_access_usm_dump_one(tbl_snmp_access_usm_t *p_snmp_access_usm, tbl_iter_args_t *pargs);

char*
tbl_snmp_access_usm_key_name_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char* str);

char*
tbl_snmp_access_usm_key_value_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char* str);

char*
tbl_snmp_access_usm_field_name_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, int32 field_id, char* str);

char*
tbl_snmp_access_usm_field_value_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, int32 field_id, char* str);

char**
tbl_snmp_access_usm_table_dump(tbl_snmp_access_usm_t *p_snmp_access_usm, char** str);

int32
tbl_snmp_access_usm_table_parser(char** array, char* key_value, tbl_snmp_access_usm_t *p_snmp_access_usm);

int32
tbl_snmp_access_usm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_access_usm_master_t*
tbl_snmp_access_usm_get_master();

tbl_snmp_access_usm_master_t*
tbl_snmp_access_usm_init_snmp_access_usm();

int32
tbl_snmp_access_usm_num_get();



#endif /* !__TBL_SNMP_ACCESS_USM_H__ */

