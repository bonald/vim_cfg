
#ifndef __TBL_SNMP_INFORM_H__
#define __TBL_SNMP_INFORM_H__

int32
tbl_snmp_inform_add_snmp_inform_sync(tbl_snmp_inform_t *p_snmp_inform, uint32 sync);

int32
tbl_snmp_inform_del_snmp_inform_sync(tbl_snmp_inform_key_t *p_snmp_inform_key, uint32 sync);

int32
tbl_snmp_inform_set_snmp_inform_field_sync(tbl_snmp_inform_t *p_snmp_inform, tbl_snmp_inform_field_id_t field_id, uint32 sync);

int32
tbl_snmp_inform_add_snmp_inform(tbl_snmp_inform_t *p_snmp_inform);

int32
tbl_snmp_inform_del_snmp_inform(tbl_snmp_inform_key_t *p_snmp_inform_key);

int32
tbl_snmp_inform_set_snmp_inform_field(tbl_snmp_inform_t *p_snmp_inform, tbl_snmp_inform_field_id_t field_id);

tbl_snmp_inform_t*
tbl_snmp_inform_get_snmp_inform(tbl_snmp_inform_key_t *p_snmp_inform_key);

char*
tbl_snmp_inform_key_val2str(tbl_snmp_inform_t *p_snmp_inform, char *str, uint32 str_len);

int32
tbl_snmp_inform_key_str2val(char *str, tbl_snmp_inform_t *p_snmp_inform);

int32
tbl_snmp_inform_dump_one(tbl_snmp_inform_t *p_snmp_inform, tbl_iter_args_t *pargs);

char*
tbl_snmp_inform_key_name_dump(tbl_snmp_inform_t *p_snmp_inform, char* str);

char*
tbl_snmp_inform_key_value_dump(tbl_snmp_inform_t *p_snmp_inform, char* str);

char*
tbl_snmp_inform_field_name_dump(tbl_snmp_inform_t *p_snmp_inform, int32 field_id, char* str);

char*
tbl_snmp_inform_field_value_dump(tbl_snmp_inform_t *p_snmp_inform, int32 field_id, char* str);

char**
tbl_snmp_inform_table_dump(tbl_snmp_inform_t *p_snmp_inform, char** str);

int32
tbl_snmp_inform_table_parser(char** array, char* key_value, tbl_snmp_inform_t *p_snmp_inform);

int32
tbl_snmp_inform_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_inform_master_t*
tbl_snmp_inform_get_master();

tbl_snmp_inform_master_t*
tbl_snmp_inform_init_snmp_inform();

#endif /* !__TBL_SNMP_INFORM_H__ */

