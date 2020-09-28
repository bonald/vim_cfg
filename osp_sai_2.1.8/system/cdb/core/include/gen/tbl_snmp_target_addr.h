
#ifndef __TBL_SNMP_TARGET_ADDR_H__
#define __TBL_SNMP_TARGET_ADDR_H__

int32
tbl_snmp_target_addr_add_snmp_target_addr_sync(tbl_snmp_target_addr_t *p_snmp_target_addr, uint32 sync);

int32
tbl_snmp_target_addr_del_snmp_target_addr_sync(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key, uint32 sync);

int32
tbl_snmp_target_addr_set_snmp_target_addr_field_sync(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_snmp_target_addr_field_id_t field_id, uint32 sync);

int32
tbl_snmp_target_addr_add_snmp_target_addr(tbl_snmp_target_addr_t *p_snmp_target_addr);

int32
tbl_snmp_target_addr_del_snmp_target_addr(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key);

int32
tbl_snmp_target_addr_set_snmp_target_addr_field(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_snmp_target_addr_field_id_t field_id);

tbl_snmp_target_addr_t*
tbl_snmp_target_addr_get_snmp_target_addr(tbl_snmp_target_addr_key_t *p_snmp_target_addr_key);

char*
tbl_snmp_target_addr_key_val2str(tbl_snmp_target_addr_t *p_snmp_target_addr, char *str, uint32 str_len);

int32
tbl_snmp_target_addr_key_str2val(char *str, tbl_snmp_target_addr_t *p_snmp_target_addr);

int32
tbl_snmp_target_addr_dump_one(tbl_snmp_target_addr_t *p_snmp_target_addr, tbl_iter_args_t *pargs);

char*
tbl_snmp_target_addr_key_name_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char* str);

char*
tbl_snmp_target_addr_key_value_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char* str);

char*
tbl_snmp_target_addr_field_name_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, int32 field_id, char* str);

char*
tbl_snmp_target_addr_field_value_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, int32 field_id, char* str);

char**
tbl_snmp_target_addr_table_dump(tbl_snmp_target_addr_t *p_snmp_target_addr, char** str);

int32
tbl_snmp_target_addr_table_parser(char** array, char* key_value, tbl_snmp_target_addr_t *p_snmp_target_addr);

int32
tbl_snmp_target_addr_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_target_addr_master_t*
tbl_snmp_target_addr_get_master();

tbl_snmp_target_addr_master_t*
tbl_snmp_target_addr_init_snmp_target_addr();

int32
tbl_snmp_target_addr_num_get();

tbl_snmp_target_addr_t *
tbl_snmp_target_addr_get_by_param_name(char *param_name);



#endif /* !__TBL_SNMP_TARGET_ADDR_H__ */

