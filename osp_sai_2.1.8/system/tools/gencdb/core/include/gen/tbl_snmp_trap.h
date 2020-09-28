
#ifndef __TBL_SNMP_TRAP_H__
#define __TBL_SNMP_TRAP_H__

int32
tbl_snmp_trap_add_snmp_trap_sync(tbl_snmp_trap_t *p_snmp_trap, uint32 sync);

int32
tbl_snmp_trap_del_snmp_trap_sync(tbl_snmp_trap_key_t *p_snmp_trap_key, uint32 sync);

int32
tbl_snmp_trap_set_snmp_trap_field_sync(tbl_snmp_trap_t *p_snmp_trap, tbl_snmp_trap_field_id_t field_id, uint32 sync);

int32
tbl_snmp_trap_add_snmp_trap(tbl_snmp_trap_t *p_snmp_trap);

int32
tbl_snmp_trap_del_snmp_trap(tbl_snmp_trap_key_t *p_snmp_trap_key);

int32
tbl_snmp_trap_set_snmp_trap_field(tbl_snmp_trap_t *p_snmp_trap, tbl_snmp_trap_field_id_t field_id);

tbl_snmp_trap_t*
tbl_snmp_trap_get_snmp_trap(tbl_snmp_trap_key_t *p_snmp_trap_key);

char*
tbl_snmp_trap_key_val2str(tbl_snmp_trap_t *p_snmp_trap, char *str, uint32 str_len);

int32
tbl_snmp_trap_key_str2val(char *str, tbl_snmp_trap_t *p_snmp_trap);

int32
tbl_snmp_trap_dump_one(tbl_snmp_trap_t *p_snmp_trap, tbl_iter_args_t *pargs);

char*
tbl_snmp_trap_key_name_dump(tbl_snmp_trap_t *p_snmp_trap, char* str);

char*
tbl_snmp_trap_key_value_dump(tbl_snmp_trap_t *p_snmp_trap, char* str);

char*
tbl_snmp_trap_field_name_dump(tbl_snmp_trap_t *p_snmp_trap, int32 field_id, char* str);

char*
tbl_snmp_trap_field_value_dump(tbl_snmp_trap_t *p_snmp_trap, int32 field_id, char* str);

char**
tbl_snmp_trap_table_dump(tbl_snmp_trap_t *p_snmp_trap, char** str);

int32
tbl_snmp_trap_table_parser(char** array, char* key_value, tbl_snmp_trap_t *p_snmp_trap);

int32
tbl_snmp_trap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_trap_master_t*
tbl_snmp_trap_get_master();

tbl_snmp_trap_master_t*
tbl_snmp_trap_init_snmp_trap();

#endif /* !__TBL_SNMP_TRAP_H__ */

