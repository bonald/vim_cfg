
#ifndef __TBL_SNMP_CONTEXT_H__
#define __TBL_SNMP_CONTEXT_H__

int32
tbl_snmp_context_add_snmp_context_sync(tbl_snmp_context_t *p_snmp_context, uint32 sync);

int32
tbl_snmp_context_del_snmp_context_sync(tbl_snmp_context_key_t *p_snmp_context_key, uint32 sync);

int32
tbl_snmp_context_set_snmp_context_field_sync(tbl_snmp_context_t *p_snmp_context, tbl_snmp_context_field_id_t field_id, uint32 sync);

int32
tbl_snmp_context_add_snmp_context(tbl_snmp_context_t *p_snmp_context);

int32
tbl_snmp_context_del_snmp_context(tbl_snmp_context_key_t *p_snmp_context_key);

int32
tbl_snmp_context_set_snmp_context_field(tbl_snmp_context_t *p_snmp_context, tbl_snmp_context_field_id_t field_id);

tbl_snmp_context_t*
tbl_snmp_context_get_snmp_context(tbl_snmp_context_key_t *p_snmp_context_key);

char*
tbl_snmp_context_key_val2str(tbl_snmp_context_t *p_snmp_context, char *str, uint32 str_len);

int32
tbl_snmp_context_key_str2val(char *str, tbl_snmp_context_t *p_snmp_context);

int32
tbl_snmp_context_dump_one(tbl_snmp_context_t *p_snmp_context, tbl_iter_args_t *pargs);

char*
tbl_snmp_context_key_name_dump(tbl_snmp_context_t *p_snmp_context, char* str);

char*
tbl_snmp_context_key_value_dump(tbl_snmp_context_t *p_snmp_context, char* str);

char*
tbl_snmp_context_field_name_dump(tbl_snmp_context_t *p_snmp_context, int32 field_id, char* str);

char*
tbl_snmp_context_field_value_dump(tbl_snmp_context_t *p_snmp_context, int32 field_id, char* str);

char**
tbl_snmp_context_table_dump(tbl_snmp_context_t *p_snmp_context, char** str);

int32
tbl_snmp_context_table_parser(char** array, char* key_value, tbl_snmp_context_t *p_snmp_context);

int32
tbl_snmp_context_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_context_master_t*
tbl_snmp_context_get_master();

tbl_snmp_context_master_t*
tbl_snmp_context_init_snmp_context();

int32
tbl_snmp_context_num_get();



#endif /* !__TBL_SNMP_CONTEXT_H__ */

