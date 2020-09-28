
#ifndef __TBL_OPENFLOW_INTERFACE_H__
#define __TBL_OPENFLOW_INTERFACE_H__

int32
tbl_openflow_interface_add_openflow_interface_sync(tbl_openflow_interface_t *p_if, uint32 sync);

int32
tbl_openflow_interface_del_openflow_interface_sync(tbl_openflow_interface_key_t *p_if_key, uint32 sync);

int32
tbl_openflow_interface_set_openflow_interface_field_sync(tbl_openflow_interface_t *p_if, tbl_openflow_interface_field_id_t field_id, uint32 sync);

int32
tbl_openflow_interface_add_openflow_interface(tbl_openflow_interface_t *p_if);

int32
tbl_openflow_interface_del_openflow_interface(tbl_openflow_interface_key_t *p_if_key);

int32
tbl_openflow_interface_set_openflow_interface_field(tbl_openflow_interface_t *p_if, tbl_openflow_interface_field_id_t field_id);

tbl_openflow_interface_t*
tbl_openflow_interface_get_openflow_interface(tbl_openflow_interface_key_t *p_if_key);

char*
tbl_openflow_interface_key_val2str(tbl_openflow_interface_t *p_if, char *str, uint32 str_len);

int32
tbl_openflow_interface_key_str2val(char *str, tbl_openflow_interface_t *p_if);

int32
tbl_openflow_interface_dump_one(tbl_openflow_interface_t *p_if, tbl_iter_args_t *pargs);

char*
tbl_openflow_interface_key_name_dump(tbl_openflow_interface_t *p_if, char* str);

char*
tbl_openflow_interface_key_value_dump(tbl_openflow_interface_t *p_if, char* str);

char*
tbl_openflow_interface_field_name_dump(tbl_openflow_interface_t *p_if, int32 field_id, char* str);

char*
tbl_openflow_interface_field_value_dump(tbl_openflow_interface_t *p_if, int32 field_id, char* str);

char**
tbl_openflow_interface_table_dump(tbl_openflow_interface_t *p_if, char** str);

int32
tbl_openflow_interface_table_parser(char** array, char* key_value, tbl_openflow_interface_t *p_if);

int32
tbl_openflow_interface_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_openflow_interface_master_t*
tbl_openflow_interface_get_master();

tbl_openflow_interface_master_t*
tbl_openflow_interface_init_openflow_interface();

#endif /* !__TBL_OPENFLOW_INTERFACE_H__ */
