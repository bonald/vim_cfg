
#ifndef __TBL_SNMP_NOTIFY_H__
#define __TBL_SNMP_NOTIFY_H__

int32
tbl_snmp_notify_add_snmp_notify_sync(tbl_snmp_notify_t *p_snmp_notify, uint32 sync);

int32
tbl_snmp_notify_del_snmp_notify_sync(tbl_snmp_notify_key_t *p_snmp_notify_key, uint32 sync);

int32
tbl_snmp_notify_set_snmp_notify_field_sync(tbl_snmp_notify_t *p_snmp_notify, tbl_snmp_notify_field_id_t field_id, uint32 sync);

int32
tbl_snmp_notify_add_snmp_notify(tbl_snmp_notify_t *p_snmp_notify);

int32
tbl_snmp_notify_del_snmp_notify(tbl_snmp_notify_key_t *p_snmp_notify_key);

int32
tbl_snmp_notify_set_snmp_notify_field(tbl_snmp_notify_t *p_snmp_notify, tbl_snmp_notify_field_id_t field_id);

tbl_snmp_notify_t*
tbl_snmp_notify_get_snmp_notify(tbl_snmp_notify_key_t *p_snmp_notify_key);

char*
tbl_snmp_notify_key_val2str(tbl_snmp_notify_t *p_snmp_notify, char *str, uint32 str_len);

int32
tbl_snmp_notify_key_str2val(char *str, tbl_snmp_notify_t *p_snmp_notify);

int32
tbl_snmp_notify_dump_one(tbl_snmp_notify_t *p_snmp_notify, tbl_iter_args_t *pargs);

char*
tbl_snmp_notify_key_name_dump(tbl_snmp_notify_t *p_snmp_notify, char* str);

char*
tbl_snmp_notify_key_value_dump(tbl_snmp_notify_t *p_snmp_notify, char* str);

char*
tbl_snmp_notify_field_name_dump(tbl_snmp_notify_t *p_snmp_notify, int32 field_id, char* str);

char*
tbl_snmp_notify_field_value_dump(tbl_snmp_notify_t *p_snmp_notify, int32 field_id, char* str);

char**
tbl_snmp_notify_table_dump(tbl_snmp_notify_t *p_snmp_notify, char** str);

int32
tbl_snmp_notify_table_parser(char** array, char* key_value, tbl_snmp_notify_t *p_snmp_notify);

int32
tbl_snmp_notify_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_notify_master_t*
tbl_snmp_notify_get_master();

tbl_snmp_notify_master_t*
tbl_snmp_notify_init_snmp_notify();

int32
tbl_snmp_notify_num_get();

tbl_snmp_notify_t *
tbl_snmp_notify_get_by_tagname(char *tag_name);



#endif /* !__TBL_SNMP_NOTIFY_H__ */

