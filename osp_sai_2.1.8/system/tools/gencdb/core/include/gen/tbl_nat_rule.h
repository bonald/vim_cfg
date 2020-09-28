
#ifndef __TBL_NAT_RULE_H__
#define __TBL_NAT_RULE_H__

int32
tbl_nat_rule_add_nat_rule_sync(tbl_nat_rule_t *p_nat_rule, uint32 sync);

int32
tbl_nat_rule_del_nat_rule_sync(tbl_nat_rule_key_t *p_nat_rule_key, uint32 sync);

int32
tbl_nat_rule_set_nat_rule_field_sync(tbl_nat_rule_t *p_nat_rule, tbl_nat_rule_field_id_t field_id, uint32 sync);

int32
tbl_nat_rule_add_nat_rule(tbl_nat_rule_t *p_nat_rule);

int32
tbl_nat_rule_del_nat_rule(tbl_nat_rule_key_t *p_nat_rule_key);

int32
tbl_nat_rule_set_nat_rule_field(tbl_nat_rule_t *p_nat_rule, tbl_nat_rule_field_id_t field_id);

tbl_nat_rule_t*
tbl_nat_rule_get_nat_rule(tbl_nat_rule_key_t *p_nat_rule_key);

char*
tbl_nat_rule_key_val2str(tbl_nat_rule_t *p_nat_rule, char *str, uint32 str_len);

int32
tbl_nat_rule_key_str2val(char *str, tbl_nat_rule_t *p_nat_rule);

int32
tbl_nat_rule_dump_one(tbl_nat_rule_t *p_nat_rule, tbl_iter_args_t *pargs);

char*
tbl_nat_rule_key_name_dump(tbl_nat_rule_t *p_nat_rule, char* str);

char*
tbl_nat_rule_key_value_dump(tbl_nat_rule_t *p_nat_rule, char* str);

char*
tbl_nat_rule_field_name_dump(tbl_nat_rule_t *p_nat_rule, int32 field_id, char* str);

char*
tbl_nat_rule_field_value_dump(tbl_nat_rule_t *p_nat_rule, int32 field_id, char* str);

char**
tbl_nat_rule_table_dump(tbl_nat_rule_t *p_nat_rule, char** str);

int32
tbl_nat_rule_table_parser(char** array, char* key_value, tbl_nat_rule_t *p_nat_rule);

int32
tbl_nat_rule_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_rule_master_t*
tbl_nat_rule_get_master();

tbl_nat_rule_master_t*
tbl_nat_rule_init_nat_rule();

#endif /* !__TBL_NAT_RULE_H__ */

