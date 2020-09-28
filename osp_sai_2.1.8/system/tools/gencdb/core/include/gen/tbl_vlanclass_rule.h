
#ifndef __TBL_VLANCLASS_RULE_H__
#define __TBL_VLANCLASS_RULE_H__

int32
tbl_vlanclass_rule_add_vlanclass_rule_sync(tbl_vlanclass_rule_t *p_vlanclass_rule, uint32 sync);

int32
tbl_vlanclass_rule_del_vlanclass_rule_sync(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key, uint32 sync);

int32
tbl_vlanclass_rule_set_vlanclass_rule_field_sync(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_vlanclass_rule_field_id_t field_id, uint32 sync);

int32
tbl_vlanclass_rule_add_vlanclass_rule(tbl_vlanclass_rule_t *p_vlanclass_rule);

int32
tbl_vlanclass_rule_del_vlanclass_rule(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key);

int32
tbl_vlanclass_rule_set_vlanclass_rule_field(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_vlanclass_rule_field_id_t field_id);

tbl_vlanclass_rule_t*
tbl_vlanclass_rule_get_vlanclass_rule(tbl_vlanclass_rule_key_t *p_vlanclass_rule_key);

char*
tbl_vlanclass_rule_key_val2str(tbl_vlanclass_rule_t *p_vlanclass_rule, char *str, uint32 str_len);

int32
tbl_vlanclass_rule_key_str2val(char *str, tbl_vlanclass_rule_t *p_vlanclass_rule);

int32
tbl_vlanclass_rule_dump_one(tbl_vlanclass_rule_t *p_vlanclass_rule, tbl_iter_args_t *pargs);

char*
tbl_vlanclass_rule_key_name_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char* str);

char*
tbl_vlanclass_rule_key_value_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char* str);

char*
tbl_vlanclass_rule_field_name_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, int32 field_id, char* str);

char*
tbl_vlanclass_rule_field_value_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, int32 field_id, char* str);

char**
tbl_vlanclass_rule_table_dump(tbl_vlanclass_rule_t *p_vlanclass_rule, char** str);

int32
tbl_vlanclass_rule_table_parser(char** array, char* key_value, tbl_vlanclass_rule_t *p_vlanclass_rule);

int32
tbl_vlanclass_rule_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vlanclass_rule_master_t*
tbl_vlanclass_rule_get_master();

tbl_vlanclass_rule_master_t*
tbl_vlanclass_rule_init_vlanclass_rule();

#endif /* !__TBL_VLANCLASS_RULE_H__ */

