
#ifndef __TBL_IPTABLES_PREVENT_H__
#define __TBL_IPTABLES_PREVENT_H__

int32
tbl_iptables_prevent_set_iptables_prevent_field_sync(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iptables_prevent_field_id_t field_id, uint32 sync);

int32
tbl_iptables_prevent_set_iptables_prevent_field(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iptables_prevent_field_id_t field_id);

tbl_iptables_prevent_t*
tbl_iptables_prevent_get_iptables_prevent();

char*
tbl_iptables_prevent_key_val2str(tbl_iptables_prevent_t *p_iptables_prevent, char *str, uint32 str_len);

int32
tbl_iptables_prevent_key_str2val(char *str, tbl_iptables_prevent_t *p_iptables_prevent);

int32
tbl_iptables_prevent_dump_one(tbl_iptables_prevent_t *p_iptables_prevent, tbl_iter_args_t *pargs);

char*
tbl_iptables_prevent_key_name_dump(tbl_iptables_prevent_t *p_iptables_prevent, char* str);

char*
tbl_iptables_prevent_key_value_dump(tbl_iptables_prevent_t *p_iptables_prevent, char* str);

char*
tbl_iptables_prevent_field_name_dump(tbl_iptables_prevent_t *p_iptables_prevent, int32 field_id, char* str);

char*
tbl_iptables_prevent_field_value_dump(tbl_iptables_prevent_t *p_iptables_prevent, int32 field_id, char* str);

char**
tbl_iptables_prevent_table_dump(tbl_iptables_prevent_t *p_iptables_prevent, char** str);

int32
tbl_iptables_prevent_table_parser(char** array, char* key_value, tbl_iptables_prevent_t *p_iptables_prevent);

int32
tbl_iptables_prevent_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_iptables_prevent_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_iptables_prevent_t*
tbl_iptables_prevent_init_iptables_prevent();

#endif /* !__TBL_IPTABLES_PREVENT_H__ */

