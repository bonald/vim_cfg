
#ifndef __TBL_SNMP_VIEW_H__
#define __TBL_SNMP_VIEW_H__

int32
tbl_snmp_view_add_snmp_view_sync(tbl_snmp_view_t *p_snmp_view, uint32 sync);

int32
tbl_snmp_view_del_snmp_view_sync(tbl_snmp_view_key_t *p_snmp_view_key, uint32 sync);

int32
tbl_snmp_view_set_snmp_view_field_sync(tbl_snmp_view_t *p_snmp_view, tbl_snmp_view_field_id_t field_id, uint32 sync);

int32
tbl_snmp_view_add_snmp_view(tbl_snmp_view_t *p_snmp_view);

int32
tbl_snmp_view_del_snmp_view(tbl_snmp_view_key_t *p_snmp_view_key);

int32
tbl_snmp_view_set_snmp_view_field(tbl_snmp_view_t *p_snmp_view, tbl_snmp_view_field_id_t field_id);

tbl_snmp_view_t*
tbl_snmp_view_get_snmp_view(tbl_snmp_view_key_t *p_snmp_view_key);

char*
tbl_snmp_view_key_val2str(tbl_snmp_view_t *p_snmp_view, char *str, uint32 str_len);

int32
tbl_snmp_view_key_str2val(char *str, tbl_snmp_view_t *p_snmp_view);

int32
tbl_snmp_view_dump_one(tbl_snmp_view_t *p_snmp_view, tbl_iter_args_t *pargs);

char*
tbl_snmp_view_key_name_dump(tbl_snmp_view_t *p_snmp_view, char* str);

char*
tbl_snmp_view_key_value_dump(tbl_snmp_view_t *p_snmp_view, char* str);

char*
tbl_snmp_view_field_name_dump(tbl_snmp_view_t *p_snmp_view, int32 field_id, char* str);

char*
tbl_snmp_view_field_value_dump(tbl_snmp_view_t *p_snmp_view, int32 field_id, char* str);

char**
tbl_snmp_view_table_dump(tbl_snmp_view_t *p_snmp_view, char** str);

int32
tbl_snmp_view_table_parser(char** array, char* key_value, tbl_snmp_view_t *p_snmp_view);

int32
tbl_snmp_view_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_view_master_t*
tbl_snmp_view_get_master();

tbl_snmp_view_master_t*
tbl_snmp_view_init_snmp_view();

int32
tbl_snmp_view_num_get_by_view_name(char *view_name);

int32
tbl_snmp_view_refcnt_add_by_view_name(char *view_name);

int32
tbl_snmp_view_refcnt_del_by_view_name(char *view_name);



#endif /* !__TBL_SNMP_VIEW_H__ */

