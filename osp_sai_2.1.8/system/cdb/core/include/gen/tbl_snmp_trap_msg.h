
#ifndef __TBL_SNMP_TRAP_MSG_H__
#define __TBL_SNMP_TRAP_MSG_H__

int32
tbl_snmp_trap_msg_set_snmp_trap_msg_field_sync(tbl_snmp_trap_msg_t *p_trap, tbl_snmp_trap_msg_field_id_t field_id, uint32 sync);

int32
tbl_snmp_trap_msg_set_snmp_trap_msg_field(tbl_snmp_trap_msg_t *p_trap, tbl_snmp_trap_msg_field_id_t field_id);

tbl_snmp_trap_msg_t*
tbl_snmp_trap_msg_get_snmp_trap_msg();

char*
tbl_snmp_trap_msg_key_val2str(tbl_snmp_trap_msg_t *p_trap, char *str, uint32 str_len);

int32
tbl_snmp_trap_msg_key_str2val(char *str, tbl_snmp_trap_msg_t *p_trap);

int32
tbl_snmp_trap_msg_dump_one(tbl_snmp_trap_msg_t *p_trap, tbl_iter_args_t *pargs);

char*
tbl_snmp_trap_msg_key_name_dump(tbl_snmp_trap_msg_t *p_trap, char* str);

char*
tbl_snmp_trap_msg_key_value_dump(tbl_snmp_trap_msg_t *p_trap, char* str);

char*
tbl_snmp_trap_msg_field_name_dump(tbl_snmp_trap_msg_t *p_trap, int32 field_id, char* str);

char*
tbl_snmp_trap_msg_field_value_dump(tbl_snmp_trap_msg_t *p_trap, int32 field_id, char* str);

char**
tbl_snmp_trap_msg_table_dump(tbl_snmp_trap_msg_t *p_trap, char** str);

int32
tbl_snmp_trap_msg_table_parser(char** array, char* key_value, tbl_snmp_trap_msg_t *p_trap);

int32
tbl_snmp_trap_msg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_snmp_trap_msg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_trap_msg_t*
tbl_snmp_trap_msg_init_snmp_trap_msg();

#endif /* !__TBL_SNMP_TRAP_MSG_H__ */

