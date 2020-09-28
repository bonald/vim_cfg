
#ifndef __TBL_SNMP_TARGET_PARAMS_H__
#define __TBL_SNMP_TARGET_PARAMS_H__

int32
tbl_snmp_target_params_add_snmp_target_params_sync(tbl_snmp_target_params_t *p_snmp_target_params, uint32 sync);

int32
tbl_snmp_target_params_del_snmp_target_params_sync(tbl_snmp_target_params_key_t *p_snmp_target_params_key, uint32 sync);

int32
tbl_snmp_target_params_set_snmp_target_params_field_sync(tbl_snmp_target_params_t *p_snmp_target_params, tbl_snmp_target_params_field_id_t field_id, uint32 sync);

int32
tbl_snmp_target_params_add_snmp_target_params(tbl_snmp_target_params_t *p_snmp_target_params);

int32
tbl_snmp_target_params_del_snmp_target_params(tbl_snmp_target_params_key_t *p_snmp_target_params_key);

int32
tbl_snmp_target_params_set_snmp_target_params_field(tbl_snmp_target_params_t *p_snmp_target_params, tbl_snmp_target_params_field_id_t field_id);

tbl_snmp_target_params_t*
tbl_snmp_target_params_get_snmp_target_params(tbl_snmp_target_params_key_t *p_snmp_target_params_key);

char*
tbl_snmp_target_params_key_val2str(tbl_snmp_target_params_t *p_snmp_target_params, char *str, uint32 str_len);

int32
tbl_snmp_target_params_key_str2val(char *str, tbl_snmp_target_params_t *p_snmp_target_params);

int32
tbl_snmp_target_params_dump_one(tbl_snmp_target_params_t *p_snmp_target_params, tbl_iter_args_t *pargs);

char*
tbl_snmp_target_params_key_name_dump(tbl_snmp_target_params_t *p_snmp_target_params, char* str);

char*
tbl_snmp_target_params_key_value_dump(tbl_snmp_target_params_t *p_snmp_target_params, char* str);

char*
tbl_snmp_target_params_field_name_dump(tbl_snmp_target_params_t *p_snmp_target_params, int32 field_id, char* str);

char*
tbl_snmp_target_params_field_value_dump(tbl_snmp_target_params_t *p_snmp_target_params, int32 field_id, char* str);

char**
tbl_snmp_target_params_table_dump(tbl_snmp_target_params_t *p_snmp_target_params, char** str);

int32
tbl_snmp_target_params_table_parser(char** array, char* key_value, tbl_snmp_target_params_t *p_snmp_target_params);

int32
tbl_snmp_target_params_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_target_params_master_t*
tbl_snmp_target_params_get_master();

tbl_snmp_target_params_master_t*
tbl_snmp_target_params_init_snmp_target_params();

int32
tbl_snmp_target_params_num_get();



#endif /* !__TBL_SNMP_TARGET_PARAMS_H__ */

