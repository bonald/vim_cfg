
#ifndef __TBL_SNMP_GROUP_USM_H__
#define __TBL_SNMP_GROUP_USM_H__

int32
tbl_snmp_group_usm_add_snmp_group_usm_sync(tbl_snmp_group_usm_t *p_snmp_group_usm, uint32 sync);

int32
tbl_snmp_group_usm_del_snmp_group_usm_sync(tbl_snmp_group_usm_key_t *p_snmp_group_usm_key, uint32 sync);

int32
tbl_snmp_group_usm_set_snmp_group_usm_field_sync(tbl_snmp_group_usm_t *p_snmp_group_usm, tbl_snmp_group_usm_field_id_t field_id, uint32 sync);

int32
tbl_snmp_group_usm_add_snmp_group_usm(tbl_snmp_group_usm_t *p_snmp_group_usm);

int32
tbl_snmp_group_usm_del_snmp_group_usm(tbl_snmp_group_usm_key_t *p_snmp_group_usm_key);

int32
tbl_snmp_group_usm_set_snmp_group_usm_field(tbl_snmp_group_usm_t *p_snmp_group_usm, tbl_snmp_group_usm_field_id_t field_id);

tbl_snmp_group_usm_t*
tbl_snmp_group_usm_get_snmp_group_usm(tbl_snmp_group_usm_key_t *p_snmp_group_usm_key);

char*
tbl_snmp_group_usm_key_val2str(tbl_snmp_group_usm_t *p_snmp_group_usm, char *str, uint32 str_len);

int32
tbl_snmp_group_usm_key_str2val(char *str, tbl_snmp_group_usm_t *p_snmp_group_usm);

int32
tbl_snmp_group_usm_dump_one(tbl_snmp_group_usm_t *p_snmp_group_usm, tbl_iter_args_t *pargs);

char*
tbl_snmp_group_usm_key_name_dump(tbl_snmp_group_usm_t *p_snmp_group_usm, char* str);

char*
tbl_snmp_group_usm_key_value_dump(tbl_snmp_group_usm_t *p_snmp_group_usm, char* str);

char*
tbl_snmp_group_usm_field_name_dump(tbl_snmp_group_usm_t *p_snmp_group_usm, int32 field_id, char* str);

char*
tbl_snmp_group_usm_field_value_dump(tbl_snmp_group_usm_t *p_snmp_group_usm, int32 field_id, char* str);

char**
tbl_snmp_group_usm_table_dump(tbl_snmp_group_usm_t *p_snmp_group_usm, char** str);

int32
tbl_snmp_group_usm_table_parser(char** array, char* key_value, tbl_snmp_group_usm_t *p_snmp_group_usm);

int32
tbl_snmp_group_usm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_group_usm_master_t*
tbl_snmp_group_usm_get_master();

tbl_snmp_group_usm_master_t*
tbl_snmp_group_usm_init_snmp_group_usm();

int32
tbl_snmp_group_usm_num_get();

int32
tbl_snmp_group_usm_security_exist_check(char *sec_name);

int32
tbl_snmp_group_usm_group_exist_check(char *grp_name);

#endif /* !__TBL_SNMP_GROUP_USM_H__ */

